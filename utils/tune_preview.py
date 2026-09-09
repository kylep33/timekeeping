#!/usr/bin/env python3
"""Render Movement signal tunes to WAV files so you can audition them without
building firmware.

Every tune in movement_custom_signal_tunes.h is guarded by its own #ifdef and
they all declare the same symbol (signal_tune[]), so a given firmware build can
only contain one. Rather than doing a clean rebuild per tune just to hear it,
this reimplements the firmware's sequence player on the host.

The playback semantics mirror cb_watch_buzzer_seq() in
watch-library/hardware/watch/watch_tcc.c, which is driven by a 64 Hz timer, so
one duration unit is 15.625 ms. Note frequencies are read out of the doc
comments in watch-library/shared/watch/watch_tcc.h rather than hardcoded here,
so retuning the table upstream carries over.

Usage:
    utils/tune_preview.py                      # render every tune
    utils/tune_preview.py --list
    utils/tune_preview.py --only KIM_POSSIBLE MARIO_THEME
    utils/tune_preview.py --combined           # plus one back-to-back file
"""

import argparse
import math
import re
import struct
import sys
import wave
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
TUNES_H = REPO / "movement_custom_signal_tunes.h"
NOTES_H = REPO / "watch-library" / "shared" / "watch" / "watch_tcc.h"

# cb_watch_buzzer_seq() runs off a 64 Hz timer; one duration unit is one tick.
TICK_MS = 1000.0 / 64.0
SAMPLE_RATE = 44100

# watch_buzzer_play_sequence_with_volume() sets the PWM duty cycle to 5 or 25.
# Duty cycle isn't amplitude, but it tracks perceived loudness closely enough
# for picking a melody.
VOLUME_AMPLITUDE = {"soft": 0.18, "loud": 0.6}

# A note boundary with no ramp clicks audibly at these short durations.
RAMP_MS = 0.6

# The sequence player treats a note or duration of 0 as end-of-sequence, and
# guards against a malformed tune spinning forever.
MAX_EVENTS = 10000


def parse_notes(path):
    """Map BUZZER_NOTE_* -> (enum index, frequency in Hz).

    The enum is documented as `BUZZER_NOTE_A1,  ///< 55.00 Hz`, with
    BUZZER_NOTE_REST as `///< no sound`.
    """
    text = path.read_text()
    body = re.search(
        r"typedef enum \{(.*?)\} watch_buzzer_note_t;", text, re.S
    )
    if not body:
        sys.exit(f"could not find watch_buzzer_note_t enum in {path}")

    notes = {}
    for index, line in enumerate(
        re.findall(r"(BUZZER_NOTE_\w+)\s*,?\s*///<\s*(.+)", body.group(1))
    ):
        name, doc = line
        hz = re.match(r"([\d.]+)\s*Hz", doc.strip())
        notes[name] = (index, float(hz.group(1)) if hz else None)
    if not notes:
        sys.exit(f"parsed zero notes from {path}")
    return notes


def parse_tunes(path, notes):
    """Map tune name -> flat [value, duration, ...] list, as the firmware sees it.

    Note names become their enum index so that the negative repeat markers stay
    distinguishable from real notes.
    """
    text = path.read_text()
    tunes = {}
    for name, body in re.findall(
        r"#ifdef SIGNAL_TUNE_(\w+)\s*\n\s*int8_t signal_tune\[\]\s*=\s*\{(.*?)\};",
        text,
        re.S,
    ):
        body = re.sub(r"//.*?$|/\*.*?\*/", "", body, flags=re.S | re.M)
        seq = []
        for token in (t.strip() for t in body.split(",")):
            if not token:
                continue
            if token in notes:
                seq.append(notes[token][0])
            else:
                try:
                    seq.append(int(token, 0))
                except ValueError:
                    sys.exit(f"{name}: unrecognized token {token!r}")
        tunes[name] = seq
    if not tunes:
        sys.exit(f"parsed zero tunes from {path}")
    return tunes


def run_sequence(seq):
    """Walk a tune the way cb_watch_buzzer_seq() does, yielding (note, ticks).

    Faithful to the firmware including the repeat marker: a negative value with
    a nonzero partner means "rewind that many note/duration pairs and repeat".
    """
    pos = 0
    repeat_counter = -1
    for _ in range(MAX_EVENTS):
        if pos + 1 >= len(seq):
            return
        if seq[pos] < 0 and seq[pos + 1]:
            if repeat_counter == -1:
                repeat_counter = seq[pos + 1]
            else:
                repeat_counter -= 1
            if repeat_counter > 0:
                # Rewind, clamping at the start of the sequence.
                pos = pos + seq[pos] * 2 if pos > seq[pos] * -2 else 0
            else:
                pos += 2
                repeat_counter = -1
            if pos + 1 >= len(seq):
                return
        if seq[pos] and seq[pos + 1]:
            yield seq[pos], seq[pos + 1]
            pos += 2
        else:
            return
    print("  warning: hit event cap, tune may repeat forever", file=sys.stderr)


def oscillator(shape, phase):
    """One cycle of the requested waveform, phase in [0, 1)."""
    if shape == "sine":
        return math.sin(2 * math.pi * phase)
    if shape == "square":
        return 1.0 if phase < 0.5 else -1.0
    # Triangle, matching the simulator's Web Audio oscillator type.
    return 4 * abs(phase - 0.5) - 1


def render(seq, notes, index_to_freq, shape, amplitude, tick_ms):
    """Render a tune to a list of float samples in [-1, 1]."""
    samples = []
    ramp = max(1, int(SAMPLE_RATE * RAMP_MS / 1000.0))
    for note_index, ticks in run_sequence(seq):
        count = int(SAMPLE_RATE * ticks * tick_ms / 1000.0)
        freq = index_to_freq.get(note_index)
        if freq is None:
            # BUZZER_NOTE_REST, or a note with no documented frequency.
            samples.extend([0.0] * count)
            continue
        for i in range(count):
            value = oscillator(shape, (i * freq / SAMPLE_RATE) % 1.0) * amplitude
            # Ease the note in and out so boundaries don't click.
            if i < ramp:
                value *= i / ramp
            elif i > count - ramp:
                value *= max(0, count - i) / ramp
            samples.append(value)
    return samples


def write_wav(path, samples):
    frames = b"".join(
        struct.pack("<h", int(max(-1.0, min(1.0, s)) * 32767)) for s in samples
    )
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SAMPLE_RATE)
        w.writeframes(frames)


def main():
    ap = argparse.ArgumentParser(
        description="Render Movement signal tunes to WAV files."
    )
    ap.add_argument("--list", action="store_true", help="list tunes and exit")
    ap.add_argument("--only", nargs="+", metavar="NAME",
                    help="render only these tunes (name without SIGNAL_TUNE_)")
    ap.add_argument("--combined", action="store_true",
                    help="also write all-tunes.wav with every tune back to back")
    ap.add_argument("--volume", choices=sorted(VOLUME_AMPLITUDE), default="loud",
                    help="signal volume defaults to loud in movement_config.h")
    ap.add_argument("--wave", choices=("triangle", "square", "sine"),
                    default="triangle",
                    help="triangle matches the simulator; square is closer to "
                         "the PWM-driven piezo")
    ap.add_argument("--tick-ms", type=float, default=TICK_MS,
                    help=f"ms per duration unit (default {TICK_MS:.3f}, the 64 Hz timer)")
    ap.add_argument("-o", "--outdir", type=Path, default=REPO / "build-tunes")
    args = ap.parse_args()

    notes = parse_notes(NOTES_H)
    tunes = parse_tunes(TUNES_H, notes)
    index_to_freq = {i: hz for i, hz in notes.values() if hz is not None}

    if args.list:
        for name in sorted(tunes):
            ticks = sum(t for _, t in run_sequence(tunes[name]))
            print(f"  SIGNAL_TUNE_{name:<20} {ticks * args.tick_ms / 1000:5.2f}s")
        return

    selected = sorted(tunes)
    if args.only:
        unknown = [n for n in args.only if n.upper() not in tunes]
        if unknown:
            sys.exit(f"unknown tune(s): {', '.join(unknown)}\n"
                     f"available: {', '.join(sorted(tunes))}")
        selected = [n.upper() for n in args.only]

    args.outdir.mkdir(parents=True, exist_ok=True)
    amplitude = VOLUME_AMPLITUDE[args.volume]
    combined = []

    for name in selected:
        samples = render(tunes[name], notes, index_to_freq,
                         args.wave, amplitude, args.tick_ms)
        path = args.outdir / f"{name.lower()}.wav"
        write_wav(path, samples)
        print(f"  {path.relative_to(REPO)}  ({len(samples) / SAMPLE_RATE:.2f}s)")
        if args.combined:
            combined.extend(samples)
            combined.extend([0.0] * int(SAMPLE_RATE * 0.75))

    if args.combined:
        path = args.outdir / "all-tunes.wav"
        write_wav(path, combined)
        print(f"  {path.relative_to(REPO)}  ({len(combined) / SAMPLE_RATE:.2f}s) "
              f"<- {', '.join(n.lower() for n in selected)}")


if __name__ == "__main__":
    main()

/*
 * MIT License
 *
 * Copyright (c) 2023 Jeremy O'Brien
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <stdint.h>
#include "movement.h"

static int8_t signal_tune_default[] = {
    BUZZER_NOTE_C8, 5,
    BUZZER_NOTE_REST, 6,
    BUZZER_NOTE_C8, 5,
    0
};

static int8_t signal_tune_zelda_secret[] = {
    BUZZER_NOTE_G5, 8,
    BUZZER_NOTE_F5SHARP_G5FLAT, 8,
    BUZZER_NOTE_D5SHARP_E5FLAT, 8,
    BUZZER_NOTE_A4, 8,
    BUZZER_NOTE_G4SHARP_A4FLAT, 8,
    BUZZER_NOTE_E5, 8,
    BUZZER_NOTE_G5SHARP_A5FLAT, 8,
    BUZZER_NOTE_C6, 20,
    0
};

static int8_t signal_tune_mario_theme[] = {
    BUZZER_NOTE_E6, 7,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E6, 7,
    BUZZER_NOTE_REST, 10,
    BUZZER_NOTE_E6, 7,
    BUZZER_NOTE_REST, 11,
    BUZZER_NOTE_C6, 7,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 7,
    BUZZER_NOTE_REST, 10,
    BUZZER_NOTE_G6, 8,
    BUZZER_NOTE_REST, 30,
    BUZZER_NOTE_G5, 8,
    0
};

static int8_t signal_tune_mgs_codec[] = {
    BUZZER_NOTE_G5SHARP_A5FLAT, 1,
    BUZZER_NOTE_C6, 1,
    BUZZER_NOTE_G5SHARP_A5FLAT, 1,
    BUZZER_NOTE_C6, 1,
    BUZZER_NOTE_G5SHARP_A5FLAT, 1,
    BUZZER_NOTE_C6, 1,
    BUZZER_NOTE_G5SHARP_A5FLAT, 1,
    BUZZER_NOTE_C6, 1,
    BUZZER_NOTE_G5SHARP_A5FLAT, 1,
    BUZZER_NOTE_C6, 1,
    BUZZER_NOTE_REST, 6,
    BUZZER_NOTE_G5SHARP_A5FLAT, 1,
    BUZZER_NOTE_C6, 1,
    BUZZER_NOTE_G5SHARP_A5FLAT, 1,
    BUZZER_NOTE_C6, 1,
    BUZZER_NOTE_G5SHARP_A5FLAT, 1,
    BUZZER_NOTE_C6, 1,
    BUZZER_NOTE_G5SHARP_A5FLAT, 1,
    BUZZER_NOTE_C6, 1,
    BUZZER_NOTE_G5SHARP_A5FLAT, 1,
    BUZZER_NOTE_C6, 1,
    0
};

static int8_t signal_tune_kim_possible[] = {
    BUZZER_NOTE_G7, 6,
    BUZZER_NOTE_G4, 2,
    BUZZER_NOTE_REST, 5,
    BUZZER_NOTE_G7, 6,
    BUZZER_NOTE_G4, 2,
    BUZZER_NOTE_REST, 5,
    BUZZER_NOTE_A7SHARP_B7FLAT, 6,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_G7, 6,
    BUZZER_NOTE_G4, 2,
    0
};

static int8_t signal_tune_power_rangers[] = {
    BUZZER_NOTE_D8, 6,
    BUZZER_NOTE_REST, 8,
    BUZZER_NOTE_D8, 6,
    BUZZER_NOTE_REST, 8,
    BUZZER_NOTE_C8, 6,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_D8, 6,
    BUZZER_NOTE_REST, 8,
    BUZZER_NOTE_F8, 6,
    BUZZER_NOTE_REST, 8,
    BUZZER_NOTE_D8, 6,
    0
};

static int8_t signal_tune_layla[] = {
    BUZZER_NOTE_A6, 5,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_C7, 5,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_D7, 5,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F7, 5,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_D7, 5,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_C7, 5,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_D7, 20,
    0
};

static int8_t signal_tune_harry_potter_short[] = {
    BUZZER_NOTE_B5, 12,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 12,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_G6, 6,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F6SHARP_G6FLAT, 6,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 16,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B6, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A6, 24,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F6SHARP_G6FLAT, 24,
    0
};

static int8_t signal_tune_harry_potter_long[] = {
    BUZZER_NOTE_B5, 12,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 12,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_G6, 6,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F6SHARP_G6FLAT, 6,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 16,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B6, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A6, 24,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F6SHARP_G6FLAT, 24,
    BUZZER_NOTE_REST, 1,

    BUZZER_NOTE_E6, 12,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_G6, 6,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F6SHARP_G6FLAT, 6,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_D6SHARP_E6FLAT, 16,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F6, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 24,

    0
};

static int8_t signal_tune_jurassic_park[] = {
    BUZZER_NOTE_B5, 7,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_A5SHARP_B5FLAT, 7,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_B5, 13,
    BUZZER_NOTE_REST, 13,
    BUZZER_NOTE_F5SHARP_G5FLAT, 13,
    BUZZER_NOTE_REST, 13,
    BUZZER_NOTE_E5, 13,
    BUZZER_NOTE_REST, 13,
    BUZZER_NOTE_B5, 7,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_A5SHARP_B5FLAT, 7,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_B5, 13,
    BUZZER_NOTE_REST, 13,
    BUZZER_NOTE_F5SHARP_G5FLAT, 13,
    BUZZER_NOTE_REST, 13,
    BUZZER_NOTE_E5, 13,
    0,
};

static int8_t signal_tune_evangelion[] = {
    BUZZER_NOTE_C5, 13,
    BUZZER_NOTE_REST, 13,
    BUZZER_NOTE_D5SHARP_E5FLAT, 13,
    BUZZER_NOTE_REST, 13,
    BUZZER_NOTE_F5, 13,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_D5SHARP_E5FLAT, 13,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_F5, 7,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_F5, 7,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_F5, 7,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_A5SHARP_B5FLAT, 7,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_G5SHARP_A5FLAT, 7,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_G5, 3,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_F5, 7,
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_G5, 13,
    0,
};

/* Unlike the rest of this file, the tunes below are transcribed from a cited
 * source rather than from memory: an RTTTL ringtone, a published Arduino
 * melody sourced from a MuseScore transcription, or a note tab, with the
 * source named in each comment. Durations are converted from that source's
 * own tempo into ticks (real ms / 15.625), so the pacing is the song's actual
 * pacing rather than a guess - split roughly 85/15 into note-on and a rest,
 * so repeated notes at the same pitch are still heard as separate hits.
 */

// Nokia RTTTL "CrazyFrog" (o=6, b=125): the full riff, one time through.
static int8_t signal_tune_axel_f[] = {
    BUZZER_NOTE_F5, 27,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_G5SHARP_A5FLAT, 20,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_F5, 7,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_REST, 8,
    BUZZER_NOTE_F5, 7,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A5SHARP_B5FLAT, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_D5SHARP_E5FLAT, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5, 27,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_C6, 20,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_F5, 7,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_REST, 8,
    BUZZER_NOTE_F5, 7,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_C6SHARP_D6FLAT, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_C6, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_G5SHARP_A5FLAT, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_C6, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F6, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5, 7,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_D5SHARP_E5FLAT, 7,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_REST, 8,
    BUZZER_NOTE_D5SHARP_E5FLAT, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_C5, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_G5, 13,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5, 27,
    0
};

/* Read from the user's "Raiders March" sheet music: cut time, march tempo
 * 120 - the half note carries the beat, so a whole note is 1000ms at strict
 * tempo. Eased back to 1600ms so the fast pickup notes stay audible instead
 * of blurring. Opens E5-F5-G5 into a held C6, then D5-E5 into a held F5 -
 * then the next phrase: the same climb-into-a-hold shape repeated a fifth
 * higher (G5-A5-B5 into a held F6), followed by a short walking line
 * (A5-B5-C6-D6) up to a final E6.
 */
static int8_t signal_tune_indiana_jones[] = {
    BUZZER_NOTE_E5, 16,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_F5, 5,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_G5, 3,
    BUZZER_NOTE_REST, 10,
    BUZZER_NOTE_C6, 24,
    BUZZER_NOTE_REST, 5,
    BUZZER_NOTE_D5, 16,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_E5, 5,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F5, 38,
    BUZZER_NOTE_REST, 6,

    BUZZER_NOTE_G5, 16,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_A5, 5,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 3,
    BUZZER_NOTE_REST, 10,
    BUZZER_NOTE_F6, 24,
    BUZZER_NOTE_REST, 5,
    BUZZER_NOTE_A5, 16,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_B5, 5,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_C6, 22,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_D6, 22,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_E6, 20,
    0
};

// RTTTL "Simpsons" (d=4, o=5, b=160): the full opening stanza, hook and answer.
static int8_t signal_tune_simpsons[] = {
    BUZZER_NOTE_C6, 31,
    BUZZER_NOTE_REST, 5,
    BUZZER_NOTE_E6, 20,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_F6SHARP_G6FLAT, 20,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_A6, 10,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_G6, 31,
    BUZZER_NOTE_REST, 5,
    BUZZER_NOTE_E6, 20,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_C6, 20,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_A5, 10,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5SHARP_G5FLAT, 10,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5SHARP_G5FLAT, 10,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5SHARP_G5FLAT, 10,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_G5, 48,
    0
};

/* robsoncouto/arduino-songs pacman.ino (tempo 105, sourced from a MuseScore
 * transcription): both phrases of the "waka waka" intro, the whole thing.
 * The second phrase's closing run is dropped an octave from the source - at
 * the written pitch it was shriller than the rest of the tune.
 */
static int8_t signal_tune_pac_man[] = {
    BUZZER_NOTE_B4, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F5SHARP_G5FLAT, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_D5SHARP_E5FLAT, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F5SHARP_G5FLAT, 12,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_D5SHARP_E5FLAT, 15,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_C5, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_C6, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_G6, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_C6, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_G6, 12,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E6, 15,
    BUZZER_NOTE_REST, 3,

    BUZZER_NOTE_B4, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F5SHARP_G5FLAT, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_D5SHARP_E5FLAT, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F5SHARP_G5FLAT, 12,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_D5SHARP_E5FLAT, 15,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_D5SHARP_E5FLAT, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E4, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F4, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F4, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_F4SHARP_G4FLAT, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_G4, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_G4, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_G4SHARP_A4FLAT, 4,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A4, 8,
    BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B4, 15,
    0
};

/* Pitches straight from noobnotes.net's own letter-note text for this song
 * ("G-^D-B B A G" / "G ^C B B-A A G" / "G ^C B B-A A G A E-D" for these three
 * lyric lines) - not their featured image, which is a different section of
 * the song. That source's "me...I ain't" is a repeated G before the rising
 * fourth up to "ain't"; this array used to collapse "me" and "I" into one
 * note, which shifted every pitch after it onto the wrong syllable.
 *
 * Real tempo is 104 BPM (songbpm.com), so a sixteenth note is ~9 ticks at the
 * 64 Hz timer; the verse is sung almost like spoken word on that grid, with
 * the multi-syllable words ("some-bo-dy", "gon-na", "sharp-est") compressed
 * to sixteenths against the surrounding eighth notes, a quarter-note pause
 * at the line break ("roll ME, I ain't..."), and a held half note on the
 * final "shed" (itself a two-note E-D slide in the source).
 */
static int8_t signal_tune_all_star[] = {
    BUZZER_NOTE_G4, 14,            // some
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_D5, 7,             // -bo-
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B4, 7,             // -dy
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B4, 14,            // once
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_A4, 14,            // told
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_G4, 14,            // me
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_G4, 14,            // the
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_C5, 14,            // world
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_B4, 14,            // is
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_B4, 7,             // gon-
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A4, 7,             // -na
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A4, 14,            // roll
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_G4, 29,            // me (line break, held)
    BUZZER_NOTE_REST, 7,
    BUZZER_NOTE_G4, 14,            // i
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_C5, 14,            // ain't (rising fourth from "i")
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_B4, 14,            // the
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_B4, 7,             // sharp-
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A4, 7,             // -est
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A4, 14,            // tool
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_G4, 14,            // in
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_A4, 14,            // the
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_E4, 14,            // shed (slide start)
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_D4, 72,            // shed (held)
    0
};

/* robsoncouto/arduino-songs imperialmarch.ino (tempo 120, sourced from a
 * MuseScore tenor-sax transcription): not the famous opening call, but the
 * syncopated turn right after it ("A5, A4-A4, A5, G#5-G5, D#5-D5-D#5...")
 * through to its cadence - real tempo, no extra scaling needed since nothing
 * in this stretch is a long held note.
 */
static int8_t signal_tune_imperial_march[] = {
    BUZZER_NOTE_A5, 28,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_A4, 21,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_A4, 6,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A5, 28,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_G5SHARP_A5FLAT, 21,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_G5, 6,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_D5SHARP_E5FLAT, 6,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_D5, 6,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_D5SHARP_E5FLAT, 14,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_REST, 16,
    BUZZER_NOTE_A4, 14,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_D5SHARP_E5FLAT, 28,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_D5, 21,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_C5SHARP_D5FLAT, 6,
    BUZZER_NOTE_REST, 2,

    BUZZER_NOTE_C5, 6,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B4, 6,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_C5, 6,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_REST, 16,
    BUZZER_NOTE_F4, 14,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_G4SHARP_A4FLAT, 28,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_F4, 21,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_A4, 10,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_C5, 28,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_A4, 21,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_C5, 6,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E5, 64,
    0
};

static int8_t *const signal_tunes[SIGNAL_TUNE_COUNT] = {
    [SIGNAL_TUNE_DEFAULT] = signal_tune_default,
    [SIGNAL_TUNE_ZELDA_SECRET] = signal_tune_zelda_secret,
    [SIGNAL_TUNE_MARIO_THEME] = signal_tune_mario_theme,
    [SIGNAL_TUNE_MGS_CODEC] = signal_tune_mgs_codec,
    [SIGNAL_TUNE_KIM_POSSIBLE] = signal_tune_kim_possible,
    [SIGNAL_TUNE_POWER_RANGERS] = signal_tune_power_rangers,
    [SIGNAL_TUNE_LAYLA] = signal_tune_layla,
    [SIGNAL_TUNE_HARRY_POTTER_SHORT] = signal_tune_harry_potter_short,
    [SIGNAL_TUNE_HARRY_POTTER_LONG] = signal_tune_harry_potter_long,
    [SIGNAL_TUNE_JURASSIC_PARK] = signal_tune_jurassic_park,
    [SIGNAL_TUNE_EVANGELION] = signal_tune_evangelion,
    [SIGNAL_TUNE_AXEL_F] = signal_tune_axel_f,
    [SIGNAL_TUNE_INDIANA_JONES] = signal_tune_indiana_jones,
    [SIGNAL_TUNE_SIMPSONS] = signal_tune_simpsons,
    [SIGNAL_TUNE_PAC_MAN] = signal_tune_pac_man,
    [SIGNAL_TUNE_ALL_STAR] = signal_tune_all_star,
    [SIGNAL_TUNE_IMPERIAL_MARCH] = signal_tune_imperial_march,
};

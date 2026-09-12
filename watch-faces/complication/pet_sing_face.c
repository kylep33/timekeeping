/*
 * MIT License
 *
 * Copyright (c) 2022 Joey Castillo
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

#include <stdlib.h>
#include <string.h>
#include "pet_sing_face.h"
#include "watch_common_display.h"

/* A resting face doesn't need ticks. A singing one runs at 64Hz - the same rate the
 * buzzer hardware itself steps through a sequence's durations at - so the mouth can
 * be read off the tune's own note data one tick at a time, tightly, instead of just
 * bobbing on a beat of its own.
 */
static const uint8_t IDLE_TICK_FREQUENCY_HZ = 1;
static const uint8_t SINGING_TICK_FREQUENCY_HZ = 64;

/* Six characters each, named after whichever tune they pick out of
 * movement_custom_signal_tunes.h. Keep this in step with signal_tune_index_t.
 *
 * A commented-out entry mutes that tune rather than crashing: _random_tune()
 * below only picks among the ones with a name.
 */
static const char *TUNE_NAMES[SIGNAL_TUNE_COUNT] = {
    // [SIGNAL_TUNE_DEFAULT]            = "CHIRP ",
    [SIGNAL_TUNE_ZELDA_SECRET]       = "ZELDA ",
    [SIGNAL_TUNE_MARIO_THEME]        = "MARIO ",
    // [SIGNAL_TUNE_MGS_CODEC]          = "MGS   ",
    [SIGNAL_TUNE_KIM_POSSIBLE]       = "KIM   ",
    [SIGNAL_TUNE_POWER_RANGERS]      = "RANGER",
    // [SIGNAL_TUNE_LAYLA]              = "LAYLA ",
    // [SIGNAL_TUNE_HARRY_POTTER_SHORT] = "HP SHT",
    [SIGNAL_TUNE_HARRY_POTTER_LONG]  = "HP LNG",
    [SIGNAL_TUNE_JURASSIC_PARK]      = "JURASC",
    [SIGNAL_TUNE_EVANGELION]         = "EVA   ",
    [SIGNAL_TUNE_AXEL_F]             = "AXEL F",
    [SIGNAL_TUNE_INDIANA_JONES]      = "INDY  ",
    [SIGNAL_TUNE_SIMPSONS]           = "SIMPSN",
    [SIGNAL_TUNE_PAC_MAN]            = "PACMAN",
    [SIGNAL_TUNE_ALL_STAR]           = "ALLSTR",
    [SIGNAL_TUNE_IMPERIAL_MARCH]     = "VADER ",
};

/* Walks a tune's own [note, duration, ...] pairs exactly the way cb_watch_buzzer_seq()
 * does in watch_tcc.c, to find whichever note is sounding at a given tick - so the
 * mouth opens and shuts on the tune's actual notes and rests rather than a guessed
 * rhythm. Doesn't handle that player's negative repeat markers, since no signal tune
 * here uses one.
 */
static bool _note_sounding(const int8_t *sequence, uint16_t elapsed_ticks) {
    uint16_t position = 0;

    if (sequence == NULL) return false;

    while (sequence[position] && sequence[position + 1]) {
        uint8_t duration = (uint8_t) sequence[position + 1];
        if (elapsed_ticks < duration) return sequence[position] != BUZZER_NOTE_REST;
        elapsed_ticks -= duration;
        position += 2;
    }

    return false;
}

/* SING sits on the big digits until a press starts a song, at which point the word
 * gives way to the pet itself, mouth opening and shutting right on the tune's own
 * notes, in whatever column it's currently standing in. SING comes back the moment
 * the tune actually stops sounding.
 */
static void _redraw(pet_sing_face_state_t *state) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "   ", "  ");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    if (state->singing) {
        char row[PET_ROW_LENGTH + 1];
        bool mouth_open = _note_sounding(movement_get_signal_tune(state->tune), state->elapsed_ticks);

        pet_row_clear(row);
        pet_row_place(row, pet_position(), pet_sing_sprite(mouth_open ? 1 : 0));
        watch_set_indicator(WATCH_INDICATOR_BELL);
        watch_display_text(WATCH_POSITION_BOTTOM, row);
    } else {
        watch_clear_indicator(WATCH_INDICATOR_BELL);
        watch_display_text(WATCH_POSITION_BOTTOM, "SING  ");
    }
}

// Skips any tune whose name above is commented out, rather than picking a name that isn't there.
static uint8_t _random_tune(void) {
    uint8_t available[SIGNAL_TUNE_COUNT];
    uint8_t count = 0;

    for (uint8_t i = 0; i < SIGNAL_TUNE_COUNT; i++) {
        if (TUNE_NAMES[i] != NULL) available[count++] = i;
    }

    return available[rand() % count];
}

static void _sing(pet_sing_face_state_t *state) {
    const pet_t *pet = pet_get();

    if (pet->dead) return;

    // Waking it up for a song is still waking it up, so it gets the same grumble as being fed.
    if (pet->asleep) {
        pet_disturb();
        return;
    }

    pet_sing();

    // A fresh spot for every performance, rather than wherever the last face left it standing.
    pet_set_position(rand() % (PET_ROW_LENGTH - PET_SPRITE_WIDTH + 1));

    state->tune = _random_tune();
    movement_play_signal_tune(state->tune);
    state->singing = true;
    state->elapsed_ticks = 0;
    movement_request_tick_frequency(SINGING_TICK_FREQUENCY_HZ);
}

void pet_sing_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr != NULL) return;

    *context_ptr = malloc(sizeof(pet_sing_face_state_t));
    memset(*context_ptr, 0, sizeof(pet_sing_face_state_t));
}

void pet_sing_face_activate(void *context) {
    pet_sing_face_state_t *state = (pet_sing_face_state_t *) context;

    state->singing = false;
    state->elapsed_ticks = 0;
    movement_request_tick_frequency(IDLE_TICK_FREQUENCY_HZ);
}

bool pet_sing_face_loop(movement_event_t event, void *context) {
    pet_sing_face_state_t *state = (pet_sing_face_state_t *) context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_LOW_ENERGY_UPDATE:
            _redraw(state);
            break;
        case EVENT_TICK:
            if (state->singing) {
                state->elapsed_ticks++;
                // The sequence player tells us itself when the tune has actually finished.
                if (!movement_is_buzzing()) {
                    state->singing = false;
                    movement_request_tick_frequency(IDLE_TICK_FREQUENCY_HZ);
                }
            }
            _redraw(state);
            break;
        case EVENT_ALARM_BUTTON_UP:
            _sing(state);
            _redraw(state);
            break;
        case EVENT_TIMEOUT:
            movement_move_to_resting_face();
            break;
        default:
            return movement_default_loop_handler(event);
    }

    return true;
}

void pet_sing_face_resign(void *context) {
    (void) context;
    watch_clear_indicator(WATCH_INDICATOR_BELL);
}

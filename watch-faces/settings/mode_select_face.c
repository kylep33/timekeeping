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

#include <stdio.h>
#include <stdlib.h>
#include "mode_select_face.h"
#include "watch_common_display.h"

// Long enough to read the name and press again, short enough that a pocket press settles quickly.
static const uint8_t COMMIT_TIMEOUT_S = 5;

// Two ticks per second lets the name blink without burning power on a faster rate.
static const uint8_t TICK_FREQUENCY_HZ = 2;

typedef struct {
    uint8_t highlighted_mode;
    uint8_t seconds_until_commit;
} mode_select_state_t;

/* Padded to the full six cells: a name shorter than whatever was on screen before it
 * (PET landing right after GAME, say) would otherwise leave a stray trailing letter.
 */
static void _display_mode(mode_select_state_t *state, bool visible) {
    char buf[7];

    watch_display_text_with_fallback(WATCH_POSITION_TOP, "MODE", "MO");
    snprintf(buf, sizeof(buf), "%-6s", visible ? movement_mode_name(state->highlighted_mode) : "");
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

static void _commit(mode_select_state_t *state) {
    movement_set_mode(state->highlighted_mode);
}

void mode_select_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) *context_ptr = malloc(sizeof(mode_select_state_t));
}

void mode_select_face_activate(void *context) {
    mode_select_state_t *state = (mode_select_state_t *) context;

    state->highlighted_mode = movement_get_mode();
    state->seconds_until_commit = COMMIT_TIMEOUT_S;
    movement_request_tick_frequency(TICK_FREQUENCY_HZ);
}

bool mode_select_face_loop(movement_event_t event, void *context) {
    mode_select_state_t *state = (mode_select_state_t *) context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            _display_mode(state, true);
            break;
        case EVENT_TICK:
            if (event.subsecond == 0 && state->seconds_until_commit > 0) state->seconds_until_commit--;
            if (state->seconds_until_commit == 0) {
                _commit(state);
                break;
            }
            _display_mode(state, event.subsecond == 0);
            break;
        case EVENT_MODE_BUTTON_UP:
            state->highlighted_mode = (state->highlighted_mode + 1) % movement_num_modes();
            state->seconds_until_commit = COMMIT_TIMEOUT_S;
            _display_mode(state, true);
            break;
        case EVENT_MODE_LONG_PRESS:
        case EVENT_TIMEOUT:
            _commit(state);
            break;
        default:
            return movement_default_loop_handler(event);
    }

    return false;
}

void mode_select_face_resign(void *context) {
    (void) context;
    movement_request_tick_frequency(1);
}

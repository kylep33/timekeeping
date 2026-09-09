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
#include "pet_play_face.h"
#include "watch_common_display.h"

// Four ticks per second, so the stick crosses the row at a readable pace.
static const uint8_t TICK_FREQUENCY_HZ = 4;

// Poking is the one entry the pet face handles itself; the rest are games.
static const uint8_t POKE_ENTRY = 0;
static const char POKE_NAME[] = "POKE  ";
static const char POKE_PROP[] = "l";

static uint8_t _num_entries(void) {
    return 1 + pet_num_games;
}

static const char *_entry_name(uint8_t selection) {
    if (selection == POKE_ENTRY) return POKE_NAME;

    return pet_games[selection - 1].name;
}

static void _redraw(pet_play_face_state_t *state) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "PLY", "PL");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    if (state->poke.running) pet_approach_draw(&state->poke, POKE_PROP, state->tick);
    else watch_display_text(WATCH_POSITION_BOTTOM, _entry_name(state->selection));
}

static void _select(pet_play_face_state_t *state) {
    const pet_t *pet = pet_get();

    // A game is worth playing whatever state the pet is in, so it is offered first.
    if (state->selection != POKE_ENTRY) {
        movement_move_to_face(pet_games[state->selection - 1].face_index);
        return;
    }

    if (pet->dead) return;

    if (pet->asleep) {
        pet_disturb();
        return;
    }

    pet_approach_start(&state->poke);
}

void pet_play_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr != NULL) return;

    *context_ptr = malloc(sizeof(pet_play_face_state_t));
    memset(*context_ptr, 0, sizeof(pet_play_face_state_t));
}

void pet_play_face_activate(void *context) {
    pet_play_face_state_t *state = (pet_play_face_state_t *) context;

    state->poke.running = false;
    movement_request_tick_frequency(TICK_FREQUENCY_HZ);
}

bool pet_play_face_loop(movement_event_t event, void *context) {
    pet_play_face_state_t *state = (pet_play_face_state_t *) context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_LOW_ENERGY_UPDATE:
            _redraw(state);
            break;
        case EVENT_TICK:
            state->tick++;
            if (pet_approach_advance(&state->poke)) pet_poke();
            _redraw(state);
            break;
        case EVENT_ALARM_BUTTON_UP:
            if (!state->poke.running) state->selection = (state->selection + 1) % _num_entries();
            _redraw(state);
            break;
        case EVENT_ALARM_LONG_PRESS:
            if (!state->poke.running) _select(state);
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

void pet_play_face_resign(void *context) {
    (void) context;
    movement_request_tick_frequency(1);
}

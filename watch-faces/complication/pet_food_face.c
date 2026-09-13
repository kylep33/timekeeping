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
#include "pet_food_face.h"
#include "watch_common_display.h"

// Four ticks per second, so the food crosses the row at a readable pace.
static const uint8_t TICK_FREQUENCY_HZ = 4;

typedef struct {
    const char *name;       ///< six characters, and no glyph the bottom row cannot draw
    const char *sprite;     ///< the single character that rolls across the row
    uint8_t nutrition;
} pet_food_t;

static const pet_food_t FOODS[] = {
    { "SNACK ", "o", 15 },
    { "APPLE ", "O", 30 },
    { "CAKE  ", "@", 50 },
};

static const uint8_t NUM_FOODS = sizeof(FOODS) / sizeof(pet_food_t);

static void _redraw(pet_food_face_state_t *state) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "EAT", "EA");
    pet_stat_draw(pet_get()->hunger);

    if (state->serving.running) {
        pet_approach_draw(&state->serving, FOODS[state->selection].sprite);
    } else {
        watch_display_text(WATCH_POSITION_BOTTOM, FOODS[state->selection].name);
    }
}

static void _select(pet_food_face_state_t *state) {
    const pet_t *pet = pet_get();

    if (pet->dead) return;

    // A midnight snack means getting it out of bed, which it charges you for.
    if (pet->asleep) pet_disturb();

    pet_approach_start(&state->serving, pet_position());
}

void pet_food_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr != NULL) return;

    *context_ptr = malloc(sizeof(pet_food_face_state_t));
    memset(*context_ptr, 0, sizeof(pet_food_face_state_t));
}

void pet_food_face_activate(void *context) {
    pet_food_face_state_t *state = (pet_food_face_state_t *) context;

    state->serving.running = false;
    movement_request_tick_frequency(TICK_FREQUENCY_HZ);
}

bool pet_food_face_loop(movement_event_t event, void *context) {
    pet_food_face_state_t *state = (pet_food_face_state_t *) context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_LOW_ENERGY_UPDATE:
            _redraw(state);
            break;
        case EVENT_TICK:
            if (pet_approach_advance(&state->serving)) pet_feed(FOODS[state->selection].nutrition);
            _redraw(state);
            break;
        case EVENT_ALARM_BUTTON_UP:
            if (!state->serving.running) state->selection = (state->selection + 1) % NUM_FOODS;
            _redraw(state);
            break;
        case EVENT_ALARM_LONG_PRESS:
            if (!state->serving.running) _select(state);
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

void pet_food_face_resign(void *context) {
    (void) context;
    movement_request_tick_frequency(1);
}

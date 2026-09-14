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
#include "pet_screen.h"
#include "pet_species.h"
#include "watch_common_display.h"

// Four ticks per second, so the food crosses the row at a readable pace.
static const uint8_t TICK_FREQUENCY_HZ = 4;

// Long enough that nobody kills their pet by accident.
static const uint8_t KILL_HOLD_S = 5;
static const watch_buzzer_note_t HOLD_BEEP_NOTE = BUZZER_NOTE_C7;
static const uint16_t HOLD_BEEP_MS = 60;

typedef struct {
    const char *name;       ///< six characters, and no glyph the bottom row cannot draw
    const char *sprite;     ///< the single character that rolls across the row
    uint8_t nutrition;
    bool is_gun;
} pet_menu_item_t;

static const pet_menu_item_t MENU_ITEMS[] = {
    { "SNACK ", "o", 15, false },
    { "APPLE ", "O", 30, false },
    { "CAKE  ", "@", 50, false },
    { "GUN   ", NULL, 0, true },
};

static const uint8_t NUM_MENU_ITEMS = sizeof(MENU_ITEMS) / sizeof(pet_menu_item_t);

// The question doesn't fit on one row, so it flips halves every second.
static const char *_confirm_text(void) {
    return movement_get_local_date_time().unit.second % 2 == 0 ? "KILL  " : "PET?  ";
}

static void _draw_confirm(pet_food_face_state_t *state) {
    watch_clear_colon();
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "GUN", "GU");

    if (state->holding) pet_screen_stat_draw(KILL_HOLD_S - state->hold_ticks / TICK_FREQUENCY_HZ);
    else watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    watch_display_text(WATCH_POSITION_BOTTOM, _confirm_text());
}

static void _redraw(pet_food_face_state_t *state) {
    const pet_menu_item_t *item = &MENU_ITEMS[state->selection];

    if (state->mode == PET_FOOD_FACE_CONFIRMING) {
        _draw_confirm(state);
        return;
    }

    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "EAT", "EA");
    pet_screen_stat_draw(pet_get()->hunger);

    switch (state->mode) {
        case PET_FOOD_FACE_EATING:
            pet_species_current()->eat_draw(item->sprite);
            break;
        case PET_FOOD_FACE_MURDERING:
            pet_species_current()->murder_draw();
            break;
        default:
            watch_clear_colon();
            watch_display_text(WATCH_POSITION_BOTTOM, item->name);
            break;
    }
}

static void _advance_eating(pet_food_face_state_t *state) {
    pet_anim_step_t step = pet_species_current()->eat_advance();

    if (step == PET_ANIM_IMPACT) pet_feed(MENU_ITEMS[state->selection].nutrition);
    if (step == PET_ANIM_DONE) state->mode = PET_FOOD_FACE_MENU;
}

static void _advance_hold(pet_food_face_state_t *state) {
    if (!state->holding) return;

    state->hold_ticks++;
    if (state->hold_ticks % TICK_FREQUENCY_HZ != 0) return;

    if (state->hold_ticks < KILL_HOLD_S * TICK_FREQUENCY_HZ) {
        movement_play_note(HOLD_BEEP_NOTE, HOLD_BEEP_MS);
        return;
    }

    state->holding = false;
    state->mode = PET_FOOD_FACE_MURDERING;
    pet_species_current()->murder_start();
}

static void _advance_murder(void) {
    pet_anim_step_t step = pet_species_current()->murder_advance();

    if (step == PET_ANIM_IMPACT) pet_murder();
    // The grave is on the home face, so that's where it ends.
    if (step == PET_ANIM_DONE) movement_move_to_resting_face();
}

static void _advance(pet_food_face_state_t *state) {
    switch (state->mode) {
        case PET_FOOD_FACE_EATING:
            _advance_eating(state);
            break;
        case PET_FOOD_FACE_CONFIRMING:
            _advance_hold(state);
            break;
        case PET_FOOD_FACE_MURDERING:
            _advance_murder();
            break;
        default:
            break;
    }
}

static void _select(pet_food_face_state_t *state) {
    const pet_t *pet = pet_get();

    if (pet->dead) return;

    if (MENU_ITEMS[state->selection].is_gun) {
        state->mode = PET_FOOD_FACE_CONFIRMING;
        return;
    }

    // A midnight snack means getting it out of bed, which it charges you for.
    if (pet->asleep) pet_disturb();

    pet_species_current()->eat_start();
    state->mode = PET_FOOD_FACE_EATING;
}

static void _handle_tap(pet_food_face_state_t *state) {
    if (state->mode == PET_FOOD_FACE_MENU) {
        state->selection = (state->selection + 1) % NUM_MENU_ITEMS;
        return;
    }

    if (state->mode != PET_FOOD_FACE_CONFIRMING) return;

    state->holding = false;
    state->mode = PET_FOOD_FACE_MENU;
}

// Counts from the press rather than the long press, so it's five real seconds of holding.
static void _start_hold(pet_food_face_state_t *state) {
    if (state->mode != PET_FOOD_FACE_CONFIRMING) return;

    state->holding = true;
    state->hold_ticks = 0;
}

void pet_food_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr != NULL) return;

    *context_ptr = malloc(sizeof(pet_food_face_state_t));
    memset(*context_ptr, 0, sizeof(pet_food_face_state_t));
}

void pet_food_face_activate(void *context) {
    pet_food_face_state_t *state = (pet_food_face_state_t *) context;

    state->mode = PET_FOOD_FACE_MENU;
    state->holding = false;
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
            _advance(state);
            _redraw(state);
            break;
        case EVENT_ALARM_BUTTON_DOWN:
            _start_hold(state);
            _redraw(state);
            break;
        case EVENT_ALARM_BUTTON_UP:
            _handle_tap(state);
            _redraw(state);
            break;
        case EVENT_ALARM_LONG_PRESS:
            if (state->mode == PET_FOOD_FACE_MENU) _select(state);
            _redraw(state);
            break;
        case EVENT_ALARM_LONG_UP:
            state->holding = false;
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

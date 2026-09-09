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
#include <stdio.h>
#include <string.h>
#include "pet_face.h"
#include "watch_common_display.h"

// Two ticks per second is enough for a creature that ambles rather than runs.
static const uint8_t TICK_FREQUENCY_HZ = 2;

static const uint8_t WANDER_CHOICES = 3;
static const uint8_t MAX_POSITION = PET_ROW_LENGTH - PET_SPRITE_WIDTH;

// The off frame is a blink, so it should be a flicker rather than half the pet's life.
static const uint8_t BLINK_EVERY_N_TICKS = 8;

// One stat per second is slow enough to read while the light button is held down.
static const uint8_t TICKS_PER_STAT = 2;

// The top right holds two digits, so longer lived pets and later generations wrap.
static const uint16_t TOP_RIGHT_WRAP = 100;

// The bottom row spends three characters on the label, leaving three for the age.
static const uint16_t GRAVE_AGE_WRAP = 1000;

typedef enum {
    PET_STAT_HUNGER,
    PET_STAT_HAPPINESS,
    PET_STAT_HEALTH,
    PET_STAT_AGE,
    PET_STAT_COUNT,
} pet_stat_t;

typedef struct {
    const char *label;
    const char *fallback;
} pet_stat_label_t;

static const pet_stat_label_t STAT_LABELS[PET_STAT_COUNT] = {
    [PET_STAT_HUNGER]    = { "HUN", "HU" },
    [PET_STAT_HAPPINESS] = { "HAP", "HA" },
    [PET_STAT_HEALTH]    = { "HEA", "HE" },
    [PET_STAT_AGE]       = { "AGE", "AG" },
};

static uint16_t _stat_value(const pet_t *pet, pet_stat_t stat) {
    switch (stat) {
        case PET_STAT_HUNGER:
            return pet->hunger;
        case PET_STAT_HAPPINESS:
            return pet->happiness;
        case PET_STAT_HEALTH:
            return pet->health;
        default:
            return pet_age_days();
    }
}

// Three ways to move, so the pet drifts about rather than marching wall to wall.
static uint8_t _wander(uint8_t position) {
    uint8_t step = rand() % WANDER_CHOICES;

    if (step == 0 && position > 0) return position - 1;
    if (step == 2 && position < MAX_POSITION) return position + 1;

    return position;
}

static uint8_t _frame_for(pet_mood_t mood, uint8_t tick) {
    // A pet this close to death should be hard to ignore, so it flashes every tick.
    if (mood == PET_MOOD_CRITICAL) return tick % PET_ANIMATION_FRAMES;

    return (tick % BLINK_EVERY_N_TICKS == 0) ? 1 : 0;
}

static void _update_indicators(pet_mood_t mood) {
    bool needs_help = mood == PET_MOOD_HUNGRY || mood == PET_MOOD_SICK || mood == PET_MOOD_CRITICAL;

    if (mood == PET_MOOD_ASLEEP) watch_set_indicator(WATCH_INDICATOR_SLEEP);
    else watch_clear_indicator(WATCH_INDICATOR_SLEEP);

    if (needs_help) watch_set_indicator(WATCH_INDICATOR_SIGNAL);
    else watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
}

static const char *_current_sprite(pet_face_state_t *state, pet_mood_t mood) {
    if (state->reaction_ticks_left > 0) return pet_interact_sprite(state->reaction, state->tick);

    return pet_sprite(mood, _frame_for(mood, state->tick));
}

static void _display_pet(pet_face_state_t *state, pet_mood_t mood) {
    char row[PET_ROW_LENGTH + 1];
    char buf[4];

    watch_clear_colon();
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "PET", "PE");
    snprintf(buf, sizeof(buf), "%2d", pet_age_days() % TOP_RIGHT_WRAP);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);

    pet_row_clear(row);
    pet_row_place(row, state->position, _current_sprite(state, mood));
    watch_display_text(WATCH_POSITION_BOTTOM, row);
}

static void _display_grave(void) {
    const pet_t *pet = pet_get();
    char buf[PET_ROW_LENGTH + 1];

    watch_clear_colon();
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "RIP", "RP");
    snprintf(buf, sizeof(buf), "%2d", pet->generation % TOP_RIGHT_WRAP);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
    snprintf(buf, sizeof(buf), "AGE%3d", pet_age_days() % GRAVE_AGE_WRAP);
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

// The peek is there to read the time, so it ignores the 12h clock mode setting.
static void _display_time(void) {
    watch_date_time_t now = movement_get_local_date_time();
    char buf[8];

    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "NOW", "NW");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    snprintf(buf, sizeof(buf), "%02d%02d%02d", now.unit.hour, now.unit.minute, now.unit.second);
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
    watch_set_colon();
}

static void _display_stats(pet_face_state_t *state) {
    const pet_t *pet = pet_get();
    char buf[8];

    watch_clear_colon();
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, STAT_LABELS[state->stat_page].label, STAT_LABELS[state->stat_page].fallback);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    snprintf(buf, sizeof(buf), "%6d", _stat_value(pet, state->stat_page));
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

static void _redraw(pet_face_state_t *state) {
    pet_mood_t mood = pet_mood();

    _update_indicators(mood);

    if (state->peeking) _display_time();
    else if (state->showing_stats) _display_stats(state);
    else if (mood == PET_MOOD_DEAD) _display_grave();
    else _display_pet(state, mood);
}

static void _advance_stats(pet_face_state_t *state) {
    state->stat_ticks++;
    if (state->stat_ticks < TICKS_PER_STAT) return;

    state->stat_ticks = 0;
    state->stat_page = (state->stat_page + 1) % PET_STAT_COUNT;
}

static void _advance(pet_face_state_t *state, pet_mood_t mood) {
    state->tick++;

    if (state->showing_stats) {
        _advance_stats(state);
        return;
    }

    // Mid-reaction the pet holds still so there's something to actually look at.
    if (state->reaction_ticks_left > 0) {
        state->reaction_ticks_left--;
        return;
    }

    // A sleeping, sick or dying pet stays put; only a comfortable one bothers moving.
    if (mood == PET_MOOD_HAPPY || mood == PET_MOOD_HUNGRY) state->position = _wander(state->position);
}

static void _handle_hold(pet_face_state_t *state) {
    if (pet_mood() == PET_MOOD_DEAD) {
        pet_hatch();
        return;
    }

    state->peeking = true;
}

static void _handle_interact(pet_face_state_t *state) {
    pet_interact_kind_t kind = pet_interact();

    if (kind == PET_INTERACT_COUNT) return;

    state->reaction = kind;
    state->reaction_ticks_left = PET_FACE_REACTION_TICKS;
}

void pet_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr != NULL) return;

    *context_ptr = malloc(sizeof(pet_face_state_t));
    memset(*context_ptr, 0, sizeof(pet_face_state_t));
}

void pet_face_activate(void *context) {
    pet_face_state_t *state = (pet_face_state_t *) context;

    state->peeking = false;
    state->showing_stats = false;
    state->reaction_ticks_left = 0;
    movement_request_tick_frequency(TICK_FREQUENCY_HZ);
}

bool pet_face_loop(movement_event_t event, void *context) {
    pet_face_state_t *state = (pet_face_state_t *) context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_LOW_ENERGY_UPDATE:
            _redraw(state);
            break;
        case EVENT_TICK:
            _advance(state, pet_mood());
            _redraw(state);
            break;
        case EVENT_ALARM_BUTTON_UP:
            _handle_interact(state);
            _redraw(state);
            break;
        case EVENT_ALARM_LONG_PRESS:
            _handle_hold(state);
            _redraw(state);
            break;
        case EVENT_ALARM_LONG_UP:
            state->peeking = false;
            _redraw(state);
            break;
        case EVENT_LIGHT_LONG_PRESS:
            state->showing_stats = true;
            state->stat_page = 0;
            state->stat_ticks = 0;
            _redraw(state);
            break;
        case EVENT_LIGHT_LONG_UP:
            state->showing_stats = false;
            _redraw(state);
            break;
        case EVENT_BACKGROUND_TASK:
            pet_call();
            break;
        case EVENT_TIMEOUT:
            movement_move_to_resting_face();
            break;
        default:
            return movement_default_loop_handler(event);
    }

    return true;
}

void pet_face_resign(void *context) {
    (void) context;

    watch_clear_indicator(WATCH_INDICATOR_SLEEP);
    watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
    movement_request_tick_frequency(1);
}

movement_watch_face_advisory_t pet_face_advise(void *context) {
    (void) context;
    movement_watch_face_advisory_t advisory = { 0 };

    advisory.wants_background_task = pet_wants_to_call();

    return advisory;
}

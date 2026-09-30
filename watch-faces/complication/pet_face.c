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
#include "pet_screen.h"
#include "pet_species.h"
#include "watch_common_display.h"
#include "watch_utility.h"

// Two ticks per second is enough for a creature that ambles rather than runs.
static const uint8_t TICK_FREQUENCY_HZ = 2;

// The top right holds two digits, so later generations wrap.
static const uint16_t TOP_RIGHT_WRAP = 100;

// Three digits is all the room a number gets, whether on the top row or beside a label.
static const uint16_t STAT_WRAP = 1000;

typedef enum {
    PET_STAT_HUNGER,
    PET_STAT_HAPPINESS,
    PET_STAT_HEALTH,
    PET_STAT_AGE,
    PET_STAT_COUNT,
} pet_stat_t;

/* Padded to the full bottom row, so a shorter label wipes whatever the last page left
 * behind. Every glyph here draws in the bottom row.
 */
static const char *STAT_LABELS[PET_STAT_COUNT] = {
    [PET_STAT_HUNGER]    = "HUNG  ",
    [PET_STAT_HAPPINESS] = "HAPY  ",
    [PET_STAT_HEALTH]    = "HEAL  ",
    [PET_STAT_AGE]       = "AGE   ",
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

static void _update_indicators(pet_mood_t mood) {
    bool needs_help = mood == PET_MOOD_HUNGRY || mood == PET_MOOD_SICK || mood == PET_MOOD_CRITICAL;

    if (mood == PET_MOOD_ASLEEP) watch_set_indicator(WATCH_INDICATOR_SLEEP);
    else watch_clear_indicator(WATCH_INDICATOR_SLEEP);

    if (needs_help) watch_set_indicator(WATCH_INDICATOR_SIGNAL);
    else watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
}

static void _display_grave(void) {
    const pet_t *pet = pet_get();
    char buf[PET_SCREEN_BOTTOM_LENGTH + 1];

    watch_clear_colon();
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "RIP", "RP");
    snprintf(buf, sizeof(buf), "%2d", pet->generation % TOP_RIGHT_WRAP);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
    snprintf(buf, sizeof(buf), "AGE%3d", pet_age_days() % STAT_WRAP);
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

static const char MONTHS[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
static const char SHORT_MONTHS[12][3] = {"JA", "FE", "MR", "AP", "MY", "JN", "JL", "AU", "SE", "OC", "NO", "DE"};

/* Laid out like the clock face, with the weekday slot swapping to the month every other
 * second so the full date fits. The peek is there to read the time, so it ignores the
 * 12h clock mode setting.
 */
static void _display_time(void) {
    watch_date_time_t now = movement_get_local_date_time();
    char buf[8];

    if (now.unit.second % 2 == 0) {
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, watch_utility_get_long_weekday(now), watch_utility_get_weekday(now));
    } else {
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, MONTHS[now.unit.month - 1], SHORT_MONTHS[now.unit.month - 1]);
    }
    snprintf(buf, sizeof(buf), "%2d", now.unit.day);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
    snprintf(buf, sizeof(buf), "%02d%02d%02d", now.unit.hour, now.unit.minute, now.unit.second);
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
    watch_set_colon();
}

/* The label gets the big digits to itself, with room to spell four letters, and the
 * number sits right-aligned along the top: hundreds in the last cell of the top left,
 * tens and ones in the top right.
 */
static void _display_stats(pet_face_state_t *state) {
    const pet_t *pet = pet_get();
    char number[4];

    watch_clear_colon();
    snprintf(number, sizeof(number), "%3d", _stat_value(pet, state->stat_page) % STAT_WRAP);

    char top_left[] = { ' ', ' ', number[0], '\0' };
    char top_left_fallback[] = { ' ', number[0], '\0' };

    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, top_left, top_left_fallback);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, number + 1);
    watch_display_text(WATCH_POSITION_BOTTOM, STAT_LABELS[state->stat_page]);
}

static void _redraw(pet_face_state_t *state) {
    pet_mood_t mood = pet_mood();

    _update_indicators(mood);

    if (state->peeking) _display_time();
    else if (state->showing_stats) _display_stats(state);
    else if (mood == PET_MOOD_DEAD) _display_grave();
    else pet_species_current()->home_draw(mood);
}

/* The pet counts as a page too, after the last stat, so the same tap that brings the
 * numbers up is the one that puts them away.
 */
static void _next_stat_page(pet_face_state_t *state) {
    if (!state->showing_stats) {
        state->showing_stats = true;
        state->stat_page = 0;
        return;
    }

    state->stat_page++;
    if (state->stat_page >= PET_STAT_COUNT) state->showing_stats = false;
}

static void _advance(pet_face_state_t *state) {
    // Nobody can see the pet behind a stat, so it holds still until it's back on screen.
    if (state->showing_stats) return;

    pet_species_current()->home_advance(pet_mood());
}

// A tap on the grave hatches the next one, so the hold is always free for peeking at the time.
static void _handle_tap(void) {
    if (pet_mood() == PET_MOOD_DEAD) {
        pet_hatch();
        return;
    }

    pet_interact_kind_t kind = pet_interact();

    if (kind == PET_INTERACT_COUNT) return;

    pet_species_current()->react(kind);
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
    pet_species_current()->home_activate();
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
            _advance(state);
            _redraw(state);
            break;
        case EVENT_ALARM_BUTTON_UP:
            _handle_tap();
            _redraw(state);
            break;
        case EVENT_ALARM_LONG_PRESS:
            state->peeking = true;
            _redraw(state);
            break;
        case EVENT_ALARM_LONG_UP:
            state->peeking = false;
            _redraw(state);
            break;
        // A tap belongs to the stats, so the LED waits for a hold instead of lighting on every press.
        case EVENT_LIGHT_BUTTON_DOWN:
            break;
        case EVENT_LIGHT_BUTTON_UP:
            _next_stat_page(state);
            _redraw(state);
            break;
        case EVENT_LIGHT_LONG_PRESS:
            movement_illuminate_led();
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

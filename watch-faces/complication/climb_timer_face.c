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
#include "climb_timer_face.h"
#include "watch_common_display.h"
#include "watch_utility.h"

static const uint16_t SECONDS_PER_MINUTE = 60;
static const uint16_t SECONDS_PER_HOUR = 3600;
static const uint16_t MINUTES_PER_HOUR = 60;

// Two ticks per second is enough to blink the idle prompt without a faster wake rate.
static const uint8_t TICK_FREQUENCY_HZ = 2;

// A segment longer than this cannot be represented, so it stops growing rather than wrapping to zero.
static const uint16_t MAX_SEGMENT_S = UINT16_MAX;

static uint16_t _elapsed_s(climb_timer_state_t *state) {
    uint32_t start = watch_utility_date_time_to_unix_time(state->segment_start, 0);
    uint32_t now = watch_utility_date_time_to_unix_time(movement_get_local_date_time(), 0);

    if (now <= start) return 0;
    if (now - start > MAX_SEGMENT_S) return MAX_SEGMENT_S;

    return now - start;
}

static void _display_segment(uint8_t segment, const char *label, const char *fallback) {
    char buf[4];

    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, label, fallback);
    snprintf(buf, sizeof(buf), "%2d", segment);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
}

static void _display_elapsed(uint16_t seconds) {
    char buf[4];

    snprintf(buf, sizeof(buf), "%2d", seconds / SECONDS_PER_HOUR);
    watch_display_text(WATCH_POSITION_HOURS, buf);
    snprintf(buf, sizeof(buf), "%02d", (seconds / SECONDS_PER_MINUTE) % MINUTES_PER_HOUR);
    watch_display_text(WATCH_POSITION_MINUTES, buf);
    snprintf(buf, sizeof(buf), "%02d", seconds % SECONDS_PER_MINUTE);
    watch_display_text(WATCH_POSITION_SECONDS, buf);
}

// Segment 0 is the walk in, everything after it is a pitch.
static void _display_running(climb_timer_state_t *state) {
    if (state->segment == 0) _display_segment(state->segment, "APR", "AP");
    else _display_segment(state->segment, "PIT", "PI");

    _display_elapsed(_elapsed_s(state));
}

static void _display_summary(climb_timer_state_t *state) {
    _display_segment(state->segment, "SUM", "SU");
    _display_elapsed(state->segment_elapsed_s[state->segment]);
}

static void _display_idle(bool prompt_visible) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "CLB", "CL");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    watch_display_text(WATCH_POSITION_BOTTOM, prompt_visible ? "APRCH " : "      ");
}

static void _redraw(climb_timer_state_t *state, bool prompt_visible) {
    switch (state->mode) {
        case CLIMB_TIMER_IDLE:
            _display_idle(prompt_visible);
            break;
        case CLIMB_TIMER_RUNNING:
            _display_running(state);
            break;
        case CLIMB_TIMER_SUMMARY:
            _display_summary(state);
            break;
    }
}

static void _bank_current_segment(climb_timer_state_t *state) {
    state->segment_elapsed_s[state->segment] = _elapsed_s(state);
    state->banked_segments = state->segment + 1;
}

static void _start_approach(climb_timer_state_t *state) {
    memset(state->segment_elapsed_s, 0, sizeof(state->segment_elapsed_s));
    state->mode = CLIMB_TIMER_RUNNING;
    state->segment = 0;
    state->banked_segments = 0;
    state->segment_start = movement_get_local_date_time();
}

// The last segment keeps running past the limit so a long route loses no time, it just stops splitting.
static void _next_segment(climb_timer_state_t *state) {
    if (state->segment + 1 >= CLIMB_TIMER_MAX_SEGMENTS) return;

    _bank_current_segment(state);
    state->segment++;
    state->segment_start = movement_get_local_date_time();
}

static void _stop_and_summarize(climb_timer_state_t *state) {
    _bank_current_segment(state);
    state->mode = CLIMB_TIMER_SUMMARY;
    state->segment = 0;
}

static void _reset(climb_timer_state_t *state) {
    state->mode = CLIMB_TIMER_IDLE;
    state->segment = 0;
    state->banked_segments = 0;
}

static void _handle_hold(climb_timer_state_t *state) {
    switch (state->mode) {
        case CLIMB_TIMER_IDLE:
            _start_approach(state);
            break;
        case CLIMB_TIMER_RUNNING:
            _stop_and_summarize(state);
            break;
        case CLIMB_TIMER_SUMMARY:
            _reset(state);
            break;
    }
}

static void _handle_tap(climb_timer_state_t *state) {
    if (state->mode == CLIMB_TIMER_RUNNING) _next_segment(state);
    else if (state->mode == CLIMB_TIMER_SUMMARY) state->segment = (state->segment + 1) % state->banked_segments;
}

void climb_timer_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr != NULL) return;

    *context_ptr = malloc(sizeof(climb_timer_state_t));
    memset(*context_ptr, 0, sizeof(climb_timer_state_t));
}

void climb_timer_face_activate(void *context) {
    (void) context;
    movement_request_tick_frequency(TICK_FREQUENCY_HZ);
}

bool climb_timer_face_loop(movement_event_t event, void *context) {
    climb_timer_state_t *state = (climb_timer_state_t *) context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_LOW_ENERGY_UPDATE:
            _redraw(state, true);
            break;
        case EVENT_TICK:
            _redraw(state, event.subsecond == 0);
            break;
        case EVENT_ALARM_BUTTON_UP:
            _handle_tap(state);
            _redraw(state, true);
            break;
        case EVENT_ALARM_LONG_PRESS:
            _handle_hold(state);
            _redraw(state, true);
            break;
        case EVENT_TIMEOUT:
            // a running route stays on screen; there is nothing to watch otherwise
            if (state->mode != CLIMB_TIMER_RUNNING) movement_move_to_resting_face();
            break;
        default:
            return movement_default_loop_handler(event);
    }

    // Elapsed time is read back from the clock, so sleeping through a segment costs nothing.
    return true;
}

void climb_timer_face_resign(void *context) {
    (void) context;
    movement_request_tick_frequency(1);
}

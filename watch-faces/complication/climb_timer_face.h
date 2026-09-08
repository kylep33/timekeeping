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

#ifndef CLIMB_TIMER_FACE_H_
#define CLIMB_TIMER_FACE_H_

/*
 * CLIMB TIMER face
 *
 * Times the approach and each pitch of a route, one segment at a time.
 *
 * The top right shows the segment number, 0 being the approach. The bottom row
 * shows elapsed time as hours, minutes and seconds.
 *
 * Elapsed time comes from the clock rather than a tick count, so a segment keeps
 * running while you are on another face.
 *
 * ALARM hold, from idle:    start the approach
 * ALARM tap,  while timing: bank the segment and start the next one
 * ALARM hold, while timing: stop and show the summary
 * ALARM tap,  in summary:   step through the banked segments
 * ALARM hold, in summary:   clear and return to idle
 */

#include "movement.h"

#define CLIMB_TIMER_MAX_SEGMENTS 25

typedef enum {
    CLIMB_TIMER_IDLE,
    CLIMB_TIMER_RUNNING,
    CLIMB_TIMER_SUMMARY,
} climb_timer_mode_t;

typedef struct {
    climb_timer_mode_t mode;
    uint8_t segment;
    uint8_t banked_segments;
    watch_date_time_t segment_start;
    uint16_t segment_elapsed_s[CLIMB_TIMER_MAX_SEGMENTS];
} climb_timer_state_t;

void climb_timer_face_setup(uint8_t watch_face_index, void ** context_ptr);
void climb_timer_face_activate(void *context);
bool climb_timer_face_loop(movement_event_t event, void *context);
void climb_timer_face_resign(void *context);

#define climb_timer_face ((const watch_face_t){ \
    climb_timer_face_setup, \
    climb_timer_face_activate, \
    climb_timer_face_loop, \
    climb_timer_face_resign, \
    NULL, \
})

#endif // CLIMB_TIMER_FACE_H_

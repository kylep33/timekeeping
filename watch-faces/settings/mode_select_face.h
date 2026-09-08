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

#ifndef MODE_SELECT_FACE_H_
#define MODE_SELECT_FACE_H_

/*
 * MODE SELECT face
 *
 * Picks the watch mode: which faces the Mode button cycles through, and which
 * tune the hourly chime plays.
 *
 * Reached by long-pressing MODE on the resting face of any mode. The name of the
 * mode blinks while you choose.
 *
 * MODE short press: show the next mode
 * MODE long press:  switch to the mode on screen
 * No input:         switches to the mode on screen after a few seconds
 */

#include "movement.h"

void mode_select_face_setup(uint8_t watch_face_index, void ** context_ptr);
void mode_select_face_activate(void *context);
bool mode_select_face_loop(movement_event_t event, void *context);
void mode_select_face_resign(void *context);

#define mode_select_face ((const watch_face_t){ \
    mode_select_face_setup, \
    mode_select_face_activate, \
    mode_select_face_loop, \
    mode_select_face_resign, \
    NULL, \
})

#endif // MODE_SELECT_FACE_H_

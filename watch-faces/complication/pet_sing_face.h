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

#ifndef PET_SING_FACE_H_
#define PET_SING_FACE_H_

/*
 * PET SING face
 *
 * A jukebox for the pet: every signal tune in the firmware is fair game, so what
 * plays is whatever the hourly chime could ever be, not a tune written just for
 * this face. SING gives way to the pet itself, planted on a fresh random spot on
 * the row and singing along with its mouth on the actual notes, then returns once
 * the tune stops. The top right shows how happy it currently is, which a song raises.
 *
 * ALARM tap:   sing something at random
 */

#include "movement.h"
#include "pet.h"

typedef struct {
    bool singing;            ///< true from the ALARM press until the tune finishes
    uint8_t tune;            ///< which signal_tune_index_t is currently playing
    uint16_t elapsed_ticks;  ///< 64Hz ticks into that tune, the same clock the buzzer itself reads on
} pet_sing_face_state_t;

void pet_sing_face_setup(uint8_t watch_face_index, void ** context_ptr);
void pet_sing_face_activate(void *context);
bool pet_sing_face_loop(movement_event_t event, void *context);
void pet_sing_face_resign(void *context);

#define pet_sing_face ((const watch_face_t){ \
    pet_sing_face_setup, \
    pet_sing_face_activate, \
    pet_sing_face_loop, \
    pet_sing_face_resign, \
    NULL, \
})

#endif // PET_SING_FACE_H_

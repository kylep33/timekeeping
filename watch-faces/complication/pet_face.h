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

#ifndef PET_FACE_H_
#define PET_FACE_H_

/*
 * PET face
 *
 * Home for the creature. It wanders the bottom row, sleeps through the night and
 * shouts when it needs something, from whichever mode has the pet turned on.
 *
 * The top right shows its age in days.
 *
 * ALARM tap:   nudge it, which it resents while it is asleep
 * ALARM hold:  peek the exact time; on a dead pet, hatch the next one
 * LIGHT hold:  step through hunger, mood, health and age
 */

#include "movement.h"
#include "pet.h"

typedef struct {
    uint8_t position;
    uint8_t tick;
    uint8_t stat_page;
    uint8_t stat_ticks;
    bool peeking;
    bool showing_stats;
} pet_face_state_t;

void pet_face_setup(uint8_t watch_face_index, void ** context_ptr);
void pet_face_activate(void *context);
bool pet_face_loop(movement_event_t event, void *context);
void pet_face_resign(void *context);
movement_watch_face_advisory_t pet_face_advise(void *context);

#define pet_face ((const watch_face_t){ \
    pet_face_setup, \
    pet_face_activate, \
    pet_face_loop, \
    pet_face_resign, \
    pet_face_advise, \
})

#endif // PET_FACE_H_

/*
 * MIT License
 *
 * Copyright (c) 2023 Jeremy O'Brien
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

#include "movement_custom_signal_tunes_special.h"
#include "movement.h"

int8_t signal_tune_birthday[] = {
    BUZZER_NOTE_G5, 10, BUZZER_NOTE_REST, 2, BUZZER_NOTE_G5, 4,
    BUZZER_NOTE_A5, 16, BUZZER_NOTE_G5, 16, BUZZER_NOTE_C6, 16, BUZZER_NOTE_B5, 32,
    BUZZER_NOTE_G5, 10, BUZZER_NOTE_REST, 2, BUZZER_NOTE_G5, 4,
    BUZZER_NOTE_A5, 16, BUZZER_NOTE_G5, 16, BUZZER_NOTE_D6, 16, BUZZER_NOTE_C6, 32,
    BUZZER_NOTE_G5, 10, BUZZER_NOTE_REST, 2, BUZZER_NOTE_G5, 4,
    BUZZER_NOTE_G6, 16, BUZZER_NOTE_E6, 16, BUZZER_NOTE_C6, 16, BUZZER_NOTE_B5, 16, BUZZER_NOTE_A5, 32,
    BUZZER_NOTE_F6, 10, BUZZER_NOTE_REST, 2, BUZZER_NOTE_F6, 4,
    BUZZER_NOTE_E6, 16, BUZZER_NOTE_C6, 16, BUZZER_NOTE_D6, 16, BUZZER_NOTE_C6, 32,
    // a breath before it loops
    BUZZER_NOTE_REST, 32,
    0
};

/* The steel guitar riff between verses, an octave up to suit the piezo. Slides and bends
 * are a short grace note into the one they land on. 6/8, an eighth is about 21 ticks.
 */
int8_t signal_tune_edmund_fitzgerald[] = {
    BUZZER_NOTE_B5, 40, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B5, 6, BUZZER_NOTE_C6SHARP_D6FLAT, 36,
    BUZZER_NOTE_B5, 21,
    BUZZER_NOTE_A5, 63, BUZZER_NOTE_REST, 12,
    BUZZER_NOTE_F5SHARP_G5FLAT, 19, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5SHARP_G5FLAT, 19, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5SHARP_G5FLAT, 21,
    BUZZER_NOTE_G5SHARP_A5FLAT, 19, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_G5SHARP_A5FLAT, 6, BUZZER_NOTE_A5, 36,
    BUZZER_NOTE_G5SHARP_A5FLAT, 21,
    BUZZER_NOTE_F5SHARP_G5FLAT, 21,
    BUZZER_NOTE_E5, 21,
    BUZZER_NOTE_F5SHARP_G5FLAT, 63,
    // a breath before it loops
    BUZZER_NOTE_REST, 32,
    0
};

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

/* Terry Clements' intro riff, measures 1-8 of Songsterr's transcription of the record
 * (song 36755, lead guitar), an octave up to suit the piezo. 6/8 at 93 bpm, so an eighth
 * is 21 ticks. Bends are a short grace note into the pitch they reach, and a note too
 * long for one duration is split in two.
 */
int8_t signal_tune_edmund_fitzgerald[] = {
    // B, hammer on to C#, back to B
    BUZZER_NOTE_B5, 60, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B5, 21, BUZZER_NOTE_C6SHARP_D6FLAT, 21, BUZZER_NOTE_B5, 20,
    // A, then F# picked three times, into G#
    BUZZER_NOTE_A5, 21,
    BUZZER_NOTE_F5SHARP_G5FLAT, 29, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5SHARP_G5FLAT, 8, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_F5SHARP_G5FLAT, 41,
    BUZZER_NOTE_G5SHARP_A5FLAT, 19, BUZZER_NOTE_REST, 2,
    // G# bent up to A and let back down, F#, E
    BUZZER_NOTE_G5SHARP_A5FLAT, 10, BUZZER_NOTE_A5, 57, BUZZER_NOTE_G5SHARP_A5FLAT, 15,
    BUZZER_NOTE_F5SHARP_G5FLAT, 21, BUZZER_NOTE_E5, 21,
    // F# held two bars
    BUZZER_NOTE_F5SHARP_G5FLAT, 124, BUZZER_NOTE_F5SHARP_G5FLAT, 124,
    // F# bent up to G# and let back down, E
    BUZZER_NOTE_F5SHARP_G5FLAT, 10, BUZZER_NOTE_G5SHARP_A5FLAT, 78, BUZZER_NOTE_F5SHARP_G5FLAT, 15,
    BUZZER_NOTE_E5, 21,
    // F#, held out to the end of the next bar
    BUZZER_NOTE_F5SHARP_G5FLAT, 124, BUZZER_NOTE_F5SHARP_G5FLAT, 124,
    0
};

typedef struct {
    uint8_t month;
    uint8_t day;
    int8_t *tune;
} holiday_t;

// Alarms and the hourly chime play the holiday's tune on these dates, whatever they're set to.
static const holiday_t HOLIDAYS[] = {
    { .month = 7, .day = 15, .tune = signal_tune_birthday },   // julian_birthday
    { .month = 11, .day = 10, .tune = signal_tune_edmund_fitzgerald },   // wreck of the edmund fitzgerald, 1975
};

int8_t *holiday_tune_today(void) {
    watch_date_time_t now = movement_get_local_date_time();

    for (uint8_t i = 0; i < sizeof(HOLIDAYS) / sizeof(HOLIDAYS[0]); i++) {
        if (HOLIDAYS[i].month == now.unit.month && HOLIDAYS[i].day == now.unit.day) return HOLIDAYS[i].tune;
    }

    return NULL;
}

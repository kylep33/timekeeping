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

#include "pet_default_settings.h"
#include "movement.h"

static int8_t _tune_hungry[] = {
    BUZZER_NOTE_E7, 3,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E7, 3,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_G7, 6,
    0
};

static int8_t _tune_sick[] = {
    BUZZER_NOTE_C5, 8,
    BUZZER_NOTE_A4, 8,
    BUZZER_NOTE_C5, 8,
    BUZZER_NOTE_A4, 12,
    0
};

static int8_t _tune_critical[] = {
    BUZZER_NOTE_C4, 10,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_C4, 10,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_C4, 20,
    0
};

static int8_t _tune_sleeping[] = {
    BUZZER_NOTE_G5, 8,
    BUZZER_NOTE_E5, 8,
    BUZZER_NOTE_C5, 16,
    0
};

static int8_t _tune_waking[] = {
    BUZZER_NOTE_C5, 8,
    BUZZER_NOTE_E5, 8,
    BUZZER_NOTE_G5, 16,
    0
};

static int8_t _tune_died[] = {
    BUZZER_NOTE_G4, 12,
    BUZZER_NOTE_E4, 12,
    BUZZER_NOTE_C4, 12,
    BUZZER_NOTE_A3, 30,
    0
};

/* A shriek that flutters between clashing notes rather than a clean scale, then
 * collapses through a chromatic fall into a long, low groan.
 */
static int8_t _tune_murdered[] = {
    BUZZER_NOTE_C8, 2,
    BUZZER_NOTE_B7, 2,
    BUZZER_NOTE_C8, 2,
    BUZZER_NOTE_A7SHARP_B7FLAT, 2,
    BUZZER_NOTE_C8, 2,
    BUZZER_NOTE_B7, 2,
    BUZZER_NOTE_A7, 3,
    BUZZER_NOTE_F7SHARP_G7FLAT, 3,
    BUZZER_NOTE_D7, 4,
    BUZZER_NOTE_C7SHARP_D7FLAT, 4,
    BUZZER_NOTE_A6, 5,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_D4, 6,
    BUZZER_NOTE_C4SHARP_D4FLAT, 8,
    BUZZER_NOTE_REST, 4,
    BUZZER_NOTE_A2, 30,
    0
};

static int8_t _tune_eat[] = {
    BUZZER_NOTE_C6, 3,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E6, 5,
    0
};

static int8_t _tune_poke[] = {
    BUZZER_NOTE_A6, 3,
    0
};

static int8_t _tune_pat[] = {
    BUZZER_NOTE_G5, 3,
    0
};

static int8_t _tune_wave[] = {
    BUZZER_NOTE_C5, 2,
    BUZZER_NOTE_E5, 2,
    0
};

const pet_settings_t pet_default_settings = {
    // full to empty in about two days, so a day away is fine and a long weekend is not
    .hunger_drain_s_per_point = 1800,
    .happiness_drain_s_per_point = 2700,
    .health_drain_s_per_point = 1200,
    .health_recovery_s_per_point = 900,

    // long enough for a meal and a game before it nods off again
    .nudged_awake_min = 10,
    .critical_grace_hours = 12,
    .call_interval_min = 15,
    .critical_call_interval_min = 5,

    .bedtime_earliest_hour = 20,
    .bedtime_spread_min = 120,
    .wake_earliest_hour = 7,
    .wake_spread_min = 150,

    .illness_odds = 12,
    .neglect_threshold = 20,
    .hungry_threshold = 35,
    .illness_health_threshold = 30,
    .pokes_to_cure = 3,

    .poke_happiness_gain = 6,
    .pat_happiness_gain = 5,
    .wave_happiness_gain = 3,
    // a whole song is more effort than a poke, so it's worth more
    .sing_happiness_gain = 8,
    .disturb_happiness_cost = 5,
    .hatch_need = 80,

    .sounds = {
        .eat = _tune_eat,
        .poke = _tune_poke,
        .pat = _tune_pat,
        .wave = _tune_wave,
        .hungry = _tune_hungry,
        .sick = _tune_sick,
        .critical = _tune_critical,
        .sleeping = _tune_sleeping,
        .waking = _tune_waking,
        .died = _tune_died,
        .murdered = _tune_murdered,
    },
};

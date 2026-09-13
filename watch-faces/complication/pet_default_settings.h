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

#ifndef PET_DEFAULT_SETTINGS_H_
#define PET_DEFAULT_SETTINGS_H_

#include <stdint.h>

typedef struct {
    int8_t *eat;
    int8_t *poke;
    int8_t *pat;
    int8_t *wave;
    int8_t *hungry;
    int8_t *sick;
    int8_t *critical;
    int8_t *sleeping;
    int8_t *waking;
    int8_t *died;
} pet_sounds_t;

typedef struct {
    uint32_t hunger_drain_s_per_point;
    uint32_t happiness_drain_s_per_point;
    uint32_t health_drain_s_per_point;
    uint32_t health_recovery_s_per_point;
    uint32_t nudged_awake_min;
    uint32_t critical_grace_hours;
    uint32_t call_interval_min;
    uint32_t critical_call_interval_min;
    uint16_t bedtime_earliest_hour;
    uint16_t bedtime_spread_min;
    uint16_t wake_earliest_hour;
    uint16_t wake_spread_min;
    uint16_t illness_odds;
    uint8_t neglect_threshold;
    uint8_t hungry_threshold;
    uint8_t illness_health_threshold;
    uint8_t pokes_to_cure;
    uint8_t poke_happiness_gain;
    uint8_t pat_happiness_gain;
    uint8_t wave_happiness_gain;
    uint8_t sing_happiness_gain;
    uint8_t disturb_happiness_cost;
    uint8_t hatch_need;
    pet_sounds_t sounds;
} pet_settings_t;

extern const pet_settings_t pet_default_settings;

#endif // PET_DEFAULT_SETTINGS_H_

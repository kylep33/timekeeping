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

#ifndef MOVEMENT_CONFIG_H_
#define MOVEMENT_CONFIG_H_

#include "movement_faces.h"

/* Every face the watch can show. Modes below pick from these by name, so a face shared
 * across modes still costs one slot and keeps one context.
 */
typedef enum {
    FACE_CLOCK,
    FACE_ISH,
    FACE_TIMER,
    FACE_STOPWATCH,
    FACE_COIN_FLIP,
    FACE_PROBABILITY,
    FACE_PULSOMETER,
    FACE_ENDLESS_RUNNER,
    FACE_PING,
    FACE_TAROT,
    FACE_SIMON,
    FACE_SUNRISE_SUNSET,
    FACE_MOON_PHASE,
    FACE_TIDE,
    FACE_CLIMB_TIMER,
    FACE_SET_TIME,
    FACE_ADVANCED_ALARM,
    FACE_FINETUNE,
    FACE_NANOSEC,
    FACE_SETTINGS,
    FACE_VOLTAGE,
    FACE_MODE_SELECT,
} face_index_t;

const watch_face_t watch_faces[] = {
    [FACE_CLOCK] = clock_face,
    [FACE_ISH] = ish_face,
    [FACE_TIMER] = timer_face,
    [FACE_STOPWATCH] = stopwatch_face,
    [FACE_COIN_FLIP] = simple_coin_flip_face,
    [FACE_PROBABILITY] = probability_face,
    [FACE_PULSOMETER] = pulsometer_face,
    [FACE_ENDLESS_RUNNER] = endless_runner_face,
    [FACE_PING] = ping_face,
    [FACE_TAROT] = tarot_face,
    [FACE_SIMON] = simon_face,
    [FACE_SUNRISE_SUNSET] = sunrise_sunset_face,
    [FACE_MOON_PHASE] = moon_phase_face,
    [FACE_TIDE] = tide_face,
    [FACE_CLIMB_TIMER] = climb_timer_face,
    [FACE_SET_TIME] = set_time_face,
    [FACE_ADVANCED_ALARM] = advanced_alarm_face,
    [FACE_FINETUNE] = finetune_face,
    [FACE_NANOSEC] = nanosec_face,
    [FACE_SETTINGS] = settings_face,
    [FACE_VOLTAGE] = voltage_face,
    [FACE_MODE_SELECT] = mode_select_face,
};

#define MOVEMENT_NUM_FACES (sizeof(watch_faces) / sizeof(watch_face_t))

/* The first face of a mode is its resting face: every timeout returns there, so it should
 * be something worth staring at rather than a screen a stray press would disturb.
 */
static const uint8_t daily_faces[] = { FACE_ISH, FACE_TIMER, FACE_STOPWATCH, FACE_COIN_FLIP };
static const uint8_t climb_faces[] = { FACE_CLOCK, FACE_CLIMB_TIMER };
static const uint8_t game_faces[] = { FACE_CLOCK, FACE_PROBABILITY, FACE_PULSOMETER, FACE_ENDLESS_RUNNER, FACE_PING, FACE_TAROT, FACE_SIMON };
static const uint8_t outdoor_faces[] = { FACE_CLOCK, FACE_SUNRISE_SUNSET, FACE_MOON_PHASE, FACE_TIDE };
static const uint8_t setup_faces[] = { FACE_CLOCK, FACE_SET_TIME, FACE_ADVANCED_ALARM, FACE_FINETUNE, FACE_NANOSEC, FACE_SETTINGS, FACE_VOLTAGE };

#define MODE(display_name, tune, faces) { display_name, tune, faces, sizeof(faces) }

/* Mode names are shown on the bottom row, so they must fit six characters. */
const movement_mode_t movement_modes[] = {
    MODE("DAILY", SIGNAL_TUNE_KIM_POSSIBLE, daily_faces),
    MODE("CLIMB", SIGNAL_TUNE_ZELDA_SECRET, climb_faces),
    MODE("GAME", SIGNAL_TUNE_MARIO_THEME, game_faces),
    MODE("OUTDR", SIGNAL_TUNE_EVANGELION, outdoor_faces),
    MODE("SETUP", SIGNAL_TUNE_MGS_CODEC, setup_faces),
};

#undef MODE

#define MOVEMENT_NUM_MODES (sizeof(movement_modes) / sizeof(movement_mode_t))

/* Hourly chime tune used until a mode selects its own. See movement_custom_signal_tunes.h for options. */
#define MOVEMENT_DEFAULT_SIGNAL_TUNE SIGNAL_TUNE_KIM_POSSIBLE

/* Determines the intensity of the led colors
 * Set a hex value 0-15 with 0x0 being off and 0xF being max intensity
 */
#define MOVEMENT_DEFAULT_RED_COLOR 0x0
#define MOVEMENT_DEFAULT_GREEN_COLOR 0xF
#define MOVEMENT_DEFAULT_BLUE_COLOR 0x0

/* Set to true for 24h mode or false for 12h mode */
#define MOVEMENT_DEFAULT_24H_MODE false

/* Enable or disable the sound on mode button press */
#define MOVEMENT_DEFAULT_BUTTON_SOUND true

#define MOVEMENT_DEFAULT_BUTTON_VOLUME WATCH_BUZZER_VOLUME_SOFT
#define MOVEMENT_DEFAULT_SIGNAL_VOLUME WATCH_BUZZER_VOLUME_LOUD
#define MOVEMENT_DEFAULT_ALARM_VOLUME WATCH_BUZZER_VOLUME_LOUD

/* Set the timeout before switching back to the main watch face
 * Valid values are:
 * 0: 60 seconds
 * 1: 2 minutes
 * 2: 5 minutes
 * 3: 30 minutes
 */
#define MOVEMENT_DEFAULT_TIMEOUT_INTERVAL 0

/* Set the timeout before switching to low energy mode
 * Valid values are:
 * 0: Never
 * 1: 10 minutes
 * 2: 1 hour
 * 3: 2 hours
 * 4: 6 hours
 * 5: 12 hours
 * 6: 1 day
 * 7: 7 days
 */
#define MOVEMENT_DEFAULT_LOW_ENERGY_INTERVAL 2

/* Set the led duration
 * Valid values are:
 * 0: No LED
 * 1: 1 second
 * 2: 3 seconds
 * 3: 5 seconds
 */
#define MOVEMENT_DEFAULT_LED_DURATION 1

/* Optionally debounce button presses (disable by default).
 * A value of 4 is a good starting point if you have issues
 * with multiple button presses firing.
*/
#define MOVEMENT_DEBOUNCE_TICKS 0

#endif // MOVEMENT_CONFIG_H_

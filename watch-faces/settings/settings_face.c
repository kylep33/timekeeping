/*
 * MIT License
 *
 * Copyright (c) 2025 Joey Castillo
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
#include "settings_face.h"
#include "watch.h"

static void clock_setting_display(uint8_t subsecond) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP, "CLOCK", "CL");
    if (subsecond % 2) {
        if (movement_clock_mode_24h()) watch_display_text(WATCH_POSITION_BOTTOM, "24h");
        else watch_display_text(WATCH_POSITION_BOTTOM, "12h");
    } else {
        watch_display_text(WATCH_POSITION_BOTTOM, "      ");
    }
}

static void clock_setting_advance(void) {
    movement_set_clock_mode_24h(((movement_clock_mode_24h() + 1) % MOVEMENT_NUM_CLOCK_MODES));
}

static void beep_setting_display(uint8_t subsecond) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "BTN", "BT");
    watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "beep  ", " beep ");
    if (subsecond % 2) {
        if (movement_button_should_sound()) {
            if (movement_button_volume() == WATCH_BUZZER_VOLUME_LOUD) {
                // H for HIGH
                watch_display_text(WATCH_POSITION_TOP_RIGHT, " H");
            }
            else {
                // L for LOW
                watch_display_text(WATCH_POSITION_TOP_RIGHT, " L");
            }
        } else {
            // N for NONE
            watch_display_text(WATCH_POSITION_TOP_RIGHT, " N");
        }
    } else {
        watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    }
}

static void beep_setting_advance(void) {
    if (!movement_button_should_sound()) {
        // was muted. make it soft.
        movement_set_button_should_sound(true);
        movement_set_button_volume(WATCH_BUZZER_VOLUME_SOFT);
        beep_setting_display(1);
        watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, WATCH_BUZZER_VOLUME_SOFT);
    } else if (movement_button_volume() == WATCH_BUZZER_VOLUME_SOFT) {
        // was soft. make it loud.
        movement_set_button_volume(WATCH_BUZZER_VOLUME_LOUD);
        beep_setting_display(1);
        watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, WATCH_BUZZER_VOLUME_LOUD);
    } else {
        // was loud. make it silent.
        movement_set_button_should_sound(false);
        beep_setting_display(1);
    }
}

static void signal_setting_display(uint8_t subsecond) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "SIG", "SI");
    watch_display_text(WATCH_POSITION_BOTTOM, "SIGNAL");
    if (subsecond % 2) {
        if (movement_signal_volume() == WATCH_BUZZER_VOLUME_LOUD) {
            // H for HIGH
            watch_display_text(WATCH_POSITION_TOP_RIGHT, " H");
        }
        else {
            // L for LOW
            watch_display_text(WATCH_POSITION_TOP_RIGHT, " L");
        }
    } else {
        watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    }
}

static void signal_setting_advance(void) {
    if (movement_signal_volume() == WATCH_BUZZER_VOLUME_SOFT) {
        // was soft. make it loud.
        movement_set_signal_volume(WATCH_BUZZER_VOLUME_LOUD);
    } else {
        // was loud. make it soft.
        movement_set_signal_volume(WATCH_BUZZER_VOLUME_SOFT);
    }

    signal_setting_display(1);
    movement_play_signal();
}

static void toggle_setting_display(const char *label, const char *fallback, bool on, uint8_t subsecond) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP, label, fallback);
    if (subsecond % 2) {
        if (on) watch_display_text(WATCH_POSITION_BOTTOM, "  on  ");
        else watch_display_text(WATCH_POSITION_BOTTOM, " off  ");
    } else {
        watch_display_text(WATCH_POSITION_BOTTOM, "      ");
    }
}

static void annoy_setting_display(uint8_t subsecond) {
    toggle_setting_display("ANNOY", "AN", movement_annoy_enabled(), subsecond);
}

static void annoy_setting_advance(void) {
    movement_set_annoy_enabled(!movement_annoy_enabled());
}

static void mari_setting_display(uint8_t subsecond) {
    toggle_setting_display("MARI ", "MA", movement_mari_enabled(), subsecond);
}

static void mari_setting_advance(void) {
    movement_set_mari_enabled(!movement_mari_enabled());
}

static void alarm_setting_display(uint8_t subsecond) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "ALM", "AL");
    watch_display_text(WATCH_POSITION_BOTTOM, "ALARM ");
    if (subsecond % 2) {
        if (movement_alarm_volume() == WATCH_BUZZER_VOLUME_LOUD) {
            // H for HIGH
            watch_display_text(WATCH_POSITION_TOP_RIGHT, " H");
        }
        else {
            // L for LOW
            watch_display_text(WATCH_POSITION_TOP_RIGHT, " L");
        }
    } else {
        watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    }
}

static void alarm_setting_advance(void) {
    if (movement_alarm_volume() == WATCH_BUZZER_VOLUME_SOFT) {
        // was soft. make it loud.
        movement_set_alarm_volume(WATCH_BUZZER_VOLUME_LOUD);
    } else {
        // was loud. make it soft.
        movement_set_alarm_volume(WATCH_BUZZER_VOLUME_SOFT);

    }

    alarm_setting_display(1);
    movement_play_alarm();
}

static void mute_setting_display(uint8_t subsecond) {
    toggle_setting_display("MUTE ", "MU", movement_alarms_muted(), subsecond);
}

static void mute_setting_advance(void) {
    movement_set_alarms_muted(!movement_alarms_muted());
}

static void timeout_setting_display(uint8_t subsecond) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP, "TMOUt", "TO");
    if (subsecond % 2) {
        switch (movement_get_fast_tick_timeout()) {
            case 0:
                watch_display_text(WATCH_POSITION_BOTTOM, "60 SeC");
                break;
            case 1:
                watch_display_text(WATCH_POSITION_BOTTOM, "2 n&in");
                break;
            case 2:
                watch_display_text(WATCH_POSITION_BOTTOM, "5 n&in");
                break;
            case 3:
                watch_display_text(WATCH_POSITION_BOTTOM, "30n&in");
                break;
        }
    } else {
        watch_display_text(WATCH_POSITION_BOTTOM, "      ");
    }
}

static void timeout_setting_advance(void) {
    movement_set_fast_tick_timeout((movement_get_fast_tick_timeout() + 1));
}

static void low_energy_setting_display(uint8_t subsecond) {
    watch_display_text_with_fallback(WATCH_POSITION_TOP, "LoEne", "LE");
    if (subsecond % 2) {
        switch (movement_get_low_energy_timeout()) {
            case 0:
                watch_display_text(WATCH_POSITION_BOTTOM, " Never");
                break;
            case 1:
                watch_display_text(WATCH_POSITION_BOTTOM, "10n&in");
                break;
            case 2:
                watch_display_text(WATCH_POSITION_BOTTOM, "1 hour");
                break;
            case 3:
                watch_display_text(WATCH_POSITION_BOTTOM, "2 hour");
                break;
            case 4:
                watch_display_text(WATCH_POSITION_BOTTOM, "6 hour");
                break;
            case 5:
                watch_display_text(WATCH_POSITION_BOTTOM, "12 hr");
                break;
            case 6:
                watch_display_text(WATCH_POSITION_BOTTOM, " 1 day");
                break;
            case 7:
                watch_display_text(WATCH_POSITION_BOTTOM, " 7 day");
                break;
        }
    } else {
        watch_display_text(WATCH_POSITION_BOTTOM, "      ");
    }
}

static void low_energy_setting_advance(void) {
    movement_set_low_energy_timeout((movement_get_low_energy_timeout() + 1));
}

static void led_duration_setting_display(uint8_t subsecond) {
    char buf[8];

    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "LED", "LT");
    if (subsecond % 2) {
        if (movement_get_backlight_dwell() == 0) {
            watch_display_text(WATCH_POSITION_BOTTOM, "instnt");
        } else if (movement_get_backlight_dwell() == 0b111) {
            watch_display_text(WATCH_POSITION_BOTTOM, "no LEd");
        } else {
            sprintf(buf, " %1d SeC", (movement_get_backlight_dwell() * 2 - 1) % 10);
            watch_display_text(WATCH_POSITION_BOTTOM, buf);
        }
    } else {
        watch_display_text(WATCH_POSITION_BOTTOM, "      ");
    }
}

static void led_duration_setting_advance(void) {
    movement_set_backlight_dwell(movement_get_backlight_dwell() + 1);
    if (movement_get_backlight_dwell() > 3) {
        // set all bits to disable the LED
        movement_set_backlight_dwell(0b111);
    }
}

static void red_led_setting_display(uint8_t subsecond) {
    char buf[8];
    movement_color_t color = movement_backlight_color();

    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "LED", "LT");
    watch_display_text(WATCH_POSITION_BOTTOM, " red  ");
    if (subsecond % 2) {
        sprintf(buf, "%2d", color.red);
        watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
    } else {
        watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    }
}

static void red_led_setting_advance(void) {
    movement_color_t color = movement_backlight_color();
    color.red++;
    movement_set_backlight_color(color);
}

static void green_led_setting_display(uint8_t subsecond) {
    char buf[8];
    movement_color_t color = movement_backlight_color();

    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "LED", "LT");
    watch_display_text(WATCH_POSITION_BOTTOM, " green");
    if (subsecond % 2) {
        sprintf(buf, "%2d", color.green);
        watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
    } else {
        watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    }
}

static void green_led_setting_advance(void) {
    movement_color_t color = movement_backlight_color();
    color.green++;
    movement_set_backlight_color(color);
}

static void blue_led_setting_display(uint8_t subsecond) {
    char buf[8];
    movement_color_t color = movement_backlight_color();

    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "LED", "LT");
    watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "blue  ", " blue ");
    if (subsecond % 2) {
        sprintf(buf, "%2d", color.blue);
        watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
    } else {
        watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    }
}

static void blue_led_setting_advance(void) {
    movement_color_t color = movement_backlight_color();
    color.blue++;
    movement_set_backlight_color(color);
}

static void  git_hash_setting_display(uint8_t subsecond) {
    (void) subsecond;
    char buf[8];
    // BUILD_GIT_HASH will already be truncated to 6 characters in the makefile, but this is to be safe.
    sprintf(buf, "%.6s", BUILD_GIT_HASH);
    watch_display_text_with_fallback(WATCH_POSITION_TOP, "Bu{d ", "bU");
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

static void git_hash_setting_advance(void) {
    return;
}

static const settings_screen_t STANDARD_SCREENS[] = {
    { clock_setting_display, clock_setting_advance },
    { beep_setting_display, beep_setting_advance },
    { timeout_setting_display, timeout_setting_advance },
#ifndef MOVEMENT_LOW_ENERGY_MODE_FORBIDDEN
    { low_energy_setting_display, low_energy_setting_advance },
#endif
#ifdef BUILD_GIT_HASH
    { git_hash_setting_display, git_hash_setting_advance },
#endif
};

static const settings_screen_t CHIME_SCREENS[] = {
    { signal_setting_display, signal_setting_advance },
    { annoy_setting_display, annoy_setting_advance },
    { mari_setting_display, mari_setting_advance },
};

static const settings_screen_t ALARM_SCREENS[] = {
    { alarm_setting_display, alarm_setting_advance },
    { mute_setting_display, mute_setting_advance },
};

// Duration first, so the color screens that keep the LED lit start at 1.
static const uint8_t LED_COLOR_START = 1;

static const settings_screen_t LED_SCREENS[] = {
    { led_duration_setting_display, led_duration_setting_advance },
#ifdef WATCH_RED_TCC_CHANNEL
    { red_led_setting_display, red_led_setting_advance },
#endif
#ifdef WATCH_GREEN_TCC_CHANNEL
    { green_led_setting_display, green_led_setting_advance },
#endif
#ifdef WATCH_BLUE_TCC_CHANNEL
    { blue_led_setting_display, blue_led_setting_advance },
#endif
};

static void setup_screens(void **context_ptr, const settings_screen_t *screens, uint8_t num_settings, uint8_t led_color_start, uint8_t led_color_end) {
    if (*context_ptr != NULL) return;

    settings_state_t *state = malloc(sizeof(settings_state_t));
    state->settings_screens = screens;
    state->num_settings = num_settings;
    state->led_color_start = led_color_start;
    state->led_color_end = led_color_end;
    *context_ptr = state;
}

void settings_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    setup_screens(context_ptr, STANDARD_SCREENS, sizeof(STANDARD_SCREENS) / sizeof(settings_screen_t), 0, 0);
}

void chime_settings_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    setup_screens(context_ptr, CHIME_SCREENS, sizeof(CHIME_SCREENS) / sizeof(settings_screen_t), 0, 0);
}

void alarm_settings_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    setup_screens(context_ptr, ALARM_SCREENS, sizeof(ALARM_SCREENS) / sizeof(settings_screen_t), 0, 0);
}

void led_settings_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    uint8_t num_settings = sizeof(LED_SCREENS) / sizeof(settings_screen_t);
#ifndef WATCH_RED_TCC_CHANNEL
    (void)red_led_setting_display;
    (void)red_led_setting_advance;
#endif
#ifndef WATCH_GREEN_TCC_CHANNEL
    (void)green_led_setting_display;
    (void)green_led_setting_advance;
#endif
#ifndef WATCH_BLUE_TCC_CHANNEL
    (void)blue_led_setting_display;
    (void)blue_led_setting_advance;
#endif
    setup_screens(context_ptr, LED_SCREENS, num_settings, LED_COLOR_START, num_settings);
}

void settings_face_activate(void *context) {
    settings_state_t *state = (settings_state_t *)context;
    state->current_page = 0;
    movement_request_tick_frequency(4); // we need to manually blink some pixels
}

bool settings_face_loop(movement_event_t event, void *context) {
    settings_state_t *state = (settings_state_t *)context;

    switch (event.event_type) {
        case EVENT_LIGHT_BUTTON_DOWN:
            state->current_page = (state->current_page + 1) % state->num_settings;
            watch_clear_display();
            // fall through
        case EVENT_TICK:
        case EVENT_ACTIVATE:
            state->settings_screens[state->current_page].display(event.subsecond);
            break;
        case EVENT_MODE_BUTTON_UP:
            movement_force_led_off();
            movement_move_to_next_face();
            return true;
        case EVENT_ALARM_BUTTON_UP:
            state->settings_screens[state->current_page].advance();
            break;
        case EVENT_TIMEOUT:
            movement_move_to_resting_face();
            break;
        default:
            return movement_default_loop_handler(event);
    }

    if (state->current_page >= state->led_color_start && state->current_page < state->led_color_end) {
        movement_color_t color = movement_backlight_color();
        // this bitwise math turns #000 into #000000, #111 into #111111, etc.
        movement_force_led_on(color.red | color.red << 4,
                              color.green | color.green << 4,
                              color.blue | color.blue << 4);
        return true;
    } else {
        movement_force_led_off();
        return true;
    }
}

void settings_face_resign(void *context) {
    (void) context;
    movement_force_led_off();
    movement_store_settings();
}

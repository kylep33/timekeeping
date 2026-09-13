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

#include <stdio.h>
#include <string.h>
#include "pet_screen.h"
#include "watch_common_display.h"

// The top right has two digits, and a full 100 wrapping round to 0 read as an empty stat.
static const uint8_t TOP_RIGHT_MAX = 99;

uint8_t pet_screen_top_length(void) {
    // The classic LCD's top left is two cells; the custom one adds a third.
    return watch_get_lcd_type() == WATCH_LCD_TYPE_CUSTOM ? PET_SCREEN_TOP_LENGTH : PET_SCREEN_TOP_LENGTH - 1;
}

static void _clear(char *row, uint8_t length) {
    memset(row, ' ', length);
    row[length] = '\0';
}

static void _place(char *row, uint8_t length, uint8_t position, const char *sprite) {
    for (uint8_t i = 0; sprite[i] != '\0'; i++) {
        if (position + i >= length) return;
        row[position + i] = sprite[i];
    }
}

void pet_screen_bottom_clear(char *row) {
    _clear(row, PET_SCREEN_BOTTOM_LENGTH);
}

void pet_screen_bottom_place(char *row, uint8_t position, const char *sprite) {
    _place(row, PET_SCREEN_BOTTOM_LENGTH, position, sprite);
}

void pet_screen_top_clear(char *row) {
    _clear(row, PET_SCREEN_TOP_LENGTH);
}

void pet_screen_top_place(char *row, uint8_t position, const char *sprite) {
    _place(row, pet_screen_top_length(), position, sprite);
}

void pet_screen_top_draw(const char *row) {
    // The fallback drops the third cell, which is the one the classic LCD hasn't got.
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, row, row);
}

void pet_screen_stat_draw(uint8_t stat) {
    char buf[3];

    snprintf(buf, sizeof(buf), "%2d", stat > TOP_RIGHT_MAX ? TOP_RIGHT_MAX : stat);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
}

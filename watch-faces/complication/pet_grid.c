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

#include "pet_grid.h"
#include "pet_screen.h"
#include "watch_common_display.h"

static bool _on_top_strip(pet_grid_level_t level) {
    return level == PET_GRID_LEVEL_TOP_LOW || level == PET_GRID_LEVEL_TOP_HIGH;
}

static uint8_t _strip_length(pet_grid_level_t level) {
    return _on_top_strip(level) ? pet_screen_top_length() : PET_SCREEN_BOTTOM_LENGTH;
}

// The top strip is shorter, so only some bottom row digits have a square above them.
static bool _square_exists(int8_t level, int8_t column) {
    if (level < 0 || level >= PET_GRID_LEVEL_COUNT || column < 0) return false;

    return column + PET_GRID_SPRITE_WIDTH <= _strip_length(level);
}

bool pet_grid_in_top_half(pet_grid_level_t level) {
    return level == PET_GRID_LEVEL_BOTTOM_HIGH || level == PET_GRID_LEVEL_TOP_HIGH;
}

void pet_grid_step(pet_grid_spot_t *spot, int8_t level_step, int8_t column_step) {
    int8_t level = spot->level + level_step;
    int8_t column = spot->column + column_step;

    if (!_square_exists(level, column)) return;

    spot->level = level;
    spot->column = column;
}

void pet_grid_clamp(pet_grid_spot_t *spot) {
    uint8_t length = _strip_length(spot->level);

    if (spot->column + PET_GRID_SPRITE_WIDTH > length) spot->column = length - PET_GRID_SPRITE_WIDTH;
}

void pet_grid_drop_to_bottom_half(pet_grid_spot_t *spot) {
    if (pet_grid_in_top_half(spot->level)) spot->level--;
}

void pet_grid_draw(const pet_grid_spot_t *spot, const char *sprite) {
    char top[PET_SCREEN_TOP_LENGTH + 1];
    char bottom[PET_SCREEN_BOTTOM_LENGTH + 1];

    pet_screen_top_clear(top);
    pet_screen_bottom_clear(bottom);

    if (_on_top_strip(spot->level)) pet_screen_top_place(top, spot->column, sprite);
    else pet_screen_bottom_place(bottom, spot->column, sprite);

    pet_screen_top_draw(top);
    watch_display_text(WATCH_POSITION_BOTTOM, bottom);
}

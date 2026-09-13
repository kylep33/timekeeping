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

#ifndef PET_GRID_H_
#define PET_GRID_H_

/*
 * The squares a one cell pet can walk: the bottom row and the top left strip, each
 * digit two squares tall. Pets that don't walk around have no use for it.
 */

#include "movement.h"

#define PET_GRID_SPRITE_WIDTH 1

// Bottom up. A pet only steps one level at a time, so it never jumps between strips.
typedef enum {
    PET_GRID_LEVEL_BOTTOM_LOW,
    PET_GRID_LEVEL_BOTTOM_HIGH,
    PET_GRID_LEVEL_TOP_LOW,
    PET_GRID_LEVEL_TOP_HIGH,
    PET_GRID_LEVEL_COUNT,
} pet_grid_level_t;

typedef struct {
    pet_grid_level_t level;
    uint8_t column;
} pet_grid_spot_t;

bool pet_grid_in_top_half(pet_grid_level_t level);

/// @brief Moves by the given steps, or stays put if that square doesn't exist.
void pet_grid_step(pet_grid_spot_t *spot, int8_t level_step, int8_t column_step);

/// @brief Pulls the column back inside whichever strip the spot is on.
void pet_grid_clamp(pet_grid_spot_t *spot);

void pet_grid_drop_to_bottom_half(pet_grid_spot_t *spot);

/// @brief Draws the sprite at the spot, blanking the rest of both strips.
void pet_grid_draw(const pet_grid_spot_t *spot, const char *sprite);

#endif // PET_GRID_H_

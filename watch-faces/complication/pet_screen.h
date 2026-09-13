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

#ifndef PET_SCREEN_H_
#define PET_SCREEN_H_

#include "movement.h"

#define PET_SCREEN_BOTTOM_LENGTH 6
#define PET_SCREEN_TOP_LENGTH 3

/// @brief How many cells the top left strip has on the LCD actually fitted.
uint8_t pet_screen_top_length(void);

/// @brief Blanks a bottom row buffer, which must hold PET_SCREEN_BOTTOM_LENGTH + 1 characters.
void pet_screen_bottom_clear(char *row);

/// @brief Draws a sprite into a blanked bottom row, clipping anything past the edge.
void pet_screen_bottom_place(char *row, uint8_t position, const char *sprite);

/// @brief Blanks a top strip buffer, which must hold PET_SCREEN_TOP_LENGTH + 1 characters.
void pet_screen_top_clear(char *row);

/// @brief Draws a sprite into a blanked top strip, clipping anything past the edge.
void pet_screen_top_place(char *row, uint8_t position, const char *sprite);

/// @brief Puts a prepared top strip on screen, using whatever cells the LCD has.
void pet_screen_top_draw(const char *row);

/// @brief Puts a stat in the top right, showing a full 100 as 99 rather than wrapping to 0.
void pet_screen_stat_draw(uint8_t stat);

#endif // PET_SCREEN_H_

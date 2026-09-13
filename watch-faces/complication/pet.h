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

#ifndef PET_H_
#define PET_H_

#include "movement.h"

#define PET_STAT_MAX 100

#define PET_ROW_LENGTH 6
#define PET_TOP_ROW_LENGTH 3
#define PET_SPRITE_WIDTH 1
#define PET_ANIMATION_FRAMES 2

typedef enum {
    PET_MOOD_HAPPY,
    PET_MOOD_HUNGRY,
    PET_MOOD_SICK,
    PET_MOOD_ASLEEP,
    PET_MOOD_CRITICAL,
    PET_MOOD_DEAD,
    PET_MOOD_COUNT,
} pet_mood_t;

/* What short-pressing ALARM on the home face does while the pet is awake. Which one
 * happens is picked at random, so the same button turns into a handful of different
 * little moments instead of one repeated animation.
 */
typedef enum {
    PET_INTERACT_POKE,
    PET_INTERACT_PAT,
    PET_INTERACT_WAVE,
    PET_INTERACT_COUNT,
} pet_interact_kind_t;

/// @brief An entry in the play menu that hands off to a game face.
typedef struct {
    const char *name;       ///< six characters, and no glyph the bottom row cannot draw
    uint8_t face_index;     ///< a position in watch_faces[]
} pet_game_t;

// Defined in movement_config.h so the menu offers whichever games are compiled in.
extern const pet_game_t pet_games[];
extern const uint8_t pet_num_games;

typedef struct {
    uint32_t settled_at_s;          ///< wall clock the needs were last brought up to date
    uint32_t hatched_at_s;
    uint32_t critical_since_s;      ///< zero unless health has bottomed out
    uint32_t awake_until_s;         ///< zero unless it was prodded awake after bedtime
    uint32_t hunger_debt_s;         ///< time banked toward the next whole point of drain
    uint32_t happiness_debt_s;
    uint32_t health_debt_s;
    uint16_t best_age_days;
    uint16_t rolled_on_day;         ///< day number of the most recent illness roll
    uint8_t version;
    uint8_t generation;
    uint8_t hunger;
    uint8_t happiness;
    uint8_t health;
    uint8_t pokes_while_sick;
    bool sick;
    bool asleep;
    bool dead;
} pet_t;

/// @brief Brings the pet up to date with the clock, then returns it.
const pet_t *pet_get(void);

/// @brief Retires the current pet's age into the record and starts a new one.
void pet_hatch(void);

pet_mood_t pet_mood(void);
uint16_t pet_age_days(void);

/// @brief The current species' sprite for a mood, alternating with frame.
const char *pet_sprite(pet_mood_t mood, uint8_t frame);

/// @brief The mid-hop frame: low in the digit on the way up, or stretched tall just before landing.
const char *pet_rise_sprite(bool stretched);

/* Where the pet is standing, as a column shared by every face, so walking off one
 * screen and onto another leaves it where you left it. Which strip and which half of
 * the digit it is in is the home face's business, since nowhere else draws those.
 */
uint8_t pet_position(void);
void pet_set_position(uint8_t position);

/// @brief How many cells the upper strip has on the LCD actually fitted.
uint8_t pet_top_row_length(void);

/// @brief Blanks a bottom row buffer, which must hold PET_ROW_LENGTH + 1 characters.
void pet_row_clear(char *row);

/// @brief Draws a sprite into a blanked row, clipping anything past the edge.
void pet_row_place(char *row, uint8_t position, const char *sprite);

/// @brief Blanks an upper strip buffer, which must hold PET_TOP_ROW_LENGTH + 1 characters.
void pet_top_clear(char *row);

/// @brief Draws a sprite into a blanked upper strip, clipping anything past the edge.
void pet_top_place(char *row, uint8_t position, const char *sprite);

/// @brief Puts a prepared upper strip on screen, using whatever cells the LCD has.
void pet_top_draw(const char *row);

/// @brief Puts a stat in the top right, showing a full 100 as 99 rather than wrapping to 0.
void pet_stat_draw(uint8_t stat);

/// @brief The stages of a feeding, in the order they play.
typedef enum {
    PET_APPROACH_WALKING,
    PET_APPROACH_INCOMING,
    PET_APPROACH_MOUTH,
    PET_APPROACH_SETTLING,
} pet_approach_phase_t;

typedef struct {
    uint8_t pet_position;
    uint8_t prop_position;
    uint8_t phase_ticks;
    pet_approach_phase_t phase;
    bool running;
} pet_approach_t;

/// @brief Starts a feeding: the pet heads for the left edge from wherever it was standing.
void pet_approach_start(pet_approach_t *approach, uint8_t pet_position);

/// @brief Steps the animation on. Returns true on the single tick the food goes down.
bool pet_approach_advance(pet_approach_t *approach);

/// @brief Draws the pet, and the prop while it is still on its way over.
void pet_approach_draw(const pet_approach_t *approach, const char *prop);

void pet_feed(uint8_t nutrition);

/// @brief Raises happiness for a performance. Call once when a song actually starts.
void pet_sing(void);

/// @brief Nudges the awake pet with a random interaction, or scolds you for waking it.
/// @return which interaction played, or PET_INTERACT_COUNT if the pet did nothing.
pet_interact_kind_t pet_interact(void);

/// @brief The current species' reaction sprite for an interaction, alternating with frame.
const char *pet_interact_sprite(pet_interact_kind_t kind, uint8_t frame);

/// @brief The current species' sprite for the top half of a digit.
const char *pet_perch_sprite(void);

/// @brief The current species' singing mouth: shut, then hinged open, alternating with frame.
const char *pet_sing_sprite(uint8_t frame);

/// @brief Prods the sleeping pet awake for a few minutes, at the cost of its mood.
void pet_disturb(void);

/// @brief True when the pet has something to shout about from the background.
bool pet_wants_to_call(void);

/// @brief Plays whatever the pet is shouting about, and stops it shouting again.
void pet_call(void);

#endif // PET_H_

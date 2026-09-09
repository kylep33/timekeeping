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

/*
 * PET
 *
 * The creature behind the pet home, food and play faces. It owns the one copy of
 * the pet, so every face is a view onto the same animal.
 *
 * Needs drain against the wall clock rather than a tick count, so the pet keeps
 * living while the watch sleeps or shows another face. Nothing is stored that can
 * be derived: the daily sleep schedule and the illness roll are hashed from the
 * date, so they hold steady all day and vary from one day to the next.
 */

#include "movement.h"

#define PET_STAT_MAX 100

// The bottom row is the pet's whole world; it wanders the width of it.
#define PET_ROW_LENGTH 6
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

/// @brief Blanks a bottom row buffer, which must hold PET_ROW_LENGTH + 1 characters.
void pet_row_clear(char *row);

/// @brief Draws a sprite into a blanked row, clipping anything past the edge.
void pet_row_place(char *row, uint8_t position, const char *sprite);

/* A prop crossing the row into the pet, which is what feeding looks like. The pet
 * stands aside so the prop has room to arrive.
 */
typedef struct {
    uint8_t prop_position;
    uint8_t hold_ticks;     ///< counts down while the pet reacts, after the prop lands
    bool running;
} pet_approach_t;

void pet_approach_start(pet_approach_t *approach);

/// @brief Steps the animation on. Returns true on the single tick the prop lands.
bool pet_approach_advance(pet_approach_t *approach);

/// @brief Draws the prop until it lands, and the given reaction sprite for the pet.
void pet_approach_draw(const pet_approach_t *approach, const char *prop, const char *reaction);

/// @brief The pet's reaction to a landed approach: eating while it is chewing, its
///        ordinary mood sprite once the plate is clear.
const char *pet_eating_reaction(const pet_approach_t *approach, uint8_t frame);

void pet_feed(uint8_t nutrition);

/// @brief Nudges the awake pet with a random interaction, or scolds you for waking it.
/// @return which interaction played, or PET_INTERACT_COUNT if the pet did nothing.
pet_interact_kind_t pet_interact(void);

/// @brief The current species' reaction sprite for an interaction, alternating with frame.
const char *pet_interact_sprite(pet_interact_kind_t kind, uint8_t frame);

/// @brief Charges the pet's mood for a button press that cut its sleep short.
void pet_disturb(void);

/// @brief True when the pet has something to shout about from the background.
bool pet_wants_to_call(void);

/// @brief Plays whatever the pet is shouting about, and stops it shouting again.
void pet_call(void);

#endif // PET_H_

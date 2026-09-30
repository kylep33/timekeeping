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

typedef enum {
    PET_MOOD_HAPPY,
    PET_MOOD_HUNGRY,
    PET_MOOD_TIRED,
    PET_MOOD_SAD,
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
    uint8_t species;                ///< a pet_species_id_t, stored as a byte since this is saved to flash
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

/// @brief Which pet_species_id_t is alive, without settling its needs.
uint8_t pet_species_id(void);

void pet_feed(uint8_t nutrition);

/// @brief Raises happiness for a performance. Call once when a song actually starts.
void pet_sing(void);

/// @brief Nudges the awake pet with a random interaction, or scolds you for waking it.
/// @return which interaction played, or PET_INTERACT_COUNT if the pet did nothing.
pet_interact_kind_t pet_interact(void);

/// @brief Kills it on the spot. A normal death, but it screams instead of the usual death call.
void pet_murder(void);

/// @brief Prods the sleeping pet awake for a few minutes, at the cost of its mood.
void pet_disturb(void);

/// @brief True when the pet has something to shout about from the background.
bool pet_wants_to_call(void);

/// @brief Plays whatever the pet is shouting about, and stops it shouting again.
void pet_call(void);

#endif // PET_H_

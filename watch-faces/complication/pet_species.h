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

#ifndef PET_SPECIES_H_
#define PET_SPECIES_H_

/*
 * PET SPECIES
 *
 * What makes one pet look, sound and live differently from another. The faces only
 * draw through this, so a new species needs no face changes.
 *
 * Adding one:
 * - give it an id below and declare it next to the others
 * - list it in pet_species.c
 * - write pet_species_<name>.c and add it to watch-faces.mk
 */

#include "pet.h"
#include "pet_default_settings.h"

typedef enum {
    PET_SPECIES_GNOCCI,
    PET_SPECIES_SMILEY,
    PET_SPECIES_COUNT,
} pet_species_id_t;

typedef enum {
    PET_ANIM_PLAYING,
    PET_ANIM_IMPACT,    ///< the one tick it lands, like the food going down
    PET_ANIM_DONE,
} pet_anim_step_t;

/* home_draw owns the whole screen. eat_draw and sing_draw run after the face has put
 * up its title and stat, and can draw over them.
 */
typedef struct {
    /// @brief Changes whichever defaults this species does differently. NULL keeps them all.
    void (*override_settings)(pet_settings_t *settings);

    void (*home_activate)(void);
    void (*home_advance)(pet_mood_t mood);
    void (*home_draw)(pet_mood_t mood);
    void (*react)(pet_interact_kind_t kind);

    void (*eat_start)(void);
    pet_anim_step_t (*eat_advance)(void);
    void (*eat_draw)(const char *food);

    void (*sing_start)(void);
    void (*sing_draw)(bool mouth_open);
} pet_species_t;

extern const pet_species_t pet_species_gnocci;
extern const pet_species_t pet_species_smiley;

const pet_species_t *pet_species_get(pet_species_id_t id);

const pet_species_t *pet_species_current(void);

#endif // PET_SPECIES_H_

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

#include <stdlib.h>
#include "pet_species.h"
#include "pet_grid.h"
#include "pet_screen.h"
#include "watch_common_display.h"

// Gnocci: a one cell blob that wanders the grid.

#define FRAME_COUNT 2

static const uint8_t WANDER_CHOICES = 3;

// Roughly one amble in eight is a step up or down rather than along.
static const uint8_t CLIMB_ODDS = 8;

// The off frame is a blink, so it should be a flicker rather than half the pet's life.
static const uint8_t BLINK_EVERY_N_TICKS = 8;

static const uint8_t REACTION_TICKS = 4;

// Close enough to see the food coming, so it rounds out in anticipation.
static const uint8_t SWELL_DISTANCE = 2;

// Long enough to read as a mouthful and a swallow at the food face's tick rate.
static const uint8_t MOUTH_TICKS = 2;
static const uint8_t SETTLE_TICKS = 2;

static const char *MOOD_SPRITES[PET_MOOD_COUNT][FRAME_COUNT] = {
    [PET_MOOD_HAPPY]    = { "o", "-" },
    [PET_MOOD_HUNGRY]   = { "O", "o" },
    [PET_MOOD_TIRED]    = { "o", "_" },
    [PET_MOOD_SAD]      = { "n", "_" },
    [PET_MOOD_SICK]     = { "x", "X" },
    [PET_MOOD_ASLEEP]   = { "z", " " },
    [PET_MOOD_CRITICAL] = { "@", " " },
    [PET_MOOD_DEAD]     = { "_", "_" },
};

static const char *INTERACT_SPRITES[PET_INTERACT_COUNT][FRAME_COUNT] = {
    [PET_INTERACT_POKE] = { "!", "o" },
    [PET_INTERACT_PAT]  = { "-", "o" },
    [PET_INTERACT_WAVE] = { "O", "o" },
};

// Only a box fits in the top half of a digit.
static const char PERCHED_SPRITE[] = "#";

static const char WAITING_SPRITE[] = "o";
static const char SWELLED_SPRITE[] = "O";
static const char MOUTH_SPRITE[] = "C";

// Open is the shut mouth without its lid. v is the low cup on both LCDs, u isn't.
static const char SING_SHUT_SPRITE[] = "o";
static const char SING_OPEN_SPRITE[] = "v";

/* Always the same scene, so it plays the same every time: the pet snaps to the right
 * edge, and he walks in from the left, stopping well short so the shot has most of
 * the row to cross.
 */
static const uint8_t MURDER_SPOT_COLUMN = PET_SCREEN_BOTTOM_LENGTH - 1;
static const uint8_t GUY_ENTER_COLUMN = 0;
static const uint8_t GUY_STOP_COLUMN = 1;

static const uint8_t AIM_TICKS = 4;
static const uint8_t SCREAM_TICKS = 6;
static const uint8_t DEAD_TICKS = 4;

// F: a vertical grip with the barrel and trigger guard sticking out to the right.
static const char GUY_WALKING_SPRITE[] = "Y";
static const char GUY_AIMING_SPRITE[] = "F";
static const char BULLET_SPRITE[] = "-";
static const char SHOCKED_SPRITE[] = "O";
static const char *SCREAM_SPRITES[FRAME_COUNT] = { "O", "8" };
static const char DEAD_SPRITE[] = "_";

typedef enum {
    EAT_PHASE_WALKING,
    EAT_PHASE_INCOMING,
    EAT_PHASE_MOUTH,
    EAT_PHASE_SETTLING,
} eat_phase_t;

typedef struct {
    uint8_t food_column;
    uint8_t phase_ticks;
    eat_phase_t phase;
} eat_t;

typedef enum {
    MURDER_PHASE_ENTERING,
    MURDER_PHASE_AIMING,
    MURDER_PHASE_FIRING,
    MURDER_PHASE_SCREAMING,
    MURDER_PHASE_DEAD,
} murder_phase_t;

typedef struct {
    uint8_t guy_column;
    uint8_t bullet_column;
    uint8_t phase_ticks;
    murder_phase_t phase;
} murder_t;

// Where it wanders is worth nobody's flash, so it starts mid row each boot.
static pet_grid_spot_t _spot = { PET_GRID_LEVEL_BOTTOM_LOW, PET_SCREEN_BOTTOM_LENGTH / 2 };
static uint8_t _tick;
static uint8_t _reaction_ticks_left;
static pet_interact_kind_t _reaction;
static eat_t _eat;
static murder_t _murder;

// Left, right or stay put, so it drifts about rather than marching wall to wall.
static void _wander(void) {
    if (rand() % CLIMB_ODDS == 0) {
        pet_grid_step(&_spot, (rand() % 2 == 0) ? 1 : -1, 0);
        return;
    }

    pet_grid_step(&_spot, 0, (rand() % WANDER_CHOICES) - 1);
}

static uint8_t _frame_for(pet_mood_t mood) {
    // A pet this close to death should be hard to ignore, so it flashes every tick.
    if (mood == PET_MOOD_CRITICAL) return _tick % FRAME_COUNT;

    return (_tick % BLINK_EVERY_N_TICKS == 0) ? 1 : 0;
}

static const char *_home_sprite(pet_mood_t mood) {
    if (pet_grid_in_top_half(_spot.level)) return PERCHED_SPRITE;
    if (_reaction_ticks_left > 0) return INTERACT_SPRITES[_reaction][_tick % FRAME_COUNT];

    return MOOD_SPRITES[mood][_frame_for(mood)];
}

static void _home_activate(void) {
    _reaction_ticks_left = 0;

    // Eating and singing move it along the bottom row, which is wider than the top strip.
    pet_grid_clamp(&_spot);
}

static void _home_advance(pet_mood_t mood) {
    _tick++;

    // Holds still mid reaction so there's something to actually look at.
    if (_reaction_ticks_left > 0) {
        _reaction_ticks_left--;
        return;
    }

    // Moods need the whole digit to read, so only a comfortable pet climbs.
    if (mood == PET_MOOD_HAPPY || mood == PET_MOOD_HUNGRY) _wander();
    else pet_grid_drop_to_bottom_half(&_spot);
}

static void _home_draw(pet_mood_t mood) {
    watch_clear_colon();
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    pet_grid_draw(&_spot, _home_sprite(mood));
}

static void _react(pet_interact_kind_t kind) {
    pet_grid_drop_to_bottom_half(&_spot);
    _reaction = kind;
    _reaction_ticks_left = REACTION_TICKS;
}

static uint8_t _food_distance(void) {
    return _eat.food_column - _spot.column;
}

static void _eat_enter(eat_phase_t phase) {
    _eat.phase = phase;
    _eat.phase_ticks = 0;
}

// It always eats at the left edge, with the food rolling in from the right.
static void _eat_start(void) {
    _eat.food_column = PET_SCREEN_BOTTOM_LENGTH - 1;
    _eat_enter(EAT_PHASE_WALKING);
}

static pet_anim_step_t _eat_advance(void) {
    _eat.phase_ticks++;

    switch (_eat.phase) {
        case EAT_PHASE_WALKING:
            if (_spot.column > 0) _spot.column--;
            else _eat_enter(EAT_PHASE_INCOMING);
            break;
        case EAT_PHASE_INCOMING:
            // Stops in the cell next to the pet, which is where it goes down.
            if (_food_distance() > 1) {
                _eat.food_column--;
                break;
            }

            _eat_enter(EAT_PHASE_MOUTH);
            return PET_ANIM_IMPACT;
        case EAT_PHASE_MOUTH:
            if (_eat.phase_ticks >= MOUTH_TICKS) _eat_enter(EAT_PHASE_SETTLING);
            break;
        case EAT_PHASE_SETTLING:
            if (_eat.phase_ticks >= SETTLE_TICKS) return PET_ANIM_DONE;
            break;
    }

    return PET_ANIM_PLAYING;
}

static const char *_eat_sprite(void) {
    switch (_eat.phase) {
        case EAT_PHASE_WALKING:
            return MOOD_SPRITES[pet_mood()][_eat.phase_ticks % FRAME_COUNT];
        case EAT_PHASE_INCOMING:
            return _food_distance() <= SWELL_DISTANCE ? SWELLED_SPRITE : WAITING_SPRITE;
        case EAT_PHASE_MOUTH:
            return MOUTH_SPRITE;
        default:
            return WAITING_SPRITE;
    }
}

static void _eat_draw(const char *food) {
    char bottom[PET_SCREEN_BOTTOM_LENGTH + 1];

    pet_screen_bottom_clear(bottom);
    pet_screen_bottom_place(bottom, _spot.column, _eat_sprite());

    // The food is only on screen while it's travelling; after that it's eaten.
    if (_eat.phase == EAT_PHASE_INCOMING) pet_screen_bottom_place(bottom, _eat.food_column, food);

    watch_display_text(WATCH_POSITION_BOTTOM, bottom);
}

// A fresh spot for every song, rather than wherever the last face left it.
static void _sing_start(void) {
    _spot.column = rand() % (PET_SCREEN_BOTTOM_LENGTH - PET_GRID_SPRITE_WIDTH + 1);
}

static void _sing_draw(bool mouth_open) {
    char bottom[PET_SCREEN_BOTTOM_LENGTH + 1];

    pet_screen_bottom_clear(bottom);
    pet_screen_bottom_place(bottom, _spot.column, mouth_open ? SING_OPEN_SPRITE : SING_SHUT_SPRITE);
    watch_display_text(WATCH_POSITION_BOTTOM, bottom);
}

static uint8_t _distance(uint8_t from, uint8_t to) {
    return from > to ? from - to : to - from;
}

static void _murder_enter(murder_phase_t phase) {
    _murder.phase = phase;
    _murder.phase_ticks = 0;
}

static void _murder_start(void) {
    _spot.column = MURDER_SPOT_COLUMN;
    _murder.guy_column = GUY_ENTER_COLUMN;
    _murder_enter(MURDER_PHASE_ENTERING);
}

static pet_anim_step_t _murder_advance(void) {
    _murder.phase_ticks++;

    switch (_murder.phase) {
        case MURDER_PHASE_ENTERING:
            if (_murder.guy_column < GUY_STOP_COLUMN) _murder.guy_column++;
            else _murder_enter(MURDER_PHASE_AIMING);
            break;
        case MURDER_PHASE_AIMING:
            if (_murder.phase_ticks < AIM_TICKS) break;

            // The barrel is already poking out a cell ahead; firing just sends it flying.
            _murder.bullet_column = _murder.guy_column + 1;
            _murder_enter(MURDER_PHASE_FIRING);
            break;
        case MURDER_PHASE_FIRING:
            if (_distance(_murder.bullet_column, _spot.column) > 1) {
                _murder.bullet_column++;
                break;
            }

            _murder_enter(MURDER_PHASE_SCREAMING);
            return PET_ANIM_IMPACT;
        case MURDER_PHASE_SCREAMING:
            if (_murder.phase_ticks >= SCREAM_TICKS) _murder_enter(MURDER_PHASE_DEAD);
            break;
        case MURDER_PHASE_DEAD:
            if (_murder.phase_ticks >= DEAD_TICKS) return PET_ANIM_DONE;
            break;
    }

    return PET_ANIM_PLAYING;
}

static const char *_murder_pet_sprite(void) {
    switch (_murder.phase) {
        case MURDER_PHASE_ENTERING:
            return MOOD_SPRITES[pet_mood()][_murder.phase_ticks % FRAME_COUNT];
        case MURDER_PHASE_AIMING:
        case MURDER_PHASE_FIRING:
            return SHOCKED_SPRITE;
        case MURDER_PHASE_SCREAMING:
            return SCREAM_SPRITES[_murder.phase_ticks % FRAME_COUNT];
        default:
            return DEAD_SPRITE;
    }
}

static const char *_guy_sprite(void) {
    return _murder.phase == MURDER_PHASE_ENTERING ? GUY_WALKING_SPRITE : GUY_AIMING_SPRITE;
}

static void _murder_draw(void) {
    char bottom[PET_SCREEN_BOTTOM_LENGTH + 1];

    pet_screen_bottom_clear(bottom);
    pet_screen_bottom_place(bottom, _spot.column, _murder_pet_sprite());
    pet_screen_bottom_place(bottom, _murder.guy_column, _guy_sprite());

    // Ready during the aim, then the same dash flies on as the shot.
    if (_murder.phase == MURDER_PHASE_AIMING) pet_screen_bottom_place(bottom, _murder.guy_column + 1, BULLET_SPRITE);
    if (_murder.phase == MURDER_PHASE_FIRING) pet_screen_bottom_place(bottom, _murder.bullet_column, BULLET_SPRITE);

    watch_display_text(WATCH_POSITION_BOTTOM, bottom);
}

const pet_species_t pet_species_gnocci = {
    .override_settings = NULL,
    .home_activate = _home_activate,
    .home_advance = _home_advance,
    .home_draw = _home_draw,
    .react = _react,
    .eat_start = _eat_start,
    .eat_advance = _eat_advance,
    .eat_draw = _eat_draw,
    .sing_start = _sing_start,
    .sing_draw = _sing_draw,
    .murder_start = _murder_start,
    .murder_advance = _murder_advance,
    .murder_draw = _murder_draw,
};

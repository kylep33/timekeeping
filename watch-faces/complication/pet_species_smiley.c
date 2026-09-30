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

#include "pet_species.h"
#include "pet_screen.h"
#include "watch_common_display.h"

// Smiley: the whole face sits in the big digits, eyes at either end and the mouth between.

#define FRAME_COUNT 2

// The top row is too small and oddly shaped to hold eyes that match the mouth.
static const uint8_t FACE_COLUMN = 0;
static const uint8_t FACE_WIDTH = 4;

static const uint8_t BLINK_EVERY_N_TICKS = 8;
static const uint8_t REACTION_TICKS = 4;

// Opens up before the food shows, so you can tell what's about to happen.
static const uint8_t OPEN_TICKS = 2;
static const uint8_t CHEW_TICKS = 4;

// Every face is FACE_WIDTH characters. # is a raised o, for eyes wide open.
static const char *MOOD_FACES[PET_MOOD_COUNT][FRAME_COUNT] = {
    [PET_MOOD_HAPPY]    = { "^__^", "-__-" },
    [PET_MOOD_HUNGRY]   = { "#oo#", "#__#" },
    [PET_MOOD_SICK]     = { "xnnx", "-nn-" },
    [PET_MOOD_ASLEEP]   = { "-__-", "----" },
    [PET_MOOD_CRITICAL] = { "xnnx", "    " },
    [PET_MOOD_DEAD]     = { "x__x", "x__x" },
};

static const char *INTERACT_FACES[PET_INTERACT_COUNT][FRAME_COUNT] = {
    [PET_INTERACT_POKE] = { "#oo#", "#__#" },
    [PET_INTERACT_PAT]  = { "-__-", "^__^" },
    [PET_INTERACT_WAVE] = { "-__^", "^__^" },
};

static const char WAITING_FACE[] = "#oo#";
static const char *CHEWING_FACES[FRAME_COUNT] = { "^__^", "^oo^" };

static const char SING_SHUT_FACE[] = "^__^";
static const char SING_OPEN_FACE[] = "^oo^";

// The last cell, leaving the one between it and the face for the bullet.
static const uint8_t GUY_COLUMN = PET_SCREEN_BOTTOM_LENGTH - 1;
static const uint8_t ENTER_TICKS = 2;
static const uint8_t AIM_TICKS = 4;
static const uint8_t SCREAM_TICKS = 6;
static const uint8_t DEAD_TICKS = 4;
static const uint8_t IDLE_FRAME = 0;

// A 7 is an arm pointing left off the top of a body.
static const char GUY_WALKING_SPRITE[] = "Y";
static const char GUY_AIMING_SPRITE[] = "7";
static const char BULLET_SPRITE[] = "-";

static const char SHOCKED_FACE[] = "#oo#";
static const char *SCREAMING_FACES[FRAME_COUNT] = { "#oo#", "xoox" };
static const char DEAD_FACE[] = "x__x";

// Gulps rather than chirps, since it's all mouth.
static int8_t _tune_gulp[] = {
    BUZZER_NOTE_C5, 3,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_C5, 3,
    0
};

typedef enum {
    EAT_PHASE_OPENING,
    EAT_PHASE_INCOMING,
    EAT_PHASE_CHEWING,
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
    uint8_t bullet_column;
    uint8_t phase_ticks;
    murder_phase_t phase;
} murder_t;

static uint8_t _tick;
static uint8_t _reaction_ticks_left;
static pet_interact_kind_t _reaction;
static eat_t _eat;
static murder_t _murder;

static void _override_settings(pet_settings_t *settings) {
    settings->sounds.eat = _tune_gulp;
}

// The bottom row can already hold food, which the face goes on top of.
static void _draw_face_over(const char *face, char *bottom) {
    pet_screen_bottom_place(bottom, FACE_COLUMN, face);
    watch_display_text(WATCH_POSITION_BOTTOM, bottom);
    // The colon lands in the middle of the mouth.
    watch_clear_colon();
}

static void _draw_face(const char *face) {
    char bottom[PET_SCREEN_BOTTOM_LENGTH + 1];

    pet_screen_bottom_clear(bottom);
    _draw_face_over(face, bottom);
}

static uint8_t _frame_for(pet_mood_t mood) {
    // Flashing every tick is hard to ignore, which is the point this close to death.
    if (mood == PET_MOOD_CRITICAL) return _tick % FRAME_COUNT;

    return (_tick % BLINK_EVERY_N_TICKS == 0) ? 1 : 0;
}

static void _home_activate(void) {
    _reaction_ticks_left = 0;
}

static void _home_advance(pet_mood_t mood) {
    (void) mood;

    _tick++;
    if (_reaction_ticks_left > 0) _reaction_ticks_left--;
}

static void _home_draw(pet_mood_t mood) {
    char top_left[PET_SCREEN_TOP_LENGTH + 1];

    pet_screen_top_clear(top_left);
    pet_screen_top_draw(top_left);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    if (_reaction_ticks_left > 0) {
        _draw_face(INTERACT_FACES[_reaction][_tick % FRAME_COUNT]);
        return;
    }

    _draw_face(MOOD_FACES[mood][_frame_for(mood)]);
}

static void _react(pet_interact_kind_t kind) {
    _reaction = kind;
    _reaction_ticks_left = REACTION_TICKS;
}

static void _eat_enter(eat_phase_t phase) {
    _eat.phase = phase;
    _eat.phase_ticks = 0;
}

static void _eat_start(void) {
    _eat.food_column = PET_SCREEN_BOTTOM_LENGTH - 1;
    _eat_enter(EAT_PHASE_OPENING);
}

static pet_anim_step_t _eat_advance(void) {
    _eat.phase_ticks++;

    switch (_eat.phase) {
        case EAT_PHASE_OPENING:
            if (_eat.phase_ticks >= OPEN_TICKS) _eat_enter(EAT_PHASE_INCOMING);
            break;
        case EAT_PHASE_INCOMING:
            if (_eat.food_column > FACE_COLUMN + FACE_WIDTH) {
                _eat.food_column--;
                break;
            }

            _eat_enter(EAT_PHASE_CHEWING);
            return PET_ANIM_IMPACT;
        case EAT_PHASE_CHEWING:
            if (_eat.phase_ticks >= CHEW_TICKS) return PET_ANIM_DONE;
            break;
    }

    return PET_ANIM_PLAYING;
}

static void _eat_draw(const char *food) {
    char bottom[PET_SCREEN_BOTTOM_LENGTH + 1];

    pet_screen_bottom_clear(bottom);

    if (_eat.phase == EAT_PHASE_CHEWING) {
        _draw_face_over(CHEWING_FACES[_eat.phase_ticks % FRAME_COUNT], bottom);
        return;
    }

    pet_screen_bottom_place(bottom, _eat.food_column, food);
    _draw_face_over(WAITING_FACE, bottom);
}

// Already fills the screen, so there's nowhere to move to.
static void _sing_start(void) {
}

static void _sing_draw(bool mouth_open) {
    _draw_face(mouth_open ? SING_OPEN_FACE : SING_SHUT_FACE);
}

static void _murder_enter(murder_phase_t phase) {
    _murder.phase = phase;
    _murder.phase_ticks = 0;
}

static void _murder_start(void) {
    _murder_enter(MURDER_PHASE_ENTERING);
}

static pet_anim_step_t _murder_advance(void) {
    _murder.phase_ticks++;

    switch (_murder.phase) {
        case MURDER_PHASE_ENTERING:
            if (_murder.phase_ticks >= ENTER_TICKS) _murder_enter(MURDER_PHASE_AIMING);
            break;
        case MURDER_PHASE_AIMING:
            if (_murder.phase_ticks < AIM_TICKS) break;

            _murder.bullet_column = GUY_COLUMN - 1;
            _murder_enter(MURDER_PHASE_FIRING);
            break;
        case MURDER_PHASE_FIRING:
            // Hits in the cell next to the face.
            if (_murder.bullet_column > FACE_COLUMN + FACE_WIDTH) {
                _murder.bullet_column--;
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

static const char *_murder_face(void) {
    switch (_murder.phase) {
        case MURDER_PHASE_ENTERING:
            return MOOD_FACES[pet_mood()][IDLE_FRAME];
        case MURDER_PHASE_AIMING:
        case MURDER_PHASE_FIRING:
            return SHOCKED_FACE;
        case MURDER_PHASE_SCREAMING:
            return SCREAMING_FACES[_murder.phase_ticks % FRAME_COUNT];
        default:
            return DEAD_FACE;
    }
}

static void _murder_draw(void) {
    char bottom[PET_SCREEN_BOTTOM_LENGTH + 1];
    const char *guy = _murder.phase == MURDER_PHASE_ENTERING ? GUY_WALKING_SPRITE : GUY_AIMING_SPRITE;

    pet_screen_bottom_clear(bottom);
    pet_screen_bottom_place(bottom, GUY_COLUMN, guy);
    if (_murder.phase == MURDER_PHASE_FIRING) pet_screen_bottom_place(bottom, _murder.bullet_column, BULLET_SPRITE);

    _draw_face_over(_murder_face(), bottom);
}

const pet_species_t pet_species_smiley = {
    .override_settings = _override_settings,
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

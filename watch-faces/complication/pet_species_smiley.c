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

// Smiley: the whole screen is its face. Eyes on the top row, the colon for a nose.

#define FRAME_COUNT 2

// The two digits either side of the colon, so the mouth sits under the nose.
static const uint8_t MOUTH_COLUMN = 1;
static const uint8_t MOUTH_WIDTH = 2;

static const uint8_t BLINK_EVERY_N_TICKS = 8;
static const uint8_t REACTION_TICKS = 4;

// Opens up before the food shows, so you can tell what's about to happen.
static const uint8_t OPEN_TICKS = 2;
static const uint8_t CHEW_TICKS = 4;

typedef struct {
    char left_eye;
    char right_eye;
    const char *mouth;  ///< MOUTH_WIDTH characters
} smiley_face_t;

static const smiley_face_t MOOD_FACES[PET_MOOD_COUNT][FRAME_COUNT] = {
    [PET_MOOD_HAPPY]    = { { 'o', 'o', "LJ" }, { '-', '-', "LJ" } },
    [PET_MOOD_HUNGRY]   = { { 'o', 'o', "[]" }, { 'o', 'o', "__" } },
    [PET_MOOD_SICK]     = { { 'x', 'x', "nn" }, { '-', '-', "nn" } },
    [PET_MOOD_ASLEEP]   = { { '_', '_', "__" }, { '_', '_', "--" } },
    [PET_MOOD_CRITICAL] = { { 'x', 'x', "nn" }, { ' ', ' ', "  " } },
    [PET_MOOD_DEAD]     = { { 'x', 'x', "__" }, { 'x', 'x', "__" } },
};

static const smiley_face_t INTERACT_FACES[PET_INTERACT_COUNT][FRAME_COUNT] = {
    [PET_INTERACT_POKE] = { { 'O', 'O', "[]" }, { 'o', 'o', "[]" } },
    [PET_INTERACT_PAT]  = { { '^', '^', "LJ" }, { '-', '-', "LJ" } },
    [PET_INTERACT_WAVE] = { { '-', 'o', "LJ" }, { 'o', 'o', "LJ" } },
};

static const smiley_face_t WAITING_FACE = { 'O', 'O', "[]" };
static const smiley_face_t CHEWING_FACES[FRAME_COUNT] = { { '^', '^', "__" }, { '^', '^', "[]" } };

static const smiley_face_t SING_SHUT_FACE = { '^', '^', "__" };
static const smiley_face_t SING_OPEN_FACE = { '^', '^', "[]" };

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

static uint8_t _tick;
static uint8_t _reaction_ticks_left;
static pet_interact_kind_t _reaction;
static eat_t _eat;

static void _override_settings(pet_settings_t *settings) {
    settings->sounds.eat = _tune_gulp;
}

// The bottom row can already hold food, which the mouth goes on top of.
static void _draw_face_over(const smiley_face_t *face, char *bottom) {
    char top_left[] = { ' ', face->left_eye, ' ', '\0' };
    char top_right[] = { face->right_eye, ' ', '\0' };

    pet_screen_bottom_place(bottom, MOUTH_COLUMN, face->mouth);
    pet_screen_top_draw(top_left);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, top_right);
    watch_display_text(WATCH_POSITION_BOTTOM, bottom);
    watch_set_colon();
}

static void _draw_face(const smiley_face_t *face) {
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
    if (_reaction_ticks_left > 0) {
        _draw_face(&INTERACT_FACES[_reaction][_tick % FRAME_COUNT]);
        return;
    }

    _draw_face(&MOOD_FACES[mood][_frame_for(mood)]);
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
            if (_eat.food_column > MOUTH_COLUMN + MOUTH_WIDTH) {
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
        _draw_face_over(&CHEWING_FACES[_eat.phase_ticks % FRAME_COUNT], bottom);
        return;
    }

    pet_screen_bottom_place(bottom, _eat.food_column, food);
    _draw_face_over(&WAITING_FACE, bottom);
}

// Already fills the screen, so there's nowhere to move to.
static void _sing_start(void) {
}

static void _sing_draw(bool mouth_open) {
    _draw_face(mouth_open ? &SING_OPEN_FACE : &SING_SHUT_FACE);
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
};

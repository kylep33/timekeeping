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
#include <stdio.h>
#include <string.h>
#include "pet.h"
#include "filesystem.h"
#include "watch_common_display.h"
#include "watch_utility.h"

static const uint32_t SECONDS_PER_MINUTE = 60;
static const uint32_t SECONDS_PER_HOUR = 3600;
static const uint32_t SECONDS_PER_DAY = 86400;
static const uint16_t MINUTES_PER_HOUR = 60;

/* A pet fed to the brim runs down over roughly two days, so a day away from it is
 * survivable and a long weekend of neglect is not.
 */
static const uint32_t HUNGER_DRAIN_S_PER_POINT = 1800;
static const uint32_t HAPPINESS_DRAIN_S_PER_POINT = 2700;
static const uint32_t HEALTH_DRAIN_S_PER_POINT = 1200;
static const uint32_t HEALTH_RECOVERY_S_PER_POINT = 900;

static const uint8_t NEGLECT_THRESHOLD = 20;
static const uint8_t HUNGRY_THRESHOLD = 35;
static const uint8_t ILLNESS_HEALTH_THRESHOLD = 30;

static const uint16_t BEDTIME_EARLIEST_HOUR = 20;
static const uint16_t BEDTIME_SPREAD_MIN = 120;
static const uint16_t WAKE_EARLIEST_HOUR = 7;
static const uint16_t WAKE_SPREAD_MIN = 150;

static const uint16_t ILLNESS_ODDS = 12;
static const uint8_t POKES_TO_CURE = 3;

static const uint8_t POKE_HAPPINESS_GAIN = 6;
static const uint8_t PAT_HAPPINESS_GAIN = 5;
static const uint8_t WAVE_HAPPINESS_GAIN = 3;
static const uint8_t SING_HAPPINESS_GAIN = 8;
static const uint8_t DISTURB_HAPPINESS_COST = 5;
static const uint8_t HATCH_NEED = 80;

// Prodded awake, it stays up long enough for a meal and a game before nodding off again.
static const uint32_t NUDGED_AWAKE_MIN = 10;

static const uint32_t CRITICAL_GRACE_HOURS = 12;
static const uint32_t CALL_INTERVAL_MIN = 15;
static const uint32_t CRITICAL_CALL_INTERVAL_MIN = 5;

static const uint32_t HASH_MULTIPLIER = 2654435761u;    // Knuth's golden ratio constant
static const uint8_t HASH_DISCARD_BITS = 16;            // a multiply barely stirs its low bits
static const uint16_t DAY_STRIDE_PER_YEAR = 372;        // 12 * 31, so every date gets its own number
static const uint16_t DAY_STRIDE_PER_MONTH = 31;

static const uint32_t SALT_BEDTIME = 1;
static const uint32_t SALT_WAKE = 2;
static const uint32_t SALT_ILLNESS = 3;

static const uint8_t PET_FORMAT_VERSION = 2;
static char PET_FILE_NAME[] = "pet.dat";

// The top right has two digits, and a full 100 wrapping round to 0 read as an empty stat.
static const uint8_t TOP_RIGHT_MAX = 99;

// Close enough that the pet can tell what is coming, and rounds out in anticipation.
static const uint8_t APPROACH_SWELL_DISTANCE = 2;

// Long enough to read as a mouthful and a swallow at the food face's tick rate.
static const uint8_t APPROACH_MOUTH_TICKS = 2;
static const uint8_t APPROACH_SETTLE_TICKS = 2;

/* What a pet looks like: its idle sprite per mood, its reaction to being interacted
 * with, and how it chews. All seven segment glyphs, and drawable in both strips it
 * wanders, though the classic LCD rounds a few of them up to uppercase in the top
 * left. Only one species exists today (Gnocci); more can join this table later
 * without the rest of the module changing.
 */
typedef struct {
    const char *mood_sprites[PET_MOOD_COUNT][PET_ANIMATION_FRAMES];
    const char *interact_sprites[PET_INTERACT_COUNT][PET_ANIMATION_FRAMES];
    const char *sing_mouth_shut;  ///< the closed half of the singing hinge; the open half is LCD-specific
    const char *eating_ball;    ///< sat waiting, with the food still some way off
    const char *eating_swell;   ///< rounded out, the food nearly here
    const char *eating_mouth;   ///< open, taking it
} pet_species_t;

static const pet_species_t SPECIES_GNOCCI = {
    .mood_sprites = {
        [PET_MOOD_HAPPY]    = { "o", "-" },
        [PET_MOOD_HUNGRY]   = { "O", "o" },
        [PET_MOOD_SICK]     = { "x", "X" },
        [PET_MOOD_ASLEEP]   = { "z", " " },
        [PET_MOOD_CRITICAL] = { "@", " " },
        [PET_MOOD_DEAD]     = { "_", "_" },
    },
    .interact_sprites = {
        [PET_INTERACT_POKE] = { "!", "o" },
        [PET_INTERACT_PAT]  = { "-", "o" },
        [PET_INTERACT_WAVE] = { "O", "o" },
    },
    .sing_mouth_shut = "o",
    .eating_ball = "o",
    .eating_swell = "O",
    .eating_mouth = "C",
};

static const pet_species_t *_species(void) {
    return &SPECIES_GNOCCI;
}

typedef enum {
    PET_CALL_NONE,
    PET_CALL_HUNGRY,
    PET_CALL_SICK,
    PET_CALL_CRITICAL,
    PET_CALL_SLEEPING,
    PET_CALL_WAKING,
    PET_CALL_DIED,
    PET_CALL_COUNT,
} pet_call_t;

static int8_t _tune_hungry[] = {
    BUZZER_NOTE_E7, 3,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E7, 3,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_G7, 6,
    0
};

static int8_t _tune_sick[] = {
    BUZZER_NOTE_C5, 8,
    BUZZER_NOTE_A4, 8,
    BUZZER_NOTE_C5, 8,
    BUZZER_NOTE_A4, 12,
    0
};

static int8_t _tune_critical[] = {
    BUZZER_NOTE_C4, 10,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_C4, 10,
    BUZZER_NOTE_REST, 3,
    BUZZER_NOTE_C4, 20,
    0
};

static int8_t _tune_sleeping[] = {
    BUZZER_NOTE_G5, 8,
    BUZZER_NOTE_E5, 8,
    BUZZER_NOTE_C5, 16,
    0
};

static int8_t _tune_waking[] = {
    BUZZER_NOTE_C5, 8,
    BUZZER_NOTE_E5, 8,
    BUZZER_NOTE_G5, 16,
    0
};

static int8_t _tune_died[] = {
    BUZZER_NOTE_G4, 12,
    BUZZER_NOTE_E4, 12,
    BUZZER_NOTE_C4, 12,
    BUZZER_NOTE_A3, 30,
    0
};

static int8_t _tune_eat[] = {
    BUZZER_NOTE_C6, 3,
    BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E6, 5,
    0
};

static int8_t _tune_poke[] = {
    BUZZER_NOTE_A6, 3,
    0
};

static int8_t _tune_pat[] = {
    BUZZER_NOTE_G5, 3,
    0
};

static int8_t _tune_wave[] = {
    BUZZER_NOTE_C5, 2,
    BUZZER_NOTE_E5, 2,
    0
};

static int8_t *const CALL_TUNES[PET_CALL_COUNT] = {
    [PET_CALL_NONE] = NULL,
    [PET_CALL_HUNGRY] = _tune_hungry,
    [PET_CALL_SICK] = _tune_sick,
    [PET_CALL_CRITICAL] = _tune_critical,
    [PET_CALL_SLEEPING] = _tune_sleeping,
    [PET_CALL_WAKING] = _tune_waking,
    [PET_CALL_DIED] = _tune_died,
};

static pet_t _pet;
static bool _loaded;
static pet_call_t _pending_call;
static uint32_t _called_at_s;

// Where it wanders to is worth nobody's flash, so it starts mid row each boot.
static uint8_t _position = PET_ROW_LENGTH / 2;

static uint8_t _raise(uint8_t stat, uint8_t amount) {
    return (amount > PET_STAT_MAX - stat) ? PET_STAT_MAX : stat + amount;
}

static uint8_t _lower(uint8_t stat, uint8_t amount) {
    return (amount > stat) ? 0 : stat - amount;
}

static uint16_t _day_number(watch_date_time_t now) {
    return (now.unit.year * DAY_STRIDE_PER_YEAR) + (now.unit.month * DAY_STRIDE_PER_MONTH) + now.unit.day;
}

/* One hash for the sleep schedule and the illness roll to share. Habits vary from day
 * to day but hold steady within a day, which is why none of them need storing.
 */
static uint32_t _day_hash(watch_date_time_t now, uint32_t salt) {
    return ((uint32_t)(_day_number(now) + salt) * HASH_MULTIPLIER) >> HASH_DISCARD_BITS;
}

static bool _is_bedtime(watch_date_time_t now) {
    uint16_t bedtime_min = (BEDTIME_EARLIEST_HOUR * MINUTES_PER_HOUR) + (_day_hash(now, SALT_BEDTIME) % BEDTIME_SPREAD_MIN);
    uint16_t wake_min = (WAKE_EARLIEST_HOUR * MINUTES_PER_HOUR) + (_day_hash(now, SALT_WAKE) % WAKE_SPREAD_MIN);
    uint16_t now_min = (now.unit.hour * MINUTES_PER_HOUR) + now.unit.minute;

    return now_min >= bedtime_min || now_min < wake_min;
}

static uint16_t _age_days(uint32_t now_s) {
    if (now_s <= _pet.hatched_at_s) return 0;

    return (now_s - _pet.hatched_at_s) / SECONDS_PER_DAY;
}

static uint32_t _now_s(void) {
    return watch_utility_date_time_to_unix_time(movement_get_local_date_time(), 0);
}

static void _save(void) {
    filesystem_write_file(PET_FILE_NAME, (char *) &_pet, sizeof(_pet));
}

/* A pet written by an older build cannot be read back safely, so it is replaced
 * rather than trusted. The same path covers a missing or truncated file.
 */
static void _load(void) {
    if (_loaded) return;
    _loaded = true;

    memset(&_pet, 0, sizeof(_pet));
    if (filesystem_read_file(PET_FILE_NAME, (char *) &_pet, sizeof(_pet)) && _pet.version == PET_FORMAT_VERSION) return;

    memset(&_pet, 0, sizeof(_pet));
    pet_hatch();
}

// Turns banked time into whole points, keeping the remainder so slow drift is not lost to rounding.
static uint8_t _apply_drift(uint8_t stat, uint32_t *debt_s, uint32_t s_per_point, bool rising) {
    uint32_t points = *debt_s / s_per_point;

    *debt_s -= points * s_per_point;
    if (points > PET_STAT_MAX) points = PET_STAT_MAX;

    return rising ? _raise(stat, points) : _lower(stat, points);
}

static void _settle_needs(uint32_t elapsed_s) {
    _pet.hunger_debt_s += elapsed_s;
    _pet.hunger = _apply_drift(_pet.hunger, &_pet.hunger_debt_s, HUNGER_DRAIN_S_PER_POINT, false);

    _pet.happiness_debt_s += elapsed_s;
    _pet.happiness = _apply_drift(_pet.happiness, &_pet.happiness_debt_s, HAPPINESS_DRAIN_S_PER_POINT, false);
}

// Health only slides while something else is wrong, so a cared for pet climbs back to full.
static void _settle_health(uint32_t elapsed_s) {
    bool suffering = _pet.sick || _pet.hunger <= NEGLECT_THRESHOLD || _pet.happiness <= NEGLECT_THRESHOLD;

    _pet.health_debt_s += elapsed_s;
    if (suffering) {
        _pet.health = _apply_drift(_pet.health, &_pet.health_debt_s, HEALTH_DRAIN_S_PER_POINT, false);
        return;
    }

    _pet.health = _apply_drift(_pet.health, &_pet.health_debt_s, HEALTH_RECOVERY_S_PER_POINT, true);
}

static void _fall_ill(void) {
    if (_pet.sick) return;

    _pet.sick = true;
    _pet.pokes_while_sick = 0;
    _save();
}

/* One chance to fall ill a day, whether from poor health or from plain bad luck, so
 * that curing an illness does not hand the pet the same one straight back. Neglect
 * still tells: the next day finds it ill again until its health is seen to.
 */
static void _settle_illness(watch_date_time_t now) {
    if (_pet.sick) return;

    uint16_t today = _day_number(now);
    if (today == _pet.rolled_on_day) return;

    _pet.rolled_on_day = today;

    if (_pet.health <= ILLNESS_HEALTH_THRESHOLD) {
        _fall_ill();
        return;
    }

    if (_day_hash(now, SALT_ILLNESS) % ILLNESS_ODDS == 0) _fall_ill();
}

// Health hitting zero starts a countdown rather than ending one, so a day away is recoverable.
static void _settle_mortality(uint32_t now_s) {
    if (_pet.health > 0) {
        _pet.critical_since_s = 0;
        return;
    }

    // A clock wound back behind the countdown would otherwise read as a huge overrun.
    if (_pet.critical_since_s == 0 || now_s < _pet.critical_since_s) {
        _pet.critical_since_s = now_s;
        return;
    }

    if (now_s - _pet.critical_since_s < CRITICAL_GRACE_HOURS * SECONDS_PER_HOUR) return;

    _pet.dead = true;
    _pending_call = PET_CALL_DIED;
    _save();
}

/* Both ends of the night are worth announcing, so the transition is what gets saved.
 * Being prodded awake holds the night off for a few minutes, and when that runs out
 * the pet drops back off on its own.
 */
static void _settle_sleep(watch_date_time_t now, uint32_t now_s) {
    bool asleep = _is_bedtime(now) && now_s >= _pet.awake_until_s;
    if (asleep == _pet.asleep) return;

    _pet.asleep = asleep;
    _pending_call = asleep ? PET_CALL_SLEEPING : PET_CALL_WAKING;
    _save();
}

static void _settle(void) {
    _load();
    if (_pet.dead) return;

    watch_date_time_t now = movement_get_local_date_time();
    uint32_t now_s = watch_utility_date_time_to_unix_time(now, 0);

    /* Only the drain needs elapsed time, and a clock wound backwards would bank a drain
     * the pet never lived through. Everything below reads the clock as it stands, so it
     * settles even when no time has passed since the last look.
     */
    if (now_s > _pet.settled_at_s) {
        uint32_t elapsed_s = now_s - _pet.settled_at_s;

        _settle_needs(elapsed_s);
        _settle_health(elapsed_s);
    }

    _pet.settled_at_s = now_s;
    _settle_illness(now);
    _settle_mortality(now_s);
    _settle_sleep(now, now_s);
}

const pet_t *pet_get(void) {
    _settle();

    return &_pet;
}

// The very first pet has no predecessor, so there is no age to retire into the record.
static uint16_t _record_after_retiring(uint32_t now_s) {
    if (_pet.hatched_at_s == 0) return _pet.best_age_days;

    uint16_t retired_age_days = _age_days(now_s);

    return (retired_age_days > _pet.best_age_days) ? retired_age_days : _pet.best_age_days;
}

void pet_hatch(void) {
    uint32_t now_s = _now_s();
    uint16_t best_age_days = _record_after_retiring(now_s);
    uint8_t generation = _pet.generation;

    memset(&_pet, 0, sizeof(_pet));
    _pet.version = PET_FORMAT_VERSION;
    _pet.generation = generation + 1;
    _pet.best_age_days = best_age_days;
    _pet.hatched_at_s = now_s;
    _pet.settled_at_s = now_s;
    _pet.hunger = HATCH_NEED;
    _pet.happiness = HATCH_NEED;
    _pet.health = PET_STAT_MAX;

    _loaded = true;
    _pending_call = PET_CALL_NONE;
    _save();
}

pet_mood_t pet_mood(void) {
    const pet_t *pet = pet_get();

    if (pet->dead) return PET_MOOD_DEAD;
    if (pet->critical_since_s != 0) return PET_MOOD_CRITICAL;
    if (pet->sick) return PET_MOOD_SICK;
    if (pet->asleep) return PET_MOOD_ASLEEP;
    if (pet->hunger <= HUNGRY_THRESHOLD) return PET_MOOD_HUNGRY;

    return PET_MOOD_HAPPY;
}

uint16_t pet_age_days(void) {
    return _age_days(pet_get()->settled_at_s);
}

const char *pet_sprite(pet_mood_t mood, uint8_t frame) {
    if (mood >= PET_MOOD_COUNT) mood = PET_MOOD_HAPPY;

    return _species()->mood_sprites[mood][frame % PET_ANIMATION_FRAMES];
}

/* The top half of a digit is drawn the same way on both LCDs, but the classic one's
 * wiring only keeps the bottom half clean at some positions. Good enough for a hop
 * that reads as lifting off rather than teleporting.
 */
const char *pet_rise_sprite(bool stretched) {
    if (stretched) return "^";

    return watch_get_lcd_type() == WATCH_LCD_TYPE_CUSTOM ? "u" : "v";
}

const char *pet_interact_sprite(pet_interact_kind_t kind, uint8_t frame) {
    if (kind >= PET_INTERACT_COUNT) kind = PET_INTERACT_POKE;

    return _species()->interact_sprites[kind][frame % PET_ANIMATION_FRAMES];
}

/* A closed mouth is C+D+E+G: a small box sitting low in the digit, its lid resting on
 * the middle segment. Rather than a wider glyph for open, the lid itself hinges up to
 * the top segment - which is exactly the low, lidless box pet_rise_sprite already
 * draws, so the open frame borrows it instead of duplicating the LCD-type check.
 */
const char *pet_sing_sprite(uint8_t frame) {
    if (frame % PET_ANIMATION_FRAMES == 0) return _species()->sing_mouth_shut;

    return pet_rise_sprite(false);
}

uint8_t pet_position(void) {
    return _position;
}

void pet_set_position(uint8_t position) {
    _position = position;
}

uint8_t pet_top_row_length(void) {
    // The classic LCD's top left is two cells; the custom one adds a third.
    return watch_get_lcd_type() == WATCH_LCD_TYPE_CUSTOM ? PET_TOP_ROW_LENGTH : PET_TOP_ROW_LENGTH - 1;
}

static void _row_clear(char *row, uint8_t length) {
    memset(row, ' ', length);
    row[length] = '\0';
}

static void _row_place(char *row, uint8_t length, uint8_t position, const char *sprite) {
    for (uint8_t i = 0; sprite[i] != '\0'; i++) {
        if (position + i >= length) return;
        row[position + i] = sprite[i];
    }
}

void pet_row_clear(char *row) {
    _row_clear(row, PET_ROW_LENGTH);
}

void pet_row_place(char *row, uint8_t position, const char *sprite) {
    _row_place(row, PET_ROW_LENGTH, position, sprite);
}

void pet_top_clear(char *row) {
    _row_clear(row, PET_TOP_ROW_LENGTH);
}

void pet_top_place(char *row, uint8_t position, const char *sprite) {
    _row_place(row, pet_top_row_length(), position, sprite);
}

void pet_top_draw(const char *row) {
    // The fallback drops the third cell, which is the one the classic LCD hasn't got.
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, row, row);
}

void pet_stat_draw(uint8_t stat) {
    char buf[3];

    snprintf(buf, sizeof(buf), "%2d", stat > TOP_RIGHT_MAX ? TOP_RIGHT_MAX : stat);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
}

static uint8_t _prop_distance(const pet_approach_t *approach) {
    if (approach->prop_position > approach->pet_position) return approach->prop_position - approach->pet_position;

    return approach->pet_position - approach->prop_position;
}

static void _approach_enter(pet_approach_t *approach, pet_approach_phase_t phase) {
    approach->phase = phase;
    approach->phase_ticks = 0;
}

void pet_approach_start(pet_approach_t *approach, uint8_t pet_position) {
    // It heads for whichever end it is already nearest, and the food comes from the other.
    bool nearer_left = pet_position < PET_ROW_LENGTH / 2;

    approach->pet_position = pet_position;
    approach->prop_from_left = !nearer_left;
    approach->prop_position = nearer_left ? PET_ROW_LENGTH - 1 : 0;
    approach->running = true;
    _approach_enter(approach, PET_APPROACH_WALKING);
}

static uint8_t _approach_target(const pet_approach_t *approach) {
    return approach->prop_from_left ? PET_ROW_LENGTH - 1 : 0;
}

bool pet_approach_advance(pet_approach_t *approach) {
    if (!approach->running) return false;

    approach->phase_ticks++;

    switch (approach->phase) {
        case PET_APPROACH_WALKING: {
            uint8_t target = _approach_target(approach);

            if (approach->pet_position != target) {
                approach->pet_position += (approach->pet_position < target) ? 1 : -1;
                // It really did walk over there, so that is where the home face finds it.
                pet_set_position(approach->pet_position);
                break;
            }

            _approach_enter(approach, PET_APPROACH_INCOMING);
            break;
        }
        case PET_APPROACH_INCOMING:
            // The food stops in the cell alongside the pet, which is where it goes down.
            if (_prop_distance(approach) > 1) {
                approach->prop_position += approach->prop_from_left ? 1 : -1;
                break;
            }

            _approach_enter(approach, PET_APPROACH_MOUTH);
            return true;
        case PET_APPROACH_MOUTH:
            if (approach->phase_ticks >= APPROACH_MOUTH_TICKS) _approach_enter(approach, PET_APPROACH_SETTLING);
            break;
        case PET_APPROACH_SETTLING:
            if (approach->phase_ticks >= APPROACH_SETTLE_TICKS) approach->running = false;
            break;
    }

    return false;
}

static const char *_approach_sprite(const pet_approach_t *approach) {
    const pet_species_t *species = _species();

    switch (approach->phase) {
        case PET_APPROACH_WALKING:
            return pet_sprite(pet_mood(), approach->phase_ticks);
        case PET_APPROACH_INCOMING:
            return _prop_distance(approach) <= APPROACH_SWELL_DISTANCE ? species->eating_swell : species->eating_ball;
        case PET_APPROACH_MOUTH:
            return species->eating_mouth;
        default:
            return species->eating_ball;
    }
}

void pet_approach_draw(const pet_approach_t *approach, const char *prop) {
    char row[PET_ROW_LENGTH + 1];

    pet_row_clear(row);
    pet_row_place(row, approach->pet_position, _approach_sprite(approach));

    // The food is only on screen while it is travelling; after that it is eaten.
    if (approach->phase == PET_APPROACH_INCOMING) pet_row_place(row, approach->prop_position, prop);

    watch_display_text(WATCH_POSITION_BOTTOM, row);
}

void pet_feed(uint8_t nutrition) {
    _settle();
    if (_pet.dead) return;

    _pet.hunger = _raise(_pet.hunger, nutrition);
    movement_play_sequence(_tune_eat, BUZZER_PRIORITY_BUTTON);
    _save();
}

/* A performance is a bigger ask than a poke or a pat - it takes a trip to its own
 * face and a whole tune played out - so it earns more happiness than those do.
 */
void pet_sing(void) {
    _settle();
    if (_pet.dead) return;

    _pet.happiness = _raise(_pet.happiness, SING_HAPPINESS_GAIN);
    _save();
}

// Prodding is what shakes an illness off, and it takes more than one go.
static void _poke(void) {
    _pet.happiness = _raise(_pet.happiness, POKE_HAPPINESS_GAIN);

    if (_pet.sick) {
        _pet.pokes_while_sick++;
        if (_pet.pokes_while_sick >= POKES_TO_CURE) _pet.sick = false;
    }

    movement_play_sequence(_tune_poke, BUZZER_PRIORITY_BUTTON);
}

static void _pat(void) {
    _pet.happiness = _raise(_pet.happiness, PAT_HAPPINESS_GAIN);
    movement_play_sequence(_tune_pat, BUZZER_PRIORITY_BUTTON);
}

static void _wave(void) {
    _pet.happiness = _raise(_pet.happiness, WAVE_HAPPINESS_GAIN);
    movement_play_sequence(_tune_wave, BUZZER_PRIORITY_BUTTON);
}

typedef void (*pet_interact_effect_t)(void);

static const pet_interact_effect_t INTERACT_EFFECTS[PET_INTERACT_COUNT] = {
    [PET_INTERACT_POKE] = _poke,
    [PET_INTERACT_PAT] = _pat,
    [PET_INTERACT_WAVE] = _wave,
};

/* Prodding it awake is the only way to feed or play with it after bedtime, and it
 * works, but it holds the grudge for the lost sleep.
 */
void pet_disturb(void) {
    _settle();
    if (_pet.dead || !_pet.asleep) return;

    _pet.happiness = _lower(_pet.happiness, DISTURB_HAPPINESS_COST);
    _pet.awake_until_s = _pet.settled_at_s + (NUDGED_AWAKE_MIN * SECONDS_PER_MINUTE);
    _pet.asleep = false;
    _save();
}

pet_interact_kind_t pet_interact(void) {
    _settle();
    if (_pet.dead) return PET_INTERACT_COUNT;

    // The tap that wakes it is spent on the waking, so the fun starts on the next one.
    if (_pet.asleep) {
        pet_disturb();
        return PET_INTERACT_COUNT;
    }

    pet_interact_kind_t kind = rand() % PET_INTERACT_COUNT;

    INTERACT_EFFECTS[kind]();
    _save();

    return kind;
}

static uint32_t _call_interval_s(void) {
    if (_pet.critical_since_s != 0) return CRITICAL_CALL_INTERVAL_MIN * SECONDS_PER_MINUTE;

    return CALL_INTERVAL_MIN * SECONDS_PER_MINUTE;
}

static pet_call_t _overdue_call(void) {
    if (_pet.dead || _pet.asleep) return PET_CALL_NONE;
    if (_pet.settled_at_s - _called_at_s < _call_interval_s()) return PET_CALL_NONE;

    if (_pet.critical_since_s != 0) return PET_CALL_CRITICAL;
    if (_pet.sick) return PET_CALL_SICK;
    if (_pet.hunger <= HUNGRY_THRESHOLD) return PET_CALL_HUNGRY;

    return PET_CALL_NONE;
}

bool pet_wants_to_call(void) {
    if (!movement_mode_pet_enabled(movement_get_mode())) return false;

    _settle();
    if (_pending_call == PET_CALL_NONE) _pending_call = _overdue_call();

    return _pending_call != PET_CALL_NONE;
}

void pet_call(void) {
    if (_pending_call == PET_CALL_NONE) return;

    movement_play_sequence(CALL_TUNES[_pending_call], BUZZER_PRIORITY_ALARM);
    _called_at_s = _pet.settled_at_s;
    _pending_call = PET_CALL_NONE;
}

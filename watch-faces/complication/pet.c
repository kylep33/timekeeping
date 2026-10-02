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
#include <string.h>
#include "pet.h"
#include "pet_species.h"
#include "filesystem.h"
#include "watch_utility.h"

static const uint32_t SECONDS_PER_MINUTE = 60;
static const uint32_t SECONDS_PER_HOUR = 3600;
static const uint32_t SECONDS_PER_DAY = 86400;
static const uint16_t MINUTES_PER_HOUR = 60;

static const uint32_t HASH_MULTIPLIER = 2654435761u;    // Knuth's golden ratio constant
static const uint8_t HASH_DISCARD_BITS = 16;            // a multiply barely stirs its low bits
static const uint16_t DAY_STRIDE_PER_YEAR = 372;        // 12 * 31, so every date gets its own number
static const uint16_t DAY_STRIDE_PER_MONTH = 31;

static const uint32_t SALT_BEDTIME = 1;
static const uint32_t SALT_WAKE = 2;
static const uint32_t SALT_ILLNESS = 3;

// Off the hour so a call never cuts off the hourly chime. Must stay below every call interval.
static const uint8_t CALL_OFFSET_MIN = 3;

static const uint8_t PET_FORMAT_VERSION = 3;
static char PET_FILE_NAME[] = "pet.dat";

static const pet_species_id_t HATCH_SPECIES = PET_SPECIES_SMILEY;

typedef enum {
    PET_CALL_NONE,
    PET_CALL_HUNGRY,
    PET_CALL_SICK,
    PET_CALL_CRITICAL,
    PET_CALL_SLEEPING,
    PET_CALL_WAKING,
    PET_CALL_DIED,
} pet_call_t;

static pet_t _pet;
static pet_settings_t _settings;
static bool _loaded;
static pet_call_t _pending_call;

static uint8_t _raise(uint8_t stat, uint8_t amount) {
    return (amount > PET_STAT_MAX - stat) ? PET_STAT_MAX : stat + amount;
}

static uint8_t _lower(uint8_t stat, uint8_t amount) {
    return (amount > stat) ? 0 : stat - amount;
}

// Topping a stat off chirps instead of the usual sound, so you know it's had enough.
static void _raise_with_sound(uint8_t *stat, uint8_t amount, int8_t *sound) {
    bool was_full = *stat == PET_STAT_MAX;

    *stat = _raise(*stat, amount);

    bool just_filled = !was_full && *stat == PET_STAT_MAX;
    movement_play_sequence(just_filled ? _settings.sounds.full : sound, BUZZER_PRIORITY_BUTTON);
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

static uint16_t _minute_of_day(watch_date_time_t now) {
    return (now.unit.hour * MINUTES_PER_HOUR) + now.unit.minute;
}

static uint16_t _bedtime_min(watch_date_time_t now) {
    return (_settings.bedtime_earliest_hour * MINUTES_PER_HOUR) + (_day_hash(now, SALT_BEDTIME) % _settings.bedtime_spread_min);
}

static uint16_t _wake_min(watch_date_time_t now) {
    return (_settings.wake_earliest_hour * MINUTES_PER_HOUR) + (_day_hash(now, SALT_WAKE) % _settings.wake_spread_min);
}

static bool _is_bedtime(watch_date_time_t now) {
    uint16_t now_min = _minute_of_day(now);

    return now_min >= _bedtime_min(now) || now_min < _wake_min(now);
}

// Drowsy either side of the night, and the whole time it's been prodded awake through it.
static bool _is_tired(watch_date_time_t now, uint32_t now_s) {
    uint16_t now_min = _minute_of_day(now);
    uint16_t bedtime_min = _bedtime_min(now);
    uint16_t wake_min = _wake_min(now);
    bool winding_down = now_min < bedtime_min && now_min + _settings.tired_before_bed_min >= bedtime_min;
    bool waking_up = now_min >= wake_min && now_min < wake_min + _settings.tired_after_wake_min;

    return now_s < _pet.awake_until_s || winding_down || waking_up;
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

static void _use_species_settings(void) {
    const pet_species_t *species = pet_species_get(_pet.species);

    _settings = pet_default_settings;
    if (species->override_settings != NULL) species->override_settings(&_settings);
}

// A missing or truncated file, an older format or a species this build doesn't have.
static bool _read_saved_pet(void) {
    if (!filesystem_read_file(PET_FILE_NAME, (char *) &_pet, sizeof(_pet))) return false;

    return _pet.version == PET_FORMAT_VERSION && _pet.species < PET_SPECIES_COUNT;
}

// A pet that can't be read back safely is replaced rather than trusted.
static void _load(void) {
    if (_loaded) return;
    _loaded = true;

    if (_read_saved_pet()) {
        _use_species_settings();
        return;
    }

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
    _pet.hunger = _apply_drift(_pet.hunger, &_pet.hunger_debt_s, _settings.hunger_drain_s_per_point, false);

    _pet.happiness_debt_s += elapsed_s;
    _pet.happiness = _apply_drift(_pet.happiness, &_pet.happiness_debt_s, _settings.happiness_drain_s_per_point, false);
}

// Health only slides while something else is wrong, so a cared for pet climbs back to full.
static void _settle_health(uint32_t elapsed_s) {
    bool suffering = _pet.sick || _pet.hunger <= _settings.neglect_threshold || _pet.happiness <= _settings.neglect_threshold;

    _pet.health_debt_s += elapsed_s;
    if (suffering) {
        _pet.health = _apply_drift(_pet.health, &_pet.health_debt_s, _settings.health_drain_s_per_point, false);
        return;
    }

    _pet.health = _apply_drift(_pet.health, &_pet.health_debt_s, _settings.health_recovery_s_per_point, true);
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

    if (_pet.health <= _settings.illness_health_threshold) {
        _fall_ill();
        return;
    }

    if (_day_hash(now, SALT_ILLNESS) % _settings.illness_odds == 0) _fall_ill();
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

    if (now_s - _pet.critical_since_s < _settings.critical_grace_hours * SECONDS_PER_HOUR) return;

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
    _pet.species = HATCH_SPECIES;
    _use_species_settings();

    _pet.generation = generation + 1;
    _pet.best_age_days = best_age_days;
    _pet.hatched_at_s = now_s;
    _pet.settled_at_s = now_s;
    _pet.hunger = _settings.hatch_need;
    _pet.happiness = _settings.hatch_need;
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
    if (pet->hunger <= _settings.hungry_threshold) return PET_MOOD_HUNGRY;
    if (_is_tired(movement_get_local_date_time(), pet->settled_at_s)) return PET_MOOD_TIRED;
    if (pet->happiness <= _settings.sad_threshold) return PET_MOOD_SAD;

    return PET_MOOD_HAPPY;
}

uint16_t pet_age_days(void) {
    return _age_days(pet_get()->settled_at_s);
}

uint8_t pet_species_id(void) {
    _load();

    return _pet.species;
}

void pet_feed(uint8_t nutrition) {
    _settle();
    if (_pet.dead) return;

    _raise_with_sound(&_pet.hunger, nutrition, _settings.sounds.eat);
    _save();
}

void pet_sing(void) {
    _settle();
    if (_pet.dead) return;

    _pet.happiness = _raise(_pet.happiness, _settings.sing_happiness_gain);
    _save();
}

void pet_murder(void) {
    _settle();
    if (_pet.dead) return;

    _pet.dead = true;
    // Anything it was about to call out would play over its own scream.
    _pending_call = PET_CALL_NONE;
    movement_play_sequence(_settings.sounds.murdered, BUZZER_PRIORITY_BUTTON);
    _save();
}

// Prodding is what shakes an illness off, and it takes more than one go.
static void _poke(void) {
    if (_pet.sick) {
        _pet.pokes_while_sick++;
        if (_pet.pokes_while_sick >= _settings.pokes_to_cure) _pet.sick = false;
    }

    _raise_with_sound(&_pet.happiness, _settings.poke_happiness_gain, _settings.sounds.poke);
}

static void _pat(void) {
    _raise_with_sound(&_pet.happiness, _settings.pat_happiness_gain, _settings.sounds.pat);
}

static void _wave(void) {
    _raise_with_sound(&_pet.happiness, _settings.wave_happiness_gain, _settings.sounds.wave);
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

    _pet.happiness = _lower(_pet.happiness, _settings.disturb_happiness_cost);
    _pet.awake_until_s = _pet.settled_at_s + (_settings.nudged_awake_min * SECONDS_PER_MINUTE);
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

static uint32_t _call_interval_min(void) {
    if (_pet.critical_since_s != 0) return _settings.critical_call_interval_min;

    return _settings.call_interval_min;
}

static bool _is_call_minute(watch_date_time_t now) {
    return now.unit.minute % _call_interval_min() == CALL_OFFSET_MIN;
}

static pet_call_t _overdue_call(void) {
    if (_pet.dead || _pet.asleep) return PET_CALL_NONE;

    if (_pet.critical_since_s != 0) return PET_CALL_CRITICAL;
    if (_pet.sick) return PET_CALL_SICK;
    if (_pet.hunger <= _settings.hungry_threshold) return PET_CALL_HUNGRY;

    return PET_CALL_NONE;
}

static int8_t *_call_tune(pet_call_t call) {
    const pet_sounds_t *sounds = &_settings.sounds;

    switch (call) {
        case PET_CALL_HUNGRY:
            return sounds->hungry;
        case PET_CALL_SICK:
            return sounds->sick;
        case PET_CALL_CRITICAL:
            return sounds->critical;
        case PET_CALL_SLEEPING:
            return sounds->sleeping;
        case PET_CALL_WAKING:
            return sounds->waking;
        case PET_CALL_DIED:
            return sounds->died;
        default:
            return NULL;
    }
}

bool pet_wants_to_call(void) {
    if (!movement_mode_pet_enabled(movement_get_mode())) return false;

    _settle();
    if (!_is_call_minute(movement_get_local_date_time())) return false;
    if (_pending_call == PET_CALL_NONE) _pending_call = _overdue_call();

    return _pending_call != PET_CALL_NONE;
}

void pet_call(void) {
    if (_pending_call == PET_CALL_NONE) return;

    movement_play_sequence(_call_tune(_pending_call), BUZZER_PRIORITY_ALARM);
    _pending_call = PET_CALL_NONE;
}

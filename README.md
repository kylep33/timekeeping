# timekeeping

this is my repo for my custom sensorboard watch.

Hardware: **Sensor Watch Pro** board + **custom LCD** (Oddly Specific Objects) +
LIS2DW accelerometer, in a **Casio F-84W** case (module 593, same as the F-91W
Sensor Watch targets).

This repo is a fork of [Second Movement](https://github.com/joeycastillo/Second-Movement),
the community firmware for Sensor Watch. Upstream's own README follows below.

## My build commands

Every build needs `BOARD` and `DISPLAY`. For this watch they are always:

```sh
# simulator (opens in a browser, no hardware needed)
emmake make BOARD=sensorwatch_pro DISPLAY=custom
python3 -m http.server -d build-sim
# -> http://localhost:8000/firmware.html

# real firmware -> build/firmware.uf2
make BOARD=sensorwatch_pro DISPLAY=custom

# flash: double-tap Reset on the back, wait for WATCHBOOT, then
make install
```

`make clean` between switching sim and hardware builds if things act strange.

## Staying current with upstream

```sh
git fetch upstream
git merge upstream/main
```

## Where my stuff goes

- `movement_config.h` — which faces are compiled in, and their order
- `movement_faces.h` — `#include` for each face
- `watch-faces.mk` — build list
- `watch-faces/` — face source; mine live alongside upstream's

## Modes

Long-press MODE on a mode's first face to open the mode picker; short-press MODE
cycles the name, long-press MODE commits (also commits on its own after a few
seconds). Each mode has its own face rotation and hourly chime tune.

| mode | faces (MODE cycles in this order) | chime | pet |
|---|---|---|---|
| DAILY | ish, timer, stopwatch, coin flip | Kim Possible | yes |
| CLIMB | clock, climb timer | Zelda secret | no |
| GAME | clock, probability, pulsometer, endless runner, ping, tarot, simon | Mario theme | no |
| PET | pet, pet food, pet play, endless runner, ping | Power Rangers | yes |
| EARTH | clock, sunrise/sunset, moon phase, tide | Evangelion | yes |
| SETUP | clock, set time, advanced alarm, finetune, nanosec, settings, voltage | MGS codec | no |

The last column is `pet_enabled` in `movement_config.h`: whether the pet may
interrupt that mode to shout for food, complain about being ill, or announce that
it is going to sleep. It lives on whether or not its faces are in the rotation.

Face notes:
- **ish** — fuzzy time; hold ALARM to peek exact time (forced 24h), tap ALARM to
  change how fuzzy.
- **climb timer** — hold ALARM to start the approach (segment 0), tap ALARM to
  bank a pitch and start the next, hold ALARM to stop and see the summary, tap
  to step through it, hold to clear. Up to 25 segments.
- **sunrise/sunset** — also holds the location presets (San Diego, Yosemite).
- **pet** — the creature wanders the bottom row, sleeps between 8-10pm and 7-9:30am
  (the exact times are hashed from the date, so they shift day to day), and gets
  hungry, ill and eventually dies if ignored. Top right is its age in days. Tap
  ALARM to nudge it, hold ALARM to peek the time, hold LIGHT to page through
  hunger, mood, health and age. Health hitting zero starts a 12 hour countdown
  rather than ending it; after that the face shows RIP and its final age, and
  holding ALARM hatches the next generation. State lives in `pet.dat`.
- **pet food** — tap ALARM to pick snack, apple or cake, hold ALARM to feed it.
  Top right shows how hungry it currently is.
- **pet play** — tap ALARM to pick, hold ALARM to go. Poke prods it with a stick,
  and three pokes cure an illness; the rest launch the games listed in
  `pet_games` in `movement_config.h`.

---

# Second Movement

This is the successor refactor of the Movement firmware for [Sensor Watch](https://www.sensorwatch.net).


Getting dependencies
-------------------------
You will need to install [the GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads/) to build projects for the watch. If you're using Debian or Ubuntu, it should be sufficient to `apt install gcc-arm-none-eabi`.

You will need to fetch the git submodules for this repository too, with `git submodule update --init --recursive`


Building Second Movement
----------------------------
You can build the default watch firmware with:

```
make BOARD=board_type DISPLAY=display_type
```

where `board_type` is any of:
- sensorwatch_pro
- sensorwatch_green
- sensorwatch_red (also known as Sensor Watch Lite)
- sensorwatch_blue

and `display_type` is any of:
- classic
- custom

Optionally you can set the watch time when building the firmware using `TIMESET=minute`.

`TIMESET` can be defined as:
- `year` = Sets the year to the PC's
- `day` = Sets the default time down to the day (year, month, day)
- `minute` = Sets the default time down to the minute (year, month, day, hour, minute)


If you'd like to modify which faces are built and included in the firmware, edit `movement_config.h`. You will get a compilation error if you enable more faces than the watch can store.

Installing firmware to the watch
----------------------------
To install the firmware onto your Sensor Watch board, plug the watch into your USB port and double tap the tiny Reset button on the back of the board. You should see the LED light up red and begin pulsing. (If it does not, make sure you didn't plug the board in upside down). Once you see the `WATCHBOOT` drive appear on your desktop, type `make install`. This will convert your compiled program to a UF2 file, and copy it over to the watch.

If you want to do this step manually, copy `/build/firmware.uf2` to your watch.


Emulating the firmware
----------------------------
You may want to test out changes in the emulator first. To do this, you'll need to install [emscripten](https://emscripten.org/), then run:

```
emmake make BOARD=sensorwatch_red DISPLAY=classic
python3 -m http.server -d build-sim
```

Finally, visit [firmware.html](http://localhost:8000/firmware.html) to see your work.

# timekeeping

My custom Sensor Watch firmware, forked from
[Second Movement](https://github.com/joeycastillo/Second-Movement).

**Hardware:** Sensor Watch Pro board + custom LCD (Oddly Specific Objects) +
LIS2DW accelerometer, in a Casio F-84W case (module 593, same as the F-91W
Sensor Watch targets).

## Build

Needs the [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads/)
(`apt install gcc-arm-none-eabi` on Debian/Ubuntu) and the git submodules
(`git submodule update --init --recursive`).

Always use `BOARD=sensorwatch_pro DISPLAY=custom`:

```sh
make BOARD=sensorwatch_pro DISPLAY=custom
# -> build/firmware.uf2
```

Run `make clean` if you switch between a hardware and simulator build and
something looks wrong. Note that each build has its own output directory, and
`clean` only removes the one it is invoked for: bare `make clean` clears
`build/`, and `emmake make clean` clears `build-sim/`.

## Simulate

Needs [emscripten](https://emscripten.org/).

```sh
emmake make BOARD=sensorwatch_pro DISPLAY=custom
python3 -m http.server -d build-sim
# -> http://localhost:8000/firmware.html
```

Browsers cache `firmware.wasm` hard, and `http.server` sends nothing to stop
them, so a plain reload can keep running the previous build long after a
rebuild. Hard-reload (Ctrl/Cmd+Shift+R) after rebuilding, or leave DevTools
open with "Disable cache" ticked.

## Flash


kyle steps for this:
1. plug into mattbot
2. sudo picocom -b 115200 /dev/ttyACM0
3. flash
3.5 get the build firwmare.uf2 file on mattbot at ~/
3.6:  lsblk -o NAME,SIZE,FSTYPE,LABEL,MOUNTPOINTS should show mattbot under a sdb            7.8M vfat     WATCHBOOT /media/kile/WATCHBOOT
4. sudo cp ~/firmware.uf2 /media/kile/WATCHBOOT/


1. Double-tap the Reset button on the back of the board.
2. Wait for the `WATCHBOOT` drive to show up.
3. `make install`

(Or copy `build/firmware.uf2` to the `WATCHBOOT` drive yourself.)

## Modes

Short-press MODE to cycle faces within the current mode. Long-press MODE from
a mode's resting face to open the mode picker — short-press to cycle mode
names, long-press (or wait) to commit.

DAILY · CLIMB · GAME · PET · EARTH · SETUP — see `movement_config.h` for what
each one contains.

## Where my stuff goes

- `movement_config.h` — which faces are compiled in, and their order
- `movement_faces.h` — `#include` for each face
- `watch-faces.mk` — build list
- `watch-faces/` — face source; mine live alongside upstream's

## Staying current with upstream

```sh
git fetch upstream
git merge upstream/main
```

# mGBA boot smoke

This boots the playable Wayfarer debug ROM in headless libmgba and checks that
its fresh-save main menu accepts input. It covers the ROM handed to testers,
including flash detection, without enabling E2E or mechanics-test hooks.

On Ubuntu 24.04, install `libmgba-dev` alongside the normal ROM build dependencies
(C compiler, Python 3, and `arm-none-eabi-nm` are also required). From the repository
root:

```sh
make -C game -j"$(nproc)" BUILD=wayfarer DEBUG=1 debug
python3 game/tools/mgba-boot-smoke/run.py game/pokewayfarer.gba game/pokewayfarer.elf
```

Always supply the unstripped ELF from the same build as the ROM. The wrapper uses
its symbols to resolve `gMain`, `gTasks`, `CB2_MainMenu`,
`Task_HandleMainMenuInput`, and `gFlashMemoryPresent`. The runner reads the GBA
struct layouts documented in `include/main.h` and `include/task.h`; update its
layout constants if those structs change.

Every run copies the ROM into a new temporary directory, isolates the XDG config/data
paths, and starts without a save file or user configuration. It never forces a
save type or writes flash commands. Existing saves next to the input ROM are not
used or modified. The runner enables the RTC peripheral for Wayfarer's `BWFE`
cartridge code, which is absent from mGBA's database; flash remains autodetected.
Start pulses skip the intro and title; the runner never presses
A, which could dismiss save/battery error windows.

Success requires `CB2_MainMenu` and an active `Task_HandleMainMenuInput` task with
`HAS_NO_SAVED_GAME` for 30 consecutive frames. The callback alone would also match
error windows. Failure occurs after 3,600 frames (about one emulated minute), or
a 120-second wall-clock timeout. Override these limits with `--frames` and
`--timeout`.

`mgba-smoke-results/` contains the boot log, resolved symbols, and final
`screen.png` (unless the emulator hangs or crashes before capture). Choose another
location with `--output`. The Smoke workflow runs the mGBA job only on PRs ready for review and uploads diagnostics.
This is a boot check; it does not exercise gameplay or verify visual rendering.

For a non-system libmgba installation, the wrapper accepts `CC`, `CFLAGS`,
`LDFLAGS`, and `NM`; configure the runtime library path as needed. Include
`mgba/flags.h` before other mGBA headers, because its feature flags affect the ABI.

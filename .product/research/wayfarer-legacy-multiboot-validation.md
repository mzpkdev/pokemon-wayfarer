# Legacy multiboot removal validation

Implementation commit: `ac16061af0` (base `7aa8db0557da310fdacdf716a9fba95db76d2536`).
The missing ROM-report fixture was restored separately in `da026cd7d4`.
The measurement pair uses identical implementation sources and GCC 13.2.1,
`-O2`, release LTO, Wayfarer product constants, map catalog, and linker script.
The baseline is a disposable build copy, not a standalone HNS ROM or the
historical accepted baseline. All tracked game files matched byte-for-byte
between the pair except the Makefile's measurement-only capability policy.
That override is not available in the shipped Makefile.

| Release measurement | Payloads enabled | Wayfarer |
| --- | ---: | ---: |
| `__rom_end` | `0x09F5D9A0` | `0x09F2A70C` |
| Used ROM bytes | 32,889,248 | 32,679,692 |
| Total savings | | **209,556** |
| Required payload savings | | 191,700 |
| Additional savings | | 17,856 |

The release ceiling is `0x09F80000`; Wayfarer has 350,452 bytes of headroom.
The three baseline spans are exactly `0x28000`, `0x3BF4`, and `0x30E0`.
Both normal and release Wayfarer omit all six payload symbols and
`GameCubeMultiBoot_ExecuteProgram`, `CB2_InitBerryFixProgram`, and
`CB2_InitEReader`. The payload files remain in the repository.

Category savings are 8,680 bytes of code, 6,424 bytes of graphics, and
194,452 bytes in `other` (including the 191,700 payload bytes). Audio,
scripts, maps/layouts, trainer data, and encounter data are unchanged.
[Paired measurements](legacy-multiboot-validation/paired-release.json) and
[normal/release symbol checks](legacy-multiboot-validation/wayfarer-symbols.json)
record the exact values. The two full size reports retain their ordinary
historical-baseline deltas; the paired JSON subtracts their absolute category
sizes to isolate this change.

Reproduction:

1. Build `make -C game -j12 wayfarer` and
   `make -C game -j12 BUILD=wayfarer release`.
2. Copy the game directory into a disposable build directory. In that copy
   only, change the Wayfarer `LEGACY_MULTIBOOT_DEFAULT` from `0` to `1` and
   remove the `Wayfarer requires ...=0` rejection expression. Change nothing
   else in the product configuration or sources.
3. Run `make -C <copy>/game -j12 BUILD=wayfarer release`. The capability stamp
   rebuilds existing objects with the enabled settings.
4. Compare `__rom_end` using `arm-none-eabi-nm --defined-only` and subtract
   absolute `rom.used_bytes` and category bytes from the generated size JSON.
   Do not compare padded `.gba` lengths.

Normal and release Wayfarer builds pass. A headless mGBA 0.10.2 run of the
normal playable ROM reaches an input-ready fresh-save main menu at frame 531.
The focused seven-test host suite and all 21 ROM-report tests pass. The host
suite executes extracted production menu/record-validation code, checks all
capability combinations and invalid settings, and verifies Trainer Hill and
Berry fallbacks. It is included in `make BUILD=wayfarer check`.

The E2E-enabled Wayfarer build, TypeScript typecheck, and complete smoke suite
pass (three files, seven tests). The smoke suite includes New Game reaching
the New Bark bedroom. Four new SkyEmu startup tests cover: natural startup, inactive `B+Select`,
ordinary `SerialCB`, the fresh-save RTC chord, Options return, and clear-save
prompt cancellation. SkyEmu needs writable isolated XDG directories in this
sandbox; an initial default-environment launch crashed before HTTP startup.
Successful runs use `LIBGL_ALWAYS_SOFTWARE=1`,
`MESA_LOADER_DRIVER_OVERRIDE=llvmpipe`, and `GALLIUM_DRIVER=llvmpipe`.

All eight standalone build-and-symbol checks pass: Emerald, FireRed,
LeafGreen, and HNS with default capability values, followed by Emerald with
each capability disabled independently and with all three disabled. Each
single-off case omits only its own payload and entry point; the other payload
spans remain exact. [Commands and symbol results](legacy-multiboot-validation/build-matrix.json)
include each configuration. These builds ran sequentially in the disposable
copy, with its ordinary Makefile restored, so generated map files were never
shared with the task's concurrent Wayfarer checks.

`make -C game -j12 BUILD=wayfarer check` exits successfully. Its static
prerequisites pass, and its 5,318 mechanics cases report 4,325 passed,
349 known failures, 629 TODOs, 9 failed assumptions, and 6 expected failures.
There are no unexpected failures. The [runner summary](legacy-multiboot-validation/check-summary.txt)
lists the assumptions that did not hold; these are not counted as passes.

The isolated Elm visitor-storage journey passes (one test, 25.5 seconds).
It checks party/PC capacity and committed gift choices, saves twice, resets
through the ordinary boot path, selects Continue, and verifies preserved
state. This is ordinary save/reload evidence, not transferred Trainer Hill
or e-Reader visiting-trainer coverage.

An optional broad parallel journey run reported failures/timeouts and then
stalled; it was terminated after more than ten minutes. The visitor-storage
failure did not reproduce in isolation. Both native-opening retries pass in isolation (two tests, 65.07 seconds):
Hoenn completes the truck/Mom/clock/rival/rescue/Birch flow and Johto completes
clock/Mom/Elm. The complete broad journey suite is not claimed green.

Hardware runtime gates remain unfulfilled: ordinary cable trade/battle,
Wireless Adapter trade/battle, GameCube link activity, and wireless Mystery
Gift receive/send. The available SkyEmu harness has no link-cable or RFU
connection API, and no physical hardware is attached. Shared cable/link,
RFU, trade, Union Room, and Mystery Gift client/server source files are
unchanged; that audit is not a hardware smoke result.

Prepared-save runtime gates also remain unfulfilled: valid/invalid visiting
trainers, a populated Trainer Hill special sector, enabled RTC reset, and
unrelated-state preservation across those save/reload cases. Host checks
cover empty, corrupt, valid, and removed-field records and built-in Trainer
Hill selection, but do not replace those emulator checks.

# Surf pixel-sheet deduplication validation

The implementation saves exactly 430,080 used ROM bytes in paired Wayfarer
production release builds. The 61 approved aliases share linked addresses;
the other 78 pairs remain separate. Runtime acceptance is recorded below.

## Build inputs and artifacts

Both builds start from `7aa8db0557da310fdacdf716a9fba95db76d2536`, with only
the implementation in `5d49b18b29` applied to the optimized snapshot. They use GCC
`arm-none-eabi-gcc (15:13.2.rel1-2) 13.2.1 20231009`, the repository's
`config.mk` (`USE_LTO_ON_RELEASE=1`), the unchanged accepted Wayfarer baseline,
and the same command from each `game/` directory:

```sh
make -j"$(nproc)" BUILD=wayfarer release syms
```

The builds use clean `git archive` source snapshots inside the owned task at
`build/surf-pixel-deduplication/{baseline,optimized}/game/`. They are not
separate registered Git worktrees as requested by the spec: task ownership
precludes creating or modifying another task. No prebuilt objects or generated
assets were copied into either clean build. No different map versions were
built together. Tracked source comparisons find only the Makefile and Surf
pixel-definition header changed; the optimized snapshot additionally contains
the alias manifest and validation tools.

Each directory preserves `pokewayfarer-release.gba`, `.elf`, `.map`, `.sym`,
and `pokewayfarer-release-size.json`. These large local artifacts are ignored
by Git. [Artifact hashes](artifact-sha256.json), both size reports, the full
139-pair [regenerated inventory](inventory.json), and the
[comparison result](comparison.json) are committed here.

## Release measurements

| Metric | Baseline | Optimized | Reduction |
|---|---:|---:|---:|
| Used ROM | 32,888,944 | 32,458,864 | 430,080 |
| `__rom_end` | `0x09F5D870` | `0x09EF4870` | `0x69000` |
| Free ROM | 665,488 | 1,095,568 | increases 430,080 |

Only the `other` report category changes, by exactly 430,080 bytes. All seven
other categories, including `graphics`, remain the same. Both reports enforce
the Wayfarer release limit `0x09F80000`. Padded `.gba` file length is not a
measurement input.

Regeneration found exactly the approved 61 byte-identical pairs among 139
active pairs: 58 sheets of 6,144 bytes, plus Lugia, Rayquaza, and Arceus at
24,576 bytes. The inventory excludes commented Rhyhorn definitions.

Linked validation checks all 278 pixel symbols, including binding, size,
approved address equality, non-approved address separation, and non-overlap
between species. Production LTO internalizes these symbols to strong local
(`l`) bindings. The validator accepts strong global or local symbols and
rejects weak symbols. The production linker map groups these bytes into
anonymous LTO ranges, so the named allocation check uses ELF-derived `.sym`
addresses and sizes; map/report checks establish `__rom_end` and category
sizes. No LTO or release flags were changed to make the measurement pass.

The paired checker also proves all 565 linked pixel/palette symbols retain
their bytes, all 287 palettes retain separate allocations, and all 550 linked
frame tables retain frame counts, pixel offsets, and sizes. The roster,
templates, palette declarations, conversion rules, PNGs, palette assets, and
runtime selection/movement code have no source changes.

## Automated checks

```sh
make -C game -j8 BUILD=wayfarer surfable-pokemon-pixel-alias-test
python3 game/tools/surfable/tests/test_surfable_pokemon_pic_aliases.py \
  --symbols build/surf-pixel-deduplication/optimized/game/pokewayfarer-release.sym
python3 game/tools/surfable/compare_release_builds.py \
  build/surf-pixel-deduplication/baseline/game \
  build/surf-pixel-deduplication/optimized/game
```

The focused source check and seven regression test methods pass. Negative
cases cover pixel drift, missing output, duplicate rows, a second shiny
INCBIN, unsized aliases, missing/weak symbols, wrong alias addresses,
non-approved sharing, and cross-species overlap. All 122 approved conversion
outputs regenerate from PNG on a clean focused run.

A deliberate one-byte change to generated Squirtle shiny pixels in the
optimized snapshot made the release ELF target fail before linking, naming
both paths. The bytes were restored and validation rerun successfully. Make
prerequisite inspection confirms the same gate on normal, release, E2E, and
mechanics ELF paths. Symbol generation now runs linked validation automatically.
The paired checker rejects a one-byte savings discrepancy and an unrelated
four-byte `graphics` category change.

Both baseline and optimized E2E ROMs build with the same revision and
`BUILD=wayfarer E2E=1`. The optimized E2E symbols pass the alias validator.
E2E sizes are not used for the release savings claim.

## Runtime acceptance

The driver is `e2e/src/journeys/wayfarer-surf-pixel-aliases.e2e.ts`. It creates
an isolated emulator per case and selects Squirtle, Alolan Raichu, Wartortle,
Hisuian Qwilfish, Lugia, and Gyarados, each normal and shiny. A test-only party
memory fixture uses PID 24 for normal and PID 0 for shiny, preserving encrypted
substructure ordering and checking the resulting shiny value. No ROM runtime
or E2E mailbox implementation changes are needed.

The test enters Surf from land, verifies the selected party species, moves in
four directions, samples bobbing, uses the rod through the Bag, exits fishing,
dismounts and remounts, then positions the mounted player at the Route 40 seam
and crosses the real Route 40/41 connection with ordinary movement input. The
fixture warp positions the seam test; the transition itself is not a warp.
The normal/shiny screenshots differ for all six species.

Use separate evidence directories for the two ROMs. From `e2e/`, with absolute
paths for each matching ROM/symbol pair and evidence directory:

```sh
env -u WAYLAND_DISPLAY LIBGL_ALWAYS_SOFTWARE=1 \
  SKYEMU_ROM=<game/pokemon-wayfarer-e2e.gba> \
  SKYEMU_SYMS=<game/pokemon-wayfarer-e2e.sym> \
  SURF_ALIAS_EVIDENCE=<build/surf-pixel-deduplication/runtime-final-optimized> \
  pnpm exec wa test src/journeys/wayfarer-surf-pixel-aliases.e2e.ts
```

Repeat with baseline ROM/symbol paths and `runtime-final-baseline`. Fishing
and transitions run by default. `SURF_ALIAS_CASE=gyarados-normal` selects one
valid case; invalid names fail. Setting `SURF_ALIAS_FISHING=0` or
`SURF_ALIAS_TRANSITION=0` explicitly skips those steps and records the missing
gates in the case JSON. Such a run cannot satisfy the complete capture checker.

From the repository root, compare the two final runs with:

```sh
python3 game/tools/surfable/compare_runtime_captures.py build/surf-pixel-deduplication
```

Local Xvfb/SkyEmu needed command-specific sandbox escalation after startup
segfaults; software GL alone did not fix them. Initial driver attempts also
found an oversized emulator memory request, encounter contamination between
cases, and a hooked Gyarados fishing encounter. The driver uses small reads,
isolated emulators, deterministic RNG, and ordinary Run-menu recovery. The
[run history](runtime-run-history.json) records those failed attempts and
reruns rather than hiding them.

The finalized full-default suite passes **12/12 on each ROM** (93.10 seconds
optimized, 93.40 seconds baseline). All **180 required screenshot pairs are
byte-identical**, with no missing or unexpected captures. The
[runtime comparison](runtime-comparison.json) records each hash. The checker
also rejects a deliberately missing required capture. Typecheck, lint, and
format checks pass; an invalid case selector fails before launching an
emulator.

Visual inspection confirms the six species/form silhouettes, distinct normal
and shiny colors, the 4x4 and 8x8 sizes, rider layering, fishing pose, transition
presentation, and clear dismounts. Samples at 0, 8, and 16 frames show bobbing
and animation on both sheet sizes. The paired images retain baseline frame
presentation at all captured stages. No representative case shows the fallback
blob or an overlay remaining after dismount. Static linked-byte and frame-table
checks complement these sampled visual checks; this is representative coverage,
not an exhaustive runtime test of all 139 mounts or every option combination.

The procedural deviation from the spec is using isolated clean source snapshots
instead of additional registered Git worktrees. All required representative
runtime stages and exact-size gates pass. Large ROM and screenshot artifacts
remain local; their recorded hashes and validation results are committed. Final
raw test logs are `build/surf-pixel-deduplication/runtime-final-optimized.log`
and `runtime-final-baseline.log`. Earlier failed attempts remain in the run
history and local diagnostic artifacts.

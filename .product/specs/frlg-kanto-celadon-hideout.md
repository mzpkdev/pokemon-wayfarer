# Celadon Rocket Hideout

This adventure restores the five existing FRLG Hideout interiors in Wayfarer:
B1F–B4F and the elevator. It is independent of badges, other regions, Pokémon
Tower, Silph Co., Mt. Moon, and rival scenes. Tower/Fuji/Flute, Silph, Bill,
Giovanni's Viridian finale, and casino economy changes are outside its scope.

## Entrance and maps

Preserve the HNS Celadon Game Corner, its NPCs, slots, roulette, prizes, and
services. Add a Wayfarer-only Rocket at `(10, 2)` and concealed wall panel at
`(11, 1)`, approached from `(11, 2)`. Defeating this Rocket permits the player
to discover the panel and enter B1F at `(12, 2)`. Declining leaves the player
in the casino. The entrance remains available after discovery.

Use existing layouts without changing any `map.bin`. Select the five FRLG maps
and layouts explicitly for Wayfarer, retaining group 66, slots 42–46. The B1F
stair exit returns to the Game Corner approach tile `(11, 2)`. Preserve the
source interior warps, arrows, barriers, and elevator destinations B1F/B2F/B4F.
No FRLG staircase animation or replacement Game Corner is required.

## Local progression

| Earned cause | Result |
| --- | --- |
| Entrance Grunt 7 defeated, panel accepted | Entrance opens |
| B1F Grunt 12 defeated | B1F barrier opens |
| B4F Grunt 18 defeated | Lift Key appears |
| Lift Key successfully received | Elevator becomes usable |
| Both B4F Grunts 16 and 17 defeated | Paired-guard barrier opens |
| Giovanni defeated | Silph Scope appears |
| Silph Scope successfully received | Local reward is claimed |

Barrier and reward visibility must reconstruct from saved local state on map
load. Losses, leaving, and save/reload preserve earned partial victories without
awarding unearned progress. Giovanni and his reward check local security
predicates even when a fixture or collision bypass reaches the room early.
Optional trainers and ordinary items remain available after Giovanni's defeat.

Key and Scope grants check prerequisites before attempting the handoff. They
record receipt and hide the object only after successful delivery. A full bag
leaves the reward available for retry, including after reload. Existing item
ownership can prevent a duplicate after the local prerequisite is satisfied;
it cannot substitute for defeating a required trainer. Object visibility and
item receipt use separate saved flags.

## Battles and rewards

Retain all 13 FRLG parties: entrance Grunt 7, interior Grunts 8–18, and Giovanni.
The 12 grunts use current ordinary trainer scaling. Giovanni uses the authored
boss policy. Allocate independent trainer defeat state through the current
Wayfarer trainer framework.

Deliberate battle interactions use the canonical usable-party check. Empty,
fainted, and Eggs-only parties receive repeatable refusal without battle or
progress. Defeated actors remain available for their non-battle dialogue.
Ordinary sight encounters follow current trainer detection and usable-party
suppression. Only victories may open barriers or reveal rewards; preserve
current Wayfarer loss and recovery conventions.

Dialogue works when this is the player's first Rocket investigation. Completing
it awards no badge, unrelated rescue, rival progress, or Giovanni finale state.

| Floor | Visible rewards | Hidden rewards |
| --- | --- | --- |
| B1F | Escape Rope, Hyper Potion | PP Up |
| B2F | X Speed, Moon Stone, TM12, Super Potion | None |
| B3F | Rare Candy, TM21, Black Glasses | Nugget |
| B4F | Silph Scope, Lift Key, TM49, Max Ether, Calcium | Nest Ball, Net Ball |

## Integration and acceptance

Audit current trainer, flag, and variable namespaces before allocating state.
The elevator uses dedicated local state rather than FRLG's generic elevator
variable, which aliases active HNS Faraway Island state. Verify elevator travel,
cancellation, exit/re-entry, save/reload, and a Faraway Island counter sentinel
together. Keep Celadon's world-map section separate from Mahogany's Hideout.

Include the required FRLG Rocket/Giovanni sprites, BuildingFrlg/SilphCo tilesets,
and door dependencies. Preserve standalone HNS and FRLG selection behavior.

Validation covers map/event selection, item and trainer inventories, scaling,
namespace isolation, entry/exit, elevators, arrow and barrier puzzles, refusals,
actual victories, loss/retry, bag-capacity retry, persistent progress, and casino
services. Run current static checks, relevant native tests and emulator journeys,
production build/budget checks, and independent code review. Confirm no
`map.bin` changes in the final diff.

## Saved-state allocation

The port appends runtime trainer IDs `1723–1735`. Local item, visibility,
entrance, elevator, boss, and world-map state uses flags `0x4B6–0x4CD`;
independent trainer victories use `0x4CE–0x4DA`. The elevator floor variable is
`0x40D9`. Source FRLG names alias these values only for Wayfarer.

## Validation

Validated on the task worktree based on `29fbcee2c8` (current `main` when the
port started). No map binaries changed. The five interior maps retain their
original object events, background events, and warp geometry. Removing the two
Wayfarer-only additions leaves the Game Corner map JSON identical to baseline.

The E2E ROM and production release build pass. The release uses 31,598,348 ROM
bytes, leaving 1,956,084 bytes free; the normal 512 KiB reserve check passes.
EWRAM is 249,417 bytes and IWRAM is 25,576 bytes. Required tileset, sprite, and
elevator-animation symbols are present in the linked E2E ROM.

All 18 Hideout emulator cases pass, covering entrance and no-party refusals,
real trainer wins, loss/retry, guard and reward persistence, full-pocket retries,
arrow tiles, both barriers, elevator round trips/cancellation/reload, the Faraway
Island sentinel, region-map placement, and preserved casino interactions. All
13 current S.S. Anne regression cases pass. Native Wayfarer tests pass 119/119,
including trainer-flag isolation, the usable-party gate, and Giovanni graphics.

The map catalog suite passes 29 tests. Trainer roster generation and scaling
checks pass with no structural failures. Sevii content and port audits pass;
their frozen-source checks retain exact insertion allowances for this port.
Standalone HNS/FRLG conditional preprocessing of the changed door, region, and
graphics-pointer code is unchanged. These are preprocessing checks, not full
standalone ROM builds.

Independent source review is clear after correcting the donor-only party
helper, guarded sight handling, and catalog warp-override handling. The
repository-wide E2E style check still reports preexisting issues in unrelated
files; type checking and scoped formatting/lint pass for this port.


Reproduce the principal checks from the repository root:

```sh
make -C game -j8 CXX=g++ e2e
SKYEMU_ROM="$PWD/game/pokemon-wayfarer-e2e.gba" \
SKYEMU_SYMS="$PWD/game/pokemon-wayfarer-e2e.sym" \
pnpm --dir e2e exec wa test src/journeys/wayfarer-celadon-hideout.e2e.ts src/journeys/wayfarer-ss-anne-adventure.e2e.ts
make -C game -j8 CXX=g++ BUILD=wayfarer check TESTS=Wayfarer
make -C game -j8 CXX=g++ BUILD=wayfarer release
```

Use the repository-supported Node 24 environment. Build configurations run
serially because they share generated map files. The `CXX=g++` override selects
the available host compiler on the validation machine.

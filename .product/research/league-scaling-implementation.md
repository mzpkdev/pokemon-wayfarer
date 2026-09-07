# League scaling implementation evidence

This record accompanies the [League scaling specification](../specs/league-scaling.md).
Structural checks and mechanics tests do not establish playable balance.

## Scope and progression

The implementation consumes `GetTrainerRating()` at admission. It preserves the
existing badge formula, rating high-water behavior, 15/5/4 League contributions,
player cap, and shop thresholds. The progression documents marked
`Implemented: Outdated` remain separate pending work.

Main already includes the Gym scaling and Mart integrations. League scaling has
its own curve and `B_LEAGUE_SCALING` switch. The existing Gym switch setting is
unchanged. League allowlist dispatch precedes the generic Trainer policy table;
its old League exclusions remain the fallback for ordinary scaling.

The following campaign examples assume no previously stored rating above the
current producer's result. Tests read actual admission TR before calculating
expected levels.

| Route | Kanto admission TR | Johto admission TR | Hoenn admission TR |
| --- | ---: | ---: | ---: |
| Minimum badges at each admission | 40 | 63 | 76 |
| All 24 badges before Kanto | 56 | 71 | 76 |

These are examples of the current producer, not a new reward contract. Both
routes reach Hoenn with first-E4 ace level 91 and Champion ace level 96 at TR 76.

## Roster inventory

`game/tools/trainer_scaling/league.py` resolves the existing sources through the
Trainer generator. Its reviewed `league_sources.json` fixture pins authored
fields, and `game/src/data/trainer_scaling/league.json` records each roster and
its levels at every integer TR from 0 through 80. Source slots remain in battle
order. Johto Lance's final Altaria remains the ace.

## Run ownership and recovery

The save-backed record stores only active state, circuit region, and entry TR.
The admission warp captures it before first-room scripts lock the entrance.
Existing room progression controls battle eligibility; a room resume does not
reset it. Construction and reconstruction read the saved TR and validate the
trainer, resolved owner, difficulty, region, and current room.

Whiteout and departures clear the run. Invalid unfinished saves take a clean
lobby warp before restoring any saved room objects or tiles. Completed Hall of
Fame saves retain their existing continue warp. Completion records the saved
circuit identity through the existing producer, preserves its one-shot handoff,
and clears the run before the completion save.

The full Hoenn emulator journey exposed an existing completion stall: the HNS
build cleared the Indigo Hall of Fame effect while Hoenn's script waited for
the Emerald effect. Wayfarer now selects the monitor visuals and effect cleanup
by venue layout. Standalone dispatch is unchanged.

## Automated validation

| Check | Result |
| --- | --- |
| League mechanics, scaling enabled | 26 passed |
| League mechanics, `B_LEAGUE_SCALING=0` | 26 passed |
| Existing TR producer mechanics | 9 passed |
| Existing Trainer/Gym scaling mechanics | 23 passed; 1 ordinary-scaling rollback-only assumption skipped |
| Existing Trainer scaling battle tests, including XP and money | 3 passed |
| Wayfarer persistence mechanics | 24 passed |
| Trainer scaling host tests | 22 top-level and 5 nested passed |
| League script structural tests | 16 passed |
| Existing League source/fixture host tests | 6 passed |
| Hoenn content audit | 518 maps passed |
| Hoenn entry audit | 5 tests passed |
| Standalone HNS authored League construction | 1 test passed, covering 10 opponents and all difficulties |
| Standalone Emerald authored League construction | 1 test passed, covering 5 opponents and all difficulties |

Production builds passed for Wayfarer, HNS, Emerald, FireRed, and LeafGreen.
After the final Hall of Fame fix, its shared `field_effect.c` translation unit
was recompiled successfully for all four standalone configurations; their full
builds and authored-constructor checks preceded that venue-dispatch fix.
The Wayfarer release build also passed with its accepted ROM-report baseline:
32,865,856 bytes of ROM, 248,553 bytes of EWRAM, and 25,568 bytes of IWRAM.
The E2E-enabled Wayfarer ROM built successfully.

Standalone linking exposed an existing Aqua-attendant flag reference in the
shared new-game script that HNS does not define. The reference now compiles only
where that flag exists. The Hoenn entry audit distinguishes standalone Emerald
from HNS when checking initialization; Wayfarer and Emerald behavior is preserved.

The League suite includes every integer TR, every enrolled roster, all three
resolved difficulties, repeated construction, source identity, species-randomizer
bypass, failed admission, progression validation, corruption recovery, and real
flash save/load followed by completion for all three circuits. The exhaustive
construction test uses 405 bounded parameter cases, covering 15 rosters, three
difficulties, and nine groups of nine ratings.

Run the normal League suite with:

```sh
make -C game -j8 BUILD=wayfarer check TESTS=League
make -C game trainer-party-scaling-test trainer-party-scaling-audit
```

The disabled-switch check compiled `battle_main.c` and `league_tiers.c` with
`-DB_LEAGUE_SCALING=0`, linked the same mechanics ROM, and ran its complete League
suite in mGBA. These are the only translation units that consume that switch.
Both were then recompiled with the default enabled setting. No source setting
or TR-progression switch was changed for this check.

The regression suites used the same mGBA runner with filename filters for
`test/trainer_rating.c`, `test/trainer_party_scaling.c`,
`test/battle/trainer_scaling_battle.c`, and `test/wayfarer_persistence.c`.

Standalone production commands were `make -C game -j8 hns`, `emerald`, `firered`,
and `leafgreen`. Standalone mechanics ROMs used `BUILD=hns` or `BUILD=emerald`,
`TEST=1`, and the `Standalone` test prefix. The release command was
`make -C game -j8 BUILD=wayfarer release`.

Comparison against the starting merge confirms the TR producer function,
`trainer_rating.c`, both authored Trainer party sources, and map layouts are
unchanged.

## Emulator journeys

All twelve distinct League E2E cases have passing evidence across three runs:

| Run | Evidence |
| --- | --- |
| Initial isolated-session run | Six passed: itinerary/TR/Trainer Card, mixed-order story state, earliest and intermediate Kanto, earliest and intermediate Johto. The run was stopped with exit 130 after later failures; it was not a clean full-suite pass. |
| Corrected Indigo routes | Two passed, ten filtered, exit 0: all-badges consecutive Kanto/Johto (155.223 s) and real battle loss followed by save/load and fresh admission (13.383 s). |
| Final Hoenn and recovery run | Four passed, eight filtered, exit 0: Hoenn's five battles, halls and completion reload (105.802 s), inconsistent saved progression (5.213 s), invalid Hall of Fame injection (1.204 s), denied admission (4.226 s). |

Earlier failed attempts exposed route-coordinate and fixture-wait issues, plus
the Hoenn effect-cleanup defect described above. The affected cases passed after
those fixes. This is coverage across focused runs, not a single twelve-case
clean invocation. The modified Aqua save/reload journey also passed (one selected
case, twelve filtered).

These journeys use actual admission TR for level expectations and force wins
to exercise progression. The loss case plays a level-one Pidgey using Tackle
through an actual whiteout. Neither fixture establishes representative campaign
balance.

The E2E protocol is version 10, exposing the saved run identity and rating.
Fixture warps track the destination after admission recovery redirects them.
Each journey uses its own emulator session; shutdown also cleans up paused
emulators that outlive their Xvfb wrapper.

Final `pnpm --dir e2e typecheck` and `pnpm --dir e2e lint` passed.
`pnpm --dir e2e test src/harness` passed all 20 tests in five files.
Reproduce the focused emulator checks from the repository root:

```sh
export LIBGL_ALWAYS_SOFTWARE=1
export SKYEMU_ROM="$PWD/game/pokemon-wayfarer-e2e.gba"
export SKYEMU_SYMS="$PWD/game/pokemon-wayfarer-e2e.sym"

pnpm --dir e2e exec node --input-type=module -e 'import { test } from "webanvil"; await test(["src/journeys/wayfarer-league-circuit.e2e.ts"], { include: ["src/**/*.e2e.ts"], testNamePattern: "consecutive Indigo|lost attempt", testTimeout: 120000, hookTimeout: 60000, reporters: ["verbose"] });'

pnpm --dir e2e exec node --input-type=module -e 'import { test } from "webanvil"; await test(["src/journeys/wayfarer-league-circuit.e2e.ts"], { include: ["src/**/*.e2e.ts"], testNamePattern: "Hoenn admission|inconsistent saved|Hall of Fame injection|denies entry", testTimeout: 120000, hookTimeout: 60000, reporters: ["verbose"] });'
```

SkyEmu crashed before connecting to Xvfb under this environment's command
sandbox. The same commands passed with command-specific escalation.

## Balance observations awaiting playtesting

At Kanto admission TR 40, the first E4 ace is level 38 and the Champion ace is
level 43. The existing moves and items still include setup and recovery on
Will's team, Toxic/evasion and Leftovers on Koga's team, and Dragon Dance,
Earthquake, and strong special coverage on Lance's team. These are source-data
observations, not measured difficulty findings.

Five-battle attrition with a representative campaign party, training needed
between Leagues on both routes, and remaining-Gym comparisons before and after
a clear require separate gameplay evidence. Any roster rebalance needs a
separate design revision.

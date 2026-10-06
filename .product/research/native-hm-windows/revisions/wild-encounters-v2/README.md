# Native HM catch windows: wild encounters v2 revision

Status: design revision. Not implemented. It applies once the
[wild encounters v2](../../../../prds/wild-encounters-v2.md) tables and
[wild level scaling](../../../../specs/wild-level-scaling.md) ship.
Base commit: `522afc588d7bb7f3e8c23ca5cf282b4a102ecd11`.

Wild encounters v2 replaces every encounter table and the way wild levels are
set. The [nearby-access revision](../nearby-access/README.md) was proven
against the old tables, the old Trainer Rating (TR) projection and TR 0–80, so
its evidence no longer holds. This revision re-audits the native HM roster
against the v2 tables, changes four roster entries, and adds the carriers the
crossings need directly to the v2 tables.

## What changes in the contract

| | Nearby-access revision | This revision |
| --- | --- | --- |
| TR scale | Today's 0–80 | v0 0–160 (0 to 24 badges) |
| Wild levels | Authored levels projected through TR, with species floors | The place's level ±2, the stage mix, young levels and prowler minimums, from wild level scaling |
| Fishing level | The rod's authored entry, projected | The place's level, whatever the rod |
| Regional coverage | Johto, Kanto and Hoenn, all eight utilities, TR 0–80 | The same regions and utilities, TR 0–160 |
| Crossing floor | 8% at one reachable source, TR 0–80 | 8% at one reachable source, **TR 20–160**, from two badges on |
| Encounter edits | 17 `encounter_replacements` on the old tables | Retired; the carriers are authored into the v2 tables (below) |
| Cinnabar | `MAP_CINNABAR_ISLAND_HNS` | `MAP_CINNABAR_ISLAND`, the map Wayfarer uses; the HNS map is retired |

- **The crossing guarantee starts at two badges.** No Kanto or Johto species
  knows Surf below level 7, and Roads are level 5 before the first badge.
  Until two badges, players cross with the Surf HM, as in the original games.
- **Everything else in the scenarios stays.** The same 11 scenarios, maps,
  methods and ranks apply. Blackthorn's Surf and the Den's Whirlpool still
  count only their local sources (rank 0), without the Ice Path detour.
- **Only land and Old Rod sources count toward the floor,** as before. Good and
  Super Rod odds are reported, not required.

## Roster changes

Four entries change. Each keeps a family within two utility types and uses
existing compatibility (`all_learnables.json`).

| Species | Change | Surf window | Why |
| --- | --- | --- | --- |
| Luvdisc | Adds Surf at 11 | 11–21 | Hoenn's sea had no carrier below Carvanha's 15. Luvdisc already has Waterfall, so it reaches the two-type limit |
| Frillish | Adds Surf at 27 | 27–48 | Hoenn's sea had no carrier between Carvanha (to 28) and Sharpedo (from 45) |
| Jellicent | Adds Surf at 45 | 45–100 | Keeps the Frillish line useful after it evolves |
| Quagsire | Surf moves from 40 to 28 | 28–47 (was 40–52) | Johto's ponds had no carrier between 28 and 34 |

Hoenn's sea chain becomes Luvdisc 11–21, Carvanha 15–28, Frillish 27–48, then
Jellicent and Sharpedo from 45. Johto's pond chain becomes Marill 7–15,
Psyduck 10–21, Poliwag 10–20, Slowpoke 12–27, Quagsire 28–47, Azumarill from
35 and Golduck from 45.

## Table changes

These edits are already in the
[Kanto](../../../../specs/kanto-encounter-tables.md),
[Johto](../../../../specs/johto-encounter-tables.md) and
[Hoenn](../../../../specs/hoenn-encounter-tables.md) table specs. Entries count
from 1, as the table specs do. [`hm_edits.json`](hm_edits.json) lists them by
zero-based index.

| Map | Time | Entry | From | To |
| --- | --- | ---: | --- | --- |
| Cianwood City | Day | 2 | Krabby–Kingler | Horsea–Seadra |
| Cianwood City | Night | 2 | Tentacool–Tentacruel | Horsea–Seadra |
| Olivine City | Day and night | 2 | Krabby–Kingler | Horsea–Seadra |
| Blackthorn City | Day | 1 | Magikarp–Gyarados | Wooper–Quagsire |
| Blackthorn City | Day | 4 | Magikarp–Gyarados | Slowpoke–Slowbro |
| Blackthorn City | Night | 3 | Poliwag–Poliwhirl | Slowpoke–Slowbro |
| Dragon's Den | Day | 2 | Poliwag–Poliwhirl | Magikarp–Gyarados |
| Dragon's Den | Day | 4 | Magikarp–Gyarados | Poliwag–Poliwhirl |
| Dragon's Den | Night | 1 | Dratini–Dragonite | Magikarp–Gyarados |
| Dragon's Den | Night | 9 | Psyduck–Golduck | Wooper–Quagsire |
| Route 6 | Day and night | 3 | Magikarp | Wooper–Quagsire |
| Cinnabar Island | Day | 1 | Krabby–Kingler | Tentacool–Tentacruel |
| Cinnabar Island | Day and night | 2 | Corsola | Horsea–Seadra |
| Cinnabar Island | Night | 3 | Horsea–Seadra | Tentacool–Tentacruel |
| Route 118 | Day | 4 | Luvdisc | Frillish–Jellicent |
| Pacifidlog Town | Night | 4 | Tynamo–Eelektrik | Carvanha–Sharpedo |

All edits are fishing entries. The Den's night Quagsire keeps its night table
distinct from the day one. Every table still passes the encounter rules
checker with no errors.

## Results

### Crossings

All 11 scenarios meet the 8% floor at every TR from 20 to 160, day and night.
The weakest point of each:

| Scenario | Lowest chance | Where |
| --- | ---: | --- |
| Cianwood Surf | 8.4% | Day, TR 62, Cianwood Old Rod |
| Olivine Surf | 8.4% | Day, TR 62, Olivine Old Rod |
| Vermilion Surf | 8.7% | Night, TR 59, Vermilion Old Rod |
| Cinnabar Surf | 9.1% | Night, TR 62, Cinnabar Old Rod |
| Lilycove Surf | 10.4% | Night, TR 20, Lilycove Old Rod |
| Mossdeep Surf | 8.0% | Night, TR 20, Mossdeep Old Rod |
| Pacifidlog Surf | 8.8% | Night, TR 20, Pacifidlog Old Rod |
| Route 118 west Surf | 10.0% | Day, TR 64, Route 118 Old Rod |
| Route 118 east Surf | 9.0% | Day, TR 84, Route 118 Old Rod |
| Blackthorn Surf | 8.0% | Day, TR 48, Blackthorn Old Rod |
| Dragon's Den Whirlpool | 8.4% | Day, TR 20, Den Old Rod |

Several margins are thin. Any later change to these maps' first fishing
entries must re-run the audit.

### Regional coverage

- **Johto and Kanto:** every utility at every TR from 0 to 160.
- **Hoenn:** every utility at every TR except Dive at TR 0–3, where no Hoenn
  place is high enough for a Dive carrier. Dive also needs Steven's grant, so
  the gap can't be felt in play.
- **Other regions** were never part of the contract. For reference: Sevii lacks
  Cut at TR 36–45, Flash from 36 and Waterfall at 0–6; Alola, Sinjoh and the
  Safari Zones have larger gaps, such as no Cut in Sinjoh and no water
  utilities in the Safari Zones.

## What the audit found

- **Hoenn lost its old carriers.** Hoenn's v2 natives exclude Generations I and
  II, which removed Tentacool and Kingler. Only Carvanha and Sharpedo carried
  Surf in Hoenn's sea, which the roster changes fix.
- **Young levels hide late windows.** A slot capped at a stage that can still
  evolve stays below the next evolution. On Roads, a Poliwhirl slot never
  passes 35, so its Surf at 37 never appears there. Staryu's Surf at 42 is
  hidden the same way. The table changes rely on other carriers instead.
- **The rod level shift was dropped.** Wild level scaling first gave the Old
  Rod −4 levels and the Good Rod −2. That pushed early Old Rod catches below
  every carrier's window, so fishing now uses the place's level.
- **Prowler minimums shape the Den.** The Dratini line is a dangerous prowler
  with a minimum of 30, so the Den's Dragonite slots never yield a Dratini
  young enough to know Whirlpool. The Den relies on Gyarados (22–32) and
  Poliwhirl (27–42) instead.

## Model and limits

- The model follows wild level scaling exactly: place levels from the reach
  and dungeon intent, a uniform −2 to +2 spread, prowler minimums, young
  levels, the downward rule with the shared evolution-level table, and the
  stage mix. It sums exact fractions within one source and never across
  sources.
- Movesets use the production initial-moveset algorithm over
  `gen_7.h` with `IS_WAYFARER`. Before the roster changes, it reproduced the
  nearby-access roster's windows for all 121 species with no mismatch.
- It is static evidence, like the nearby-access revision, not production
  acceptance. The implementation must replay these cells through the
  production code once v2 is built.
- The shared evolution-level table lacks some v2 form lines, such as Pumpkaboo's
  sizes and Floette's colours. None of them carries a utility, so the gaps
  don't affect these results.

## Files

| File | Purpose |
| --- | --- |
| [hm_model.py](hm_model.py) | Learnsets, movesets, wild level scaling and the roster changes |
| [hm_audit.py](hm_audit.py) | Regional coverage and the 11 crossing scenarios |
| [hm_opt.py](hm_opt.py) | The greedy search that chose the table changes |
| [hm_edits.json](hm_edits.json) | The table changes, by zero-based entry |
| [audit_regional.json](audit_regional.json) | Regional gaps and sample witnesses per region and utility |
| [audit_scenarios.json](audit_scenarios.json) | Best source per scenario, TR and time, with Good and Super Rod odds |
| [data/](data/) | The v2 tables with the changes applied, map metadata and species facts, as the tools read them |

Run `python3 hm_audit.py regional` and `python3 hm_audit.py scenarios` from
this folder. `hm_opt.py` finds no further edits on the included tables.

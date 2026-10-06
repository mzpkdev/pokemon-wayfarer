# Native HM catch windows: wild encounters v2 revision

Status: design revision. Not implemented. It applies once the
[wild encounters v2](../../../../prds/wild-encounters-v2.md) tables and
[wild level scaling](../../../../specs/wild-level-scaling.md) ship.
Base commit: `522afc588d7bb7f3e8c23ca5cf282b4a102ecd11`.

Wild encounters v2 replaces every encounter table and the way wild levels are
set. The [nearby-access revision](../nearby-access/README.md) was proven
against the old tables, the old Trainer Rating (TR) projection and TR 0–80, so
its evidence no longer holds. This revision re-audits the native HM roster
against the v2 tables and extends the guarantee from Johto, Kanto and Hoenn
to the Sevii Islands and Alola. It changes twelve roster entries and adds the
carriers the crossings need directly to the v2 tables.

The Safari Zones and the Bug-Catching Contest stay outside the guarantee.
Sinjoh is next.

## What changes in the contract

| | Nearby-access revision | This revision |
| --- | --- | --- |
| Regions | Johto, Kanto and Hoenn | Johto, Kanto, Hoenn, the Sevii Islands and Alola |
| TR scale | Today's 0–80 | v0 0–160 (0 to 24 badges) |
| Wild levels | Authored levels projected through TR, with species floors | The place's level ±2, the stage mix, young levels and prowler minimums, from wild level scaling |
| Fishing level | The rod's authored entry, projected | The place's level, whatever the rod |
| Regional coverage | All eight utilities, TR 0–80 | The utilities each region's maps need, TR 0–160 |
| Crossing floor | 11 scenarios, 8% at one reachable source, TR 0–80 | 22 scenarios, 8% at one reachable source, **TR 20–160**, from two badges on |
| Encounter edits | 17 `encounter_replacements` on the old tables | Retired; the carriers are authored into the v2 tables (below) |
| Cinnabar | `MAP_CINNABAR_ISLAND_HNS` | `MAP_CINNABAR_ISLAND`, the map Wayfarer uses; the HNS map is retired |

- **Regional coverage follows each region's field moves.** Johto, Kanto and
  Hoenn keep all eight. The Sevii Islands need Surf, Cut, Rock Smash,
  Strength and Waterfall: their maps hold water, Cut trees, breakable rocks,
  Strength boulders and waterfalls, but no dark caves, dive spots or
  whirlpools. Alola needs only Surf: its maps hold water and no other field
  obstacle.
- **The crossing guarantee starts at two badges.** No Kanto or Johto species
  knows Surf below level 7, and Roads are level 5 before the first badge.
  Until two badges, players cross with the Surf HM, as in the original games.
- **The original 11 scenarios keep their maps, methods and ranks.** Blackthorn's
  Surf and the Den's Whirlpool still count only their local sources (rank 0),
  without the Ice Path detour.
- **Only land and Old Rod sources count toward the floor,** as before. Good and
  Super Rod odds are reported, not required.

### Sevii and Alola arrivals

A player reaches each Sevii island by the Seagallop ferry and Alola's first
island by the boat from Route 13. Alola's islands are linked only by Surf until
the Tapu signs open fast travel, which needs all four Tapu battles. So every
island gets a Surf scenario. Its sources are the land tables and shore fishing a
player reaches on foot from the arrival point, with no field move.
[`walker.py`](walker.py) walks the source layouts as the engine does, and
[`walk_arrivals.py`](walk_arrivals.py) lists the sources it finds:

| Scenario | Arrival | Sources reached on foot |
| --- | --- | --- |
| One Island | Harbor | One Island and Kindle Road fishing |
| Two Island | Harbor | Cape Brink land and fishing |
| Three Island | Harbor | Bond Bridge and Berry Forest land and fishing |
| Four Island | Harbor | Four Island fishing |
| Five Island | Harbor | Five Island fishing; Five Isle Meadow land and fishing |
| Six Island | Harbor | Water Path land and fishing |
| Seven Island | Harbor | Sevault Canyon and its entrance land; Tanoby Ruins and Trainer Tower fishing |
| Melemele | Kanto boat | Melemele land and fishing |
| Akala | Tapu sign | Akala land and fishing; Akala Forest and Akala Cave land |
| Ula'ula | Tapu sign | Ula'ula land and fishing; both Ula'ula caves |
| Poni | Tapu sign | Poni land and fishing; Poni Cave |

Starting from any shore of an Alola island instead of its sign reaches the
same sources. The scenarios are in
[`scenarios_regions.json`](scenarios_regions.json); all their sources are
rank 0.

## Roster changes

Twelve entries change. Each keeps a family within two utility types and uses
existing compatibility (`all_learnables.json`).

| Species | Change | Window | Why |
| --- | --- | --- | --- |
| Luvdisc | Adds Surf at 11 | Surf 11–21 | Hoenn's sea had no carrier below Carvanha's 15. Luvdisc already has Waterfall, so it reaches the two-type limit |
| Frillish | Adds Surf at 27 | Surf 27–48 | Hoenn's sea had no carrier between Carvanha (to 28) and Sharpedo (from 45) |
| Jellicent | Adds Surf at 45 | Surf 45–100 | Keeps the Frillish line useful after it evolves |
| Quagsire | Surf moves from 40 to 28 | Surf 28–47 (was 40–52) | Johto's ponds had no carrier between 28 and 34 |
| Arrokuda | Adds Surf at 11 | Surf 11–29 | Galar's sea had no Surf carrier. Arrokuda already has Dive |
| Barraskewda | Adds Surf at 34 | Surf 34–100 | Carries the line's Surf to the outer islands' high levels |
| Drednaw | Adds Surf at 22 | Surf 22–56 | Bridges Arrokuda and Barraskewda in Sevii's mid levels |
| Chewtle | Adds Waterfall at 5 | Waterfall 5–27 | Sevii had no Waterfall carrier at TR 0–6 |
| Weepinbell | Cut moves from 30 to 20 | Cut 20–38 (was 30–46) | A Road Weepinbell never passes 35, so Sevii had no Cut at TR 36–45 |
| Mareanie | Adds Surf at 11 | Surf 11–24 | Alola's sea natives had no Surf carrier |
| Wishiwashi | Adds Surf at 22 | Surf 22–37 | Bridges Mareanie and Toxapex. Wishiwashi already has Dive; both forms share its learnset |
| Toxapex | Adds Surf at 38 | Surf 38–100 | Carries Alola's Surf to its high levels |

The chains these build:

- **Hoenn's sea:** Luvdisc 11–21, Carvanha 15–28, Frillish 27–48, then
  Jellicent and Sharpedo from 45.
- **Johto's ponds:** Marill 7–15, Psyduck 10–21, Poliwag 10–20, Slowpoke 12–27,
  Quagsire 28–47, Azumarill from 35 and Golduck from 45.
- **Sevii's sea:** Arrokuda 11–29, Drednaw 22–56 and Barraskewda from 34.
- **Alola's sea:** Mareanie 11–24, Wishiwashi 22–37 and Toxapex from 38, with
  Luvdisc and Pelipper from the blend.

## Table changes

These edits are already in the
[Kanto](../../../../specs/kanto-encounter-tables.md),
[Johto](../../../../specs/johto-encounter-tables.md),
[Hoenn](../../../../specs/hoenn-encounter-tables.md),
[Sevii](../../../../specs/sevii-encounter-tables.md) and
[Alola](../../../../specs/alola-encounter-tables.md) table specs. Entries count
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
| Four Island | Day | 4 | Seel–Dewgong | Spheal–Walrein |
| Four Island | Night | 1 | Clobbopus | Arrokuda–Barraskewda |
| Melemele | Day | 4 | Staryu | Wishiwashi |
| Akala | Day | 5 | Luvdisc | Mareanie–Toxapex |
| Ula'ula | Day | 5 | Shellder | Mareanie–Toxapex |

All edits are fishing entries. The Den's night Quagsire keeps its night table
distinct from the day one. Alola's three edits replace blend species with
natives, so each island stays within its 20% blend share. Every table still
passes the encounter rules checker with no errors.

## Results

### Crossings and arrivals

All 22 scenarios meet the 8% floor at every TR from 20 to 160, day and night.
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
| One Island Surf | 40.8% | Day, TR 64, Kindle Road Old Rod |
| Two Island Surf | 28.1% | Night, TR 53, Cape Brink Old Rod |
| Three Island Surf | 36.1% | Night, TR 53, Berry Forest Old Rod |
| Four Island Surf | 9.0% | Night, TR 64, Four Island Old Rod |
| Five Island Surf | 24.0% | Day, TR 20, Five Island Old Rod |
| Six Island Surf | 14.0% | Night, TR 53, Water Path Old Rod |
| Seven Island Surf | 8.5% | Day, TR 20, Tanoby Ruins Old Rod |
| Melemele Surf | 11.2% | Day, TR 79, Melemele Old Rod |
| Akala Surf | 9.0% | Day, TR 70, Akala Old Rod |
| Ula'ula Surf | 9.0% | Day, TR 62, Ula'ula Old Rod |
| Poni Surf | 11.4% | Day, TR 62, Poni Old Rod |

Several margins are thin. Any later change to these maps' first fishing
entries must re-run the audit.

### Regional coverage

- **Johto and Kanto:** every utility at every TR from 0 to 160.
- **Hoenn:** every utility at every TR except Dive at TR 0–3, where no Hoenn
  place is high enough for a Dive carrier. Dive also needs Steven's grant, so
  the gap can't be felt in play.
- **The Sevii Islands:** Surf, Cut, Rock Smash, Strength and Waterfall at every
  TR from 0 to 160.
- **Alola:** Surf at every TR from 0 to 160.
- **Outside the guarantee,** for reference: Sevii has no Flash carrier from
  TR 36, and Alola lacks several moves its maps don't use. Sinjoh and the
  Safari Zones have larger gaps, such as no Cut in Sinjoh and no water
  utilities in the Safari Zones.

## What the audit found

- **Hoenn lost its old carriers.** Hoenn's v2 natives exclude Generations I and
  II, which removed Tentacool and Kingler. Only Carvanha and Sharpedo carried
  Surf in Hoenn's sea, which the roster changes fix.
- **Galar and Alola's sea natives carried no Surf.** The roster was built for
  Generations I to III, so Sevii's Galar natives and Alola's Generation VII
  natives needed their own carriers.
- **Young levels hide late windows.** A slot capped at a stage that can still
  evolve stays below the next evolution. On Roads, a Poliwhirl slot never
  passes 35, so its Surf at 37 never appears there. Staryu's Surf at 42 and
  Weepinbell's old Cut at 30 were hidden the same way.
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
- The walker follows collision, elevation, ledges, map connections and warps,
  and treats Cut trees, breakable rocks, Strength boulders and whirlpools as
  blocking. It treats other people as passable, so a story blocker that stands
  in a path would need a manual check.
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
| [hm_audit.py](hm_audit.py) | Regional coverage and all 22 scenarios |
| [hm_opt.py](hm_opt.py) | The greedy search that chose the table changes |
| [walker.py](walker.py) | On-foot reachability over the source layouts |
| [walk_arrivals.py](walk_arrivals.py) | The sources reached from each Sevii and Alola arrival |
| [scenarios_regions.json](scenarios_regions.json) | The 11 Sevii and Alola arrival scenarios |
| [hm_edits.json](hm_edits.json) | The table changes, by zero-based entry |
| [audit_regional.json](audit_regional.json) | Regional gaps and sample witnesses per region and utility |
| [audit_scenarios.json](audit_scenarios.json) | Best source per scenario, TR and time, with Good and Super Rod odds |
| [data/](data/) | The v2 tables with the changes applied, map metadata and species facts, as the tools read them |

Run `python3 hm_audit.py regional` and `python3 hm_audit.py scenarios` from
this folder. `hm_opt.py` finds no further edits on the included tables.

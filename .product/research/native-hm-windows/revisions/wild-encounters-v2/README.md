# Native HM catch windows: wild encounters v2 revision

Status: implemented for Wayfarer. The tables, the model and the checker now live in the game
(see [Where things moved](#where-things-moved)); this folder keeps the research record and
the authoring scripts. It applies with the
[wild encounters v2](../../../../prds/wild-encounters-v2.md) tables and
[wild level scaling](../../../../specs/wild-level-scaling.md).
Base commit: `522afc588d7bb7f3e8c23ca5cf282b4a102ecd11`.

Wild encounters v2 replaces every encounter table and the way wild levels are
set. The [nearby-access revision](../nearby-access/README.md) was proven
against the old tables, the old Trainer Rating (TR) projection and TR 0–80, so
its evidence no longer holds. This revision re-audits the native HM roster
against the v2 tables and extends the guarantee from Johto, Kanto and Hoenn
to the Sevii Islands, Alola and Sinjoh, and adds the Hoenn places reached by
boat or left only with a field move. It changes fourteen roster entries and
adds the carriers the crossings need directly to the v2 tables.

The Safari Zones and the Bug-Catching Contest stay outside the guarantee.

## What changes in the contract

| | Nearby-access revision | This revision |
| --- | --- | --- |
| Regions | Johto, Kanto and Hoenn | Johto, Kanto, Hoenn, the Sevii Islands, Alola and Sinjoh |
| TR scale | Today's 0–80 | v0 0–160 (0 to 24 badges) |
| Wild levels | Authored levels projected through TR, with species floors | The place's level ±2, the stage mix, young levels and prowler minimums, from wild level scaling |
| Fishing level | The rod's authored entry, projected | The place's level, whatever the rod |
| Regional coverage | All eight utilities, TR 0–80 | The utilities each region's maps need, TR 0–160 |
| Crossing floor | 11 scenarios, 8% at one reachable source, TR 0–80 | 28 scenarios, 8% at one reachable source, **TR 20–160**, from two badges on |
| Encounter edits | 17 `encounter_replacements` on the old tables | Retired; the carriers are authored into the v2 tables (below) |
| Cinnabar | `MAP_CINNABAR_ISLAND_HNS` | `MAP_CINNABAR_ISLAND`, the map Wayfarer uses; the HNS map is retired |

- **Regional coverage follows each region's field moves.** Johto, Kanto and
  Hoenn keep all eight. The Sevii Islands need Surf, Cut, Rock Smash,
  Strength and Waterfall: their maps hold water, Cut trees, breakable rocks,
  Strength boulders and waterfalls, but no dark caves, dive spots or
  whirlpools. Alola needs only Surf: its maps hold water and no other field
  obstacle. Sinjoh needs only Rock Smash: Snowswept Cavern's rocks are its only
  way in and out, while Route 49's lake and waterfalls and the cavern's two
  Strength boulders open nothing the walker can't reach on foot.
- **The crossing guarantee starts at two badges.** No Kanto or Johto species
  knows Surf below level 7, and Roads are level 5 before the first badge.
  Until two badges, players cross with the Surf HM, as in the original games.
  TR 20 is also the first TR at which every scenario passes: Hoenn's Lilycove
  and Pacifidlog (at night), Mossdeep and Route 118 east crossings, the Den's
  Whirlpool and Sootopolis's Dive all still fall short at TR 19, whatever their
  carriers.
- **The original 11 scenarios keep their maps, methods and ranks.** Blackthorn's
  Surf and the Den's Whirlpool still count only their local sources (rank 0),
  without the Ice Path detour.
- **Only land and Old Rod sources count toward the floor,** as before. Good and
  Super Rod odds are reported, not required.

### Arrivals

A player reaches each Sevii island by the Seagallop ferry and Alola's first
island by the boat from Route 13. Alola's islands are linked only by Surf until
the Tapu signs open fast travel, which needs all four Tapu battles. So every
island gets a Surf scenario. Hoenn adds three places:

- **Dewford Town,** reached by Mr. Briney's boat, gets a Surf scenario.
- **Sootopolis City** sits in a crater with no map connections, so it is left
  only by Dive. It gets a Surf and a Dive scenario, from its own shore.
- **Ever Grande City** is reached by climbing Route 128's waterfall, but a
  surfer is pushed down a waterfall without the move, so it is left by Surf
  alone. It gets a Surf scenario.
- **The Battle Frontier** needs none: the ferry runs both ways and no
  encounter lies within walking distance of its dock.

Sinjoh adds two Rock Smash scenarios, one on each side of Snowswept Cavern's
rocks, since smashed rocks return when you re-enter: the way in from Mt.
Silver's waterfall room, and the way out from New Sinjoh.

Each scenario's sources are the land tables and shore fishing a player reaches
on foot from the arrival point, with no field move.
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
| Dewford | Mr. Briney's boat | Dewford, Route 106 and Route 107 fishing; Granite Cave land |
| Sootopolis (Surf and Dive) | Pokémon Center | Sootopolis fishing |
| Ever Grande | Pokémon Center | Ever Grande fishing; Victory Road 1F and B1F land |
| Into Sinjoh (Rock Smash) | Mt. Silver's waterfall room | That room's land and fishing; Snowswept Cavern's mouth |
| Out of Sinjoh (Rock Smash) | New Sinjoh | Route 49 land and fishing; Route 50, the Hot Springs, the Sinjoh Ruins and its temple, and the cavern's far side |

Starting from any shore of an Alola island instead of its sign reaches the
same sources. The scenarios are in
[`scenarios_regions.json`](scenarios_regions.json); all their sources are
rank 0.

## Roster changes

Fourteen entries change. Each keeps a family within two utility types and uses
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
| Carvanha | Adds Dive at 12 | Dive 12–21 | Sootopolis had no Dive carrier below Clamperl's 20, and Hoenn none at TR 0–3 |
| Graveler | Adds Rock Smash at 26 | Rock Smash 26–43 | Sinjoh had no Rock Smash carrier above Nosepass's 36. A Graveler slot stays below Golem's 38, so it keeps the move at any TR |

The chains these build:

- **Hoenn's sea:** Luvdisc 11–21, Carvanha 15–28, Frillish 27–48, then
  Jellicent and Sharpedo from 45.
- **Johto's ponds:** Marill 7–15, Psyduck 10–21, Poliwag 10–20, Slowpoke 12–27,
  Quagsire 28–47, Azumarill from 35 and Golduck from 45.
- **Sevii's sea:** Arrokuda 11–29, Drednaw 22–56 and Barraskewda from 34.
- **Alola's sea:** Mareanie 11–24, Wishiwashi 22–37 and Toxapex from 38, with
  Luvdisc and Pelipper from the blend.
- **Sootopolis's Dive:** Carvanha 12–21, Clamperl 20–41, then Wailmer and
  Wailord from 41.
- **Sinjoh's Rock Smash:** Hisuian Sneasel 6–24, Teddiursa 14–28, Nosepass
  25–36 and Graveler 26–37, the highest a Graveler slot reaches.

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
| Sootopolis City | Day | 2 | Wailmer–Wailord | Clamperl |
| Sootopolis City | Day | 4 | Luvdisc | Carvanha–Sharpedo |
| Sootopolis City | Day | 5 | Tynamo–Eelektrik | Ducklett–Swanna |
| Sootopolis City | Night | 3 | Tynamo–Eelektrik | Clamperl |
| Sootopolis City | Night | 4 | Frillish | Carvanha–Sharpedo |
| Snowswept Cavern | Night | 3 | Zubat–Golbat | Geodude–Graveler |
| Route 49 | Night | 6 | Swinub–Piloswine | Teddiursa–Ursaring |
| Ever Grande City | Day | 4 | Wailmer–Wailord | Carvanha–Sharpedo |
| Ever Grande City | Night | 4 | Frillish | Carvanha–Sharpedo |

Two of the 33 edits are land entries, Snowswept Cavern's and Route 49's night
tables; the rest are fishing entries. The Den's night Quagsire keeps its night
table distinct from the day one. Alola's three edits replace blend species with
natives, so each island stays within its 20% blend share. Clamperl is a
harmless reward, which normally takes only rare slots; it holds a common
Sootopolis slot as the city's resident Dive carrier, as the Hoenn rules record.
Sootopolis's day Swanna leaves Eelektrik to the night, so the night table stays
distinct. Every table still passes the committed `check.py` with no
errors.

Four of the 33 edits were picked by hand, since they lie outside
[`hm_opt.py`](hm_opt.py)'s search space, which tries only fishing entries 1–4
(and land entries 1–6 on a few maps), from the species pools in the script: the
Den's night entry 9 (Golduck to Quagsire), Akala's and Ula'ula's day entry 5
(both to Toxapex) and Sootopolis's day entry 5 (Eelektrik to Swanna, which no
pool holds). The other 29 came from the search. Re-running it from the pre-edit
tables didn't produce a single edit in about 25 minutes, so the search isn't
practically reproducible; treat the edit list as the record.

The table review that followed the audit changed other table slots, none of
them a crossing carrier: it moved dangerous prowlers off the first floor of
Mt. Silver and Lost Cave, replaced night Pelipper, Swanna and the Safari
Zones' day-only species, and added the Faraway Island and Southern Island
tables. The audit and its results above include them; no scenario result
changed.

## Results

### Crossings and arrivals

All 28 scenarios meet the 8% floor at every TR from 20 to 160, day and night.
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
| Dewford Surf | 8.0% | Day, TR 62, Route 107 Old Rod |
| Sootopolis Surf | 10.0% | Day, TR 48, Sootopolis Old Rod |
| Sootopolis Dive | 8.0% | Day, TR 20, Sootopolis Old Rod |
| Ever Grande Surf | 11.0% | Day, TR 48, Ever Grande Old Rod |
| Into Sinjoh, Rock Smash | 10.0% | Day, TR 42, Mt. Silver's waterfall room land |
| Out of Sinjoh, Rock Smash | 9.0% | Night, TR 46, Route 49 land |

Several margins are thin. Any later change to these maps' first fishing
entries must re-run the audit.

### Regional coverage

- **Johto and Kanto:** every utility at every TR from 0 to 160.
- **Hoenn:** every utility at every TR from 0 to 160. Carvanha's Dive closes
  the old gap at TR 0–3.
- **The Sevii Islands:** Surf, Cut, Rock Smash, Strength and Waterfall at every
  TR from 0 to 160.
- **Alola:** Surf at every TR from 0 to 160.
- **Sinjoh:** Rock Smash at every TR from 0 to 160.
- **Outside the guarantee,** for reference: Sevii has no Flash carrier from
  TR 36, and Alola and Sinjoh lack several moves their maps don't use. The
  Safari Zones have no water utilities.

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
- **Conditional evolutions were misread at first.** Friendship, held-item and
  known-move evolutions are stored as level 0 with conditions. The model first
  read them as level-0 evolutions, which pinned slots such as Golbat, Sneasel
  and Nosepass to level 1. Using the shared evolution-level table for them
  changed no crossing result, but it made Sinjoh's Rock Smash gap visible.
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
- Conditional evolutions, such as friendship or a held item at night, use the
  shared evolution-level table like any other non-level evolution.
- Movesets use the production initial-moveset algorithm over
  `gen_7.h` with `IS_WAYFARER`. Before the roster changes, it reproduced the
  nearby-access roster's windows for all 121 species with no mismatch.
- The walker follows collision, elevation, ledges, map connections and warps,
  and treats Cut trees, breakable rocks, Strength boulders and whirlpools as
  blocking. A surfer moves only down a waterfall, as the engine's forced
  movement pushes it south. It treats other people as passable, so a story blocker that stands
  in a path would need a manual check.
- It is static evidence, like the nearby-access revision. Production
  acceptance is `game/test/native_hm_catch_coverage.c`, which replays every
  scenario cell and the required regional coverage through the production
  level code and real movesets, and must equal this model exactly.
- The model resolves species aliases on both sides of an evolution edge, since
  the shared evolution-level table files some edges under form constants
  (`FLORGES_RED`, `AEGISLASH_SHIELD`, `DUDUNSPARCE_TWO_SEGMENT` and others).
  Where a species reaches the same successor by trade and by level, as
  Alolan Graveler, Phantump and Pumpkaboo's sizes do at 38, the model uses the
  level. With that, every v2 edge has a level, and no result changes. Dunsparce, which carries Rock
  Smash at 20, keeps a young limit of 34 from its edge at 35.
- Every reward takes its prowler minimum in any slot, common or rare, at any
  stage of its line (babies skipped when looking for the line's first stage).
  Only Sinjoh's residents at home (its Hisuian natives, Stantler and Scyther)
  and Kalos in the Safari Zones are exempt.
- A dungeon's floor and floor count come from the `floor` and `floors` of
  `meta.json`, which follow the steps in
  [reach-assignments](../../../../specs/reach-assignments.md#dungeon-floors).
  The model also parses `specs/prowlers.md` and `specs/reach-assignments.md`
  when it runs, so a later edit to either one changes its results and must
  be followed by a re-run.

## Where things moved

The v2 tables are build input for the game, so the data and the build-time checks moved out of this folder:

| Was here | Now |
| --- | --- |
| `data/*.json` (tables, `meta.json`, `species.json`, tile files) | `game/src/data/wild_encounters_v2/` (encounter rates are not data: `v2_emit.py` sets them by method and terrain) |
| `check.py` | `game/tools/wild_encounters/v2/check.py`, run by `make wild-encounters-v2-check` (before the header is generated, and part of `make check`) |
| `hm_model.py`, `hm_audit.py` | `game/tools/wild_encounters/v2/`; `make wild-encounters-v2-hm-audit` runs the audit (about a minute) |

The generator (`game/tools/wild_encounters/wild_encounters_to_header.py`, `v2/v2_emit.py`) builds
`gWildMonHeaders`, `gWildEncounterPlaces` and `gWildProwlerMinimums` from that data;
`game/tools/wild_encounters/tests/test_v2_emit.py` cross-checks them against `hm_model.py`.
`render.py`, `hm_opt.py`, `export_roster.py` and `walk_arrivals.py` stay here and import the moved modules.
`roster_v2.json`, the scenarios and the `audit_*.json` outputs stay here. The 14 roster changes now ship in
`gen_7.h`, so `hm_model.apply_roster_v2()` skips entries already there.

## Editing the tables

`game/src/data/wild_encounters_v2/*.json` is the source of truth for the v2 encounter tables, not the
specs. The table specs' tables are rendered from it. To change a table:

1. **Edit** the region's `<region>.json` in that folder (`kanto`, `johto`, `hoenn`,
   `alola`, `sevii`, `safari` or `sinjoh`). A slot is a species constant
   written as the slot's stage cap. A new map also needs an entry in
   `meta.json` (region, place, reach, band, map type, floor, methods) and
   its terrain counts in `tiles.json`, `safari_tiles.json` or
   `sinjoh_tiles.json`, which `python3 scan_tiles.py maps.json out.json`
   produces from a JSON list of map constants.
2. **Render** it into the spec: `python3 render.py [region ...]` rewrites
   everything after "### Tables" in `specs/<region>-encounter-tables.md` and
   keeps the hand-written preamble above it. `python3 render.py --check`
   reports a spec that differs from its data without writing.
3. **Check** the rules: `python3 game/tools/wild_encounters/v2/check.py` (or `make wild-encounters-v2-check` in `game/`) checks every region and exits with
   an error if any rule breaks. It enforces the slot counts and weights,
   native generations, reach and temperament (including no dangerous prowler
   on a dungeon's first floor step), night rules, crossing carriers, terrain
   coverage and the generation-share bands, and it prints notes that need a
   human look, such as a species outside its water type's cast. It reads
   `specs/prowlers.md` for the reward table.
4. **Audit** the native HM guarantee: `make wild-encounters-v2-hm-audit` in `game/`, or
   `python3 game/tools/wild_encounters/v2/hm_audit.py scenarios` and `... regional`. Both write the `audit_*.json` files. Every
   scenario must still pass from TR 20 to 160, and regional coverage must not
   regress. If a change breaks a crossing, fix the carrier in the data and
   run the loop again.

If the roster changes, edit `ROSTER_V2` in `game/tools/wild_encounters/v2/hm_model.py` and run
`python3 export_roster.py` to rebuild `roster_v2.json`, which is the v2
roster's machine-readable form: 130 species and 166 roles, the nearby-access
roster plus fourteen changes.

The scripts that first generated these tables (the per-region `gen_*.py` and
`*_fix.py` authoring scripts) are **not included**. They were scratch work, and
the data files they produced are the record. Nothing here rebuilds the data
from the specs or from game files; edit the data and render.

## Files

| File | Purpose |
| --- | --- |
| `hm_model.py` (moved to `game/tools/wild_encounters/v2/`) | Learnsets, movesets, wild level scaling and the roster changes |
| `hm_audit.py` (moved to `game/tools/wild_encounters/v2/`) | Regional coverage and all 28 scenarios |
| [hm_opt.py](hm_opt.py) | Greedy search for crossing-carrier edits; chose 29 of the 33 edits (see Table changes) |
| `check.py` (moved to `game/tools/wild_encounters/v2/`) | The encounter rules checker |
| [render.py](render.py) | Renders the v2 table data into the table specs |
| [scan_tiles.py](scan_tiles.py) | Counts encounter terrain on a list of maps, from the source layouts |
| [export_roster.py](export_roster.py) | Exports the v2 roster |
| [roster_v2.json](roster_v2.json) | The v2 native HM roster: 130 species, 166 roles |
| [walker.py](walker.py) | On-foot reachability over the source layouts |
| [walk_arrivals.py](walk_arrivals.py) | The sources reached from each Sevii, Alola and Hoenn arrival |
| [scenarios_regions.json](scenarios_regions.json) | The 17 Sevii, Alola, Hoenn and Sinjoh arrival scenarios |
| [hm_edits.json](hm_edits.json) | The 33 crossing edits, by zero-based entry |
| [audit_regional.json](audit_regional.json) | Regional gaps and sample witnesses per region and utility |
| [audit_scenarios.json](audit_scenarios.json) | Best source per scenario, TR and time, with Good and Super Rod odds |
| `data/` (moved to `game/src/data/wild_encounters_v2/`) | The source of truth: the v2 tables, map metadata, terrain counts and species facts, as the tools read them |

Run every script from this folder. `hm_opt.py` finds no further edits on the
included tables.

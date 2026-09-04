# Native HM utility learnsets

PRD: [Native HM utility learnsets](../prds/native-hm-learnsets.md)
Implemented: Yes

## Scope

This specification defines the level-up learnset additions that let selected wild Pokémon provide Cut, Flash, Surf, Strength, Rock Smash, Waterfall, Dive, or Whirlpool before the matching HM is obtained. It covers Emerald, FireRed, LeafGreen, and HNS in both the normal and Generation III legacy-moves modes. Dive additions apply only to Emerald. HNS receives Whirlpool additions but no Dive additions.

The feature changes species learnsets only. It does not change encounters, HM compatibility, move data, field-action eligibility, story rewards, map access, fishing availability, or Trainer Rating scaling. The existing HM field-use system remains responsible for resolving a party Pokémon that knows the move and for enforcing terrain and map context.

Fly remains excluded. Wild-species and learnset randomizers may replace the authored species or moves and are outside the native-roster guarantee.

## Behavior

### Learnset sources and ordering

Add the utility moves directly to both active learnset sources:

- `game/src/data/pokemon/level_up_learnsets/gen_7.h` for normal mode;
- `game/src/data/pokemon/level_up_learnsets/gen_3.h` for Generation III legacy-moves mode.

Do not add a runtime moveset override, encounter-specific move field, or post-catch correction. Gate the additions by build: compile the Kanto rows only when `IS_FRLG` is true, the Johto rows only when `IS_HNS` is true, and the Hoenn rows only when both `IS_FRLG` and `IS_HNS` are false. Apply the same condition to an anchor and all of its successor additions in both learnset sources. Within a build, normal and legacy-moves mode use the same utility roster but different repeat levels because their existing native move cadences differ.

At a level that already contains one or more moves, place the additions after all existing entries at that level. When a row assigns several utility moves at one level, preserve the order shown. Existing entries and their relative ordering remain unchanged.

Normal wild creation keeps the last four distinct level-up moves available at the caught level. Repeating a utility move before it has left that four-move set does not refresh its position, so every repeat below is placed at the first level where the move would otherwise be absent after existing same-level moves are processed.

### Coverage inventory

The following existing encounter profiles establish the required two-place coverage. Each regional row applies only to its corresponding build: Kanto to FireRed and LeafGreen, Johto to HNS, and Hoenn to Emerald. Level ranges are authored levels before Trainer Rating projection. Floors or rooms of one dungeon count as one place. No encounter data changes are authorized.

| Region | Anchor | Utility moves | Qualifying existing places and authored levels |
| --- | --- | --- | --- |
| Kanto | Paras | Cut | Mt. Moon, 5 to 12; Safari Zone, 22 to 23 |
| Kanto | Rattata | Cut | Route 1 and Route 2 or Route 22, 2 to 5 |
| Kanto | Voltorb | Flash | Route 10, 14 to 17; Power Plant, 22 to 25 |
| Kanto | Pikachu | Flash | Viridian Forest, 3 to 5; Power Plant, 22 to 26 |
| Kanto | Horsea | Surf, Waterfall | Pallet Town and Cinnabar Island fishing, 5 to 25 in both versions |
| Kanto | Krabby | Surf | Pallet Town and Cinnabar Island fishing, 5 to 25 in both versions |
| Kanto | Machop | Strength | Rock Tunnel, 16 to 17; Mt. Ember, 31 to 39 |
| Kanto | Geodude | Strength, Rock Smash | Mt. Moon, 7 to 10; Rock Tunnel land encounters, 15 to 17 |
| Kanto | Mankey | Rock Smash | Route 22 and Route 3 or Route 4, 2 to 12 |
| Kanto | Goldeen | Waterfall | Route 6 and Route 22 or Route 25 fishing, 5 to 15 |
| Johto | Gligar | Cut | Route 42, level 21; Route 45, level 31 |
| Johto | Aipom | Cut, Rock Smash | Azalea Town and Route 33 Headbutt-backed profiles, level 10 |
| Johto | Chinchou | Flash, Surf, Whirlpool | Olivine port, levels 20 and 40; Cianwood fishing, level 20 |
| Kanto | Chinchou | Flash, Surf, Whirlpool | Vermilion and Cinnabar fishing, authored level 5 and effective levels 9 to 90 |
| Johto | Mareep | Flash | Route 31 and Route 32, 5 to 7 |
| Johto | Wooper | Surf, Waterfall | Route 32 and Ruins of Alph, 4 to 19 |
| Johto | Snubbull | Strength | Route 34 and Route 35, 13 to 15 |
| Johto | Miltank | Strength, Rock Smash | Route 38 and Route 39, level 21 |
| Johto | Marill | Waterfall | Union Cave, 8 to 9; Route 42, level 20 |
| Johto | Mantine | Whirlpool | Whirl Islands, 15 to 24; Route 41, 22 to 26 |
| Hoenn | Corphish | Cut, Rock Smash | Petalburg City and Route 102 or Route 117 fishing, 10 to 45 |
| Hoenn | Sableye | Cut, Flash | Granite Cave, 9 to 12; Cave of Origin, 30 to 34 |
| Hoenn | Electrike | Flash | Route 110, 12 to 13; Route 118, 24 to 26 |
| Hoenn | Lotad | Surf | Route 102, 3 to 4; Route 114, 15 to 16 |
| Hoenn | Wailmer | Surf, Dive | Lilycove City and Mossdeep City or Pacifidlog Town fishing, 10 to 45 |
| Hoenn | Makuhita | Strength | Granite Cave, 6 to 10; Victory Road, level 36 |
| Hoenn | Torkoal | Strength | Fiery Path, 14 to 16; Magma Hideout, 28 to 30 |
| Hoenn | Aron | Rock Smash | Granite Cave, 7 to 12; Victory Road, level 36 |
| Hoenn | Barboach | Waterfall | Route 111 or Route 114 and Route 120 or Meteor Falls fishing, 10 to 45 |
| Hoenn | Carvanha | Waterfall, Dive | Route 118 and Route 119 fishing, 10 to 45 |

Aipom's qualifying profiles are stored as Rock Smash encounter data but are also consumed by the independent Headbutt interaction. Aipom counts only through Headbutt access; Rock Smash cannot be required to obtain its native Rock Smash user.

Chinchou also supplies the HNS Kanto Surf crossings. The six Vermilion, port,
and Cinnabar day/night profiles author it at level 5. Standalone HNS projects
those catches to levels 9 through 90 over its Rating 10 to 80 range, so its
implemented schedule starts at level 9 and preserves all three utility moves
through level 100. Wayfarer's Rating 0 circuit adds a level-5 entry through its
own specification so the same sources remain usable there.

Trainer Rating projects ordinary encounters above their authored levels. The authored additions must therefore preserve every assigned utility move in the active four-move set at every level from the anchor's lowest qualifying level through level 100. This is deliberately stronger than checking only the listed authored ranges and covers every current projection, including the convergence toward level 90 at Rating 80.

### Anchor schedules

Add the following entries to the anchor species. `L` means level. Multiple moves at one level are inserted in the displayed order after existing entries at that level.

| Build | Anchor | Normal mode additions | Legacy-moves additions |
| --- | --- | --- | --- |
| FireRed and LeafGreen | Paras | L5 Cut; L17 Cut; L38 Cut | L5 Cut; L25 Cut; L49 Cut |
| FireRed and LeafGreen | Rattata | L2 Cut; L13 Cut; L25 Cut | L2 Cut; L27 Cut |
| FireRed and LeafGreen | Voltorb | L14 Flash; L26 Flash; L37 Flash | L14 Flash; L32 Flash; L49 Flash |
| FireRed and LeafGreen | Pikachu | L3 Flash; L13 Flash; L26 Flash; L39 Flash; L50 Flash | L3 Flash; L15 Flash; L41 Flash |
| FireRed and LeafGreen | Horsea | L5 Surf, Waterfall; L17 Surf, Waterfall; L31 Surf, Waterfall; L46 Surf, Waterfall | L5 Surf, Waterfall; L22 Surf, Waterfall; L43 Surf, Waterfall |
| FireRed and LeafGreen | Krabby | L5 Surf; L19 Surf; L31 Surf; L45 Surf | L5 Surf; L27 Surf |
| FireRed and LeafGreen | Machop | L16 Strength; L27 Strength; L39 Strength | L16 Strength; L31 Strength; L49 Strength |
| FireRed and LeafGreen | Geodude | L7 Strength, Rock Smash; L16 Strength, Rock Smash; L24 Strength, Rock Smash; L34 Strength, Rock Smash; L42 Strength, Rock Smash | L7 Strength, Rock Smash; L21 Strength, Rock Smash; L36 Strength, Rock Smash |
| FireRed and LeafGreen | Mankey | L2 Rock Smash; L15 Rock Smash; L29 Rock Smash; L43 Rock Smash | L2 Rock Smash; L27 Rock Smash; L51 Rock Smash |
| FireRed and LeafGreen | Goldeen | L5 Waterfall; L21 Waterfall; L40 Waterfall | L5 Waterfall; L29 Waterfall |
| HNS | Gligar | L19 Cut; L35 Cut; L55 Cut | L19 Cut; L44 Cut |
| HNS | Aipom | L10 Cut, Rock Smash; L18 Cut, Rock Smash; L29 Cut, Rock Smash; L39 Cut, Rock Smash | L10 Cut, Rock Smash; L25 Cut, Rock Smash; L38 Cut, Rock Smash |
| HNS | Chinchou | L9 Flash, Surf, Whirlpool; L17 Flash, Surf, Whirlpool; L23 Flash, Surf, Whirlpool; L31 Flash, Surf, Whirlpool; L39 Flash, Surf, Whirlpool; L45 Flash, Surf, Whirlpool; L50 Flash, Surf, Whirlpool | L9 Flash, Surf, Whirlpool; L17 Flash, Surf, Whirlpool; L29 Flash, Surf, Whirlpool; L41 Flash, Surf, Whirlpool |
| HNS | Mareep | L5 Flash; L18 Flash; L32 Flash; L46 Flash | L5 Flash; L30 Flash |
| HNS | Wooper | L4 Surf, Waterfall; L15 Surf, Waterfall; L29 Surf, Waterfall; L43 Surf, Waterfall | L4 Surf, Waterfall; L21 Surf, Waterfall; L41 Surf, Waterfall |
| HNS | Snubbull | L13 Strength; L37 Strength | L13 Strength; L43 Strength |
| HNS | Miltank | L21 Strength, Rock Smash; L35 Strength, Rock Smash; L50 Strength, Rock Smash | L21 Strength, Rock Smash; L43 Strength, Rock Smash |
| HNS | Marill | L8 Waterfall; L16 Waterfall; L31 Waterfall | L8 Waterfall; L28 Waterfall |
| HNS | Mantine | L15 Whirlpool; L27 Whirlpool; L46 Whirlpool | L15 Whirlpool; L43 Whirlpool |
| Emerald | Corphish | L10 Cut, Rock Smash; L20 Cut, Rock Smash; L31 Cut, Rock Smash; L39 Cut, Rock Smash | L10 Cut, Rock Smash; L23 Cut, Rock Smash; L35 Cut, Rock Smash |
| Emerald | Sableye | L9 Cut, Flash; L16 Cut, Flash; L24 Cut, Flash; L31 Cut, Flash; L39 Cut, Flash; L46 Cut, Flash | L9 Cut, Flash; L21 Cut, Flash; L33 Cut, Flash; L45 Cut, Flash |
| Emerald | Electrike | L12 Flash; L24 Flash; L44 Flash | L12 Flash; L28 Flash |
| Emerald | Lotad | L3 Surf; L15 Surf; L27 Surf | L3 Surf; L31 Surf |
| Emerald | Wailmer | L10 Surf, Dive; L19 Surf, Dive; L29 Surf, Dive; L45 Surf, Dive | L10 Surf, Dive; L23 Surf, Dive; L37 Surf, Dive; L50 Surf, Dive |
| Emerald | Makuhita | L6 Strength; L16 Strength; L28 Strength; L40 Strength | L6 Strength; L22 Strength; L40 Strength |
| Emerald | Torkoal | L14 Strength; L25 Strength; L38 Strength; L47 Strength | L14 Strength; L30 Strength; L46 Strength |
| Emerald | Aron | L7 Rock Smash; L19 Rock Smash; L31 Rock Smash; L43 Rock Smash | L7 Rock Smash; L21 Rock Smash; L39 Rock Smash |
| Emerald | Barboach | L10 Waterfall; L20 Waterfall; L32 Waterfall | L10 Waterfall; L26 Waterfall |
| Emerald | Carvanha | L10 Waterfall, Dive; L18 Waterfall, Dive; L29 Waterfall, Dive; L39 Waterfall, Dive | L10 Waterfall, Dive; L22 Waterfall, Dive; L37 Waterfall, Dive |

These additions keep every table below `MAX_LEVEL_UP_MOVES` and `MAX_RELEARNER_MOVES`. The largest resulting anchor table is the normal-mode Sableye table with 32 learned entries.

### Evolution and Move Reminder behavior

Ensure that each assigned utility move appears at level 1 for every ordinary forward successor listed below in both learnset sources. Apply each family under the same build condition as its anchor schedule. Add an entry only where that mode does not already have the move at level 1. Place additions after the successor's existing level 1 entries, in the move order shown. One level 1 entry per assigned move is sufficient because successors do not need to generate as wild native users; the entry makes the move available to the Move Reminder while normal evolution preserves a move that is already known. Normal-mode Machamp already has Strength at level 1 and needs no duplicate entry; its legacy table still needs the addition.

| Anchor family | Successors receiving the assigned moves |
| --- | --- |
| Paras | Parasect: Cut |
| Rattata | Raticate: Cut |
| Voltorb | Electrode: Flash |
| Pikachu | Raichu and Alolan Raichu: Flash |
| Horsea | Seadra and Kingdra: Surf, Waterfall |
| Krabby | Kingler: Surf |
| Machop | Machoke and Machamp: Strength |
| Geodude | Graveler and Golem: Strength, Rock Smash |
| Mankey | Primeape and Annihilape: Rock Smash |
| Goldeen | Seaking: Waterfall |
| Gligar | Gliscor: Cut |
| Aipom | Ambipom: Cut, Rock Smash |
| Chinchou | Lanturn: Flash, Surf, Whirlpool |
| Mareep | Flaaffy and Ampharos: Flash |
| Wooper | Quagsire: Surf, Waterfall |
| Snubbull | Granbull: Strength |
| Marill | Azumarill: Waterfall |
| Corphish | Crawdaunt: Cut, Rock Smash |
| Electrike | Manectric: Flash |
| Lotad | Lombre and Ludicolo: Surf |
| Wailmer | Wailord: Surf, Dive |
| Makuhita | Hariyama: Strength |
| Aron | Lairon and Aggron: Rock Smash |
| Barboach | Whiscash: Waterfall |
| Carvanha | Sharpedo: Waterfall, Dive |

Miltank, Mantine, Sableye, and Torkoal have no ordinary forward successor and need no additional species entry. Pre-evolutions do not inherit the role: Pichu, Azurill, and Mantyke receive no addition. Distinct regional base forms, including Alolan Rattata, Hisuian Voltorb, Alolan Geodude, and Paldean Wooper, also receive no addition. Mega Evolutions and temporary battle forms need no separate learnset entry because the known move persists through the form change.

### HM compatibility and unchanged move behavior

Every anchor must retain compatibility with each assigned HM in `all_learnables.json` and in the generated runtime teachable data. This feature does not broaden compatibility. Successor compatibility remains as authored: in particular, Alolan Raichu is not made compatible with Flash and Annihilape is not made compatible with Rock Smash. A known move is sufficient for field use, so their level 1 Move Reminder entries still preserve the evolved utility role.

The utility move remains an ordinary move. Its type, power, accuracy, PP, battle effect, relearning behavior, replacement behavior, and field PP rules do not change. Forgetting or replacing the move removes known-move field access until the Pokémon learns or recalls it again.

The feature adds no persistent state and requires no save migration. Existing Pokémon do not gain a move retroactively. Existing saves use the additions when a new Pokémon is generated, levels into a listed entry, or recalls the move.

### Validation

Add deterministic learnset coverage around the existing Pokémon learnset tests. For every anchor in its applicable build, run both normal and legacy-moves selection and construct its initial moveset at every integer level from the lowest qualifying level in the coverage inventory through level 100. At every level, assert that all assigned utility moves are among the four known moves. The test must exercise the production initial-moveset path rather than a separately reimplemented last-four calculation.

Add data validation in each applicable standalone build that enumerates the
named qualifying encounter profiles, all applicable version and time-of-day
variants, every authored level in each selected slot, and Trainer Ratings 10
through 80. Wayfarer extends this validation through Rating 0 in its
interregional circuit specification. Project each level through the production
scaling function, construct the caught anchor's moveset, and assert that all
assigned utility moves remain present. Also assert that the profile still
contains the intended anchor, so an encounter edit cannot silently invalidate
coverage.

Expose the production Move Reminder result to tests through a test-only helper or a public read-only list function, or drive the Move Reminder UI and inspect the offered move IDs. For each listed successor in its applicable build and both learnset modes, assert that every assigned utility move is offered when it is not currently known. `CanBoxMonRelearnMoves` alone is insufficient because it proves only that some move is relearnable. Confirm separately that evolving an anchor which knows its utility moves preserves them. Assert that Pichu, Azurill, Mantyke, and the excluded regional base forms do not gain the role.

Keep the existing all-species learnset limit test passing. Validate `all_learnables.json` and the generated teachable learnsets for every anchor and assigned move. Assert that this feature does not change any species' HM compatibility, including the existing Alolan Raichu and Annihilape exceptions. The highest resulting table count must remain below both configured limits.

For each build and learnset mode, assert that only its regional additions are present. In particular, HNS must not receive the Wailmer, Wailord, Carvanha, or Sharpedo Dive additions; FireRed and LeafGreen must not receive Dive or Whirlpool additions; and Emerald must not receive Johto Whirlpool additions. Existing moves outside this feature remain unchanged.

The field-use behavior is already covered by its own resolver tests. Integration playtesting for this feature must catch at least one anchor for each active regional HM with the HM item absent and badges unset, then use that caught Pokémon at a valid field interaction. Multi-role catches must show every assigned move. HNS testing covers Whirlpool and explicitly excludes Dive. Include FireRed and LeafGreen Pallet-to-Cinnabar Surf coverage in both directions, HNS Olivine-to-Cianwood and Kanto-mainland-to-Cinnabar Surf coverage in both directions, and Emerald Route 118 plus Lilycove-to-Mossdeep or Pacifidlog Surf coverage. Fishing checks assume the Standard Rod feature and available capture supplies.

Compile the affected learnset and Pokémon objects for Emerald, FireRed, LeafGreen, and HNS. Run deterministic tests in both learnset modes and with the learnset randomizer disabled.

## References

- [HM field-use specification](hm-field-use.md)
- [Normal level-up learnsets](../../game/src/data/pokemon/level_up_learnsets/gen_7.h)
- [Generation III legacy learnsets](../../game/src/data/pokemon/level_up_learnsets/gen_3.h)
- [Legacy learnset species table](../../game/src/data/pokemon/level_up_learnsets_gen3.c)
- [Initial moveset and learnset selection](../../game/src/pokemon.c)
- [HM compatibility data](../../game/src/data/pokemon/all_learnables.json)
- [Wild encounter data](../../game/src/data/wild_encounters.json)
- [Wild encounter scaling](../../game/src/data/wild_encounter_scaling.json)
- [Pokémon learnset tests](../../game/test/pokemon.c)

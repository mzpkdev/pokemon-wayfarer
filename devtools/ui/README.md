# Pokémon Wayfarer browser tools

The Svelte app provides Cartographer, Metatiles, Trainer balance, and a formatted
viewer for the Markdown files in `.product/`. Cartographer reads the checkout's generated catalog
and image assets, then places default-visible exterior maps from their cardinal
source connections.

Run these commands from `devtools/`:

```sh
pnpm run dev
pnpm run e2e
```

`pnpm run dev` generates the Cartographer and metatile catalogs before starting
the app. Use `pnpm run catalog` when you need to refresh the generated data
without starting the development server.

`wa build` writes a compact client bundle to `ui/dist`. `pnpm run build` from
the parent `devtools/` directory stages that bundle alongside the generated
catalog in `build/cartographer/map-catalog/`, which is the standalone
static-host deployment artifact. `ui/dist` remains a bundle only.

Cartographer defaults to the Wayfarer build. Choose a build above the regions to
browse its maps; region counts, search, and warp navigation use that selection.
Build membership comes from each map's `game_version`, so Wayfarer includes both
HNS, Emerald, and developer-visible Sinnoh maps. Sinnoh remains unreachable in
gameplay. The URL preserves the build, selected map, and camera state.
Cartographer also provides native and overview image switching and map facts.
Its generated input is ignored under
`build/cartographer/map-catalog/`.

Docs groups files by their folder below `.product/`, takes each page title from its
first level-one heading, and keeps the selected page and heading in the URL. Files
named with the `__NAME__.md` convention are treated as authoring templates and do
not appear in navigation. Vite bundles the Markdown at startup and build time.
Restart the development server after adding or renaming a document.

`src/App.svelte` owns the page shell and module navigation. Each module lives under
`src/modules/`. Cartographer and its styled interface primitives live in
`src/modules/cartographer/`, including `ui-toolkit/`. The map search combobox and exits
checkbox wrap Ark UI; compose these local controls to keep the cartographer's visual and
accessibility contracts consistent.

The root `pnpm run e2e` command generates both catalogs before running the
browser test.

## Trainer balance explorer

Open `#trainer-balance` to author the 38 notable trainer entries (37
characters plus the Tate & Liza duo) under the TR v0 model. The module bundles its trainer catalog, so it works without generating
the map catalogs:

```sh
pnpm --filter @wayfarer/ui dev
```

Each notable trainer's **TR** is a non-negative integer with no upper limit
that grows with **world progress** (the player TR as the world sees it). It
is a pure function of world progress, and it is the only input to the
trainer's team. The player TR is never computed from a notable trainer's TR.

- **Growth**: each trainer has a **start TR** (their TR at world progress 0),
  an **archetype** and a **peak TR** (the most they can ever reach). For
  every archetype, trainer TR = start TR + roundHalfUp((peak TR − start TR) ×
  growth % / 100), where the growth % is that archetype's scaler over world
  progress (read exactly, flat past the last anchor). Defaults at world
  progress 0 / 40 / 80 / 120 / 160: steady 0 / 25 / 50 / 75 / 100, early
  bloomer 0 / 50 / 80 / 95 / 100, late bloomer 0 / 10 / 25 / 55 / 100, plateau
  0 / 60 / 100 / 100 / 100. The **rival** is an ordinary archetype with an
  extra anchor: 0 / 20 / 40 / 80 / 120 / 160 → 0 / 15 / 29 / 53 / 76 / 100%.
  Blue (rival, start TR 0, peak TR 170) is TR 0 at Pallet (one Eevee at
  Lv 5), TR 26 at world progress 20 (two Pokémon, team level 18), TR 49 at
  40, then 9–10 ahead of the player until he reaches 170. Peak TR must be at
  least start TR.

- **Scalers** turn TR into values. Each is an editable table of anchors
  (TR, value), linear between anchors with halves rounded up, and flat past
  the last anchor, whose TR is the scaler's ceiling TR. TR itself is never
  clamped.
- **Team level** has its own low end, then the level cap anchors from TR 40:
  (0, 5) (20, 14) (40, 28) (80, 50) (120, 75) (160, 100). TR 200 still gives
  Lv 100. The level cap itself is unchanged.
- **Team size** is a step table built from paired anchors: TR 0–10 → 1,
  11–28 → 2, 29–43 → 3, 44–70 → 4, 71–95 → 5, 96+ → 6.
- **Roster**: one ordered list of six roster slots per trainer. Each roster
  slot has a species (normally a final stage, such as Steelix), a level
  offset (-6 to 0), moves (`LEVEL_UP` or one to four authored moves), a held
  item, and an optional ability and nature. Roster slot 1 (the signature
  Pokémon) must be at offset 0.
- **Team** = the first N roster slots (N = team size). Each member's level is
  clamp(team level + offset, 1, 100). **Battle order** is the team reversed,
  so roster slot 1 is fought last.
- **Evolution (downward only)**: a member whose level is below its stage's
  evolution level steps down its predecessor chain until the level supports
  the stage. It never evolves forward. Level evolutions use their
  `species_info` threshold; non-level evolutions (item, trade, friendship,
  other) use the shared evolution-level table in the catalog script, which
  covers only evolutions without a level in the game data. Baby pre-evolutions
  are not stepped down to. Brock at start TR 20 (team level 14) fields Onix
  Lv 14 and Geodude Lv 12; his Steelix appears from Lv 35 and Golem from Lv 38. A member at its
  authored stage uses the authored moves; one that stepped down uses
  `LEVEL_UP`.

The world panel sets the player TR: a number field and a 0–200 slider (type
any larger whole TR; TR has no upper limit). Badges (0–24) are presets that
set the player TR on the rescaled formula (badges 1–8 give +10 each, badges
9–24 +5 each, league wins give nothing, so 24 badges = TR 160). A typed TR
shows the badge count it matches, "between N and N+1 badges" or "beyond 24
badges". The panel shows the level cap the player TR sets, and world
scaling: the wild level curve (0, 6) (40, 24) (80, 40) (120, 58) (160, 78)
and the regular trainer level curve (0, 9) (40, 27) (80, 44) (120, 62)
(160, 82), each with its gap to the level cap, and world progress (equal to
the player TR). The trainer list shows each trainer's TR at the current world
progress, their start TR → peak TR, archetype,
team level, team size, the gap between team level and the level cap, and how
many roster slots are filled. The trainer panel edits start TR, archetype
and peak TR, shows the trainer's TR, team level and each roster slot's stage
and level at world progress 0 / 40 / 80 / 120 / 160, the trainer's
**milestones**, a chart of their team level against the level cap across
player TR 0–200 (the current player TR and the milestones marked; hover or
focus it and use the arrow keys to read values), the team at the current
TR in battle order (a stepped-down member shows its authored stage, e.g.
"Onix → Steelix at Lv 35"), and a roster editor: reorder roster slots, edit
species, offset, moves and item, and add or remove roster slots. Each roster
slot shows its line with evolution levels and warns when the species is not a
final stage or has no evolution data in the catalog. **Edit settings as JSON** covers ability and
nature too.

**Milestones** list every player TR (world progress) where the trainer's
team changes, scanning each whole world progress from 0 to the later of the
archetype growth ceiling and the level cap ceiling (TR 160): the starting
team, a roster slot joining (team size steps up), a member's stage changing
along its evolution line, the team level moving strictly above the level cap
or strictly below it again (equal keeps the side, so rounding cannot flicker),
and peak TR reached. The player's current TR is marked in the list. Brock
reads "0: Onix, Geodude · 19: 3rd slot (Aerodactyl) joins · 38: Geodude →
Graveler · 51: 4th slot (Kabuto) joins · 68: Onix → Steelix · 87: Graveler →
Golem · 95: Kabuto → Kabutops · 108: 5th slot (Omastar) joins · 159: peak TR
95".

Exports and the saved browser state keep version 8 and store the point as
`{ "playerTR": n }`. Earlier version 8 files with `{ "badges": n }` still
import and set the matching player TR.

**Tate & Liza** are one entry (role Gym Leader duo, Hoenn) with one start TR,
archetype, peak TR and six-slot roster (Solrock and Lunatone as the signature
pair, both at offset 0). They are fought as a double battle: both leaders send
Pokémon from the shared roster in order, with team size from the same table.
The explorer marks them as a double battle. They follow every notable trainer
rule but are league-ineligible (`leagueEligible: false`; leagues are singles
only).

**Gym ladder** lists the 24 Gym Leader entries (Tate & Liza included) at the
current world progress, sorted by TR, each marked below, near (within 10 of
the player TR) or above.

**League lineup** previews the top five trainers by TR at the current world
progress from one global pool of the 37 league-eligible entries (the catalog
has no Red, and Tate & Liza are ineligible), as entering a league would
compute it. Ties keep catalog order. The
five are shown as matches 1–5, ascending TR with the strongest last, each with
their own team and levels. v0 uses one pool, so Indigo, Sevii Masters and Hoenn
all use the same lineup. With the catalog defaults, at world progress 80
(8 badges, level cap Lv 50) the lineup is Giovanni (TR 93), Bruno, Will,
Norman (TR 94) and Agatha (TR 95), team level 58–59. At 120 (level cap Lv 75)
it is Steven (TR 130), Norman, Giovanni, Lance and Jasmine (TR 131–132), team
level 81–83. At 160 it is Clair, Juan, Wallace, Steven and Lance (TR
185–200), team level 100: the team level scaler stops at Lv 100, the same as
the level cap there, so the lineup can match the cap but not exceed it.

All nine scaler tables (team level, team size, wild level, regular trainer
level, and the steady, early bloomer, late bloomer, plateau and rival growth
scalers) are editable under **Scalers & experiment settings**. Anchors start at 0,
rise and never decrease in value; growth scalers run 0–100% and start at 0%.
Experiments persist in browser storage. JSON export and import (format
version 8) round-trip the experiment (growth, rosters and the nine
scalers), the player badges and the selected trainer. Files from versions 1–7
are rejected with a message (version 7 gave the rival a fixed lead, copied the
level cap into team level and had no Tate & Liza; version 6 gave each notable
trainer one fixed TR; version 5 used the retired 0–80 player TR scale).
There is no migration. Reset restores the catalog defaults. **Restore this
trainer’s defaults** updates only the selected trainer.

The tool models species, team size and levels. It does not simulate moves,
items, abilities, stats, AI, matchup difficulty or battle outcomes. Seeded
archetypes, other sources of world progress and lineup rules beyond the top
five are out of scope for v0. The
scaler the ROM uses today is unchanged.

The catalog's growth and rosters are **placeholders**, for the authoring
session to replace. The growth defaults live in the catalog script's `GROWTH`
table, chosen by lore (veterans such as Bruno, Agatha, Lorelei, Pryce and
Chuck plateau; rising stars such as Whitney and Falkner bloom early; Clair,
Winona, Juan and Sabrina bloom late; most others are steady; Champions and
Lance have the highest peaks). The region openers start high enough to stay
classic-like on the team level low end: Brock and Roxanne at start TR 20 (two
Pokémon at Lv 14 and 12), Falkner at 14 (Lv 11 and 9). The defaults are tuned
to the v0 balance targets, which `engine.test.ts` checks: the lineup at TR
85–95 at world progress 80, 2–8 levels above the level cap at 120, Lv 100 at
160, at least three Gym Leaders below, near and above the player at every
checkpoint (at world progress 0 the openers are two Pokémon at or under the
level cap), and Blue about 10 ahead from world progress 40. Rosters flatten
the earlier ace/filler prototype: its ace (now the signature Pokémon) first,
then the other members in order, cut to six, then each species converted to
the final stage of its line (a converted slot uses `LEVEL_UP` with no
ability; branching lines take the script's `FINAL_CHOICE`, e.g. Scyther →
Scizor). Blue's roster slot 1 stays Eevee (a user choice, flagged as not a
final stage). Brock's roster is Steelix (signature Pokémon), Golem (keeping
its source battle content), Aerodactyl, Kabutops, Omastar and Relicanth. That prototype came
from each trainer's competitive party (the curated six in
`game/src/data/trainer_scaling/gym_leaders.json`) or otherwise its reference
party (the highest-level member is roster slot 1; other offsets are the source level
gap clamped to -6..0). Handwritten early species fill in where no line covers
them. Roster slot 1 keeps its source moves, item, ability and nature; the rest
use `LEVEL_UP` as a label (learnsets are not resolved). Eleven rosters have
only five roster slots (Lorelei, Bruno, Agatha, Koga, Lance, Will, Karen, Sidney, Phoebe,
Glacia, Drake). The explorer flags them as incomplete, and the catalog script
prints them as a warning, not a failure. Tate & Liza's roster is Solrock, Lunatone,
Claydol and Xatu from their Emerald party, then placeholder Hoenn Psychic
picks Gardevoir and Grumpig from other Emerald parties.

The catalog uses local FRLG, Emerald and HNS source records, including their
provenance and explicit variant notes. HNS is not substituted with HGSS;
Steven's local Emerald postgame party is labeled as such. The catalog script
validates growth (start and peak TR, archetype), roster length (at most six), offsets, moves and roster slot 1
at offset 0, and that the catalog has exactly 38 entries with only the Tate &
Liza duo fought as a double battle. It also validates the shared
evolution-level table (one level per edge, each a real `species_info`
evolution without an `EVO_LEVEL`; a row for a level evolution fails, since the
game's level wins), requires a table entry for every non-level edge on a roster line,
and checks that levels increase along each line with no ambiguous ancestry or
cycles. Table rows are `authored` (the contract examples: Onix → Steelix 35,
Staryu → Starmie 30, Growlithe → Arcanine 35) or `placeholder`;
the script prints the placeholders, the conversions and a warning for roster
slots that are not final stages. The catalog records each roster species'
chain compactly under `evolution.chains` (e.g. `["Geodude", 25, "Graveler",
38, "Golem"]`), which the explorer indexes; the saved experiment format is
unchanged. Regenerate or verify the checked-in catalog from the repository root
(requires Python 3, `cc`, `cpp`):

```sh
python3 devtools/scripts/trainer-balance-catalog.py
pnpm --filter @wayfarer/ui exec wa format src/modules/trainer-balance/catalog.json
python3 devtools/scripts/trainer-balance-catalog.py --check
```

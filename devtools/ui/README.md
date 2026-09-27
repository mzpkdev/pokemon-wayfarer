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

Open `#trainer-balance` to author the 37 notable trainers under the TR v0
model. The module bundles its trainer catalog, so it works without generating
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
  steady, early bloomer, late bloomer and plateau, trainer TR = start TR +
  roundHalfUp((peak TR − start TR) × growth % / 100), where the growth % is
  that archetype's scaler over world progress (read exactly, flat past the
  last anchor). Defaults at world progress 0 / 40 / 80 / 120 / 160: steady
  0 / 25 / 50 / 75 / 100, early bloomer 0 / 50 / 80 / 95 / 100, late bloomer
  0 / 10 / 25 / 55 / 100, plateau 0 / 60 / 100 / 100 / 100. The **rival**
  (Blue only) instead has a **lead**: trainer TR = min(peak TR, world
  progress + lead). Peak TR must be at least start TR, and only the rival has
  a lead.

- **Scalers** turn TR into values. Each is an editable table of anchors
  (TR, value), linear between anchors with halves rounded up, and flat past
  the last anchor, whose TR is the scaler's ceiling TR. TR itself is never
  clamped.
- **Team level** uses the level cap anchors: (0, 15) (40, 28) (80, 50)
  (120, 75) (160, 100). TR 200 still gives Lv 100.
- **Team size** is a step table built from paired anchors: TR 0–15 → 2,
  16–43 → 3, 44–70 → 4, 71–95 → 5, 96+ → 6.
- **Roster**: one ordered list of six roster slots per trainer. Each roster
  slot has a species, a level offset (-6 to 0), moves (`LEVEL_UP` or one to
  four authored moves), a held item, and an optional ability and nature.
  Roster slot 1 (the signature Pokémon) must be at offset 0.
- **Team** = the first N roster slots (N = team size). Each member's level is
  clamp(team level + offset, 1, 100). **Battle order** is the team reversed,
  so roster slot 1 is fought last.

The world panel sets player badges (0–24). It shows the player TR on the
rescaled formula (badges 1–8 give +10 each, badges 9–24 +5 each, league wins
give nothing, so 24 badges = TR 160), the level cap it sets, and world
scaling: the wild level curve (0, 6) (40, 24) (80, 40) (120, 58) (160, 78)
and the regular trainer level curve (0, 9) (40, 27) (80, 44) (120, 62)
(160, 82), each with its gap to the level cap, and world progress (equal to
the player TR). The trainer list shows each trainer's TR at the current world
progress, their start TR → peak TR (lead → peak TR for the rival), archetype,
team level, team size, the gap between team level and the level cap, and how
many roster slots are filled. The trainer panel edits start TR, archetype,
peak TR and lead, shows the trainer's TR and team level at world progress
0 / 40 / 80 / 120 / 160, the team at the current TR in battle order, and a
roster editor: reorder roster slots, edit species, offset, moves and item, and
add or remove roster slots. **Edit settings as JSON** covers ability and
nature too.

**Gym ladder** lists the 23 Gym Leaders at the current world progress,
sorted by TR, each marked below, near (within 10 of the player TR) or above.

**League lineup** previews the top five trainers by TR at the current world
progress from one global pool of all 37 (the catalog has no Red or Tate &
Liza), as entering a league would compute it. Ties keep catalog order. The
five are shown as matches 1–5, ascending TR with the strongest last, each with
their own team and levels. v0 uses one pool, so Indigo, Sevii Masters and Hoenn
all use the same lineup. With the catalog defaults, at world progress 80
(8 badges, level cap Lv 50) the lineup is Giovanni (TR 93), Bruno, Will,
Norman (TR 94) and Agatha (TR 95), team level 58–59. At 120 (level cap Lv 75)
it is Blue (TR 130), Norman, Giovanni, Lance and Jasmine (TR 131–132), team
level 81–83. At 160 it is Clair, Juan, Wallace, Steven and Lance (TR
185–200), team level 100: the team level scaler stops at Lv 100, the same as
the level cap there, so the lineup can match the cap but not exceed it.

All eight scaler tables (team level, team size, wild level, regular trainer
level, and the steady, early bloomer, late bloomer and plateau growth
scalers) are editable under **Scalers & experiment settings**. Anchors start at 0,
rise and never decrease in value; growth scalers run 0–100% and start at 0%.
Experiments persist in browser storage. JSON export and import (format
version 7) round-trip the experiment (growth, rosters and the eight
scalers), the player badges and the selected trainer. Files from versions 1–6
are rejected with a message (version 6 gave each notable trainer one fixed TR;
version 5 used the retired 0–80 player TR scale).
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
Lance have the highest peaks) and tuned to the v0 balance targets, which
`engine.test.ts` checks: the lineup at TR 85–95 at world progress 80, 2–8
levels above the level cap at 120, at least three Gym Leaders below, near
and above the player at every checkpoint (openers such as Brock at TR 1–2 at
world progress 0), and Blue 10 ahead. Rosters flatten
the earlier ace/filler prototype: its ace (now the signature Pokémon) first,
then the other members in order, each at the end of its species line and cut to six. That prototype came
from each trainer's competitive party (the curated six in
`game/src/data/trainer_scaling/gym_leaders.json`) or otherwise its reference
party (the highest-level member is roster slot 1; other offsets are the source level
gap clamped to -6..0). Handwritten early species fill in where no line covers
them. Roster slot 1 keeps its source moves, item, ability and nature; the rest
use `LEVEL_UP` as a label (learnsets are not resolved). Eleven rosters have
only five roster slots (Lorelei, Bruno, Agatha, Koga, Lance, Will, Karen, Sidney, Phoebe,
Glacia, Drake). The explorer flags them as incomplete, and the catalog script
prints them as a warning, not a failure.

The catalog uses local FRLG, Emerald and HNS source records, including their
provenance and explicit variant notes. HNS is not substituted with HGSS;
Steven's local Emerald postgame party is labeled as such. The catalog script
validates growth (start and peak TR, archetype, the rival's lead), roster length (at most six), offsets, moves and roster slot 1
at offset 0. Regenerate or verify the checked-in catalog from the repository root
(requires Python 3, `cc`, `cpp`):

```sh
python3 devtools/scripts/trainer-balance-catalog.py
pnpm --filter @wayfarer/ui exec wa format src/modules/trainer-balance/catalog.json
python3 devtools/scripts/trainer-balance-catalog.py --check
```

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

Open `#trainer-balance` to author the 37 well-known trainers under the TR v0
model. The module bundles its trainer catalog, so it works without generating
the map catalogs:

```sh
pnpm --filter @wayfarer/ui dev
```

Each well-known trainer has an authored, fixed **TR**: a non-negative integer
with no upper limit. A trainer's TR is the only input to their team. The
player's TR never feeds trainer results.

- **Scalers** turn TR into values. Each is an editable table of anchors
  (TR, value), linear between anchors with halves rounded up, and flat past
  the last anchor. TR itself is never clamped.
- **Team level** uses the player soft-cap anchors: (0, 15) (4, 16) (8, 18)
  (16, 23) (30, 30) (40, 42) (55, 60) (65, 80) (80, 100). TR 120 gives Lv 100.
- **Team size** is a step table built from paired anchors: TR 0–10 → 2,
  11–29 → 3, 30–41 → 4, 42–54 → 5, 55+ → 6.
- **Roster**: one ordered list of six entries per trainer. Each entry has a
  species, a level offset (-6 to 0), moves (`LEVEL_UP` or one to four authored
  moves), a held item, and an optional ability and nature. Entry 1 must be at
  offset 0.
- **Team** = the first N entries (N = team size). Each member's level is
  clamp(team level + offset, 1, 100). **Battle order** is the team reversed,
  so entry 1 is fought last.

The world panel sets player badges (0–24) and first league clears (0–3). It
shows the player TR (unchanged formula) and the player cap for comparison
only. The trainer list shows each trainer's TR (editable in place), team
level, team size, the gap between team level and the player cap, and how many
roster entries exist. The trainer panel shows the team at the current TR in
battle order and a roster editor: reorder entries, edit species, offset, moves
and item, and add or remove entries. **Edit settings as JSON** covers ability
and nature too.

**League field** previews the top five trainers by TR from one global pool of
all 37 (the catalog has no Red or Tate & Liza). Ties keep catalog order. The
five are shown in battle order, ascending TR with the strongest last, each with
their own team and levels. v0 uses one pool, so Indigo, Sevii Masters and Hoenn
all field the same five. With the catalog defaults they are Bruno (52), Agatha
(53), Wallace (53), Steven (53) and Lance (55).

Both scaler tables are editable under **Scalers & experiment settings**.
Anchors start at TR 0, rise in TR and never decrease in value. Experiments
persist in browser storage. JSON export and import (format version 5)
round-trip the experiment (TRs, rosters and both scalers), the player point and
the selected trainer. Files from versions 1–4 are rejected with a message.
There is no migration. Reset restores the catalog defaults. **Restore this
trainer’s defaults** updates only the selected trainer.

The tool models species, team size and levels. It does not simulate moves,
items, abilities, stats, AI, matchup difficulty or battle outcomes. How TR
changes, seeded variation and league signup are out of scope for v0. The
current ROM scaler is unchanged.

The catalog's TRs and rosters are **provisional**, for the roster authoring
session to replace. TRs are the start ratings from the first explorer catalog
(Gym Leaders 1–5, Blue 6, Elite Four 40–53, Champions 53–55). Rosters flatten
the earlier ace/filler prototype: the ace first, then the other members in
order, each at the end of its species line and cut to six. That prototype came
from each trainer's competitive party (the curated six in
`game/src/data/trainer_scaling/gym_leaders.json`) or otherwise its reference
party (the highest-level member is entry 1; other offsets are the source level
gap clamped to -6..0). Handwritten early species fill in where no line covers
them. Entry 1 keeps its source moves, item, ability and nature; the rest use
`LEVEL_UP` as a label (learnsets are not resolved). Eleven rosters have only
five entries (Lorelei, Bruno, Agatha, Koga, Lance, Will, Karen, Sidney, Phoebe,
Glacia, Drake). The explorer flags them as incomplete, and the catalog script
prints them as a warning, not a failure.

The catalog uses local FRLG, Emerald and HNS source records, including their
provenance and explicit variant notes. HNS is not substituted with HGSS;
Steven's local Emerald postgame party is labeled as such. The catalog script
validates TRs, roster length (at most six), offsets, moves and entry 1 at
offset 0. Regenerate or verify the checked-in catalog from the repository root
(requires Python 3, `cc`, `cpp`):

```sh
python3 devtools/scripts/trainer-balance-catalog.py
pnpm --filter @wayfarer/ui exec wa format src/modules/trainer-balance/catalog.json
python3 devtools/scripts/trainer-balance-catalog.py --check
```

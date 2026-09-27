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

Open `#trainer-balance` to inspect the experimental 37-trainer pool at any
badge count from 0 to 24, zero to three first league clears and, at 24 badges
with three clears, zero to three or more completed circuit editions. The module
bundles its trainer catalog, so it works without generating the map catalogs:

```sh
pnpm --filter @wayfarer/ui dev
```

The explorer models the Living Rivals design. There is no NPC trainer rating.
Each trainer has an integer **standing** relative to the world cap:

- `p = B + 8 × min(completedEditions, 3)` is the progress index, where `B` is
  global badges. A completed edition (all three venues won) implies 24 badges
  and three first clears, so the **Completed editions** input is only enabled
  at that point and resets to 0 when you leave it. p runs 0–24 in edition 1,
  and editions 2, 3 and 4+ enter at 32, 40 and 48, after which standings stop
  changing.
- `standing = bias + arcDelta(arc, p)`.
- `levelBase = min(worldCap(B, C), 100 - headroom)`, with a default headroom
  of 4 (editable in the global settings).
- `strengthLevel = clamp(levelBase + standing, 1, 100)`. The top ace sits at
  the strength level, fillers add their level offset (-6 to 0), and team size
  follows the strength level (see rosters below).
- `worldCap(B, C)` is the player soft cap from the unchanged player TR formula
  and cap curve, using only badges and first league clears (`C`). Clears affect
  levels only through the world cap.

The headroom changes nothing while the cap is at most 96, which covers every
edition-1 point that the signup rules make reachable. At 24 badges and three
clears the cap is 100 and the base is 96, so contenders stay at 94 or below,
elites at 95–97 and headliners at 98–100. Roles still come from standing, so
headroom never changes a role. Once the base stops rising, a declining arc
lowers levels between editions (early and late drop by one), and the selected
trainer panel reports it.

Growth arcs are `arcDelta` tuples at p = 0 / 8 / 16 / 24 / 32 / 40 / 48,
interpolated linearly and rounded half up: steady all 0, early
0/+3/+2/+1/0/0/0, late 0/-2/0/+3/+3/+2/+2, plateau 0/+1/-1/-3/-3/-2/-1 and
rival 0/+2/+3/+3/+3/+3/+3. Every arc is 0 at p = 0, so a trainer's first
encounter is identical under every arc. The catalog proposes a `bias` (Gym -2;
strong leaders Giovanni, Sabrina, Morty, Clair, Norman, Winona and Juan -1, so
every region has a leader who can headline in some saves; Elite Four 0,
Champions +2, Blue +1) and two or three lore-fitting
`allowedArcs` per trainer, stored in canonical arc order (steady, early, late,
plateau, rival). Blue only gets fast arcs (early, rival). These defaults are
content under review, not approved balance.

The pool table shows each trainer's arc (a selector limited to their allowed
arcs), standing, strength level, a signed **strength − cap** gap, role and team size.
Roles come from standing windows: contender at most -2, elite -1 to +1,
headliner at least +2. The explorer shows a preview arc per trainer (steady
when allowed, otherwise the first allowed arc). It does not sample arcs from a
seed. The selected trainer panel shows the same values, the team, the roster,
and a chart of the strength level under each allowed arc against the world cap across p 0–48.
The chart's edition-1 part uses the selected clears, and its post-game part
(p 32–48) uses 24 badges and three clears. The world panel shows p, the world
cap and the level base.

### Rosters of aces and fillers

Each trainer has a **roster**, its true potential. Badge-keyed team stages are
gone.

- **Aces**: one to three, in priority order. Each has an evolution line with
  evolve levels (the level where each later species is reached) and authored
  moves, held item, ability and nature per form.
- **Fillers**: a pool of species lines, each with a `baseScore` (0–100), a
  level offset (-6 to 0, default -2), a `LEVEL_UP` moves policy with an
  optional signature move, and an optional `requiresFlag`. A flagged filler
  stays out until its gameplay flag is set.

Composition at the current world point:

- Team size is `sizeFor(strength level)`, an editable table: below Lv. 20 → 2,
  20 → 3, 30 → 4, 45 → 5, 60 → 6. Sizes must not decrease.
- The **ace allowance** is a maximum tied to size: 1–3 members → 1 ace, 4–5 → 2,
  6 → 3. Aces fill first in priority order, up to min(allowance, aces), and
  every other slot takes a filler. A single-ace trainer at strength 65 fields
  1 ace + 5 fillers. The panel spells out how many ace slots went to fillers.
- Fillers are ranked by `score = baseScore + jitter + active modifiers`, with
  ties broken by filler ID, and the top `size − aces used` join. Scores do not
  depend on size, so growth never removes a filler. Only a modifier or flag
  change can displace one.
- Jitter is uniform over 0..JITTER (default 30, editable). The explorer hashes
  the **Save seed** input with the trainer and filler IDs (FNV-1a), so a seed
  always gives the same teams. The ROM draws it from the TRAINER_ROSTER /
  FILLER_JITTER seed-framework key instead, so the numbers differ from the game.
- Members evolve along their line at their own level (an ace at the strength
  level, a filler at strength + offset).
- Battle order: fillers by ascending score, then aces in reverse priority, so
  the top ace comes last.
- **Modifiers** add a delta (-100 to +100) to one trainer's filler while a
  gameplay flag is set. The **Gameplay flags** bar lists every flag that a
  modifier or `requiresFlag` mentions, and toggles it for the whole pool.

The trainer panel shows the team (species, form, level, ace priority or filler
score, and moves), the size, max aces, aces used and fillers, and the roster:
aces with their lines and whether they play, and each filler's base, jitter,
modifier, score, rank and whether it is in the team (or locked by a flag).

The catalog ships **prototype** rosters, marked as such, for the roster
authoring session to replace. The catalog script derives them from each
trainer's competitive party (the curated six in
`game/src/data/trainer_scaling/gym_leaders.json`, including its ace and level
offsets) or otherwise its reference party (the highest-level member is the
ace, last on ties, and filler offsets are the source level gap clamped to -6..0).
Every other member becomes a filler line with base score 50. Handwritten early
Gym species that no line already covers are added as extra fillers. Lines and
`EVO_LEVEL` thresholds come from `game/src/data/pokemon/species_info`, without
baby pre-evolutions. Stone, trade and friendship evolutions use
max(32, previous level + 10) as a placeholder evolve level. Only the final ace
form carries the source moves, item, ability and nature. Level-up learnsets
are not resolved (the local learnset tables sit behind generation config
branches), so fillers show `LEVEL_UP` as a label.

The summary strip reports the mean, minimum and maximum strength − cap for Gym
Leaders on their selected arcs, and the extremes across every allowed arc.
Strength − cap always compares with the player cap, so at the ceiling Gym Leaders
read about -6. The Blue check asks whether Blue is above the level base under
every allowed arc. Wherever the cap is at most 96 the base is the cap, so this
is the "a step above the cap" target and shows **Yes**. At the ceiling it
compares with the base instead and shows **At ceiling** (default arcs: early
+2, rival +4 above base 96 at (24, 3), and +1 / +4 in later editions). It shows
**No** only if an arc falls to or below the base.

**League feasibility** counts, at the current progress index p, how many
candidates fall in each role window under **every** allowed arc (guaranteed),
under **some** allowed arc (possible) and under the selected arcs (current).
Indigo and Hoenn count their authored home pool. Sevii Masters is an open
invitational, so it counts every candidate and ignores home leagues. Every
trainer is a candidate and brings a team sized by their strength level.
Validation needs every roster to reach six at max size (up to three aces plus
unlocked fillers under the current flags), so each venue lists its shorter
rosters as content gaps. With the prototype rosters these are Lorelei, Bruno,
Agatha, Koga, Lance, Will, Karen, Sidney, Phoebe, Glacia and Drake (5/6). A role
whose guaranteed count is below its need (two contenders, two elites, one
headliner) is flagged as a fallback risk. Standing depends only on p, so
league clears and the headroom do not change these counts. The explorer does not generate
league lineups or simulate the seeded draws.

Edit a trainer's bias and allowed arcs in **Tune this trainer**. **Edit
roster** reorders aces and edits evolve levels, filler base scores, offsets,
signature moves, `requiresFlag` and the prototype marker. **Edit settings as
JSON** covers everything else, such as adding or removing aces and fillers or
authoring ace moves, items, abilities and natures. Arc tuples, role windows,
level headroom, the team size table, JITTER and modifiers are editable under
**Growth arcs, role windows & experiment settings**. Experiments persist in
browser storage. JSON export and import (format version 4) round-trip the
experiment (including rosters, the size table, jitter and modifiers), the world
point, the seed, the set flags and the selected trainer. Version 1 (retired
trainer-rating model), version 2 (four-checkpoint arcs, no headroom) and
version 3 (badge-keyed team stages) files are rejected with a message. There is
no migration. Reset restores the prototype defaults. **Restore this trainer’s
defaults** updates only the selected trainer.

The tool models species, team size and levels. It does not simulate moves,
items, abilities, stats, AI, matchup difficulty or battle outcomes, and it does
not model evolution-item gifts or trades (a trade becomes an ordinary filler
entry with a score boost). Reference moves and items describe source parties
only. The current ROM scaler is unchanged.

The catalog uses local FRLG, Emerald and HNS source records, including their
provenance and explicit variant notes. HNS is not substituted with HGSS;
Steven's local Emerald postgame party is labeled as such. The catalog script
owns each trainer's bias, allowed arcs, home pools and prototype roster. It
validates ace counts (1–3), unique member IDs, evolve levels, offsets and base
scores, and prints the rosters that cannot reach six as a warning, not a
failure. Regenerate or verify
the checked-in catalog from the repository root (requires Python 3, `cc`, `cpp`):

```sh
python3 devtools/scripts/trainer-balance-catalog.py
pnpm --filter @wayfarer/ui exec wa format src/modules/trainer-balance/catalog.json
python3 devtools/scripts/trainer-balance-catalog.py --check
```

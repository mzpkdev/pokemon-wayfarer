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
- `aceLevel = clamp(levelBase + standing, 1, 100)`; members add their level
  offsets (-30 to 0) and are clamped to 1–100.
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
arcs), standing, ace level, a signed **ace − cap** gap, role and party size.
Roles come from standing windows: contender at most -2, elite -1 to +1,
headliner at least +2. The explorer shows a preview arc per trainer (steady
when allowed, otherwise the first allowed arc). It does not sample arcs from a
seed. The selected trainer panel shows the same values, the party, and a chart
of the ace level under each allowed arc against the world cap across p 0–48.
The chart's edition-1 part uses the selected clears, and its post-game part
(p 32–48) uses 24 badges and three clears. The world panel shows p, the world
cap and the level base.

Gym-eligible trainers grow through badge-keyed stages of 2 / 3 / 4 / 5 / 6
members at 0 / 3 / 6 / 10 / 16 badges. The opening stage uses the handwritten
two-species early party. Middle stages combine the strongest reference members
with competitive members, and the last stage is the six-member competitive
profile. A trainer with a shorter competitive profile stops at its size.
Offsets never fall below the previous stage's weakest offset, so party size and
the weakest member's level never drop with default arcs. Other trainers use
their reference party from 0 badges.

The summary strip reports the mean, minimum and maximum ace − cap for Gym
Leaders on their selected arcs, and the extremes across every allowed arc.
Ace − cap always compares with the player cap, so at the ceiling Gym Leaders
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
invitational, so it counts every candidate and ignores home leagues. League
slots need a six-member competitive profile, so five-member profiles are
excluded and listed unless **Count five-member profiles** is checked. A role
whose guaranteed count is below its need (two contenders, two elites, one
headliner) is flagged as a fallback risk. Standing depends only on p, so
league clears and the headroom do not change these counts. The explorer does not generate
league lineups or simulate the seeded draws.

Edit a trainer's bias and allowed arcs in **Tune this trainer**, or their
complete settings, including team stages, as JSON. Team stages stay keyed by
global badges, not p. Arc tuples, the two role window thresholds and the level
headroom are editable under **Growth arcs, role windows & experiment
settings**. Experiments persist in browser storage. JSON export and import
(format version 3) also keep the world point, including completed editions,
and the selected trainer. Version 1 files (retired trainer-rating model) and
version 2 files (four-checkpoint arcs, no headroom) are rejected with a
message. There is no migration.
Reset restores the prototype defaults. **Restore this trainer’s defaults**
updates only the selected trainer.

The tool models species, party size and levels. It does not simulate moves,
items, abilities, stats, AI, matchup difficulty or battle outcomes. Reference
moves and items describe source parties only. Team stages are explicit
prototypes: species do not automatically evolve. Several later rosters still
contain lower evolutions or five-member Elite Four teams, making those gaps
visible for content authoring. The current ROM scaler is unchanged.

The catalog uses local FRLG, Emerald and HNS source records, including their
provenance and explicit variant notes. HNS is not substituted with HGSS;
Steven's local Emerald postgame party is labeled as such. The catalog script
owns each trainer's bias, allowed arcs and home pools. Regenerate or verify
the checked-in catalog from the repository root (requires Python 3, `cc`, `cpp`):

```sh
python3 devtools/scripts/trainer-balance-catalog.py
pnpm --filter @wayfarer/ui exec wa format src/modules/trainer-balance/catalog.json
python3 devtools/scripts/trainer-balance-catalog.py --check
```

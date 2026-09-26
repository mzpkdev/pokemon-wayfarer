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
badge count from 0 to 24 and zero to three first league clears. The module
bundles its trainer catalog, so it works without generating the map catalogs:

```sh
pnpm --filter @wayfarer/ui dev
```

Select a trainer to see their current TR, party, player-cap comparison, source
party, and growth chart. Edit the four TR checkpoints or the team-stage JSON,
then apply the changes. The shared NPC level curve is editable separately.
Experiments persist in browser storage; JSON export/import also preserves the
world point and selected trainer. Reset restores the prototype defaults.
After a default is revised, saved experiments keep their existing values.
Use **Restore this trainer’s defaults** to update only the selected trainer
without discarding other trainer edits or your shared NPC level curve.

The tool models species, party size and levels. It does not simulate moves,
items, abilities, stats, AI, matchup difficulty or battle outcomes. Reference
moves and items describe source parties only. Team stages are explicit
prototypes: species do not automatically evolve. Several later rosters still
contain lower evolutions or five-member Elite Four teams, making those gaps
visible for content authoring. Stage thresholds, growth rates and checkpoint
ratings are provisional, not approved ROM behavior. The current ROM scaler is
unchanged. The [circuit PRD](../../.product/prds/seeded-trainer-circuit.md#world-progression-revision-and-balance-explorer)
records the new direction and the strength/snapshot decisions still to resolve.

Default NPC levels start at 12 at TR 0, allowing Brock's TR 2 team to match
Geodude 12 / Onix 14. Gym leaders start with two-member prototypes and develop
through stages at TR 28 and 48. Four personal TR checkpoints cover 0, 8, 16 and
24 badges; interpolation rounds halves upward. Each first league clear adds
an editable six TR by default, with visible saturation warnings at TR 80.
The existing player TR formula and soft-cap curve remain separate. World-point
controls allow exploratory combinations without enforcing league entry gates.
Growth is deterministic; seed variation and league lineup generation are not
part of this first tool.

Gym eligibility does not dictate a trainer's development speed. An optional
catalog `badgeTRCheckpoints` tuple authors their four checkpoints directly.
Blue uses TR **6 / 54 / 57 / 59** at **0 / 8 / 16 / 24 badges**, giving ace
levels **17 / 59 / 64 / 68** before league-clear growth. His opening remains
approachable, but his eight-badge team is near the other Champions rather than
the ordinary Gym curve. This deliberately puts him above the player's level-42
soft cap at eight badges; it is a Champion-strength experiment, not a claim
that every Gym is equally difficult throughout the journey.

The catalog uses local FRLG, Emerald and HNS source records, including their
provenance and explicit variant notes. HNS is not substituted with HGSS;
Steven's local Emerald postgame party is labeled as such. Regenerate or verify
the checked-in catalog from the repository root (requires Python 3, `cc`, `cpp`):

```sh
python3 devtools/scripts/trainer-balance-catalog.py
pnpm --filter @wayfarer/ui exec wa format src/modules/trainer-balance/catalog.json
python3 devtools/scripts/trainer-balance-catalog.py --check
```

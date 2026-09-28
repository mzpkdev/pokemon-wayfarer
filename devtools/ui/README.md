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
  progress (read exactly, flat past the last anchor). The nine archetypes are
  named as proper nouns in prose and stored as lowercase identifiers
  (`steady`, `prodigy`, `sleeper`, `veteran`, `rival`, `legend`, `star`,
  `comeback`, `burst`). Defaults at world progress 0 / 40 / 80 / 120 / 160:
  **Steady** 0 / 25 / 50 / 75 / 100; **Prodigy** (brilliant early, then evens
  out) 0 / 50 / 80 / 95 / 100; **Sleeper** (underestimated, strong at the end)
  0 / 10 / 25 / 55 / 100; **Veteran** (peaked already, you overtake them) 0 /
  60 / 100 / 100 / 100; **Star** (explodes mid-journey) 0 / 10 / 50 / 90 / 100;
  **Comeback** (stalls, then returns stronger) 0 / 45 / 50 / 55 / 100;
  **Burst** (trains in jumps at milestones) 0 / 25 / 50 / 75 / 100. The
  **Rival** is an ordinary archetype with an extra anchor: 0 / 20 / 40 / 80 /
  120 / 160 → 0 / 15 / 29 / 53 / 76 / 100%. The **Legend** (never changes,
  waits at the top) has anchors at 0 and 160 only, both 0%, so a Legend stays
  at start TR and its peak TR must equal start TR (no Gym Leader is a Legend).
  Every archetype is interpolated except Burst, a step scaler: it holds each
  anchor's growth % until the next anchor, so it matches Steady at 0 / 4 / 8 /
  16 / 24 badges and jumps there, but sits below it in between.
  Blue (the Rival, start TR 0, peak TR 170) is TR 0 at Pallet (one Eevee at
  Lv 5, his signature Umbreon stepped down), TR 26 at world progress 20 (two Pokémon, team level 18), TR 49 at
  40, then 9–10 ahead of the player until he reaches 170. Peak TR must be at
  least start TR.

- **Scalers** turn TR into values. Each is an editable table of anchors
  (TR, value), flat past the last anchor, whose TR is the scaler's ceiling TR.
  An **interpolated** scaler is linear between anchors with halves rounded up;
  a **step** scaler holds each anchor's value until the next one. The kind is
  fixed per scaler and shown in the editor: only Burst is a step scaler. TR
  itself is never clamped.
- **Team level** has its own low end, then the level cap anchors from TR 40:
  (0, 5) (20, 14) (40, 28) (80, 50) (120, 75) (160, 100). TR 200 still gives
  Lv 100. The level cap itself is unchanged.
- **Team size** is interpolated, made a step table by paired anchors: TR 0–10 → 1,
  11–28 → 2, 29–43 → 3, 44–70 → 4, 71–95 → 5, 96+ → 6.
- **Roster**: one ordered list of six roster slots per trainer. Each roster
  slot has a species (normally a final stage, such as Steelix), a level
  offset (-6 to 0), moves (`LEVEL_UP` or one to four authored moves), a held
  item, an optional ability and nature, and an ace flag (`isAce`). Roster
  slot 1 (the signature Pokémon) must be an ace at offset 0, and a roster has
  1–3 aces; every other slot is a filler slot.
- **Team** = the first N roster slots (N = team size), so join order is list
  order and the author sets the rhythm (e.g. ace, filler, ace, filler, filler,
  ace). Each member's level is clamp(team level + offset, 1, 100). **Battle
  order** is the filler slots in reverse list order, then the aces in reverse
  list order, so roster slot 1 is always fought last. Brock (Steelix ace,
  Golem, Crobat, Kabutops, Omastar, Aerodactyl ace) at team size 6 fights
  Omastar, Kabutops, Crobat, Golem, Aerodactyl, Steelix.
- **Evolution (downward only)**: a member whose level is below its stage's
  evolution level steps down its predecessor chain until the level supports
  the stage. It never evolves forward. Level evolutions use their
  `species_info` threshold; non-level evolutions (item, trade, friendship,
  other) use the shared evolution-level table in the catalog script, which
  covers only evolutions without a level in the game data. Baby pre-evolutions
  are not stepped down to. Brock at start TR 25 (team level 18) fields Onix
  Lv 18 and Geodude Lv 16; his Graveler appears from Lv 25, Steelix from Lv 35 and Golem from Lv 38. A member at its
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
and level at world progress 0 / 40 / 80 / 120 / 160 (aces marked), the trainer's
**milestones**, a chart of their team level against the level cap across
player TR 0–200 (the current player TR and the milestones marked; hover or
focus it and use the arrow keys to read values), the team at the current
TR in battle order (aces marked; a stepped-down member shows its authored
stage, e.g. "Onix → Steelix at Lv 35"), and a roster editor: reorder roster
slots, edit species, offset, moves and item, toggle each slot's ace flag
(roster slot 1 is locked as an ace; a fourth ace is refused with a message),
and add or remove roster slots. Each roster
slot shows its line with evolution levels and warns when the species is not a
final stage or has no evolution data in the catalog. **Edit settings as JSON** covers ability and
nature too.

**Milestones** list every player TR (world progress) where the trainer's
team changes, scanning each whole world progress from 0 to the later of the
archetype growth ceiling and the level cap ceiling (TR 160): the starting
team, a roster slot joining (team size steps up; "ace joins" or "joins"), a member's stage changing
along its evolution line, the team level moving strictly above the level cap
or strictly below it again (equal keeps the side, so rounding cannot flicker),
and peak TR reached. The player's current TR is marked in the list. Brock
reads "0: Onix, Geodude (team level above the level cap) · 8: 3rd slot
(Zubat) joins · 19: Zubat → Golbat · 27: Geodude → Graveler · 40: 4th slot
(Kabuto) joins · 49: team level Lv 32 drops below the level cap Lv 33 · 57:
Onix → Steelix · 76: Graveler → Golem, Golbat → Crobat · 85: Kabuto →
Kabutops · 98: 5th slot (Omastar) joins · 151: 6th slot (Aerodactyl) ace
joins · 159: peak TR 100". His late ace Aerodactyl (roster slot 6) joins only
at team size 6 (TR 96, team level 60), which his placeholder peak TR 100
reaches at world progress 151.

Exports and the saved browser state use version 11 (nine archetypes with the
single-word identifiers above; rosters carry `isAce`)
and store the point as `{ "playerTR": n }`; a `{ "badges": n }` point still
imports and sets the matching player TR.

**Tate & Liza** are one entry (role Gym Leader duo, Hoenn) with one start TR,
archetype, peak TR and six-slot roster (Solrock and Lunatone as the signature
pair, both aces at offset 0, with Gardevoir as the third ace). They are fought as a double battle: both leaders send
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
(8 badges, level cap Lv 50) the lineup is Lt. Surge, Giovanni, Agatha,
Jasmine and Norman (all TR 95), team level 59. At 120 (level cap Lv 75) it is
Norman and Steven (TR 130), Giovanni and Jasmine (TR 131) and Lance (TR 132),
team level 81–83. At 160 it is Clair, Juan, Wallace, Steven and Lance (TR
185–200), team level 100: the team level scaler stops at Lv 100, the same as
the level cap there, so the lineup can match the cap but not exceed it.

All thirteen scaler tables (team level, team size, wild level, regular trainer
level, and the Steady, Prodigy, Sleeper, Veteran, Rival, Legend, Star,
Comeback and Burst growth scalers) are editable under
**Scalers & experiment settings**, each labeled interpolated or step. Anchors
start at 0, rise and never decrease in value; growth scalers run 0–100% and
start at 0%. Experiments persist in browser storage. JSON export and import
(format version 11) round-trip the experiment (growth, rosters with their ace
flags and the thirteen scalers), the player TR and the selected trainer. The
importer also rejects a Legend whose peak TR differs from start TR and a
Gym Leader Legend. Files from versions 1–10 are rejected with a message (version
10 used the old archetype names; version
9 had only five archetypes and the old assignments; version 8 had no ace slots and fought the team
simply reversed; version 7 gave the Rival a fixed lead, copied the
level cap into team level and had no Tate & Liza; version 6 gave each notable
trainer one fixed TR; version 5 used the retired 0–80 player TR scale).
There is no migration. Reset restores the catalog defaults. **Restore this
trainer’s defaults** updates only the selected trainer.

The tool models species, team size and levels. It does not simulate moves,
items, abilities, stats, AI, matchup difficulty or battle outcomes. Seeded
archetypes, other sources of world progress and lineup rules beyond the top
five are out of scope for v0. The
scaler the ROM uses today is unchanged.

The catalog's growth and roster battle content are **placeholders**, for the
authoring session to replace. The growth defaults live in the catalog script's `GROWTH`
table, with the approved lore assignments: Lt. Surge, Lorelei, Wattson,
Glacia and Drake are Veterans; Misty, Bugsy, Whitney, Flannery and Tate & Liza are
Stars; Blaine, Pryce and Bruno are Comebacks; Giovanni, Chuck and
Brawly are Bursts; Janine, Falkner, Will and Sidney are Prodigies; Sabrina,
Morty, Clair, Winona, Juan, Lance, Wallace and Steven are Sleepers; Agatha is
a Legend at TR 95; Blue is the Rival; the rest are Steady. Champions and Lance
have the highest peaks. Every Gym Leader entry (Tate & Liza included) has a
placeholder start TR in the 18–40 Gym band, set by archetype rather than Gym
order and varied a little by lore: Sleepers and Stars 18–26, Prodigies 22–30,
Steadies and Bursts 24–34, Veterans and Comebacks 30–40. The
script rejects a Gym Leader start TR outside its archetype's sub-band and a
Gym Leader Legend. At world progress 0 that is team level Lv 13 (Winona, TR 18)
to Lv 28 (Pryce, TR 40), two or three Pokémon; Brock at start TR 25 opens at
Lv 18. League-eligible Steady and Burst Gym Leaders keep start + peak TR at
most 190, and the Veteran Lt. Surge peaks at TR 95, so they stay at TR 95 or less
at world progress 80. The defaults are tuned to the
v0 balance targets, which `engine.test.ts` checks: the lineup at TR 85–95 at
world progress 80, 2–8 levels above the level cap at 120, Lv 100 at 160, every
Gym Leader opening at team level 12 or more and within 16 levels of each other,
Blue about 10 ahead from world progress 40, and the five hardest Gym Leaders
at 24 badges Sleepers or Steadies with peak TR 170 or more (Morty's peak
TR 171 keeps the Star Tate & Liza, peak 170, out of them). The Gym ladder has at least
three Gym Leaders near and above the player at every checkpoint and three below
from world progress 80. At world progress 0 all 24 are above (every start TR
is past the near band), and the lowest three are two Pokémon at or under the
level cap; at 40 none is below yet, but at least three are at or under the
player TR. Rosters are the user-directed roster draft v1 (identity and anime
picks) in the script's `DRAFT` table: six roster slots per trainer in join
order, 1–3 aces at offset 0 (roster slot 1, the signature Pokémon, always
one), fillers at offset -2. Off-type, anime or lore picks carry a tag (e.g.
Misty's Golduck `[anime Psyduck]`) that the roster provenance note lists. Every
species must resolve in `species_info` and `species.h`, or the script fails.
Brock's roster is Steelix (ace), Golem, Crobat, Kabutops, Omastar and
Aerodactyl (ace); Blue's is Umbreon (ace, Eevee early), Pidgeot, Alakazam
(ace), Nidoking, Scizor and Arcanine (ace). Battle content is a placeholder: a
roster slot whose species is in the trainer's source party (the curated
composition in `game/src/data/trainer_scaling/gym_leaders.json`, otherwise its
reference party) keeps that source slot's moves, item, ability and nature;
every other slot uses `LEVEL_UP` (a label: learnsets are not resolved) with no
item. Every roster lists six Pokémon; the explorer still flags a roster edited
below six as incomplete, and the script would warn about one. Four draft
picks are not final stages in this game (Primeape, Ursaring and Girafarig have
later-generation evolutions): allowed, and the script prints them as a warning.

The catalog uses local FRLG, Emerald and HNS source records, including their
provenance and explicit variant notes. HNS is not substituted with HGSS;
Steven's local Emerald postgame party is labeled as such. The catalog script
validates growth (start and peak TR, archetype, peak TR = start TR for a
Legend, the Gym sub-bands, no Gym Leader Legend), roster length (at most six), offsets, moves, roster slot 1
as an ace at offset 0, 1–3 aces per roster, and that the catalog has exactly 38 entries with only the Tate &
Liza duo fought as a double battle. It also validates the shared
evolution-level table (one level per edge, each a real `species_info`
evolution without an `EVO_LEVEL`; a row for a level evolution fails, since the
game's level wins), requires a table entry for every non-level edge on a roster line,
and checks that levels increase along each line with no ambiguous ancestry or
cycles. Table rows are `authored` (the contract examples: Onix → Steelix 35,
Staryu → Starmie 30, Growlithe → Arcanine 35) or `placeholder`;
the script prints the placeholders, the roster slots that kept source battle
content and a warning for roster slots that are not final stages. The catalog records each roster species'
chain compactly under `evolution.chains` (e.g. `["Geodude", 25, "Graveler",
38, "Golem"]`), which the explorer indexes; the saved experiment format is
unchanged. Regenerate or verify the checked-in catalog from the repository root
(requires Python 3, `cc`, `cpp`):

```sh
python3 devtools/scripts/trainer-balance-catalog.py
pnpm --filter @wayfarer/ui exec wa format src/modules/trainer-balance/catalog.json
python3 devtools/scripts/trainer-balance-catalog.py --check
```

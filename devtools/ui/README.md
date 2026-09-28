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
  every archetype, trainer TR = start TR + floor(((peak TR − start TR) ×
  growth % + 50) / 100), where the growth % is that archetype's scaler over
  world progress as a whole percent: the standard scaler rule (interpolated
  with halves rounded up, or held for Burst; flat past the last anchor) rounds
  it first, then the TR gain rounds half up. Brock (Steady, 25 → 100) at world
  progress 97 reads 60.625%, so 61%, and TR 25 + floor((75 × 61 + 50) / 100) = 71. The nine archetypes are
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
  40, then 9–11 ahead of the player (a whole percent of his range is 1.7 TR)
  until he reaches 170. Peak TR must be at least start TR.

- **Scalers** turn TR into values. Each is an editable table of anchors
  (TR, value), flat past the last anchor, whose TR is the scaler's ceiling TR.
  An **interpolated** scaler is linear between anchors with halves rounded up;
  a **step** scaler holds each anchor's value until the next one. The kind is
  fixed per scaler and shown in the editor: team size and Burst are step
  scalers; the rest are interpolated. TR itself is never clamped.
- **Team level** has its own low end, then the level cap anchors from TR 40:
  (0, 5) (20, 14) (40, 28) (80, 50) (120, 75) (160, 100). TR 200 still gives
  Lv 100. The level cap itself is unchanged.
- **Team size** is a step scaler with an anchor at each step's start, (0, 1)
  (11, 2) (29, 3) (44, 4) (57, 5) (71, 6): TR 0–10 → 1, 11–28 → 2, 29–43 → 3,
  44–56 → 4, 57–70 → 5, 71+ → 6 (the same sizes as the retired paired-anchor
  table at every whole TR). Every notable trainer's peak TR is 71 or more, so
  every one fields a full team of six at their peak.
- **Roster**: one ordered list of six roster slots per trainer. Each roster
  slot has a species (normally a final stage, such as Steelix), a level
  offset (-6 to 0), a held item, an optional ability and nature, and an ace
  flag (`isAce`). Roster slots carry no moves. Roster slot 1 (the signature
  Pokémon) must be an ace at offset 0, and a roster has 1–3 aces; every other
  slot is a filler slot.
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
  are not stepped down to: a line stops above its baby (Raichu steps down to
  Pikachu, never Pichu), so baby edges need no table row and the script never
  asks for one. Brock at start TR 25 (team level 18) fields Onix
  Lv 18 and Geodude Lv 16; his Graveler appears from Lv 25, Steelix from Lv 35 and Golem from Lv 38.
- **Move pool**: each trainer authors one ordered list of moves they like,
  each with an optional from level. After every member's
  species and level are resolved (stepping down included), each member starts
  from its default level-up moveset: the constructor's last four level-up
  moves learned by its level, for its current species, oldest first. Then the
  aces, then the fillers, each in list order, walk the pool top to bottom and
  take every entry that is unassigned and eligible, up to four. A move the
  member already knows from its level-up moveset is **claimed**: the entry
  counts toward the four and that move is protected from being replaced. A
  member can learn a move by level-up on its current species **or an earlier
  form of its line** (the natural learn level is the lowest such level), by
  TM/tutor on its current species, or as an **egg move** of its line. An
  entry **without a from level** is eligible only by level-up (own species or
  an earlier form), once the member reaches that natural learn level (an
  evolution move, level 0, counts at once): moves a Pokémon learns naturally
  arrive on their natural schedule, and a member that could get the move only
  by TM/tutor or as an egg move can't take it. An entry **with a from level**
  is eligible for any learner from that level. Pool moves fill empty move
  slots, then replace the oldest unclaimed level-up moves. An entry goes to at
  most one member, so a move listed twice can go to two members. Pool order is
  content: put an ace's own moves above moves meant for later members, so the
  ace fills up first. An entry nobody takes is **dormant**: nobody on the team
  can learn it; it has no from level and the members learn it only by TM/tutor
  ("TM/tutor only — needs a from level") or only as an egg move ("egg move:
  needs a from level"); every eligible member is below its from level (or,
  without one, its learn level, e.g. "below its learn level: earlier form
  (Meowth) at Lv 30"); or every eligible member already has it or four pool
  moves ("taken"). It wakes when a member that can use it joins, evolves or
  reaches the level. Resolution is a pure function of the team and the pool,
  with no randomness. At 0 badges Brock's Onix takes Bind (a Lv 1 level-up
  move) and Curse from his pool and claims Stealth Rock (already in its
  level-up moves), next to Rage, and his Geodude keeps Rollout, Magnitude,
  Strength and Rock Throw; Sandstorm waits for its from level (Lv 20), Earthquake for its learn level
  (Lv 34), Heavy Slam is only an Onix egg move until Graveler evolves into
  Golem, and Cross
  Poison stays dormant until Crobat joins (no one can learn it yet). From Lv
  34 his Graveler takes Earthquake through its earlier form Geodude.

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
and peak TR, the trainer's home region (Kanto, Johto or Hoenn) and traits
(the **Traveller** and **Aloof** checkboxes) and their **play style** (a
select; each option's tooltip and the line under it give the style's flags
and how it plays), shows the resolved **Trainer AI** at the current TR (the AI
skill tier, the aces in the team, the boss mark, and the flag list), shows a checkpoint report at world progress 0 / 40 / 80 / 120 / 160: the
trainer's TR, team level, each roster slot's stage, level and moves (pool
moves highlighted, level-up moves muted; aces marked), the battle order, the
number of dormant pool entries and the resolved AI skill tier (Brock: TR 25 /
44 / 63 / 81 / 100, 8 / 7 / 2 / 2 / 2 dormant entries, None / Aware / Aware /
Smart / Smart), the trainer's
**milestones**, a chart of their team level against the level cap across
player TR 0–200 (the current player TR and the milestones marked; hover or
focus it and use the arrow keys to read values), the team at the current
TR in battle order (aces marked; a stepped-down member shows its authored
stage, e.g. "Onix → Steelix at Lv 35"; each member lists its resolved moves,
each tagged `pool` or `level-up`; a claimed level-up move reads `pool`), the
**dormant** pool entries at the current player TR with the reason for each (no
one can learn it, TM/tutor only or egg move — needs a from level, below from
level or learn level, or taken), a roster editor: reorder roster
slots, edit species, offset and item, toggle each slot's ace flag
(roster slot 1 is locked as an ace; a fourth ace is refused with a message),
and add or remove roster slots, and a **move pool editor**: reorder, add and
remove entries, edit each move name (checked against the game's move list,
any case) and optional from level; each entry shows the member that took it at
the current TR or "dormant", and how the roster's lines learn the move: the
earliest level-up learner (e.g. "Level-up: Golem, earlier form (Geodude) at
Lv 34"), or TM/tutor only or an egg move, which needs a from level. Each roster
slot shows its line with evolution levels and warns when the species is not a
final stage or has no evolution data in the catalog. **Edit settings as JSON** covers ability,
nature and the move pool too.

**Milestones** list every player TR (world progress) where the trainer's
team changes, scanning each whole world progress from 0 to the later of the
archetype growth ceiling and the level cap ceiling (TR 160): the starting
team, a roster slot joining (team size steps up; "ace joins" or "joins"), a member's stage changing
along its evolution line, the team level moving strictly above the level cap
or strictly below it again (equal keeps the side, so rounding cannot flicker),
a dormant move pool entry assigned for the first time ("Earthquake wakes
(Graveler)"), and peak TR reached. The player's current TR is marked in the list. Brock
reads "0: Onix, Geodude (team level above the level cap) · 6: Sandstorm wakes
(Onix) · 8: 3rd slot (Zubat) joins · 19: Zubat → Golbat · 27: Geodude →
Graveler · 40: 4th slot (Kabuto) joins · 46: Rock Blast wakes (Graveler) · 49:
team level Lv 32 drops below the level cap Lv 33 · 57: Onix → Steelix · 60:
Earthquake wakes (Graveler) · 67: 5th slot (Omanyte) joins · 70: Explosion
wakes (Graveler) · 76: Graveler → Golem, Golbat → Crobat, Heavy Slam wakes
(Golem), Cross Poison wakes (Crobat) · 84: Kabuto → Kabutops, Omanyte →
Omastar · 92: Stone Edge wakes (Golem) · 97: 6th slot (Aerodactyl) ace joins
· 160: peak TR 100" (Rock Slide never wakes: its only level-up learner, Onix
and then Steelix, already holds four pool moves by its learn level). His late ace Aerodactyl (roster slot 6)
joins at team size 6 (TR 71, team level 45), which his placeholder growth
reaches at world progress 97; at 159 the growth is 99%, so peak TR 100 comes
at 160.

Exports and the saved browser state use version 20 (nine archetypes with the
single-word identifiers above; rosters carry `isAce` and no moves; each
trainer has a `movePool` of `{ "move", "fromLevel"? }` entries, a
`homeRegion`, the `traveller` and `aloof` trait flags and a `playStyle`)
and store the point as `{ "playerTR": n }` and the invitation simulator
settings as `{ "choices": ["win" | "lose" | "decline", …], "split": null |
{ "total": n, "Kanto": n, "Johto": n, "Hoenn": n } }` (the answers to 1–24
invitations, and the badge split: null follows the badge total, or the badge
total (0–24) it was set for with 0–8 badges per region; a split with no
`total` belongs to the saved player TR's badge total); a
`{ "badges": n }` point still imports and sets the matching player TR. The
call counter lives only in the simulation (the saved settings are the answers
and the split), and team size keeps its `[TR, size]` anchors, so neither
changes the format: a version 20 file with the older paired team size
anchors (0–10, 11–28, …) reads the same size at every whole TR.

**Tate & Liza** are one entry (role Gym Leader duo, Hoenn) with one start TR,
archetype, peak TR and six-slot roster (Solrock and Lunatone as the signature
pair, both aces at offset 0, with Gardevoir as the third ace). They are fought as a double battle: the resolved team
(team size from the same table) is sent in battle order, fillers first and
aces last, and its members alternate between Tate and Liza along that order,
anchored at its end: the last member goes to Tate, the one before it to Liza,
and so on, so each leader's last Pokémon is an ace whenever the team has two.
The full team splits Tate: Xatu, Gardevoir, Solrock and Liza: Grumpig,
Claydol, Lunatone; at world progress 0 (TR 26, two Pokémon) Tate sends
Solrock and Liza Lunatone. The explorer marks them as a double battle and
shows the split under the team and on each member. They follow every notable trainer
rule but are league-ineligible (`leagueEligible: false`; leagues are singles
only).

**Gym ladder** lists the 24 Gym Leader entries (Tate & Liza included) at the
current world progress, sorted by TR, each marked below, near (within 10 of
the player TR) or above.

**Invitation simulator** ([Leagues spec](../../.product/specs/leagues.md#invitations))
simulates the league invitations and ranks every eligible trainer by league
score for each invitation's event. Each league is
a location: Indigo's location regions are Kanto and Johto, Hoenn's is Hoenn,
and the Sevii Masters is a neutral location where everyone is at home. The
catalog gives each notable trainer a home region (`homeRegion`) and traits:
opt-in yes/no flags that default to no ([Traits](../../.product/specs/notable-trainers.md#traits)). The **traveller** trait
(`traveller`; the [Home region and travel](../../.product/specs/notable-trainers.md#home-region-and-travel) lore assignments, editable per trainer as the
**Traveller** checkbox) makes away travel cheap. Each league event gives every league-eligible trainer (singles
only: the catalog has no Red and Tate & Liza are ineligible) a
**willingness** of max(5, 100 − travel cost − fatigue). Travel cost is 0 at
home; away it is 10 for a traveller and 80 for everyone else; fatigue is 50 for a
trainer in the lineup of the most recent resolved event (the invitation
before, whether the player accepted or declined it). The **league score** is floor(TR × willingness / 100);
the top five by score make the lineup (ties in catalog order, standing in for
`characterId`), which fights by ascending TR, strongest last (equal TRs in
catalog order). There is no seed or randomness: the same world progress and
previous event always give the same five (the ROM computes the lineup when
the player accepts or declines and freezes it for that event).

Some trainers are **aloof** (`aloof`, a trait; the [Aloof](../../.product/specs/notable-trainers.md#aloof) lore
assignments, editable per trainer as the **Aloof** checkbox): Lance, Wallace,
Steven, Agatha, Glacia, Clair, Sabrina and Karen. They won't join a league
whose base lineup is well below their level. The top five trainers who are not
aloof, by league score (fatigue included), are the **base lineup**; its
strongest team level is the **base lineup level**. An aloof trainer joins only
when their team level is at most the base lineup level plus 10 (levels, not
TR; a placeholder margin), judged against the base lineup and never against
other aloof trainers; with no base lineup (no eligible trainer who is not
aloof), every aloof trainer skips. Everyone who joins is then ranked by league
score as above. An aloof trainer who skips is listed unranked at the end of
the table.

The **Sevii Masters** ([Leagues spec](../../.product/specs/leagues.md#reign-records))
is an off-the-record club, not an official league. The aloof rule is off
there: there is no base lineup, and every aloof trainer joins ("joins (aloof
rule off at the Masters)"). A **Master** is anyone who has been champion at
both Indigo and Hoenn; a Masters title never counts. The simulator records,
for each notable trainer, whether they have reigned at Indigo and at Hoenn
(the event's strongest takes the title whenever the player loses or
declines). Notable trainers who are Masters get guaranteed seats at the
Masters, the highest league scores first when more than five are Masters,
fatigue included; the remaining seats go to the highest league scores.

The simulator runs at the player's world progress (days never change anyone's
strength). The player qualifies at player TR 80 (in v0 exactly 8 badges; the
explorer counts the qualifying moment as day 0); below it the panel says no
league calls. An invitation arrives after a countdown of 7 in-game days,
started at qualifying and restarted when an invitation resolves; no countdown
runs while an invitation or accepted event is pending, and the simulator has
the player arrive the day they accept, so the calls come on days 7, 14, 21 and
so on. The countdown counts only forward day advances: when the day counter
goes backwards (a clock turned back) the countdown neither advances nor
resets, and a counter far ahead makes one call due, never a backlog. When the
call is due but no league is eligible, no call is made and the countdown
keeps waiting, re-checked at every later day. **Invitations**
(default 8, up to 24) sets how many; the **Kanto**, **Johto** and **Hoenn**
fields set the badge split (by default the badge floor of the player TR,
filling Kanto, then Johto, then Hoenn, 8 each). A split set by hand is saved
with its badge total and applies only while the player TR gives that total:
typing or dragging the player TR through other totals keeps it, and it applies
again back at its total. **Follow badge total** drops it, and so do a badge
preset or slider move to another total and a committed player TR (Enter,
leaving the field, or releasing the slider) at another total. Each number field
shows the value the simulation uses: an entry out of range is refused (or,
for the player TR, clamped to 0 or more) and the field reverts to it. Which league calls ([Leagues spec](../../.product/specs/leagues.md#which-league-calls)):
a league calls only where the player is known: Indigo with at least one Kanto
or Johto badge, Hoenn with at least one Hoenn badge, and the Sevii Masters,
whose caretaker calls, once the player is a Master (they have won both
Indigo and Hoenn in the run). With no badges at all the panel says no league calls.
Calls are ordered by a monotonic **call counter**, never by days: each call
takes the next sequence number (1, 2, 3, …) and each league remembers the
number of its last call, so a clock turned back cannot reorder them. Among the
eligible leagues the one that called least recently, the lowest sequence
number, calls (a league that never called counts as least recent); ties go to the most badges (Indigo
= Kanto + Johto, Hoenn = Hoenn; the Masters has none but is never in a tie,
since Indigo and Hoenn have both called before it can), then to Indigo. So the first call comes from the eligible league with
the most badges, and a single eligible league calls every time. Each row shows
the invitation, its day, the league and why ("only eligible (Indigo 8, Hoenn
0); the Masters needs Indigo and Hoenn wins", "least recently called (Indigo
call 1, Sevii Masters never, Hoenn call 2); the caretaker calls: Indigo and
Hoenn both won", "tie (never called) → most badges (Indigo 16, Hoenn 8)" or
"tie (never called, Indigo 4, Hoenn 4) → Indigo"; the Masters note follows
every reason until the Masters can call), the lineup in battle order with
league scores (a Master seat marked "(Master)"),
the invitation whose lineup it fatigues, an **Answer** (accept & win, accept &
lose, or decline; decline by default), the result (first win, repeat win,
loss, declined) and the reigning champion: the player after a win, otherwise
the lineup's strongest member (last in battle order). A line above the table
names each league's reigning champion after the last invitation, and another
who has reigned at Indigo and at Hoenn, the notable Masters, and the Masters
Gallery (each Masters event's winner by invitation, "#3 Lance (TR 200)" or
"#12 the player"; the game keeps only each winner's win count). An answer changes who calls next (wins at both Indigo and Hoenn open
the Masters) and who reigns, which decides the Masters' guaranteed seats; it
never changes a rule.
**Show invitation N** selects an event; below the table it shows the base
lineup level, each aloof trainer's check ("team Lv X vs base lineup Lv Y + 10
→ joins/skips"; at the Masters "aloof rule off at the Masters · N Master
seats" and "team Lv X, aloof rule off at the Masters → joins"), the lineup
with teams, levels and league scores (a "Master seat" tag on a guaranteed
seat), and every eligible trainer's rank, TR, team level, home region,
whether they are a traveller, at home or away, travel cost, fatigue,
willingness, league score, whether an aloof trainer joins or skips, where
they have reigned (Master, Indigo or Hoenn), and whether they are in the
lineup ("in lineup (Master seat)" for a guaranteed seat).
The summary strip names the Indigo finalist (the strongest of a fresh Indigo
lineup at the current world progress). With the catalog defaults (lineup in
battle order, league score in brackets where fatigue or travel lowers it; the
reigning champion last):

- 8 badges (world progress 80, base lineup Lv 59–61), all Kanto, declining
  everything: only Indigo knows the player, so it calls every time. Its
  fresh lineup is Lt. Surge, Giovanni, Will, Koga and Karen (TR 95–101, all
  at home; champion Karen); the best away trainer, Drake (a Hoenn traveller,
  TR 93), scores 83 and ranks 12th. Karen (Lv 63), Agatha (Lv 59), Glacia,
  Sabrina and Clair join; Lance (Lv 100), Steven (Lv 77) and Wallace (Lv 74)
  skip. The next event, with those five tired, fields Erika, Blue, Lorelei,
  Agatha and Jasmine (champion Jasmine), and the two lineups alternate.
- 8 Hoenn badges, declining the first invitation, then accepting and winning
  every one: day 7 Hoenn (only eligible, declined; Glacia, Sidney, Drake,
  Norman, Phoebe, all at home; Phoebe reigns), day 14 Hoenn again (only
  eligible; Juan, Wattson, Blue (81), Giovanni (85), Will (86); first win),
  then Hoenn every week, alternating those two lineups (repeat wins). Indigo,
  with no Kanto or Johto badge, never calls, so the player never wins it and
  the Masters never calls.
- 16 badges, all Kanto and Johto, accepting and winning everything: Indigo
  calls every time, alternating Koga, Karen, Blue, Giovanni, Jasmine (first
  win) with Morty, Will, Sabrina, Clair, Erika. Hoenn and the Masters never
  call. Declining everything instead gives the same two lineups (champions
  Jasmine and Erika); the Champions (Lv 99–100) skip every base lineup
  (Lv 75–82).
- 16 badges, Kanto 8 and Hoenn 8 (the [Leagues PRD](../../.product/prds/leagues.md#sample-playthrough)
  sample), winning the first two invitations and declining the rest: day 7
  Indigo (tie → Indigo; first win), day 14 Hoenn (first win), day 21 the
  Masters (the caretaker calls; the aloof rule off, so Giovanni, Jasmine,
  Steven, Wallace, Lance; Lance reigns and goes in the Gallery). Blue reigns
  at Indigo on day 28, Giovanni (a traveller) at Hoenn on day 35 and at
  Indigo on day 49, which makes him a Master; at the day 63 Masters his seat
  is guaranteed, and on day 84, tired from Hoenn (league score 65), he keeps
  it over Blue (129).
- 24 badges (world progress 160, base lineup Lv 100, every aloof trainer
  joins), declining everything: Indigo (tie → most badges), Morty, Sabrina,
  Clair, Steven (175), Lance (Lance); Hoenn (least recently called: never),
  Norman, Blue (153), Winona, Juan, Wallace (Wallace); and so on
  alternately. Lance, a Kanto traveller, fights last at every Indigo event,
  each fatigued only by the Hoenn event before. With no win the Masters never
  calls, and no one is a Master: Lance reigns only at Indigo, Wallace only at
  Hoenn.

Lance, a Legend at TR 200 (team Lv 100), first joins a league at world
progress 135 (base lineup Lv 91) when no fatigue holds the base lineup back.
No aloof lineup member sits more than 10 levels above the base lineup level.

**Trainer AI** ([spec](../../.product/specs/trainer-ai.md)) resolves each
trainer's AI flags at their current TR, as the ROM writes them once per
battle: the **Basic** bundle (Check Bad Move, Try To Faint, Check Viability),
then the flags of the trainer's **play style** (`playStyle`, exactly one of
eight, the [Trainer AI spec](../../.product/specs/trainer-ai.md#play-styles) assignments; editable), then the **AI skill** tiers up to
their TR (a step scaler, placeholder: TR 0–29 None; 30–69 Aware adds Smart
Mon Choices and Assume STAB; 70–109 Smart adds Smart Switching, Assume Status
Moves and Weigh Ability Prediction; 110+ Predictive adds Predict Switch,
Predict Incoming Mon and Predict Move), then **ace protection** from the aces
in the resolved team (Ace Pokemon for one, Double Ace Pokemon for two or
more; roster slot 1 makes at least one), then Omniscient for a trainer with
the authored **boss flag** (`bossOmniscient`, Lance only; not editable), then
the engine's own additions (Double Battle for Tate & Liza, Smart Mon Choices
with Smart Switching, Predict Switch with Predict Incoming Mon), listed in
engine bit order. The styles: Gambler (Risky), Bomber (Risky, Will Suicide),
Sweeper (Force Setup First Turn), Field marshal (Powerful Status), Hexer
(Prefer Status Moves, HP Aware), Turtle (Conservative, HP Aware), Brawler (Try
To 2HKO, Prefer Highest Damage Move) and Tactician (HP Aware). No resolution
combines Risky with Conservative or uses Stall. At the badge points, Brock
(Field marshal) is Basic + Powerful Status + Ace Pokemon at 0 badges (TR 25),
adds the Aware tier at 8 (TR 63) and the Smart tier with Double Ace Pokemon
once Aerodactyl joins (16 and 24 badges, TR 81 and 100); Lance is Predictive,
Omniscient and Double Ace (three aces, the third unprotected) throughout.

All thirteen scaler tables (team level, team size, wild level, regular trainer
level, and the Steady, Prodigy, Sleeper, Veteran, Rival, Legend, Star,
Comeback and Burst growth scalers) are editable under
**Scalers & experiment settings**, each labeled interpolated or step. Anchors
start at 0, rise and never decrease in value; growth scalers run 0–100% and
start at 0%. Experiments persist in browser storage (key
`wayfarer-trainer-balance-v20`). JSON export and import
(format version 20) round-trip the experiment (growth, rosters with their ace
flags, move pools, home regions, traveller and aloof flags, play styles and
the thirteen scalers), the
player TR, the invitation simulator (answers and badge split), and the selected trainer. The
importer also rejects a Legend whose peak TR differs from start TR, a
Gym Leader Legend, a roster slot with moves, an unknown move name, a from level
outside 1–100, a pool of more than 64 entries, an unknown home region, and
a traveller or aloof flag that is not true or false, and a play style that
is not one of the eight. Import accepts the league settings' keys in any
order. Files from versions
1–19 are rejected with a message (version 19 may carry catalog defaults that
have since changed, such as the Elite Four and Champion starts and Karen and
Koga no longer travellers; version 18 saved league calendar settings,
days and wins, for the retired league calendar; version 17 saved a league
entry point for the retired standard entry sequence; version 16 had no play styles; version 15 saved a `travel` style, homebody
or traveller, instead of the traveller flag; version 14 had no aloof trait and the old
Sleeper Champions; version 13 saved a league seed for the retired
seeded lineup draw; version 12 had no home regions or traveller trait and
assumed fixed league lineups; version 11 authored moves per roster slot and had no
move pools; version
10 used the old archetype names; version
9 had only five archetypes and the old assignments; version 8 had no ace slots and fought the team
simply reversed; version 7 gave the Rival a fixed lead, copied the
level cap into team level and had no Tate & Liza; version 6 gave each notable
trainer one fixed TR; version 5 used the retired 0–80 player TR scale).
There is no migration. Reset restores the catalog defaults. If the saved
browser state fails to restore, the explorer shows why and saves nothing over
it until Reset or a successful import. **Restore this
trainer’s defaults** updates only the selected trainer.

The tool models species, team size, levels and each member's moves from the
move pool. It does not simulate items, abilities, stats, how the AI plays (it only lists
the resolved AI flags), matchup difficulty or battle outcomes, and it skips the randomizer precedence (a
species or learnset randomizer keeps the plain level-up moveset in the ROM). Seeded
archetypes and other sources of world progress are out of scope for v0. The
scaler the ROM uses today is unchanged.

The catalog's growth and roster battle content are **placeholders**, for the
authoring session to replace. The growth defaults live in the catalog script's `GROWTH`
table, with the approved lore assignments: Lt. Surge, Lorelei, Wattson,
Glacia and Drake are Veterans; Misty, Bugsy, Whitney, Flannery and Tate & Liza are
Stars; Blaine, Pryce and Bruno are Comebacks; Giovanni, Chuck and
Brawly are Bursts; Janine, Falkner, Will and Sidney are Prodigies; Sabrina,
Morty, Clair, Winona and Juan are Sleepers; Agatha is a Legend at TR 95; Blue
is the Rival; the rest are Steady. The Champions follow lore ([Growth with world progress](../../.product/specs/notable-trainers.md#growth-with-world-progress)):
Lance is a Legend fixed at TR 200 (start = peak), Steven a Burst (50 → 195)
and Wallace a Star (48 → 190); they have the highest peaks. Every Gym Leader entry (Tate & Liza included) has a
placeholder start TR in the 18–40 Gym band, set by archetype rather than Gym
order and varied a little by lore: Sleepers and Stars 18–26, Prodigies 22–30,
Steadies and Bursts 24–34, Veterans and Comebacks 30–40. The
script rejects a Gym Leader start TR outside its archetype's sub-band and a
Gym Leader Legend. At world progress 0 that is team level Lv 13 (Winona, TR 18)
to Lv 28 (Pryce, TR 40), two or three Pokémon; Brock at start TR 25 opens at
Lv 18. League-eligible Steady and Burst Gym Leaders keep start + peak TR at
most 190, and the Veteran Lt. Surge peaks at TR 95, so they stay at TR 95 or less
at world progress 80. The Elite Four and Champions start above the Gym band,
at TR 41 or more (the script's `ELITE_START_MIN`; it rejects a lower start),
except Blue, the Rival, who leaves Pallet with the player at TR 0. The members
who once started inside the band start at TR 41–50 by lore, with their peaks
unchanged (the lowest Elite Four start per region: Will and Sidney 41, from 30
and 25, and Lorelei 42, from 40; then Phoebe 43, from 25, Koga and Glacia 44,
from 30 and 40, and Karen 47, from 30); Bruno (45),
Drake (45), Wallace (48), Steven (50), Agatha (95) and Lance (200) already
started above it. The defaults are tuned to the
v0 balance targets, which `engine.test.ts` checks: every
notable trainer at team size 6 at their peak TR, every
Gym Leader opening at a team level of 12 or more (a team-level floor: a filler
at offset -2 may open lower, e.g. Winona's Lv 11 fillers at team level 13) and
within 16 levels of each other, Blue about 10 ahead (9–11) from world progress
40, and the five hardest Gym Leaders
at 24 badges Sleepers or Steadies with peak TR 170 or more (Morty's peak
TR 171 keeps the Star Tate & Liza, peak 170, out of them). The Gym ladder
follows the spec's target: from the first badges on, some Gym Leaders sit near
the player and some above, and from world progress 80 some also sit below
(at least three below, three near, and three above). At world progress 0 all
24 are above (every start TR is past the near band), and the lowest three are
two Pokémon at or under the level cap; at 40 none is below yet, but at least
three are at or under the player TR. League lineups have no fixed targets: they are informational, and
the tests check the league score and its flooring, the top-five selection and
tie-break, the invitation gate (none at player TR 79, calls from 80), which
league calls (known-there eligibility: no badges in a region means that
league never calls, the Masters only after wins at both Indigo and Hoenn; the least recently called
eligible league, round-robin over three; ties to the most badges, then
Indigo; a single eligible league calling again and again), the call counter
(each call takes the next sequence number; a clock turned back reorders
nothing), the countdown (7 days from qualifying, restarted at each resolution,
stopped while an invitation or accepted event waits, counting only forward day
advances, and waiting, due, while no league is eligible), fatigue from the most recent resolved event whether accepted or
declined, reigning champions (the player after a win, otherwise the lineup's
strongest, at decline too), Sevii neutrality, determinism (days never change
TR), and
the aloof rule (joining at exactly the base lineup level + 10 and skipping
one level above, never compared with other aloof trainers, skipping when there
is no base lineup, fatigue still applying, Lance skipping fresh 8- and
16-badge Indigo events and fighting last at a fresh 24-badge event, and no aloof lineup member more than 10 levels
above the base lineup level at Indigo or Hoenn), and the Masters (never
calling before wins at both Indigo and Hoenn; the aloof rule off, so an aloof
trainer attends however far above the other trainers; a Master, someone who reigned at
both Indigo and Hoenn, seated over a higher-scoring trainer who is not one;
more than five Masters seated by league score; reigns recorded at Indigo and
Hoenn only, never for a Masters title). Home regions and the travellers live in the
script's `HOME_REGION` table and `TRAVELLERS` set, and the aloof trainers in its `ALOOF` set (which may name only
league-eligible catalog trainers). Play styles live in its `PLAY_STYLES`
table (the script rejects an unknown style and a trainer with none or more
than one) and the boss flag in `BOSS_OMNISCIENT`. Rosters are the user-directed roster draft v1 (identity and anime
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
reference party) keeps that source slot's item, ability and nature; every
other slot has no item. Each trainer's move pool is the user-directed pool
draft v1 in the script's `POOL_DRAFT` table (`movePoolSource` reads
"user-directed pool draft v1: " and the trainer's gimmick): 8–12 ordered
entries, identity moves first so the aces take them, each ace's own moves
above moves meant for later members (a move meant for two members is listed
twice, e.g. Jasmine's Spikes for Skarmory and Forretress). An entry has a from
level only when a roster line it is meant for learns it only by TM/tutor or as
an egg move, or to hold back a strong move a line learns by level-up too early,
by tier: status and utility 20; attacks under 90 power and setup 30 (Shell
Smash, Belly Drum and Quiver Dance 40); 90–100 power 38; 110+ power or a heavy
drawback 45; OHKO 55. Earlier forms and egg moves make draft moves such as
Giovanni's Pay Day (Meowth Lv 30), Brawly's Spore (Shroomish Lv 40), Lance's
Extreme Speed and Wallace's Mirror Coat (egg moves, from Lv 30) legal. A draft move no roster line can learn by
any of these is replaced by the closest legal move that keeps the gimmick (a
comment in the table names it); Brock's pool is
Bind, Stealth Rock, Sandstorm (from Lv 20), Curse (for Iron Defense), Stone Edge,
Earthquake, Rock Slide, Heavy Slam, Rock Blast, Cross Poison and Explosion.
The script fails when a pool draws on more than one frustration category (its
`FRUSTRATION` map: sleep, evasion, OHKO, trapping, Perish Song, Destiny Bond,
infatuation/confusion) or pairs evasion with Toxic or Toxic Spikes. The map
covers moves whose **main effect** is the frustration, including a damaging
move whose effect always happens (Mud Slap's accuracy drop, Dynamic Punch's
confusion); trapping includes Fairy Lock and Thousand Waves. Secondary-chance
effects are excluded: Hurricane's or Water Pulse's chance to confuse, and
damaging moves with a chance to lower accuracy (Muddy Water, Octazooka,
Mirror Shot). Every drafted pool passes the rule.
An old move name (Faint Attack) is stored as the move itself (Feint Attack).
The script warns about pool entries
no stage on the trainer's roster lines can learn, and about entries without a
from level that every stage on those lines learns only by TM/tutor or as an egg
move (both stay dormant; the second needs a from level). Every roster lists six Pokémon; the explorer still flags a roster edited
below six as incomplete, and the script would warn about one. Four draft
picks are not final stages in this game (Primeape, Ursaring and Girafarig have
later-generation evolutions): allowed, and the script prints them as a warning.

**Learnsets.** The catalog records, for every stage on every roster line, the
level-up learnset (move and level, level 0 being an evolution move) and the
TM/tutor (teachable) list, and for each line's first stage the line's egg
moves, as the Wayfarer ROM builds them, under `learnsets` (`moves` is the
sorted list of valid move names; each species stores `levelUp` as flattened
`[level, move index, …]` pairs in the game's order, `teachable` as move
indices, and a line's first stage also `egg` as move indices). The script runs the game's own
learnset helpers (`make_tutors.py`, `make_teaching_types.py`,
`make_teachables.py --build POKEMON_WAYFARER`, the Makefile's recipe for
`BUILD=wayfarer`) in a scratch copy of their inputs, so nothing is written
under `game/`, to produce `teachable_learnsets.h`. It then runs the C
preprocessor (`arm-none-eabi-cpp`, else `cpp`) with the Makefile's
`CPPFLAGS` for that build (`-DPOKEMON_WAYFARER -DPOKEMON_HNS -DMODERN=1
-DTESTING=0`, the legacy multiboot capabilities off) over global.h's leading
include block (without `constants/maps.h`, which needs generated map
headers), pokemon.c's own `P_LVL_UP_LEARNSETS` `#if` chain (currently
`GEN_7`), the teachable header, `egg_moves.h` and `species_info.h`, as
pokemon.c includes them, and parses the expanded arrays, so every
generation-config `#if` resolves as it does in the ROM. A line's egg moves are
those of the species its Egg hatches as: daycare.c's `GetEggSpecies` walks
predecessors back to the root, babies included (a Pikachu Egg hatches as
Pichu), and that species' `species_info` `.eggMoveLearnset` array is read
(none means no egg moves). With `P_INCENSE_BREEDING` at Gen 9 no incense item
changes the Egg; the script fails if the build ever sets it lower. Valid
moves are the `enum Move` members before `MOVES_COUNT` (no Z-Moves or Max
Moves), aliases excluded. The script fails if any roster stage lacks a
learnset or has no level-up move at the lowest level it can appear at.

The catalog uses local FRLG, Emerald and HNS source records, including their
provenance and explicit variant notes. HNS is not substituted with HGSS;
Steven's local Emerald postgame party is labeled as such. The catalog script
validates growth (start and peak TR, archetype, peak TR = start TR for a
Legend, the Gym sub-bands, no Gym Leader Legend, Elite Four and Champion
starts of TR 41 or more except the Rival), roster length (at most six), offsets, no per-slot moves,
move pools (known move names, from levels 1–100, at most 64 entries), roster slot 1
as an ace at offset 0, 1–3 aces per roster, and that the catalog has exactly 38 entries with only the Tate &
Liza duo fought as a double battle. It also validates the shared
evolution-level table (one level per edge, each a real `species_info`
evolution without an `EVO_LEVEL`; a row for a level evolution fails, since the
game's level wins), requires a table entry for every non-level edge on a roster line
(never for a baby edge: lines stop above their babies, so Pichu → Pikachu,
Munchlax → Snorlax or Mime Jr. → Mr. Mime need no row),
and checks that levels increase along each line with no ambiguous ancestry or
cycles. Table rows are `authored` (the contract examples: Onix → Steelix 35,
Staryu → Starmie 30, Growlithe → Arcanine 35) or `placeholder`;
the script prints the placeholders, the roster slots that kept source battle
content and a warning for roster slots that are not final stages. The catalog records each roster species'
chain compactly under `evolution.chains` (e.g. `["Geodude", 25, "Graveler",
38, "Golem"]`), which the explorer indexes; the saved experiment format is
unchanged. Regenerate or verify the checked-in catalog from the repository root
(requires Python 3, `cc`, `cpp`; `arm-none-eabi-cpp` is used for learnsets when installed):

```sh
python3 devtools/scripts/trainer-balance-catalog.py
pnpm --filter @wayfarer/ui exec wa format src/modules/trainer-balance/catalog.json
python3 devtools/scripts/trainer-balance-catalog.py --check
```

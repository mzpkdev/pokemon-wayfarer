# Wayfarer interregional League circuit

PRD: [Wayfarer interregional League circuit](../prds/wayfarer-interregional-league-circuit.md)

Implemented: Yes

Proposed successor: [Seeded circuit runtime](seeded-league-circuit.md), with
[trainer selection and strength](circuit-trainer-pool.md). Recurring editions
separate current competition progress from lifetime first clears and unlocks.
The parent
[draft PRD](../prds/seeded-trainer-circuit.md) records confirmed requirements and
remaining design decisions. This document remains the approved baseline until
that successor is adopted.

The runtime now uses one persisted circuit stage identity for FRLG Indigo,
Sevii Masters, and Hoenn. Regional Champion flags are projections of committed
circuit clears rather than the source of circuit progression.

## Scope

This specification owns:

- global badge aggregation;
- the fixed Indigo, Sevii Masters, Hoenn circuit order;
- admission, run identity, loss/retry, replay, and first-clear recording;
- circuit Trainer Rating contributions;
- shared Kanto/Johto Champion recognition from Indigo;
- the Seven Island battle-house entrance and HNS room-chain reuse;
- Trainer Card circuit presentation; and
- Red's circuit-completion admission at Mt. Silver.

Regional badge awards, local stories, ordinary travel, wild populations,
ordinary-Trainer scaling, Gym scaling, and each source roster's authored party
data remain with their owning specifications. League opponent levels remain
owned by [League scaling](league-scaling.md).

## Circuit identities

Use a circuit-stage identity distinct from `enum Region`:

```text
CIRCUIT_STAGE_INDIGO
CIRCUIT_STAGE_MASTERS
CIRCUIT_STAGE_HOENN
```

Region identifies map and story provenance. Circuit stage identifies
qualification, roster, run state, first-clear state, reward, and presentation.
The Masters maps resolve to Sevii/Kanto for ordinary regional systems and to
`CIRCUIT_STAGE_MASTERS` only while dispatching the challenge.

The three stages are:

| Stage | Qualification | Venue | Authored roster |
| --- | --- | --- | --- |
| Indigo | At least 8 global badges | FRLG Indigo rooms | Lorelei, Bruno, Agatha, Lance, Blue |
| Masters | Indigo first clear and at least 16 global badges | Seven Island battle house into HNS rooms | Will, Koga, Bruno, Karen, Lance |
| Hoenn | Indigo and Masters first clears and all 24 badges | Emerald League rooms | Existing Emerald Elite Four and Champion |

Red at Mt. Silver requires all three first clears. Red is not a circuit stage,
does not create a run record, and contributes no circuit Trainer Rating.

## Global badges

Derive the global count without separate mutable storage:

```text
globalBadges = kantoBadges + johtoBadges + hoennBadges
```

Each regional count is between zero and eight. Re-reading an earned badge or
re-running its award script cannot increase the result. Regional displays and
story checks retain badge identity and origin.

All twenty-four badges remain obtainable with zero circuit clears. Initial Gym
entry, reward-bearing Leader battles, and deferred badge awards must not read a
circuit-clear cap. Eight, sixteen, or twenty-four badges with no clears are
valid states.

Coherent local prerequisites remain. These include Jasmine's medicine errand,
Misty's Power Plant and Route 25 sequence, Juan's weather story, Clair's Chuck,
Jasmine, and Pryce badge requirements, and Norman's four Hoenn badges and Wally
tutorial. Prerequisites must recognize already-satisfied facts on revisit and
must not depend on an exact global badge count.

The Johto Rocket takeover must use explicit regional story facts rather than
the historical exact-seven shared badge check. Whitney and Clair's deferred
rewards remain reachable and once-only at every global badge count and circuit
state. Wattson's New Mauville relocation still requires Norman's defeat and the
Dynamo Badge in either order. Preserve initial-badge protections for Chuck,
Giovanni or the currently selected Viridian owner, Clair, and Blaine.

## Opening and travel

The existing Johto opening initializes zero badges, no circuit clears, no
regional Champion/game-clear result, and Trainer Rating 0. Kanto and Hoenn
starts remain separate future work.

The S.S. Aqua maiden voyage remains available without a Ticket, awards the
Ticket during its reunion flow, and reaches state 8 on arrival at Vermilion.
State 8 plus the Ticket permits the Olivine, Vermilion, and Slateport circuit.

Numbered Sevii-island service remains available from Vermilion independently
of badges, circuit clears, the Rainbow Pass, Bill, Celio, the National Pokédex,
Trainer Tower, or a Sevii quest. Qualification controls only the hidden Masters
entrance inside its existing Seven Island house.

Every threshold-badge location must have a usable route to the eligible venue.
Reaching a venue cannot require another badge, the clear being pursued, or an
unrelated local story. Completion, loss, or voluntary exit must return the
player to a location connected to the wider travel network.

## Admission and order

First-clear admission uses these predicates:

| Stage | First-clear admission |
| --- | --- |
| Indigo | `globalBadges >= 8 && !indigoCleared` |
| Masters | `indigoCleared && globalBadges >= 16 && !mastersCleared` |
| Hoenn | `indigoCleared && mastersCleared && globalBadges == 24 && !hoennCleared` |

Badge origin, current map region, local story completion, another regional
Champion flag, and a generic game-clear flag cannot substitute for these facts.

After a stage's first clear, its venue offers replay admission without changing
the next required stage. Replay admission does not require completing later
stages. The venue must distinguish first-clear and replay runs before creating
the persistent run record.

Admission refusal states only the immediate unmet requirement. The Trainer
Card owns the complete overview.

## Indigo League

Use these existing maps and their linked layouts, scripts, audio, and tilesets:

```text
PokemonLeague_LoreleisRoom_Frlg
PokemonLeague_BrunosRoom_Frlg
PokemonLeague_AgathasRoom_Frlg
PokemonLeague_LancesRoom_Frlg
PokemonLeague_ChampionsRoom_Frlg
PokemonLeague_HallOfFame_Frlg
```

Enroll the authored FRLG Tier 1 parties for Lorelei, Bruno, Agatha, Lance, and
Blue through Wayfarer-owned Trainer IDs; raw FRLG Trainer IDs are source
provenance only because they collide with active HNS IDs. For the current
Johto-origin release, Blue always resolves the
`TRAINER_CHAMPION_FIRST_SQUIRTLE` source party, whose starter is Blastoise. Do
not dispatch this choice from `VAR_STARTER_MON`, which contains the Johto
starter. The future FRLG Kanto story port may resolve the same Wayfarer Blue ID
to the three FRLG starter-dependent source variants using Kanto-origin state
and owns origin-specific rival dialogue. Other origins still battle Blue as
Champion with the visitor default.

### Wayfarer map and content adaptation

Add the six named FRLG Indigo maps to the explicit Wayfarer selected-map
allowlist and dependency closure. Select only their required layouts, scripts,
audio, tilesets, objects, text, movements, Trainer presentation, and Hall of
Fame dependencies; do not enable general FRLG map or campaign content.

Under `IS_WAYFARER`, route the existing Indigo Plateau League entrance to
`PokemonLeague_LoreleisRoom_Frlg` instead of the HNS Will room. Adapt Lorelei's
back warp to the existing Wayfarer Indigo lobby at a validated walkable tile;
it must not target the unselected FRLG Pokémon Center. Adapt completion, loss,
voluntary exit, invalid-run recovery, and Hall of Fame return to the same lobby
or another explicitly named connected travel-network location. Make the six
source script tables available through narrow Wayfarer-owned entry points;
do not broadly widen `IS_FRLG` guards around unrelated scripts.

Clone the five Indigo battle positions into collision-audited Wayfarer Trainer
IDs. Lorelei through Lance each reference one FRLG source party. Blue uses one
Wayfarer runtime ID with a roster resolver: Blastoise for the current visitor
branch and, when the Kanto opening exists, one of the three reviewed FRLG
first-clear variants. Generate only the reachable party, class, portrait,
palette, text, item, music, and AI dependency closure. Never index the active
Wayfarer Trainer table with a raw FRLG opponent ID.

The first clear atomically:

1. records the canonical Indigo circuit clear;
2. establishes Champion and game-clear recognition for both Kanto and Johto;
3. performs one FRLG Hall of Fame registration and Champion Ribbon flow;
4. applies one +8 Trainer Rating contribution;
5. resets Indigo room/run state; and
6. returns the player to the travel network without full completion credits.

Kanto and Johto recognition may be projected into both existing regional
helpers, but neither flag is an independent circuit clear or reward input.
Clearing or resetting unrelated Hoenn content cannot change Indigo. Indigo
cleanup must not run a second regional Hall of Fame path.

An Indigo replay uses the same rooms and roster. It may run the ordinary
victory presentation needed to leave the chain, but it does not register
another circuit Hall of Fame record, award another Champion Ribbon, replay full
credits, or change a first-clear fact.

## Sevii Masters Challenge

### Public entrance

Reuse the registered maps:

```text
SevenIsland_House_Room1_Frlg
SevenIsland_House_Room2_Frlg
```

Do not register or add an entrance for `SevenIsland_UnusedHouse`.

Room 1 is the public Masters House. Replace its passive Wayfarer presentation
with a `masters` content-domain handler for the existing elderly former Trainer
and the existing box background event. Preserve the exterior warp, map layout,
old-woman graphics, coordinates, and local ID.

Before first-clear qualification, the caretaker states the immediate missing
badge or Indigo requirement and the box remains closed. When eligible, she
moves the box and selects the existing door-open layout. Room 2 becomes the
Masters antechamber and forwards an admitted run into the HNS room chain. It no
longer dispatches e-Reader or visiting-Trainer behavior in Wayfarer. Standalone
FRLG behavior remains unchanged.

After first clear, the caretaker offers replay admission. Declining, leaving,
losing, or abandoning a run changes no first-clear state.

### Challenge chain

Reuse these HNS maps:

```text
PokemonLeague_WillsRoom_hns
PokemonLeague_KogasRoom_hns
PokemonLeague_BrunosRoom_hns
PokemonLeague_KarensRoom_hns
PokemonLeague_ChampionsRoom_hns
PokemonLeague_HallOfFame_hns
```

The source maps remain `REGION_JOHTO` and `MAPSEC_INDIGO_PLATEAU` in
standalone HNS. Whenever any of the six maps is loaded in Wayfarer, apply a
Wayfarer-only map-context override: ordinary region dispatch resolves to
`REGION_KANTO`, the displayed map section resolves to Seven Island or Masters
Hall, and healing, blackout, Dig, Escape Rope, Pokédex-area, and invalid-run
recovery return through the Masters House rather than the HNS Indigo Plateau
lobby. This context does not depend on a valid active-run record. Adapt the
Will-room back warp to
`SevenIsland_House_Room2_Frlg` at a validated walkable tile. Do not edit the
shared HNS map metadata or change standalone HNS behavior.

Enroll the authored Tier 2 parties for Will, Koga, Bruno, Karen, and Lance.
Presentation identifies them as invited Masters. Lance is the final Master and
may be described as a former Champion; Blue remains current Indigo Champion.
Do not author replacement parties merely to avoid Bruno, Koga, or Lance
appearing elsewhere in the open world.

Rename the final room in Wayfarer presentation to `Masters Gallery`. Preserve
its map asset and transition role, but bypass regional Hall of Fame recording,
Champion Ribbon award, regional cleanup, game-clear state, and credits.

The first clear atomically:

1. records the dedicated Masters circuit clear;
2. applies one +8 Trainer Rating contribution;
3. resets HNS room/run state;
4. returns the player to the Masters House; and
5. leaves Seven Island harbor and all independent Sevii content available.

Masters completion must not start, finish, or inspect Celio's repair, Lostelle,
Lorelei's Icefall confrontation, Selphy, Tanoby Key, Trainer Tower, Moltres, the
Rocket Warehouse, or another Sevii objective.

### Sevii content ownership

Extend `game/src/data/wayfarer_sevii_maps.json` with a selectable `masters`
domain owned by this specification. It may replace the currently projected
passive old-woman script, the box event, and the empty Room 2 handlers. The
manifest must name each displaced baseline or passive record and prove that
the exterior warp and return path remain reachable.

The domain may override the existing actor's script and the two maps' selected
map/background handlers. It does not authorize a new actor, coordinate change,
layout byte edit, ferry change, or modification to another Sevii domain.
It may select the existing closed- or open-box layout through a reviewed
Wayfarer map-layout predicate; this is a selection between source layouts, not
a layout edit.

## Hoenn League and Red

Hoenn retains its Emerald room chain, authored Tier 3 roster, regional Hall of
Fame, Champion and game-clear state, cleanup, and first-clear +8 Rating
contribution. Its first clear records circuit completion and runs the full
completion credits. A replay grants no additional circuit reward or credits.

Red's Mt. Silver encounter requires `indigoCleared && mastersCleared &&
hoennCleared`. The predicate does not require a duplicate Kanto or Johto
League result because no such results exist. Preserve Red's authored encounter,
party, loss/retry, and one-time or repeat behavior unless its owning content
specification says otherwise. This circuit adds no Red reward.

## Persistent circuit state

Store canonical first-clear state separately from region identity:

```text
indigoCleared
mastersCleared
hoennCleared
```

The implementation may retain Hoenn's existing regional bit as the canonical
Hoenn fact if one write and one read authority are proved. Indigo must have one
canonical saved fact even if completion projects true into both Kanto and
Johto Champion helpers. Masters requires a dedicated saved fact in the
Wayfarer circuit or Sevii state bank.

Do not calculate canonical clears from a sum of regional Champion flags. Do not
store copied badge totals, reward counts, roster choices, or derived Rating.
New game initializes every clear false. Prerelease migration is not required.

First-clear recording is transactional. A failed ceremony, save interruption,
duplicate callback, or return-map load cannot award +8 without retaining the
corresponding clear, retain a clear without its reward fact, or consume the
wrong stage. The high-water Rating getter must recover the derived value from
the committed facts.

## Run lifecycle

The persistent active-run record stores:

- circuit stage;
- first-clear or replay mode;
- Trainer Rating captured at admission;
- active status; and
- enough venue identity to validate the saved room.

It does not store copied parties or derived opponent levels. Existing room
progression remains authoritative for defeated members.

1. Validate admission before locking the entrance or changing a room.
2. Capture stage, mode, and `GetTrainerRating()` once on successful admission.
3. Reset that venue's room progression only when starting a new run.
4. Every enrolled battle validates the matching run, room, and opponent.
5. Save/load preserves the snapshot and defeated-room progression.
6. A loss or voluntary exit clears the run and resets only that venue.
7. A final victory validates complete room progression before its ceremony.
8. The ceremony records a first clear only in first-clear mode, then ends and
   resets the run exactly once.

Loading an invalid or absent record inside a circuit room returns the player to
that venue's lobby without a clear or reward. Do not silently start a run or
capture live Rating midway through the chain. Indigo returns to its League
lobby, Masters to Room 1 of the Seven Island battle house, and Hoenn to its
native lobby.

An out-of-context debug battle may use its authored fallback but cannot create
a circuit run, defeat a room member, or award a clear.

## Opponent scaling

League scaling consumes the saved stage and `ratingAtEntry`. The stage selects
the authored Tier 1, Tier 2, or Tier 3 roster independently of geographic
region, player origin, badge distribution, current party, prior losses, and
replay mode.

Ace offsets remain -4, -3, -2, -1 for the ordered preliminary opponents and +1
for the final opponent. Supporting members remain one or two levels below the
ace according to the scaling specification. Clamp levels to 1 through 100.

Scaling changes effective levels only. Preserve source slot identity, species
and forms, party size, moves, items, abilities, IVs, EVs, natures, genders,
balls, AI, healing inventory, and battle type. Existing randomizer precedence
continues to apply.

A replay captures a fresh Rating snapshot and reconstructs the same authored
tier roster. It never selects a later tier or rematch party merely because the
stage was previously cleared.

## Trainer Rating

Let `b` be global badge count. Preserve the badge contribution:

```text
0 <= b <= 4:   4 * b
5 <= b <= 8:   16 + 6 * (b - 4)
9 <= b <= 24:  40 + (b - 8)
```

Add eight for each canonical first-clear fact:

```text
rating = badgeContribution
       + (indigoCleared  ? 8 : 0)
       + (mastersCleared ? 8 : 0)
       + (hoennCleared   ? 8 : 0)
```

Clamp the result to 80 and preserve high-water semantics. Regional Champion
projection, individual victories, losses, replay clears, repeated ceremonies,
Hall of Fame revisits, and Red add nothing.

Required milestones are:

| Facts | Rating | Soft cap where applicable |
| --- | ---: | ---: |
| No badges or clears | 0 | |
| 4 badges | 16 | |
| 8 badges | 40 | |
| 8 badges and Indigo clear | 48 | |
| 16 badges and Indigo clear | 56 | |
| 16 badges and Indigo plus Masters clears | 64 | |
| 24 badges and first two clears | 72 | |
| 24 badges and circuit complete | 80 | 100 |
| 24 badges and no clears | 56 | 62 |
| 24 badges and Indigo clear | 64 | 78 |
| 24 badges and Indigo plus Masters clears | 72 | 89 |

The HNS Chinchou learnset retains the existing Wayfarer additions of `Flash`,
`Surf`, and `Whirlpool` at level 5 and the established forty-entry adjustment.
This keeps the Johto start's native utility route available at Rating 0.

## Trainer Card and dialogue

The local Trainer Card status view displays:

```text
Badges: NN/24
Indigo League: <state>
  8 badges
Sevii Masters: <state>
  16 badges + Indigo clear
Hoenn League: <state>
  24 badges + Masters clear
```

Use `Locked`, `Available`, or `Cleared` for first-clear state. After Hoenn,
append `Circuit complete`. Red does not occupy a fourth status row.

Opening, badge awards, qualification, Hall of Fame returns, and unrelated Sevii
interactions produce no automatic circuit announcements. Venue dialogue may
state its immediate unmet requirement. Masters dialogue must not call the event
a League, call its winner Champion, or claim Lance is current Indigo Champion.

## Future Kanto story ownership

The future FRLG Kanto story port assigns the Earth Badge to Giovanni after its
named Rocket finale. It keeps exactly one initial Earth Badge and one badge/TR
contribution. Blue is Indigo Champion for every origin and a personal rival
only for Kanto-origin players.

This supersedes the older Blue Gym invitation and initial Earth Badge path when
that story port lands. The independent Cinnabar/Seafoam port may retain its
current Blue invitation until then. Neither version may add a local badge,
story, or origin requirement to Indigo admission.

The [Viridian finale specification](frlg-kanto-viridian-finale.md) selects the
full FRLG Gym, Giovanni's permanent departure, and no Blue successor or Gym
rematches. Blue's existing Dojo battle unlocks on the committed first Indigo
Champion victory, including when Indigo precedes Giovanni. A battle start or
loss does not unlock it. This reads Indigo's canonical first-clear fact and
does not change the shared eight-global-badge admission or add a circuit reward.

The [Viridian implementation](../research/viridian-finale-implementation.md)
projects Blue's Dojo visibility from `HasCommittedFirstIndigoVictory()` on each
Dojo entry. League commit and Hall of Fame rollback never touch the Dojo flag,
so a rolled-back first victory leaves Blue hidden.

## Validation

### Static and mechanics coverage

1. Prove global badge totals 0 through 24 from every regional distribution and
   reject duplicate badge awards.
2. Cover every first-clear and replay admission boundary, including 7/8,
   15/16, and 23/24 badges; missing prior clears; all-badges-first progression;
   and cleared-stage replay while a later stage remains locked or available.
3. Prove stage identity is independent of region: Masters maps remain
   Sevii/Kanto for ordinary systems while their battles resolve Tier 2.
4. Verify the exact five-member roster order and authored party identity for
   all three stages across every supported difficulty and randomizer boundary.
5. Cover every integer Rating 0 through 80, level offsets, interpolation,
   clamping, run snapshots, and fresh replay snapshots.
6. Prove Indigo projects both Kanto and Johto Champion recognition while adding
   one canonical clear and one +8 contribution. Masters changes neither
   regional Champion state. Hoenn changes only Hoenn's regional result.
7. Exercise transactional first-clear recording, duplicate ceremony callbacks,
   interrupted returns, and derived Rating recovery.
8. Verify the Trainer Card text and states for every threshold, first clear,
   replay availability, circuit completion, and mixed regional badge order.
9. Update static consumers that still name Kanto and Johto League clears,
   regional Tier 2 dispatch, consecutive Indigo tiers, or three Hall of Fame
   callbacks. No live circuit path may retain those assumptions.

### Map and script coverage

1. Enter Seven Island's registered battle house from its existing exterior
   warp before and after qualification. Confirm its ordinary return warp and
   island services remain reachable.
2. Verify the old woman and box use exact retained source identities and that
   the `masters` domain conflicts with no story, ordinary-Trainer, or Tower
   record.
3. Exercise the closed box, each refusal, accepted admission, Room 2 transition,
   every HNS room, Masters Gallery, loss at every opponent, voluntary exit,
   blackout, save/load, invalid-run recovery, first-clear return, and replay.
4. Confirm no e-Reader or visiting-Trainer route is reachable in Wayfarer and
   standalone FRLG retains its source behavior.
5. Exercise FRLG Indigo and Emerald Hoenn with equivalent run-lifecycle cases,
   including one Hall of Fame registration where required and none at Masters.
6. Confirm Indigo and Masters never run full completion credits; Hoenn first
   clear does, and a Hoenn replay does not.
7. After circuit completion, reach and battle Red at Mt. Silver. Before each
   missing clear, prove the final encounter remains unavailable without
   changing the rest of Mt. Silver.

### Regional journey coverage

Play a Johto-start journey to all twenty-four badges with zero clears. Include
mixed regional order, deferred Whitney and Clair rewards, the selected Viridian
badge owner, Wattson/Norman in both orders, and the audited Rocket takeover
predicate. Then clear Indigo, Masters, and Hoenn consecutively and reach Red.

Repeat representative routes by taking each stage at its minimum badge count.
Confirm transport from every threshold-badge region, return to ordinary travel,
and the exact Rating milestones. Record live emulator evidence separately from
mechanics and source audits.

Build Wayfarer and the affected mechanics/E2E configuration. If shared engine
or source scripts change, compile standalone Emerald, FireRed, LeafGreen, and
HNS and confirm their native League and Seven Island behavior remains intact.
Run map-version builds serially because generated map files are shared.

## Implementation evidence

Validated on 2026-09-25 with the production Wayfarer ROM and E2E-enabled ROM.
The live emulator circuit journey covers first clears and replays across FRLG
Indigo, Sevii Masters, and Hoenn; save/reload recovery; loss recovery; Rating
snapshots; the single Indigo Hall of Fame commit; and Red after circuit
completion. A separate Hoenn-origin journey covers admission to all three
venues without the maiden voyage. Native mechanics include a fault-injected
Indigo Hall of Fame save rollback, Red's authored completion handoff, circuit
admission/persistence/status tests, exact League roster and tier tests, and the
map, traversal, Sevii-content, Trainer-scaling, and generator audits.

## References

- [Product requirements](../prds/wayfarer-interregional-league-circuit.md)
- [League scaling](league-scaling.md)
- [Runtime foundation](wayfarer-runtime-foundation.md)
- [Sevii content overlay](sevii-content-overlay.md)
- [Sevii Trainer Tower](sevii-trainer-tower.md)
- [FRLG Kanto story port](../prds/frlg-kanto-story-on-hns-maps.md)
- [Trainer Rating party progression](trainer-rating-party-progression.md)

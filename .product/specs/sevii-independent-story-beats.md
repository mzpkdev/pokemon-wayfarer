# Sevii independent story

PRD: [Sevii independent story beats](../prds/sevii-independent-story-beats.md)

Implemented: No

## Scope and authority

This specification restores the ordinary FRLG Sevii adventures, quest actors,
dialogue, scripted encounters, items, gifts, trades, tutors, and local rewards
on Wayfarer's explorable Sevii maps. It adapts their dependencies so each local
adventure can be discovered independently while preserving its internal order.

The [Sevii content overlay](sevii-content-overlay.md) owns event selection,
script linkage, state ownership, and generated audits. Ordinary route Trainers
and Trainer Tower belong to their separate specifications.

The merged exploration behavior remains authoritative. Story never grants,
revokes, or gates the numbered-island ferry; never disables PC storage; and
never requires Blaine, a mainland League clear, the National Pokédex, or the
Ruby to visit an island.

## Story entry points

Restore the original Bill and Celio objects in the One Island Pokémon Center as
Wayfarer-owned actors. They offer independent work through direct conversation;
arrival on One Island starts no quest automatically.

- Bill introduces himself when necessary and offers the Meteorite delivery.
  Accepting it adds the actual Meteorite before recording receipt. Declining or
  lacking Key Item capacity leaves the offer available. This interaction does
  not require or advance Blaine, Cinnabar, Bill's mainland rescue, or travel.
- Celio offers the two-gem repair investigation. Accepting it opens both the
  Ruby and Sapphire leads together. It does not grant a pass, change PC access,
  set Champion state, or change Pokédex eligibility.
- Bill and Celio retain completion-conditioned dialogue. Neither actor blocks
  the Center desk, PC, stairs, door, or ferry route.

This settles the PRD's entry-point question: Bill in the One Island Center owns
the Meteorite; Celio in the same Center owns the gem investigation. The two
offers may be accepted in either order.

## Saved state

Allocate the following symbolic state in the Wayfarer Sevii namespace. Numeric
IDs are generated only after a collision audit.

| Objective | Required state |
| --- | --- |
| Lostelle | `LOSTELLE_STARTED`, `BIKERS_CLEARED`, `LOSTELLE_FOUND`, `LOSTELLE_RESCUED`, `IAPAPA_BERRY_RECEIVED`, `THREE_ISLAND_FULL_RESTORE_RECEIVED` |
| Meteorite | `METEORITE_RECEIVED`, `METEORITE_DELIVERED`, `MOON_STONE_RECEIVED` |
| Lorelei | `LORELEI_STARTED`, `LORELEI_ROCKETS_DEFEATED`, `LORELEI_COMPLETE` |
| Selphy | `SELPHY_FOUND`, `SELPHY_DEFEATED`, `SELPHY_RETURNED`; requested-species index, pending-reward item ID, and request-active flag |
| Memorial Pillar | `TECTONIX_OFFERING_COMPLETE`, `TECTONIX_REWARD_RECEIVED` |
| Water Labyrinth | `EGG_OFFERED`, `EGG_RECEIVED` |
| Gem investigation | `CELIO_GEMS_STARTED`, `RUBY_RECOVERED`, `RUBY_DELIVERED`, `PASSWORD_ONE_LEARNED`, `SAPPHIRE_FOUND`, `SAPPHIRE_STOLEN`, `PASSWORD_TWO_LEARNED`, `WAREHOUSE_CLEARED`, `SAPPHIRE_RECOVERED`, `SAPPHIRE_DELIVERED`, `CELIO_REPAIR_COMPLETE` |
| Rival | `SEVII_RIVAL_SCENE_SEEN` |
| Moltres | `MOLTRES_RESOLVED` |

Use the existing Wayfarer Tanoby completion flag rather than creating a second
Tanoby state. Preserve source defeat flags only through generated Wayfarer
aliases with unique storage. Do not infer receipt from an unrelated campaign
flag or from currently owning an item that can be consumed elsewhere.

Every state transition is enumerated in the map manifest with its script
caller. The audit rejects a transition not listed below or an objective that can
skip a required predecessor.

## Lostelle and the Meteorite

Restore Lostelle's father in Two Island's Game Corner. Talking to him while the
rescue is incomplete starts or resumes the local search whether or not the
player has met Bill or owns the Meteorite.

1. The father starts the search.
2. Three Island stages the four original ordered battles: Biker Goon, Biker
   Goon 2, Biker Goon 3, then Cue Ball Paxton.
3. Clearing the bikers opens the investigation path to Bond Bridge and Berry
   Forest without changing ferry access.
4. Lostelle's scripted Hypno encounter starts only from her interaction.
5. Defeating or catching the level-30 Hypno records `LOSTELLE_FOUND`; running,
   losing, or an interrupted start leaves the encounter available. Then attempt
   the source Iapapa Berry gift; a full pocket leaves the Berry pending without
   replaying Hypno.
6. The reunion records `LOSTELLE_RESCUED` before changing actor presentation.

The rescue does not require the Meteorite. Before rescue completion, the father
acknowledges an owned Meteorite but does not consume it. After the reunion he
accepts the actual item, removes it only when the transaction can complete,
records delivery, and offers the original Moon Stone reward. A full Items
pocket leaves the Moon Stone claim available without replaying the reunion or
consuming another Meteorite. Preserve the Three Island defender's original Full
Restore as a separate post-biker claim; a full pocket leaves that claim pending.
Each receipt is independent and one-time.

If the player rescues Lostelle first, Bill's later offer and the delivery still
work. Accepting Bill's errand never resets the bikers, Lostelle, or reunion.

## Celio's two-gem investigation

Celio starts one investigation with two simultaneously available leads.

### Ruby lead

- Remove the source Champion, National Pokédex, and network-repair gates.
- Restore the Mt. Ember Rocket guards, their battles, the Ruby chamber, the Ruby,
  and the first Warehouse password.
- The guards are objective battles. A loss returns through normal recovery and
  leaves the pending actor, approach, and password state intact.
- The Ruby transaction records recovery only after the Key Item enters the Bag.
- Learning the first password is a separate state and remains meaningful if the
  Ruby is delivered immediately.

Moltres is independent of the guards, Ruby, and Celio completion.

Restore the three Five Island Meadow Rocket Trainers and the one Outcast Island
Rocket Trainer as story-owned local confrontations after Celio's investigation
starts. They remain optional, do not teach either Warehouse password, and hide
after Warehouse completion. Their victories use independent defeat state and
do not advance Ruby or Sapphire recovery.

### Sapphire lead

- Celio's accepted request makes Dotted Hole's existing Cut entrance and floor
  route lead to the restored Sapphire scene.
- The scientist appears in the Sapphire room only after the gem request starts
  and before the theft is complete.
- The player discovers the actual Sapphire before the scientist steals it. The
  scene records discovery, theft, and the second password in that order.
- Leaving before the scene commits keeps the Sapphire room pending. No early
  Warehouse visit can create or recover an unstolen Sapphire.

### Rocket Warehouse

The exterior door remains physically open for exploration, as in the merged
baseline. Story progress, not collision, controls the confrontation:

| State | Warehouse behavior |
| --- | --- |
| Sapphire not stolen | Empty exploration; no gem or story actors |
| Sapphire stolen, fewer than two passwords | Grunts may give local locked-operation dialogue, but the confrontation cannot advance |
| Sapphire stolen, both passwords learned | Restore the ordered Rocket battles, administrators, Gideon, and Sapphire recovery |
| Warehouse cleared | Show completion dialogue and no repeat battles or duplicate Sapphire |

Victory over each required opponent commits its own defeat state. Loss or
departure resumes at the first undefeated opponent. Gideon's victory writes
`WAREHOUSE_CLEARED`, then attempts the Sapphire recovery transaction. If the Key
Items pocket is full, Gideon remains available to hand over the Sapphire; the
ordered battles do not replay.

Use two administrator-ending text variants:

- when the shared mainland-Giovanni completion predicate is true, retain the
  recognition that Giovanni has been defeated and Team Rocket disbanded;
- otherwise, state only that the Sevii operation is abandoned after this loss.

The implementation supplies one semantic predicate rather than reading a raw
FRLG flag. When the corresponding mainland Wayfarer milestone is absent, the
predicate is false. Neither variant writes mainland story state.

### Deliveries and repair

Celio accepts either recovered gem first. For each delivery, verify actual item
ownership, remove that item, record its delivery, and select dialogue naming the
one still missing. The final repair completes only after both deliveries.

Celio's completion retains the local repair presentation and acknowledgement.
It does not enable trading, alter communication services, grant a Pokédex,
unlock Cerulean Cave, set Champion or game-clear state, or change another region.
These exclusions settle the PRD's communications-effects question.

## Independent local adventures

### Lorelei and Icefall Cave

Restore Lorelei and the three Rocket actors in Icefall Cave without requiring
Celio, either gem, Giovanni, or a League clear. Preserve the cave traversal and
the source battle against Rocket Grunt 45. Loss or departure leaves that battle
and all actors needed to continue pending.

Lorelei's completion dialogue has two variants. Before Warehouse completion she
may point the player toward the operation; afterward she acknowledges that it is
already resolved. Her rescue never opens Dotted Hole, teaches a password, or
changes Warehouse completion.

### Selphy and Lost Cave

Restore Selphy at the end of Lost Cave. Her battle is an objective battle;
winning records defeat and stages her return. Loss, escape from the cave, or
blackout leaves her available without resetting the directional puzzle.

After her return, restore the original Pokémon-showing requests as repeatable
local interactions. Freeze the source request pool and reward table in the map
manifest. Check the shown species and reward capacity before committing
each request result; a failed reward delivery remains claimable.

### Memorial Pillar and Water Labyrinth

Restore the Tectonix memorial interaction. Consume Lemonade only when the
offering is accepted, then grant the original source reward through a separate
receipt. A full destination pocket preserves the reward claim.

Restore the Water Labyrinth Egg giver. Preserve the source friendship test and
Togepi Egg. Do not add a Trainer Rating or story gate. When the party is full,
retain the offer until a party slot is free; do not redirect the Egg to the PC.
Record receipt only after the Egg exists in the party.

### Tutors, trades, and passive island actors

Restore locally usable tutors, trades, Pokémon-showing interactions, signs, and
flavor actors whose complete dependencies fit the Wayfarer overlay. Preserve
their source prerequisites unless those prerequisites are one of the removed
campaign gates named by this specification. One-time moves, trades, gifts, and
rewards use independent receipts.

The story inventory classifies every stripped non-Trainer source actor on the 135
maps as restored or explicitly excluded. Do not silently omit an actor because a
dependency was inconvenient to link.

## Rival scene

Restore the shared later-island rival scene only after the player has cleared at
least one Wayfarer League. Early arrival at Four or Six Island does nothing and
does not consume the scene. Once eligible, the first visit to either source
location plays its location-appropriate version and sets the shared
`SEVII_RIVAL_SCENE_SEEN` flag. The other location then uses post-scene dialogue.

The two source variants are non-battle cutscenes. The rival scene does not start
or gate a local adventure. Use the player's established Wayfarer rival identity;
do not infer Red/Leaf identity from the imported FRLG map or allocate a rival
Trainer party for this scene.

## Moltres

Restore Moltres at the Mt. Ember summit. Use the shared bird threshold
`WAYFARER_BIRD_CAPTURE_TR = 55`, which also governs future Articuno and Zapdos
restoration.

- Below TR 55, interaction explains that the Pokémon is too dangerous and ends
  without starting a battle or hiding Moltres.
- At TR 55 or above, start the original static encounter with its source species,
  level, and presentation.
- Catching or knocking out Moltres sets `MOLTRES_RESOLVED` and hides it.
- Running, losing, or failing to start the encounter leaves it available.

TR is not consumed. The encounter does not require the Ruby, Celio's repair, or
either other bird.

## Battle ownership

Story Trainers retain source parties, battle types, AI, items, and prize money.
Classify Rocket grunts that exist only as independent guards as ordinary scaling
when the trainer-scaling inventory supports their exact IDs. Classify bikers,
administrators, Gideon, Selphy, and Lorelei's opponent as story-owned and
exclude them from automatic ordinary scaling. Record every battle
classification explicitly.

All objective battles commit progression only after a win. A native loss or draw
performs ordinary Wayfarer blackout/recovery and returns the objective to a
stable pending state. Post-battle dialogue or rewards never run on loss. No story
scene enables the temporary empty-party trainer-only encounter flag.

## Generated audit

The story section of `wayfarer-sevii-content-audit` reports:

- every source and restored actor, script, movement, text, state transition,
  battle, static encounter, item, Pokémon, tutor, trade, and reward;
- objective graphs with all legal orders and no skipped predecessor;
- every capacity check, transaction order, and one-time receipt;
- all story-battle classifications and win/loss continuation targets;
- all dialogue variants and their predicates;
- the exact Moltres threshold and encounter outcomes; and
- explicit exclusions for removed campaign/network behavior.

Fail on an unowned source actor, story write outside the namespace, reward
without a receipt, item consumption before success, impossible continuation,
travel-state write, or source-party mutation.

## Validation

Add mechanics and emulator coverage for:

1. Lostelle before and after accepting the Meteorite, with every reunion and
   reward-capacity order.
2. Ruby-first and Sapphire-first investigation, both gem-delivery orders, all
   four Warehouse password states, and Warehouse completion before/after
   mainland Giovanni.
3. Loss, blackout, ferry departure, save/reload, and re-entry at every objective
   battle and transaction boundary.
4. Lorelei before/during/after Warehouse completion.
5. Selphy loss and return, request success, wrong species, and full reward pocket.
6. Memorial and Egg capacity failures without duplicate consumption or receipt.
7. Rival visits to Four and Six Island in both orders before and after League
   eligibility.
8. Moltres at TR 54 and 55, then catch, knockout, run, loss, save, and reload.
9. Unchanged ferry, PC, wild encounters, environmental puzzles, ordinary
   Trainers, Trainer Tower, Birth Island, Navel Rock, and standalone builds.

Run map-manifest tests, `wayfarer-sevii-content-audit`, existing Sevii
audits, `make -C game check`, serial product builds, and a production-equivalent
Wayfarer release. Record the ROM delta after each bounded objective group.

## Delivery milestones

1. Lostelle, bikers, Hypno, Meteorite, reunion, and Moon Stone.
2. Lorelei and Icefall Cave.
3. Selphy, Memorial Pillar, Water Labyrinth, tutors, trades, and local actors.
4. Celio request, Ruby branch, Sapphire theft, Warehouse, deliveries, and repair.
5. Rival scene and Moltres.

Each milestone must be independently playable and must leave later objectives
absent rather than half-linked.

## References

- [Sevii content overlay](sevii-content-overlay.md)
- [Sevii Trainer restoration](sevii-trainer-restoration.md)
- [Trainer-only story encounter audit](../research/trainer-only-story-encounter-gates.md)
- [Trainer party scaling](trainer-party-scaling.md)
- [Sevii exploration map port](sevii-exploration-map-port.md)

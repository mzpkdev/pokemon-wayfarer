# Sevii ordinary Trainer restoration

PRDs: [Sevii exploration port](../prds/sevii-exploration-port.md) and
[Sevii independent story beats](../prds/sevii-independent-story-beats.md)

Implemented: No

## Scope and authority

Restore every ordinary FRLG Trainer on the 135-map Wayfarer Sevii exploration
catalog, including their source placement, approach behavior, dialogue, party,
defeat persistence, and rematches. This specification owns ordinary Trainer
selection and behavior. The [Sevii content overlay](sevii-content-overlay.md)
owns projection, generated IDs, state isolation, source provenance, and audits.

Story encounters and Trainer Tower facility opponents are not ordinary
Trainers. Restoring this domain must not start, complete, reveal, or gate an
island adventure; change ferry service; alter wild encounters; or enable the
temporary trainer-only wild mechanic.

## Frozen inventory

The source inventory contains 87 ordinary Trainer objects and 81 unique base
Trainer IDs:

- 75 single-battle objects and 12 objects forming six paired Double Battles;
- 86 sight-based objects and one talk-to-battle object, Fisherman Tommy;
- 70 objects whose source scripts offer a rematch and 17 single-stage objects;
- authored base parties of one to five Pokémon at levels 33 through 52; and
- two ordinary exterior psychics, Dario and Rodette, at Seven Island Trainer
  Tower. They remain in this domain; no exterior battle uses facility rules.

The map manifest must enumerate every object individually with its exact source
map, source index, local ID, coordinates, movement, sight radius, script,
Trainer identity, dialogue labels, pair identity, and rematch family. It must
also freeze normalized hashes of every selected base and rematch party. A
checked-in generated report groups the inventory by island and map; the numeric
totals above are release assertions, not a substitute for that exhaustive list.

The 17 non-rematch objects are the three Mt. Ember exterior Trainers, two Lost
Cave room Trainers, and twelve Pattern Bush Trainers. All other selected objects
retain their source rematch path.

## Explicit exclusions

Reject these source `TRAINER_TYPE_NORMAL` objects from the ordinary domain:

- Rocket Warehouse: Grunts 42, 47, and 48; both administrators; and Gideon;
- Five Island Meadow's three Rocket opponents;
- Six Island Outcast Island's Rocket opponent; and
- every dynamically selected Trainer Tower floor opponent.

Those Rocket battles are story-owned, including the Meadow and Outcast Island
encounters. Trainer Tower floor opponents are facility-owned. Bikers, Lorelei's
opponents, Selphy, rivals, and static Pokémon are likewise outside this domain
even where their scripts resemble normal battles.

## Projection and scripts

Tag each selected event `owner: ordinary_trainer` in schema-v2
`wayfarer_sevii_maps.json`. Preserve its source local ID, coordinates, graphics,
movement, trainer type, sight radius, and pair layout. Override only its script
and persistent defeat flag with Wayfarer-owned symbols.

For a sight Trainer, the first executable command in the event entry must be
the battle command so battle setup can resolve the Trainer from the object
script pointer. The replacement then preserves the source intro, defeat, and
post-battle text. Fisherman Tommy remains talk-to-battle. Do not add movement
scenes, items, gifts, story writes, or callbacks to an ordinary wrapper.

Paired objects are atomic manifest records sharing one battle and one defeat
state. Both source objects must be present, adjacent as authored, visible or
hidden together, and use the same generated Trainer identity. If the player
lacks two usable non-Egg Pokémon, preserve the engine's normal Double-Battle
denial and leave the pair undefeated.

## Trainer IDs, parties, and defeat state

Source FRLG Trainer numbers are provenance only because they collide with
Wayfarer's active HNS and Emerald rosters. After auditing the complete active ID
space, reserve a fixed contiguous range below partner ID 2048 and generate one
stable `TRAINER_WAYFARER_SEVII_*` ID for every selected base and rematch party.
Do not calculate the base from the current Trainer count.

Generate a dense selected roster from `trainers_frlg.party` and the selected
Trainer records. Preserve source name, class, gender, sprite, music, AI flags,
items, prize money, party order, species, moves, held items, ability choices,
IVs, and authored levels. Never link all 624 FRLG Trainer records or use a raw
`TRAINER_*` FRLG constant at runtime.

Set `TRAINERS_COUNT_WAYFARER` to one past the highest populated generated ID.
Every populated ID receives exactly one scaling classification. Allocate a
dedicated Sevii defeat bitset and route its exact ID range before the existing
appended-HNS remap. Base defeat flags persist across maps, ferry travel,
blackout, saving, and reload. Rematch availability is separate from base defeat
state.

## Scaling

Classify every selected ordinary base and rematch party `ORDINARY` in the
existing Trainer scaling inventory. Extend its generator to read only the
selected FRLG party records with the same product defines used for Wayfarer.
Apply the current ordinary-Trainer policy at battle construction; do not
pre-scale or rewrite the checked-in source parties.

The generated audit tests representative low, equal, and high Trainer Rating
values against every selected party and proves legal levels, moves, evolutions,
and party size under the existing scaling contract. It also proves that
excluded story and facility IDs do not enter ordinary scaling.

## Defeat, loss, and rematches

A first victory sets only the generated base defeat bit and shows source
post-battle dialogue. A loss or draw follows normal Wayfarer blackout and money
rules, leaves the Trainer undefeated, and never executes victory text or a
story continuation. Returning to the map resumes the same encounter.

Add a Sevii rematch registry for the 70 rematch-capable objects. It maps each
generated base ID to its complete selected FRLG rematch chain and participates
in the existing Wayfarer Vs. Seeker flow. Preserve source rematch party order
and dialogue, but remove any Champion, National Pokédex, Sevii pass, or campaign
gate: once the base Trainer is defeated, ordinary Vs. Seeker rules alone govern
eligibility. The 17 single-stage objects never advertise a rematch.

For paired Trainers, the rematch registry advances and battles the pair as one
unit. Base victory and every rematch use the same persistent object presentation;
rematch victory does not create a second map defeat flag.

## Generated audit

The ordinary-Trainer section of `wayfarer-sevii-content-audit` reports and
asserts:

- 87 selected objects, 81 unique base IDs, 75 single objects, six complete
  pairs, 86 sight objects, one talk object, 70 rematch-capable objects, and 17
  single-stage objects;
- exact source-event and script hashes, generated event identity, pair
  membership, sight radius, dialogue, base party, and complete rematch chain;
- unique generated IDs below 2048, dense selected data, and no raw FRLG runtime
  IDs or full FRLG Trainer table;
- one `ORDINARY` classification for every populated ordinary party ID and one
  valid defeat-bit mapping for every base Trainer identity;
- no selected story or facility opponent, story-state access, item transaction,
  movement scene, or unowned helper; and
- all assets and symbols reachable from the selected closure and no others.

Fail on a changed source event or party, an incomplete pair, a missing rematch
row, an unknown classification, duplicate ID, defeat-bit alias, changed
placement, source campaign dependency, or inventory-count drift.

## Validation

Add schema and generator tests for every failure above. Mechanics tests cover
first victory, loss, draw, blackout, post-battle talk, save/reload, Vs. Seeker
eligibility, every rematch stage, and paired eligibility. Exercise every party
through the ordinary scaler at low, equal, and high Trainer Rating.

Emulator journeys cover at least:

- Kindle Road sight and Fisherman Tommy talk encounters;
- the Crush Kin pair and a pair with only one usable Pokémon;
- one no-rematch Trainer from Mt. Ember, Lost Cave, and Pattern Bush;
- ordinary Trainers on Three, Five, Six, and Seven Island;
- Dario and Rodette outside Trainer Tower without facility routing;
- a base fight and full rematch chain; and
- loss, ferry departure, map reload, save/reload, and return after defeat.

Run the overlay and scaling audits, `make -C game check`, serial supported-product
builds, and a production-equivalent Wayfarer release. Standalone FRLG keeps its
original roster and scripts; HNS and Emerald builds contain no generated Sevii
Trainer IDs, parties, defeat bits, or rematch registry.

## References

- [Sevii content overlay](sevii-content-overlay.md)
- [Sevii independent story](sevii-independent-story-beats.md)
- [Sevii Trainer Tower](sevii-trainer-tower.md)
- [Trainer party scaling](trainer-party-scaling.md)
- [Sevii exploration map port](sevii-exploration-map-port.md)

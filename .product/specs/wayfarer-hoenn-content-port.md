# Wayfarer Hoenn content port

PRD: [Wayfarer Hoenn integration](../prds/wayfarer-hoenn-integration.md)
Implemented: No

## Scope

This specification defines the Hoenn content included in Wayfarer and the rules
for making that content work with the HNS engine. It covers maps, NPCs, items,
Trainers, wild encounters, Gyms, the Emerald main story, the Pokémon League,
and content-completeness validation.

The runtime foundation owns map generation, persistent storage, regional state
isolation, and the ROM budget. The regional travel specification owns first
entry, the adapted Birch rescue, inter-region travel, Town Map and Fly behavior,
healing, and whiteout recovery. The implemented Emerald open-world traversal
specification remains authoritative for Hoenn's opening settlement network.

## Behavior

### Required Hoenn content

Wayfarer includes the Emerald versions of the following content:

- all maps, layouts, connections, warps, tilesets, map scripts, and map events
  needed to traverse Hoenn and complete its main campaign;
- all ordinary towns and cities, including their accessible interiors;
- the routes, caves, sea routes, hideouts, dungeons, and League maps used by
  the main campaign;
- map NPCs, dialogue, shops, Pokémon Centers, Marts, item balls, hidden items,
  gifts, and in-game trades on those maps;
- ordinary Trainers, rivals, Team Aqua and Team Magma battles, Gym Leaders,
  Elite Four members, and the Champion;
- authored land, water, Rock Smash, and fishing populations; and
- the Emerald main story from the adapted Route 101 opening through the Hoenn
  Hall of Fame.

Every included map uses its Emerald layout, tilesets, music, weather, object
events, scripts, and connections unless this specification, the Wayfarer entry
specification, or the existing Emerald traversal specification names a change.

### Content manifest

A generated Wayfarer Hoenn manifest is the completeness authority. For every
Hoenn map it records:

- map group, map number, layout, primary and secondary tileset;
- map script and event-table presence;
- warp and connection targets;
- object, coordinate, background, and warp event counts;
- heal and Fly destinations where applicable;
- referenced Trainer IDs;
- available wild encounter methods; and
- whether the map is required, optional, or intentionally excluded.

No required entry may be omitted silently. A missing required asset, script,
Trainer, encounter profile, map target, or persistent constant fails the
content audit. Every intentional exclusion appears in the manifest with its
reason and the specification that owns the decision.

### NPCs, items, shops, and interactions

Hoenn NPCs retain their Emerald dialogue, movement, graphics, schedules, gifts,
shops, trades, and ordinary interaction behavior unless another approved
Wayfarer rule requires an adaptation.

Every one-time interaction commits its reward and persistent state together:

- An item ball or hidden item disappears only after the item is delivered.
- A gift remains available when the party and storage cannot accept it.
- A full Bag leaves an item reward available to retry.
- A one-time trade cannot be repeated after its result is saved.
- An NPC hidden by story progression remains hidden after travel, save and
  reload, blackout, and another region's League completion.

Shops use the HNS Bag and money systems. Pokémon Centers use the shared party
and storage systems. Entering a Hoenn facility cannot replace the player,
Pokédex, Bag, party, storage, money, options, or play time.

### Trainer registry

Existing HNS Trainer IDs remain unchanged. Every Hoenn Trainer referenced by an
included map or script receives a stable, noncolliding Wayfarer Trainer ID. A
generated mapping rewrites or resolves every Hoenn Trainer reference to that
ID.

Trainer records follow these rules:

- Species, levels, moves, held items, AI, party size, battle type, and double-
  battle behavior match the Emerald-authored record.
- The same authored party is selected regardless of the player's Trainer
  Rating or HNS difficulty option.
- Ordinary Trainer object coordinates, movement types, trainer types, and sight
  ranges remain unchanged.
- A traversal change may move a scripted story actor, but it cannot silently
  move or shorten the sight range of an ordinary Trainer.
- Defeat state uses the dedicated Wayfarer Trainer bitset rather than a flag
  calculated from the global Trainer ID.
- Defeating a Hoenn Trainer cannot mark an HNS Trainer defeated, and the reverse
  is also forbidden.
- Saving and reloading preserves each defeat result.

Rematches are included only where the required campaign or an already
compatible Emerald interaction uses them. Full Match Call compatibility is an
optional system and does not block the required Hoenn campaign milestone.

### Wild encounters

Each included Hoenn map receives Wayfarer encounter profiles for every method
authored for that map in Emerald:

- land;
- water;
- Rock Smash;
- Old Rod;
- Good Rod; and
- Super Rod.

The source species, slot ordering, method weights, and encounter rates remain
the Emerald values. Profile generation targets Wayfarer explicitly. A profile
must not disappear merely because its original label belonged to the Emerald
build.

Ordinary wild Pokémon pass through the existing HNS Trainer Rating level
projection. Hoenn badges, Hoenn story variables, and Hoenn Champion state do
not add new Trainer Rating inputs. The effective population exposed to normal
encounters is also exposed to systems such as DexNav and Pokédex area data when
those systems already consume ordinary encounter profiles.

The following encounters retain their authored level and do not enter ordinary
wild scaling unless an existing approved specification already says otherwise:

- gifts and starters;
- static and scripted encounters;
- legendary encounters;
- hidden encounters;
- roaming encounters; and
- battle-facility or other special populations.

Tests must cover at least one Hoenn profile for every encounter method at
several Trainer Ratings, including the minimum and maximum production rating.
The population before level projection must match Emerald exactly.

### Existing open-world progression

The implemented Emerald open-world regional traversal specification applies to
Wayfarer Hoenn maps. Its public Route 104 ferry, road lanes, Mt. Chimney states,
Route 111 survey, Route 120 Kecleon transaction, native Surf crossings, and
early-arrival rules remain unchanged unless the Wayfarer travel specification
defines a cross-region addition.

Using an open travel lane does not complete its attached Emerald story. A
player may visit the opening-network settlements first, return to Route 101,
and complete every preserved story afterward. Each original battle, item,
dialogue, and aftermath remains available once.

Sootopolis stays outside the opening settlement network. Its Dive entrance,
weather crisis, Cave of Origin scenes, Gym, and related rewards retain their
late-game Hoenn progression.

### Main campaign and regional progression

The Emerald main campaign remains completable in its intended order after the
adapted Wayfarer introduction, including:

- Norman and Wally's Petalburg events;
- the Devon, Peeko, Letter, and delivery sequences;
- Team Aqua and Team Magma encounters and hideouts;
- rival and Steven encounters;
- Mt. Chimney, the Weather Institute, Mt. Pyre, and the submarine sequence;
- Mossdeep, Seafloor Cavern, the weather crisis, and the Cave of Origin;
- all eight Hoenn Gyms;
- Victory Road, the Elite Four, the Champion, and the Hall of Fame.

Hoenn story scripts use only Hoenn variables and flags for Hoenn progress. A
Johto or Kanto badge, Champion result, global game-clear flag, Trainer defeat,
item event, or NPC state cannot satisfy a Hoenn campaign check by numeric
coincidence.

Hoenn has eight badge results. Gym scripts award and check the corresponding
Hoenn badge. Badge-count checks such as Norman's availability and League entry
count only Hoenn badges. Trainer cards or badge displays that show regional
progress must identify the region rather than treating all badges as one
contiguous array.

Hoenn badges and story milestones do not alter HNS Trainer Rating. HNS
field-move rules remain active in Hoenn. Story checks that deliberately require
a Hoenn badge still use the Hoenn badge, but ordinary field-move execution does
not restore Emerald's badge or HM-item ownership requirements.

Dive in Hoenn and Whirlpool in HNS remain separate usable field moves. Obtaining
or using one cannot replace the other, consume the other's item, or satisfy the
other's script check.

### Hoenn League and game clear

The Hoenn League uses a Hoenn-specific Champion and game-clear result.
Completing it must:

1. record the Hoenn Hall of Fame entry;
2. set Hoenn Champion state;
3. run only the Hoenn League and story cleanup;
4. preserve Johto and Kanto campaign state;
5. show the intended Hall of Fame and credits presentation; and
6. return the player to a valid Wayfarer location with regional travel
   available.

Completing the Johto or Kanto League before Hoenn cannot reveal Hoenn postgame
NPCs, ferries, gifts, or encounters that require the Hoenn Champion result.
Completing Hoenn cannot repeat or erase another region's League rewards.

### Optional Emerald systems

Battle Frontier, Contests, Secret Bases, Match Call, television events,
multiplayer features, event islands, Safari extensions, and other optional
Emerald systems are included when they already function safely in Wayfarer.
They are not required for acceptance of the first complete Hoenn campaign.

The content manifest reports each optional system as one of:

- compatible and included;
- included with a named limitation;
- excluded from the first milestone; or
- deferred to another specification.

An optional system cannot corrupt shared or regional state even when it is
excluded from acceptance.

### Validation

Static and automated validation must prove all of the following:

1. Every required Hoenn map and asset appears in the generated manifest.
2. Every warp and connection resolves and every required interior can be
   entered and exited.
3. Every reachable Hoenn flag and variable is valid in the Wayfarer namespace.
4. Every Trainer reference resolves to the expected Emerald-authored party and
   a distinct defeat bit.
5. Global difficulty and Trainer Rating changes do not change a Hoenn Trainer
   party.
6. Every authored ordinary wild profile exists in Wayfarer, preserves its
   source population, and uses HNS level projection.
7. Item balls, hidden items, gifts, trades, Trainer defeats, NPC visibility,
   and story scenes persist through save and reload.
8. Each Hoenn Gym awards the correct regional badge and only Hoenn badges
   satisfy Hoenn badge-count checks.
9. Sootopolis remains unavailable until its original late-game progression.
10. A fresh player may visit the opening network first and then complete the
    Emerald main campaign through the Hoenn Hall of Fame.
11. Completing any regional League in every possible order leaves all other
    regional campaigns valid.
12. The complete required content passes the runtime foundation's ROM and
    memory gates.

The end-to-end campaign run must include save and reload checkpoints before and
after every Gym, major team event, Sootopolis unlock, League entry, and Hall of
Fame result. At least one run must visit every opening-network settlement before
starting the adapted Route 101 campaign.

## References

- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Wayfarer regional travel and Hoenn entry](wayfarer-regional-travel-and-hoenn-entry.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Emerald and Hoenn traversal research](../research/emerald-hoenn-story-traversal-blockers.md)

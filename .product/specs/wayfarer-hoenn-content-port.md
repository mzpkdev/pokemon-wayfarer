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

The generated port also owns source-scoped symbol rewrites required by the
union build. In particular, every included Hoenn use of Emerald's shared
`VAR_STARTER_MON` resolves to the dedicated Hoenn starter choice defined by the
regional-entry specification. The rewrite must not change HNS consumers or the
standalone Emerald build. The content audit enumerates every source occurrence
and fails if an included Wayfarer Hoenn consumer retains the raw shared symbol.

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
ID. Ordinary IDs remain below the runtime foundation's partner boundary at
2,048, while partner Trainer IDs begin at that boundary.

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

### Native utility learnsets

Wayfarer compiles the HNS and Hoenn rows from the native HM utility learnset
specification together. It does not inherit the standalone HNS rule that
suppresses Hoenn rows when `IS_HNS` is selected.

The union behavior follows these rules:

- Existing HNS utility additions remain available to their Johto and Kanto
  encounter populations.
- Existing Hoenn utility additions remain available to Hoenn encounter
  populations in both normal and Generation III legacy-moves modes.
- Duplicate additions for the same species, move, level, and mode are emitted
  once.
- Standalone HNS and Emerald builds retain their existing regional rows.
- Wayfarer adds no new utility species or schedules beyond the two existing
  regional sets.

A fresh player who enters Hoenn without first catching a Johto utility user
must be able to obtain the Hoenn-native users required by the Emerald traversal
specification. End-to-end coverage must catch the required users from Hoenn
encounters and cross Route 118 and the eastern sea network without relying on a
Pokémon transferred from another region.

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

Dive in Hoenn and Whirlpool in HNS remain separate usable field moves. Wayfarer
uses the following machine and permission contract:

- HM08 remains the Whirlpool machine used by HNS content.
- Wayfarer adds HM09 as the Dive machine.
- Steven's Mossdeep event grants HM09 instead of attempting to replace HM08.
- The HM09 item teaches Dive and appears as `HM09` in the TM and HM pocket.
- A successful HM09 delivery sets a dedicated Hoenn Dive authorization.
- If the player already owns HM09 while the authorization is missing, Steven's
  interaction reconciles the authorization without granting a duplicate.
- If HM09 cannot be delivered, Steven's reward and authorization remain
  available to retry.
- Using Dive on a Hoenn dive spot requires both a valid Dive user under HNS
  field-move rules and the Hoenn Dive authorization.
- Using Dive on an HNS map retains that map set's existing HNS permission rule.
- Whirlpool continues to use HM08, Whirlpool-compatible users, and the existing
  HNS field action.

Wayfarer has a build-specific nine-machine registry. Its machine enumeration
retains the complete HNS HM01 through HM08 sequence, including Whirlpool as
HM08, then appends Dive as HM09. The generated constants and tables satisfy all
of the following:

- `NUM_HIDDEN_MACHINES` is 9 in Wayfarer.
- `ITEM_HM_WHIRLPOOL` resolves to HM08 and `ITEM_HM_DIVE` resolves to the
  distinct HM09 item rather than `ITEM_NONE`.
- The item-to-move and move-to-item mappings resolve HM08 to Whirlpool and HM09
  to Dive.
- TM and HM indexing, compatibility generation, item metadata, Bag sorting and
  display, teaching, and `IsMoveHM` recognize both entries.
- Any range checks or storage sized from the machine count include HM09 without
  changing the identities of HM01 through HM08.

Standalone HNS retains its eight-machine registry ending in HM08 Whirlpool.
Standalone Emerald retains its eight-machine registry ending in HM08 Dive.

Wayfarer replaces the global Dive-unlocked preflight with a map-aware dispatch
before either diving or resurfacing starts. Every Dive caller, including the
automatic step trigger, uses that dispatch:

- On a Hoenn surface or underwater map, the preflight checks the dedicated
  Hoenn Dive authorization and does not check `FLAG_BADGE07_GET`.
- On an HNS surface or underwater map, the preflight checks the existing HNS
  Badge 7 rule and does not check the Hoenn authorization.
- A map outside those registered contexts cannot inherit permission from
  either state.

No Wayfarer path may call a global HNS Badge 7 predicate before this regional
dispatch. The same result governs both directions and is combined with the
valid-user requirement before either field effect begins.

No HNS badge, global game-clear result, HM08 ownership, or Whirlpool user can
authorize Dive in Hoenn. HM09 ownership and Hoenn Dive authorization cannot
authorize Whirlpool. Standalone Emerald continues to present Dive as HM08, and
standalone HNS continues to present Whirlpool as HM08.

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
5. Every ordinary HNS and Hoenn Trainer remains below ID 2,048, every partner
   Trainer remains at or above 2,048, and partner battles resolve correctly.
6. Global difficulty and Trainer Rating changes do not change a Hoenn Trainer
   party.
7. Every authored ordinary wild profile exists in Wayfarer, preserves its
   source population, and uses HNS level projection.
8. HNS and Hoenn native utility schedules are both present, and Hoenn-sourced
   users satisfy the traversal coverage without a Johto capture.
9. Item balls, hidden items, gifts, trades, Trainer defeats, NPC visibility,
   and story scenes persist through save and reload.
10. The Wayfarer registry reports nine HMs; forward and reverse lookups map HM08
    to Whirlpool and HM09 to Dive; both moves pass `IsMoveHM`; Steven grants
    HM09 atomically; and standalone HNS and Emerald retain their eight-HM
    mappings.
11. With a valid Dive user present, the map-aware Dive preflight passes this
    matrix for both diving and resurfacing:

    | Map context | HNS Badge 7 | Hoenn authorization | Result |
    | --- | --- | --- | --- |
    | HNS | unset | unset | denied |
    | HNS | unset | set | denied |
    | HNS | set | unset | allowed |
    | HNS | set | set | allowed |
    | Hoenn | unset | unset | denied |
    | Hoenn | unset | set | allowed |
    | Hoenn | set | unset | denied |
    | Hoenn | set | set | allowed |

12. Each Hoenn Gym awards the correct regional badge and only Hoenn badges
    satisfy Hoenn badge-count checks.
13. Sootopolis remains unavailable until Hoenn Dive authorization and its
    original late-game progression are satisfied.
14. A fresh player may visit the opening network first and then complete the
    Emerald main campaign through the Hoenn Hall of Fame.
15. Completing any regional League in every possible order leaves all other
    regional campaigns valid.
16. The complete required content passes the runtime foundation's ROM and
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

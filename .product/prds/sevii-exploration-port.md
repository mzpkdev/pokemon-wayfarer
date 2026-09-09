# Sevii exploration port

Status: Draft product design. Not implemented.

## Intent

Add the seven ordinary Sevii Islands to the Wayfarer build as a faithful FRLG
exploration region. Players can visit the island hubs, follow their outdoor
routes, enter their caves and buildings, and find the local wild Pokemon.

This first port is geography and encounters only. It does not bring over the
FRLG Sevii story or its Trainer battles.

## Design

### Region access

Add Sevii Islands as a permanent special destination at Wayfarer's existing
Vermilion dock. It must coexist with the S.S. Aqua route and any separately
approved S.S. Anne option. Selecting Sevii for the first time sails to One
Island. Every numbered-island harbor then offers Vermilion and all seven
numbered islands, with a return option available on every visit.

Birth Island and Navel Rock already exist in Wayfarer through HNS and Emerald
content. Connect their existing harbors to the same transport system using the
FRLG special-island routes. Birth Island remains available only when the
current origin can use the regular S.S. Aqua service and the player has the
Aurora Ticket. Navel Rock remains available only after the League clear and
with the Mystic Ticket. These are read-only travel checks: the connection does
not grant tickets or write ticket, shown-ticket, unlock, or story flags. The
Sevii port does not own or change either island's maps, encounters, actors,
battles, static Pokemon, puzzles, items, rewards, or non-ferry scripts.

Wayfarer does not require Bill's Cinnabar trip, Celio's repair, the National
Pokedex, a League clear, a Rainbow Pass, or another Sevii quest to use the
ordinary One through Seven Island service. This rule does not remove the
separate League-clear requirement for Navel Rock. Entering the region does not
start or complete any FRLG story state.

### Geography

Use the FRLG layouts, tilesets, music, weather, collision, elevation, warps,
and outdoor connections for One through Seven Island. Include the registered
island hubs, routes, caves, dungeons, harbors, houses, Pokemon Centers, Marts,
the Four Island Day Care, Joyful Game Corner, Rocket Warehouse, Lost Cave,
Trainer Tower, Dotted Hole, Tanoby Key, and the seven Tanoby chambers.

The FRLG portion contains every registered map whose source name begins with
`OneIsland_`, `TwoIsland_`, `ThreeIsland_`, `FourIsland_`, `FiveIsland_`,
`SixIsland_`, `SevenIsland_`, `MtEmber_`, or `TrainerTower_`. The unused Seven
Island house is excluded because it is not registered in the source map
catalog.

Do not import the FRLG copies of Birth Island or Navel Rock. Their existing
Wayfarer versions remain separate, unchanged features. This port may add only
the ferry destination and return routing needed to connect them in the same
way as FRLG.

Keep self-contained environmental traversal where it is part of exploring the
map. This includes Icefall Cave's ice and holes, Lost Cave's directional route,
Dotted Hole's floor puzzle, Tanoby Key's Strength puzzle, currents, breakable
rocks, boulders, and ordinary field-move terrain. Remove story-only blockers,
credentials, actors, and scene checks. The Rocket Warehouse entrance is open,
the Mt. Ember Ruby path is not guarded by Rockets, and visiting a map early
does not wait for a story flag.

### Scripted content

On the 135 imported maps, the first port retains only the scripts needed for:

- ferry entry, departure, and return;
- Pokemon Center healing and ordinary PC access;
- Mart purchases and the Four Island Day Care's ordinary service;
- signs and other fixed map information that does not read or write story
  state; and
- environmental traversal and local puzzle state.

Do not import or retain story actors, rivals, Team Rocket scenes, sight-based
Trainers, talk-to-battle Trainers, Trainer Tower battles, scripted wild
battles, static legendary encounters, gifts, trades, item rewards, or quest
handoffs. Maps and rooms that originally hosted those interactions remain
physically present and explorable.

### Wild encounters

Every FRLG Sevii map with an ordinary land, Surf, Rock Smash, or fishing table
gets the corresponding Wayfarer profile. Preserve the FRLG species roles, slot
counts, slot order, authored level ranges, and encounter rates before normal
Wayfarer runtime processing.

Wayfarer is one build, while FireRed and LeafGreen contain some different
species assignments. Resolve those pairs with the same deterministic
counterpart rules used by the mainland Kanto encounter design. Do not add a
species that is absent from both FRLG source profiles. The default FRLG
Altering Cave table is the only Altering Cave population in this milestone;
event rotations remain out of scope.

Each nighttime profile is an explicit alias of its resolved daytime profile.
Day and night therefore use the same species, slots, levels, encounter rate,
and method data. Morning and evening keep the configured daytime fallback.

Ordinary encounters still use Wayfarer's Trainer Rating level projection,
standard fishing behavior, Lures, abilities, and other shared encounter
mechanics. Those systems may affect effective levels or selection odds, but
they do not replace the authored Sevii roster.

## Boundaries

- This feature changes only the Wayfarer build. Standalone FireRed, LeafGreen,
  HNS, and Emerald map catalogs and behavior remain unchanged.
- The [Sevii independent story design](sevii-independent-story-beats.md) is not
  implemented by this port. A later story task may add compatible actors and
  scenes on top of the exploration baseline.
- Birth Island and Navel Rock remain owned by their existing Wayfarer content.
  Preserve all of their current story, Trainers, encounters, static Pokemon,
  puzzles, tickets, items, rewards, and state. Only their ferry connection is
  part of this plan. Their special routes pass through Vermilion as in FRLG;
  they are not added to the ordinary One through Seven Island harbor pages.
- Moltres, Hypno, the Sapphire theft, Ruby delivery, Lostelle, Lorelei's
  confrontation, Selphy, and Rocket Warehouse battles are out of scope.
- Trainer Tower is an empty explorable facility. Its timed challenge, Trainer
  data, prizes, and record systems are out of scope.
- New text is limited to ferry destination labels and service prompts. No new
  narrative or flavor NPC dialogue, quest journal, island story, reward
  economy, Pokemon, field move, or encounter-balancing pass is included.
- Save compatibility with prerelease Wayfarer builds is not required. The port
  must still preserve all unrelated save data and regional progress.

## Interactions

The existing Wayfarer Vermilion dock remains the authority for the S.S. Aqua
and other approved special destinations. Sevii adds one independent choice and
must not advance, consume, or hide another route.

Field moves follow the existing Wayfarer field-use and native-learnset rules.
Optional island branches may require Surf, Cut, Rock Smash, Strength, or
Waterfall where the FRLG geography uses them. Ferry access to each island hub
never requires one of those moves.

The first milestone uses the ferry for numbered-island regional travel. It
does not add Sevii Fly destinations or change existing Birth Island and Navel
Rock travel behavior beyond their FRLG-style ferry connection.

Wild encounters remain ordinary regional encounters for Pokedex area data,
DexNav, Lures, ability attraction, and Trainer Rating. Static and scripted
Pokemon are absent from the imported numbered-island content. Existing Birth
Island and Navel Rock Pokemon remain unchanged.

The complete port must fit the 32 MiB ROM and retain the active release reserve.
If the full catalog cannot meet that gate, implementation pauses for an
approved content-preserving storage optimization. It does not silently remove
islands, interiors, or encounter profiles.

## Playtesting

- Enter Sevii from the existing Vermilion dock without FRLG story progress,
  visit all seven numbered-island harbors, and return to Vermilion from each
  one.
- Walk every outdoor connection and enter and exit every registered interior.
  No missing map, null header, invalid warp, story actor, or Trainer battle may
  interrupt the route.
- With their existing access conditions satisfied, sail from Vermilion to the
  existing Birth Island and Navel Rock maps and return through their harbors.
  Confirm that their story, Trainers, encounters, static Pokemon, puzzles,
  items, rewards, and save state behave exactly as before this port.
- Exercise Icefall Cave, Lost Cave, Dotted Hole, Tanoby Key, Rocket Warehouse,
  Mt. Ember, and the empty Trainer Tower. Each remains recoverable after save,
  reload, escape, blackout, and ferry travel where those actions apply.
- Sample every encounter method at day and night. Each paired profile resolves
  to the same authored table before ordinary runtime modifiers.
- Verify Pokemon Centers, Marts, the Day Care, PCs, and island ferries without
  enabling any Sevii story state.
- Repeat the route at low and high Trainer Rating to confirm that scaling does
  not change the resolved species roster.

## References

- [Sevii map port specification](../specs/sevii-exploration-map-port.md)
- [Sevii wild encounter specification](../specs/sevii-wild-encounters.md)
- [FRLG open-world regional traversal](frlg-open-world-region-traversal.md)
- [Kanto wild encounters](kanto-wild-encounters.md)
- [Badge-free HM field use](hm-field-use.md)
- [Wayfarer Hoenn content port](../specs/wayfarer-hoenn-content-port.md)

# Wayfarer Hoenn integration

## Intent

Wayfarer combines Johto, Kanto, and Hoenn in one HNS-based playthrough. After
the opening gives the player a starter, the player can travel to Hoenn, explore
its towns and routes, challenge its Trainers and Gyms, and complete its main
campaign without starting a separate game or changing ROMs.

Hoenn keeps the identity and authored content of Pokémon Emerald. Wayfarer uses
the HNS engine and progression rules around that content, including HNS wild
level scaling, field-move behavior, battle mechanics, and quality-of-life
features. The finished ROM must remain within the standard 32 MiB GBA ROM
limit.

## Design

### One continuous adventure

- The HNS player, party, Bag, Pokédex, money, options, and play time continue
  across all regions.
- Hoenn is part of the same save file. Entering or leaving it never starts a
  second campaign slot or swaps the player to Emerald's protagonist.
- The HNS opening remains the start of the game. Regional travel becomes
  available after the player receives the first starter and gains normal
  control in the opening Johto network.
- The regional service connects Johto, Kanto, and a Littleroot-area arrival in
  Hoenn. Its basic travel and return options require no badge, HM, ticket,
  payment, scripted victory, or regional story completion.
- The original S.S. Aqua and other regional transport stories may remain as
  content, but they cannot be the only way to enter or leave a region.

### Hoenn arrival and campaign start

The player arrives in Hoenn as a visiting Trainer. The first arrival prepares
only Hoenn's local starting state and does not reset or replace any HNS player
state.

The Emerald campaign begins from an adapted Route 101 Birch rescue:

1. The player reaches Littleroot and Route 101 with their existing party.
2. Birch's rescue uses a Pokémon from that party instead of forcing the stock
   Emerald starter-selection sequence.
3. After the rescue, Birch offers one Hoenn starter as an optional gift.
4. Brendan or May fills Emerald's local rival role. The player does not become
   Brendan or May and does not repeat the moving-house, clock-setting, player
   creation, or initial Pokédex setup.
5. Completing the adapted introduction establishes the minimum Hoenn campaign
   state needed for the original story to proceed.

Declining or postponing the Hoenn starter does not block travel or the main
campaign. The gift remains available until accepted.

### Open exploration

- Hoenn's opening-network towns, cities, roads, caves, and sea routes are
  available early under the existing Emerald open-world traversal rules.
- Visiting a later location does not complete an earlier story scene, award its
  reward, defeat its opponent, or synthesize its completion flags.
- Later story scenes appear only when their own Hoenn prerequisites are met.
  Maps must tolerate being visited before those scenes become active.
- A story actor or scripted battle cannot occupy the only route to a town or
  city. A passable lane or independent transport option remains available.
- Ordinary sight-based Trainers may still challenge the player. Their battles
  are content, not story gates.
- Sootopolis remains a late-game town. Its Dive entrance, weather crisis, Cave
  of Origin scenes, Gym, and related rewards retain their original Hoenn story
  progression.
- Ever Grande can be visited early, but Victory Road, the Elite Four challenge,
  and Champion completion retain their Hoenn requirements.
- Optional and secret locations may retain their original field-move, item, or
  story requirements when they are not the only route to an ordinary
  settlement.

### Authored Hoenn content

Wayfarer includes the following Emerald content as part of the Hoenn region:

- The mainland, towns, cities, routes, caves, sea routes, interiors, and
  required campaign locations.
- Map NPCs, shops, healing facilities, item pickups, hidden items, trades, and
  ordinary interactions.
- Ordinary Trainers, rivals, Team Aqua and Team Magma encounters, Gym Leaders,
  the Elite Four, and the Champion.
- The eight Hoenn Gyms and the Emerald main story through the Hoenn Hall of
  Fame.
- Emerald's authored wild species, encounter methods, slot weights, and
  locations.

Trainer parties keep their Emerald species, levels, moves, items, AI, and battle
formats. Wayfarer does not add Trainer Rating scaling, badge scaling, or another
dynamic level system to Hoenn Trainers. Ordinary Trainer positions, movement,
trainer types, and sight ranges also remain unchanged unless a separate
traversal requirement explicitly moves a blocking story actor.

Wild Pokémon use Emerald's authored populations and the existing HNS Trainer
Rating level projection. Hoenn progress does not add new inputs to Trainer
Rating. Fixed, gift, legendary, hidden, and scripted Pokémon keep their authored
levels unless another approved feature already governs them.

### Regional progression

- Hoenn has eight independent badge states. They do not reuse or overwrite the
  Johto and Kanto badge states.
- A Hoenn story check that asks for a badge count uses only Hoenn badges.
- Hoenn badges do not alter HNS Trainer Rating.
- Hoenn Champion completion is independent of Johto and Kanto Champion
  completion.
- Completing one region's League cannot start, finish, reset, or unlock another
  region's campaign.
- One-time Hoenn Trainers, NPCs, items, gifts, and story rewards remain consumed
  after leaving the region, saving, reloading, blacking out, or completing a
  League elsewhere.

HNS field-move rules apply throughout Wayfarer. A Pokémon that can use a field
move under HNS rules can use it in Hoenn without restoring Emerald's badge and
HM ownership checks. Dive in Hoenn and Whirlpool in HNS retain distinct player
behaviors even though the source games assign them to the same HM number.

### Region-aware travel and recovery

- The Town Map opens on the player's current region. The player can switch
  between Johto, Kanto, and Hoenn views.
- Each view uses that region's map art, labels, player marker, location data,
  and visited destinations.
- Fly exposes only destinations the player has visited. Region tabs may be used
  to select a visited destination in another region.
- The physical regional service remains available even after Fly is obtained
  and never becomes dependent on Fly.
- Each region remembers its own most recent healing location. A blackout
  returns the player to the current region's last valid healing location, or to
  that region's safe arrival point if none has been registered.
- Saving and reloading preserves the current region, map position, Hoenn
  campaign state, visited locations, badges, defeated Trainers, collected
  items, and healing destinations.

## Boundaries

- Wayfarer does not redesign or rebalance Emerald Trainer parties.
- Wayfarer does not add Hoenn-based Trainer Rating milestones.
- Free regional exploration does not mean every dungeon, shortcut, legendary
  room, Gym interaction, or story reward is available immediately.
- Battle Frontier, Contests, Secret Bases, Match Call, television events,
  multiplayer features, event islands, and other optional Emerald systems are
  preservation targets when they already work, but they are not required for
  the first complete Hoenn campaign milestone.
- This PRD does not define Sinnoh integration. The ROM budget must leave useful
  headroom for a later Sinnoh slice, but it does not promise that a full Sinnoh
  campaign will fit.
- Compatibility with saves from prerelease builds is not required. A released
  Wayfarer version must not silently reinterpret saved state after its save
  format becomes public.

## Balance

Hoenn's ordinary and boss Trainers use their Emerald-authored levels. This
means the player may meet battles that are much stronger or weaker than their
current party when regions are explored out of order. Wayfarer does not correct
that mismatch with scaling.

Wild levels continue to follow HNS Trainer Rating wherever the player travels.
The same rating produces the same intended wild-level band in Johto, Kanto, and
Hoenn. Hoenn badges and story milestones do not raise it.

Rewards remain attached to their original Hoenn interactions. Opening a travel
lane cannot grant a skipped reward. A one-time reward must either be delivered
and recorded together or remain available to retry.

## Presentation

- Player-facing build and save identifiers use the name "Wayfarer" rather than
  "HNS All Region" or another internal integration name.
- Regional transport clearly lists Johto, Kanto, and Hoenn destinations and
  always includes Cancel and a valid return journey.
- The map and Fly interfaces identify the selected region and do not mix
  Hoenn's destinations with the Johto and Kanto map layout.
- The Hoenn introduction acknowledges that the player is visiting with an
  existing party. It must not present the player as having just moved from a
  different home.
- Existing Emerald music, map presentation, dialogue, and encounter identity
  remain intact unless HNS already applies a global presentation change.

## Interactions

### Existing open-world traversal

The Emerald open-world traversal behavior remains the basis for Hoenn's road
network. Its bypasses, public Route 104 ferry, native Surf crossings, and local
state isolation continue to apply. Wayfarer keeps Sootopolis behind its
original late-game progression. Ever Grande may be visited early without
unlocking Victory Road, the Elite Four, or Champion content.

The HNS open-world traversal behavior remains the basis for Johto and Kanto.
Wayfarer's regional service supersedes any rule that makes the S.S. Aqua maiden
voyage the required first entry to Kanto. The maiden voyage remains optional
story content and must coexist with prior Kanto visits.

### Shared systems

- Pokédex area data follows the selected region while species ownership and
  seen status remain global.
- Pokémon Centers, shops, the PC, party storage, the Bag, money, and player
  identity remain shared across regions.
- Hoenn events must not read a Johto or Kanto flag or variable merely because
  the source games assigned both meanings to the same numeric identifier.
- Whiteout, Hall of Fame, credits, daily resets, and new-game initialization
  run only the region-specific story cleanup that applies to the current event.
- Global game-clear state cannot stand in for Hoenn Champion state when it
  would reveal or complete Hoenn postgame content.

## Constraints

- The final ROM must be no larger than 32 MiB and must not rely on a 64 MiB ROM
  mapping, emulator-only storage, or a custom cartridge mapper.
- During Hoenn development, release builds must keep at least 512 KiB of unused
  ROM space. A 1 MiB reserve is preferred until the required Hoenn campaign is
  complete.
- Space savings must preserve player-facing content. Deduplication, removal of
  empty data, release-only debug removal, and transparent compression take
  priority over cutting maps, Trainers, NPCs, encounters, Gyms, or story.
- Existing HNS map identifiers and persistent meanings cannot be renumbered by
  the union build. Hoenn receives distinct persistent state even when Emerald
  and HNS originally used the same numeric IDs.
- Map group and map numbers must remain representable by the GBA engine's
  existing warp format.
- Added persistent state and runtime buffers must stay within the save-sector,
  EWRAM, heap, and decompression limits of the GBA engine.
- Every release build reports total ROM use and fails before exceeding either
  the 32 MiB limit or the active reserve threshold.

## Playtesting

- Can a new HNS player reach Johto, Kanto, and Hoenn after receiving the first
  starter without earning a badge or completing a regional campaign?
- Can the player return from every region with no Fly user and no story ticket?
- Can the player visit every town and city in Hoenn's opening network before
  advancing the Emerald campaign, then return to Route 101 and complete the
  full campaign normally?
- Does Birch's adapted rescue work with an existing party, and does the
  optional starter remain available after being declined?
- Do representative Hoenn wild encounters use the expected Emerald species and
  HNS-scaled level band at several Trainer Ratings?
- Do ordinary Trainers, each Gym Leader, rival battles, team bosses, the Elite
  Four, and the Champion use their authored Emerald parties without scaling?
- Do Hoenn badges, Champion state, defeated Trainers, items, NPC visibility,
  and story scenes survive travel, save and reload, blackout, and another
  region's League completion?
- Does Sootopolis remain unavailable until the original late-game Dive and
  crisis progression opens it?
- Can the player visit Ever Grande early without accidentally starting or
  completing Victory Road, Elite Four, or Champion content?
- Do the Town Map, Fly, Pokédex area display, healing, blackout, and Hall of Fame
  behavior select the correct region in every tested order?
- Can the complete required Hoenn campaign and all required regional content be
  built with the active ROM reserve intact?

## References

- [Wayfarer runtime foundation specification](../specs/wayfarer-runtime-foundation.md)
- [Wayfarer Hoenn content port specification](../specs/wayfarer-hoenn-content-port.md)
- [Wayfarer regional travel and Hoenn entry specification](../specs/wayfarer-regional-travel-and-hoenn-entry.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Badge-free HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Emerald and Hoenn story traversal blockers](../research/emerald-hoenn-story-traversal-blockers.md)
- [Story-blocking traversal audit](../research/story-blocking-traversal-audit.md)

# Native HM utility learnsets

## Intent

Let players solve field obstacles before receiving the matching HM by catching a
wild Pokémon that already knows the move. The selected species are common enough
to find in several parts of their home region, have an obvious relationship to
the move, and are generally weak or underused enough to benefit from a utility
role.

This creates an alternative to story-based HM acquisition without giving the
ability to the player for free. The Pokémon occupies a party slot, and the HM
move occupies one of its four move slots.

Native Surf users may also provide the ordinary route to a settlement when the
existing encounter geography provides a listed user on both sides. Surf is the
only native HM allowed to fill this role. Other native HM moves remain tools
for optional routes, shortcuts, and sequence breaks. This establishes route
availability for a prepared player, not recovery after the player gives up
access to Surf.

This behavior applies to Emerald, FireRed and LeafGreen, and HNS. It builds on
the existing rule that a Pokémon which knows an HM move may use it in the field
without owning the HM or holding the former authorizing badge.

## Design

Each active regional HM except Fly has two native utility species in that
region's generation. A listed species receives the HM move in its level-up
learnset and can be encountered knowing it throughout at least two qualifying
places in its home region.

The two species for one move should cover different parts of the region when
the encounter data allows it. Inland and coastal water encounters, early and
late routes, or routes and caves are better pairs than two species concentrated
in the same dungeon.

A utility species must meet these requirements:

- Its anatomy, behavior, or elemental identity makes the move easy to believe.
- Its existing HM compatibility includes the move.
- It appears in at least two distinct routes, towns, or dungeons in its home
  region. Floors of one dungeon count as one place.
- Each qualifying encounter can be reached without using the same HM move. A
  species cannot be the answer to an obstacle if the player needs that
  obstacle's move to catch it.
- Every configured wild level in at least two qualifying places leaves the HM
  move in the Pokémon's active four-move set when caught.
- The species is preferably weak, underused, or overshadowed. Stronger or
  popular species are acceptable only when theme or regional coverage has no
  convincing weaker alternative.

The move is an ordinary learned move. It has its normal battle effect, type,
power, accuracy, and PP. It may be replaced or forgotten, and using it in the
field does not consume PP. If the player forgets the move before obtaining the
HM, that Pokémon stops providing the field action until it learns the move
again.

The named species are the anchors for each utility role. Each anchor and every
intended direct evolution must have the move in each applicable level-up
learnset. A known move persists through evolution, but Move Reminder access is
defined separately for each species. Evolved forms do not count as extra
regional candidates.

## Boundaries

Fly is excluded. Fast travel has separate destination, discovery, and pacing
concerns and continues to depend on its existing HM acquisition paths or a
Pokémon that already learns Fly under existing content.

Only HMs active in a build receive regional coverage. FireRed and LeafGreen do
not need native Dive or Whirlpool users. HNS adds native Whirlpool users but
does not change Dive, which remains a non-HM field action there. Emerald adds
native Dive users but does not add Whirlpool as a field action.

This feature does not:

- move, duplicate, or remove HM items;
- change story rewards or the conditions for receiving an HM;
- add a separate innate field-skill system;
- change species encounter locations or encounter rates;
- make a move usable when its normal terrain, direction, map, or destination
  conditions are not met;
- guarantee that the player always carries every field move;
- prevent the player from becoming stranded after losing access to the last
  native Surf user.

This PRD does not add grass, fishing encounters, encounter slots, or new
species placements to make native Surf coverage work. A settlement route may
rely on native Surf only when the existing encounter data already provides a
listed native user on every land-connected side of the crossing. A water
encounter that requires Surf to reach does not provide directional coverage.
This encounter rule is a content prerequisite, not a softlock guarantee.

The open-world traversal PRDs define the approved native Surf crossings. They
continue to require ordinary access for every other core obstacle.

## Balance

The utility roster favors Pokémon that players often pass over. Catching one
should feel like finding a practical regional companion, not like receiving a
free permanent upgrade.

The party slot and move slot are the primary costs. Strong HM attacks such as
Surf, Strength, Waterfall, and Dive may also improve the selected Pokémon in
battle. That benefit is intentional, but early access must not turn a common
utility catch into the dominant combat choice for its part of the game.

Learn levels should support wild-catch usability without placing every HM at
level 1. A move may appear again at a later learn level when one placement
cannot keep it active across both qualifying encounter ranges. The move must
remain available through normal move relearning.

Repeated species are intentional. Geodude, Horsea, Aipom, Chinchou, Wooper,
Miltank, Corphish, Sableye, Wailmer, and Carvanha each support two related field roles.
A player may build a compact utility party around them, but each additional HM
still consumes another move slot.

## Content

### Kanto native users

FireRed and LeafGreen use Generation I species for all six regional HMs other
than Fly.

| HM | Native species | Regional coverage and theme |
| --- | --- | --- |
| Cut | Paras and Rattata | Claws and incisors across Mt. Moon, the Safari Zone, and early routes. |
| Flash | Voltorb and Pikachu | Electrical light from Route 10, the Power Plant, Viridian Forest, and the Safari Zone. |
| Surf | Horsea and Krabby | Fishing coverage on both the Pallet and Cinnabar sides of Kanto's ocean crossing. |
| Strength | Machop and Geodude | Physical power across Mt. Moon, Rock Tunnel, Victory Road, and other caves. |
| Rock Smash | Mankey and Geodude | Striking fists and a rock-bodied partner found on routes and in caves. |
| Waterfall | Goldeen and Horsea | Inland and coastal fishing coverage from two natural swimmers. |

Pikachu is the main exception to the underused-species preference. It replaces
Staryu because Staryu is unavailable in FireRed's Kanto encounters, while
Pikachu provides a clear theme and multiple-place coverage in both versions.

### Johto native users

HNS uses Generation II species for its seven regional HMs other than Fly.

| HM | Native species | Regional coverage and theme |
| --- | --- | --- |
| Cut | Gligar and Aipom | Pincers and a tail-hand across mountain routes, the Safari Zone, and Headbutt encounters. |
| Flash | Chinchou and Mareep | A lantern fish and a glowing tail across water, caves, and early land routes. |
| Surf | Chinchou and Wooper | Shore fishing around Olivine and Cianwood paired with early land encounters on Route 32 and around the Ruins of Alph. |
| Strength | Snubbull and Miltank | Compact physical power across Routes 34, 35, 38, 39, and 47. |
| Rock Smash | Aipom and Miltank | A tail-hand strike and a horned battering ram across Headbutt areas and Routes 38, 39, and 47. |
| Waterfall | Wooper and Marill | Inland pools and caves across Route 32, the Ruins of Alph, Union Cave, Route 42, and Mt. Mortar. |
| Whirlpool | Chinchou and Mantine | Open-water species found along the coast, Route 41, caves, and the Whirl Islands. |

Most of Aipom's broad Johto coverage comes from Headbutt encounters. Headbutt
must therefore remain obtainable independently of Cut and Rock Smash for Aipom
to count toward practical coverage.

Miltank is an exception to the weak-species preference. Its physical theme and
separate route coverage make it the most practical second Rock Smash user.

### Hoenn native users

Emerald uses Generation III species for its seven regional HMs other than Fly.

| HM | Native species | Regional coverage and theme |
| --- | --- | --- |
| Cut | Corphish and Sableye | Pincers and claws across ponds and several cave systems. |
| Flash | Electrike and Sableye | Electrical light and luminous gem eyes across Routes 110 and 118 and Hoenn's caves. |
| Surf | Lotad and Wailmer | Early land encounters paired with widespread fishing throughout Hoenn's eastern waterways and ocean settlements. |
| Strength | Makuhita and Torkoal | Muscle and a heavy shell across Granite Cave, Victory Road, Fiery Path, and Magma Hideout. |
| Rock Smash | Aron and Corphish | An iron head and crushing pincers across cave networks and ponds. |
| Waterfall | Barboach and Carvanha | River and cave encounters paired with the eastern waterways on Routes 118 and 119. |
| Dive | Wailmer and Carvanha | A whale and a predatory fish available through widespread fishing and eastern water routes. |

Sableye is a softer visual fit for Flash than the other candidates, but its gem
eyes, existing compatibility, cave distribution, and low usage make it the best
second choice. Volbeat is more literal but appears only on Route 117.

Carvanha begins as a frail utility catch, but Sharpedo is a strong evolution.
Waterfall and Dive still consume separate move slots after evolution, so the
combat improvement does not remove the utility cost.

## Native Surf settlement coverage

Native Surf may replace an ordinary ferry or constructed crossing only on the
following routes. Coverage uses existing encounters and map geometry.

| Build and route | Existing departure-side source | Existing arrival-side source |
| --- | --- | --- |
| FireRed and LeafGreen, Pallet to Cinnabar | Horsea and Krabby are available through Pallet's existing fishing encounters in both versions. | Horsea and Krabby are available through Cinnabar's existing fishing encounters in both versions. |
| HNS Johto, Olivine to Cianwood | Wooper is available on the connected Johto mainland, while Chinchou is available around Olivine. | Cianwood's existing fishing encounters provide Chinchou during the day and Krabby, a Kanto native Surf user, at night. |
| HNS Kanto, mainland to Cinnabar | Chinchou is available around Vermilion on the connected Kanto mainland. | Chinchou is available in Cinnabar's existing fishing encounters during the day and at night. |
| Emerald, Route 118 and the eastern mainland | Lotad is available on the western land network. | Wailmer is available through existing fishing encounters on the eastern land network. |
| Emerald, Lilycove to Mossdeep and Pacifidlog | Wailmer is available through Lilycove's existing fishing encounters. | Wailmer is available through the existing fishing encounters in Mossdeep and Pacifidlog. |

Sootopolis is not part of this coverage. It remains unlockable content whose
entrance, Dive requirement, story state, and return behavior are defined by its
own progression.

The Standard Rod fishing PRD makes every authored fishing slot eligible at
every rod quality. A player who already has the Old Rod and capture supplies
can therefore hook each listed fishing species without an encounter edit or a
stronger rod. It does not guarantee that the Old Rod is obtainable before a
crossing or that the required species has a practical fishing probability.

Acceptance in this PRD covers prepared travel in both directions. It does not
cover missing rods or Poké Balls, a full party, depositing or releasing the
last Surf user, forgetting Surf, or emergency return behavior. A separate
traversal-recovery PRD owns those softlock-prevention requirements.

This PRD may be accepted before that recovery work. It must not be used alone
as evidence that a regional settlement network is softlock-safe.

## Presentation

The game does not label these Pokémon as HM users or add a new tutorial for
them. The player discovers the option by catching the Pokémon and seeing the
move in its ordinary moveset. Existing field-use prompts identify the Pokémon
as the performer.

HM tutorials may mention that some wild Pokémon already know field moves and
can use them before the matching HM is found. They should not list the full
roster or tell the player that one of these Pokémon is required.

## Interactions

- A native user follows the known-move tier of HM field-user selection. It does
  not need the matching HM item or a badge.
- Party order decides between several Pokémon that know the same move during an
  automatic field interaction.
- A fainted native user still qualifies, while an Egg does not.
- Forgetting or replacing the native move removes that route to field use. The
  ordinary HM-in-Bag compatibility route remains available if the player later
  obtains the machine.
- Evolution preserves the known move. The evolved species' learnset also keeps
  it available to the Move Reminder.
- Catch swapping, storage, and release remain allowed. The game does not
  protect a native user's party slot because it is the player's only way past
  an obstacle.
- HMs Overwrite remains a separate challenge option and is not needed for any
  listed species.
- Native users depend on the badge-free HM field-use behavior. In particular,
  HNS Whirlpool must resolve a known-move user and ignore badge ownership before
  Chinchou or Mantine can provide early Whirlpool traversal.

## Constraints

Normal wild generation derives a Pokémon's initial moves from its species and
level, keeping the last four distinct level-up moves available at that level.
There is no encounter-specific moveset field. The authored level-up schedule
must therefore keep every assigned utility move in the generated four-move set
at every possible wild level in the two qualifying places.

An encounter-specific exception would require a separate product and technical
decision to add encounter data and a wild-creation hook. This PRD does not imply
or authorize that system.

The game has an active normal learnset and a Generation III legacy learnset
path. Native utility moves must be present in both modes, either by updating
both data sources or by centralizing the additions in shared learnset
selection. Every build must produce the same native utility roster in normal
and legacy-moves mode.

Existing HM compatibility must remain intact in the generated runtime
teachable learnsets. If the compatibility source changes, regenerate and check
the compiled teachable data before accepting the roster.

## References

- [Badge-free HM field use](hm-field-use.md)
- [Standard Rod fishing](standard-rod-fishing.md)
- [HM field-use technical specification](../specs/hm-field-use.md)
- [HM compatibility data](../../game/src/data/pokemon/all_learnables.json)
- [Teachable learnset generation](../../game/docs/tutorials/teachable_learnsets.md)
- [Wild encounter data](../../game/src/data/wild_encounters.json)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [FireRed and LeafGreen open-world regional traversal](frlg-open-world-region-traversal.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)

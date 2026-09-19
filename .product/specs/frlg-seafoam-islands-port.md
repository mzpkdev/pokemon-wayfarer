# FRLG Seafoam Islands port

PRD: [FRLG Cinnabar and Seafoam Islands port](../prds/frlg-cinnabar-seafoam-port.md)
Implemented: No

## Scope

Select and adapt Seafoam Islands 1F, B1F, B2F, B3F, and B4F for Wayfarer,
including the boulder/current puzzle, Articuno, items, encounters, and two
Route 20 entrances. The [route specification](frlg-kanto-coastal-routes-port.md)
owns the selected FRLG Route 20 exterior; coastal integration owns its
boundaries, persistent-state allocation, and release measurement. The
Cinnabar specification owns Blaine's Gym.

## Behavior

### Maps and access

Wayfarer selects `SeafoamIslands_{1F,B1F,B2F,B3F,B4F}_Frlg` and their FRLG
layouts, graphics, music, scripts, events, and FRLG wild encounter sources.
The existing map files' floor warps are the topology baseline. Both Route 20
doorways lead into 1F and both 1F exits return to the corresponding Route 20
side. Every internal floor warp has a reciprocal destination; falls retain
their authored landing points. A player can enter from either coast and leave
through either coast after completing the necessary local traversal.

The selected Wayfarer catalog excludes the HNS Seafoam 1F, B1F, Gym, and
Secret Cave maps. The HNS ice-step and cracked-floor scripts, Seafoam Blaine
and Gym Guy, and Secret Cave Groudon are not active in Wayfarer. Do not leave
an orphaned Route 20 warp or a hidden route into the old maps. Standalone HNS
continues to select its own content.

### Strength boulders and currents

Preserve the FRLG boulder positions and floor-to-floor movement on 1F, B1F,
B2F, B3F, and B4F. On B3F and B4F, the two required boulders stop the local
current and select the corresponding `*_CurrentStopped_Layout`. Current and
stopped-current layouts remain separate selectable layouts. Each floor
restores the correct boulder, collision, and current state on entrance and
save/reload. The selected FRLG Route 20 must run the FRLG reset on every
transition into that route: reset only unfinished B3F/B4F boulder paths to their
starting positions, while preserving completed stopped-current state. One
boulder alone does not stop a two-boulder current; completed current state
does not regress on reentry.

Use Wayfarer-owned persistent identities for boulder and stopped-current
state. Stock FRLG constants that resolve to zero or overlap HNS/Emerald state
cannot be used as saved flags. Keep temporary counters temporary. Preserve
the existing Strength and Surf field-use rules rather than adding a Blaine,
badge-count, or campaign-clear requirement.

### Articuno, items, and encounters

Articuno appears on B4F only. Reuse one Wayfarer Articuno ownership state so
the removed HNS B1F encounter cannot become a second capture. Its battle is
level 50 before any separately approved scaling rule. Apply the Kanto
legendary-readiness rule to starting the battle, not to entering or exploring
Seafoam. An ineligible interaction explains that Articuno cannot yet be
challenged and leaves it present. Preserve FRLG's battle outcomes: a run or
teleport removes Articuno for the current map visit without setting its fought
flag, so it returns on reentry; a catch, defeat, or player loss sets the fought
flag and resolves the encounter. Map the source flags to the single Wayfarer
Articuno state without changing these outcomes.

Preserve FRLG floor items and hidden items, including Ice Heal, Water Stone,
Revive, Big Pearl, Ultra Ball, Nugget, and the B4F Water Stone. Each uses
collision-safe placement and a one-time Wayfarer flag. Reuse HNS Seafoam 1F
and B1F land encounters as donors under the selected FRLG map identities.
Adapt FireRed/LeafGreen tables for B2F, B3F, and B4F through Wayfarer's
day/night and Trainer Rating pipeline. A floor without an authored night
source may use its day table at night. Keep standalone HNS bindings available;
the selected Wayfarer catalog covers every active Seafoam floor.

Groudon is absent from Seafoam. Do not replace the old Secret Cave with a new
room, alternate entrance, or second Groudon. Hoenn's Terra Cave remains the
capture destination under its owning story rules.

### Acceptance

1. Validate every selected map, layout, event, warp, encounter, and flag
   reference in generated Wayfarer data; standalone HNS and FRLG still select
   their original maps.
2. Traverse both Route 20 entrances, every floor, each boulder path, both
   current states, Articuno's shore, and both exits in an emulator. Repeat
   after save/reload and after leaving through each coast, both before and
   after each current is stopped. An unfinished path resets on Route 20;
   completed paths stay solved.
3. Verify item persistence, encounter species and methods per floor, and
   Articuno's ineligible, run/teleport and reentry, catch, defeat, and player
   loss paths.
4. Confirm no Wayfarer path enters Seafoam Gym or Secret Cave and no Seafoam
   interaction starts a Groudon battle or awards Blaine's badge.

## References

- [FRLG Seafoam maps](../../game/data/maps/SeafoamIslands_1F_Frlg/map.json)
- [B3F current script](../../game/data/maps/SeafoamIslands_B3F_Frlg/scripts.inc)
- [B4F current and Articuno script](../../game/data/maps/SeafoamIslands_B4F_Frlg/scripts.inc)
- [FRLG stopped-current layouts](../../game/data/layouts/layouts.json)
- [Coastal and state integration](frlg-cinnabar-seafoam-integration.md)

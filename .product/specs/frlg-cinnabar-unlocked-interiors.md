# FRLG Cinnabar unlocked interior preview

PRD: [FRLG Cinnabar and Seafoam Islands port](../prds/frlg-cinnabar-seafoam-port.md)
Implemented: No

The behavior is implemented in draft [PR #110](https://github.com/mzpkdev/pokemon-wayfarer/pull/110).
Change the marker to `Yes` once that implementation lands on `main`.

## Scope

Make twelve FRLG-layout Cinnabar interiors playable from the current
`CinnabarIsland_SeamPoc` exterior: Mansion 1F, 2F, 3F, B1F; Gym; Lab entrance,
lounge, research room, experiment room; Center 1F and 2F; and Mart. This is an
unlocked exploration preview. The [full town specification](frlg-cinnabar-town-port.md)
owns Mansion switches, encounters, items and Secret Key; Gym quizzes, Trainers
and Blaine; Lab trades, tutor and fossils; the complete Center service floor;
and town presentation. [Coastal integration](frlg-cinnabar-seafoam-integration.md)
owns the final route seams and destination selection.

## Behavior

Each of the five exterior doors enters its matching building, and each
building returns to the exterior through its declared exit. The three Lab
rooms connect both ways through the entrance hall. Center 1F and 2F connect
by their stairs. Mansion stairs, floor links, and the basement passage permit
travel between all four floors. The Gym and Mansion barriers start open so
these otherwise empty maps remain traversable without story flags or a key.

The Center nurse heals the party; entering Center 1F sets Cinnabar's current
HNS respawn location. The Mart clerk sells Ultra Ball, Great Ball, Hyper
Potion, Revive, Full Heal, Escape Rope, and Max Repel. The other interiors have
no active NPC services or reward interactions in this milestone.

FRLG behavior IDs in FRLG-version layouts are translated when the runtime
reads a metatile's behavior attribute. Other attributes retain their source
values.
Existing door, stair, escalator, and fall behavior takes precedence. A player
who steps onto a normal or cave tile with a declared warp event can take that
warp only in these twelve interior layouts. Other maps keep their existing
warp handling.

### Acceptance

1. Enter and exit all five buildings; traverse the Lab rooms, both Center
   floors, and all Mansion floors and basement without being stranded.
2. Heal a fainted party member at the Center and purchase an Ultra Ball at the
   Mart before any Mansion or Gym progress.
3. Confirm behavior translation preserves nonbehavior attributes and the
   production ROM still meets its 512 KiB free-space reserve.

## References

- [Warp investigation and verification](../research/cinnabar-interior-port-blocker.md)
- [SkyEmu interior journey](../../e2e/src/journeys/cinnabar-interior-port.e2e.ts)

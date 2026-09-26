# Gym Leader scaling

Implemented: No. The earlier player-rating Gym scaler exists in code but is
disabled by default, and Giovanni's Wayfarer finale has its own path. The ROM
keeps that behavior until this design is adopted.
Design status: Accepted direction under the living-rivals model; team content
and numbers are provisional.

## Intent

Let players take the Gyms in any order and always find a fair, recognizable
fight. A leader challenged first opens with a small, approachable team; a
leader postponed until late has grown with the world and fields a full team
near the player's cap. Leaders feel like living rivals: in one save Whitney
shot ahead early, in another she is still finding her feet, and townsfolk and
the leader's own dialogue hint at which.

## Design

Gym Leaders use [trainer world progression](trainer-world-progression.md): a
leader's strength follows the world's badges and first league clears, never
the player's party or training. Each leader sits a little under the cap on
average, then follows a growth arc chosen once per save from a few that suit
them. Fast arcs put a leader a little above the cap, slow arcs a little below,
and the gap never widens as the journey goes on. A few strong leaders in each
region (Giovanni, Sabrina, Clair, Morty, Norman, Winona, and Juan) sit closer
to the cap, so in some saves one of them can headline a championship.

Teams grow in hand-authored stages as the world earns badges: two Pokémon at
the start, three by around three badges, four by around six, and five or six
later. Each stage is a complete reviewed team with a recognizable ace.
Evolutions and new teammates appear only where authored; a team never shrinks
and its weakest member never gets weaker. Original FRLG, Emerald, and HNS
parties are references, not required opening teams.

The team is set when the battle starts and kept for the whole fight. Retrying
without new progress brings the same team at the same levels; a badge or
first league clear earned elsewhere first means the leader has grown. The
challenged Gym's badge is awarded after the battle.

## Coverage

Wayfarer has 24 badge encounters. This policy covers the 23 singles badge
opponents: Brock, Misty, Lt. Surge, Erika, Janine, Sabrina, Blaine, Giovanni
(his Viridian finale); Falkner, Bugsy, Whitney, Morty, Chuck, Jasmine, Pryce,
Clair; Roxanne, Brawly, Wattson, Flannery, Norman, Winona, and Juan.

Tate and Liza's double battle keeps its existing policy and badge; it is not
converted to singles. Blue's growth applies only to his league appearances;
Wayfarer has no Blue badge encounter. Rematches, Giovanni's villain
scenes, Blue's rival and Dojo battles, facilities, and story battles keep their
own policies unless explicitly added.

## Teams and construction

Each stage supplies exact species, moves, items, abilities, stats, ace, and
battle order, reviewed for its stage rather than inheriting an endgame moveset.
Moves, items, and abilities stay attached to the right Pokémon when a team is
reordered. Rewards, prize money, badge scripts, and AI are preserved unless a
stage deliberately changes them. Trainer-species randomization keeps its
existing path; other randomizer and challenge options keep their precedence.

## Boundaries

- Gym members, ordinary trainers, wild encounters, and the player's cap keep
  their existing policies.
- Standalone builds are unchanged.

## Balance

Target: leaders average a couple of levels under the cap, spread within a few
levels either side. The
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) is being
reworked to show arcs and each leader's gap to the cap; it predicts species,
team size, and levels only. Combat balance needs reviewed content and ROM
playtesting before enablement, including checks that Gym members do not
routinely outclass their leader.

## References

- [Gym Leader scaling specification](../specs/gym-leader-scaling.md)
- [Trainer world progression](trainer-world-progression.md)
- [Ordinary Trainer and Gym-member scaling](trainer-party-scaling.md)
- [Player party progression](../specs/trainer-rating-party-progression.md)

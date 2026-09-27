# Gym Leader scaling

Implemented: No. The earlier player-rating Gym scaler exists in code but is
disabled by default, and Giovanni's Wayfarer finale has its own path. The ROM
keeps that behavior until this design is adopted.
Design status: v0 accepted; each leader's rating and team are provisional.

## Intent

Let players take the Gyms in any order and always find a fair, recognizable
fight. Each leader has a known strength: a low-rated leader is a good first
challenge, and a high-rated one is a clear goal to build towards.

## Design

A Gym battle uses the leader's own Trainer Rating, team, and levels, exactly as
[well-known trainer rating](trainer-world-progression.md) describes. The Gym
adds nothing on top: the same leader met anywhere else is the same trainer at
the same strength. A higher-rated leader brings more Pokémon at higher levels,
and the leader's signature Pokémon always comes last. Original FRLG, Emerald,
and HNS parties are references, not required teams.

The team is set when the battle starts and kept for the whole fight. Retrying
brings the same team at the same levels. The badge is awarded after the
battle.

## Coverage

Wayfarer has 24 badge encounters. This policy covers the 23 singles badge
opponents: Brock, Misty, Lt. Surge, Erika, Janine, Sabrina, Blaine, Giovanni
(his Viridian finale); Falkner, Bugsy, Whitney, Morty, Chuck, Jasmine, Pryce,
Clair; Roxanne, Brawly, Wattson, Flannery, Norman, Winona, and Juan.

Tate and Liza's double battle keeps its existing policy and badge; it is not
converted to singles. Blue has no badge encounter in Wayfarer.

## Teams and construction

Each leader's hand-written team supplies exact species, moves, items, and
abilities. Moves, items, and abilities stay attached to the right Pokémon when
a team is reordered. Rewards, prize money, badge scripts, and AI are preserved
unless a team deliberately changes them. Trainer-species randomization keeps
its existing path; other randomizer and challenge options keep their
precedence.

## Boundaries

- Gym members, ordinary trainers, wild encounters, and the player's cap keep
  their existing policies.
- Standalone builds are unchanged.

## Balance

Leader ratings and teams need content review and ROM playtesting before
enablement, including checks that Gym members do not routinely outclass their
leader. The
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) predicts
species, team size, and levels only.

## Later

- Leaders whose rating grows over the journey, and per-save variety between
  leaders ([Later](trainer-world-progression.md#later)).

## References

- [Gym Leader scaling specification](../specs/gym-leader-scaling.md)
- [Well-known trainer rating](trainer-world-progression.md)
- [Ordinary Trainer and Gym-member scaling](trainer-party-scaling.md)
- [Player party progression](../specs/trainer-rating-party-progression.md)

# Gym Leader scaling

## Intent

Let players challenge the 24 Gym Leaders across Kanto, Johto, and Hoenn in
different orders while facing teams appropriate to their Trainer Rating (TR).
Each leader keeps a recognizable, deliberately authored team: progression
changes how many Pokémon they bring and their levels.

## Design

Author one six-Pokémon roster for each initial badge battle, with the iconic
ace first in retention order. Put a second ace or essential partner next, then
the other members in order of importance to the leader's identity and tactics.
The game retains the first N entries. Every prefix from two through six must
work as a complete team; support required by an ace belongs in the first two.

Keep every member's exact authored species and form at every TR. Do not reverse
evolutions or evolve members automatically. Preserve existing hand-authored
movesets, including moves normally learned above the scaled level. Members
without custom moves use their normal level-up moves at their effective level.
New roster members need deliberately authored species, moves policy, and items.

Use these initial party-size thresholds:

| TR before battle | Pokémon | Global badges without League clears |
| --- | ---: | --- |
| 0 through 7 | 2 | 0 or 1 |
| 8 through 21 | 3 | 2 through 4 |
| 22 through 33 | 4 | 5 or 6 |
| 34 through 39 | 5 | 7 |
| 40 through 80 | 6 | 8 or more |

The first two badge challenges therefore use two Pokémon; the eighth uses
five. The next challenge after earning eight badges uses six. Badge counts
explain these examples but are not an additional scaling input.

Define battle order separately from retention order. Construct the selected
team in its authored battle order, normally placing the ace last. This is a
roster preference, not a scripted guarantee about the last Pokémon encountered:
existing battle AI remains free to choose replacements and make switches.

Tate and Liza share one double-battle team. Lunatone and Solrock occupy the
first two retention slots and are both aces. At the smallest size they open
together; larger teams can have other authored opening pairs. Their battle
always remains a double battle.

Read TR when battle setup starts. Keep that value for the fight, including
party reconstruction. A new attempt after a loss reads current TR again.

## Boundaries

Cover the initial badge battles for all 24 leaders, including their selectable
difficulty variants. Existing rematches, League opponents, rivals, villain
bosses, facilities, and other story battles retain their existing behavior.
Do not add rematch access or change badges, scripts, travel, League eligibility,
TR gains, or player progression. Standalone builds retain their existing teams.

Preserve authored held items, abilities, IVs, EVs, natures, trainer healing
items, and AI settings. They do not gain TR tiers in this version. Do not
rebalance existing custom movesets as part of implementing the scaler.

Trainer-species randomization retains its existing complete battle-party path;
this version does not apply the new leader transformation in that mode. Other
existing challenge and move-randomizer rules keep their normal precedence.

## Balance

Give leaders a separate TR-to-level curve, initially seeded with the same
values as the player's soft cap. Changing either curve must not change the
other implicitly.

| TR | Ace level |
| ---: | ---: |
| 0 | 15 |
| 4 | 16 |
| 8 | 18 |
| 16 | 23 |
| 30 | 30 |
| 40 | 42 |
| 55 | 60 |
| 65 | 80 |
| 80 | 100 |

Interpolate between anchors. Aces use that level; other members have an
authored offset of minus one or minus two. Both Tate/Liza aces use zero.
For example, at TR 0 a leader can bring a level 13 partner and a level 15 ace;
at TR 40 their full team spans levels 40 through 42.

The original roster's absolute levels do not affect this curve. The leader's
region, historical Gym order, and the player's current party levels or size
also do not affect it. Training can therefore still give the player an edge.
League clears increase TR and strengthen any remaining badge opponents.

These values are the initial implementation balance, subject to later reviewed
tuning. Evolved species with powerful custom moves can produce sharply
different challenges at equal levels. Record those outliers during playtesting;
do not silently replace moves, species, or add leader-specific level exceptions.

## Content

Expand and review the initial teams for Brock, Misty, Lt. Surge, Erika, Janine,
Sabrina, Blaine, Blue; Falkner, Bugsy, Whitney, Morty, Chuck, Jasmine,
Pryce, Clair; and Roxanne, Brawly, Wattson, Flannery, Norman, Winona,
Tate/Liza, and Juan. Tate/Liza count as one of the 24 badge encounters.

Each leader needs an explicit ace designation, retention order, battle order,
and level offsets. Reuse existing members and custom move tuples where
possible. Review new additions and any source replacement explicitly; filling
empty slots is content work, not a runtime random selection. This PRD defines
the authoring contract, not the final 144 Pokémon choices. Completing and
reviewing that roster inventory is required before production enablement.

## Interactions

The ordinary Trainer scaler remains responsible for ordinary Trainers and Gym
members. Gym Leaders use a separate policy so ordinary evolution reversal and
move replacement cannot change their authored identity.

Battle experience naturally follows actual opponents and levels. Preserve
existing prize-money inputs and badge rewards; expanding an authored roster
must not accidentally increase money through a new highest source level.
The feature adds no save fields or new player-facing menus or announcements.

This design supersedes static-party requirements only for the initial badge
battles in the related League and content designs. Their other opponent
policies remain in force.

## Playtesting

Check each leader as an early opponent, at every team-size transition, and
with a full roster. Pay particular attention to fully evolved aces, strong
custom moves, healing items, support dependencies, and Tate/Liza at two members.
Check that early Gym members do not consistently overshadow their leader.
Compare routes that postpone League challenges with routes that take them
as soon as eligible. Exercise party rebuilding and preparation after a loss.

## References

- [Gym Leader scaling specification](../specs/gym-leader-scaling.md)
- [Ordinary Trainer and Gym-member scaling](trainer-party-scaling.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Player party progression](../specs/trainer-rating-party-progression.md)

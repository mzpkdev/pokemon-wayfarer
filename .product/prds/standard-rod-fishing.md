# Standard Rod fishing progression

For Wayfarer, the approved [Native HM catch windows](native-hm-catch-windows.md)
revision replaces named-carrier accessibility assertions with eligible catches
that actually know the required utility at each TR and time. Its selected
[nearby-access proposal](../research/native-hm-windows/revisions/nearby-access/proposal.json)
requires an 8% chance at one reachable land source or successful Old Rod fishing
source, without summing across places. A qualifying fishing source therefore
provides at least 2% per unmodified cast. The code-only simulation passes its
selected scenarios; production acceptance remains pending.

Only the proposal's enumerated encounter replacements override the no-table-edit
boundaries below for Wayfarer. Global weights, bite rates, rod progression and
selection rules remain unchanged.

## Intent

Let the first fishing rod reach every species in a location's authored fishing
population. Better rods improve bite reliability and the odds of uncommon
catches instead of unlocking separate species pools.

The player carries only their best Standard Rod. This keeps rod progression
without filling the Key Items pocket with obsolete equipment, and it makes the
existing fishing content available without rewriting encounter tables map by
map.

This PRD covers only the HNS/Wayfarer build, including its connected Johto,
Kanto, and Hoenn regions.

## Design

Standard Rod is the product term for one fishing tool with three quality
states. The Bag presents the current state through the traditional item names:
Old Rod, Good Rod, and Super Rod.

Every quality state can select every eligible entry in the current map's
authored fishing table. The existing ten entries remain in their current order:

| Entries | Former partition | New role |
| --- | --- | --- |
| 0 and 1 | Old Rod | Common catches |
| 2 through 4 | Good Rod | Less common catches |
| 5 through 9 | Super Rod | Rare catches |

The former partitions describe rarity bands only. They no longer determine
which species a rod can encounter.

Each quality state has one global ten-entry weight profile shared by every map
and time-of-day variant in Wayfarer. The game selects directly
from the ten eligible entries using that profile. It must not first roll for a
former partition and then apply that partition's old internal weights, because
that would compound probabilities and make the last entries functionally
unavailable.

The profiles below total 100. When all entries are eligible, each value is the
entry's percentage among successful fishing encounters. If Trainer Rating
makes an entry ineligible, the game removes that entry and renormalizes the
remaining values.

| Quality | Entry 0 | Entry 1 | Entry 2 | Entry 3 | Entry 4 | Entry 5 | Entry 6 | Entry 7 | Entry 8 | Entry 9 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Old Rod | 38 | 22 | 10 | 8 | 8 | 4 | 3 | 3 | 2 | 2 |
| Good Rod | 25 | 18 | 12 | 10 | 9 | 7 | 6 | 5 | 4 | 4 |
| Super Rod | 12 | 10 | 11 | 10 | 10 | 10 | 10 | 9 | 9 | 9 |

The resulting rarity-band shares make each upgrade visible without removing
the common population:

| Quality | Common entries 0 and 1 | Less common entries 2 through 4 | Rare entries 5 through 9 |
| --- | ---: | ---: | ---: |
| Old Rod | 60% | 26% | 14% |
| Good Rod | 43% | 31% | 26% |
| Super Rod | 22% | 31% | 47% |

All eligible entries have a nonzero chance at every quality. Good Rod shifts
weight toward the middle and rare entries. Super Rod shifts it further toward
the rare entries. An upgrade never removes a species from the local fishing
population.

The base chance that a fish bites remains tied to rod quality:

| Quality | Base bite chance |
| --- | ---: |
| Old Rod | 25% |
| Good Rod | 50% |
| Super Rod | 75% |

Existing bite modifiers apply after the quality's base chance and keep their
existing cap.

All six existing rod givers in Wayfarer's connected world share one global
progression. They retain their locations and access conditions:

| Region | Contributor |
| --- | --- |
| Johto | Route 32 Pokémon Center Fishing Guru |
| Johto | Olivine City House 3 fisherman |
| Kanto | Route 12 house fisherman |
| Hoenn | Dewford Town fisherman |
| Hoenn | Route 118 fisherman |
| Hoenn | Mossdeep City House 3 fisherman |

No giver owns a fixed quality. Any first three distinct contributors, including
Hoenn-first and mixed-region routes, award the same progression:

| Prior successful contributors | Award from an unused giver |
| ---: | --- |
| 0 | Old Rod |
| 1 | Good Rod, replacing the Old Rod |
| 2 | Super Rod, replacing the Good Rod |
| 3 | No further upgrade; use capped dialogue or the existing follow-up |

The same giver cannot advance the rod twice. After contributing, that NPC uses
repeat dialogue or their existing follow-up activity. Declining an offer or a
failed transaction does not consume the contribution. After the third award,
unused givers acknowledge the completed rod or offer their follow-up; they do
not award another item, report an invalid state, or set a contribution flag.

Permanent per-giver flags record successful awards only. All six flags and the
current rod persist across regional travel. Traveling to Hoenn, Johto, or Kanto
never resets progression or starts a separate regional sequence.

The player owns at most one of the three rod items after an award or upgrade.
If the replaced rod appears in either registered-item shortcut slot, the
replacement remains registered in that slot.

The first Old Rod award requires a free Key Items slot. If that award fails,
the giver remains unused and the player can try again after making Bag space.
Good and Super upgrades reuse the lower rod's occupied slot and must not require
spare Key Item capacity. Replacement is atomic: any failure leaves the current
rod, registration, and giver state unchanged.

## Boundaries

The ten fishing entries already attached to each map remain unchanged. This
feature does not move species between entries, edit their authored levels, or
change land, Surf, Rock Smash, hidden, fixed, scripted, or facility encounters.
The separately selected Wayfarer catch-window replacements are the sole
exception; they do not authorize unlisted ecology changes.

Rod giver locations and their existing story or geography gates remain intact.
The redesign does not remove their personal dialogue, relationships, tutorials,
or side activities. Their item handling and mechanical explanations change to
support order-independent upgrades.

This feature does not redesign the fishing animation or reeling minigame. The
Easier Fishing option and the existing manual behavior for each displayed rod
quality remain available.

This feature targets new saves. It does not define conversion or compatibility
behavior for saves created before the feature is introduced.

## Balance

The Old Rod should make the full local fishing population real rather than
merely theoretical. When all ten entries are eligible, the least likely entry
must have at least a 0.5% chance among successful fishing encounters. Any
species used as a required traversal recovery must receive a separate
accessibility check and cannot rely on a trophy-level probability.

Under the selected profile, the least likely Old Rod entries have a 2% chance
among successful encounters. Required traversal recovery follows the approved
Native HM catch-window contract: an eligible catch must actually know the
required utility at the relevant Trainer Rating and time. One reachable source
must provide at least 8% per successful Old Rod encounter, or the qualifying
land-source chance, without adding probabilities across locations. A fishing
source therefore provides at least 2% per unmodified cast at the Old Rod's 25%
bite rate, averaging no more than 50 casts. Lure use is not required.

The selected nearby-source report owns the qualifying catches and includes
Surf and the Den's required Whirlpool acquisition. Ineligible-entry filtering
must preserve a qualifying nearby known-move source at every Trainer Rating
from 0 through 80 under that contract.

An upgrade should be noticeable during ordinary play. Good Rod should make the
former Good and Super entries collectively more common than they are with Old
Rod. Super Rod should further increase the former Super entries. Common entries
remain available at every quality so upgrading does not create a reverse
collection gate.

The selected profiles remain global rather than map-specific. Acceptance
requires a deterministic distribution report and playtesting that confirm the
authored species results, the rarity-band shifts, and the approved catch-window
accessibility thresholds.

## Content

Standard Rod itself reuses fishing tables without map-specific edits. The
Wayfarer catch-window revision owns its explicitly enumerated exceptions. Rod
content work is limited to rod items, rod giver scripts, and dialogue that currently
describes different rods as separate species unlocks.

Item descriptions should communicate the progression:

- Old Rod can catch any Pokémon living in the local fishing population.
- Good Rod improves the chance of uncommon catches and the chance of a bite.
- Super Rod provides the best chance of rare catches and the chance of a bite.

Rod giver dialogue must work whether that NPC is the first, second, or third
specialist the player visits. Each conversation keeps the NPC's personal and
regional flavor, then uses progression-aware text for the award:

- A first contributor introduces fishing and gives the Old Rod.
- A second contributor recognizes the player's experience and improves the rod
  to Good Rod quality.
- A third contributor completes the rod's improvement to Super Rod quality.
- A used contributor switches to repeat dialogue or their follow-up activity.
- An unused contributor met after Super Rod acknowledges the completed rod
  without an award or a failure message.

Dewford keeps its fishing tutorial. It must remain useful whether he gives the
first rod, either upgrade, or meets a player whose rod is already complete.
Olivine keeps the seaside setting and the fisherman's 30 years of experience
as the basis for his help.

Dialogue must not identify a location with a fixed rod tier or claim that
different rods reveal exclusive groups of Pokémon. Lines that discuss quality
should instead explain better bite rates and improved odds for uncommon catches.

## Presentation

The Key Items pocket shows only the player's current rod quality. Upgrading
uses the ordinary item-received presentation, then replaces the previous rod
without requiring the player to discard or unregister it. The award text and
fanfare use the quality granted in the current playthrough, not the giver's
original tier.

The traditional Old Rod, Good Rod, and Super Rod names, icons, fishing
animations, and item-use flow remain. Standard Rod does not need to appear as a
player-facing term.

Pokédex and other ordinary population displays should treat all ten authored
entries as part of the location's fishing population regardless of the current
rod quality. They do not need to display exact slot probabilities.

## Interactions

- Trainer Rating determines entry eligibility before the unified weighted
  roll. After selection, level projection and predecessor resolution determine
  the encounter outcome as they do for other ordinary encounters.
- If one or more entries are ineligible, the game renormalizes the current
  quality's weights across the eligible entries. A selected empty or locked
  entry must not turn a successful bite into a silent failure.
- Time-of-day variants retain their own ten authored entries and use the same
  quality profile.
- Randomizer mode receives the selected authored entry and raw entry index as
  it does for other ordinary encounters.
- The special Route 119 Feebas check remains separate from normal table
  selection and works with every rod quality.
- Sticky Hold, Suction Cups, follower friendship, and configured fishing boosts
  continue to modify bite chance without changing the local species pool.
- An active Lure retains its existing 20% slot-reversal behavior across the
  unified eligible sequence. This intentionally favors the rare end of the
  ten-entry pool and must be balanced alongside the three quality profiles.
- Easier Fishing continues to skip the reactive reeling sequence. Manual
  Fishing keeps the existing timing and repeat-round behavior associated with
  the displayed rod quality.
- Ordinary encounter readers use the same unified eligible population as an
  actual fishing encounter.

This feature supersedes the Old, Good, and Super partition-selection rule in
the Trainer Rating wild encounter scaling design. Trainer Rating continues to
own the effective level and species outcome after an authored entry is
selected.

## Constraints

The feature must preserve the existing ten-entry fishing data shape. It may add
global quality profiles and change runtime selection, but it must not require
new per-map fishing data.

Rod quality uses the existing mutually exclusive rod items rather than a new
quality save field. Wayfarer needs six distinct permanent contribution flags,
one for each giver. The number of set flags determines the next award, and
the physical rod item determines the quality available for use.

The award or replacement, transfer of either registered-item shortcut, and the
giver's contribution flag form one atomic transaction. The giver flag is set
only after the item and registration changes succeed. A failed transaction
leaves all three unchanged and remains retryable.

Contribution state must use dedicated nonzero flags, not current rod possession
or regional item-received aliases. Regional transitions, including the S.S.
Aqua transition, must never clear any of the six contribution flags.

Acceptance covers all 120 ordered selections of three distinct givers from the
six, plus refusal, repeat visits, Hoenn-first routes, mixed-region routes,
travel between awards, and unused givers after Super Rod. Capped Dewford visits
must retain access to the tutorial without consuming his contribution.

## References

- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Trainer Rating wild encounter scaling specification](../specs/trainer-rating-wild-encounter-scaling.md)
- [Standard Rod fishing specification](../specs/standard-rod-fishing.md)
- [Authored under-level wild encounters](../research/authored-under-level-wild-encounters.md)
- [Wild encounter data](../../game/src/data/wild_encounters.json)
- [Fishing configuration](../../game/include/config/fishing.h)

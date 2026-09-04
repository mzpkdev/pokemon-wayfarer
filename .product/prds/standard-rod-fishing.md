# Standard Rod fishing progression

## Intent

Let the first fishing rod reach every species in a location's authored fishing
population. Better rods improve bite reliability and the odds of uncommon
catches instead of unlocking separate species pools.

The player carries only their best Standard Rod. This keeps rod progression
without filling the Key Items pocket with obsolete equipment, and it makes the
existing fishing content available without rewriting encounter tables map by
map.

This behavior applies to Emerald, FireRed and LeafGreen, and HNS.

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
and time-of-day variant in every supported build. The game selects directly
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

The three existing rod givers in each build retain their locations and
progression conditions, but no giver owns a fixed quality. Each giver can
contribute once. The quality awarded depends on how many distinct rod givers
have already contributed in that build:

| Prior contributors | Award from an unused giver |
| ---: | --- |
| 0 | Old Rod |
| 1 | Good Rod, replacing the Old Rod |
| 2 | Super Rod, replacing the Good Rod |

The same giver cannot advance the rod twice. After contributing, that NPC uses
repeat dialogue or their existing follow-up activity. Declining an offer does
not consume the contribution, and a failed item transaction does not mark the
giver as used.

The reward is based on permanent per-giver contribution state, not the
player's current item possession. This makes every order valid while preserving
a reason to find three different fishing specialists.

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
among successful encounters. Every species relied upon for native Surf
coverage has at least an 8% aggregate chance among successful Old Rod
encounters at its named source and at least a 2% chance per unmodified cast
after the Old Rod's 25% bite rate. This limits the least accessible required
user to an average of 50 casts. Lure use is not required for this guarantee.

The baseline accessibility results are:

| Build and source | Native Surf user | Chance per successful Old Rod encounter | Chance per unmodified cast | Average casts |
| --- | --- | ---: | ---: | ---: |
| FireRed and LeafGreen, Pallet or Cinnabar | The version's less common Horsea or Krabby assignment | 8% | 2% | 50 |
| FireRed and LeafGreen, Pallet or Cinnabar | The version's more common Horsea or Krabby assignment | 14% | 3.5% | 28.6 |
| HNS, Olivine port, Vermilion, or Cinnabar | Chinchou | 11% | 2.75% | 36.4 |
| HNS, Cianwood during the day | Chinchou | 12% | 3% | 33.3 |
| Emerald, Lilycove | Wailmer | 19% | 4.75% | 21.1 |
| Emerald, Mossdeep or Pacifidlog | Wailmer | 18% | 4.5% | 22.2 |

These figures aggregate duplicate entries for the named species and do not
count any additional availability from species resolution. Ineligible-entry
filtering cannot be allowed to remove a required native Surf source at any
standalone Trainer Rating from 10 through 80 or Wayfarer Rating from 0 through
80.

An upgrade should be noticeable during ordinary play. Good Rod should make the
former Good and Super entries collectively more common than they are with Old
Rod. Super Rod should further increase the former Super entries. Common entries
remain available at every quality so upgrading does not create a reverse
collection gate.

The selected profiles remain global rather than map-specific. Acceptance
requires a deterministic distribution report and playtesting that confirm the
authored species results, the rarity-band shifts, and the native Surf
accessibility values above.

## Content

All existing fishing tables are reused without map-specific edits. The content
work is limited to rod items, rod giver scripts, and dialogue that currently
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

The FireRed and LeafGreen givers remain members of the Fishing Guru family. A
successful contribution from the Route 12 giver unlocks his Magikarp size-record
activity regardless of whether he awarded the Old, Good, or Super Rod. Emerald's
Dewford giver keeps his fishing tutorial, and the tutorial must remain useful
even when he is the second or third contributor. HNS Olivine keeps the seaside
setting and the fisherman's 30 years of experience as the basis for his help.

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
quality save field. Each build needs one permanent contribution flag for each
of its three rod givers. The number of set flags determines the next award, and
the physical rod item determines the quality available for use.

The award or replacement, transfer of either registered-item shortcut, and the
giver's contribution flag form one atomic transaction. The giver flag is set
only after the item and registration changes succeed. A failed transaction
leaves all three unchanged and remains retryable.

HNS must not use current rod possession as the contribution state for its Old
and Good Rod NPCs. Its three giver states must remain permanent across regional
progression; a flag cleared during the Kanto transition cannot serve as the
Route 12 contribution flag.

## References

- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Trainer Rating wild encounter scaling specification](../specs/trainer-rating-wild-encounter-scaling.md)
- [Authored under-level wild encounters](../research/authored-under-level-wild-encounters.md)
- [Wild encounter data](../../game/src/data/wild_encounters.json)
- [Fishing configuration](../../game/include/config/fishing.h)

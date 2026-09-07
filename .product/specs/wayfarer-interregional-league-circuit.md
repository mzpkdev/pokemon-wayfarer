# Wayfarer interregional League circuit

PRD: [Wayfarer interregional League circuit](../prds/wayfarer-interregional-league-circuit.md)
Implemented: Partially. The original circuit is implemented; this revision's
unrestricted badge collection, regional prerequisite repairs, and Trainer Card
presentation are approved and pending implementation.

## Scope

This specification defines Wayfarer's global badge count, unrestricted badge
collection, fixed Kanto to Johto to Hoenn League sequence, League eligibility, and
Trainer Rating inputs. This release uses the existing Johto opening as
Wayfarer's sole new-game start and defines the travel guarantees needed by the
circuit. It supersedes build-specific League entry and Trainer Rating
progression in the completed Wayfarer circuit. Kanto and Hoenn new-game starts
are separate future features.

Regional badge storage, Champion state, and local story dispatch remain owned
by the Wayfarer runtime foundation and regional content specifications. The
ordinary wild population and level projection remain owned by the Trainer
Rating wild encounter scaling specification. Soft level caps, experience
reduction, and obedience remain owned by the Trainer Rating party progression
specification.

## Behavior

### Global badge count

The global badge count is the sum of the earned Kanto, Johto, and Hoenn badge
states:

```text
globalBadges = kantoBadges + johtoBadges + hoennBadges
```

Each regional count is between zero and eight, so the global result is between
zero and twenty-four. The calculation reads the region-aware badge helpers and
does not maintain a separate saved count. Re-reading an earned badge or running
its award script again cannot increase the result.

Global aggregation does not change the meaning of a regional badge. Local
story scripts continue to read their regional state unless this specification
explicitly assigns a global circuit check.

### New-game opening and regional travel

The existing Johto opening is the sole new-game start for this release. It
initializes the three regional state banks with zero global badges, no regional
Champion or game-clear result, and Trainer Rating 0, then releases the player
through the established Johto settlement network and starter transaction.

The Johto start supplies the only required travel entitlement: its S.S. Aqua
maiden voyage starts without a Ticket, awards the Ticket during the existing
reunion flow, and reaches state 8 on arrival at Vermilion. State 8 plus the
Ticket permits the fixed Olivine -> Vermilion -> Slateport -> Olivine circuit.
That route must remain available without a badge, League clear, or unrelated
story requirement.

Kanto and Hoenn new-game openings, starters, recovery points, and early ship
credentials are future-scoped. They do not gate this circuit, its production
switch, or any eligibility predicate.

### Badge collection and regional prerequisites

All twenty-four badges can be earned before any League clear. Gym entry,
initial reward-bearing challenges, and deferred badge awards must not inspect
a certification cap or require Champion status. Having eight, sixteen, or
twenty-four badges with no clears is a valid state. Existing one-time badge,
reward, and story effects remain idempotent.

Coherent regional quests retain their own prerequisites. This includes
Jasmine's medicine errand, Misty's Power Plant and Route 25 sequence, and Juan's
weather/Rayquaza story. Clair retains Chuck, Jasmine, and Pryce's badges as
prerequisites; Norman retains four Hoenn badges and Wally's tutorial. These
rules do not promise that every Gym is available in every order.

Prerequisites must be discoverable and recoverable when the player arrives
late or mixes regions. Progress must not depend on hitting an exact shared
badge count in one of a few Gym scripts, or returning merely to activate an
unrelated earlier flag. A relevant location or interaction must recognize
already-satisfied prerequisites on revisit, trigger an event only once, and
never regress completed story state.

The Rocket takeover trigger must use explicit regional story prerequisites
and completion state. Replace the exact-seven shared Kanto/Johto badge check;
changing it to `globalBadges >= 7` is insufficient because unrelated regional
badges must not substitute for story prerequisites. Implementation must identify
and test the regional prerequisite predicate against the Mahogany/Radio Tower
sequence before implementing it; the exact prerequisite set still needs that
audit. Include late arrivals, revisits, and an already-completed takeover, and
ensure earlier event writes such as Whitney's cannot overwrite later takeover
state.

Blue's Cinnabar interaction is sufficient to invite him back to his Gym. It
must not require the other fifteen Kanto/Johto badges or any League clear.
Wattson's New Mauville relocation requires both Norman's defeat and the Dynamo
Badge. Recognize either completion order, including earning Dynamo after
Norman's one-time victory event; an undefeated Wattson remains in his Gym.
Preserve the initial-badge protections already present for Chuck, Blue, Clair,
and Blaine. Whitney and Clair's deferred reward interactions must remain
reachable and award exactly once regardless of global badge count or clears.

### League eligibility and order

League challenges are enabled only by the following predicates:

| League | Eligibility |
| --- | --- |
| Kanto | At least 8 global badges and Kanto League not cleared |
| Johto | Kanto League cleared, at least 16 global badges, and Johto League not cleared |
| Hoenn | Kanto and Johto Leagues cleared, all 24 global badges, and Hoenn League not cleared |

Badge origin is not part of any predicate. A local badge count, local story
completion, another region's Champion state, or a generic game-clear flag
cannot substitute for or strengthen these requirements.

The applicable League entrance and challenge scripts use these predicates.
Before qualification, they leave the League uncleared and may state the
immediate unmet admission requirement. They do not display a circuit overview.
After qualification, no regional story or local badge check may prevent access
to the League challenge.

The travel graph must provide a usable route from the Johto start and every
possible threshold-badge location to the assigned League. Completing that
League must release the player at a location connected to the wider regional
travel network. The route may be directional and a direct fast-travel option is
not required. Reaching the League cannot require another badge or the clear
currently being pursued.

Completing a League atomically records that region's Champion and game-clear
state, applies only that region's Hall of Fame and cleanup behavior, advances
the global circuit, and preserves every badge and all other regional state.
Out-of-order League completion is unreachable through normal play. At
twenty-four badges and zero clears, each clear immediately enables the next
League without any intervening badge or story transaction.

Kanto and Johto share the HNS Indigo venue and opponent lineup. Their existing
Tier 1 and Tier 2 party selection must still work when challenged consecutively.
The Hall of Fame return must reset the venue for the next eligible tier without
carrying over room, defeat, or pending-clear state that skips or blocks it.
Save/load and loss/retry must preserve the appropriate tier and consume a
successful clear handoff only once.

### League and Trainer difficulty

The Kanto League uses its authored Tier 1 parties, the Johto League uses its
authored Tier 2 parties, and the Hoenn League uses its authored Tier 3 parties.
The selected parties do not depend on Trainer Rating, badge distribution,
starting region, current party, or prior losses.

This revision preserves the current static party sets. League strength and
tuning for early or delayed participation belong to a future PRD and are not
acceptance requirements for this revision.

Ordinary Trainers and Gym members retain authored roster selection and apply
the separate [Trainer-party scaling specification](trainer-party-scaling.md).
Initial Gym Leader badge battles follow the separate [Gym Leader scaling
specification](gym-leader-scaling.md). Leader rematches retain their existing
authored parties and never select a party from global badge count, League
progress, or Trainer Rating.

### Trainer Rating

Wayfarer Trainer Rating is an integer from zero through eighty. A new Wayfarer
game initializes the saved value to zero. Reads clamp it to that range and
preserve the higher of the saved rating and the rating derived from current
facts.

Let `b` be the global badge count. The badge contribution is:

```text
0 <= b <= 4:   4 * b
5 <= b <= 8:   16 + 6 * (b - 4)
9 <= b <= 24:  40 + (b - 8)
```

League clears add these fixed contributions:

| Clear | Contribution |
| --- | ---: |
| Kanto League | 15 |
| Johto League | 5 |
| Hoenn League | 4 |

The derived result is clamped to eighty. The formula remains unchanged. Taking
each League at its minimum badge requirement gives these milestones:

| Facts | Derived rating |
| --- | ---: |
| No badges or League clears | 0 |
| 4 badges | 16 |
| 8 badges | 40 |
| 8 badges and Kanto clear | 55 |
| 16 badges and Kanto clear | 63 |
| 16 badges and Kanto and Johto clears | 68 |
| 24 badges and Kanto and Johto clears | 76 |
| 24 badges and all three Leagues cleared | 80 |

Taking all badges before any League gives these additional required states:

| Facts | Derived rating | Soft level cap |
| --- | ---: | ---: |
| 24 badges, no clears | 56 | 62 |
| 24 badges and Kanto clear | 71 | 88 |
| 24 badges and Kanto and Johto clears | 76 | 95 |
| 24 badges and all three clears | 80 | 100 |

Soft level caps remain defined by the party progression specification; these
values document the effect of keeping its existing formula.

The value remains a high-water mark. It scales ordinary wild encounters through
the existing projection pipeline and determines the soft level cap through the
Trainer Rating party progression specification. Ordinary Trainers and Gym
members consume it through the Trainer-party scaling specification. Enrolled
initial Gym Leader badge battles consume it through the separate [Gym Leader
scaling specification](gym-leader-scaling.md); other excluded bosses, leader
rematches, and League parties remain authored.

The HNS Chinchou learnsets add `Flash`, `Surf`, and `Whirlpool` at level 5 in
both normal and legacy-moves mode, after any existing level-5 entries. The
later repeat entries remain unchanged. In the HNS modern learnset only, the
otherwise final level-50 `Charge` entry is omitted so the added level-5 triplet
and all utility repeats fit below the forty-entry engine limit; non-HNS
learnsets retain `Charge`. This ensures that the authored level-5 Chinchou
fishing sources around Vermilion and Cinnabar still provide the native Surf user
required by Kanto traversal at Rating 0.

### Presentation

Select on the local Trainer Card opens a repeatable circuit view; B or Select
returns to the card. A visible Select hint makes the view discoverable. The
view reports the global badge total out of twenty-four and all three Leagues:

| League | Badge minimum | Prerequisite clears |
| --- | ---: | --- |
| Kanto | 8 | None |
| Johto | 16 | Kanto |
| Hoenn | 24 | Kanto and Johto |

Each League displays `Cleared` when its clear is recorded, `Available` when its
eligibility predicate is true, and `Locked` otherwise. Requirements remain
readable in every state so the player can distinguish a badge shortfall from a
missing prior clear. The layout must fit legibly in the Trainer Card; if pages
are needed, their controls must be visible. At twenty-four badges with no
clears, Kanto is available while Johto and Hoenn are locked by prerequisite
clears. After Hoenn, all three remain visible as cleared and the view reports
`Circuit complete` with no next destination.

Remove automatic circuit announcements from the opening, every badge-award
path, and every League-clear continuation. Do not add milestone popups or NPC
copies of the status overview. League admission may explain only its immediate
unmet requirement when the player requests entry. Normal story, badge reward,
and Hall of Fame scenes remain; they must not append a circuit status message.
Remove certification-cap language and dialogue that assumes Champion status
from badge progress alone, including Blue's Johto Champion address when the
player has no such clear.

Regional badge displays continue to identify each badge and its region. The
global status supplements rather than replaces those displays.

### Validation

Deterministic tests must cover:

1. Every distribution of zero through twenty-four badges across the three
   regions and the resulting deduplicated global count.
2. Initial badge access across all twenty-four Gyms with zero League clears
   when their retained regional prerequisites are satisfied. No certification
   check remains in entry, challenge, or award paths.
3. Badge awards crossing totals eight and sixteen without a clear, continuing
   through twenty-four. Whitney and Clair's deferred rewards stay reachable,
   commit once, and preserve their normal reward and story effects.
4. Mixed qualification such as four Johto plus four Hoenn badges for Kanto.
5. Rejection of Johto and Hoenn League challenges before their prerequisite
   clears even when the player has enough global badges.
6. Acceptance of each League with the exact threshold and no badges from its
   host region where such a distribution is possible.
7. Preservation of all badges and unrelated regional state after each League
   clear.
8. Trainer Rating derivation at every milestone and clamp boundary, including
   new-game Rating 0 and final Rating 80.
9. High-water behavior after regional cleanup, repeated reads, save and load,
   and a League loss.
10. Static excluded-boss party selection at several badge distributions,
    ratings and circuit tiers. Ordinary Trainer and Gym-member projection
    follows its separate specification.
11. Regional prerequisites in mixed badge orders: a Kanto Gym supplying shared
    badge seven cannot make the Rocket takeover missable. Cover arriving after
    that count, revisit recovery, once-only triggering, and no regression after
    completion. Verify that unrelated badges do not replace regional story
    prerequisites. Cover Blue's Cinnabar invitation before fifteen HNS badges,
    Norman before Wattson, and all retained Leader protections.
12. Eighth-badge routes ending in Kanto, Johto, and Hoenn can all reach the
    Kanto League; equivalent sixteenth-badge routes with Kanto cleared can reach
    the Johto League; and twenty-fourth-badge routes with both previous clears
    can reach the Hoenn League. Each clear
    releases the player back into the wider regional travel network.
13. The Johto opening initializes a clean three-region save at Rating 0, gives
    the player the existing starter transaction, and its maiden-voyage reward
    path reaches Kanto and the completed Aqua route to Hoenn.
14. At Wayfarer Ratings 0 through 80, the Standard Rod and production encounter
    pipeline keep the named Vermilion and Cinnabar Chinchou sources eligible at
    their required probabilities, and a caught Chinchou knows Surf.
15. The Trainer Card shows badge totals, every League's requirements, and all
    locked/available/cleared combinations reachable in normal progression,
    including twenty-four badges without clears and final completion. Check
    Select/B controls, legibility, and preserved regional badge displays.
16. Source wiring and gameplay checks confirm no automatic circuit overview at
    the opening, any badge award, or any Hall of Fame return, and no NPC status
    duplicate. Admission refusals give only the immediate unmet requirement.
17. Consecutive Kanto, Johto, and Hoenn clears starting with twenty-four badges
    and no clears. Verify shared Indigo tier and room resets, loss/retry,
    save/load between clears, one-shot Hall of Fame handoffs, and ratings
    56, 71, 76, and 80 with soft caps 62, 88, 95, and 100.

A gameplay traversal from the Johto opening must earn all twenty-four badges
with zero League clears, completing retained regional prerequisites in mixed
orders, then clear the Leagues consecutively. Seeded badge tests alone do not
prove that the initial badges are reachable. Record source audit results and
emulator evidence separately; an untested route is not a confirmed softlock or
a proven accessible path.

The ordinary encounter balance audit covers Wayfarer ratings zero through
eighty. Implementation fails acceptance if an approved core-route native
utility source becomes unavailable or loses its required move at any rating.

## References

- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Trainer Rating party progression](trainer-rating-party-progression.md)
- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [Kanto wild encounters](kanto-wild-encounters.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](standard-rod-fishing.md)

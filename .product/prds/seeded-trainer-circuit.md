# Seeded Trainer Circuit

Implemented: No
Design status: Recurring regional championships, trainer-owned world progression,
and projected whole-edition snapshots are approved directions. Numeric balance,
production content, qualification, and supporting defaults remain under review.

## Intent

Make Indigo, Sevii Masters, and Hoenn recurring championships with recognizable
competitors and changing fields. Each edition visits all three venues in a
seeded order. Every venue hosts two contenders, two elite competitors, and one
headliner. Strength follows the trainer's personal development at that stop;
no region is permanently the weak or final championship.

## World progression revision and balance explorer

[Trainer world progression](trainer-world-progression.md) owns starting
`baselineTR`, personal badge growth, and first-lifetime-clear growth. A trainer's
effective TR determines competitive eligibility and battle levels. World Gym
encounters use live milestones; circuit encounters use saved projected snapshots.
Both use trainer-owned ratings rather than the player's TR or party levels.

Original FRLG, Emerald, and repository HNS parties supply references. Supported
Gym Leaders need approachable opening stages even when their original encounter
was late-game. Blue starts approachable and develops toward Champion strength
sooner through his own curve. Authored party stages preserve recognizable
identity while deliberately changing membership and species over progression.

The [balance explorer](../../devtools/ui/README.md#trainer-balance-explorer)
provides 37 experimental singles records, badge/first-clear controls, editable
curves and stages, original-party references, and saved/exportable experiments.
Its numeric values are provisional. It models species, party size, and levels;
it does not generate leagues or certify production moves, items, AI, or battle
balance. The target contracts below no longer use static NPC strength.

## Fields and eligibility

| Battle slots | Role | Count per venue |
| --- | --- | ---: |
| 1–2 | Contender | 2 |
| 3–4 | Elite competitor | 2 |
| 5 | Headliner | 1 |

Author three ordered, disjoint inclusive effective-TR bands for each supported
world point. At the same badge/first-clear point, every venue uses the same
bands. D3 owns their endpoints and content review. A later stop may have a
stronger field because mandatory first clears advance the world, not because
Indigo, Masters, or Hoenn has a permanent difficulty tier.

Resolve every candidate's personal TR and applicable stage/profile at the
stop's projected world point before eligibility. A Gym Leader can qualify as
a headliner; an Elite Four or Champion title does not guarantee a place. Sort
each contender/elite pair by saved effective TR, using canonical character ID
to break ties. The headliner band is above the other two bands.

Keep starting TR believable for its opening encounter. Do not raise a baseline
to force league attendance or use baseline instead of effective TR. Party
content must pass the consumer's requirements; six-member competitive singles
profiles remain the proposed circuit default. Opening Gym stages and incomplete
prototype Elite Four parties do not automatically satisfy that requirement.

Use supported Kanto, Johto, and Hoenn characters. Five distinct canonical people
appear within each league. Multiple appearances across venues remain allowed,
with a soft weight penalty. Aliases, different teams, costumes, and titles do
not create new people. Red remains a separate mastery encounter; Tate and Liza
remain outside the singles pool. Story availability does not remove a trainer
from sanctioned competition or enroll their story encounters in this policy.

## Home leagues and rotation

Each trainer has an authored `homeLeagues` set with a lore rationale. Multiple
home leagues are allowed; map location alone does not establish membership.
After effective-TR/content eligibility, remove people already chosen at that
venue, then partition candidates into home and visitor pools.

When both pools contain candidates, select home with 85% probability or visitors
with 15%, using a seeded per-slot category draw. When no visitor qualifies, use
home. Every slot must have an eligible home option; missing home content is an
error rather than permission to force a visitor or relax a band. Home status
has no effect on rating or team strength.

The category chance is independent of pool sizes and participation weights.
It is neither an individual appearance probability nor an exact visitor quota;
a field may contain zero, one, or several visitors. There is no 50% outsider
drop gate, visitor cap, reserved guest, or additional home multiplier.

Build all three fields together. Within the selected category, start with flat
base weight and propose these D5 factors:

| Condition | Factor |
| --- | ---: |
| Already scheduled at zero / one / two venues in this edition | 16 / 4 / 1 |
| Appeared at this venue in the immediately previous completed edition | 1 |
| Did not appear there, or this is edition 1 | 2 |

Multiply applicable factors. All eligible candidates keep positive weight;
no participation preference overrides TR eligibility or the category draw.
Missing an edition at that venue clears the prior-participation penalty.
Gym fights, retries, replays, and room visits do not count as circuit participation.

Roughly two returning and three different opponents per venue is a tuning
target, not a quota. A trainer may return in any later edition, and consecutive
fields may coincide. Never reroll or increment an edition to force novelty.
Rotation must remain useful when personal ratings mature and stop changing.

## Order, registration, and strength snapshots

At new game, persist the shared playthrough root and the first edition's seeded
venue order. This is a valid unregistered circuit state with no trainer lineup.
Do not select league contestants from their opening ratings. Early itinerary
views reveal travel order and qualification, but no unregistered roster.

At first eligible registration, capture badge count, lifetime venue-clear
identities, content/growth/band versions, and the required participation history.
Resolve all fifteen slots atomically. At stop i, project the lifetime clears
already owned plus all earlier venues in that seeded order. Count the union,
so a venue contributes at most once. Badges remain at the registration count.

| Edition context | First stop clear count | Second stop | Third stop |
| --- | ---: | ---: | ---: |
| First fresh edition | 0 | 1 | 2 |
| Later editions after all lifetime clears | 3 | 3 | 3 |

Evaluate each candidate at that projected world point before selecting them.
Save selected effective TR and stage/profile references with the registration
inputs and schedule versions. A person selected at two first-edition stops can
therefore have different ratings and teams. The same saved rating and profile
still reconstruct identical ordinary strength wherever used.

Retries, departures, reloads, and replays retain that complete schedule and its
strength. A replay is an exhibition of the registered edition; live world Gym
opponents may have progressed further. Existing Pokémon gameplay RNG and
explicit randomizers keep their current behavior.

After all three results commit and no run or ceremony is pending, manually
register the next edition. Generate using the same root, next edition identity,
current milestones, and outgoing lineups as prior-venue history. Commit the
new schedule, history, identity, and empty current results together. Failure
preserves the old edition and consumes no identity. Counter overflow refuses
registration. There is no calendar wait, automatic rollover, cancel-to-reroll,
next-edition preview, or skip path.

Edition count itself adds no strength. After all lifetime clears, further
editions supply new matchups through the seeded selection and rotation rules.

## Qualification and player progression

D4 proposes all 24 global badges before first registration/admission. Later
stops require the previous current-edition result. Travel and badge collection
remain independent of the seeded order. The 24-badge gate is a proposed balance
choice, not an approved change to current ROM admission.

The snapshot model holds badges constant because this gate exhausts badge
progression before registration. Adopting a different gate requires an explicit
badge-projection contract and balance review; changing a threshold alone is
insufficient.

Retain the existing player +8 TR reward for each first lifetime venue clear,
at most three contributions. Under the proposed gate, player TR progresses
56 → 64 → 72 → 80, with soft caps 62 → 78 → 89 → 100. NPC growth follows each
trainer's separate curve and first-clear amount. Sharing milestones does not
mean sharing a rating, cap curve, or growth rate.

Later editions and replays provide ordinary battle rewards without new player
TR contributions. Replays do not grant another edition result, Hall of Fame
entry, Champion Ribbon, or completion credits. Badges, lifetime clear facts,
player TR high-water, recognition, regional cleanup, and Blue/Red access survive
edition rollover.

## Records, presentation, and regional integration

Save the current schedule and bounded prior-edition roster IDs by venue, not
unlimited old teams or a lifetime participation blacklist. Propose a bounded
completed-edition counter. Current results and lifetime facts have distinct
owners; regional projections cannot substitute for either.

Show seeded order and unregistered status from new game. After registration,
show all five names, assigned TR, specialties, and battle order for each venue
before entry, including future stops. Explain that those strengths include the
projected progression to that stop. Preview reads saved state and reveals no
uncommitted future edition. Moves and held items are not revealed by default.

Actors, portraits, names, dialogue, battles, and ceremonies follow selected
characters. A displaced fixed opponent must not remain in room dialogue. Present
a Gym Leader headliner as the final opponent without inventing Champion history.

Indigo projects shared Kanto/Johto Champion recognition on its first-ever clear;
Hoenn owns regional recognition/cleanup; Masters owns its result without a
regional Champion title. Propose one winning-team Hall of Fame and Champion
Ribbon flow per Indigo/Hoenn edition result, and the Masters Gallery for its
result. Regional cleanup remains once per lifetime. Edition 1's final result
triggers full credits; later editions use a brief completion presentation.

Red unlocks after all three lifetime venue clears and remains available.
Retain the proposed Blue Saffron Dojo unlock after the first committed lifetime
venue clear, regardless of venue or Blue's participation. His other authored
battles and rewards remain separately owned.

## Content and validation

Review identities, actual encounter aliases, home rationales, personal curves,
team stages, competitive profiles, role bands, and presentation together. Prove
home-only 2/2/1 feasibility at every supported projected world point and seeded
order, including C=0–3 and TR saturation. Bands must remain ordered and disjoint;
missing candidates block content enablement. Do not widen bands at runtime,
invent memberships, force visitors, or substitute a fixed roster.

Feasibility does not prove variety. Evaluate at least 10,000 documented roots
over ten editions, measuring returning/changed fields, cross-venue repetition,
eligible opportunities per character, absence runs, home/visitor share, and
strength distribution. Separate raw category odds from fallback outcomes.
There is no complete-attendance quota or guaranteed Elite Four/Champion turnout.

Playtest each possible first venue, later projected stops, and fully progressed
editions against the player-cap curve. Check stage transitions and effective
strength, not only numeric TR. The explorer does not establish combat balance.

## Persistence and adoption

The shared seed framework owns the root, derivation, unbiased draws, and golden
vectors. The circuit owns edition/order, registration inputs, projected snapshots,
current results, and bounded history. Resolve an outcome only from its semantic
keys and declared content/history/milestone inputs; other seeded features and
ordinary gameplay RNG cannot perturb it. Store no circuit random cursor.

The runtime and pool specs pin schedule schema and decision versions. Roster
rules changes need their own version without invalidating unrelated ORDER or
POOL_KIND keys. Missing/corrupt registered snapshots are invalid saves, not
opportunities to regenerate from live progress. A valid unregistered state is
explicit and distinguishable from corruption. Follow prerelease save policy;
no migration of obsolete prerelease layouts is required.

This target supersedes fixed roster/order, player-entry-TR circuit strength,
and enrolled singles Gym player-TR/prefix rules upon adoption. Current runtime
behavior stays documented separately until implementation. Player progression,
ordinary trainers/Gym members, wild encounters, shops, standalone builds, and
unenrolled story/rematch battles retain their contracts. Historical research
reports remain evidence for the builds tested, not implementation of this target.

## Decisions before implementation

D1 is resolved as a design direction: personal NPC growth, world-Gym snapshots,
and projected whole-edition league snapshots. D2 is resolved: headliners qualify
by effective TR rather than historical Champion title. Singles, home leagues,
85%/15% regional selection, recurring editions, and soft rotation are confirmed.

| ID | Remaining decision | Proposed default |
| --- | --- | --- |
| D3 | Personal curves, stages, NPC anchors, world-point role bands, production content and balance? | Supported Kanto/Johto/Hoenn singles, six-member competitive circuit profiles; use explorer data as provisional evidence and prove local feasibility through growth/saturation. |
| D4 | Qualification and progression balance? | All 24 badges before first registration, mandatory predecessor clears afterward; retain lifetime player +8 per venue. |
| D5 | Rotation factors and measured variety acceptance? | Flat in-role weights, 16/4/1 current-edition factors and 1/2 prior-venue factors; roughly two returning/three changed without quotas or rerolls. |

Supporting presentation, reception registration, bounded records, ceremony,
Ribbon, and Blue unlock defaults remain reviewable. Resolve content and balance
acceptance before enabling the new ROM behavior.

## References

- [Trainer world progression](trainer-world-progression.md)
- [Trainer world progression specification](../specs/trainer-world-progression.md)
- [Playthrough-seeded variation](playthrough-seeded-variation.md)
- [Shared playthrough seed framework](../specs/playthrough-seed-framework.md)
- [Trainer pool and generation](../specs/circuit-trainer-pool.md)
- [Seeded circuit runtime](../specs/seeded-league-circuit.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Player progression](../specs/trainer-rating-party-progression.md)

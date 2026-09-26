# Seeded Trainer Circuit

Implemented: No
Design status: Recurring regional championships, trainer-owned world progression,
and individual competition locks are approved directions. A loss ends the event
and requires waiting for the next competition. Numeric balance, production
content, signup thresholds, waiting rules, and supporting defaults remain under review.

## Intent

Make Indigo, Sevii Masters, and Hoenn recurring championships with recognizable
competitors and changing fields. Each edition visits all three venues in a
seeded order. Every venue hosts two contenders, two elite competitors, and one
headliner. Strength follows the trainer's personal development at that stop;
no region is permanently the weak or final championship.

## World progression revision and balance explorer

[Trainer world progression](trainer-world-progression.md) owns starting
`baselineTR`, personal badge growth, and first-lifetime-clear growth. A trainer's
effective TR determines competitive eligibility and battle levels. Each canonical
trainer has one save-seeded growth modifier, initially 90–110%, shared across
all aliases and encounters. It scales authored badge and first-clear growth
above the unchanged baseline. Derive it independently of competition draws;
never reroll it after a badge, loss, or new event. Keep the TR 80 ceiling. World Gym
encounters use live milestones; circuit encounters use the current competition's
saved entry snapshot.
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
it currently uses neutral 100% growth, does not implement seeded growth
modifiers, and does not generate leagues or certify moves, items, AI, or battle
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
stronger field because badges and actual first clears advance the world, not because
Indigo, Masters, or Hoenn has a permanent difficulty tier.

Resolve every candidate's personal TR and applicable stage/profile at the
competition's actual entry world point before eligibility. A Gym Leader can qualify as
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

Build one competition's field at a time. Within the selected category, start
with flat base weight and propose these D5 factors:

| Condition | Factor |
| --- | ---: |
| Appeared at zero / one / two other completed venues in this circuit edition | 16 / 4 / 1 |
| Appeared in this venue's most recently completed competition | 1 |
| Did not appear there, or no previous competition exists | 2 |

The first factor counts distinct earlier venues where the trainer appeared in
any completed competition, including losses; repeated events at one venue count
only once. Exclude the current venue. The second factor uses the last completed
competition at the current venue, whether won or lost, across circuit editions.
Use bounded participation sets rather than an unlimited event log.

Multiply applicable factors. All eligible candidates keep positive weight;
no participation preference overrides TR eligibility or the category draw.
A competition without that trainer clears the prior-venue penalty. Gym fights,
reloads, failed registration requests, and room visits do not add participation.

Roughly two returning and three different opponents between successive
competitions at a venue is a tuning target, not a quota. A trainer may return in any later edition, and consecutive
fields may coincide. Never reroll or advance an event identity to force novelty.
Rotation must remain useful when personal ratings mature and stop changing.

## Order, registration, and strength snapshots

At new game, persist the shared playthrough root and the first circuit edition's
seeded venue order. An edition is a three-venue traversal; each venue can host
multiple competitions before the player wins it. There is no circuit-wide roster
registration. Early itinerary views reveal order and qualification, not future
participants.

When the player enters the currently available competition at the current venue,
capture actual badges, lifetime venue-clear identities, content/growth/band
versions, and required participation history. Resolve that event's five slots
atomically and save their TR, stages, and profiles. Future leagues stay unlocked:
their fields and strength are determined only when entered, from the progress
actually reached then. Do not predict badges or first clears.

The active event's participants and strength remain fixed through battle
reconstruction, departures, saves, and reloads. A loss ends the competition,
releases its active lock, and records participation. The player must wait for the
next competition before entering again. Losing does not clear the venue, advance
the circuit position, grant first-clear growth, or immediately offer a retry
against the same event. A future event may naturally draw some or all of the
same trainers, at their then-current strength.

A win commits that venue's result and any lifetime first-clear reward. The player
can proceed to the next seeded venue once its qualification and availability
rules are met. After all three wins and their ceremonies finish, a new circuit
edition can use a newly seeded venue order. Actual entry remains subject to the
competition's availability rule. Neither an event ordinal nor a circuit edition
number grants NPC strength on its own.

Persist distinct circuit and competition identities. An entry request, preview,
reload, or cancellation cannot create a new event or bypass the wait. New events
must follow the adopted availability rule; no calendar, real-time delay, steps,
or milestone-based wait has been chosen yet. Failed entry transactions leave
the available event and history unchanged. Corrupt snapshots are errors, not
opportunities to redraw. Existing Pokémon gameplay RNG keeps its current behavior.

## Qualification and player progression

Exact signup timing is unresolved. The user's tentative pacing is the first
league around 8 global badges, the second around 16, and the third around 24.
These are reviewable targets rather than hard requirements. Later positions
still require winning the preceding venue in the current circuit. Travel and
badge collection remain independent of the seeded order.

The former universal 24-badge entry proposal and whole-edition lock are removed.
Each competition reads live progress at entry, so players can earn badges between
leagues and while waiting after a loss. The next competition reflects that growth
without changing any active event's saved field.

Retain the existing player +8 TR reward for each first lifetime venue clear,
at most three contributions. NPC growth follows each trainer's separate curve
and first-clear amount. Sharing milestones does not mean sharing a rating,
cap curve, or growth rate. Balance the tentative 8/16/24 pacing against the
existing player progression function before finalizing thresholds.

Later circuit clears provide ordinary battle rewards without new player TR
contributions. Losses never grant victory recognition. Badges, lifetime clear
facts, player TR high-water, recognition, regional cleanup, and Blue/Red access
survive event termination and circuit rollover. Exhibition replays of retired
lineups are outside this revised lifecycle; they must not restore immediate
competitive retries after a loss.

## Records, presentation, and regional integration

Save the seeded order, current circuit results, active competition snapshot,
competition availability/identity, and bounded participation history. Keep
historical result facts separate from an active lineup lock; retaining a result
must not retain the retired event as a playable retry. Propose a bounded
completed-circuit counter. Regional projections cannot substitute for these facts.

Show seeded order and registration status from new game. For a registered active
competition, show its five names, assigned TR, specialties, and battle order.
Future venues display qualification and availability, with no generated roster
or guaranteed participants. After a loss, explain that the competition has ended
and entry must wait for the next event. Once the waiting rule is chosen, show its
actual condition; do not promise a date or countdown before that design exists.
Moves and held items are not revealed by default.

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
home-only 2/2/1 feasibility at every supported entry world point and seeded
order, including C=0–3, TR saturation, and heterogeneous 90–110% personal rates.
Check role/stage crossings throughout that range, not only uniform slow/fast
endpoints; random seed samples alone do not prove universal coverage. Bands must remain ordered and disjoint;
missing candidates block content enablement. Do not widen bands at runtime,
invent memberships, force visitors, or substitute a fixed roster.

Feasibility does not prove variety. Evaluate at least 10,000 documented roots
over ten circuit editions and documented loss/wait/progression sequences. Measure
returning/changed fields, cross-venue repetition,
eligible opportunities per character, absence runs, home/visitor share, and
strength distribution. Separate raw category odds from fallback outcomes.
There is no complete-attendance quota or guaranteed Elite Four/Champion turnout.

Playtest each possible first venue, later stops after actual badge and first-clear
growth, and fully progressed
editions against the player-cap curve. Check stage transitions and effective
strength, not only numeric TR. The explorer does not establish combat balance.

## Persistence and adoption

The shared seed framework owns the root, derivation, unbiased draws, and golden
vectors. The circuit owns edition/order, competition identities and availability, entry
snapshots, results, and bounded history. Resolve an outcome only from its semantic
keys and declared content/history/milestone inputs; other seeded features and
ordinary gameplay RNG cannot perturb it. Store no circuit random cursor.

The runtime and pool specs pin event schema and decision versions. Competition
identity changes require new POOL_KIND and ROSTER versions; ORDER remains separate. Missing/corrupt registered snapshots are invalid saves, not
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

D1 now specifies personal NPC growth with a stable per-save 90–110% modifier, world-Gym snapshots, and a lock scoped to
one league competition. Losing ends that event and requires waiting for the next;
this supersedes the earlier whole-edition snapshot decision. D2 is resolved: headliners qualify
by effective TR rather than historical Champion title. Singles, home leagues,
85%/15% regional selection, recurring editions, and soft rotation are confirmed.

| ID | Remaining decision | Proposed default |
| --- | --- | --- |
| D3 | Personal curves, stages, NPC anchors, world-point role bands, production content and balance? | Supported Kanto/Johto/Hoenn singles, six-member competitive circuit profiles; use explorer data as provisional evidence and prove local feasibility through growth/saturation. |
| D4 | Signup thresholds and progression balance? | Tentatively around 8/16/24 badges by circuit position, with predecessor wins; retain lifetime player +8 per venue. Exact thresholds remain undecided. |
| D5 | Rotation factors and measured variety acceptance? | Flat in-role weights, 16/4/1 current-edition factors and 1/2 prior-venue factors; roughly two returning/three changed between venue competitions without quotas or rerolls. |
| D6 | When does the next competition become available after a loss or win? How is that availability persisted? | Undecided. Require waiting for a later competition, stable event identity, and no reload/registration bypass; choose the time or progress mechanism before implementation. |

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

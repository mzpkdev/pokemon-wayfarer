# Seeded Trainer Circuit

Implemented: No
Design status: Living rivals, recurring championships in a seeded order,
standing-based roles, and unlimited retry of a frozen field are approved.
Numeric balance, trainer content, signup thresholds, rotation tuning, and role
windows remain under review (D3, D4, D5, D7).

## Intent

Familiar trainers should feel alive. Across a save, the people the player met
as Gym Leaders keep training, and some of them get noticeably stronger than
others. When the player walks into a championship, the field is a mix of old
acquaintances and rising names, and the question is "who got scary this time?"
rather than "which fixed roster is next?" In one save Clair rises early and
is soon strong enough to headline; in another she keeps a steady pace as an
elite, and Whitney's fast start is the story instead. Rivals keep developing
through the first few editions after the player's journey ends, so the
post-game fields still change shape.

Indigo, Sevii Masters, and Hoenn are recurring championships. Each circuit
edition visits all three in a seeded order, so no region is permanently the
easy one or the final boss. Every championship is a five-battle singles field
that ramps from two contenders, through two elites, to one headliner.

## Sample playthrough

1. The player collects their eighth badge and signs up for the first stop on
   their seeded order: Hoenn.
2. On entry, Hoenn's five-trainer field is decided and frozen. The two
   contenders sit a little under the player's level cap, the two elites near
   it, and the headliner (Steven this time) at or slightly above it. Lobby
   gossip hints that one of the elites "has been training nonstop."
3. The player beats four and loses to Steven. Nothing is lost: the same five
   trainers at the same levels wait for them.
4. They leave, win a ninth badge, level up, and return. The field has not
   moved, so the retry is easier. They win. It is their first Hoenn clear, so
   their own progression gets its usual boost.
5. Around sixteen badges they sign up for Sevii Masters, an open invitational
   that draws from Kanto, Johto, and Hoenn alike. The world has grown, so the
   field is stronger, and it has only a couple of faces from Hoenn. Janine, a
   slow starter in this save, fills a contender slot.
6. Around twenty-four badges they win Indigo and complete the edition with its
   ceremonies. A new edition begins with a new seeded order.
7. In the second edition, Janine turns up as an elite: her slow start has
   become a strong finish. In a save where she started fast instead, she would
   have peaked early and settled back.

## How a championship works

**Fields.** Five slots per venue: contenders in battles 1–2, elites in 3–4, the
headliner in 5. A trainer's role comes from how strong they are at that moment
relative to the world, not from their title. In some saves one of a few strong
Gym Leaders rises far enough to headline; an Elite Four member who has stalled
can be a contender. Everyone fights at their real current strength, and each
field has five different people. The battles always climb from weakest to
strongest; when a venue runs short of trainers at the right strength,
neighbouring battles may sit closer together in level. League battles always
use the trainer's full six-member competitive team.

**Regional flavour.** Indigo and Hoenn each have an authored home roster.
Each slot usually goes to a home trainer, with a smaller chance of a visitor.
There is no visitor quota and no guest slot. Sevii Masters is an open
invitational: every eligible trainer from any region is equally welcome.

**Losing.** A loss never ends the championship. The field and its levels are
frozen from the player's first entry until they win. The player can retry at
once or leave, earn badges, train, and come back as often as they like. Saving
and reloading keeps the same field.

**Winning.** A win records the result and opens the next venue in the
order. The first-ever win at each venue gives the player the existing
progression reward, which also raises the world's level for later fields.
After all three venues and their ceremonies, the next edition starts with a
new seeded order and fresh fields.

**Post-game.** Rivals keep developing across the first few editions after
the journey: a fast starter may have peaked and a slow starter may finally hit
their stride. After that their strength settles, and rotation keeps the
fields fresh. Once the player's cap reaches level 100, fields stay ramped just
below it: contenders a few levels under 100, and the headliner at or near it.

**Variety between editions.** Rotation gently favours people the player has
not just faced. The target is roughly two returning and three new trainers at
a venue between editions, with no quotas and no rerolls. Headliner variety
comes from trainers' own development, not from forced rotation.

**Signup.** The first stop opens around eight global badges, and later stops
around sixteen and twenty-four, each also requiring a win at the previous
venue in this edition (D4). Travel and badge collection never depend on the
circuit order.

## Living rivals

[Trainer world progression](trainer-world-progression.md) owns how trainers
grow: each trainer's strength follows the world's progress rather than the
player's party, with a per-save growth arc hinted in the world rather than
shown as a label. Every trainer's first encounter is the same in every save.
The same model drives Gym Leaders (near the player's cap on average, with
teams that grow with badges) and Blue (a step above the cap, never a wall).
This PRD covers only how the circuit uses those trainers.

## Records and recognition

- Indigo's first-ever win grants shared Kanto/Johto Champion recognition;
  Hoenn owns its own recognition and regional cleanup; Masters records its
  result in the Masters Gallery and never grants a regional Champion title.
- Indigo and Hoenn wins each run one winning-team Hall of Fame and Champion
  Ribbon flow per edition. Regional cleanup happens once per lifetime.
- The first edition's final win plays full credits; later editions use a
  brief completion presentation.
- Red unlocks after all three venues have been won at least once and stays
  available. Blue's Saffron Dojo battle unlocks after the first venue win of
  any kind.
- A headliner who is a Gym Leader is presented as the final opponent without
  inventing Champion history. Dialogue never assumes who occupies a venue or
  which venue ends the circuit.

## Scope

In: Indigo, Sevii Masters, and Hoenn; supported Kanto, Johto, and Hoenn
trainers; singles only. Out: Tate and Liza (double battle); Red (separate
mastery encounter); exhibition replays of past fields; new prizes or
currencies. Player progression, wild encounters, shops, ordinary trainers,
standalone builds, and unenrolled story or rematch battles keep their current
contracts.

The [balance explorer](../../devtools/ui/README.md#trainer-balance-explorer)
predicts species, party sizes, and levels for review. Playtesting still owns
combat balance: moves, items, and AI.

## Specifications

- [Circuit trainer pool](../specs/circuit-trainer-pool.md): registry, roles,
  selection, rotation, and battle construction.
- [Seeded circuit runtime](../specs/seeded-league-circuit.md): signup,
  entry, retry, results, ceremonies, persistence, and presentation.
- [Trainer world progression specification](../specs/trainer-world-progression.md):
  growth model, arcs, stages, and levels.

## Decisions

Resolved: D1, growth follows per-save arcs relative to the world
([trainer world progression](trainer-world-progression.md)). D2, headliners
are chosen by current standing, not title. D6 is deleted: a loss means
retrying the same frozen field, with no waiting mechanism.

| ID | Open decision | Proposed default |
| --- | --- | --- |
| D3 | Per-trainer bias, allowed arcs, arc shapes, team stages, competitive teams, and hint lines | Balance explorer catalog defaults. Content blockers: full six-member competitive teams for every Elite Four member, Lance, and Giovanni are missing, so elite slots may fall back until they exist; Hoenn has few contenders once the player holds 24 badges |
| D4 | Signup thresholds | About 8/16/24 global badges by circuit position, plus a win at the previous venue |
| D5 | Rotation factors and variety acceptance | Weight ×16/×4/×1 for 0/1/2 earlier venues this edition; ×2 if absent from this venue's last field; wins only |
| D7 | Role windows (levels relative to the world's cap) | Contender −2 or lower, elite −1 to +1, headliner +2 or higher |

## References

- [Trainer world progression](trainer-world-progression.md)
- [Playthrough-seeded variation](playthrough-seeded-variation.md)
- [Shared playthrough seed framework](../specs/playthrough-seed-framework.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Player progression](../specs/trainer-rating-party-progression.md)

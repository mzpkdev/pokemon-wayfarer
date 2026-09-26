# Seeded Trainer Circuit

Implemented: No
Design status: Draft; confirmed requirements and proposed defaults are separated below.

## Intent

Make Wayfarer's circuit a competition among recognizable trainers. A new game
draws its itinerary and opponents from shared data, giving each save a different
career while preserving a challenge the player can learn and prepare for.
An opponent's own Trainer Rating determines both where they can compete and
their battle strength.

## Design

### Confirmed requirements

- Replace the preset League lineups with a pool of notable Gym Leaders,
  Elite Four members, and Champions.
- Each trainer has an authored baseline Trainer Rating (NPC TR). It affects
  both selection into a competition and strength in battle.
- Each league lineup contains five distinct characters, including across
  alternate teams, Trainer IDs, or titles. The same character may appear in
  multiple leagues when independently TR-qualified and lore-admitted at each.
- Check TR suitability first. A trainer who fits a league's lore still cannot
  bypass its competitive rating requirements.
- After TR eligibility, each trainer without a sensible affiliation to that
  league has exactly a 50% seeded chance of being dropped from its candidate
  list. Affiliated trainers pass this filter. Trainers may have multiple
  sensible league affiliations, supported by explicit lore rationale.
- Randomize the entire circuit order at the start of a new game and retain
  that seeded order for the save.
- Use one long-term playthrough root seed for opted-in authored Wayfarer
  variation. The circuit is its first consumer; future features opt in
  individually and cannot perturb circuit decisions.
- Repeating an unresolved decision with the same seed, key, and inputs gives
  the same outcome. Saving and reloading cannot provide a new circuit draw.
  Existing Pokémon gameplay RNG and randomizer behavior remain unchanged.

### Proposed defaults

The remaining rules form a concrete draft for review. They are recommendations,
not individually approved decisions. D1–D3 under **Open questions** identify the
largest choices; the supporting defaults below can also be revised.

Use the three existing destinations: Indigo League, Sevii Masters, and Hoenn
League. Each appears once. Circuit position controls admission and expected
strength; the destination controls its location and local ceremony.

| Position | Qualification | First-clear reward |
| --- | --- | --- |
| 1 | At least 8 global badges | +8 player TR |
| 2 | At least 16 global badges and position 1 cleared | +8 player TR |
| 3 | All 24 badges and positions 1–2 cleared | +8 player TR |

For example, a seed may produce Hoenn, Indigo, then Masters. Hoenn hosts the
opening field, Indigo the middle field, and Masters the strongest field. That
sequence is mandatory for first clears. Exploration and earning badges remain
independent of the itinerary; all 24 badges can be collected before position 1.

Generate all three five-opponent lineups alongside the itinerary. Store the
resolved choices for the save. Draw each league independently without replacement
within its five-person lineup; there is no global exclusion or repeat penalty.
Gym battles, story encounters, and personal rematches remain separate
appearances and do not consume a circuit place.

Use fixed NPC strength for the initial version (D1). Each position selects from
an appropriate NPC TR range. A trainer's rating determines their team levels,
independently of the player's rating or party. Their authored competitive team,
specialty, ace, moves, items, and AI give that strength a recognizable identity.
Their NPC TR does not rise after player victories or fall after defeats.

Within each competition, order the selected trainers by NPC TR. The strongest
selected trainer is the final opponent, regardless of whether their background
is Gym Leader, Elite Four member, or Champion (D2). Historical titles do not
reserve a slot. Red remains a separate mastery encounter after all three clears
and is excluded from the pool.

First form the TR-suitable candidate list for each scheduled venue. Then apply
one seeded lore filter per canonical character and venue: an affiliated trainer
passes; an unaffiliated trainer stays only when `Uniform(2) = 1` and is dropped
when it is 0. The result is fixed at new game and cannot be retried after a loss
or reload. Select from the retained feasible candidates using only the proposed
rating-proximity weight; there is no affiliation bonus or visitor quota.

Author `leagueAffiliations` directly as a set of Indigo, Sevii Masters, and
Hoenn, with a rationale per trainer. More than one affiliation is allowed;
current map region or a blanket Kanto/Johto/Hoenn mapping does not assign it.
Membership affects the lore filter, never strength or TR suitability. Do not
invent affiliations to make an undersupplied catalog pass validation.

The 50% rule is per-candidate retention, not final appearance probability.
Multiple unaffiliated trainers may be selected in one field. A large global
pool can still crowd out affiliated candidates after filtering; measure the
actual field composition during balance review without altering the gate.

The initial pool uses supported Kanto, Johto, and Hoenn characters with reviewed
competitive singles teams (D3). Author six-member circuit profiles so battle
size is consistent across roles. Paired battles such as Tate and Liza require a
separate format decision and are excluded from this first version. Story
availability does not decide participation: the circuit is a sanctioned
appearance, separate from a trainer's local Gym or quest role.

### Attempts and replays

Losing or leaving resets that competition's room progress. Returning faces the
same people, in the same order, with the same team profiles and NPC TR. Saving,
reloading, earning badges, and changing parties cannot reroll the field or raise
its strength. Ordinary battle RNG still operates normally.

Cleared competitions remain replayable with the same lineups and strength.
Replays grant ordinary battle rewards, but no additional progression TR,
circuit clear, Hall of Fame entry, Champion Ribbon, or completion credits.
Rating changes, seasons, roster reshuffles, and stronger rematch profiles are
separate future features.

## Boundaries

This changes Wayfarer's circuit selection, order, opponent strength, and their
dependent presentation and unlocks. It preserves the three venues, global badge
accounting, independent travel, and the player's existing TR progression model.
NPC TR is a new authored strength measure; it is not an Elo system and does not
replace the player's badge-and-clear-derived rating.

Ordinary trainers, initial Gym battles, wild encounters, player caps, obedience,
local adventures, and their scaling policies retain their own contracts.
Reusing a character in the pool does not enroll their other encounters in the
circuit policy. Standalone Emerald, FRLG, and HNS behavior remains unchanged.

The [playthrough seed framework](../specs/playthrough-seed-framework.md) owns
the root seed, derivation protocol, and unbiased bounded draws. This feature
owns its resolved schedule and named circuit decisions. It does not move
existing battle randomness, encounters, or randomizers onto the new framework.

## Balance

Position strength is calibrated against the earliest intended admission points:
player TR 40, 56, and 72 under the retained +8-per-clear progression. The pool
specification defines inclusive NPC rating ranges around these balance anchors;
D3 must settle their endpoints with the catalog review. Ranges may overlap,
allowing one fixed-rating trainer to qualify at more than one position. Membership
cannot bypass the ranges, and repetition is never forced by upscaling a weak
trainer. These are design values, not playtested results.

At equal preparation, a higher-rated selected trainer should usually be the
harder opponent. Team matchup can still produce upsets; TR is not a prediction
that the higher number always wins. Review team quality alongside levels so a
weak profile does not receive an impressive number without matching strength.

The player can outgrow an early competition by earning additional badges and
training first. With all 24 badges before any clear, player TR is 56 rather than
40 at opening admission. The first field remains unchanged. This preparation
advantage is intentional under D1, and must be playtested rather than erased
through hidden player scaling.

## Content

Each pool entry needs one canonical character identity, visible name, explicit
league affiliations and rationale, NPC TR, a competitive party profile, battle and overworld graphics,
and circuit-appropriate dialogue. Existing encounter IDs may supply assets and
authored content; they do not define uniqueness.

The exact initial membership, trainer-by-trainer ratings, and six-member teams
remain an explicit content deliverable under D3. The draft does not claim that
every existing source party is already suitable. Before enabling the feature,
review an inventory proving supported presentation, compatible battle formats,
and at least five distinct TR-qualified affiliated characters for every
venue/position pairing across all six venue orders. This covers the worst case
where every unaffiliated candidate is dropped. Each venue needs its own valid
five-person list. The current catalog is not assumed to satisfy it. D3 must review
affiliations, ratings, and teams together; generation cannot restore dropped
trainers, widen TR eligibility, or invent affiliations to fill a shortfall.

## Presentation

Show the full seeded itinerary on the Trainer Card from the beginning, with
qualification and Locked, Available, or Cleared state. A roster view reachable
from the card or each venue's reception lists all five names, their NPC TR,
specialties, and battle order before entry, including locked future stops.
Do not disclose full movesets or held items by default.

Label the player's progression value and opponent rating clearly in context.
The displayed opponent TR must be the same value used to construct that fight.
Previewing the schedule never generates, mutates, or rerolls it.

Room actors, portraits, names, dialogue, battle introductions, and ceremony
references follow the selected character. Retained themed rooms must not claim
that a displaced fixed trainer occupies them. A Gym Leader finalist is presented
as the competition's final opponent; the draw does not rewrite their history or
claim they were already the reigning Champion.

## Interactions

Indigo still grants the shared Kanto/Johto Champion recognition on its first
clear. Hoenn grants its own regional recognition and performs its local cleanup
on its first clear. Masters grants its dedicated result without a regional
Champion title, Hall of Fame entry, or Champion Ribbon, even when it is last.

Run full completion credits after the third first clear, regardless of venue.
Unlock Red after all three. Completing Hoenn early must not prematurely finish
the circuit; finishing at Masters must not omit circuit completion.

Propose unlocking Blue's Saffron Dojo appearance after the first committed
circuit clear. It does not require drawing or defeating Blue, who may be absent
from the circuit. Remove dialogue that assumes Blue is always Indigo's final
opponent. His independently authored rival and story encounters retain their
own roles; story uses of his current Champion title need an explicit text audit.

Keep player TR at `badgeContribution + 8 * distinctCircuitClears`, with the
existing high-water rule and ceiling of 80. Each venue contributes once.
Individual opponents, repeat ceremonies, losses, and replays contribute nothing.

## Constraints

Persist the generated schedule alongside a valid playthrough root before play
can consume either. The foundation proposes a 64-bit root, stored once in shared
Wayfarer persistence as two 32-bit words with seed-format and derivation-version
metadata. The circuit stores no independent seed or persistent random cursor;
it stores resolved character/profile choices and schema, ORDER/LORE_FILTER/ROSTER decision,
and catalog versions for reliable resume and reproduction.

Circuit order, per-character lore filtering, and roster allocation use separate keyed
decisions. With the same root and respective rules versions, roster catalog
changes cannot reroll venue order or existing character/venue filter draws.
Unrelated seeded feature calls or content, menu opens, fights,
queries, gameplay RNG, elapsed time, starter choice, and save/load cannot alter
these circuit decisions. Legitimate roster membership/weight changes may alter
new-game roster draws; they cannot rewrite an existing saved schedule.

Missing or damaged roots and schedules follow invalid-save/new-game handling,
never regeneration on load. The framework's hash/mixer and golden vectors must
be pinned before enabling the feature; choosing the implementation is an
engineering requirement rather than another user-facing design choice.

Use existing venue layouts and respect the normal ROM reserve and RAM budgets.
No direct `map.bin` editing is part of this feature. Existing prerelease saves
need no migration; use the repository's save-version policy when implementing.

This is a proposed successor to the fixed
[interregional circuit](wayfarer-interregional-league-circuit.md) and
[League scaling](league-scaling.md) designs. Until this draft is adopted, those
remain the approved baseline. Once adopted, the two linked specifications below
take precedence for changed behavior; their integration audit identifies the
remaining documents and story consumers that must be reconciled.

## Playtesting

- Do all six destination orders work at minimum qualification and after earning
  all badges first, without requiring the clear currently being attempted?
- Does NPC TR communicate meaningful strength across different team styles?
- Are opening fields manageable and late fields demanding across sampled seeds?
- Does the TR-first lore filter produce convincing fields across catalog sizes?
  Report affiliated/unaffiliated shares without assuming a local majority or
  treating 50% retention as final appearance probability.
- Does a non-Champion finalist feel credible, and does the ending still work
  when Masters is the final destination?
- Can players understand qualification and prepare for the published lineup,
  including after a loss or a long break from the save?
- Do equivalent keyed decisions and committed schedules stay stable across
  reloads and unrelated opted-in variation, while ordinary Pokémon RNG still
  behaves as before?

## Open questions

| ID | Decision still requiring review | Proposed default used by the specs |
| --- | --- | --- |
| D1 | Fixed NPC difficulty or bounded adaptation to player TR? | Fixed NPC TR and strength; preparation can outgrow early fields. |
| D2 | Can any qualified character be a finalist, or must the slot be Champion-qualified? | Highest selected NPC TR, with no title restriction. |
| D3 | Initial character scope, league affiliations and rationale, concrete roster, ratings, eligibility ranges, and teams? | Supported Kanto/Johto/Hoenn singles characters first; approve inclusive TR ranges and five qualified affiliated candidates per venue/position. |

Supporting defaults also proposed here are whole-save fixed lineups, the
8/16/24 mandatory progression, +8 first-clear rewards, six-member singles teams,
affiliated-only feasibility validation, unchanged-strength replays, a separate
Red encounter, first-stop Blue Dojo access, and advance roster disclosure.
Adopting the draft should resolve these as one coherent contract; changing D1,
D2, or D3 requires updating both specs before implementation.

## References

- [Playthrough-seeded variation](playthrough-seeded-variation.md)
- [Shared playthrough seed framework](../specs/playthrough-seed-framework.md)
- [Trainer pool, generation, and strength](../specs/circuit-trainer-pool.md)
- [Seeded circuit runtime and progression](../specs/seeded-league-circuit.md)
- [Player TR progression](../specs/trainer-rating-party-progression.md)
- [Viridian finale and Blue Dojo](../specs/frlg-kanto-viridian-finale.md)

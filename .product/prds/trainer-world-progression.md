# Trainer world progression

Implemented: No; the balance explorer is implemented, while the ROM still uses
the existing Gym and League scaling policies.
Design status: Trainer-owned ratings, approachable Gym openings, and personal
growth are the accepted direction. Numeric balance and production team content
remain provisional. Projected whole-edition registration snapshots are confirmed.

## Intent

Let familiar trainers develop alongside the player's journey while retaining
their own strength and identity. Players can challenge later Gym Leaders early
without facing their original late-game teams, and postponed leaders remain
competitive as the world progresses.

## Design

Each supported canonical trainer has an authored starting rating, called
`baselineTR`, and a personal growth curve. Their effective TR depends on global
badges and first-ever league venue clears. The trainer's baseline stays authored;
their effective rating changes. Training Pokémon, changing the player's party,
waiting, reloading, or registering another edition does not itself raise NPC TR.

The same milestone can raise the player's progression TR and an NPC's personal
TR through separate rules. Never use the player's TR as the NPC rating input.
Player caps, experience, obedience, wild encounters, shops, ordinary trainers,
and Gym members retain their own existing player-TR policies.

Use personal badge checkpoints and a per-trainer first-clear growth amount as
the initial model. Authored stages determine party membership, species/forms,
and battle content; the NPC level curve converts effective TR into levels.
Stages can deliberately replace an opening Pokémon with an evolved form, but
the engine does not infer evolution or rebuild a team from its historical title.

FRLG, Emerald, and the repository's HNS parties provide recognizable content and
strength references. They are not mandatory opening parties. Brock can begin
near Geodude 12 / Onix 14; later Gym Leaders also need approachable opening
stages. Blue remains approachable early but reaches Champion strength sooner
through his personal curve, independently of his Gym eligibility.

## Encounter rules

For enrolled initial singles Gym battles, resolve the leader's effective TR and
stage from current world milestones before battle construction. Keep that plan
through the fight. A retry at unchanged milestones has the same authored team
and levels; earning a badge or first clear before retrying advances the world.

Pool identity does not automatically enroll story, rival, Dojo, rematch, or
facility battles. Red remains separate. Tate and Liza stay outside the singles
pool; their existing double Gym encounter and badge remain available under its
current policy. The Gym coverage inventory owns these explicit boundaries.

For leagues, seed the first travel order at new game, then generate the whole
edition at eligible registration. For each scheduled stop, project the number
of lifetime venue clears after completing its mandatory predecessors. Apply
that world point to every candidate before TR eligibility and regional draws.
Save all selected trainers, ratings, stages, and profiles together. This lets
later first-edition stops reflect first-clear growth without moving the target
during an attempt. Retries and replays use the registered version.

Use role bands authored for the projected world point, shared across venues at
that point. Content validation must prove the local pool can fill all roles
through progression and TR saturation. Never widen a band during generation to
repair missing candidates. Recurring editions change participation; edition
count alone does not raise strength after lifetime growth is exhausted.

## Balance explorer

The [explorer](../../devtools/ui/README.md#trainer-balance-explorer) exercises 37
singles trainers at 0–24 badges and 0–3 first clears. Its editable curves, stages,
NPC level anchors, source references, and saved experiments support content
review. Blue's current experimental checkpoints are 6/54/57/59 at 0/8/16/24
badges; the default first-clear increment is six. These are tuning inputs, not
approved production balance.

The explorer predicts species, party size, and levels. It does not validate
combat difficulty, moves, items, AI, league eligibility, or a complete production
catalog. Incomplete five-member championship profiles and unevolved prototype
species must be reviewed before enabling ROM content.

## Acceptance

- Every supported trainer has a traceable baseline, growth curve, and authored
  stages; canonical aliases cannot create different personal ratings.
- Exhaust badge counts and first-clear counts, including stage boundaries and
  the TR ceiling. Party size and the weakest member's level must not drop at
  default stage transitions; report exceptions as content failures.
- Check early, delayed, and post-league Gym encounters, including members that
  may be stronger than their leader. Equal levels do not prove equal difficulty.
- Confirm seed-independent growth and repeatable battle plans at unchanged
  world points. Existing Pokémon battle RNG retains its current behavior.
- Prove feasible championship role pools and measure field variety separately.
  Validate first-entry, subsequent-stop, and fully progressed editions against
  the unchanged player-cap curve.

## References

- [Trainer world progression specification](../specs/trainer-world-progression.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Seeded Trainer Circuit](seeded-trainer-circuit.md)
- [Player progression](../specs/trainer-rating-party-progression.md)

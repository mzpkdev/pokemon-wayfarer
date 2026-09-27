# Player Trainer Rating

Implemented: Partial. Today's badge and first-clear formula, its saved
high-water value, and every consumer listed below exist. The new badge scale,
uncapped TR, and the world curves that follow it are the v0 target.
Design status: v0 accepted. How TR is earned and how the world answers it are
approved; the exact numbers are provisional and tuned by playtesting.

## Intent

Give the player one measure of how far their career has come, and let the
world answer to it. Every badge raises it; the level cap, wild Pokémon,
ordinary trainers, and shops follow it, whatever order the player explores in.
The early world should feel dangerous, the late world should feel like home
ground, and the real tests should come from the trainers everyone knows.

## Design

### A hidden measure

You have a Trainer Rating (TR). The game never shows it as a number: you feel
it through the world instead, as a higher level cap, stronger wild Pokémon and
trainers, and better shops.

### How you earn it

Every badge counts, whichever region it comes from. Your first eight badges
each give a big step up; every badge after that still gives a smaller one, all
the way to the last. TR never goes down: losses and replays add nothing, and a
later check never lowers what you have earned. A new game starts at 0.

League wins give you nothing. A league is a test of what you have built, not a
source of power: winning one proves you are ready, and your next step still
comes from badges.

Today's ROM still uses the earlier formula, where each first League venue win
also raised TR and the total stopped at a fixed ceiling. It stays in place
until the new scale is adopted.

### No ceiling

In the target, TR has no upper limit. All 24 badges take you to the top of
today's content, but stronger future content can go higher.

Everything TR drives has its own natural limit instead. Your level cap, for
example, rises with every badge until it reaches level 100 with all 24 badges
and then stays there. Future content can add new things that keep growing past
today's range, so extra TR still means something.

### How the world answers

- **Early on, the world is dangerous.** With a handful of badges, wild Pokémon
  and route trainers sit just under your level cap. Every fight on the road
  matters.
- **Late on, routes are no threat.** With many badges, your cap climbs far
  ahead of the wild and of ordinary trainers. You travel freely.
- **The challenge comes from people you know.** Gym Leaders, league fields,
  and famous trainers met on the road bring the late-game fights, at their own
  strength ([Well-known trainers](well-known-trainers.md)).

### What your TR drives

- **Your level cap, experience, and obedience.** A higher TR raises your soft
  level cap; Pokémon past it earn less experience and may disobey
  ([wild encounter and party progression](trainer-rating-wild-encounter-scaling.md)).
- **Wild and static encounters**
  ([wild encounter scaling](trainer-rating-wild-encounter-scaling.md)).
- **Ordinary trainers and Gym members**
  ([ordinary Trainer and Gym-member scaling](trainer-party-scaling.md)).
- **Poké Mart stock**, unlocking at the same badge counts as today
  ([Poké Marts](global-tr-pokemarts.md)).

Your TR never makes a well-known trainer stronger or weaker: they have their
own rating ([Well-known trainers](well-known-trainers.md)), and League fields
are drawn from those ratings ([Leagues](leagues.md)).

## Boundaries

- Each consumer keeps its own rules and owns its own curve; this document only
  gathers what they share.
- Obedience, reduced experience past the cap, and Candy rules keep working the
  same way; they follow the cap wherever it sits.
- Well-known trainers' ratings never read yours, and yours never reads theirs.
- Standalone builds keep their existing progression.

## Later

- More ways to earn TR: renown, and exploration or catching paths.
- Scalers that saturate beyond today's range, arriving with the content that
  pushes TR higher.
- Further tuning of the treadmill (the world scaling alongside the player's
  growth) and the widening world gap, now that late routes fall behind.

## References

- [Player Trainer Rating specification](../specs/player-trainer-rating.md)
- [Well-known trainers](well-known-trainers.md)
- [Leagues](leagues.md)
- [Wild encounter and party progression](trainer-rating-wild-encounter-scaling.md)
- [Ordinary Trainer and Gym-member scaling](trainer-party-scaling.md)
- [Poké Marts](global-tr-pokemarts.md)

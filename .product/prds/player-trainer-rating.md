# Player Trainer Rating

Implemented: Partial. The badge and first-clear formula, its saved
high-water value, and every consumer listed below exist today. Uncapped TR and
the scaler framing are the v0 target.
Design status: v0 accepted. The formula stays as it is; changing how TR is
earned is Later.

## Intent

Give the player one measure of how far their career has come, and let the
world answer to it. Badges and League wins raise it; the level cap, wild
Pokémon, ordinary trainers, and shops follow it, whatever order the player
explores in.

## Design

### A hidden measure

You have a Trainer Rating (TR). The game never shows it as a number: you feel
it through the world instead, as a higher level cap, stronger wild Pokémon and
trainers, and better shops.

### How you earn it

TR rises with badges and with each League venue you win for the first time,
+8 per venue. Any badge counts, whichever region it comes from. TR never goes
down: losses, replays, and individual League victories add nothing, and a
later check never lowers what you have earned. A new game starts at 0.

This is today's formula, and v0 keeps it. How TR is earned may change later.

### No ceiling

In the target, TR has no upper limit. Today's content happens to take you to
about 80, but stronger future content can go higher.

Everything TR drives has its own natural limit instead. Your level cap, for
example, rises with TR until it reaches level 100 at TR 80 and then stays
there. Future content can add new things that keep growing past today's range,
so extra TR still means something.

### What your TR drives

- **Your level cap, experience, and obedience.** A higher TR raises your soft
  level cap; Pokémon past it earn less experience and may disobey
  ([wild encounter and party progression](trainer-rating-wild-encounter-scaling.md)).
- **Wild and static encounters**, under their current policies
  ([wild encounter scaling](trainer-rating-wild-encounter-scaling.md)).
- **Ordinary trainers and Gym members**
  ([ordinary Trainer and Gym-member scaling](trainer-party-scaling.md)).
- **Poké Mart stock** ([Poké Marts](global-tr-pokemarts.md)).

Your TR never makes a well-known trainer stronger or weaker: they have their
own rating ([Well-known trainers](well-known-trainers.md)), and League fields
are drawn from those ratings ([Leagues](leagues.md)).

## Boundaries

- Each consumer keeps its current rules; this document only gathers what they
  share.
- Well-known trainers' ratings never read yours, and yours never reads theirs.
- Standalone builds keep their existing progression.

## Later

- New ways to earn TR: a reworked badge curve, renown, exploration or catching
  paths, and leagues as tests.
- Scalers that saturate beyond today's range, arriving with the content that
  pushes TR higher.
- The treadmill concern (the world scaling alongside the player's growth) and
  the widening world gap.

## References

- [Player Trainer Rating specification](../specs/player-trainer-rating.md)
- [Well-known trainers](well-known-trainers.md)
- [Leagues](leagues.md)
- [Wild encounter and party progression](trainer-rating-wild-encounter-scaling.md)
- [Ordinary Trainer and Gym-member scaling](trainer-party-scaling.md)
- [Poké Marts](global-tr-pokemarts.md)

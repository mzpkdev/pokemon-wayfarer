# Player Trainer Rating

Implemented: Partial. Today's badge and first-league-win formula, its saved
value that never decreases, and every consumer listed below exist. The new
badge scale, uncapped TR, and the world scaling that follows it are v0.
Design status: v0 accepted. How TR is earned and how the world answers it are
approved; the exact numbers are placeholders tuned by playtesting.

## Intent

Give the player one measure of how far their career has come, and let the
world answer to it. Every badge raises it; the level cap, wild Pokémon,
regular trainers, and shops follow it, whatever order the player explores in.
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
the way to the last. TR never decreases: losses and replays add nothing, and a
later check never lowers what you have earned. A new game starts at 0.

League wins give you nothing. A league is a test of what you have built, not a
source of power: winning one proves you are ready, and your next step still
comes from badges.

Today, the ROM still uses the earlier formula, where each first league win
also raised TR and the total stopped at a fixed ceiling. It stays in place
until the new scale is adopted.

### No ceiling

In v0, TR has no upper limit. All 24 badges take you to the top of today's
content, but stronger future content can go higher.

Everything TR drives has its own ceiling instead. Your level cap, for example,
rises with every badge until it reaches level 100 with all 24 badges and then
stays there. Future content can add new things that keep growing past today's
range, so extra TR still means something.

### How the world answers

- **Early on, the world is dangerous.** With a handful of badges, wild Pokémon
  and regular trainers sit just under your level cap. Every fight on the road
  matters.
- **Late on, routes are no threat.** With many badges, your level cap climbs
  far ahead of the wild and of regular trainers. You travel freely.
- **The challenge comes from people you know.** Gym Leaders, league lineups,
  and notable trainers met on the road bring the late-game fights, at their
  own strength ([Notable trainers](notable-trainers.md)).
- **Stronger forms appear only once they've reached the right level, for
  everyone.** A wild Pokémon, a regular trainer's, or a notable trainer's
  below that level appears as an earlier form
  ([evolution stages](../specs/player-trainer-rating.md#evolution-stages)).

### What your TR drives

- **Your level cap, experience, and obedience.** A higher TR raises your level
  cap. The cap is soft: Pokémon past it earn less experience and may disobey
  ([wild encounter and party progression](trainer-rating-wild-encounter-scaling.md)).
- **Wild and static encounters**
  ([wild encounter scaling](trainer-rating-wild-encounter-scaling.md)).
- **Regular trainers and Gym members**
  ([regular trainer and Gym member scaling](trainer-party-scaling.md)).
- **Poké Mart stock**, unlocking at the same badge counts as today
  ([Poké Marts](global-tr-pokemarts.md)).
- **World progress.** Notable trainers read your TR as world progress and
  grow with it, each in their own way and up to their own best
  ([Notable trainers](notable-trainers.md)). League lineups are drawn from
  their TR when you enter ([Leagues](leagues.md)).

Your TR is never worked out from a notable trainer's TR, and nothing they do
changes yours.

## Glossary

- **Trainer Rating (TR):** a hidden measure of strength. You have one, and so
  does every notable trainer; after first use, docs just say TR.
- **Player TR / trainer TR:** yours, or a notable trainer's, where it matters
  which.
- **World progress:** your TR as notable trainers see it; they grow with it.
- **Archetype:** a notable trainer's growth shape: steady, early bloomer, late
  bloomer, plateau, rival, fixed, rising star, second wind, or bursts.
- **Start TR / peak TR:** a notable trainer's TR at the very start of your
  journey, and the most they can ever reach.
- **Never decreases:** once earned, TR is never lowered.
- **Notable trainer:** a Gym Leader, Elite Four member, Champion, or Blue, each
  with their own TR: 38 entries in v0 (37 people plus the Tate & Liza duo).
- **Regular trainer:** every other trainer you battle.
- **Gym member:** a regular trainer who works in a Gym.
- **Level cap:** the level your Pokémon can reach before they earn less
  experience and may disobey.
- **World scaling:** wild Pokémon and regular trainers following your TR.
- **Wild level curve / regular trainer level curve:** how wild Pokémon and
  regular trainers' levels follow your TR.
- **Authored level bonus:** a small, hand-set level bump for one regular
  trainer.
- **Ceiling:** the point where something TR drives stops growing, and the TR
  at which that happens.
- **Roster:** a notable trainer's ordered list of six Pokémon.
- **Roster slot (1-6):** one position in that list.
- **Signature Pokémon:** roster slot 1, the Pokémon a trainer is known for. It
  is on every team they bring and is always fought last.
- **Ace:** one of a trainer's one to three star Pokémon, the signature
  Pokémon included; aces are fought last.
- **Filler slot:** any roster slot that isn't an ace; its Pokémon is fought
  before the aces.
- **Battle snapshot:** the team fixed when a battle starts and kept for the
  whole fight.
- **League:** Indigo, Sevii Masters, or Hoenn.
- **Lineup:** the five opponents of a league.
- **Locked lineup:** the lineup kept from when you enter a league until you win
  it.
- **Match 1-5:** a position in the lineup.
- **First league win:** your first win at a given league.
- **Challenge options:** the opt-in challenge menu.
- **Today / v0 / Later:** what the game does now, the accepted design, and
  ideas deferred for now.
- **Placeholder:** a value waiting to be authored or tuned.

## Boundaries

- Each consumer keeps its own rules and owns its own curve; this document only
  gathers what they share.
- Obedience, reduced experience past the level cap, and Candy rules keep
  working the same way; they follow the level cap wherever it sits.
- Yours never reads a notable trainer's TR; theirs reads yours only as world
  progress, through their own growth shape.
- Standalone builds keep their existing progression.

## Later

- More ways to earn TR: renown, and exploration or catching paths.
- Scalers whose ceilings lie beyond today's range, arriving with the content
  that pushes TR higher.
- Further tuning of the treadmill (the world scaling alongside the player's
  growth) and the widening world gap, now that late routes fall behind.

## References

- [Player Trainer Rating specification](../specs/player-trainer-rating.md)
- [Notable trainers](notable-trainers.md)
- [Leagues](leagues.md)
- [Wild encounter and party progression](trainer-rating-wild-encounter-scaling.md)
- [Regular trainer and Gym member scaling](trainer-party-scaling.md)
- [Poké Marts](global-tr-pokemarts.md)

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
the way to the last. TR never decreases: losses and repeat wins add nothing, and a
later check never lowers what you have earned. A new game starts at 0.

League wins give you nothing. A league is a test of what you have built, not a
source of power: winning one proves you are ready, and your next step still
comes from badges.

Today, the ROM uses a different formula, where each first league win also
raises TR and the total stops at a fixed ceiling. It stays in place until the
v0 scale is adopted.

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
  and the other notable trainers bring the late-game fights, at their own
  strength ([Notable trainers](notable-trainers.md)).
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
  ([Notable trainers](notable-trainers.md)). League lineups come from
  their TR when you answer a league's invitation ([Leagues](leagues.md)), and
  reaching TR 80 qualifies you for invitations.

Your TR is never worked out from a notable trainer's TR, and nothing they do
changes yours.

## Glossary

- **Trainer Rating (TR):** a hidden measure of strength. You have one, and so
  does every notable trainer; after first use, docs just say TR.
- **Player TR / trainer TR:** yours, or a notable trainer's, where it matters
  which.
- **World progress:** your TR as notable trainers see it; they grow with it.
- **Archetype:** a notable trainer's growth shape, named as a proper noun
  ("Brock is a Steady"): Steady (keeps a fixed share of your pace), Prodigy
  (brilliant early, then evens out), Sleeper (underestimated, strong at the
  end), Veteran (peaked already, you overtake them), Rival (level with you,
  then a step ahead), Legend (never changes, waits at the top), Star (explodes
  mid-journey), Comeback (stalls, then returns stronger), or Burst (trains in
  jumps at milestones).
- **Start TR / peak TR:** a notable trainer's TR at the very start of your
  journey, and the most they can ever reach.
- **Never decreases:** once earned, TR is never lowered.
- **Notable trainer:** a Gym Leader, Elite Four member, Champion, or Blue, each
  with their own TR: 38 entries in v0 (37 people plus the Tate & Liza duo).
- **Regular trainer:** every other trainer you battle.
- **Gym member:** a regular trainer who works in a Gym.
- **Scaler:** a TR-driven property, authored as a few anchor points (a TR
  and a value). Between anchors the value follows a straight line, or, for a
  **step** scaler, holds each anchor's value until the next anchor; past the
  last anchor it stays flat
  ([scalers](../specs/player-trainer-rating.md#scalers)).
- **Level cap:** the level your Pokémon can reach before they earn less
  experience and may disobey.
- **World scaling:** wild Pokémon and regular trainers following your TR.
- **Wild level curve / regular trainer level curve:** how wild Pokémon and
  regular trainers' levels follow your TR.
- **Team level:** the level a notable trainer's team is built around, a
  scaler over their own TR; from TR 40 up it matches the level cap at the
  same TR ([trainer scalers](../specs/notable-trainers.md#trainer-scalers)).
- **Team size:** how many Pokémon a notable trainer brings, one to six: a
  step scaler over their own TR, so it holds each size until the next step
  ([trainer scalers](../specs/notable-trainers.md#trainer-scalers)).
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
- **Evolution level:** the level a Pokémon needs for its stage: the game's
  own level for a level evolution, or one shared authored level for a trade,
  stone, friendship, or other evolution.
- **Downward rule:** a Pokémon below its stage's evolution level appears as
  an earlier form, one stage at a time, until its level fits. It never steps
  into a baby form and never evolves forward
  ([evolution stages](../specs/player-trainer-rating.md#evolution-stages)).
- **Play style:** a notable trainer's battle identity, exactly one each
  (Gambler, Bomber, Sweeper, Field marshal, Hexer, Turtle, Brawler, or
  Tactician); it adds its AI preferences on top of the basics
  ([Trainer AI](trainer-ai.md)).
- **AI skill:** how smart a notable trainer plays, a step scaler over their
  own TR that adds smarter AI in tiers.
- **Ace protection:** the AI holds a trainer's aces back until the end: the
  last one when the team has one ace, the last two when it has two or more.
- **Boss flag:** an authored mark that makes a notable trainer know the
  player's whole party (Lance only in v0).
- **Move pool:** a notable trainer's ordered list of the moves they like; the
  aces pick first.
- **Dormant entry:** a move-pool entry that none of the trainer's current
  Pokémon can use yet; it wakes once one can.
- **Battle snapshot:** the team fixed when a battle starts and kept for the
  whole fight.
- **League:** Indigo, Sevii Masters, or Hoenn.
- **Qualify:** your TR reaching 80 (a placeholder), which makes you eligible
  for league invitations. TR never decreases, so it happens once.
- **Known (by a league):** a league knows you once you hold a badge from its
  region: Indigo with a Kanto or Johto badge, Hoenn with a Hoenn badge. Sevii
  Masters knows you after any league win. Only a league that knows you calls.
- **Invitation:** a league's phone call inviting you to its next event. Once
  you qualify, a league that knows you calls every seven in-game days, the
  leagues taking turns
  ([which league calls](../specs/leagues.md#which-league-calls)). Days only
  schedule calls; they never change anyone's strength.
- **Accept / decline:** your answer to an invitation. Accepting fixes the
  event lineup and the event waits for you, with one attempt; declining lets
  the event run without you.
- **League event:** one tournament at a league, held for one invitation.
- **Event lineup:** the five opponents of a league event, picked when you
  accept or decline; an accepted event keeps them, with their teams, until it
  ends.
- **Reigning champion:** who holds a league's title until its next event: you
  if you won the last one, otherwise the strongest trainer of that event's
  lineup.
- **Home region:** a notable trainer's region (Kanto, Johto, or Hoenn).
- **Trait:** an opt-in yes/no behaviour of a notable trainer; every trait
  defaults to no. v0 has two: traveller and aloof.
- **Traveller:** a trait; being away from home costs a traveller little
  willingness.
- **Location:** a place that picks which notable trainers turn up there, by
  their willingness; in v0 the leagues are the only locations. Gym and story
  battles are not locations: each always brings its own trainer.
- **Location region:** the region or regions a location belongs to (Indigo:
  Kanto and Johto; Hoenn: Hoenn).
- **Neutral location:** a location that is home to everyone (Sevii Masters).
- **At home / away:** at home when the location is in the trainer's home
  region or neutral; away otherwise.
- **Travel cost:** how much being away from home puts a trainer off: nothing
  at home, a little for a traveller, a lot for anyone else
  ([travel](../specs/notable-trainers.md#home-region-and-travel)).
- **Fatigue:** the tiredness of a trainer who played in the most recent
  resolved league event, whether you accepted it or declined it; it makes
  them less willing to join the next
  ([selection](../specs/leagues.md#selection-and-order)).
- **Willingness:** how keen a trainer is to turn up at a location: full at
  home, lowered by travel cost and fatigue, and never quite zero
  ([travel](../specs/notable-trainers.md#home-region-and-travel)).
- **Aloof:** a trait; an aloof trainer joins a league only when its base
  lineup is close enough to their own team level, and skips it when there is
  no base lineup ([selection](../specs/leagues.md#selection-and-order)).
- **Base lineup:** a league event's five best trainers by league score who
  are not aloof, picked before any aloof trainer is considered.
- **Base lineup level:** the highest team level in the base lineup.
- **League score:** a trainer's TR scaled down by their willingness; the
  event lineup is the five highest among those who join
  ([selection](../specs/leagues.md#selection-and-order)).
- **Match 1-5:** a position in the lineup.
- **First league win:** your first win at a given league, which brings its
  one-time effects; later wins are repeat wins (prize money and the title).
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
  growth) and the widening world gap, as late routes fall behind.

## References

- [Player Trainer Rating specification](../specs/player-trainer-rating.md)
- [Notable trainers](notable-trainers.md)
- [Leagues](leagues.md)
- [Trainer AI](trainer-ai.md)
- [Wild encounter and party progression](trainer-rating-wild-encounter-scaling.md)
- [Regular trainer and Gym member scaling](trainer-party-scaling.md)
- [Poké Marts](global-tr-pokemarts.md)

# Notable trainers

Implemented: No. Today, the ROM keeps its existing Gym and league scaling
until this design is adopted; the balance explorer is placeholder tooling.
Design status: v0 accepted. Every trainer's team is approved (draft v1:
who they bring, in what order, and which are aces); their favourite moves
and items, their growth numbers, and the exact team-size steps are still
placeholder content under review. Home regions and traits follow lore and
are open to review. Terms follow the
[glossary](player-trainer-rating.md#glossary).

## Intent

Make familiar trainers feel like people with a place in the world. Gym
Leaders, Elite Four members, Champions, and Blue each have a strength of their
own, measured the same way as the player's, and they keep pace with your
journey in their own way. Whenever you meet one of them, anywhere, you face the
same trainer at their current strength, and you can tell at a glance roughly
how you compare.

## Design

### Their own rating, on your scale

You have a Trainer Rating (TR); so does every notable trainer. Yours is never
worked out from theirs. Their TR uses the same scale as yours, so a leader
rated like a player with eight badges brings Pokémon around the level your own
cap reaches with eight badges.

A trainer's TR decides every battle with them: their Gym, a story battle, a
rematch, or a league. Story battles include Blue's
rival fights, Giovanni's Rocket battles, and the Saffron Dojo. There are no
special cases.

### They keep pace with your journey

Each notable trainer starts from how established they already are when your
journey begins. Blue leaves Pallet the same day you do, so he starts from
nothing. Gym Leaders are established trainers, so none of them is a pushover
even at the start, and the Elite Four and Champions start higher still.

From there, notable trainers grow as your journey goes on, each in their own
way:

- **The Steady** keeps a fixed share of your pace, from their starting
  strength to their best, like Brock.
- **The Prodigy** is brilliant early, then evens out: they grow fast early and
  then slow, a wall in the middle of your journey.
- **The Sleeper** is underestimated and strong at the end: they start slow and
  finish strong, a challenge waiting late in the game.
- **The Veteran** peaked already, and you overtake them: they reach their best
  early and stop there.
- **The Rival**, Blue, starts level with you and pulls a step ahead over your
  first badges, then stays there until he reaches his best.
- **The Legend** never changes and waits at the top: already at their best,
  they wait for you to catch up, like Agatha and Lance.
- **The Star** explodes mid-journey: they start modestly, explode in the
  middle of your journey, then settle at their best, like Misty, Whitney, and
  Wallace.
- **The Comeback** stalls, then returns stronger: they grow fast early, stall
  for a long stretch, then surge again late, like Blaine, Pryce, and Bruno.
- **The Burst** trains in jumps at milestones: they keep pace on average but
  get stronger all at once at milestones in your journey, like Chuck,
  Giovanni, Brawly, and Steven.

Everyone grows by the same rule: only the shape, the starting strength, and
the best differ. They only move when you do. A big step, like one of your first badges, moves
them a lot; a small one barely moves them; wandering around moves nobody, and
saving and reloading changes nothing. Every trainer has a best they never go
past. Their growth is the same in every save.

### Who is notable

In this first version there are 38 notable trainers: 37 people (the 23
singles Gym Leaders, the Kanto, Johto, and Hoenn Elite Four, Lance, Wallace,
Steven, and Blue) plus Tate and Liza as one duo. Tate and Liza share one
strength and one list of six Pokémon, grow like everyone else, and still fight
you together in their double battle. Leagues field trainers who battle
alone, so the duo never appears in one, as opponents or as a partner.
Everyone else, including Red, keeps their current rules.

### Home and travel

Every notable trainer comes from a home region, Kanto, Johto, or Hoenn.
At a location, a place such as a league that picks who turns up, a trainer
is at home when the place belongs to their region, or is neutral ground like
Sevii Masters, and away otherwise.
Most trainers aren't travellers: being away costs them a lot of willingness
to turn up. Leagues use this ([Leagues](leagues.md)), and so do haunts,
where a trainer who isn't a traveller only appears in their home region
([Notable haunts](notable-haunts.md)).

### Traits

A **trait** is an opt-in yes-or-no quirk of a notable trainer; a trainer
without it behaves the default way. This first version has two traits, which
leagues and haunts pay attention to ([Leagues](leagues.md),
[Notable haunts](notable-haunts.md)). Neither has anything to do with how a
trainer grows.

A **traveller** goes wherever the action is: being away costs them very
little willingness. The travellers are Brock, Misty, Giovanni, Blue, Bruno,
and Lance from Kanto; Bugsy and Will from Johto; and Brawly, Glacia, Drake,
Wallace, and Steven from Hoenn. Everyone else stays close to home.

Some proud trainers are **aloof**: they won't bother with a league whose base
lineup (its best trainers who aren't aloof) is far below them, but they join
once it is strong enough. The aloof trainers are the Champions Lance, Wallace, and Steven, who
only grace elite leagues; Agatha, Oak's proud old rival; Glacia, who came to
Hoenn looking for worthy opponents; Clair, the proud dragon tamer; cold,
distant Sabrina; and Karen, whose "strong Pokémon, weak Pokémon" disdains
weak company. Everyone else is not aloof. The Sevii Masters is the exception:
it is the elite company they seek, so the aloof rule doesn't apply there
([Sevii Masters](sevii-masters.md#design)).

These picks follow the trainers' stories and are open to review
([assignments](../specs/notable-trainers.md#traits)).

### No ceiling

Like yours, a trainer's TR has no ceiling
([Player Trainer Rating](player-trainer-rating.md#no-ceiling)). Today's
trainers span roughly the same range as the player, but stronger future
content can go higher.

Every trainer's team is set; their starting strength, growth, and best are
placeholders for now, set on the v0 badge scale
([Player Trainer Rating](player-trainer-rating.md)) and re-set with
playtesting.

### Bigger and stronger teams at higher TR

A higher TR means both higher levels and a bigger team. The team grows in
steps: it keeps its size until the trainer reaches the next step. Early
fights are fair:
the lowest-rated trainers bring a single Pokémon, so Blue's first fight is one
on one at your level (level 5), and he pulls ahead over your first badges.
For trainers rated like a player with four badges or more, levels follow the
same curve as your level cap. Every notable trainer eventually fields a full
team of six, and a trainer rated like a player with all 24 badges brings six
at level 100. League lineups are the strongest trainers of the moment who
are willing to come; how hard each league feels is reported for now and tuned later
([Leagues](leagues.md)). Each Pokémon has a small, hand-set level
difference, so a team feels shaped rather than uniform.

### Their stars come last

Every trainer has one hand-picked list of six Pokémon, the same in every battle
and every save; Blue brings the same six whichever starter you chose. A small
team is the start of that list, and each step up adds the next one. The first
Pokémon on the list is their signature Pokémon: it is on every team they bring.

The signature Pokémon and up to two more are the trainer's aces, their stars.
The rest of the list are filler slots, whose Pokémon join between them as the
trainer grows. You face the fillers first and the aces last, with the
signature Pokémon at the very end. Where each ace sits in the list decides when it first
appears, so a trainer can keep one back until their team is full and reveal it
late in your journey.

Lists name each Pokémon at its final form, such as Brock's Steelix and Golem.
Stronger forms appear only once they've reached the right level: until then a
Pokémon comes as an earlier form, so Brock opens with Onix and Geodude and
brings Steelix once his team reaches level 35. Pokémon never evolve past what
the list names, and a trainer can name a form that isn't final on purpose,
like Whitney's Ursaring, which still comes as Teddiursa below its evolution
level ([evolution stages](../specs/player-trainer-rating.md#evolution-stages)).

Each trainer has a move pool: an ordered list of the moves they like. Their
Pokémon start from their usual moves for their level, keep the pool moves
they already know, and swap in the others they can use; the aces get first
pick. A Pokémon can also use moves from its earlier forms or from breeding.
Moves they learn naturally arrive on their natural schedule; special moves
wait until the level set for them. An entry no Pokémon on the team can use
yet stays dormant until the right Pokémon joins the team, evolves, or grows
into it ([move pools](../specs/notable-trainers.md#move-pools)).
Each trainer leans on at most one frustrating trick, such as sleep, evasion,
or trapping, so no single battle stacks them; only moves whose main purpose
is the trick count, not a damaging move's side chance.

### They battle in their own style

Every notable trainer has a play style that fits their story, such as
Brock's wall of rocks or Whitney's stubborn Miltank. Stronger trainers play
smarter, their aces are held back until the end, and a rare boss like Lance
knows everything about your party ([Trainer AI](trainer-ai.md)).

### Friendship

Every notable trainer has a friendship with you that only grows. It starts
at Stranger and moves to **Met** when you first talk, **Friend** once you
have won against them or helped them with a few quests, and **Close** after
a lot more of both. Nothing else moves it, and chatting again and again adds
nothing ([friendship](../specs/notable-trainers.md#friendship)). A trainer
who becomes a Friend gives you their phone number, which the
[Sevii Masters](sevii-masters.md#design) uses to find you a partner.

### Out in the world

Between their Gym and league battles, notable trainers also turn up at
haunts around the overworld, where each offers the haunt's quest
([Notable haunts](notable-haunts.md)). Once routines and travel arrive,
they spend the rest of their time at everyday [spots](notable-spots.md),
such as a Pokémon Center or the edge of a pond.

FRLG, Emerald, and HNS parties are references for recognizable content, not
required teams. Challenge options such as trainer items, trainer IVs and EVs,
and the level cap apply on top, as they do today.

## Encounters

A battle's team is set from the trainer's strength when it starts and kept for
the whole fight. Retrying brings the same team at the same levels, unless you
have grown in the meantime. League opponents also fight with
their own TR and team; [Leagues](leagues.md) decides who is in each lineup,
which is set when you accept a league's invitation and kept for that event. A
trainer who played in the most recent event, at any league, whether you took
part or declined it, is tired for the next.

## Gym battles

Players can take the Gyms in any order and always find a fair, recognizable
fight. From your first badges on, some leaders sit near you and some clearly
above, and from about eight badges some sit below you too: fair fights, clear
goals to build towards, and later some easy wins. At the very start every
leader is ahead of you.

A Gym battle uses the leader's own TR at that point in your journey, with their
team and levels. The Gym adds nothing on top: the same leader met anywhere else
at that point is the same trainer at the same strength. The badge is awarded
after the battle. Until adoption, the ROM keeps today's behavior: the earlier
player-TR Gym scaler exists in code but is disabled by default, and Giovanni's
Wayfarer finale has its own path.

Wayfarer has 24 badge encounters, and this design covers all of them: Brock, Misty, Lt. Surge, Erika, Janine, Sabrina, Blaine, Giovanni
(his Viridian finale); Falkner, Bugsy, Whitney, Morty, Chuck, Jasmine, Pryce,
Clair; Roxanne, Brawly, Wattson, Flannery, Norman, Winona, Tate and Liza, and
Juan.

Tate and Liza stay a double battle. Their team size comes from their shared
TR, and their Pokémon come out in battle order like any notable trainer's,
fillers first and aces last, taking turns between the two of them so that
each ends on an ace where possible: with a full team, Tate brings Xatu,
Gardevoir, and Solrock, and Liza brings Grumpig, Claydol, and Lunatone. Blue has no badge encounter in Wayfarer.

Each leader's hand-written team supplies species, items, and abilities, and
their move pool supplies the moves; all of it stays attached to the right
Pokémon when a team is reordered.
Rewards, prize money, and badge scripts are preserved unless a team
deliberately changes them; how the leader battles comes from
[Trainer AI](trainer-ai.md). Trainer-species randomization keeps its existing
path; other randomizer and challenge options keep their precedence.

## Boundaries

- Your own TR, level cap, experience, obedience, wild and static encounters,
  shops, regular trainers, and Gym members follow your TR under their own
  rules ([Player Trainer Rating](player-trainer-rating.md)).
- The same TR decides every battle with a notable trainer; how each
  battle is built stays with the
  [Gym Leader scaling specification](../specs/gym-leader-scaling.md).
- Standalone builds are unchanged.

## Balance

Each trainer's TR and team are content under review. Leader ratings and teams
need ROM playtesting before enablement, including checks that Gym members do
not routinely outclass their leader. The
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) predicts
species, team size, levels, moves, and each trainer's AI settings; items, how
well they play, and combat difficulty belong to playtesting.

## Later

- More notable trainers, such as Red.
- Hints in the world about who is too strong for you right now: Gym guides,
  gossip, or a line on your Trainer Card.
- Concrete feel checks for playtesters, such as winning the first Gym with a
  lightly trained team.
- A meaning for the "next Gym's highest or lowest level" cap options in an
  open world.
- Growth shapes that differ from save to save, and other per-save variety.
- Faster or slower growers, and hand-made growth shapes for single trainers.
- Some of your progress counting more than other progress for how trainers
  grow.
- Trainers who also grow from their own battles.
- Filler slots whose Pokémon vary from save to save or with what you do, and
  trades that put one of your Pokémon on a trainer's team; aces never change
  ([Trainer roster influence](trainer-roster-influence.md)).
- Better items and moves once teams reach level 100; smarter AI is
  [Trainer AI](trainer-ai.md#later)'s.
- Tighter level spreads at the top.
- Gym arenas with their own field conditions for both sides, like the
  leagues' [halls](leagues.md#design).

## Specifications

- [Notable trainers specification](../specs/notable-trainers.md):
  inventory, TR and its growth, scalers, rosters, friendship, and the battle
  snapshot.
- [Sevii Masters specification](../specs/sevii-masters.md): phone contacts,
  given when a notable trainer becomes a Friend.
- [Notable haunts specification](../specs/notable-haunts.md): where notable
  trainers appear in the overworld, and the buddy value.
- [Notable spots specification](../specs/notable-spots.md): the everyday
  places of the future routine and travel design.
- [Gym Leader scaling](../specs/gym-leader-scaling.md): badge-encounter
  coverage and battle construction.
- [Trainer AI specification](../specs/trainer-ai.md): play styles, AI skill,
  ace protection, and the boss flag.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Leagues](leagues.md)
- [Sevii Masters](sevii-masters.md)
- [Notable haunts](notable-haunts.md)
- [Notable spots](notable-spots.md)
- [Trainer AI](trainer-ai.md)
- [Regular trainer and Gym member scaling](trainer-party-scaling.md)
- [Player progression](../specs/trainer-rating-party-progression.md)

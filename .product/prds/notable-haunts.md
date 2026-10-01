# Notable haunts

Implemented: No Specification: [Notable haunts
specification](../specs/notable-haunts.md) Design status: v0 draft for review:
notable trainers spend their time between Gyms and leagues at **haunts**, one
shared pool of overworld spots that names no trainer. Placement fills each haunt
with the best-fitting free trainer, recomputed whenever world progress changes,
with no seeds. A haunt offers only its own **quest**: no battle offers, no
rematches, and no menu. Every talk greets the player by the trainer's
**friendship** (a stranger, then met, a friend, and close), proposes the quest
while it is open, and says goodbye. Haunt lines describe only the place and
activity, and each trainer's **voice bits** supply the personality. v0 authors
the Kanto list; weights, gates, and reward pools are placeholders. Terms follow
the [glossary](player-trainer-rating.md#glossary).

## Intent

Notable trainers should feel like people who live in the world, not
statues waiting in their Gyms. Between their Gym and league battles, the
player should bump into them out in the overworld: Brock resting in a cave,
Lt. Surge watching the harbour, a Hoenn traveller passing through Kanto.
Meetings should emerge rather than be scripted one by one: a small pool of
places, each able to host many trainers, filled by who fits best at this
point in the journey. Every meeting should sound like that trainer, and
every placement should give the player one thing to do with them: the
haunt's quest.

## Design

### Haunts

A **haunt** is an overworld spot where a notable trainer can be found. No
haunt names a trainer, and one haunt serves many trainers over a
playthrough. Each haunt has tags that describe it:

- its **region**;
- its **theme types**, such as Ground for Diglett's Cave;
- whether it is **elite**: an elite haunt only opens once the player's TR
  reaches its gate, and it is the only kind an aloof trainer visits;
- its **hometown**, when it sits in a Gym Leader's city;
- up to two **activities** from one shared list: train, care, study, home,
  relax, gamble, shop, lie low, and sightsee;
- its **setting**: public, such as a harbour, or remote, such as a cave;
- its **capacity**, how many trainers it holds at once: 1 unless a haunt
  says otherwise, and 1 for every haunt in v0; and
- the map details its quest needs.

### Who goes where

Placement puts at most one trainer in each haunt and each trainer in at most
one haunt. It is worked out again whenever world progress changes, so the
cast shifts as the player earns badges, and it uses no seeds: the same world
progress, league lineup, and partner give the same placement.

Some rules keep the text true, and a trainer who breaks one never goes to
that haunt:

- **Region.** A trainer visits haunts in their home region, and a traveller
  goes anywhere.
- **Aloof.** An aloof trainer only visits elite haunts, and never public
  ones.

Among the trainers who pass, preferences rank the fit:

- **Theme.** A haunt whose theme matches a trainer's aces draws them.
- **Activity.** A trainer's play style leans them towards some activities,
  and so does their **momentum**: a trainer who is rising, getting stronger
  fast right now, goes training, while a settled one relaxes or sightsees.
- **Hometown.** A Gym Leader likes the haunts of their own city.

Each haunt takes the best-fitting free trainer, and the order in which haunts
choose shifts with every step of world progress, so the same trainers turn
up in different places over a journey. Trainers are not placed while they
are in an accepted league lineup, or while they are the player's
[Masters partner](sevii-masters.md#design).

### Meeting a trainer

A haunt offers only its quest. There is no battle offer, rematch, or menu there,
and every talk goes the same way:

1. **A greeting** by the trainer's **friendship** with the player
   ([Notable trainers](notable-trainers.md#friendship)), which only grows:
   - **Stranger:** the player has never talked to them. They introduce
     themselves, or greet the player as someone they have heard of when the
     player's fame has reached them (the player is famous everywhere at TR
     80, or close to their own TR, or holds a league title or is a Master).
   - **Met:** they remember the player and greet them again.
   - **Friend:** they greet the player warmly. They have given their number.
   - **Close:** a warmer greeting still.
2. **The quest**, proposed while it is still open during this placement; the
   player says yes or no, and no costs nothing: the next talk asks again.
   Once the quest is done, the trainer shares a quirk instead.
3. **A goodbye.**

The first talk makes the trainer Met, so a first meeting both introduces them
and proposes the quest. Friendship grows through that first talk, through
finished quests, and through battles won against them elsewhere (Gym, story,
league); haunts offer none of their own. The player can revisit a haunt as often
as they like while the trainer is placed there.

### Quests

Each haunt has one quest, which the trainer proposes at every talk until
it is done:

| Quest | What happens |
| --- | --- |
| Walk with me | The trainer follows the player to the haunt's exit; wild POKéMON on the way are tag battles beside them. |
| Lost something | Something valuable went missing at the haunt; the player finds it. |
| Catch me one | The player catches a POKéMON that lives at the haunt and shows it to the trainer, keeping it. |
| Quiz | The trainer quizzes the player on type matchups for their aces. |
| One on one | One of the player's POKéMON against the trainer's lead ace. |
| Bring me | The player brings one cheap item of a kind that fits the place, such as a berry, a healing item, or a stone, and hands it over. |
| Swap battle | It looks like a One on one: the player picks their best POKéMON, and only then the trainer reveals the twist. The two trade places: the player battles with the trainer's ace, which may not listen, against a copy of their own pick. The player's own party comes back untouched. |
| Trade | The trainer offers one of their fillers, never an ace, for a POKéMON of their type from the player's party. It's accepted if its line is worth at least as much as the filler's; the player's POKéMON then takes that filler's place on the trainer's team, keeps who it is, and evolves as the trainer grows. A later Trade quest offers it back. |

Every trainer can give every quest. Haunt quest lines never imply the trainer
needs help or is asking a favour, and personality comes only from the
trainer's voice bits.

The reward comes from the trainer, never from the quest: the haunt carries
the ask, and the trainer carries the reward. Each trainer has a short
**reward pool**, an ordered list of about five items and lessons, much like
their move pool. A lesson teaches the player's chosen POKéMON the next move
from the trainer's move pool that it can learn. Each entry opens once
world progress reaches its gate, so modest gifts come first and strong held
items come late. Every quest the player finishes with a trainer, at any
haunt, pays their next entry; if it isn't open yet, or the pool is used up,
the trainer pays prize money instead, as much as a win over them. A trainer
gives one reward while they are placed at a haunt; when someone else takes
the haunt, its quest can be done again. The starting pools are in
[notable trainer reward pools](../research/notable-trainer-rewards.md).

### Voices

Haunt lines describe only the place and the activity. Personality comes from
each trainer's fifteen **voice bits**, short reusable lines for greeting,
asking, thanking, and so on ([voice
bits](../research/notable-trainer-voices.md)). Lines can mention the player, the
reward item, the trainer's ace, and their **buddy**. Haunts carry no gossip in
v0. A trainer's buddy is their companion POKéMON, one slot of their roster,
shown at its current stage: Brock's buddy is Onix early in the journey and
Steelix later.

## Sample playthrough

Diglett's Cave is a Kanto haunt with a Ground theme, remote, where trainers
come to train or lie low; its quest is Walk with me, from the Route 2
entrance to the Vermilion exit. Any Kanto trainer or non-aloof traveller can
be placed there; Giovanni fits it best, then Brock and Drake. (The switch to
the shared activity list moved these scores, so exactly who is free for the
cave at each badge is pending the routine design.)

1. With one badge, the player finds Giovanni there. They have never talked,
   and the player's fame hasn't reached him, so he introduces himself, and in
   the same talk proposes a walk through the tunnel. The player says no; it
   costs nothing, and the next time Giovanni greets them as someone he has
   met and asks again. This time they say yes.
2. On the walk, pairs of wild Diglett are tag battles with Giovanni at the
   player's side. Heading back towards Route 2 asks whether they are giving
   up; saying yes ends the walk with no reward, and so does a whiteout.
   Either way Giovanni is back at his spot and the walk can start again. At
   the Vermilion exit he hands over a Nugget, the first entry of his reward
   pool, and says goodbye.
3. The player comes back: Giovanni is still there, but his reward is
   claimed, so he greets them, remarks on his Meowth, and says goodbye.
4. After a later badge the cast reshuffles and Misty takes the cave. The
   player beat her at her Gym, so she is already a friend. The walk is open
   again with her, and it pays the first entry of her own pool, Mystic
   Water; after that, she greets the player, shares her quirk, and says
   goodbye.
5. Late in the game, Brock is close with the player after a Gym win and
   several quests. At the cave he greets them warmly and proposes the walk,
   which now pays a lesson from his pool, open once world progress reaches
   40. His quirk now names Steelix, since his Onix has grown up with the
   world. Giovanni's next quest, wherever he turns up, will pay his second
   entry, Soft Sand.

A second worked haunt, the Celadon Game Corner, is public and draws
trainers by one activity, gamble: Lt. Surge is its natural regular, Blaine
the rarer one. Its quest is One on one framed as a bet, "One POKéMON each,
winner takes the pot": the player's one POKéMON against the trainer's ace
at its current stage, such as Lt. Surge's Pikachu early on. A win pays the
trainer's next reward; a loss costs nothing, and the bet stays open. The
[specification](../specs/notable-haunts.md#worked-example-celadon-game-corner)
works it through.

A third, Cerulean Cape, is the quiet clifftop past Bill's house on Route 25:
Water-themed, Misty's hometown, remote, where trainers relax and sightsee.
Its quest is Lost something: the trainer mentions something went missing
"by the rocks" or "near the fence", and the player searches that spot,
which stays open while they come and go until the cast reshuffles; the
Dowsing Machine helps. Finding it and reporting back pays the trainer's
next reward. Misty is its natural regular, the iconic pairing from HGSS,
and Lorelei takes it on her Water aces when Misty is elsewhere. The
[specification](../specs/notable-haunts.md#worked-example-cerulean-cape)
works it through.

A fourth, the Pewter Museum, is public and indoors, Rock-themed, Brock's
hometown, where trainers study and sightsee among the fossils. Its quest is
a Quiz: three type-matchup questions about the trainer's own aces as they
stand right now, such as what hits Brock's Onix super effectively. All
three right pays the trainer's next reward; a wrong answer costs nothing,
and the quiz stays open. Brock is its regular, and Blue, Oak's grandson,
comes on his studious play style when Brock is elsewhere; Steven, who would
love the fossils, is aloof and never visits a public place. The
[specification](../specs/notable-haunts.md#worked-example-pewter-museum)
works it through.

A fifth, Viridian Forest, is remote, Bug- and Grass-themed, where trainers
study and sightsee among the trees. Its quest is Catch me one: "A wild
Pikachu lives around here. Catch one and show me?" Pikachu comes from the
forest's own grass, about one encounter in four; the player catches one,
shows it from the party or the boxes, and keeps it, and the trainer pays
their next reward. Bugsy, a Johto traveller on a Bug-hunting trip, is its
natural regular, as the HNS cameo already has it, and Erika takes it on her
Grass aces when Bugsy is elsewhere. The
[specification](../specs/notable-haunts.md#worked-example-viridian-forest)
works it through. With it, the five worked haunts cover the first five
quest types; Bring me, Swap battle, and Trade have no worked haunt yet.

Trade is the first slice of the parked
[trainer roster influence](trainer-roster-influence.md) design. The trainer
offers the filler that leads their battle order, "I'd trade OMANYTE for one of
yours. Interested?", and accepts a POKéMON that shares a type with their aces
when the best final form its line can reach has a base stat total at least
as high as the filler's line. A refusal costs nothing. An accepted trade
plays the usual in-game trade scene, and the player gets the filler at the
level of the POKéMON they gave. That POKéMON keeps its nickname, shininess,
nature, IVs, and ability in the trainer's battles, uses the trainer's moves
and levels, evolves as they grow, and never appears weaker than when it was
traded. A later Trade quest with the same trainer swaps the two back. The
[specification](../specs/notable-haunts.md#trade) works out the rules and
the save cost.

## Boundaries

In: the haunt pool and tags, placement, momentum, the talk flow (greetings
by friendship stage, the quest proposal, and the quirk), the eight quest
types, including Trade and the traded slot it leaves on a trainer's team,
quest rewards from each trainer's reward pool, the voice-bit writing rule,
buddy and reward pool as trainer values, the Kanto haunt list, retiring the
HNS cameos and the Dojo rematch hub, and the saved state for all of this.

Unchanged: every battle with a notable trainer uses their current TR and team
([Notable trainers](notable-trainers.md)), which reads a traded slot; phone
numbers are given as [Sevii Masters](sevii-masters.md#design) describes, at
Friend; league lineups belong to [Leagues](leagues.md).

Out of scope for v0: haunt lists for Johto, Hoenn, and Sevii; two trainers in
one haunt; authored trade offers, item swaps, and gifts at haunts (and as
friendship sources), beyond the one item a Bring me quest asks for and the
generic Trade quest; Tate & Liza at haunts, since the duo gives no number and
could never become a friend.

## Balance

Informational for now. Haunts offer no battles of their own, so they give no TR,
no Battle Points, and no prize money beyond a quest's fallback. The Dojo
rematches and their Battle Points are gone. The placement weights, the elite
gate (player TR 80), the momentum window, and the reward pools and their gates
are placeholders to tune in playtesting. With only the Kanto list in v0, every
traveller from Johto and Hoenn is placed in Kanto, and trainers from those
regions who don't travel have no haunt yet.

## Presentation

The placed trainer stands at the haunt's spot with their buddy beside them.
Talking to them runs straight through: a greeting for their friendship
stage, the quest proposal with a yes or no (or a quirk once the quest is
done), and a goodbye, with no menu. Dialogue is spliced from haunt lines and
the trainer's voice bits.

## Interactions

- **Phone numbers.** A trainer who becomes a Friend, through a win in any
  battle elsewhere or through quests at haunts, gives their number
  ([Sevii Masters](sevii-masters.md#design)). Asking them to be the Masters
  partner happens by phone, never at a haunt.
- **Leagues.** A trainer in an accepted league lineup leaves their haunt
  until the event ends.
- **Existing cameos.** The HNS cameos, one-off meetings that send the player
  to the Saffron Dojo for a rematch, and the Dojo's rematch room are replaced
  by haunts. Today several Johto cameos never appear, because nothing
  reveals them, which also locks away those leaders' Dojo rematches and
  Jasmine's trade ([cameo bug](../specs/notable-haunts.md#cameos-and-the-dojo)).
  Haunts host no rematches, so the Dojo's rematches have no replacement
  until rematch spots come ([Later](#later)).

## Open risks

- Walk with me needs the engine's NPC followers, which are switched off and
  cost save space when enabled, and they may clash with the overworld
  POKéMON that already follow the player. The
  [travel proof of concept](../research/notable-trainer-travel-poc.md) switched those POKéMON off to run.
- Trades need saved space for the player's POKéMON on trainers' teams. The
  main save blocks have little room left, so the
  [specification](../specs/notable-haunts.md#saved-state) puts sixteen
  records in the PC storage's spare bytes, which caps traded POKéMON at
  sixteen across all trainers.

## Specifications

- [Notable haunts specification](../specs/notable-haunts.md): haunt tags,
  trainer values, placement and momentum, the talk flow, quests and
  rewards, dialogue assembly, the Kanto list, engine notes, saved state,
  load validation, and acceptance.

## Later

- Situations beyond one trainer and one quest: "The coach", two trainers in
  one haunt, "Show me", services, and challenge battles.
- Item swaps, and gifts from a trainer's roster.
- Trade value overrides for outliers such as Shuckle, only if playtest shows
  the plain value check being abused.
- Haunts behind access conditions, such as a key item or a story beat.
- More ways to raise friendship: gifts and tag battles.
- Rematches at haunts or at dedicated rematch spots.
- Gossip at haunts: a friend telling the player where another trainer is.
- Trainers who walk to their haunts along routines instead of being
  placed, with at most two or three in one place; a
  [travel proof of concept](../research/notable-trainer-travel-poc.md) showed on-map pathfinding and saved travel between
  maps are viable.
- Haunt lists for Johto, Hoenn, and Sevii.
- Explorer support: placements per world progress and the fit of each
  trainer.

## References

- [Notable trainers](notable-trainers.md)
- [Sevii Masters](sevii-masters.md)
- [Leagues](leagues.md)
- [Trainer AI](trainer-ai.md)
- [Trainer roster influence](trainer-roster-influence.md)
- [Notable trainer voice bits](../research/notable-trainer-voices.md)
- [Notable trainer travel proof of concept](../research/notable-trainer-travel-poc.md)

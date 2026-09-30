# Notable haunts

Implemented: No Specification: [Notable haunts
specification](../specs/notable-haunts.md) Design status: v0 draft for review:
notable trainers spend their time between Gyms and leagues at **haunts**, one
shared pool of overworld spots that names no trainer. Placement fills each
haunt with the best-fitting free trainer, recomputed whenever world progress
changes, with no seeds. The player meets a trainer by their **friendship**: a
stranger, then met, a friend, and close; quests open once they have met, and
friends offer gossip and a rematch. A trainer gives the haunt's one **quest**.
Haunt lines describe only the place and activity, and each trainer's **voice
bits** supply the personality. v0 authors the Kanto list; weights, gates, and
reward pools are placeholders. Terms follow the
[glossary](player-trainer-rating.md#glossary).

## Intent

Notable trainers should feel like people who live in the world, not
statues waiting in their Gyms. Between their Gym and league battles, the
player should bump into them out in the overworld: Brock resting in a cave,
Lt. Surge watching the harbour, a Hoenn traveller passing through Kanto.
Meetings should emerge rather than be scripted one by one: a small pool of
places, each able to host many trainers, filled by who fits best at this
point in the journey. Every meeting should sound like that trainer, and a
friend should always have something to do with the player.

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
- its **activity**: training, study, leisure, worship, or sightseeing;
- its **setting**: public, such as a harbour, or remote, such as a cave; and
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

How a meeting goes depends on the trainer's **friendship** with the player
([Notable trainers](notable-trainers.md#friendship)), which only grows:

- **Stranger:** the player has never talked to them. They introduce
  themselves, or greet the player as someone they have heard of when the
  player has a reputation with them (a badge from their home region, a reign
  at any league, or being a Master), and offer an optional battle; beat them
  and they give their number.
- **Met:** they remember the player and greet them again. Quests are open,
  so the player can help them and build the friendship.
- **Friend:** they greet the player warmly, share a piece of gossip, offer a
  rematch at their current strength, and give the haunt's quest. They have
  given their number.
- **Close:** the same, with a warmer greeting.

A trainer whose first fight belongs to their Gym or league won't battle the
player until that fight is won, at any stage. The player can revisit a haunt
as often as they like while the trainer is placed there.

### Quests

Each haunt has one quest, which a trainer gives once they have met the
player:

| Quest | What happens |
| --- | --- |
| Walk with me | The trainer follows the player to the haunt's exit; wild POKéMON on the way are tag battles beside them. |
| Lost something | Something valuable went missing at the haunt; the player finds it. |
| Catch me one | The player hands the trainer a POKéMON that lives at the haunt. |
| Quiz | The trainer quizzes the player on type matchups for their aces. |
| One on one | One of the player's POKéMON against the trainer's lead ace. |

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
the trainer pays prize money instead, as much as a rematch win. A trainer
gives one reward while they are placed at a haunt; when someone else takes
the haunt, its quest can be done again. The starting pools are in
[notable trainer reward pools](../research/notable-trainer-rewards.md).

### Voices

Haunt lines describe only the place and the activity. Personality comes from
each trainer's fifteen **voice bits**, short reusable lines for greeting,
asking, thanking, and so on
([voice bits](../research/notable-trainer-voices.md)). Lines can mention the
player, the reward item, the trainer's ace, their **buddy**, and a piece of
gossip. A trainer's buddy is their companion POKéMON, one slot of their
roster, shown at its current stage: Brock's buddy is Onix early in the
journey and Steelix later.

## Sample playthrough

Diglett's Cave is a Kanto haunt with a Ground theme, remote, where trainers
go sightseeing; its quest is Walk with me, from the Route 2 entrance to the
Vermilion exit. Any Kanto trainer or non-aloof traveller can be placed
there; Brock and Giovanni fit it best.

1. With one badge, the player finds Giovanni there. The player has a
   reputation with him (a Kanto badge) but hasn't beaten him, so he greets
   them as someone he has heard of and sends the player to his Gym.
2. With three badges, the order has shifted and Brock takes the cave. The
   player has beaten him at Pewter, so he is a friend. He greets them,
   passes on gossip about where Misty is, and asks them to walk the tunnel
   with him. On the way, pairs of wild Diglett are tag battles with Brock
   at the player's side. At the Vermilion exit he thanks them with Pewter
   Crunchies, the first entry of his reward pool.
3. The player comes back: Brock is still there, but his reward is claimed,
   so he chats and offers a rematch.
4. After the next badge, Brock and Giovanni are both placed elsewhere
   before the cave's turn, and Will, a Johto traveller the player has never
   met, turns up. He introduces himself and offers a battle; the player
   wins and gets his number. Now he is a Friend, and the walk can be done
   again with him, paying the first entry of Will's own pool. Brock's next
   quest, wherever he turns up, will pay his second entry, a Hard Stone.

## Boundaries

In: the haunt pool and tags, placement, momentum, the relationship beat
(friendship stages), the five quest types, quest
rewards from each trainer's reward pool, the voice-bit writing rule,
buddy and reward pool as trainer values, the Kanto haunt list,
retiring the HNS cameos and the Dojo rematch hub, and the saved state for
all of this.

Unchanged: every battle with a notable trainer uses their current TR and team
([Notable trainers](notable-trainers.md)); phone numbers are given as [Sevii
Masters](sevii-masters.md#design) describes, at Friend; league lineups belong
to [Leagues](leagues.md).

Out of scope for v0: haunt lists for Johto, Hoenn, and Sevii; two trainers in
one haunt; trades, item swaps, and gifts at haunts (and as friendship
sources); Tate & Liza at haunts, since the duo gives no number and could never
become a friend.

## Balance

Informational for now. Rematches pay prize money like any notable battle,
and haunt battles give no TR. The Dojo rematches' Battle Points are gone.
The placement weights, the elite gate (player TR 80), the momentum window,
and the reward pools and their gates are placeholders to tune in
playtesting. With only the Kanto
list in v0, every traveller from Johto and Hoenn is placed in Kanto, and
trainers from those regions who don't travel have no haunt yet.

## Presentation

The placed trainer stands at the haunt's spot with their buddy beside them.
Talking to them runs the meeting for their friendship stage; a trainer who has
met the player offers a short menu of battle, quest, and chat, plus teaming up
for the Masters once the player is a Master and they are a Friend. Dialogue is
spliced from haunt lines and the trainer's voice bits.

## Interactions

- **Phone numbers.** A win over a stranger at a haunt makes them a Friend
  and gives their number ([Sevii Masters](sevii-masters.md#design)). A Friend
  at a haunt can also be asked to be the Masters partner in person, with the
  same effect as asking by phone.
- **Leagues.** A trainer in an accepted league lineup leaves their haunt
  until the event ends.
- **Existing cameos.** The HNS cameos, one-off meetings that send the player
  to the Saffron Dojo for a rematch, and the Dojo's rematch room are replaced
  by haunts. Today several Johto cameos never appear, because nothing
  reveals them, which also locks away those leaders' Dojo rematches and
  Jasmine's trade ([cameo bug](../specs/notable-haunts.md#cameos-and-the-dojo)).

## Open risks

- Walk with me needs the engine's NPC followers, which are switched off and
  cost save space when enabled, and they may clash with the overworld
  POKéMON that already follow the player.

## Specifications

- [Notable haunts specification](../specs/notable-haunts.md): haunt tags,
  trainer values, placement and momentum, the relationship beat, quests and
  rewards, dialogue assembly, the Kanto list, engine notes, saved state,
  load validation, and acceptance.

## Later

- Situations beyond one trainer and one quest: "The coach", two trainers in
  one haunt, "Show me", services, and challenge battles.
- Trades at haunts under trade fairness rules, item swaps, and gifts from a
  trainer's roster.
- Haunts behind access conditions, such as a key item or a story beat.
- More ways to raise friendship: gifts, tag battles, trades.
- Haunt lists for Johto, Hoenn, and Sevii.
- Explorer support: placements per world progress and the fit of each
  trainer.

## References

- [Notable trainers](notable-trainers.md)
- [Sevii Masters](sevii-masters.md)
- [Leagues](leagues.md)
- [Trainer AI](trainer-ai.md)
- [Notable trainer voice bits](../research/notable-trainer-voices.md)

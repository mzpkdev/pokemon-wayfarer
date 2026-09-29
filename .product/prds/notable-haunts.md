# Notable haunts

Implemented: No
Specification: [Notable haunts specification](../specs/notable-haunts.md)
Design status: v0 draft for review: notable trainers spend their time between
Gyms and leagues at **haunts**, one shared pool of overworld spots that names
no trainer. Placement fills each haunt with the best-fitting free trainer,
recomputed whenever world progress changes, with no seeds. The player meets
a trainer as a **stranger**, as someone **famous**, or as a **friend** whose
number they hold; friends offer gossip, a rematch, and the haunt's one
**quest**. Haunt lines describe only the place and activity, and each
trainer's **voice bits** supply the personality. v0 authors the Kanto list;
weights, gates, and rewards are placeholders. Terms follow the
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
- **Manner.** The haunt's quest must suit the trainer's manner.

Among the trainers who pass, preferences rank the fit:

- **Theme.** A haunt whose theme matches a trainer's aces draws them.
- **Activity.** A trainer's play style leans them towards some activities,
  and so does their **momentum**: a trainer who is rising, getting stronger
  fast right now, goes training, while a settled one relaxes or sightsees.
- **Hometown.** A Gym Leader likes the haunts of their own city.
- **Setting.** Warm trainers lean towards public places, cold ones towards
  remote ones.

Each haunt takes the best-fitting free trainer, and the order in which haunts
choose shifts with every step of world progress, so the same trainers turn
up in different places over a journey. Trainers are not placed while they
are in an accepted league lineup, or while they are the player's
[Masters partner](sevii-masters.md#design).

### Meeting a trainer

How a meeting goes depends on what the trainer is to the player:

- **Stranger:** a trainer from another region whose number the player
  doesn't have. They introduce themselves and offer an optional battle; beat
  them and they give their number.
- **Famous:** a trainer from this region whose number the player doesn't
  have. They greet the player but won't battle yet: the first fight belongs
  to their Gym or league.
- **Friend:** a trainer whose number the player has. They share a piece of
  gossip, offer a rematch at their current strength, and give the haunt's
  quest.

A trainer remembers meeting the player, so a stranger or famous trainer met
before skips the introduction. The player can revisit a haunt as often as
they like while the trainer is placed there.

### Quests

Each haunt has one quest, which only friends give:

| Quest | What happens | Warm | Proud | Cold |
| --- | --- | --- | --- | --- |
| Walk with me | The trainer follows the player to the haunt's exit; wild POKéMON on the way are tag battles beside them. | ✅ | ✅ | ✅ |
| Lost something | The trainer lost something at the haunt; the player finds it. | ✅ | ❌ | ❌ |
| Catch me one | The trainer wants a POKéMON that lives at the haunt. | ✅ | ✅ | ❌ |
| Quiz | The trainer quizzes the player on type matchups for their aces. | ✅ | ✅ | ✅ |
| One on one | One of the player's POKéMON against the trainer's lead ace. | ✅ | ✅ | ✅ |

**Manner** is a trainer value that says which quests they give, not how
they sound: warm trainers ask for help, proud ones challenge or enlist the
player, and cold ones command. A warm trainer can admit they lost something;
a cold one would never ask the player to fetch a POKéMON.

The reward comes from the trainer: an item for their signature POKéMON's
type, or a lesson that teaches one of the moves they like. A friend gives
the reward once while they are placed at that haunt; when someone else takes
the haunt, its quest can be done again.

### Voices

Haunt lines describe only the place and the activity. Personality comes from
each trainer's twelve **voice bits**, short reusable lines for greeting,
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

1. With one badge, the player finds Giovanni there. He is famous and
   unbeaten, so he introduces himself and sends the player to his Gym.
2. With three badges, the order has shifted and Brock takes the cave. The
   player has beaten him at Pewter, so he is a friend. He greets them,
   passes on gossip about where Misty is, and asks them to walk the tunnel
   with him. On the way, pairs of wild Diglett are tag battles with Brock
   at the player's side. At the Vermilion exit he thanks them with a Metal
   Coat.
3. The player comes back: Brock is still there, but his reward is claimed,
   so he chats and offers a rematch.
4. After the next badge, Brock and Giovanni are both placed elsewhere
   before the cave's turn, and Will, a Johto traveller the player has never
   met, turns up. He introduces himself and offers a battle; the player
   wins and gets his number. Now he is a friend, and the walk can be done
   again with him.

## Boundaries

In: the haunt pool and tags, placement, momentum, the relationship beat
(stranger, famous, friend), the five quest types and their manners, quest
rewards, the voice-bit writing rule, manner and buddy as trainer values, the
Kanto haunt list, retiring the HNS cameos and the Dojo rematch hub, and the
saved state for all of this.

Unchanged: every battle with a notable trainer uses their current TR and
team ([Notable trainers](notable-trainers.md)); phone numbers are given as
[Sevii Masters](sevii-masters.md#design) describes; league lineups belong to
[Leagues](leagues.md).

Out of scope for v0: haunt lists for Johto, Hoenn, and Sevii; two trainers
in one haunt; trades, item swaps, and gifts at haunts; friendship scores;
Tate & Liza at haunts, since the duo gives no number and could never become
a friend.

## Balance

Informational for now. Rematches pay prize money like any notable battle,
and haunt battles give no TR. The Dojo rematches' Battle Points are gone.
The placement weights, the elite gate (player TR 80), the momentum window,
and the rewards are placeholders to tune in playtesting. With only the Kanto
list in v0, every traveller from Johto and Hoenn is placed in Kanto, and
trainers from those regions who don't travel have no haunt yet.

## Presentation

The placed trainer stands at the haunt's spot with their buddy beside them.
Talking to them runs the meeting for what they are to the player; a friend
offers a short menu of battle, quest, and chat, plus teaming up for the
Masters once the player is a Master. Dialogue is spliced from haunt lines
and the trainer's voice bits.

## Interactions

- **Phone numbers.** A win over a stranger at a haunt gives their number
  like any first win ([Sevii Masters](sevii-masters.md#design)). A friend at
  a haunt can also be asked to be the Masters partner in person, with the
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
- A friendship score with each trainer, raised by quests and shared play.
- Haunt lists for Johto, Hoenn, and Sevii.
- Explorer support: placements per world progress and the fit of each
  trainer.

## References

- [Notable trainers](notable-trainers.md)
- [Sevii Masters](sevii-masters.md)
- [Leagues](leagues.md)
- [Trainer AI](trainer-ai.md)
- [Notable trainer voice bits](../research/notable-trainer-voices.md)

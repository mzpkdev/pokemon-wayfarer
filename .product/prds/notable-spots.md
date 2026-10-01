# Notable spots

Implemented: No

Specification: [Notable spots specification](../specs/notable-spots.md)

Design status: the everyday layer of the future **routine and travel**
design. Spots switch on together with routines and travel; until then,
nothing changes and v0 haunt placement works exactly as the
[haunts PRD](notable-haunts.md) describes. Terms follow the
[glossary](player-trainer-rating.md#glossary).

## Intent

Haunts are the highlights: a handful of authored places, each with a quest.
But a trainer who only ever stands at a highlight still feels placed rather
than alive. Once trainers walk their own routines, they need somewhere to be
the rest of the time. **Spots** are those everyday places: the Pokémon Center
counter, a Mart shelf, a slot machine, a patch of tall grass, the edge of a
pond. Running into Lt. Surge buying Potions, or Misty waiting for a bite at
the water's edge, should make the world feel lived in, at almost no authoring
cost.

## Design

### Spots and haunts

A **spot** is an everyday place where a notable trainer can be found doing
something ordinary. Spots differ from haunts in three ways:

- **Found, not authored.** Spots are found automatically from map data:
  doors, tile types, and the people already standing on each map. Nobody
  writes a spot list by hand, apart from a few benches and lookouts and a
  short list of **named spots**: one-off places with character, such as
  Oak's Lab, the Olivine Café, or Mt. Pyre's summit.
- **One pool for every region.** Kanto, Johto, Hoenn, and Sevii all have
  spots from the start, wherever the maps have Centers, Marts, grass, and
  water.
- **No quests.** A spot is somewhere to say hello. Quests stay at haunts.

Trainers spend most of their time at spots; haunts are where the memorable
meetings happen.

### Spot kinds

Every spot has a **kind**, which says how it's found and what the trainer
visibly does there. Trainers only do things the overworld can already show:
walking to a tile, facing something, a small emote bubble, and going through
doors.

| Kind | Found from | What the trainer does |
| --- | --- | --- |
| Pokémon Center | doors that lead into a Center | Stands at the counter, or sits at the side. |
| Poké Mart and department store | Mart and store doors, and store floors | Faces a shelf; moves between store floors. |
| Game Corner | its door | Stands at a slot machine. |
| Other Gyms | Gym doors | Is "just leaving" (below). |
| Tall grass | tall-grass tiles | Roams the patch, with the odd "!". |
| Water's edge | land tiles beside fishable water | Faces the water in a "fishing" pose, with the odd "!". |
| Town squares and benches | open town areas, plus a few authored benches and lookouts | Idles and looks around. |
| Chatting with an NPC | a person already standing on the map | Stands beside them, facing them, with the odd "…". |
| Named spot | a short authored list | Stands at the place, doing what it's for: studying at a lab, relaxing at a café, sightseeing at a tower. |

**Just leaving.** A trainer visiting another Gym is never found standing
inside it. When the player walks into a Gym a visitor is at, the visitor
appears near the exit and walks out. It's a brief "oh, it's you" moment,
and the player can still stop them for a word. A Gym's own leader is never
a visitor there. A trainer can visit a Gym whose leader the player hasn't
beaten yet, and the visitor never stands in the way of the badge battle.

**Behaviour.** Each kind of spot behaves the same way everywhere: a
handful of templates (stand and face, wander in an area, browse the
shelves, just leaving, sit or idle) cover every kind. The spot finder only
says where a spot is and which tiles it needs; nobody scripts what a
trainer does at each detected spot, and a named spot picks its template
from its activity unless its author chooses otherwise
([behaviour templates](../specs/notable-spots.md#behaviour-templates)).

### Who goes where

Each trainer's routine (future work) moves them through **activities**.
The activity picks a spot kind, and the kind picks a spot:

| Activity | Spot kinds |
| --- | --- |
| care | Pokémon Center |
| shop | Poké Mart, department store |
| gamble | Game Corner |
| train | tall grass (or a training haunt) |
| relax | town squares and benches, water's edge (or a waterside haunt) |
| fish | water's edge |
| visit | other Gyms, chatting with an NPC |

The activities are the ones haunts already use, plus two new ones: **fish**
and **visit**. Every activity can also use the named spots that list it, so
study, sightseeing, and lying low have everyday places too. A trainer
picks a spot near their home base, or anywhere further afield if they are
a traveller. A Gym Leader's home base is their
Gym city; everyone else has one home map from canon, such as Pallet Town
for Blue or Mossdeep City for Steven
([home base](../specs/notable-spots.md#home-base)). How far "near" reaches
is still a placeholder. The same situation always gives the
same choice; nothing is random.

Places have room for only a few notables at a time. An indoor map, such as
a Center or a Mart, holds one. An outdoor map holds two or three in all,
counting both spots and haunts. A trainer never goes to a full place; they
pick the next one instead.

**Favourites.** A trainer has up to three favourite spots for a signature
habit: Lt. Surge at the Celadon Game Corner, or Erika on the Celadon
department store's gift floor. A favourite wins whenever it has room;
otherwise the trainer picks as usual. Favourites are authored with the
routines in [notable trainer routines](../research/notable-trainer-routines.md).

### Meeting a trainer at a spot

Talking to a trainer at a spot is short:

1. **A greeting**, by friendship, exactly as at a haunt
   ([haunts](notable-haunts.md#meeting-a-trainer)). Meeting a trainer for
   the first time at a spot makes them Met, just as at a haunt.
2. **What they are doing**, one line shared by every trainer at that kind
   of spot, such as "Just stocking up on supplies." or "Taking a breather."
   If a follow-up from an earlier quest is waiting, it plays here instead:
   handing over a Courier parcel, a check-in about an Egg sitting egg, or a
   Courier sender's news of where the recipient was last seen.
3. **A goodbye.**

There are no quests, battles, or menus at spots.

## Boundaries

In: the spot kinds, how each is found from map data, the named spots, the
per-map room for notables, how an activity picks a spot, the activity line
per spot kind, follow-ups at spots, and the "just leaving" Gym visit.

Unchanged: v0 haunt placement, haunt quests, and the haunt talk flow; Gym
battles and badges; every battle with a notable trainer
([Notable trainers](notable-trainers.md)).

Out of scope: the routines themselves (when a trainer does which activity)
and travel between maps, which the
[world simulation](notable-world-simulation.md) owns. Quests, battles, rewards, and gifts at spots. Spots are the
destinations of the [world simulation](notable-world-simulation.md).

## Presentation

A trainer at a spot stands on an ordinary tile and acts out the activity
with what the overworld already has: facing, walking a few steps, an emote
bubble, going through a door. There is no new animation, and no sitting
sprite: "sitting at the side" of a Center is a trainer standing still
beside a seat. Talking to them runs straight through with no menu.

## Interactions

- **Haunts.** Spots and haunts share the same room on a map. A trainer at a
  haunt is placed there; a trainer at a spot is on their routine. Haunt
  quests are unaffected.
- **Egg sitting and Courier.** A trainer with an egg out asks after it at a
  spot as they would at a haunt, and a Courier recipient takes the parcel
  wherever the player finds them
  ([talk flow](../specs/notable-haunts.md#talk-flow)). Once routines run,
  the recipient can be out at a spot, and the sender names the town or
  route they were last seen in, such as "CERULEAN CITY".
- **Gyms.** A visitor walks out as the player walks in, and never blocks
  the Gym battle or its trainers.
- **Following Pokémon.** The Pokémon that walks behind the player competes
  with notables for the same limited overworld slots
  ([open risk](../specs/notable-haunts.md#open-risk-overworld-followers)).

## Constraints

- Spots add little or no saved data beyond what routines and travel will
  save: which spot a trainer is at is worked out, not stored.
- Few notables per map, because the overworld has room for only 16 people
  at once and each walker costs processing time
  ([travel proof of concept](../research/notable-trainer-travel-poc.md)).

## Playtesting

- Do spots make trainers feel present, or does meeting them too often make
  them feel cheap?
- Is two or three notables on one outdoor map lively or crowded?
- Is the one shared line per spot kind enough, or does it get repetitive?
- Does "just leaving" read as a visit, or as a bug?

## Specifications

- [Notable spots specification](../specs/notable-spots.md): spot kinds and
  their detection sources, capacity, spot choice, the talk flow, engine
  notes, and saved state.

## Later

- More found kinds, such as libraries, if the named-spot list grows too
  long to write by hand.
- Per-trainer activity lines, if the shared ones get stale.
- Trainers whose pace at a spot follows their play style or momentum, such
  as a rising trainer moving faster.

## References

- [Notable haunts](notable-haunts.md)
- [Notable trainers](notable-trainers.md)
- [Notable trainer travel proof of concept](../research/notable-trainer-travel-poc.md)
- [Travel and on-map walking](../specs/notable-haunts.md#travel-and-on-map-walking)
- [Notable spots inventory (draft)](../research/notable-spots-inventory.md)
- [Notable named spots (starting list)](../research/notable-named-spots.md)

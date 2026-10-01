# Notable haunts

PRD: [Notable haunts](../prds/notable-haunts.md)
Implemented: No
Design status: v0 draft for review: one shared pool of **haunts**, each
tagged by region, theme types, elite gate, hometown, up to two activities,
setting, capacity, and one **quest**, and naming no trainer. **Placement**
is a pure function of world progress and a few saved facts, with no seeds:
hard qualifiers (region or traveller, aloof only at remote elite haunts)
pick candidates, a placeholder score (theme from aces, activity from play
style and **momentum**, hometown) ranks them, and haunts fill one at a time
in an order that rotates with world progress. A haunt offers only its own
quest: no battle offers, no rematches, and no menu. Every
talk runs a greeting by the trainer's **friendship stage** (Stranger, Met,
Friend, Close), then a follow-up (a Courier delivery, an Egg sitting
follow-up while the trainer's egg is outstanding, or a Courier sender's
news of the recipient), or else the quest proposal while the quest is open
or the trainer's quirk once it is done, then a farewell; a quest pays once
per placement.
The reward comes from the trainer's own **reward pool**, never from the quest
type. Dialogue splices favour-free haunt lines with each trainer's
**voice bits**. Weights, gates, momentum values, and reward pools are
placeholders; balance is informational.

## Scope

Own, in `IS_WAYFARER`: the haunt catalog and its tags, the meaning of the
**buddy** and **reward pool** trainer values, placement and
momentum, the talk flow, the twelve quest types (with the traded slots of
Trade and Wanted, Egg sitting's outstanding eggs, Courier's parcel, and
Handicap's one attempt per trainer),
rewards and claims, dialogue assembly, the Kanto haunt list, retiring the
HNS cameos and the Saffron Dojo rematch room, the haunts' saved state and
load validation, presentation, the balance report, and acceptance.

- [Notable trainers](notable-trainers.md) owns the trainers: inventory,
  home region, the traveller and aloof traits, TR and its growth, rosters,
  the downward rule's use, move pools, and the battle snapshot. It also holds
  the buddy and reward pool data in the catalog; this spec owns
  what they mean, and the rules of a [traded slot](#trade).
- [Trainer roster influence](trainer-roster-influence.md) is the parked
  design that Trade starts; it stays parked beyond this v0 slice.
- [Notable spots](notable-spots.md) is the everyday layer of the future
  routine and travel design: places found from map data, with no quests,
  where trainers spend most of their time; it switches on with routines and
  travel and leaves v0 placement unchanged.
- [Sevii Masters](sevii-masters.md) owns phone contacts (a trainer at Friend
  or above), asking a partner by phone, the partner choice, and the runtime
  partner slot.
- [Leagues](leagues.md) owns invitations and the accepted event lineup.
- [Trainer AI](trainer-ai.md) owns the AI flags of every battle here.
- [Gym Leader scaling](gym-leader-scaling.md) owns battle construction for
  every notable battle, including a walk's partner and One on one's lead
  ace.
- The voice bits themselves are content in
  [notable trainer voice bits](../research/notable-trainer-voices.md), and
  the v0 reward pools in
  [notable trainer reward pools](../research/notable-trainer-rewards.md).

## Haunts

A **haunt** is one authored entry in the haunt catalog: a standing tile on
one map, plus the maps its quest uses. The catalog is an ordered list (the
**catalog order**) across all regions; v0 authors only [Kanto](#kanto-haunts).
No haunt names or prefers a specific trainer. Each haunt authors:

| Tag | Values | Meaning |
| --- | --- | --- |
| Region | `kanto`, `johto`, `hoenn` | The haunt's region. |
| Theme types | zero to three types | Draws trainers whose aces share a type. |
| Elite | none, or a gate TR | An elite haunt is active only while world progress is at least its gate. |
| Hometown | none, or a Gym city | Draws the Gym Leader of that city. |
| Activities | one or two [activities](#activities) | What trainers do there. |
| Setting | `public`, `remote` | Public places are busy; remote ones are out of the way. |
| Capacity | a whole number, 1 by default | How many trainers the haunt holds at once. |
| Name | display text, at most 20 characters | What `{PLACE}` says for the haunt, such as "CERULEAN CAPE" ([Courier](#courier)). |
| Quest | one [quest type](#quests) and its details | The quest a trainer gives there. |
| Standing tile | map and tile | Where the placed trainer stands. |

### Activities

A haunt carries one or two activities from one shared list: **train**,
**care**, **study**, **home**, **relax**, **gamble**, **shop**, **lie low**,
and **sightsee**. The list is closed; a haunt never invents its own. Play
styles and momentum point at the same words ([score](#score)).

### Capacity

Capacity is 1 unless a haunt authors more. Every v0 haunt keeps 1, so
placement holds at most one trainer per haunt and the talk flow assumes one.
What a capacity above 1 does waits for the routine design.

### Active haunts

A non-elite haunt is always active. Elite gates are placeholders; every
Kanto elite haunt uses TR 80, the league qualification TR. An inactive haunt
takes part in nothing: no placement, no meeting, no quest. A haunt is not a
[location](notable-trainers.md#home-region-and-travel): it reads no
willingness, travel cost, or fatigue.

## Trainer values

Each notable trainer entry authors two values for haunts, alongside its
other catalog content
([Notable trainers](notable-trainers.md#haunt-values-buddy-reward-pool)):

- **Buddy:** one roster slot number (1-6), the trainer's companion: their
  anime companion where there is one, otherwise a later-game companion or
  their iconic ace. `{BUDDY}` resolves to that slot's species after the
  [downward rule](player-trainer-rating.md#evolution-stages) at the slot's
  member level (`teamLevel(tr) + levelOffset`, clamped to 1-100) at the
  current world progress. It resolves the same way whether or not the slot
  has joined the trainer's team yet: the buddy travels with the trainer even
  when it doesn't battle.
- **Reward pool:** the ordered list of what the trainer gives for finished
  quests ([rewards and claims](#rewards-and-claims)).

The v0 buddies are in the
[voice bits](../research/notable-trainer-voices.md) (the `Buddy:` lines), and
the v0 pools are in
[notable trainer reward pools](../research/notable-trainer-rewards.md); both
are to be copied into the catalog. Tate & Liza author both like
everyone else, though the duo is never placed in v0.

A trainer's **hometown** is not authored: it is the city of their badge
encounter ([coverage](gym-leader-scaling.md#coverage-and-identity)), so only
Gym Leaders have one. Kanto: Brock Pewter, Misty Cerulean, Lt. Surge
Vermilion, Erika Celadon, Janine Fuchsia, Sabrina Saffron, Blaine Cinnabar,
Giovanni Viridian.

## Placement

A **placement** maps each active haunt to at most one trainer (its
[capacity](#capacity), 1 in v0), and each trainer to at most one haunt. It
is a pure function of:

- world progress (`GetTrainerRating()`);
- the accepted league event, if any, and its
  [event lineup](leagues.md#event-lineup);
- the resolved [partner](sevii-masters.md#partner), once the player is a
  [Master](sevii-masters.md#master): the partner choice, or Lorelei without
  one; and
- content (the haunt catalog and the trainer catalog).

Friendship, claims, and history play no part, and nothing is random. Placement
is recomputed whenever any input changes (world progress rises, an event is
accepted or ends, a partner is asked, the player becomes a Master) and on
load. While a [walk](#walk-with-me) is in progress, the recompute waits until
the walk ends, so the walking trainer keeps their haunt.

### Candidates

A trainer is a **candidate** for an active haunt when every hard qualifier
holds:

1. **Placeable.** The trainer is not Tate & Liza (the duo gives no number,
   so it could never be a friend), is not in the accepted event lineup, and
   is not the resolved partner.
2. **Region.** The trainer's home region is the haunt's region, or the
   trainer is a traveller.
3. **Aloof.** An aloof trainer is a candidate only at an elite haunt whose
   setting is remote.

### Score

Each candidate gets a whole-number **score** for the haunt, the sum of these
parts. The weights are placeholders to tune:

| Part | Points | Condition |
| --- | ---: | --- |
| Theme (signature) | 4 | A type of the signature POKéMON's authored species is one of the haunt's theme types. |
| Theme (other ace) | 2 | Otherwise, a type of another ace's authored species is a theme type. |
| Activity (play style) | 2 | One of the haunt's activities is one of the play style's activities (table below). |
| Activity (momentum) | 1 | One of the haunt's activities is one of the momentum's activities. |
| Hometown | 3 | The haunt's hometown is the trainer's hometown. |

Types come from the authored (final-stage) species of the roster's ace
slots, so a trainer's themes never change with world progress: Brock's
aces are Steelix (Steel, Ground) and Aerodactyl (Rock, Flying).

| Play style | Activities |
| --- | --- |
| Gambler | gamble |
| Bomber | study |
| Sweeper | train |
| Field marshal | study, sightsee |
| Hexer | lie low |
| Turtle | relax |
| Brawler | train |
| Tactician | study |

| Momentum | Activities |
| --- | --- |
| Rising | train |
| Settled | relax, sightsee |

Each part counts once, however many of the haunt's activities match.

### Momentum

**Momentum** is `rising` or `settled`, derived only from the trainer's TR
at two points of world progress
([growth](notable-trainers.md#growth-with-world-progress)):

```text
recent   = TR(wp) - TR(max(0, wp - 20))
momentum = rising if recent >= 5, otherwise settled
```

The window (20) and threshold (5) are placeholders. Archetype names are
never read: momentum comes from the TR curve alone, so a Legend is always
settled, a Veteran settles at peak, and a Burst is rising for a while after
each jump. At world progress 0 everyone is settled.

### Fill

1. List the active haunts in catalog order, `h[0..n-1]`, and let
   `start = wp mod n`. The **fill order** is `h[start], h[start+1], …`,
   wrapping around, so it shifts with every step of world progress.
2. In fill order, each haunt takes its highest-scoring candidate who is not
   yet placed. Ties go to the trainer earliest in the trainer catalog order
   (the order of the
   [inventory](notable-trainers.md#notable-trainer-inventory)).
3. A haunt with no free candidate stays empty.

When the new placement differs from the saved one at a haunt (another
trainer, or empty), that haunt's claim bit is cleared
([rewards and claims](#rewards-and-claims)), and so is its
[lost-something](#lost-something) search state,
[Catch me one](#catch-me-one) asked bit, or [Trade](#accepted) or
[Wanted](#wanted) owed bit, and its Egg sitting
[follow-up bit](#the-follow-up). When the trainer changes at a haunt that
held or now holds an active parcel's sender or recipient, the parcel's
[trail bit](#refreshing-the-trail) clears too. Then the new placement is
saved.

## Talk flow

A haunt offers only its own quest. There are no battle offers, no rematches, no
team-up, and no menu at haunts. Talking to the placed trainer runs the talk
flow, written once for all haunts. It reads the trainer's [friendship
stage](notable-trainers.md#friendship) (Stranger, Met, Friend, or Close) when
the talk starts, and never the score or the events behind it.

Every talk runs three steps:

1. **Greeting**, picked by the stage:

   | Stage | Greeting |
   | --- | --- |
   | Stranger | `MEET`, or `HEARD` when the player's **fame** reaches them. |
   | Met | `AGAIN`. |
   | Friend | `HELLO`. |
   | Close | `CLOSE`. |

2. **Follow-up, proposal, or quirk.** A **follow-up** comes first: a
   quest from an earlier meeting, settled or recalled at this one. At most
   one plays per talk, it takes this step's place in that talk, and the
   next talk returns to the haunt's own quest. In order, the first that
   applies:
   1. **Courier delivery**, while the trainer is the recipient of the
      player's active parcel, at every talk until it is delivered
      ([delivery](#delivery)).
   2. **Egg sitting follow-up**, while the trainer has an outstanding egg
      from [Egg sitting](#egg-sitting): once per meeting, or whenever the
      hatchling is found ([follow-up](#the-follow-up)).
   3. **Courier trail**, while the trainer is the sender of the active
      parcel and its trail bit is clear
      ([refreshing the trail](#refreshing-the-trail)).

   A delivery comes first because it settles the one parcel and pays; an
   egg follow-up before the trail because it may pay, while the trail only
   informs. Each keeps its own bit, so one never uses up another: the one
   that waits plays at the next talk. Otherwise, while the haunt's
   claim bit is clear (the quest is still open during this placement), the
   [quest](#quests) proposal: `ASK`,
   the haunt's proposal line, and [YES / NO]. `YES` gives the trainer's
   `YES` and runs the quest as its section says; `NO` gives the trainer's
   `NO`, costs nothing, and the next talk proposes again. At a Lost
   something haunt whose keepsake the player has found, the completion
   takes the proposal's place ([Lost something](#lost-something)), and at
   a Catch me one haunt whose quest the player has accepted, the showing
   or `NOT_READY` does ([Catch me one](#catch-me-one)), and at a Trade
   or Wanted haunt whose reward is owed, the completion does
   ([Trade](#accepted)), and at a Wanted haunt with no fair offer, its
   no-offer line and `NOT_READY` do ([Wanted](#no-fair-offer)). At an
   Egg sitting haunt while the trainer's egg is outstanding or their eggs
   are used up, `QUIRK` does ([Egg sitting](#giving-the-egg)), and so it
   does at a Courier haunt while a parcel is active or no recipient
   exists ([Courier](#giving-the-parcel)). At a Handicap haunt, an owed
   reward's completion, the callback, or the gate's line and `QUIRK` does
   ([Handicap](#the-gate)).
   Once the
   claim bit is set (the quest was completed during this placement), the
   trainer's `QUIRK` instead.
3. **Farewell:** `BYE`. A talk that starts a walk ends at `YES` instead;
   the walk's own `BYE` comes at the exit ([walk](#walk-with-me)).

The first talk makes a Stranger Met (+1) at its start, after the greeting
is picked, so a first meeting introduces the trainer (`MEET` or `HEARD`)
**and** proposes the quest in the same talk. After the quest is completed
during this placement, every talk is the greeting, `QUIRK`, and `BYE`, until
the placement changes and the claim bit clears, except a talk a follow-up
takes.

The player's **fame** reaches a trainer when the player's TR is at least
`min(80, their TR − 10)`, or the player reigns as champion at any
[league](leagues.md) or is a [Master](sevii-masters.md#master). TR 80 is
where every league knows the player
([qualifying](leagues.md#qualification)), so from there every stranger has
heard of them; below it, a trainer notices the player once the player's TR
comes within 10 of their own, read at the moment of the meeting. Titles
count because league wins give no TR. Fame needs no saved data and only
changes a Stranger's greeting: `HEARD` is one line per trainer, with no
levels, since it plays at most once per trainer.

**Points.** A haunt adds [friendship](notable-trainers.md#friendship)
through two events only: the first talk (+1) and a completed quest (+10).
An Egg sitting quest completes at its hatched follow-up, not when the egg
is given, and a Courier quest at its delivery, which adds +10 with both
the sender and the recipient.
Repeat talks, declining, and quirks add nothing. Haunts offer no battles
of their own, so no battle win happens here: One on one, Swap battle, and
Handicap are quests, and their wins count as the completed quest, not as a
battle won. A Handicap loss counts as the completed quest too.
A trade or a trade-back counts as a completed quest too.

**Teaming up.** Asking a trainer to be the Masters partner happens only by
phone ([partner](sevii-masters.md#partner)), never at a haunt. A trainer who
becomes the partner stops being placed, so they leave their haunt.

## Quests

Each haunt has one quest type. Only a trainer at Met or above gives quests,
which every talk reaches, since the first talk makes the trainer Met before
the proposal ([talk flow](#talk-flow)); every trainer can give every quest
type. Each type has one proposal
line, written so it never implies the trainer needs help or is asking a
favour ([dialogue](#dialogue)), except Egg sitting's and Courier's, which
ask one by design:

| Quest | Proposal line |
| --- | --- |
| Walk with me | "Walk it with me, out to the VERMILION side?" (the haunt names its own destination) |
| Lost something | "Something valuable went missing around here, {HINT}. Find it?" |
| Catch me one | "A wild {LOCAL} lives around here. Catch one and show me?" |
| Quiz | "Three questions on type matchups. Think you can answer them?" |
| One on one | "Your best POKéMON against {ACE}. Up for it?" |
| Bring me | "Got a {KIND} on you? Bring me one." |
| Swap battle | "One POKéMON each. Pick your best?" (the swap is revealed only after the pick) |
| Trade | "I'd trade {FILLER} for one of yours. Interested?" (a [trade-back](#trade-back) has its own line) |
| Wanted | "I've been looking for a {WANTED}. Got one to trade?" (a [trade-back](#trade-back) takes its place, as at Trade) |
| Egg sitting | "Could you hold on to this egg for a while?" (a favour by design; its [follow-up](#the-follow-up) comes at a later meeting) |
| Courier | "Could you take this to {OTHER}? Last I heard, they were around {PLACE}." (a favour by design; the [delivery](#delivery) comes at a meeting with {OTHER}) |
| Handicap | "Your whole team against my {ACE}. Think that's enough?" (only while the trainer's TR is far above the player's, and one attempt per trainer, ever; [the gate](#the-gate)) |

A haunt may author its own proposal line for its quest type, under the same
writing rules; the [Celadon Game Corner](#worked-example-celadon-game-corner)
frames One on one as a bet. The quest type decides only the proposal. Every
quest pays the same way, from the trainer's
[reward pool](#rewards-and-claims).

Every quest runs `ASK` (an attention-getter), then the haunt's own quest
line (the proposal), then a yes or no:
`YES` starts it, `NO` ends it ([talk flow](#talk-flow)). A requirement the
player doesn't meet, or a failed attempt, uses `NOT_READY`, unless there is
a loss line (One on one's, if the haunt authors one, the Quiz's, and Swap
battle's). A trade the value check refuses plays "That's not a fair
trade." and the trainer's `NO` ([Trade](#the-value-check)), and a Wanted
haunt with no fair offer plays its no-offer line and `NOT_READY` in place
of the proposal ([Wanted](#no-fair-offer)). A Handicap whose gate isn't
met plays its own line and `QUIRK` instead of the proposal, and its loss
line is no failure: a Handicap pays on a loss too
([Handicap](#the-handicap-outcome)).
Completing it runs the haunt's done line if it has one, then `PRAISE`, then
the [reward](#rewards-and-claims), then `BYE`; a Handicap loss skips
`PRAISE`.

### Walk with me

The haunt authors a **start** (its standing tile), an **exit** (one warp
or map edge of its quest maps), and its **wrong-exit triggers** (every
other warp or map edge that leaves the quest maps).

- **Start.** `YES` makes the trainer a follower NPC who is also the
  player's battle partner ([follower NPCs](#follower-npcs)), with the
  follower flags that clear them on whiteout and forbid leaving by Fly,
  Teleport, or an Escape Rope. The player needs at least one able POKéMON;
  otherwise `NOT_READY`.
- **Wild battles.** While the walk is in progress, every wild encounter on
  the haunt's quest maps is a double wild battle against two wild POKéMON
  beside the trainer. The player fights with their first three able
  POKéMON (the engine's choice); the trainer brings their best three, the
  last three of their battle order, as a
  [Masters tag match](sevii-masters.md#tag-matches) does, from a battle
  snapshot built at each battle's start into the runtime partner slot.
  Regular trainer battles on the way are unchanged.
- **Complete.** Stepping through the exit with the trainer following ends
  the walk: the haunt's done line, `PRAISE`, the reward, and `BYE`. The
  follower leaves, and the trainer is back at their haunt.
- **Wrong exit.** While the walk is in progress, a trigger at each
  wrong-exit warp or map edge stops the player and shows the haunt line
  "Giving up on the walk?" with [YES / NO].
  - **YES:** the trainer's `NO` line. The walk ends with no reward and the
    claim bit still clear, the trainer returns to their standing tile, and the
    player leaves.
  - **NO:** the player is stepped back one tile and the walk continues.
- **Unfinished.** A whiteout or a reload ends the walk silently, and giving
  up at a wrong exit ends it as above: no reward and the claim bit still
  clear; the trainer is back at their haunt and the walk can be started
  again. Fly, Teleport, and an Escape Rope stay blocked by the follower
  flags. Each Walk with me haunt authors its wrong-exit triggers (Diglett's
  Cave: the north entrance's warp to Route 2).

### Lost something

The haunt authors two or three **lost spots** on its quest maps, in order.
Each spot is one tile, walkable or faced from a walkable tile, with a
**hint**: a short neutral phrase of a few words that follows "around here,"
("near the fence", "by the rocks"). Hints name only the place, so they read
true for every trainer.

**The active spot.** One spot per placement is active, picked from the
placed trainer: spot `i mod k` in authored order (counting from 0), where
`i` is the trainer's position in the trainer catalog order (the order that
breaks [fill](#fill) ties) and `k` is the haunt's spot count. It is a pure
function of the placement, with no seed and no saved data, and it holds for
the whole placement; world progress is not used because it rises while the
same trainer stays, and the spot would move mid-search. A trainer placed at
the same haunt again uses the same spot. `{HINT}` in the proposal resolves
to the active spot's hint.

- **Proposal.** "Something valuable went missing around here, {HINT}. Find
  it?" (the quest line after `ASK`).
- **YES:** the trainer's `YES`, then `BYE`. The active spot becomes
  searchable: an invisible scripted background event, found by facing its
  tile and pressing A, like a hidden item. It stays searchable while the
  claim bit is clear and nothing has been found during this placement,
  through leaving the map, whiteouts, and reloads, so the player can leave
  and come back. Unlike a hidden item, whose flag is set once for good, it
  is **re-armable per placement**: the next placement arms a spot again
  after its own `YES`. A talk while the search is open and nothing is found
  proposes again as usual; `YES` changes nothing, and `NO` doesn't close
  the search.
- **Finding it** plays the haunt system line "{PLAYER} found the lost
  keepsake!" and sets the haunt's **found** state. The keepsake is flavour,
  not a Bag item: it takes no room and cannot be used or lost.
- **Completing.** The next talk with the placed trainer is the greeting,
  then, in place of the proposal, the haunt's done line, `PRAISE`, the
  [reward](#rewards-and-claims) (an item through `GIFT`), and `BYE`: +10
  friendship as a completed quest. Giving the reward sets the claim bit and
  returns the search state to none; a reward that waits (a full Bag, a
  cancelled lesson) leaves the found state set, so the next talk completes
  it again.
- **NO:** the trainer's `NO`, then `BYE`; nothing is armed, and the next
  talk proposes again.
- **Expiry.** When the haunt's placement changes, its search and found
  state clear with its claim bit ([fill](#fill)), so an unfinished search
  expires at the next reshuffle.

**Item finder.** The build has one: `ITEM_DOWSING_MACHINE` (alias
`ITEM_ITEMFINDER`), a key item given in Ecruteak City (HNS), on Route 110
(Emerald), and in the Route 11 gate (FRLG). Its scan,
`ItemfinderCheckForHiddenItems` in
[item_use.c](../../game/src/item_use.c), only counts `BG_EVENT_HIDDEN_ITEM`
events whose flag is clear, so today it ignores a scripted background
event. Haunts extend that scan to also count the active lost spot while its
search is open and nothing is found, so the Dowsing Machine beeps near it
as it does near a hidden item.

### Catch me one

The haunt authors one species from its own wild table, the `{LOCAL}` of its
quest line. The player catches one and shows it; nothing is handed over or
traded, and the player keeps the POKéMON. The haunt keeps a saved **asked
bit** for this placement.

- **Proposal.** "A wild {LOCAL} lives around here. Catch one and show me?"
  (the quest line after `ASK`).
- **YES:** the trainer's `YES`, then `BYE`, and the asked bit is set.
- **NO:** the trainer's `NO`, then `BYE`; the next talk proposes again.
- **Showing.** Once the asked bit is set, a talk is the greeting and then,
  in place of the proposal, the storage selection screen over the party and
  the boxes ([storage-screen modes](#storage-screen-modes)), with every
  POKéMON that isn't that species dimmed. Any form of the species counts;
  an Egg never does. Picking a match shows it: the haunt's done line "That's
  a fine one. Thanks for showing me.", `PRAISE`, the
  [reward](#rewards-and-claims) (an item through `GIFT`), and `BYE`, +10
  friendship as a completed quest. The POKéMON stays where it was.
  Cancelling, or picking a dimmed one, ends the talk with `BYE` and changes
  nothing.
- **None to show.** With the asked bit set and no match in the party or the
  boxes, the talk is the greeting, `NOT_READY`, and `BYE`, and the storage
  screen doesn't open.
- **Expiry.** The quest stays open for the whole placement. The asked bit
  clears when the reward is given, as the claim bit is set, and when the
  haunt's placement changes, with the claim bit ([fill](#fill)).

### Quiz

Three multiple-choice type-matchup questions about the trainer's aces at
their current stage, generated from the engine's type chart
(`gTypeEffectivenessTable` in
[types_info.h](../../game/src/data/types_info.h)) without randomness.

- **Proposal.** "Three questions on type matchups. Think you can answer
  them?" (the quest line after `ASK`).
- **YES:** the trainer's `YES`, then the questions, one at a time.
- **All three right:** the done line "All correct. You really know your
  stuff.", `PRAISE`, the [reward](#rewards-and-claims) (an item through
  `GIFT`), and `BYE`: +10 friendship as a completed quest.
- **A wrong answer** ends the attempt at once: the loss line "Not quite.
  Brush up and try again.", then `BYE`. There is no penalty; the claim bit
  stays clear, so the quiz stays open this placement, and the next attempt
  asks the same questions while the aces' stages are unchanged.
- **NO:** the trainer's `NO`, then `BYE`.

**Templates.** Three haunt-line question templates, neutral like every
haunt line; `{ACE}` names the question's ace:

| # | Template | Correct answer: an attacking type whose multiplier against the ace is |
| ---: | --- | --- |
| 1 | "What type hits {ACE} super effectively?" (weakness) | 2 or more |
| 2 | "What does {ACE}'s type resist?" (resistance) | above 0 and below 1 |
| 3 | "Which move type can't touch {ACE}?" (immunity) | 0 |

**Generating the questions:**

1. **Aces.** List the aces on the trainer's current team at their current
   species, in battle order; with no ace on the team, use the signature
   POKéMON's current species. Repeat the list until it has three entries:
   question `i` asks about entry `i`.
2. **Multiplier.** An attacking type's multiplier against a species is the
   product of the chart's entries for each of the species' types, so a
   dual type counts both (Water against Rock/Ground is 2 × 2 = 4, Electric
   against it 1 × 0 = 0). Types are the eighteen battle types in the
   chart's order (Normal, Fighting, Flying, Poison, Ground, Rock, Bug,
   Ghost, Steel, Fire, Water, Grass, Electric, Psychic, Ice, Dragon, Dark,
   Fairy); `TYPE_NONE`, `TYPE_MYSTERY`, and `TYPE_STELLAR` never appear.
   Abilities, held items, and moves with special matchups play no part.
3. **Template.** Question `i` prefers template `i`. It takes the first
   template, trying the preferred one and then the others in the order 1,
   2, 3, that has a correct answer for its ace and hasn't been asked about
   that species in this quiz yet. So an ace with no immunity (or no
   resistance) is asked another template instead. If every template with a
   correct answer was already asked about that species (one ace for all
   three questions), the question takes the first template in the same
   order that still has an unused correct answer, and uses that. If nothing
   is left, which only a lone pure Normal-type ace reaches (one weakness,
   one immunity, no resistance), the question repeats question 1.
4. **Correct answer.** The first qualifying type in chart order (or the
   first not used yet, in the case above).
5. **Wrong options.** Three types that are clearly wrong under the chart:
   - weakness: types with a multiplier below 1 (resisted or immune) in
     chart order, then neutral ones (exactly 1) if fewer than three;
   - resistance and immunity: types with a multiplier of 2 or more in
     chart order, then neutral ones if fewer than three. Immune types are
     never offered on a resistance question, nor resisted ones on an
     immunity question, so no option is arguable.
6. **Options.** The correct answer and the three wrong options, four in
   all, shown in chart order, so the correct one's place varies.

Every type combination in the current chart has at least one weakness, so
every question has an answer.

### One on one

The player chooses one able party POKéMON on the existing choose-3 screen
([choose three](sevii-masters.md#party-and-heal)) limited to one, and it
fights the trainer's **lead ace**: their signature POKéMON (roster slot 1)
exactly as their battle snapshot resolves it at current TR, alone, so
`{ACE}` names the stage it fights at. It is a singles battle with no prize
money and no blackout: the chosen POKéMON keeps its damage, experience, and
level-ups, and the party is restored around it, as a
[tag match's party restore](sevii-masters.md#party-and-heal) does.
Cancelling the choice starts nothing.

- **Win:** the haunt's done line, if it authors one, then `PRAISE`, the
  reward, and `BYE`. The win counts as the completed quest (+10), never as
  a battle won ([talk flow](#talk-flow)).
- **Loss:** the haunt's loss line, or `NOT_READY` if it authors none, then
  `BYE`. There is no penalty; the claim bit stays clear, so the quest stays
  open this placement and the next talk proposes it again.

### Bring me

The haunt authors one **item kind** that fits the place, the `{KIND}` of
its quest line: a berry for a garden or a forest, a healing item for a
POKéMON Center area or a training ground, a fossil or a stone for a cave or
the museum, a pearl or shell for the seaside. The player hands over one
item of that kind from the Bag. It is one talk, with no saved state.

**Kinds.** A kind is one row of a closed kind table, resolved from the
item data (`gItemsInfo` in [items.h](../../game/src/data/items.h), whose
`struct ItemInfo` is in [item.h](../../game/include/item.h)). Each row
names one Bag **pocket** and a **match**: the whole pocket, a set of
`sortType` values (`enum ItemSortType` in
[item.h](../../game/include/item.h)), or an authored list of `ITEM_*`
constants when no field isolates the kind. An item belongs to a kind when
it sits in the row's pocket, passes the match, and is not important
(`importance` 0), so key items never count. Every row keeps to one
pocket, since the Bag screen below can't switch pockets. With
`I_COMBINE_BAG_POCKETS` `TRUE`
([config/item.h](../../game/include/config/item.h)), the treasures fold
into the Items pocket (`POCKET_TREASURES` is `POCKET_ITEMS` in
[constants/item.h](../../game/include/constants/item.h)).

| Kind | `{KIND}` | Pocket | Match | Items today |
| --- | --- | --- | --- | --- |
| berry | "BERRY" | `POCKET_BERRIES` | the whole pocket | the berries, `FIRST_BERRY_INDEX` to `LAST_BERRY_INDEX` |
| healing | "healing item" | `POCKET_MEDICINE` | `ITEM_TYPE_HEALTH_RECOVERY` | 21, from POTION and FRESH WATER to REVIVE and SACRED ASH |
| fossil | "fossil" | `POCKET_ITEMS` | `ITEM_TYPE_FOSSIL` | 15, HELIX FOSSIL to FOSSILIZED DINO |
| stone | "stone" | `POCKET_ITEMS` | `ITEM_TYPE_EVOLUTION_STONE`, `ITEM_TYPE_SHARD` | the ten evolution stones and the four shards |
| pearl | "pearl or shell" | `POCKET_ITEMS` | list: `ITEM_PEARL`, `ITEM_BIG_PEARL`, `ITEM_PEARL_STRING`, `ITEM_SHOAL_SHELL` | those four |

The pearl kind is a list because its items share `ITEM_TYPE_SELLABLE`
with nuggets, mushrooms, and stardust. Fossils are ordinary items in
Wayfarer (`I_KEY_FOSSILS >= GEN_4 || IS_WAYFARER` in
[items.h](../../game/src/data/items.h)), so they are not key items. Each
`{KIND}` text starts with a consonant, so "a {KIND}" reads right. A new
kind is a new row, under the same rules.

- **Proposal.** "Got a {KIND} on you? Bring me one." (the quest line after
  `ASK`).
- **YES:** the trainer's `YES`. With no item of the kind in the Bag, then
  `NOT_READY` and `BYE`, and the Bag doesn't open. Otherwise the Bag opens
  on the kind's pocket with pocket switching off, in a new "choose an item
  of kind K" mode ([Bag kind screen](#bag-kind-screen)).
  - **A matching item:** one is removed from the Bag, then the haunt's
    done line (default "That's the one. Just what this place calls
    for."), `PRAISE`, the [reward](#rewards-and-claims) (the next
    reward-pool entry through `GIFT`), and `BYE`: +10 friendship as a
    completed quest. The removal goes with the reward: a reward that
    waits (a full Bag, a cancelled lesson) leaves the item in the Bag, and
    the next talk proposes again.
  - **A wrong pick** (an item of the pocket outside the kind) is refused
    on the Bag screen with a system line, "That's not a {KIND}.", and the
    screen stays open. **Cancelling** closes it, then `BYE`. Nothing is
    lost, and the quest stays open this placement.
- **NO:** the trainer's `NO`, then `BYE`; the next talk proposes again.

**Cost.** The only real cost is the item: one cheap, thematic item of the
player's choosing, so they can give the cheapest they hold (an ORAN BERRY,
a POTION, a shard). The fossil kind is the dearest, since a fossil given
away is a revival forgone, so it suits a haunt where fossils are no
rarity.

### Swap battle

A sibling of [One on one](#one-on-one): the player and the trainer trade
places for a singles battle. The player fights with the trainer's
**signature ace**, their signature POKéMON (roster slot 1) exactly as
their battle snapshot resolves it at current TR, so `{ACE}` names the
stage it fights at; the trainer fights with a copy of the POKéMON the
**player picks**.

**The twist stays hidden until the pick.** The proposal reads exactly like
a [One on one](#one-on-one), so the player chooses their best, expecting
to fight with it; only then does the trainer reveal the swap, and the
player faces their own pick.

- **Proposal.** "One POKéMON each. Pick your best?" (the quest line after
  `ASK`; it never mentions a swap).
- **YES:** the trainer's `YES`, then the pick: the player chooses one able,
  non-Egg POKéMON from the party on the same choose-one screen One on one
  uses. Cancelling the pick ends the talk with nothing lost, as `NO` does.
  With no able POKéMON, `NOT_READY` and `BYE` instead.
- **Reveal.** After the pick, the haunt's reveal line: "Here's the twist:
  you take {ACE}, and I'll take yours!" (default; a haunt may author its
  own). The battle follows at once, with no further choice, since nothing
  can be lost. Then the swap, the battle, and the restore run in one script
  with no save point ([save and reset](#save-and-reset-during-a-swap)).
  1. **Park.** The whole party is saved as it is (`SavePlayerParty`), as
     tag battles do (`PrepareForFollowerNPCBattle` in
     [follower_npc.c](../../game/src/follower_npc.c)) and as the Battle
     Factory does for its rentals ([engine](#swap-battle-engine)).
  2. **Build.** The party becomes the ace alone: slot 0, a party of one,
     built from the battle snapshot as a One on one lead ace is, with the
     trainer's name as OT and the trainer's fixed trainer ID, never the
     player's. The trainer's party is a copy of the picked POKéMON, fully
     healed
     (HP, PP, and status), with its species, form, level, moves, ability,
     nature, IVs, EVs, held item, and nickname as they are. The trainer
     controls it with their own AI flags
     ([Trainer AI](trainer-ai.md)).
  3. **Battle.** A singles trainer battle against the placed trainer, with
     no prize money and no blackout, like One on one, and without the
     Frontier battle type, which would make the ace always obey.
  4. **Restore.** Won or lost, the parked party is reloaded
     (`LoadPlayerParty`), untouched, and the overworld follower is
     refreshed. The ace and the copy leave with the battle.
- **Win:** the haunt's done line (default "You made it listen. Not bad at
  all."), `PRAISE`, the [reward](#rewards-and-claims) (an item through
  `GIFT`), and `BYE`. The win counts as the completed quest (+10), never as
  a battle won (+20) ([talk flow](#talk-flow)).
- **Loss:** the haunt's loss line (default "It had a mind of its own. Try
  again?"), then `BYE`. There is no penalty; the claim bit stays clear, so
  the quest stays open this placement and the next talk proposes it again.
- **NO:** the trainer's `NO`, then `BYE`.

**Obedience.** The ace may ignore orders, and that is intended: it is the
point of the quest. Nothing new is needed; the existing Wayfarer rule does
it ([Swap battle engine](#swap-battle-engine)). For a POKéMON whose OT is
someone else, the rule compares its current level with the player's soft
level cap from their TR, so an ace above the cap may loaf, use another
move, or nap, more often the further it stands above the cap. An ace at or
below the cap always obeys, so the quest is easiest early in a trainer's
growth, or once the player's TR catches up.

**Borrowed POKéMON.** Neither side gains experience (the battle runs with
`FLAG_DISABLE_EXP_GAIN` set), so the ace never levels up, learns a move, or
evolves. Catching is impossible, as in every trainer battle. Held items are
consumed only on the copies: the pick's copy may eat its berry, and the
real POKéMON still holds it afterwards. Items the player uses from the Bag
during the battle are spent, as in any battle.

**Restored untouched.** The real party comes back exactly as it was parked:
no damage, no status, no fainting, no friendship loss, and no held item
used. The Pokédex changes only harmlessly: the battle marks the enemy's
sent-out POKéMON as seen, which here is the player's own species, already
registered, and the player's side is never marked
([Swap battle engine](#swap-battle-engine)).

#### Save and reset during a swap

No save may happen while the party is swapped. The swap, the battle, and
the restore run in one script, as a
[Masters tag match](sevii-masters.md#reset) does, so a reset at any moment
reloads the last save, where the party is whole: the quest is simply
unfought and stays open. The parked party lives in the save block's party
(`gSaveBlock1Ptr->playerParty`), which every save overwrites with the live
party (`CopyPartyAndObjectsToSave` in
[load_save.c](../../game/src/load_save.c)). So a save routine that ever
runs in that window (such as a future autosave) must write the parked
party, never the swapped one, as the Frontier's own save does
(`SaveGameFrontier` in [frontier_util.c](../../game/src/frontier_util.c)
reloads the real party, saves, then puts the rentals back). The quest in
progress records the swap while it lasts ([saved state](#saved-state));
[load validation](#load-validation) recovers a save that holds it.

### Trade

The trainer offers one of their filler POKéMON for one of the player's, and
the player's POKéMON then joins the trainer's team in that filler's place.
It is the v0 slice of
[trainer roster influence](trainer-roster-influence.md#trades): one generic
offer per trainer, decided by a fixed value check, with no authored offers.
Trade has no worked haunt yet.

#### The offer

The **offered filler** is the trainer's filler slot that comes earliest in
their [battle order](notable-trainers.md#rosters) when all six roster slots
are counted: the highest-numbered slot that isn't an ace. It is never an ace.
Like the [buddy](#trainer-values), it resolves whether or not the slot has
joined the team yet: its species is the slot's authored species after the
[downward rule](player-trainer-rating.md#evolution-stages) at the slot's
member level at the current world progress. Every roster has at least three
filler slots, so every trainer has an offer. Brock's is slot 5, so at world
progress 0 he offers OMANYTE (slot 5 is Omastar, stepped down).

`{FILLER}` names the offered filler at that stage. While the trainer holds a
[traded slot](#the-trainers-team-after-a-trade), the quest is a
[trade-back](#trade-back) instead.

- **Proposal.** "I'd trade {FILLER} for one of yours. Interested?" (the
  quest line after `ASK`).
- **NO:** the trainer's `NO`, then `BYE`; the next talk proposes again.
- **YES:** the trainer's `YES`, then the pick.

#### The pick

The storage selection screen opens over the **party only**, since a trade
is a commitment ([storage-screen modes](#storage-screen-modes)). Boxed
POKéMON never count. A party POKéMON is eligible when it is:

1. **of the trainer's type:** one of its current species' types is a type
   of one of the trainer's aces, read from the ace slots' authored species
   as the [score](#score) reads them (Brock: Steel, Ground, Rock, Flying);
2. **not an Egg;** and
3. **not the player's last able POKéMON:** not the only party member that
   is neither fainted nor an Egg.

Every other POKéMON is dimmed and can't be picked. Cancelling ends the talk
with `BYE` and changes nothing. With no eligible POKéMON in the party, the
talk is `NOT_READY` and `BYE`, and the screen doesn't open.

#### The value check

A picked POKéMON is accepted when, exactly:

```text
lineValue(picked species) >= lineValue(offered filler's species)
```

A species' **line value** is the highest base stat total
(`GetTotalBaseStat`) among the final forms it can reach through its
evolutions (`GetSpeciesEvolutions`), every branch included. A final form is
a species with no evolution, so a species with none uses its own total.
Mega Evolutions, Gigantamax forms, and other battle-only forms never count;
they aren't evolution targets, and the generator skips any that is. A
regional form follows its own line: Alolan Geodude counts Alolan Golem.
Values are read from the built species data, so they follow the build's
stat configuration ([trade engine](#trade-engine)).

There is no tolerance, no band, and no exception list. Buffing the trainer
with a stronger POKéMON is always allowed, legendaries included. For Brock's
OMANYTE (line value 495, Omastar's), a Geodude (Golem's 495) is accepted,
and a Pidgey (Pidgeot's 479; Mega Pidgeot doesn't count) is refused.
Outliers such as Shuckle, whose own 505 sits far above its use, pass as
their totals say; overrides wait for playtest ([Later](#later)).

**Refused:** the haunt line "That's not a fair trade.", then the trainer's
`NO` and `BYE`. Nothing is lost, and the quest stays open this placement.

#### Accepted

1. **Held item.** The picked POKéMON's held item goes to the Bag. With no
   room for it, the system line "There's no room in the BAG for its held
   item." ends the talk with `BYE`: nothing is traded, and the quest stays
   open.
2. **The scene.** The in-game trade scene plays (`DoInGameTradeScene`), and
   the player receives the offered filler in the picked POKéMON's party
   slot.
3. **The received filler.** It is built at **the level of the POKéMON the
   player gave**, as in-game trades already do, at the offered stage
   stepped down by the downward rule if that level is below the stage's
   evolution level; it is never built above the offered stage. It carries:
   - OT: the trainer, with the fixed trainer ID per trainer that
     [Swap battle](#swap-battle-engine) uses and the trainer's name, cut to
     the OT name's seven characters ("LT. SUR");
   - personality: a fixed function of the trainer's `characterId` and the
     slot, never shiny, so the same trainer and slot always give the same
     individual ([trade-back](#trade-back) relies on it);
   - from the roster slot: its resolved ability, nature, and IVs; no EVs and
     no held item;
   - its level-up moveset at its level, as `CreateInGameTradePokemon` gives,
     a POKé BALL, no nickname, and the in-game trade met location.

   The scene's own trade evolution then runs as in any in-game trade, so a
   received Kadabra becomes Alakazam.
4. **Completion.** The haunt's done line (default "A fair trade. Look after
   that one."), `PRAISE`, the [reward](#rewards-and-claims) (the next
   reward-pool entry through `GIFT`), and `BYE`: +10 friendship as a
   completed quest. The trade has happened by then, so a reward that waits
   (a full Bag, a cancelled lesson) sets the haunt's **owed bit**: the next
   talk is the greeting, then, in place of the proposal, the done line,
   `PRAISE`, the reward, and `BYE`, as a found keepsake does at
   [Lost something](#lost-something). The owed bit clears when the reward is
   given and, with the claim bit, when the haunt's placement changes
   ([fill](#fill)); the trade stands either way.

The pick, the scene, and the record run in one script with no save point,
so a reset reloads a save that holds neither the trade nor the record. The
next save writes the party and the record together.

#### The trainer's team after a trade

The player's POKéMON **takes the offered filler's roster slot**, its
**traded slot**. A trainer holds at most one traded slot at a time.

- **Identity.** It keeps its species and form as traded, its nickname,
  shininess, gender (its personality), nature, IVs, and ability.
- **Level and moves.** In the trainer's battles it is a member like any
  other: it joins when the team reaches the slot, takes the trainer's team
  level and the slot's level offset, takes its moves from the trainer's
  [move pool](notable-trainers.md#move-pools), visited as a filler in list
  order, and keeps the slot's held item and EVs.
- **Evolution.** Unlike every other member, it **evolves forward** and
  never steps down. Its stage starts from the species it was traded as and
  follows each next evolution whose
  [evolution level](player-trainer-rating.md#evolution-stages) (the game's
  level, or the shared table's entry) is at most its member level. At a
  branch it takes the first evolution in the game's evolution data whose
  gender condition its gender meets, ignoring every other condition. An
  evolution with no level, such as one out of a baby form, never happens.
  It **never steps down below the stage it was traded at**, at any member
  level: a Golem traded to Brock is a Golem even at Lv 20.
- **Place.** It is always a filler, never an ace, and the slot's battle
  order position applies.
- **Buddy and ace.** The signature POKéMON, slot 1, is always an ace, so
  `{ACE}` and the Quiz never read a traded slot. When the traded slot is the
  trainer's buddy slot, the traded POKéMON is the buddy: `{BUDDY}` names its
  current species on the trainer's team, and it stands beside the trainer.
  It stays the buddy after a trade-back, as the slot's own filler again.

[Notable trainers](notable-trainers.md#battle-snapshot) reads the traded slot
when it resolves the team; this section owns the rules.

#### Trade-back

A later Trade or Wanted quest with the same trainer, at any Trade or
[Wanted](#wanted) haunt and in another placement, offers the player's
POKéMON back for the exact filler they received.

- **Proposal.** "I'd trade {TRADED} back for the one you got from me.
  Interested?" `{TRADED}` is the traded POKéMON's nickname.
- **YES:** the trainer's `YES`, then the party-only selection screen, where
  only the received filler counts: the party POKéMON with the personality
  that trainer and slot give and the trainer as OT (ID and name), whatever it
  has become, and not the player's last able POKéMON. Everything else is dimmed.
  With no match in the party, `NOT_READY` and `BYE`. Cancelling changes nothing.
- **No value check:** a trade-back is a reversal, so nothing is weighed.
- **Accepted.** The held item rule and the scene run as above. The player
  receives their POKéMON at the level of the filler handed back, which is
  never below its trade level, at the stage it reaches from its traded
  species by the forward rule above. It keeps everything the record holds:
  species and form as traded (or later), personality, OT ID, OT name and
  gender, nickname, nature, IVs, ability, shininess, and ball. It returns
  with its level-up moveset at that level and no held item; its old moves,
  EVs, friendship, and ribbons are not kept. Then the done line, `PRAISE`,
  the reward, and `BYE`, +10 as a completed quest, with the owed bit as
  above. The traded slot clears, and the slot is its own filler again.

Nothing is duplicated: the received filler goes back, and the player's
POKéMON leaves the trainer's team. A player who no longer has the filler,
released or traded away, gets `NOT_READY` from every Trade or
[Wanted](#wanted) haunt that trainer stands at, while the trainer keeps
the POKéMON.

#### Limits

- One traded slot per trainer.
- One trade, or one trade-back, per placement: the claim bit.
- At most sixteen traded slots across all trainers, the size of the saved
  record pool ([saved state](#saved-state)). With every record in use, a
  trade with a trainer who holds none plays the proposal as usual, and its
  `YES` is followed by `NOT_READY` and `BYE` before the screen opens. A
  trade-back frees a record.

### Wanted

The trainer asks for one wild species that lives at the haunt and offers
one of their fillers for it. It is a [Trade](#trade) with a fixed ask: the
held item rule, the trade scene, the received filler, the traded slot, the
trade-back, and the limits are Trade's, and this section gives only what
differs. Wanted has no worked haunt yet.

#### The wanted species

The **wanted species** is derived, never authored: the strongest wild
species that lives at the haunt and shares a type with the trainer's aces.

1. **Lives at the haunt.** A species lives at the haunt when it fills a
   slot of a wild table on one of the haunt's maps. The tables are the
   `gWildMonHeaders` entries of
   [wild_encounters.json](../../game/src/data/wild_encounters.json) whose
   `map` is one of the haunt's maps, under every time-of-day label the
   Wayfarer build emits for that map (`_Day`, `_Night`, or none), and all
   four methods count: `land_mons`, `water_mons`, `fishing_mons`, and
   `rock_smash_mons`. A table whose `encounter_rate` is 0 never runs, so
   it is skipped, and so is a `SPECIES_NONE` slot. The tables mark no slot
   as gated by an item, a rod, or a flag, so every other slot counts as
   catchable: fishing with any rod, and surfing and Rock Smash too, since
   the quest stays open the whole placement. A retired map whose header
   the Wayfarer build drops has no tables.
2. **Shares a type.** One of its types is a type of one of the trainer's
   aces, read from the ace slots' authored species, as
   [Trade's pick](#the-pick) reads them.
3. **Strongest.** The highest [line value](#the-value-check), from the same
   generated table. Ties go to the lowest species number. Every stage of a
   line shares its value unless the line branches, so a tie inside a line
   names its earliest local stage.
4. **Fallback.** With no local species sharing a type, the strongest local
   species of any type, by the same order.

The wanted species reads only the haunt's tables, the line values, and the
aces' authored species. A traded slot is never an ace, so no trade changes
it, and day and night tables are pooled, so the time of day doesn't either.
It is stable for the whole placement, and the same for that trainer at that
haunt in every placement. `{WANTED}` names it.

A Wanted haunt needs local species: one whose maps have no table with a
species, such as an indoor haunt, fails the build
([trade engine](#trade-engine)).

#### Wanted's offer

The **offered filler** is the trainer's best filler worth no more than the
wanted species: of their filler slots (never an ace), the one with the
highest line value that is at most the wanted species' line value. A
filler's line value is read at its current stage, as Trade's
[offer](#the-offer) reads it. Ties go to the slot earliest in battle order,
the highest-numbered, as Trade's offer does. `{FILLER}` names it.

The player's POKéMON is worth at least the filler by construction, so there
is no value check.

#### No fair offer

When every filler's line value is above the wanted species', the trainer
has nothing fair to offer. In place of the proposal, the talk plays the
shared haunt line "I've been looking for a {WANTED}, but I've nothing fair
to trade for one.", then the trainer's `NOT_READY` and `BYE`, with no
[YES / NO]. The claim bit stays clear, and since the case depends only on
the haunt and the trainer, it lasts the placement.

Offering the lowest filler anyway was rejected: the player would get more
than they give, and the trainer's team would lose value in that slot, so
the trade would be unfair the other way.

#### Giving one

The talk checks, in order:

1. **Trade-back.** While the trainer holds a
   [traded slot](#the-trainers-team-after-a-trade), from a Trade or a
   Wanted haunt, the quest is the [trade-back](#trade-back), exactly as at
   a Trade haunt; the wanted species plays no part.
2. **No fair offer**, as above.
3. **Proposal.** "I've been looking for a {WANTED}. Got one to trade?"
   (the quest line after `ASK`). It is neutral, so any trainer can say it.

- **NO:** the trainer's `NO`, then `BYE`; the next talk proposes again.
- **YES:** the trainer's `YES`. With every traded-slot record in use,
  `NOT_READY` and `BYE` follow, as at Trade ([limits](#limits)). With no
  eligible POKéMON in the party, `NOT_READY` and `BYE`, the screen doesn't
  open, and the quest stays open this placement. Otherwise the shared haunt
  line "My {FILLER} for your {WANTED}, then." names the offer, and the
  party-only selection screen opens, as at [Trade's pick](#the-pick). A
  party POKéMON is eligible when it is of the wanted species (any form, as
  at [Catch me one](#catch-me-one)), not an Egg, and not the player's last
  able POKéMON. Every other POKéMON is dimmed and can't be picked.
  Cancelling ends the talk with `BYE` and changes nothing.
- **Accepted:** exactly as at Trade ([accepted](#accepted)): the held item,
  the scene, the received filler at the given POKéMON's level and stepped
  down, the done line (a Wanted haunt may author its own), `PRAISE`, the
  reward, and `BYE`, +10 as a completed quest, with the owed bit. The given
  POKéMON takes the offered filler's slot under
  [the trainer's team after a trade](#the-trainers-team-after-a-trade).

Trade's [limits](#limits) cover both quest types together: one traded slot
per trainer, one trade or trade-back per placement (the claim bit), and
sixteen records across all trainers.

#### Wanted examples

Illustrative, from the current wild tables and catalog rosters; line values
come from the species data (the best final form's base stat total). None of
these haunts is a Wanted haunt in the [Kanto list](#kanto-haunts).

| Haunt | Local species (line value) | Trainer (ace types) | Wanted | Offer |
| --- | --- | --- | --- | --- |
| Diglett's Cave | Tunnel only, day and night: SWINUB (530, Mamoswine), DIGLETT and DUGTRIO (425), WOBBUFFET (405); the entrances have no tables | Brock (Steel, Ground, Rock, Flying) | SWINUB, Ground | Slot 5, OMANYTE at world progress 0 (Omastar 495; ties with Golem and Kabutops, higher slot wins; Crobat's 535 is above) |
| Diglett's Cave | As above | Giovanni (Ground, Rock, Normal, Poison) | SWINUB, Ground | Slot 3, the Nidoqueen line (505), over Dugtrio and Marowak (425) |
| Cerulean Cape | Fishing: MAGIKARP and GYARADOS (540), POLIWAG and POLIWHIRL (510), GOLDEEN (450), WOOPER (430); water and fishing: PSYDUCK (500), SLOWPOKE (490); land: ODDISH (490), ABRA (500), BELLSPROUT (490), PIDGEY (479), and four Bug basics (395) | Misty (Water, Psychic, Flying, Dragon) | MAGIKARP (ties GYARADOS at 540, lower number) | Slot 5, LAPRAS (535), over Golduck and Politoed (500) |
| Viridian Forest | PIKACHU (485, Raichu), SPINARAK (400, Ariados), CATERPIE, METAPOD, WEEDLE, KAKUNA (395) | Bugsy (Bug, Steel, Fighting) | SPINARAK, Bug | Slot 6, the Ariados line (400), equal value, over Beedrill and Butterfree (395) |
| Viridian Forest | As above | Erika (Grass, Poison) | SPINARAK, Poison (no fallback: Weedle, Kakuna, and Spinarak are Poison) | None: Tangrowth 535, Bellossom 490, Jumpluff 460, Parasect 405 are all above 400, so [no fair offer](#no-fair-offer) |

Neither Diglett's Cave trainer wants DIGLETT: SWINUB, in both of the
tunnel's tables, outranks it through Mamoswine. Misty wants MAGIKARP, so a
fished GYARADOS doesn't count ([open questions](#open-questions)).

### Egg sitting

The trainer asks the player to hold on to an egg for a while. It is the
first quest that spans **two meetings**: the haunt quest hands the egg
over, and a [follow-up](#the-follow-up) at a later meeting with the same
trainer, at any haunt, settles it. A **meeting** here is one placement of
the trainer at a haunt. The player keeps the hatchling, and only the
hatched follow-up says so: no line before it mentions hatching or hints
that the player keeps anything. Egg sitting has no worked haunt yet.

Its proposal asks a favour by design, as [Courier](#courier)'s does
([dialogue](#dialogue)); it names no place, person, or need, so every
trainer can voice it.

#### Giving the egg

The talk checks, in order:

1. **Follow-up.** While the trainer has an outstanding egg, the
   [follow-up](#the-follow-up) may take the talk, as the
   [talk flow](#talk-flow) says.
2. **No egg to give.** While the trainer has an outstanding egg, or has
   given fifteen ([egg limits](#egg-limits)), the haunt behaves as if its
   quest were completed for that trainer: the greeting, `QUIRK`, and `BYE`.
   The claim bit stays clear, so the quest opens again in this placement
   once the trainer's egg is settled.
3. **Proposal.** "Could you hold on to this egg for a while?" (the quest
   line after `ASK`). It mentions no hatching and no keeping.

- **NO:** the trainer's `NO`, then `BYE`; the next talk proposes again.
- **YES:** the trainer's `YES`, then the [egg](#the-egg) is given with the
  system line "{PLAYER} received an EGG.", and `BYE`. With a full party
  the egg goes to the PC, as the `giveegg` command already does, and the
  system line "There's no room in the party. The EGG was sent to the PC."
  follows. With the PC full too, nothing is given: the trainer's
  `NOT_READY` and `BYE`, and the quest stays open.

Giving the egg sets the haunt's claim bit, with no reward and no
friendship: the quest's reward and its +10 wait for the hatched
follow-up. It also sets the trainer's **outstanding bit**, raises their
**eggs-given count** by one, and sets the haunt's
[follow-up bit](#the-follow-up), all in one script with the gift and no
save point, as at a trade.

#### The egg

- **Species.** The **egg species** comes from the filler slot that
  Trade's [offer](#the-offer) names: the highest-numbered slot that isn't an
  ace, so never an ace line. It is **the species breeding would produce**
  from that slot's authored species, by the Day Care's own rules: its
  pre-evolutions walked all the way back (`GetEggSpecies`,
  [daycare.c](../../game/src/daycare.c)), then an incense baby turned back
  into the regular offspring, as breeding without the incense does
  (`AlterEggSpeciesWithIncenseItem` and `sIncenseBabyTable` in the same
  file). Babies that need no incense do hatch: a Raichu filler gives a
  PICHU egg, a Clefable filler a CLEFFA. Incense babies don't: a Snorlax
  filler gives SNORLAX, an Azumarill filler MARILL. Brock's slot 5
  (Omastar) gives an OMANYTE egg. The
  [downward rule](player-trainer-rating.md#evolution-stages)'s "never into a
  baby" is about trainers' teams and doesn't apply to eggs. The authored
  species is read, never a
  [traded slot](#the-trainers-team-after-a-trade)'s POKéMON, and world
  progress plays no part, so the species is fixed per trainer.
- **The trainer's touch.** The egg knows the egg species' level-up moveset
  at the hatch level, plus the first move in the trainer's
  [move pool](notable-trainers.md#move-pools), in pool order and whatever
  its from level, that is an egg move of the egg species and that the egg
  doesn't already know. It is added as the Day Care adds an inherited egg
  move: into a free slot, or pushing out the first move when four are
  known. With no such move in the pool, the egg has its moveset only. The
  moves carry over at hatching.
- **Identity.** The egg's personality is a fixed function of the
  trainer's `characterId` and their eggs-given count once this egg is
  counted (1-15), salted apart from Trade's
  [received filler](#accepted), so nothing per egg needs storing: the
  outstanding egg is the one at the trainer's current count. Gender,
  nature, and every other trait the game derives from personality follow.
  It is never shiny. Like every given egg it has the player as OT, random
  IVs, a POKé BALL, and met level 0.
- **Hatching.** The normal egg steps, with `P_EGG_CYCLE_LENGTH` at
  `GEN_3`: the egg starts with its species' egg cycles, one cycle passes
  per 256 steps while it is in the party (a boxed egg doesn't progress),
  and it hatches at the first 256-step tick after the cycles reach 0. That
  is up to (cycles + 1) × 256 steps: about 4,100 for the 15-cycle fillers
  (Geodude, Zubat, Pidgey, Rattata), 5,400 for 20 (Horsea, Psyduck,
  Oddish), 6,700 for 25 (Onix), and 7,900 for 30 (Omanyte). Flame Body,
  Magma Armor, or Steam Engine in the party nearly halves it. It hatches at
  Lv 1.

**Recognition.** The trainer's **egg** is a POKéMON with the outstanding
egg's personality and the player's OT ID that is still an Egg. Their
**hatchling** has the same personality and OT ID, is no longer an Egg,
and has met level 0, which hatching sets: so the player hatched it. The
egg is looked for in the party and every box; the hatchling in the party,
every box, and the Day Care, so a hatchling left there still counts. A
chance match on personality and OT ID, 1 in 2^32, is ignored. A
randomizer that changes the species at hatching changes nothing here,
since the species is never checked.

#### The follow-up

At a talk with a trainer who has an outstanding egg, the follow-up takes
the place of the proposal or `QUIRK` when the hatchling is found, or when
the haunt's **follow-up bit** is clear. The bit is set when the egg is
given or an Egg sitting follow-up plays at the haunt (a Courier follow-up
never sets it), and clears with the claim bit when
the haunt's placement changes. So the meeting that gave the egg never asks
after it unhatched, the other follow-ups play once per meeting and the next
talk returns to the haunt's own quest, and a hatched follow-up plays as
soon as the hatchling exists.

The trainer asks "Still holding on to my egg?", and then:

- **Hatched:** the hatchling is found. The trainer is surprised: "It
  hatched?! …It's already attached to you. Keep it. It belongs with you
  now." Then `PRAISE`, the [reward](#rewards-and-claims) (the next
  reward-pool entry through `GIFT`), and `BYE`, +10 friendship as a
  completed quest. This is the first line to say the player keeps it. The
  egg is settled when the reward is given; a reward that waits (a full Bag,
  a cancelled lesson, no POKéMON able to learn) leaves it outstanding, and
  the next talk follows up again.
- **Still an egg:** the egg is found unhatched. "Thanks for keeping it
  safe. Could you hold on to it a little longer?" and [YES / NO].
  - **YES:** the trainer's `YES`, then `BYE`. It stays outstanding.
  - **NO:** the player gives the egg back: it is removed from the party or
    its box. "Of course. Thanks for looking after it.", then `BYE`. The egg
    is settled with no reward and no friendship.
- **Gone:** neither is found, since the egg or hatchling was traded or
  released. The player answers silently, as the games' protagonist does:
  the system line "{PLAYER} explained what happened…", then the trainer's
  "…You WHAT?!" and, calmer, "…Well. I hope it's in good hands, wherever
  it is.", then `BYE`. The egg is settled silently, with no reward, no
  friendship change, and no penalty, so the follow-up can't loop forever
  on an egg that won't come back.

A settled egg clears the outstanding bit; the eggs-given count stays, so a
later Egg sitting quest, below the cap, gives the next egg.

**The follow-up and the claim.** The follow-up is separate from the haunt
claim: it never reads or sets the claim bit, so the haunt's own quest is
still open on the next talk if it was before. A hatched follow-up moves the
reward counter like any completed quest. The follow-up comes before every
other replacement of the proposal (a found keepsake, an owed reward, a
Catch me one showing, a Courier trail), which waits for the next talk; only
a Courier [delivery](#delivery) comes before it
([talk flow](#talk-flow)).

#### Egg limits

- One outstanding egg per trainer.
- At most fifteen eggs per trainer, the most the 4-bit count records; after
  that, an Egg sitting haunt gives that trainer's `QUIRK`.
- One egg per placement at a haunt: the claim bit.
- **Abuse.** The prize is a Lv 1 hatchling of a filler line, after a walk
  of 4,000 to 8,000 steps, with one egg outstanding per trainer. Its worth
  is capped by the filler tier, as Trade's received filler is, and its one
  egg move is a move the trainer already uses. The hatched follow-up pays
  from the same ordered reward pool as every quest, so it reaches the pool's
  entries sooner but never adds to them; past the pool, it pays the
  fallback.

### Courier

The trainer, the **sender**, asks the player to take a parcel to another
notable trainer, the **recipient**. Like [Egg sitting](#egg-sitting), it
spans **two meetings**: the haunt quest hands the parcel over, and a
follow-up at a meeting with the recipient, at any haunt, settles it. It
reuses Egg sitting's follow-up rules (a follow-up takes the place of the
proposal or `QUIRK` for one talk, never touches the claim bit, and the next
talk returns to the haunt's own quest) and its place in the
[talk flow](#talk-flow)'s follow-up step, where the order between the two
quests is set. Courier has no worked haunt yet.

Its proposal asks a favour by design, as Egg sitting's does
([dialogue](#dialogue)). It names a person and a place, but both are
derived, never authored, so every trainer can voice it.

#### The recipient

The recipient is derived when the sender would propose, from the current
placement only:

1. **Region list.** Every trainer placed at a haunt right now who shares
   the sender's home region
   ([home region](notable-trainers.md#home-region-and-travel)), since they
   would plausibly know each other. Never the sender; Tate & Liza are never
   placed, so never them.
2. **Fallback list.** If the region list is empty, every trainer placed
   at a haunt right now, of any region, other than the sender.
3. **Pick.** Order the list by trainer catalog order (the order that breaks
   [fill](#fill) ties), starting with the first trainer after the sender
   and wrapping around. The recipient is entry `wp mod m` (counting from
   0), where `wp` is world progress and `m` the list's length. At offset 0
   it is the next qualifying trainer after the sender; the offset rotates
   with world progress, as the [fill](#fill) order does, so the same
   sender at the same haunt names someone else as the journey goes on.
4. **No one.** With both lists empty, there is no recipient, and the haunt
   offers no parcel ([giving the parcel](#giving-the-parcel)).

The recipient is saved at `YES`, so a rising world progress never changes
it afterwards. Lost something's active spot avoids world progress because it is
derived for the whole placement and never saved; the recipient is derived
once, in the talk that names them, so world progress is safe here.

**No pin.** The parcel never holds the recipient in place: placement
ignores it, as it ignores friendship and claims, and the recipient keeps
moving with every reshuffle (and, later, with travel). `{PLACE}` is true
only at the moment the sender says it.

#### Giving the parcel

The talk checks, in order:

1. **Follow-up.** A follow-up may take the talk, as the
   [talk flow](#talk-flow) says; the sender's own trail plays here.
2. **No parcel to give.** While a parcel is active (one at a time across
   the whole game, [limits](#courier-limits)), or with no recipient, the
   haunt behaves as if its quest were completed for that talk: the
   greeting, `QUIRK`, and `BYE`. The claim bit stays clear, so the quest
   opens again in this placement once the parcel is delivered or a
   recipient exists.
3. **Proposal.** "Could you take this to {OTHER}? Last I heard, they were
   around {PLACE}." (the quest line after `ASK`). `{OTHER}` is the
   recipient's name and `{PLACE}` the name of the recipient's current
   haunt: a hint, not a pin. The recipient is always placed at this moment,
   so the proposal never needs an [away line](#away-lines).

- **NO:** the trainer's `NO`, then `BYE`; the next talk proposes again,
  naming whoever the placement gives then.
- **YES:** the trainer's `YES`, then the key-item fanfare and the system
  line "{PLAYER} received the PARCEL.", and `BYE`. With no room in the Key
  Items pocket, nothing is given: the trainer's `NOT_READY` and `BYE`, and
  the quest stays open.

Giving the parcel sets the haunt's claim bit, with no reward and no
friendship: both wait for the [delivery](#delivery). It also saves the
**courier state** (the sender, the recipient, the active flag, and the
[trail bit](#refreshing-the-trail), set) and adds the
[PARCEL](#courier-engine) to the Bag, all in one script with no save point,
as at a trade.

#### Away lines

Wherever the sender names the recipient, an unplaced recipient is named by
why they are away instead of by `{PLACE}`. The lines are shared and
neutral, and each replaces the whole "Last I heard…" sentence:

| Recipient | Line |
| --- | --- |
| In the accepted event lineup | "{OTHER}'s off at the {LEAGUE} right now." |
| The resolved [partner](sevii-masters.md#partner) | "Last I heard, {OTHER} was travelling with you!" |
| Not placed for any other reason | "Nobody's seen {OTHER} around lately." |

`{LEAGUE}` names the accepted event's league: "INDIGO LEAGUE", "HOENN
LEAGUE", or, since the Masters stays off the record
([identity](sevii-masters.md#identity)), "SEVII ISLANDS". The checks go in
the table's order. In v0 only the [trail](#refreshing-the-trail) uses them,
since the proposal always names a placed recipient.

#### Refreshing the trail

At a talk with the sender while the parcel is active and its **trail
bit** is clear, the trail follow-up takes the place of the proposal or
`QUIRK`: "Still got my parcel? Last I heard, {OTHER} was around {PLACE}."
with the recipient's current haunt, or "Still got my parcel?" and the
[away line](#away-lines), then `BYE`. It sets the trail bit, so it plays
once and the next talk returns to the haunt's own quest.

The trail bit is set when the parcel is given or the trail plays, and
clears when a recompute changes the trainer at a haunt that held or now
holds the sender or the recipient ([fill](#fill)): the sender is at a new
meeting, or the recipient may have moved. So the sender retells the trail
only when it can have changed.

#### Delivery

At any talk with the recipient, at any haunt, while the parcel is active,
the delivery takes the place of the proposal or `QUIRK`. It works at a
first meeting too: the greeting (`MEET` or `HEARD`) and the Stranger's +1,
then the delivery. The recipient says "A parcel from {SENDER}? For me?
Thank you!", where `{SENDER}` is the sender's name; then the recipient's
`PRAISE`, the recipient's [reward](#rewards-and-claims) (their next
reward-pool entry through `GIFT`, or their fallback), and `BYE`.

When the reward is given, in one script: the PARCEL is removed, the
courier state is cleared, and friendship rises by +10 with the sender and
+10 with the recipient, each as a completed quest; the recipient's reward
counter moves, the sender's doesn't. The courier is then settled. A reward
that waits (a full Bag, a cancelled lesson, no POKéMON able to learn)
changes nothing, the parcel stays active, and the next talk with the
recipient delivers again. The delivery never reads or sets any haunt's
claim bit: the recipient's own haunt quest is still open on the next talk
if it was before.

The sender needn't be met again: their +10 counts wherever they are,
placed or not, as the quest completed at the placement that gave the
parcel. A sender or recipient who crosses Friend this way hands over their
number at once, with the one system line
[Notable trainers](notable-trainers.md#friendship) gives any route.

#### Courier limits

- **One parcel at a time**, across all trainers: one PARCEL key item and
  one saved courier state.
- **No expiry and no penalty.** A parcel stays active through reshuffles,
  league events, partnering, and reloads until it is delivered; the
  player loses nothing by holding it.
- One parcel per placement at a haunt: the claim bit.
- **Abuse.** Each parcel pays one reward-pool entry, from the recipient's
  ordered pool, and +10 twice; one parcel at a time and the claim bit cap
  it at one per placement, so it pays no faster than two ordinary quests.

### Handicap

A trainer far ahead of the player offers to fight the player's whole team
with their signature ace alone. It reuses [One on one](#one-on-one)'s ace
and its no-blackout route, but the player brings everything: the full party
and the Bag. Handicap has no worked haunt yet.

**Not retryable.** Handicap is the only quest type that isn't retryable:
each trainer gives **one attempt, ever**, at any of their Handicap haunts,
and it pays the reward whether the player wins or loses. Every other quest
stays open after a failure; this one is a humbling one-shot moment with a
trainer far ahead, not a wall to grind against, so a loss still pays and
the trainer never offers it again.

#### The gate

The trainer offers a Handicap only while their **current TR** (from their
[battle snapshot](notable-trainers.md#battle-snapshot)) is at least the
player's TR (`GetTrainerRating()`) plus the **handicap margin**, a
placeholder **40**. It is checked at every talk that would propose the
quest, and nothing about it is saved.

The talk checks, in order, after any [follow-up](#talk-flow):

1. **Owed reward.** While the haunt's owed bit is set
   ([the outcome](#the-handicap-outcome)), the owed completion.
2. **Claim bit set**, the trainer's `QUIRK`, as at any completed quest.
3. **Attempted.** While the trainer's **handicap attempted** bit is set,
   the [callback](#after-a-handicap), whatever the TRs.
4. **Gate not met.** The shared line "Heh. You don't need my handicap
   anymore.", then `QUIRK`, then `BYE`. The claim bit stays clear and the
   attempt is not used up, so the proposal returns at any later talk where
   the gate holds, at this placement or another.
5. **Proposal.** `ASK`, "Your whole team against my {ACE}. Think that's
   enough?", and [YES / NO]. `NO` gives the trainer's `NO`, then `BYE`, and
   changes nothing.

The gate fails in two ways that play the same: the player has caught up
(the trainer was once 40 TR ahead and is no longer), or the trainer was
never that far ahead (Brock at world progress 0 has TR 25, under the
player's 0 plus 40). The haunt keeps no history to tell them apart, and it
needs none: either way the trainer's TR isn't well above the player's, so
there is no offer, and the line reads true for both. Both TRs move over a
journey, so the gate can open and close again; Lance, at TR 200, passes it
at every player TR up to 160.

#### The handicap battle

`YES` gives the trainer's `YES`, and the battle follows at once: a normal
singles trainer battle against the placed trainer.

- **The trainer's team** is the signature ace (roster slot 1) alone,
  exactly as their battle snapshot resolves it at current TR, as
  One on one's lead ace is: its stage by the
  [downward rule](player-trainer-rating.md#evolution-stages) at
  `teamLevel(tr)` (slot 1's offset is 0), capped at Lv 100, the moves it has
  in the full resolved team, and its normal held item, ability, nature, and
  IVs and EVs. It is **never a second ace** or a filler, even for a Double
  Ace trainer. `{ACE}` names the stage it fights at.
- **Totem aura.** The ace starts the battle at **+1 Attack, Defense,
  Speed, Sp. Atk, and Sp. Def**, shown before the first turn with the
  engine's "…aura flared to life!" message; accuracy and evasion stay at
  0, as a totem's "all stats" boost leaves them. The boost is ordinary
  stat stages, set up by the engine's totem boost command
  ([Handicap engine](#handicap-engine)), so the player can answer it the
  usual ways (Haze, Clear Smog, Spectral Thief, a copied boost).
- **The smartest AI.** The ace battles with the trainer's
  [resolved flags](trainer-ai.md#resolution) at their best: Basic, their
  play style, **every AI skill tier up to 3 (Predictive)** whatever their
  TR, Ace Pokemon (one ace), and the boss flag's **Omniscient**, whether or
  not the trainer has the boss flag. This is a per-battle override of the
  resolution, written at the [override
  point](trainer-ai.md#runtime-and-the-override-point) and into the
  prediction slots, as every notable battle's flags are.
- **The player** uses their full party and Bag, unrestricted: any lead,
  switching, and items, with experience, level-ups, and evolution as in
  any trainer battle.
- **No prize money and no blackout**, by the route One on one uses. After
  a win the party stays as the battle left it, as after any battle. After
  a loss the whole party is fainted and no whiteout heals it, so the party
  is fully healed (`HealPlayerParty`) before the loss line.

Win or loss counts as the completed quest (+10), never as a battle won
(+20), even as a first win over the trainer ([talk flow](#talk-flow)).

#### The handicap outcome

When the battle ends, in one script with no save point, the trainer's
**handicap attempted** bit is set, and after a win their **handicap
beaten** bit too. Then:

- **Win:** "…Six against one, and you actually did it.", then `PRAISE`,
  the [reward](#rewards-and-claims) (the next reward-pool entry through
  `GIFT`), and `BYE`: +10 friendship as a completed quest. The line is
  fixed text and says six whatever the party's size.
- **Loss:** "Not bad for being outnumbered. Take this anyway.", then the
  reward through `GIFT`, and `BYE`: +10 friendship as a completed quest.
  There is **no `PRAISE`** after a loss: every v0 `PRAISE` congratulates a
  success ("Even I did not foresee that. Well done."), so it reads wrong
  after a defeat, and the loss line is already the neutral beat. A lesson
  reward, which normally follows `PRAISE`, follows the loss line instead.

The reward sets the claim bit and moves the counter as for any quest. The
attempt is spent before the reward, so a reward that waits (a full Bag, a
cancelled lesson) sets the haunt's **owed bit**, as at
[Trade](#accepted): the next talk is the greeting, then, in place of the
callback, the outcome line again (win or loss by the beaten bit), `PRAISE`
after a win, the reward, and `BYE`. The owed bit clears when the reward is
given and, with the claim bit, when the haunt's placement changes.

#### After a handicap

At any Handicap haunt whose placed trainer's attempted bit is set, the
**callback** takes the proposal's place while the claim bit is clear:

| Outcome | Callback |
| --- | --- |
| Won (beaten bit set) | "Still thinking about our six-on-one?" |
| Lost | "Come back when you can take me one-on-one." |

then `BYE`, with no `QUIRK` in that talk: the callback is the trainer's
remark for the step. It sets the claim bit, with no reward and no
friendship, as Egg sitting's egg and Courier's parcel do, so it plays once
per placement; every later talk there is the greeting, `QUIRK`, and `BYE`,
as after any completed quest, until the placement changes. A Handicap haunt
whose trainer has made their attempt therefore never proposes again.

#### Handicap limits

- **One attempt per trainer, ever**, across all their Handicap haunts, so
  at most one Handicap reward per trainer.
- **Resets.** The attempted bit is saved with the next save, like every
  outcome; a reset before saving reloads a save in which the attempt is
  unused. That is accepted, as for any one-time battle in the game.
- **The beaten bit is a record.** In v0 it only picks the callback line
  and the owed completion's outcome line; nothing else reads it. It is kept
  for later gossip, phone calls, and the trainer card ([Later](#later)).

### Rewards and claims

Each haunt has a saved **claim bit**. It is set when the reward is given
(at an Egg sitting haunt, when the egg is given; at a Courier haunt, when
the parcel is given; at a Handicap haunt, also when a
[callback](#after-a-handicap) plays) and cleared when the haunt's
placement changes, so each placement pays out once, and a new trainer at
the haunt can give it again.

The reward comes from the trainer, never from the quest: the haunt carries
the ask, and the trainer carries the reward. Finishing any haunt quest with
a trainer pays from their **reward pool**, wherever they are placed.

**Reward pool.** Each notable trainer authors one ordered reward pool, held
in the catalog
([Notable trainers](notable-trainers.md#haunt-values-buddy-reward-pool))
like the buddy. Each entry is an **item** (one `ITEM_*` constant) or a
**lesson** (below), and has a **from world progress** gate: the entry opens
once world progress (`GetTrainerRating()`, the player's TR) reaches the
gate. The trainer's own TR plays no part, so Lance's and Agatha's pools, whose
TR never changes, no longer open all at once. The v0 pools are in
[notable trainer reward pools](../research/notable-trainer-rewards.md).

**What a quest pays.** Each trainer has a saved **reward counter**: a plain
count (0-15, 4 bits) of the pool entries the player has received from them.
When a quest completes, the next entry (the one after the counter) is the
reward if it exists and its gate is at most the current world progress.
Otherwise, because the pool is used up or the next entry is still gated, the
trainer pays the **fallback** and the counter doesn't move. Entries come
strictly in order: a gated entry is never skipped.

- **Item:** given with `GIFT`, whose `{ITEM}` names it. With no room in the
  Bag, the reward waits: the claim stays open and the counter doesn't move.
- **Lesson:** the player picks a POKéMON from the party or boxes on the
  storage screen's move tutor mode, and the trainer teaches it the first move
  in their [move pool](notable-trainers.md#move-pools) that it can learn and
  doesn't know yet. Only POKéMON with such a move can be picked. "Can learn"
  is the move pool's eligibility for an entry with a from level, at any
  level: a level-up move of the current species or an earlier form, a
  TM/tutor move of the current species, or an egg move of the line's base
  species. The move pool's from levels play no part here. The move is
  learned through the usual learn-move prompts, replacing a move if the
  POKéMON knows four. The lesson uses `PRAISE` and no `GIFT`. Cancelling
  leaves the claim open and the counter unchanged; with no POKéMON that can
  learn anything from the pool, the trainer answers `NOT_READY`.
- **Fallback:** prize money, exactly what a win over the trainer would pay
  right now: the [prize money](#prize-money) rule for their
  signature POKéMON (slot 1 has offset 0 and is always last in battle order,
  so its level is `teamLevel(tr)`) with their class's rate. A system line
  names the amount; there is no `GIFT`, and the counter doesn't move.

The fallback is money because it always fits: it needs no Bag room, so it
never waits; it grows with the trainer; it adds nothing a battle with the
trainer doesn't already pay, so it can't bend the item curve; and it leaves
no pile of
repeated items. A repeat of the trainer's type item was the alternative,
rejected because it can wait on a full Bag and, repeated, is worth nothing.

The claim bit and the counter change together, when the reward is given
(for the fallback, only the claim bit). A reward that waits or is cancelled
changes neither. Egg sitting is the exception: the egg sets the claim bit,
and the hatched [follow-up](#the-follow-up) moves only the counter.
Courier is the same: the parcel sets the claim bit at the sender's haunt,
and the [delivery](#delivery) moves only the recipient's counter.

**Pool rules**, checked with the catalog:

- A pool has 1 to 15 entries, the most a 4-bit counter records; v0 authors
  five or six.
- Every item is an existing `ITEM_*` constant in
  [items.h](../../game/include/constants/items.h), never `ITEM_NONE`.
- Gates are whole numbers from 0 and never decrease along the pool.
- A lesson entry needs a trainer with a non-empty move pool.
- Every `GIFT` line fits 70 characters with the longest item name in that
  trainer's pool (v0 names reach 16 characters: PEWTER CRUNCHIES).

## Dialogue

A haunt's dialogue is assembled from two sources:

- **Haunt lines**, authored per haunt: its quest line, a done line for a
  walk, a Lost something, or a Catch me one (and optionally for a One on
  one), a hint per
  lost spot, and optionally a loss line for a One on one. The Quiz's
  question templates, done line, and loss line are shared by every Quiz
  haunt. A Bring me haunt authors its kind, and a Bring me or Swap battle
  haunt may author its own done line (and a Swap battle one its loss
  line), with the defaults in their sections. A Trade or Wanted haunt may
  author its done line; the refusal line, Wanted's proposal, offer, and
  no-offer lines, and the trade-back proposal are shared. Every Egg
  sitting line is shared: the proposal and the
  [follow-up](#the-follow-up)'s question, hatched, still-an-egg,
  given-back, and gone lines. Every Courier line is shared too: the
  proposal, the away lines, the trail, and the delivery. So is every
  Handicap line: the proposal, the gate's line, the win and loss lines, and
  the two callbacks ([Handicap](#handicap)). They
  describe only the place and the activity, never a trainer's personality or
  history, so they read true for every candidate.
  A quest line is the proposal that follows `ASK` ("Walk it with me, out to
  the VERMILION side?"), so the same line works after any attention-getter.
  **Writing rules:** haunt lines carry the content of the proposal, and
  haunt lines never imply the trainer needs help or is asking a favour, so
  every trainer can voice every quest; voice bits carry only personality.
  Egg sitting's proposal and its still-an-egg line, and Courier's proposal
  and trail, are the exceptions: a plain favour with no need behind it,
  which every trainer can still voice. Courier's lines name another
  trainer and a haunt, but only through derived slots.
  No Egg sitting line before the hatched follow-up mentions hatching or
  hints that the player keeps the hatchling.
- **Voice bits**, fifteen per trainer: `HELLO`, `AGAIN`, `CLOSE`, `MEET`,
  `HEARD` (with `{PLAYER}`), `NOT_YET`, `NEWS`,
  `ASK` (an attention-getter only, never a request), `YES` (pure approval
  of the answer: no movement, timing, or assumed activity), `NO`,
  `NOT_READY`, `PRAISE` (these three assume no specific activity, movement,
  or place), `GIFT`, `BYE`, and `QUIRK`
  ([voice bits](../research/notable-trainer-voices.md)). They carry the
  personality and never mention a place. Haunts use thirteen of them in v0:
  `NOT_YET` is reserved for places that host battles (such as Gyms or
  future rematch spots), and `NEWS` is unused by haunts, reserved for world
  news or later uses. Haunts carry no gossip in v0.

Both use these slots:

| Slot | Resolves to |
| --- | --- |
| `{PLAYER}` | The player's name. |
| `{ITEM}` | The reward item (only in `GIFT`). |
| `{ACE}` | The signature POKéMON's current species; in a quiz question, that question's ace at its current species. |
| `{BUDDY}` | The buddy slot's current species ([trainer values](#trainer-values)). |
| `{LOCAL}` | The haunt's catch species (only in a Catch me one line). |
| `{HINT}` | The active lost spot's hint (only in a Lost something line). |
| `{KIND}` | The haunt's item kind's display text, such as "BERRY" or "healing item" (only in a Bring me line and its refusal). |
| `{FILLER}` | The trainer's offered filler at its current stage, such as "OMANYTE" (only in a Trade proposal, [the offer](#the-offer), or Wanted's offer line, [Wanted's offer](#wanteds-offer)). |
| `{WANTED}` | The haunt's wanted species for the placed trainer, such as "SWINUB" (only in Wanted's proposal, offer, and no-offer lines; [the wanted species](#the-wanted-species)). |
| `{TRADED}` | The nickname of the POKéMON the player traded to the trainer (only in a trade-back proposal). |
| `{OTHER}` | The [recipient](#the-recipient)'s name (only in Courier's proposal, trail, and away lines). |
| `{PLACE}` | The `Name` of the recipient's current haunt at that moment, such as "CERULEAN CAPE": a hint, not a pin (only in Courier's proposal and trail). |
| `{SENDER}` | The sender's name (only in Courier's delivery line). |
| `{LEAGUE}` | The accepted event's league, such as "INDIGO LEAGUE" (only in Courier's [away line](#away-lines)). |

Speaker labels ("BROCK:") and system messages (the [YES / NO] prompt, the
number given, the quiz's answer options, a lesson's pick, the fallback
amount, the found keepsake, a refused Bring me pick, a held item with no
Bag room at a trade, a received egg or one sent to the PC, the
player's explanation of a gone egg, and a received PARCEL) are generic
text, the same for every trainer.

## Worked example: Diglett's Cave

**Tags.** Region Kanto; theme Ground; not elite; no hometown; activities
train and lie low (trainers come to train in the tunnel or to keep out of
sight); setting remote; capacity 1; quest Walk with me. Maps
`DiglettsCave_EntranceNorth_hns`, `DiglettsCave_Tunnel_hns`, and
`DiglettsCave_EntranceSouth_hns`. The standing tile is in the north
entrance, where Brock's cameo stands today; the exit is the south
entrance's warp to `MAP_VERMILION_CITY_HNS`. The tunnel's wild table holds
Diglett and Dugtrio (with Swinub and Wobbuffet).

**Candidates.** Every Kanto trainer who isn't aloof (Brock, Misty,
Lt. Surge, Erika, Janine, Blaine, Giovanni, Blue, Lorelei, Bruno) and every
non-aloof traveller from elsewhere (Bugsy, Will, Brawly, Drake). Sabrina,
Agatha, Lance, Glacia, Wallace, and Steven are aloof, and this haunt is not
elite. 
Every quest type is open to every trainer.

**Scores** at world progress 30 (three badges; placeholder weights;
momentum from the explorer's growth curves):

| Trainer | Theme | Style | Momentum | Score |
| --- | ---: | ---: | ---: | ---: |
| Giovanni | 4 (Rhyperior, Ground) | 2 (Brawler) | 0 (settled) | 6 |
| Brock | 4 (Steelix, Ground) | 0 (Field marshal) | 1 (rising) | 5 |
| Drake | 2 (Flygon, Ground) | 2 (Sweeper) | 1 (rising) | 5 |
| Erika | 0 | 2 (Hexer) | 1 (rising) | 3 |
| Janine | 0 | 2 (Hexer) | 1 (rising) | 3 |
| Lorelei | 0 | 2 (Sweeper) | 1 (rising) | 3 |
| Bruno | 0 | 2 (Brawler) | 1 (rising) | 3 |
| Bugsy | 0 | 2 (Sweeper) | 0 (settled) | 2 |
| Brawly | 0 | 2 (Brawler) | 0 (settled) | 2 |

Everyone else scores 0 or 1. Giovanni fits best, then Brock and Drake. The
switch from "sightseeing" to train and lie low moved these numbers: Brock
lost his Field marshal points, and Giovanni and the training styles gained.
Who is still free at the cave's turn in the fill depends on every other
haunt's scores, which the new activities also shift, so the fill order
walk-through is **pending the routine design** and not restated here. The
scenes below keep their world progress and assume the placements they
name.

The scenes below follow one save through the cave. Friendship points are
the [talk flow's](#talk-flow) and
[Notable trainers'](notable-trainers.md#friendship) events; rewards come
from each trainer's
[reward pool](../research/notable-trainer-rewards.md).

**Scene 1: Giovanni's first meeting** (world progress 10, assuming Giovanni
holds the cave, as his top score there suggests; the player's TR 10 is below
`min(80, 24 − 10) = 14`, so his fame hasn't reached him and he greets with
`MEET`). The talk makes him Met (+1) at its start, so the quest is proposed
at once:

```text
GIOVANNI: I am GIOVANNI. Remember the name. Others have       MEET
          learned to.
GIOVANNI: I have a proposition.                               ASK
GIOVANNI: Walk it with me, out to the VERMILION side?         quest line
> No
GIOVANNI: Hmph. You'll regret wasting my time.                NO
GIOVANNI: Go. We will meet again.                             BYE
```

Declining costs nothing. The next talk greets the player as Met and
proposes again:

```text
GIOVANNI: You again. I am beginning to remember your face.    AGAIN
GIOVANNI: I have a proposition.                               ASK
GIOVANNI: Walk it with me, out to the VERMILION side?         quest line
> Yes
GIOVANNI: A wise decision.                                    YES
```

Answering Yes in the first talk gives the same `YES` straight after `MEET`
and the proposal. Either way the talk ends at `YES`, and the walk starts.

**Scene 2: the walk.** Giovanni follows the player through the tunnel:

```text
  (wild pairs, such as two DIGLETT, are tag battles, with
   GIOVANNI's best three beside the player's first three)
```

If the player heads back to the north entrance's warp to Route 2:

```text
HAUNT: Giving up on the walk?
> Yes
GIOVANNI: Hmph. You'll regret wasting my time.                NO
```

The walk ends with no reward and the claim bit clear, and Giovanni is back
at his standing tile; answering No steps the player back one tile and the walk
goes on. A whiteout in a tag battle ends the walk the same way, silently:
the player wakes at the POKéMON Center, and the next talk at the cave is
`AGAIN` and the proposal again. Stepping through the Vermilion exit with
Giovanni following completes it:

```text
GIOVANNI: That's the VERMILION side. We made it through.      done line
GIOVANNI: Impressive. I rarely have cause to say that.        PRAISE
GIOVANNI: Take this NUGGET. Consider it a loan, not a         GIFT
          kindness.
GIOVANNI: Go. We will meet again.                             BYE
```

The NUGGET is the first entry of Giovanni's
[pool](../research/notable-trainer-rewards.md#giovanni) (from world progress
0); the walk plays no part. His reward counter is now 1, so his next quest,
here after a placement change or at any other haunt, pays SOFT SAND. The
quest adds 10 points (1 + 10 = 11), so he is still Met.

**Scene 3: talking after completion.** The claim bit is set for this
placement, so every talk is the greeting, `QUIRK`, and `BYE`:

```text
GIOVANNI: You again. I am beginning to remember your face.    AGAIN
GIOVANNI: Ah, MEOWTH. The only one who never disappoints me.  QUIRK
GIOVANNI: Go. We will meet again.                             BYE
```

His buddy is roster slot 4, which steps down to MEOWTH below level 28: at
world progress 10 (TR 24, team level 17) the line says MEOWTH; from world
progress 40 (team level 39) it says PERSIAN. Repeat talks add no points.

**Scene 4: Misty, a Friend, after a reshuffle.** World progress rises, the
placement is recomputed, and say Misty now takes the cave (an illustration:
the real placement depends on the whole fill). The haunt's trainer changed,
so its claim bit cleared and the walk is open again. The player beat Misty
at Cerulean, and that first win made her a Friend (20 points) and gave her
number. Her stage is Friend when the talk starts, so she greets with
`HELLO`, and this first talk adds nothing (only a Stranger's does):

```text
MISTY: Hey, {PLAYER}! Took you long enough!                   HELLO
MISTY: Okay, listen up!                                       ASK
MISTY: Walk it with me, out to the VERMILION side?            quest line
> Yes
MISTY: Now that's what I like to hear!                        YES
  (the walk runs as in scene 2, with MISTY beside the player)
MISTY: That's the VERMILION side. We made it through.         done line
MISTY: Wow, not bad! You'd almost make a decent Water         PRAISE
       trainer!
MISTY: Take this MYSTIC WATER! Don't say I never gave you     GIFT
       anything!
MISTY: See ya! ...{BUDDY}, get back in your ball! Not again!  BYE
```

MYSTIC WATER is the first entry of her
[pool](../research/notable-trainer-rewards.md#misty), and the quest takes
her to 30 points. Talking to her again during this placement:

```text
MISTY: Hey, {PLAYER}! Took you long enough!                   HELLO
MISTY: Just keep Bug POKéMON away from me. Seriously. Ugh!    QUIRK
MISTY: See ya! ...{BUDDY}, get back in your ball! Not again!  BYE
```

`{BUDDY}` is her slot 2 at its current stage: PSYDUCK early, GOLDUCK later.

**Scene 5: Brock, Close, late game** (world progress 60, if Brock stands
here). The player met him at a haunt before his Gym (+1), beat him at Pewter
(+20), and finished four quests with him at haunts (+40), so his score is
61: Close. The first two paid PEWTER CRUNCHIES and the HARD STONE; the next
two came before world progress 40, while his third entry was still gated,
so they paid the fallback prize money and left his reward counter at 2.

```text
BROCK: {PLAYER}! Good to see you, friend. There is always a   CLOSE
       plate for you.
BROCK: Hey, I've got an idea.                                 ASK
BROCK: Walk it with me, out to the VERMILION side?            quest line
> Yes
BROCK: Great! That's the spirit!                              YES
  (the walk runs as in scene 2, with BROCK beside the player)
BROCK: That's the VERMILION side. We made it through.         done line
BROCK: Nicely done! That's rock-hard willpower if I ever saw  PRAISE
       it.
  (the player picks a POKéMON on the storage screen; BROCK
   teaches it the first move in his pool it can learn and
   doesn't know yet)
BROCK: Take care! And keep your POKéMON well fed!             BYE
```

The lesson is the third entry of Brock's
[pool](../research/notable-trainer-rewards.md#brock), open from world
progress 40; a lesson uses `PRAISE` and no `GIFT`. His counter is now 3, and
his next entry, the OVAL STONE (from world progress 50), is open too.
Talking to him again during this placement:

```text
BROCK: {PLAYER}! Good to see you, friend. There is always a   CLOSE
       plate for you.
BROCK: STEELIX gets fussy if I burn the rice. So do my little QUIRK
       siblings.
BROCK: Take care! And keep your POKéMON well fed!             BYE
```

His buddy is roster slot 1, ONIX until world progress 57, so at world
progress 60 the `QUIRK` names STEELIX.

## Worked example: Celadon Game Corner

**Tags.** Region Kanto; no theme types; not elite; no hometown; activity
gamble; setting public; capacity 1; quest One on one, framed as a bet. Map
`CeladonCity_GameCorner_hns`. The standing tile is tile (12, 11), the
walkable tile at the south end of the east bank of slot machines (the
machines fill x 12-13, y 6-10); the buddy stands beside it at (13, 11). Both
tiles are free of collision and of the room's existing objects.

**Quest.** The haunt authors its own One on one lines
([One on one](#one-on-one)):

| Line | Text |
| --- | --- |
| Quest line | "One POKéMON each, winner takes the pot. Your best against {ACE}?" |
| Done line (win) | "Jackpot! You cleaned out the house." |
| Loss line | "The house wins this time. Come back for a rematch." |

`YES` runs the trainer's `YES`, then the player picks one POKéMON on the
choose-3 screen limited to one, and the trainer sends their signature ace
alone at its current stage, the one `{ACE}` just named. A win is the done
line, `PRAISE`, the next reward-pool entry through `GIFT`, and `BYE`, worth
+10 friendship as a completed quest (not +20 as a battle won). A loss is the
loss line and `BYE`: no penalty, no coins, and the quest stays open this
placement. `NO` is the trainer's `NO`, as anywhere.

**Who's likely, and why.** With no theme and no hometown, only the activity
scores here, so the v0 score is 2 for a Gambler and 0 for everyone else
(no momentum lists gamble). The setting is public, so no aloof trainer is a
candidate.

- **Lt. Surge** is a Gambler: 2 points. He is also earliest in catalog order
  among the Gamblers who can reach Kanto, so he wins ties. His hometown,
  nearby Vermilion, scores nothing here; a pull towards places near home
  waits for the routine design.
- **Giovanni** is a Brawler, so v0 gives him 0 here; he lands at the Game
  Corner only as a leftover, when Surge and Blaine are placed elsewhere and
  he is the earliest free trainer left. The pull the design wants, the Game
  Corner as the old Rocket hideout's front and a gambling or business angle,
  needs a trainer-level affinity that v0 lacks and waits for the routine
  design.
- **Erika** is Celadon's own Gym Leader, but the Game Corner carries no
  hometown tag, and as a Hexer (lie low) gambling isn't her thing: 0 points.
  v0 can still seat her as a zero-score leftover, since she comes before
  Giovanni in catalog order; keeping her out entirely is for the routine
  design.
- **Blaine** is a Gambler too, with the same 2 points, so ties go to Surge
  and Blaine takes the Game Corner only while Surge is placed elsewhere. He
  is no traveller and Cinnabar is far away, which should make him rare
  here, but v0 has no distance; that also waits for the routine design.

**Lt. Surge, a Friend, early** (world progress 10, if Surge holds the Game
Corner). The player's one badge so far is Surge's, and that first win made
him a Friend (20 points). At world progress 10 his TR is 40 and his team
level 28, so his signature Raichu (slot 1) steps down to PIKACHU (Raichu
needs Lv 30): `{ACE}` says PIKACHU, and PIKACHU, Lv 28, is what he sends.
His stage is Friend, so he greets with `HELLO`:

```text
LT. SURGE: Hey, kid! {PLAYER}! Still standing? Good!          HELLO
LT. SURGE: Listen up, soldier!                                ASK
LT. SURGE: One POKéMON each, winner takes the pot. Your best  quest line
           against PIKACHU?
> Yes
LT. SURGE: Now that's a soldier!                              YES
  (the player picks one POKéMON on the choose-3 screen,
   limited to one; LT. SURGE sends PIKACHU, Lv 28, alone,
   and it faints)
LT. SURGE: Jackpot! You cleaned out the house.                done line
LT. SURGE: Ahaha! You're the real deal, kid! Outstanding!     PRAISE
LT. SURGE: Supply drop, kid: one MAGNET! Use it well!         GIFT
LT. SURGE: Dismissed, kid! Stay sharp!                        BYE
```

MAGNET is the first entry of his
[pool](../research/notable-trainer-rewards.md#lt-surge) (from world progress
0). The quest adds 10 points (20 + 10 = 30), and he stays a Friend. Talking
to him again during this placement:

```text
LT. SURGE: Hey, kid! {PLAYER}! Still standing? Good!          HELLO
LT. SURGE: ELECTRIC POKéMON saved me in the war! PIKACHU      QUIRK
           never lets me forget.
LT. SURGE: Dismissed, kid! Stay sharp!                        BYE
```

His buddy is slot 1 too, so the `QUIRK` names PIKACHU until his team level
reaches 30 (world progress 13 on his curve, so in practice from the second
badge, at world progress 20, it says RAICHU).

**Giovanni at Met: a loss, then a win** (world progress 40, if Giovanni holds
the Game Corner). This is the save from Diglett's Cave: he has 11 points
(Met) and his reward counter is 1. World progress 40 is four badges, where
his Burst growth jumps: his TR goes from 24 to 60 and his team level from 17
to 39. His signature Rhyperior (slot 1) steps down below Lv 55 to Rhydon and
below Lv 42 to RHYHORN, so at Lv 39 `{ACE}` says RHYHORN. (His Burst curve
steps his team level through 17, 39, 59, 82, and 100, so the v0 curve never
shows Rhydon: Rhyhorn until world progress 80, then Rhyperior.) He greets
with `AGAIN`:

```text
GIOVANNI: You again. I am beginning to remember your face.    AGAIN
GIOVANNI: I have a proposition.                               ASK
GIOVANNI: One POKéMON each, winner takes the pot. Your best   quest line
          against RHYHORN?
> Yes
GIOVANNI: A wise decision.                                    YES
  (the player picks one POKéMON; GIOVANNI sends RHYHORN,
   Lv 39, alone, and the player's POKéMON faints)
GIOVANNI: The house wins this time. Come back for a rematch.  loss line
GIOVANNI: Go. We will meet again.                             BYE
```

The claim bit stays clear, his counter stays at 1, and no points change.
The next talk proposes the bet again:

```text
GIOVANNI: You again. I am beginning to remember your face.    AGAIN
GIOVANNI: I have a proposition.                               ASK
GIOVANNI: One POKéMON each, winner takes the pot. Your best   quest line
          against RHYHORN?
> Yes
GIOVANNI: A wise decision.                                    YES
  (this time the player's POKéMON wins)
GIOVANNI: Jackpot! You cleaned out the house.                 done line
GIOVANNI: Impressive. I rarely have cause to say that.        PRAISE
GIOVANNI: Take this SOFT SAND. Consider it a loan, not a      GIFT
          kindness.
GIOVANNI: Go. We will meet again.                             BYE
```

SOFT SAND is the second entry of Giovanni's
[pool](../research/notable-trainer-rewards.md#giovanni), as Diglett's Cave
promised: the pool follows the trainer, whichever haunt he is at. The quest
adds 10 points (11 + 10 = 21), which crosses the Friend threshold, so he
hands over his number with the one system line
[Notable trainers](notable-trainers.md#friendship) describes.

**What this example shows.** The public setting keeps every aloof trainer
out before any score is read, so the aloof filter works on setting alone. A
haunt with no theme and no hometown draws its trainers by activity alone,
here gamble from the Gambler play style, and the reasons the design wants
beyond that (Giovanni's history, Erika's distaste, Blaine's distance) are
left to the routine design rather than faked with tags. One on one is a
battle quest whose loss is harmless: no blackout, no money, no penalty, and
the proposal comes back at the next talk, while the win pays like any other
quest. Capacity 1 indoors means one trainer at one standing tile beside the slot
machines, with room for the buddy and nothing else in the way.

## Worked example: Cerulean Cape

**Tags.** Region Kanto; theme Water; not elite; hometown Cerulean;
activities relax and sightsee (trainers come to unwind by the sea or take
in the view); setting remote; capacity 1; quest Lost something. Map
`Route25_hns`, the cape at the east end of Route 25, past Bill's house
(door warp at (85, 13)). The cape proper is the fenced clifftop at x
97-100, y 12-19, reached by the steps at (98-99, 21-22) from the lower path
(y 23). The standing tile is (100, 14), near the cape's north-east corner;
the buddy stands beside it at (99, 14). Both tiles are free of collision
and of the map's objects, clear of the Suicune scene's tiles ((99, 12),
(98, 14), and its triggers on y 16) and of the story date's (100, 17) and
(100, 18).

**Lost spots**, in authored order, each checked against the map's
collision:

| # | Spot | Tile | Found from | Hint |
| ---: | --- | --- | --- | --- |
| 0 | The grass inside the cape's west fence | (97, 18), walkable | (97, 17), (98, 18), or (97, 19) | "near the fence" |
| 1 | The cliff foot at the east end of the lower path, where the cape's rock meets the sea | (102, 22), rock | (102, 23), facing north | "by the rocks" |
| 2 | The south bank of the pond in front of Bill's house | (84, 22), walkable | (83, 22), (85, 22), or (84, 23) | "by the pond" |

There is no bench outside Bill's house, so the third spot uses the pond,
the landmark in front of the house (x 78-91, y 19-21). The active spot is
the placed trainer's catalog position mod 3
([Lost something](#lost-something)): Misty (position 1) hides it by the
rocks, Lorelei (position 9) near the fence.

**Quest.** The haunt authors:

| Line | Text |
| --- | --- |
| Quest line | "Something valuable went missing around here, {HINT}. Find it?" |
| Done line | "That's the one. Good eyes." |

`YES` gives the trainer's `YES` and `BYE`, and arms the active spot. Facing
it and pressing A finds the keepsake; the Dowsing Machine, if the player has
it, responds near it. The next talk is the greeting, the done line,
`PRAISE`, the next reward-pool entry through `GIFT`, and `BYE`, worth +10
friendship. `NO` is the trainer's `NO` and `BYE`. The search survives
leaving Route 25 and coming back, and expires when the Cape's placement
changes.

**Who's likely, and why.** The setting is remote but the Cape is not elite,
so no aloof trainer is a candidate. Scores at world progress 20 (two badges;
placeholder weights; momentum from the explorer's growth curves):

| Trainer | Theme | Style | Momentum | Hometown | Score |
| --- | ---: | ---: | ---: | ---: | ---: |
| Misty | 4 (Starmie, Water) | 2 (Field marshal) | 1 (settled) | 3 | 10 |
| Lorelei | 4 (Lapras, Water) | 0 (Sweeper) | 0 (rising) | 0 | 4 |
| Will | 2 (Slowbro, Water) | 2 (Field marshal) | 0 (rising) | 0 | 4 |

Everyone else scores 0 or 1: a settled trainer's 1 for relax and sightsee.

- **Misty** is the iconic pairing: her hometown, her Water theme, and a
  Field marshal's sightsee, plus relax while she is settled. HGSS puts her
  on the cape, on the date that HNS keeps at (100, 17) during the Power
  Plant story, and HNS's own Misty cameo stands on Route 25 at (81, 17),
  just west of Bill's house ("I love the sunset here!"). She scores 10
  while settled and 9 while rising (around world progress 34-37 and from
  41 on). Even so, v0 doesn't always seat her here: in [fill](#fill) order
  she goes to the first haunt where she is the best free candidate. Below
  TR 80 (18 active haunts, `start = wp mod 18`), Pallet Town (Water, relax)
  takes her whenever its turn comes before the Cape's, since she scores 4
  or 5 there and wins ties with Lorelei on catalog order; the Vermilion
  harbour does the same when its turn comes first. So she stands on the
  Cape when `wp mod 18` is 1-6, and never while she is in an accepted
  event lineup or is the partner.
- **Lorelei** has Water/Ice aces (Lapras, Cloyster), is from Kanto, and is
  not aloof, so the Cape scores her 4 on theme alone; as a rising Sweeper
  she gets no activity points. She holds it when Misty is away: when `wp
  mod 18` is 17 or 0 (Pallet takes Misty first, and Lorelei is the best
  free candidate left), or whenever Misty is booked into a league event or
  asked as the partner. Will ties her on 4 but comes later in catalog
  order.
- **Travellers** come only as leftovers: when `wp mod 18` is 11-16,
  Seafoam takes Lorelei and Pallet takes Misty before the Cape's turn, and
  it gets a 0- or 1-point fit such as Brawly, a traveller (or Lt. Surge
  or Bruno); when it is 7-10, all 14 candidates are placed before the Cape's
  turn and it stays empty.

What v0 can't say waits for the routine design: a pull that keeps Misty at
her own Cape rather than Pallet Town, the sunset hour the HNS cameo loves,
and a reason beyond theme for Lorelei to be here. These counts follow the
v0 weights and the current Kanto list, and move with them.

**Misty, a Friend** (world progress 20, where `20 mod 18 = 2` and she holds
the Cape with 10). The player's two badges are Brock's and Misty's, and that
win made her a Friend (20 points); no quest with her yet, so her reward
counter is 0. Her lost spot is by the rocks. At world progress 20 her TR is 30
and her team level 21, so her buddy, slot 2 (Golduck, offset −2), is Lv 19
and steps down to PSYDUCK (Golduck needs Lv 33); `{BUDDY}` says PSYDUCK
until world progress 61, where her team level reaches 35. She greets with
`HELLO`:

```text
MISTY: Hey, {PLAYER}! Took you long enough!                   HELLO
MISTY: Okay, listen up!                                       ASK
MISTY: Something valuable went missing around here, by the    quest line
       rocks. Find it?
> Yes
MISTY: Now that's what I like to hear!                        YES
MISTY: See ya! ...PSYDUCK, get back in your ball! Not again!  BYE
```

The lost spot is armed. Leaving for Cerulean and coming back changes nothing;
the player searches the rocky corner below the cape:

```text
  (the player walks down the steps to the lower path, faces
   the rock at (102, 22) from (102, 23), and presses A)
HAUNT: {PLAYER} found the lost keepsake!
```

Talking to Misty again:

```text
MISTY: Hey, {PLAYER}! Took you long enough!                   HELLO
MISTY: That's the one. Good eyes.                             done line
MISTY: Wow, not bad! You'd almost make a decent Water         PRAISE
       trainer!
MISTY: Take this MYSTIC WATER! Don't say I never gave you     GIFT
       anything!
MISTY: See ya! ...PSYDUCK, get back in your ball! Not again!  BYE
```

MYSTIC WATER is the first entry of her
[pool](../research/notable-trainer-rewards.md#misty) (from world progress
0). The quest adds 10 points (20 + 10 = 30), and she stays a Friend; from
now on every talk this placement is `HELLO`, her `QUIRK`, and `BYE`.

**Lorelei at Met: no, then yes** (world progress 36, where `36 mod 18 = 0`:
Pallet Town's turn comes first and takes Misty, and Lorelei holds the Cape
with 4). The player met her once at a haunt (+1), so she is Met, and her
reward counter is 0. Her lost spot is near the fence. Her buddy is slot 1,
Lapras, which never evolves, so `{BUDDY}` is always LAPRAS. She greets with
`AGAIN`, and the player declines first:

```text
LORELEI: Ah, you again. I am beginning to know you.           AGAIN
LORELEI: Hmph. I have something to say.                       ASK
LORELEI: Something valuable went missing around here, near    quest line
         the fence. Find it?
> No
LORELEI: Pity. I'll find someone else.                        NO
LORELEI: Come along, LAPRAS. Until next time.                 BYE
```

Declining arms nothing and costs nothing. The next talk proposes again:

```text
LORELEI: Ah, you again. I am beginning to know you.           AGAIN
LORELEI: Hmph. I have something to say.                       ASK
LORELEI: Something valuable went missing around here, near    quest line
         the fence. Find it?
> Yes
LORELEI: Good. I expected nothing less.                       YES
LORELEI: Come along, LAPRAS. Until next time.                 BYE
```

The player searches inside the cape's west fence, then talks to her:

```text
  (the player faces the grass at (97, 18), inside the west
   fence, and presses A)
HAUNT: {PLAYER} found the lost keepsake!
LORELEI: Ah, you again. I am beginning to know you.           AGAIN
LORELEI: That's the one. Good eyes.                           done line
LORELEI: Impressive. You kept your cool. I respect that.      PRAISE
LORELEI: Take this NEVER-MELT ICE. Consider it a cool         GIFT
         reward.
LORELEI: Come along, LAPRAS. Until next time.                 BYE
```

NEVER-MELT ICE is the first entry of Lorelei's
[pool](../research/notable-trainer-rewards.md#lorelei) (from world progress
0). Her next entry, a lesson, opens at world progress 45, so a quest with
her before then pays the fallback prize money. The quest adds 10 points (1 +
10 = 11), and she is still Met.

**What this example shows.** Exploration is the quest: the proposal points
at a place, the player walks the cape and its lower path looking for it,
and the Dowsing Machine helps, with no battle at all. The lost spot comes from
the placement, so Misty always hides it by the rocks and Lorelei near the
fence, and the search waits while the player comes and goes. A hometown
pull and a theme pull compete for one haunt: Misty brings both, but v0's
fill can send her to Pallet Town first, leaving the Cape to Lorelei on
theme alone, and to travellers when neither is free. And it is the calmest
mood in the list: remote, relax and sightsee, a clifftop by the sea, where
the quest is a stroll.

## Worked example: Pewter Museum

**Tags.** Region Kanto; theme Rock; not elite; hometown Pewter; activities
study and sightsee (trainers come to read up on fossils or just look
around); setting public; capacity 1; quest Quiz. Map
`PewterCity_Museum_1F_hns`. The fossil displays are two glass cases at x
3-6 on y 4 and y 7, each with a "Bones of an ancient dragon POKéMON" sign at
(4, 4) and (4, 7). The standing tile is (7, 4), the walkable tile at the east
end of the upper case; the buddy stands beside it at (8, 4). Both tiles are
free of collision and of the room's objects, and they block neither the
signs' reading tiles ((4, 3), (4, 5), (4, 6), (4, 8)) nor the way from the
doors at (14, 9) and (22, 9) to the stairs at (8, 8).

**Quest.** The Museum uses the shared [Quiz](#quiz) lines:

| Line | Text |
| --- | --- |
| Quest line | "Three questions on type matchups. Think you can answer them?" |
| Done line | "All correct. You really know your stuff." |
| Loss line | "Not quite. Brush up and try again." |

`YES` gives the trainer's `YES` and three questions on their aces at their
current stages. All three right is the done line, `PRAISE`, the next
reward-pool entry through `GIFT`, and `BYE`, worth +10 friendship; a wrong
answer is the loss line and `BYE`, with no penalty, and the quiz stays open
this placement. `NO` is the trainer's `NO`, as anywhere.

**Who's likely, and why.** The setting is public, so no aloof trainer is a
candidate. That keeps out Steven, who collects rare stones and would love
the fossils, and never comes. Scores at world progress 20 (two badges;
placeholder weights; momentum from the explorer's growth curves):

| Trainer | Theme | Style | Momentum | Hometown | Score |
| --- | ---: | ---: | ---: | ---: | ---: |
| Brock | 2 (Aerodactyl, Rock) | 2 (Field marshal) | 0 (rising) | 3 | 7 |
| Misty | 0 | 2 (Field marshal) | 1 (settled) | 0 | 3 |
| Blue | 0 | 2 (Tactician) | 0 (rising) | 0 | 2 |
| Will | 0 | 2 (Field marshal) | 0 (rising) | 0 | 2 |

Everyone else scores 0 or 1. Study and sightsee are both play-style
activities (study for Bombers, Field marshals, and Tacticians; sightsee for
Field marshals), so the Museum draws on style more than most haunts.

- **Brock** is the home fit: Pewter is his hometown, Aerodactyl gives the
  Rock theme (his signature Steelix is Steel/Ground), and as a Field
  marshal studying is his routine. He scores 7 while rising and 8 while
  settled, the Museum's best by far, so he holds it unless an earlier haunt
  in the [fill](#fill) takes him. Below TR 80 that happens when `wp mod
  18` is 5-9 (Mt. Moon outside or the Celadon rooftop) or 10-14 (Viridian
  City, Ground and sightsee, where his Steelix scores 6); the Museum is his
  when it is 15-17 or 0-4.
- **Blue** is Oak's grandson, and the play-style table already maps his
  Tactician style to study, so the Museum scores him 2 with no Rock ace at
  all. He holds it when Brock is away: when `wp mod 18` is 10-14, Viridian
  City takes Brock first, and Blue ties Will on 2 and wins on catalog
  order.
- **Blaine** is a scientist, and his Gym's quiz machines (RBY/FRLG) make him a
  natural quizmaster, but as a Gambler with Fire aces he scores 0, or 1
  while settled. He is from Kanto, so v0 lets him in; that Cinnabar is far
  away and he doesn't travel isn't modelled. He could only come as a
  leftover, and below TR 80 the fill never seats him here.

What v0 can't say waits for the routine design: distance from home for
Blaine, a scholarly pull for Blue beyond his play style, and a pull that
keeps Brock at his own town's museum rather than Viridian City. When `wp
mod 18` is 5-8 every candidate is placed before the Museum's turn and it
stays empty; at 9 it gets a leftover (Brawly). These counts follow the v0
weights and the current Kanto list, and move with them.

**Brock, a Friend** (world progress 20, where `20 mod 18 = 2`: Viridian
City and Viridian Forest go to Giovanni and Bugsy, and the Museum's turn
gives Brock 7). The player beat him at Pewter, which made him a Friend (20
points); no quest with him yet, so his reward counter is 0. At world
progress 20 his TR is 35 and his team level 25, and his team of three is
ONIX, GEODUDE, and GOLBAT: his Aerodactyl slot hasn't joined yet, so his
only ace on the team is his signature Steelix, stepped down to ONIX (Lv 25;
Steelix needs Lv 35). All three questions are about ONIX (Rock/Ground), one
per template, following the [generation rules](#quiz):

| # | Template | Correct | Options (multiplier against ONIX) |
| ---: | --- | --- | --- |
| 1 | weakness | FIGHTING | NORMAL (0.5), FIGHTING (2), FLYING (0.5), POISON (0.25) |
| 2 | resistance | NORMAL | NORMAL (0.5), FIGHTING (2), GROUND (2), STEEL (2) |
| 3 | immunity | ELECTRIC | FIGHTING (2), GROUND (2), STEEL (2), ELECTRIC (0) |

Water and Grass hit ONIX four times over, but Fighting comes first in
chart order, so it is the answer. He greets with `HELLO`:

```text
BROCK: Hey, {PLAYER}! Good to see you. Eating well, I hope?   HELLO
BROCK: Hey, I've got an idea.                                 ASK
BROCK: Three questions on type matchups. Think you can        quest line
       answer them?
> Yes
BROCK: Great! That's the spirit!                              YES
BROCK: What type hits ONIX super effectively?                 question 1
  [NORMAL / FIGHTING / FLYING / POISON]
> FIGHTING
BROCK: What does ONIX's type resist?                          question 2
  [NORMAL / FIGHTING / GROUND / STEEL]
> NORMAL
BROCK: Which move type can't touch ONIX?                      question 3
  [FIGHTING / GROUND / STEEL / ELECTRIC]
> ELECTRIC
BROCK: All correct. You really know your stuff.               done line
BROCK: Nicely done! That's rock-hard willpower if I ever saw  PRAISE
       it.
BROCK: Take this PEWTER CRUNCHIES. A good breeder always      GIFT
       shares supplies.
BROCK: Take care! And keep your POKéMON well fed!             BYE
```

PEWTER CRUNCHIES is the first entry of his
[pool](../research/notable-trainer-rewards.md#brock) (from world progress
0). The quest adds 10 points (20 + 10 = 30), and he stays a Friend. The
questions teach the matchups of the ONIX the player just faced at his Gym.

**Blue at Met: a wrong answer, then all right** (world progress 30, where
`30 mod 18 = 12`: Viridian City's turn comes first and takes Brock with 6,
and Blue holds the Museum with 2). The player met him once at a haunt (+1)
and hasn't beaten him, so he is Met, and his reward counter is 0. At world
progress 30 his TR is 37 and his team level 26, and his team is PIDGEOTTO,
KADABRA, and EEVEE in battle order: his aces are KADABRA (Alakazam, Lv 26,
stepped down below Lv 42) and EEVEE (his signature Umbreon, Lv 26, stepped
down below Lv 30). Their
battle order, KADABRA then EEVEE, repeats to KADABRA, EEVEE, KADABRA:

| # | Ace | Template | Correct | Options (multiplier) |
| ---: | --- | --- | --- | --- |
| 1 | KADABRA (Psychic) | weakness | BUG | NORMAL (1), FIGHTING (0.5), BUG (2), PSYCHIC (0.5) |
| 2 | EEVEE (Normal) | weakness, for resistance | FIGHTING | NORMAL (1), FIGHTING (2), FLYING (1), GHOST (0) |
| 3 | KADABRA (Psychic) | resistance, for immunity | FIGHTING | FIGHTING (0.5), BUG (2), GHOST (2), DARK (2) |

A pure Normal type resists nothing, so question 2 falls back to the
weakness template; a Psychic type is immune to nothing, so question 3 falls
back to resistance, the first template not yet asked about KADABRA. KADABRA
resists only Fighting and Psychic, so question 1's third wrong option is a
neutral NORMAL. He greets with `AGAIN`, and the player
slips on question 2:

```text
BLUE: Oh, you again. Keep this up and I'll remember you!      AGAIN
BLUE: Hey, {PLAYER}. A second?                                ASK
BLUE: Three questions on type matchups. Think you can answer  quest line
      them?
> Yes
BLUE: Heh, knew you'd say yes!                                YES
BLUE: What type hits KADABRA super effectively?               question 1
  [NORMAL / FIGHTING / BUG / PSYCHIC]
> BUG
BLUE: What type hits EEVEE super effectively?                 question 2
  [NORMAL / FIGHTING / FLYING / GHOST]
> NORMAL
BLUE: Not quite. Brush up and try again.                      loss line
BLUE: Smell ya later!                                         BYE
```

No penalty and no change: the claim bit stays clear, his counter stays at
0, and no points change. The next talk asks the same questions, since his
stages haven't changed:

```text
BLUE: Oh, you again. Keep this up and I'll remember you!      AGAIN
BLUE: Hey, {PLAYER}. A second?                                ASK
BLUE: Three questions on type matchups. Think you can answer  quest line
      them?
> Yes
BLUE: Heh, knew you'd say yes!                                YES
BLUE: What type hits KADABRA super effectively?               question 1
  [NORMAL / FIGHTING / BUG / PSYCHIC]
> BUG
BLUE: What type hits EEVEE super effectively?                 question 2
  [NORMAL / FIGHTING / FLYING / GHOST]
> FIGHTING
BLUE: What does KADABRA's type resist?                        question 3
  [FIGHTING / BUG / GHOST / DARK]
> FIGHTING
BLUE: All correct. You really know your stuff.                done line
BLUE: Not bad! ...For someone who isn't me, anyway.           PRAISE
BLUE: Take this SILK SCARF. Gramps says I should share. Ugh.  GIFT
BLUE: Smell ya later!                                         BYE
```

SILK SCARF is the first entry of Blue's
[pool](../research/notable-trainer-rewards.md#blue) (from world progress
0); his next, BLACK GLASSES, is open too (from world progress 20). The quest
adds 10 points (1 + 10 = 11), and he is still Met.

**What this example shows.** Knowledge is the challenge: the quiz is about
the POKéMON the trainer actually uses right now, so it teaches the matchups
the player will need against them, and it changes as their team grows.
Studying is a routine step, drawn from play style: Brock comes for his
hometown and Rock theme, but Blue comes on his Tactician style alone, the
Museum as the home of Oak's grandson, with no Rock ace at all. A public,
indoor place keeps every aloof trainer out before any score is read, which
is why Steven, the one trainer who would travel for fossils, never visits.

## Worked example: Viridian Forest

**Tags.** Region Kanto; themes Bug and Grass; not elite; no hometown;
activities study and sightsee (trainers come to watch the forest's bugs or
just wander it); setting remote; capacity 1; quest Catch me one, `{LOCAL}`
PIKACHU. Map `ViridianForest_hns`. The standing tile is (39, 53) on the
strip of short grass (x 39-40, y 50-57) that runs through the tall-grass
clearing south of the forest's centre (x 25-42, y 49-60); the buddy stands
beside it at (40, 53). Both tiles are free of collision and of the map's
objects, outside every wandering trainer's range and clear of the Pichu
scene triggers. HNS's Bugsy cameo stands at (37, 30), on the path north of
the clearing ("I came to KANTO to look for BUG-TYPE POKéMON").

**Quest.** PIKACHU is on the forest's own land tables,
`gViridianForest_hns_Day` and `gViridianForest_hns_Night` in
[wild_encounters.json](../../game/src/data/wild_encounters.json), at Lv 3
among Caterpie, Weedle, Metapod, Kakuna, and Spinarak. It fills slots 1
and 9, worth 20% and 4% of land encounters, so 24% day and night: about one
encounter in four in the clearing's grass. The haunt authors:

| Line | Text |
| --- | --- |
| Quest line | "A wild {LOCAL} lives around here. Catch one and show me?" |
| Done line | "That's a fine one. Thanks for showing me." |

`YES` gives the trainer's `YES` and `BYE`. The next talk with a PIKACHU in
the party or the boxes opens the storage screen with everything else
dimmed; showing it plays the done line, `PRAISE`, the next reward-pool
entry through `GIFT`, and `BYE`, worth +10 friendship, and the player keeps
the PIKACHU. With none to show, it is `NOT_READY` and `BYE`. `NO` is the
trainer's `NO`. The quest stays open all placement.

**Who's likely, and why.** The setting is remote but the forest is not
elite, so no aloof trainer is a candidate. Scores at world progress 20 (two
badges; placeholder weights; momentum from the explorer's growth curves):

| Trainer | Theme | Style | Momentum | Score |
| --- | ---: | ---: | ---: | ---: |
| Bugsy | 4 (Scizor, Bug) | 0 (Sweeper) | 1 (settled) | 5 |
| Erika | 4 (Vileplume, Grass) | 0 (Hexer) | 0 (rising) | 4 |
| Janine | 4 (Venomoth, Bug) | 0 (Hexer) | 0 (rising) | 4 |
| Misty | 0 | 2 (Field marshal) | 1 (settled) | 3 |

Brock, Blue, and Will score 2 on study or sightsee; everyone else 0 or 1.

- **Bugsy** is a Johto traveller with Bug aces (Scizor and Heracross), and
  the HNS cameo already puts him here. While he is settled he scores 5 to
  Erika's 4, so he holds the forest whenever he is free at its turn: below
  TR 80, world progress 10-21 and 28-39 (`wp mod 18` 10-17 or 0-3). From
  about world progress 44 he is rising, 4 like Erika, and ties go to her,
  earlier in catalog order, so later he holds it only while she is placed
  elsewhere first (world progress 45-48 and 62-66, when the Celadon
  rooftop takes her).
- **Erika** has Grass aces (Vileplume and Victreebel) and is from Kanto.
  She holds the forest when Bugsy isn't placed there: world progress 0-3,
  where both are settled on 5 and she wins the tie, and 49-57 and 67-75.
  Janine's Venomoth scores the same as Erika, but Janine comes later in
  catalog order and v0 never seats her here below TR 80.
- **Others** come only as leftovers: Brawly or Blue at world progress 8-9,
  26-27, and 44. When `wp mod 18` is 4-7 every candidate is placed before
  the forest's turn and it stays empty.

What v0 can't say waits for the routine design: Bugsy's research trip as a
reason to prefer the forest over any Bug-friendly haunt, Erika's taste for
gardens over a wild wood, and why Janine, Kanto's other Bug user, would
come. These counts follow the v0 weights and the current Kanto list, and
move with them.

**Bugsy, a Friend** (world progress 20, where `20 mod 18 = 2` and he holds
the forest with 5). The player beat him at the Azalea Gym, which made him a
Friend (20 points); no quest with him yet, so his reward counter is 0. At
world progress 20 his TR is 28 and his team level 20, so his buddy, slot 1
(Scizor), is Lv 20 and steps down to SCYTHER (Scizor needs Lv 40). He
greets with `HELLO`:

```text
BUGSY: Hi, {PLAYER}! Seen any interesting Bug POKéMON         HELLO
       lately?
BUGSY: Oh! Hey, hey, wait a second!                           ASK
BUGSY: A wild PIKACHU lives around here. Catch one and show   quest line
       me?
> Yes
BUGSY: Yes! Science thanks you! I do, too!                    YES
BUGSY: Bye! If you spot a rare bug, tell me first!            BYE
```

The player walks into the clearing's tall grass and catches a PIKACHU
(Lv 3, about one encounter in four), then talks to him again:

```text
BUGSY: Hi, {PLAYER}! Seen any interesting Bug POKéMON         HELLO
       lately?
  (the storage screen opens over the party and boxes, with
   everything but PIKACHU dimmed; the player picks the
   PIKACHU, which stays in the party)
BUGSY: That's a fine one. Thanks for showing me.              done line
BUGSY: Wow! That was amazing! Can I write it down?            PRAISE
BUGSY: Take this SILVER POWDER! I found it during my          GIFT
       research.
BUGSY: Bye! If you spot a rare bug, tell me first!            BYE
```

SILVER POWDER is the first entry of his
[pool](../research/notable-trainer-rewards.md#bugsy) (from world progress
0). The quest adds 10 points (20 + 10 = 30), and he stays a Friend. Talking
to him again during this placement:

```text
BUGSY: Hi, {PLAYER}! Seen any interesting Bug POKéMON         HELLO
       lately?
BUGSY: I measure SCYTHER every day. For research! It hates    QUIRK
       that.
BUGSY: Bye! If you spot a rare bug, tell me first!            BYE
```

His buddy is slot 1, so the `QUIRK` names SCYTHER until his team level
reaches 40.

**Erika at Met** (world progress 50, where `50 mod 18 = 14`: Bugsy and
Erika both score 4, and she wins the tie on catalog order). The player met
her once at a haunt (+1), so she is Met, and her reward counter is 0. The
PIKACHU caught for Bugsy is in a box by now. She greets with `AGAIN`:

```text
ERIKA: Oh, hello again. I remember you. Please, relax a       AGAIN
       while.
ERIKA: Might I suggest something?                             ASK
ERIKA: A wild PIKACHU lives around here. Catch one and show   quest line
       me?
> Yes
ERIKA: How kind of you. Thank you ever so much.               YES
ERIKA: Farewell. Oh... I think I'll close my eyes a           BYE
       moment...
```

The asked bit is set. The next talk opens the storage screen at once:

```text
ERIKA: Oh, hello again. I remember you. Please, relax a       AGAIN
       while.
  (the storage screen opens; the player picks the PIKACHU
   in box 2, which stays there)
ERIKA: That's a fine one. Thanks for showing me.              done line
ERIKA: Oh my, how splendid! You have a gardener's patience.   PRAISE
ERIKA: Please accept this ROSE INCENSE. A small gift, but     GIFT
       sincere.
ERIKA: Farewell. Oh... I think I'll close my eyes a           BYE
       moment...
```

ROSE INCENSE is the first entry of Erika's
[pool](../research/notable-trainer-rewards.md#erika) (from world progress
0). The quest adds 10 points (1 + 10 = 11), and she is still Met. A player
with no PIKACHU would have heard her `NOT_READY` instead ("Perhaps not
yet. Patience helps flowers bloom, too.") and could come back after
catching one.

**What this example shows.** Catching is the challenge: the ask sends the
player into the grass for a species they may not have yet, which pushes
the POKéDEX along, and they keep what they catch. The species comes from
the haunt's own wild table, so the ask always fits the place and never
asks for something that doesn't live there. The forest is a traveller's
natural home: Bugsy, from Johto, turns up where the HNS cameo already had
him, on theme alone. And it gives the player a reason to linger in an
early area, and to come back to it later with a new trainer in the
clearing.

With this example, the five worked haunts cover the first five quest
types: Walk with me (Diglett's Cave), One on one (Celadon Game Corner), Lost
something (Cerulean Cape), Quiz (Pewter Museum), and Catch me one
(Viridian Forest). Bring me, Swap battle, Trade, Wanted, Egg sitting,
Courier, and Handicap have no worked haunt yet.

## Kanto haunts

The v0 Kanto list, in catalog order. It is a draft: the tags, quests, and
species are content for review. Every map exists under
`game/data/maps/`; Seafoam and Cinnabar use the FRLG port's maps, since
Wayfarer retired their HNS versions
([retirement](frlg-cinnabar-seafoam-hns-retirement.md)). Standing tiles, exits,
and lost-spot tiles are chosen at implementation after checking collision and
existing objects; the
[Celadon Game Corner's](#worked-example-celadon-game-corner),
[Cerulean Cape's](#worked-example-cerulean-cape),
[Pewter Museum's](#worked-example-pewter-museum), and
[Viridian Forest's](#worked-example-viridian-forest) are worked out
already.
Every haunt has capacity 1.

| # | Haunt | Maps | Themes | Elite | Hometown | Activities | Setting | Quest |
| ---: | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Pallet Town | `PalletTown_hns` | Water | – | – | relax | public | Catch me one: Krabby |
| 2 | Route 1 | `Route1_hns` | Normal, Flying | – | – | train | public | Walk with me: from the Pallet end to the Viridian City edge |
| 3 | Viridian City | `ViridianCity_hns` | Ground | – | Viridian | sightsee | public | One on one |
| 4 | Viridian Forest | `ViridianForest_hns` | Bug, Grass | – | – | study, sightsee | remote | Catch me one: Pikachu |
| 5 | Pewter Museum | `PewterCity_Museum_1F_hns` | Rock | – | Pewter | study, sightsee | public | Quiz |
| 6 | Mt. Moon outside | `MtMoon_Outside_hns` | Rock, Fairy | – | – | sightsee | remote | Lost something |
| 7 | Cerulean Cape | `Route25_hns` | Water | – | Cerulean | relax, sightsee | remote | Lost something: near the fence, by the rocks, by the pond |
| 8 | Rock Tunnel | `RockTunnel_B1F_hns`, `RockTunnel_1F_hns` | Rock, Fighting | – | – | train | remote | Walk with me: from the north Route 10 entrance to the south one |
| 9 | Power Plant | `Route10_PowerPlantEntrance_hns`, `PowerPlant_Frlg` | Electric | – | – | study | remote | Catch me one: Voltorb |
| 10 | Lavender Soul House | `LavenderTown_SoulHouse_hns` | Ghost | – | – | lie low | public | Quiz |
| 11 | Vermilion harbour | `VermilionCity_PortOutside_hns` | Water, Electric | – | Vermilion | sightsee | public | Catch me one: Chinchou |
| 12 | Celadon Game Corner | `CeladonCity_GameCorner_hns` | – | – | – | gamble | public | One on one |
| 13 | Celadon rooftop | `CeladonCity_DepartmentStore_RoofDay_hns`, `CeladonCity_DepartmentStore_RoofNight_hns` | Grass | – | Celadon | sightsee | public | Lost something |
| 14 | Saffron Fighting Dojo | `SaffronCity_FightingDojo_hns` | Fighting | – | Saffron | train | public | One on one |
| 15 | Dojo back room | `SaffronCity_FightingDojoVIP_hns` | Psychic | TR 80 | Saffron | study | remote | Quiz |
| 16 | Diglett's Cave | `DiglettsCave_EntranceNorth_hns`, `DiglettsCave_Tunnel_hns`, `DiglettsCave_EntranceSouth_hns` | Ground | – | – | train, lie low | remote | Walk with me: from the Route 2 entrance to the Vermilion exit |
| 17 | Safari Zone | `FuchsiaCity_SafariZoneEntrance_hns` and its Beach, Brush, Cave, and Mountain areas | Normal, Poison | – | Fuchsia | sightsee | public | Catch me one: Kangaskhan |
| 18 | Seafoam Islands | `SeafoamIslands_1F_Frlg` | Ice, Water | – | – | train | remote | One on one |
| 19 | Cinnabar shore | `CinnabarIsland_Frlg` | Fire | – | Cinnabar | relax | public | Quiz |
| 20 | Victory Road | `VictoryRoadKanto_1F_hns`, `VictoryRoadKanto_B1F_hns`, `VictoryRoadKanto_B2F_hns` | Rock, Fighting, Dragon | TR 80 | – | train | remote | Walk with me: from the Route 23 entrance to the Reception Gate exit |
| 21 | Cerulean Cave | `CeruleanCave_1F_hns`, `CeruleanCave_B1F_hns`, `CeruleanCave_B2F_hns` | Psychic | TR 80 | – | train | remote | Catch me one: Ditto |
| 22 | Indigo Plateau Pokémon Center | `IndigoPlateau_PokemonCenter_hns` | – | TR 80 | – | relax | public | Quiz |

Notes:

- **Names.** The Haunt column is each haunt's `Name` tag, in capitals in
  game ("CERULEAN CAPE"). Two exceed 20 characters and shorten: Saffron
  Fighting Dojo to "FIGHTING DOJO", and Indigo Plateau Pokémon Center to
  "INDIGO PLATEAU".
- **Sabrina's haunt.** Sabrina is aloof, so she can only be placed at a
  remote elite haunt. The Dojo back room (the Dojo's rematch room today) is
  Saffron's: its Psychic theme and Saffron hometown make her its best fit
  from TR 80, and it absorbs the rematch room.
- **Aloof trainers in Kanto** can only reach the Dojo back room, Victory
  Road, and Cerulean Cave. Cerulean Cave's Catch me one is open to all of
  them (Agatha, Sabrina, and Glacia), and the Indigo Plateau Pokémon Center
  is public, so no aloof trainer goes there.
- **Catch species** come from each haunt's current wild table
  (`game/src/data/wild_encounters.json`): Krabby (Pallet's water), Pikachu,
  Voltorb (the old generating hall), Chinchou, Kangaskhan, and Ditto.
  Recheck them against [Kanto wild encounters](kanto-wild-encounters.md)
  before content lands.
- **Retired cameos.** The cameos at Diglett's Cave (Brock), Route 25
  (Misty), Route 10 (Lt. Surge), Celadon City (Erika), the Reception Gate
  (Janine), and Cinnabar (Blaine) fall inside or next to these haunts.

## Engine notes

Verified facts about the current build that the design depends on.

### Travel and on-map walking

A proof of concept ([research](../research/notable-trainer-travel-poc.md),
closed PR [#146](https://github.com/mzpkdev/pokemon-wayfarer/pull/146)) ran
one notable trainer in the real ROM. It showed that the design direction
beyond v0's static placement is viable: a small AI with a bounded
breadth-first search over the engine's collision checks walks one map, and a
12-byte saved world record per trainer, advanced by a roamer-style heartbeat
on map changes, lets them really travel between maps. Following them across
an edge and finding them again after lingering both held, and so did save
and reload.

Its constraints shape any travel built on haunts:

- the travel graph needs walkable regions within maps, not whole maps
  (Route 2 is split and gated by Cut);
- heartbeat time only moves on map changes, so dwelling and interiors need a
  gameplay timer;
- about 0.6 of a frame per search slice and a 16-slot object pool fit two or
  three walkers per map, with a shared search schedule;
- doors need one collision exception;
- the proof of concept disabled the player's following POKéMON.

### Follower NPCs

- The engine's follower NPCs are compiled out: `FNPC_ENABLE_NPC_FOLLOWERS`
  is `FALSE` in
  [follower_npc.h](../../game/include/config/follower_npc.h). Enabling them
  adds `struct NPCFollower` to SaveBlock3
  ([global.h](../../game/include/global.h), under
  `#if FNPC_ENABLE_NPC_FOLLOWERS`), which costs save space.
- Wild partner battles are toggled by the flag that
  `FNPC_FLAG_PARTNER_WILD_BATTLES` names; it is 0 today, so they are off,
  and haunts need it to name a real flag. With
  `FNPC_NPC_FOLLOWER_WILD_BATTLE_VS_2` `TRUE`, a follower partner brings two
  wild POKéMON (`TryDoDoubleWildBattle` in
  [wild_encounter.c](../../game/src/wild_encounter.c)). A walk sets that
  flag while it is in progress.
- `PrepareForFollowerNPCBattle` in
  [follower_npc.c](../../game/src/follower_npc.c) saves the party, takes the
  player's first three able POKéMON, and fills the partner party from
  `TRAINER_PARTNER(battlePartner)`; `RestorePartyAfterFollowerNPCBattle`
  restores it. The walk points that partner at the runtime partner slot.
- Follower flags in
  [constants/follower_npc.h](../../game/include/constants/follower_npc.h)
  gate the player: without `FOLLOWER_NPC_FLAG_CAN_LEAVE_ROUTE` the player
  cannot use Fly, Teleport, or an Escape Rope. With
  `FOLLOWER_NPC_FLAG_CLEAR_ON_WHITE_OUT`, a whiteout clears the follower
  (`FollowerNPC_TryRemoveFollowerOnWhiteOut`, called from
  [overworld.c](../../game/src/overworld.c)); without it, the follower
  reappears after the warp. Walks set it.
- Every notable trainer already has an overworld graphic
  (`OBJ_EVENT_GFX_*` in
  [event_objects.h](../../game/include/constants/event_objects.h)).

### Partner team

A walk's partner battles use the **runtime partner slot** of
[Sevii Masters](sevii-masters.md#tag-matches): its trainer struct and
three-member party are filled for each battle from the trainer's battle
snapshot and presentation, and no static partner entry changes. The slot is
free during a walk, since Masters tag matches never run in the overworld.

### Storage-screen modes

The storage-screen selection modes are the `sPcMonSelectionTypes` table in
[chooseboxmon.c](../../game/src/chooseboxmon.c), named by `SELECT_PC_MON_*`
in [party_menu.h](../../game/include/constants/party_menu.h). Catch me one
reuses `SELECT_PC_MON_TRADE`, the mode Jasmine's Steelix trade uses today
([Olivine Gym](../../game/data/maps/OlivineCity_Gym_hns/scripts.inc)),
only to pick and show; nothing is traded. Its filter, `IsMatchingSpecies`,
matches the species in `gSpecialVar_0x8009` by `MON_DATA_SPECIES_OR_EGG`,
so Eggs never match; the storage screen draws every excluded POKéMON
transparent (`ShouldBoxmonSpriteBeTransparent` in
[pokemon_storage_system.c](../../game/src/pokemon_storage_system.c)), which
is the dimming, and picking one returns `VAR_RESULT` `FALSE`. The filter
compares exact species, so "any form" needs a variant that compares the
base species. `SELECT_PC_MON_MOVE_TUTOR` filters by one move
(`gSpecialVar_0x8005`), using the species' teachable list
(`CanMonLearnMove`). A lesson needs a filter over the whole move pool with
the [lesson](#rewards-and-claims) eligibility, so it needs a variant of
that mode that also picks the move.

### Bag kind screen

- **Today.** `special Bag_ChooseBerry` (`Bag_ChooseBerry` in
  [berry.c](../../game/src/berry.c)) opens `CB2_ChooseBerry` in
  [item_menu.c](../../game/src/item_menu.c):
  `GoToBagMenu(ITEMMENULOCATION_BERRY_TREE, POCKET_BERRIES, ...)`. For that
  location, and for `ITEMMENULOCATION_BERRY_BLENDER_CRUSH` and
  `ITEMMENULOCATION_BERRY_TREE_MULCH`, `GoToBagMenu` sets
  `pocketSwitchDisabled`. The locations are the `ITEMMENULOCATION_*` enum
  in [item_menu.h](../../game/include/item_menu.h). A pick leaves the item
  in `gSpecialVar_ItemId` (`VAR_ITEM_ID`); cancelling sets it to
  `ITEM_NONE`. The Route 14 Pidgeotto
  ([scripts](../../game/data/maps/Route14_hns/scripts.inc)) runs
  `special Bag_ChooseBerry`, `waitstate`, then `removeitem VAR_ITEM_ID, 1`
  before it checks the berry, so a wrong berry is lost there.
- **Refusing in place.** The mulch location already refuses a wrong item
  without closing: `Task_FadeAndCloseBagMenuIfMulch` closes the Bag only
  for the eight mulches and otherwise prints a can't-use message
  (`DisplayDadsAdviceCannotUseItemMessage`), leaving the list open.
- **Needed.** A new location, `ITEMMENULOCATION_HAUNT_KIND`, and a special
  `Bag_ChooseItemOfKind` that reads the kind from a script variable, opens
  the Bag on that kind's pocket with pocket switching off, and closes on a
  pick only if the item belongs to the kind (the mulch pattern with the
  kind table as the test), printing "That's not a {KIND}." otherwise. A
  check `HasItemOfKind` answers the none-owned case before the screen
  opens. The script removes the item with `removeitem VAR_ITEM_ID, 1` only
  after the pick passed, never before as Route 14 does.

### Swap battle engine

- **Parking.** `SavePlayerParty` in
  [load_save.c](../../game/src/load_save.c) copies the live party and its
  count into the save block's party (`SavePlayerPartyMon` in
  [pokemon.c](../../game/src/pokemon.c) writes
  `gSaveBlock1Ptr->playerParty`), and `LoadPlayerParty` copies them back.
  Tag battles park this way: `PrepareForFollowerNPCBattle` and
  `RestorePartyAfterFollowerNPCBattle` in
  [follower_npc.c](../../game/src/follower_npc.c), and the Masters
  reference flow ([tag matches](sevii-masters.md#tag-matches)).
- **Factory rentals.** The Battle Factory lobby runs
  `special SavePlayerParty` when the attendant starts a challenge, then
  `SetPlayerAndOpponentParties` in
  [battle_factory.c](../../game/src/battle_factory.c) zeroes the party and
  builds the rentals into `gPlayerParty` with `CreateFacilityMon`, under
  `READ_OTID_FROM_SAVE`, so rentals carry the player's ID. After a
  challenge the lobby runs `special LoadPlayerParty` and
  `callnative UpdateFollowingPokemon`
  ([lobby](../../game/data/maps/BattleFrontier_BattleFactoryLobby/scripts.inc)).
  Swap battle copies the park and restore, and refreshes the follower the
  same way, but not the OT: the ace must read as someone else's.
- **Building the ace into the party.** `CreateTrainerPartyForPlayer` in
  [battle_main.c](../../game/src/battle_main.c) already writes a trainer's
  party into `gPlayerParty`. The ace needs the same, from the battle
  snapshot, into slot 0 alone. NPC trainer POKéMON get a random OT ID but
  the player's OT name (`CreateMon` in
  [pokemon.c](../../game/src/pokemon.c) sets `MON_DATA_OT_NAME` to
  `playerName`), so the build sets `MON_DATA_OT_NAME` to the trainer's name
  and `MON_DATA_OT_ID` to a fixed ID per trainer, flipped by one bit if it
  equals the player's.
- **The enemy party from a copy.** `CreateNPCTrainerParty` runs for every
  non-link trainer battle and rewrites `gEnemyParty` from the trainer's
  data (`CB2_InitBattleInternal` in
  [battle_main.c](../../game/src/battle_main.c)); it returns early only for
  `TRAINER_SECRET_BASE`, a partner ID, or an unknown trainer, which is how
  a secret base battle keeps its prebuilt party
  (`CreateSecretBaseEnemyParty` in [pokemon.c](../../game/src/pokemon.c)).
  The union room writes `gEnemyParty[i]` straight from `gPlayerParty`
  copies ([union_room_battle.c](../../game/src/union_room_battle.c)). Swap
  battle needs a flag that skips `CreateNPCTrainerParty` for one battle
  while keeping the notable trainer's identity (name, class, pics, AI
  flags), with `gEnemyParty[0]` set from a `struct Pokemon` copy of the
  lead beforehand.
- **Obedience.** `GetAttackerObedienceForAction` in
  [battle_util.c](../../game/src/battle_util.c): AI battlers always obey,
  and so does any POKéMON in a `BATTLE_TYPE_FRONTIER` battle. Under
  `IS_WAYFARER` the obedience level is `GetTrainerRatingSoftLevelCap()`
  ([trainer_rating.c](../../game/src/trainer_rating.c)), with only Eggs
  exempt. The level checked is the POKéMON's met level when
  `IsOtherTrainer` ([pokemon.c](../../game/src/pokemon.c)) says it is the
  player's own (same OT ID and name), and its current level when it is
  someone else's. A level above the cap then risks disobeying at random,
  more often the further above it is. So the ace, with the trainer's OT,
  is checked at its current level against the player's cap, and the
  trainer's copy of the pick, under AI, always obeys.
- **Experience.** `Cmd_getexp` in
  [battle_script_commands.c](../../game/src/battle_script_commands.c)
  skips experience when `FLAG_DISABLE_EXP_GAIN`
  ([flags.h](../../game/include/constants/flags.h)) is set; the swap sets
  it for the battle and clears it at the restore. The Frontier battle
  type would also stop experience (`BattleTypeAllowsExp`), but it would
  also force obedience, so it is not used.
- **Catching.** A ball thrown in a trainer battle is blocked
  (`BattleScript_TrainerBallBlock` in
  [battle_script_commands.c](../../game/src/battle_script_commands.c)),
  so nothing extra is needed.
- **Pokédex.** At the end of a battle, `battle_main.c` marks every
  sent-out enemy POKéMON as seen (`HandleSetPokedexFlagFromMon` with
  `FLAG_SET_SEEN`), and skips the player's side while
  `B_PARTNER_MONS_MARKED_SEEN` is `FALSE`
  ([config/battle.h](../../game/include/config/battle.h)). The enemy here
  is the player's own species, so nothing new is recorded, and the ace is
  never marked seen through this battle.
- **Risks.**
  - *Gimmicks.* The snapshot may give the ace a gimmick (Mega Evolution,
    Tera) that the trainer's data enables; in the player's hands it would
    depend on the player's key items instead. Neither side uses a gimmick
    in a Swap battle; the ace keeps its held item.
  - *Held items and abilities.* The pick's copy is a whole
    `struct Pokemon` copy, so its ability number, personality, nature,
    IVs, EVs, moves, PP, friendship, and held item carry over exactly; a
    form tied to a held item stays as it is. The ace carries what the
    snapshot resolves.
  - *Names.* The copy keeps its nickname and the player's OT name, so the
    battle shows the trainer sending out the player's nickname ("BROCK
    sent out SPARKY"); this is intended, since it is the player's POKéMON
    on loan. The ace shows its species name.
  - *Post-battle effects.* Anything the engine does to `gPlayerParty` as
    the battle ends (friendship loss on fainting, Pickup, Pokérus spread)
    touches only the ace, which the restore discards. The restore must run
    before any post-battle evolution check; with experience off, none
    triggers.
  - *Whiteout.* A loss must not black out: the party is a lone fainted
    ace. The engine skips the whiteout when `B_FLAG_NO_WHITEOUT` names a
    set flag ([battle_setup.c](../../game/src/battle_setup.c)); it is 0
    today, so this needs the same no-blackout route as One on one.

### Trade engine

- **The scene.** `DoInGameTradeScene` in [trade.c](../../game/src/trade.c)
  starts `CB2_InitInGameTrade`, which shows `gEnemyParty[0]` as the
  partner's POKéMON and takes the partner's name from its OT name. The
  scene's `TradeMons` swaps the player's POKéMON and `gEnemyParty[0]`
  whole, clears the given POKéMON's mail, and sets the received one's
  friendship to 70; then the in-game trade path checks a trade evolution
  (`GetEvolutionTargetSpecies` with `EVO_MODE_TRADE`) on the received
  POKéMON. So the given POKéMON sits in `gEnemyParty[0]` after the scene,
  where the record is read from.
- **Building the filler.** `CreateInGameTradePokemon` calls
  `CreateInGameTradePokemonInternal`, which builds `gEnemyParty[0]` from
  the static `sIngameTrades` table at the level of the selected POKéMON
  (`GetLevelFromBoxMonExp` on `GetSelectedBoxMonFromPcOrParty()`), with
  `GiveMonInitialMoveset` and `METLOC_IN_GAME_TRADE`. Trade needs a variant
  that takes the species, personality, OT, ability, nature, and IVs from
  the trainer's [offer](#accepted) instead of the table, keeping the level
  rule; the trade-back builds the player's POKéMON from its record the
  same way. `MON_DATA_OT_NAME` holds `PLAYER_NAME_LENGTH` (7) characters,
  shorter than a trainer name (`TRAINER_NAME_LENGTH`, 10), so long trainer
  names are cut. The OT gender comes from the trainer's `gender` bit
  (`struct Trainer` in [data.h](../../game/include/data.h)).
- **Party-only pick.** Every storage-screen mode opens the party and the
  boxes together while `OW_CHOOSE_FROM_PC_AND_PARTY` is `TRUE`
  ([config/overworld.h](../../game/include/config/overworld.h)); the
  screen tests party members by `&gPlayerParty[i].box`. Trade needs a new
  `SELECT_PC_MON_*` mode whose filter excludes any POKéMON outside
  `gPlayerParty`, an Egg, the last able member, a POKéMON with none of the
  trainer's ace types (for Wanted, any not of the wanted species; for a
  trade-back, any but the received filler),
  and is strict (`isStrict`), so an excluded one can't be picked, as
  the Day Care mode does.
- **Line values.** `GetTotalBaseStat` in
  [battle_ai_util.c](../../game/src/battle_ai_util.c) sums the six base
  stats; `GetSpeciesEvolutions` ([pokemon.h](../../game/include/pokemon.h))
  lists a species' evolutions. Megas and other in-battle forms are form
  changes, not evolutions, and the species flags `isMegaEvolution`,
  `isPrimalReversion`, `isUltraBurst`, `isGigantamax`, and `isTeraForm`
  mark the rest. A host generator next to the trainer scaling tools
  (`game/tools/trainer_scaling/line_values.py`) walks the built species
  data and emits one `u16` line value per species into a generated header,
  about 3 KB of ROM; the build regenerates it, and a test checks it against
  the species data, so a stat or evolution change can't leave it stale.
- **Wanted species.** The same generator emits each Wanted haunt's local
  species ([the wanted species](#the-wanted-species)) in wanted order, line
  value descending and then species number ascending, read from the wild
  headers the build emits for its maps. At runtime the first entry that
  shares a type with the trainer's aces is the wanted species, else the
  first entry. An empty list fails the build, and a test checks the lists
  against the wild encounter data.
- **Where the records live.** SaveBlock1 measures 15,760 of its 15,872
  bytes, leaving 112; SaveBlock2 3,892 of 3,968; SaveBlock3 1,160 of its
  1,624 (`WayfarerSaveBlock3SectorAllocation` in
  [save.c](../../game/src/save.c)), and the rest of SaveBlock3 is wanted by
  the other haunt and notable-trainer state and by follower NPCs.
  `struct PokemonStorage`
  ([pokemon_storage_system.h](../../game/include/pokemon_storage_system.h))
  measures 34,144 of its nine sectors' 35,712 bytes, leaving 1,568, and its
  size is already asserted (`PokemonStorageFreeSpace` in save.c). The
  record pool is appended there ([saved state](#saved-state)). Sizes were
  measured by compiling `sizeof` against this branch's headers.

### Egg engine

- **Giving an egg.** The `giveegg` macro
  ([event.inc](../../game/asm/macros/event.inc)) takes only a species.
  `ScrCmd_giveegg` ([scrcmd.c](../../game/src/scrcmd.c)) calls
  `ScriptGiveEgg`
  ([script_pokemon_util.c](../../game/src/script_pokemon_util.c)), which
  builds the egg with `CreateEgg` ([daycare.c](../../game/src/daycare.c))
  and hands it over through `GiveCapturedMonToPlayer`
  ([pokemon.c](../../game/src/pokemon.c)), returning
  `MON_GIVEN_TO_PARTY`, `MON_GIVEN_TO_PC`, or `MON_CANT_GIVE` in
  `VAR_RESULT`. `CreateEgg` calls `CreateRandomMonWithIVs`, so the
  personality is random; it gives the level-up moveset at
  `EGG_HATCH_LEVEL` (1, since `P_EGG_HATCH_LEVEL` is `GEN_LATEST`), a POKé
  BALL, the egg nickname, met level 0, and the species' `eggCycles` in the
  friendship field. `GiveCapturedMonToPlayer` sets the player as OT and,
  with no free party slot, sends the egg to the PC (`CopyMonToPC`), which is
  where Egg sitting's full-party rule comes from.
- **A variant is needed.** Egg sitting needs a command beside `giveegg`
  that takes the trainer: it builds the egg as `CreateEgg` does, but with
  `CreateMonWithIVs` and the derived [personality](#the-egg), clears
  `MON_DATA_IS_SHINY`, adds the trainer's egg move as `BuildEggMoveset`
  does (`GiveMoveToMon`, then `DeleteFirstMoveAndGiveMoveToMon` when four
  are known), and gives it through `GiveCapturedMonToPlayer`, keeping the
  result codes. Egg moves come from `GetSpeciesEggMoves`
  ([pokemon.h](../../game/include/pokemon.h)).
- **Steps.** `TryProduceOrHatchEgg` in daycare.c counts one cycle each
  256 steps under `P_EGG_CYCLE_LENGTH` `GEN_3`
  ([config/pokemon.h](../../game/include/config/pokemon.h)), over
  `gPlayerParty` only, and lowers each party egg's friendship by
  `GetEggCyclesToSubtract()` (2 with Magma Armor, Flame Body, or Steam
  Engine in the party, [egg_hatch.c](../../game/src/egg_hatch.c)); an egg
  already at 0 hatches.
- **Hatching.** `CreateHatchedMon` in egg_hatch.c rebuilds the POKéMON
  with the egg's personality, moves, IVs, shininess, and ball and the
  player's OT ID; `AddHatchedMonToParty` then sets met level 0 ("hatched
  at" on the summary) and the current map as met location. The egg itself
  also has met level 0, so recognition tells an egg from a hatchling by
  `MON_DATA_IS_EGG`. With the randomizer's egg option on,
  `CreateHatchedMon` may change the species, which recognition ignores.
- **Finding and removing.** A small special scans the party, every box,
  and the Day Care for a POKéMON with a given personality and the player's
  OT ID, and reports which it found, an egg or a hatchling. Giving the egg
  back removes it: from a box through `RemoveSelectedPcMon`
  ([pokemon_storage_system.c](../../game/src/pokemon_storage_system.c)),
  which reads the box and position from `gSpecialVar_MonBoxId` and
  `gSpecialVar_MonBoxPos` (or `ZeroBoxMonAt`), and from the party with
  `ZeroMonData` and `CompactPartySlots`.

### Courier engine

- **The item.** FRLG's `ITEM_OAKS_PARCEL` is an alias of `ITEM_PARCEL`
  (746, [items.h](../../game/include/constants/items.h)), a key item named
  "PARCEL" whose description is Oak's ("A parcel for Prof. Oak from a
  Pokémon Mart's clerk.", [items.h data](../../game/src/data/items.h)).
  Reusing it would collide with the Pallet opening:
  `WayfarerKanto_TryReceiveParcel` gives Oak's parcel only when the Bag
  has none, and `WayfarerKanto_TryAcceptParcel` takes any one away
  ([wayfarer_kanto_opening.c](../../game/src/wayfarer_kanto_opening.c)),
  so a courier parcel held at that point would stand in for Oak's.
  **Recommendation:** a new Wayfarer-only key item, `ITEM_COURIER_PARCEL`,
  added after `ITEM_SILPH_CARD_KEY` under `POKEMON_WAYFARER`, which sets the
  precedent of a Wayfarer key item reusing an existing icon. Its name is
  "PARCEL", its description generic ("A parcel to deliver for a friend."),
  its pocket `POCKET_KEY_ITEMS`, its use `ItemUseOutOfBattle_CannotUse`,
  its importance nonzero so it can't be tossed, sold, or held, and its icon
  `gItemIcon_Parcel`.
- **Delivery is script logic.** The talk script compares the talked-to
  trainer with the saved recipient, and the sender's trail and away lines
  read the current placement (`{PLACE}` from the recipient's haunt). When
  real travel replaces placement, the same reads go to the world record
  ([travel](#travel-and-on-map-walking)); nothing in the parcel depends on
  how the recipient got where they are.

### Handicap engine

- **Totem boost.** The `settotemboost battler, atk, def, speed, spatk,
  spdef, acc, evas` script macro
  ([event.inc](../../game/asm/macros/event.inc)) runs
  `callnative ScriptSetTotemBoost`
  ([battle_main.c](../../game/src/battle_main.c)), which queues the stage
  changes for that battler position in `gQueuedStatBoosts`. The next
  battle applies them before the first turn
  (`FIRST_TURN_EVENTS_TOTEM_BOOST` in battle_main.c runs
  `BattleScript_TotemVar` in
  [battle_scripts_1.s](../../game/data/battle_scripts_1.s) for every
  battler with a queued boost); `BS_GetTotemBoost` in
  [battle_script_commands.c](../../game/src/battle_script_commands.c)
  plays `BattleScript_TotemFlaredToLife` ("{B_DEF_NAME_WITH_PREFIX}'s aura
  flared to life!", `STRINGID_AURAFLAREDTOLIFE` in
  [battle_message.c](../../game/src/battle_message.c)) and raises each
  stat in turn, then the queue is cleared. Nothing checks the battle type,
  so it works in a trainer battle as in a wild one, and HNS already uses
  it that way: `settotemboost B_POSITION_OPPONENT_LEFT, …` directly before
  `trainerbattle_no_intro TRAINER_ZUKI_HNS`
  ([Ecruteak Theater](../../game/data/maps/EcruteakCity_Theater_hns/scripts.inc)).
  The Handicap talk script runs
  `settotemboost B_POSITION_OPPONENT_LEFT, 1, 1, 1, 1, 1` immediately
  before it starts the battle, with nothing between them that could start
  another battle. `B_POSITION_OPPONENT_LEFT` is position 1
  ([constants/battle.h](../../game/include/constants/battle.h)), the
  opponent's battler in singles.
- **The ace-only team.** [Gym Leader scaling](gym-leader-scaling.md) owns
  battle construction; the Handicap needs it to resolve the full snapshot
  and then build the enemy party from slot 1 alone, so the ace keeps the
  moves, item, and stage it has in the full team. Swap battle's prebuilt
  `gEnemyParty[0]` with `CreateNPCTrainerParty` skipped
  ([Swap battle engine](#swap-battle-engine)) is one route; a construction
  parameter is the other.
- **AI override.** `BattleAI_SetupFlags` in
  [battle_ai_main.c](../../game/src/battle_ai_main.c) sets the opponent's
  flags from the trainer's data (`GetAiFlags` for `B_BATTLER_1`) and copies
  them into the player-side prediction slots (`B_BATTLER_0` and
  `B_BATTLER_2`). The [Trainer AI override
  point](trainer-ai.md#runtime-and-the-override-point) writes after it; for
  a Handicap it writes the resolution with skill tiers 0-3 and
  `AI_FLAG_OMNISCIENT`
  ([battle_ai.h](../../game/include/constants/battle_ai.h)), whose
  comments recommend Omniscient alongside Predict Switch and Predict Move.
- **No whiteout.** The engine skips the whiteout when `B_FLAG_NO_WHITEOUT`
  names a set flag (`battle_setup.c`), and its config note warns that the
  party is not healed then
  ([config/battle.h](../../game/include/config/battle.h)); it is 0 today,
  so this is One on one's route. After a loss the script heals the party
  with `special HealPlayerParty`
  ([script_pokemon_util.c](../../game/src/script_pokemon_util.c)).
- **Doubles against one POKéMON** (for the [Later](#later) two-on-one
  variant). A trainer battle becomes a double battle from the trainer's
  battle type alone, with no party-size check
  ([battle_setup.c](../../game/src/battle_setup.c)). At battle start,
  battle_main.c marks every battler with no valid POKéMON absent, on
  either side ("for example when starting a double battle with only one
  pokemon"), and `TwoOpponentIntroMons`
  ([battle_controllers.c](../../game/src/battle_controllers.c)) sends out
  one opponent when its partner slot is empty. So a double battle against
  a one-POKéMON team is supported by the engine as read, but that marking
  is skipped for trainer-only encounters (`IsTrainerOnlyEncounter`) and in
  the Safari Zone, and it is untested here; it needs a battle test before
  the variant is designed.

### Cameos and the Dojo

- **Cameos.** HNS places a one-off cameo of many Gym Leaders on the
  overworld; talking to it hides the cameo and reveals that leader in the
  Saffron Dojo's rematch room (`SaffronCity_FightingDojoVIP_hns`), where
  rematches pay Battle Points (`givebp`). Haunts replace the cameos and
  absorb the rematch room as the Dojo back room; its rematch seats, nurse,
  and clerk are retired with it.
- **Bug: cameos that never appear.** The Morty, Pryce, and Jasmine cameos
  are hidden at New Game (`FLAG_HIDE_BELLCHIME_MORTY`, `FLAG_HIDE_LAKE_PRYCE`,
  `FLAG_HIDE_CAFE_JASMINE` in
  [new_game.inc](../../game/data/scripts/new_game.inc)) and no script
  clears those flags; only their own cameo scripts set them again. So their
  Dojo seats (`FLAG_HIDE_DOJO_*`) never open, and neither does Jasmine's
  Steelix trade, which needs `FLAG_HIDE_DOJO_JASMINE` clear. The same
  search finds no clear for Falkner's, Bugsy's, Whitney's, and Clair's cameo
  flags (`FLAG_HIDE_CELADON_FALKNER`, `FLAG_HIDE_VIRIDIAN_BUGSY`,
  `FLAG_HIDE_DEPTSTORE_WHITNEY`, `FLAG_HIDE_DEN_CLAIR`), so those look
  unreachable too; this needs an in-game check. Blaine's cameo stands on
  `CinnabarIsland_hns`, which Wayfarer retired, so it is likely unreachable
  as well.
- Retiring the cameos removes these dead ends; Jasmine's trade needs a new
  home, since haunt [trades](#trade) are generic, never a trainer's
  authored offer.

### Prize money

Haunts host no trainer battles that pay: a walk's battles are wild, and
One on one, Swap battle, and Handicap pay no prize money. Only the quest
[fallback](#rewards-and-claims) pays money, on the snapshot level basis a
[tag match](sevii-masters.md#tag-matches) uses: the level of the last member
the trainer brings, with their class's rate. Battle Points from Dojo
rematches are not carried over.

### Open risk: overworld followers

Overworld POKéMON followers are on (`OW_FOLLOWERS_ENABLED` `TRUE` in
[overworld.h](../../game/include/config/overworld.h)). Whether a follower
NPC and the player's following POKéMON can share the path behind the player
is unchecked; this needs checking before walks are built. The
[travel proof of concept](../research/notable-trainer-travel-poc.md) disabled the player's following POKéMON to
free object slots, so this stays open.

## Saved state

Haunts add:

- one **claim bit** per haunt. Friendship is saved by
  [Notable trainers](notable-trainers.md#friendship), and haunts read only
  its stage;
- one **reward counter** per notable trainer entry, 4 bits each (0-15),
  so 38 × 4 bits in v0. Tate & Liza's counter stays 0 until the duo is
  placed ([rewards and claims](#rewards-and-claims));
- the **current placement**: one `characterId` or none per haunt. It is
  derivable from the inputs, and is saved to detect changes for the claim
  bits and to hold the walking trainer during a walk; and
- one **search state** per Lost something haunt, 2 bits: none, searching
  (the player said `YES`), or found (the keepsake is in hand). It lasts the
  whole placement, through reloads and whiteouts, and clears with the claim
  bit when the haunt's placement changes. The active lost spot is derived
  from the placement and not saved ([Lost something](#lost-something));
- one **asked bit** per Catch me one haunt, set by `YES` and cleared with
  the claim bit ([Catch me one](#catch-me-one)); and
- one **owed bit** per Trade, Wanted, or Handicap haunt, set when a
  trade's or a handicap battle's reward waits and cleared with the claim
  bit ([Trade](#accepted), [Handicap](#the-handicap-outcome));
- one **follow-up bit** per haunt, set when an egg is given or an Egg
  sitting follow-up plays there and cleared with the claim bit
  ([follow-up](#the-follow-up));
- per notable trainer entry, one **outstanding bit** and a 4-bit
  **eggs-given count** (0-15), 5 bits each, so 38 × 5 bits, 190 bits or 24
  bytes in v0 ([Egg sitting](#egg-sitting)). They sit with the reward
  counters in the haunt state in SaveBlock3, whose 464 free bytes the
  haunt and notable-trainer state already claims
  ([trade engine](#trade-engine)); the egg itself is a normal POKéMON in
  the player's party or boxes, so nothing per egg is stored;
- per notable trainer entry, a **handicap attempted** bit and a **handicap
  beaten** bit ([Handicap](#handicap)), 2 bits each, so 38 × 2 bits, 76
  bits or **10 bytes** in v0, beside the reward counters in SaveBlock3's
  haunt state, well within its 464 free bytes. They never clear: one
  attempt per trainer, ever. Beyond picking the callback line, nothing
  reads the beaten bit in v0; it is a record for later gossip, phone
  calls, and the trainer card;
- one **courier state** for the whole game ([Courier](#courier)): the
  sender's and the recipient's `characterId` (6 bits each, as in a
  traded-slot record), the active flag, and the
  [trail bit](#refreshing-the-trail), 14 bits packed into **2 bytes**
  beside the reward counters in SaveBlock3's haunt state, well within its
  464 free bytes. The PARCEL itself is a key item in the Bag, saved with
  it;
- the **traded-slot records** (below); and
- the **quest in progress**: a walk (its haunt), cleared on load and on
  whiteout, or a Swap battle (its haunt), set when the party is parked and
  cleared at the restore. Bring me adds nothing: it runs within one talk.
  Swap battle adds nothing per placement either, and a trade runs in one
  script, so it needs no quest in progress. Neither does a handicap
  battle: the battle and its bits run in one script, and the party is
  never swapped.

**Traded-slot records.** A pool of sixteen records, each naming its
trainer, holds every [traded slot](#the-trainers-team-after-a-trade); a
trainer has at most one. A record holds everything needed to rebuild the
player's POKéMON on the trainer's team and to hand it back:

| Field | Size | Holds |
| --- | ---: | --- |
| `personality` | 32 bits | Gender, the natural nature, and every personality-derived trait. |
| `otId` | 32 bits | The original OT ID. |
| `species` | 16 bits | Species and form as traded (forms are species IDs); the stage floor. |
| `otName` | 7 bytes | The original OT name. |
| `nickname` | 12 bytes | `POKEMON_NAME_LENGTH` characters. |
| `characterId` | 6 bits | The trainer. |
| `slot` | 3 bits | The roster slot it replaced, 2-6; 0 marks a free record. |
| `tradeLevel` | 7 bits | Its level when traded, 1-100. |
| `ivs` | 30 bits | Six IVs, 5 bits each. |
| `nature` | 5 bits | Its nature, mints included (the hidden nature). |
| `abilityNum` | 2 bits | Its ability slot. |
| `shiny` | 1 bit | Shininess as it was. |
| `otGender` | 1 bit | The original OT's gender. |
| `ball` | 6 bits | Its POKé BALL. |
| `language` | 3 bits | Its language. |

That is 29 bytes of whole fields and 64 bits packed into two words, 37
bytes, padded to **40 bytes** a record with three reserved zero bytes, so
the pool takes **640 bytes**. It is appended to `struct PokemonStorage`,
whose nine sectors have 1,568 bytes free; the 928 left stay free. It
never goes in SaveBlock1, which has 112 bytes left, and it doesn't fit in
SaveBlock3, whose 464 free bytes the other haunt and notable-trainer state
needs first ([trade engine](#trade-engine)). The received filler needs no
record: its personality and OT derive from the trainer and slot.

**Budget risk.** The pool caps traded slots at sixteen across all
trainers ([limits](#limits)). One record for each of the 37 placeable
trainers would take 1,480 of the storage sectors' 1,568 free bytes,
leaving 88; that removes the cap but spends nearly all the save's last
large free space, so it is not proposed. The storage struct is shared with
the PC boxes, so a later change to box count or box layout competes for
the same bytes; `PokemonStorageFreeSpace` fails the build rather than
overflow.

New Game saves every claim bit clear, every reward counter at 0, the placement
for world progress 0, every search state at none, every asked bit, owed bit,
follow-up bit, outstanding bit, and handicap bit clear, every eggs-given
count at 0, every traded-slot record free, no active parcel (the courier
state all zero), and no quest in progress. With follower NPCs enabled,
SaveBlock3 also holds the engine's follower state, which a walk uses.

## Load validation

On every load, before the overworld runs:

1. **Quest in progress.** Clear it; if a follower NPC is present, remove
   it. A walk interrupted by a reload is unfinished
   ([walk](#walk-with-me)). A Swap battle in progress means the save was
   written mid-swap, which [normal play never
   does](#save-and-reset-during-a-swap); the saved party is the parked
   one, so keep it as it is, clear `FLAG_DISABLE_EXP_GAIN`, and leave the
   quest unfought and open: neither a win nor a loss.
2. **Pruning.** Drop the reward counters of characters no longer in the
   registry, and the claim bits, search states, asked bits, and placements of
   haunts no longer in the catalog, or search states and asked bits of haunts
   whose quest type changed, and owed bits of haunts that are no longer Trade,
   Wanted, or Handicap haunts. Drop the handicap bits of characters no
   longer in the registry. Drop the follow-up bits of haunts no longer in the
   catalog, and the outstanding bits and eggs-given counts of characters no
   longer in the registry; their eggs stay with the player as ordinary
   POKéMON. An active parcel whose sender or recipient is no longer in the
   registry, or is no longer placeable (Tate & Liza), is settled: the
   PARCEL is removed from the Bag, the courier state is cleared, and
   nothing is paid. The Bag follows the courier state: a PARCEL with no
   active parcel is removed, and an active parcel with no PARCEL in the
   Bag gets it back. A reward counter above its trainer's current pool
   length (the pool got shorter) is lowered to that length: the pool counts as
   used up, and nothing is taken back or paid. A traded-slot record is freed
   when its trainer is no longer in the registry, its species no longer
   exists or has become an Egg, Mega, or other battle-only form, or its slot
   is no longer a filler slot: the slot reverts to its authored filler, the
   player's POKéMON is gone, and nothing is paid back.
3. **Checks.** Reward counters exist only for known trainers; claim bits
   and placements only for known haunts, and search states only for known
   Lost something haunts, never the unused fourth value, and never
   searching or found at an empty haunt or with the claim bit set; asked
   bits only for known Catch me one haunts, never set at an empty haunt or
   with the claim bit set; a saved
   placement names known characters, each at most once; an owed bit only
   at a known Trade, Wanted, or Handicap haunt, never at an empty one, and
   at a Handicap haunt only when its placed trainer's attempted bit is set;
   handicap bits only for known trainers, and never a beaten bit without
   the attempted bit; a follow-up bit
   only at a known haunt, never at an empty one; an outstanding bit and an
   eggs-given count only for known trainers, never a count above the cap
   (15, so the check guards a lowered cap), and never an outstanding bit
   with a count of 0; an active courier state names a sender and a
   recipient that differ, and an inactive one is all zero. Each
   traded-slot record in use names a known character, at most one record
   per character, a slot from 2 to 6, a trade level from 1 to 100, a
   nature below 25, an ability slot below 3, a valid ball, a nickname and
   OT name each ended within their length, and zero reserved bytes; a free
   record is all zero. A failed check is an invalid save, never a reason to
   reward anything.
4. **Recompute.** Compute the placement from the current inputs and compare
   it with the saved one, clearing the claim bit, search state, asked bit,
   and follow-up bit of every haunt whose trainer changed, and the trail
   bit when that haunt held or now holds the parcel's sender or recipient,
   then save it.

## Presentation

- The placed trainer stands at the haunt's standing tile with their buddy
  beside them as a POKéMON object at its current species, as the Dojo's
  rematch room shows its leaders' POKéMON today. An empty haunt shows no
  one.
- Speaker labels name the trainer ("BROCK:"); Tate & Liza's lines keep
  their split format for when the duo is placed later.
- Talking runs the [talk flow](#talk-flow) straight through; there is no
  menu. The quest proposal ends in a [YES / NO] prompt.
- During a walk the trainer follows the player; the double wild battles
  show them beside the player with their back pic.
- In a Swap battle the player's side shows the ace, and the trainer sends
  out the copy of the player's pick under its own nickname.
- A handicap battle is an ordinary singles battle against the lone ace,
  which opens with the totem animation and "…aura flared to life!" before
  the first turn.
- A trade plays the in-game trade scene, with the trainer's name as the
  partner. When the traded slot is the buddy slot, the POKéMON object
  beside the trainer is the traded POKéMON at its current stage.

## Balance report

Informational. The
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) is to show,
for a chosen world progress, accepted lineup, and partner: each trainer's
momentum and candidate haunts, each haunt's score table, the fill order,
and the placement, plus a sweep over world progress 0-160 showing how often
each trainer sits at each haunt and how many haunts are empty. For each
Wanted haunt it also shows every candidate's wanted species and offer, and
flags pairs with [no fair offer](#no-fair-offer). It asserts no target.
It is not built yet ([Later](#later)).

**Handicap margin.** For each world progress the report shows which
trainers pass the Handicap [gate](#the-gate), and for each the ace's level
against the player's level cap, so the margin is tuned with numbers toward
"humbling but fair and beatable". The ace's level is `teamLevel` at the
trainer's TR, and the cap is the
[v0 level cap curve](trainer-rating-party-progression.md#v0-level-cap-curve)
at the player's. At the gate itself (trainer TR exactly the player's plus
the margin), the placeholder anchors give:

| Player TR | Level cap | Margin 30 | Margin 40 | Margin 50 |
| ---: | ---: | ---: | ---: | ---: |
| 0 | 15 | Lv 21 (+6) | Lv 28 (+13) | Lv 34 (+19) |
| 20 | 22 | Lv 34 (+12) | Lv 39 (+17) | Lv 45 (+23) |
| 40 | 28 | Lv 45 (+17) | Lv 50 (+22) | Lv 56 (+28) |
| 60 | 39 | Lv 56 (+17) | Lv 63 (+24) | Lv 69 (+30) |
| 80 | 50 | Lv 69 (+19) | Lv 75 (+25) | Lv 81 (+31) |
| 100 | 63 | Lv 81 (+18) | Lv 88 (+25) | Lv 94 (+31) |
| 120 | 75 | Lv 94 (+19) | Lv 100 (+25) | Lv 100 (+25) |
| 140 | 88 | Lv 100 (+12) | Lv 100 (+12) | Lv 100 (+12) |
| 160 | 100 | Lv 100 (0) | Lv 100 (0) | Lv 100 (0) |

At margin 40 the ace sits 13 to 25 levels above the cap through most of
the journey, before its +1 aura; past TR 120 the Lv 100 ceiling shrinks
the gap, so the aura and the smartest AI carry the handicap at the end.
The gate is only a floor: a trainer further ahead sits higher. Agatha (TR
95, Lv 59) faces a player at TR 0 with a 44-level gap, and Lance (TR 200,
Lv 100) an 85-level gap ([open questions](#open-questions)).

## Acceptance

Required implementation evidence (not yet run):

1. **Catalog.** Every haunt has valid tags (one or two activities from the
   shared list, capacity 1 in v0, and a name of at most 20 characters),
   existing maps, an active standing
   tile, and its quest details (a walk's start and exit, two or three lost
   spots, each on a reachable tile with a hint, a catch species on its wild
   table, a Bring me kind from the kind table, local species on a Wanted
   haunt's maps); every notable trainer has
   a buddy slot 1-6 and a reward pool that passes the
   [pool rules](#rewards-and-claims).
2. **Buddy.** `{BUDDY}` resolves to the slot's stepped-down species at every
   world progress, including a slot not yet on the team (Giovanni's slot 4
   is MEOWTH at world progress 10 and PERSIAN at 40; Brock's slot 1 is ONIX
   until world progress 57).
3. **Qualifiers.** No placement breaks a hard qualifier at any world
   progress 0-200: region or traveller, aloof only at remote elite haunts,
   and no Tate & Liza, accepted-lineup trainer, or
   resolved partner.
4. **Placement.** The same inputs always give the same placement; each
   trainer holds at most one haunt; scores, momentum, the fill order, and
   ties match golden fixtures from host tooling; an inactive elite haunt is
   empty; accepting an event, asking a partner, and becoming a Master each
   recompute it. A walking trainer keeps their haunt until the walk ends.
5. **Talk flow.** Every talk is a greeting, then the proposal or `QUIRK` (or a
   follow-up in their place: a Courier delivery, else an Egg sitting
   follow-up, else a Courier trail), then `BYE`, with no menu, battle
   offer, or team-up at any stage. The greeting follows the stage: a Stranger
   gets `MEET`, or `HEARD` when the player's fame reaches them (the player's TR
   at least `min(80, their TR − 10)`, a reign at any league, or being a Master),
   Met gets `AGAIN`, Friend `HELLO`, Close `CLOSE`. A first talk makes the
   trainer Met and proposes the quest in the same talk; while the claim bit is
   clear every talk proposes it (`ASK`, the proposal, [YES / NO]), `NO` changes
   nothing and the next talk proposes again, and a talk that starts a walk ends
   at `YES`; once the claim bit is set every talk is the greeting, `QUIRK`, and
   `BYE`. No haunt line uses `NOT_YET` or `NEWS`. Only the first talk (+1) and a
   completed quest (+10) add points at a haunt; a quest that crosses the Friend
   threshold hands over the number once.
6. **Quests.** Each quest completes, fails, and retries as specified; a walk
   ends unfinished on whiteout and on reload, and a wrong exit asks "Giving up
   on the walk?" (YES ends it with the trainer's `NO` line, NO steps the player
   back one tile and the walk continues); Fly, Teleport, and an Escape Rope are
   refused during a walk; its wild battles are double battles beside the trainer
   with their best three from the runtime partner slot; Catch me one opens the
   storage screen only after `YES` and with a match, dims every non-match,
   accepts any form but never an Egg, leaves the shown POKéMON with the player,
   answers `NOT_READY` with none to show, and stays open through the placement;
   the quiz's questions, answers, and options match golden fixtures built from
   the type chart (Brock's lone ONIX gives Fighting, Normal, and Electric;
   Blue's KADABRA and EEVEE give Bug, Fighting, and Fighting), every wrong
   option is wrong under the chart, a wrong answer ends the attempt with the
   loss line and no penalty, and the same stages give the same questions on
   every attempt; a One on one loss plays the haunt's loss line (or
   `NOT_READY`), costs nothing, and leaves the claim bit clear, and a win adds
   +10 as a quest, never +20. Lost something picks the same active spot for the
   same placed trainer (catalog position mod spot count) and names its hint in
   the proposal; the spot is inert before `YES`, after it is found, and once the
   claim bit is set; after `YES` it is found by facing it and pressing A, stays
   armed across leaving, whiteout, and reload, is re-armed by the next
   placement's `YES`, and makes the Dowsing Machine respond; finding it adds no
   Bag item; the next talk plays the done line, `PRAISE`, the reward, and `BYE`
   for +10; and a reshuffle clears an open search.
7. **Bring me.** The Bag opens only after `YES` and only with an item of
   the kind in the Bag (otherwise `NOT_READY`), on the kind's pocket with
   no pocket switching; every item the kind table resolves is accepted and
   every other is refused in place with nothing removed; key items never
   count; a matching pick removes exactly one, plays the done line,
   `PRAISE`, the reward, and `BYE` for +10; a cancel or a waiting reward
   removes nothing and leaves the quest open.
8. **Swap battle.** `YES` parks the party, and the player fights with only
   the signature ace at its snapshot stage, with the trainer's OT name and
   ID, against a healed copy of the able non-Egg POKéMON the player picked
   under the trainer's AI, the proposal never mentions a swap, and the
   reveal line plays only after the pick; with the player's TR giving a
   soft cap below the ace's
   level the ace can disobey, and at or above it never does; neither side
   gains experience; afterwards the party equals the parked one exactly
   (HP, PP, status, held items, friendship) after a win and after a loss;
   a win adds +10 as a quest, never +20; a loss plays the loss line, costs
   nothing, and leaves the claim bit clear; a reset mid-swap reloads a
   whole party with the quest open, and a save written mid-swap is
   recovered on load with the parked party.
9. **Trade.** The offer is the highest-numbered filler slot of the six at
   its stepped-down stage, never an ace, and `{FILLER}` names it (Brock's
   OMANYTE at world progress 0); the screen opens only after `YES`, over
   the party only, dimming boxed POKéMON, Eggs, the last able member, and
   any without one of the aces' types, and none of them can be picked;
   with none eligible, `NOT_READY`; the value check matches golden line
   values from host tooling (final forms through every branch, own total
   without evolutions, no Megas, Gigantamax or battle-only forms, regional
   lines apart), accepts at equal values and refuses one below with "That's
   not a fair trade." and `NO`, leaving everything as it was; a held item
   goes to the Bag, or with no room nothing is traded; an accepted trade
   plays the in-game trade scene and gives the filler at the given
   POKéMON's level, stepped down and never above the offered stage, with
   the trainer as OT, the same personality for the same trainer and slot,
   and the scene's trade evolution; then the done line, `PRAISE`, the
   reward, and `BYE` for +10, and a waiting reward sets the owed bit and is
   paid at the next talk. In the trainer's battles the traded POKéMON holds
   the slot's place in battle order, never an ace, at their team level and
   slot offset, with its species, nickname, shininess, gender, nature, IVs,
   and ability, moves from their pool, evolving forward by the evolution
   levels and never below its traded stage; as the buddy slot it is
   `{BUDDY}`. A later Trade quest with that trainer offers it back for the
   received filler only, identified by personality and OT, with no value
   check, returns it with its record intact at the filler's level, and
   frees the slot; one trade or trade-back per placement; with all sixteen
   records in use a new trade answers `NOT_READY`; a reset during a trade
   leaves neither the trade nor the record.
10. **Wanted.** The wanted species matches golden fixtures from host
    tooling: the local species of the land, water, fishing, and rock smash
    tables of every time of day on the haunt's maps, skipping zero-rate
    tables and `SPECIES_NONE`, ranked by line value and then lowest species
    number; the first sharing an ace type, else the first of any type
    (Brock and Giovanni at Diglett's Cave want SWINUB, Misty at the
    Cerulean Cape MAGIKARP, Bugsy and Erika at Viridian Forest SPINARAK);
    it is the same at every talk, time of day, and placement for that
    trainer and haunt; a Wanted haunt with no local species fails the
    build. The offer is the filler with the highest line value at most the
    wanted species', ties to the highest-numbered slot, never an ace
    (Brock's OMANYTE at world progress 0, Misty's LAPRAS); with no such
    filler, the no-offer line, `NOT_READY`, and `BYE` replace the proposal
    and the claim bit stays clear (Erika at Viridian Forest). After `YES`
    the offer line names `{FILLER}`, and the screen opens over the party
    only, where only the wanted species (any form, not an Egg, not the last
    able member) can be picked; with none, `NOT_READY` and no screen; there
    is no value check; an accepted trade runs exactly as Trade's (held
    item, scene, received filler, done line, `PRAISE`, reward, `BYE`, +10,
    owed bit, traded slot). A trainer holding a traded slot offers the
    trade-back at a Wanted haunt, and Trade and Wanted share one traded
    slot per trainer, one trade per placement, and the sixteen records.
11. **Egg sitting.** The proposal is "Could you hold on to this egg for a
    while?", and no line before the hatched follow-up mentions hatching or
    hints that the player keeps the hatchling. `NO` changes nothing; `YES`
    gives an egg of the species breeding would produce from the
    highest-numbered filler slot's authored species, by the Day Care's own
    rules without incense, never an ace line or a traded slot (Brock's
    OMANYTE; a Raichu filler gives PICHU, a Snorlax filler SNORLAX), knowing
    the first egg move of that species in the trainer's move pool order,
    never shiny, with the personality its golden fixture gives for that
    `characterId` and count; with a full party it goes to the PC, and with
    the PC full too the talk is `NOT_READY` and nothing changes. Giving it
    sets the claim bit, pays nothing, and adds no friendship. With an egg
    outstanding, an Egg sitting haunt gives `QUIRK`, and so does one whose
    trainer has given fifteen. At a later meeting with that trainer, at any
    haunt, the follow-up replaces the proposal or `QUIRK` once per meeting,
    or whenever the hatchling is found, and never at the meeting that gave
    the egg while it is unhatched; the next talk returns to the haunt's own
    quest, whose claim bit the follow-up never touches. A hatchling, found
    by personality, the player's OT ID, not an Egg, and met level 0, in the
    party, a box, or the Day Care, plays the surprised reveal, `PRAISE`,
    the next reward-pool entry, and `BYE` for +10, and settles the egg; a
    waiting reward leaves it outstanding. An unhatched egg, in the party or
    a box, plays the still-an-egg line and [YES / NO]: `YES` keeps it
    outstanding, and `NO` removes exactly that egg and settles it with no
    reward. With neither found, the explanation, "…You WHAT?!", the calmer
    line, and `BYE` play and the egg is settled with no reward and no
    change in friendship. No outcome costs the player anything; a settled
    egg lets a later Egg sitting quest give the next one, with a new
    personality. A different POKéMON of the same species never counts.
12. **Courier.** The proposal names a recipient placed right now, never the
    sender: the trainer at offset `wp mod m`, from the first after the
    sender in catalog order, among placed trainers of the sender's home
    region, or of any region when none is placed, matching golden fixtures
    from host tooling; `{PLACE}` is the recipient's current haunt name. With
    no one to name, or while a parcel is active anywhere, a Courier haunt
    gives `QUIRK` with the claim bit clear. `NO` changes nothing; `YES` gives
    the PARCEL (`ITEM_COURIER_PARCEL`, never `ITEM_OAKS_PARCEL`, so the
    Pallet opening's parcel works with a courier parcel held), sets the
    claim bit, and pays nothing. Placement never reads the parcel: the
    recipient moves as usual. At the sender's next meeting, the trail names
    the recipient's current haunt, or the away line for a league lineup,
    the partner, or no haunt, once, and again only after a recompute
    changes the sender's or the recipient's haunt. At any talk with the
    recipient, at any haunt and at a first meeting after `MEET` or `HEARD`,
    the delivery line, the recipient's `PRAISE`, their next reward-pool
    entry, and `BYE` play, +10 with each of the sender and the recipient;
    the PARCEL is removed and the courier settled, and a waiting reward
    leaves it active. A delivery comes before an Egg sitting follow-up, and
    that before the trail; none touches a claim bit, and the haunt's own
    quest returns at the next talk. No expiry or penalty applies.
13. **Handicap.** The proposal, "Your whole team against my {ACE}. Think
    that's enough?", plays only while the trainer's current TR is at least
    the player's TR plus the margin (40), checked at each talk: one TR
    below, the gate's line, `QUIRK`, and `BYE` play, the claim bit stays
    clear, and the attempt stays unused. `NO` changes nothing. `YES` starts
    a singles battle against the signature ace alone at its snapshot stage,
    level (capped at Lv 100), moves, and held item, never with a second ace
    or a filler (a Double Ace trainer brings one POKéMON); the ace opens at
    +1 Attack, Defense, Speed, Sp. Atk, and Sp. Def with the aura message,
    and accuracy and evasion at 0; its flags are the trainer's resolution
    with every skill tier through Predictive and Omniscient, at every
    trainer TR and with or without the boss flag, written to the opponent
    and the prediction slots; the player uses the full party and Bag; no
    prize money is paid and a loss never blacks out, leaving the party
    fully healed. A win plays "…Six against one, and you actually did
    it.", `PRAISE`, the next reward-pool entry, and `BYE`, sets the
    attempted and beaten bits, and adds +10, never +20; a loss plays "Not
    bad for being outnumbered. Take this anyway.", the reward with no
    `PRAISE`, and `BYE`, sets only the attempted bit, and adds +10. It is
    the only quest that isn't retryable: after either outcome no Handicap
    haunt ever proposes again with that trainer, in this or any later
    placement. There, the first talk of each placement plays the callback,
    "Still thinking about our six-on-one?" after a win or "Come back when
    you can take me one-on-one." after a loss, then `BYE`, and sets the
    claim bit with no reward or friendship; later talks give `QUIRK`. A
    waiting reward sets the owed bit and is paid with the outcome line at
    the next talk.
14. **Claims.** A reward is given once per placement; a changed placement
    reopens it; a full Bag, a cancelled lesson, or no POKéMON able to learn
    keeps it open and leaves the reward counter unchanged.
15. **Reward pools.** The reward never depends on the quest type. Quests
    with one trainer at different haunts pay that trainer's pool in order,
    one entry each; a gated next entry or a used-up pool pays the fallback
    (prize money equal to a win over the trainer at current TR) and leaves
    the counter alone; entries open by world progress, never the trainer's
    TR, so a quest just before and just after a gate pays the fallback and
    then the entry, and Lance's and Agatha's pools open gradually. A lesson
    teaches the first move in pool order the chosen POKéMON can learn and
    doesn't know, and offers only POKéMON with such a move. Brock's first
    quest gives PEWTER CRUNCHIES and Giovanni's gives a NUGGET.
16. **Dialogue.** Every assembled line resolves its slots and fits its text
    box with worst-case values; no haunt dialogue carries gossip.
17. **Cameos.** No cameo or Dojo rematch seat remains in Wayfarer; the Dojo
    back room works as a haunt; no stranger battle, rematch, prize money
    beyond the quest fallback, or Battle Points come from haunts.
18. **Save.** A new game and a reload give the saved state described above; a
    search state, asked bit, owed bit, follow-up bit, outstanding bit,
    eggs-given count, handicap bit, traded-slot record, or courier state
    survives a reload, and the first four clear with their haunt's
    placement, while handicap bits never clear; corrupt reward counters,
    claim bits, search states, asked bits, owed bits, follow-up bits,
    outstanding bits, eggs-given counts, handicap bits (a beaten bit
    without its attempted bit), placements, traded-slot records, or courier
    states are rejected; removed characters
    or haunts are pruned, and so are records of removed species or of slots
    no longer fillers; a parcel with a removed sender or recipient is
    settled with the PARCEL removed and nothing paid, and the PARCEL in the
    Bag follows the courier state; a counter past a shortened pool is
    lowered to its length; and the record pool fits
    `PokemonStorageFreeSpace`.

## Open questions

- Whether a minimum score should leave a haunt empty rather than host a
  poor fit, since v0 has more Kanto haunts than Kanto candidates.
- When to retire the Johto cameos and their Dojo seats: with the Kanto
  list, which turns the rematch room into the Dojo back room, or later, with
  the back room waiting for Johto's list too.
- Where Gym Leader rematches go: haunts offer no rematches, so retiring the
  Dojo rematch room leaves no rematches until rematch spots exist
  ([Later](#later)).
- Where Jasmine's Steelix trade goes once the Dojo seats retire.
- Whether a trainer whose received filler the player no longer has should
  ever offer something else: today every Trade or Wanted haunt they stand
  at answers `NOT_READY` ([trade-back](#trade-back)).
- Whether placement should steer a trainer with
  [no fair offer](#no-fair-offer) away from a Wanted haunt, as Erika at
  Viridian Forest would be, instead of leaving its quest unavailable.
- Whether the Handicap gate needs a ceiling as well as its margin: the
  margin is only a floor, so Lance (TR 200) offers his Lv 100 ace to a
  player capped at Lv 15, 85 levels above the cap
  ([balance report](#balance-report)).
- Whether a later stage of the wanted species' line should count at a
  Wanted haunt: Misty at the Cerulean Cape wants MAGIKARP, so a fished
  GYARADOS doesn't ([Wanted examples](#wanted-examples)).
- Whether sixteen traded-slot records are enough, or the pool should hold
  one per placeable trainer ([saved state](#saved-state)).
- Whether a follower NPC turns regular trainer battles on the way into
  partner battles; the design assumes it doesn't.
- What a haunt does while its map hosts a story scene with a trainer it
  could hold: Misty's Route 25 date stands at (100, 17) on the Cerulean
  Cape while `FLAG_HIDE_ROUTE25_MISTY` is clear, so a placed Misty would
  meet herself.

## Later

- **Situations:** "The coach" (a trainer training the player's POKéMON),
  two trainers in one haunt, "Show me" (the player shows a POKéMON rather
  than handing it over), services, and challenge battles with conditions.
- **Item swaps** at haunts.
- **Trade value overrides**, only if playtest shows abuse: per-species line
  values for outliers such as Shuckle, whose base stat total outruns its
  use.
- **Access:** haunts gated by a key item, an HM, or a story beat, beyond
  the elite gate.
- **Gifts from the roster:** a friend giving the player a POKéMON of their
  own line.
- **More friendship sources:** gifts, tag battles beside a trainer, and
  partnering ([friendship](notable-trainers.md#friendship)).
- **Rematches** at haunts or at dedicated rematch spots, where `NOT_YET`
  could gate a trainer whose first fight is still ahead.
- **Gossip at haunts:** a friend telling the player where another trainer
  is, led in by `NEWS`.
- **Other regions:** haunt lists for Johto, Hoenn, and Sevii, and Tate &
  Liza at haunts.
- **Explorer support** for the [balance report](#balance-report).
- **Favourite species for Wanted:** a per-trainer override of the derived
  wanted species for canon moments, such as Brock's existing Rhyhorn trade.
- **Worked Bring me, Swap battle, Trade, Wanted, Egg sitting, Courier, and
  Handicap haunts**, and their places in the [Kanto list](#kanto-haunts).
- **"Show me a hatchling":** a cheap variant of
  [Egg sitting](#egg-sitting) with no egg handed over: the player shows any
  POKéMON of the trainer's type that they hatched (met level 0, the
  player's OT ID), as Catch me one's showing works.
- **Eggs as reward-pool entries:** a third entry type beside items and
  lessons, such as a Close friend's egg of their buddy's line: a Dratini
  egg from Lance, after HGSS's Dratini gift.
- **More Courier hint sources:** phone calls to the sender, gossip at
  haunts, a Gym guide's "out at {PLACE}", and routines the player can
  learn. Once real travel exists, `{PLACE}` can become "heading toward
  {PLACE}" while the recipient is on the move ([Courier](#courier)).
- **Travel between haunts:** trainers walking to their haunts instead of
  being placed, following routines with a per-place cap of two or three,
  built on the two-layer model the
  [travel proof of concept](../research/notable-trainer-travel-poc.md) validated.
- **More item kinds** for Bring me, and a Bag screen that lists only the
  kind's items instead of refusing the rest in place.
- **Two-on-one Handicap:** a variant format in which two of the player's
  POKéMON battle the lone ace at once, as a double battle against a
  one-POKéMON team. The engine appears to support it but it is untested
  ([Handicap engine](#handicap-engine)).
- **Last stand:** the reverse of [Handicap](#handicap): one of the
  player's POKéMON against the trainer's full team.
- **Gossip about handicap wins:** other trainers, phone calls, and the
  trainer card reading the [handicap beaten](#saved-state) bit.

## References

- [Notable haunts PRD](../prds/notable-haunts.md)
- [Notable trainers](notable-trainers.md)
- [Notable spots](notable-spots.md)
- [Sevii Masters](sevii-masters.md)
- [Leagues](leagues.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Trainer AI](trainer-ai.md)
- [Notable trainer voice bits](../research/notable-trainer-voices.md)
- [Notable trainer reward pools](../research/notable-trainer-rewards.md)
- [Notable trainer travel proof of concept](../research/notable-trainer-travel-poc.md)
- [Trainer roster influence](trainer-roster-influence.md)

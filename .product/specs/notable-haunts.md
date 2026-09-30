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
Friend, Close), then the quest proposal while the quest is open or the
trainer's quirk once it is done, then a farewell; a quest pays once per
placement.
The reward comes from the trainer's own **reward pool**, never from the quest
type. Dialogue splices favour-free haunt lines with each trainer's
**voice bits**. Weights, gates, momentum values, and reward pools are
placeholders; balance is informational.

## Scope

Own, in `IS_WAYFARER`: the haunt catalog and its tags, the meaning of the
**buddy** and **reward pool** trainer values, placement and
momentum, the talk flow, the five quest types, rewards and claims,
dialogue assembly, the Kanto haunt list, retiring the HNS cameos and the
Saffron Dojo rematch room, the haunts' saved state and load validation,
presentation, the balance report, and acceptance.

- [Notable trainers](notable-trainers.md) owns the trainers: inventory,
  home region, the traveller and aloof traits, TR and its growth, rosters,
  the downward rule's use, move pools, and the battle snapshot. It also holds
  the buddy and reward pool data in the catalog; this spec owns
  what they mean.
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

A **haunt** is one authored entry in the haunt catalog: a meeting spot on
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
| Quest | one [quest type](#quests) and its details | The quest a trainer gives there. |
| Meeting spot | map and tile | Where the placed trainer stands. |

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
[lost-something](#lost-something) search and found state; then the new
placement is saved.

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

2. **Proposal or quirk.** While the haunt's claim bit is clear (the quest is
   still open during this placement), the [quest](#quests) proposal: `ASK`,
   the haunt's proposal line, and [YES / NO]. `YES` gives the trainer's
   `YES` and runs the quest as its section says; `NO` gives the trainer's
   `NO`, costs nothing, and the next talk proposes again. At a Lost
   something haunt whose keepsake the player has found, the completion
   takes the proposal's place ([Lost something](#lost-something)). Once the
   claim bit is set (the quest was completed during this placement), the
   trainer's `QUIRK` instead.
3. **Farewell:** `BYE`. A talk that starts a walk ends at `YES` instead;
   the walk's own `BYE` comes at the exit ([walk](#walk-with-me)).

The first talk makes a Stranger Met (+1) at its start, after the greeting
is picked, so a first meeting introduces the trainer (`MEET` or `HEARD`)
**and** proposes the quest in the same talk. After the quest is completed
during this placement, every talk is the greeting, `QUIRK`, and `BYE`, until
the placement changes and the claim bit clears.

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
Repeat talks, declining, and quirks add nothing. Haunts offer no battles
of their own, so no battle win happens here: One on one is a quest, and
its win counts as the completed quest, not as a battle won.

**Teaming up.** Asking a trainer to be the Masters partner happens only by
phone ([partner](sevii-masters.md#partner)), never at a haunt. A trainer who
becomes the partner stops being placed, so they leave their haunt.

## Quests

Each haunt has one quest type. Only a trainer at Met or above gives quests,
which every talk reaches, since the first talk makes the trainer Met before
the proposal ([talk flow](#talk-flow)); every trainer can give every quest
type. Each type has one proposal
line, written so it never implies the trainer needs help or is asking a
favour ([dialogue](#dialogue)):

| Quest | Proposal line |
| --- | --- |
| Walk with me | "Walk it with me, out to the VERMILION side?" (the haunt names its own destination) |
| Lost something | "Something valuable went missing around here, {HINT}. Find it?" |
| Catch me one | "A wild {LOCAL} lives around here. Catch one and show me?" |
| Quiz | "Three questions on type matchups. Think you can answer them?" |
| One on one | "Your best POKéMON against {ACE}. Up for it?" |

A haunt may author its own proposal line for its quest type, under the same
writing rules; the [Celadon Game Corner](#worked-example-celadon-game-corner)
frames One on one as a bet. The quest type decides only the proposal. Every
quest pays the same way, from the trainer's
[reward pool](#rewards-and-claims).

Every quest runs `ASK` (an attention-getter), then the haunt's own quest
line (the proposal), then a yes or no:
`YES` starts it, `NO` ends it ([talk flow](#talk-flow)). A requirement the
player doesn't meet, or a failed attempt, uses `NOT_READY`, unless the haunt
authors its own loss line (One on one). Completing it runs the haunt's done
line if it has one, then `PRAISE`, then the [reward](#rewards-and-claims),
then `BYE`.

### Walk with me

The haunt authors a **start** (its meeting spot), an **exit** (one warp
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
    claim bit still clear, the trainer returns to their haunt spot, and the
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

The haunt authors one species from its own wild table. The player shows
one POKéMON of that species (any form, not an Egg) from the party or the
boxes, picked on the storage screen
([storage-screen modes](#storage-screen-modes)); the player keeps it, and
nothing is traded. With none to show, `NOT_READY`.

### Quiz

Three type-matchup questions about the trainer's aces, generated without
randomness:

1. List the distinct types of the aces in the trainer's current team, at
   their current species, in battle order; repeat the list until it has
   three entries. With no ace on the team, use the signature POKéMON's
   current species.
2. Question `i` names type `T` of entry `i`: "Which type is weak to `T`?"
   The correct answer is the first type, in the type chart's order, that `T`
   hits super effectively; the other two choices are the first two types
   `T` doesn't. If `T` hits nothing super effectively, the question asks
   "Which type resists `T`?" instead, with the choices built the same way.
3. All three right completes the quest. A wrong answer ends the attempt with
   `NOT_READY`; the next attempt asks the same questions.

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

### Rewards and claims

Each haunt has a saved **claim bit**. It is set when the reward is given
and cleared when the haunt's placement changes, so each placement pays out
once, and a new trainer at the haunt can give it again.

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
changes neither.

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
  walk or a Lost something (and optionally for a One on one), a hint per
  lost spot, and optionally a loss line for a One on one. They describe
  only the place and the activity, never a trainer's personality or
  history, so they read true for every candidate.
  A quest line is the proposal that follows `ASK` ("Walk it with me, out to
  the VERMILION side?"), so the same line works after any attention-getter.
  **Writing rules:** haunt lines carry the content of the proposal, and
  haunt lines never imply the trainer needs help or is asking a favour, so
  every trainer can voice every quest; voice bits carry only personality.
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
| `{ACE}` | The signature POKéMON's current species. |
| `{BUDDY}` | The buddy slot's current species ([trainer values](#trainer-values)). |
| `{LOCAL}` | The haunt's catch species (only in a Catch me one line). |
| `{HINT}` | The active lost spot's hint (only in a Lost something line). |

Speaker labels ("BROCK:") and system messages (the [YES / NO] prompt, the
number given, the quiz, a lesson's pick, the fallback amount, the found
keepsake) are generic text, the same for every trainer.

## Worked example: Diglett's Cave

**Tags.** Region Kanto; theme Ground; not elite; no hometown; activities
train and lie low (trainers come to train in the tunnel or to keep out of
sight); setting remote; capacity 1; quest Walk with me. Maps
`DiglettsCave_EntranceNorth_hns`, `DiglettsCave_Tunnel_hns`, and
`DiglettsCave_EntranceSouth_hns`. The meeting spot is in the north
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
at his haunt spot; answering No steps the player back one tile and the walk
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
`CeladonCity_GameCorner_hns`. The meeting spot is tile (12, 11), the
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
quest. Capacity 1 indoors means one trainer at one spot beside the slot
machines, with room for the buddy and nothing else in the way.

## Worked example: Cerulean Cape

**Tags.** Region Kanto; theme Water; not elite; hometown Cerulean;
activities relax and sightsee (trainers come to unwind by the sea or take
in the view); setting remote; capacity 1; quest Lost something. Map
`Route25_hns`, the cape at the east end of Route 25, past Bill's house
(door warp at (85, 13)). The cape proper is the fenced clifftop at x
97-100, y 12-19, reached by the steps at (98-99, 21-22) from the lower path
(y 23). The meeting spot is (100, 14), near the cape's north-east corner;
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
  it gets a 0- or 1-point fit such as Bugsy or Brawly (or Lt. Surge or
  Bruno); when it is 7-10, all 14 candidates are placed before the Cape's
  turn and it stays empty.

What v0 can't say waits for the routine design: a pull that keeps Misty at
her own Cape rather than Pallet Town, the sunset hour the HNS cameo loves,
and a reason beyond theme for Lorelei to be here. These counts follow the
v0 weights and the current Kanto list, and move with them.

**Misty, a Friend** (world progress 20, where `20 mod 18 = 2` and she holds
the Cape with 10). The player's two badges are Brock's and Misty's, and that
win made her a Friend (20 points); no quest with her yet, so her reward
counter is 0. Her spot is by the rocks. At world progress 20 her TR is 30
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

The spot is armed. Leaving for Cerulean and coming back changes nothing;
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
reward counter is 0. Her spot is near the fence. Her buddy is slot 1,
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
and the Dowsing Machine helps, with no battle at all. The spot comes from
the placement, so Misty always hides it by the rocks and Lorelei near the
fence, and the search waits while the player comes and goes. A hometown
pull and a theme pull compete for one spot: Misty brings both, but v0's
fill can send her to Pallet Town first, leaving the Cape to Lorelei on
theme alone, and to travellers when neither is free. And it is the calmest
mood in the list: remote, relax and sightsee, a clifftop by the sea, where
the quest is a stroll.

## Kanto haunts

The v0 Kanto list, in catalog order. It is a draft: the tags, quests, and
species are content for review. Every map exists under
`game/data/maps/`; Seafoam and Cinnabar use the FRLG port's maps, since
Wayfarer retired their HNS versions
([retirement](frlg-cinnabar-seafoam-hns-retirement.md)). Spots, exits, and
lost-spot tiles are chosen at implementation after checking collision and
existing objects; the
[Celadon Game Corner's](#worked-example-celadon-game-corner) and
[Cerulean Cape's](#worked-example-cerulean-cape) are worked out already.
Every haunt has capacity 1.

| # | Haunt | Maps | Themes | Elite | Hometown | Activities | Setting | Quest |
| ---: | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Pallet Town | `PalletTown_hns` | Water | – | – | relax | public | Catch me one: Krabby |
| 2 | Route 1 | `Route1_hns` | Normal, Flying | – | – | train | public | Walk with me: from the Pallet end to the Viridian City edge |
| 3 | Viridian City | `ViridianCity_hns` | Ground | – | Viridian | sightsee | public | One on one |
| 4 | Viridian Forest | `ViridianForest_hns` | Bug, Grass | – | – | study | remote | Catch me one: Pikachu |
| 5 | Pewter Museum | `PewterCity_Museum_1F_hns` | Rock | – | Pewter | study | public | Quiz |
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

- **Sabrina's spot.** Sabrina is aloof, so she can only be placed at a
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
uses `SELECT_PC_MON_TRADE`, which filters by the species in
`gSpecialVar_0x8009`, as Jasmine's Steelix trade does today
([Olivine Gym](../../game/data/maps/OlivineCity_Gym_hns/scripts.inc)).
`SELECT_PC_MON_MOVE_TUTOR` filters by one move (`gSpecialVar_0x8005`), using
the species' teachable list (`CanMonLearnMove`). A lesson needs a filter over
the whole move pool with the [lesson](#rewards-and-claims) eligibility, so it
needs a variant of that mode that also picks the move.

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
  home, since trades at haunts are [Later](#later).

### Prize money

Haunts host no trainer battles that pay: a walk's battles are wild, and
One on one pays no prize money. Only the quest
[fallback](#rewards-and-claims) pays money, on the snapshot level basis a
[tag match](sevii-masters.md#tag-matches) uses: the level of the last member
the trainer brings, with their class's rate. Battle Points from Dojo
rematches are not carried over.

### Open risk: overworld followers

Overworld POKéMON followers are on (`OW_FOLLOWERS_ENABLED` `TRUE` in
[overworld.h](../../game/include/config/overworld.h)). Whether a follower
NPC and the player's following POKéMON can share the path behind the player
is unchecked; this needs checking before walks are built.

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
  bit when the haunt's placement changes. The active spot is derived from
  the placement and not saved ([Lost something](#lost-something)); and
- the **quest in progress**: a walk (its haunt), cleared on load and on
  whiteout.

New Game saves every claim bit clear, every reward counter at 0, the
placement for world progress 0, every search state at none, and no quest in
progress. With follower NPCs enabled, SaveBlock3 also holds the engine's
follower state, which a walk uses.

## Load validation

On every load, before the overworld runs:

1. **Quest in progress.** Clear it; if a follower NPC is present, remove
   it. A walk interrupted by a reload is unfinished
   ([walk](#walk-with-me)).
2. **Pruning.** Drop the reward counters of characters no longer in the
   registry, and the claim bits, search states, and placements of haunts no
   longer in the catalog, or search states of haunts whose quest is no
   longer Lost something. A reward counter above its trainer's current pool
   length (the pool got shorter) is lowered to that length: the pool counts
   as used up, and nothing is taken back or paid.
3. **Checks.** Reward counters exist only for known trainers; claim bits
   and placements only for known haunts, and search states only for known
   Lost something haunts, never the unused fourth value, and never
   searching or found at an empty haunt or with the claim bit set; a saved
   placement names known characters, each at most once. A failed check is
   an invalid save, never a reason to reward anything.
4. **Recompute.** Compute the placement from the current inputs and compare
   it with the saved one, clearing the claim bit and search state of every
   haunt whose trainer changed, then save it.

## Presentation

- The placed trainer stands at the haunt's meeting spot with their buddy
  beside them as a POKéMON object at its current species, as the Dojo's
  rematch room shows its leaders' POKéMON today. An empty haunt shows no
  one.
- Speaker labels name the trainer ("BROCK:"); Tate & Liza's lines keep
  their split format for when the duo is placed later.
- Talking runs the [talk flow](#talk-flow) straight through; there is no
  menu. The quest proposal ends in a [YES / NO] prompt.
- During a walk the trainer follows the player; the double wild battles
  show them beside the player with their back pic.

## Balance report

Informational. The
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) is to show,
for a chosen world progress, accepted lineup, and partner: each trainer's
momentum and candidate haunts, each haunt's score table, the fill order,
and the placement, plus a sweep over world progress 0-160 showing how often
each trainer sits at each haunt and how many haunts are empty. It asserts no
target. It is not built yet ([Later](#later)).

## Acceptance

Required implementation evidence (not yet run):

1. **Catalog.** Every haunt has valid tags (one or two activities from the
   shared list, and capacity 1 in v0), existing maps, an active meeting
   spot, and its quest details (a walk's start and exit, two or three lost
   spots, each on a reachable tile with a hint, a catch species on its wild
   table); every notable trainer has
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
5. **Talk flow.** Every talk is a greeting, then the proposal or `QUIRK`, then
   `BYE`, with no menu, battle offer, or team-up at any stage. The greeting
   follows the stage: a Stranger gets `MEET`, or `HEARD` when the player's fame
   reaches them (the player's TR at least `min(80, their TR − 10)`, a reign at
   any league, or being a Master), Met gets `AGAIN`, Friend `HELLO`, Close
   `CLOSE`. A first talk makes the trainer Met and proposes the quest in the
   same talk; while the claim bit is clear every talk proposes it (`ASK`, the
   proposal, [YES / NO]), `NO` changes nothing and the next talk proposes again,
   and a talk that starts a walk ends at `YES`; once the claim bit is set every
   talk is the greeting, `QUIRK`, and `BYE`. No haunt line uses `NOT_YET` or
   `NEWS`. Only the first talk (+1) and a completed quest (+10) add points at a
   haunt; a quest that crosses the Friend threshold hands over the number once.
6. **Quests.** Each quest completes, fails, and retries as specified; a
   walk ends unfinished on whiteout and on reload, and a wrong exit asks
   "Giving up on the walk?" (YES ends it with the trainer's `NO` line, NO
   steps the player back one tile and the walk continues);
   Fly, Teleport, and an Escape Rope are refused during a walk; its wild
   battles are double battles beside the trainer with their best three from
   the runtime partner slot; Catch me one refuses the last able party
   POKéMON; the quiz asks the same questions on every attempt; a One on one
   loss plays the haunt's loss line (or `NOT_READY`), costs nothing, and
   leaves the claim bit clear, and a win adds +10 as a quest, never +20.
   Lost something picks the same active spot for the same placed trainer
   (catalog position mod spot count) and names its hint in the proposal;
   the spot is inert before `YES`, after it is found, and once the claim
   bit is set; after `YES` it is found by facing it and pressing A, stays
   armed across leaving, whiteout, and reload, is re-armed by the next
   placement's `YES`, and makes the Dowsing Machine respond; finding it
   adds no Bag item; the next talk plays the done line, `PRAISE`, the
   reward, and `BYE` for +10; and a reshuffle clears an open search.
7. **Claims.** A reward is given once per placement; a changed placement
   reopens it; a full Bag, a cancelled lesson, or no POKéMON able to learn
   keeps it open and leaves the reward counter unchanged.
8. **Reward pools.** The reward never depends on the quest type. Quests
   with one trainer at different haunts pay that trainer's pool in order,
   one entry each; a gated next entry or a used-up pool pays the fallback
   (prize money equal to a win over the trainer at current TR) and leaves
   the counter alone; entries open by world progress, never the trainer's
   TR, so a quest just before and just after a gate pays the fallback and
   then the entry, and Lance's and Agatha's pools open gradually. A lesson
   teaches the first move in pool order the chosen POKéMON can learn and
   doesn't know, and offers only POKéMON with such a move. Brock's first
   quest gives PEWTER CRUNCHIES and Giovanni's gives a NUGGET.
9. **Dialogue.** Every assembled line resolves its slots and fits its text
   box with worst-case values; no haunt dialogue carries gossip.
10. **Cameos.** No cameo or Dojo rematch seat remains in Wayfarer; the Dojo
    back room works as a haunt; no stranger battle, rematch, prize money
    beyond the quest fallback, or Battle Points come from haunts.
11. **Save.** A new game and a reload give the saved state described above;
    a search state survives a reload and clears with its haunt's placement;
    corrupt reward counters, claim bits, search states, or placements are
    rejected; removed characters or haunts are pruned; and a counter past a
    shortened pool is lowered to its length.

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
- **Trades at haunts**, under the trade fairness rules, and **item swaps**.
- **Access:** haunts gated by a key item, an HM, or a story beat, beyond
  the elite gate.
- **Gifts from the roster:** a friend giving the player a POKéMON of their
  own line.
- **More friendship sources:** gifts, tag battles beside a trainer, trades,
  and partnering ([friendship](notable-trainers.md#friendship)).
- **Rematches** at haunts or at dedicated rematch spots, where `NOT_YET`
  could gate a trainer whose first fight is still ahead.
- **Gossip at haunts:** a friend telling the player where another trainer
  is, led in by `NEWS`.
- **Other regions:** haunt lists for Johto, Hoenn, and Sevii, and Tate &
  Liza at haunts.
- **Explorer support** for the [balance report](#balance-report).

## References

- [Notable haunts PRD](../prds/notable-haunts.md)
- [Notable trainers](notable-trainers.md)
- [Sevii Masters](sevii-masters.md)
- [Leagues](leagues.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Trainer AI](trainer-ai.md)
- [Notable trainer voice bits](../research/notable-trainer-voices.md)
- [Notable trainer reward pools](../research/notable-trainer-rewards.md)

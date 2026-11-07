# Hoenn independent story beats

PRD: [Hoenn independent story beats](../prds/hoenn-independent-story-beats.md)
Implemented: Yes

## Scope

This specification makes the whole Hoenn PRD precise for the Wayfarer build.
It covers every Hoenn local adventure, the connected villain story, the lines
that change with the order of play, Brendan, May, and Wally, Norman and
Wattson, and the later Rayquaza, Groudon, and Kyogre captures. It also settles
the PRD's five scene-level questions.

The [gate audit](../research/hoenn-story-beat-gates.md) is the evidence for
today's behavior. It names the scripts and lines quoted here. Where it marks a
fact as inferred or not covered, confirm the fact before relying on it.

Only the Wayfarer build matters. Edit the Emerald Hoenn scripts in place, and
add no `IS_WAYFARER` guards or dual paths just to preserve standalone Emerald.
Standalone Emerald may pick up the new behavior. Keep a guard only where a
change calls Wayfarer-only code, such as the Trainer Rating or the player's
origin. Changes are scripts, `map.json` events, and local Wayfarer state.
No `map.bin`, layout, tileset, collision, or elevation edit is authorized.
Prerelease saves need no migration. Packaging the work into one or several PRs
is up to the implementation, as long as each merged step leaves every adventure
completable.

## Behavior

### Shared rules

**Adventure progress is separate.** Each adventure owns its progress. A scene
writes only the progress of its own adventure, and only forward. A Gym win
awards its badge and nothing that belongs to another adventure. Today these
writes cross adventures:

| Write today | Problem |
| --- | --- |
| Roxanne's defeat sets `VAR_RUSTBORO_CITY_STATE` to 1 | It is the only start of the Rustboro theft, and it would rewind a theft already under way. |
| Rustboro's, Dewford's, Mauville's, and Lavaridge's defeats each raise `VAR_PETALBURG_GYM_STATE` | Petalburg Gym is readied for battle only when the count reaches 6. |
| Norman's defeat hides Mr. Briney and relocates Wattson | Both belong to other adventures. |
| Tate & Liza's defeat starts the Space Center invasion, hides the Aqua Hideout cast, reopens Slateport's harbor, and shows Mr. Briney in the shipyard | The first needs Magma Hideout, the second strands the Aqua branch, the third belongs to the harbor scene, and the fourth puts Briney in two places. |
| Steven's Dive scene removes the Seafloor Cavern guard | Seafloor needs the Aqua branch too. |
| Petalburg's Scott meeting sets `VAR_SCOTT_STATE` to 1 | It resets the count when it is not the first meeting. |

Split shared variables into per-adventure Wayfarer state, or guard every write
so it cannot rewind or skip another adventure. Either way, every interleaving
in [Validation](#validation) must hold. Map scripts that read the old
variables for object visibility or dialogue must read the owning adventure's
state. Allocate new flags and variables through the existing Wayfarer
framework, as the Kanto and Johto adventures do.

**State is derived where it can be.** When a scene needs two adventures to be
done, check both where the scene starts, in the map's load or transition
script or in the scene's own trigger. Don't make one adventure's ending script
set the other's flags. A pure check survives every order, a save and reload,
and regional travel.

**Truthful dialogue.** A line stays when it is true. When it recalls an event
the player has not played, use a short first-meeting line or drop the recall.
When it announces a plan the player has already seen carried out, use a short
withdraw line. The variants in this specification give the meaning; final copy
matches the speaker's voice and the existing text style. The named lines are
the ones the audit found. While editing a scene, read its other lines and
apply the same rule.

**An item proves a delivery.** A delivery completes only while the player
holds its item. Without the item the recipient has a first-meeting line, sets
nothing, and gives nothing.

**Reward handoffs.** Every one-time handoff checks for room before giving,
records receipt only after the item or Pokémon is added, and stays claimable
from its owner after a full Bag or party. A repeat conversation never gives it
twice. This applies to the Great Ball, the Letter, the PokéNav, TM Steel Wing,
the Exp. Share, Castform, the Devon Scope, the Go-Goggles, the Basement Key,
TM Thunderbolt, the Magma Emblem, the Master Ball, HM Dive, the Itemfinder,
and HM Fly, among others.

**Hoenn starter.** A player has a Hoenn starter once `VAR_HOENN_STARTER_CHOICE`
holds Treecko, Torchic, or Mudkip. Hoenn-origin players have one from their
opening. Visitors never have one: Birch gives them no starter, as the
[content specification](wayfarer-hoenn-content-port.md) defines.

**Legendary threshold.** The later Rayquaza, Groudon, and Kyogre captures use
the one constant Johto's legendaries use, TR 55 (`WAYFARER_LEGENDARY_TR` in
`game/include/wayfarer_sevii_story.h`). Compare with `GetTrainerRating()`.

**Scott.** Petalburg's meeting raises `VAR_SCOTT_STATE` with `addvar`, as the
other twelve do. Scott's meetings may then happen in any order.

### Rustboro theft and Peeko

- The street scene outside Devon (`RustboroCity_EventScript_StolenGoodsScene`)
  plays the first time the player crosses its five tiles while the theft has
  not started. Roxanne's badge neither starts it nor is needed for it. Remove
  the Gym's write of `VAR_RUSTBORO_CITY_STATE`.
- The scene plays once, and beating Roxanne at any point never starts it,
  replays it, or rewinds it. Nothing reads Rustboro state 0 today, and the two
  writers that jump the state to 8 (Briney's sail and the Route 104 rival)
  cannot run before state 6, so keying the five triggers on state 0 is enough.
- The scene shows the employee, the grunt, Peeko, and Briney as it does today,
  including for visitors, whose baseline hides them.
- From there the adventure keeps its order: the employee's request, the
  Rusturf Tunnel grunt and the Devon Parts, Peeko's return to Briney, the
  Great Ball, and Mr. Stone's Letter and PokéNav.
- The employee's robbery line recalls the player's help in Petalburg Woods.
  Trace the Woods scene's state. Without that scene, he asks for help with no
  recall.
- Mr. Briney keeps sailing until the Hoenn Hall of Fame, which puts him on the
  S.S. Tidal as in Emerald and retires him from sailing at the same moment
  (`EventScript_HideMrBriney`). Norman's defeat no longer
  hides him, and Tate & Liza's defeat no longer shows the second Briney in
  Stern's shipyard. That shipyard cameo is dropped: it is the price of Briney
  never standing in two places and never retiring in the middle of an errand.

### Devon deliveries

- Steven takes the Letter and Stern takes the Parts only from a player who
  holds the item (see [Shared rules](#shared-rules)). The rewards and state
  each delivery sets today stay the same.
- The deliveries complete in either order, and neither needs the other. From
  Dewford, Briney offers Slateport once the Letter is delivered or the player
  holds the Parts.
- Dock mentions the Devon Goods only to a player who holds the Parts.
- Talking to Dock sends Team Aqua's queue away from the museum door
  (`FLAG_HIDE_SLATEPORT_CITY_TEAM_AQUA`) for every player and on every branch
  of his script. Today only his default branch does, and the queue stands on
  the only tiles that reach the door, so a player with the Mind Badge or a
  Kanto or Johto Hall of Fame who talks to him first never opens the museum.
  Hide the queue before his script branches. The queue is also gone once the
  harbor scene has played.
- Dock's Mind Badge line, that Mr. Briney has joined the shipyard, goes with
  the shipyard cameo (see [Rustboro theft and Peeko](#rustboro-theft-and-peeko)).
- Ordinary access to the museum, the shipyard, the harbor, Granite Cave, and
  transport stays open at every step. The one exception is Slateport's ferry
  berth while the harbor scene is waiting to play, described below.

**Stern.** Stern stands in one place at a time:

| State | Where Stern is | Parts delivery |
| --- | --- | --- |
| Before Magma Hideout | Museum 2F. | The existing confrontation with the grunts and Archie. |
| Magma Hideout done, harbor scene not yet played | The Slateport street, then the harbor, as today. The museum Stern is hidden. | Waits for the harbor scene. |
| Harbor scene played | The harbor. | A plain handover: Stern takes the Parts with a short line. There are no grunts and no Archie. |

The plain handover sets what the museum delivery sets, so every later reader
of `FLAG_DELIVERED_DEVON_GOODS` behaves the same, and it hides the museum's
grunts. One thing differs: Scott's meeting outside the museum is staged at the
museum door and does not play after a harbor handover. Count it toward
`VAR_SCOTT_STATE`, register his call, and mark it done, so nothing waits on
it. Stern stays at the harbor after the harbor scene, as today, and no later
scene may hide him from a player who still holds the Parts. Completing the
harbor scene never removes the Parts or their delivery.

The street Stern greets the player as an old acquaintance back from an
expedition. When the Parts are undelivered and the two have not met, he opens
with a first-meeting line.

**The harbor.** The harbor's patrons, its ferry attendant, and Slateport's
ferry berth are closed only while the harbor scene is waiting to play (harbor
state 1). They return when the scene ends. Tate & Liza's defeat no longer
reopens them: today it is the only thing that does, so a player who wins that
badge first loses the berth for good.

### Meteor Falls and Mt. Chimney

- The adventure keeps its start, with no badge or story requirement, and its
  internal order: Meteor Falls, Mt. Chimney, and the Meteorite's return to
  Professor Cozmo.
- It comes before the central villain story (see
  [Mt. Pyre](#mt-pyre-and-magma-hideout)). Maxie's whole purpose on Mt. Chimney
  is to wake what sleeps in the volcano, which cannot be told once Groudon has
  woken. Its scenes therefore never play after Magma Hideout and need no late
  variants.
- At Meteor Falls, Archie recalls the museum only when the museum
  confrontation happened.

### Weather Institute

- The occupation stays available from the first visit until the player clears
  it, in any order with every other adventure. Castform's handoff stays as it
  is.
- The grunt's and Shelly's Mt. Pyre lines play only while they are a true
  lead: Mt. Chimney is done and the Mt. Pyre theft has not happened. Before
  Mt. Chimney the summit is empty, and after the theft
  (`FLAG_RECEIVED_RED_OR_BLUE_ORB`) the plan is carried out. In both cases
  they use a withdraw variant: the order came to pull out, and Shelly leaves
  without naming Mt. Pyre or Team Magma's plans.
- The Route 119 rival scene has its own condition (see
  [Brendan and May](#brendan-and-may)).

### Steven and Kecleon, and the desert survey

Both meet the design. Keep the Route 120 scene's early and late greetings and
its Kecleon recovery, and keep the desert survey's and Lavaridge's shared
Go-Goggles check. The survey and that check were compiled out of Wayfarer, so
both are compiled in with this work.

### Wattson and New Mauville

- Wattson stays in his Gym until the player holds the Dynamo Badge and has
  taken TM Shock Wave from him. After that, and until TM Thunderbolt is
  claimed, he is outside in Mauville City with the New Mauville request, as in
  Emerald. Norman plays no part: remove the `FLAG_DEFEATED_PETALBURG_GYM`
  condition from the relocation and the relocation call from Norman's defeat.
- Where he stands is decided only on entering Mauville City and on entering
  the Gym, from those three facts. Remove the relocation call from the badge
  scene: it runs before TM Shock Wave is given, and he must not vanish in
  front of the player.
- A full Bag in the badge scene leaves TM Shock Wave with the Gym's Wattson.
  He does not step outside while it is owed.
- Once TM Thunderbolt is claimed he is back in the Gym for good, including for
  a player who walks straight from the street into the Gym.
- The Basement Key, New Mauville's door and generator, TM Thunderbolt, his
  return to the Gym, and his rematch stay as they are.
- His first badge battle is never out of reach, since he leaves the Gym only
  after it.

### Norman and Wally

- Norman accepts the challenge once Wally's catching tutorial is done. Remove
  the four-Hoenn-badge condition and the advice lines that send the player to
  other Gyms.
- The end of the tutorial readies the Gym for battle. Today the Gym is ready
  only when four other Gyms have raised `VAR_PETALBURG_GYM_STATE` to 6. At
  that state the Gym opens its rooms and puts Norman at the back, and the same
  moment runs `Common_EventScript_ReadyPetalburgGymForBattle`, which shows the
  Gym guide and expands Petalburg Mart's stock. At the end of the tutorial,
  set the state to 6, call that script, and reload the Gym around the player,
  since its load script has already run for this visit. The rooms open and
  Norman stands at the back: left at the entrance he would battle there, and
  his victory scene is staged for the back room. Remove the other Gyms'
  increments.
- No Norman line counts or asks for badges. `NormanIntro` and
  `VisitorNormanIntro` open without "four GYM BADGES" and, for the visitor,
  without "as I promised".
- The tutorial and the west-entrance redirect that leads to it stay. The
  native and visitor variants stay as the regional-origin design selects them.
- No other Gym's defeat changes Petalburg Gym's state.
- Juan's line after his battle counts the badges left ("There remains but one
  BADGE… FORTREE"). Make it badge-neutral.
- Wally's Mauville battle is offered only after the Petalburg tutorial, since
  his lines recall it. Verdanturf keeps its Mauville prerequisite. On Victory
  Road, Wally appears only after the tutorial, and his "losing to you that
  time" recall plays only after the Mauville win.
- A waiting Wally scene never blocks the Mauville Gym, Wattson's request, or
  Victory Road.

### Birch's opening and his rating

Birch's opening is over when he has given the Pokédex, for a Hoenn-origin
player, or when he has been rescued, for a visitor. A visitor's rescue uses
their own party. In his lab he thanks them and rates their Pokédex, and that is
the whole scene: no starter, no gift, no rival. This matches Elm's and Oak's
visitors. A visitor who somehow reaches him with no Pokédex is handed one first,
so there is something to rate.

- `ProfBirch_EventScript_CheckOpeningOver` is the one test, and it derives the
  answer from the origin and those two facts, so saves need no new state.
- His rating and his place follow that test, and a Hoenn League win also puts
  him in his lab, so the National Dex and Johto starter scenes have him even for
  a visitor who never rescued him. Petalburg Gym and Wally play no part in the
  test.
- Once his opening is over he roams as in Emerald (lab, Route 101, Route 103 by
  day): the clock advances Hoenn's copy of `VAR_BIRCH_STATE`, and Petalburg
  Gym's `InitBirchState` restarts his week in the lab. Before the opening is
  over the clock does not move him; the opening's own scenes place him. After a
  Hoenn League win, and while a rescued visitor's thanks is still waiting in the
  lab (`VAR_BIRCH_LAB_STATE` 2), he stays in his lab. His Route
  110 registration scene is a separate beat that this design doesn't change, and
  it still talks as if the player knows him and the rival.
- A Hoenn-origin player can ask for the rating right after the Pokédex, in the
  same visit.
- Birch's Treecko, Torchic, and Mudkip stay for a Hoenn-origin player. His
  Johto starters for a completed Hoenn Pokédex are a late reward for every
  origin and are not part of the opening.

### Brendan and May

Their rivalry belongs to Hoenn-origin players, as Blue's and Silver's belong to
their own. For those players nothing here changes.

A visitor never battles them, never advances a rival chapter, and is never told
of a past meeting. No rival scene reads the Hoenn starter for a visitor or
depends on Birch. Where a scene carries a gift, the visitor gets a short scene
once, with no battle. The rival introduces themself the first time, by name and
as Professor Birch's child, hands the gift over, and leaves. A later scene skips
the introduction.

| Scene | Hoenn origin | Visitor |
| --- | --- | --- |
| Route 103 | Talk, battle | The rival is not shown. |
| Oldale and the lab handoff | Follow the Route 103 win | They never play. |
| Rustboro and the Route 104 fallback | Triggers, optional battle | The rival is not shown; the triggers do nothing. |
| Route 110 | Forced trigger, battle, the Itemfinder | The trigger plays once with no battle and gives the Itemfinder. |
| Route 119 | Forced trigger, battle, HM Fly, then Scott | The trigger plays once with no battle and gives HM Fly. Scott does not walk on. |
| Lavaridge | After Flannery, the Go-Goggles | Plays after Flannery with no battle, with the visitor's lines. |
| Lilycove | Talk, optional battle | The rival is not shown. |
| National Dex scene | The rival is present | The rival is not shown. Birch's lines do not change, including those that mention a Pokédex he gave and the rival's help. |
| Their mother | Says they went to Route 103 | Says they are too busy to notice the visit. |

- `WayfarerIsHoennOrigin` is the test. `FLAG_MET_HOENN_RIVAL_AS_VISITOR`
  remembers the introduction.
- A visitor who already holds the Itemfinder or HM Fly gets no scene there, and
  the introduction moves to the next one. These are the same items Johto gives.
  With no room for the gift the scene waits for a later visit.
- Scott's Route 119 line says the player just beat the rival, so he stays away
  from a visitor. The meeting still raises `VAR_SCOTT_STATE`, so his top reward
  stays reachable, including for a visitor who skips the scene by holding Fly.
- **Triggers must not run a special and end.** The engine treats that as a
  started script, and because the player's step has not finished it runs again
  every frame, which freezes the player. So a trigger reads a flag that the
  map's transition script set, and that script makes the origin check:
  `FLAG_HIDE_ROUTE_110_RIVAL`, `FLAG_HIDE_RUSTBORO_CITY_RIVAL` (read by Route
  104 too), and Route 119's `FLAG_TEMP_1`.
- Lavaridge's scene is a frame script on `VAR_LAVARIDGE_TOWN_STATE` 1, staged for
  a player at the Gym door or the Pokémon Center's front door. It has no other
  layout. Flannery's defeat and a full Bag park it at state 4, and walking out
  of one of those two doors arms it, so leaving the Gym after Flannery plays it
  at once. The Pokémon Center's back exit, the road, and Fly do not arm it.
- No rival scene blocks a road, the Weather Institute, the Devon errands, or
  Flannery's Gym.
- The Mauville Game Corner girl's spare doll is the player's Hoenn starter, so a
  visitor never gets it. The doll stays a coin prize. No reader of
  `VAR_HOENN_STARTER_CHOICE` may freeze a player or default to a starter.
- Rustboro and Route 104 stay missable for a Hoenn-origin player as in Emerald:
  sailing with Briney before meeting the rival there skips that scene.

### Mt. Pyre and Magma Hideout

- The Mt. Pyre theft starts only after Mt. Chimney
  (`FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY`). Until then Archie and his grunts are
  not on the summit and its three triggers do nothing. The rest of Mt. Pyre is
  open as usual.
- With Archie gone the player can reach the old couple before the theft, which
  today they cannot. The old lady's line at that state already mourns the
  stolen orbs. Before the theft she speaks of the orbs she and her husband
  guard.
- Derive this on entering the summit. The baseline never hides Archie and the
  grunts, so the summit must hide them before Mt. Chimney and show them after
  it, until the theft has played.
- The theft, the Magma Emblem, the Jagged Pass entrance, and Magma Hideout
  keep their scenes and what they set. None needs a badge, the Devon errands,
  or the Weather Institute.
- The later return of the orbs (`VAR_MT_PYRE_STATE` 2, set in Sootopolis) is
  unchanged.
- Route 121's Aqua grunts and their "move out" scene are there only while
  Mt. Pyre is a true lead (see [Weather Institute](#weather-institute)). They
  fill the passage south, so entering the route hides them otherwise. Before
  Mt. Chimney their trigger does nothing and keeps its state, so the scene can
  play later. After the theft they are gone without it.

### The two branches

After Magma Hideout, the submarine pursuit and the Mossdeep story are sibling
adventures. The player may finish either first, alternate between them, and
leave Hoenn between their steps.

**Submarine theft and Aqua Hideout.**

- The Slateport street and harbor scenes start after Magma Hideout, as today.
- Aqua Hideout stays fully playable until Matt's battle is won. Only Aqua's
  own escape removes its grunts and Matt: the escape scene sets
  `FLAG_HIDE_AQUA_HIDEOUT_GRUNTS`, and Tate & Liza's defeat no longer does.
- Lines that tell of the hideout's fall follow Aqua's escape
  (`FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE`), not the seventh badge: the harbor
  Stern's, and the Lilycove lines in the motel, the city, and the Pokémon
  Center.
- The hideout's entrance guard sends the player to Mt. Pyre only while that is
  a true lead (see [Weather Institute](#weather-institute)). Otherwise he says
  only that the boss is away.

**Mossdeep and the Space Center.**

- Tate & Liza's Gym is always open, and its win awards the badge.
- The Space Center invasion starts only when the player holds the Mind Badge
  and Groudon has woken (`FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT`), in either
  order. Check both on entering Mossdeep City, and only while the invasion has
  never started (`VAR_MOSSDEEP_CITY_STATE` 0), so it starts once. A player who
  wins the Gym first finds Mossdeep quiet, and the invasion begins on their
  first visit after Magma Hideout.
- Everything the Gym's defeat writes for the invasion moves to that check: the
  city and Space Center states, the Magma objects, and Steven's placement.
- From there the invasion, Steven's tag battle, and Steven's house keep their
  order and what they set. HM Dive and Hoenn's Dive authorization still come
  from Steven. Nothing in the branch checks Aqua Hideout.
- `FLAG_DEFEATED_MAGMA_SPACE_CENTER` is cleared by a later call, so nothing
  uses it to mean the branch is done. Use the receipt of Steven's Dive,
  `FLAG_RECEIVED_HM_DIVE`.

### Seafloor Cavern and the weather crisis

- Seafloor Cavern's story starts only when both branches are done: Aqua has
  escaped in the submarine (`FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE`), and the
  player has Steven's Dive (`FLAG_RECEIVED_HM_DIVE`).
- The Hall of Fame must not overwrite a pending Dive. It writes
  `VAR_STEVENS_HOUSE_STATE` only at 0; states 1 and 3 still owe the HM. So a
  Hoenn League win between the Space Center and Steven's house loses nothing.
- As a result Steven can be home after a League win. His letter says he has
  left, so the letter and the Beldum stay out of sight while he is home and
  appear once he has gone, in Seafloor Cavern.
- That flag is also Hoenn's Dive authorization, and only Steven sets it. A
  player therefore cannot reach the cavern without the Mossdeep branch, and
  the state the guard exists for is "Steven's Dive, but Aqua has not escaped".
- The Aqua grunt in front of the cavern door stays until both are true. He
  stands on the only tile that reaches the door. Check both on entering the
  cavern entrance. Steven's Dive scene no longer removes him.
- Room 9's scene checks the same two conditions before it starts.
- From Room 9 on, the story keeps its order: Route 128, Sootopolis, the Cave
  of Origin, the Sky Pillar awakening, and the return to Sootopolis.
- No step requires catching a legendary Pokémon or a Trainer Rating.
- Juan's Gym opens when the crisis is resolved, as today, with no other badge
  condition.

### Dialogue variants

| Speaker | Line today | Variant |
| --- | --- | --- |
| Devon employee, Rustboro street | Recalls the player's help "in PETALBURG WOODS". | Without the Woods scene: drop the recall. |
| Devon employee, Route 116 | Recalls the player's help "in PETALBURG WOODS". | Without the Woods scene: drop the recall. |
| Mr. Briney, his house | "You have a LETTER bound for DEWFORD and a package for SLATEPORT", and "You're to deliver a LETTER to DEWFORD and a package to SLATEPORT." | He names only what is still to deliver. |
| Archie, Meteor Falls | "Didn't I see you before? At SLATEPORT's MUSEUM?" | Without the museum confrontation: drop the recall. |
| Archie, Oceanic Museum | Speaks as on a first meeting. | If Meteor Falls came first: confirm the lines still read true, and add a short recall only if they do not. |
| Stern, Slateport street | "Oh! {PLAYER}! You're looking great!… our last seafloor exploration". | Parts undelivered and not yet met: a first-meeting line. |
| Stern, Slateport harbor | "The same thugs" tried to rob the museum. | Without the museum confrontation: a neutral line about the thieves. |
| Stern, late Parts delivery | The museum confrontation and "now we can prepare for the expedition". | After the harbor scene: he thanks the player and takes the Parts, with no expedition line. |
| Aqua grunt and Shelly, Weather Institute | "A TEAM MAGMA mob… headed for MT. PYRE!", "We have to hurry to MT. PYRE, too!" | Before Mt. Chimney or after the Mt. Pyre theft: the order came to pull out. |
| Aqua Hideout entrance guard | The boss is "on his way to MT. PYRE". | Before Mt. Chimney or after the theft: the boss is away. |
| Aqua grunts, Route 121 | "We're to move out to MT. PYRE!" | Before Mt. Chimney or after the theft: no line; the grunts are absent. |
| Old lady, Mt. Pyre summit | "The BLUE ORB and RED ORB taken by those sinister men…" | Before the theft: the orbs are still in her care. |
| Sailor, Mossdeep City | "someone came along and wiped the floor with them!" | Before Aqua's escape: drop the recall. |
| Aqua grunt, Seafloor Cavern entrance | "I hear they were spotted near MOSSDEEP." | He is met only after Steven's Dive: the door stays shut until word comes from the Lilycove hideout. |
| Lilycove motel, city, and Pokémon Center | Team Aqua's hideout was busted; they are gone for good. Keyed on the Mind Badge. | Keyed on Aqua's escape. |
| Juan, after his battle | "There remains but one BADGE… FORTREE". | Badge-neutral. |
| Norman, intro | "So, you did get four GYM BADGES." | Badge-neutral. |
| Wally, Victory Road | "Losing to you that time…" | Without the Mauville win: drop the recall. |

Maxie's Space Center lines need no variant: Mt. Chimney and Magma Hideout
always precede them.

### Legendary captures

The crisis, Rayquaza's awakening, and Juan need no Trainer Rating. The later
captures do. "TR 55" here means `WAYFARER_LEGENDARY_TR`: 55 on today's scale and
120 on the v0 scale that Wayfarer runs.

- **Rayquaza.** It waits at the top of Sky Pillar once the crisis is resolved,
  as today. Interacting with it below TR 55 gives a not-ready line and leaves
  it there. At TR 55 the battle starts.
- **Groudon and Kyogre.** The Weather Institute scientist reports abnormal
  weather once the crisis is resolved and the player has TR 55. He is the only
  source of the report, so these two also need the Institute rescued. Remove the
  `FLAG_SYS_GAME_CLEAR` check from the scientist and from the 1F hint: no Hall
  of Fame in any region is needed, and none is enough. Below the threshold the
  scientist keeps the line he has when there is no abnormal weather. The
  scientist is the only activator and the Trainer Rating never falls, so Terra
  Cave and Marine Cave need no second check.
- "Crisis resolved" is the completed return to Sootopolis
  (`VAR_SKY_PILLAR_STATE` at least 2).
- The not-ready line follows Zapdos's: the Pokémon watches warily, and the
  player needs more experience before challenging it.
- An unmet requirement never consumes an encounter. Fleeing, losing, and
  catching behave as today.
- Other Hoenn legendary and event encounters are untouched.

### Related documents updated with this work

When implementing, update:

- the [content specification](wayfarer-hoenn-content-port.md)'s "Main campaign
  and regional progression" section, which keeps Emerald's campaign in its
  intended order and describes the Norman and Dynamo Badge relocation;
- the [league circuit specification](wayfarer-interregional-league-circuit.md)'s
  Norman and Wattson conditions (lines 76–77 and 87) and its validation line
  for Wattson and Norman in both orders;
- the [traversal specification](emerald-open-world-region-traversal.md), if
  its Lilycove, Mt. Pyre, Slateport, or Aqua statements change;
- this specification's `Implemented` marker and the PRD's status line.

## Validation

Mechanical checks extend the existing script and journey tests. Cover:

- **Theft:** start it with zero badges. Beat Roxanne before it, during it, and
  after it; sail with Briney and meet the Route 104 rival before it. It plays
  once and never rewinds.
- **Deliveries:** visit Stern and Steven without their items, then return with
  them. Deliver the Letter and the Parts in both orders, with travel and a
  save and reload between. Each delivery pays once.
- **Museum door:** as a player with no Parts, talk to Dock. The Aqua queue
  leaves and the museum can be entered. Repeat with the Mind Badge, and with a
  Kanto Hall of Fame, before the first talk.
- **Late Parts:** finish Magma Hideout and the harbor scene first, then hand
  the Parts to Stern at the harbor. No confrontation plays, Scott's count
  includes the museum meeting, and harbor progress does not regress. Stern is
  never in two places.
- **Harbor:** win the Mind Badge before Magma Hideout, then play the harbor
  scene. The attendant and the ferry berth return when the scene ends. Do the
  same without the Mind Badge. Sail toward Slateport while the scene is
  waiting.
- **Briney:** beat Norman in the middle of the Peeko rescue and before either
  delivery, and win the Mind Badge. Briney still sails and is in one place.
- **Weather Institute:** clear it before and after the Mt. Pyre theft and
  after the crisis. The right variant plays, and Castform is claimable once,
  including after a full party and box.
- **Wattson:** win the Dynamo Badge with and without Norman's badge. Wattson
  is in the Gym before the badge and outside after it. Win it with a full Bag;
  he stays in the Gym until TM Shock Wave is taken. Finish New Mauville and
  claim TM Thunderbolt once. He returns to the Gym and stays there on every
  later visit.
- **Norman:** finish the tutorial and battle Norman with zero Hoenn badges, as
  a native and as a visitor. The Gym's rooms are open, its trainers are there,
  the Gym guide is present, and no line mentions four badges.
- **Wally:** visit Mauville and Victory Road before the tutorial; Wally is
  absent and nothing is blocked. Reach Victory Road with and without the
  Mauville win.
- **Rivals:** as a visitor who never met Birch, collect Fly on Route 119, the
  Go-Goggles at Lavaridge, and the Itemfinder on Route 110, in that order. No
  battle, one introduction, nothing freezes. Cross the Rustboro and Route 104
  triggers and visit Route 103 and Lilycove: no rival, and the player walks on.
  Repeat holding the Itemfinder and Fly: no scene, and Scott's count still
  rises. Play the National Dex scene as a visitor: no rival. As a Hoenn-origin
  player, the Route 110 and 119 battles still play.
- **Birch:** a visitor is thanked and rated right after the rescue with no
  starter offered; a Hoenn-origin player is rated right after the Pokédex. Never
  enter Petalburg Gym.
- **Steven:** win the Hoenn League at Steven's-house state 1, then collect Dive.
- **Mt. Pyre:** climb to the summit before Mt. Chimney. No Aqua, no theft,
  the old lady speaks of orbs still in her care, and access is ordinary.
  Finish Mt. Chimney and return; the theft plays. Cross Route 121 before
  Mt. Chimney, while it is a true lead, and after the theft.
- **Branches:** after Magma Hideout, finish Aqua Hideout before Mossdeep, then
  the reverse. Win Tate & Liza's badge before Magma Hideout, between the
  branches, and last. Aqua Hideout stays completable in every order and
  empties after Matt's battle. The Space Center never starts before Groudon
  wakes, starts once, and never replays.
- **Seafloor:** reach the cavern entrance with Steven's Dive and Aqua not yet
  escaped; the guard blocks the door. Finish Aqua Hideout and return; it is
  open. The states without Steven's Dive cannot be reached in play; cover them
  from arranged state only.
- **Crisis:** finish it and beat Juan below TR 55. Rayquaza's awakening awards
  and requires no capture.
- **Captures:** below and at TR 55, try Rayquaza at Sky Pillar and the Weather
  Institute scientist, before and after the crisis, with and without a Kanto
  or Johto Hall of Fame. Flee from each and return.
- **Scott:** meet him in Petalburg last. The count is not reset.
- **Persistence:** save and reload and regional travel at each multi-step
  boundary. Full-Bag and full-party retries for every one-time handoff listed
  above.

## References

- [Hoenn story beat gates](../research/hoenn-story-beat-gates.md)
- [Regional story independence audit](../research/regional-story-beat-independence.md)
- [Johto independent story beats](johto-independent-story-beats.md)
- [Wayfarer Hoenn content specification](wayfarer-hoenn-content-port.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)

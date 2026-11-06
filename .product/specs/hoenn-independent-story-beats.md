# Hoenn independent story beats

PRD: [Hoenn independent story beats](../prds/hoenn-independent-story-beats.md)
Implemented: No

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
| Briney's sail and the Route 104 rival set `VAR_RUSTBORO_CITY_STATE` to 8 | They would end a theft that has not started. |
| Norman's defeat hides Mr. Briney and relocates Wattson | Both belong to other adventures. |
| Tate & Liza's defeat starts the Space Center invasion and hides the Aqua Hideout cast | The first needs Magma Hideout; the second strands the Aqua branch. |
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
opening. Visitors have one only after Birch's rescue and the choice in his
lab, as the [content specification](wayfarer-hoenn-content-port.md) defines.

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
- The scene plays once. Beating Roxanne, sailing with Briney, and the Route
  104 rival scene never start it, replay it, end it, or rewind it. Guard the
  two writers that jump the Rustboro state to 8, or key the theft on its own
  state.
- The scene shows the employee, the grunt, Peeko, and Briney as it does today,
  including for visitors, whose baseline hides them.
- From there the adventure keeps its order: the employee's request, the
  Rusturf Tunnel grunt and the Devon Parts, Peeko's return to Briney, the
  Great Ball, and Mr. Stone's Letter and PokéNav.
- The employee's robbery line recalls the player's help in Petalburg Woods.
  Trace the Woods scene's state. Without that scene, he asks for help with no
  recall.
- Norman's defeat no longer hides Mr. Briney. Briney keeps sailing while the
  Peeko rescue or either delivery is unfinished.

### Devon deliveries

- Steven takes the Letter and Stern takes the Parts only from a player who
  holds the item (see [Shared rules](#shared-rules)). The rewards and state
  each delivery sets today stay the same.
- The deliveries complete in either order, and neither needs the other. From
  Dewford, Briney offers Slateport once the Letter is delivered or the player
  holds the Parts.
- Dock mentions the Devon Goods only to a player who holds the Parts.
- Ordinary access to the museum, the shipyard, the harbor, Granite Cave, and
  transport stays open at every step.

**Stern.** Stern stands in one place at a time:

| State | Where Stern is | Parts delivery |
| --- | --- | --- |
| Before Magma Hideout | Museum 2F. | The existing confrontation with the grunts and Archie. |
| Magma Hideout done, harbor scene not yet played | The Slateport street, then the harbor, as today. The museum Stern is hidden. | Waits for the harbor scene. |
| Harbor scene played | The harbor. | A plain handover: Stern takes the Parts with a short line. There are no grunts and no Archie. |

The plain handover sets everything the museum delivery sets, so Scott's
meeting outside the museum and every later reader of `FLAG_DELIVERED_DEVON_GOODS`
behave the same, and it hides the museum's grunts. Stern stays reachable while
the Parts are undelivered: no later scene may hide him from a player who still
holds them. Completing the harbor scene never removes the Parts or their
delivery.

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
- Once the Mt. Pyre theft has happened (`FLAG_RECEIVED_RED_OR_BLUE_ORB`), the
  grunt's and Shelly's Mt. Pyre lines become a withdraw variant: the order
  came to pull out, and Shelly leaves without naming Mt. Pyre or Team Magma's
  plans.
- The Route 119 rival scene has its own condition (see
  [Brendan and May](#brendan-and-may)).

### Steven and Kecleon, and the desert survey

Both already meet the design. Keep the Route 120 scene's early and late
greetings and its Kecleon recovery, and keep the desert survey's and
Lavaridge's shared Go-Goggles check. No change.

### Wattson and New Mauville

- Wattson stays in his Gym until the player holds the Dynamo Badge. After
  that, he is outside in Mauville City with the New Mauville request, as in
  Emerald. Norman plays no part: remove the `FLAG_DEFEATED_PETALBURG_GYM`
  condition from the relocation and the relocation call from Norman's defeat.
- The move never happens under the player's eyes. Apply it when the player
  enters Mauville City or the Gym, not inside the badge scene.
- The Basement Key, New Mauville's door and generator, TM Thunderbolt, his
  return to the Gym, and his rematch stay as they are.
- His first badge battle is never out of reach, since he leaves the Gym only
  after it.

### Norman and Wally

- Norman accepts the challenge once Wally's catching tutorial is done. Remove
  the four-Hoenn-badge condition and the advice lines that send the player to
  other Gyms.
- No Norman line counts or asks for badges. `NormanIntro` and
  `VisitorNormanIntro` open without "four GYM BADGES" and, for the visitor,
  without "as I promised".
- The tutorial and the west-entrance redirect that leads to it stay. The
  native and visitor variants stay as the regional-origin design selects them.
- No other Gym's defeat changes Petalburg Gym's state.
- Wally's Mauville battle is offered only after the Petalburg tutorial, since
  his lines recall it. Verdanturf keeps its Mauville prerequisite. On Victory
  Road, Wally appears only after the tutorial, and his "losing to you that
  time" recall plays only after the Mauville win.
- A waiting Wally scene never blocks the Mauville Gym, Wattson's request, or
  Victory Road.

### Brendan and May

A player without a Hoenn starter does not meet Brendan or May. Every scene
below waits, and plays once the player has a starter and meets its other
conditions.

| Chapter | Kind | Waiting behavior |
| --- | --- | --- |
| Route 103 | Talk | Already waits. |
| Oldale and the lab handoff | Follow the Route 103 win | No change. |
| Rustboro and the Route 104 fallback | Triggers, optional battle | The triggers do nothing; the rival is not shown. |
| Route 110 | Forced triggers; the Itemfinder | The triggers do nothing; the rival is not shown. |
| Route 119 | Forced trigger; HM Fly | The trigger does nothing. |
| Lavaridge | After Flannery; the Go-Goggles | The scene does not start. The desert survey gives this player the Go-Goggles. |
| Lilycove | Talk | The rival is not shown. |

- A waiting trigger returns before it locks the player and leaves its own
  state untouched, so it fires on a later crossing.
- No rival scene blocks a road, the Weather Institute, the Devon errands, or
  Flannery's Gym while it waits.
- The Itemfinder and HM Fly stay rewards of their scenes.
- The same guard applies to every other reader of `VAR_HOENN_STARTER_CHOICE`
  that has no branch for a missing starter, such as the Mauville Game Corner
  doll. No reader may freeze a player or default to a starter.
- Rustboro and Route 104 stay missable as in Emerald: sailing with Briney
  before meeting the rival there skips that scene.
- Chapters key on the starter, not on the Route 103 battle. A player who takes
  a starter and skips Route 103 hears lines that assume that meeting. This is
  accepted.

### Mt. Pyre and Magma Hideout

- The Mt. Pyre theft starts only after Mt. Chimney
  (`FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY`). Until then Archie and his grunts are
  not on the summit, its three triggers do nothing, and the old couple's lines
  describe orbs that are still there. The rest of Mt. Pyre is open as usual.
- Derive this on entering the summit. The baseline never hides Archie and the
  grunts, so the summit must hide them before Mt. Chimney and show them after
  it, until the theft has played.
- The theft, the Magma Emblem, the Jagged Pass entrance, and Magma Hideout
  keep their scenes and what they set. None needs a badge, the Devon errands,
  or the Weather Institute.
- The later return of the orbs (`VAR_MT_PYRE_STATE` 2, set in Sootopolis) is
  unchanged.

### The two branches

After Magma Hideout, the submarine pursuit and the Mossdeep story are sibling
adventures. The player may finish either first, alternate between them, and
leave Hoenn between their steps.

**Submarine theft and Aqua Hideout.**

- The Slateport street and harbor scenes start after Magma Hideout, as today.
- Aqua Hideout stays fully playable until Matt's battle is won. Only Aqua's
  own escape (`FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE`) removes its grunts and
  Matt. Tate & Liza's defeat no longer sets `FLAG_HIDE_AQUA_HIDEOUT_GRUNTS`.
- The harbor Stern's lines follow the story, not the seventh badge.

**Mossdeep and the Space Center.**

- Tate & Liza's Gym is always open, and its win awards the badge.
- The Space Center invasion starts only when the player holds the Mind Badge
  and Groudon has woken (`FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT`), in either
  order. Check both on entering Mossdeep City. A player who wins the Gym first
  finds Mossdeep quiet, and the invasion begins on their first visit after
  Magma Hideout.
- From there the invasion, Steven's tag battle, and Steven's house keep their
  order and what they set. HM Dive and Hoenn's Dive authorization still come
  from Steven. Nothing in the branch checks Aqua Hideout.
- `FLAG_DEFEATED_MAGMA_SPACE_CENTER` is cleared by a later call, so nothing
  uses it to mean the branch is done. Use the receipt of Steven's Dive.

### Seafloor Cavern and the weather crisis

- Seafloor Cavern's story starts only when both branches are done: Aqua has
  escaped in the submarine, and the player has Steven's Dive.
- The Aqua grunt in front of the cavern door stays until both are true. Check
  both on entering the cavern entrance. Steven's Dive scene no longer removes
  him. His line holds in every state and names nothing that has not happened.
  Confirm that he blocks the door; if he does not, close the door another way
  with existing objects.
- Room 9's scene checks the same two conditions before it starts.
- The ability to Dive proves nothing. No story check reads the HM, the
  authorization, or the player's position.
- From Room 9 on, the story keeps its order: Route 128, Sootopolis, the Cave
  of Origin, the Sky Pillar awakening, and the return to Sootopolis.
- No step requires catching a legendary Pokémon or a Trainer Rating.
- Juan's Gym opens when the crisis is resolved, as today, with no other badge
  condition. Read the `FLAG_BADGE06_GET` branch in his script and remove it if
  it gates the battle.

### Dialogue variants

| Speaker | Line today | Variant |
| --- | --- | --- |
| Devon employee, Rustboro street | Recalls the player's help "in PETALBURG WOODS". | Without the Woods scene: drop the recall. |
| Archie, Meteor Falls | "Didn't I see you before? At SLATEPORT's MUSEUM?" | Without the museum confrontation: drop the recall. |
| Archie, Oceanic Museum | Speaks as on a first meeting. | If Meteor Falls came first: confirm the lines still read true, and add a short recall only if they do not. |
| Stern, Slateport harbor | "The same thugs" tried to rob the museum. | Without the museum confrontation: a neutral line about the thieves. |
| Stern, late Parts delivery | The museum confrontation and "now we can prepare for the expedition". | After the harbor scene: he thanks the player and takes the Parts, with no expedition line. |
| Aqua grunt and Shelly, Weather Institute | "A TEAM MAGMA mob… headed for MT. PYRE!", "We have to hurry to MT. PYRE, too!" | After the Mt. Pyre theft: the order came to pull out. |
| Norman, intro | "So, you did get four GYM BADGES." | Badge-neutral. |
| Wally, Victory Road | "Losing to you that time…" | Without the Mauville win: drop the recall. |
| Seafloor Cavern entrance grunt | His guard line. | True in every state. |

Maxie's Space Center lines need no variant: Mt. Chimney and Magma Hideout
always precede them.

### Legendary captures

The crisis, Rayquaza's awakening, and Juan need no Trainer Rating. The later
captures do.

- **Rayquaza.** It waits at the top of Sky Pillar once the crisis is resolved,
  as today. Interacting with it below TR 55 gives a not-ready line and leaves
  it there. At TR 55 the battle starts.
- **Groudon and Kyogre.** The Weather Institute scientist reports abnormal
  weather once the crisis is resolved and the player has TR 55. Remove the
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
- **Late Parts:** finish Magma Hideout and the harbor scene first, then hand
  the Parts to Stern at the harbor. No confrontation plays, Scott's museum
  meeting still happens, and harbor progress does not regress. Stern is never
  in two places.
- **Briney:** beat Norman in the middle of the Peeko rescue and before either
  delivery. Briney still sails.
- **Weather Institute:** clear it before and after the Mt. Pyre theft and
  after the crisis. The right variant plays, and Castform is claimable once,
  including after a full party and box.
- **Wattson:** win the Dynamo Badge with and without Norman's badge. Wattson
  is in the Gym before the badge and outside after it. Finish New Mauville and
  claim TM Thunderbolt once. He returns to the Gym.
- **Norman:** finish the tutorial and battle Norman with zero Hoenn badges, as
  a native and as a visitor. No line mentions four badges.
- **Wally:** visit Mauville and Victory Road before the tutorial; Wally is
  absent and nothing is blocked. Reach Victory Road with and without the
  Mauville win.
- **Rivals:** as a visitor with no starter, cross the Rustboro, Route 104,
  Route 110, and Route 119 triggers, beat Flannery, and visit Lilycove. The
  player keeps control and meets no rival. Take a starter and return; each
  scene plays and pays once. Use the Game Corner doll without a starter.
- **Mt. Pyre:** climb to the summit before Mt. Chimney. No Aqua, no theft, and
  ordinary access. Finish Mt. Chimney and return; the theft plays.
- **Branches:** after Magma Hideout, finish Aqua Hideout before Mossdeep, then
  the reverse. Win Tate & Liza's badge before Magma Hideout, between the
  branches, and last. Aqua Hideout stays completable in every order, and the
  Space Center never starts before Groudon wakes.
- **Seafloor:** reach the cavern entrance with neither branch, with only the
  Aqua branch, with only Steven's Dive, and with both. Only both opens it.
  Reach it by Dive from another source.
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

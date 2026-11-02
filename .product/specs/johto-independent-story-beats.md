# Johto independent story beats

PRD: [Johto independent story beats](../prds/johto-independent-story-beats.md)
Implemented: Yes

## Scope

This specification makes the whole Johto PRD precise for the Wayfarer build.
It covers every Johto adventure, Silver's chapters in Johto and Kanto, the
visitor rules, the legendary ceremony and Pewter's Wing, and the Mahogany road.
It also settles the PRD's three scene-level questions: what starts the Radio
Tower takeover, who introduces the ceremony, and how the Rocket endings read.

The [gate audit](../research/johto-story-beat-gates.md) is the evidence for
today's behavior. It names the scripts and lines quoted here.

Only the Wayfarer build matters. Edit shared HNS scripts in place, and add no
`IS_WAYFARER` guards or dual paths just to preserve standalone HNS. Standalone
HNS may pick up the new behavior. Changes are scripts, `map.json` events, and
local Wayfarer state.
No `map.bin`, layout, tileset, collision, or elevation edit is authorized.
Prerelease saves need no migration. Packaging the work into one or several PRs
is up to the implementation, as long as each merged step leaves every adventure
completable.

## Behavior

### Shared rules

**Origin.** A Johto-origin player is one whose registered Wayfarer origin is
New Bark. Everyone else is a visitor. Every origin check reads the registered
origin, never the saved map, the starter variable, or
`FLAG_JOHTO_STARTER_CHOICE_COMMITTED`.

**Adventure progress is separate.** Each adventure owns its progress. A scene
writes only the progress of its own adventure, and only forward. Today these
town variables are shared and overwritten:

| Variable | Adventures that write it today |
| --- | --- |
| `VAR_AZALEA_TOWN_STATE` | Slowpoke Well, Kurt, Bugsy, Silver, Ilex Forest, the GS Ball |
| `VAR_MAHOGANY_TOWN_STATE` | Lake of Rage, the hideout, Pryce, the merchant, the takeover finale |
| `VAR_ECRUTEAK_CITY_THEATER` | The Theater Rocket, the Surf giver, Elm, the ceremony |
| `VAR_GOLDENROD_CITY_STATE` | Whitney, the radio quiz, the takeover |
| `VAR_ECRUTEAK_CITY_STATE` | Ecruteak setup, the Burned Tower, Morty |
| `VAR_VIOLET_CITY_STATE` | Violet setup, Falkner, Elm's Togepi call, the Route 32 guard |
| `VAR_NEWBARKTOWN_LABSTATE` | Elm's errand, Blackthorn's Elm call, Elm's Master Ball, the ceremony, the post-League call |

Split them into per-adventure Wayfarer state, or guard every write so it
cannot rewind or skip another adventure. Either way, every interleaving in
[Validation](#validation) must hold. Map scripts that read the old variables
for object visibility or dialogue must read the owning adventure's state.
Allocate new flags and variables through the existing Wayfarer framework, as
the Kanto local adventures do.

**Town setup.** Violet, Azalea, Goldenrod, Ecruteak, Olivine, Mahogany, and
Blackthorn run their first-visit setup on the player's first entry into the
town by any route, including ship, Magnet Train, and Fly. Today it runs from
coordinate triggers on approach roads: the Route 31 gate for Violet, and Routes
33, 34, 37, 39, and 42 for the others. Blackthorn's setup lives inside
Mahogany's, so arriving by Routes 45 and 46 skips it. Move each to its own
town's map load, behind a dedicated setup-done flag. Don't reuse the visited
flags: `FLAG_VISITED_BLACKTHORN_CITY` is set on transition, before load
scripts run, and `FLAG_VISITED_MAHOGANY_TOWN` is set only by the setup itself.
Move the Blackthorn and Dragon's Den writes, including the Den cavern hides
for Clair, Lance, and Silver, out of Mahogany's setup into Blackthorn's.
Goldenrod's setup starts with Elm's Togepi call, which a load script cannot
show. Do the state initialization on load, and play the call, which is
Johto-origin only, from an OnFrame entry. Remove the old road triggers.

Setup only initializes state that is still at its initial value, and never
clears a reward, a branch choice, or another adventure's progress, including
on other maps the player may have reached first. Among the current writes:

- Violet must keep `FLAG_RECEIVED_TOGEPI_EGG`, the Sprout Tower state, and the
  Ruins of Alph state.
- Goldenrod must keep `VAR_LUGIA_OR_HOOH` and its gift flags.
- Ecruteak must keep the theater, legendary progress, `FLAG_RELEASED_BEASTS`,
  `FLAG_AMPHAROS_HEALED`, the Route 42 Suicune, and the Tin Tower guard.
- Mahogany and Blackthorn must keep Dragon's Den progress and
  `FLAG_GOT_DRATINI`.

**Reward handoffs.** Every one-time handoff checks for room before giving,
records receipt only after the item or Pokémon is added, and leaves the reward
claimable from its owner after a full Bag or party. A repeat conversation never
gives it twice. This applies to the Wings, Bells, HM Surf, HM Whirlpool, the
Secret Potion, and Elm's Master Ball, among others.

**Legendary threshold.** One constant, TR 55, serves Johto's legendary stages.
Share it with the existing `WAYFARER_BIRD_CAPTURE_TR`
(`game/include/wayfarer_sevii_story.h`) so every legendary tier reads one
number. Compare with `GetTrainerRating()`.

### Visitors

Remove the visitor Elm flow from #75. That means `NewBarkTown_Lab_EventScript_VisitorIntro`
and the visitor starter choice, gift, and storage retry, plus their
`FLAG_JOHTO_STARTER_CHOICE_COMMITTED` writes. For visitors:

- Elm has ordinary professor dialogue. He offers no errand, starter, or gift,
  and never resets lab state.
- Mr. Pokémon has ordinary dialogue and gives no Mystery Egg.
  `MrPokemonHouse_EventScript_WaitForElm` must not hold a visitor.
- Elm's calls are Johto-origin only: the Togepi call after Falkner
  (`VioletCity_hns:9,178-188`), the Goldenrod call, the Blackthorn call, and
  the post-League call. The Violet Pokémon Center aide never appears for
  visitors, so they get no Togepi Egg.
- Silver never appears. Every Silver object and trigger stays hidden or inert,
  and no host adventure waits on him (see [Silver](#silver)). That includes
  the New Bark Silver, which the Hoenn-origin setup reveals today
  (`wayfarer_origin.c:53-56`, `NewBarkTown_hns:26-33`). The Route 22 Celebi
  scene is exempt: the Kanto story PRD owns it, and it plays for every origin
  that meets its conditions.
- The Route 32 guard gives the Miracle Seed on first conversation, with no
  Silver, Togepi Egg, or Violet Gym check. The same applies to Johto-origin
  players. Record it with its own receipt flag. Today its completion is Violet
  state 5, which Falkner's win rewrites to 3.
- Jessica's Tauros trade on Melemele (`Melemele_House_hns:67-69`) uses the
  visitor's own origin starter slot in place of the Johto starter slot. The
  slot order is Grass, Fire, Water in all three regions, so today's mapping
  carries over unchanged.
- Audit every other reader of `FLAG_JOHTO_STARTER_CHOICE_COMMITTED` or
  `VAR_STARTER_MON`, including the starter preview at `game/src/scrcmd.c:2664`.
  No reader may lock a visitor out of content or default to a Johto starter.

Johto-origin players keep the opening and Elm's errand as the start-choice
specification defines them, including the Togepi Egg.

### Silver

Silver's scenes exist only for Johto-origin players. Save:

- the highest battle chapter the player has started;
- a win flag per battle chapter, set for any of the three starter variants;
- whether the player has ever won a Silver battle;
- whether the Olivine (chapter 6) and hideout (chapter 7) scenes played.

A battle chapter has started when its battle begins. The intro text alone, a
trigger with no battle, or a declined offer does not start it.

| Chapter | Scene | Battle | Plays when |
| --- | --- | --- | --- |
| 1 | New Bark static Silver | No | No battle started |
| 2 | Cherrygrove (56,10-11) | Yes | Started chapter ≤ 2 and not won |
| 3 | Sprout Tower 3F | No | Started chapter < 4 |
| 4 | Azalea (11,16) | Yes | Started chapter ≤ 4 and not won |
| 5 | Burned Tower 1F | Yes | Started chapter ≤ 5 and not won |
| 6 | Olivine (15,28-30) | No | Started chapter < 8 |
| 7 | Mahogany hideout B3F (8,13) | No | Started chapter < 8 |
| 8 | Goldenrod Underground (26,5-6) | Yes | Started chapter ≤ 8 and not won |
| 9 | Ecruteak (39,41) | No | Radio Tower resolved and started chapter < 10 |
| 10 | Victory Road 1F | Yes | Started chapter ≤ 10 and not won |
| 11 | Mt. Moon | Yes | Started chapter = 10 or 11, and not won |
| 12 | Indigo Center | Yes, daily | Mt. Moon battle won, as today |

In this table "not won" refers to that chapter's own battle. Each scene also
keeps its local condition, for example being inside the hideout or the
occupation. Azalea's battle still follows Bugsy's win, as today; that is
Silver's own scene, not a host adventure. Starting a battle sets the stored value to that chapter, which
retires every earlier scene. A retired scene's object is hidden and its
trigger does nothing. Retiring grants no money, experience, items, or victory
credit. A loss leaves the stored value unchanged, so the same battle is
offered again. Non-battle scenes never change the stored value.

Replace every existing `FLAG_JOHTO_STARTER_CHOICE_COMMITTED` deferral on
Silver with these checks.

**Host adventures do not wait on him.**

- Burned Tower: when the chapter 5 battle is unavailable (visitor, retired, or
  won earlier), the 1F trigger runs the floor collapse and B1F release without
  Silver, using neutral text. The release, roamers, and Morty's sage follow as
  today. Give Silver his own hide flag; today he shares
  `FLAG_HIDE_BURNED_TOWER_NPCS` with Eusine and Morty.
- Goldenrod Underground: the occupation's step past the Switches room (today
  Goldenrod state 8 → 9) happens without Silver when chapter 8 is unavailable.
- Mahogany hideout and Ecruteak: their scenes are skipped; nothing depends on
  them.

When his battle is available, the forced Burned Tower and Underground battles
keep today's loss behavior: a loss sends the player back, and the host step
waits for a win or for a later chapter to retire the battle.

**Victory Road.** Replace the coordinate trigger with a Silver object on a tile
beside the three-tile lane at `(27-29,7)`, off every traversal path, shown while
chapter 10 is playable. Talking to him asks whether to
battle. Declining or losing never blocks the corridor. Remove his dependency
on `VAR_ROUTE27_STATE` and on the `LeagueCircuit_IsKnownThere` skip. Update
`game/tools/league_circuit/test_scripts.py:287-294`
(`test_corridor_bypass_leaves_story_unwritten`), which pins that skip, so the
corridor still writes no story state while Silver's offer stays available.
If the player has already competed at Indigo, his opener drops "Are you going
to take the Pokémon League challenge?" and opens from "…Don't make me laugh."
"Competed" means the player has started at least one Indigo run, whether they
won or lost. Record it with a new flag set when an Indigo circuit run begins.

**Ecruteak.** The chapter 9 scene leaves the Kimono chain. It plays at its
existing tile once the Radio Tower is resolved. It no longer reads or writes
the theater variable (today it sets theater 5, `EcruteakCity_hns:97`) and no
longer reads Elm's handoff.

**Mt. Moon.** Silver at Mt. Moon is hidden until chapter 11 is playable. This
supersedes the Mt. Moon specification's always-visible Silver
(`frlg-kanto-mt-moon.md:26-29`), the new-game flag clear
(`data/scripts/new_game.inc:438`), and its traversal test
(`test_hns_open_world_traversal.py:523-550`). Crossing the cave never needs him.

**Dialogue that must match what happened:**

| Scene | Line today | Variant condition |
| --- | --- | --- |
| Azalea (chapter 4) | "Is it true that Team Rocket has returned? What? You beat them?" | Without the Well resolved: drop the Rocket question and open from "Then let's see how good you are." |
| Underground (chapter 8) | Before: "You beat me before, didn't you?" | Only if the player has won a Silver battle; otherwise a first-battle line. |
| Underground (chapter 8) | After: "Is what that Lance guy said true?" | Only if chapter 7 (hideout) played, where Silver met Lance; otherwise end before the question. |
| Hideout B3F (chapter 7) | "Didn't I tell you that I was going to destroy Team Rocket?" | Neutral variant when chapter 6 (Olivine) did not play. |
| Ecruteak (chapter 9) | "Don't get cocky just because you defeated Team Rocket!" | Plays only after the Radio Tower is resolved, so the line holds. |
| Victory Road (chapter 10) | After: "I'm beginning to understand what that dragon master said to me…" | Only if chapter 7 played; otherwise end at "What you possess, and what I lack…" |
| Mt. Moon (chapter 11) | "It's been a while… Since I lost to you…" | If the player has never won a Silver battle, drop "Since I lost to you". |

### Slowpoke Well and Azalea

Kurt's introduction, the Well, Proton, and Kurt's reward keep their order in
Well-owned state. Bugsy's badge, Azalea's Silver chapter, Ilex Forest, and the
GS Ball keep theirs. Doing the Well before or after any of them neither
rewinds nor skips anything. Azalea's chapter 4 trigger reads Silver's chapter
rules, not the Well's state.

### Burned Tower, beasts, and Morty

Eusine's 1F introduction plays on first entry however the player reached
Ecruteak. The release follows the [Silver](#silver) rule. Morty's gym-door sage
steps aside only after the release, as today. Morty's badge no longer writes
Burned Tower state. Ecruteak's setup never clears `FLAG_RELEASED_BEASTS` or
the beasts' caught flags.

### Theater Rocket and Surf

The Theater Rocket and the Surf handoff own their progress, separately from
the ceremony. The Surf giver gives HM Surf only after the Rocket is beaten,
from either his conversation or his trigger, once, with the handoff rule. His
talk script stops writing theater state 3 from any other state. Ceremony
staging and Elm no longer set `FLAG_HIDE_ECRUTEAK_CITY_THEATER_NPCS`; the
gauntlet hides only what it needs while it runs and restores it after.

### Lighthouse medicine

Jasmine's request and the pharmacist's Secret Potion keep their order when
played normally. When the player brings the medicine before Jasmine asked:

- The pharmacist's handoff explains that the potion treats a sick Pokémon at
  the Olivine Lighthouse, without claiming Jasmine sent the player.
- At the lighthouse top, Jasmine's delivery opens with a short variant. She
  explains that Amphy is ill before asking whether the medicine will cure it.

Delivery completes the errand and returns Jasmine to her Gym, as today.

### Lake of Rage, the hideout, and Pryce

The Gyarados, Lance, the shop raid, the hideout, and its finale keep their
order in Lake-owned state. Pryce's victory no longer writes it, and the
Gyarados no longer rewinds anything. The Route 43 toll runs only while the Lake
and hideout adventure is unresolved. Lance's lines stay as they are, since they
describe this adventure.

### Mahogany road

Remove the RageCandyBar merchant's road block: the 48 coordinate events at
`(30,12-14)` and their push-back script. He keeps his shop, the Wayfarer TR-mart
menu, and neutral dialogue that no longer refers to the road. Route 44, Ice
Path, and Blackthorn follow ordinary traversal; the Ice Path Kimono push stays.

Update the HNS traversal specification's merchant invariant
(`hns-open-world-region-traversal.md:339`) and the merchant test at
`game/tools/traversal/tests/test_hns_open_world_traversal.py:1116-1144`, which
pin the block.

### Radio Tower takeover

**Start.** The Rocket scout outside the Radio Tower at `(18,22)` is visible from
the player's first Goldenrod visit until the takeover starts. His conversation
keeps "So this is the Radio Tower…" and "What do you want, pest? Scram
already!", then warns: "If you follow him, Team Rocket will take over
Goldenrod. Their leader is tough!" and asks: "Follow the suspicious man into
the Radio Tower?" The warning is the only guard: there is no badge floor.
Accepting starts the occupation, using the setup in
`Mahoganytown_EventScript_RocketTrigger` without the phone call. That setup
shows the Rockets, hides the Goldenrod NPCs, hides the scout and the 2F
policeman, and sets the takeover's first step. The player stays in Goldenrod.
Declining changes nothing, and the nearby Cooltrainer's line about the man
still stands. The scout's start writes no Mahogany state and no
`VAR_TRIGGER_ELM_ROCKET_CALL`; today's setup sets Mahogany 16 and the call
variable 2 (`Mahoganytown_hns:32-33`), and those writes must not carry over.

Remove `MahoganyTown_Gym_EventScript_TryArmRocketTakeover`, its calls from the
Chuck, Jasmine, and Pryce victories and Mahogany's OnLoad, and the
`Mahoganytown_EventScript_RocketTrigger` calls: the OnFrame entries in
Mahogany and Olivine and the `goto` in Cianwood's Fly handoff
(`CianwoodCity_hns:46`). Remove Elm's takeover call. Update the tests that pin
the arming: `game/tools/league_circuit/test_scripts.py:67-87` and
`game/tools/traversal/tests/test_hns_open_world_traversal.py:1026-1049`.

**During the occupation.** It persists through travel and save/reload until
resolved. The 1F radio quiz is unavailable while it runs and can never regress
it. Whitney's progress and the occupation's are separate. The internal order
stays the same: Petrel's Basement Key, the Underground Kimono Girl, the
Switches room, the Director's Card Key, Archer, and the Wing.

**The Wing.** When Archer is beaten and `VAR_LUGIA_OR_HOOH` is 0, record the
Wing as owed. The office Director (`LOCALID_DIRECTOR_OFFICE`, whose script
routes to the Director at Goldenrod state ≥ 10) holds it. Until a branch is
chosen, his line says the Wing will go to the trainer who hears out the man at
the Route 39 farm, Baoba. Once Baoba's choice sets the branch, the Director
gives the selected Wing on the next conversation, under the handoff rule. The
Wing is given exactly once.

**Resolution.** Finishing the takeover unlocks the Mahogany merchant's dialogue
only, not the road. It does not resolve the Well or the hideout.

### Rocket dialogue

| Speaker | Line today | Variant |
| --- | --- | --- |
| Proton, Radio Tower 4F | "Aren't you the one who got in our way at Slowpoke Well?" | Without the Well: a first-meeting line. |
| Hideout grunt (`GruntM28Seen`) | "I remember you from the well!" | Without the Well: drop the recognition. |
| Ariana, hideout retreat | "The broadcast experiment was a total success… We have much bigger plans." | If the Radio Tower is resolved: "The broadcast experiment was a total success. GOLDENROD was a setback, nothing more." |
| Ariana, Radio Tower 5F | "Remember me from the hideout in Mahogany Town?" | Without the hideout: a first-meeting line. |
| Radio Tower 3F grunt, after battle | "You made it past our men in the Underground?" | Only once the Underground step is done; otherwise a neutral after-battle line. |
| Archer, after battle | "Like Giovanni did before me, I will disband Team Rocket here today." | If the Well and the hideout are both resolved, keep it. Otherwise: "This operation is over. Withdraw, all of you!" followed by "Farewell." |

Backstory lines such as "Team Rocket was broken up three years ago" stay. The
Radio Tower's broadcast motive ("announce our comeback… bring our boss Giovanni
back") stays, since it holds in every order.

### Clair, Dragon's Den, and Elm's Master Ball

Remove the Blackthorn gym boy's badge 5–7 check (`BlackthornCity_hns:8-16,80-101`)
and any Wayfarer-only Clair badge branch in `BlackthornCity_Gym_hns`. Clair,
the Den, and the deferred Rising Badge keep their sequence and recovery. Update
the circuit specification's line on Clair's Chuck, Jasmine, and Pryce badges
and the traversal test that pins the gym boy
(`test_hns_open_world_traversal.py:1178-1206`).

After the Den, Blackthorn advances from its post-Den state (today 4) to done
(today 5) for every origin:

- **Johto-origin:** Elm's call plays, as today, and Elm gives the Master Ball
  at the lab.
- **Visitors:** there is no call, and Blackthorn advances when they arrive.
  The call is a frame script, so it must leave the post-Den state or it would
  hold the player. While Elm's Master Ball is owed and not received, the
  Dragon's Den Elder's closing line tells the player that Professor Elm in New
  Bark asked to meet whoever earned the Rising Badge. At the lab, Elm
  introduces himself in one short line and gives the Master Ball.

The Master Ball has its own owed and received state, set when the Den trial
completes. Elm checks it before any other lab branch, so it works:

- before the errand's egg return;
- before the Violet aide has been met (today `FLAG_HIDE_NEWBARKTOWN_LAB_AIDE`
  is checked first, `NewBarkTown_Lab_hns:97`);
- after the ceremony.

Clair's trial no longer writes lab state, so it cannot overwrite the errand.
Elm gives exactly one Master Ball under the handoff rule, with no TR gate. His
handoff no longer writes theater state, `FLAG_HIDE_ECRUTEAK_SILVER`, the
theater NPC flag, or the Kimono flag. His "the Kimono Girls are waiting" line
plays only while the trial is available and unfinished. The ceremony stops
writing lab state. Silph's Master Ball is unaffected.

### Kimono ceremony

**Introduction.** Zuki at the theater introduces the ceremony. Outside the
Theater Rocket scene, her ordinary conversation explains that the Kimono Girls
test trainers worthy of the legendary Pokémon. She names what is still
missing:

- the Wing, held by the Radio Tower's Director;
- more experience as a trainer (TR below 55).

With no branch chosen yet, she speaks of the legendary Pokémon without naming
one, and points the player to Baoba at the Route 39 farm.

**Trial.** With the selected branch's Wing in the Bag, the Radio Tower
resolved, and TR ≥ 55, Zuki offers the trial. Accepting runs the existing
`LegendaryCutscene` and five battles. The Kimono Girls appear only for the
trial and afterwards. Remove the theater state-5 entry triggers and Elm's
activation. The Bell, the sage-office Clear Bell access, the ceremony, and the
level 50 encounter follow as today. The encounter maps keep dispatching on
legendary progress, which only the ceremony and Pewter's gift can now set.
The ceremony sets only the selected legendary's progress (forward only) and
never the other's, so Pewter's gift can tell an unresolved encounter from a
resolved one. The ceremony variable runs 1 (trial started), 2 (ceremony done),
3 (encounter resolved); the offer itself is derived, not stored.

**Callbacks.** Each Kimono Girl line plays only when its event happened:

| Speaker | Callback | Condition |
| --- | --- | --- |
| Zuki | Met in Violet; the Mystery Egg passed from Mr. Pokémon to Elm to the player | Johto-origin and the egg was delivered; otherwise a first-meeting greeting |
| Naoko | Ilex Forest | The Ilex Farfetch'd scene played |
| Naoko | Slowpoke Well | The Well resolved |
| Miki | Chased off the theater's Rocket | The Theater Rocket beaten |
| Sayo | Pushed her off the Ice Path ice | The Ice Path push happened |
| Sayo | Passed the Elder's test in the Dragon's Den | The Den trial completed |
| Kuni | The Goldenrod Tunnel and thwarting the Radio Tower | Both are always true here, because the trial requires the Radio Tower |

A line with no true callback falls back to the speaker's challenge without the
memory.

### Pewter's other Wing

Pewter's old man gives the other branch's Wing only when all hold:

- a branch is chosen;
- the main legendary encounter is resolved, meaning its progress is 3, which
  both chambers set on a win, a catch, or the Pokémon fleeing
  (`WhirlIslands_LugiaChamber_hns:184-198`, `TinTower_RoofDay_hns:184-195`); a
  run or Teleport writes nothing and leaves it unresolved;
- TR ≥ 55;
- he has not given it before.

Before a branch is chosen he has ordinary dialogue, and the branch-0 path no
longer ends with the lock held. Once a branch is chosen, he explains whichever
requirement is unmet. Record his gift in its own flag; a tossed Wing is not
given again. His gift sets the other legendary's progress to its level 70
available state (4) only while that legendary's own encounter is unresolved.
It keeps the `FLAG_HIDE_TINTOWER_GUARD` or `FLAG_HIDE_WHIRL_ISLANDS_TENTACRUEL`
write (`PewterCity_hns:24-41`).

### Suicune and Eusine

Suicune's chase starts on the first Olivine arrival however the player gets
there. The sightings keep their order and do not require the Burned Tower.
Eusine's Cianwood greeting (`CianwoodCity_hns:271`) becomes a first meeting
when the player has not seen his Burned Tower introduction.

### Related documents updated with this work

When implementing, update:

- the league circuit specification's Johto story lines: the takeover's arming,
  Clair's badges, and the Victory Road corridor rule;
- the HNS traversal specification's merchant invariant;
- the start-choice specification's visitor Elm flow, marking it superseded:
  step 4's dormant Johto pre-Elm state (lines 222–223), the visitor Elm
  section (lines 317–372), and Validation lines 461 and 465;
- the Mt. Moon specification's always-visible Silver;
- `notable-world-simulation.md:297-301`, which assumes Silver blocks the Burned
  Tower for visitors;
- this specification's `Implemented` marker.

## Validation

Mechanical checks extend the existing script and journey tests. Cover:

- **Rocket orders:** all six relative orders of the Well, the Lake and hideout,
  and the Radio Tower. Each truthful variant appears, and no adventure is
  completed, rewound, or blocked by another.
- **Interleavings:** the Well before and after Bugsy, Azalea Silver, Ilex, and
  the GS Ball; the Lake before and after Pryce and the Radio Tower. The
  merchant's block never returns.
- **Town setup:** enter Violet from Route 32 or 36, Azalea from Ilex Forest,
  Goldenrod from Route 35 and by Magnet Train, Ecruteak from Route 38, Olivine
  by ship, Mahogany from Route 43 or 44, and Blackthorn from Route 45. Then walk
  the old trigger roads. Setup ran once, and nothing resets.
- **Radio Tower:** start with zero badges. Use the radio quiz before starting,
  decline the scout, then accept. Save/reload and travel mid-occupation. Finish
  it before Baoba's choice, then claim the owed Wing exactly once.
- **Road and Clair:** walk Mahogany → Route 44 → Ice Path → Blackthorn with no
  Radio Tower progress. Beat Clair and the Den with zero other badges. Elm's
  Master Ball arrives with TR 0, once, including after a full Ball pocket. As
  Johto-origin, claim it before returning the Mystery Egg, before meeting the
  Violet aide, and after the ceremony; the errand still completes.
- **Visitors:** as both Kanto- and Hoenn-origin visitors, Elm and Mr. Pokémon
  offer nothing, no Elm call plays in Violet, Goldenrod, or Blackthorn, the
  Violet aide never appears, and no Silver object ever shows, including in New
  Bark. The Burned Tower release, Morty's Gym, the Underground step, and the
  ceremony all complete. Jessica trades. The starter preview shows no Johto
  starter.
- **Miracle Seed:** take it before and after Falkner; it is given once, and
  Elm's Violet call does not repeat.
- **Silver:** as Johto-origin, start a later battle first and confirm earlier
  scenes retire with no rewards. Walk into Olivine before Azalea and confirm
  both Azalea and Burned Tower battles remain. Lose a battle and retry it.
  Decline and lose Victory Road and confirm the corridor stays passable.
  Meet him there before and after competing at Indigo and check the opener.
  Visit Mt. Moon before and after Victory Road. Check every dialogue variant.
- **Theater:** beat the Rocket before and after the ceremony is available.
  Take Surf from the giver's conversation and from his trigger; it is given
  once.
- **Ceremony:** Zuki with neither requirement, with only the Wing, with only
  TR 55, and with both. Only both offers the trial. Cover both branches, the
  Kimono callbacks with and without their events, and visitors.
- **Pewter:** before the branch choice, before the trial, after the trial but
  before resolving the encounter, after each completing outcome, and below TR
  55. Toss the Wing and confirm no second gift.
- **Lighthouse:** medicine first and Jasmine first.
- **Lake:** the Route 43 toll stops once the Lake and hideout are resolved.
- **Suicune:** first Olivine arrival by ship and by Route 39. Meet Eusine in
  Cianwood before and after the Burned Tower.
- **Persistence:** save/reload and regional travel at each multi-step boundary.
  Full-Bag retries for every one-time handoff listed above.

## References

- [Johto story beat gates](../research/johto-story-beat-gates.md)
- [Johto legendary ceremony gates](../research/johto-legendary-ceremony-gates.md)
- [Silph President Master Ball](silph-president-master-ball.md)
- [Power Plant old generating hall](frlg-kanto-power-plant.md)
- [Wayfarer regional start choice](wayfarer-regional-start-choice.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)

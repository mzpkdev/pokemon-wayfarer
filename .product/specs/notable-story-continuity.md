# Notable trainer story continuity

PRD: [Notable trainers](../prds/notable-trainers.md)
Implemented: No

## Scope

Wayfarer plays each region's story beats in any order, from several player
origins. The notable trainers also have a life outside the story: walkers,
haunts, Gym greetings and friendship. Most story scenes were written for one
fixed playthrough. They do not know what the player has already done with the
same trainer, so trainers misbehave in three ways:

1. **Introductions.** A trainer introduces themselves to a player they have
   already met, or never introduces themselves at a first meeting.
2. **History claims.** A line recalls a meeting, battle, badge or family tie
   that the player may never have had.
3. **Double presence.** The same trainer stands in two places at once: a
   story copy and a Gym copy, or a story copy and a walker.

This spec sets one rule for each problem and lists the scenes that break
each rule today. It also defines the **story beats** that move friendship
([PRD](../prds/notable-trainers.md#friendship)), so that seeing a story
through with a trainer counts the way a quest does. Lance at the Lake of Rage is the example case. His first
Johto scene never uses the self-introduction HNS wrote for it
(`LakeOfRage_Text_LanceIntro1`). It greets every player the same, including
one who beat him at Indigo. If the lake is skipped, the Indigo Champion
intro ("I knew that you, with your skills, would eventually reach me here")
assumes a meeting that never happened.

The scene lists come from a read-only audit of the 38 notable trainers'
scripts. It was not played in game. Rows marked *inferred* follow the code
path without being run end to end.

Out of scope:

- Retiring the dead HNS cameos (Pryce at the Lake, Morty at Bellchime, the
  Dojo pointers). See [notable haunts](notable-haunts.md).
- The Celebi flashbacks of Giovanni. See
  [FRLG/HNS Kanto story conflicts](../research/frlg-hns-kanto-story-conflicts.md).
- Story TMs that repeat notable lesson rewards.
- The HNS post-badge cameos that point to the Saffron Dojo: Janine at the
  Reception Gate, Erika at the Celadon fountain, Misty at Cerulean Cape,
  Chuck's Dojo pointer, and the Den Clair. Each one stands while the leader's
  Gym copy and walker also stand. [Notable haunts](notable-haunts.md#cameos-and-the-dojo)
  retires them with the Dojo seats.

## Behavior

### 1. Acquaintance

Each notable trainer gets one saved **introduced** bit. The bit means "this
trainer and the player know each other's faces". It is not friendship. It
adds no points, it does not change the stage, and it never gives a phone
number. The [friendship](notable-trainers.md#friendship) rule still holds:
talking never makes a trainer a friend.

The bits live in `struct NotableFriendshipState`, one bit per character
id: 38 bits, 5 bytes. Prerelease saves need no migration. A bit is never
cleared. Four things set it:

| Event | When |
| --- | --- |
| A battle with the trainer | At the end of any battle against them, won or lost: Gym, story, rematch, league or Masters. |
| A notable greeting | Whenever `NotableTalk_EventScript_Greeting` plays a line: walker talk, haunt talk, or a Gym greeting. |
| A story introduction | A scene runs the [introduction](#2-introductions) for this trainer. |
| A story beat | A [story beat](#5-story-beats) with them completes. |

The greeting rule changes in one place. `NotableTalk_GetGreeting` still reads
the stage, except that an introduced Stranger gets **AGAIN** instead of
MEET or HEARD:

| Stage | Introduced | Greeting |
| --- | --- | --- |
| Stranger | No | MEET, or HEARD if famous (unchanged) |
| Stranger | Yes | AGAIN |
| Met | Either | AGAIN (unchanged) |
| Friend | Either | HELLO (unchanged) |
| Close | Either | CLOSE (unchanged) |

### 2. Introductions

**Rule:** a trainer introduces themselves exactly once. Every story scene
where a notable trainer could be meeting the player for the first time
branches on the introduced bit.

- **Not introduced:** the scene plays its own first-meeting text, so the
  scene keeps its story voice. It then sets the bit.
- **Introduced:** the scene plays the trainer's notable greeting in place of
  the self-introduction, then the rest of the scene's text.

Scene text that mixes a self-introduction with story content is split in
two. The self-naming sentence becomes the first-meeting part, and the rest
is shared by both branches. Whitney's Radio Tower line splits into "Hi! My
name is WHITNEY!" and "I heard about the QUIZ…".

Two specials do the work: `NotableStory_IsIntroduced` and
`NotableStory_MarkIntroduced`. Both take the character id in `VAR_0x8004`. A
shared script, `NotableStory_EventScript_Introduce`, wraps the branch, and
the AGAIN-or-warmer greeting comes from `NotableTalk_EventScript_Greeting`.
Gym pre-battle intros follow the same rule. Today only the post-badge
branches call the greeting.

Scenes to convert:

| Trainer | Scene | Problem today |
| --- | --- | --- |
| Lance | Lake of Rage, `LakeOfRage_EventScript_Lance` | `LakeOfRage_Text_LanceIntro1` is unused, so he never introduces himself. Restore it as the first-meeting text. |
| Lance | Indigo Champion, `PokemonLeague_ChampionsRoom_Text_BattleIntro` | Assumes the Lake. A player who never met him needs a first-meeting variant. |
| Whitney | Radio Tower 1F, `RadioTower1F_Text_Whitney1`, and Goldenrod Gym ("Hi! I'm WHITNEY!") | Both are first-meeting intros, so every player hears two. |
| Wallace | Cave of Origin B1F ("My name is WALLACE.") | Can follow an Indigo league event against him, or a walker talk. Inferred: that the league draw picks him. |
| Steven | Mossdeep Space Center 1F and 2F | Can be the first meeting, but he never gives his name. |
| Steven | Granite Cave, Steven's room | Re-introduces him after the Space Center or a Kanto meeting. |
| Blaine | Cinnabar Gym pre-battle ("I am BLAINE, the red-hot LEADER") | Ignores an earlier walker talk. |
| Blue | Indigo Champion: the circuit fallback "I'm BLUE, the INDIGO CHAMPION" and `league_circuit.inc` "I'm {STR_VAR_1}." | A fresh introduction, even to his own rival. See also [history claims](#3-history-claims). |
| Blue | Pokémon Tower 2F | Never names himself. The Tower and the S.S. Anne keep separate met flags (`FLAG_WAYFARER_TOWER_BLUE_MET`, `FLAG_WAYFARER_SS_ANNE_BLUE_MET`), so the introduced bit replaces both for introduction purposes. |
| Elite Four and Masters | `wayfarer_indigo_league.inc` ("I am LORELEI of the ELITE FOUR", "I am BRUNO…") and `wayfarer_masters_league.inc` ("I am WILL, an invited MASTER", "I am BRUNO…", "I am KAREN…") | Always a cold introduction. Bruno introduces himself in both leagues. |
| League rooms | `league_circuit.inc` generic "I'm {STR_VAR_1}." intros | Ignore acquaintance for every host. |

### 3. History claims

**Rule:** a line may claim a shared past only when a saved flag proves that
past. Proxies such as an item in the bag, a global game clear, or the
player's region are not proof. When the proof is missing, the scene uses a
neutral variant. That variant still follows the [introduction](#2-introductions)
rule.

| Trainer | Scene | Claim | Proof | Without the proof |
| --- | --- | --- | --- | --- |
| Steven | Route 118, `Route118_Text_StevenQuestions` | "We met in the cave near DEWFORD." | `FLAG_DELIVERED_STEVEN_LETTER` | The same scene without the cave line. Route 120's before/after-Dewford variants are the model. |
| Steven | Meteor Falls, Steven's cave | "We battled alongside each other at the … SPACE CENTER", "our paths first crossed in GRANITE CAVE" | Space Center: `FLAG_DEFEATED_MAGMA_SPACE_CENTER`. Granite Cave: `FLAG_DELIVERED_STEVEN_LETTER`. | Drop each unproven line. The cave also opens on *Hoenn's* clear, not `FLAG_SYS_GAME_CLEAR`, which clearing Johto or Kanto sets (`wayfarer_persistence.c`). |
| Steven | Vermilion City | "The Champion from the HOENN region." | None. Wallace is Hoenn's Champion. | Past tense, as Fuchsia already has it: "for some time, I was the champion." |
| Blue | S.S. Anne 2F, `SSAnne_2F_Corridor_Text_VisitorIntro` | "I didn't expect another visiting TRAINER here." | The player is not Pallet-origin. | For a Pallet-origin player, a rival line. |
| Blue | Four Island rival scene | A childhood rival ("quit copying me") | Pallet origin | No scene for other origins. Today it is gated on Sevii progress only. |
| Blue | Indigo Champion | Rivalry | Pallet origin | The introduction rule. |
| Misty | Route 25, `Route25_Text_MistyDate` | "Are they JOHTO GYM BADGES? If you have eight…" | Eight Johto badges | A line about the player's actual badges, or none. |
| Chuck's wife | Cianwood City, `CianwoodCity_Text_ChucksWifeChubby` | "My husband lost to you" | Chuck's badge. Today the check is holding HM Fly, which Hoenn's Route 119 also gives. | Her plain HM Fly gift. Inferred: that a Hoenn player reaches her with Fly before beating Chuck. |
| Norman | Route 104 to 105 Pokénav call (`Route104_Text_DadPokenavCall`, "Registered DAD NORMAN") and his match calls | "DAD" | `WayfarerUsesNativeHoennHousehold`, already used by Petalburg Gym | "NORMAN" and non-family text. The visitor Gym intro's "how much you've grown" also goes. |
| Lance | Indigo Champion | "I knew that you … would eventually reach me here" | Lance introduced | The introduction rule. |
| Clair | Blackthorn Gym and Den League directions ("go to NEW BARK TOWN. Then SURF east") | A Johto route to the League | None | Neutral directions. Inferred: that the circuit makes the route wrong. |
| Route trainers | e.g. Route 32 Bird Keeper Peter ("You beat FALKNER?") | A badge | That badge's flag | A line without the claim. |

Scenes that only ever play after their proof stay as they are. For example,
Lance's Mahogany shop and hideout lines can only follow the Lake.

### 4. One place at a time

**Rule:** a notable trainer is visible in at most one place. A temporary
story placement wins over the Gym copy, and both win over the walker.

- **Story-away table.** A new data table lists each notable trainer's
  *temporary* story placements: the character, the map and local id, and
  the flag or state that shows the copy.
  - While any listed placement is showing, the walker does not spawn
    anywhere. Its simulation record keeps moving unseen.
  - This replaces the same-map-only check in `IsStorySuppressed`
    (`wayfarer_walkers.c`) for listed trainers. It also answers the
    [world simulation](notable-world-simulation.md#open-questions) question
    about which story objects suppress an actor.
- **Gym copy.** While a leader's listed placement is showing, their Gym
  object is hidden. The Gym's guide line says where they went, so a line
  like the Goldenrod Beauty's becomes true.
- **Permanent duplicates.** A story copy that is always visible is not a
  temporary placement. Retire it, or turn it into the walker's home spot.
  It must not be added to the table.
- **Cutscene actors.** A copy that appears and leaves inside one scene,
  such as Clair at the Den Shrine, needs no entry.

Known cases:

| Trainer | Copies | Change |
| --- | --- | --- |
| Whitney | The Radio Tower 1F copy, shown from New Game until the quiz is solved (`FLAG_ENABLE_RADIO`), plus the always-visible Gym copy | The Tower copy becomes a morning outing (`gettimeofday` is `TIME_MORNING`) until the quiz is solved. During it, the Gym copy hides and the Goldenrod Beauty says Whitney may be at the Radio Tower. At other times the Tower copy hides, Whitney is in her Gym, and the Beauty gives a plain Gym line. Once the quiz is solved she is always in the Gym. |
| Steven, Wallace | Walkers simulated from New Game, while the story places them elsewhere (Wallace inside the Cave of Origin during the Sootopolis crisis; Steven at the Space Center) | Add their temporary story placements to the table. Inferred: not play-tested. |
| Giovanni | His walker keeps Viridian and the Game Corner after `FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS` removes him | After the departure, his walker stops spawning. Inferred: the sim reads no story flags today. |

### 5. Story beats

**Rule:** a story beat is a story event where the player and a notable
trainer actually do something together: the player helps them, fights beside
them, or sees their crisis through. Completing a beat adds the
[friendship](notable-trainers.md#friendship) "Story beat completed" points
(+10, placeholder) and sets the introduced bit.

- **Listed beats only.** Only the beats in the table below count. Points come
  only from explicit, bounded events, as with quests.
- **Once each.** Each beat has a saved bit, so it pays once even if its
  scene can run again. For example, Lance asks again when you decline.
- **On completion.** A beat pays when it completes, never on an attempt, a
  decline or a lost battle. A partner battle inside a beat adds nothing on
  its own; the beat pays when it ends.
- **No hellos, no enemies.** A scene where the trainer only greets the
  player, or one where they are the player's enemy (Giovanni), is not a
  beat. A battle against them already counts as a first win.
- **Never Close from the story.** The beats listed for one trainer add up to
  less than the Close threshold. A test checks this against the placeholder
  weights.
- **The number handover** follows the existing rule: crossing Friend owes
  the "Registered X's number" line, which plays once the scene hands back
  control.

Beats in v0. The implementation confirms each completion point in the
scripts:

| Trainer | Beat | Completes when |
| --- | --- | --- |
| Lance | The Lake of Rage request | The player accepts his request (`LakeOfRage_EventScript_LanceAccept`). |
| Lance | Shutting down the Mahogany hideout | He hands over HM Whirlpool at the hideout finale (`RocketHideout_B2F_hns`). |
| Steven | Granite Cave letter | `FLAG_DELIVERED_STEVEN_LETTER` is set. |
| Steven | Space Center tag battle | `FLAG_DEFEATED_MAGMA_SPACE_CENTER` is set. |
| Tate & Liza | Space Center | `FLAG_DEFEATED_MAGMA_SPACE_CENTER` is set. They keep a score but give no number. |
| Wallace | The Sootopolis crisis | The weather crisis ends after the Cave of Origin and Sky Pillar scenes. |
| Jasmine | Olivine Lighthouse medicine | The SecretPotion is delivered to Amphy. |
| Clair | The Dragon's Den test | She grants the Rising Badge after the Den. |
| Wattson | New Mauville generator | He thanks the player for shutting down the generator. |
| Norman | Wally's first catch | Wally catches his Ralts on Route 102 with the player. |
| Lorelei | Icefall Cave poachers | The Icefall Cave poaching scene ends with the poachers stopped. |

### Guard

A script test scans every `.inc` string for a self-introduction by a
notable trainer's name, such as "I'm X", "I am X" or "My name is X". It
fails unless the line sits in the first-meeting branch of
`NotableStory_EventScript_Introduce` or is on an allow-list. New scenes then
cannot reintroduce the bug.

Each converted scene needs script or e2e coverage in two orders: met first
and not met first.

## Open questions

- **Steven's two contacts.** Granite Cave registers Steven in the Pokénav,
  and the notable phone gives him a number at Friend. Confirm whether he
  shows twice. Inferred.

## References

- [Notable trainers](notable-trainers.md#friendship), friendship and greeting stages
- [Notable world simulation](notable-world-simulation.md), walkers and story suppression
- [Johto independent story beats](johto-independent-story-beats.md), [Hoenn independent story beats](hoenn-independent-story-beats.md), [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)
- [Regional story beat independence](../research/regional-story-beat-independence.md)

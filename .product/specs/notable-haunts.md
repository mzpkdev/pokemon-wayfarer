# Notable haunts

PRD: [Notable haunts](../prds/notable-haunts.md)
Implemented: No
Design status: v0 draft for review: one shared pool of **haunts**, each
tagged by region, theme types, elite gate, hometown, activity, setting, and
one **quest**, and naming no trainer. **Placement** is a pure function of
world progress and a few saved facts, with no seeds: hard qualifiers
(region or traveller, aloof only at remote elite haunts, the quest's
manners) pick candidates, a placeholder score (theme from aces, activity from
play style and **momentum**, hometown, setting) ranks them, and haunts fill
one at a time in an order that rotates with world progress. The player meets
a placed trainer as a **stranger**, **famous**, or a **friend**; friends
give gossip, a rematch, and the quest, whose reward is claimed once per
placement. Dialogue splices manner-neutral haunt lines with each trainer's
**voice bits**. Weights, gates, momentum values, and rewards are
placeholders; balance is informational.

## Scope

Own, in `IS_WAYFARER`: the haunt catalog and its tags, the meaning of the
**manner** and **buddy** trainer values, placement and momentum, the
relationship beat, the five quest types and their rewards and claims,
dialogue assembly, the Kanto haunt list, retiring the HNS cameos and the
Saffron Dojo rematch room, the haunts' saved state and load validation,
presentation, the balance report, and acceptance.

- [Notable trainers](notable-trainers.md) owns the trainers: inventory,
  home region, the traveller and aloof traits, TR and its growth, rosters,
  the downward rule's use, move pools, and the battle snapshot. It also holds
  the manner and buddy fields in the catalog; this spec owns what they mean.
- [Sevii Masters](sevii-masters.md) owns phone contacts (the contact bit and
  the first-win rule), the partner choice, and the runtime partner slot.
- [Leagues](leagues.md) owns invitations and the accepted event lineup.
- [Trainer AI](trainer-ai.md) owns the AI flags of every battle here.
- [Gym Leader scaling](gym-leader-scaling.md) owns battle construction for
  every notable battle, including the ones at haunts.
- The voice bits themselves are content in
  [notable trainer voice bits](../research/notable-trainer-voices.md).

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
| Activity | `training`, `study`, `leisure`, `worship`, `sightseeing` | What trainers do there. |
| Setting | `public`, `remote` | Public places are busy; remote ones are out of the way. |
| Quest | one [quest type](#quests) and its details | The quest a friend gives there. |
| Meeting spot | map and tile | Where the placed trainer stands. |

A non-elite haunt is always active. Elite gates are placeholders; every
Kanto elite haunt uses TR 80, the league qualification TR. An inactive haunt
takes part in nothing: no placement, no meeting, no quest. A haunt is not a
[location](notable-trainers.md#home-region-and-travel): it reads no
willingness, travel cost, or fatigue.

## Trainer values

Each notable trainer entry authors two values for haunts, alongside its
other catalog content
([Notable trainers](notable-trainers.md#manner-and-buddy)):

- **Manner:** `warm`, `proud`, or `cold`. Manner is a quest qualifier, not a
  tone: warm trainers ask the player for help, proud ones challenge or enlist
  them, and cold ones command them. It decides which
  [quest types](#quests) a trainer can give, and it sets the
  [setting preference](#score). The voice bits lean the same way, but
  nothing reads them for manner.
- **Buddy:** one roster slot number (1-6), the trainer's companion: their
  anime companion where there is one, otherwise a later-game companion or
  their iconic ace. `{BUDDY}` resolves to that slot's species after the
  [downward rule](player-trainer-rating.md#evolution-stages) at the slot's
  member level (`teamLevel(tr) + levelOffset`, clamped to 1-100) at the
  current world progress. It resolves the same way whether or not the slot
  has joined the trainer's team yet: the buddy travels with the trainer even
  when it doesn't battle.

The v0 values are in the
[voice bits](../research/notable-trainer-voices.md) (the `Manner:` and
`Buddy:` lines), to be copied into the catalog. Tate & Liza author both like
everyone else, though the duo is never placed in v0.

A trainer's **hometown** is not authored: it is the city of their badge
encounter ([coverage](gym-leader-scaling.md#coverage-and-identity)), so only
Gym Leaders have one. Kanto: Brock Pewter, Misty Cerulean, Lt. Surge
Vermilion, Erika Celadon, Janine Fuchsia, Sabrina Saffron, Blaine Cinnabar,
Giovanni Viridian.

## Placement

A **placement** maps each active haunt to at most one trainer, and each
trainer to at most one haunt. It is a pure function of:

- world progress (`GetTrainerRating()`);
- the accepted league event, if any, and its
  [event lineup](leagues.md#event-lineup);
- the resolved [partner](sevii-masters.md#partner), once the player is a
  [Master](sevii-masters.md#master): the partner choice, or Lorelei without
  one; and
- content (the haunt catalog and the trainer catalog).

Contacts, met bits, claims, and history play no part, and nothing is
random. Placement is recomputed whenever any input changes (world progress
rises, an event is accepted or ends, a partner is asked, the player becomes
a Master) and on load. While a [walk](#walk-with-me) is in progress, the
recompute waits until the walk ends, so the walking trainer keeps their
haunt.

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
4. **Manner.** The haunt's quest type allows the trainer's manner
   ([quests](#quests)).

### Score

Each candidate gets a whole-number **score** for the haunt, the sum of these
parts. The weights are placeholders to tune:

| Part | Points | Condition |
| --- | ---: | --- |
| Theme (signature) | 4 | A type of the signature POKéMON's authored species is one of the haunt's theme types. |
| Theme (other ace) | 2 | Otherwise, a type of another ace's authored species is a theme type. |
| Activity (play style) | 2 | The haunt's activity is one of the play style's activities (table below). |
| Activity (momentum) | 1 | The haunt's activity is one of the momentum's activities. |
| Hometown | 3 | The haunt's hometown is the trainer's hometown. |
| Setting | 1 | Warm and the haunt is public, or cold and the haunt is remote; proud never scores it. |

Types come from the authored (final-stage) species of the roster's ace
slots, so a trainer's themes never change with world progress: Brock's
aces are Steelix (Steel, Ground) and Aerodactyl (Rock, Flying).

| Play style | Activities |
| --- | --- |
| Gambler | leisure |
| Bomber | study |
| Sweeper | training |
| Field marshal | study, sightseeing |
| Hexer | worship |
| Turtle | leisure |
| Brawler | training |
| Tactician | study |

| Momentum | Activities |
| --- | --- |
| Rising | training |
| Settled | leisure, sightseeing |

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
([rewards and claims](#rewards-and-claims)); then the new placement is
saved.

## Relationship beat

Talking to the placed trainer runs the relationship beat, written once for
all haunts. The trainer's **standing** with the player is read when the talk
starts:

- **Friend:** their contact bit is set
  ([phone contacts](sevii-masters.md#phone-contacts)).
- **Famous:** no contact bit, and their home region is the haunt's region.
- **Stranger:** no contact bit, and their home region is another region
  (a traveller away from home).

Each placeable trainer has a saved **met bit**, set the first time a beat
runs with them. The beat, with the voice bits it uses:

| Standing | Beat |
| --- | --- |
| Stranger | `MEET` (met bit clear) or `HELLO` (set), then an offer to battle: `YES` and the battle, or `NO`. A win: `PRAISE`, the number (below), and `BYE`. |
| Famous | `MEET` or `HELLO` as above, then `NOT_YET` and `BYE`. No battle: the first fight belongs to the Gym, story, or league. |
| Friend | `HELLO`, then `NEWS` and the [gossip](#gossip) line, then the friend menu. |

A stranger's battle is optional, and winning it is a first win like any
other: it sets the contact bit under Sevii Masters' rule, and the trainer
says they'll give their number, so the next beat treats them as a friend.
A loss is an ordinary trainer loss, and the player blacks out as after any
trainer battle. Declining or losing can be retried on any later visit.

The **friend menu** offers, until the player picks **Bye** (`BYE`):

- **Battle:** a rematch, singles with the trainer's whole team at their
  current TR (every battle uses current TR:
  [Notable trainers](notable-trainers.md#trainer-rating)). It can be
  repeated.
- **Quest:** only while the haunt's claim bit is clear
  ([quests](#quests)).
- **Chat:** the trainer's `QUIRK`.
- **Team up:** only once the player is a Master. It asks the trainer to be
  the player's Masters partner in person, with exactly the effect of the
  phone ask ([partner](sevii-masters.md#partner)). The trainer then stops
  being placed, so they leave the haunt once the talk ends.

Haunt battles (a stranger's battle and a rematch) are built like any
notable battle, from a battle snapshot at the start, and pay prize money
([Prize money](#prize-money)). They give no TR and no Battle Points.

## Quests

Each haunt has one quest type. Only friends give quests, and the manner
table is a hard qualifier for placement:

| Quest | Warm | Proud | Cold | Reward |
| --- | --- | --- | --- | --- |
| Walk with me | ✅ | ✅ | ✅ | type item |
| Lost something | ✅ | ❌ | ❌ | type item |
| Catch me one | ✅ | ✅ | ❌ | type item |
| Quiz | ✅ | ✅ | ✅ | lesson |
| One on one | ✅ | ✅ | ✅ | lesson |

Every quest runs `ASK`, then the haunt's own quest line, then a yes or no:
`YES` starts it, `NO` ends the talk. A requirement the player doesn't meet,
or a failed attempt, uses `NOT_READY`. Completing it runs `PRAISE`, then the
[reward](#rewards-and-claims).

### Walk with me

The haunt authors a **start** (its meeting spot) and an **exit** (one warp
or map edge of its quest maps).

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
  the walk: the haunt's done line, `PRAISE`, and the reward. The follower
  leaves, and the trainer is back at their haunt.
- **Unfinished.** A whiteout, leaving the quest maps any other way, or a
  reload ends the walk with no reward and the claim bit still clear; the
  trainer is back at their haunt and the walk can be started again.

### Lost something

The haunt authors one hidden spot on its quest maps. While a friend is
placed there and the claim bit is clear, the spot can be searched: finding
it gives the player the trainer's lost thing, a transient **found** state,
not a Bag item. Talking to the trainer with it completes the quest; before
that, the quest line says what was lost and roughly where, and the trainer
answers `NOT_READY`. The found state clears on reload, on whiteout, and
when the placement changes, and the spot can be searched again while the
claim is open.

### Catch me one

The haunt authors one species from its own wild table. The player hands
over one POKéMON of that species (any form, not an Egg) from the party or
the boxes, on the storage screen's trade mode
([storage-screen modes](#storage-screen-modes)); the trainer keeps it, and it
never joins their roster. The player cannot hand over their last able party
POKéMON. With none to give, `NOT_READY`.

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

The player chooses one able party POKéMON, which fights the trainer's
**lead ace**: their signature POKéMON (roster slot 1) exactly as their
battle snapshot resolves it at current TR, alone. It is a singles battle
with no prize money and no blackout: the chosen POKéMON keeps its damage,
experience, and level-ups, and the party is restored around it, as a
[tag match's party restore](sevii-masters.md#party-and-heal) does. A win
completes the quest; a loss is `NOT_READY`, and it can be tried again.

### Rewards and claims

Each haunt has a saved **claim bit**. It is set when the reward is given
and cleared when the haunt's placement changes, so each placement pays out
once, and a new trainer at the haunt can give it again.

- **Type item:** the type-boosting held item for the primary type of the
  signature POKéMON's authored species (Brock's Steelix gives Metal Coat,
  Giovanni's Rhyperior gives Soft Sand), given with `GIFT`, whose `{ITEM}`
  names it. Type item names reach 14 characters (NEVER-MELT ICE), and
  every `GIFT` line still fits 70 characters with that. With no room in the
  Bag, the reward waits and the claim stays open.
- **Lesson:** the trainer teaches one move from their
  [move pool](notable-trainers.md#move-pools). The player picks a pool
  entry, then a POKéMON from the party or boxes that can learn it, on the
  storage screen's move tutor mode; entries no POKéMON can learn are shown
  but refused. The lesson uses `PRAISE` and no `GIFT`. Cancelling leaves
  the claim open.

## Dialogue

A haunt's dialogue is assembled from two sources:

- **Haunt lines**, authored per haunt: its quest line and, for a walk, its
  done line. They describe only the place and the activity, never a
  trainer's personality, manner, or history, so they read true for every
  candidate. A quest line is an instruction that follows `ASK` ("Walk the
  tunnel with me, out to the VERMILION side."), so the same line works
  after a warm plea, a proud dare, or a cold order.
- **Voice bits**, twelve per trainer: `HELLO`, `MEET`, `NOT_YET`, `NEWS`,
  `ASK`, `YES`, `NO`, `NOT_READY`, `PRAISE`, `GIFT`, `BYE`, and `QUIRK`
  ([voice bits](../research/notable-trainer-voices.md)). They carry the
  personality and never mention a place.

Both use these slots:

| Slot | Resolves to |
| --- | --- |
| `{PLAYER}` | The player's name. |
| `{ITEM}` | The reward item (only in `GIFT`). |
| `{ACE}` | The signature POKéMON's current species. |
| `{BUDDY}` | The buddy slot's current species ([trainer values](#trainer-values)). |
| `{GOSSIP}` | The gossip line (below). |

Speaker labels ("BROCK:") and system messages (the battle offer, the number
given, the menu, the quiz) are generic text, the same for every trainer.

### Gossip

`{GOSSIP}` is one system line naming where another trainer is: the next
placed trainer after the speaker in trainer catalog order, wrapping, whose
met bit is set, and their haunt's name ("I hear LT. SURGE hangs around the
VERMILION harbour."). With no such trainer, `NEWS` and the gossip are both
skipped. The template's wording is content.

## Worked example: Diglett's Cave

**Tags.** Region Kanto; theme Ground; not elite; no hometown; activity
sightseeing; setting remote; quest Walk with me. Maps
`DiglettsCave_EntranceNorth_hns`, `DiglettsCave_Tunnel_hns`, and
`DiglettsCave_EntranceSouth_hns`. The meeting spot is in the north
entrance, where Brock's cameo stands today; the exit is the south
entrance's warp to `MAP_VERMILION_CITY_HNS`. The tunnel's wild table holds
Diglett and Dugtrio (with Swinub and Wobbuffet).

**Candidates.** Every Kanto trainer who isn't aloof (Brock, Misty,
Lt. Surge, Erika, Janine, Blaine, Giovanni, Blue, Lorelei, Bruno) and every
non-aloof traveller from elsewhere (Bugsy, Will, Brawly, Drake). Sabrina,
Agatha, Lance, Glacia, Wallace, and Steven are aloof, and this haunt is not
elite. Walk with me takes every manner.

**Scores** at world progress 30 (three badges; placeholder weights):

| Trainer | Theme | Style | Momentum | Setting | Score |
| --- | ---: | ---: | ---: | ---: | ---: |
| Brock | 4 (Steelix, Ground) | 2 (Field marshal) | 0 (rising) | 0 (warm) | 6 |
| Giovanni | 4 (Rhyperior, Ground) | 0 (Brawler) | 1 (settled) | 1 (cold) | 6 |
| Misty | 0 | 2 (Field marshal) | 1 (settled) | 0 (warm) | 3 |
| Will | 0 | 2 (Field marshal) | 0 (rising) | 0 (proud) | 2 |
| Drake | 2 (Flygon, Ground) | 0 (Sweeper) | 0 (rising) | 0 (proud) | 2 |

Everyone else scores 0 or 1. Brock and Giovanni fit best, and Brock wins
their tie by catalog order. At world progress 30 the fill order starts at
the Celadon rooftop (`30 mod 18 = 12`, with the four elite haunts
inactive), so the cave is the third haunt to fill, and Brock is still free:
he takes it. At world progress 10 the cave fills while Giovanni is free but
Brock is already on the Celadon rooftop, so Giovanni takes it; at world
progress 40 both are placed before the cave's turn, and Will takes it.

**Brock, a friend** (world progress 30, TR 39: his buddy is ONIX):

```text
BROCK: Hey, {PLAYER}! Good to see you. Eating well, I hope?   HELLO
BROCK: Word travels fast between breeders. Listen to this...  NEWS
BROCK: I hear MISTY hangs around PALLET TOWN.                 gossip
BROCK: Could you lend a hand? A good breeder never turns      ASK
       down help.
BROCK: Walk the tunnel with me, out to the VERMILION side.    quest line
> Yes
BROCK: Great! I'll have a hot meal ready when you're done.    YES
  (wild DIGLETT pairs are tag battles beside BROCK)
BROCK: That's the VERMILION side. We made it through.         done line
BROCK: Nicely done! That's rock-hard willpower if I ever saw  PRAISE
       it.
BROCK: Take this METAL COAT. A good breeder always shares     GIFT
       supplies.
```

Back at the cave, **Chat** gives "ONIX gets fussy if I burn the rice. So do
my little siblings." (`QUIRK`), and **Bye** gives "Take care! And keep your
POKéMON well fed!"

**Giovanni, famous** (world progress 10, before his Viridian battle, first
meeting):

```text
GIOVANNI: I am GIOVANNI. Remember the name. Others have       MEET
          learned to.
GIOVANNI: Earn your way through my GYM first. Then I may      NOT_YET
          notice you.
GIOVANNI: Go. We will meet again.                             BYE
```

**Giovanni, a friend** (world progress 140, after the player beat him):

```text
GIOVANNI: So. {PLAYER}. You keep turning up.                  HELLO
GIOVANNI: TEAM ROCKET hears everything. Listen well.          NEWS
GIOVANNI: I hear BLUE hangs around ROUTE 1.                   gossip
GIOVANNI: I have a task for you. Consider it... an            ASK
          opportunity.
GIOVANNI: Walk the tunnel with me, out to the VERMILION side. quest line
> Yes
GIOVANNI: A wise decision.                                    YES
GIOVANNI: That's the VERMILION side. We made it through.      done line
GIOVANNI: Impressive. I rarely have cause to say that.        PRAISE
GIOVANNI: Take this SOFT SAND. Consider it a loan, not a      GIFT
          kindness.
```

His **Chat** is "Ah, PERSIAN. The only one who never disappoints me.": his
buddy is roster slot 4, which steps down to MEOWTH below level 28. At world
progress 10 (TR 24, team level 17) the same line would say MEOWTH; from
world progress 40 (team level 39) it says PERSIAN.

## Kanto haunts

The v0 Kanto list, in catalog order. It is a draft: the tags, quests, and
species are content for review. Every map exists under
`game/data/maps/`; Seafoam and Cinnabar use the FRLG port's maps, since
Wayfarer retired their HNS versions
([retirement](frlg-cinnabar-seafoam-hns-retirement.md)). Spots, exits, and
hidden-item tiles are chosen at implementation after checking collision and
existing objects.

| # | Haunt | Maps | Themes | Elite | Hometown | Activity | Setting | Quest |
| ---: | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Pallet Town | `PalletTown_hns` | Water | – | – | leisure | public | Catch me one: Krabby |
| 2 | Route 1 | `Route1_hns` | Normal, Flying | – | – | training | public | Walk with me: from the Pallet end to the Viridian City edge |
| 3 | Viridian City | `ViridianCity_hns` | Ground | – | Viridian | sightseeing | public | One on one |
| 4 | Viridian Forest | `ViridianForest_hns` | Bug, Grass | – | – | study | remote | Catch me one: Pikachu |
| 5 | Pewter Museum | `PewterCity_Museum_1F_hns` | Rock | – | Pewter | study | public | Quiz |
| 6 | Mt. Moon outside | `MtMoon_Outside_hns` | Rock, Fairy | – | – | sightseeing | remote | Lost something |
| 7 | Cerulean Cape | `Route25_hns` | Water | – | Cerulean | leisure | remote | Lost something |
| 8 | Rock Tunnel | `RockTunnel_B1F_hns`, `RockTunnel_1F_hns` | Rock, Fighting | – | – | training | remote | Walk with me: from the north Route 10 entrance to the south one |
| 9 | Power Plant | `Route10_PowerPlantEntrance_hns`, `PowerPlant_Frlg` | Electric | – | – | study | remote | Catch me one: Voltorb |
| 10 | Lavender Soul House | `LavenderTown_SoulHouse_hns` | Ghost | – | – | worship | public | Quiz |
| 11 | Vermilion harbour | `VermilionCity_PortOutside_hns` | Water, Electric | – | Vermilion | sightseeing | public | Catch me one: Chinchou |
| 12 | Celadon Game Corner | `CeladonCity_GameCorner_hns` | – | – | Celadon | leisure | public | One on one |
| 13 | Celadon rooftop | `CeladonCity_DepartmentStore_RoofDay_hns`, `CeladonCity_DepartmentStore_RoofNight_hns` | Grass | – | Celadon | sightseeing | public | Lost something |
| 14 | Saffron Fighting Dojo | `SaffronCity_FightingDojo_hns` | Fighting | – | Saffron | training | public | One on one |
| 15 | Dojo back room | `SaffronCity_FightingDojoVIP_hns` | Psychic | TR 80 | Saffron | study | remote | Quiz |
| 16 | Diglett's Cave | `DiglettsCave_EntranceNorth_hns`, `DiglettsCave_Tunnel_hns`, `DiglettsCave_EntranceSouth_hns` | Ground | – | – | sightseeing | remote | Walk with me: from the Route 2 entrance to the Vermilion exit |
| 17 | Safari Zone | `FuchsiaCity_SafariZoneEntrance_hns` and its Beach, Brush, Cave, and Mountain areas | Normal, Poison | – | Fuchsia | sightseeing | public | Catch me one: Kangaskhan |
| 18 | Seafoam Islands | `SeafoamIslands_1F_Frlg` | Ice, Water | – | – | training | remote | One on one |
| 19 | Cinnabar shore | `CinnabarIsland_Frlg` | Fire | – | Cinnabar | leisure | public | Quiz |
| 20 | Victory Road | `VictoryRoadKanto_1F_hns`, `VictoryRoadKanto_B1F_hns`, `VictoryRoadKanto_B2F_hns` | Rock, Fighting, Dragon | TR 80 | – | training | remote | Walk with me: from the Route 23 entrance to the Reception Gate exit |
| 21 | Cerulean Cave | `CeruleanCave_1F_hns`, `CeruleanCave_B1F_hns`, `CeruleanCave_B2F_hns` | Psychic | TR 80 | – | training | remote | Catch me one: Ditto |
| 22 | Indigo Plateau Pokémon Center | `IndigoPlateau_PokemonCenter_hns` | – | TR 80 | – | leisure | public | Quiz |

Notes:

- **Sabrina's spot.** Sabrina is aloof, so she can only be placed at a
  remote elite haunt. The Dojo back room (the Dojo's rematch room today) is
  Saffron's: its Psychic theme and Saffron hometown make her its best fit
  from TR 80, and it absorbs the rematch room.
- **Aloof trainers in Kanto** can only reach the Dojo back room, Victory
  Road, and Cerulean Cave. Cerulean Cave's Catch me one excludes the cold
  ones (Agatha, Sabrina, and Glacia), and the Indigo Plateau Pokémon Center
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
([Olivine Gym](../../game/data/maps/OlivineCity_Gym_hns/scripts.inc)). A
lesson uses `SELECT_PC_MON_MOVE_TUTOR`, which filters by whether a POKéMON
can learn the move.

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

Haunt battles pay prize money on the snapshot level basis a
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

- one **met bit** per placeable notable trainer (every entry but Tate &
  Liza);
- one **claim bit** per haunt;
- the **current placement**: one `characterId` or none per haunt. It is
  derivable from the inputs, and is saved to detect changes for the claim
  bits and to hold the walking trainer during a walk; and
- the **quest in progress**: a walk (its haunt) or a found lost thing (its
  haunt), cleared on load and on whiteout.

New Game saves every met bit and claim bit clear, the placement for world
progress 0, and no quest in progress. With follower NPCs enabled,
SaveBlock3 also holds the engine's follower state, which a walk uses.

## Load validation

On every load, before the overworld runs:

1. **Quest in progress.** Clear it; if a follower NPC is present, remove
   it. A walk interrupted by a reload is unfinished
   ([walk](#walk-with-me)).
2. **Pruning.** Drop the met bits of characters no longer in the registry
   and the claim bits and placements of haunts no longer in the catalog.
3. **Checks.** Met bits exist only for known placeable trainers; claim bits
   and placements only for known haunts; a saved placement names known
   characters, each at most once. A failed check is an invalid save, never
   a reason to reward anything.
4. **Recompute.** Compute the placement from the current inputs and compare
   it with the saved one, clearing the claim bit of every haunt whose
   trainer changed, then save it.

## Presentation

- The placed trainer stands at the haunt's meeting spot with their buddy
  beside them as a POKéMON object at its current species, as the Dojo's
  rematch room shows its leaders' POKéMON today. An empty haunt shows no
  one.
- Speaker labels name the trainer ("BROCK:"); Tate & Liza's lines keep
  their split format for when the duo is placed later.
- The friend menu lists Battle, the quest's name (while unclaimed), Chat,
  Team up (for a Master), and Bye.
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

1. **Catalog.** Every haunt has valid tags, existing maps, an active
   meeting spot, and its quest details (a walk's start and exit, a lost
   spot, a catch species on its wild table); every notable trainer has a
   manner and a buddy slot 1-6.
2. **Buddy.** `{BUDDY}` resolves to the slot's stepped-down species at every
   world progress, including a slot not yet on the team (Giovanni's slot 4
   is MEOWTH at world progress 10 and PERSIAN at 40; Brock's slot 1 is ONIX
   until world progress 57).
3. **Qualifiers.** No placement breaks a hard qualifier at any world
   progress 0-200: region or traveller, aloof only at remote elite haunts,
   the quest's manners, and no Tate & Liza, accepted-lineup trainer, or
   resolved partner.
4. **Placement.** The same inputs always give the same placement; each
   trainer holds at most one haunt; scores, momentum, the fill order, and
   ties match golden fixtures from host tooling; an inactive elite haunt is
   empty; accepting an event, asking a partner, and becoming a Master each
   recompute it. A walking trainer keeps their haunt until the walk ends.
5. **Standing.** A stranger's first beat uses `MEET` and later ones
   `HELLO`; a stranger battle win sets the contact bit and the next beat is
   a friend's; a famous trainer never offers a battle; a friend gets the
   menu, and Team up appears only for a Master and sets the partner choice.
6. **Quests.** Each quest completes, fails, and retries as specified; a
   walk ends unfinished on whiteout, on leaving another way, and on reload;
   Fly, Teleport, and an Escape Rope are refused during a walk; its wild
   battles are double battles beside the trainer with their best three from
   the runtime partner slot; Catch me one refuses the last able party
   POKéMON; the quiz asks the same questions on every attempt.
7. **Claims.** A reward is given once per placement; a changed placement
   reopens it; a full Bag or a cancelled lesson keeps it open.
8. **Dialogue.** Every assembled line resolves its slots and fits its text
   box with worst-case values; gossip names only met, placed trainers.
9. **Cameos.** No cameo or Dojo rematch seat remains in Wayfarer; the Dojo
   back room works as a haunt; no Battle Points come from haunts.
10. **Save.** A new game and a reload give the saved state described above;
    corrupt met bits, claim bits, or placements are rejected, and removed
    characters or haunts are pruned.

## Open questions

- Whether rematches need a limit, such as one per placement, so prize money
  can't be farmed.
- Whether a minimum score should leave a haunt empty rather than host a
  poor fit, since v0 has more Kanto haunts than Kanto candidates.
- When to retire the Johto cameos and their Dojo seats: with the Kanto
  list, which turns the rematch room into the Dojo back room and leaves
  Johto trainers who don't travel without rematches until Johto's list, or
  later, with the back room waiting for Johto's list too.
- Where Jasmine's Steelix trade goes once the Dojo seats retire.
- Whether a follower NPC turns regular trainer battles on the way into
  partner battles; the design assumes it doesn't.

## Later

- **Situations:** "The coach" (a trainer training the player's POKéMON),
  two trainers in one haunt, "Show me" (the player shows a POKéMON rather
  than handing it over), services, and challenge battles with conditions.
- **Trades at haunts**, under the trade fairness rules, and **item swaps**.
- **Access:** haunts gated by a key item, an HM, or a story beat, beyond
  the elite gate.
- **Gifts from the roster:** a friend giving the player a POKéMON of their
  own line.
- **Friendship scores** with each trainer, raised by quests and partnering.
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

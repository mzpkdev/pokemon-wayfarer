# World news

PRD: [World news](../prds/world-news.md)
Implemented: No
Design status: v0 draft for review: the **feed** is a pure function of world
state (league titles, reign records, the call counter, haunt placement,
momentum, badges, TR, lifetime wins) and adds no saved data. Each
**news item** is written with one of 15 neutral third-person **templates**,
tagged by topic, scope, and weight. Four **channels** filter the feed and
frame the item: notable friends (`NEWS` bit, whereabouts first), townsfolk
gossips (shared lead-ins, local scope), TV (anchor intro and outro,
headlines), and a new PokéNav radio station (DJ intro and outro, headlines).
Picks are deterministic from the speaker and `step = world progress + call
counter`; broadcasts walk the headlines with a cursor kept in RAM. Emerald's
TV system and PokéNews are retired: every API entry point keeps its name,
most as stubs, 964 bytes of SaveBlock1 are freed, and record mixing loses
its TV part.
Template wording, wrapper lines, and the gossip catalog are draft content.

## Scope

Own, in `IS_WAYFARER`: the feed and its items, the template pool and its
tags, fame gating, the four channels and their wrapper lines, rotation, the
townsfolk gossip catalog, the World news TV script and screen rule, the
World news radio station, retiring Emerald's TV system and PokéNews, the
dependency decisions, and acceptance.

- [Notable haunts](notable-haunts.md) owns the notable channel's rules: who
  gossips (Friend and above), the whereabouts pick
  ([gossip](notable-haunts.md#gossip)), the `NEWS` voice bit, placement, and
  the fame rule for a notable speaker
  ([relationship beat](notable-haunts.md#relationship-beat)). This spec
  owns the wording of the item that follows `NEWS`.
- [Leagues](leagues.md) and [Sevii Masters](sevii-masters.md) own titles,
  reign records, the call counter, and who is a Master; World news only
  reads them.
- [Notable trainers](notable-trainers.md) owns friendship, home regions,
  traits, and TR growth.
- [Emerald Lati roaming](emerald-lati-roaming.md) owns the Littleroot
  return-home scene and the Latias or Latios choice.
- [Global TR Poké Marts](global-tr-pokemarts.md) owns mart stock and prices.
- The later world events (swarms, market sales, roamer sightings, Gabby &
  Ty) get their own specs; this one only fixes what they reuse
  ([Later](#later)).

Standalone Emerald, FRLG, and HNS builds keep Emerald's TV system unchanged.

## The feed

The **feed** is the ordered list of every news item true right now. It is
built on demand, whenever a channel speaks or a map with a TV loads, from
these inputs only:

- per league, the [reigning champion](leagues.md#reigning-champion) (none,
  the player, or a `characterId`);
- the [invitation state](leagues.md#saved-state), the **call counter**, and
  each league's **last call number**
  ([which league calls](leagues.md#which-league-calls));
- each notable trainer's two [reign flags](leagues.md#reign-records), and the
  player's lifetime wins (`indigoCleared`, `hoennCleared`), which decide who
  is a [Master](sevii-masters.md#master);
- the current haunt [placement](notable-haunts.md#placement) and each placed
  trainer's [momentum](notable-haunts.md#momentum);
- the player's badges per region (`GetBadgeCountForRegion` in
  [wayfarer_persistence.h](../../game/include/wayfarer_persistence.h)) and
  TR (`GetTrainerRating()`); and
- content: the template pool, the league registry, the haunt catalog, and
  the trainer catalog.

Nothing is random, and the feed has no memory: the same inputs always give
the same feed.

### Items

Each item is one template with its slots filled. Items are generated in
this **feed order**, which every channel keeps:

1. **League items**, per league in league identity order (Indigo, Masters,
   Hoenn):
   - no reigning champion: `LEAGUE_VACANT`;
   - a notable trainer reigns and this league held the
     [latest event](#latest-event): `LEAGUE_LATEST`;
   - a notable trainer reigns otherwise: `LEAGUE_REIGN`;
   - the player reigns: `PLAYER_TITLE` (a player item).
2. **Masters:** `LEAGUE_MASTER` for each notable trainer with both reign
   flags, in trainer catalog order.
3. **Player items**, gated by [fame](#fame):
   - per region (Kanto, Johto, Hoenn, in that order) where the player holds
     one to seven badges, `PLAYER_BADGES`; where they hold eight,
     `PLAYER_ALL_BADGES`;
   - `PLAYER_KNOWN` once the player's TR is at least 80;
   - `PLAYER_MASTER` once the player is a Master.
4. **Rising trainers:** `TRAINER_RISING` for each notable trainer whose
   momentum is rising, in trainer catalog order. Momentum is computed for
   every trainer here, placed or not.
5. **Whereabouts:** one item per placed trainer, in haunt catalog order,
   using the first template that fits:
   1. `WHERE_ELITE` if the haunt is elite;
   2. `WHERE_HOMETOWN` if the haunt's hometown is the trainer's hometown;
   3. `WHERE_TRAVELLER` if the trainer's home region is not the haunt's
      region;
   4. `WHERE_TRAINING` if the haunt's activity is `training`;
   5. `WHERE_HAUNT` otherwise.

### Latest event

The league that held the **latest event** is found from the call counter,
with no new saved data:

- With no invitation waiting and no accepted event, it is the league with
  the highest last call number that has a reigning champion.
- While league X has an invitation waiting or an accepted event, X's last
  call number belongs to that pending event. Every other league that has
  called did so after X's previous call, since the lowest number calls next.
  So the latest event is the other league with the highest last call number
  that has a reigning champion, or X itself when no other league has one.
- A new game has none.

A declined event and a lost accepted event both leave a reigning champion,
so `LEAGUE_LATEST` covers both ("took the title").

### Fame

Player items reach a speaker only through **fame**:

- **Notable speaker:** the haunts' rule: the player's TR is at least
  `min(80, speaker's TR − 10)`, or the player reigns at any league, or is a
  Master ([relationship beat](notable-haunts.md#relationship-beat)).
- **Every other speaker** (townsfolk, TV, radio): its global part: the
  player's TR is at least 80, or the player reigns at any league, or is a
  Master.

Without fame, a channel skips every player item. Fame needs no saved data.

## Templates

One shared pool. **Writing rule:** a template states one fact in the third
person and never assumes who says it: no "I", "we", "here", "our town", or
"I hear". Pronouns for trainers are avoided too, so no template needs a
gender. Channel framing carries all voice.

Tags:

| Tag | Values | Meaning |
| --- | --- | --- |
| Topic | `league`, `player`, `trainer`, `whereabouts` | What the fact is about. |
| Scope | `global`, or local to a region set | Who counts it as local news. |
| Weight | `everyday`, `headline` | Headlines are what TV and radio air. |

A local item's **regions** come from its subject:

- a league: its location regions (Indigo: Kanto and Johto; Hoenn: Hoenn).
  The Masters is a neutral location, so its items are global;
- a haunt: its region tag;
- a trainer: their home region; and
- `PLAYER_BADGES` and `PLAYER_ALL_BADGES`: the region counted.

The v0 pool (draft wording):

| ID | Template | Topic | Scope | Weight |
| --- | --- | --- | --- | --- |
| `LEAGUE_VACANT` | No one holds the {LEAGUE} title yet. | league | league | headline |
| `LEAGUE_REIGN` | {NAME} holds the {LEAGUE} title. | league | league | headline |
| `LEAGUE_LATEST` | {NAME} took the {LEAGUE} title in the latest event. | league | league | headline |
| `LEAGUE_MASTER` | {NAME} is a Master, a champion at both INDIGO and HOENN. | league | global | headline |
| `PLAYER_TITLE` | {PLAYER} holds the {LEAGUE} title. | player | league | headline |
| `PLAYER_BADGES` | {PLAYER} holds {COUNT} of the eight {REGION} badges. | player | region | everyday |
| `PLAYER_ALL_BADGES` | {PLAYER} holds all eight {REGION} badges. | player | region | headline |
| `PLAYER_KNOWN` | {PLAYER} has qualified for the leagues. | player | global | headline |
| `PLAYER_MASTER` | {PLAYER} is a Master, a champion at both INDIGO and HOENN. | player | global | headline |
| `TRAINER_RISING` | {NAME} is on the rise, getting stronger by the day. | trainer | trainer | headline |
| `WHERE_ELITE` | {NAME} was seen at {PLACE}, where only top trainers go. | whereabouts | haunt | headline |
| `WHERE_HOMETOWN` | {NAME} is back home, at {PLACE}. | whereabouts | haunt | everyday |
| `WHERE_TRAVELLER` | {NAME} is visiting from {HOME}, at {PLACE}. | whereabouts | haunt | everyday |
| `WHERE_TRAINING` | {NAME} is training hard at {PLACE}. | whereabouts | haunt | everyday |
| `WHERE_HAUNT` | {NAME} hangs around {PLACE}. | whereabouts | haunt | everyday |

Slots:

| Slot | Resolves to |
| --- | --- |
| `{NAME}` | A notable trainer's name as their speaker label writes it (BROCK, LT. SURGE). |
| `{PLAYER}` | The player's name. |
| `{LEAGUE}` | The league's name in text: INDIGO LEAGUE, SEVII MASTERS, HOENN LEAGUE. |
| `{PLACE}` | The haunt's name as the haunt catalog writes it for text, with its article where English needs one (PALLET TOWN, the VERMILION harbour). |
| `{REGION}` | KANTO, JOHTO, or HOENN. |
| `{HOME}` | The trainer's home region, written like `{REGION}`. |
| `{COUNT}` | A number from 1 to 7. |

**Fit.** Every resolved template fits two lines of the standard text box
with worst-case slot values; on the radio it is wrapped to the radio's line
length ([Radio](#radio)). Tate & Liza's `{NAME}` is TATE & LIZA.

## Channels

A channel filters the feed to its **candidates**, keeping feed order, then
picks by [rotation](#rotation) and frames the pick. Every channel drops
player items without [fame](#fame).

| Channel | Speaker's region | Candidates | Frame |
| --- | --- | --- | --- |
| Notable friends | – | Whereabouts first, then every other item | `NEWS` bit |
| Townsfolk | The gossip's map | Local items for that region, any weight | Lead-in |
| TV | The TV's map | Headlines: global, or local for that region | Anchor |
| Radio | The player's map | Headlines: global, or local for that region | DJ |

The **region of a map** is `GetRegionForSectionId` of its map section
([regions.h](../../game/include/regions.h)); Sevii maps count as Kanto. A
region outside Kanto, Johto, and Hoenn has no local items.

### Notable friends

A notable trainer at Friend or above says `NEWS` and one item during their
haunt talk ([relationship beat](notable-haunts.md#relationship-beat)):

1. **Whereabouts first.** The haunts' gossip rule picks a trainer: the next
   placed trainer after the speaker in trainer catalog order, wrapping,
   whose stage is Met or above ([gossip](notable-haunts.md#gossip)). The
   item is that trainer's whereabouts item.
2. **Otherwise**, the candidates are every item that is not a whereabouts
   item and does not name the speaker, with player items only when the
   player's fame reaches this speaker. The pick is by
   [rotation](#rotation).
3. With no candidate, `NEWS` and the item are both skipped.

```text
BROCK: Word travels fast between breeders. Listen to this...  NEWS
BROCK: MISTY hangs around PALLET TOWN.                        WHERE_HAUNT
```

### Townsfolk

A **gossip** is an existing NPC listed in the **gossip catalog**: an ordered
list of `(map, local id)` entries. A gossip's **rank** is its position among
the catalog entries in the same map section, from 0.

- **Picking gossips (content).** Only NPCs whose authored line is flavour:
  never a story, gift, trade, quest, service, or trainer NPC, and never one
  whose line changes with flags. v0 lists at least one gossip in every
  Kanto, Johto, and Hoenn town and city, chosen at implementation.
- **Talk.** With at least one candidate, the gossip says its **lead-in**
  and then the item, in place of its authored line. With none, it says its
  authored line as today.
- **Lead-in.** Each gossip always uses the same lead-in, number
  `(map section + rank) mod 8` in this pool:

| # | Lead-in |
| ---: | --- |
| 0 | Have you heard? |
| 1 | Word around here is... |
| 2 | Here's something I heard the other day. |
| 3 | People keep talking about this. |
| 4 | You didn't hear it from me, but... |
| 5 | Did you catch the news? |
| 6 | My neighbour told me this one. |
| 7 | Everyone's been saying it... |

```text
Word around here is...
LT. SURGE is back home, at the VERMILION harbour.
```

### TV

Every `MB_TELEVISION` tile runs the World news script, `EventScript_TV`,
through the existing tile dispatch
([field_control_avatar.c](../../game/src/field_control_avatar.c), which
returns `EventScript_TV` for a player facing north at a TV tile when the
build is not FRLG). A background event on the same tile still wins, as
today, so authored TV scenes keep their scripts: the Kanto-origin TV in
`PalletTown_RedsHouse_1F_hns`, the Celadon Department Store's display TVs,
and the Cove Lily Motel's blocking TV. Red's house's other branch, "They
have programs that aren't shown in JOHTO…", goes to `EventScript_TV`
instead.

**Broadcast.** The script:

1. increments `GAME_STAT_WATCHED_TV`, as today, which Mauville's Old Man
   reads;
2. with no candidate, shows the **empty line** and turns the screen off;
3. otherwise shows the **intro**, then up to three headlines from the
   [broadcast cursor](#rotation), with the **link** before every headline
   after the first, then the **outro**, and turns the screen off.

| Line | Text (draft) |
| --- | --- |
| Intro | WORLD NEWS! Good to have you with us. Here are the headlines. |
| Link | In other news... |
| Outro | That's WORLD NEWS for now. Stay tuned! |
| Empty | There's nothing on right now. |

The lines use a sign-style box with no speaker label. `FLAG_SYS_TV_START`,
`FLAG_SYS_TV_WATCH`, the Mom or Dad lines, and the Littleroot movie line no
longer gate or replace the program in Wayfarer.

**Screens.** The on and off metatile swap stays (`TurnOnTVScreen`,
`TurnOffTVScreen`, and `UpdateTVScreensOnMap` in
[tv.c](../../game/src/tv.c)). On each map load, the TV tiles show the on
metatile when the map's TV has at least one candidate, and the off metatile
otherwise; after a broadcast they are off until the next map load. Two
exceptions keep today's behaviour: the Cove Lily Motel's TV is always on,
and the player's Littleroot house keeps its screens on while
`FLAG_SYS_TV_LATIAS_LATIOS` is set during the return-home scene.

### Radio

World news is a new station on the PokéNav radio
([pokenav_radio.c](../../game/src/pokenav_radio.c)):

- **Channel.** A new `RADIO_STATION_WORLD_NEWS` entry in `sRadioChannels`
  at tuning position 45, named WORLD NEWS, with the same music as Places and
  People (`MUS_HG_RADIO_OAK`, a placeholder).
- **Availability.** In every region, whenever the radio can be opened
  (`FLAG_ENABLE_RADIO`, which the PokéNav menu reads). It needs no
  expansion card, and the Team Rocket takeover does not replace it, like
  Places and People.
- **Content.** Built when the station is tuned: the DJ **intro**, up to
  three headlines from the shared [broadcast cursor](#rotation) with the
  **link** between them, and the **outro**; with no candidate, the **empty
  line**. Each headline is wrapped at word breaks into lines of at most 38
  characters, since the radio's line buffers hold 40 bytes
  (`RADIO_LINE_BUF_SIZE`) and the station has eight of them
  (`NUM_RADIO_LINE_BUFS`); three headlines need at most six.

| Line | Text (draft) |
| --- | --- |
| Intro | WORLD NEWS on the air! Here's the talk of the day. |
| Link | And there's more! |
| Outro | That's the news. Keep it tuned right here! |
| Empty | Quiet out there. Check back soon! |

Each DJ line fits one radio line.

## Rotation

Every pick uses the saved, never-decreasing **step**:

```text
step = world progress + call counter
```

World progress is `GetTrainerRating()`; the call counter is 0 until the
first league call. With `n` candidates:

| Channel | Pick |
| --- | --- |
| Notable friends (after whereabouts) | candidate `(characterId + step) mod n` |
| Townsfolk | candidate `(map section + rank + step) mod n` |
| TV and radio | candidates `(step + cursor + j) mod n`, for `j` from 0 to `min(3, n) − 1` |

- **Two gossips in one town** differ whenever there are at least as many
  candidates as gossips in that map section, since their ranks differ. Both
  move on when world progress rises or a league calls.
- **Broadcast cursor.** One transient counter in RAM, shared by TV and
  radio, not saved. It starts at 0 on load and on New Game, and each
  broadcast adds the number of headlines it aired, so repeated broadcasts
  walk the headline list in order and wrap.
- **Daily rotation** is [Later](#later): once the in-game clock exists,
  `step` can add the day count.

## Retiring Emerald TV

In Wayfarer, Emerald's TV system is replaced by World news: the shows,
PokéNews, their texts and data (the show and PokéNews parts of
[tv.c](../../game/src/tv.c), [data/text/tv.inc](../../game/data/text/tv.inc),
and [constants/tv.h](../../game/include/constants/tv.h)), and their save
slots. Prerelease saves don't carry over.

### Entry points

Every function in [tv.h](../../game/include/tv.h) and every TV special in
[specials.inc](../../game/data/specials.inc) keeps its name and signature,
so callers compile unchanged and no special ID is renumbered. Each is one
of:

- **Stub:** does nothing. One that returns a value returns the neutral one
  in the table.
- **Kept:** keeps its behaviour, because a system other than TV depends on
  it; it moves out of the show code.
- **Reworked:** keeps its name and serves World news.

| Entry point | Callers | Treatment |
| --- | --- | --- |
| `TryPutPokemonTodayOnAir` | battle_main.c:6024 | Stub. It was the only starter of PokéNews and outbreaks. |
| `TryPutBreakingNewsOnAir` | battle_main.c:6061 | Stub |
| `TryPutBattleSeminarOnAir`, `PutBattleUpdateOnTheAir` | battle_tv.c:1344, 753, 758 | Stub |
| `Put3CheersForPokeblocksOnTheAir` | berry_blender.c:3809, 3823 | Stub, returns FALSE |
| `IncrementDaily*` (wild battles, slots, roulette, blender, berries, Battle Points) | battle_setup.c:462-1385, slot_machine.c:1440, roulette.c:1485, berry_blender.c:3556, berry_tree.inc:32-450, scrcmd.c:1692, frontier_util.c:2048, 2052 | Stub |
| `AlertTVThatPlayerPlayed*`, `TryPutFindThatGamerOnAir` | slot_machine.c:1228, 1744, roulette.c:1261, 1984 | Stub |
| `RecordFishingAttemptForTV`, `SetPokemonAnglerSpecies` | fishing.c:410, 472, wild_encounter.c:1311 | Stub |
| `TryPutTodaysRivalTrainerOnAir` | overworld.c:2395, 2400 | Stub |
| `TryPutTrendWatcherOnAir` | dewford_trend.c:194, 203 | Stub |
| `TryPutTreasureInvestigatorsOnAir` | obtain_item.inc:259 | Stub |
| `TryPutSmartShopperOnAir` | shop.c:1068 | Stub |
| `TryPutSafariFanClubOnAir` | safari_zone.c:78 | Stub |
| `TryPutSpotTheCutiesOnAir` | contest_util.c:2000-2040, 2564, battle_tower.c:1838, post_battle_event_funcs.c:290, field_specials.c:1524, 6105-6273 | Stub |
| `TryPutSecretBaseVisitOnAir`, `TryPutSecretBaseSecretsOnAir` | decoration.c:1736, 2576, secret_base.c:1853 | Stub |
| `TryPutFrontierTVShowOnAir` | frontier_util.c:1679-1766 | Stub |
| `ShouldAirFrontierTVShow` | frontier_util.c:1674-1764 | Stub, returns FALSE |
| `HideBattleTowerReporter` | battle_tower.c:1848, 1851 | Stub |
| `BravoTrainerPokemonProfile_BeforeInterview1`, `_2`, `ContestLiveUpdates_*` | contest.c:2722, 5922-5926, contest_util.c:1008 | Stub |
| `InterviewBefore` | interview.inc, contest_util.c:597, LilycoveCity_PokemonTrainerFanClub:466 | Stub, sets `VAR_RESULT` TRUE ("already interviewed") |
| `InterviewAfter` | interview.inc:2, 316, contest_util.c:599 | Stub |
| `PutLilycoveContestLadyShowOnTheAir` | lilycove_lady.inc:433 | Stub |
| `TryPutLotteryWinnerReportOnAir` | GoldenrodCity_RadioTower_1F_hns:298, 363, LilycoveCity_DepartmentStore_1F:44, 109 | Stub |
| `TryPutTrainerFanClubOnAir`, `PutFanClubSpecialOnTheAir` | LilycoveCity_PokemonTrainerFanClub:147, 523 | Stub |
| `ShouldHideFanClubInterviewer` | LilycoveCity_PokemonTrainerFanClub:101 | Stub, returns TRUE |
| `TryPutNameRaterShowOnTheAir` | GoldenrodCity_House1_hns:89, LavenderTown_House2_Frlg:52, SlateportCity_NameRatersHouse:52 | Kept: returns whether the nickname changed, which the Name Rater's reply reads; puts nothing on air |
| `IsPokeNewsActive` (and `getpokenewsactive`) | shop.c:1302, 1707, 1791, 1871, field_specials.c:1459, scrcmd.c:3421, LilycoveCity_DepartmentStoreRooftop:6, 52, LilycoveCity_ContestLobby:14 | Stub, returns FALSE |
| `ClearTVShowData` | new_game.c:271 | Stub |
| `UpdateTVShowsPerDay` | clock.c:50 | Stub |
| `DeactivateAllNormalTVShows`, `ReceiveTvShowsData`, `ReceivePokeNewsData`, `SanitizeTVShowsForRuby`, `SanitizeTVShowLocationsForRuby` | record_mixing.c:192, 208, 225, 264-265, 275-276 | Stub; the calls go with [record mixing's TV part](#record-mixing) |
| `DoTVShow`, `DoPokeNews`, `DoTVShowInSearchOfTrainers` | tv.inc:52, 72, 80 | Stub, sets `VAR_RESULT` TRUE (program over) |
| `GetRandomActiveShowIdx`, `GetNextActiveShowIfMassOutbreak` | tv.inc:16, 18 | Stub, returns 255 |
| `GetSelectedTVShow` | tv.inc:21 | Stub, returns 0 |
| `ResetTVShowState`, `GetMomOrDadStringForTVMessage`, `CheckForPlayersHouseNews`, `IsTVShowAlreadyInQueue`, `IsGabbyAndTyShowOnTheAir`, `IsLeadMonNicknamedOrNotEnglish`, `SetContestCategoryStringVarForInterview` | tv.inc, interview.inc:97, 195 | Stub. `CheckForPlayersHouseNews` returns `PLAYERS_HOUSE_TV_NONE`, `IsTVShowAlreadyInQueue` TRUE, and `IsGabbyAndTyShowOnTheAir` and `IsLeadMonNicknamedOrNotEnglish` FALSE |
| `TurnOnTVScreen`, `TurnOffTVScreen`, `UpdateTVScreensOnMap` | players_house.inc:220, 491, 503, fieldmap.c:218, overworld.c:1080 | Reworked ([Screens](#tv)) |
| `ResetGabbyAndTy`, `GabbyAndTyGetBattleNum`, `GetGabbyAndTyLocalIds`, the battle count in `GabbyAndTyBeforeInterview` | new_game.c:272, gabby_and_ty.inc:13, 207-236 | Kept ([Gabby & Ty](#dependencies)) |
| `GabbyAndTyAfterInterview`, `GabbyAndTyGetLastQuote`, `GabbyAndTyGetLastBattleTrivia` | gabby_and_ty.inc:241-304 | Stub (unreachable while the interview is skipped) |
| `CountDigits`, `ConvertIntToDecimalString`, `GetPlayerIDAsU32`, `GetRibbonCount`, `ChangePokemonNickname`, `BufferMonNickname`, `IsMonOTIDNotPlayers`, `gTVStringVarPtrs` | money.c:187-225, scrcmd.c:2971, field_specials.c:548, 1733-1759, 5837, pokemon_summary_screen.c:3822, 5012, event_scripts.s:1482, 1533, pc_transfer.inc:11, and the Name Rater maps | Kept: general helpers that live in tv.c |

Callers are paths under `game/src/` and `game/data/`; map scripts are named
by map.

The `EventScript_TV` body in
[data/scripts/tv.inc](../../game/data/scripts/tv.inc) is replaced by the
[broadcast](#tv); its show, PokéNews, Mom or Dad, movie, and Lati-replay
branches go.

**Reporters.** Emerald's TV reporters exist only to feed shows: the Battle
Tower lobby reporter (`FLAG_HIDE_BATTLE_TOWER_REPORTER`), the Contest Lobby
reporter (`FLAG_HIDE_LILYCOVE_CONTEST_HALL_REPORTER`, on both lobby maps),
the Trainer Fan Club interviewer (`FLAG_HIDE_LILYCOVE_FAN_CLUB_INTERVIEWER`),
and the Slateport reporters in the Oceanic Museum and the Pokémon Fan Club.
New Game sets their hide flags; a reporter with no hide flag of its own
(the Slateport Fan Club one) is removed from the Wayfarer map. The
`InterviewBefore` stub is the safety net: any reporter still reached says
its "already interviewed" line.

### Save slots

Measured with `arm-none-eabi-gcc` on this tree's headers:

| Field | Size | Offset in SaveBlock1 | Treatment |
| --- | ---: | ---: | --- |
| `tvShows[25]` (`TVShow`, 36 bytes each) | 900 | 12,012 | Removed |
| `pokeNews[16]` (`PokeNews`, 4 bytes each) | 64 | 12,912 | Removed |
| `outbreak*` fields | 20 | 12,976 | Kept for swarms |
| `gabbyAndTyData` | 12 | 12,996 | Kept for Gabby & Ty |

SaveBlock1 uses 15,752 of its 15,872 bytes today; removing the two fields
frees 964 bytes, leaving 1,084. The offset comments in
[global.h](../../game/include/global.h) (`0x27CC` for `tvShows`) predate
fields added above them and are stale.

The other direct users of the removed fields change with them:

- **Easy Chat** ([easy_chat.c](../../game/src/easy_chat.c):1475-1509) reads
  `tvShows` words for the interview, fan club, contest, Battle Tower, and
  fan-question screens. Those screens are reached only from the retired
  interviews, so their cases go.
- **Indigo Hall of Fame rollback**
  ([post_battle_event_funcs.c](../../game/src/post_battle_event_funcs.c):34,
  60, 121) snapshots `tvShows` around the Wayfarer Indigo commit, because
  the Champion Ribbon could air a show. The snapshot drops the field.

### Record mixing

Record mixing's TV part goes: the `tvShows` and `pokeNews` fields of both
record packets ([record_mixing.c](../../game/src/record_mixing.c):55-56,
68-69), their copies (177-178, 191-193, 207-209, 238-239), and the receive
and deactivate calls (225, 264-265, 275-276). Daycare mail mixing used the
received `tvShows` bytes as its random source (260, 262, 273, 279); it takes
another part of the received packet instead. The packet no longer matches
Ruby's or Emerald's, which is accepted: record mixing with the original
games is not supported.

### Dependencies

Each system that touched TV, and what happens to it:

| Dependency | Today | Decision | Target |
| --- | --- | --- | --- |
| Mass outbreaks | The only outbreak starter is TV: after the Hoenn title (`FLAG_SYS_GAME_CLEAR`), a battle has a 1 in 200 chance to queue a show (tv.c:1653) from a Hoenn-only table (Routes 102, 114, 116, 117, 120; tv.c:208-237); watching it calls `StartMassOutbreak` (tv.c:1575, 4915), the only writer of the outbreak fields, for two days; `UpdateTVShowsPerDay` ends it (tv.c:1697-1750). The wild code reads `outbreakPokemonSpecies`, the map number and group, level, moves, and probability (`DoMassOutbreakEncounterTest` and `SetUpMassOutbreakEncounter`, wild_encounter.c:896-920, called at 1061 and 1229). | Keep, move | [Swarms](#swarms). Retiring TV removes the only starter, so swarms are off until that event exists. Already broken in Wayfarer: the show writes map group 0 (tv.c:1685), but Route 102 is in group 31, so no Emerald outbreak ever matches a map. |
| PokéNews sales | Also queued by `TryPutPokemonTodayOnAir` after the Hoenn title. Slateport: the Energy Guru's stall charges half (`>> IsPokeNewsActive(POKENEWS_SLATEPORT)`, shop.c:1302, 1707, 1791, 1871). Lilycove: the rooftop clearance-sale clerk appears (DepartmentStoreRooftop:6, 52). Game Corner: slot machines get their service-day odds (field_specials.c:1459). Blend Master: appears in the Contest Lobby (ContestLobby:14). | Keep, move | [Market sales](#market-sales). Until then, no sale: the stall charges full price, the rooftop clerk stays hidden, slots keep normal odds, and the Blend Master's replacement stays. |
| Latios/Latias news flash | The return-home scene turns the TV on, shows the flash, and starts the roamer from Mom's color question (players_house.inc:488-513). `EventScript_TV` replays the flash at the Littleroot TV and sets `FLAG_LATIOS_OR_LATIAS_ROAMING` with a junk `InitRoamer` (tv.inc:37-47), which also runs whenever neither `FLAG_SYS_TV_LATIAS_LATIOS` nor `FLAG_SYS_TV_HOME` is set (tv.c:3361-3386). | Keep the scene; drop the replay | [Emerald Lati roaming](emerald-lati-roaming.md#player-house-television-transaction) keeps the scene and its `TurnOnTVScreen` and `TurnOffTVScreen`. The replay goes, which matches that spec (no TV path starts a roamer) and removes the stray start. Roamer news later: [Roamer sightings](#roamer-sightings). |
| Gabby & Ty | Their battles on Routes 111, 118, and 120 rotate by the battle count in `gabbyAndTyData`: each map script calls `GabbyAndTy_EventScript_UpdateLocation` (Route111:47, Route118:8, Route120:56), which reads `GabbyAndTyGetBattleNum` and shows the right pair of objects (`FLAG_HIDE_ROUTE_*_GABBY_AND_TY_*`). The count goes up in `GabbyAndTyBeforeInterview` (tv.c:956), run after each win before the interview. The interview's Easy Chat quote and flags feed the "In Search of Trainers" show (`onAir`). | Keep battles, move interview | Stubbing the count would freeze them on Route 111 with their first party, so the count, `GabbyAndTyGetBattleNum`, and `GetGabbyAndTyLocalIds` are kept. After a win they skip the interview (the `FLAG_TEMP_SKIP_GABBY_INTERVIEW` path) until [Gabby & Ty](#gabby--ty) returns; `gabbyAndTyData` stays for it. |
| Dewford trend | Changing the trend phrase can air Trend Watcher (dewford_trend.c:194, 203). | Drop show | The trend, its record mixing, and the slot-machine seed that reads it (field_specials.c:1458) stay. |
| Contests | Contest results and the Lilycove reporter feed Bravo Trainer, Contest Live Updates, and Spot the Cuties; the contest lady airs a show. | Drop shows | Contests are unchanged; the reporters are hidden. |
| Secret bases | Visiting and redecorating queue record-mix shows. | Drop shows | Secret bases are unchanged. |
| Battle Tower and Frontier | Win streaks queue Frontier shows (frontier_util.c:1674-1766); the lobby reporter interviews the player; Battle Points feed a daily counter. | Drop shows | Facilities, streaks, and Battle Points are unchanged; the reporter is hidden. |
| Game stats | `GAME_STAT_WATCHED_TV` (tv.inc:3) and `GAME_STAT_GOT_INTERVIEWED` (interview.inc:3, tv.c:992) unlock two of the Old Man's stories (mauville_old_man.c:993, 1149). | Keep watched; interviewed pauses | World news TV keeps counting watches. Interviews stop counting until Gabby & Ty's interview returns, so that story stays locked until then. |
| Name Rater | The nickname check lives in `TryPutNameRaterShowOnTheAir`. | Keep check, drop show | See the entry-point table. |
| Lottery, Fan Club, Smart Shopper, Safari, fishing, Game Corner, rival trainer, treasure, berries, blender, battle seminar | Each queues a show. | Drop shows | The systems are unchanged. |

**Gameplay-breaking if stubbed blindly:** Gabby & Ty's rotation, the Name
Rater's reply, the general helpers (`CountDigits` and the rest, used by money
and the summary screen), and the TV screen swap. They are kept or reworked
above. Swarms and PokéNews sales stop, which is accepted until their events.

## Later

World events, all regions. Each is a news item plus a real effect, and each
gets its own spec.

### Swarms

- **Table.** Its own all-region table of routes and species, in Kanto,
  Johto, Hoenn, and Sevii; each entry names a map, a species, and a level
  rule following world scaling.
- **Effect.** Writes the existing outbreak fields directly
  (`outbreakPokemonSpecies`, `outbreakLocationMapNum`,
  `outbreakLocationMapGroup` with the map's real group, level, moves, and
  probability), so `DoMassOutbreakEncounterTest` works unchanged. It
  replaces `StartMassOutbreak`'s TV path and owns its own end
  (`outbreakDaysLeft` or a world-progress rule), since `UpdateTVShowsPerDay`
  no longer counts it down.
- **News.** A headline, local to the route's region, while it lasts; later
  also a phone call from a Close friend, as in Gold and Silver.

### Market sales

- **Effect.** A real discount at one converted mart: the sale names a
  `MART_PROFILE_*` ([runtime contract](global-tr-pokemarts.md#runtime-contract))
  and halves its prices while it lasts, at the four price sites that apply
  the PokéNews shift today (shop.c:1302, 1707, 1791, 1871). Stock is
  unchanged.
- **News.** A headline, local to the shop's region.

### Roamer sightings

- **Effect.** None beyond the hint: an item naming where an active roamer is
  now, from `GetRoamerLocation` ([roamer.h](../../game/include/roamer.h)),
  for each regional roamer the
  [Lati roaming spec](emerald-lati-roaming.md) defines.
- **News.** Everyday, local to the roamer's region.

### Gabby & Ty

- **Effect.** Roaming reporters in every region who turn up after big
  moments (a badge, a title, a Master) and interview the player, reusing
  their battles, parties, and Easy Chat interview.
- **News.** The interview airs as a headline, and counts
  `GAME_STAT_GOT_INTERVIEWED` again.

### Also later

- **Phone calls:** Close friends call with an item from the feed
  ([phone contacts](sevii-masters.md#phone-contacts)).
- **Daily rotation:** `step` adds the in-game day count.

## Saved state

World news adds no saved data. Everything it reads is saved by its owners:
league titles, reign records, the call counter and last call numbers
([Leagues](leagues.md#saved-state)); lifetime wins; the haunt placement
([Notable haunts](notable-haunts.md#saved-state)); friendship
([Notable trainers](notable-trainers.md#friendship)); badges and TR. The
broadcast cursor lives in RAM only.

Retiring TV removes `tvShows` and `pokeNews` from SaveBlock1
([save slots](#save-slots)); New Game no longer clears them. The outbreak
fields and `gabbyAndTyData` stay, cleared at New Game as today.

## Load validation

Nothing of World news's own is loaded. On load:

1. The broadcast cursor is 0.
2. Items are rebuilt from their owners' state, after those owners' own
   load validation and the haunts' placement recompute.
3. A reigning champion or placement naming an unknown character is its
   owner's invalid save; World news never repairs it and skips any item it
   cannot resolve.

## Presentation

- **Notable friends:** the trainer's speaker label on both lines.
- **Townsfolk:** the gossip's usual text box, lead-in and fact on separate
  pages.
- **TV:** sign-style boxes with the screen lit; the screen turns off after
  the outro or the empty line.
- **Radio:** the WORLD NEWS station name on the dial, the music, and the
  DJ's lines scrolling like other stations.

## Acceptance

Required implementation evidence (not yet run):

1. **Feed.** The same inputs give the same feed; feed order and template
   choice match golden fixtures over world progress 0-200 with scripted
   league histories (vacant titles, a decline, a loss, a player win, a
   notable Master).
2. **Latest event.** With an invitation waiting or an accepted event at one
   league, `LEAGUE_LATEST` names the other league that called most recently;
   with one league calling alone, it names that league.
3. **Fame.** Player items reach townsfolk, TV, and radio only at TR 80, a
   title, or Master; a notable speaker uses the haunts' rule.
4. **Templates.** Every template follows the writing rule, and every
   resolved item fits its box with worst-case slots; radio lines fit 38
   characters.
5. **Channels.** A friend gives whereabouts first and falls back as
   specified; townsfolk give only local items; TV and radio give only
   headlines, global or local.
6. **Rotation.** Two gossips on one map give different items whenever there
   are at least two candidates; repeated TV broadcasts walk the headlines in
   order and wrap; the cursor resets on load.
7. **TV.** Every `MB_TELEVISION` tile in Wayfarer runs the World news script
   unless a background event covers it; screens are on only when the map's
   TV has a candidate, with the Cove Lily Motel and Lati-scene exceptions;
   each broadcast increments `GAME_STAT_WATCHED_TV`.
8. **Radio.** The station is found at position 45 in every region once the
   radio is enabled, is not replaced by the Rocket takeover, and never
   overflows its line buffers.
9. **Stubs.** The build compiles with every caller unchanged except record
   mixing, Easy Chat, and the Hall of Fame snapshot; no code writes
   `tvShows` or `pokeNews`; `IsPokeNewsActive` is always FALSE; interviews
   never start.
10. **Kept behaviour.** Gabby & Ty rotate routes and parties after each win;
    the Name Rater says a changed nickname is new; money and summary-screen
    numbers are unchanged; the return-home scene still turns its TV on and
    off and starts the chosen roamer only through Mom's question.
11. **Save.** SaveBlock1 is 964 bytes smaller; New Game and a reload work.

## Open questions

- Whether townsfolk should also know the player locally before TR 80, for
  example once the player holds a badge from the gossip's region (the
  leagues' known-there rule).
- Whether a gossip should say its authored line first and the news second,
  rather than instead of it.
- Whether townsfolk should also repeat global headlines (`LEAGUE_MASTER`,
  the Masters' title, `PLAYER_KNOWN`, `PLAYER_MASTER`), which v0 leaves to
  TV, radio, and friends.
- The radio station's position, name, and music.
- Where a World news DJ or anchor could be met in person: the Lavender
  Radio Station (`LavenderTown_RadioStation_hns`, today a lobby with the
  Pokémon Tower door and the director who gives the Kanto expansion,
  `FLAG_KANTO_RADIO_GOT`) or Goldenrod's Radio Tower.
- Whether FRLG-layout interiors need their own TV metatile ids for the
  screen swap: `GetTVMetatileId` (tv.c:865-872) picks HNS ids only for
  `LAYOUT_VERSION_HNS` layouts and Emerald ids for every other one.

## References

- [World news PRD](../prds/world-news.md)
- [Notable haunts](notable-haunts.md)
- [Notable trainers](notable-trainers.md)
- [Leagues](leagues.md)
- [Sevii Masters](sevii-masters.md)
- [Global TR Poké Marts](global-tr-pokemarts.md)
- [Emerald Lati roaming](emerald-lati-roaming.md)
- [Notable trainer voice bits](../research/notable-trainer-voices.md)

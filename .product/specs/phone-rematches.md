# Phone rematches

PRDs: [Hoenn phone rematches](../prds/hoenn-phone-rematches.md),
[Sevii and Kanto coast phone rematches](../prds/sevii-coast-phone-rematches.md),
[Daily world slots](../prds/daily-world-slots.md#phone-numbers-and-rematches)
Implemented: No

## Scope

One phone system for every route trainer with a number: HNS's 39 Johto and
Kanto trainers as today (Nicole is compiled out of Wayfarer), Emerald's 64
Hoenn rematch trainers, and 41 FRLG families on Sevii and the Kanto coast. It covers registration, the contact list, calls,
readiness, rematch teams, the save state and every new or converted call
text. `IS_WAYFARER` only.

## Contacts

| Group | Trainers | Rematch teams | Source of call texts |
| --- | ---: | --- | --- |
| HNS (Johto, Kanto) | 39 | HNS rematch table | `game/data/text/match_call_hns.inc`, unchanged apart from the Battle and gift text rules [below](#hns-battle-and-gift-texts) |
| Hoenn | 64 | Emerald's rematch table rows | Converted Emerald Match Call texts, [below](#hoenn-call-texts) |
| Sevii | 32 families | Sevii rematch registry | New, [below](#sevii-and-coast-call-texts) |
| Kanto coast | 9 families | Coast rematch registry | New, [below](#sevii-and-coast-call-texts) |

Pairs share one contact and battle together.

## Rematch table and save state

- **Hoenn rows.** Wayfarer compiles HNS's `gRematchTable`, which reuses
  Emerald's entry names for different people (Emerald's Rose slot holds Joey).
  Hoenn trainers get new entries and never reuse an HNS index. Indexes 28–63
  are empty and take the first 36; the table grows past its 90 entries for the
  other 28. Entries are appended, never inserted, so existing indexes keep
  their meaning.
- **Appended range.** The 28 appended indexes 90–117 are normal trainers. The
  special rows (from `REMATCH_WALLY_VR`) and the Elite Four rows (from
  `REMATCH_ELITE_FOUR_ENTRIES`, `REMATCH_SIDNEY` = 85) and rows 76–89 keep
  their constants and meaning. Several checks treat every index at or above
  `REMATCH_ELITE_FOUR_ENTRIES` as Elite Four, so a new
  `REMATCH_APPENDED_START` (90) to `REMATCH_TABLE_ENTRIES` range is defined and
  each of these treats it as normal:
  - `IsRematchForbidden` (`battle_setup.c`): not forbidden.
  - `MatchCall_IsRematchable_Trainer` (`pokenav_match_call_data.c`): reads
    `trainerRematches`.
  - `GetRematchTrainerIdVSSeeker` (`vs_seeker.c`): returns the normal
    rematch level path (its Sevii and coast branches are retired).
  - The loops over `REMATCH_SPECIAL_TRAINER_START` also cover the appended
    range: `UpdateRandomTrainerRematches` (`battle_setup.c`), and in
    `match_call.c` the random-call candidate loop with its
    `candidates[REMATCH_SPECIAL_TRAINER_START]` array (resized to the loop's
    new bound), `GetNumRegisteredTrainers` and `GetActiveMatchCallTrainerId`.
    The `GetNumRematchTrainersFought` loop is under `#if !IS_HNS` and isn't
    compiled in Wayfarer, so it needs no change.
  `gym_leader_rematch.c` iterates only the special-to-Elite-Four rows and
  stays as is.
- **Sevii and coast families** stay in their own registries. Their stage and
  ready bits already live in Wayfarer's Sevii and coast save state, not in the
  shared rematch array.
- **Registration flags.** One per contact. HNS and Hoenn contacts use the
  registered-flag block at `0x310`–`0x369` for table indexes 0–89 (Hoenn takes
  the empty indexes 28–63 inside it, so nothing moves). The block can't grow,
  because it ends flush against `HNS_EXTENDED_CONTENT_START` (`0x36A`) and the
  flags beyond `0x495` are taken, so the 28 appended indexes (90–117) use
  `0x8BE`–`0x8D9` (`SYS_FLAGS + 0x5E` to `+ 0x79`). No Wayfarer flag uses that
  window; it sits between the Battle Frontier symbols (`0x8BD`) and the Tower
  flags (`0x8E5`), and `0x8DA`–`0x8E4` stay spare. One helper maps an index to
  `0x310 + index` below 90 and `0x8BE + (index - 90)` from 90, and every
  registered-flag access goes through it. These sites add the base to the
  index directly today and switch to it: `IsRematchEntryRegistered`
  (`pokenav_match_call_list.c`), `TrainerIsMatchCallRegistered` (which
  already wraps the access in `battle_setup.c`, so it calls the helper),
  `GetTrainerMatchCallFlag` (`battle_setup.c`), `IsTrainerRegistered`
  (`field_specials.c`), `GetNumRegisteredTrainers` and
  `GetActiveMatchCallTrainerId` (`match_call.c`) and
  `SetMatchCallRegisteredFlag` (`pokenav_match_call_data.c`). A grep for
  `TRAINER_REGISTERED_FLAGS_START +` finds no other site.
  The gym leader, Elite Four, Wally and Steven rows have their own
  `FLAG_REGISTERED_*` flags and are unaffected.
  Sevii and coast contacts use 41 new Sevii bank flags, in slots 61 upward
  beside the 97 restored item flags (138 of the 195 free slots).
- **Rematch progress.** `trainerRematches` grows from 100 (`MAX_REMATCH_ENTRIES`)
  to 118, the table size. This adds 18 bytes to SaveBlock1, which has 112
  free, leaving 94. The day-start sets and the other daily slot state live in
  `PokemonStorage`, not here ([daily world slots](daily-world-slots.md#save-state)).
  It changes the SaveBlock1 layout; Wayfarer has no released
  saves, so no migration is needed (`AGENTS.md`).
- **Contact list.** `matchCallEntries` grows from 99 to cover every contact plus
  the special headers, with a bounds check. The list also reads the Sevii and
  coast registries.

## Getting a number

After you win against a trainer who has a number and isn't registered, they
offer it with a yes/no prompt. Yes registers them and shows the "Registered …
in the POKéGEAR" message. No leaves them unregistered, and they ask again
after your next win against them, home or wandering. This replaces the
automatic registration in `RegisterTrainerInMatchCall` for Wayfarer. A trainer who isn't registered, because they have no number or you said no, unlocks rematch teams by Trainer Rating instead (see [Readiness and teams](#readiness-and-teams)).

## Readiness and teams

- **Readiness.** HNS makes a trainer ready after 255 steps on their home map.
  Wayfarer counts steps anywhere in the trainer's region instead: every 255
  steps in a region, one registered, already-fought trainer of that region
  who isn't ready becomes ready (chosen by `roll`, as in
  [daily world slots](daily-world-slots.md#deterministic-draws)). There is no
  call at that moment: the trainer is placed and calls from the next day (see
  [Placement](#readiness-and-teams)).
- **Teams, registered contacts.** A ready trainer's next battle uses the tier
  after the higher of the highest tier you have fought and the highest tier
  unlocked by Trainer Rating (below), in the rematch table or the Sevii or coast
  registry. After the last, they keep it. Registering later never lowers it.
  Winning clears readiness. Levels come from [regular trainer scaling](trainer-party-scaling.md);
  the HNS badge-level ceiling no longer applies in Wayfarer.
- **Teams, trainers without a registered number.** A trainer with no number,
  or whose number you declined, doesn't use readiness. Their rematch teams
  unlock with Trainer Rating, counted in authored order so a trainer with
  fewer teams uses the lowest steps (placeholders):

  | Rematch team | Unlocks at Trainer Rating |
  | ---: | ---: |
  | 2 | 40 |
  | 3 | 80 |
  | 4 | 120 |
  | 5 | 160 |

  The team is derived from the current Trainer Rating when the battle starts,
  so it needs no save state. Registering later keeps every team already
  unlocked: a registered contact's next team follows the higher of its fought
  and Trainer Rating tiers, as above.
- **Placement.** The sets of ready trainers and of trainers holding a gift are
  fixed when the day starts and stamped with the day (`readyAtDayStart` and
  `giftAtDayStart` in [daily world slots](daily-world-slots.md#save-state),
  one bit per contact each). A trainer in either set always has a spot that day
  ([daily world slots](daily-world-slots.md#who-stands-where-today)). A trainer
  who becomes ready during the day is placed from the next day. Winning
  against a ready trainer clears readiness, but today's arrangement stays. If
  their group has no rotating spot, they are not called until a day when it
  has one. A contact whose slot is fixed (excluded from rotation) is placed at
  home for ready and gift calls.

## Calls

- **Rate.** The total call rate stays HNS's: the random-call timer and chance
  are unchanged, and the caller is chosen among registered contacts. More
  contacts means each one calls less often.
- **Text selection.** A contact in today's day-start ready set uses their
  Battle text. A contact in the day-start gift set uses their FoundItem text.
  Any other call uses a random General text, as in HNS. A contact who became
  ready or got a gift later in the day, whose slot is cleared today, or who
  `WhereIsTrainerToday` can't place, makes a General call: calls never name a
  trainer who isn't standing somewhere unbeaten today.
- **Today's place.** Before a call, `{STR_VAR_2}` is set to the map name from
  `WhereIsTrainerToday`. Every Battle and FoundItem text names it; HNS's
  existing Battle texts gain a line that does, and its FoundItem texts are
  rewritten.
- **Text format.** Every contact has `General1`–`General3` and `Battle`, in
  the `sHnsMatchCallTrainers` shape. Twelve HNS contacts have only
  `General1` and `General2`. Gift contacts also have a `FoundItem` text. A
  contact without texts fails the build rather than showing an empty box.

## Gifts

Nine HNS contacts hand out an item: Wade, Alan, Dana, Derek, Tully, Wilton,
Kenji, Beverly and Jose (`HNS_MC_ITEM*` in `game/src/match_call.c`). HNS rolls
a gift on a call, sets the contact's `HAS_ITEM` flag and item variable, and the
trainer hands it over from their home map script. Under rotation that script no
longer runs once the home slot rotates, so:

- **Placement.** A contact whose gift flag is set when the day starts is in
  `giftAtDayStart` and is placed that day, like a ready rematch.
- **Hand-off.** The shared rotating-trainer script runs the hand-off whenever
  the player talks to, or is spotted by, an occupant whose gift flag is set:
  the `HasItem` text, `giveitem`, clearing the flag and the `GaveItem` or
  `NoRoom` text, exactly as the home script does. The hand-off comes before
  the battle.
- **Text.** The FoundItem call names today's place with `{STR_VAR_2}` instead
  of the home route (the new texts are [below](#hns-battle-and-gift-texts)).
- **Roll.** The gift roll keeps HNS's 1-in-5 chance. It sets the flag when the
  contact is picked by the step event, and the call announcing it comes from
  the next day, like readiness.

## Vs. Seeker

Retired. Its giver isn't compiled into Wayfarer. Remove its readiness paths for
the Sevii and coast registries and stop it from suppressing step-based
rematches; the item itself is left unobtainable.

## Validation

- Every Hoenn, Sevii and coast contact registers, appears in the list, calls
  with each of its texts, and rematches through every team in order.
- Existing HNS contacts keep their indexes, texts, gifts and team order.
- A trainer who becomes ready, or whose gift flag is set, mid-day is placed
  and calls only from the next day; a win against a ready trainer leaves
  today's arrangement unchanged.
- Trainers without a registered number unlock team 2 to 5 at Trainer Rating
  40, 80, 120 and 160.
- Every gift hand-off still works from a rotating slot, including `NoRoom`.
- No flag overlaps, and the contact list never overflows.
- Registering an appended contact (index 90 or above) sets a flag in
  `0x8BE`–`0x8D9` and leaves `0x36A` onward (`HNS_EXTENDED_CONTENT_START`, the
  decoration flags) unchanged.
- A contact whose slot is fixed still gets Battle and FoundItem calls, naming
  its home map.
- Every Battle and FoundItem text contains `{STR_VAR_2}`, and every text fits the text box.

## Hoenn call texts

Source: Emerald Match Call texts (`game/data/text/match_call.inc`) selected per trainer by `sMatchCallTrainers[]` in `game/src/match_call.c`, for the 64 normal entries of the non-HNS `gRematchTable` (`REMATCH_ROSE` .. `REMATCH_KIRA_AND_DAN`; Wally onward excluded).

How Emerald picks the texts (so the conversion is traceable):
- General1 = the trainer's `generalTextId` (a `PersonalizedTextN`). Emerald also sometimes swaps in Battle Frontier streak chats; those are dropped.
- General2 / General3 = two of the three battle-topic chats `WildBattleTextN` / `NegativeBattleTextN` / `PositiveBattleTextN` (N = the trainer's `BATTLE_TEXT_IDS(N)`; Thalia and Sawyer have individual ids). Many trainers share the same N, so the same text sets recur; to avoid identical pairs, I rotated which two of the three each trainer in a shared-N group gets. Wild chats about a species keep their story ("almost caught it") but use `{STR_VAR_3}`.
- Battle = the trainer's `differentRouteMatchCallTextId` (`DifferentRouteBattleRequestTextN`), whose `{STR_VAR_2}` is the map name (kept as `{STR_VAR_2}` = today's place).

Conventions in the converted text: `{STR_VAR_1}` -> the trainer's name in capitals (pairs: `LILA & ROY` etc.); `{PLAYER}{KUN}` -> `{PLAYER}`; every line is at most 30 characters with `{PLAYER}` = 7, `{STR_VAR_2}` = 16, `{STR_VAR_3}` = 10 (machine-checked, longest line is exactly 30); at most 2 lines per box. Emerald's lines are wider, so every text was re-wrapped; box pauses were re-flowed (the first greeting box is kept, the rest flows two lines at a time), so a few `\p` pauses sit mid-sentence, as in the HNS file.

### Contact table

| # | Contact (label) | Trainer constants | Class | Home | Voice | Chats (G2 / G3) |
|---|---|---|---|---|---|---|
| 1 | ROSE (`Rose`) | `ROSE_1..ROSE_5` | Aroma Lady | Route 118 | gentle, refined, loves sweet scents | lost / won |
| 2 | ANDRES (`Andres`) | `ANDRES_1..ANDRES_5` | Ruin Maniac | Route 105 | earnest, a bit down on himself, ruin-hunter | lost / won |
| 3 | DUSTY (`Dusty`) | `DUSTY_1..DUSTY_5` | Ruin Maniac | Route 111 | gruff veteran explorer | catch / lost |
| 4 | LOLA (`Lola`) | `LOLA_1..LOLA_5` | Tuber | Route 109 | dreamy beach girl, always hungry | lost / won |
| 5 | RICKY (`Ricky`) | `RICKY_1..RICKY_5` | Tuber | Route 109 | laid-back beach boy, eats a lot | lost / won |
| 6 | LILA & ROY (`LilaAndRoy`) | `LILA_AND_ROY_1..LILA_AND_ROY_5` | Sis and Bro | Route 124 | bickering siblings | lost / won |
| 7 | CRISTIN (`Cristin`) | `CRISTIN_1..CRISTIN_5` | Cooltrainer | Route 121 | polite, serious | lost / won |
| 8 | BROOKE (`Brooke`) | `BROOKE_1..BROOKE_5` | Cooltrainer | Route 111 | bubbly, friendly rival | lost / won |
| 9 | WILTON (`Wilton`) | `WILTON_1..WILTON_5` | Cooltrainer | Route 111 | weary cooltrainer, confides in you | lost / won |
| 10 | VALERIE (`Valerie`) | `VALERIE_1..VALERIE_5` | Hex Maniac | Mt. Pyre | spooky whisperer, giggles | catch / lost |
| 11 | CINDY (`Cindy`) | `CINDY_1..CINDY_6` | Lady | Route 104 | sheltered, delighted by phones | catch / won |
| 12 | THALIA (`Thalia`) | `THALIA_1..THALIA_5` | Beauty | Abandoned Ship | restless, bored, wants a win | catch / won |
| 13 | JESSICA (`Jessica`) | `JESSICA_1..JESSICA_5` | Beauty | Route 121 | chatty, always getting lost | catch / lost |
| 14 | WINSTON (`Winston`) | `WINSTON_1..WINSTON_5` | Rich Boy | Route 104 | pompous, loudly rich | lost / won |
| 15 | STEVE (`Steve`) | `STEVE_1..STEVE_5` | Pokémaniac | Route 114 | creepy chuckle, collector's eye | lost / won |
| 16 | TONY (`Tony`) | `TONY_1..TONY_5` | Swimmer | Route 107 | bold sea-man, loud | lost / won |
| 17 | NOB (`Nob`) | `NOB_1..NOB_5` | Black Belt | Route 115 | karate fanatic, shouts | lost / won |
| 18 | KOJI (`Koji`) | `KOJI_1..KOJI_5` | Black Belt | Route 127 | fanatic about the sea, a little jealous | catch / lost |
| 19 | FERNANDO (`Fernando`) | `FERNANDO_1..FERNANDO_5` | Guitarist | Route 123 | laid-back musician, rhymes | catch / lost |
| 20 | DALTON (`Dalton`) | `DALTON_1..DALTON_5` | Guitarist | Route 118 | songwriter, sings his chats | catch / lost |
| 21 | BERNIE (`Bernie`) | `BERNIE_1..BERNIE_5` | Kindler | Route 114 | camping-expert, upbeat | lost / won |
| 22 | ETHAN (`Ethan`) | `ETHAN_1..ETHAN_5` | Camper | Jagged Pass | cheerful, girl-crazy hiker | catch / lost |
| 23 | JOHN & JAY (`JohnAndJay`) | `JOHN_AND_JAY_1..JOHN_AND_JAY_5` | Old Couple | Meteor Falls | warm old pair, proud of youngsters | catch / won |
| 24 | JEFFREY (`Jeffrey`) | `JEFFREY_1..JEFFREY_5` | Bug Maniac | Route 120 | long, silent pauses | catch / lost |
| 25 | CAMERON (`Cameron`) | `CAMERON_1..CAMERON_5` | Psychic | Route 123 | intense, obsessed with you as a rival | catch / won |
| 26 | JACKI (`Jacki`) | `JACKI_1..JACKI_5` | Psychic | Route 123 | intuitive, sensing strong trainers | lost / won |
| 27 | WALTER (`Walter`) | `WALTER_1..WALTER_5` | Gentleman | Route 121 | courtly world traveler | catch / won |
| 28 | KAREN (`Karen`) | `KAREN_1..KAREN_5` | School Kid | Route 116 | earnest schoolgirl, idolizes ROXANNE | catch / lost |
| 29 | JERRY (`Jerry`) | `JERRY_1..JERRY_5` | School Kid | Route 116 | teary schoolboy, learns from ROXANNE | catch / won |
| 30 | ANNA & MEG (`AnnaAndMeg`) | `ANNA_AND_MEG_1..ANNA_AND_MEG_5` | Sr. and Jr. | Route 117 | bossy senior, put-upon junior | lost / won |
| 31 | ISABEL (`Isabel`) | `ISABEL_1..ISABEL_5` | Pokéfan | Route 110 | gushes over her POKéMON | lost / won |
| 32 | MIGUEL (`Miguel`) | `MIGUEL_1..MIGUEL_5` | Pokéfan | Route 103 | giddy fan, can't hold back | catch / lost |
| 33 | TIMOTHY (`Timothy`) | `TIMOTHY_1..TIMOTHY_5` | Expert | Route 115 | solemn sage | lost / won |
| 34 | SHELBY (`Shelby`) | `SHELBY_1..SHELBY_5` | Expert | Mt. Chimney | relaxed old soul, hot-spring lover | lost / won |
| 35 | CALVIN (`Calvin`) | `CALVIN_1..CALVIN_5` | Youngster | Route 102 | goofy kid with odd theories | lost / won |
| 36 | ELLIOT (`Elliot`) | `ELLIOT_1..ELLIOT_5` | Fisherman | Route 106 | fishing-obsessed, hearty | catch / won |
| 37 | ISAIAH (`Isaiah`) | `ISAIAH_1..ISAIAH_5` | Triathlete | Route 128 | swimmer, a bit lost | catch / lost |
| 38 | MARIA (`Maria`) | `MARIA_1..MARIA_5` | Triathlete | Route 117 | high-altitude training evangelist | catch / lost |
| 39 | ABIGAIL (`Abigail`) | `ABIGAIL_1..ABIGAIL_5` | Triathlete | Route 110 | cyclist, wind-chasing | catch / won |
| 40 | DYLAN (`Dylan`) | `DYLAN_1..DYLAN_5` | Triathlete | Route 117 | mid-race, panting | catch / won |
| 41 | KATELYN (`Katelyn`) | `KATELYN_1..KATELYN_5` | Triathlete | Route 128 | sunbather swimmer, scatterbrained | lost / won |
| 42 | BENJAMIN (`Benjamin`) | `BENJAMIN_1..BENJAMIN_5` | Triathlete | Route 110 | chill, 'take it casual' | lost / won |
| 43 | PABLO (`Pablo`) | `PABLO_1..PABLO_5` | Triathlete | Route 126 | swimmer, prune-fretting | catch / lost |
| 44 | NICOLAS (`Nicolas`) | `NICOLAS_1..NICOLAS_5` | Dragon Tamer | Meteor Falls | short and dignified | lost / won |
| 45 | ROBERT (`Robert`) | `ROBERT_1..ROBERT_5` | Bird Keeper | Route 120 | driven, hates to lose | catch / won |
| 46 | LAO (`Lao`) | `LAO_1..LAO_5` | Ninja Boy | Route 113 | formal, vanishing ninja | catch / lost |
| 47 | CYNDY (`Cyndy`) | `CYNDY_1..CYNDY_5` | Battle Girl | Route 115 | disciplined trainer | catch / lost |
| 48 | MADELINE (`Madeline`) | `MADELINE_1..MADELINE_5` | Parasol Lady | Route 113 | delicate, fussy about ash | catch / lost |
| 49 | JENNY (`Jenny`) | `JENNY_1..JENNY_5` | Swimmer | Route 124 | dreamy floater, directionless | catch / won |
| 50 | DIANA (`Diana`) | `DIANA_1..DIANA_5` | Picnicker | Jagged Pass | cheerful camper | catch / won |
| 51 | AMY & LIV (`AmyAndLiv`) | `AMY_AND_LIV_1..AMY_AND_LIV_6` | Twins | Route 103 | bouncy twins, finish each other | lost / won |
| 52 | ERNEST (`Ernest`) | `ERNEST_1..ERNEST_5` | Sailor | Route 125 | landlocked-sailor musing | lost / won |
| 53 | CORY (`Cory`) | `CORY_1..CORY_5` | Sailor | Route 108 | shouting radio-host energy | catch / lost |
| 54 | EDWIN (`Edwin`) | `EDWIN_1..EDWIN_5` | Collector | Route 110 | quietly demanding for new POKéMON | catch / won |
| 55 | LYDIA (`Lydia`) | `LYDIA_1..LYDIA_5` | Pokémon Breeder | Route 117 | caring, curious breeder | catch / won |
| 56 | ISAAC (`Isaac`) | `ISAAC_1..ISAAC_5` | Pokémon Breeder | Route 117 | clean-air evangelist | catch / won |
| 57 | GABRIELLE (`Gabrielle`) | `GABRIELLE_1..GABRIELLE_5` | Pokémon Breeder | Mt. Pyre | kind mentor | lost / won |
| 58 | CATHERINE (`Catherine`) | `CATHERINE_1..CATHERINE_5` | Pokémon Ranger | Route 119 | prepared, vigilant | lost / won |
| 59 | JACKSON (`Jackson`) | `JACKSON_1..JACKSON_5` | Pokémon Ranger | Route 119 | calm advocate of cooperation | lost / won |
| 60 | HALEY (`Haley`) | `HALEY_1..HALEY_5` | Lass | Route 104 | chipper, scoreboard-keeping | lost / won |
| 61 | JAMES (`James`) | `JAMES_1..JAMES_5` | Bug Catcher | Petalburg Woods | bug-obsessed schoolboy | catch / won |
| 62 | TRENT (`Trent`) | `TRENT_1..TRENT_5` | Hiker | Route 112 | breathless mountaineer | catch / won |
| 63 | SAWYER (`Sawyer`) | `SAWYER_1..SAWYER_5` | Hiker | Mt. Chimney | mountain-lover | catch / won |
| 64 | KIRA & DAN (`KiraAndDan`) | `KIRA_AND_DAN_1..KIRA_AND_DAN_5` | Young Couple | Abandoned Ship | lovey-dovey treasure hunters | catch / won |

### Changes from Emerald

Applied to every text (not repeated below): `{STR_VAR_1}` -> literal name; `{KUN}` removed; line re-wrap to 30 columns; for Wild/Positive chats `{STR_VAR_2}` (route species / party species) -> `{STR_VAR_3}` (the engine's random species from the last-beaten party); Emerald's frontier-streak alternates not carried. All other Emerald wording is as-is, including the many shared Battle and chat texts.

Lines reworded (trainer / slot / Emerald source id / what and why):

- ROSE / General1 (PersonalizedText3): 'I love where I am now' made location-specific; now names her home route.
- LOLA / General1 (PersonalizedText5): 'Know what I'm doing today?' / 'from here' implied she is on the beach now; made it a habit.
- LILA & ROY / General1 (PersonalizedText61): Pair voice; Emerald's ROY-complaining-about-his-sister retold as the pair's shared banter.
- LILA & ROY / General2 (NegativeBattleText1): Pair voice: I/my -> we/our; name written as the pair.
- LILA & ROY / General3 (PositiveBattleText1): Pair voice: I/my -> we/our; name written as the pair. {STR_VAR_2} -> {STR_VAR_3}.
- LILA & ROY / Battle (DifferentRouteBattleRequestText1): Pair voice: I/my/me -> we/our/us, pair named in greeting.
- WILTON / General1 (PersonalizedText7): Trimmed the 'But I guess that goes with being a COOLTRAINER' box for length; otherwise original.
- VALERIE / General1 (PersonalizedText9): 'sleep here at MT. PYRE' (present location) -> 'sleep at MT. PYRE'.
- THALIA / General1 (PersonalizedText14): 'Since I'm already here' (implied present location) removed; 'sticking around' made a habit; trimmed.
- WINSTON / General1 (PersonalizedText12): Trimmed the 'my POKéMON have also grown stronger' box for length.
- STEVE / General1 (PersonalizedText13): {STR_VAR_2} (wild species of the route) replaced with {STR_VAR_3}, since the Pokégear has no route-species variable.
- TONY / General1 (PersonalizedText15): 'Whoops, giant surf rising!' (happening now) turned into a general remark.
- KOJI / General1 (PersonalizedText59): Cut the BRUNO / 'big wave' tangent and the 'toughened by wild waves' line (Elite Four cross-reference, long).
- DALTON / General1 (PersonalizedText18): {STR_VAR_2} (party species) -> {STR_VAR_3}; name written out.
- ETHAN / General1 (PersonalizedText20): 'than this one' -> JAGGED PASS (his home) so it no longer implies where he is today.
- JOHN & JAY / General1 (PersonalizedText60): Pair voice; trimmed 'long and enduring relationships' box.
- JOHN & JAY / General2 (WildBattleText12): Pair voice: I/my -> we/our; name written as the pair.
- JOHN & JAY / General3 (PositiveBattleText12): Pair voice: I/my -> we/our; name written as the pair.
- JOHN & JAY / Battle (DifferentRouteBattleRequestText12): Pair voice: I/my/me -> we/our/us, pair named in greeting.
- ANNA & MEG / General1 (PersonalizedText27): Pair voice (ANNA & MEG greet together); trimmed the first two lines about wishing for siblings and the 'Huh?' beat.
- ANNA & MEG / General2 (NegativeBattleText9): Pair voice: I/my -> we/our; name written as the pair.
- ANNA & MEG / General3 (PositiveBattleText9): Pair voice: I/my -> we/our; name written as the pair. {STR_VAR_2} -> {STR_VAR_3}.
- ANNA & MEG / Battle (DifferentRouteBattleRequestText9): Pair voice: I/my/me -> we/our/us, pair named in greeting.
- ISABEL / General1 (PersonalizedText29): {STR_VAR_2} -> {STR_VAR_3}; dropped 'I'm so glad I could share' box for length.
- MIGUEL / General1 (PersonalizedText28): {STR_VAR_2} -> {STR_VAR_3}; {POKEBLOCK} -> 'a treat' (no POKéBLOCK on the Pokégear).
- SHELBY / General1 (PersonalizedText31): 'I am feeling… soaking in this hot-spring tub' (present location) -> a general liking.
- CALVIN / General1 (PersonalizedText32): Trimmed the 'What do I mean by that?' explanation boxes for length.
- DYLAN / General1 (PersonalizedText36): Trimmed one beat for length.
- KATELYN / General1 (PersonalizedText39): POKéNAV -> POKéGEAR; dropped the DEVON brand line (brand/story reference).
- BENJAMIN / General1 (PersonalizedText34): Trimmed the 'teacher or maybe an artist' tangent for length.
- ROBERT / General1 (PersonalizedText42): {STR_VAR_2} -> {STR_VAR_3}.
- CYNDY / General1 (PersonalizedText44): {STR_VAR_2} -> {STR_VAR_3}.
- DIANA / General1 (PersonalizedText47): 'I'm up in the mountains now' (present location) -> 'I love the mountains'.
- AMY & LIV / General1 (PersonalizedText48): Pair voice: 'I'm raising POKéMON with LIV' -> both speak.
- AMY & LIV / General2 (NegativeBattleText2): Pair voice: I/my -> we/our; name written as the pair.
- AMY & LIV / General3 (PositiveBattleText2): Pair voice: I/my -> we/our; name written as the pair.
- AMY & LIV / Battle (DifferentRouteBattleRequestText2): Pair voice: I/my/me -> we/our/us, pair named in greeting.
- ERNEST / General1 (PersonalizedText49): 'I'm not on a boat now' -> 'sometimes'; avoids stating his current location.
- CORY / General1 (PersonalizedText63): 'I'm out on ROUTE 108 now! … where I always am!' -> 'always out on' (no present-location claim).
- LYDIA / General1 (PersonalizedText52): {STR_VAR_2} -> {STR_VAR_3}; POKéBLOCK -> 'a treat'.
- ISAAC / General1 (PersonalizedText51): {STR_VAR_2} (current map) -> ROUTE 117, his home, since Pokégear chats must not state where he is today.
- HALEY / General1 (PersonalizedText55): {STR_VAR_2} (current map) -> ROUTE 104 (home); 'Today I won…' -> 'Some days' so it makes no claim about today.
- SAWYER / General1 (PersonalizedText1): Dropped the {STR_VAR_2} 'we'll meet around <map>' tail, which tied the chat to wherever he is now.
- KIRA & DAN / General1 (PersonalizedText58): Pair voice: KIRA's solo chat shared between the pair.
- KIRA & DAN / General2 (WildBattleText9): Pair voice: I/my -> we/our; name written as the pair.
- KIRA & DAN / General3 (PositiveBattleText9): Pair voice: I/my -> we/our; name written as the pair. {STR_VAR_2} -> {STR_VAR_3}.
- KIRA & DAN / Battle (DifferentRouteBattleRequestText9): Pair voice: I/my/me -> we/our/us, pair named in greeting.

Pair-voice texts (also listed above where they carry a reason): every General2, General3 and Battle text for LILA & ROY, JOHN & JAY, ANNA & MEG, AMY & LIV and KIRA & DAN was rewritten from I/my/me to we/our/us with the pair named in the greeting.

Not reworded but worth knowing: no Emerald chat in the selected sets refers to Team Magma/Aqua or to gym order; ROXANNE (Karen, Jerry), BRAWLY (Koji) and the TRAINER'S SCHOOL / DEWFORD GYM are plain place/NPC mentions and were kept; SAFARI ZONE (Jessica), CYCLING ROAD (Abigail), EVERGRANDE (Isaiah) and the FAN CLUB (Miguel) are Hoenn place names kept as written.


### Text

```asm
@ ROSE - Aroma Lady, Route 118

MatchCall_HOENN_Rose_General1::
	.string "Hello! It's ROSE.\p"
	.string "I love ROUTE 118. It's\n"
	.string "pleasant with sweet aromas! I\p"
	.string "think someone planted BERRIES,\n"
	.string "and they burst into bloom. See\p"
	.string "you again sometime!$"

MatchCall_HOENN_Rose_General2::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is ROSE speaking.\p"
	.string "How are your POKéMON doing? I\n"
	.string "lost a match the other day. I\p"
	.string "need to try harder! See you\n"
	.string "again!$"

MatchCall_HOENN_Rose_General3::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is ROSE speaking.\p"
	.string "I hope you've been well. I\n"
	.string "wanted to tell you I just won.\p"
	.string "My {STR_VAR_3} worked\n"
	.string "especially hard to get the\p"
	.string "win. See you again!$"

MatchCall_HOENN_Rose_Battle::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is ROSE speaking.\p"
	.string "I hope you're doing well. My\n"
	.string "POKéMON are very frisky. If\p"
	.string "you're ever in the area,\n"
	.string "please give me a rematch. I'll\p"
	.string "be around {STR_VAR_2}.\n"
	.string "Until then, good-bye!$"
```

```asm
@ ANDRES - Ruin Maniac, Route 105

MatchCall_HOENN_Andres_General1::
	.string "ANDRES here, yes.\n"
	.string "I headed out to sea yesterday.\p"
	.string "I had been hoping to find a\n"
	.string "new ruin to explore. But the\p"
	.string "tides somehow seemed to carry\n"
	.string "me back where I started. I'm\p"
	.string "still weak at battling, too…\n"
	.string "Feel free to mock me… But I\p"
	.string "won't give up. My day will\n"
	.string "come when I discover a new\p"
	.string "ruin! That's all I have to\n"
	.string "say! Farewell for now!$"

MatchCall_HOENN_Andres_General2::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, ANDRES.\p"
	.string "Are you still battling hard?\n"
	.string "As for me, I lost recently, so\p"
	.string "I've been training my team all\n"
	.string "over. Let's meet again.$"

MatchCall_HOENN_Andres_General3::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, ANDRES.\p"
	.string "I trust you've been well? I'm\n"
	.string "still bursting with life! Why,\p"
	.string "just now, I won another match.\n"
	.string "I'm not stepping aside to you\p"
	.string "youngsters yet!$"

MatchCall_HOENN_Andres_Battle::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, ANDRES.\p"
	.string "I should tell you, my POKéMON\n"
	.string "have grown to be quite robust\p"
	.string "lately. I would like to see\n"
	.string "them in a battle with you,\p"
	.string "{PLAYER}. We'll be around\n"
	.string "{STR_VAR_2}. Come see us\p"
	.string "anytime!$"
```

```asm
@ DUSTY - Ruin Maniac, Route 111

MatchCall_HOENN_Dusty_General1::
	.string "Hello! Thirty years of\n"
	.string "exploration, DUSTY at your\p"
	.string "service! It seems that you're\n"
	.string "energetically traveling here\p"
	.string "and there. Have you discovered\n"
	.string "any new ruins? Please tell if\p"
	.string "you have! Now, if you'll\n"
	.string "excuse me, I have ruins to\p"
	.string "explore.$"

MatchCall_HOENN_Dusty_General2::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, DUSTY.\p"
	.string "Are you still catching\n"
	.string "POKéMON? I've been trying to\p"
	.string "catch them myself, but it's\n"
	.string "not so easy. The way of\p"
	.string "POKéMON is deep!$"

MatchCall_HOENN_Dusty_General3::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, DUSTY.\p"
	.string "Are you still battling hard?\n"
	.string "As for me, I lost recently, so\p"
	.string "I've been training my team all\n"
	.string "over. Let's meet again.$"

MatchCall_HOENN_Dusty_Battle::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, DUSTY.\p"
	.string "I should tell you, my POKéMON\n"
	.string "have grown to be quite robust\p"
	.string "lately. I would like to see\n"
	.string "them in a battle with you,\p"
	.string "{PLAYER}. We'll be around\n"
	.string "{STR_VAR_2}. Come see us\p"
	.string "anytime!$"
```

```asm
@ LOLA - Tuber, Route 109

MatchCall_HOENN_Lola_General1::
	.string "It's LOLA!\p"
	.string "Know what I love doing?\n"
	.string "Looking at the waves from the\p"
	.string "beach! Sigh… The waves are all\n"
	.string "sparkly. The sea is the\p"
	.string "prettiest from the beach. I'm\n"
	.string "getting hungry, so bye-bye!$"

MatchCall_HOENN_Lola_General2::
	.string "Hello, {PLAYER}.\n"
	.string "It's LOLA.\p"
	.string "I challenged someone else\n"
	.string "after we battled. I came\p"
	.string "close, but I ended up losing.\n"
	.string "Oh, well!$"

MatchCall_HOENN_Lola_General3::
	.string "Hello, {PLAYER}!\n"
	.string "It's LOLA!\p"
	.string "I had a battle yesterday and I\n"
	.string "won! It's fantastic!$"

MatchCall_HOENN_Lola_Battle::
	.string "Hello, {PLAYER}!\n"
	.string "It's LOLA.\p"
	.string "Would you like to have a\n"
	.string "battle with me again? You can\p"
	.string "find me around\n"
	.string "{STR_VAR_2}. I'll be\p"
	.string "waiting!$"
```

```asm
@ RICKY - Tuber, Route 109

MatchCall_HOENN_Ricky_General1::
	.string "Munch-chew…\n"
	.string "Oh, hi, it's RICKY.\p"
	.string "I love eating on the beach. My\n"
	.string "POKéMON and I have been doing\p"
	.string "great. We're fully fueled! I'm\n"
	.string "going for a swim. Bye!$"

MatchCall_HOENN_Ricky_General2::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is RICKY.\p"
	.string "I tried battling another\n"
	.string "TRAINER, but I lost. It was\p"
	.string "really disappointing. Well,\n"
	.string "see you again!$"

MatchCall_HOENN_Ricky_General3::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is RICKY!\p"
	.string "I battled another TRAINER\n"
	.string "earlier. I won! I won! My\p"
	.string "{STR_VAR_3} really worked hard\n"
	.string "for me. This is so great!$"

MatchCall_HOENN_Ricky_Battle::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is RICKY.\p"
	.string "Want to have a battle with me?\n"
	.string "I'll be waiting for you around\p"
	.string "{STR_VAR_2}!$"
```

```asm
@ LILA & ROY - Sis and Bro, Route 124

MatchCall_HOENN_LilaAndRoy_General1::
	.string "Hi, this is LILA & ROY! We\n"
	.string "just won a battle! We don't\p"
	.string "win often, but it was this\n"
	.string "cool SWIMMER guy. Then we\p"
	.string "started bickering! LILA wanted\n"
	.string "to pretend we were weak to\p"
	.string "make an impression! And ROY\n"
	.string "complains whether we win or\p"
	.string "lose! {PLAYER}, can you say\n"
	.string "something to us next time?\p"
	.string "Okay, see you!$"

MatchCall_HOENN_LilaAndRoy_General2::
	.string "Hi! {PLAYER}, hello!\p"
	.string "This is LILA & ROY. We tried\n"
	.string "battling another TRAINER, but\p"
	.string "we lost. It was really\n"
	.string "disappointing. Well, see you\p"
	.string "again!$"

MatchCall_HOENN_LilaAndRoy_General3::
	.string "Hi! {PLAYER}, hello!\p"
	.string "This is LILA & ROY! We battled\n"
	.string "another TRAINER earlier. We\p"
	.string "won! We won! Our {STR_VAR_3}\n"
	.string "really worked hard for us.\p"
	.string "This is so great!$"

MatchCall_HOENN_LilaAndRoy_Battle::
	.string "Hi! {PLAYER}, hello!\p"
	.string "This is LILA & ROY. Want to\n"
	.string "have a battle with us? We'll\p"
	.string "be waiting for you around\n"
	.string "{STR_VAR_2}!$"
```

```asm
@ CRISTIN - Cooltrainer, Route 121

MatchCall_HOENN_Cristin_General1::
	.string "It's CRISTIN!\p"
	.string "I'm kind of busy, but I\n"
	.string "figured I should let you know\p"
	.string "that I've beaten five TRAINERS\n"
	.string "again today. If I keep this\p"
	.string "pace up, I can probably beat\n"
	.string "you next time. I think we'll\p"
	.string "be good rivals, you and I.\n"
	.string "Good-bye for now!$"

MatchCall_HOENN_Cristin_General2::
	.string "Oh, {PLAYER}, hello…\n"
	.string "This is CRISTIN.\p"
	.string "A little earlier, I was in a\n"
	.string "battle. I lost, though. I need\p"
	.string "to raise my POKéMON more. See\n"
	.string "you around.$"

MatchCall_HOENN_Cristin_General3::
	.string "Oh, {PLAYER}, hello…\n"
	.string "This is CRISTIN.\p"
	.string "How has life been treating\n"
	.string "you? My POKéMON appear to be\p"
	.string "charged with energy. I just\n"
	.string "won a battle with them. See\p"
	.string "you around.$"

MatchCall_HOENN_Cristin_Battle::
	.string "Oh, {PLAYER}, hello…\n"
	.string "This is CRISTIN.\p"
	.string "So, how are things with you?\n"
	.string "My POKéMON have grown much\p"
	.string "stronger than before. I'd love\n"
	.string "another battle with you,\p"
	.string "{PLAYER}. I'll be around\n"
	.string "{STR_VAR_2}. Come see me\p"
	.string "if you're close.$"
```

```asm
@ BROOKE - Cooltrainer, Route 111

MatchCall_HOENN_Brooke_General1::
	.string "Yahoo, it's BROOKE!\n"
	.string "How do you do?\p"
	.string "I've been raising my POKéMON\n"
	.string "with you as the target. I\p"
	.string "don't intend to lose when we\n"
	.string "battle again. Isn't it great\p"
	.string "to have TRAINER friends? Let's\n"
	.string "meet again!$"

MatchCall_HOENN_Brooke_General2::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is BROOKE!\p"
	.string "Listen, listen, you have to\n"
	.string "hear this! I had a POKéMON\p"
	.string "battle earlier, but I lost at\n"
	.string "the last second. Oh, it burns\p"
	.string "me up!$"

MatchCall_HOENN_Brooke_General3::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is BROOKE!\p"
	.string "How are your POKéMON holding\n"
	.string "up? Mine just won a battle! My\p"
	.string "{STR_VAR_3} was spectacular, I\n"
	.string "must say! I wish I could've\p"
	.string "shown you! See you again!$"

MatchCall_HOENN_Brooke_Battle::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is BROOKE!\p"
	.string "How are your POKéMON doing? My\n"
	.string "POKéMON keep getting better.\p"
	.string "I'd like to show you, {PLAYER}.\n"
	.string "I'm around {STR_VAR_2}\p"
	.string "now, so let's battle if you're\n"
	.string "close by. Hope I see you soon!$"
```

```asm
@ WILTON - Cooltrainer, Route 111

MatchCall_HOENN_Wilton_General1::
	.string "Hello, this is WILTON…\p"
	.string "I've grown a little jaded with\n"
	.string "this whole COOLTRAINER thing…\p"
	.string "Everyone thinks I'm a perfect\n"
	.string "TRAINER, and that makes me try\p"
	.string "to live up to that\n"
	.string "expectation. I'll just have to\p"
	.string "buckle down… and grin and bear\n"
	.string "it. You're the only person I\p"
	.string "could confide in like this.\n"
	.string "But when I see you next, don't\p"
	.string "worry, I won't whine!$"

MatchCall_HOENN_Wilton_General2::
	.string "Hey, {PLAYER}.\n"
	.string "WILTON here.\p"
	.string "How's it going for you? I've\n"
	.string "been battling hard lately, but\p"
	.string "to little success. I can't get\n"
	.string "into the groove. You take\p"
	.string "care.$"

MatchCall_HOENN_Wilton_General3::
	.string "Hey, {PLAYER}.\n"
	.string "WILTON here.\p"
	.string "How's it going for you? I've\n"
	.string "been riding a hot streak. Why,\p"
	.string "I just won a battle. When we\n"
	.string "have our next battle, I'm sure\p"
	.string "not going to lose!$"

MatchCall_HOENN_Wilton_Battle::
	.string "Hey, {PLAYER}.\n"
	.string "WILTON here.\p"
	.string "I hope you're on top of\n"
	.string "things. I was thinking I'd\p"
	.string "like another battle with you.\n"
	.string "What do you say? If you feel\p"
	.string "like a battle, come to\n"
	.string "{STR_VAR_2}. See you!$"
```

```asm
@ VALERIE - Hex Maniac, Mt. Pyre

MatchCall_HOENN_Valerie_General1::
	.string "It's VALERIE… Right now,\n"
	.string "behind you… Wasn't there\p"
	.string "something…? The power of the\n"
	.string "POKéMON that sleep at MT.\p"
	.string "PYRE… It's telling me about\n"
	.string "you… You should walk away\p"
	.string "quickly and never once look\n"
	.string "back… Giggle… Farewell…$"

MatchCall_HOENN_Valerie_General2::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is VALERIE speaking.\p"
	.string "Have you had success catching\n"
	.string "POKéMON lately? I came very\p"
	.string "close a little while ago, but\n"
	.string "my target got free. I need to\p"
	.string "try harder! See you again!$"

MatchCall_HOENN_Valerie_General3::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is VALERIE speaking.\p"
	.string "How are your POKéMON doing? I\n"
	.string "lost a match the other day. I\p"
	.string "need to try harder! See you\n"
	.string "again!$"

MatchCall_HOENN_Valerie_Battle::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is VALERIE speaking.\p"
	.string "I hope you're doing well. My\n"
	.string "POKéMON are very frisky. If\p"
	.string "you're ever in the area,\n"
	.string "please give me a rematch. I'll\p"
	.string "be around {STR_VAR_2}.\n"
	.string "Until then, good-bye!$"
```

```asm
@ CINDY - Lady, Route 104

MatchCall_HOENN_Cindy_General1::
	.string "This is CINDY.\n"
	.string "How do you do?\p"
	.string "Isn't it convenient that we\n"
	.string "can chat like this at a\p"
	.string "distance? Before, if I wanted\n"
	.string "to speak with anyone, I had to\p"
	.string "have my father drive me… I\n"
	.string "should be going now. I'm glad\p"
	.string "we had this chat.$"

MatchCall_HOENN_Cindy_General2::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is CINDY speaking.\p"
	.string "Have you had success catching\n"
	.string "POKéMON lately? I came very\p"
	.string "close a little while ago, but\n"
	.string "my target got free. I need to\p"
	.string "try harder! See you again!$"

MatchCall_HOENN_Cindy_General3::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is CINDY speaking.\p"
	.string "I hope you've been well. I\n"
	.string "wanted to tell you I just won.\p"
	.string "My {STR_VAR_3} worked\n"
	.string "especially hard to get the\p"
	.string "win. See you again!$"

MatchCall_HOENN_Cindy_Battle::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is CINDY speaking.\p"
	.string "I hope you're doing well. My\n"
	.string "POKéMON are very frisky. If\p"
	.string "you're ever in the area,\n"
	.string "please give me a rematch. I'll\p"
	.string "be around {STR_VAR_2}.\n"
	.string "Until then, good-bye!$"
```

```asm
@ THALIA - Beauty, Abandoned Ship

MatchCall_HOENN_Thalia_General1::
	.string "Oh, it's THALIA!\p"
	.string "I get bored of the ABANDONED\n"
	.string "SHIP sometimes. But I keep\p"
	.string "going back, because I want to\n"
	.string "beat you once. If you feel\p"
	.string "compelled, why don't you come\n"
	.string "see me? I think today will be\p"
	.string "the day I finally challenge\n"
	.string "the man next door to a match.\p"
	.string "Be seeing you!$"

MatchCall_HOENN_Thalia_General2::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is THALIA speaking.\p"
	.string "Have you had success catching\n"
	.string "POKéMON lately? I came very\p"
	.string "close a little while ago, but\n"
	.string "my target got free. I need to\p"
	.string "try harder! See you again!$"

MatchCall_HOENN_Thalia_General3::
	.string "Oh, {PLAYER}, hello…\n"
	.string "This is THALIA.\p"
	.string "How has life been treating\n"
	.string "you? My POKéMON appear to be\p"
	.string "charged with energy. I just\n"
	.string "won a battle with them. See\p"
	.string "you around.$"

MatchCall_HOENN_Thalia_Battle::
	.string "Oh, {PLAYER}, hello…\n"
	.string "This is THALIA.\p"
	.string "So, how are things with you?\n"
	.string "My POKéMON have grown much\p"
	.string "stronger than before. I'd love\n"
	.string "another battle with you,\p"
	.string "{PLAYER}. I'll be around\n"
	.string "{STR_VAR_2}. Come see me\p"
	.string "if you're close.$"
```

```asm
@ JESSICA - Beauty, Route 121

MatchCall_HOENN_Jessica_General1::
	.string "It's JESSICA!\n"
	.string "Will you listen to this?\p"
	.string "I like the SAFARI ZONE a lot,\n"
	.string "but whenever I go, I get lost!\p"
	.string "All that tall grass! And it's\n"
	.string "much too spread out! I feel\p"
	.string "better getting that off my\n"
	.string "chest! I'm off to the SAFARI\p"
	.string "ZONE again! Catch you!$"

MatchCall_HOENN_Jessica_General2::
	.string "Oh, {PLAYER}, hello…\n"
	.string "This is JESSICA.\p"
	.string "Listen, I came within a\n"
	.string "whisker of catching this\p"
	.string "{STR_VAR_3}… But, it gave me\n"
	.string "the slip… I need to try\p"
	.string "harder. See you around.$"

MatchCall_HOENN_Jessica_General3::
	.string "Oh, {PLAYER}, hello…\n"
	.string "This is JESSICA.\p"
	.string "A little earlier, I was in a\n"
	.string "battle. I lost, though. I need\p"
	.string "to raise my POKéMON more. See\n"
	.string "you around.$"

MatchCall_HOENN_Jessica_Battle::
	.string "Oh, {PLAYER}, hello…\n"
	.string "This is JESSICA.\p"
	.string "So, how are things with you?\n"
	.string "My POKéMON have grown much\p"
	.string "stronger than before. I'd love\n"
	.string "another battle with you,\p"
	.string "{PLAYER}. I'll be around\n"
	.string "{STR_VAR_2}. Come see me\p"
	.string "if you're close.$"
```

```asm
@ WINSTON - Rich Boy, Route 104

MatchCall_HOENN_Winston_General1::
	.string "Hello, WINSTON here. Yes,\n"
	.string "correct, I am rich, yes. I\p"
	.string "should tell you, my wealth has\n"
	.string "grown since we last met. I\p"
	.string "can't shake the feeling that\n"
	.string "this world exists for me! Oh,\p"
	.string "no need to say a word!\n"
	.string "Everyone knows it's true! Oh,\p"
	.string "you must excuse me, I have\n"
	.string "this formal dinner to attend.$"

MatchCall_HOENN_Winston_General2::
	.string "Hey, {PLAYER}.\n"
	.string "WINSTON here.\p"
	.string "I tried another battle\n"
	.string "yesterday, but I couldn't pull\p"
	.string "out the win. My team needs\n"
	.string "more raising. Okay, catch you\p"
	.string "later.$"

MatchCall_HOENN_Winston_General3::
	.string "Hey, {PLAYER}.\n"
	.string "WINSTON here.\p"
	.string "I had a match earlier. I\n"
	.string "managed to win, but it was\p"
	.string "close. My {STR_VAR_3} put on\n"
	.string "one inspired showing.$"

MatchCall_HOENN_Winston_Battle::
	.string "Hey, {PLAYER}.\n"
	.string "WINSTON here.\p"
	.string "How are things with you? My\n"
	.string "POKéMON have grown pretty\p"
	.string "tough lately. Hey, how would\n"
	.string "you like to have another\p"
	.string "battle with me? Let's meet up\n"
	.string "around {STR_VAR_2}, okay?$"
```

```asm
@ STEVE - Pokémaniac, Route 114

MatchCall_HOENN_Steve_General1::
	.string "Ufufufufu…\p"
	.string "It's me, STEVE… Can you guess\n"
	.string "what I'm seeing? A pair of\p"
	.string "{STR_VAR_3} in a battle. Maybe\n"
	.string "I'll try catching the winner…\p"
	.string "Ufufufufufu… I… I'm kind of\n"
	.string "busy now. I have to go.$"

MatchCall_HOENN_Steve_General2::
	.string "STEVE here.\n"
	.string "How's it going lately?\p"
	.string "I lost a battle yesterday, and\n"
	.string "it's filled my thoughts. I\p"
	.string "have to devise a plan… See\n"
	.string "you.$"

MatchCall_HOENN_Steve_General3::
	.string "{PLAYER}?\n"
	.string "STEVE here.\p"
	.string "My {STR_VAR_3} is a force! It\n"
	.string "won me another battle just\p"
	.string "now! I can't wait to have a\n"
	.string "rematch with you.$"

MatchCall_HOENN_Steve_Battle::
	.string "…Er, {PLAYER}?\n"
	.string "STEVE here…\p"
	.string "So? Are your POKéMON growing?\n"
	.string "Mine sure got stronger. I'd\p"
	.string "like to show you. I'll be\n"
	.string "around {STR_VAR_2}. Come\p"
	.string "see me for a match. See you\n"
	.string "around.$"
```

```asm
@ TONY - Swimmer, Route 107

MatchCall_HOENN_Tony_General1::
	.string "I'm TONY! The man of the sea!\p"
	.string "You know what I think? The\n"
	.string "TRAINERS out at sea are the\p"
	.string "toughest of the tough! You\n"
	.string "should learn from me and train\p"
	.string "in the sea… Whenever a giant\n"
	.string "surf rises, it's a great\p"
	.string "training opportunity! Sorry,\n"
	.string "but I have to go!$"

MatchCall_HOENN_Tony_General2::
	.string "Hiya, {PLAYER}!\n"
	.string "It's TONY.\p"
	.string "How are things with you? I've\n"
	.string "been battling on, but I\p"
	.string "haven't won very often. I\n"
	.string "can't get it together. Right,\p"
	.string "take care!$"

MatchCall_HOENN_Tony_General3::
	.string "Hiya, {PLAYER}!\n"
	.string "It's TONY.\p"
	.string "How are things with you?\n"
	.string "Battling much? I just won a\p"
	.string "while back! My {STR_VAR_3} was\n"
	.string "brilliant! You wait. I'm going\p"
	.string "to beat you next time! Right,\n"
	.string "take care!$"

MatchCall_HOENN_Tony_Battle::
	.string "Hiya, {PLAYER}!\n"
	.string "It's TONY.\p"
	.string "My POKéMON are growing up in\n"
	.string "decent ways. I'd really like\p"
	.string "to have another battle with\n"
	.string "you. I'll keep an eye out for\p"
	.string "you around {STR_VAR_2}.\n"
	.string "See you soon!$"
```

```asm
@ NOB - Black Belt, Route 115

MatchCall_HOENN_Nob_General1::
	.string "It's NOB! Listen, I've been\n"
	.string "teaching karate to my POKéMON.\p"
	.string "But now they're better than\n"
	.string "me! I've done nothing but lose\p"
	.string "to them! But even though I may\n"
	.string "lose to POKéMON, I won't lose\p"
	.string "to another TRAINER, no sir! We\n"
	.string "have to battle again! Ugwaah!$"

MatchCall_HOENN_Nob_General2::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, NOB.\p"
	.string "I just got cleaned in a\n"
	.string "battle. I guess I need to\p"
	.string "raise my team some more!$"

MatchCall_HOENN_Nob_General3::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, NOB!\p"
	.string "How's your battling? Me, I had\n"
	.string "a battle the other day, and my\p"
	.string "{STR_VAR_3} came up huge! The\n"
	.string "next time I battle you,\p"
	.string "{PLAYER}, it won't be me\n"
	.string "losing!$"

MatchCall_HOENN_Nob_Battle::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, NOB.\p"
	.string "My POKéMON have grown a lot\n"
	.string "tougher since last time. I\p"
	.string "want to see how strong they've\n"
	.string "become with your POKéMON,\p"
	.string "{PLAYER}. So, let's have a\n"
	.string "battle! I'll be waiting for\p"
	.string "you around {STR_VAR_2}.$"
```

```asm
@ KOJI - Black Belt, Route 127

MatchCall_HOENN_Koji_General1::
	.string "This is KOJI!\p"
	.string "I went to DEWFORD's GYM again\n"
	.string "for training. BRAWLY, the GYM\p"
	.string "LEADER, seems to be tougher\n"
	.string "now. He's still as cool as\p"
	.string "ever. The ladies adore him! It\n"
	.string "makes me envious, frankly.\p"
	.string "But, hey, this jealousy thing\n"
	.string "isn't very seemly, is it?\p"
	.string "Forget this chat ever\n"
	.string "happened, how about it? So\p"
	.string "long!$"

MatchCall_HOENN_Koji_General2::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, KOJI.\p"
	.string "I just took a shot at catching\n"
	.string "this {STR_VAR_3}, but it took\p"
	.string "off. I came oh so close, too!\n"
	.string "It spoiled my day… All right,\p"
	.string "see you!$"

MatchCall_HOENN_Koji_General3::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, KOJI.\p"
	.string "I just got cleaned in a\n"
	.string "battle. I guess I need to\p"
	.string "raise my team some more!$"

MatchCall_HOENN_Koji_Battle::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, KOJI.\p"
	.string "My POKéMON have grown a lot\n"
	.string "tougher since last time. I\p"
	.string "want to see how strong they've\n"
	.string "become with your POKéMON,\p"
	.string "{PLAYER}. So, let's have a\n"
	.string "battle! I'll be waiting for\p"
	.string "you around {STR_VAR_2}.$"
```

```asm
@ FERNANDO - Guitarist, Route 123

MatchCall_HOENN_Fernando_General1::
	.string "It's me, FERNANDO.\n"
	.string "How're your travels unwinding?\p"
	.string "…Whoa, is that right? Sounds\n"
	.string "awfully stimulating! I think I\p"
	.string "could write a song about one\n"
	.string "of your episodes. …Oh, hey,\p"
	.string "I'm feeling it. I hear the\n"
	.string "riffs in my head. I'd better\p"
	.string "get this tune properly\n"
	.string "written, so I've got to fly!\p"
	.string "Later!$"

MatchCall_HOENN_Fernando_General2::
	.string "Hey, {PLAYER}.\n"
	.string "FERNANDO here.\p"
	.string "Caught any POKéMON lately? I\n"
	.string "nearly nabbed one the other\p"
	.string "day. But it evaded me somehow.\n"
	.string "You take care.$"

MatchCall_HOENN_Fernando_General3::
	.string "Hey, {PLAYER}.\n"
	.string "FERNANDO here.\p"
	.string "How's it going for you? I've\n"
	.string "been battling hard lately, but\p"
	.string "to little success. I can't get\n"
	.string "into the groove. You take\p"
	.string "care.$"

MatchCall_HOENN_Fernando_Battle::
	.string "Hey, {PLAYER}.\n"
	.string "FERNANDO here.\p"
	.string "I hope you're on top of\n"
	.string "things. I was thinking I'd\p"
	.string "like another battle with you.\n"
	.string "What do you say? If you feel\p"
	.string "like a battle, come to\n"
	.string "{STR_VAR_2}. See you!$"
```

```asm
@ DALTON - Guitarist, Route 118

MatchCall_HOENN_Dalton_General1::
	.string "This is DALTON… Hear my new\n"
	.string "song. Lalala, {STR_VAR_3},\p"
	.string "{STR_VAR_3}! Why are you that\n"
	.string "{STR_VAR_3}? Why can't I be\p"
	.string "you, {STR_VAR_3}? Lala,\n"
	.string "{STR_VAR_3} and DALTON, DALTON\p"
	.string "and {STR_VAR_3}… Repeat chorus,\n"
	.string "fade…$"

MatchCall_HOENN_Dalton_General2::
	.string "Hey, {PLAYER}.\n"
	.string "DALTON here.\p"
	.string "You know the POKéMON\n"
	.string "{STR_VAR_3}? I came close to\p"
	.string "getting one. It was just a\n"
	.string "while back. I thought I had it\p"
	.string "but it escaped. If I see it\n"
	.string "again, I'll get it for sure,\p"
	.string "though. Okay, catch you later.$"

MatchCall_HOENN_Dalton_General3::
	.string "Hey, {PLAYER}.\n"
	.string "DALTON here.\p"
	.string "I tried another battle\n"
	.string "yesterday, but I couldn't pull\p"
	.string "out the win. My team needs\n"
	.string "more raising. Okay, catch you\p"
	.string "later.$"

MatchCall_HOENN_Dalton_Battle::
	.string "Hey, {PLAYER}.\n"
	.string "DALTON here.\p"
	.string "How are things with you? My\n"
	.string "POKéMON have grown pretty\p"
	.string "tough lately. Hey, how would\n"
	.string "you like to have another\p"
	.string "battle with me? Let's meet up\n"
	.string "around {STR_VAR_2}, okay?$"
```

```asm
@ BERNIE - Kindler, Route 114

MatchCall_HOENN_Bernie_General1::
	.string "I'm BERNIE, you know,\n"
	.string "the camping expert!\p"
	.string "When we battled, I couldn't\n"
	.string "help but lose to you. After\p"
	.string "all, my expertise is in\n"
	.string "camping. But win or lose, I\p"
	.string "like to battle when I'm\n"
	.string "camping. Battle with us again,\p"
	.string "okay? Oh, and let's go\n"
	.string "camping, too!$"

MatchCall_HOENN_Bernie_General2::
	.string "Ah, {PLAYER}.\n"
	.string "This is BERNIE.\p"
	.string "How are your POKéMON? I just\n"
	.string "lost yet another battle. Well,\p"
	.string "see you!$"

MatchCall_HOENN_Bernie_General3::
	.string "Ah, {PLAYER}.\n"
	.string "This is BERNIE.\p"
	.string "Been in any battles lately? I\n"
	.string "just won another one today!\p"
	.string "I'm on a roll! Gahahaha! Well,\n"
	.string "see you!$"

MatchCall_HOENN_Bernie_Battle::
	.string "Ah, {PLAYER}.\n"
	.string "This is BERNIE.\p"
	.string "Where might you be now? My\n"
	.string "POKéMON are full of life. They\p"
	.string "appear to be looking forward\n"
	.string "to seeing your POKéMON,\p"
	.string "{PLAYER}. I'm around\n"
	.string "{STR_VAR_2} now. I hope\p"
	.string "you'll seek us out.$"
```

```asm
@ ETHAN - Camper, Jagged Pass

MatchCall_HOENN_Ethan_General1::
	.string "It's me, me, ETHAN!\p"
	.string "I'd like to climb other\n"
	.string "mountains than JAGGED PASS, to\p"
	.string "be honest. But I'm not sure if\n"
	.string "there'd be any ladies like on\p"
	.string "MT. CHIMNEY. If you know any\n"
	.string "other mountain with ladies\p"
	.string "around, let me know! Ehehehe,\n"
	.string "see you around!$"

MatchCall_HOENN_Ethan_General2::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is ETHAN.\p"
	.string "I saw this {STR_VAR_3} a while\n"
	.string "back but I couldn't catch it.\p"
	.string "It was so close, too! Well,\n"
	.string "see you again!$"

MatchCall_HOENN_Ethan_General3::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is ETHAN.\p"
	.string "I tried battling another\n"
	.string "TRAINER, but I lost. It was\p"
	.string "really disappointing. Well,\n"
	.string "see you again!$"

MatchCall_HOENN_Ethan_Battle::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is ETHAN.\p"
	.string "Want to have a battle with me?\n"
	.string "I'll be waiting for you around\p"
	.string "{STR_VAR_2}!$"
```

```asm
@ JOHN & JAY - Old Couple, Meteor Falls

MatchCall_HOENN_JohnAndJay_General1::
	.string "This is JOHN & JAY.\p"
	.string "It's a pleasure to chat with a\n"
	.string "young TRAINER like you. We\p"
	.string "imagine you'll continue to\n"
	.string "enjoy POKéMON whatever your\p"
	.string "age. Wouldn't it be good if\n"
	.string "you had a partnership like\p"
	.string "ours? Of course, {PLAYER}, you\n"
	.string "already enjoy the trust and\p"
	.string "companionship of your POKéMON.\n"
	.string "Hahaha! Never be discouraged!$"

MatchCall_HOENN_JohnAndJay_General2::
	.string "Hello, {PLAYER}.\p"
	.string "It's JOHN & JAY. Are you still\n"
	.string "catching POKéMON? We've been\p"
	.string "trying to catch them\n"
	.string "ourselves, but it's not so\p"
	.string "easy. The way of POKéMON is\n"
	.string "deep!$"

MatchCall_HOENN_JohnAndJay_General3::
	.string "Hello, {PLAYER}.\p"
	.string "It's JOHN & JAY. We trust\n"
	.string "you've been well? We're still\p"
	.string "bursting with life! Why, just\n"
	.string "now, we won another match.\p"
	.string "We're not stepping aside to\n"
	.string "you youngsters yet!$"

MatchCall_HOENN_JohnAndJay_Battle::
	.string "Hello, {PLAYER}.\p"
	.string "It's JOHN & JAY. We should\n"
	.string "tell you, our POKéMON have\p"
	.string "grown to be quite robust\n"
	.string "lately. We would like to see\p"
	.string "them in a battle with you,\n"
	.string "{PLAYER}. We'll be around\p"
	.string "{STR_VAR_2}. Come see us\n"
	.string "anytime!$"
```

```asm
@ JEFFREY - Bug Maniac, Route 120

MatchCall_HOENN_Jeffrey_General1::
	.string "… … … … … …\n"
	.string "… … … … … …\p"
	.string "It's JEFFREY… … … … … … … … …\n"
	.string "… … … … That's all today…$"

MatchCall_HOENN_Jeffrey_General2::
	.string "…Uh, {PLAYER}?\n"
	.string "It's me, JEFFREY.\p"
	.string "Oh, wait! Wait! I can catch\n"
	.string "this {STR_VAR_3}… Aaarrrgh! It\p"
	.string "bolted loose! That wasn't just\n"
	.string "close!$"

MatchCall_HOENN_Jeffrey_General3::
	.string "JEFFREY here.\n"
	.string "How's it going lately?\p"
	.string "I lost a battle yesterday, and\n"
	.string "it's filled my thoughts. I\p"
	.string "have to devise a plan… See\n"
	.string "you.$"

MatchCall_HOENN_Jeffrey_Battle::
	.string "…Er, {PLAYER}?\n"
	.string "JEFFREY here…\p"
	.string "So? Are your POKéMON growing?\n"
	.string "Mine sure got stronger. I'd\p"
	.string "like to show you. I'll be\n"
	.string "around {STR_VAR_2}. Come\p"
	.string "see me for a match. See you\n"
	.string "around.$"
```

```asm
@ CAMERON - Psychic, Route 123

MatchCall_HOENN_Cameron_General1::
	.string "This is CAMERON. Today, I had\n"
	.string "this feeling I would chat with\p"
	.string "you. My desire to defeat you\n"
	.string "builds by day and by night.\p"
	.string "You have a rival like that,\n"
	.string "yes? I wish it were me… I'm\p"
	.string "glad you heard me out. See\n"
	.string "you!$"

MatchCall_HOENN_Cameron_General2::
	.string "Hey, {PLAYER}.\n"
	.string "CAMERON here.\p"
	.string "You know the POKéMON\n"
	.string "{STR_VAR_3}? I came close to\p"
	.string "getting one. It was just a\n"
	.string "while back. I thought I had it\p"
	.string "but it escaped. If I see it\n"
	.string "again, I'll get it for sure,\p"
	.string "though. Okay, catch you later.$"

MatchCall_HOENN_Cameron_General3::
	.string "Hey, {PLAYER}.\n"
	.string "CAMERON here.\p"
	.string "I had a match earlier. I\n"
	.string "managed to win, but it was\p"
	.string "close. My {STR_VAR_3} put on\n"
	.string "one inspired showing.$"

MatchCall_HOENN_Cameron_Battle::
	.string "Hey, {PLAYER}.\n"
	.string "CAMERON here.\p"
	.string "How are things with you? My\n"
	.string "POKéMON have grown pretty\p"
	.string "tough lately. Hey, how would\n"
	.string "you like to have another\p"
	.string "battle with me? Let's meet up\n"
	.string "around {STR_VAR_2}, okay?$"
```

```asm
@ JACKI - Psychic, Route 123

MatchCall_HOENN_Jacki_General1::
	.string "It's JACKI.\p"
	.string "When there's a strong TRAINER\n"
	.string "nearby, I can sometimes sense\p"
	.string "that somehow. Did you pass\n"
	.string "close by, {PLAYER}? Maybe it\p"
	.string "was you. I'll be waiting for\n"
	.string "your visit. Bye!$"

MatchCall_HOENN_Jacki_General2::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is JACKI speaking.\p"
	.string "How are your POKéMON doing? I\n"
	.string "lost a match the other day. I\p"
	.string "need to try harder! See you\n"
	.string "again!$"

MatchCall_HOENN_Jacki_General3::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is JACKI speaking.\p"
	.string "I hope you've been well. I\n"
	.string "wanted to tell you I just won.\p"
	.string "My {STR_VAR_3} worked\n"
	.string "especially hard to get the\p"
	.string "win. See you again!$"

MatchCall_HOENN_Jacki_Battle::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is JACKI speaking.\p"
	.string "I hope you're doing well. My\n"
	.string "POKéMON are very frisky. If\p"
	.string "you're ever in the area,\n"
	.string "please give me a rematch. I'll\p"
	.string "be around {STR_VAR_2}.\n"
	.string "Until then, good-bye!$"
```

```asm
@ WALTER - Gentleman, Route 121

MatchCall_HOENN_Walter_General1::
	.string "Hello, this is WALTER.\n"
	.string "You sound well, {PLAYER}.\p"
	.string "I've traveled around the\n"
	.string "world, but I must say I've\p"
	.string "taken a great shine to this\n"
	.string "region. I plan to stay here a\p"
	.string "while. Perhaps we can meet\n"
	.string "again? I've not forgotten your\p"
	.string "dazzling techniques. I do hope\n"
	.string "for a rematch.$"

MatchCall_HOENN_Walter_General2::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, WALTER.\p"
	.string "Are you still catching\n"
	.string "POKéMON? I've been trying to\p"
	.string "catch them myself, but it's\n"
	.string "not so easy. The way of\p"
	.string "POKéMON is deep!$"

MatchCall_HOENN_Walter_General3::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, WALTER.\p"
	.string "I trust you've been well? I'm\n"
	.string "still bursting with life! Why,\p"
	.string "just now, I won another match.\n"
	.string "I'm not stepping aside to you\p"
	.string "youngsters yet!$"

MatchCall_HOENN_Walter_Battle::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, WALTER.\p"
	.string "I should tell you, my POKéMON\n"
	.string "have grown to be quite robust\p"
	.string "lately. I would like to see\n"
	.string "them in a battle with you,\p"
	.string "{PLAYER}. We'll be around\n"
	.string "{STR_VAR_2}. Come see us\p"
	.string "anytime!$"
```

```asm
@ KAREN - School Kid, Route 116

MatchCall_HOENN_Karen_General1::
	.string "It's KAREN!\p"
	.string "ROXANNE let me battle with her\n"
	.string "yesterday. The results…\p"
	.string "Terrible, like you needed to\n"
	.string "ask. But I was delighted that\p"
	.string "ROXANNE would even let me\n"
	.string "challenge her! You wouldn't\p"
	.string "believe how much more I admire\n"
	.string "her! I'm going to really focus\p"
	.string "and work! I'd better go!$"

MatchCall_HOENN_Karen_General2::
	.string "Hello, {PLAYER}.\n"
	.string "It's KAREN.\p"
	.string "I tried to catch a nice\n"
	.string "{STR_VAR_3} a little while ago.\p"
	.string "But, it got away. I was sure\n"
	.string "disappointed! Okay, bye!$"

MatchCall_HOENN_Karen_General3::
	.string "Hello, {PLAYER}.\n"
	.string "It's KAREN.\p"
	.string "I challenged someone else\n"
	.string "after we battled. I came\p"
	.string "close, but I ended up losing.\n"
	.string "Oh, well!$"

MatchCall_HOENN_Karen_Battle::
	.string "Hello, {PLAYER}!\n"
	.string "It's KAREN.\p"
	.string "Would you like to have a\n"
	.string "battle with me again? You can\p"
	.string "find me around\n"
	.string "{STR_VAR_2}. I'll be\p"
	.string "waiting!$"
```

```asm
@ JERRY - School Kid, Route 116

MatchCall_HOENN_Jerry_General1::
	.string "Snivel… It's… JERRY…\n"
	.string "…Sob…\p"
	.string "ROXANNE chewed me out in class\n"
	.string "today. But I don't dislike her\p"
	.string "or anything. ROXANNE tells me\n"
	.string "exactly what I did wrong so I\p"
	.string "can learn from it. You bet\n"
	.string "I'll be going to the TRAINER'S\p"
	.string "SCHOOL tomorrow! See you\n"
	.string "later!$"

MatchCall_HOENN_Jerry_General2::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is JERRY.\p"
	.string "I saw this {STR_VAR_3} a while\n"
	.string "back but I couldn't catch it.\p"
	.string "It was so close, too! Well,\n"
	.string "see you again!$"

MatchCall_HOENN_Jerry_General3::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is JERRY!\p"
	.string "I battled another TRAINER\n"
	.string "earlier. I won! I won! My\p"
	.string "{STR_VAR_3} really worked hard\n"
	.string "for me. This is so great!$"

MatchCall_HOENN_Jerry_Battle::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is JERRY.\p"
	.string "Want to have a battle with me?\n"
	.string "I'll be waiting for you around\p"
	.string "{STR_VAR_2}!$"
```

```asm
@ ANNA & MEG - Sr. and Jr., Route 117

MatchCall_HOENN_AnnaAndMeg_General1::
	.string "Hi, it's ANNA & MEG! We're\n"
	.string "together again! We really love\p"
	.string "caring for POKéMON. They're so\n"
	.string "cute! Oh, MEG, did you get the\p"
	.string "buns? No, no, I'm not treating\n"
	.string "you like my personal slave!\p"
	.string "You lost the match, so you\n"
	.string "have to buy the bread as\p"
	.string "punishment! I wouldn't treat\n"
	.string "you like a slave, MEG! You're\p"
	.string "too special to me! I have to\n"
	.string "go now. It's time for our\p"
	.string "snack!$"

MatchCall_HOENN_AnnaAndMeg_General2::
	.string "Oh, {PLAYER}, hi there!\p"
	.string "This is ANNA & MEG! Listen,\n"
	.string "listen, you have to hear this!\p"
	.string "We had a POKéMON battle\n"
	.string "earlier, but we lost at the\p"
	.string "last second. Oh, it burns us\n"
	.string "up!$"

MatchCall_HOENN_AnnaAndMeg_General3::
	.string "Oh, {PLAYER}, hi there!\p"
	.string "This is ANNA & MEG! How are\n"
	.string "your POKéMON holding up? Ours\p"
	.string "just won a battle! Our\n"
	.string "{STR_VAR_3} was spectacular, we\p"
	.string "must say! We wish we could've\n"
	.string "shown you! See you again!$"

MatchCall_HOENN_AnnaAndMeg_Battle::
	.string "Oh, {PLAYER}, hi there!\p"
	.string "This is ANNA & MEG! How are\n"
	.string "your POKéMON doing? Our\p"
	.string "POKéMON keep getting better.\n"
	.string "We'd like to show you,\p"
	.string "{PLAYER}. We're around\n"
	.string "{STR_VAR_2} now, so let's\p"
	.string "battle if you're close by.\n"
	.string "Hope we see you soon!$"
```

```asm
@ ISABEL - Pokéfan, Route 110

MatchCall_HOENN_Isabel_General1::
	.string "Ohoho!\p"
	.string "This is ISABEL! I can't wait\n"
	.string "to tell you about my darling\p"
	.string "POKéMON! Please, you must\n"
	.string "listen to this. It's about my\p"
	.string "darling {STR_VAR_3}. Whenever\n"
	.string "anyone picks it up, it leaps\p"
	.string "straight into my arms! Oh… Oh…\n"
	.string "Could there be anything more\p"
	.string "blissful? Oh, it feels so\n"
	.string "heavenly! Well, I must be\p"
	.string "going. Bye, now!$"

MatchCall_HOENN_Isabel_General2::
	.string "Oh, hi, {PLAYER}.\n"
	.string "This is ISABEL.\p"
	.string "Are you doing good? You should\n"
	.string "go home every so often,\p"
	.string "though. Bye-bye!$"

MatchCall_HOENN_Isabel_General3::
	.string "Oh, hi, {PLAYER}.\n"
	.string "This is ISABEL.\p"
	.string "I was in a battle recently,\n"
	.string "and my {STR_VAR_3} was\p"
	.string "exceptional! I wish you could\n"
	.string "have seen it, {PLAYER}.\p"
	.string "Bye-bye!$"

MatchCall_HOENN_Isabel_Battle::
	.string "Oh, hi, {PLAYER}.\n"
	.string "This is ISABEL.\p"
	.string "Keeping well, I hope. Oh, yes!\n"
	.string "My POKéMON are much stronger\p"
	.string "than before. Don't you think\n"
	.string "we ought to have a battle,\p"
	.string "{PLAYER}? We'll be waiting for\n"
	.string "you around {STR_VAR_2}.\p"
	.string "Come see us anytime, okay?$"
```

```asm
@ MIGUEL - Pokéfan, Route 103

MatchCall_HOENN_Miguel_General1::
	.string "I love POKéMON!\p"
	.string "It's MIGUEL from the FAN CLUB!\n"
	.string "You have to hear this! My\p"
	.string "sweet POKéMON… Snort! Wahaha!\n"
	.string "I can't say any more! It's a\p"
	.string "secret! It's just too cute for\n"
	.string "words! Oh, my sweet {STR_VAR_3}\p"
	.string "is begging for a treat! It's\n"
	.string "the picture of cuteness!\p"
	.string "Sorry, but I can't talk now!\n"
	.string "You'll have to hear this next\p"
	.string "time!$"

MatchCall_HOENN_Miguel_General2::
	.string "Ah, {PLAYER}.\n"
	.string "This is MIGUEL.\p"
	.string "How are things with you? I\n"
	.string "tried to catch a wild\p"
	.string "{STR_VAR_3} earlier, but it\n"
	.string "managed to flee. I feel\p"
	.string "defeated…$"

MatchCall_HOENN_Miguel_General3::
	.string "Ah, {PLAYER}.\n"
	.string "This is MIGUEL.\p"
	.string "How are your POKéMON? I just\n"
	.string "lost yet another battle. Well,\p"
	.string "see you!$"

MatchCall_HOENN_Miguel_Battle::
	.string "Ah, {PLAYER}.\n"
	.string "This is MIGUEL.\p"
	.string "Where might you be now? My\n"
	.string "POKéMON are full of life. They\p"
	.string "appear to be looking forward\n"
	.string "to seeing your POKéMON,\p"
	.string "{PLAYER}. I'm around\n"
	.string "{STR_VAR_2} now. I hope\p"
	.string "you'll seek us out.$"
```

```asm
@ TIMOTHY - Expert, Route 115

MatchCall_HOENN_Timothy_General1::
	.string "I am… TIMOTHY.\n"
	.string "People call me an EXPERT.\p"
	.string "But there is one thing I know.\n"
	.string "I could not be an EXPERT on my\p"
	.string "own power. Only with the help\n"
	.string "of POKéMON can a TRAINER\p"
	.string "become an EXPERT. Humph! I\n"
	.string "believe I may have said\p"
	.string "something deep and profound! I\n"
	.string "shall leave you in good\p"
	.string "spirits!$"

MatchCall_HOENN_Timothy_General2::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, TIMOTHY.\p"
	.string "Are you still battling hard?\n"
	.string "As for me, I lost recently, so\p"
	.string "I've been training my team all\n"
	.string "over. Let's meet again.$"

MatchCall_HOENN_Timothy_General3::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, TIMOTHY.\p"
	.string "I trust you've been well? I'm\n"
	.string "still bursting with life! Why,\p"
	.string "just now, I won another match.\n"
	.string "I'm not stepping aside to you\p"
	.string "youngsters yet!$"

MatchCall_HOENN_Timothy_Battle::
	.string "Hello, {PLAYER}.\n"
	.string "It's me, TIMOTHY.\p"
	.string "I should tell you, my POKéMON\n"
	.string "have grown to be quite robust\p"
	.string "lately. I would like to see\n"
	.string "them in a battle with you,\p"
	.string "{PLAYER}. We'll be around\n"
	.string "{STR_VAR_2}. Come see us\p"
	.string "anytime!$"
```

```asm
@ SHELBY - Expert, Mt. Chimney

MatchCall_HOENN_Shelby_General1::
	.string "It's SHELBY. I'm glad to chat\n"
	.string "with you! I do love soaking in\p"
	.string "a hot-spring tub. I feel alive\n"
	.string "and refreshed! I've been\p"
	.string "battling young TRAINERS since\n"
	.string "we met, but you're still the\p"
	.string "best of the lot. I imagine\n"
	.string "you'll become an EXPERT in\p"
	.string "your old age! Ohohoho…$"

MatchCall_HOENN_Shelby_General2::
	.string "Ah, hello, {PLAYER}!\n"
	.string "This is SHELBY!\p"
	.string "I hope you've been keeping\n"
	.string "well. I was in a battle just a\p"
	.string "little while before this.\n"
	.string "{PLAYER}, try to be active like\p"
	.string "me. See you again!$"

MatchCall_HOENN_Shelby_General3::
	.string "Ah, hello, {PLAYER}!\n"
	.string "This is SHELBY!\p"
	.string "I hope you've been keeping\n"
	.string "well. I still have a bounce in\p"
	.string "my step! Why, I just won a\n"
	.string "battle yet again. Oh, I won't\p"
	.string "lose to young people quite\n"
	.string "yet! See you again!$"

MatchCall_HOENN_Shelby_Battle::
	.string "Ah, hello, {PLAYER}!\n"
	.string "This is SHELBY!\p"
	.string "Are your POKéMON keeping well?\n"
	.string "My POKéMON have been so\p"
	.string "healthy, they don't look\n"
	.string "capable of losing! I would\p"
	.string "surely love to have another\n"
	.string "battle with you. If you're\p"
	.string "near {STR_VAR_2}, do come\n"
	.string "see us.$"
```

```asm
@ CALVIN - Youngster, Route 102

MatchCall_HOENN_Calvin_General1::
	.string "Yay! This is CALVIN! What's\n"
	.string "up? I might be imagining this,\p"
	.string "but when I win battles, my\n"
	.string "shorts seem to feel, like,\p"
	.string "better. Materially. My shorts\n"
	.string "feel silkier! And when I\p"
	.string "battled you, {PLAYER}, my\n"
	.string "shorts felt icky and coarse. …\p"
	.string "… … … … … You didn't really\n"
	.string "believe that? Ehehehe, that's\p"
	.string "all! Bye now!$"

MatchCall_HOENN_Calvin_General2::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is CALVIN.\p"
	.string "I tried battling another\n"
	.string "TRAINER, but I lost. It was\p"
	.string "really disappointing. Well,\n"
	.string "see you again!$"

MatchCall_HOENN_Calvin_General3::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is CALVIN!\p"
	.string "I battled another TRAINER\n"
	.string "earlier. I won! I won! My\p"
	.string "{STR_VAR_3} really worked hard\n"
	.string "for me. This is so great!$"

MatchCall_HOENN_Calvin_Battle::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is CALVIN.\p"
	.string "Want to have a battle with me?\n"
	.string "I'll be waiting for you around\p"
	.string "{STR_VAR_2}!$"
```

```asm
@ ELLIOT - Fisherman, Route 106

MatchCall_HOENN_Elliot_General1::
	.string "Ahoy!\n"
	.string "ELLIOT here!\p"
	.string "As always, I'm fishing with\n"
	.string "wild abandon! Are there other\p"
	.string "places I can fish than the sea\n"
	.string "and rivers? I get these\p"
	.string "powerful urges to fish just\n"
	.string "about anywhere! Oh, gosh, darn\p"
	.string "it! My line's tangled up!\n"
	.string "Gotta go! Find me some new\p"
	.string "fishing spots!$"

MatchCall_HOENN_Elliot_General2::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, ELLIOT.\p"
	.string "I just took a shot at catching\n"
	.string "this {STR_VAR_3}, but it took\p"
	.string "off. I came oh so close, too!\n"
	.string "It spoiled my day… All right,\p"
	.string "see you!$"

MatchCall_HOENN_Elliot_General3::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, ELLIOT!\p"
	.string "How's your battling? Me, I had\n"
	.string "a battle the other day, and my\p"
	.string "{STR_VAR_3} came up huge! The\n"
	.string "next time I battle you,\p"
	.string "{PLAYER}, it won't be me\n"
	.string "losing!$"

MatchCall_HOENN_Elliot_Battle::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, ELLIOT.\p"
	.string "My POKéMON have grown a lot\n"
	.string "tougher since last time. I\p"
	.string "want to see how strong they've\n"
	.string "become with your POKéMON,\p"
	.string "{PLAYER}. So, let's have a\n"
	.string "battle! I'll be waiting for\p"
	.string "you around {STR_VAR_2}.$"
```

```asm
@ ISAIAH - Triathlete, Route 128

MatchCall_HOENN_Isaiah_General1::
	.string "Oh, it's ISAIAH, hello.\p"
	.string "I've been swimming a lot but I\n"
	.string "still can't seem to reach\p"
	.string "EVERGRANDE. Maybe I'm just\n"
	.string "going in circles. No, no, that\p"
	.string "can't be possible. Wahahaha.\n"
	.string "Take care!$"

MatchCall_HOENN_Isaiah_General2::
	.string "Hiya, {PLAYER}!\n"
	.string "It's ISAIAH.\p"
	.string "Catching any POKéMON lately? A\n"
	.string "little while ago I came close\p"
	.string "to nabbing one, but it got\n"
	.string "loose. Right, take care!$"

MatchCall_HOENN_Isaiah_General3::
	.string "Hiya, {PLAYER}!\n"
	.string "It's ISAIAH.\p"
	.string "How are things with you? I've\n"
	.string "been battling on, but I\p"
	.string "haven't won very often. I\n"
	.string "can't get it together. Right,\p"
	.string "take care!$"

MatchCall_HOENN_Isaiah_Battle::
	.string "Hiya, {PLAYER}!\n"
	.string "It's ISAIAH.\p"
	.string "My POKéMON are growing up in\n"
	.string "decent ways. I'd really like\p"
	.string "to have another battle with\n"
	.string "you. I'll keep an eye out for\p"
	.string "you around {STR_VAR_2}.\n"
	.string "See you soon!$"
```

```asm
@ MARIA - Triathlete, Route 117

MatchCall_HOENN_Maria_General1::
	.string "Hi, it's MARIA. If you want to\n"
	.string "improve endurance,\p"
	.string "high-altitude training is it!\n"
	.string "Try running on a mountaintop.\p"
	.string "You'll be gasping in no time!\n"
	.string "I'm getting oxygen starved,\p"
	.string "too! See you!$"

MatchCall_HOENN_Maria_General2::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is MARIA!\p"
	.string "So? Are you getting more\n"
	.string "POKéMON together? I'm having a\p"
	.string "rotten time of it! They all\n"
	.string "get away from me! See you!$"

MatchCall_HOENN_Maria_General3::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is MARIA!\p"
	.string "Listen, listen, you have to\n"
	.string "hear this! I had a POKéMON\p"
	.string "battle earlier, but I lost at\n"
	.string "the last second. Oh, it burns\p"
	.string "me up!$"

MatchCall_HOENN_Maria_Battle::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is MARIA!\p"
	.string "How are your POKéMON doing? My\n"
	.string "POKéMON keep getting better.\p"
	.string "I'd like to show you, {PLAYER}.\n"
	.string "I'm around {STR_VAR_2}\p"
	.string "now, so let's battle if you're\n"
	.string "close by. Hope I see you soon!$"
```

```asm
@ ABIGAIL - Triathlete, Route 110

MatchCall_HOENN_Abigail_General1::
	.string "This is ABIGAIL!\n"
	.string "I'm cycling right now.\p"
	.string "I love swimming and running,\n"
	.string "but cycling is my first love!\p"
	.string "It makes my whole body feel as\n"
	.string "if I'm one with the wind. It\p"
	.string "exhilarates me as if I were\n"
	.string "flying! Okay! Today, I'm going\p"
	.string "to set a new CYCLING ROAD\n"
	.string "record! You should make the\p"
	.string "challenge, too! See you!$"

MatchCall_HOENN_Abigail_General2::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is ABIGAIL!\p"
	.string "So? Are you getting more\n"
	.string "POKéMON together? I'm having a\p"
	.string "rotten time of it! They all\n"
	.string "get away from me! See you!$"

MatchCall_HOENN_Abigail_General3::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is ABIGAIL!\p"
	.string "How are your POKéMON holding\n"
	.string "up? Mine just won a battle! My\p"
	.string "{STR_VAR_3} was spectacular, I\n"
	.string "must say! I wish I could've\p"
	.string "shown you! See you again!$"

MatchCall_HOENN_Abigail_Battle::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is ABIGAIL!\p"
	.string "How are your POKéMON doing? My\n"
	.string "POKéMON keep getting better.\p"
	.string "I'd like to show you, {PLAYER}.\n"
	.string "I'm around {STR_VAR_2}\p"
	.string "now, so let's battle if you're\n"
	.string "close by. Hope I see you soon!$"
```

```asm
@ DYLAN - Triathlete, Route 117

MatchCall_HOENN_Dylan_General1::
	.string "Yo, this is DYLAN! I'm smack\n"
	.string "in the middle of a triathlon!\p"
	.string "But, hey, I've always got time\n"
	.string "to shoot the breeze! Working\p"
	.string "out with POKéMON feels mighty\n"
	.string "good! Without exchanging\p"
	.string "words, we synch as if we\n"
	.string "shared a heart. It's\p"
	.string "inspiring! Gasp… Chatting\n"
	.string "while running… I'm getting run\p"
	.string "down… Gasp… Have…to…go…$"

MatchCall_HOENN_Dylan_General2::
	.string "Hiya, {PLAYER}!\n"
	.string "It's DYLAN.\p"
	.string "Catching any POKéMON lately? A\n"
	.string "little while ago I came close\p"
	.string "to nabbing one, but it got\n"
	.string "loose. Right, take care!$"

MatchCall_HOENN_Dylan_General3::
	.string "Hiya, {PLAYER}!\n"
	.string "It's DYLAN.\p"
	.string "How are things with you?\n"
	.string "Battling much? I just won a\p"
	.string "while back! My {STR_VAR_3} was\n"
	.string "brilliant! You wait. I'm going\p"
	.string "to beat you next time! Right,\n"
	.string "take care!$"

MatchCall_HOENN_Dylan_Battle::
	.string "Hiya, {PLAYER}!\n"
	.string "It's DYLAN.\p"
	.string "My POKéMON are growing up in\n"
	.string "decent ways. I'd really like\p"
	.string "to have another battle with\n"
	.string "you. I'll keep an eye out for\p"
	.string "you around {STR_VAR_2}.\n"
	.string "See you soon!$"
```

```asm
@ KATELYN - Triathlete, Route 128

MatchCall_HOENN_Katelyn_General1::
	.string "Hey, it's KATELYN… Whoops!\p"
	.string "Splash! Blug-blug-blug-blug…\n"
	.string "Sploosh! Hey! Sorry about\p"
	.string "that! I just put on some\n"
	.string "suntan oil. So my POKéGEAR\p"
	.string "went whoopsy out of my hand\n"
	.string "into the water! But, boy, it's\p"
	.string "built tough. It survived that\n"
	.string "dunking! Anyways, I'm busy\p"
	.string "sunbathing, so let's chat\n"
	.string "another time.$"

MatchCall_HOENN_Katelyn_General2::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is KATELYN!\p"
	.string "Listen, listen, you have to\n"
	.string "hear this! I had a POKéMON\p"
	.string "battle earlier, but I lost at\n"
	.string "the last second. Oh, it burns\p"
	.string "me up!$"

MatchCall_HOENN_Katelyn_General3::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is KATELYN!\p"
	.string "How are your POKéMON holding\n"
	.string "up? Mine just won a battle! My\p"
	.string "{STR_VAR_3} was spectacular, I\n"
	.string "must say! I wish I could've\p"
	.string "shown you! See you again!$"

MatchCall_HOENN_Katelyn_Battle::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is KATELYN!\p"
	.string "How are your POKéMON doing? My\n"
	.string "POKéMON keep getting better.\p"
	.string "I'd like to show you, {PLAYER}.\n"
	.string "I'm around {STR_VAR_2}\p"
	.string "now, so let's battle if you're\n"
	.string "close by. Hope I see you soon!$"
```

```asm
@ BENJAMIN - Triathlete, Route 110

MatchCall_HOENN_Benjamin_General1::
	.string "Hey, there! It's BENJAMIN. Are\n"
	.string "you taking it casually? Ever\p"
	.string "since I was a kid, you know,\n"
	.string "I've always been placid, huh?\p"
	.string "I never was much for getting\n"
	.string "all stressed or rushing\p"
	.string "things. But wouldn't you know\n"
	.string "it, I'm now a TRIATHLETE. You\p"
	.string "just can't tell where life\n"
	.string "will take you, if you get my\p"
	.string "drift. So, hey, be cool. Take\n"
	.string "it casual. See you around.$"

MatchCall_HOENN_Benjamin_General2::
	.string "Hiya, {PLAYER}!\n"
	.string "It's BENJAMIN.\p"
	.string "How are things with you? I've\n"
	.string "been battling on, but I\p"
	.string "haven't won very often. I\n"
	.string "can't get it together. Right,\p"
	.string "take care!$"

MatchCall_HOENN_Benjamin_General3::
	.string "Hiya, {PLAYER}!\n"
	.string "It's BENJAMIN.\p"
	.string "How are things with you?\n"
	.string "Battling much? I just won a\p"
	.string "while back! My {STR_VAR_3} was\n"
	.string "brilliant! You wait. I'm going\p"
	.string "to beat you next time! Right,\n"
	.string "take care!$"

MatchCall_HOENN_Benjamin_Battle::
	.string "Hiya, {PLAYER}!\n"
	.string "It's BENJAMIN.\p"
	.string "My POKéMON are growing up in\n"
	.string "decent ways. I'd really like\p"
	.string "to have another battle with\n"
	.string "you. I'll keep an eye out for\p"
	.string "you around {STR_VAR_2}.\n"
	.string "See you soon!$"
```

```asm
@ PABLO - Triathlete, Route 126

MatchCall_HOENN_Pablo_General1::
	.string "Hello, this is PABLO.\p"
	.string "Out of the three triathlon\n"
	.string "events, I like swimming best.\p"
	.string "But if I stay in the sea too\n"
	.string "long, won't I get all\p"
	.string "prune-like? Ooh, triathlon is\n"
	.string "such a grueling test of human\p"
	.string "endurance! Bye!$"

MatchCall_HOENN_Pablo_General2::
	.string "Hiya, {PLAYER}!\n"
	.string "It's PABLO.\p"
	.string "Catching any POKéMON lately? A\n"
	.string "little while ago I came close\p"
	.string "to nabbing one, but it got\n"
	.string "loose. Right, take care!$"

MatchCall_HOENN_Pablo_General3::
	.string "Hiya, {PLAYER}!\n"
	.string "It's PABLO.\p"
	.string "How are things with you? I've\n"
	.string "been battling on, but I\p"
	.string "haven't won very often. I\n"
	.string "can't get it together. Right,\p"
	.string "take care!$"

MatchCall_HOENN_Pablo_Battle::
	.string "Hiya, {PLAYER}!\n"
	.string "It's PABLO.\p"
	.string "My POKéMON are growing up in\n"
	.string "decent ways. I'd really like\p"
	.string "to have another battle with\n"
	.string "you. I'll keep an eye out for\p"
	.string "you around {STR_VAR_2}.\n"
	.string "See you soon!$"
```

```asm
@ NICOLAS - Dragon Tamer, Meteor Falls

MatchCall_HOENN_Nicolas_General1::
	.string "Hello, {PLAYER}.\n"
	.string "NICOLAS here.\p"
	.string "How are your POKéMON doing? My\n"
	.string "DRAGON POKéMON appear to be in\p"
	.string "peak form. Bye for now.$"

MatchCall_HOENN_Nicolas_General2::
	.string "Hey, {PLAYER}.\n"
	.string "NICOLAS here.\p"
	.string "I tried another battle\n"
	.string "yesterday, but I couldn't pull\p"
	.string "out the win. My team needs\n"
	.string "more raising. Okay, catch you\p"
	.string "later.$"

MatchCall_HOENN_Nicolas_General3::
	.string "Hey, {PLAYER}.\n"
	.string "NICOLAS here.\p"
	.string "I had a match earlier. I\n"
	.string "managed to win, but it was\p"
	.string "close. My {STR_VAR_3} put on\n"
	.string "one inspired showing.$"

MatchCall_HOENN_Nicolas_Battle::
	.string "Hey, {PLAYER}.\n"
	.string "NICOLAS here.\p"
	.string "How are things with you? My\n"
	.string "POKéMON have grown pretty\p"
	.string "tough lately. Hey, how would\n"
	.string "you like to have another\p"
	.string "battle with me? Let's meet up\n"
	.string "around {STR_VAR_2}, okay?$"
```

```asm
@ ROBERT - Bird Keeper, Route 120

MatchCall_HOENN_Robert_General1::
	.string "ROBERT here.\p"
	.string "My {STR_VAR_3} has grown even\n"
	.string "more tough than that last\p"
	.string "time. I'm not going to lose\n"
	.string "again to you. You wait till\p"
	.string "next time! See you around!$"

MatchCall_HOENN_Robert_General2::
	.string "Hey, {PLAYER}.\n"
	.string "ROBERT here.\p"
	.string "Caught any POKéMON lately? I\n"
	.string "nearly nabbed one the other\p"
	.string "day. But it evaded me somehow.\n"
	.string "You take care.$"

MatchCall_HOENN_Robert_General3::
	.string "Hey, {PLAYER}.\n"
	.string "ROBERT here.\p"
	.string "How's it going for you? I've\n"
	.string "been riding a hot streak. Why,\p"
	.string "I just won a battle. When we\n"
	.string "have our next battle, I'm sure\p"
	.string "not going to lose!$"

MatchCall_HOENN_Robert_Battle::
	.string "Hey, {PLAYER}.\n"
	.string "ROBERT here.\p"
	.string "I hope you're on top of\n"
	.string "things. I was thinking I'd\p"
	.string "like another battle with you.\n"
	.string "What do you say? If you feel\p"
	.string "like a battle, come to\n"
	.string "{STR_VAR_2}. See you!$"
```

```asm
@ LAO - Ninja Boy, Route 113

MatchCall_HOENN_Lao_General1::
	.string "It is LAO here.\p"
	.string "I have continued with my\n"
	.string "studies in the art of\p"
	.string "concealment. But I have been\n"
	.string "too successful. No one has\p"
	.string "been able to find me. My\n"
	.string "success makes me lonely… Like\p"
	.string "smoke I disappear! Farewell!$"

MatchCall_HOENN_Lao_General2::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is LAO.\p"
	.string "I saw this {STR_VAR_3} a while\n"
	.string "back but I couldn't catch it.\p"
	.string "It was so close, too! Well,\n"
	.string "see you again!$"

MatchCall_HOENN_Lao_General3::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is LAO.\p"
	.string "I tried battling another\n"
	.string "TRAINER, but I lost. It was\p"
	.string "really disappointing. Well,\n"
	.string "see you again!$"

MatchCall_HOENN_Lao_Battle::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is LAO.\p"
	.string "Want to have a battle with me?\n"
	.string "I'll be waiting for you around\p"
	.string "{STR_VAR_2}!$"
```

```asm
@ CYNDY - Battle Girl, Route 115

MatchCall_HOENN_Cyndy_General1::
	.string "This is CYNDY. I kept up my\n"
	.string "training since we met. My\p"
	.string "{STR_VAR_3} is getting pretty\n"
	.string "tough. Training on a beach is\p"
	.string "effective, just as I thought.\n"
	.string "Bye now!$"

MatchCall_HOENN_Cyndy_General2::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is CYNDY!\p"
	.string "So? Are you getting more\n"
	.string "POKéMON together? I'm having a\p"
	.string "rotten time of it! They all\n"
	.string "get away from me! See you!$"

MatchCall_HOENN_Cyndy_General3::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is CYNDY!\p"
	.string "Listen, listen, you have to\n"
	.string "hear this! I had a POKéMON\p"
	.string "battle earlier, but I lost at\n"
	.string "the last second. Oh, it burns\p"
	.string "me up!$"

MatchCall_HOENN_Cyndy_Battle::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is CYNDY!\p"
	.string "How are your POKéMON doing? My\n"
	.string "POKéMON keep getting better.\p"
	.string "I'd like to show you, {PLAYER}.\n"
	.string "I'm around {STR_VAR_2}\p"
	.string "now, so let's battle if you're\n"
	.string "close by. Hope I see you soon!$"
```

```asm
@ MADELINE - Parasol Lady, Route 113

MatchCall_HOENN_Madeline_General1::
	.string "How do you do?\n"
	.string "This is MADELINE.\p"
	.string "I wonder when this yucky\n"
	.string "volcanic ash will stop\p"
	.string "falling? If it gets too deep,\n"
	.string "it will cover up the pattern\p"
	.string "on my parasol… Let's promise\n"
	.string "to meet again!$"

MatchCall_HOENN_Madeline_General2::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is MADELINE speaking.\p"
	.string "Have you had success catching\n"
	.string "POKéMON lately? I came very\p"
	.string "close a little while ago, but\n"
	.string "my target got free. I need to\p"
	.string "try harder! See you again!$"

MatchCall_HOENN_Madeline_General3::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is MADELINE speaking.\p"
	.string "How are your POKéMON doing? I\n"
	.string "lost a match the other day. I\p"
	.string "need to try harder! See you\n"
	.string "again!$"

MatchCall_HOENN_Madeline_Battle::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is MADELINE speaking.\p"
	.string "I hope you're doing well. My\n"
	.string "POKéMON are very frisky. If\p"
	.string "you're ever in the area,\n"
	.string "please give me a rematch. I'll\p"
	.string "be around {STR_VAR_2}.\n"
	.string "Until then, good-bye!$"
```

```asm
@ JENNY - Swimmer, Route 124

MatchCall_HOENN_Jenny_General1::
	.string "Hi, JENNY here.\p"
	.string "Did you know that it's easier\n"
	.string "to float in the sea than a\p"
	.string "pool? Just by lying still,\n"
	.string "your body will float on its\p"
	.string "own. But if you float for too\n"
	.string "long, watch that you don't get\p"
	.string "carried off too far out.\n"
	.string "…Where am I, anyway? I'd\p"
	.string "better go!$"

MatchCall_HOENN_Jenny_General2::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is JENNY!\p"
	.string "So? Are you getting more\n"
	.string "POKéMON together? I'm having a\p"
	.string "rotten time of it! They all\n"
	.string "get away from me! See you!$"

MatchCall_HOENN_Jenny_General3::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is JENNY!\p"
	.string "How are your POKéMON holding\n"
	.string "up? Mine just won a battle! My\p"
	.string "{STR_VAR_3} was spectacular, I\n"
	.string "must say! I wish I could've\p"
	.string "shown you! See you again!$"

MatchCall_HOENN_Jenny_Battle::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is JENNY!\p"
	.string "How are your POKéMON doing? My\n"
	.string "POKéMON keep getting better.\p"
	.string "I'd like to show you, {PLAYER}.\n"
	.string "I'm around {STR_VAR_2}\p"
	.string "now, so let's battle if you're\n"
	.string "close by. Hope I see you soon!$"
```

```asm
@ DIANA - Picnicker, Jagged Pass

MatchCall_HOENN_Diana_General1::
	.string "Oh, {PLAYER}, hello! This is\n"
	.string "DIANA. I love the mountains.\p"
	.string "But the ground is too bumpy to\n"
	.string "pitch my tent. Oh, I had a\p"
	.string "brilliant idea! Maybe I can\n"
	.string "get my POKéMON to tamp the\p"
	.string "ground flat! I'm going to try\n"
	.string "that! Bye-bye!$"

MatchCall_HOENN_Diana_General2::
	.string "Hello, {PLAYER}.\n"
	.string "It's DIANA.\p"
	.string "I tried to catch a nice\n"
	.string "{STR_VAR_3} a little while ago.\p"
	.string "But, it got away. I was sure\n"
	.string "disappointed! Okay, bye!$"

MatchCall_HOENN_Diana_General3::
	.string "Hello, {PLAYER}!\n"
	.string "It's DIANA!\p"
	.string "I had a battle yesterday and I\n"
	.string "won! It's fantastic!$"

MatchCall_HOENN_Diana_Battle::
	.string "Hello, {PLAYER}!\n"
	.string "It's DIANA.\p"
	.string "Would you like to have a\n"
	.string "battle with me again? You can\p"
	.string "find me around\n"
	.string "{STR_VAR_2}. I'll be\p"
	.string "waiting!$"
```

```asm
@ AMY & LIV - Twins, Route 103

MatchCall_HOENN_AmyAndLiv_General1::
	.string "Oh, hi, hi, this is AMY & LIV!\p"
	.string "We're raising POKéMON\n"
	.string "together! We're trying very\p"
	.string "hard! If we try harder, can we\n"
	.string "become number one? Bye-bye!$"

MatchCall_HOENN_AmyAndLiv_General2::
	.string "Hello, {PLAYER}.\p"
	.string "It's AMY & LIV. We challenged\n"
	.string "someone else after we battled.\p"
	.string "We came close, but we ended up\n"
	.string "losing. Oh, well!$"

MatchCall_HOENN_AmyAndLiv_General3::
	.string "Hello, {PLAYER}!\p"
	.string "It's AMY & LIV! We had a\n"
	.string "battle yesterday and we won!\p"
	.string "It's fantastic!$"

MatchCall_HOENN_AmyAndLiv_Battle::
	.string "Hello, {PLAYER}!\p"
	.string "It's AMY & LIV. Would you like\n"
	.string "to have a battle with us\p"
	.string "again? You can find us around\n"
	.string "{STR_VAR_2}. We'll be\p"
	.string "waiting!$"
```

```asm
@ ERNEST - Sailor, Route 125

MatchCall_HOENN_Ernest_General1::
	.string "ERNEST here!\p"
	.string "I'm a SAILOR, but sometimes\n"
	.string "I'm not on a boat. It makes me\p"
	.string "wonder--what should a SAILOR\n"
	.string "on land be called? That's what\p"
	.string "I've been thinking while\n"
	.string "staring out across the waves.\p"
	.string "All right, next time!$"

MatchCall_HOENN_Ernest_General2::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, ERNEST.\p"
	.string "I just got cleaned in a\n"
	.string "battle. I guess I need to\p"
	.string "raise my team some more!$"

MatchCall_HOENN_Ernest_General3::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, ERNEST!\p"
	.string "How's your battling? Me, I had\n"
	.string "a battle the other day, and my\p"
	.string "{STR_VAR_3} came up huge! The\n"
	.string "next time I battle you,\p"
	.string "{PLAYER}, it won't be me\n"
	.string "losing!$"

MatchCall_HOENN_Ernest_Battle::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, ERNEST.\p"
	.string "My POKéMON have grown a lot\n"
	.string "tougher since last time. I\p"
	.string "want to see how strong they've\n"
	.string "become with your POKéMON,\p"
	.string "{PLAYER}. So, let's have a\n"
	.string "battle! I'll be waiting for\p"
	.string "you around {STR_VAR_2}.$"
```

```asm
@ CORY - Sailor, Route 108

MatchCall_HOENN_Cory_General1::
	.string "Ahoy there! It's me, CORY!\p"
	.string "I'm always out on ROUTE 108!\n"
	.string "Today, a gorgeous SWIMMER swam\p"
	.string "by me! So I startled her with\n"
	.string "a shout! And she gave me a\p"
	.string "nasty glare! That's all from\n"
	.string "ROUTE 108! Brought to you by\p"
	.string "CORY!$"

MatchCall_HOENN_Cory_General2::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, CORY.\p"
	.string "I just took a shot at catching\n"
	.string "this {STR_VAR_3}, but it took\p"
	.string "off. I came oh so close, too!\n"
	.string "It spoiled my day… All right,\p"
	.string "see you!$"

MatchCall_HOENN_Cory_General3::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, CORY.\p"
	.string "I just got cleaned in a\n"
	.string "battle. I guess I need to\p"
	.string "raise my team some more!$"

MatchCall_HOENN_Cory_Battle::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, CORY.\p"
	.string "My POKéMON have grown a lot\n"
	.string "tougher since last time. I\p"
	.string "want to see how strong they've\n"
	.string "become with your POKéMON,\p"
	.string "{PLAYER}. So, let's have a\n"
	.string "battle! I'll be waiting for\p"
	.string "you around {STR_VAR_2}.$"
```

```asm
@ EDWIN - Collector, Route 110

MatchCall_HOENN_Edwin_General1::
	.string "It's EDWIN.\n"
	.string "So? Get any more POKéMON?\p"
	.string "If you catch a new POKéMON,\n"
	.string "you have to come show me. I\p"
	.string "won't whine for it, honest.\n"
	.string "I'll be waiting. See you.$"

MatchCall_HOENN_Edwin_General2::
	.string "…Uh, {PLAYER}?\n"
	.string "It's me, EDWIN.\p"
	.string "Oh, wait! Wait! I can catch\n"
	.string "this {STR_VAR_3}… Aaarrrgh! It\p"
	.string "bolted loose! That wasn't just\n"
	.string "close!$"

MatchCall_HOENN_Edwin_General3::
	.string "{PLAYER}?\n"
	.string "EDWIN here.\p"
	.string "My {STR_VAR_3} is a force! It\n"
	.string "won me another battle just\p"
	.string "now! I can't wait to have a\n"
	.string "rematch with you.$"

MatchCall_HOENN_Edwin_Battle::
	.string "…Er, {PLAYER}?\n"
	.string "EDWIN here…\p"
	.string "So? Are your POKéMON growing?\n"
	.string "Mine sure got stronger. I'd\p"
	.string "like to show you. I'll be\n"
	.string "around {STR_VAR_2}. Come\p"
	.string "see me for a match. See you\n"
	.string "around.$"
```

```asm
@ LYDIA - Pokémon Breeder, Route 117

MatchCall_HOENN_Lydia_General1::
	.string "Hi, this is LYDIA.\p"
	.string "I gave a treat to my\n"
	.string "{STR_VAR_3}. It seemed to enjoy\p"
	.string "it very much. It looks like\n"
	.string "POKéMON have their likes and\p"
	.string "dislikes with treats. I find\n"
	.string "that quite fascinating. Please\p"
	.string "do take care.$"

MatchCall_HOENN_Lydia_General2::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is LYDIA speaking.\p"
	.string "Have you had success catching\n"
	.string "POKéMON lately? I came very\p"
	.string "close a little while ago, but\n"
	.string "my target got free. I need to\p"
	.string "try harder! See you again!$"

MatchCall_HOENN_Lydia_General3::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is LYDIA speaking.\p"
	.string "I hope you've been well. I\n"
	.string "wanted to tell you I just won.\p"
	.string "My {STR_VAR_3} worked\n"
	.string "especially hard to get the\p"
	.string "win. See you again!$"

MatchCall_HOENN_Lydia_Battle::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is LYDIA speaking.\p"
	.string "I hope you're doing well. My\n"
	.string "POKéMON are very frisky. If\p"
	.string "you're ever in the area,\n"
	.string "please give me a rematch. I'll\p"
	.string "be around {STR_VAR_2}.\n"
	.string "Until then, good-bye!$"
```

```asm
@ ISAAC - Pokémon Breeder, Route 117

MatchCall_HOENN_Isaac_General1::
	.string "This is ISAAC.\p"
	.string "Are you raising your POKéMON\n"
	.string "in the optimal way? The air is\p"
	.string "clean out on ROUTE 117. It's\n"
	.string "the perfect environment for\p"
	.string "raising POKéMON. If you're\n"
	.string "going to focus on raising\p"
	.string "POKéMON, you should come out\n"
	.string "to the country sometime. Take\p"
	.string "care now.$"

MatchCall_HOENN_Isaac_General2::
	.string "Hiya, {PLAYER}!\n"
	.string "It's ISAAC.\p"
	.string "Catching any POKéMON lately? A\n"
	.string "little while ago I came close\p"
	.string "to nabbing one, but it got\n"
	.string "loose. Right, take care!$"

MatchCall_HOENN_Isaac_General3::
	.string "Hiya, {PLAYER}!\n"
	.string "It's ISAAC.\p"
	.string "How are things with you?\n"
	.string "Battling much? I just won a\p"
	.string "while back! My {STR_VAR_3} was\n"
	.string "brilliant! You wait. I'm going\p"
	.string "to beat you next time! Right,\n"
	.string "take care!$"

MatchCall_HOENN_Isaac_Battle::
	.string "Hiya, {PLAYER}!\n"
	.string "It's ISAAC.\p"
	.string "My POKéMON are growing up in\n"
	.string "decent ways. I'd really like\p"
	.string "to have another battle with\n"
	.string "you. I'll keep an eye out for\p"
	.string "you around {STR_VAR_2}.\n"
	.string "See you soon!$"
```

```asm
@ GABRIELLE - Pokémon Breeder, Mt. Pyre

MatchCall_HOENN_Gabrielle_General1::
	.string "This is GABRIELLE.\n"
	.string "Hello.\p"
	.string "I was just telling a new\n"
	.string "TRAINER about you. I told her\p"
	.string "about a strong TRAINER who\n"
	.string "raises POKéMON with care. I\p"
	.string "hope you'll become a TRAINER\n"
	.string "that everyone will admire. I\p"
	.string "hope we meet again!$"

MatchCall_HOENN_Gabrielle_General2::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is GABRIELLE speaking.\p"
	.string "How are your POKéMON doing? I\n"
	.string "lost a match the other day. I\p"
	.string "need to try harder! See you\n"
	.string "again!$"

MatchCall_HOENN_Gabrielle_General3::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is GABRIELLE speaking.\p"
	.string "I hope you've been well. I\n"
	.string "wanted to tell you I just won.\p"
	.string "My {STR_VAR_3} worked\n"
	.string "especially hard to get the\p"
	.string "win. See you again!$"

MatchCall_HOENN_Gabrielle_Battle::
	.string "Oh, {PLAYER}, how do you do?\n"
	.string "This is GABRIELLE speaking.\p"
	.string "I hope you're doing well. My\n"
	.string "POKéMON are very frisky. If\p"
	.string "you're ever in the area,\n"
	.string "please give me a rematch. I'll\p"
	.string "be around {STR_VAR_2}.\n"
	.string "Until then, good-bye!$"
```

```asm
@ CATHERINE - Pokémon Ranger, Route 119

MatchCall_HOENN_Catherine_General1::
	.string "Hi, it's CATHERINE. You know,\n"
	.string "the TRAINER who's always\p"
	.string "prepared! {PLAYER}, do you have\n"
	.string "enough items? Are your POKéMON\p"
	.string "fit for action? Keeping\n"
	.string "everything perfect around you\p"
	.string "all the time is the secret to\n"
	.string "keeping your journey going.\p"
	.string "I'd better go check my own\n"
	.string "supplies! Be vigilant!$"

MatchCall_HOENN_Catherine_General2::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is CATHERINE!\p"
	.string "Listen, listen, you have to\n"
	.string "hear this! I had a POKéMON\p"
	.string "battle earlier, but I lost at\n"
	.string "the last second. Oh, it burns\p"
	.string "me up!$"

MatchCall_HOENN_Catherine_General3::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is CATHERINE!\p"
	.string "How are your POKéMON holding\n"
	.string "up? Mine just won a battle! My\p"
	.string "{STR_VAR_3} was spectacular, I\n"
	.string "must say! I wish I could've\p"
	.string "shown you! See you again!$"

MatchCall_HOENN_Catherine_Battle::
	.string "Oh, {PLAYER}, hi there!\n"
	.string "This is CATHERINE!\p"
	.string "How are your POKéMON doing? My\n"
	.string "POKéMON keep getting better.\p"
	.string "I'd like to show you, {PLAYER}.\n"
	.string "I'm around {STR_VAR_2}\p"
	.string "now, so let's battle if you're\n"
	.string "close by. Hope I see you soon!$"
```

```asm
@ JACKSON - Pokémon Ranger, Route 119

MatchCall_HOENN_Jackson_General1::
	.string "JACKSON here.\p"
	.string "If you cooperate with POKéMON,\n"
	.string "one can be comfortable in the\p"
	.string "wild. Everyone should realize\n"
	.string "that and cooperate with\p"
	.string "POKéMON more. That would be\n"
	.string "ideal. I really think so. I\p"
	.string "think you're on the right\n"
	.string "track! Catch you later!$"

MatchCall_HOENN_Jackson_General2::
	.string "Hiya, {PLAYER}!\n"
	.string "It's JACKSON.\p"
	.string "How are things with you? I've\n"
	.string "been battling on, but I\p"
	.string "haven't won very often. I\n"
	.string "can't get it together. Right,\p"
	.string "take care!$"

MatchCall_HOENN_Jackson_General3::
	.string "Hiya, {PLAYER}!\n"
	.string "It's JACKSON.\p"
	.string "How are things with you?\n"
	.string "Battling much? I just won a\p"
	.string "while back! My {STR_VAR_3} was\n"
	.string "brilliant! You wait. I'm going\p"
	.string "to beat you next time! Right,\n"
	.string "take care!$"

MatchCall_HOENN_Jackson_Battle::
	.string "Hiya, {PLAYER}!\n"
	.string "It's JACKSON.\p"
	.string "My POKéMON are growing up in\n"
	.string "decent ways. I'd really like\p"
	.string "to have another battle with\n"
	.string "you. I'll keep an eye out for\p"
	.string "you around {STR_VAR_2}.\n"
	.string "See you soon!$"
```

```asm
@ HALEY - Lass, Route 104

MatchCall_HOENN_Haley_General1::
	.string "It's HALEY! It's HALEY!\p"
	.string "ROUTE 104 is a very busy\n"
	.string "thoroughfare, so I get\p"
	.string "challenged by all kinds of\n"
	.string "TRAINERS every day. Some days\p"
	.string "I win five battles and lose\n"
	.string "only three! How did you do\p"
	.string "today? Tell me about it next\n"
	.string "time, okay?$"

MatchCall_HOENN_Haley_General2::
	.string "Hello, {PLAYER}.\n"
	.string "It's HALEY.\p"
	.string "I challenged someone else\n"
	.string "after we battled. I came\p"
	.string "close, but I ended up losing.\n"
	.string "Oh, well!$"

MatchCall_HOENN_Haley_General3::
	.string "Hello, {PLAYER}!\n"
	.string "It's HALEY!\p"
	.string "I had a battle yesterday and I\n"
	.string "won! It's fantastic!$"

MatchCall_HOENN_Haley_Battle::
	.string "Hello, {PLAYER}!\n"
	.string "It's HALEY.\p"
	.string "Would you like to have a\n"
	.string "battle with me again? You can\p"
	.string "find me around\n"
	.string "{STR_VAR_2}. I'll be\p"
	.string "waiting!$"
```

```asm
@ JAMES - Bug Catcher, Petalburg Woods

MatchCall_HOENN_James_General1::
	.string "It's me, JAMES.\p"
	.string "I'm popular because I have\n"
	.string "lots of BUG POKéMON, right?\p"
	.string "Well, I took a bunch of my\n"
	.string "fave bugs to school today.\p"
	.string "This girl I like started\n"
	.string "crying! Go ahead and laugh if\p"
	.string "you want. I have to try\n"
	.string "teaching her what makes BUG\p"
	.string "POKéMON so appealing. Snivel…\n"
	.string "See you!$"

MatchCall_HOENN_James_General2::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is JAMES.\p"
	.string "I saw this {STR_VAR_3} a while\n"
	.string "back but I couldn't catch it.\p"
	.string "It was so close, too! Well,\n"
	.string "see you again!$"

MatchCall_HOENN_James_General3::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is JAMES!\p"
	.string "I battled another TRAINER\n"
	.string "earlier. I won! I won! My\p"
	.string "{STR_VAR_3} really worked hard\n"
	.string "for me. This is so great!$"

MatchCall_HOENN_James_Battle::
	.string "Hi! {PLAYER}, hello!\n"
	.string "This is JAMES.\p"
	.string "Want to have a battle with me?\n"
	.string "I'll be waiting for you around\p"
	.string "{STR_VAR_2}!$"
```

```asm
@ TRENT - Hiker, Route 112

MatchCall_HOENN_Trent_General1::
	.string "Hah! Hah! Hah! Hah!\p"
	.string "Hi! It's TRENT! Hah! Hah!\n"
	.string "Trying to chat… While\p"
	.string "climbing… Is harsh exercise…\n"
	.string "Hah! Hah! Urgh! Oof… It's\p"
	.string "steeper now… We'll\n"
	.string "chat…another time… Hah! Hah!\p"
	.string "Hah!$"

MatchCall_HOENN_Trent_General2::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, TRENT.\p"
	.string "I just took a shot at catching\n"
	.string "this {STR_VAR_3}, but it took\p"
	.string "off. I came oh so close, too!\n"
	.string "It spoiled my day… All right,\p"
	.string "see you!$"

MatchCall_HOENN_Trent_General3::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, TRENT!\p"
	.string "How's your battling? Me, I had\n"
	.string "a battle the other day, and my\p"
	.string "{STR_VAR_3} came up huge! The\n"
	.string "next time I battle you,\p"
	.string "{PLAYER}, it won't be me\n"
	.string "losing!$"

MatchCall_HOENN_Trent_Battle::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, TRENT.\p"
	.string "My POKéMON have grown a lot\n"
	.string "tougher since last time. I\p"
	.string "want to see how strong they've\n"
	.string "become with your POKéMON,\p"
	.string "{PLAYER}. So, let's have a\n"
	.string "battle! I'll be waiting for\p"
	.string "you around {STR_VAR_2}.$"
```

```asm
@ SAWYER - Hiker, Mt. Chimney

MatchCall_HOENN_Sawyer_General1::
	.string "It's me, the mountain-loving\n"
	.string "SAWYER! Well, since we met,\p"
	.string "have you grown to appreciate\n"
	.string "the mountains more? I rarely\p"
	.string "see you in the mountains… Next\n"
	.string "time, let's meet up on a\p"
	.string "mountain trail somewhere.$"

MatchCall_HOENN_Sawyer_General2::
	.string "Hey, {PLAYER}!\n"
	.string "This is SAWYER!\p"
	.string "I've been thinking about\n"
	.string "trying to catch me some\p"
	.string "POKéMON. But I can't seem to\n"
	.string "find any. It's a real puzzler\p"
	.string "for me! I'm at my wit's end!\n"
	.string "See you around!$"

MatchCall_HOENN_Sawyer_General3::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, SAWYER!\p"
	.string "How's your battling? Me, I had\n"
	.string "a battle the other day, and my\p"
	.string "{STR_VAR_3} came up huge! The\n"
	.string "next time I battle you,\p"
	.string "{PLAYER}, it won't be me\n"
	.string "losing!$"

MatchCall_HOENN_Sawyer_Battle::
	.string "Hey there, {PLAYER}!\n"
	.string "It's me, SAWYER.\p"
	.string "My POKéMON have grown a lot\n"
	.string "tougher since last time. I\p"
	.string "want to see how strong they've\n"
	.string "become with your POKéMON,\p"
	.string "{PLAYER}. So, let's have a\n"
	.string "battle! I'll be waiting for\p"
	.string "you around {STR_VAR_2}.$"
```

```asm
@ KIRA & DAN - Young Couple, Abandoned Ship

MatchCall_HOENN_KiraAndDan_General1::
	.string "Oh, hi! It's KIRA & DAN!\p"
	.string "We're still searching for\n"
	.string "treasure together! Maybe there\p"
	.string "isn't any treasure to find at\n"
	.string "all… But the important thing\p"
	.string "is to search side by side. Oh,\n"
	.string "DAN, why are you pouting? I'm\p"
	.string "not ignoring you, honey!\n"
	.string "You're my one and only!\p"
	.string "…{PLAYER}, we have to go, bye!$"

MatchCall_HOENN_KiraAndDan_General2::
	.string "Oh, {PLAYER}, hi there!\p"
	.string "This is KIRA & DAN! So? Are\n"
	.string "you getting more POKéMON\p"
	.string "together? We're having a\n"
	.string "rotten time of it! They all\p"
	.string "get away from us! See you!$"

MatchCall_HOENN_KiraAndDan_General3::
	.string "Oh, {PLAYER}, hi there!\p"
	.string "This is KIRA & DAN! How are\n"
	.string "your POKéMON holding up? Ours\p"
	.string "just won a battle! Our\n"
	.string "{STR_VAR_3} was spectacular, we\p"
	.string "must say! We wish we could've\n"
	.string "shown you! See you again!$"

MatchCall_HOENN_KiraAndDan_Battle::
	.string "Oh, {PLAYER}, hi there!\p"
	.string "This is KIRA & DAN! How are\n"
	.string "your POKéMON doing? Our\p"
	.string "POKéMON keep getting better.\n"
	.string "We'd like to show you,\p"
	.string "{PLAYER}. We're around\n"
	.string "{STR_VAR_2} now, so let's\p"
	.string "battle if you're close by.\n"
	.string "Hope we see you soon!$"
```

## Sevii and coast call texts

Labels: `MatchCall_<Group>_<Name>_{General1,General2,General3,Battle}`. Pair contacts speak as the pair. Battle texts use `{STR_VAR_2}`.

| Group | Name | Trainer constant(s) | Class | Home map | Voice note |
|---|---|---|---|---|---|
| SEVII | Finn | TRAINER_WAYFARER_SEVII_SWIMMER_MALE_FINN (+_2) | Swimmer | One Island, Kindle Road | Starmie; looks up at MT. EMBER and the big sky |
| SEVII | Sharon | TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON (+_2, _3) | Crush Girl | One Island, Kindle Road | Mankey line; wants sparring partners, admires skilled people |
| SEVII | Tanya | TRAINER_WAYFARER_SEVII_CRUSH_GIRL_TANYA (+_2, _3) | Crush Girl | One Island, Kindle Road | Daily-training devotion; Hitmon pair; competes with Sharon |
| SEVII | Shea | TRAINER_WAYFARER_SEVII_BLACK_BELT_SHEA (+_2, _3) | Black Belt | One Island, Kindle Road | Black Belt; swims laps around the island every morning |
| SEVII | Hugh | TRAINER_WAYFARER_SEVII_BLACK_BELT_HUGH (+_2, _3) | Black Belt | One Island, Kindle Road | Black Belt; stickler for proper battle attire |
| SEVII | MikKia | TRAINER_WAYFARER_SEVII_CRUSH_KIN_MIK_KIA (+_2, _3) | Crush Kin | One Island, Kindle Road | Siblings (Kia loud about big brother Mik); speak as the pair |
| SEVII | Amira | TRAINER_WAYFARER_SEVII_TUBER_AMIRA (+_2) | Tuber | Three Island, Bond Bridge | Little kid with a float ring; learning to swim |
| SEVII | JoyMeg | TRAINER_WAYFARER_SEVII_TWINS_JOY_MEG (+_2) | Twins | Three Island, Bond Bridge | Twin girls; Clefairy fans; finish each other's sentences |
| SEVII | Rayna | TRAINER_WAYFARER_SEVII_PAINTER_RAYNA (+_2) | Painter | Five Island, Resort Gorgeous | Dreamy painter chasing the right angle; Smeargle |
| SEVII | Destin | TRAINER_WAYFARER_SEVII_YOUNGSTER_DESTIN (+_2) | Youngster | Five Island, Resort Gorgeous | Fast runner; loves wind; Pidgeot/Raticate |
| SEVII | Alize | TRAINER_WAYFARER_SEVII_PKMN_BREEDER_ALIZE (+_2) | Pokémon Breeder | Five Island, Water Labyrinth | Gentle Breeder; Azumarill, Clefable, Raichu |
| SEVII | Milo | TRAINER_WAYFARER_SEVII_BIRD_KEEPER_MILO (+_2) | Bird Keeper | Five Island, Memorial Pillar | Eldest Bird Brother; obsessed with beaks |
| SEVII | Chaz | TRAINER_WAYFARER_SEVII_BIRD_KEEPER_CHAZ (+_2) | Bird Keeper | Five Island, Memorial Pillar | Middle Bird Brother; wing enthusiast |
| SEVII | Harold | TRAINER_WAYFARER_SEVII_BIRD_KEEPER_HAROLD (+_2) | Bird Keeper | Five Island, Memorial Pillar | Youngest Bird Brother; sleepy, loves down, Hoothoot/Noctowl |
| SEVII | Nicole | TRAINER_WAYFARER_SEVII_SWIMMER_FEMALE_NICOLE (+_2) | Swimmer | Six Island, Outcast Island | Swimmer who finds mid-swim sending-out awkward |
| SEVII | Jaclyn | TRAINER_WAYFARER_SEVII_PSYCHIC_JACLYN (+_2) | Psychic | Six Island, Green Path | Scatterbrained Psychic whose TELEPORT goes astray |
| SEVII | Samir | TRAINER_WAYFARER_SEVII_SWIMMER_MALE_SAMIR (+_2) | Swimmer | Six Island, Water Path | Swimmer chasing off the SWIMMER stigma |
| SEVII | Earl | TRAINER_WAYFARER_SEVII_HIKER_EARL (+_2) | Hiker | Six Island, Water Path | Hiker obsessed with finding mountains; keeps getting lost |
| SEVII | Larry | TRAINER_WAYFARER_SEVII_RUIN_MANIAC_LARRY (+_2) | Ruin Maniac | Six Island, Ruin Valley | Ruins obsessive; mysterious stones; cannot find the door |
| SEVII | Hector | TRAINER_WAYFARER_SEVII_POKEMANIAC_HECTOR (+_2) | Pokémaniac | Six Island, Ruin Valley | Knows the local land; door pattern lore; collector |
| SEVII | Dario | TRAINER_WAYFARER_SEVII_PSYCHIC_DARIO (+_2) | Psychic | Seven Island, Trainer Tower | Quiet, eerie Psychic; senses calls before they arrive |
| SEVII | Rodette | TRAINER_WAYFARER_SEVII_PSYCHIC_RODETTE (+_2) | Psychic | Seven Island, Trainer Tower | Dreamy Psychic; sleeping Pokémon legends |
| SEVII | Nicolas | TRAINER_WAYFARER_SEVII_PKMN_RANGER_NICOLAS (+_2) | Pokémon Ranger | Seven Island, Sevault Canyon | Patrol ranger; island too spread out; Victreebel |
| SEVII | Madeline | TRAINER_WAYFARER_SEVII_PKMN_RANGER_MADELINE (+_2) | Pokémon Ranger | Seven Island, Sevault Canyon | Strict, kind-hearted ranger; punishes POKéMON abusers |
| SEVII | Mason | TRAINER_WAYFARER_SEVII_JUGGLER_MASON (+_2) | Juggler | Seven Island, Sevault Canyon | Showman with a fan-club complex; sings |
| SEVII | Cyndy | TRAINER_WAYFARER_SEVII_CRUSH_GIRL_CYNDY (+_2) | Crush Girl | Seven Island, Sevault Canyon | Fitness-first Crush Girl who might hold off on TRAINER TOWER |
| SEVII | Evan | TRAINER_WAYFARER_SEVII_TAMER_EVAN (+_2) | Tamer | Seven Island, Sevault Canyon | Lecturing Tamer; trainers must grow up too |
| SEVII | Jackson | TRAINER_WAYFARER_SEVII_PKMN_RANGER_JACKSON (+_2) | Pokémon Ranger | Seven Island, Sevault Canyon | Nature-protecting Ranger; planet as a drop of water |
| SEVII | Katelyn | TRAINER_WAYFARER_SEVII_PKMN_RANGER_KATELYN (+_2) | Pokémon Ranger | Seven Island, Sevault Canyon | Teasing, fashion-minded ranger; loves RUNNING SHOES |
| SEVII | Leroy | TRAINER_WAYFARER_SEVII_COOLTRAINER_LEROY (+_2) | Cooltrainer | Seven Island, Sevault Canyon | Data-minded Cooltrainer who asks for battles to learn |
| SEVII | Michelle | TRAINER_WAYFARER_SEVII_COOLTRAINER_MICHELLE (+_2) | Cooltrainer | Seven Island, Sevault Canyon | Highly educated, hot-tempered Cooltrainer practicing calm |
| SEVII | LexNya | TRAINER_WAYFARER_SEVII_COOL_COUPLE_LEX_NYA (+_2) | Cool Couple | Seven Island, Sevault Canyon | Elegant couple; speak as the pair |
| COAST | Alice | TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_ALICE (+_2) | Swimmer | Route 19 | Sun-wary Swimmer; talks about sunscreen and boyfriend's Seafoam swim |
| COAST | Darrin | TRAINER_WAYFARER_COAST_SWIMMER_MALE_DARRIN (+_2) | Swimmer | Route 20 | Swimmer baffled by people riding POKéMON |
| COAST | Missy | TRAINER_WAYFARER_COAST_PICNICKER_MISSY (+_2, _3) | Picnicker | Route 20 | Picnicker who swam from Cinnabar; mentions the mansion |
| COAST | Wade | TRAINER_WAYFARER_COAST_FISHERMAN_WADE (+_2) | Fisherman | Route 21 North | Fisherman who only catches MAGIKARP |
| COAST | Jack | TRAINER_WAYFARER_COAST_SWIMMER_MALE_JACK (+_2) | Swimmer | Route 21 South | Sea-caught Pokémon; dive jokes |
| COAST | LilIan | TRAINER_WAYFARER_COAST_SIS_AND_BRO_LIL_IAN (+_2, _3) | Sis and Bro | Route 21 North | Reluctant LIL (tired), cheerful IAN; speak as pair |
| COAST | Matthew | TRAINER_WAYFARER_COAST_SWIMMER_MALE_MATTHEW (+_2) | Swimmer | Route 19 | Overconfident Swimmer who swims better than marine Pokémon |
| COAST | Tony | TRAINER_WAYFARER_COAST_SWIMMER_MALE_TONY (+_2) | Swimmer | Route 19 | Stares at the sea to forget something bad; gentle humor |
| COAST | Melissa | TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_MELISSA (+_2) | Swimmer | Route 20 | Daughter of a CINNABAR LAB worker; volcano-island facts |

```asm
@ Finn - Swimmer, One Island, Kindle Road
MatchCall_SEVII_Finn_General1::
	.string "{PLAYER}, it's FINN!\n"
	.string "Floating on my back again.\p"
	.string "Out here MT. EMBER looks like\n"
	.string "it holds up the whole sky.\p"
	.string "Staring up is half my day.$"

MatchCall_SEVII_Finn_General2::
	.string "Hey, {PLAYER}, FINN here.\n"
	.string "Water's calm, mind's calm.\p"
	.string "My {STR_VAR_3} spins through\n"
	.string "the waves like a pinwheel.\p"
	.string "I swear it hums when the\n"
	.string "tide goes out.$"

MatchCall_SEVII_Finn_General3::
	.string "FINN calling, {PLAYER}!\n"
	.string "I just counted clouds.\p"
	.string "Forty-one, and the sky still\n"
	.string "had room for more.\p"
	.string "You ought to try floating\n"
	.string "sometime.$"

MatchCall_SEVII_Finn_Battle::
	.string "{PLAYER}! FINN!\n"
	.string "The sea's glassy today.\p"
	.string "I'd love a rematch, and\n"
	.string "I'm out at {STR_VAR_2}.\p"
	.string "Come make a splash with me!$"

@ Sharon - Crush Girl, One Island, Kindle Road
MatchCall_SEVII_Sharon_General1::
	.string "Hi, {PLAYER}! It's SHARON.\n"
	.string "I did a hundred punches.\p"
	.string "Then I did a hundred more.\n"
	.string "My {STR_VAR_3} kept count!$"

MatchCall_SEVII_Sharon_General2::
	.string "{PLAYER}, SHARON here!\n"
	.string "I'm still hunting for a\l"
	.string "good sparring partner.\p"
	.string "My {STR_VAR_3} hits hard, but\n"
	.string "it needs someone sharper.$"

MatchCall_SEVII_Sharon_General3::
	.string "SHARON again, {PLAYER}!\n"
	.string "I filmed my own form\l"
	.string "in a puddle today.\p"
	.string "My elbow droops. I'll fix\n"
	.string "that before we meet again.$"

MatchCall_SEVII_Sharon_Battle::
	.string "It's SHARON, {PLAYER}!\n"
	.string "Help me out with my training?\p"
	.string "I'm at {STR_VAR_2} now.\n"
	.string "Don't hold back, okay?$"

@ Tanya - Crush Girl, One Island, Kindle Road
MatchCall_SEVII_Tanya_General1::
	.string "{PLAYER}? TANYA!\n"
	.string "Sorry, I'm mid-set.\p"
	.string "Not one day off this year.\n"
	.string "My {STR_VAR_3} says the same!$"

MatchCall_SEVII_Tanya_General2::
	.string "TANYA calling, {PLAYER}.\n"
	.string "Rest days make me restless.\p"
	.string "So I train through them,\n"
	.string "then train some more.$"

MatchCall_SEVII_Tanya_General3::
	.string "Hello, {PLAYER}, TANYA.\n"
	.string "I keep losing in my head!\p"
	.string "Replaying our match, I find\n"
	.string "new mistakes every time.$"

MatchCall_SEVII_Tanya_Battle::
	.string "{PLAYER}! TANYA!\n"
	.string "I'm itching to prove myself.\p"
	.string "Meet me at {STR_VAR_2}!\n"
	.string "I won't be a fool again!$"

@ Shea - Black Belt, One Island, Kindle Road
MatchCall_SEVII_Shea_General1::
	.string "Ahem. SHEA calling, {PLAYER}.\n"
	.string "Morning laps are done.\p"
	.string "Around the island, then\n"
	.string "a swim before breakfast.$"

MatchCall_SEVII_Shea_General2::
	.string "{PLAYER}, SHEA here.\n"
	.string "A strong body needs a\l"
	.string "strong breakfast.\p"
	.string "My {STR_VAR_3} eats three\n"
	.string "bowls. I try to keep up.$"

MatchCall_SEVII_Shea_General3::
	.string "SHEA, {PLAYER}. Hah!\n"
	.string "I outran a Tauros at dawn.\p"
	.string "Well, it ran off first,\n"
	.string "but I counted it a win.$"

MatchCall_SEVII_Shea_Battle::
	.string "{PLAYER}, it is SHEA.\n"
	.string "Care for a bout?\p"
	.string "I wait at {STR_VAR_2}.\n"
	.string "Bring your best spirit!$"

@ Hugh - Black Belt, One Island, Kindle Road
MatchCall_SEVII_Hugh_General1::
	.string "HUGH speaking, {PLAYER}.\n"
	.string "Are you dressed properly?\p"
	.string "A fighter's outfit shows\n"
	.string "a fighter's resolve.$"

MatchCall_SEVII_Hugh_General2::
	.string "{PLAYER}, HUGH here.\n"
	.string "I ironed my belt today.\p"
	.string "My {STR_VAR_3} thinks it's\n"
	.string "silly, but it respects form.$"

MatchCall_SEVII_Hugh_General3::
	.string "This is HUGH, {PLAYER}.\n"
	.string "Folks ask why I never\l"
	.string "wear anything flashy.\p"
	.string "Plain cloth, clear mind.\p"
	.string "That's the whole secret.$"

MatchCall_SEVII_Hugh_Battle::
	.string "{PLAYER}! HUGH!\n"
	.string "I stand at {STR_VAR_2}.\p"
	.string "Wear a proper outfit, and\n"
	.string "we shall clash!$"

@ MikKia - Crush Kin, One Island, Kindle Road
MatchCall_SEVII_MikKia_General1::
	.string "MIK: {PLAYER}. It's us.\p"
	.string "KIA: Hi, hi, hi!\n"
	.string "MIK is the best, you know!\p"
	.string "MIK: Please stop, KIA.$"

MatchCall_SEVII_MikKia_General2::
	.string "KIA: {PLAYER}, it's KIA!\n"
	.string "MIK and I did a\l"
	.string "team throw today.\p"
	.string "MIK: She landed on me.\p"
	.string "KIA: Fine, I landed WITH him!$"

MatchCall_SEVII_MikKia_General3::
	.string "MIK: Greetings, {PLAYER}.\p"
	.string "KIA: My brother's {STR_VAR_3}\n"
	.string "is so strong, so cool.\p"
	.string "MIK: Mine is just trained.$"

MatchCall_SEVII_MikKia_Battle::
	.string "KIA: Hey, {PLAYER}!\n"
	.string "MIK wants a rematch.\p"
	.string "MIK: It's {STR_VAR_2}.\n"
	.string "KIA: Say yes! Say yes!$"

@ Amira - Tuber, Three Island, Bond Bridge
MatchCall_SEVII_Amira_General1::
	.string "H-hi, {PLAYER}! It's AMIRA!\p"
	.string "I put my face in the water\n"
	.string "yesterday. For one second!\p"
	.string "My {STR_VAR_3} clapped.$"

MatchCall_SEVII_Amira_General2::
	.string "{PLAYER}, hi! AMIRA!\n"
	.string "Mommy said I get a new\l"
	.string "float ring if I kick well.\p"
	.string "I kick really, really well!$"

MatchCall_SEVII_Amira_General3::
	.string "Hello, it's AMIRA!\n"
	.string "I counted my floaties.\p"
	.string "I have two. Mommy has none.\n"
	.string "She says they're mine.$"

MatchCall_SEVII_Amira_Battle::
	.string "{PLAYER}! It's AMIRA!\n"
	.string "Play at {STR_VAR_2}?\p"
	.string "I promise I won't cry!\n"
	.string "Maybe a little. Okay!$"

@ JoyMeg - Twins, Three Island, Bond Bridge
MatchCall_SEVII_JoyMeg_General1::
	.string "JOY: Hi, {PLAYER}!\n"
	.string "MEG: It's both of us!\p"
	.string "JOY: We wore the same bow.\p"
	.string "MEG: By accident! Again!$"

MatchCall_SEVII_JoyMeg_General2::
	.string "MEG: {PLAYER}, hello!\n"
	.string "JOY: We were just talking\l"
	.string "about your {STR_VAR_3}.\p"
	.string "MEG: It looks lovely.\p"
	.string "JOY: We like lovely things!$"

MatchCall_SEVII_JoyMeg_General3::
	.string "JOY: It's JOY, {PLAYER}!\n"
	.string "MEG: And MEG!\p"
	.string "JOY: We picked a lullaby\n"
	.string "for our CLEFAIRY.\p"
	.string "MEG: It fell asleep first!$"

MatchCall_SEVII_JoyMeg_Battle::
	.string "MEG: {PLAYER}, guess what!\n"
	.string "JOY: We found you a spot.\p"
	.string "MEG: {STR_VAR_2}!\p"
	.string "JOY: Come see our favorite!$"

@ Rayna - Painter, Five Island, Resort Gorgeous
MatchCall_SEVII_Rayna_General1::
	.string "Ah, {PLAYER}. RAYNA, it is.\p"
	.string "The light, it was perfect.\p"
	.string "Then a cloud, it moved.\n"
	.string "Alas, my canvas, it wept.$"

MatchCall_SEVII_Rayna_General2::
	.string "{PLAYER}, RAYNA speaking.\p"
	.string "My {STR_VAR_3}, it posed\n"
	.string "so well for my brush today.\p"
	.string "It scowled when I painted\n"
	.string "its nose. Artists, they sulk.$"

MatchCall_SEVII_Rayna_General3::
	.string "Hello, {PLAYER}. RAYNA.\p"
	.string "I mixed a blue for the sea.\n"
	.string "It was not the sea's blue.\p"
	.string "I begin once more.$"

MatchCall_SEVII_Rayna_Battle::
	.string "{PLAYER}, it is RAYNA.\p"
	.string "The muse, she led me to\n"
	.string "{STR_VAR_2}.\p"
	.string "Come, and we shall make\n"
	.string "art with our fists.$"

@ Destin - Youngster, Five Island, Resort Gorgeous
MatchCall_SEVII_Destin_General1::
	.string "{PLAYER}! DESTIN!\n"
	.string "I ran here so fast, my\l"
	.string "hat went home without me.\p"
	.string "Ha ha, it's behind me!$"

MatchCall_SEVII_Destin_General2::
	.string "Yo, {PLAYER}, DESTIN!\n"
	.string "Raced my {STR_VAR_3} and\l"
	.string "I won by a nose!\p"
	.string "Okay, it let me win.$"

MatchCall_SEVII_Destin_General3::
	.string "It's DESTIN, {PLAYER}!\n"
	.string "I got new running shoes.\p"
	.string "They squeak, but squeaking\n"
	.string "means speed, right?$"

MatchCall_SEVII_Destin_Battle::
	.string "{PLAYER}! Hurry up!\n"
	.string "I'm at {STR_VAR_2}.\l"
	.string "Race me there, then battle!$"

@ Alize - Pokémon Breeder, Five Island, Water Labyrinth
MatchCall_SEVII_Alize_General1::
	.string "Hello, {PLAYER}. It's ALIZE.\p"
	.string "It's so quiet here that\n"
	.string "you hear them breathe.\p"
	.string "My {STR_VAR_3} naps beside me.$"

MatchCall_SEVII_Alize_General2::
	.string "{PLAYER}, ALIZE here.\p"
	.string "Soft food, warm water, and\n"
	.string "lots of patience. That's all.\p"
	.string "A good BREEDER listens.$"

MatchCall_SEVII_Alize_General3::
	.string "This is ALIZE, {PLAYER}.\p"
	.string "I've been brushing my\n"
	.string "POKéMON for an hour.\p"
	.string "{STR_VAR_3} purrs when I reach\n"
	.string "the spot behind its ear.$"

MatchCall_SEVII_Alize_Battle::
	.string "{PLAYER}? ALIZE.\p"
	.string "Fancy a check-up battle?\p"
	.string "We're at {STR_VAR_2}.\n"
	.string "Bring your well-fed team.$"

@ Milo - Bird Keeper, Five Island, Memorial Pillar
MatchCall_SEVII_Milo_General1::
	.string "{PLAYER}! MILO, eldest\n"
	.string "of the BIRD BROTHERS!\p"
	.string "I was studying beaks again.\n"
	.string "Sharp, perfect, elegant.$"

MatchCall_SEVII_Milo_General2::
	.string "Hey, {PLAYER}, MILO here.\p"
	.string "My {STR_VAR_3} cracked a nut\n"
	.string "with a single peck.\p"
	.string "That's craftsmanship!$"

MatchCall_SEVII_Milo_General3::
	.string "MILO calling, {PLAYER}!\n"
	.string "The younger two squabble\l"
	.string "about wings and down.\p"
	.string "Beaks end the argument.$"

MatchCall_SEVII_Milo_Battle::
	.string "{PLAYER}! It's MILO!\n"
	.string "Rematch time. I'm at\l"
	.string "{STR_VAR_2}.\p"
	.string "Don't forget a helmet. My\n"
	.string "birds can really peck!$"

@ Chaz - Bird Keeper, Five Island, Memorial Pillar
MatchCall_SEVII_Chaz_General1::
	.string "Flap-flap, {PLAYER}! CHAZ!\p"
	.string "I watched wings catch the\n"
	.string "wind off the pillar.\p"
	.string "So smooth. So graceful!$"

MatchCall_SEVII_Chaz_General2::
	.string "{PLAYER}, CHAZ here!\n"
	.string "MILO talks about beaks,\l"
	.string "but wings are what fly.\p"
	.string "My {STR_VAR_3} agrees, loudly.$"

MatchCall_SEVII_Chaz_General3::
	.string "It's CHAZ, {PLAYER}!\n"
	.string "I tried flapping my arms\l"
	.string "today. Zero lift.\p"
	.string "Wings are harder than that.$"

MatchCall_SEVII_Chaz_Battle::
	.string "{PLAYER}, it's CHAZ!\n"
	.string "Take off for {STR_VAR_2}!\p"
	.string "My wings are warmed up.\n"
	.string "Are yours?$"

@ Harold - Bird Keeper, Five Island, Memorial Pillar
MatchCall_SEVII_Harold_General1::
	.string "...Oh. {PLAYER}. HAROLD.\p"
	.string "Sorry. I was napping in\n"
	.string "some down. So soft...$"

MatchCall_SEVII_Harold_General2::
	.string "{PLAYER}... HAROLD here.\p"
	.string "My {STR_VAR_3} fluffed up\n"
	.string "and I fell asleep on it.\p"
	.string "Best pillow in the world.$"

MatchCall_SEVII_Harold_General3::
	.string "HAROLD, {PLAYER}...\p"
	.string "The BIRD BROTHERS argue\n"
	.string "a lot. I just doze.\p"
	.string "Down is the quiet answer.$"

MatchCall_SEVII_Harold_Battle::
	.string "{PLAYER}... it's HAROLD.\p"
	.string "Battle me at {STR_VAR_2}.\p"
	.string "Wake me if I nod off, okay?$"

@ Nicole - Swimmer, Six Island, Outcast Island
MatchCall_SEVII_Nicole_General1::
	.string "NICOLE here, {PLAYER}.\n"
	.string "Tried sending out my POKéMON\l"
	.string "mid-stroke. Got soaked.\p"
	.string "Wait, I was already wet.$"

MatchCall_SEVII_Nicole_General2::
	.string "{PLAYER}, NICOLE speaking.\n"
	.string "The tide at OUTCAST ISLAND\l"
	.string "is so peaceful.\p"
	.string "My {STR_VAR_3} ignores it\n"
	.string "and splashes everyone.$"

MatchCall_SEVII_Nicole_General3::
	.string "This is NICOLE, {PLAYER}.\n"
	.string "Wore a new swimsuit today.\p"
	.string "My {STR_VAR_3} tried to\n"
	.string "nibble the strap. Rude.$"

MatchCall_SEVII_Nicole_Battle::
	.string "{PLAYER}! NICOLE!\n"
	.string "Want a rematch? I'll be\l"
	.string "at {STR_VAR_2}.\p"
	.string "I'll even stay on dry land!$"

@ Jaclyn - Psychic, Six Island, Green Path
MatchCall_SEVII_Jaclyn_General1::
	.string "{PLAYER}? Oh, JACLYN!\n"
	.string "I'm calling from... where\l"
	.string "am I? Never mind.\p"
	.string "I meant to be home.$"

MatchCall_SEVII_Jaclyn_General2::
	.string "JACLYN, {PLAYER}!\n"
	.string "I thought of a sandwich\l"
	.string "and TELEPORTed. Wrong spot.\p"
	.string "My {STR_VAR_3} looked so tired.$"

MatchCall_SEVII_Jaclyn_General3::
	.string "Hi, {PLAYER}. JACLYN.\p"
	.string "I misplaced my spoon again.\n"
	.string "It's bent. Psychic stuff.\p"
	.string "Probably in my pocket.$"

MatchCall_SEVII_Jaclyn_Battle::
	.string "{PLAYER}! JACLYN!\n"
	.string "I'm at {STR_VAR_2}.\p"
	.string "Hopefully. Let's battle!$"

@ Samir - Swimmer, Six Island, Water Path
MatchCall_SEVII_Samir_General1::
	.string "Hey, {PLAYER}, SAMIR!\n"
	.string "Don't groan. Not another\l"
	.string "SWIMMER, right?\p"
	.string "I'm trying to make it cool.$"

MatchCall_SEVII_Samir_General2::
	.string "{PLAYER}, SAMIR here.\n"
	.string "I did a fancy turn today\l"
	.string "and nobody clapped.\p"
	.string "My {STR_VAR_3} did. It always\n"
	.string "has my back.$"

MatchCall_SEVII_Samir_General3::
	.string "It's SAMIR, {PLAYER}.\n"
	.string "Tried a snorkel. Looked\l"
	.string "like a walrus.\p"
	.string "Still cooler than a swim cap!$"

MatchCall_SEVII_Samir_Battle::
	.string "{PLAYER}! SAMIR!\n"
	.string "Want to change my image?\p"
	.string "Battle me at {STR_VAR_2}!\n"
	.string "Splash, baby!$"

@ Earl - Hiker, Six Island, Water Path
MatchCall_SEVII_Earl_General1::
	.string "EARL calling, {PLAYER}!\p"
	.string "Still looking for the\n"
	.string "mountains. Mountains!\p"
	.string "Mountains must be somewhere.$"

MatchCall_SEVII_Earl_General2::
	.string "{PLAYER}, it's EARL.\n"
	.string "My {STR_VAR_3} is carrying\l"
	.string "my pack. I'm the guide.\p"
	.string "I think. We're a bit lost.$"

MatchCall_SEVII_Earl_General3::
	.string "Hey, {PLAYER}. EARL.\p"
	.string "Found a hill. A hill!\n"
	.string "Bigger than a molehill!\p"
	.string "It's no mountain, but still.$"

MatchCall_SEVII_Earl_Battle::
	.string "{PLAYER}! EARL here!\n"
	.string "I can see a trail near\l"
	.string "{STR_VAR_2}. Come battle!\p"
	.string "I'll find my way to you.$"

@ Larry - Ruin Maniac, Six Island, Ruin Valley
MatchCall_SEVII_Larry_General1::
	.string "LARRY here, {PLAYER}.\n"
	.string "These old carvings are\l"
	.string "driving me mad.\p"
	.string "Each line hides a secret.$"

MatchCall_SEVII_Larry_General2::
	.string "{PLAYER}, it's LARRY!\n"
	.string "I traced an old stone with\l"
	.string "a pencil all morning.\p"
	.string "My {STR_VAR_3} sniffed it\n"
	.string "and sneezed. Dust!$"

MatchCall_SEVII_Larry_General3::
	.string "LARRY, {PLAYER}!\n"
	.string "The mysterious stones may\l"
	.string "answer something deep.\p"
	.string "I just don't know the\n"
	.string "question yet.$"

MatchCall_SEVII_Larry_Battle::
	.string "{PLAYER}! LARRY!\n"
	.string "I'm at {STR_VAR_2} now,\l"
	.string "surrounded by old stones.\p"
	.string "Come battle, then help me\n"
	.string "read the walls.$"

@ Hector - Pokémaniac, Six Island, Ruin Valley
MatchCall_SEVII_Hector_General1::
	.string "Hey, {PLAYER}. HECTOR.\n"
	.string "I mapped another corner\l"
	.string "of the valley today.\p"
	.string "Every rock has a name.$"

MatchCall_SEVII_Hector_General2::
	.string "HECTOR here, {PLAYER}.\n"
	.string "My {STR_VAR_3} is my best\l"
	.string "find. Rare as can be.\p"
	.string "Don't tell the others!$"

MatchCall_SEVII_Hector_General3::
	.string "{PLAYER}, it's HECTOR.\n"
	.string "That odd pattern on the\l"
	.string "door still bugs me.\p"
	.string "Dots, lines, a circle...$"

MatchCall_SEVII_Hector_Battle::
	.string "{PLAYER}! HECTOR!\n"
	.string "I'm showing off my team at\l"
	.string "{STR_VAR_2}.\p"
	.string "Come on, I'll be happy to\n"
	.string "battle and tell you more.$"

@ Dario - Psychic, Seven Island, Trainer Tower
MatchCall_SEVII_Dario_General1::
	.string "...{PLAYER}. DARIO.\p"
	.string "I knew you would call.\p"
	.string "Not to show off. It simply\n"
	.string "arrived in my head.$"

MatchCall_SEVII_Dario_General2::
	.string "DARIO, {PLAYER}.\p"
	.string "My {STR_VAR_3} sensed a\n"
	.string "storm before I did.\p"
	.string "It does that. I'm pleased.$"

MatchCall_SEVII_Dario_General3::
	.string "Hello. DARIO.\p"
	.string "I felt a ripple today,\n"
	.string "like many roads touching.\p"
	.string "You were part of it.$"

MatchCall_SEVII_Dario_Battle::
	.string "{PLAYER}. DARIO.\p"
	.string "Come to {STR_VAR_2}.\p"
	.string "I already see the battle.\n"
	.string "Let us find out how it ends.$"

@ Rodette - Psychic, Seven Island, Trainer Tower
MatchCall_SEVII_Rodette_General1::
	.string "Hello, {PLAYER}. RODETTE.\n"
	.string "Last night I dreamed\l"
	.string "of something sleeping.\p"
	.string "It breathed in time with me.$"

MatchCall_SEVII_Rodette_General2::
	.string "{PLAYER}, RODETTE here.\n"
	.string "My {STR_VAR_3} hums when\l"
	.string "I dream. Does yours?\p"
	.string "Dreams are a shared sea.$"

MatchCall_SEVII_Rodette_General3::
	.string "This is RODETTE, {PLAYER}.\n"
	.string "Do you hear strange\l"
	.string "lullabies sometimes?\p"
	.string "I think they're calling.$"

MatchCall_SEVII_Rodette_Battle::
	.string "{PLAYER}, it's RODETTE.\n"
	.string "A dream told me you'd be\l"
	.string "at {STR_VAR_2}.\p"
	.string "Shall we see what's true?$"

@ Nicolas - Pokémon Ranger, Seven Island, Sevault Canyon
MatchCall_SEVII_Nicolas_General1::
	.string "NICOLAS reporting, {PLAYER}.\n"
	.string "Patrol round complete.\p"
	.string "This island just keeps\n"
	.string "stretching. My boots hurt!$"

MatchCall_SEVII_Nicolas_General2::
	.string "{PLAYER}, NICOLAS here.\n"
	.string "Found a trail full of\l"
	.string "prints I couldn't name.\p"
	.string "My {STR_VAR_3} knows them all.$"

MatchCall_SEVII_Nicolas_General3::
	.string "Ranger NICOLAS, {PLAYER}.\n"
	.string "Dry rations again today.\p"
	.string "If you see a food stand,\n"
	.string "call me first. Please.$"

MatchCall_SEVII_Nicolas_Battle::
	.string "{PLAYER}! NICOLAS!\n"
	.string "Patrol brought me to\l"
	.string "{STR_VAR_2}.\p"
	.string "Care for a quick battle\n"
	.string "on duty? Sure, why not.$"

@ Madeline - Pokémon Ranger, Seven Island, Sevault Canyon
MatchCall_SEVII_Madeline_General1::
	.string "This is MADELINE, {PLAYER}.\n"
	.string "Anyone mistreating POKéMON\l"
	.string "answers to me.\p"
	.string "Be good to yours, okay?$"

MatchCall_SEVII_Madeline_General2::
	.string "MADELINE here, {PLAYER}.\n"
	.string "My {STR_VAR_3} rests in\l"
	.string "the shade after a long day.\p"
	.string "Rest matters. Remember that.$"

MatchCall_SEVII_Madeline_General3::
	.string "{PLAYER}, it's MADELINE.\n"
	.string "I gave a lecture on\l"
	.string "gentle handling today.\p"
	.string "Three people nodded. Progress.$"

MatchCall_SEVII_Madeline_Battle::
	.string "{PLAYER}! MADELINE!\n"
	.string "Meet me at {STR_VAR_2}.\p"
	.string "Let me see how kind you\n"
	.string "are in battle.$"

@ Mason - Juggler, Seven Island, Sevault Canyon
MatchCall_SEVII_Mason_General1::
	.string "Howdy, {PLAYER}! MASON!\n"
	.string "Join my fan club yet?\p"
	.string "It's a small club. Just me,\n"
	.string "and my {STR_VAR_3}.$"

MatchCall_SEVII_Mason_General2::
	.string "Lalalah, {PLAYER}! MASON!\n"
	.string "I wrote a new verse today.\l"
	.string "It rhymes. Mostly.\p"
	.string "I sing: Balls fly, fans sigh!$"

MatchCall_SEVII_Mason_General3::
	.string "{PLAYER}, MASON speaking!\n"
	.string "I dropped a ball at\l"
	.string "the show. A gasp!\p"
	.string "Then I caught it. Applause!$"

MatchCall_SEVII_Mason_Battle::
	.string "{PLAYER}! MASON!\n"
	.string "Showtime at {STR_VAR_2}!\p"
	.string "Bring your cheering section\n"
	.string "and don't forget a smile!$"

@ Cyndy - Crush Girl, Seven Island, Sevault Canyon
MatchCall_SEVII_Cyndy_General1::
	.string "CYNDY here, {PLAYER}!\n"
	.string "Morning stretches done.\p"
	.string "I could touch my toes\n"
	.string "behind my head. Almost.$"

MatchCall_SEVII_Cyndy_General2::
	.string "{PLAYER}! It's CYNDY!\n"
	.string "My {STR_VAR_3} lifted a boulder\l"
	.string "for a warm-up.\p"
	.string "I lifted a rock too. Small.$"

MatchCall_SEVII_Cyndy_General3::
	.string "Hey, {PLAYER}. CYNDY.\p"
	.string "My protein shake exploded.\n"
	.string "Banana on the ceiling.\p"
	.string "Worth it. Still strong.$"

MatchCall_SEVII_Cyndy_Battle::
	.string "{PLAYER}, CYNDY!\n"
	.string "Tune up with me at\l"
	.string "{STR_VAR_2}!\p"
	.string "My conditioning is in top\n"
	.string "form. Is yours?$"

@ Evan - Tamer, Seven Island, Sevault Canyon
MatchCall_SEVII_Evan_General1::
	.string "EVAN speaking, {PLAYER}.\n"
	.string "A strong POKéMON is\l"
	.string "nothing without a TRAINER.\p"
	.string "Remember it.$"

MatchCall_SEVII_Evan_General2::
	.string "{PLAYER}, this is EVAN.\n"
	.string "My {STR_VAR_3} listens\l"
	.string "before I even say it.\p"
	.string "That's trust, not magic.$"

MatchCall_SEVII_Evan_General3::
	.string "EVAN again, {PLAYER}.\n"
	.string "A kid yelled at his\l"
	.string "POKéMON in the canyon.\p"
	.string "I handed him a lesson.$"

MatchCall_SEVII_Evan_Battle::
	.string "{PLAYER}. EVAN here.\n"
	.string "Come to {STR_VAR_2}.\p"
	.string "Show me you can use what\n"
	.string "you have. Seriously.$"

@ Jackson - Pokémon Ranger, Seven Island, Sevault Canyon
MatchCall_SEVII_Jackson_General1::
	.string "Hey, {PLAYER}, JACKSON!\n"
	.string "I picked up litter along\l"
	.string "the canyon today.\p"
	.string "Nature owes me a hug.$"

MatchCall_SEVII_Jackson_General2::
	.string "{PLAYER}, JACKSON here.\n"
	.string "My {STR_VAR_3} watered a\l"
	.string "sapling for me. Proud!\p"
	.string "It looked up and smiled.$"

MatchCall_SEVII_Jackson_General3::
	.string "It's JACKSON, {PLAYER}!\n"
	.string "Saw stars from the\l"
	.string "canyon rim last night.\p"
	.string "Our world's so small. So worth\n"
	.string "protecting.$"

MatchCall_SEVII_Jackson_Battle::
	.string "{PLAYER}! JACKSON!\n"
	.string "Battle at {STR_VAR_2}!\p"
	.string "Nature's backing me up.\n"
	.string "Hope you're ready!$"

@ Katelyn - Pokémon Ranger, Seven Island, Sevault Canyon
MatchCall_SEVII_Katelyn_General1::
	.string "KATELYN calling, {PLAYER}!\p"
	.string "Those shoes of yours!\n"
	.string "Still snazzy? Mine have\l"
	.string "a tiny scuff. Disaster.$"

MatchCall_SEVII_Katelyn_General2::
	.string "{PLAYER}, it's KATELYN!\n"
	.string "My {STR_VAR_3} nursed a\l"
	.string "hurt Pidgey back to health.\p"
	.string "Ranger work is cute work!$"

MatchCall_SEVII_Katelyn_General3::
	.string "KATELYN, {PLAYER}!\n"
	.string "Tried your kind of shoes.\p"
	.string "I hopped ten minutes.\n"
	.string "Fast? No. Fun? Yes.$"

MatchCall_SEVII_Katelyn_Battle::
	.string "{PLAYER}! KATELYN!\n"
	.string "Meet me at {STR_VAR_2}!\p"
	.string "Cute looks don't mean I go\n"
	.string "easy, you know.$"

@ Leroy - Cooltrainer, Seven Island, Sevault Canyon
MatchCall_SEVII_Leroy_General1::
	.string "LEROY here, {PLAYER}.\n"
	.string "I logged our last battle.\l"
	.string "Pages and pages of data.$"

MatchCall_SEVII_Leroy_General2::
	.string "{PLAYER}, it's LEROY.\n"
	.string "Updating my charts on\l"
	.string "{STR_VAR_3}'s stamina.\p"
	.string "Numbers never lie. Mostly.$"

MatchCall_SEVII_Leroy_General3::
	.string "LEROY calling, {PLAYER}.\n"
	.string "My notebook grew a stain.\l"
	.string "Soup, I think.\p"
	.string "Data saved, though!$"

MatchCall_SEVII_Leroy_Battle::
	.string "{PLAYER}, it's LEROY!\n"
	.string "I need more data. Join me\l"
	.string "at {STR_VAR_2}?\p"
	.string "I'll bring a fresh page.$"

@ Michelle - Cooltrainer, Seven Island, Sevault Canyon
MatchCall_SEVII_Michelle_General1::
	.string "MICHELLE here, {PLAYER}.\n"
	.string "Deep breath in, deep breath\l"
	.string "out. Better already.\p"
	.string "Mostly better.$"

MatchCall_SEVII_Michelle_General2::
	.string "{PLAYER}, MICHELLE speaking.\n"
	.string "My {STR_VAR_3} stayed calm\l"
	.string "while I stomped around.\p"
	.string "It's the wiser of us two.$"

MatchCall_SEVII_Michelle_General3::
	.string "It's MICHELLE, {PLAYER}.\n"
	.string "I got grumpy over a\l"
	.string "crooked picture frame.\p"
	.string "I breathed. Still crooked.$"

MatchCall_SEVII_Michelle_Battle::
	.string "{PLAYER}. MICHELLE.\p"
	.string "I'm at {STR_VAR_2}.\p"
	.string "Battle me. I won't lose to\n"
	.string "anyone, so be prepared!$"

@ LexNya - Cool Couple, Seven Island, Sevault Canyon
MatchCall_SEVII_LexNya_General1::
	.string "LEX: {PLAYER}, hello.\p"
	.string "NYA: We just shared tea.\p"
	.string "LEX: With my darling, always.$"

MatchCall_SEVII_LexNya_General2::
	.string "NYA: Hello, {PLAYER}!\n"
	.string "LEX: Our {STR_VAR_3} carried\l"
	.string "our picnic basket today.\p"
	.string "NYA: It's so well-behaved.$"

MatchCall_SEVII_LexNya_General3::
	.string "LEX: This is LEX, {PLAYER}.\p"
	.string "NYA: And NYA. We danced\n"
	.string "under the stars last night.\p"
	.string "LEX: Clumsily, but happily.$"

MatchCall_SEVII_LexNya_Battle::
	.string "NYA: {PLAYER}, we have news!\p"
	.string "LEX: It's {STR_VAR_2}.\n"
	.string "NYA: Join us for a battle!$"

@ Alice - Swimmer, Route 19
MatchCall_COAST_Alice_General1::
	.string "ALICE here, {PLAYER}!\n"
	.string "Swimming is great, but\l"
	.string "sunburn is not.\p"
	.string "I smell like coconut.$"

MatchCall_COAST_Alice_General2::
	.string "{PLAYER}, it's ALICE!\n"
	.string "My {STR_VAR_3} splashes me\l"
	.string "every time I apply lotion.\p"
	.string "It's a conspiracy!$"

MatchCall_COAST_Alice_General3::
	.string "Hi, {PLAYER}, ALICE!\n"
	.string "My boyfriend still wants\l"
	.string "to swim to SEAFOAM.\p"
	.string "I said I'd pack sandwiches.$"

MatchCall_COAST_Alice_Battle::
	.string "{PLAYER}! ALICE!\n"
	.string "Beach day? Not quite: I'm\l"
	.string "at {STR_VAR_2}.\p"
	.string "Come battle! Sunscreen's on!$"

@ Darrin - Swimmer, Route 20
MatchCall_COAST_Darrin_General1::
	.string "DARRIN here, {PLAYER}!\n"
	.string "You still ride your POKéMON?\l"
	.string "Come on, kick a little!$"

MatchCall_COAST_Darrin_General2::
	.string "{PLAYER}, it's DARRIN!\n"
	.string "My {STR_VAR_3} lets me hold\l"
	.string "its fin and tow me along.\p"
	.string "Is that cheating?$"

MatchCall_COAST_Darrin_General3::
	.string "DARRIN, {PLAYER}!\n"
	.string "I swam a full lap with\l"
	.string "zero rests. Zero!\p"
	.string "Okay, maybe two rests.$"

MatchCall_COAST_Darrin_Battle::
	.string "{PLAYER}! DARRIN!\n"
	.string "Torpedo me again at\l"
	.string "{STR_VAR_2}!\p"
	.string "I'll dive straight in!$"

@ Missy - Picnicker, Route 20
MatchCall_COAST_Missy_General1::
	.string "Hiya, {PLAYER}! MISSY!\p"
	.string "I still can't believe I swam\n"
	.string "all the way from CINNABAR.\p"
	.string "My legs are noodles!$"

MatchCall_COAST_Missy_General2::
	.string "{PLAYER}, MISSY here!\n"
	.string "I packed rice balls for the\l"
	.string "trip. They got soggy.\p"
	.string "{STR_VAR_3} ate them anyway.$"

MatchCall_COAST_Missy_General3::
	.string "Hi, {PLAYER}, MISSY!\n"
	.string "I heard POKéMON moved into\l"
	.string "that old CINNABAR mansion.\p"
	.string "Imagine the sleepovers!$"

MatchCall_COAST_Missy_Battle::
	.string "{PLAYER}! It's MISSY!\n"
	.string "Let's have our rematch at\l"
	.string "{STR_VAR_2}!\p"
	.string "I'll bring the picnic blanket!$"

@ Wade - Fisherman, Route 21 North
MatchCall_COAST_Wade_General1::
	.string "Hey, {PLAYER}. WADE.\n"
	.string "Another big haul today.\p"
	.string "Surprise: all MAGIKARP.$"

MatchCall_COAST_Wade_General2::
	.string "{PLAYER}, WADE here.\n"
	.string "Got a bite! Probably\l"
	.string "another MAGIKARP.\p"
	.string "My {STR_VAR_3} looked at me\n"
	.string "with pity.$"

MatchCall_COAST_Wade_General3::
	.string "Heh, {PLAYER}. WADE.\p"
	.string "My cooler's full of ice\n"
	.string "and a lonely sandwich.\p"
	.string "I might switch bait.$"

MatchCall_COAST_Wade_Battle::
	.string "{PLAYER}! WADE!\n"
	.string "I'm at {STR_VAR_2}.\l"
	.string "Care to reel in a battle?$"

@ Jack - Swimmer, Route 21 South
MatchCall_COAST_Jack_General1::
	.string "JACK here, {PLAYER}!\p"
	.string "Found my {STR_VAR_3} at sea.\n"
	.string "It was shy and shiny.\p"
	.string "Now it's my diving buddy.$"

MatchCall_COAST_Jack_General2::
	.string "{PLAYER}, JACK speaking!\n"
	.string "Dove down deep and found\l"
	.string "a pretty glimmering shell.\p"
	.string "I kept it. It's lucky!$"

MatchCall_COAST_Jack_General3::
	.string "Hey, {PLAYER}, JACK!\n"
	.string "Where'd you catch your\l"
	.string "best POKéMON? Tell me!\p"
	.string "I'm collecting sea stories.$"

MatchCall_COAST_Jack_Battle::
	.string "{PLAYER}! JACK!\n"
	.string "I'm at {STR_VAR_2}. Dive\l"
	.string "toward a rematch!\p"
	.string "Don't be late. Down we go!$"

@ LilIan - Sis and Bro, Route 21 North
MatchCall_COAST_LilIan_General1::
	.string "LIL: Ugh. Hi, {PLAYER}.\p"
	.string "IAN: It's us! We're awake!\p"
	.string "LIL: You're awake. I'm not.$"

MatchCall_COAST_LilIan_General2::
	.string "IAN: {PLAYER}, hello!\n"
	.string "LIL: Is this call over yet?\p"
	.string "IAN: Our {STR_VAR_3} is great!\p"
	.string "LIL: It's okay. Sure.$"

MatchCall_COAST_LilIan_General3::
	.string "LIL: {PLAYER}, it's us.\p"
	.string "IAN: We're out on a trip!\p"
	.string "LIL: A walk. A very long one.$"

MatchCall_COAST_LilIan_Battle::
	.string "IAN: {PLAYER}, battle time!\p"
	.string "LIL: Don't say yes.\n"
	.string "IAN: It's {STR_VAR_2}!$"

@ Matthew - Swimmer, Route 19
MatchCall_COAST_Matthew_General1::
	.string "MATTHEW here, {PLAYER}!\p"
	.string "I love swimming! Every day,\n"
	.string "rain or shine!\p"
	.string "Do you swim much?$"

MatchCall_COAST_Matthew_General2::
	.string "{PLAYER}, it's MATTHEW!\n"
	.string "I raced a Seel. Won!\p"
	.string "Okay, it was napping.\p"
	.string "Still counts, right?$"

MatchCall_COAST_Matthew_General3::
	.string "Hey, {PLAYER}, MATTHEW!\n"
	.string "My {STR_VAR_3} thinks it's\l"
	.string "faster than me. It is.\p"
	.string "I'll never admit it aloud.$"

MatchCall_COAST_Matthew_Battle::
	.string "{PLAYER}! MATTHEW!\n"
	.string "Race you to {STR_VAR_2}!\p"
	.string "Then we battle. Belly flops\n"
	.string "are on the house!$"

@ Tony - Swimmer, Route 19
MatchCall_COAST_Tony_General1::
	.string "Hello, {PLAYER}. TONY.\p"
	.string "The sea's calm today.\n"
	.string "It helps me forget things.\p"
	.string "I forgot what, actually.$"

MatchCall_COAST_Tony_General2::
	.string "{PLAYER}, TONY here.\p"
	.string "My {STR_VAR_3} brings me\n"
	.string "shells when I'm sad.\p"
	.string "It's the best medicine.$"

MatchCall_COAST_Tony_General3::
	.string "TONY, {PLAYER}.\p"
	.string "I looked at the waves for\n"
	.string "hours. Dried out.\p"
	.string "Totally worth it.$"

MatchCall_COAST_Tony_Battle::
	.string "{PLAYER}. TONY.\p"
	.string "Battle at {STR_VAR_2}.\n"
	.string "It'll take my mind off\l"
	.string "the old bad stuff.$"

@ Melissa - Swimmer, Route 20
MatchCall_COAST_Melissa_General1::
	.string "Hi, {PLAYER}! MELISSA!\p"
	.string "My daddy's working late at\n"
	.string "the lab on CINNABAR again.\p"
	.string "He brought me a snack!$"

MatchCall_COAST_Melissa_General2::
	.string "{PLAYER}, MELISSA here!\n"
	.string "I read that CINNABAR rose\l"
	.string "from the sea in a blast.\p"
	.string "Rock from water! Cool!$"

MatchCall_COAST_Melissa_General3::
	.string "It's MELISSA, {PLAYER}!\n"
	.string "My {STR_VAR_3} found a warm\l"
	.string "spot near the volcano.\p"
	.string "It refused to leave.$"

MatchCall_COAST_Melissa_Battle::
	.string "{PLAYER}! MELISSA!\n"
	.string "I'm at {STR_VAR_2}!\p"
	.string "Wait for me to start!\n"
	.string "No cheating this time.$"
```

## HNS Battle and gift texts

Every existing `MatchCall_HNS_*_Battle` text is replaced as below so it names today's place, and so is each gift contact's `FoundItem` text, which named the home route. The General texts are unchanged.

39 Battle labels and 9 FoundItem labels rewritten (labels unchanged). Nicole's Battle text is dropped with her entry, which Wayfarer doesn't compile.

```asm
@ Joey - Youngster, Route 30
MatchCall_HNS_Joey_Battle::
	.string "Yo, {PLAYER}!\p"
	.string "Let's get together at\n"
	.string "{STR_VAR_2} and battle!\p"
	.string "I promise things will be\n"
	.string "different!$"

@ Wade - Bug Catcher, Route 31
MatchCall_HNS_Wade_Battle::
	.string "{PLAYER}, howdy!\p"
	.string "Do you feel like a POKéMON\n"
	.string "battle at {STR_VAR_2}?\p"
	.string "It won't be like last\n"
	.string "time!$"

@ Ralph - Fisher, Route 32
MatchCall_HNS_Ralph_Battle::
	.string "Hey, {PLAYER}!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Liz - Picnicker, Route 32
MatchCall_HNS_Liz_Battle::
	.string "Hi, {PLAYER}!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Anthony - Hiker, Route 33
MatchCall_HNS_Anthony_Battle::
	.string "Hey there, {PLAYER}!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Todd - Camper, Route 34
MatchCall_HNS_Todd_Battle::
	.string "Hey, {PLAYER}!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Gina - Picnicker, Route 34
MatchCall_HNS_Gina_Battle::
	.string "Hi, {PLAYER}!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Irwin - Juggler, Route 35
MatchCall_HNS_Irwin_Battle::
	.string "Uh, hello, {PLAYER}?\n"
	.string "It's your pal, IRWIN!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Arnie - Bug Catcher, Route 35
MatchCall_HNS_Arnie_Battle::
	.string "Hey, {PLAYER}!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Alan - Schoolboy, Route 36
MatchCall_HNS_Alan_Battle::
	.string "Hi, {PLAYER}!\n"
	.string "It's ALAN!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Dana - Lass, Route 38
MatchCall_HNS_Dana_Battle::
	.string "Hi, {PLAYER}!\n"
	.string "It's DANA!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Chad - Schoolboy, Route 38
MatchCall_HNS_Chad_Battle::
	.string "Hello, {PLAYER}!\n"
	.string "It's CHAD!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Derek - Pokefan M, Route 39
MatchCall_HNS_Derek_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's DEREK!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Tully - Fisher, Route 42
MatchCall_HNS_Tully_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's TULLY!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Brent - Pokemaniac, Route 43
MatchCall_HNS_Brent_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's BRENT!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Tiffany - Picnicker, Route 43
MatchCall_HNS_Tiffany_Battle::
	.string "Hi, {PLAYER}!\n"
	.string "It's TIFFANY!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Vance - Bird Keeper, Route 44
MatchCall_HNS_Vance_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's VANCE!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Wilton - Fisher, Route 44
MatchCall_HNS_Wilton_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's WILTON!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Kenji - Blackbelt, Route 45
MatchCall_HNS_Kenji_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "This is KENJI!\p"
	.string "I'm taking a break at\n"
	.string "{STR_VAR_2} today!\p"
	.string "Why not drop by if you\n"
	.string "are free?$"

@ Parry - Hiker, Route 45
MatchCall_HNS_Parry_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's PARRY!\p"
	.string "You'll battle with me\n"
	.string "again at {STR_VAR_2}?$"

@ Erin - Picnicker, Route 46
MatchCall_HNS_Erin_Battle::
	.string "Hi, {PLAYER}!\n"
	.string "It's ERIN!\p"
	.string "I'm working hard to raise\n"
	.string "my POKéMON!\p"
	.string "Come to {STR_VAR_2}\n"
	.string "for another battle!$"

@ Jack - Schoolboy, National Park
MatchCall_HNS_Jack_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's JACK!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Beverly - Pokefan F, National Park
MatchCall_HNS_Beverly_Battle::
	.string "Hello, {PLAYER}!\n"
	.string "It's BEVERLY!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Huey - Sailor, Olivine Lighthouse
MatchCall_HNS_Huey_Battle::
	.string "Yo, {PLAYER}!\n"
	.string "It's HUEY!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Gaven - Cooltrainer M, Route 26
MatchCall_HNS_Gaven_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's GAVEN!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Beth - Cooltrainer F, Route 26
MatchCall_HNS_Beth_Battle::
	.string "Hi, {PLAYER}!\n"
	.string "It's BETH!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Jose - Bird Keeper, Route 27
MatchCall_HNS_Jose_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's JOSE!\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Reena - Cooltrainer F, Route 27
MatchCall_HNS_Reena_Battle::
	.string "Hello, {PLAYER}.\n"
	.string "It's REENA.\p"
	.string "I was waiting for you\n"
	.string "at {STR_VAR_2} today!\p"
	.string "Let's battle!$"

@ Alex - Battle Girl, Route 13
MatchCall_HNS_Alex_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's ALEX!\p"
	.string "My regal POKéMON are ready\n"
	.string "for a rematch!\p"
	.string "Come to {STR_VAR_2}!$"

@ Riley - Biker, Route 17
MatchCall_HNS_Riley_Battle::
	.string "Yo, {PLAYER}!\n"
	.string "RILEY here.\p"
	.string "Get down to {STR_VAR_2}!\n"
	.string "I want a rematch!$"

@ Trevor - Juggler, Route 14
MatchCall_HNS_Trevor_Battle::
	.string "Hi, {PLAYER}!\n"
	.string "It's TREVOR.\p"
	.string "My friends are ready for\n"
	.string "battle! Come see us at\l"
	.string "{STR_VAR_2}!$"

@ Kyle - Fisherman, Route 12
MatchCall_HNS_Kyle_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "KYLE here.\p"
	.string "I've reeled in some tough\n"
	.string "new POKéMON!\p"
	.string "Come to {STR_VAR_2} for\n"
	.string "a battle!$"

@ Carter - Pokefan, Route 14
MatchCall_HNS_Carter_Battle::
	.string "Hi, {PLAYER}!\n"
	.string "CARTER here.\p"
	.string "I have new starters to\n"
	.string "show off! Come to\l"
	.string "{STR_VAR_2}!$"

@ Hillary - Expert, Route 15
MatchCall_HNS_Hillary_Battle::
	.string "Hello, {PLAYER}.\n"
	.string "It's HILLARY.\p"
	.string "My POKéMON have graduated\n"
	.string "to the next level!\p"
	.string "Come test us at\n"
	.string "{STR_VAR_2}!$"

@ Rob - Bug Catcher, Route 2
MatchCall_HNS_Rob_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "ROB here!\p"
	.string "My bug team is stronger\n"
	.string "than ever!\p"
	.string "Come to {STR_VAR_2} for a\n"
	.string "rematch!$"

@ Billy - School Kid, Route 15
MatchCall_HNS_Billy_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "It's BILLY.\p"
	.string "I learned some new tricks!\n"
	.string "Come to {STR_VAR_2}!$"

@ Kenny - Hiker, Route 13
MatchCall_HNS_Kenny_Battle::
	.string "Hey there, {PLAYER}!\n"
	.string "KENNY here.\p"
	.string "I've trained hard in the\n"
	.string "mountains!\p"
	.string "Come to {STR_VAR_2} for\n"
	.string "a battle!$"

@ Joel - Biker, Route 17
MatchCall_HNS_Joel_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "JOEL here.\p"
	.string "I look AND fight cool now!\n"
	.string "Come to {STR_VAR_2}!$"

@ Charles - Biker, Route 17
MatchCall_HNS_Charles_Battle::
	.string "Hey, {PLAYER}!\n"
	.string "CHARLES here.\p"
	.string "The highway stars want\n"
	.string "revenge! Come to\l"
	.string "{STR_VAR_2}!$"
```

### Gift texts

```asm
@ Wade - Bug Catcher, Route 31
MatchCall_HNS_Wade_FoundItem::
	.string "{PLAYER}, howdy!\p"
	.string "I found all kinds of\n"
	.string "BERRIES.\p"
	.string "If you want, I'll share\n"
	.string "some with you.\p"
	.string "I'll be waiting at\n"
	.string "{STR_VAR_2} today.$"

@ Alan - Schoolboy, Route 36
MatchCall_HNS_Alan_FoundItem::
	.string "Hi, {PLAYER}!\n"
	.string "It's ALAN!\p"
	.string "Hehehe, I picked up\n"
	.string "something nice!\p"
	.string "You can have it!\p"
	.string "I'm at {STR_VAR_2}.\n"
	.string "Come pick it up!$"

@ Dana - Lass, Route 38
MatchCall_HNS_Dana_FoundItem::
	.string "Hi, {PLAYER}!\n"
	.string "It's DANA!\p"
	.string "You know what?\n"
	.string "I got a good gift!\p"
	.string "As I promised, it's yours!\n"
	.string "I'm sure you'd like it.\p"
	.string "Come get it! I'm waiting\n"
	.string "at {STR_VAR_2} today!$"

@ Derek - Pokefan M, Route 39
MatchCall_HNS_Derek_FoundItem::
	.string "Hey, {PLAYER}!\n"
	.string "It's DEREK!\p"
	.string "I'd like you to have a\n"
	.string "NUGGET.\p"
	.string "I'm at {STR_VAR_2}.\n"
	.string "Come pick it up!$"

@ Tully - Fisher, Route 42
MatchCall_HNS_Tully_FoundItem::
	.string "Hey, {PLAYER}!\n"
	.string "It's TULLY!\p"
	.string "I picked up a good little\n"
	.string "thing at the water's edge.\p"
	.string "Like I promised, it's\n"
	.string "yours.\p"
	.string "I'll be waiting at\n"
	.string "{STR_VAR_2} today.$"

@ Wilton - Fisher, Route 44
MatchCall_HNS_Wilton_FoundItem::
	.string "Hey, {PLAYER}!\n"
	.string "It's WILTON!\p"
	.string "I snagged an item while\n"
	.string "fishing.\p"
	.string "Come pick it up at\n"
	.string "{STR_VAR_2} today.$"

@ Kenji - Blackbelt, Route 45
MatchCall_HNS_Kenji_FoundItem::
	.string "Hey, {PLAYER}!\n"
	.string "This is KENJI!\p"
	.string "I'm taking a break from\n"
	.string "training.\p"
	.string "I found something good.\p"
	.string "Come get it at\n"
	.string "{STR_VAR_2} if you want!$"

@ Beverly - Pokefan F, National Park
MatchCall_HNS_Beverly_FoundItem::
	.string "Hello, {PLAYER}!\n"
	.string "It's BEVERLY!\p"
	.string "My husband got some\n"
	.string "NUGGETS.\p"
	.string "If you'd like, you could\n"
	.string "have one as thanks for\p"
	.string "helping me out. I'll be at\n"
	.string "{STR_VAR_2}. Come see me!$"

@ Jose - Bird Keeper, Route 27
MatchCall_HNS_Jose_FoundItem::
	.string "Hey, {PLAYER}!\n"
	.string "It's JOSE!\p"
	.string "My FARFETCH'D had\n"
	.string "something pretty in its\p"
	.string "beak. Like I promised, you\n"
	.string "can have it.\p"
	.string "Catch up to me at\n"
	.string "{STR_VAR_2} today.$"
```

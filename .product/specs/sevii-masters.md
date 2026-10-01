# Sevii Masters

PRD: [Sevii Masters](../prds/sevii-masters.md)
Implemented: No
Design status: v0 approved: the Sevii Masters as an off-the-record club, one
of the three [Leagues](leagues.md), that calls the player once they are a
**Master** (lifetime wins at both Indigo and Hoenn). Its lineup has eight
seats, with guaranteed **Master seats** for notable trainers who have
reigned at both Indigo and Hoenn, the aloof rule off, and the **partner**
(the phone contact the player last asked, Lorelei by default) never seated;
the eight pair up in ascending order. Four **tag matches** (the player and
partner against a pair, three Pokémon each, the player picking three at each
door with damage carrying over) lead to a full-team singles final against
the partner after a full heal. A notable trainer's phone number comes when their
[friendship](notable-trainers.md#friendship) reaches Friend, which a first
win over them does. The **Masters Gallery** tallies every
Masters winner; the partner reigns when the player loses the final.

## Scope

Own, for the Sevii Masters in `IS_WAYFARER`: its identity and presentation,
the Master rule, phone contacts, the partner, the Masters' changes to lineup
selection (the lineup size, Master seats, no aloof rule, the partner's
exclusion, and pairs), the partner in the event lineup, the Masters' title
rules, the Masters Gallery, the tag matches, party choice, restore, and heal,
the final, the Masters' saved state and load validation, its balance report,
and its acceptance.

- [Leagues](leagues.md) owns the shared machinery this spec plugs into: the
  league registry (with the Masters as league 2, a neutral location) and
  eligibility, invitations (the qualification gate, the countdown, which
  league calls, including the Masters once the player is a
  [Master](#master), accepting and declining), fatigue, the league score,
  the base selection steps and battle order, the halls and their conditions
  (the Masters' four halls and Champion's Room included), the event lineup,
  the reigning champion and the Indigo and Hoenn reign flags, the lifecycle,
  dispatch, battle construction, and the saved-state and load-validation
  framework. This spec states only where the Masters differs.
- [Notable trainers](notable-trainers.md) owns the trainers themselves:
  TR, teams, rosters, battle order, and the aloof trait. This spec reads a
  trainer's stored team and never restates how it is composed.
- [Trainer AI](trainer-ai.md#options-and-double-battles) owns how the three
  notable trainers of a tag match resolve their AI flags and which battler
  gets whose.

## Identity

The Masters is presented as an off-the-record club, not an official league:
a hidden basement under Seven Island's battle house (the Masters House
entrance), where champions find out who is actually best. An old caretaker,
the retired trainer who runs the house, keeps the door and makes its calls.
Its title is the Masters title, never a regional Champion title, and a win
brings no Hall of Fame registration or Ribbon. The final HNS ceremony room
is the [Masters Gallery](#masters-gallery).

### Master

A **Master** is anyone, the player or a notable trainer, who has been
champion at both Indigo and Hoenn at some point, whether or not they reign
anywhere now:

- A notable trainer is a Master, for good, once both
  [reign flags](leagues.md#reign-records) are set (has reigned at Indigo and
  has reigned at Hoenn).
- The player is a Master once they have lifetime wins at both Indigo and
  Hoenn (`indigoCleared` and `hoennCleared`), in either order; the saved
  first-league-win facts already record that, so the player has no reign
  flags.

A Masters title never counts ([it sets no reign
flag](leagues.md#reign-records)), and a lifetime win at only one of Indigo and
Hoenn, or a Masters win, never makes anyone a Master.
Being a Master changes only the Masters: it knows the player and may call
them ([Which league calls](leagues.md#which-league-calls)), the player can
ask a partner, and notable trainers who are Masters get guaranteed seats.
Winning the Masters adds a Gallery win and gives no new status.

## Phone contacts

A notable trainer other than the Tate & Liza duo is a **contact** on the
phone ([phone calls](../../game/src/match_call.c)) when their
[friendship stage](notable-trainers.md#friendship) is Friend or above, and
the player then holds their phone number. The number is handed over when the
score first crosses the Friend threshold, by any route: the first committed
win against them (a Gym, rematch, story, or league battle, singles or a tag
match, where both opponents count) is enough, and so are finished haunt
quests. Nothing else sets it: a loss, a draw, fleeing, a declined league
event, a battle beside the trainer as a partner, and debug battles add no
friendship. Friendship never decreases, so a contact stays one. Tate & Liza
keep a score but give no number, since in v0 a contact's only use is being
asked as a partner, which the duo cannot be.

On the phone, the only use is asking a contact to be the player's
[partner](#partner); calling a contact otherwise does nothing new.
[Notable haunts](notable-haunts.md#talk-flow) read the friendship
stage, not the phone.

## Partner

At the Masters the player fights beside a **partner**: the notable trainer
they last asked. The saved **partner choice** is none or one `characterId`.

- **Asking.** The player can ask any contact to be their partner once they
  are a Master (the Masters knows them), by phone only: from the phone's
  contact list whenever they can make a call (in the overworld, with no
  script, battle, or ceremony running). The contact exists from Friend, so
  any Friend or closer can be asked; there is no in-person ask at
  [haunts](notable-haunts.md#talk-flow). The ask always succeeds: it
  saves that contact as the partner choice, replacing any earlier one, and
  does nothing else. Only a league-eligible contact can be asked (Tate & Liza
  give no number), and no trait, TR, or reign check applies: Friend or above
  is the only requirement.
- **Default.** With no partner choice (the player has never asked anyone),
  the partner is **Lorelei**, who spoke of the player to the caretaker. She
  needs no friendship.
- **Resolution.** The partner is resolved when the player accepts or
  declines a Masters invitation, before any other selection step: the
  partner choice, or Lorelei without one. An accepted event records that
  partner in its [event lineup](#event-lineup), so asking someone else while
  it waits changes only later Masters events. A declined event reads the
  partner only to keep them out of its lineup; they do not play in it.

The partner is never in the lineup, is a notable trainer like any other
(their own TR, team, and AI), and plays no part in Indigo or Hoenn events.
Battling as a partner needs a back pic, so league eligibility at every
league includes one ([Registry and
eligibility](leagues.md#registry-and-eligibility)).

## Lineup

The Masters runs [Selection and order](leagues.md#selection-and-order) with
these changes:

1. **Partner first.** Before anything else, resolve the [partner](#partner)
   and remove them from the eligible trainers for this event.
2. **Eight seats.** The lineup size is eight.
3. **No aloof rule.** The Masters is elite by definition, so there is no base
   lineup, no base lineup level, and no aloof check: every eligible trainer
   stays eligible, however far above the others.
4. **Master seats.** Before the lineup is filled, rank the eligible notable
   trainers who are [Masters](#master) by league score, ties by ascending
   `characterId`, and seat the top eight (all of them if fewer than eight).
   Fatigue lowers a Master's league score but never removes the guarantee; a
   Master who is the partner was removed first and has no seat. The
   remaining seats, eight minus the Master seats, go to the highest league
   scores among everyone else still eligible, as at any league. Being a
   Master gives no seat at Indigo or Hoenn.
5. **Pairs.** After battle order, pair the eight in that order: the first
   and second are pair 1, the third and fourth pair 2, and so on, so the
   strongest two are pair 4. In each pair the first is **opponent A** and
   the second **opponent B**.
6. **Halls.** Match N is pair N's [tag match](#tag-matches), fought in the
   Masters' [hall](leagues.md#halls) N, for N from 1 to 4; match 5 is the
   [final](#the-final) against the partner in the Champion's Room. Hall
   conditions play no part in selection, order, or pairing.

Everything else, TR, willingness and fatigue (with the Masters as a neutral
location, home to everyone), the league score, battle order, and the
tie-breaks, is the shared rule. The same world progress, most recent
resolved lineup, reign records, partner, and content always give the same
eight; the reign records and the partner are the only history the Masters
adds, and contacts play no other part.

For an accepted Masters event, the [most recent resolved
lineup](leagues.md#selection-and-order) also holds the partner, who played
in it; a declined Masters event holds only its eight.

## Event lineup

An accepted Masters event's [event lineup](leagues.md#event-lineup) holds
the eight in battle order, whose positions give the pairs, and also captures
the partner the same way: `characterId`, TR, and composed team. The composed
team is each trainer's whole resolved team; a tag match takes its three from
it at construction ([Tag matches](#tag-matches)), so nothing else is stored.
The event's end keeps the eight and the partner as the most recent resolved
lineup. A declined Masters event saves the eight `characterId`s and composes
no teams.

## Title

The Masters has a [reigning champion](leagues.md#reigning-champion) like any
league, with one exception: if the player loses the final, the reigning
champion is the partner, who beat them. A loss in any hall (matches 1-4) or
leaving before the final crowns the frozen lineup's strongest, and a decline
the strongest of the lineup computed at decline, as at any league.

### Masters Gallery

The Masters Gallery, the final ceremony room on the basement wall, tallies
each Masters winner's wins, adding one for the winner of every resolved
Masters event: the player after a win, otherwise the reigning champion that
event crowned (after a decline, a loss, or leaving), so trainers who won
events the player declined appear too. It keeps a saved win count per
winner (one per notable trainer and one for the player), shown on the wall;
a count stops at its maximum rather than wrapping. The Gallery win is
committed in the same transaction as the title
([Lifecycle](leagues.md#lifecycle)).

Every Masters win, first or repeat, gives the player a Gallery win, the
prize money of its battles, and the title, and nothing else: no regional
awards, Hall of Fame registration, Ribbon, or new status
([First and repeat wins](leagues.md#first-and-repeat-wins)).

## Run

The run follows [Active run and dispatch](leagues.md#active-run-and-dispatch),
with the partner and their team from the event lineup; a later partner
choice never mutates an event lineup.

### Party and heal

In a tag match each trainer brings three Pokémon, the engine's fixed
per-trainer share of a multi battle. The player's three come from their
current party, and their state carries from hall to hall:

1. **Choose three.** At each hall's door (matches 1-4), after the pair is
   introduced, the player picks three party members on the engine's
   existing choose-3 screen (`ChooseHalfPartyForBattle` in
   [script_pokemon_util.c](../../game/src/script_pokemon_util.c)). The
   screen's own rules apply: it refuses fainted Pokémon, and it accepts one
   or two when the player chooses fewer, so a party with fewer than three
   able Pokémon can still fight. Cancelling returns to the door with nothing
   started, as Mossdeep's Steven battle does.
2. **Save and reduce.** Save the whole party as the backup
   (`SavePlayerParty`), record the chosen slots as the run's tag selection,
   then reduce the battle party to the three in the chosen order
   (`ReducePlayerPartyToSelectedMons`, which the `multi_2_vs_2` macro's
   `multi_do` runs); the partner's three fill `gPlayerParty[3..5]`.
3. **Restore.** When the battle ends, won or lost, write each of the three
   back to its original slot in the backup, then reload the whole party, as
   `CB2_EndDebugBattle` in
   [battle_setup.c](../../game/src/battle_setup.c) does with
   `frontier.selectedPartyMons` and `LoadPlayerParty`, and clear the tag
   selection. The three keep their HP, PP, status, fainting, experience,
   levels, and consumed items; the rest of the party is untouched; the
   partner's Pokémon leave with the battle. The multi macro's own
   end-of-battle copy stays skipped (`MULTI_BATTLE_CHOOSE_MONS`), so this is
   the only restore. A tag match loss restores the party first, then blacks
   out as [Loss](leagues.md#loss) describes.
4. **No heal between halls.** Damage carries over: nothing heals the party
   between matches 1-4 beyond what the player does themselves, as between
   any two league matches. The pair and the partner are built fresh from the
   event lineup for each match.
5. **Full heal before the final.** When match 4 is won and the party is
   restored, fully heal the player's whole party (HP, PP, and status), and
   the partner's team is built fresh for the final: both sides start it
   whole.

### Reset

No save point exists between the choice and the end of the restore: the
choose-3 screen, the battle, and the restore run in one script, so a reset
at any moment of a tag match reloads the last save, where the party is whole
and no tag selection is recorded. That is neither a win nor a loss: the run
resumes wherever that save was made, and the match is fought again, as after
a reset in any league match. A save routine that ever runs in that window
(such as a future autosave) must write the backup as the party, never the
reduced battle party; [load validation](#load-validation) recovers such a
save.

## Battle construction

Masters matches follow [Battle construction](leagues.md#battle-construction):
each trainer fights with their own stored team at their own TR, with the
hall condition written as for any league match.

### Tag matches

Masters matches 1-4 are tag matches: the player and the partner against
pair N, in the engine's existing two-versus-two partner battle. Mossdeep's
Steven battle
([scripts](../../game/data/maps/MossdeepCity_SpaceCenter_2F/scripts.inc))
is the reference flow: `SavePlayerParty`, then `ChooseHalfPartyForBattle`,
then the `multi_2_vs_2` macro
([battle_tower.inc](../../game/asm/macros/battle_frontier/battle_tower.inc)),
which calls `SetMultiTrainerBattle`
([battle_setup.c](../../game/src/battle_setup.c)) and starts
`SPECIAL_BATTLE_MULTI` in
[battle_special.c](../../game/src/battle_special.c); that sets
`BATTLE_TYPE_TRAINER | DOUBLE | TWO_OPPONENTS | MULTI | INGAME_PARTNER` and
calls `FillPartnerParty`, and the script then reads `VAR_RESULT` and calls
`SetCB2WhiteOut` on a loss.

- **Three each.** The engine fixes three Pokémon per trainer: the player's
  in `gPlayerParty[0..2]`, the partner's in `gPlayerParty[3..5]`
  ([battle_partner.c](../../game/src/battle_partner.c) caps them at 3), and
  each opponent's half of `gEnemyParty`, capped at `PARTY_SIZE / 2` in a
  two-opponent battle (`CreateNPCTrainerPartyInternal` in
  [battle_main.c](../../game/src/battle_main.c)). Opponent A fills the first
  half and opponent B the second.
- **Best three, aces first.** Each notable trainer in a tag match brings
  the last three members of their stored team's
  [battle order](notable-trainers.md#rosters) (all of it with fewer than
  three), in that order. Battle order sends fillers first and aces last,
  and a roster has one to three aces, so this takes every ace, then the
  filler slots closest to them in battle order (the lowest-numbered filler
  slots in the team); the signature Pokémon still comes out last. Every
  member keeps the species, level, moves, item, ability, nature, and IVs/EVs
  the stored team resolved for it (its moves resolved against the whole
  team), so a Pokémon is the same in a tag match as anywhere else. Brock's
  full team (Omastar, Kabutops, Crobat, Golem, Aerodactyl, Steelix) brings
  Golem, Aerodactyl, Steelix.
- **Notable team path.** Today's scaling skips these battles: the scaling
  context excludes `BATTLE_TYPE_INGAME_PARTNER`
  (`IsTrainerScalingBattleContext` in
  [trainer_party_scaling.c](../../game/src/trainer_party_scaling.c)), and
  league rosters are skipped under `BATTLE_TYPE_TWO_OPPONENTS` in
  battle_main.c. A Masters tag match must build both opponents and the
  partner from the event lineup's stored teams through a notable team path
  that those exclusions do not stop; every other partner or two-opponent
  battle keeps today's construction.
- **Runtime partner slot.** Partners are the static `gBattlePartners`
  ([battle_partners.party](../../game/src/data/battle_partners.party)).
  Reserve one runtime partner slot whose trainer struct (name, class,
  front pic, back pic, AI flags) and three-member party are filled for each
  battle from the partner's stored snapshot and presentation, and point
  `gPartnerTrainerId` at it; `FillPartnerParty` builds `gPlayerParty[3..5]`
  from that party. No other partner entry changes.
- **AI.** Opponent A, opponent B, and the partner each get their own AI
  flags from their own snapshot, as
  [Trainer AI](trainer-ai.md#options-and-double-battles) maps them to
  battlers.
- **Money.** `Cmd_getmoneyreward` in
  [battle_script_commands.c](../../game/src/battle_script_commands.c) adds
  both opponents' rewards, each from their authored party's last level. A
  tag match uses the snapshot level basis instead: each opponent's reward
  reads the level of the last member they bring (their signature Pokémon)
  with their class's rate, and the two are summed as the engine sums them.
- **Experience and whiteout.** The player's Pokémon gain experience as
  usual; the partner's gain none (the engine skips partner slots under
  `BATTLE_TYPE_INGAME_PARTNER`). With `B_MULTI_BATTLE_WHITEOUT` at
  `GEN_LATEST` ([config](../../game/include/config/battle.h)), the match is
  lost only when the player's and the partner's Pokémon have all fainted.
- **Halls.** The [hall condition](leagues.md#hall-condition) is written the
  same way. The player and partner share one side and the pair the other;
  each side's starting status covers both battlers on that side, and
  starting hazards hit all four leads, two per side, as Bruno's Hall's
  Stealth Rock does; a Sticky Web start would lower the Speed of each
  grounded lead the same way.
- **Rooms.** The Masters
  [room scripts](../../game/data/scripts/wayfarer_masters_league.inc)
  (`wayfarer_masters_league.inc`) start one singles battle per room, and
  each HNS hall room map holds one opponent object. Each of the four tag
  halls needs a second opponent object for opponent B, and its script
  starts the tag match through the flow above.

### The final

Match 5 at the Masters is a singles battle in the neutral Champion's Room
against the partner, after the [full heal](#party-and-heal): the player's
whole party against the partner's whole stored team, in its battle order.
It is built like any league match: the partner's snapshot, AI flags from
[Trainer AI](trainer-ai.md) at their TR, and prize money as in any league
singles match. Winning it wins the event; losing it crowns the partner
([Title](#title)).

## Saved state

The Masters adds to the [league saved state](leagues.md#saved-state):

- no contact state of its own: a contact is read from
  [friendship](notable-trainers.md#friendship), which Notable trainers save;
- the [partner choice](#partner): none or one `characterId`;
- the [Masters Gallery](#masters-gallery) win counts, one per notable
  trainer and one for the player; and
- in the active run, the **tag selection**: the party slots the player
  chose at the current door, present only between that choice and the end
  of its [party restore](#party-and-heal).

An accepted Masters event's lineup also holds the partner, and the most
recent resolved lineup holds eight `characterId`s after a declined Masters
event, or eight and the partner after an accepted one. The reign flags are
unchanged by the tag format. New Game saves no
partner choice, and every Gallery count 0.

The composed teams of the eight and the partner live in `PokemonStorage`
with the league teams; the room for them comes from the [save layout
decision](notable-world-simulation.md#where-it-lives) to drop one PC box
(14 to 13).

## Load validation

The Masters adds to [Load validation](leagues.md#load-validation):

- **Mid-tag recovery.** On every load, first, a Masters run holding a tag
  selection was saved between a door's choice and its restore, which
  [normal play never does](#reset). Its saved party is the whole backup from
  the door, so keep that party as it is (each chosen member as it was at the
  door), clear the tag selection, and resume the run at that match's door
  with the match unfought; the reset is neither a win nor a loss. A tag
  selection that names slots outside the saved party is corrupt.
- **Pruning.** Alongside the shared pruning, whatever the content versions
  say: drop the Gallery counts of characters no longer in the registry
  (friendship scores prune under [Notable
  trainers](notable-trainers.md#friendship)), keeping the rest, and clear a
  partner choice naming one (so Lorelei steps in again). If a content version
  differs from the build's, also clear a partner choice who is no longer an
  eligible character.
- **Checks.** An accepted Masters event holds eight distinct eligible
  characters and an eligible partner who is none of the eight. The most
  recent resolved lineup may hold eight or nine distinct known characters.
  The partner choice is none or a known eligible character at friendship
  stage Friend or above. Gallery counts exist only for known notable
  trainers (and the player's Gallery count). A trainer reigning at the Masters
  has a Gallery count of at least 1, and so does the player reigning there.
  A tag selection exists only in a Masters run at matches 1-4 and names one
  to three distinct party slots.

## Presentation

The Masters' calls come from the caretaker, not from an official league.
Her first call says that Lorelei, retired to Four Island, spoke of the
player; beyond being the default [partner](#partner), Lorelei stays an
ordinary notable trainer with no guaranteed seat. Accepting names the four
pairs in order, which of the eight hold a Master's seat, and the partner who
will join the player. When the run starts, the caretaker introduces the
pairs in the lobby, and each hall's door names its pair along with the hall
and its condition. The Masters Gallery shows every recorded winner
([Masters Gallery](#masters-gallery)).

The player asks a partner with an outgoing phone call to a contact
([phone calls](../../game/src/match_call.c)); the contact agrees, and the
call says they will join the player at the next Masters event. The phone's
contact list shows who can be asked and who is the current partner. In the
tag matches the partner appears beside the player with their back pic, and
in the final they are the opponent and the last opponent of the event, with
their own lines. Dialogue cannot assume Lance ends the Masters, and the
Masters never calls its winner a regional Champion.

## Integration

Existing code to review, not new APIs, for the tag match path:
[partner battle setup](../../game/src/battle_special.c),
[partner parties](../../game/src/battle_partner.c),
[AI flag setup](../../game/src/battle_ai_main.c),
[party choice](../../game/src/script_pokemon_util.c),
[money and experience](../../game/src/battle_script_commands.c),
[Masters room scripts](../../game/data/scripts/wayfarer_masters_league.inc),
and [phone calls](../../game/src/match_call.c) for contacts and asking.

## Balance report

The [league balance report](leagues.md#balance-report) also shows, for each
invitation, the Master seats in the frozen lineup, and after the run the
notable Masters and the Masters Gallery; for a selected Masters event, each
trainer's Master status, the Master seats, and that the aloof rule is off.
It doesn't show the Masters' tag format yet: its Masters lineups still take
five seats, with no partner, pairs, or three-member tag teams
([Later](#later)).

## Acceptance

Required implementation evidence (not yet run), alongside
[Leagues acceptance](leagues.md#acceptance):

1. **Registry.** The build holds at least nine eligible trainers (the eight
   and a partner); Lorelei is eligible; every eligible trainer has a back
   pic for partnering.
2. **Master.** A trainer who reigned at only one of Indigo and Hoenn is not a
   Master, and a Masters title never makes one; the second of the player's
   Indigo and Hoenn first wins makes them a Master.
3. **Lineup.** No base lineup, and every aloof trainer eligible however far
   above the others; a Master seated over a higher-scoring trainer who is
   not one, more than eight Masters seated by league score with ties by
   `characterId`, a fatigued Master keeping the seat, and Master status
   ignored at Indigo and Hoenn; eight seats with the eight highest scores
   and ties at the last seat broken by `characterId`; the partner never
   seated (a Master who is the partner gives up the seat and the next score
   moves in); the same eight whether the player accepts or declines with the
   same partner; pairs taken two by two in battle order; fatigue read from
   an accepted Masters event including its partner.
4. **Contacts.** A trainer is a contact exactly when their friendship stage
   is Friend or above; the number is handed over once, when the score first
   crosses the Friend threshold, by a first win in each kind of battle (both
   opponents after a won tag match) or by finished haunt quests; a loss, a
   decline, partnering, and debug battles add none; Tate & Liza give no
   number.
5. **Partner.** With no ask, the partner is Lorelei, with or without her
   number; asking any contact always succeeds and sets the partner, the latest
   ask winning; a trainer below Friend and Tate & Liza cannot be asked, and no
   one can be asked before the player is a Master; an ask while an accepted
   Masters event waits leaves that event's partner and lineup unchanged and
   applies to the next. Accepting saves the partner with the lineup, and
   golden lineups cover different partners, matched between host tooling and
   game C.
6. **Title and Gallery.** A decline adds the new champion's Gallery win and
   no reign flag; a loss in any hall or leaving before the final crowns the
   lineup's strongest, and a loss in the final crowns the partner, each with
   a Gallery win; a Masters win adds the player's Gallery win and no status,
   on every Masters win, and a repeat win gives prize money, the title, and
   the Gallery win only. A new game has every Gallery count 0 and no partner
   choice. The Gallery shows the winners of declined Masters events too.
7. **Tag matches.** Each tag match is a two-versus-two partner battle
   against its pair, opponent A the weaker, under its hall's condition,
   with starting hazards on all four leads. Each notable brings the last
   three of their stored battle order, with fixtures for teams of one, two,
   three, and six members and one, two, and three aces, each member matching
   its stored species, level, moves, and item. Both opponents and the
   partner come from the event lineup despite today's scaling exclusions;
   the runtime partner slot shows the partner's name, class, pics, and AI,
   and no static partner changes. The AI of battlers 1, 2, and 3 is written.
   Prize money sums both opponents at their snapshot levels; the partner's
   Pokémon gain no experience; the match goes on while the partner can
   still fight. Reconstruction after a reload builds the same match.
   Mossdeep's Steven battle and every other partner or two-opponent battle
   are built as today.
8. **Party and heal.** At each door the player picks one to three able
   Pokémon, and cancelling starts nothing; after a win or a loss the fought
   Pokémon return to their own slots with their damage, experience, and
   evolutions, and the rest of the party is unchanged; a tag match loss
   restores the party before the blackout; damage carries from hall to hall;
   after hall 4 the whole party is fully healed, and the final uses the
   player's whole party against the partner's whole team. A reset at any
   moment of a tag match reloads a save with the whole party and no tag
   selection, and the match is fought again; a save carrying a tag selection
   loads with the party whole, resuming at that match's door.
9. **Presentation.** The Masters' calls come from the caretaker, the first
   naming Lorelei; accepting names the pairs, the Master seats, and the
   partner; the partner shows their back pic beside the player.
10. **Load validation.** Corrupt partner choices, tag
    selections, and Gallery counts are rejected without regenerating or
    rewarding, and so is a trainer reigning at the Masters with a Gallery
    count of 0. A save that went through one content change and was saved
    again before the next event resolved, then loaded under a build that
    removes a character with a Gallery count, or who is the
    partner choice, is pruned, not rejected.

Extend the [league tests](leagues.md#acceptance) with these cases.

## Later

- The caretaker's later calls and lines, and what the Masters Gallery shows
  beyond each winner's count.
- Explorer support for the Masters' tag format: the partner, eight seats,
  the pairs, and each notable's three for a tag match.
- Partnering as a source of friendship points
  ([Notable trainers](notable-trainers.md#friendship)).
- Picky partners who can refuse, by a higher stage or by trait (such as
  aloof).
- Gifts and trades with contacts.

## References

- [Sevii Masters PRD](../prds/sevii-masters.md)
- [Leagues](leagues.md)
- [Notable trainers](notable-trainers.md)
- [Notable haunts](notable-haunts.md)
- [Trainer AI](trainer-ai.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)

# Regional starter gifts

PRD: [Regional starter gifts](../prds/regional-starter-gifts.md)
Implemented: No

## Scope

This specification makes the whole PRD precise for the Wayfarer build: the
seven gifters, what earns each gift, who may receive it, the shared offer and
give flow, the saved state, and the two removals (Steven's Silph Co. gift and
Birch's Hoenn-list Johto starter). Line references are to main at
`d51c02f602`. Birch's visitor flow assumes
[PR #231](https://github.com/mzpkdev/pokemon-wayfarer/pull/231), where a visitor
never plays the Route 101 rescue and is thanked in the lab at their first talk.

It does not change the opening starters, rivals, wild starters, or any
gifter's existing quest steps and rewards.

## Behavior

### Terms

- **Origin** is the starting region: Pallet (Kanto), New Bark (Johto), or
  Littleroot (Hoenn), read by `WayfarerKanto_IsPalletOrigin`,
  `WayfarerIsJohtoOrigin`, and `WayfarerIsHoennOrigin`.
- A **gifter** offers one **trio**. A gift is **earned** once its deed is done,
  **offered** at each talk with the gifter while earned and not taken, and
  **taken** once the chosen Pokémon has been added to the party or PC.

### The gifters

| Gifter | Where they offer | Trio (slot 0, 1, 2) | Earned when | Not offered to |
| --- | --- | --- | --- | --- |
| Oak | Pallet lab, `PalletTown_Lab_hns` | Bulbasaur, Charmander, Squirtle | The player has a first win against Blue | Pallet origin |
| Elm | New Bark lab, `NewBarkTown_Lab_hns` | Chikorita, Cyndaquil, Totodile | The player shows Elm a hatched Pokémon | New Bark origin |
| Birch | Wherever Birch stands: his lab, Route 101, or Route 103 | Treecko, Torchic, Mudkip | Norman is beaten (`FLAG_DEFEATED_PETALBURG_GYM`) | Littleroot origin |
| Cozmo | Fallarbor, `FallarborTown_CozmosHouse` | Snivy, Tepig, Oshawott | The Meteorite is returned (`FLAG_RECEIVED_TM_RETURN`) | No one |
| Baoba | Safari Zone Gate, `SafariZoneGate_SafariZoneEntrance_hns` | Chespin, Fennekin, Froakie | His challenge is finished (`VAR_BAOBA_QUEST_STATE >= 5`) | No one |
| Samson Oak | His villa, `Melemele_PlayerHouse_hns` | Rowlet, Litten, Popplio | Melemele Isle's survey is reported (`FLAG_CAUGHT_ALL_MELEMELE`) | No one |
| Celio | One Island Pokémon Center | Grookey, Scorbunny, Sobble | The Network Machine is repaired (`FLAG_WAYFARER_SEVII_CELIO_REPAIR_COMPLETE`) | No one |

The flags in Birch's and Cozmo's rows are the Hoenn bank's copies, as Hoenn map
scripts assemble them. Nothing here adds a badge, league, Hall of Fame, or
Trainer Rating condition.

A player who is not offered a gift never hears about it: the gifter's other
talk is unchanged. Each player can take at most one gift per gifter, so a
Pallet player ends with the opening trio plus six gifts, as does every origin.

### Earning conditions

**Oak: beat Blue.** Earned when the notable-trainer first-win bit for
`NOTABLE_TRAINER_BLUE` is set (`NotableFriendship_CountFirstWin`,
`game/src/notable_friendship.c:98`). Every Wayfarer battle against Blue
counts through it: the Pallet lab rival (`TRAINER_WAYFARER_KANTO_BLUE_*`),
Indigo and its league events (`TRAINER_WAYFARER_INDIGO_BLUE`), and the Saffron
Dojo rematch (`TRAINER_BLUE_DOJO_HNS`), all aliased to Blue in
`game/src/data/notable_trainers/catalog.h`. A loss never counts. Add a public
read, `NotableFriendship_HasFirstWin(characterId)`, and a script special that
takes the character id in `VAR_0x8004` and returns the bit in `VAR_RESULT`. Do
not use the friendship score: quest points also raise it.

**Elm: hatch an Egg and show him.** At a talk, Elm asks to see a Pokémon that
hatched from an Egg. The player picks one with `special ChoosePartyMon`. It is
earned, and offered at once, when the picked Pokémon is not an Egg and its met
level is 0, which `AddHatchedMonToParty` (`game/src/egg_hatch.c:391`) writes
for every hatch. A new special, `IsPartyMonHatched`, reads party slot
`VAR_0x8004`. Any hatched Pokémon counts, including a Togepi from the Mystery
Egg. Picking another Pokémon gets a short "that one wasn't hatched from an
Egg" line, and cancelling ends the talk; both can be repeated. Elm asks only
while the gift is not taken, and showing is not saved: a player who declines
the choice is asked to show a hatched Pokémon again next time. Elm's ask comes
after his owed Master Ball and S.S. Ticket handoffs.

**Birch: beat Norman.** Earned by `FLAG_DEFEATED_PETALBURG_GYM`, and offered
only once Birch's own opening for this player is over (`VAR_BIRCH_LAB_STATE`
is 5: the Pokédex handover for a Hoenn-origin player, the lab thanks for a
visitor). The offer goes at the top of `ProfBirch_EventScript_RatePokedexOrRegister`
(`game/data/scripts/prof_birch.inc:54`). That script is Birch's talk on Route
101 and Route 103 (`Route101/map.json:93`, `Route103/map.json:170`), and his
lab talk reaches it after the visitor thanks and the National upgrade. A
player can therefore claim the gift wherever Birch is that day, before the
Pokédex rating.

**Cozmo: return the Meteorite.** Earned by `FLAG_RECEIVED_TM_RETURN`, which his
existing thanks sets after TM Return. The offer follows the TM handoff in the
same talk, and on later talks it replaces his "gave Meteorite" line until taken.
His offer line explains the Unova Pokémon in his own words, as specimens from a
far-off region he corresponds with.

**Baoba: finish the Safari challenge.** Earned at `VAR_BAOBA_QUEST_STATE >= 5`.
The offer follows his stage-4 thanks in the same talk, then sits at the top of
`..._BaobaDone`, so later talks offer it before the "thriving" line. His is the
only Baoba challenge; the Gold Teeth Baoba in Fuchsia
(`game/data/scripts/wayfarer_fuchsia_safari.inc`) is not a gifter.

**Samson Oak: survey Melemele.** Earned by `FLAG_CAUGHT_ALL_MELEMELE`, which
his villa dialogue sets when the player reports Melemele Isle's Alolan Rattata,
Raticate, Meowth and Persian as caught
(`Alola_Melemele_SamsonOakVilla_Dialogue`, `Melemele_PlayerHouse_hns/scripts.inc`;
the report branches that set the flag end in `release`). The offer follows the
Melemele report in the same talk, before his "keep exploring" or all-isles
lines. On later talks it goes first in his dialogue, before the Tapu and survey
branches. The PRD's original deed, the Melemele totem, appears only after all
four isles are surveyed (`Alola_Melemele_TotemBattle_Trigger`), so it was
moved earlier.

**Celio: repair the Network Machine.** Earned by
`FLAG_WAYFARER_SEVII_CELIO_REPAIR_COMPLETE`
(`game/data/scripts/wayfarer_sevii/story/celio_network.inc`). Both completion
paths set the flag, show the repair-finished line, then go to the offer.
`WayfarerSevii_Celio_RepairComplete` offers it on later talks until taken. The
gift is a deliberate exception to that file's rule that Sevii story scripts
never touch the PC or Pokédex: a gift Pokémon may go to the PC and is
registered like any received Pokémon.

### The offer

Every gifter uses one shared script, `StarterGift_EventScript_Offer`, with the
gifter's id in `VAR_0x8004`:

1. The gifter's own offer line, naming the deed ("You beat my grandson!", "The
   SAFARI ZONE is thriving!").
2. A three-way choice of the trio's species names, plus "Not now".
3. For the picked species: its picture (`showmonpic`) and cry, and a yes/no
   confirmation. No returns to the choice.
4. The give (below), then the gifter's short farewell line.

"Not now", B, or a "no" at the confirmation ends the offer with a short "come
back any time" line. Nothing is saved, and the next talk offers it again.

### The give

A special, `StarterGift_Give`, takes the gifter id and slot. It resolves the
species and gives it at level 5 with no held item.

- **Party, then PC.** It goes to the party when there is room, else to the PC,
  through `Common_EventScript_GiftMon`, with the fanfare, the nickname prompt,
  and the forced naming of the Nuzlocke option.
- **No room anywhere.** When the party and PC are both full (`MON_CANT_GIVE`),
  the gifter says so, nothing is saved, and the offer waits.
- **Receipt.** The gifter's taken flag is set only after the Pokémon has been
  added. A repeat talk never gives twice.
- **Pokédex.** The Pokémon is registered seen and caught like any received
  Pokémon.
- **Randomizer.** The species follows the "starter and gift Pokémon" option
  (`RANDOMIZE_STARTER_AND_GIFT_MON`, reason
  `RANDOMIZER_REASON_STARTER_AND_GIFT_MON`) at every point in the game. Plain
  `givemon` after the first Pokémon uses the fixed-encounter option instead
  (`ScriptGiveMon`, `game/src/script_pokemon_util.c:655`), so the special
  resolves the species itself and gives it without a second roll. The one-type
  challenge does not apply: it belongs to the opening starter.

The trio table, the taken-flag lookup, and the earned and offered checks live in
one C file, `game/src/wayfarer_starter_gifts.c`, so every gifter shares one
definition. Constant data stays in ROM tables.

### Saved state

One taken bit per gifter, seven in all, and nothing else: showing Elm a hatched
Pokémon and declining an offer are not saved.

| Gifter | Bank for the taken bit |
| --- | --- |
| Oak, Elm, Baoba, Samson Oak | The shared Kanto–Johto coast bank (`WAYFARER_COAST_FLAG_ID`), which has free slots |
| Birch, Cozmo | The Hoenn bank, at an Emerald-unused source id, as `FLAG_MET_HOENN_RIVAL_AS_VISITOR` is |
| Celio | The Sevii bank |

Each bit must be readable from its gifter's map scripts. The checks for "not
offered to" read the origin, never a gift bit. Prerelease saves need no
migration.

### Removals

**Steven's Silph Co. gift.** Remove `LOCALID_SILPHCO_STEVEN` from
`SaffronCity_SilphCo_hns/map.json` and `SaffronCity_SilphCo_EventScript_Steven`
with its movements and text. `FLAG_GOT_HOENN_STARTER` is left unused.
`tools/mapjson/tests/test_silph_liberation.py` asserts the gift exists and is
rewritten to assert that Steven and the gift are gone.

**Birch's Hoenn-list Johto starter.** In `LittlerootTown_ProfessorBirchsLab`,
remove the Johto-starter states of `VAR_DEX_UPGRADE_JOHTO_STARTER_STATE` (4, 5,
and 6): the readiness check (`HasAllHoennMons`), the table layouts, the
Chikorita, Cyndaquil, and Totodile ball objects and scripts, and the "can have
any one of these rare Pokémon" branches. States 1 to 3, the National upgrade,
stay. Completing the Hoenn list keeps Birch's Pokédex rating and earns nothing
else. The ball objects' hide flags may be left unused.

### Interactions

- **Johto visitors.** Elm gives Pallet and Littleroot players nothing else
  from the removed #75 flow: no errand, starter choice, Mystery Egg, or Silver.
  Only this gift is added.
- **Hoenn visitors.** Birch's lab thanks (PR #231) is unchanged and gives no
  starter; his gift comes after Norman.
- **Rivals.** Taking a gift never changes a rival's team, the player's origin,
  or any recorded opening starter (`VAR_STARTER_MON`,
  `VAR_HOENN_STARTER_CHOICE`, the Pallet opening's `starterSlot`).
- **Gifters' quests.** Each existing quest keeps its steps and other rewards.
  A gift offer never blocks or replaces them.

### Documents to update with the implementation

- The PRD's status: implemented, with this specification linked.
- [Home Pokédex lists](../prds/home-pokedex-lists.md#completion-and-rewards):
  the Hoenn list no longer earns Birch's Johto starter.
- [Johto, Hoenn, and Kanto starting regions](../prds/wayfarer-regional-start-choice.md):
  point its starter-gift lines at this design.

## Validation

- **Mechanics tests** (`make check`): each trio resolves to its three
  species; with the randomizer option on, a gift is rerolled under the
  starter-and-gift reason and only once; the taken bit is set only after a
  successful give; `IsPartyMonHatched` is true for met level 0 and false for a
  wild catch and an Egg; `NotableFriendship_HasFirstWin` follows the bit.
- **E2E**, one journey per gifter, starting from state set by the harness:
  - Earned: the offer plays, the pick arrives at level 5, and a second talk
    offers nothing.
  - Not earned: the gifter's talk is unchanged.
  - Declined, then accepted on a later talk.
  - Full party: the gift goes to the PC.
  - Not offered: a Pallet player at Oak, a New Bark player at Elm, and a
    Littleroot player at Birch hear no offer.
  - Elm: showing a wild-caught Pokémon earns nothing.
  - Birch: the offer plays at his Route 103 placement.
- **Removals:** the Silph Co. lobby has no Steven; Birch with a complete Hoenn
  list rates the Pokédex and offers no Johto starter; the existing Birch,
  Pallet, New Bark, and Silph journeys still pass.

## References

- [Regional starter gifts PRD](../prds/regional-starter-gifts.md)
- [Hoenn independent story beats](hoenn-independent-story-beats.md)
- [Johto independent story beats](johto-independent-story-beats.md)
- [Sevii independent story beats](sevii-independent-story-beats.md)
- [Notable trainers](notable-trainers.md), for the first-win bit
- [Pokédex obtainability audit](../research/pokedex-obtainability.md#starters)

# Johto legendary ceremony gates

Source audit of Wayfarer's HNS scripts at `9ab8058a6e`. A runner traced the
reward and encounter paths; the root reconciled the main reward and alternate
Pewter branch against source. No ROM journey was performed.

## Main ceremony

Baoba's Route 39 Gold/Silver choice selects the main legendary branch. Gold sets
the Ho-Oh branch; Silver sets Lugia. The Radio Tower Director gives the matching
Wing after Archer's defeat. The five Kimono Girl battles give the matching Bell
and enable the corresponding ceremony state.

| Main branch | Radio Tower reward | Kimono reward | Story encounter |
| --- | --- | --- | --- |
| Lugia | Silver Wing | Tidal Bell | Level 50, Whirl Islands |
| Ho-Oh | Rainbow Wing | Clear Bell | Level 50, Tin Tower roof |

The actual theater activation still runs through Clair, Dragon's Den, and Elm.
Elm's Master Ball scene sets theater progress to 4; a subsequent Ecruteak scene
advances it. Radio Tower's Wing handoff does not itself activate the gauntlet.

The final encounter maps check legendary progress values rather than Wing/Bell
inventory. Theater completion sets the relevant value to 2, enabling the story
encounter. Therefore simply giving the player a Wing does not reproduce the
main ceremony activation. Conversely, replacing Elm's activation with a local
start would not automatically preserve Radio Tower as a prerequisite.

Clear Bell does have a local access check in Ecruteak's sage office. The theater
finale also clears that guard. This access interaction is distinct from the
encounter's ceremony state.

Evidence, paths relative to `game/data/maps/`:

- `Route39_hns/scripts.inc:113`: branch selection.
- `GoldenrodCity_RadioTower_5F_hns/scripts.inc:149`, `:215`, `:222`: Archer aftermath and Wings.
- `DragonsDen_Cavern_hns/scripts.inc:28`, `BlackthornCity_hns/scripts.inc:27`,
  `NewBarkTown_Lab_hns/scripts.inc:441`: Clair/Elm activation.
- `EcruteakCity_Theater_hns/scripts.inc:785`, `:832`: Bells and ceremony progress.
- `EcruteakCity_SageOffice1_hns/scripts.inc:21`: Clear Bell access check.
- `WhirlIslands_LugiaChamber_hns/scripts.inc:5`, `:180`: state dispatch and level 50 Lugia.
- `TinTower_RoofDay_hns/scripts.inc:5`, `:177`: state dispatch and level 50 Ho-Oh.

## Alternate legendary through Pewter

Pewter's Gramps gives the Wing for the opposite legendary: selecting Lugia on
Route 39 makes him give Rainbow Wing and enable Ho-Oh; selecting Ho-Oh makes him
give Silver Wing and enable Lugia. He sets that legendary's progress to 4 and
clears its access blocker. The encounter uses the level 70 branch without the
Kimono ceremony.

His interaction checks the branch choice and whether the Wing is already owned.
It has no badge, League, Radio Tower, or main-ceremony completion check. His map
object has no hide flag. Although the encounter labels say postgame, the reward
script does not enforce a postgame condition. This is an alternate early activation
route in the open Kanto network; physical traversal to and capture of either
legendary on an early save were not tested.

Evidence: `PewterCity_hns/scripts.inc:17` and `map.json:51`;
`WhirlIslands_LugiaChamber_hns/scripts.inc:174`, `:207`;
`TinTower_RoofDay_hns/scripts.inc:171`, `:204`.

## Design implications

The user accepts Radio Tower as a prerequisite for the Kimono legendary encounter.
The smallest coherent proposed main chain is Radio Tower and its Wing reward,
then the existing Kimono trial, Bell, and ceremony. Remove the unrelated Clair/Elm
activation bridge while explicitly retaining the Radio Tower condition; do not
assume a Wing inventory check already supplies it.

This preserves reward owners and the original adventure. However, making Radio
Tower independently available means that its completion alone does not establish
late-game readiness. The broader late-game reward exception still needs a chosen
progression threshold. Any such policy must cover the alternate Pewter unlock as
well as the main ceremony, or the level 70 encounter path bypasses it.

No new characters, factions, or replacement ritual plot are needed for this
direction. Exact readiness thresholds and trigger wiring remain design/spec work.

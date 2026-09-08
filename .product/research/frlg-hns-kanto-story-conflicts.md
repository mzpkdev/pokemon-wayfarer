# HNS Kanto stories against the FRLG baseline

## Scope and decision rule

This inventory supports [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md).
FRLG is Kanto's narrative baseline; preserve as much HNS Kanto content as
possible through adaptation. Wayfarer's date is deliberately imprecise.
Recommendations below are options for product review, not approved cuts.
The full FRLG Cinnabar port and a persistent S.S. Anne accepting any S.S.
Ticket are selected. Anne travel routes belong in a separate specification. Subsequent accepted
quest and League choices are recorded in the PRD; alternatives in this audit
remain historical analysis, not competing requirements.

Evidence comes from scripts and map metadata in this task's source snapshot.
A script's presence does not establish that every inherited scene is reachable
in Wayfarer. Full initialization, conditional compilation, and runtime
reachability remain implementation-audit work. The older traversal inventory
also includes standalone HNS restrictions that Wayfarer has since changed.

## Cinnabar: selected full FRLG port

The user selected a full FRLG Cinnabar port, with the island remaining intact. It is a cleaner authoring starting point than restoring
intact buildings inside HNS's damaged island. This is an engineering judgment,
not a measured implementation estimate. The FRLG exterior already connects to
the Mansion, Gym, laboratory, Pokémon Center, and Mart. HNS has only a Center
building warp, plus an unrelated New Bark warp that must not be carried over
as an intended regional connection.

The source layouts differ: FRLG is 24 by 20 tiles; HNS is 72 by 44. HNS Route 21
is 24 tiles wide and Route 20 is 20 tiles high, which makes the FRLG island a
plausible boundary fit, but dimensions alone do not prove shore alignment or
seamless rendering. Both neighboring maps' reciprocal connections need review.
FRLG uses zero offsets; HNS's island uses +4 north and -1 east offsets.

Retain the HNS routes and verify connection geometry in Porymap. Also adapt
interior exits, Fly/healing locations, map identity, services, encounters, and
story state. A full map port does not mean blindly importing Bill's Sevii trip.
The HNS island is not a planned later Wayfarer state. Its source may remain for
standalone HNS, but no resizing, alternate-map routing, or eruption transition
is required. Earlier discussion established that equal map dimensions would
not be necessary for an alternate layout; that possibility is not a product
commitment. Any future eruption needs a separate design.

Evidence: [FRLG island](../../game/data/maps/CinnabarIsland_Frlg/map.json),
[HNS island](../../game/data/maps/CinnabarIsland_hns/map.json),
[Route 20](../../game/data/maps/Route20_hns/map.json),
[Route 21](../../game/data/maps/Route21_hns/map.json), and
[FRLG island scripts](../../game/data/maps/CinnabarIsland_Frlg/scripts.inc).
Layout dimensions are recorded in `game/data/layouts/layouts.json`.

## Conflicts and preservation options

| HNS content | Conflict with the selected baseline | Options and recommendation |
| --- | --- | --- |
| Cinnabar eruption and Blaine at Seafoam | Treats Cinnabar's destruction and Gym relocation as completed history. | Selected: full intact FRLG island/Gym, with laboratory, Mansion, Center, and Mart. Adapt destruction dialogue and omit Blaine's Seafoam relocation. No eruption transition or alternate island is required. |
| Blue as Viridian Leader and former Champion | Giovanni must still run the FRLG Gym finale; Blue also appears as the rival. | Giovanni awards the initial Earth Badge. Preserve Blue's leadership content for possible later succession, without a second badge. Rewrite prior-championship claims that contradict the player's story. |
| Janine, Koga, and the League | HNS Janine awards Fuchsia's badge while Koga serves in the League; FRLG has Koga at Fuchsia. | Recommend Koga at the Gym and Janine retained as apprentice/optional challenger. Decide whether Koga also serves the League in the loose chronology, or whether a later promotion/alternate League member is required. Do not silently gate Johto behind Kanto to solve it. |
| Machine Part theft | Grunt dialogue assumes Rocket's disbandment and a Johto return. | Preserve sabotage, pursuit, part recovery, and repair. Rewrite dialogue according to actual progress, or avoid claims about the whole organization's status. No need to delete the quest. |
| Power Plant and Zapdos | Operating HNS facility differs from FRLG's abandoned exploration site. | Options: operational wing plus abandoned annex; repair story set in a facility being recommissioned; retain HNS site with adapted exploration. Recommend working/abandoned sections if bounded map work supports them. An annex is a proposal, not an existing verified destination. One unique Zapdos capture reward, with normal retry rules. |
| Misty's date | Plant repair activates her return-to-Gym sequence, entangling the badge with a separate quest. | Selected: retain Machine Part repair → Cape date → Gym return. The League circuit PRD explicitly allows this badge prerequisite; it does not gate settlement travel. Adapt conflicting dialogue without moving the outing after the badge. |
| Lavender radio station, Soul House, and Fuji | Station occupies Tower's role; Fuji is already safe; radio guard assumes Johto's takeover has happened. | Restore Tower/Fuji. Relocate radio services to a studio or shared building and condition Johto references. Keep Soul House if it complements rather than replaces the Tower story. Choose exact studio placement later. |
| Vermilion radio Snorlax | Additional encounter beyond FRLG Route 12/16; radio awakening could bypass Fuji's Flute reward if indiscriminately applied. | Keep as a distinct optional third encounter, relocate it, or remove the duplicate. Preservation-first recommendation: retain only if its location has a normal bypass, and restrict radio awakening to this encounter. Physical Flute remains required for the FRLG pair. No deletion is selected. |
| Copycat's lost doll and Magnet Train Pass | Mother attributes doll to a boy three years ago; quest and train use plant-repair state. | Preserve doll search, fan-club recovery, Pass, and optional train-repair connection. Rewrite exact dates and donor identity; if a FRLG doll-gift event is included, reconcile ownership explicitly. |
| Safari Warden and Baoba | HNS text describes departure/closure or changed management, while FRLG requires the Warden and Gold Teeth reward. | Keep the Warden present for the local quest and HNS Safari activities. Rewrite travel/management dialogue to support the Johto Safari without declaring Fuchsia's required quest unavailable. Some closure strings may be inactive; do not treat text alone as a live gate. |
| Bill's grandfather and regional-form teleporter | Occupy the same house needed for Bill's rescue. | Preserve both; share the house or arrange the grandfather's visit after the rescue. Keep optional services recoverable without rewriting ship eligibility. |
| Silph staff, services, and Steven gift | Peaceful lobby and gift scenes conflict with occupation; Steven may assume a prior meeting. | Preserve by staging staff/services around the occupation or relocating them temporarily. Restore after rescue and use first-meeting dialogue when needed. Ordinary gifts must remain recoverable. |
| Giovanni/Silver/Celebi scenes | Route 22 flashback discusses Giovanni's defeat; Tohjo dialogue names a three-year gap. | Preserve as an explicitly framed time-travel story, or allow it after the player's Giovanni finale with adapted temporal references. Recommend a named post-finale eligibility decision rather than presenting unplayed defeat as completed. Exact Celebi/Johto dependencies need a separate state audit. |
| Red and past-hero references | HNS dialogue and Gym records can imply Red already performed the player's pending Kanto campaign. | Preserve Red and Mt. Silver as a separate trainer challenge. Remove or condition claims about the same Rocket/Gym feats and exact elapsed years. Do not remove Red merely to erase chronology. |
| League roster and Blue's Champion scene | Importing FRLG League assumptions can conflict with Wayfarer's fixed regional League circuit and Koga's current role. | Preserve the circuit's qualification, order, scaling, and rewards. Explicitly choose how FRLG rival/Champion scenes map to it; no automatic roster replacement in this map port. This is more than a dialogue edit. |

### Source evidence by conflict

Paths below are directly reviewable; line numbers identify the inspected source
snapshot and may move with edits.

- Eruption/Blue: [Cinnabar scripts](../../game/data/maps/CinnabarIsland_hns/scripts.inc), lines 36–95; [Viridian Gym](../../game/data/maps/ViridianCity_Gym_hns/scripts.inc), lines 7, 134–160.
- Janine/Koga: [Fuchsia Gym](../../game/data/maps/FuchsiaCity_Gym_hns/scripts.inc), lines 44–77; [Koga League room](../../game/data/maps/PokemonLeague_KogasRoom_hns/scripts.inc), lines 15–23, 93.
- Machine Part: [Route 24](../../game/data/maps/Route24_hns/scripts.inc), lines 140–153; [plant manager](../../game/data/maps/Route10_PowerPlantBackRoom_hns/scripts.inc), lines 35–57.
- Plant/Zapdos: [plant entrance](../../game/data/maps/Route10_PowerPlantEntrance_hns/scripts.inc), line 193; [Route 10](../../game/data/maps/Route10_hns/scripts.inc), line 52.
- Misty: [Route 25](../../game/data/maps/Route25_hns/scripts.inc), lines 95–130; [Cerulean Gym](../../game/data/maps/CeruleanCity_Gym_hns/scripts.inc), line 186.
- Lavender: [Lavender house](../../game/data/maps/LavenderTown_House2_hns/scripts.inc), line 12; [Soul House](../../game/data/maps/LavenderTown_SoulHouse_hns/scripts.inc), line 18; [radio station](../../game/data/maps/LavenderTown_RadioStation_hns/scripts.inc), lines 16–62.
- Snorlax: [Vermilion](../../game/data/maps/VermilionCity_hns/scripts.inc), line 179 onward. The upgrade-flag test is commented out; awakening actually checks the tuned radio station. Adapt the real condition, not only the upgrade reward.
- Copycat/train: [house 1F](../../game/data/maps/SaffronCity_CopyCatsHouse_1F_hns/scripts.inc), line 36; [house 2F](../../game/data/maps/SaffronCity_CopyCatsHouse_2F_hns/scripts.inc), lines 28, 143; [station](../../game/data/maps/SaffronCity_TrainStation_hns/scripts.inc), line 26.
- Safari: [Fuchsia](../../game/data/maps/FuchsiaCity_hns/scripts.inc), lines 24–33, 124–134; [house](../../game/data/maps/FuchsiaCity_House2_hns/scripts.inc), line 19.
- Bill's grandfather: [Bill's house](../../game/data/maps/Route25_BillsHouse_hns/scripts.inc), lines 368, 487, 538.
- Silph: [lobby](../../game/data/maps/SaffronCity_SilphCo_hns/scripts.inc), lines 4, 64, 105, 148.
- Celebi: [Route 22](../../game/data/maps/Route22_hns/scripts.inc), lines 26–45, 293–324. Triggers check Celebi and local state, not the future FRLG finale. [Tohjo room](../../game/data/maps/TohjoFalls_GiovanniRoom_hns/scripts.inc), lines 194–227, explicitly references three years.
- Red: [Red's house](../../game/data/maps/PalletTown_RedsHouse_1F_hns/scripts.inc), line 24; [Mt. Silver summit](../../game/data/maps/MtSilver_SummitDay_hns/scripts.inc), line 29.

## Content to preserve with ordinary integration work

These are not reasons to cut HNS content. Event placement and reward state still
need verification when FRLG scenes are added to the same locations.

- Suicune/Eusine's Kanto continuation, sharing Route 25 with Misty and Bill.
- Silver's Mt. Moon encounter, sharing the cave with the fossil adventure.
- Route 25's Trainer gauntlet as a distinct challenge from Route 24's FRLG
  Nugget Bridge; avoid duplicate one-time prize ownership.
- Gym outings, rematch invitations, and the Fighting Dojo, adjusted for each
  character's selected current role. Preserve Wayfarer's own rematch rosters.
- Oak's research and Mt. Silver access under existing Wayfarer progression.
- Compatible gifts, trades, services, and local flavor after checking dialogue
  for references to completed adventures and fixed dates.

Evidence: [Route 25](../../game/data/maps/Route25_hns/scripts.inc), lines 8, 15,
69, 137; [Mt. Moon](../../game/data/maps/MtMoon_Cave_hns/scripts.inc), lines 18,
105; [Dojo](../../game/data/maps/SaffronCity_FightingDojoVIP_hns/scripts.inc),
lines 478–486; [Oak's lab](../../game/data/maps/PalletTown_Lab_hns/scripts.inc),
lines 25–37. Standalone post-OBC/Sinjoh variants are excluded by some Wayfarer
conditionals and are not promised here as currently playable content.

## League detail confirmed during follow-up

Wayfarer currently uses Will, Koga, Bruno, Karen, and Lance for both the Kanto
and Johto Indigo runs. The runs have separate regional completion and scaling;
they are not already separate FRLG and HNS lineups. See
[room dispatch](../../game/src/league_circuit.c) and
[Champion dispatch](../../game/data/maps/PokemonLeague_ChampionsRoom_hns/scripts.inc).

A proposed FRLG Kanto lineup of Lorelei, Bruno, Agatha, Lance, then Blue would
therefore be an explicit content change, while preserving circuit eligibility,
order, scaling, and reward accounting. Keeping the current lineup and placing
Blue's finale outside it is an alternative. The user selected distinct FRLG Kanto and HNS Johto lineups. Blue's origin-specific progression is now selected: Kanto-origin players
receive the rivalry; later encounters retire earlier Blue chapters without
completing host adventures. Visitors receive introductions and the Champion
battle without the rival itinerary. See the PRD for trigger and retry rules.
Koga's Gym/Johto League duties need reconciliation without requiring a future
succession system to ship this port.

## Selected Vermilion boarding approach

Add Board S.S. Anne to the existing Vermilion sailor menu, retaining Slateport,
Southern Island, Birth Island, Faraway Island, Battle Frontier, and Exit.
Anne boarding checks the shared Ticket and warps to its interior; exiting
returns to the current dock. Reuse existing terrain and the visible Aqua
object. No second berth, dock variant, or dock `map.bin` edit is required by
this design. Anne's future travel routes remain separately specified.

Evidence: [Vermilion dock events](../../game/data/maps/VermilionCity_PortInside_hns/map.json)
contain one exterior return warp, a sailor, and an Aqua object. The
[boarding scripts](../../game/data/maps/VermilionCity_PortInside_hns/scripts.inc)
already drive travel through the sailor rather than a ship-door warp.
The inherited destinations stay on this same menu, with their existing option
identifiers and eligibility. Anne must remain accessible to any S.S. Ticket
holder without opening other services before their regular eligibility.
Olivine and Slateport interactions remain unchanged. See the PRD.

## Cross-region follow-up

Preserving HNS Johto also requires reviewing its claims that Giovanni has
already disappeared or Rocket has already been defeated. Those scenes must
remain coherent if Johto is played before Kanto. Do not solve the discrepancy
by requiring the player to finish Kanto first. This inventory establishes the
conflict, but is not an exhaustive Johto dialogue/state audit.

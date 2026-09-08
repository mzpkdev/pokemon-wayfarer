# Trainer-only encounters and story battle gates

Status: Source audit and design recommendations. No gameplay changes.

## Question and baseline

Which rival scenes can be deferred while the player has no usable Pokémon, and
which grunts or authored trainers must continue protecting their local objective?
The proposed trainer-only mode lets unprotected players pass ordinary trainers;
that must not automatically rescue hostages, teach passwords, grant battle rewards
or end an occupation.

Runtime references use task `trainer-only-encounters` at `56fb1e68b4`. The regional
independence PRDs are upcoming designs, not implemented script behavior. FRLG and
Sevii source availability does not establish inclusion in the Wayfarer ROM.

Also read the uncommitted `frlg-kanto-story-on-hns-maps.md` draft in the separate
`frlg-kanto-hns-story-port` task. It proposes adapting FRLG stories onto HNS
geography and selected imported interiors, but leaves character/location conflicts
open. Its decisions are an upcoming overlay, not current Wayfarer content or a
settled choice for Giovanni/Blue, Lavender, Cinnabar or the ship.

## Product rules used for classification

| Source design | Consequence for this audit |
| --- | --- |
| [Johto independence](../prds/johto-independent-story-beats.md) | Well, Mahogany and Radio Tower can be played independently, but their internal rescues, passwords, battles and rewards remain ordered. Silver chapters remain at their locations, defer without blocking the host adventure, and stay recoverable. |
| [Hoenn independence](../prds/hoenn-independent-story-beats.md) | Local deliveries/rescues retain their real items and victories. Magma Hideout precedes both Aqua/submarine and Mossdeep branches; both precede Seafloor. Rival chapters must not become prerequisites for unrelated adventures. |
| [FRLG Kanto independence](../prds/frlg-kanto-independent-story-beats.md) | Celadon/Scope/Tower/Fuji remains a causal investigation; Silph is independent; both lead to Giovanni's finale. Rival deferral cannot block Bill, the captain, Fuji or Silph, nor silently consume a chapter. |
| [Sevii independence](../prds/sevii-independent-story-beats.md) | Rescue battles and both learned Warehouse passwords remain meaningful. Lorelei does not gate Dotted Hole; the Ruby does not gate travel. Temporary rival appearances remain recoverable. |
| [HNS traversal](../prds/hns-open-world-region-traversal.md) | Preserve approved public lanes around Silver and road blockers. The maiden Aqua rescue remains, but its sailor battle is optional. Passing does not complete story flags. |
| [Hoenn traversal](../prds/emerald-open-world-region-traversal.md) | Route 110/119 walls and the cable-car/Jagged Pass closure cannot be restored merely because enemies are present. Keep local team battles available off the public lane. |

The governing distinction is public travel versus resolving an adventure. A
guard can keep the hostage room, key, machine or boss objective unavailable while
the player can still leave and visit unrelated towns. Actor class alone is not
sufficient: a Rocket on a route and a Rocket holding a password need different
rules even if they share trainer graphics.

## Classification

| Code | Recommendation |
| --- | --- |
| D | Defer a rival's battle-only trigger or presence; no chapter/completion/reward writes. Restore it when both narrative and usable-party conditions hold. |
| R | Deferral needs a scene-state or recoverability change first, often because the host adventure removes the map staging or shares a variable/reward. |
| G | Retain a visible local objective guard and an explicit objective predicate. Refuse battle safely without a usable party; keep retreat possible. Do not grant access just by hiding the actor or skipping a trainer-sight check. |
| T | Public traversal must remain open under the regional PRD; preserve the local battle/reward separately. Do not retain an obsolete campaign roadblock. |
| U | Future port, identity, authored outcome or source-inclusion decision remains unresolved. Do not claim the scene is current or implementation-ready. |

Rows can combine classes: a rival may require R to defer without blocking T,
while the adjacent villain remains G. These are policy recommendations based on
the planned stories, not automatic authorization to edit every listed scene.

## Inventory

The source inventory follows below. Routine dungeon trainers with no distinct
state or objective effect are grouped; ordinary roadside trainers are excluded.

### Current Johto and HNS Kanto: Silver

Every numbered HNS Silver battle family was found in the source. A family can
have three starter-dependent trainer IDs; these are one scene, not three separate
chapters. Existing starter/origin checks remain distinct from future rival-story
prerequisites and the new usable-party condition.

| Encounter / trigger | Current state effect and evidence | Policy |
| --- | --- | --- |
| Cherrygrove #1, off-screen coordinate encounter | `CherrygroveCity_EventScript_TriggerSilver*`, [scripts:212](../../game/data/maps/CherrygroveCity_hns/scripts.inc#L212); starter branches at 248; completion hides Silver and sets `VAR_CHERRYGROVE_CITY_STATE=4` around 282. | R/T. Suppress before spawning or locking. Preserve the existing travel bypass and leave the chapter pending; do not write state 4 merely to skip it. |
| Azalea #2, coordinate approaches toward Ilex | `SilverTriggerTop/Bot`, [scripts:151](../../game/data/maps/AzaleaTown_hns/scripts.inc#L151), battles around 212; postscene advances `VAR_AZALEA_TOWN_STATE=6`. | R/T. Rival progression shares town state. Keep the public lane and independent Well/Ilex objectives usable without granting rival completion. |
| Burned Tower #3, persistent Silver interaction | `BurnedTower_1F_EventScript_Silver`, [scripts:101](../../game/data/maps/BurnedTower_1F_hns/scripts.inc#L101); after battle changes floor tile, drops player to B1F and sets `VAR_ECRUTEAK_CITY_STATE=3`. | R. Hiding the rival alone does not decouple Tower discovery. Provide a locally owned discovery path while preserving the rival chapter at its location. |
| Goldenrod Underground #4, two off-screen coordinate approaches | `SilverTriggerTop/Bot`, [scripts:348](../../game/data/maps/GoldenrodCity_UndergroundSwitches_hns/scripts.inc#L348), battles at 420; both postscenes hide Silver and write `VAR_GOLDENROD_CITY_STATE=9`. | R. Highest-priority state split: Radio Tower must remain completable while Silver is deferred, and completing the occupation must not erase his later encounter. |
| Victory Road #5, three-position corridor trigger | `VictoryRoadKanto_1F_Trigger`, [scripts:14](../../game/data/maps/VictoryRoadKanto_1F_hns/scripts.inc#L14); completion writes `VAR_ROUTE27_STATE=3`. Current Wayfarer also has a League-eligibility branch before the rival scene. | R/U. Preserve the existing League/travel decision; no skipped-rival completion. Reconcile the chapter with League entry separately rather than treating every corridor trigger as an ordinary trainer. |
| Mt. Moon #6, optional talk and Yes/No | `MtMoon_Cave_EventScript_Silver`, [scripts:17](../../game/data/maps/MtMoon_Cave_hns/scripts.inc#L17); battle completion hides Silver, reveals Indigo Silver and sets `VAR_PEWTER_CITY_STATE=2`. | D. Already declinable and off the through-lane. Temporarily suppress battle interaction/appearance without executing the writer. Do not resurrect the older forced-roadblock behavior. |
| Indigo Plateau Center #7, recurring contact battle | [scripts:66](../../game/data/maps/IndigoPlateau_PokemonCenter_hns/scripts.inc#L66); daily rematch handling. | D. No local adventure objective identified. Preserve normal rematch eligibility and do not mark a daily battle played when suppressed. |

Silver also appears in non-battle scenes. Sprout Tower's scene around
[scripts:41](../../game/data/maps/SproutTower_3F_hns/scripts.inc#L41) and Mahogany
B3F's scene around [scripts:157](../../game/data/maps/RocketHideout_B3F_hns/scripts.inc#L157)
must not automatically inherit a blanket "hide every rival" rule. The latter
writes Mahogany state 8; if deferred for narrative reasons, local hideout progress
needs its own owner. These are scene-policy cases, not ordinary trainer defeats.

### Current Johto: local adventure gates

| Adventure / encounter | Actual objective and source | Policy |
| --- | --- | --- |
| Slowpoke Well, Proton | `SlowpokeWell_B1F_EventScript_Proton`, [scripts:41](../../game/data/maps/SlowpokeWell_B1F_hns/scripts.inc#L41); battle precedes Rocket removal, Slowpoke/Kurt restaging, Azalea state 3 and return to Kurt's house. | G. Keep the rescue unresolved without victory. Other Well grunts are not automatically all mandatory; the rescue boss and any real access guards remain explicit. |
| Mahogany Hideout, password and Petrel chain | [B3F scripts:301](../../game/data/maps/RocketHideout_B3F_hns/scripts.inc#L301); Petrel advances password/town state. Murkrow, doors, later fights and generator resolution are in [B2F scripts:93](../../game/data/maps/RocketHideout_B2F_hns/scripts.inc#L93) and its later state writers. | G/R. Passwords, security doors, confrontation and generator/Electrode resolution remain local dependencies. Split Silver's shared state; do not remove guards to fake passwords or shutdown. |
| Radio Tower, fake Director/Petrel | `...EventScript_Petrel`, [5F scripts:17](../../game/data/maps/GoldenrodCity_RadioTower_5F_hns/scripts.inc#L17); battle then Basement Key and Goldenrod state 7. | G. Refuse safely without protection; preserve key acquisition and disguise/retry staging. Guard before transformation, not only before `trainerbattle`. |
| Radio Tower, Underground Storage Director | [scripts:9](../../game/data/maps/GoldenrodCity_UndergroundStorage_hns/scripts.inc#L9); Card Key handoff is part of the staff-rescue path. | G/R. The NPC is a reward/rescue actor, not a trainer to hide. Preserve actual rescue/key prerequisites while decoupling Silver. |
| Radio Tower, executives and Archer | [4F scripts:28](../../game/data/maps/GoldenrodCity_RadioTower_4F_hns/scripts.inc#L28), [5F scripts:143](../../game/data/maps/GoldenrodCity_RadioTower_5F_hns/scripts.inc#L143); Archer's battle precedes occupation cleanup, Goldenrod state 10 and Director/Wing staging. | G/T. Protect the occupation finale and reward. Do not preserve an unrelated Mahogany road gate just because the old Director script also changes it. |
| Tohjo Falls Giovanni/Celebi episode | [Giovanni room scripts:32](../../game/data/maps/TohjoFalls_GiovanniRoom_hns/scripts.inc#L32), battle around 109, later scene state 2. | G/U. Optional authored encounter with its own retry/outcome rules. This is HNS Giovanni, not the future FRLG Gym finale; no identity or state merge by inference. |

### Current Hoenn: local adventure gates

| Adventure / encounter | Actual objective and source | Policy |
| --- | --- | --- |
| Rusturf Tunnel Aqua grunt | [scripts:269](../../game/data/maps/RusturfTunnel/scripts.inc#L269), staging at 12 and later state at 337; Peeko/Devon Goods rescue and Briney follow-up. | G/T. Keep rescue/recovery earned; public regional travel and the independent ferry cannot depend on this fight. |
| Oceanic Museum Stern and two Aqua grunts | [2F scripts:4](../../game/data/maps/SlateportCity_OceanicMuseum_2F/scripts.inc#L4); scene spawns grunts before battles at 30/38, then cleanup, Parts handoff, healing and delivery state at 65 onward. | G/T/U. Guard before staging; preserve actual Parts and successful local sequence. Chained loss handling needs explicit adaptation. Route 110 remains independent of delivery. |
| Mt. Chimney Maxie | `MtChimney_EventScript_Maxie`, [scripts:49](../../game/data/maps/MtChimney/scripts.inc#L49); battle at 61 then conflict/character cleanup. | G/T. Protect the meteorite adventure, not the cable car or Jagged Pass travel lane. The planned theft prerequisite controls the local conflict's presence. |
| Magma Hideout Maxie | `MagmaHideout_4F_EventScript_Maxie`, [scripts:4](../../game/data/maps/MagmaHideout_4F/scripts.inc#L4); Groudon animation occurs BEFORE battle at 57. The post-battle writer commits `FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT`, Slateport/Harbor state 1 and grunt cleanup. | G. Guard the entire scene before irreversible staging, not only the battle call. A loss/refusal must not run its progression writer. Tabitha/routine final-floor grunts have ordinary battle scripts; map geometry must establish which are genuine security boundaries. |
| Aqua Hideout Matt | `...EventScript_Matt` and `SubmarineEscape`, [B2F scripts:25](../../game/data/maps/AquaHideout_B2F/scripts.inc#L25); victory continuation launches the submarine and sets `FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE`. | G/T. Preserve confrontation/escape objective; the public Lilycove sea route must remain independent. |
| Weather Institute Shelly | [2F scripts:46](../../game/data/maps/Route119_WeatherInstitute_2F/scripts.inc#L46); victory resolves occupation and leads to Castform. | G/T. Staff rescue and reward stay gated. The Route 119 bridge closure is a separate removed travel gate. |
| Mossdeep Space Center | [2F scripts:31](../../game/data/maps/MossdeepCity_SpaceCenter_2F/scripts.inc#L31); decline/retreat, three-grunt sequence, then Steven partner battle around 173; success clears invasion and stages Dive. | G/U. Preserve local sequence and refusal. Never start a partner/chained battle without a valid party. The planned Magma prerequisite remains; the Aqua branch does not become a new prerequisite. |
| Seafloor Archie | [Room9 scripts:4](../../game/data/maps/SeafloorCavern_Room9/scripts.inc#L4), battle around 29; continuation stages Route 128/Sootopolis and crisis/weather state. | G. Preserve the local finale and both planned branch prerequisites. Alternate access/Dive or suppressing the opponent must not activate the crisis. |

Mt. Pyre's orb/Emblem scene and Slateport Harbor's submarine theft are **not trainer
battles**. Preserve their authored causal progression rather than hiding every
villain. The [Harbor scene:64](../../game/data/maps/SlateportCity_Harbor/scripts.inc#L64)
removes hideout entrance guards after the theft. That local eligibility change
is distinct from the public ferry and open-ocean travel. Sootopolis villain
farewell dialogue is likewise not a new battle to suppress.

### Special conflict: Route 120 Steven and Kecleon

[Route120 scripts:183](../../game/data/maps/Route120/scripts.inc#L183) stages a
scripted wild encounter; the loss branch restores the blocker, while non-loss
resolution leads to the Scope handoff and permanent bridge clearance. The
traversal PRD explicitly permits fleeing as a successful local resolution and
requires reward delivery before committing completion.

The trainer-only spec currently excludes scripted wild encounters. Blanket
rival hiding would remove the wrong actor, and blanket trigger suppression would
not deliver the Scope or open the lane. This needs an explicit design decision:
add a compatible exception for this scripted encounter, author another permitted
local resolution, or retain a retryable no-party refusal. The last option would
not promise unprotected passage through this lane. Do not silently grant the Scope
or mark the encounter resolved. This is separate from ordinary trainer bypass.

### Current Hoenn: rivals and adjacent scenes

| Encounter / trigger | State or reward evidence | Policy |
| --- | --- | --- |
| Route 103 starter rival | [scripts:20](../../game/data/maps/Route103/scripts.inc#L20), completion at 154 advances Oldale and rival defeat state. | R. Preserve the starter tutorial's authored rules; do not treat suppression as completion. |
| Rustboro optional rival | [scripts:830](../../game/data/maps/RustboroCity/scripts.inc#L830); optional battle, encounter flags and Match Call staging. | D/R. Refusal already exists, but preserve downstream conversation state independently. |
| Route 110 off-screen rival | [scripts:356](../../game/data/maps/Route110/scripts.inc#L356); three triggers, starter-dependent battles, Dowsing Machine at 460, route state 1. | R/T. Keep chapter/reward pending and road open. No reward on skipped battle. |
| Route 119 off-screen rival | [scripts:34](../../game/data/maps/Route119/scripts.inc#L34); two triggers, battles, Fly at 135, route state 1. | R/T. Separate chapter/reward from public bridge access and Institute rescue. |
| Lilycove optional rival | [scripts:209](../../game/data/maps/LilycoveCity/scripts.inc#L209); decline flags, battle, decoration and departure/meeting state. | D. Suppression must not execute either victory or authored decline writers automatically. |
| Mauville Wally | [scripts:86](../../game/data/maps/MauvilleCity/scripts.inc#L86); Yes/No encounter, victory moves Wally/uncle and sets defeat/call flags. | D. Preserve refusal and pending encounter without awarding victory. |
| Victory Road Wally | [scripts:20](../../game/data/maps/VictoryRoad_1F/scripts.inc#L20); entrance triggers and defeat flag. Later rematch around 83. | R for the first chapter; D for optional rematches. Reconcile with League entry rules separately. |
| Lavaridge rival gives Go-Goggles | [scripts:64](../../game/data/maps/LavaridgeTown/scripts.inc#L64); gift and town-state/bag-full branches. | R/U. Non-battle reward scene: do not hide solely because the actor is a rival. Desert access follows its own planned survey design. |
| Petalburg Wally catching tutorial | [scripts:32](../../game/data/maps/PetalburgCity/scripts.inc#L32); scripted tutorial temporarily substitutes a party. | U. Authored tutorial, not an ordinary rival battle or trainer-only wild encounter. |

### Additional current local gates

| Encounter | Evidence | Policy |
| --- | --- | --- |
| Ecruteak Theater Rocket rescue | [scripts:129](../../game/data/maps/EcruteakCity_Theater_hns/scripts.inc#L129); victory removes Rocket and advances theater state; Surf handoff at 256 depends on rescue. | G. Preserve rescue and earned reward. |
| Five Kimono Girls trial | [scripts:700](../../game/data/maps/EcruteakCity_Theater_hns/scripts.inc#L700); five battles followed by Bell reward branches. | G. Retain the planned Wing/TR eligibility and local trial victory; no bypass into legendary completion. |
| Sprout Tower Elder Li | [scripts:103](../../game/data/maps/SproutTower_3F_hns/scripts.inc#L103); battle followed by Flash. | G. Local challenge reward stays earned; Silver's earlier non-battle scene has separate policy. |
| Kanto Power Plant stolen part | [Route 24 grunt:50](../../game/data/maps/Route24_hns/scripts.inc#L50); battle reveals the part and advances state. [Manager:34](../../game/data/maps/Route10_PowerPlantBackRoom_hns/scripts.inc#L34) consumes the returned part and advances restoration/Misty staging. | G. Keep investigation/recovery predicates; Route 24 itself stays traversable. |
| Current Viridian Blue | [Gym scripts:4](../../game/data/maps/ViridianCity_Gym_hns/scripts.inc#L4); badge/TM progression. | G/U. Gym objective, not an ordinary trainer bypass. Future Giovanni/Blue ownership remains a port decision. |

No Silver partner battle was found in the audited HNS Dragon's Den scripts. Do
not infer the presence of a vanilla HGSS encounter from its familiar location.

### Future FRLG Kanto source: rival chapters

These are source references for the proposed port, **not a claim that the FRLG
maps are currently playable in Wayfarer**. Preserve chapters at their approved
locations; a temporary host scene disappearing is a recoverability problem to
solve, not permission to relocate or auto-complete the rival chapter.

| Chapter | Source | Policy |
| --- | --- | --- |
| Oak's Lab starter battles | [scripts:333](../../game/data/maps/PalletTown_ProfessorOaksLab_Frlg/scripts.inc#L333), other starter branches at 386/442. | U. Authored starter/tutorial outcome. |
| Route 22 early and late rival | [scripts:61](../../game/data/maps/Route22_Frlg/scripts.inc#L61), later family at 211. | R/T. Defer chapters while preserving road access and narrative order. |
| Cerulean rival | [scripts:15](../../game/data/maps/CeruleanCity_Frlg/scripts.inc#L15), Fame Checker continuation around 68. | R/T. Bill access cannot depend on completing this chapter; reward remains local. |
| S.S. Anne corridor rival | [scripts:4](../../game/data/maps/SSAnne_2F_Corridor_Frlg/scripts.inc#L4). | R/U. Captain access must remain independent; ship departure cannot erase a pending chapter. Anne/Aqua adaptation remains unresolved. |
| Pokémon Tower 2F rival | [scripts:60](../../game/data/maps/PokemonTower_2F_Frlg/scripts.inc#L60). | R. Defer without blocking the Fuji investigation; port geography unresolved. |
| Silph 7F rival | [scripts:71](../../game/data/maps/SilphCo_7F_Frlg/scripts.inc#L71). | R. Rescue/occupation must complete independently while leaving chapter recoverable. |
| Champion rival | [scripts:102](../../game/data/maps/PokemonLeague_ChampionsRoom_Frlg/scripts.inc#L102). | G/U. League finale, excluded from generic rival deferral. Follow authored League entry/loss policy. |
| Four Island rival appearance | [scripts:12](../../game/data/maps/FourIsland_Frlg/scripts.inc#L12). | R/U. Existing scene arbitration is a useful deferral pattern, but is not a no-party implementation or authorization to move chapters. |

### Future FRLG Kanto and Sevii source: protected local objectives

| Encounter / objective | Source and effect | Policy |
| --- | --- | --- |
| Cerulean burglary Rocket | [scripts:169](../../game/data/maps/CeruleanCity_Frlg/scripts.inc#L169); local TM recovery. | G. Keep stolen-item reward earned, not a campaign gate. |
| Nugget Bridge recruiter | [scripts:25](../../game/data/maps/Route24_Frlg/scripts.inc#L25); reward/recruitment sequence. | G/T. Preserve authored reward order, without making the encounter a regional travel lock. |
| Mt. Moon fossil Super Nerd | [scripts:23](../../game/data/maps/MtMoon_B2F_Frlg/scripts.inc#L23); victory advances fossil choice scene. | G. Fossil access remains earned. |
| Celadon Hideout Lift Key grunt, door guards and Giovanni | [B4F scripts:18](../../game/data/maps/RocketHideout_B4F_Frlg/scripts.inc#L18); boss/Scope cleanup, key drop around 49, paired door guards around 72. | G. Preserve actual key, both guard victories where required, and Scope reward. |
| Silph Card Key | [item script:249](../../game/data/scripts/item_ball_scripts_frlg.inc#L249), [doors:509](../../game/data/scripts/silphco_doors.inc#L509). | G. This is an item pickup, not a trainer reward. Preserve key access; do not manufacture it when suppressing a battle. |
| Silph Giovanni | [11F scripts:46](../../game/data/maps/SilphCo_11F_Frlg/scripts.inc#L46); occupation cleanup precedes President reward availability. | G. Rival deferral cannot substitute for boss victory. |
| Viridian Giovanni | [scripts:4](../../game/data/maps/ViridianCity_Gym_Frlg/scripts.inc#L4); badge and Rocket finale state. | G/U. Preserve proposed investigation/Silph dependencies; reconcile Blue identity before porting. |
| Three Island biker gang | [scripts:211](../../game/data/maps/ThreeIsland_Frlg/scripts.inc#L211); sequence ends with gang removal/town state 4. | G. Do not clear the harassment/rescue objective through no-party bypass. |
| Icefall Lorelei/poacher rescue | [scripts:46](../../game/data/maps/FourIsland_IcefallCave_Back_Frlg/scripts.inc#L46). | G/T. Rescue remains local; its completion cannot gate Dotted Hole or public travel. |
| Mt. Ember Rocket pair/password | [scripts:24](../../game/data/maps/MtEmber_Exterior_Frlg/scripts.inc#L24); password scene around 84. | G/T. Preserve local cave/password progression, not island travel locks. |
| Warehouse password entrance | [Meadow scripts:66](../../game/data/maps/FiveIsland_Meadow_Frlg/scripts.inc#L66); requires both learned passwords. | G. Neither bypass nor trainer hiding teaches passwords. Preserve the independent Ruby and Dotted Hole branches. |
| Warehouse admins and Gideon | [scripts:46](../../game/data/maps/FiveIsland_RocketWarehouse_Frlg/scripts.inc#L46); Sapphire at Gideon, admin cage/arrow changes around 75/107. | G. Preserve security changes and actual Sapphire recovery. |
| Lost Cave Selphy rescue | [Room10 scripts:6](../../game/data/maps/FiveIsland_LostCave_Room10_Frlg/scripts.inc#L6); battle at 18, then hide cave Selphy/reveal resort Selphy and set scene states. | G. Refuse before the on-frame scene mutates; leave her rescue retryable. Her later home request loop is a different episode. |

Lostelle's rescue and Tower Marowak are scripted wild objectives, outside this
trainer inventory. Their local completion predicates still need preservation;
ordinary trainer bypass must not complete them indirectly. The S.S. Aqua maiden
rescue likewise remains meaningful, but its sailor fight is explicitly optional
under the traversal PRD. Do not add that sailor to the mandatory-guard list.

## Recommended implementation boundary

Use an explicit policy per encounter script or event, not a rule for all rivals,
all Rockets or all trainer sprites. Ordinary trainers can ignore an unprotected
player. Battle-only rivals can defer where their chapter remains recoverable.
Objective guards remain present and refuse battle without a usable Pokémon;
protect the objective with state checks as well as collision, so another entrance
cannot accidentally bypass it.

Check eligibility before spawning, locking movement, transforming an actor or
starting any irreversible scene animation. Temporarily hiding a rival must not
write their canonical completion/hide flags. Restore eligibility when the party
recovers, subject to the chapter's own prerequisites. Non-battle interactions need
their own policy.

Returning to the overworld after losing introduces a second hazard: existing
scripts often assume code after `trainerbattle` means victory. Every protected
scene needs an explicit success-only continuation or safe abort/retry path before
ordinary loss can return there. Refusal and defeat must never grant keys, remove
guards, advance a chapter or resolve an occupation.

Prioritize shared-state rivals (Goldenrod Underground, Burned Tower, Azalea),
temporary host scenes (Silph and the ship), then boss/key/password gates. Keep
public roads open as the upcoming regional designs require. This audit recommends
these boundaries; it does not settle the unresolved scripted-wild, League or
future map-port policies.

## Validation boundaries

This is a script/state audit. Object and coordinate events establish how a scene
starts, but do not alone prove every collision/elevation route. Any proposed
physical objective boundary needs a map/emulator walkthrough from both sides.
For deferred scenes, test arrival before and after prerequisites, host-adventure
completion, save/reload, loss, revival on a trigger tile and regional travel.
No skipped scene may award victory, consume a chapter or become unrecoverable.

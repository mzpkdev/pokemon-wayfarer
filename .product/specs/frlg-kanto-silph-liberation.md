# Silph Co. liberation on Wayfarer

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)  
Dependencies: [FRLG Kanto independent story beats](../prds/frlg-kanto-independent-story-beats.md), [trainer party scaling](trainer-party-scaling.md), [trainer-only story encounters](trainer-only-story-encounters.md), [Wayfarer runtime foundation](wayfarer-runtime-foundation.md), [compressed map-layout runtime loading](compressed-map-layout-runtime-loading.md), [Wayfarer Hoenn entry](wayfarer-hoenn-entry.md), [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)  
Implemented: No

## Scope

Implement an independent Silph Co. liberation adventure: keep the existing HNS
lobby, add the FRLG floors 2 through 11 and elevator cabin, preserve the Card
Key and warp-panel puzzle, rescue the staff, defeat Giovanni, and restore the
ordinary HNS lobby after liberation. This is a multi-floor adventure, not a
single final-boss room.

The milestone preserves 30 ordinary occupation Trainers and one objective
Giovanni battle. They are 31 non-Blue battle records total, not 31 ordinary
Trainers plus a second Giovanni. It also preserves all 17 visible item balls,
12 hidden items, the Card Key, the Thunder Wave tutor, floor utilities, the
source Lapras gift, doors, staff dialogue, and the post-liberation HNS
reception and Up-Grade service.

The player may begin Silph without Bill, the S.S. Anne, Tower, Fuji, Scope,
Flute, Celadon completion, badge, League clear, Kanto origin, or any regional
campaign result. Liberating Silph completes only this local investigation. It
does not award a badge, complete Celadon, Tower, Snorlax, Viridian Gym, the
Giovanni finale, Celebi, or any other story.

Import MAP_SILPH_CO_2F through MAP_SILPH_CO_11F and MAP_SILPH_CO_ELEVATOR
under Wayfarer filtering. Keep MAP_SAFFRON_CITY_SILPH_CO_HNS as the lobby and
do not import FRLG 1F. The FRLG layouts, interior connections, warp panels,
door topology, map music, and authored map identities remain the source
baseline unless this specification names an HNS destination or isolated
Wayfarer state.

This is Wayfarer only. Standalone HNS retains its current lobby; standalone
FRLG retains its current 1F, maps, flags, items, scripts, and Trainer records.
No static terrain, collision, elevation, layout-version, or map.bin edit is
authorized. Porymap is not needed for the event and script changes in this
milestone.

## Boundaries left for later work

The Master Ball is explicitly deferred. Liberation records a future reward
handoff but grants no Master Ball, chooses no Trainer Rating threshold, and
installs no permanent gate. A later reward specification must define the
threshold, President interaction, item receipt, full-Bag retry, and the
handoff's consumption. The future reward cannot gate liberation, recovered
lobby services, or the Giovanni finale.

The current HNS Steven starter interaction is not a settled Wayfarer reward
policy. During occupation Steven gives only an appropriate occupied-building
line. After liberation he gives an acknowledgement that his future reward
remains pending. Neither line opens the existing starter choice, grants a
Pokemon, changes a receipt, or writes FLAG_GOT_HOENN_STARTER. The existing
HNS gift path remains standalone-only. A later specification owns any
cross-origin or Steven reward decision.

Current Johto and Hoenn visitors have no personal Blue rivalry. Exclude the
7F Blue object and all three TRAINER_RIVAL_SILPH source branches. Do not read
a starter variable, add a visitor substitute, invent a rival win, or use Blue
to gate the Card Key, panels, Lapras, staff rescue, traversal, or liberation.

Mark the source 7F coordinate events at (2,4) and (2,5) as
wayfarer_exclude: true, alongside the Blue object. Exclude the rival entry and
branch labels from reachable Wayfarer script flow: neither coordinate may lock
the player, start a scene, write story state, or make the source 7F scene
variable reachable for a local alias. Retain the 7F map OnLoad callback and
all unrelated 7F local scripts. The Wayfarer 11F callback continues to use
only VAR_SILPH_GIOVANNI_SCENE_HNS.

Do not change Goldenrod's Card Key, radio-door state, shared item policy,
global Trainer Rating formula, League accounting, exterior Saffron state, the
Kanto opening, or a future Blue chapter.

## Maps, access, and graphics

The HNS lobby is 36 by 22. Its entrance at (8,20) remains unchanged. Existing
Officer (31,2), Steven (17,14), and receptionist (5,6) remain in place. Use
event interactions rather than static lobby warps:

| Player action | Required result |
| --- | --- |
| Talk to the Officer from (30,2) facing right, or (31,3) facing up | Explain the occupation and offer upstairs access. After liberation, offer that access separately from ordinary service. Script-warp to 2F (30,3). |
| Take 2F return warp 3 at (30,2) | Arrive at HNS lobby (31,3). This is arrival-only and never immediately returns to 2F. |
| Face blocked elevator art (22,3) north from passable (22,4) | An elevation-3 background interaction enters the FRLG elevator cabin at (2,4). |
| Select elevator floor 1 | Return to HNS lobby (22,4), not FRLG 1F. |
| Any other interior stair, elevator, or panel destination | Preserve the authored source destination and puzzle relationship. |

The approach cells (22,4), (31,3), and (30,2) are passable at elevation 3.
The art at (22,3) is collision-blocked, while (31,2) is occupied by the
Officer. A static warp on either cannot satisfy this contract. The access
events must not produce an automatic lobby-to-2F return loop. Existing HNS
signs at (30,1) and (24,3), including their approaches, remain intact.

The imported dimensions are 36 by 22 on 2F through 5F, 31 by 19 on 6F through
9F, 17 by 19 on 10F, 17 by 20 on 11F, and 5 by 7 in the elevator. Register
headers, events, map groups, layouts, layout tables, and compressed-layout
runtime entries only under the established Wayfarer include mechanism. Do not
change the HNS lobby's layout or map ID to fit a source 1F warp.

The source elevator must use VAR_SILPH_ELEVATOR_FLOOR_HNS at 0x40DA on imported
Silph paths, including the special that records the current floor and initializes
the selection cursor. It must not use source VAR_ELEVATOR_FLOOR at 0x403A,
an unrelated HNS variable, or Celadon's dedicated elevator variable. Existing
non-Silph elevator paths remain unchanged.

Use narrow Wayfarer graphics overrides in map data, retaining source graphics
and event IDs in standalone FRLG. Reuse existing HNS assets without importing
pointers or graphics:

| Source graphic | Wayfarer graphic |
| --- | --- |
| OBJ_EVENT_GFX_SCIENTIST | OBJ_EVENT_GFX_SCIENTIST_M_HNS |
| OBJ_EVENT_GFX_WORKER_F | OBJ_EVENT_GFX_WORKER_F_HNS |
| OBJ_EVENT_GFX_ROCKER | OBJ_EVENT_GFX_ROCKER_HNS |
| OBJ_EVENT_GFX_OLD_MAN_2 | OBJ_EVENT_GFX_OLD_MAN_2_HNS |

Existing linked Rocket, Giovanni, Worker M, Woman 2 FRLG, and item-ball
graphics may remain. Blue is excluded, not remapped.

## Local state and recovery

| State | Liberation flag | Result |
| --- | --- | --- |
| Occupied | FLAG_SILPH_LIBERATED_HNS clear | Occupation battles, Card Key puzzle, panels, pickups, staff, and Lapras are available. Lobby uses occupation dialogue but preserves entry access. |
| Giovanni retry | Clear | A no-party refusal or non-win leaves Giovanni and all occupation state retryable. |
| Liberated | Set | Occupation combat actors are removed; staff and lobby use recovery dialogue; ordinary lobby service is available; the future Master Ball handoff is present. |

Each Wayfarer occupation combat object on floors 2 through 11 uses
FLAG_SILPH_LIBERATED_HNS as its hide state. It hides all occupation Rockets and
Scientists at once after a Giovanni win, including on the current floor and
after later map loads. It must not set individual Trainer defeat bits, award
skipped money or experience, consume pickups, or suppress Lapras. Noncombat
staff remain and use recovered dialogue.

VAR_SILPH_GIOVANNI_SCENE_HNS at 0x40DB replaces the source 11F approach and
post-battle scene variable only for Wayfarer. It may track occupied and
liberated callbacks but does not replace any unrelated HNS state. Do not
allocate the source 7F rival state because the whole Blue branch is excluded.
Do not use source VAR_MAP_SCENE_SILPH_CO_7F at 0x405C or source
VAR_MAP_SCENE_SILPH_CO_11F at 0x4060; both alias unrelated HNS story state.

The Wayfarer source aliases for Silph occupation, door, pickup, Card Key,
Lapras, and tutor scripts may be rebound to the unique local constants below,
so existing source routines and their generated engine constants continue to
work. Their Wayfarer values must be the local values, never the original
FRLG or HNS zero alias. Liberation must never write global
FLAG_HIDE_SAFFRON_ROCKETS, Saffron civilian flags, or any unrelated HNS
Saffron flag. This state uses ordinary saved flag and variable storage with no
new save block or migration.

## Card Key, doors, and normal pickups

Define ITEM_SILPH_CARD_KEY at Wayfarer item ID 902, after existing ITEM_HM09 at
901, and use ITEMS_COUNT 903 for Wayfarer. Keep all existing IDs and standalone
item counts unchanged. The item uses the current Card Key icon and appropriate
Key Items behavior, but it is distinct from ITEM_CARD_KEY at 750.

Silph must never grant, inspect, consume, or satisfy ITEM_CARD_KEY. That shared
item belongs to Goldenrod's Director and radio-door story. The 5F ball gives
exactly one ITEM_SILPH_CARD_KEY. It remains visible when the relevant item add
fails and hides only after a successful add. The Silph doors inspect the local
receipt flag, never the shared item. A Wayfarer-local source-name alias may
represent that receipt for the retained door routine.

Preserve all 20 source door groups and their map-load closed-state callbacks.
A first Card Key interaction opens the selected source door with its authored
animation and sets only its own local door flag. Every opened door survives
map load; every unopened door is reconstructed closed. Keep the full puzzle
topology and panels intact. Do not replace it with generic building access.

Visible item balls use distinct local receipts and hide only after the actual
item delivery succeeds. They preserve source locations, source items, and
actual pocket capacity behavior.

| Floor | Visible item balls |
| --- | --- |
| 3F | Hyper Potion |
| 4F | Max Revive, Escape Rope, Full Heal, TM41 |
| 5F | Protein, TM01, Silph Card Key |
| 6F | HP Up, X Special |
| 7F | Calcium, TM08 |
| 8F | Iron |
| 10F | Carbos, Ultra Ball, Rare Candy |
| 11F | Zinc |

Preserve the following 12 hidden items independently. A hidden item uses its
own local receipt and normal successful-add semantics; it remains retriable
after a full pocket and never duplicates after delivery.

| Floor | Hidden items |
| --- | --- |
| 2F | Ultra Ball |
| 3F | Protein |
| 4F | Iron |
| 5F | Elixir, PP Up |
| 6F | Carbos |
| 7F | Zinc |
| 8F | Nugget |
| 9F | Max Potion, Calcium |
| 10F | HP Up |
| 11F | Revive |

The 2F Thunder Wave tutor is an ordinary local service. Include only the
necessary source tutor routine, text, and helpers, or a behavior-identical
Wayfarer wrapper. Do not bulk-link unrelated FRLG tutor content. Preserve its
one-time local receipt with the isolated flag, and set it only after the
selected Pokemon learns Thunder Wave successfully. Declining, cancelling the
party chooser, or an ineligible Pokemon leaves it clear and retryable. This
milestone does not change the global I_REUSABLE_TMS configuration. The
Wayfarer build may rebind source FLAG_TUTOR_THUNDER_WAVE to this local
receipt; it must not retain the HNS zero value.

Allocate the following Wayfarer-only flags, defined as zero in other products.

| Flags | Meaning |
| ---: | --- |
| 0x4C4 | FLAG_SILPH_CARD_KEY_RECEIVED_HNS |
| 0x4C5 through 0x4D4 | Sixteen ordinary visible-item receipts |
| 0x4D5 through 0x4E0 | Twelve hidden-item receipts |
| 0x4E1 through 0x4F4 | Twenty persistent door-open states |
| 0x4F5 | FLAG_SILPH_LIBERATED_HNS |
| 0x4F6 | FLAG_GOT_SILPH_LAPRAS_HNS |
| 0x4F7 | FLAG_SILPH_THUNDER_WAVE_TUTOR_HNS |
| 0x4F8 | FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS |

Rebind retained source identifiers such as FLAG_SILPH_*_DOOR, source item
flags, FLAG_GOT_LAPRAS_FROM_SILPH, and
FLAG_GOT_MASTER_BALL_FROM_SILPH to their named Wayfarer-local constants when
an included source routine requires one. Regenerate the product-specific
engine constants after that binding. Do not let any included routine retain a
zero-valued source alias or a shared HNS/FRLG numeric value.

## Trainers and Giovanni

Append 31 Wayfarer Trainer records after Nugget Bridge Rocket runtime ID 1555.
Their compact slots are 702 through 732 and runtime IDs are 1556 through 1586.
Preserve exact source class, portrait, gender metadata, battle music, battle
type, AI, zero-IV party, moves, and held items. Assign the records in the
following explicit sequence.

| Runtime IDs | Source records | Scaling classification and encounter profile |
| ---: | --- | --- |
| 1556 through 1574 | TRAINER_TEAM_ROCKET_GRUNT_23 through _41 | ORDINARY; hostile faction fallback through the ordinary resolver |
| 1575 through 1584 | Scientists Beau, Connor, Ed, Jerry, Jose, Joshua, Parker, Rodney, Taylor, and Travis | ORDINARY; stable NP_ORDINARY encounter profile |
| 1585 | TRAINER_JUGGLER_DALTON | ORDINARY; stable NP_ORDINARY encounter profile |
| 1586 | TRAINER_BOSS_GIOVANNI_2 | Objective-boss exclusion; NP_ROCKET_GUARD |

Set TRAINERS_COUNT_SILPH_LIBERATION_WAYFARER to 31 and extend
TRAINERS_COUNT_WAYFARER to 1587. Add range assertions proving that 1556
immediately follows 1555 and that Giovanni 1586 plus one equals the total.
The ordinary records use existing appended-HNS defeat storage and ordinary
Trainer Rating scaling. Giovanni uses the existing objective-boss exclusion.
No new Trainer enters the Hoenn fixed runtime bank, and shared scaling changes
are out of scope.

Every ordinary encounter and Giovanni checks the canonical usable-party
predicate before sight resolution, scene movement, dialogue, or battle staging.
The stepped 11F coordinate triggers begin with `lockall`, as required to
synchronize the engine's coordinate-script control handoff, then immediately
perform this predicate before any scene action. A refusal releases that lock.
An empty party, fainted-only party, Eggs-only party, and a party containing
only fainted Pokemon and Eggs receive the ordinary retryable no-party
response. A mixed party with a healthy non-Egg Pokemon can battle. A refusal
does not set a defeat bit, change a door or panel, hide an object, grant an
item, or alter story state.

Every non-win leaves its battle retryable. Giovanni must explicitly inspect
GetBattleOutcome. Only B_OUTCOME_WON may set local liberation state. A loss,
forfeit, escape, reset, or no-party refusal leaves the local flag and approach
state clear. His first and repeat dialogue must be neutral when the player has
not defeated Celadon's Giovanni; Silph-first dialogue may not claim a prior
meeting or loss.

Only a Giovanni win commits local liberation and sets
FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS. It then updates current map
visibility and recovered dialogue and releases control. No other interaction
may set, clear, or inspect that pending marker. Do not give a Master Ball,
Earth Badge, League reward, Champion result, Celebi flag, or future
Giovanni-finale result.

## Staff, Lapras, and the restored lobby

Preserve staff, floor utility, and recovery dialogue from the source building.
Occupied and recovered variants must make sense whether Silph or Celadon is
completed first. Staff recovery remains local to Silph and does not modify
unrelated Saffron scenes.

The 9F healer remains available during occupation and after liberation. The
recovered interaction uses recovery dialogue and retains the healing offer.

The 7F Lapras employee remains reachable through the authored Card Key and
warp-panel route without Blue. Give the authored Level 25 Lapras without a
Trainer Rating, badge, or Giovanni-defeat requirement. The normal gift delivery
may place it in party or PC. If both are full, leave the employee and local
receipt clear for retry. Set FLAG_GOT_SILPH_LAPRAS_HNS only after a successful
delivery and preserve the nickname flow. This is an ordinary one-time local
gift, not a legendary threshold or reward for a removed rival battle.

Before liberation, the Officer and receptionist use occupation dialogue while
the Officer still provides the separate 2F transition. After liberation, they
resume ordinary lobby interactions. The Officer's Up-Grade handoff must use an
actual item transaction:

1. A set FLAG_GOT_UP_GRADE gives existing post-receipt dialogue.
2. With a clear receipt, check capacity and add exactly one ITEM_UP_GRADE,
   including when an existing Up-Grade stack can accept another copy.
3. Set FLAG_GOT_UP_GRADE only after that add succeeds.
4. A failed add leaves the receipt clear and the Officer retryable. It never
   removes access to 2F.

Steven gives acknowledgement only, as defined in the reward boundary. The
President has an explicit Wayfarer handler in every local state: an
occupied-building hostile line while FLAG_SILPH_LIBERATED_HNS is clear, and a
liberated acknowledgement derived only from that flag after a win. It gives no
Master Ball, writes or reads no receipt or pending marker, and fabricates no
Trainer Rating condition. Bypass the original Master Ball body under
Wayfarer filtering even if an included source entry point retains its name.
The later reward specification owns every pending-marker read and the
recoverable delivery transaction.

## Validation and release budget

Static and generator coverage must prove Wayfarer-only inclusion of 2F through
11F and the elevator; unchanged standalone HNS and FRLG preprocessing; unchanged
source map.bin hashes; correct layout versions and compressed-layout registration;
all source panel connections; the 2F lobby return; floor-one elevator return;
and no automatic lobby loop. Verify the HNS lobby geometry and elevations at
(22,4), (30,2), and (31,3), and prove that no static warp occupies blocked
(22,3) or Officer cell (31,2).

Audit each imported map for event capacity, local IDs, visible objects,
connections, and graphics overrides. Verify 20 doors and their closed/open
persistence, 17 visible balls, 12 hidden items, all isolated receipts, item
ID 902, unchanged Goldenrod Card Key behavior, and full-pocket retries.
Verify the tutor's success-only receipt and that no unnecessary FRLG tutor
linkage is introduced.

Native coverage must prove Trainer IDs 1556 through 1586, total 1587,
source-party parity, metadata, appended defeat storage, ordinary scaling, and
Giovanni's exclusion. Exercise each no-party shape, a usable party, ordinary
and Giovanni wins, every non-win retry, save/reload, and liberation only after
Giovanni wins. Prove liberation removes occupation objects without setting
unfought Trainer defeats and leaves HNS Saffron, Celadon, Goldenrod, Blue,
League, and finale state untouched.

Static and runtime checks must prove that 7F (2,4) and (2,5) have no
Wayfarer coordinate event. Walk through both cells before and after liberation
and assert that neither invokes a script, locks control, writes state, or
starts an encounter; retain the unrelated 7F OnLoad and local behavior.

Exercise Card Key and each door through map loads; all visible and hidden item
transactions; Lapras party, PC, and full-party-plus-PC retry; Thunder Wave
decline/cancel/success retry behavior; and Up-Grade's existing-item
pre-owned-plus-one transaction with a clear receipt, full-pocket retry,
successful receipt, and continued 2F access. Confirm Steven never starts or
records a Wayfarer gift and that the
Master Ball marker grants nothing and gates no current service or finale.

Emulator coverage must enter through the HNS entrance, walk to (22,4), (31,3),
and (30,2), use both access interactions, return from 2F and floor 1 without
a loop, visit every imported floor, inspect occupied and liberated states,
solve door and panel progression, receive Lapras without Blue, lose and retry
Giovanni, observe current-floor occupation removal, use restored lobby
services, and save/reload. Obtain native critic review before accepting the
implementation.

Measure incremental release-ROM size and remaining physical free space. Preserve
the mandatory 512 KiB production reserve check. If the reserve is exceeded,
record the measured cost under the user's development authorization; do not
silently cut this specified content or begin unrelated optimization.

## References

- [FRLG Kanto story conflicts](../research/frlg-hns-kanto-story-conflicts.md)
- [FRLG Kanto implementation sequence](../research/frlg-kanto-implementation-sequence.md)
- [Celadon Rocket Hideout on Wayfarer](frlg-kanto-celadon-hideout.md)
- [Nugget Bridge on Wayfarer](frlg-kanto-nugget-bridge.md)
- [HNS Silph lobby](../../game/data/maps/SaffronCity_SilphCo_hns/map.json)
- [FRLG Silph 2F](../../game/data/maps/SilphCo_2F_Frlg/map.json)
- [FRLG Silph 7F](../../game/data/maps/SilphCo_7F_Frlg/map.json)
- [FRLG Silph 11F](../../game/data/maps/SilphCo_11F_Frlg/map.json)
- [FRLG elevator](../../game/data/maps/SilphCo_Elevator_Frlg/map.json)
- [Source Silph door scripts](../../game/data/scripts/silphco_doors.inc)

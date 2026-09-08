# Persistent S.S. Anne on Wayfarer

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)  
Supporting requirements: [Wayfarer Hoenn entry and S.S. Aqua circuit](wayfarer-hoenn-entry.md), Vermilion harbor menu audit; [FRLG Kanto independent story beats](../prds/frlg-kanto-independent-story-beats.md), independently playable local adventures  
Implemented: Yes

## Scope

Wayfarer imports the FRLG S.S. Anne as a permanent, self-contained Vermilion
adventure. It preserves the ship's ordinary trainers, items, Captain, and its
interior traversal. It does not import the exterior map, create an Anne route,
change the Aqua route, or alter any dock tiles, collision, or map binary.

This is an import of exactly these 25 interiors:

| Area | Maps |
| --- | --- |
| First floor | `SSAnne_1F_Corridor_Frlg`, `SSAnne_1F_Room1_Frlg` through `SSAnne_1F_Room7_Frlg` |
| Second floor | `SSAnne_2F_Corridor_Frlg`, `SSAnne_2F_Room1_Frlg` through `SSAnne_2F_Room6_Frlg` |
| Other guest areas | `SSAnne_3F_Corridor_Frlg`, `SSAnne_B1F_Corridor_Frlg`, `SSAnne_B1F_Room1_Frlg` through `SSAnne_B1F_Room5_Frlg` |
| Service and deck | `SSAnne_CaptainsOffice_Frlg`, `SSAnne_Deck_Frlg`, `SSAnne_Kitchen_Frlg` |

`SSAnne_Exterior_Frlg`, including its Lava Cookie, is excluded. The existing
`gMapGroup_Dungeons_Frlg` source group keeps every slot, including `NULL`
entries for excluded maps, but only the listed maps and their nine layouts opt
in to Wayfarer generation. The opt-in must not select unrelated FRLG maps or
change the generated IDs of existing HNS and Emerald maps. The relevant fixed
Wayfarer IDs are group 66, with 1F at map 5, 2F at map 6, B1F at map 8, and the
Captain at map 11.

The map generator accepts a Wayfarer-only `wayfarer_include: true` marker on
both the selected map records and the layouts they use. HNS and Emerald remain
selected by their existing source-version rules. Wayfarer includes the selected
Anne event scripts explicitly; FRLG continues to include its complete original
Anne script set under its existing build guard.

## Behavior

### Vermilion boarding and return

Wayfarer appends **Board S.S. Anne** at slot 6 of the existing Vermilion sailor
menu. Slots 0 through 5 retain their meanings and their per-service
eligibility:

| Slot | Service | Required Wayfarer eligibility |
| --- | --- | --- |
| 0 | Slateport | Existing regular Aqua eligibility, S.S. Ticket, and existing Hoenn-entry preparation |
| 1 | Southern Island | Existing regular Aqua eligibility and `ITEM_EON_TICKET` |
| 2 | Birth Island | Existing regular Aqua eligibility and `ITEM_AURORA_TICKET` |
| 3 | Faraway Island | Existing regular Aqua eligibility and `ITEM_OLD_SEA_MAP` |
| 4 | Battle Frontier | Existing regular Aqua eligibility only |
| 5 | Exit | No travel state change |
| 6 | Board S.S. Anne | Possession of `ITEM_SS_TICKET` only |

The Wayfarer sailor reaches the menu before testing regular Aqua eligibility so
a ticket holder can select Anne even when no regular Aqua service is available.
Each old service checks that regular eligibility inside its own branch before
its existing checks and destination behavior. Slot 6 neither calls Aqua-boat
movement nor changes Aqua state, respawn state, Hoenn-entry state, or ticket
ownership. A player without the ticket receives the established ticket refusal
and remains at the dock. Cancelling or declining has no state effect.

Wayfarer dismisses the existing **Where would you like to sail?** field message
before drawing its seven-entry menu so slot 6 is visible. Standalone HNS keeps
the original prompt and menu sequence unchanged.

Boarding fades directly to `MAP_SSANNE_1F_CORRIDOR` at `(19, 2)`. The
source's two exterior door warps at `(19, 1)` and `(20, 0)` remain door warps,
but use `MAP_DYNAMIC` in Wayfarer. The corridor's Wayfarer on-load script sets
their dynamic destination to `MAP_VERMILION_CITY_PORT_INSIDE_HNS` at `(8, 9)`
on every entry. This uses the existing dock and sailor. `(8, 9)` is a
walkable public-dock tile with an unobstructed path through `(8, 8)` to the
dock exit; `(19, 2)` is a walkable corridor tile with a clear route into the
ship. No dock event, exterior berth, dock variant, or `map.bin` changes are
permitted.

Standalone HNS keeps its existing Olivine and optional-travel menu structure
and eligibility behavior byte-for-byte at preprocessing level. FRLG keeps its
original menu and departure behavior.

### Persistent ship content

The imported interior graph keeps all ordinary FRLG inter-room warps, NPCs,
movement, dialogue, trainer battles, and rewards except where the rules below
adapt a source-only state dependency. Leaving the Captain's Office never
triggers the FRLG departure sequence, removes the ship, or closes access to the
ship. The S.S. Ticket remains in the player's bag.

The imported `METATILE_SSAnne_Door` uses the original S.S. Anne two-tile door
animation, including its tile data and palette, in Wayfarer. Those three door
assets are selected only for FRLG and Wayfarer; standalone HNS does not import
them.

### Trainers

All 16 ordinary trainers remain present with their authored FRLG parties and
battle/reward scripts: Tyler, Ann, Arthur, Thomas, Dale, Brooks, Lamar, Dawn,
Barny, Phillip, Huey, Dylan, Leonard, Duncan, Edmond, and Trevor. Their
Wayfarer trainer constants and parties are a separate Wayfarer-only appended
data set, not raw FRLG IDs and not additions to the standalone HNS trainer
table. They use compact HNS defeated-state slots 661 through 676, exposed in
Wayfarer as runtime trainer IDs 1515 through 1530 after the fixed Emerald
range. `TRAINERS_COUNT_HNS` remains 661; `TRAINERS_COUNT_WAYFARER` becomes
1531 through `TRAINERS_COUNT_SS_ANNE_WAYFARER = 16` and the explicit formula
`TRAINERS_COUNT_HNS + TRAINERS_COUNT_EMERALD - 1 +
TRAINERS_COUNT_SS_ANNE_WAYFARER`. Their defeated-state routing uses the
existing appended-HNS handling and cannot overlap HNS, Emerald, system, or
partner trainer flags. Each is entered in the existing trainer-scaling
classification with the same authored-party treatment as its original trainer
role.

The visitor Blue scene is deliberately not one of those trainer battles. It
does not create a rival battle, award a skipped win or reward, set a host-story
completion flag, or rely on a playable Kanto origin. Its three source approach
triggers route to one Wayfarer visitor introduction that marks an Anne-specific
Blue-met flag and retires the scene. The introduction describes Blue as a
visiting trainer, avoids childhood-rival claims, and never blocks the Captain
or the rest of the ship. The original FRLG rival state machine remains in the
FRLG branch for future Kanto-origin work.

#### Items

The seven visible item balls and four included hidden items remain one-time
rewards with distinct nonzero Wayfarer flags:

| Flag range | Rewards |
| --- | --- |
| `0x496` through `0x49C` | TM31, Stardust, X Attack, TM44, Ether, Super Potion, Great Ball |
| `0x49E` through `0x4A1` | B1F Hyper Potion; Kitchen Chesto, Pecha, and Cheri Berries |

The source flag names resolve to these values only in the Wayfarer branch of
the HNS header; standalone HNS and FRLG definitions remain unchanged. The
omitted exterior Lava Cookie has no imported reward flag.

#### Captain and Cut

The Captain gives Cut through a retry-safe transaction. The captain completion
flag is `FLAG_SS_ANNE_CAPTAIN_REWARD_HNS` (`0x49D`). If Cut is already owned,
the script records completion without granting a duplicate. Otherwise it checks
bag space, grants Cut, confirms the item is owned, and only then sets the
completion flag. A full bag leaves the flag clear and the Captain available for
retry. The post-reward dialogue treats the ship as still in port and never
starts the source departure sequence. The source FRLG `FLAG_GOT_HM01`, map
scene, and departure state are not used by Wayfarer.

## State allocation and save behavior

The eleven item flags, Captain completion flag, and Anne visitor state use the
existing unused HNS flag/variable capacity. No save block changes or migration
are needed. The source Anne rival-hide label maps to `0x4A2` only in Wayfarer,
so the visitor is hidden until its scene is activated. The separate
`FLAG_SS_ANNE_BLUE_MET_HNS` at `0x4A3` records completion. The source Anne
corridor scene label maps to `VAR_UNUSED_HNS_0x40D8` only in Wayfarer, renamed
`VAR_MAP_SCENE_S_S_ANNE_2F_CORRIDOR`; it does not alias a Johto map-state
variable. Standalone HNS keeps every source flag alias at its prior value.

The selected interiors use `MAPSEC_S_S_ANNE`. Its source record has the
Wayfarer-only `wayfarer_hns` marker, which appends the section after the current
HNS range: Wayfarer assigns it `125` and moves its `MAPSEC_NONE` sentinel to
`126`. Existing concrete HNS map-section IDs, including Vermilion (`25`) and
Whirl Islands (`124`), retain their values. Standalone HNS continues to assign
Anne `0` and keeps `MAPSEC_NONE` at `125`. The Wayfarer region-map entry names
the interiors **S.S. ANNE** at `(14, 9)`. `GetRegionForSectionId` classifies
the appended Wayfarer section as Kanto, despite its appended numeric position.
Before Kanto is visited, its generated HNS region-map entry uses `(14, 9)`.
Afterward, the combined Johto/Kanto region-map table places its named entry at
Vermilion's `(24, 7)` harbor coordinate; neither table can expose a zero-sized
or unnamed Anne entry.

## Validation

Implementation must provide static coverage that:

1. proves only the 25 named maps and their layouts are newly selected for
   Wayfarer, and existing HNS and Emerald map constants remain stable;
2. verifies all 16 Wayfarer Anne trainers have unique IDs, parties, and valid
   defeated-state routing while standalone HNS counts and tables are unchanged;
3. verifies all eleven included item flags and the Captain flag are distinct,
   nonzero, and within the existing saved-flag range;
4. checks the dynamic corridor return target and both documented safe
   coordinates without changing a map binary; and
5. audits the Wayfarer sailor menu's old slots, slot 6 ticket rule, rejection,
   return, Captain retry, revisiting, and retained ticket in end-to-end coverage.
6. derives every imported Anne object graphic from the selected map records and
   confirms its non-null Wayfarer graphics pointer and graphics-info definition;
   proves the live corridor exit `(19, 1)` decodes to
   `METATILE_SSAnne_Door` (`0x281`) and that its Wayfarer door-animation
   tile/palette/table entry is present while standalone HNS excludes it; and
   confirms the appended Anne map section, its name, its pre- and
   post-Kanto-visit region-map entries, and the retained HNS map-section values
   under both preprocessing modes.

The exact-menu audit in `wayfarer-hoenn-entry.md` must be updated for the
appended slot and its individual eligibility contract. A coordinated Wayfarer
release build is required after generated map files are refreshed; it is not a
license to reduce this content or to begin unrelated ROM optimization.

## Validation evidence

- The eight-case map catalog suite passes for the 25 selected interiors, nine
  selected layouts, stable group slots, both `MAP_DYNAMIC` exits, the `(19, 2)`
  boarding and `(8, 9)` return coordinates, all eleven item flags, the Captain
  flag, all twenty map-derived Anne graphics pointers and definitions, and the
  Wayfarer Anne scene and visitor-state allocations. It verifies that the live
  corridor exit `(19, 1)` is `METATILE_SSAnne_Door` (`0x281`), that only
  Wayfarer receives its S.S. Anne door tile/palette/table assets, and that the
  appended Anne map section is Kanto while standalone HNS retains
  `MAPSEC_S_S_ANNE = 0`. The Anne scene remains `0x40D8`, not Cianwood's
  `0x405B` slot.
- The generated region-map constants, entries, and `GetRegionForSectionId`
  preprocessing are byte-identical to `HEAD` for standalone HNS. Wayfarer
  assigns Anne `125` with its S.S. ANNE map-name entry and leaves all preexisting
  concrete HNS section IDs unchanged.
- The exact sailor-menu audit passes 16 cases, including dismissal of the field
  prompt before Wayfarer's seven-entry menu is drawn.
- Trainer-scaling's 29 Python generator cases pass for all 16 exact authored
  party signatures and their isolated Wayfarer data range. The five native
  `wayfarer_trainers` mechanics tests separately pass the 1515--1530 runtime
  IDs, compact 661--676 HNS defeated-state slots, isolated defeat state, and
  unchanged standalone HNS trainer count.
- The two existing Aqua regression suites pass: `wayfarer-hoenn-entry` and
  `wayfarer-hoenn-origin-travel` preserve the original travel behavior after
  the Anne menu addition.
- The final coordinated default-LTO Wayfarer release build passed. Its ROM
  ends at `0x09F7A774` (33,007,476 bytes), leaving 546,956 bytes unused and
  22,668 bytes above the accepted reserve. This is a 118,628-byte increase
  from the 32,888,848-byte Machine Part milestone baseline.
- All ten isolated standard-E2E Anne cases pass: the five regular-Aqua
  refusal paths, Exit/cancellation, Anne ticket refusal, ticketed boarding and
  save/reload return/reboarding, the normal Captain reward and revisit, visitor
  Blue, prior-owned Cut reconciliation, an ordinary trainer's persisted win,
  full TM/HM-pocket retry, and both Anne region-map tables. The emulator uses
  the standard LTO-off E2E configuration with
  `XDG_DATA_HOME=/tmp/frlg-skyemu-data`.
- Native critic review is clear for catalog selection, trainer routing, runtime
  scripts, the S.S. Anne door animation, and standalone preprocessing parity.

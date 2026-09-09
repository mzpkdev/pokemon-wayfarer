# Fuchsia Safari, Surf, and Gold Teeth on Wayfarer

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)

Dependencies: [FRLG Kanto independent story beats](../prds/frlg-kanto-independent-story-beats.md), [HM field use](hm-field-use.md), [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)

Implemented: Yes

Implemented on `task/frlg-kanto-story-implementation` in [PR #86](https://github.com/mzpkdev/pokemon-wayfarer/pull/86); not yet merged or released. See the [milestone index](frlg-kanto-story-milestones.md) for the bounded delivery scope.

## Scope and integration

Use the existing HNS Fuchsia Safari rather than importing any FRLG Safari map.
The two local objectives are parallel: obtain Surf from a Devon attendant and
find Gold Teeth for the visiting Baoba, who gives Strength. Both are available
through the ordinary Fuchsia Safari admission and are reachable without Surf,
Strength, Koga, a badge, or another story adventure. Existing local admission
is deliberately retained; it is not an unrelated story prerequisite.

Devon continues to operate Fuchsia Safari. Baoba, the former Warden and the
existing Johto Safari research host, is visiting his former Fuchsia home. He
lost his Gold Teeth while inspecting the Safari; when the player finally returns
them, he keeps the Teeth while giving the Strength reward. Dialogue must state
this arrangement, rather than claim that Fuchsia Safari is closed or that Baoba
is unavailable overseas.

This milestone changes only Wayfarer-selected events and Wayfarer dialogue
variants in these existing HNS locations:

| Location | Wayfarer addition or adaptation |
| --- | --- |
| `MAP_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS` | Gold Teeth item ball and the Devon Surf attendant |
| `MAP_FUCHSIA_CITY_HOUSE2_HNS` | Visiting Baoba and the Teeth-to-Strength handoff |
| `MAP_FUCHSIA_CITY_HNS` | Truthful Fuchsia Safari/Warden signs and local dialogue |

Do not add a map, layout, tileset, sprite asset, connection, warp, map binary,
field-move rule, badge check, Koga/Janine content, Steven content, or a new
Safari admission path. Use the existing HNS `OBJ_EVENT_GFX_ATTENDANT_M_HNS`
for the Devon attendant and `OBJ_EVENT_GFX_GENTLEMAN_HNS` for Baoba; neither
requires a new graphics import. Preserve the granddaughter at `(4, 4)` and the
photo interactions at `(3, 1)` and `(4, 1)` in House 2.

The additions must be selected only by Wayfarer. Standalone HNS preserves its
existing Fuchsia maps, event inventory, text, Devon operation, and Safari
entry/exit behavior. FRLG remains governed by its existing source maps and
scripts.

## Accessible local objectives

Both placements are on the existing Beach map, on opposite walkable branches
from the normal Safari arrival area. They are not cosmetic shortcuts beside the
entrance, behind a progression gate, on an existing actor/trigger, or dependent
on an imported terrain change.

| Objective | Exact location and interaction | Placement contract |
| --- | --- | --- |
| Gold Teeth | Beach item ball at `(39, 23)`; interact from `(39, 24)` | Grass terrain, 37 ordinary traversal steps from the Beach entry; its safe route excludes fixed NPC tiles and the `(1, 12)` Safari-progress trigger. |
| Surf | Devon attendant at Beach `(4, 23)`; interact from `(4, 24)` | 34 ordinary traversal steps from the Beach entry; its safe route excludes fixed NPC tiles and the `(1, 12)` Safari-progress trigger. |
| Strength | Baoba at Fuchsia House 2 `(8, 2)`; interact from `(8, 3)` | The house entry at `(4, 8)` has a collision-, elevation-, and behavior-valid path to Baoba that avoids the granddaughter and photographs. |

The audited Beach routes start at the normal entrance arrival tiles `(19, 39)`
through `(21, 39)`. They respect collision, elevation, and directional behavior
and do not take the existing level-3 trigger at `(1, 12)`. The Beach's existing
connections, trigger, NPCs, roaming ranges, and Safari state must not be
changed. The Cave is not used because its upstream route has existing Safari
progress requirements, which would violate the independent objective contract.

## State and transactions

Allocate three unused HNS flags immediately after the Hideout allocation. They
are Wayfarer-local state, with no reuse of zero-valued HNS aliases or HNS/Hoenn
progress flags:

| Flag | Meaning |
| --- | --- |
| `0x4BC` | The Beach Gold Teeth item ball has been claimed and is hidden. |
| `0x4BD` | The player completed the local Surf delivery at the Beach attendant. |
| `0x4BE` | The player completed the local Baoba Strength delivery. |

No new variable or save-block field is needed. In particular, preserve
`VAR_SAFARI_ZONE_STATE` (`0x40A4`),
`VAR_KANTO_SAFARI_ZONE_PROGRESS` (`0x406D`), and
`VAR_BAOBA_QUEST_STATE` (`0x4054`), as well as the existing HNS Safari flags
`FLAG_GOOD_LUCK_SAFARI_ZONE`, `FLAG_IN_KANTO_SAFARI_ZONE`,
`FLAG_SAFARI_ZONE_EAST_EXPANSION`, and
`FLAG_SAFARI_ZONE_WEST_EXPANSION`. Do not use or change
`FLAG_RECEIVED_HM_STRENGTH` (`0x1F8`), which is used by existing Strength
reward scripts, including Olivine's cafe. The Wayfarer additions use their own scripts and do not
remap HNS's zero-valued FRLG aliases such as `FLAG_GOT_HM03`, `FLAG_GOT_HM04`,
or `FLAG_HIDE_SAFARI_ZONE_WEST_GOLD_TEETH`.

### Gold Teeth

The item ball grants `ITEM_GOLD_TEETH` only when its Key Items pocket accepts
the item. It sets `0x4BC` and hides only after that successful grant. If the
player already owns Gold Teeth, interacting with the item ball reconciles the
local object by setting `0x4BC` without a duplicate. A full Key Items pocket
leaves the ball and `0x4BC` unchanged so the player can retry after making
space.

Before the Strength delivery, Baoba requires actual possession of
`ITEM_GOLD_TEETH`; Surf ownership, Beach discovery, a prior Safari session, or
pre-owned Strength alone cannot stand in for the Teeth. This ensures the local
return objective is not skipped by unrelated inventory.

### Surf

The Devon attendant is the sole local Surf giver. On interaction, the script
checks `ITEM_HM03` ownership. If it is already owned, it records `0x4BD` only
at this giver and does not duplicate the HM. Otherwise it checks the TM/HM
pocket, attempts the item grant, confirms success, then sets `0x4BD`. A full
TM/HM pocket leaves `0x4BD` clear and the attendant available for a later
retry. Receipt of Surf never changes Gold Teeth, Baoba's handoff, existing
Safari progress, or a Gym state.

### Strength

Baoba's local delivery remains pending until the player brings Gold Teeth. If
`ITEM_HM04` is already owned, Baoba reconciles that ownership only after the
player presents actual Teeth. Otherwise he checks the TM/HM pocket, grants
Strength, and confirms success. Only after either successful new grant or this
at-giver pre-owned-HM reconciliation does the script remove
`ITEM_GOLD_TEETH`, set `0x4BE`, and use the completed dialogue. It also sets
`0x4BC`, so a previously unclaimed Beach object cannot recreate the returned
Teeth.

If the TM/HM pocket is full, Baoba keeps neither the Teeth nor completion state:
`ITEM_GOLD_TEETH` remains with the player and `0x4BE` remains clear. The player
can free space and retry the same interaction. A completed delivery never
awards a duplicate HM. All successful receipt, reconciliation, object-visibility,
and retry states persist across area changes and save/reload.

The milestone awards HM items only. Their use on surfable water and Strength
obstacles continues to be governed by the shared HM field-use contract; this
story must not reintroduce badge, learned-move, or safari-session field-use
requirements.

## Preserved HNS Safari systems

Keep the Fuchsia admission script's 500-money fee, Pokéblock Case condition,
storage/party-capacity condition, Safari-ball session initialization, and
normal entry destination exactly intact. Keep its normal exit, timeout,
re-entry, and state cleanup behavior intact as well. The new rewards do not
reset, replace, consume, or otherwise alter an active Safari session.

Keep every existing Fuchsia and Johto Safari activity, including the Devon
research explanation, Safari Levels, habitat/expansion progression, Steven's
existing interactions, and Baoba's research and competition quest. In
particular, retain the `VAR_BAOBA_QUEST_STATE` research sequence, its
`baobacheckmon` conditions, prizes, and all normal Safari sessions. This
milestone neither grants nor advances any of those activities.

Adapt only statements that make the required Fuchsia objective unavailable:
Fuchsia's closed-office/Warden-travel claims and the granddaughter's departure
text must instead explain Devon's operation and Baoba's temporary visit. Keep
the speakers, their local context, and non-conflicting dialogue. Do not imply
that Koga, Steven, Janine, a badge, or a later Kanto story was completed.

## Validation and budget

Static coverage must prove all three Wayfarer-only object placements and their
safe approach routes, excluding fixed actor tiles and the `(1, 12)` trigger
and accounting for existing roaming actors. It must prove the House 2 route,
existing-object preservation, all three nonzero state allocations, and no map
binary edit. It must also prove no Wayfarer event selection leaks into standalone
HNS, and compare changed standalone HNS/FRLG preprocessing where relevant.

Emulator coverage must exercise ordinary paid Safari admission and exit, then
complete Surf and Teeth/Strength in either order in separate Safari sessions.
It must prove both Beach rewards are reachable without Surf, Strength, a badge,
Koga, or Safari-level progression; existing research/competition and session
behavior remain available; and each giver supplies only its own HM. Cover full
Key Items and TM/HM pockets, pre-owned Gold Teeth, pre-owned Surf, and
pre-owned Strength. Assert every failed handoff is retryable without an
unearned receipt or lost Teeth, while every reconciled pre-owned result requires
its actual local interaction. Save/reload between pickup, handoff, and retry.

Run focused native/map-generator and static tests plus admission-script parity,
the relevant Wayfarer runtime tests, and the previous Anne and Hideout
regressions after shared generator/runtime changes. Obtain a native critic
review before committing. Build only in the coordinated map-version sequence.

The Hideout baseline uses 33,067,852 bytes, leaving 486,580 bytes physically
free. Its normal production-reserve check already fails by 37,708 bytes under
the user's development waiver. Measure this milestone's actual release delta;
report the normal check honestly, do not cut accepted content, and do not turn
this feature into a ROM-space recovery project.


The pre-rebase release links at 33,069,244 used bytes (`__rom_end =
0x09F898BC`), adding 1,392 bytes over Hideout. It leaves 485,188 physical bytes
free and fails the unchanged reserve check by 39,100 bytes. EWRAM/IWRAM remain
248,557/25,616 bytes. After this milestone's validated commit, rebase onto latest
main as requested by the user and measure the combined recovery and story build;
this pre-rebase measurement does not establish the recovered budget.


All 11 Safari emulator cases and five static tests pass. Static coverage runs
real HNS/Wayfarer map generation, checks state namespaces and preserved events,
and verifies the reward branches and untouched layout binaries. E2E type
checking and lint pass. Final native critic review is clear.

# Sevii story implementation coordination

Status: story implementation complete through reduction follow-up `50b510a95a`; base saved-state ABI published by commit `0357e4a423`; Trainer rematch extension published separately by commit `7664a169ad`; foundation base `fbc3bc33f7a37b89be850013938595155bb6ecc0`.

## Saved bank and version

Story owns `SaveBlock3.wayfarerSevii`, its initializer, validation, and the event flag/variable routing. The aggregate is 208 bytes and raises the Wayfarer `SAVE_VERSION` from 8 to 9. With the foundation's 904-byte `SaveBlock3`, the result is 1112 bytes, below the 1624-byte hard bound.

```c
struct WayfarerSeviiPersistentState {
    u8 flags[32];
    u16 vars[32];
    u8 trainerFlags[67];
    u8 rematchStages[16];
    u8 rematchPending[8];
    u8 magic;
    struct WayfarerSeviiTrainerTowerRecords trainerTower;
};
```

`WayfarerInitPersistentState()` calls `WayfarerSeviiInitPersistentState()`. The initializer zeroes the complete aggregate and writes `WAYFARER_SEVII_STATE_MAGIC`. `WayfarerPersistentStateIsValid()` includes the Sevii validator. This prerelease save-layout change intentionally has no migration.

Flags use namespace `0xC000` with 256 slots; variables use namespace `0xD000` with 32 slots. All ordinary event-data entry points route these namespaces to the new bank.

## Frozen allocation

The following slots are owned and must not be independently reused:

| Kind | Slots | Allocation |
| --- | ---: | --- |
| Flags | 0–10 | Foundation exploration: Dotted Hole 0; Icefall switches 1–9; Tanoby Key 10 |
| Flags | 11–19 | Lostelle/bikers/Hypno/Meteorite and their retry-safe rewards |
| Flags | 20–22 | Lorelei objective and reward |
| Flags | 23–25 | Selphy objective and reward |
| Flags | 26–29 | Tectonix and Egg Move Tutor one-time rewards |
| Flags | 30–40 | Celio, Ruby, Sapphire, passwords, Warehouse, and Network Machine repair |
| Flags | 41–42 | Rival scene and Moltres completion |
| Flags | 43–51 | Heracross/Nugget/Rock Smash rewards and Explosion, Body Slam, Swords Dance, and Cape Brink tutor receipts |
| Flags | 52 | Trainer-owned Wayfarer Vs. Seeker charging lifecycle |
| Flags | 53–60 | Derived inverse actor presentation: returned Selphy/Butler; Dotted Hole scientist; optional local Rockets; Warehouse combatants; Warehouse Gideon; Ruby guards; rival actors; returned Lostelle |
| Variables | 0–2 | Selphy requested species, pending reward, and active request |
| Variables | 3 | `trainer_tower.pending_prize` transaction mirror |
| Variables | 4 | Heracross size record, initialized to `0x8000` |
| Trainer defeat | 0–532 | Trainer-owned fixed allocation capacity; current frozen inventory uses 0–135 |
| Rematch family | 0–63 | Trainer-owned two-bit stages and pending-ready bits |

The exact flag and variable symbols are authoritative in `include/constants/flags.h` and `include/constants/vars.h`. Unallocated ranges begin at flag slot 61 and variable slot 5.

## Story actor presentation

Slots 53–60 are condition-specific inverse flags because several source maps have no source map-script slot that the provenance-checked overlay can select. `WayfarerSeviiInitPersistentState()` initializes all eight hidden. Objective scripts write only the presentation flags affected by their durable transition; no arrival handler hides a still-undefeated actor. During Trainer integration, returned Lostelle moved from slot 52 to slot 60 so Trainer can own slot 52 for Vs. Seeker charging.

| Slot | Hide flag | Actor is visible exactly when |
| ---: | --- | --- |
| 53 | `FLAG_WAYFARER_SEVII_HIDE_RETURNED_SELPHY` | Selphy returned; shared by Selphy and Butler |
| 54 | `FLAG_WAYFARER_SEVII_HIDE_DOTTED_HOLE_SCIENTIST` | Celio gems started and Sapphire not stolen |
| 55 | `FLAG_WAYFARER_SEVII_HIDE_LOCAL_ROCKETS` | Celio gems started and Warehouse not cleared; shared by Meadow and Outcast actors |
| 56 | `FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_COMBATANTS` | Sapphire stolen, both passwords learned, and Warehouse not cleared |
| 57 | `FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_GIDEON` | Sapphire stolen and both passwords learned; intentionally remains visible after clear for Sapphire delivery retry and post-dialogue |
| 58 | `FLAG_WAYFARER_SEVII_HIDE_RUBY_GUARDS` | Celio gems started and Ruby not recovered |
| 59 | `FLAG_WAYFARER_SEVII_HIDE_RIVALS` | shared rival scene seen; the eligible on-frame scene clears this flag and explicitly spawns the current actor |
| 60 | `FLAG_WAYFARER_SEVII_HIDE_RETURNED_LOSTELLE` | Lostelle rescued |

Warehouse readiness is recomputed by `WayfarerSevii_Story_UpdateWarehouseReadiness` after either password/theft transition, so password order does not matter. These flags are saved so handlerless maps load correctly. The Warehouse clear transition removes the five combatants immediately but preserves Gideon; a full Key Items pocket therefore cannot strand the Sapphire reward.

## Trainer defeat ABI

Trainer allocation commits `7fa44c8b75`, `501ebf73e2`, and `662bc57560` freeze runtime IDs 1515–1650 as allocation slots 0–135. Story storage accepts allocation slots only:

```c
bool32 WayfarerSeviiTrainerDefeatGet(u16 slot);
void WayfarerSeviiTrainerDefeatSet(u16 slot);
void WayfarerSeviiTrainerDefeatClear(u16 slot);
```

All functions are safe for `slot >= 533`: get returns false and mutations do nothing. They never inspect or subtract a runtime Trainer ID. Trainer's generated router maps rematches to the base allocation slot before calling this ABI.

Story consumes the following Trainer-owned source identities, frozen at slots 118–135:

| Slots | Story source identities |
| ---: | --- |
| 118–121 | `TRAINER_BIKER_GOON`, `_2`, `_3`; `TRAINER_CUE_BALL_PAXTON` |
| 122–123 | `TRAINER_TEAM_ROCKET_GRUNT_43`, `_44` (Ruby guards) |
| 124 | `TRAINER_TEAM_ROCKET_GRUNT_45` (Lorelei) |
| 125–128 | `TRAINER_TEAM_ROCKET_GRUNT_46`, `_49`, `_50`, `_51` (local Rockets) |
| 129–134 | `TRAINER_TEAM_ROCKET_GRUNT_42`, `_47`, `_48`; `TRAINER_TEAM_ROCKET_ADMIN`, `_2`; `TRAINER_SCIENTIST_GIDEON` (Warehouse) |
| 135 | `TRAINER_LADY_SELPHY` |

Story scripts must use Trainer's generated `TRAINER_WAYFARER_SEVII_*` symbols and routing. They must not copy parties, allocate replacement IDs, or write defeat bits directly.

## Trainer rematch state ABI

Trainer commits `e4674af709` and `714a70fa3b` consume the story-owned extension in commit `7664a169ad`:

```c
u8 WayfarerSeviiRematchStageGet(u8 family);
void WayfarerSeviiRematchStageSet(u8 family, u8 stage);
bool8 WayfarerSeviiRematchPendingGet(u8 family);
void WayfarerSeviiRematchPendingSet(u8 family, bool8 pending);
void WayfarerSeviiRematchClearAllPending(void);
```

Stages use 16 packed bytes (two bits per family); pending-ready state uses 8 packed bytes. Every accessor is bounds-safe for `family >= 64`. Both banks are covered by whole-aggregate initialization and SaveBlock3 sidecar serialization, so pending readiness survives save/reload.

## Trainer Tower record ABI

Tower owns the record semantics and consumes this story-owned saved payload:

```c
struct WayfarerSeviiTrainerTowerRecords {
    u32 bestTime[NUM_TOWER_CHALLENGE_TYPES];
    u16 pendingPrize;
    u8 completedMask;
    u8 reserved;
};

struct WayfarerSeviiTrainerTowerRecords *WayfarerSevii_GetTrainerTowerRecords(void);
```

The payload is 20 bytes with 4-byte alignment. Zero is the initial state. Completed formats require a best time in `1..TRAINER_TOWER_MAX_TIME`; incomplete formats require zero. Pending prizes are restricted to `ITEM_NONE`, `ITEM_UP_GRADE`, `ITEM_DRAGON_SCALE`, `ITEM_METAL_COAT`, and `ITEM_KINGS_ROCK`; `reserved` remains zero. Tower keeps its active run snapshot transient.

## Integration

Consumers may integrate commit `0357e4a423` as the base prerequisite and `7664a169ad` as the coherent Trainer rematch extension; published history is not rewritten. The final integration must structurally merge each domain's records into the single schema-v2 manifest, regenerate combined projections, and repeat full validation; isolated branch results do not establish combined readiness.

## Story delivery and isolated validation

The complete story-domain implementation is committed through `ffac958849`. The final closure commits are:

- `9f40bfb4f4`: schema-v2 story projection (131 records, including 125 objects, 3 background events, and 3 scripts; 71 declared states, 18 Trainer slots, and 17 atomic transactions).
- `e167904683` and `ab8b43fb0a`: story actor graphics and full pointer/info/picture/raw-asset closure, plus `specialvar` ABI and legendary retry corrections.
- `ffac958849`: emulator fixtures and journeys for full-pocket Meteorite retry, ordered biker/Hypno loss retries, and Moltres TR gating plus loss/save/reload retry. The exploration sweep also explicitly suppresses the optional rival arrival scene so the ungated 135-map traversal remains deterministic.

Isolated branch validation at `ffac958849`:

- Wayfarer production release: 32,859,932 bytes used, ROM end `0x09F5671C`, 694,500 bytes unused. This is 186,596 bytes above starting main and leaves 170,212 bytes beyond the required 512 KiB reserve.
- SaveBlock3: 1,112 bytes, leaving 512 bytes below the 1,624-byte bound. The Sevii aggregate is 208 bytes, including the exact 24-byte rematch extension.
- Sevii content audit: 73 tests passed; Sevii port/catalog/script audit: 11 + 15 + 15 tests passed.
- Emulator: all 3 independent story journeys and all 6 Sevii exploration journeys passed against a freshly built E2E ROM.
- FireRed, LeafGreen, Emerald, HNS, and Wayfarer standalone release builds passed serially earlier in this branch; the final graphics closure was additionally compiled under Wayfarer, HNS, and FireRed provider guards.

These results cover the isolated story branch only. The parent integration remains responsible for merging sibling manifest domains structurally, regenerating the combined outputs, and renewing all validation and ROM measurements.

## Reduction follow-up

Commit `874d590047` removes 1,669 lines while adding 95 lines of simplified
requirements/documentation. Gameplay, saved-state layouts, allocations, map
selection, script providers, and generated runtime output are unchanged.
Focused critic follow-up `50b510a95a` removes the stale assertion that treated
`src/seagallop.c` as a script-generation input; `data/specials.inc` remains the
precise generator dependency and the complete Sevii port audit passes.

Concrete removals:

- deleted the Python graphics-provider parser and its three snapshot tests;
- deleted bespoke reward, retry, Move Maniac, Egg, Memorial, and handoff control-flow recognizers;
- deleted the nine-test actor-staging source recognizer;
- removed 43 duplicate C implementation path/hash pin pairs while retaining external table hashes and symbol checks;
- removed duplicate per-event transaction/receipt fields and the parallel passive-actor JSON inventory; the schema-v2 manifest remains authoritative;
- removed the content audit's reserialized selected/excluded inventory payload; and
- removed its duplicate Trainer compiler/scaling pass, leaving selected-party compilation to the Trainer-owned generator.

The content suite is now 44 tests instead of 73. On this host, removing the
duplicate Trainer compiler pass reduced the already-trimmed 45-test intermediate
suite from 13.8 seconds to 1.1 seconds. Stable post-reduction validation passed:

- `make -C game wayfarer-sevii-content-audit` (44 tests plus report generation);
- `make -C game wayfarer-sevii-port-audit` (11 catalog, 15 script, and 15 port tests);
- checked-in Sevii script generation with `generate.py --check`; and
- 3 focused story plus 6 exploration emulator journeys against the existing
  fresh E2E ROM.

No production rebuild was repeated because this commit changes only host
metadata/tests/docs and a script comment. The last measured production ROM and
SaveBlock3 figures above therefore remain the applicable runtime measurements.

## Trainer integration validation

Story is structurally integrated with Trainer main
`8347917770864a8d9a72811bea0587e0379cedd5`. The schema-v2 manifest retains all
135 maps, 261 unique retained events, and 10 map scripts: 87 ordinary-Trainer
objects plus 131 Story inventory records, with shared actor keys unioned by identity. Regeneration
retains Trainer's guarded rematch runtime and `EXCLUDED` policy for Story
objectives. The `HAS_SEVII_CONTENT` graphics pointer closure contains 52 unique
designated initializers; the 20 merge-duplicated ordinary entries were removed.

The confirmed flag collision is resolved with Trainer charging at slot 52 and
returned Lostelle presentation at slot 60. New-game initialization still sets
Story variable 4 to `0x8000`, initializes all eight Story hide flags, and
preserves the rematch and Tower-record payloads. Tower transient reset hooks are
deliberately left for the Tower integration stage.

Combined validation before push:

- all five shared generators pass `--check`; Trainer scaling reports 1,649
  populated IDs (`EXCLUDED` 215, `GYM_LEADER` 30, `GYM_MEMBER` 104,
  `ORDINARY` 1,300);
- Sevii content and port audits pass (45 content, 11 catalog, 15 script, and 15
  port tests), and the generated content audit passes against the release ROM;
- the focused Wayfarer mechanics run passes 103/103 tests;
- the exact concurrent Trainer 7 + Story 3 + exploration 6 emulator selection
  passes 16/16. A two-line Story driver guard now advances only overworld or
  explicit battle text while waiting for an action menu, eliminating redundant
  transition inputs without disabling global file parallelism; and
- the production release uses 32,909,152 bytes (delta +235,816 from foundation
  `fbc3bc33f7a37b89be850013938595155bb6ecc0`), ends at `0x09F62760`, and leaves
  645,280 bytes free: 120,992 bytes beyond the required 512 KiB reserve.
  SaveBlock3 remains 1,112 bytes, 512 bytes below the 1,624-byte bound.

These checks establish the combined Trainer + Story branch only. Final Tower
integration still requires structural regeneration and renewed acceptance.

# Sevii story implementation coordination

Status: base saved-state ABI published by commit `0357e4a423`; Trainer rematch extension published separately by commit `7664a169ad`; foundation base `fbc3bc33f7a37b89be850013938595155bb6ecc0`.

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
| Flags | 52–59 | Derived inverse actor presentation: returned Lostelle; returned Selphy/Butler; Dotted Hole scientist; optional local Rockets; Warehouse combatants; Warehouse Gideon; Ruby guards; rival actors |
| Variables | 0–2 | Selphy requested species, pending reward, and active request |
| Variables | 3 | `trainer_tower.pending_prize` transaction mirror |
| Variables | 4 | Heracross size record, initialized to `0x8000` |
| Trainer defeat | 0–532 | Trainer-owned fixed allocation capacity; current frozen inventory uses 0–135 |
| Rematch family | 0–63 | Trainer-owned two-bit stages and pending-ready bits |

The exact flag and variable symbols are authoritative in `include/constants/flags.h` and `include/constants/vars.h`. Unallocated ranges begin at flag slot 60 and variable slot 5.

## Story actor presentation

Slots 52–59 are condition-specific inverse flags because several source maps have no source map-script slot that the provenance-checked overlay can select. `WayfarerSeviiInitPersistentState()` initializes all eight hidden. Objective scripts write only the presentation flags affected by their durable transition; no arrival handler hides a still-undefeated actor.

| Slot | Hide flag | Actor is visible exactly when |
| ---: | --- | --- |
| 52 | `FLAG_WAYFARER_SEVII_HIDE_RETURNED_LOSTELLE` | Lostelle rescued |
| 53 | `FLAG_WAYFARER_SEVII_HIDE_RETURNED_SELPHY` | Selphy returned; shared by Selphy and Butler |
| 54 | `FLAG_WAYFARER_SEVII_HIDE_DOTTED_HOLE_SCIENTIST` | Celio gems started and Sapphire not stolen |
| 55 | `FLAG_WAYFARER_SEVII_HIDE_LOCAL_ROCKETS` | Celio gems started and Warehouse not cleared; shared by Meadow and Outcast actors |
| 56 | `FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_COMBATANTS` | Sapphire stolen, both passwords learned, and Warehouse not cleared |
| 57 | `FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_GIDEON` | Sapphire stolen and both passwords learned; intentionally remains visible after clear for Sapphire delivery retry and post-dialogue |
| 58 | `FLAG_WAYFARER_SEVII_HIDE_RUBY_GUARDS` | Celio gems started and Ruby not recovered |
| 59 | `FLAG_WAYFARER_SEVII_HIDE_RIVALS` | shared rival scene seen; the eligible on-frame scene clears this flag and explicitly spawns the current actor |

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

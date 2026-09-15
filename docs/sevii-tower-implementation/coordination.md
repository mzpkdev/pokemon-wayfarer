# Sevii Trainer Tower coordination

Foundation base: `fbc3bc33f7a37b89be850013938595155bb6ecc0`.
Published persistence/allocation contract: `d3eafb4a05`.
Integrated story-owned persistence prerequisite: upstream `0357e4a423`,
cherry-picked here as `d2afb48de3`.
Published facility runtime and overlay: `50f1dfcda4` and `11adca27d2`.
Published pending-prize transaction contract: `215d8d4836`.
Published E2E lifecycle ABI/journey: `37f987fa6f`.
Published transient lifecycle reset integration: `c51bb2afea`.

## Persistent payload contract

The story implementation owns the shared Wayfarer Sevii `SaveBlock3` aggregate,
save-version bump, validation, and new-game initialization. It should embed this
Tower-owned payload without exposing the standalone FRLG Trainer Tower save
layout:

```c
struct WayfarerSeviiTrainerTowerRecords
{
    u32 bestTime[NUM_TOWER_CHALLENGE_TYPES];
    u16 pendingPrize;
    u8 completedMask;
    u8 reserved;
};
```

The ABI is 20 bytes with 4-byte alignment. All bytes initialize to zero.
`bestTime[i] == 0` and a clear bit `i` in `completedMask` mean that format has no
record. Established times are in source Trainer Tower frames and must be in
`1..TRAINER_TOWER_MAX_TIME`. `pendingPrize == ITEM_NONE` means no claim; the only
valid nonzero values in this milestone are `ITEM_UP_GRADE`,
`ITEM_DRAGON_SCALE`, `ITEM_METAL_COAT`, and `ITEM_KINGS_ROCK`. `reserved` must
remain zero and is available only through a later coordinated layout revision.

The shared persistence module should expose:

```c
struct WayfarerSeviiTrainerTowerRecords *WayfarerSevii_GetTrainerTowerRecords(void);
```

The pointer is valid after save blocks are allocated. Whole-bank initialization
must zero the payload. Whole-bank validation must reject a completed bit with a
zero/out-of-range time, an established time without its completed bit, an
unknown `completedMask` bit, a nonzero `reserved`, or an item outside the prize
allowlist. Invalid current-version state follows the existing incompatible-save
path; Tower code does not repair or index invalid persistent data.

Tower runtime owns record comparison, pending-prize transitions, and the
accessor's consumers. Story owns only storage, initialization, and whole-bank
validity. The active run, final time, selected format/set/floor/opponent, cleared
floors, timer state, and healed party snapshot are transient and must never be
added to the saved aggregate.

`WayfarerTrainerTowerResetTransientState()` clears the active run and source
timer without restoring the transient snapshot. It is required after every
ordinary save load and during whole-bank new-game initialization: the loaded or
new party is authoritative, while an interrupted Tower run is discarded. Hall
of Fame auxiliary loads do not reset the run. Tower start scripts heal before
the final start-clock message, and the runtime defensively heals again before
capturing the entry snapshot.

## State and allocation inventory

- Tower facility opponents allocate **no Trainer IDs**, ordinary defeat bits,
  rematch identities, or ordinary scaling rows.
- Tower requests one transactional Sevii state key,
  `trainer_tower.pending_prize`, for manifest ownership/audit. The story state
  allocator assigned it variable slot 3 as
  `VAR_WAYFARER_SEVII_TRAINER_TOWER_PENDING_PRIZE`. The saved Tower payload is
  authoritative; this variable is the transaction contract mirror exposed to
  scripts and audits.
- `trainer_tower.pending_prize` transitions `ITEM_NONE -> <exact source prize>`
  only after immediate Bag delivery fails, and `<prize> -> ITEM_NONE` only after
  a later Bag delivery succeeds. A pending prize blocks starting another run.
- The four best times and completion mask are Tower payload fields, not script
  flags/variables. Tower C specials mediate them so scripts cannot perform raw
  state writes.

## Facility-owned identities

Formats are fixed in menu order: Single `0`, Double `1`, Knockout `2`, Mixed
`3`. Prizes are respectively Up-Grade, Dragon Scale, Metal Coat, and King's
Rock. Every format has eight floors. The frozen Mixed source rows are Mixed 1,
Mixed 2, Mixed 3, Double 8, Mixed 5, Knockout 8, Double 3, and Knockout 2.

The starting built-in set source is
`game/src/trainer_tower_sets.c`, SHA-256
`a3d5b451ae73b2ca6dcaf4b260544f979f6a5199ff15ab62510f5513ba515dc0`.
The local frozen data generator must pin a reviewed normalized checksum rather
than accept e-Reader, Mystery Gift, record-mixed, or downloaded payloads.

## Integration rule

The Tower branch changes only the manifest's `trainer_tower` domain plus Tower
states/transactions after the story owner publishes their allocation. Combined
integration must merge domain records structurally and regenerate the shared
artifact; do not resolve manifest conflicts by taking a whole-file version.

## Isolated branch validation

- Wayfarer focused mechanics: 8/8 Tower tests passed; all 48 Sevii content
  audit tests also passed in the same host check.
- Full Wayfarer check: 5,360 total tests with no unexpected failures (4,367
  passed; repository-known failures/assumptions/TODO/expected-failing retained).
- SkyEmu Tower lifecycle journey: 1/1 passed; E2E protocol suite 17/17 passed.
- Release products built serially for Wayfarer, Emerald, FireRed, LeafGreen,
  and HNS.
- Wayfarer release ROM uses 32,682,188 bytes and leaves 872,244 bytes total;
  the enforced 512 KiB reserve has 347,956 bytes of additional headroom.
- SaveBlock3 is 1,088 bytes, within the 1,624-byte bound; the Tower-owned
  persistent payload is 20 bytes.

These are isolated-branch results. Final three-branch integration must
regenerate combined outputs and rerun the same acceptance checks.

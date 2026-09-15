# Sevii Trainer implementation coordination

Status: allocation and runtime interface freeze published by commit `7fa44c8b75`.

## Shared Trainer allocation

The collision-audited allocation starts at 1515 and is dense through 1650 inclusive. `TRAINERS_COUNT_WAYFARER` becomes 1651 when the shared roster lands. Slots 0–117 are every distinct ordinary base/rematch party source; slots 118–135 reserve every planned story opponent. Trainer Tower opponents are facility-local and never enter this range. Source FRLG numeric IDs are provenance only and must never reach runtime.

The checked-in machine-readable `allocation-inventory.json` is the single authoritative per-key inventory. It records every slot, runtime ID, generated symbol, source key, owner, kind, defeat base, object reference, and normalized party hash; this coordination note deliberately does not duplicate its 136 rows.

## Saved defeat interface required from story

Story owns the `SaveBlock3` aggregate and new-game/save-version plumbing. Its published bank reserves the fixed 533-bit allocation capacity (67 bytes) indexed by slot; the current dense inventory uses slots 0–135. The payload is exposed without revealing the aggregate:

```c
bool32 WayfarerSeviiTrainerDefeatGet(u16 slot);
void WayfarerSeviiTrainerDefeatSet(u16 slot);
void WayfarerSeviiTrainerDefeatClear(u16 slot);
```

The functions must ignore slots at or above 533 safely. Trainer runtime maps every generated ID to its frozen `defeat_base` slot before calling them. Base victories set one bit; rematch IDs alias that base bit for object presentation and never allocate another persistent bit. Story objective callers may use their reserved slots through the same accessors. The Trainer branch will not define another saved aggregate.

## Runtime surfaces owned here

The Trainer branch will publish generated constants/roster data for all 136 keys, `TRAINERS_COUNT_WAYFARER = 1651`, one scaling classification per populated ID, the generated defeat-base lookup, and a separate Sevii Vs. Seeker registry. The registry consumes Story's compact persistent rematch-stage and pending-ready accessors and does not append 64 families to the existing fixed SaveBlock1 rematch-index array. Trainer owns Sevii flag slot 52 (`FLAG_WAYFARER_SEVII_VS_SEEKER_CHARGING`) for the Wayfarer-only Vs. Seeker charge lifecycle; Story owns slots 11-51 and presentation flags 53-60 (including returned Lostelle at 60); unallocated slots begin at 61.

Ordinary content uses normal victory/blackout routing. Story callers use their own objective continuations but consume the same generated roster IDs and defeat accessors. Tower remains excluded from ordinary scaling, persistent Trainer defeat, and this allocation.

## Integration rule

Consumers should integrate coherent prerequisite commits by hash and must not renumber the published range. Combined integration must merge domain-specific manifest records and regenerate the single projection outputs; no isolated branch can claim combined-manifest readiness.

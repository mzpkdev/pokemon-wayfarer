# A-C release evidence

This is a one-time paired release comparison and a limited inspection record for
the A-C refactor. It is not a profiling subsystem and does not run in `make
check` or CI. Permanent contracts run in the existing Expansion Suite; the
separate migration-equivalence job is temporary and does not run this evidence.
The compact source record is
[release-profile-evidence.json](release-profile-evidence.json).

The baseline is merged PR #91 squash commit
`f4d6861481ee8ac1f7d06351f0e7cab9fafb1ab4`. The measured implementation is
`8a88294762b756fefbfd0654a6c4f97c125b543a`, which contains the final shared
curve-dispatch representation. Each pair used the same product, feature
configuration, GCC 13.2.1 release toolchain, and `report.py` comparison of
matched ELF/map artifacts. The reporter verifies ROM boundaries, configuration,
toolchain, artifact hashes, linked used-ROM bytes, and static RAM. Padded `.gba`
sizes and generated JSON are not used as storage measurements.

The JSON keeps the exact retained command, configuration digest, and paired
artifact metadata. The final FireRed and LeafGreen ELF files were purged before
this compact record replaced the draft, so those two implementation ELF hashes
are marked unavailable rather than replaced with historical hashes. Their matched
maps, configuration digests, sizes, and static-RAM measurements remain recorded.

## Paired release resources

All figures are bytes. Static RAM is unchanged in every pair.

| Pair | Used ROM, baseline to implementation | Delta | Notable linked categories |
| --- | ---: | ---: | --- |
| Wayfarer | 32,249,612 to 32,249,296 | -316 | code -352; other +36 |
| Wayfarer with Gym scaling | 32,263,420 to 32,263,040 | -380 | code -416; other +36 |
| Wayfarer with trainer, League, and mart switches disabled | 32,246,004 to 32,245,872 | -132 | code -208; other +76 |
| HNS | 30,803,868 to 30,804,016 | +148 | code +136; graphics +8; other +4 |
| Emerald | 28,521,156 to 28,521,156 | 0 | all zero |
| FireRed | 28,616,076 to 28,616,076 | 0 | all zero |
| LeafGreen | 28,616,480 to 28,616,480 | 0 | all zero |

The complete A-C delivery reduces normal Wayfarer ROM by 316 bytes without a
static-RAM increase. A separately prepared clean phase-B build produced the
same padded ROM as phase C, so services add no additional final runtime bytes.

The small HNS increase is recorded rather than hidden. A controlled descriptor
representation comparison returned HNS to the baseline used-ROM size; it did
not retain the evaluator or progression points. The shared switch translation
unit is the controlled trigger; LTO's internal reason is an inference. The
increase is therefore an explained release-representation difference, not a
claim that HNS consumes the progression payload. It remains within the existing
ROM reserve and preserves static RAM.

## Representative release inspection

The changed optimized routines and their callers were inspected for the bounded
curve dispatch, fixed point arrays, and ordinary/Gym/League, daycare, EXP, and
obedience consumers. The implementation adds no per-frame callback, whole-world
scan, recursion, input-sized stack allocation, save field, heap allocation, or
interrupt registration; no framework query runs in an ISR.

Host-side direct stepping of matched, unmodified release code in mGBA
`012919433e331981d265f5a2fc3c550af57183b8` supplied these representative
diagnostics. They are not exhaustive CPU or peak-stack proofs:

| Route and selected input | Baseline to implementation delta |
| --- | ---: |
| EXP reduction, Caterpie level 50, Rating 80, reward 1000 | +243 cycles; equal 156-byte local stack observation |
| Obedience/canceler, deterministic success at Rating 80 | +212 cycles; 12 fewer observed stack bytes |
| Rod/Alice Gym-member route with Gym scaling disabled, Rating 0 / 80 | +34 / -99 cycles per query; 4 fewer observed stack bytes |
| Gym-member constructor, Rating 0 / 80 | +20 / -50 cycles per query; 12 fewer observed stack bytes |

The original probe used a hardcoded Trainer Rating save offset. Its outputs were
withdrawn and are not used here. The listed diagnostics use the corrected
fixture. The previously proposed exact cycle, per-frame, and peak-stack ceilings
were withdrawn from the specification; no general tracing tool is retained to
enforce them.

## Behavioral checks

After the final dispatch change, the framework host suite passed 35 checks and
the gameplay progression mechanics suite passed 3 checks. A matched E2E ROM and
symbol pair passed all 6 rod journeys in 22.45 seconds and 3 scoped League
journeys in 130.93 seconds (9 cases filtered). Those League cases cover
snapshot/save-load behavior, admission levels through all five earliest-Kanto
rooms, and defeat/save-load/retry. Exact hashes, commands, configurations, and
the name filter are in the JSON record.

Phase D is intentionally absent. It begins by rebasing #85 onto merged A-C and
comparing unrefactored and refactored trainer-only content on that same base.
This A-C evidence does not claim the overall specification is complete.

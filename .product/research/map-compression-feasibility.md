# Map compression feasibility and transition budget

The completed isolated POC supports a bounded playable integration. It demonstrates
**1,315,072 bytes of linked storage saving** and correct map copying in three runtime
fixtures. It does not establish shipping savings, real field memory safety, or acceptable
gameplay latency.

This summary records the 2026-09-12 investigation of
`ec18cfffd21f31b54a60a73d750863bde30be83d` and the subsequent product decision. It
supersedes the initial POC report's recommendation to treat its timing result as a
blocker before playable integration. The measurements have not changed.

## Product decision

An occasional short map-transition pause can be an acceptable tradeoff for at least
1 MiB of net ROM headroom in Wayfarer. The earlier one-frame p95 and two-frame maximum
limits were conservative requirements, not demonstrated limits of acceptable play.

Use a **provisional maximum of five additional frames** for a complete load or
transition against the equivalent pre-feature legacy load. Five GBA frames are
1,404,480 cycles, approximately 84 ms. This is one budget for the entire operation,
including current layout, all neighbors, validation, allocation, copies and release;
it is not an allowance per layout or checksum. Record median, p95 and maximum.

Evaluate hidden loads and visible crossings separately. A short extension while the
screen is already hidden or a brief pause at a seamless boundary is not an automatic
failure. Playable tests must establish acceptable walking, cycling and repeated
crossings, uninterrupted audio, correct input resumption, and unchanged gameplay.
Meeting the numerical ceiling alone does not approve rollout. The
[validation specification](../specs/compressed-map-layout-validation-rollout.md#timing-proof)
owns the current acceptance procedure.

## Linked storage evidence

The production selector regenerated **1,089 layouts**, including 102 enabled
FRLG/Sevii entries. All complete source files round-tripped exactly, including trailing
bytes beyond the logical grid. The hybrid catalog has 1,053 compressed entries and
36 entries selected raw at build time.

| Measure | Bytes |
| --- | ---: |
| Complete raw payloads | 1,875,152 |
| Hybrid payloads | 530,008 |
| Gross payload saving | 1,345,144 |
| 1,089 descriptors at 28 bytes, including CRC fields | 30,492 |
| Arithmetic saving before remaining costs and alignment | 1,314,652 |
| Measured storage-only linked saving | 1,315,072 |
| Measured margin above 1,048,576, before remaining shipping costs | 266,496 |

| Matched link | Used ROM bytes | `__rom_end` |
| --- | ---: | --- |
| Unmodified legacy release | 32,668,400 | `0x09F27AF0` |
| Raw descriptors | 32,698,896 | `0x09F2F210` |
| Hybrid descriptors | 31,353,328 | `0x09DE69F0` |

The experiment replaced only the generated maps object and relinked the same production
objects. Binary verification checked all 1,089 layouts and 1,331 table slots, including
excluded `NULL` entries, descriptor pointers, dimensions, sizes, codecs, alignment,
checksums and payload bytes. Actual placement and padding explain the 420-byte
difference from simple arithmetic. The raw descriptor link adds 30,496 bytes.

These release-flag links are deliberately **nonplayable**: the unchanged game loader
still expects raw pointers. They include descriptor CRC fields and linked alignment,
but exclude new loader and checksum code, error handling, special-map raw exceptions
and other shipping costs. The at-least-1-MiB gate still requires paired playable
production releases from the same revision and configuration. Padded ROM file lengths
are not the measure.

## Runtime evidence

The isolated ARM ROM uses the repository allocator and Fast-LZ decoder, extracted
unchanged legacy map-copy functions, and a one-buffer descriptor loader. It alternates
legacy, raw control and hybrid from equivalent grid, flag and heap state, 100 times
for each of three fixtures: **900 complete-load samples**.

| Fixture | Legacy cycles | Raw-control cycles | Hybrid cycles | Added cycles vs legacy | Added frames |
| --- | ---: | ---: | ---: | ---: | ---: |
| Route47, synthetic no-connection fixture | 145,233 | 760,776 | 1,061,189 | 915,956 | 3.26 |
| Route47, real north connection to Route48 | 151,871 | 915,488 | 1,300,491 | 1,148,620 | 4.09 |
| Route48, only its south connection to Route47 | 71,502 | 834,973 | 1,225,433 | 1,153,931 | 4.11 |

These are approximately **55–69 ms of added work**. Every fixture's minimum, median,
p95 and maximum are identical because the harness has no workload jitter. The reverse
Route48 fixture omits its real north connection to Safari Zone. The connection-heaviest
map and east/west cases were not measured. These results fit the new provisional
five-frame allowance; they do not establish the game's worst case.

All tested loads match every byte of the 20,480-byte backup grid, dimensions, destination
pointer and connection flags. Scratch poisoning after copying preserves the result,
and an isolated immutable point query succeeds after buffer reuse. An independent
emulator rerun reproduced the sample log and `PASS` result.

For the connected pair, separate component probes measured 146,288 cycles for stored
CRCs, 162,243 for bounded preflight, 210,960 for decoding and 635,710 for decoded CRCs.
Checksum work dominates this prototype's added cost. Raw control alone adds 763,617
cycles versus legacy; comparing hybrid only with raw control would conceal most of
the slowdown. Keeping a layout raw does not eliminate the proposed raw CRC cost.
The product decision changes the timing budget, not the required integrity checks.

The runtime environment was mGBA `0.11-7856-dbffb46c4`, its replacement BIOS,
`WAITCNT=0x40B4`, and Thumb `-O2` **without LTO**. It had no gameplay, audio, audio DMA
or enabled IRQ workload. The storage links used production release LTO. These are
different experiments; neither supplies production-equivalent gameplay timing.

## Where the cost occurs

| Context | Current engine behavior and acceptance implication |
| --- | --- |
| Ordinary movement within a map | Tile and collision reads use the existing RAM backup grid. They must remain free of repeated source decompression. |
| Warp and Fly | The ordinary load path hides the display before initializing the layout. Added work can extend the hidden transition; measure the complete return to control. |
| Seamless connected crossing | `CameraMove` calls `LoadMapFromCameraTransition` and `InitMap` synchronously before camera redraw. Added work can pause visible movement. Playtest walking, cycling and repeated crossings. |
| Continue | Layout initialization precedes restored field control. Measure load time and audio in the real resume path. |
| Origin, entry and special-map source reads | These are additional callers, sometimes in visible menus or scripts. Measure their complete caller operations; they are not covered by the three load fixtures. |

The call paths are in [fieldmap.c](../../game/src/fieldmap.c),
[field_camera.c](../../game/src/field_camera.c), [overworld.c](../../game/src/overworld.c)
and [fldeff_flash.c](../../game/src/fldeff_flash.c). A microbenchmark cannot determine
how noticeable a pause or audio interruption is in these paths.

## Memory evidence and remaining work

The fresh release ELF uses **248,525 static EWRAM bytes**, including its 115,968-byte
heap and 20,480-byte backup grid. The earlier 248,484-byte observation is historical.
The largest decoded layout is 14,640 bytes; its padded grid is 20,250 bytes, only
230 below the existing capacity. No additional permanent full-map buffer is allowed.

The synthetic empty heap has 115,952 contiguous bytes before and after a load and
101,296 during it: the 14,640-byte scratch consumes 14,656 bytes with its allocator
header. A fragmentation fixture with 59,824 bytes free in total but only 14,000
contiguous rejects that allocation. Total free bytes therefore cannot replace live
contiguous-heap measurements.

Stack paint found 996 bytes below the benchmark caller's stack pointer, including
Fast-LZ's 800-byte local buffer. It excludes the caller's existing frame and IRQ/SVC
stack peaks. The prototype's selected loader/checksum functions and CRC table occupy
2,154 bytes, not a complete shipping code budget.

The next evidence must come from bounded playable integration: actual largest and
connection-heaviest crossings, full connection sets, live heap fragmentation and stack
peaks, audio, input resumption, and all source-read callers. Exhaustive equivalence,
failure handling, integrity, saves, final linked savings and rollback remain production
acceptance gates. Further speed optimization is useful if those measurements require
it; failure of the retired one/two-frame rule alone does not block this next step.

## Evidence identity

The experiment is preserved in [PR #99](https://github.com/mzpkdev/pokemon-wayfarer/pull/99)
at [commit `0acaf41dd6ce15ed8ae86ff4d9a8fb6a82a7ae89`](https://github.com/mzpkdev/pokemon-wayfarer/commit/0acaf41dd6ce15ed8ae86ff4d9a8fb6a82a7ae89).
It is an evidence archive, not a dependency to merge or production rollout approval.
Its original latency recommendation used the earlier one/two-frame rule; the product
decision above records the later provisional five-frame budget.

These links pin the published evidence to that commit:

- [Experiment report](https://github.com/mzpkdev/pokemon-wayfarer/blob/0acaf41dd6ce15ed8ae86ff4d9a8fb6a82a7ae89/.product/research/map-compression-feasibility.md)
  and [reproduction instructions](https://github.com/mzpkdev/pokemon-wayfarer/blob/0acaf41dd6ce15ed8ae86ff4d9a8fb6a82a7ae89/tools/map_compression_poc/README.md).
- [Per-layout catalog](https://github.com/mzpkdev/pokemon-wayfarer/blob/0acaf41dd6ce15ed8ae86ff4d9a8fb6a82a7ae89/tools/map_compression_poc/artifacts/catalog/per-layout.tsv)
  and [linked storage verification](https://github.com/mzpkdev/pokemon-wayfarer/blob/0acaf41dd6ce15ed8ae86ff4d9a8fb6a82a7ae89/tools/map_compression_poc/artifacts/linked/verified.json).
- [Runtime summary](https://github.com/mzpkdev/pokemon-wayfarer/blob/0acaf41dd6ce15ed8ae86ff4d9a8fb6a82a7ae89/tools/map_compression_poc/artifacts/runtime/summary.json)
  and [all 900 timing samples](https://github.com/mzpkdev/pokemon-wayfarer/blob/0acaf41dd6ce15ed8ae86ff4d9a8fb6a82a7ae89/tools/map_compression_poc/artifacts/runtime/samples.csv).

The source and compact text results are committed. Full ROMs, ELFs, compressed streams,
logs and the original independent-review disposition remain local in
`tasks/map-compression-feasibility/pokemon-wayfarer/tools/map_compression_poc/artifacts/`;
the published report summarizes the review findings. Their hashes identify the original
artifacts but do not make those artifacts available from GitHub. The `runtime-initial`
directory is superseded exploratory work and is excluded from these results.

| Artifact | SHA-256 |
| --- | --- |
| Published POC report | `cee856111968ff7b315ce86fa2b0bee6f8cec7ed9d9ea6eaacd5531b37a3171e` |
| Published catalog TSV | `83b2f9f16b773ee2120b9a00d6647ad25a1d0bef551b1d5d80a3a653889d5e29` |
| Linked verification | `4dc9dfd6a5d026f9e303c33cd910f70223bdace5b026fa300a52371db9bb2c52` |
| Runtime summary | `19b5bb8f98258f17737dacf29ea86a60c71231d27ce8d2f73697b1ae43c1a0f3` |
| Published 900-sample CSV | `eb09f9a7494e71f4d808b4ade8b44dcaea6d8972ce08fceb2ae6bde89924517b` |
| Local runtime sample log and independent rerun | `9c5d8f1e4fee736373d615c92ad3d690bd233a71493b80e5d322ae8e8e6e727a` |
| Local runtime ELF | `3b7c02aa4224af4a9d05d1d10f236b1f81bdb959e00aa73f1fd573345df5eafb` |

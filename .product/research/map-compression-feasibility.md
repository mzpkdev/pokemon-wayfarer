# Map compression feasibility

Investigation at `ec18cfffd21f31b54a60a73d750863bde30be83d`, 2026-09-12,
on `task/map-compression-feasibility`. Recommendation: **conditional go for further
implementation, with latency as the next blocker; no-go for rollout of this POC**.

The space opportunity is credible. A matched production release link with compressed
payloads and all proposed descriptors saves **1,315,072 bytes**. This is a
**storage-only experiment**, before new loader code, failure handling, and special-map
exceptions. Its images are deliberately nonplayable: the original loader still expects
raw pointers. They are not shipping candidates or rollback builds.

The ARM experiment passes exact backup-grid comparisons and sequential-buffer lifetime
checks, but its straightforward verified loader adds **3.26–4.11 frames** against the
legacy loader. It misses both latency limits even without gameplay interrupts or audio.
Optimize and remeasure validation/decoding before building the broader release
framework. No production PRD gate is waived. All authored `map.bin` files, gameplay
code, IDs, save structures, and planning documents remain unchanged. The sibling
planning task owns corrections to the PRD/specs.

## Source and configuration

The inputs are the baseline PRD and three `compressed-map-layout-*.md` specs, the
`rom-footprint-spec-audit` sibling task's research report, the real `mapjson.cpp`
selector, and `map_data_rules.mk`'s
`wayfarer_sevii_maps.json` input. The old 987/991 counts omit current production scope.

Fresh generation selected **1,089 layouts**, including 102 enabled FRLG/Sevii layouts.
The existing temporary inventories match paths, dimensions, raw lengths and compressed
lengths, but carry no source hashes. They were comparison inputs only; every payload was
regenerated from this checkout and independently decoded in Python. The unrelated old
NewSinjoh source sample in that temporary directory differs from current source and was
not reused.

Tools: Ubuntu ARM GCC `13.2.1 20231009` (`15:13.2.rel1-2`), GNU binutils `2.42`,
repository `gbagfx`. Release configuration: `BUILD=wayfarer release`, `RELEASE=1`,
`TEST=0`, `E2E=0`, `DEBUG=0`, `-O2`, default release LTO enabled, all three legacy
multiboot capabilities disabled. Builds ran sequentially in the assigned task.
This draft publishes the research and experiment for review; no source in another
task was changed, and no merge or rollout was performed.

## Catalog and linked measurements

| Measure | Bytes |
| --- | ---: |
| Complete authored payloads | 1,875,152 |
| Hybrid payloads, including 36 raw fallbacks | 530,008 |
| Gross payload saving | 1,345,144 |
| 1,089 descriptors × 28 bytes, including CRC fields | 30,492 |
| Arithmetic saving before code/alignment/exceptions | 1,314,652 |

There are 1,053 profitable compressed entries. Complete files, including trailing bytes
beyond the logical grid, round-trip exactly. All selected padded grids fit the existing
buffer. The largest is Route47 HNS: 120×61, 14,640 decoded bytes and
135×75×2 = **20,250 padded bytes**, just 230 below capacity.

| Linked image | `__rom_end` | Used bytes | `maps_layouts` bytes |
| --- | --- | ---: | ---: |
| Unmodified legacy release | `0x09f27af0` | 32,668,400 | 2,230,876 |
| Storage raw descriptors | `0x09f2f210` | 32,698,896 | 2,261,368 |
| Storage hybrid descriptors | `0x09de69f0` | 31,353,328 | 915,796 |

The script replaced only the generated maps object and relinked the same production
objects with the same flags. It emitted schema-1 descriptors, replaced the pointer at
the existing layout offset, and retained all 1,331 table slots and `NULL` exclusions.
Binary verification checked every layout's dimensions, slot, payload bytes, descriptor
pointer, alignment, sizes, codec, and CRCs directly in all three linked ROMs.

The whole-ROM delta is **1,315,072**, with **266,496 bytes** remaining above the
1,048,576 target. Raw descriptors cost 30,496 linked bytes: 30,492 in maps and four in
downstream alignment. Hybrid reduces the map category by 1,315,080; downstream
graphics alignment consumes eight bytes. Code and every other category's size stay
constant. The 420-byte improvement over simple arithmetic comes from actual placement
and padding. Padded `.gba` file lengths are not used as the savings measure.

This is stronger than a source-size estimate, but it excludes all new executable loader
and failure-path costs. A heuristic family inventory would forgo 17,844 gross bytes if
Battle Pyramid, Trainer Hill, Secret Base, and identified player-room entries stayed
raw. That is an estimate, not a reviewed exception policy or exhaustive consumer map.
The final playable release must be linked against the same legacy baseline again.

## Runtime and memory boundary

The source audit supports sequential scratch lifetime. `fieldmap.c` copies the current
map and all connected strips inside `InitMapLayoutData`. Camera transitions call
`InitMap` before `CopySecondaryTilesetToVramUsingHeap` (`overworld.c:945`), and normal
loading also completes layout copying before later tileset phases. This makes early
release possible; it does not establish live heap availability.

The measured release ELF contains:

| Allocation | Bytes or address |
| --- | ---: |
| `.ewram` + `.ewram.sbss` | 4 + 248,521 = 248,525 |
| `gHeap`, within that EWRAM total | 115,968 at `0x02001634` |
| `sBackupMapData`, within that EWRAM total | 20,480 at `0x02024098` |
| `.iwram` + `.iwram.bss` | 408 + 25,144 = 25,552 |
| System-stack initial SP | `0x03007e40` |
| Static IWRAM end to initial system SP | 6,768 |

The 6,768-byte gap is a nominal stack budget, not a measured safe high-water bound.
`FastLZ77UnCompWram` itself uses `u32 funcBuffer[200]` (800 bytes), plus its wrapper
and the copied ARM routine's register saves. The old 248,484-byte EWRAM audit is not
this toolchain's current result. Another permanent 14,640-byte map buffer would exceed
the remaining static EWRAM space.

Route47 has a real north connection to Route48, offset 19; Route48 connects south back
with offset -19. A one-connection reverse fixture exercises the largest map as neighbor;
it omits Route48's other real connection, north to Safari Zone.
Route47's north strip uses the last seven Route48 rows, at destination columns 26–80.
The full source audit also confirms east strips are eight columns, other depths seven.

The migration inventory must include `wayfarer_persistence.c` (entry validation),
`wayfarer_origin.c` (`IsProfileValid`), and `test/wayfarer_hoenn_entry.c:45`, alongside fieldmap,
Battle Pyramid, Trainer Hill, Secret Bases and decoration restoration. Origin and
persistence checks can query a map before it becomes current: the mutable backup grid
cannot replace immutable source access there.

## ARM runtime experiment

The isolated ROM compiles the repository allocator and decoder assembly. Its generator
extracts eight complete, unchanged `fieldmap.c` function bodies for the legacy oracle
and shared copy primitives. The POC supplies one-buffer descriptor orchestration around
those copies. It uses actual BIOS `CpuSet`/`CpuFastSet` calls and copies
`FastUnsafeCopy32` into IWRAM at startup. Fast-LZ retains its 800-byte local function
buffer. No POC code is linked into `game/`.

Execution used bundled **mGBA `0.11-7856-dbffb46c4`**, production `WAITCNT=0x40b4`,
Thumb `-O2` without LTO, and the built-in replacement BIOS. The pinned upstream
[ROM-test driver](https://github.com/mgba-emu/mgba/blob/dbffb46c4e7d2e7a2cbed7c3488cece4c2176d4c/src/platform/test/rom-test-main.c)
requires `-R r0`; `-R 0` exits before executing a ROM. No physical BIOS, hardware,
gameplay, audio, DMA audio traffic, or enabled IRQ workload was supplied. These are
emulated microbenchmarks, not production-equivalent journey or release timing signoff.

The same ROM alternates legacy, raw-control and hybrid from equivalent reset grid/flags
and heap state, 100 times per case. The complete timer starts before loader entry and
stops after allocation, checks, clear/copy, all fixture connections, and release.
Comparisons and functional scratch poisoning occur outside those samples. Timer 2 is
stopped before reading its cascaded Timer 3, avoiding torn counter reads.

| Case | Legacy cycles | Raw-control cycles | Hybrid cycles | Hybrid minus legacy | Added frames |
| --- | ---: | ---: | ---: | ---: | ---: |
| Route47, synthetic no connections | 145,233 | 760,776 | 1,061,189 | 915,956 | 3.26 |
| Route47, real north connection to Route48 | 151,871 | 915,488 | 1,300,491 | 1,148,620 | 4.09 |
| Route48 south-strip fixture, only Route47 connection | 71,502 | 834,973 | 1,225,433 | 1,153,931 | 4.11 |

Every row's median, nearest-rank p95, minimum and maximum are identical across its 100
samples; no runtime jitter workload was present. A frame is 280,896 cycles, as used by
the game's audio timer (`m4a.c`). All 900 samples are retained. These fixture deltas
exceed the numerical one-frame p95 and two-frame maximum limits against legacy. The
production timing gate remains untested; this is not proof that every possible
implementation must fail.

For the connected pair, separate single-shot stage measurements were 146,288 cycles
for stored CRCs, 162,243 for bounded preflight, 210,960 for Fast-LZ and 635,710 for
decoded CRCs. These component probes are separate from the complete-load samples.
Raw control scans once and compares that CRC with both fields. Its Route47 connected
cost is already **763,617 cycles above legacy**; hybrid minus raw is only 385,003.
Using raw control as the sole timing baseline would hide most of the total slowdown.
Keeping a map raw under this validation implementation also retains substantial CRC
cost. LZ preflight is compressed-only, not shared overhead.

All cases compare every one of the **20,480 backup bytes**, dimensions, destination
pointer and connection flags after each load. Functional tests overwrite the scratch
with `0xdead` before freeing it and still match the oracle, demonstrating that current
and neighbor copies do not retain its tiles. A bounded immutable point query subsequently
decodes again and checks the authored tile, collision and elevation after scratch reuse;
the real origin/persistence consumers remain unmigrated. Fixture identities retain the
two authored connection lists and source hashes.

The actual allocator reports **115,952 contiguous bytes before and after**,
**101,296 during** the single **14,640-byte allocation**; the additional block header
cost is 16 bytes. This is an empty synthetic heap, not the live field peak. A separate
fragmentation test leaves 59,824 bytes free in total but only **14,000 contiguous**;
the 14,640-byte request fails, leaves an undefined grid, and restores the heap after
fixture cleanup. This concretely demonstrates why total heap capacity is insufficient.

Stack paint observes **996 bytes below the benchmark caller's SP `0x03007ca0`** in
each hybrid case, including Fast-LZ's local buffer, copied ARM routine and nested calls.
Lowest observed write is `0x030078bc`. The leaf paint/read probes do not push stack.
This excludes the caller's existing frame and IRQ/SVC stack peaks and cannot establish
a production high-water bound. No gameplay heap topology or stack measurement is claimed.

The runtime ELF contains 2,154 bytes for the selected CRC, preflight, open-view and
candidate functions and their 1,024-byte CRC table.
This measures a small prototype's code/data cost, not all shipping overhead. The release
error UI, ROM-section range checks, complete API/consumer migration, exhaustive bounds
and failure suite, special facilities, and gameplay journeys are omitted. The three
fixture paths do not cover the connection-heaviest map or east/west boundaries. The
shipping net-ROM gate remains outstanding despite the ample measured storage margin.

## Reproduction and retained evidence

Paths below are relative to this repository; the task itself is managed by uberepo.
The full compiler/assembler/link commands are retained in local logs. The
[POC README](../../tools/map_compression_poc/README.md) supplies a fresh-checkout
reproduction sequence, including building the compressor; the commands below record
the original investigation, whose optional `/tmp` audit inputs remain local.
They assume the compressor already exists at the specified path. Absolute paths in
retained command/identity JSONs are archival host provenance, not portable commands.

```sh
uberepo context map-compression-feasibility
make -C game -j6 BUILD=wayfarer release > tools/map_compression_poc/artifacts/builds/legacy-build.log 2>&1
python3 tools/map_compression_poc/catalog.py --root . --gbagfx tools/map_compression_poc/artifacts/catalog/gbagfx --audit-dir /tmp/rom-spec-audit-maps --output tools/map_compression_poc/artifacts/catalog
python3 tools/map_compression_poc/linked_storage.py
python3 tools/map_compression_poc/verify_linked.py
make -C tools/map_compression_poc/runtime > tools/map_compression_poc/artifacts/runtime/build.log 2>&1
game/tools/mgba/mgba-rom-test -S 3 -R r0 tools/map_compression_poc/artifacts/runtime/map-compression-runtime.gba > tools/map_compression_poc/artifacts/runtime/mgba-run.log 2>&1
python3 tools/map_compression_poc/runtime/analyze.py
```

Use a fresh `--output` directory for another linked experiment, passing it to both
scripts. The link script restores the original maps object, ELF, ROM, map and size report
in `game/` on success or failure. It never edits authored assets or generated assembly.

Evidence lives under `tools/map_compression_poc/artifacts/`:

- `catalog/per-layout.tsv` and `summary.json`: committed source SHA-256 and CRC
  identities, sizes and decode results. The equivalent `per-layout.json` and
  compressed `streams/` remain local.
- `linked/{legacy,raw-control,hybrid}/`: ELF, ROM, linker map, ROM report, maps object,
  generated assembly and logs. `linked/comparison.json`, `commands.json` and
  `verified.json` retain the raw comparison and binary verification.
- `builds/legacy-build.log` and `builds/storage-experiment.log`: baseline and relink logs.
- `runtime/summary.json`, `samples.csv`, `mgba-run.log`, ELF, ROM and disassembly:
  accepted microbenchmark evidence and source/artifact hashes. `runtime/data/` retains
  source fixtures, generated CRCs and exact extracted legacy bodies with hashes.
- `runtime-initial/`: superseded exploratory harness and logs; excluded from results.
- `review/runtime-mgba-rerun.log` and `review/disposition.json`: local-only independent
  rerun and review dispositions, recorded before publication-only report edits.
  The reviewer reproduced the catalog hash, all linked values,
  all 900 runtime samples and `PASS`. Its two runtime findings concern the truncated
  Route48 connection fixture and deterministic timing samples; both limits are stated
  above. No timer, ABI, extraction or exercised-buffer defect was found.
- `builds/source-identity.json`, `legacy-sections.txt`, `legacy-symbols.txt` and toolchain
  version files; `provenance/catalog-build-command.json` contains the exact standalone
  `gbagfx` compilation command. Large generated artifacts are retained locally and
  ignored by Git. The draft PR includes the report, POC source/scripts and compact
  catalog, linked-ROM and runtime text evidence. Full binary artifacts and logs remain
  local; their hashes identify the original measurements.

| Artifact | SHA-256 |
| --- | --- |
| Committed catalog `per-layout.tsv` | `83b2f9f16b773ee2120b9a00d6647ad25a1d0bef551b1d5d80a3a653889d5e29` |
| Local catalog `per-layout.json` | `6a5b9928b341eb5f7cd2390e8133b894f0fa7c0f7b5d7a44767eccef40ea0377` |
| `gbagfx` | `cd98c8ac15a1d7f93c89e18d724a547f19322e826e262c98778b7d5efed3fc15` |
| Legacy ELF | `70711cd2e57585906366e391063f3266682bf57c0874a090dd6c56b209cce0d7` |
| Storage raw-descriptor ELF | `772434b798a23f55e5b46d12dc77f3aa24b88c3de09368151c2eba32a43c7ec0` |
| Storage hybrid ELF | `74e26d55fc41d2c0950b0eb2193c700dbc5728554bd5eff5eefb220b4cbf6c0d` |
| Runtime ELF | `3b7c02aa4224af4a9d05d1d10f236b1f81bdb959e00aa73f1fd573345df5eafb` |
| Bundled mGBA executable | `4304688a0b44007cca23c9e887af17c44530fd602ee3d1e57a232da397bec769` |

Before production acceptance, missing evidence includes a fully migrated loader and
failure UI; exhaustive real/synthetic connection and direct-consumer differentials;
real field heap fragmentation and stack high water during warp, camera, Fly and save
reload; complete legacy/raw-control/hybrid timings under equivalent gameplay states;
audio/fade observations; malformed-input and lifetime failures; and final playable
linked savings after all shipping costs. A list of required tests is not evidence that
they passed.

The investigation's compilers, linkers and emulator runs have stopped, and original
`game/` release artifacts are restored byte for byte. Make's untracked
`.map_version.wayfarer` stamp is excluded from the PR. This draft publishes research
and an isolated experiment; it makes no game-source or production-configuration change.

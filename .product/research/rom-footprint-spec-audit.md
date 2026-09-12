# ROM footprint proposal audit

The five proposals use credible techniques. Two already have measured release
savings. The other three are plausible. This report records the source-contract,
inventory, and validation defects found in the audited planning baseline.
Map compression has the largest potential benefit. Its subsequent
[isolated feasibility POC](map-compression-feasibility.md) now supports bounded
playable integration; its space is not yet accepted shipping headroom.

This audit covers five PRDs and their nine specifications at
`ec18cfffd21f31b54a60a73d750863bde30be83d`, the local `origin/main` snapshot on
2026-09-12. The resulting PRD/spec corrections have since been applied in this
task: Ice is an independent Normal-backed control, tileset production rules and
task-slot accounting are explicit, map planning uses the production catalog and complete consumer
inventory, timing includes the legacy loader, and multiboot validation status
is explicit. The findings below describe the original baseline; they are not
unresolved documentation defects. The later map POC measured 1,315,072 bytes of
storage-only linked savings and 3.26–4.11 added frames in three isolated runtime
fixtures. Product discussion replaces the original one/two-frame timing veto
with a provisional five-additional-frame complete-transition budget and separate
playable evaluation of hidden loads and visible crossings. Integrity, live-memory,
gameplay, final net-ROM and rollback gates remain. The linked report distinguishes
that decision from measured evidence. Game sources remain unchanged. Separate
feature implementations and the additional-savings research are outside this audit.

| Proposal | Judgment | What the evidence supports |
| --- | --- | --- |
| [Surf pixel sharing](../prds/wayfarer-surf-pokemon-pixel-sheet-deduplication.md) | Proven and worthwhile; already implemented. | Historical paired releases save **430,080 bytes (420 KiB)**. Fresh regeneration still verifies all 61 approved pairs. |
| [Legacy multiboot removal](../prds/wayfarer-legacy-multiboot-removal.md) | Proven space recovery through an explicit feature cut; already implemented. | Historical paired releases save **209,556 bytes**. This deliberately removes GameCube, Berry-fix transmitter, and e-Reader transfer support. |
| [Map compression](../prds/compressed-map-layouts.md) | Credible, high-value, conditional on memory and loading proof. | Fresh production-catalog compression saves **1,345,144 gross bytes** with per-layout raw selection at build time. The **at least 1 MiB net** requirement remains unproven by a paired release. |
| [Exact asset aliases](../prds/exact-rom-asset-aliasing.md) | Viable after correcting the Arceus source contract. | The stated **67,544-byte ceiling** includes an Ice-form comparison that fails. Conservatively excluding that alias reduces the ceiling to **66,260 bytes**, before linked-build verification. |
| [Tileset storage](../prds/tileset-storage-optimization.md) | Viable but small; original production sizes are correct. | Fresh conversion reproduces **19,600 gross bytes** for the two selected assets. The **19,500-byte net minimum** still requires a paired release. |

The Surf and multiboot amounts are already incorporated into the audited
baseline. They are not new headroom. Their sum is also a sum of two separate
historical experiments, not a fresh measurement of today's combined change.
The production budget is based on `__rom_end` and `rom.used_bytes`; the
32 MiB padded ROM file length cannot measure these savings. The current
[ROM-report implementation](../../game/tools/rom_report/rom_report.py) enforces
the 512 KiB reserve at `0x09F80000`.

1. **Correct the Arceus Ice source and rollback contract before implementing
   exact aliases.** The [graphics specification](../specs/exact-rom-asset-aliasing-graphics.md#arceus-overworld-allowlist)
   requires regenerating each form's own PNG and comparing its generated pixels
   and compressed bytes to Normal. It also says removing an alias restores an
   `INCBIN_COMP` from that form's own path. The
   [validation specification](../specs/exact-rom-asset-aliasing-validation.md)
   requires 12,288 decoded and 1,284 encoded bytes for every form.

   Current source emits all 18 Arceus objects from Normal's compressed asset.
   Normal and 16 other form inputs regenerate identically, but Ice differs in
   **262 decoded bytes** and compresses to **1,268 bytes**, compared with Normal's
   1,284. Thus the current ROM has duplicate bytes that can be shared, while the
   proposed source-art equality proof cannot pass. The own-path removal policy
   would also change Ice's currently displayed pixels.

   The conservative correction is to leave Ice outside the alias batch,
   preserving its current independent allocation from the Normal input, and
   reduce the claimed saving by 1,284 bytes. Alternatively, explicitly review
   a manifest that describes the inputs the ROM actually consumes. Neither
   approach should silently normalize the separate Ice artwork. This is a
   mismatch between the specification and source provenance, not a failure of
   linker aliasing.

2. **Correction: the original tileset size finding is withdrawn.** The first
   audit incorrectly treated a direct converter's default 3,072-byte output as
   the production input. [graphics_file_rules.mk](../../game/graphics_file_rules.mk)
   explicitly uses `-num_tiles 83` for each active Secret Base sheet,
   `-num_tiles 82` for its legacy counterpart, and `-num_tiles 120` for the
   unknown Cable Club sheet. Reproducing those actual rules yields **2,656**,
   **2,624**, and **3,840 bytes**, respectively. The original specification's
   sizes and active-versus-legacy distinction were correct. The six active
   inputs total 15,936 bytes; the padded 17,340-byte LZ result is indeed a
   1,404-byte regression. The original 29,044-byte legacy inventory is also
   consistent with these rules; it is not a new linked-ROM saving.

   Task accounting must distinguish added work from total work. None of the
   31 affected layouts uses both selected assets, so each adds one compressed
   copy. Cable Club also loads an already-compressed Building primary: its
   paired explicit-heap route needs two total free task slots before both
   copies, preserving the original protection. Secret Base pairs need one.
   Count all compressed copies, including unchanged companions, alongside
   active tasks and outstanding DMA work; check headroom before each copy.
   The docs now make that distinction explicit and retain the correct sizes.

   The actual selected inputs reproduce exactly: Secret Base primary is
   **16,384 → 9,140 bytes**, and Cable Club secondary is
   **16,384 → 4,028 bytes**, with byte-perfect host decompression. This is
   evidence for a real but modest optimization. Avoid treating removal of
   already garbage-collected legacy definitions as additional space recovery.

3. **Treat map compression's runtime and net-ROM gates as outstanding work.**
   The production selector now includes **1,089 layouts**, including 102 enabled
   FRLG/Sevii entries; the PRD's 987/991-layout inventories predate that scope.
   The exact selector in [mapjson.cpp](../../game/tools/mapjson/mapjson.cpp)
   consumes the [Sevii manifest](../../game/src/data/wayfarer_sevii_maps.json)
   through [map_data_rules.mk](../../game/map_data_rules.mk). All 1,089 selected
   layouts round-trip with the repository's GBA LZ77 tool. They total
   **1,875,152 raw bytes**, or **530,008 stored bytes** when unprofitable
   compressed files stay raw: **1,345,144 gross bytes saved**.

   At 28 bytes each, the proposed descriptors nominally cost **30,492 bytes**.
   That leaves **1,314,652 bytes before loader code, integrity checks, error UI,
   alignment, and raw exceptions**, or 266,076 bytes above the 1 MiB gate.
   The space budget is credible. A source conversion inventory still cannot
   replace a matched linked-ROM measurement.

   The design keeps authoring data raw, preserves map identities, and decodes
   layouts independently. Reusing a temporary heap buffer is plausible; adding
   another permanent map-sized EWRAM array would violate the documented memory
   budget. The PRD correctly recognizes that the heap already sits inside
   static EWRAM allocation and cannot be counted as additional RAM. The current
   load order permits releasing the layout scratch buffer before later tileset
   allocations. This is a sound architectural basis, not evidence that the
   live heap has already passed stress testing.

   Two specification changes are needed. First, extend the direct-consumer
   migration inventory to include [Hoenn entry safety](../../game/src/wayfarer_persistence.c)
   around line 321 and [origin validation](../../game/src/wayfarer_origin.c)
   around line 97, plus the Hoenn-entry mechanics test. These still read
   `MapLayout.map` directly and need the proposed checked access API.

   Second, the [timing gate](../specs/compressed-map-layout-validation-rollout.md#timing-proof)
   compares hybrid against a raw control that already performs descriptor and
   checksum checks. Any slowdown common to both builds is cancelled by that
   comparison. Keep the useful raw-control comparison, but also measure
   against the pre-feature raw loader or set an explicit total-load ceiling.
   LZ-specific preflight is only on the compressed path and is already included
   in the existing relative measurement.

   The remaining proof must cover largest contiguous heap availability,
   buffer lifetimes during transitions, connected-map strips, dynamic layouts,
   and the CPU cost of decoding and integrity checks. A detailed failure policy
   and a list of tests do not establish those results. The first deliverable
   should measure a largest-layout load and a connected-map transition with
   the proposed verification enabled, followed by matched release links. Keep
   the 1 MiB threshold as an acceptance condition, not a forecast already
   earned by the payload compression ratio.

4. **Keep implementation status separate from completed acceptance.** Surf
   has the strongest retained evidence: paired release measurements, linked
   pixel and palette checks, and representative runtime comparisons. Its
   [validation report](surf-pixel-deduplication/validation.md) records 12 cases
   on each ROM and 180 identical screenshot pairs. Those runtime results were
   reviewed here, not rerun.

   Multiboot's `Implemented: Yes` describes existing code, but its
   [validation report](wayfarer-legacy-multiboot-validation.md) explicitly leaves
   cable/RFU/Mystery Gift hardware checks and prepared-save runtime cases
   unfulfilled. Its ROM saving is well supported; complete compatibility
   acceptance is not. Close or explicitly revise those outstanding acceptance
   requirements instead of treating the header as evidence that they passed.

The exact-sharing mechanism itself is conventional. GCC supports same-type
aliases whose targets are defined in the same translation unit; the specs
appropriately preserve array identity rather than replacing arrays with pointer
objects. [GCC variable-attribute documentation](https://gcc.gnu.org/onlinedocs/gcc-15.1.0/gcc/Common-Variable-Attributes.html).
Current matching MIDI inputs, equivalent voicegroup definitions, matching
Bike Shop assets, and matching current-frame inputs support the remaining
alias families. Audio still needs relocation-aware linked comparison and
playback validation; matching source bytes alone does not prove every song
reference resolves correctly.

With the Arceus source contract corrected, implement the reviewed exact aliases
as a small release, with map-compression prototyping
in parallel as the main capacity investment. The tileset change can follow as
a bounded cleanup. The two small proposals offer roughly **84 KiB gross** after
excluding Ice; they cannot substitute for the map work if another megabyte is
needed. Keep artifact checks proportional to the affected data and preserve
the substantive byte-equivalence, memory-lifetime, and release-budget gates.

Fresh checks on the audited checkout passed:

- `make -C game -j4 BUILD=wayfarer surfable-pokemon-pixel-alias-test`: regenerated
  all 122 approved source outputs, verified 61 equal pairs, and passed seven
  validator regression tests. Log: `/tmp/rom-spec-audit-surf-generation.log`.
- `python3 -m unittest discover -s tools/legacy_multiboot -p 'test_*.py' -q`
  from `game/`: seven tests passed.
- `python3 -m unittest discover -s tools/rom_report/tests -p 'test_*.py' -q`
  from `game/`: 21 tests passed.
- Independent host asset conversions and decompression checks for the selected
  tilesets and Arceus forms produced the measurements above.
- All 1,089 production-selected map layouts compressed and decompressed without
  a byte mismatch. The largest source layout remains 14,640 bytes, and every
  selected layout fits the existing padded map buffer.

The Arceus commands, hashes, ARM alias probes, and their limitations are retained
in `/tmp/rom-spec-audit-alias/report.md`. Corrected tileset production-rule
measurements are in
`/tmp/rom-spec-audit-tileset-production.CUwl1b/measurements.log`.
The earlier default-converter results under `/tmp/rom-spec-audit-tilesets.7VjYpd`,
`/tmp/rom-spec-audit-tileset-pairs.e4T0M4`, and
`/tmp/rom-spec-audit-tileset-padding.NBkIwm` do not establish production sheet
sizes and must not be used for that claim. The scoped input manifest is
`/tmp/rom-spec-audit-doc-inputs.json`; all 14 document hashes remained unchanged
through the original audit, before the authorized corrections above.
The exact production map selection is retained in
`/tmp/rom-spec-audit-maps/wayfarer-production-selected-layouts.tsv`, alongside
the temporary compression outputs.

No fresh full-ROM pair, hardware test, or new gameplay implementation was run
for this audit. Historical savings are attributed to their retained
[Surf comparison](surf-pixel-deduplication/comparison.json) and
[multiboot comparison](legacy-multiboot-validation/paired-release.json).

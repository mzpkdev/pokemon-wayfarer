# Map-layout compression POC / experiment

This is a feasibility experiment. It does not implement or enable production map
compression. The storage-only relinks are **nonplayable** because the game loader is
not migrated. The separate ARM ROM is a synthetic benchmark, not a playable game.

Read the [feasibility report](../../.product/research/map-compression-feasibility.md)
for the measured results and limitations. Compact text evidence is committed under
`artifacts/`; ROMs, ELFs, compressed streams and full logs remain local build outputs.
Absolute paths in the retained command and identity JSONs record the original host;
use the commands below to reproduce the experiment in another checkout.
In particular, the 1,315,072-byte storage result excludes shipping loader/failure costs,
and the 3.26–4.11-frame timing deltas are deterministic microbenchmarks.

## Reproduce

Run from the repository root with the normal game build dependencies, ARM GCC and
Python installed. Use this isolated task checkout: the storage experiment temporarily
replaces the generated maps object and release outputs, then restores the baseline.
Do not run another game build concurrently. All authored map files remain unchanged.

```sh
mkdir -p tools/map_compression_poc/artifacts/builds tools/map_compression_poc/artifacts/catalog tools/map_compression_poc/artifacts/runtime
make -C game/tools/gbagfx
cp game/tools/gbagfx/gbagfx tools/map_compression_poc/artifacts/catalog/gbagfx
python3 tools/map_compression_poc/catalog.py --root . --gbagfx tools/map_compression_poc/artifacts/catalog/gbagfx --output tools/map_compression_poc/artifacts/catalog
make -C game -j6 BUILD=wayfarer release > tools/map_compression_poc/artifacts/builds/legacy-build.log 2>&1
python3 tools/map_compression_poc/linked_storage.py --output tools/map_compression_poc/artifacts/linked-rerun
python3 tools/map_compression_poc/verify_linked.py --output tools/map_compression_poc/artifacts/linked-rerun
make -C tools/map_compression_poc/runtime > tools/map_compression_poc/artifacts/runtime/build.log 2>&1
timeout 60 game/tools/mgba/mgba-rom-test -S 3 -R r0 tools/map_compression_poc/artifacts/runtime/map-compression-runtime.gba > tools/map_compression_poc/artifacts/runtime/mgba-run.log 2>&1
python3 tools/map_compression_poc/runtime/analyze.py
```

Choose a fresh linked-output directory for each rerun. Regeneration can change the
committed measurement snapshots; preserve the original evidence when comparing runs.
The optional historical `--audit-dir` used in the original investigation is unnecessary
for reproducing the current catalog.

The runtime harness covers Route47 without connections, Route47's north connection,
and a Route48 south-strip fixture that omits its Safari Zone connection. It has no live
gameplay heap, audio/IRQ workload, shipping error UI, complete consumer migration or
release acceptance suite. No ROM, ELF, raw map payload or compressed map stream is
included in this draft PR.

#!/usr/bin/env python3
"""Validate and summarize the exact-fieldmap mGBA experiment's retained samples."""
from pathlib import Path
import csv,hashlib,json,re,statistics,subprocess
root=Path(__file__).resolve().parents[3]
out=root/'tools/map_compression_poc/artifacts/runtime'
log=(out/'mgba-run.log').read_text()
assert 'GBA Debug: PASS\n' in log and 'GBA Debug: FAIL' not in log
rows=[tuple(map(int,m.groups())) for m in re.finditer(r'GBA Debug: S (\d+) (\d+) (\d+) (\d+)',log)]
assert len(rows)==900 and len(set((c,m,n) for c,m,n,v in rows))==900
with (out/'samples.csv').open('w') as f:
 w=csv.writer(f);w.writerow(['case','mode','sample','cycles']);w.writerows(rows)
def stats(values):
 return {'n':len(values),'median':statistics.median(values),'p95':sorted(values)[int(len(values)*.95)-1],'maximum':max(values),'minimum':min(values)}
summary={'cases':{},'notes':['Exact extracted fieldmap loader/copy functions; independent one-buffer descriptor orchestration.',
 'Minimal ARM ROM, -O2 without LTO; production WAITCNT=0x40b4; mGBA built-in BIOS; no IRQ/audio/gameplay state.',
 'Complete timings include allocation, validation, checksums, LZ preflight/decode, backup clear/copy, connections and Free. Functional poisoning and comparison are outside timings.',
 'Raw verifies one CRC scan against both descriptor CRC fields; hybrid scans stored and decoded bytes.',
 'Stage timings are separate one-shot measurements, not components instrumented inside the 100 complete-load samples.',
 'Stack paint measures below benchmark caller SP, includes actual 800-byte FastLZ local plus copied ARM routine; excludes parent, IRQ/SVC stack high-water.'],
 'frame_cycles':280896,'emulator':'mGBA 0.11-7856-dbffb46c4',
 'command':'game/tools/mgba/mgba-rom-test -S 3 -R r0 tools/map_compression_poc/artifacts/runtime/map-compression-runtime.gba'}
for case,name in enumerate(['Route47 isolated','Route47 north to Route48','Route48 south to Route47']):
 sets={m:[v for c,mode,n,v in rows if c==case and mode==m] for m in range(3)}
 assert all(len(v)==100 for v in sets.values())
 entry={'name':name,'legacy':stats(sets[0]),'raw_control':stats(sets[1]),'hybrid':stats(sets[2])}
 for dest,a,b in [('hybrid_minus_legacy',2,0),('raw_minus_legacy',1,0),('hybrid_minus_raw',2,1)]:
  entry[dest]=stats([x-y for x,y in zip(sets[a],sets[b])]);entry[dest]['p95_frames']=entry[dest]['p95']/280896
 summary['cases'][str(case)]=entry
for tag in ['HEAP','SCRATCH','STACK','POINT','STAGES','FRAGMENT']:
 summary[tag.lower()]=[list(map(int,m.groups())) for m in re.finditer(r'GBA Debug: '+tag+r' (\d+) (\d+) (\d+) (\d+)',log)]
paths=list((root/'tools/map_compression_poc/runtime').glob('*'))+[out/'map-compression-runtime.elf',out/'map-compression-runtime.gba',out/'mgba-run.log',out/'data/identity.json',out/'data/legacy.inc',out/'data/legacy-identity.json',root/'game/tools/mgba/mgba-rom-test',root/'game/src/malloc.c',root/'game/src/decompress_asm.s']
summary['sha256']={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths if p.is_file()}
(out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps({k:{f:v for f,v in x.items() if f in ['name','hybrid_minus_legacy','hybrid_minus_raw']} for k,x in summary['cases'].items()},indent=2))

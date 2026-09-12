#!/usr/bin/env python3
"""Extract byte-for-byte legacy loader bodies; no game source is changed."""
from pathlib import Path
import hashlib,json,re
root=Path(__file__).resolve().parents[3]
source=root/'game/src/fieldmap.c'
s=source.read_text()
names=['InitMapLayoutData','InitBackupMapLayoutData','InitBackupMapLayoutConnections','FillConnection','FillSouthConnection','FillNorthConnection','FillWestConnection','FillEastConnection']
bodies=[]
for name in names:
 m=re.search(r'static void '+name+r'\([^;]*?\)\n\{',s)
 assert m,name
 level=1;end=m.end()
 while level:
  level+=(s[end]=='{')-(s[end]=='}');end+=1
 bodies.append(s[m.start():end])
out=root/'tools/map_compression_poc/artifacts/runtime/data'
out.mkdir(parents=True,exist_ok=True)
(out/'legacy.inc').write_text('\n'.join(b.split('\n{')[0]+';' for b in bodies)+'\n\n'+'\n\n'.join(bodies)+'\n')
(out/'legacy-identity.json').write_text(json.dumps({'source':str(source.relative_to(root)),'sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'functions':{n:hashlib.sha256(b.encode()).hexdigest() for n,b in zip(names,bodies)}},indent=2)+'\n')

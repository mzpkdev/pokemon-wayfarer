# FRLG Cinnabar seam probe

Base revision: `3ff46adcfabc41278d1a8ab6ab700be126b24c3f`.

This task branch contains a temporary, empty 24×20 Cinnabar exterior using the
checked-in FRLG layout and tilesets. Route 20 west and Route 21 south connect to
it at offset zero, with reciprocal links. The original HNS Cinnabar remains in
the catalog. This geometry probe has no town events, warps, wild encounters,
healing, or save point; it is not a port of the town.

Run `python3 .product/research/frlg-cinnabar-seam-poc.py` to inspect the source
coastlines. The script reads the checked-in map and attribute binaries; it does
not write `map.bin`. It finds 16 shared open-water rows on the east seam and
seven shared open-water columns on the north seam at zero offset. Its simple
flood fill finds a Route 21 water path to the north and coastal water adjacent
to the town's dry approach. The flood fill ignores directional collision and
elevation, so emulator traversal remains necessary.

`make -C game wayfarer -j4 CXX=g++` links a Wayfarer ROM with this map. The
linked ROM reports 33,366,068 bytes used of 32 MiB (99.44%). This is the
whole build's footprint, not the PoC's isolated incremental cost. The map
generator accepts the new map, layout, and catalog. The playable E2E ROM also
builds, and four focused SkyEmu checks pass. Porymap inspection remains open.

The visual defect came from the primary tileset: the FRLG town uses
`gTileset_General_Frlg`, while both HNS routes use
`gTileset_Kanto_General_Hns`. The old camera transition reloaded only the
secondary graphics and palette, and restarted only the secondary animation.
For example, both maps use water metatile 299, but two referenced primary
tiles (65 and 321) differ. The HNS and FRLG primary animation callbacks also
write different graphics over tile 416. A second defect remained after loading
the primary graphics: camera updates redrew only the entering slice, leaving
visible BG entries decoded with the previous map's metatile definitions.

The PoC now reloads the primary graphics and palette, reapplies weather color,
and restarts both animations when the primary tileset changes. Both ordinary
and credits camera updates redraw the whole map view after a map transition.
The focused SkyEmu checks pass, and the Route 21 view **after** crossing now
matches a full load visually apart from animation phase. The Route 20 return
no longer shows garbled town art. This repairs the entering frame only.

The departing frame still previews the neighboring map through the current
map's tileset. `FillConnection` copies raw neighbor metatile IDs into the
current map backup, and the renderer resolves them using the current layout.
In Route 21, that means FRLG Cinnabar building IDs are decoded with HNS
Pallet Town secondary art. The manually played mGBA ROM and the saved
`route21-approach.png` both show the garbled building one step before the
crossing. Route 20 has the same underlying issue on its Cinnabar preview.

A temporary Route 21 layout trial using Cinnabar's secondary tileset made
the pre-crossing building look correct (`trial-route21-cinnabar-secondary-approach.png`).
It was reverted: Route 21 also previews Pallet Town at its north edge, whose
secondary metatiles require Pallet Town art. A corresponding Route 20 trial
still showed mismatched shoreline art and was also reverted. Neither trial
changed `map.bin`. A complete visual seam needs compatible metatile graphics
and definitions on both sides, or a context-sensitive rendering scheme that
preserves the other route connections.

SkyEmu traverses both links while Surfing. From Route 21 `(2,99)`, moving down
loads the PoC town at `(2,0)`; from the town `(23,10)`, moving right loads
Route 20 at `(0,10)`. Save/reload preserves the Surf state and coordinates in
both cases. Both packaged ROM/save pairs were booted in fresh SkyEmu processes
and returned to their intended approach positions.

The original mismatched screenshots remain in `artifacts/baseline-*.png` for
comparison. The exact black pixels in the user screenshot were not traced to
individual tile frames, but the corrected SkyEmu views no longer show them.
Both Route 20 and Route 21 were traversed in both directions while Surfing.
The screenshots and playable ROM/save pairs live in the task's `artifacts/`
directory, outside the source worktree. The focused reproduction is
`e2e/src/journeys/cinnabar-seam-poc.e2e.ts`; set `SKYEMU_ROM`, `SKYEMU_SYMS`,
and `CINNABAR_POC_ARTIFACTS` to run it.

Remaining acceptance for the full port includes a fix for the departing-frame
preview on both routes, shoreline inspection in Porymap, and music transitions.
This PoC tests only an empty exterior, so it does not validate town
events, entrances, services, or the full port's assets. Full-view redraw on
map crossings should be checked for frame cost on hardware before general use.

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
linked ROM reports 33,365,812 bytes used of 32 MiB (99.44%). This is the
whole build's footprint, not the PoC's isolated incremental cost. The map
generator accepts the new map, layout, and catalog. The playable E2E ROM also
builds, and four focused SkyEmu checks pass. Porymap inspection remains open.

The major visual blind spot is the primary tileset: the FRLG town uses
`gTileset_General_Frlg`, while both HNS routes use
`gTileset_Kanto_General_Hns`. `LoadMapFromCameraTransition` reloads the
secondary tileset and palette, but not the primary tileset; connection fill
copies raw metatile IDs. Equal edge collision signatures therefore do not
establish a seamless visual transition. For example, both maps use water
metatile 299, but two of its referenced primary tile graphics (indices 65 and
321) differ between the FRLG and HNS files. A runtime fix or compatible map
art must be verified in both directions before the full port can claim a
seamless crossing.

SkyEmu traverses both links while Surfing. From Route 21 `(2,99)`, moving down
loads the PoC town at `(2,0)`; from the town `(23,10)`, moving right loads
Route 20 at `(0,10)`. Save/reload preserves the Surf state and coordinates in
both cases. Both packaged ROM/save pairs were booted in fresh SkyEmu processes
and returned to their intended approach positions.

The visual mismatch is observable. The screenshot taken immediately after the
Route 21 camera crossing renders the town's coast and buildings differently
from a full warp into the same town position. This confirms that coordinate
and collision continuity alone cannot certify a seamless visual transition.
The screenshots and playable ROM/save pairs live in the task's `artifacts/`
directory, outside the source worktree. The focused reproduction is
`e2e/src/journeys/cinnabar-seam-poc.e2e.ts`; set `SKYEMU_ROM`, `SKYEMU_SYMS`,
and `CINNABAR_POC_ARTIFACTS` to run it.

Remaining acceptance for the full port includes Surf traversal in both
directions on both links, shoreline inspection in Porymap, music transitions,
and the final tileset solution. This probe exercises Route 21 → Cinnabar and
Cinnabar → Route 20; the other two directions have not been exercised.

# Content-preserving map layout compression

## Intent

Pokemon Wayfarer needs ROM headroom without deleting or simplifying maps. Map layout
block data is repetitive, immutable in ROM, and already copied before field gameplay,
so it is a suitable build-time compression target.

The feature is successful when it saves at least 1 MiB in the linked Wayfarer ROM while
preserving every authored layout byte and keeping map transitions acceptable in play.

## Product requirements

- Source `map.bin` and `border.bin` files remain unchanged and Porymap-compatible.
- The build independently round-trips every selected layout before linking it.
- Wayfarer may store a layout with GBA LZ77 only when the aligned compressed payload is
  smaller than the aligned raw payload.
- Raw and compressed layouts use one descriptor and one runtime access path.
- The loader owns one reusable scratch allocation for the current map and its connected
  neighbors, then releases it before normal field play.
- Existing tiles, collision, elevation, connections, warps, Secret Bases, Battle
  Pyramid, Trainer Hill, decorations, save behavior, and standalone games remain
  unchanged.
- Raw mode remains available for generator and runtime mechanics tests. A full raw
  Wayfarer ROM is not a release artifact because the linked image exceeds 32 MiB.
- Hybrid is Wayfarer's default storage mode.
- Standalone Emerald, HNS, and FireRed-family builds remain raw.
- A brief hitch while crossing an unfaded connected-map seam is an accepted hybrid-mode
  tradeoff. Faded warps normally conceal the same loading work. Do not add adjacent-map
  preloading, a resident decoded-map cache, or extra lifetime state solely to eliminate
  this minor pause.

## Deliberate non-requirements

The ROM is a trusted, immutable build artifact. This feature does not add runtime CRCs,
verification caches, storage schemas, staged rollout policies, per-layout exceptions,
or a second legacy loader. Those mechanisms add code and state without protecting the
rest of the ROM.

The runtime still rejects impossible descriptor ranges, sizes, and malformed LZ streams
before decoding. This is a small safety boundary around the decompressor, not a general
ROM-integrity system.

## Acceptance

Acceptance requires:

- byte-exact generator round trips;
- raw and compressed copy, rectangle, connection, allocation, and special-consumer
  tests;
- a playable warp/save journey and connected-map seam journey on hybrid;
- at least 1 MiB of net linked-ROM saving.

There is no permanent multi-stage evidence bureaucracy, synthetic timing dossier, or
per-layout numerical gate. A material, player-visible regression in the playable journeys
must be investigated before enabling hybrid by default. The accepted brief hitch at an
otherwise seamless connection is not such a regression; prolonged stalls, broken input,
audio disruption, or incorrect map presentation are.

## Recovery

Release recovery uses the ordinary prior-known-good ROM. It does not maintain a second
map loader or pretend that an over-capacity raw Wayfarer image is deployable. Storage does
not affect save data.

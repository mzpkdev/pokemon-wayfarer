# Map compression feasibility

The 2026-09-12 proof of concept established that independent GBA LZ77 compression of
Wayfarer's map layouts could save about 1.3 MiB while reproducing layout bytes exactly.
It tested 1,089 round trips, three representative runtime fixtures, and one-buffer
lifetime behavior.

The proof was deliberately narrower than the shipping implementation: it did not run a
full playable ROM, connection-heavy journeys, real field heap pressure, or rollback.
Those gaps were later covered by integration tests and playable checks.

## Decision retained from the proof

- Keep authored layout files raw.
- Compress layouts independently so one map load never expands the whole catalog.
- Use GBA LZ77 because the engine already has a compatible decoder.
- Allocate one temporary buffer sized for the largest layout needed by a load.
- Require at least 1 MiB of net linked-ROM saving.

## Complexity deliberately rejected

Early integration work added runtime CRCs, a verification cache, a 28-byte versioned
descriptor, rollout stages, 44 raw exceptions, Git-revision stamps, 100-sample cold/warm
timing, and generated evidence bundles. Those mechanisms were removed after review.

ROM payloads are trusted build outputs; checking only map payload CRCs did not create a
meaningful integrity boundary. The cache and rollout state cost code, latency, tests,
and maintenance without player value. The final design uses one 16-byte descriptor,
build-time byte round trips, bounded runtime range/size/LZ checks, and raw/hybrid modes.

## Current interpretation

The POC numbers justify the product direction, not a permanent performance protocol.
Shipping acceptance comes from linked size, focused mechanics coverage, short
representative timing samples, playable warp/connection journeys, and hybrid-to-raw
save compatibility.

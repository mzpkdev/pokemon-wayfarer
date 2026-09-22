# Compressed map layout validation

PRD: [Content-preserving map layout compression](../prds/compressed-map-layouts.md)
Implemented: Yes

## Scope

This specification defines the permanent tests and the one-time checks required before
hybrid becomes Wayfarer's default. It intentionally does not define rollout stages,
evidence bundles, cache protocols, or long-lived performance thresholds.

## Permanent automated coverage

### Generator and source boundary

- Every selected hybrid candidate round-trips byte-for-byte during generation.
- Generator tests cover raw selection, profitable compression, non-profitable raw
  fallback, descriptor layout, bounds, and compact reporting.
- The source audit rejects direct layout-payload reads outside approved generator and
  runtime owners.

### Runtime mechanics

Focused mechanics tests cover:

- raw and compressed full copies;
- clipped rectangular copies and invalid bounds;
- north, south, east, and west connections with offsets;
- largest-layout scratch sizing, one-allocation reuse, release, and allocation failure;
- malformed/truncated LZ streams before decompression; and
- scoped view lifetime.

Special-consumer tests exercise Battle Pyramid, Trainer Hill, Secret Bases, and
decorations through the opaque API.

### Playable coverage

The hybrid E2E image must pass:

- a representative warp/save journey;
- a connection-heavy journey that crosses and returns across seams; and
- existing gameplay regression journeys relevant to map loading.

## Size and memory checks

Before default enablement, compare the linker's computed raw and hybrid Wayfarer sizes.
Net saving must be at least 1 MiB after descriptors, loader code, and alignment. The raw
link may exceed the 32 MiB output ceiling; its computed region use is still the baseline.

Mechanics telemetry verifies one scratch allocation per field-load context and release
after copying. The generated maximum decoded size must fit the established field heap
budget.

## Recovery

Use the ordinary prior-known-good release if a production regression is discovered. Raw
mode remains a test baseline, not a deployable Wayfarer rollback image. No separate legacy
implementation is maintained, and storage mode does not alter save data.

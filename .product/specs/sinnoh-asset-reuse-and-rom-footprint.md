# Sinnoh asset authority and ROM footprint

PRD: [Sinnoh geography foundation](../prds/sinnoh-exploration-port.md)
Implemented: Yes

## Scope

This specification owns asset reuse rules for the checked-in Sinnoh geography and the
linked-ROM review. It does not require a parallel asset inventory.

## Asset authority

The checked-in layout, tileset, palette, and graphics references are the build authority.
The separate asset manifest, reuse taxonomy, blocker gate, hashes, proof commands, output
sections, consumer lists, and per-map asset references were import evidence and are not
maintained as a second database.

## Reuse rules

An alias must refer to an intentional checked-in canonical asset with compatible runtime
shape and meaning. Approximate visual similarity and matching filenames are insufficient.
Non-aliased Sinnoh variants retain their own symbols so existing Hoenn, HNS, FRLG, or
shared assets cannot change.

Map generation and the compiler verify that referenced paths and symbols exist. The
checked-in compiled assets are the normal build authority.

## Maintenance audit

Full donor comparison is an explicit maintainer action, not a normal object dependency
or `make check` prerequisite. The offline audit may receive a checkout of the pinned
donor and compare catalog membership, IDs, dimensions, properties, topology, and empty
content. Routine builds do not fetch the donor or rehash its complete asset tree.

If an alias or donor baseline changes, review that concrete change. Do not rebuild a
generic provenance framework in anticipation of future drift.

## ROM accounting

Measure the linked Wayfarer image, not PNG sizes or padded ROM length. Sinnoh layout data
uses the common raw/hybrid storage modes and automatic profitable compression. The
catalog has no Sinnoh-specific thresholds, raw exceptions, or rollout stages.

The release must remain within Wayfarer's ROM limit and protected reserve. If it does
not, prefer exact aliases and content-preserving compression. Removing geography or
changing decoded map content requires a separate product decision.

## Validation

- Catalog tests pass and all compiled asset references resolve.
- The default hybrid Wayfarer build links within the ROM limit; the linker's computed
  raw size is retained only as a comparison baseline.
- Existing maps render from unchanged asset owners.
- A release size report confirms the ROM and reserve limits before shipping.

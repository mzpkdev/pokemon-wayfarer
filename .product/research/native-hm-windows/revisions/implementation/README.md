# Native utility catch-window implementation

Implementation base: `f7a2b95b7bde184141cf5b91215dc4e7ab9068a6`.
Approved source baseline: `479b0c83aea4ad90feb0af649e83ccb1a5916770`.
Distribution authority: [nearby-access proposal](../nearby-access/proposal.json).
Assignment SHA-256: `7b87886fd4b1c9bbd9b5fc3400aeed278761ce77f6a0db12b57581f01860eaa8`.

The implementation preserves the approved research snapshots. Production
learnsets contain the selected 121 species and 154 utility roles, with 137
modern additions and 147 legacy additions. Wayfarer excludes the previous
repeated anchor entries and successor reminders; standalone builds retain
their previous tables. The encounter generator applies the 17 selected
replacements to 23 authored time-profile slots behind `IS_WAYFARER`.

## Baseline review

The implementation base has no intervening changes to native learnsets,
species compatibility, evolution data, encounter populations, scaling rules,
Standard Rod data, or the production wild-encounter algorithm. The encounter
generator has a refactor that shares numeric predecessor validation with
Trainer scaling. Those changes are preserved.

The host tests compare the proposal hash with the approved results, preserve
native entries and order, check both learnset limits and evolution-path caps,
and verify HM fallback against the production teachable generator. They also
compare standalone tables with the source baseline.

## Validation commands

Verified on the implementation worktree:

| Check | Result |
| --- | --- |
| Host learnset contract | 8 tests pass |
| Existing encounter suite with replacement regressions | 56 tests pass |
| Wayfarer report regressions | 4 tests pass |
| Generated C fixtures | Both freshness checks pass |
| Production ordered entries and fresh-catch moves | All 121 species, 100 levels, both modes pass |
| Production regional coverage | All 3,888 cells pass |
| Production directional acquisition | All 10,692 cases pass |
| Field-use mechanics | All 18 tests pass |
| Owned retention, evolution, and listed-descendant reminders | Pass |
| All-species learnset/relearner limits in both modes | Pass |
| Blackthorn and Den emulator boundary tests | All 5 tests pass |
| E2E TypeScript | Pass |
| Normal and E2E Wayfarer ROMs | Both build successfully |
| Standalone modern/legacy learnset objects | Emerald, FireRed, LeafGreen, HNS compile |
| Standalone table preservation | All eight baseline hashes match |

Directional cases are split into blocks of nine Ratings to fit the test ROM's
per-case timer. SkyEmu required execution outside the local sandbox; inside it,
the process crashed before becoming ready. The passing emulator run used the
built E2E ROM and matching symbols.

Run from the repository root:

```sh
make -C game BUILD=wayfarer native-hm-windows-test
make -C game BUILD=wayfarer wild-encounter-scaling-test
make -C game -j8 BUILD=wayfarer check TESTS='Wayfarer catch windows'
make -C game -j8 BUILD=wayfarer check TESTS='Wayfarer native HM'
make -C game -j8 BUILD=wayfarer check TESTS='Wayfarer listed descendants'
make -C game -j8 BUILD=wayfarer check TESTS='HM field use'
make -C game -j8 BUILD=wayfarer check TESTS='Wayfarer native catch carriers'
make -C game -j8 BUILD=wayfarer check TESTS='Wayfarer Dive authorization'
make -C game -j8 BUILD=wayfarer
make -C game -j8 e2e
SKYEMU_ROM="$PWD/game/pokemon-wayfarer-e2e.gba" \
  SKYEMU_SYMS="$PWD/game/pokemon-wayfarer-e2e.sym" \
  pnpm --dir e2e exec wa test src/journeys/hns-native-hm-route-boundaries.e2e.ts
```

The production tests compare all four initial move slots at levels 1 through
100 for each selected species in both modes. Coverage tests replay the 3,888
approved regional witnesses and evaluate all 10,692 directional cases through
production slot eligibility, projection, effective species, and initial
moveset creation. They aggregate exact fractions within each source. The
Blackthorn and Den tests require local sources without the Ice Path detour.

The probability model assumes uniform authored-level rolls conditional on a
successful encounter. It excludes modulo bias, lure and lead modifiers, bite
rate, and walking encounter frequency. Old Rod or land sources must meet the
8% floor; Good and Super Rod fishing must retain nonzero coverage.

The [coverage summary](coverage-summary.json) and [full compressed report](coverage.json.gz)
record source identities, carrier probabilities, replacements, and input hashes.
These are a model of production data, labeled separately from C test results.
All cells pass in this model. Old and Good Rod minimum best-source odds are
8%; four Super Rod cases remain at 5%, as in the approved design. The regional
inventory flags optional, late, underwater, and unclassified sources, including
Whirl Islands witnesses. Those regional witnesses do not certify practical
acquisition; Whirl Islands are excluded from the 11 directional scenarios.

Regenerate the model from the repository root:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 game/tools/wild_encounters/wayfarer_native_hm_audit.py \
  --output .product/research/native-hm-windows/revisions/implementation/coverage.json.gz \
  --summary .product/research/native-hm-windows/revisions/implementation/coverage-summary.json
```

## Route and balance acceptance

The emulator route tests use explicit learned-move fixtures because the current
E2E fixture protocol overwrites generated moves. These tests target the
Blackthorn Surf bank and both southwest Den Whirlpool lanes, including the
invisible obstacle, with the HM and badge 8 absent. Production moveset and
probability tests establish different parts of the acceptance contract.

The Trainer scaling inventory is regenerated from the revised learnsets.
Authored custom movesets and scaling policy remain unchanged. The existing
scaling policy continues to generate ordinary Trainer moves from learnsets;
excluded bosses retain their authored construction.
The generated audit evaluates 439,668 slot, Rating, and mode combinations
without structural failures. Of the eligible authored slots, 1,357 use selected
roster species across 852 Trainer IDs; 150 of those slots have authored custom
moves already subject to the existing scaling policy. Changes to generated
moves can displace battle moves, so those encounters remain balance-playtest
targets. The inventory does not count source slots as unique encounters at
every Rating, because scaling may resolve an evolved species to a predecessor.

Full route playthroughs, rod/capture preparation, the optional cleared Ice Path
return journey, and release balance playtesting remain separate acceptance
work. This document does not mark the feature release-ready.

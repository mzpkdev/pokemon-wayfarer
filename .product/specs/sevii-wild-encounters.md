# Sevii wild encounters

PRD: [Sevii exploration port](../prds/sevii-exploration-port.md)
Implemented: No

## Scope

This specification defines the ordinary wild encounter profiles compiled for
the Wayfarer Sevii map port. It covers FRLG source provenance, FireRed and
LeafGreen counterpart resolution, explicit day/night identity, Altering Cave,
Trainer Rating interaction, and validation.

The [map port specification](sevii-exploration-map-port.md) owns map inclusion,
travel, interiors, environmental scripts, and the absence of story and
Trainers. Static, scripted, gift, trade, roaming, and legendary Pokemon are not
ordinary encounter profiles and remain out of scope.

Birth Island and Navel Rock are existing Wayfarer content outside this
encounter port. Do not add, remove, alias, or otherwise modify any encounter or
static Pokemon behavior they already own.

## Behavior

### Encounter ownership

Freeze a separate Sevii encounter manifest at
`game/src/data/wayfarer_sevii_wild_encounters.json`. Do not add Sevii ownership
to the mainland Kanto manifest, whose current contract deliberately excludes
these maps.

The Sevii manifest derives its topology from the FRLG entries already authored
in `game/src/data/wild_encounters.json`. It contains exactly the 58 included map
IDs that have at least one FRLG encounter method. Using the default Altering
Cave population, those maps resolve to 99 map and method profiles:

| Method | Profiles |
| --- | ---: |
| Land | 49 |
| Surf | 20 |
| Rock Smash | 10 |
| Fishing | 20 |

Each profile records the map ID, method, target day label, target night label,
FireRed source label, LeafGreen source label, active slot count, encounter
rate, source-resolution result, and per-slot provenance. The manifest also
records all selected port maps with no encounters so their absence is explicit.

`game/src/data/wild_encounters.json` remains the only authored source of FRLG
species, levels, encounter rates, and slot order. The Sevii manifest names and
validates those rows; it does not copy their Pokemon lists into a second file.

### FireRed and LeafGreen resolution

For each method, pair the same numeric slots from the FireRed and LeafGreen
source labels. Resolve the Wayfarer daytime table with the mainland Kanto
counterpart rules and these Sevii restrictions:

- Keep a shared assignment in its source slot when both versions agree.
- When the versions differ, retain both source species when the fixed target
  slot count and method weights permit it.
- Do not change the method's slot count, weights, encounter rate, or source
  minimum and maximum levels to make room.
- Do not introduce Generation II additions, later-family additions, habitat
  substitutions, or a species absent from both source profiles.
- If every distinct counterpart cannot fit, use the existing deterministic
  exhaustive selection and tie-break order. Record the omitted counterpart and
  proof in the generated audit.
- A retained species uses the exact complete level range from its contributing
  source slot. When both versions author the same species in a paired role with
  different ranges, use the existing lower-midpoint, lower-maximum, FireRed
  tie-break order.

The result is a single version-neutral Wayfarer table. It must be reproducible
from the two frozen source rows without a hand-edited duplicate roster.

### Time of day

Every resolved daytime method has one explicit `DAY_ALIAS` night binding. The
generated `TIME_NIGHT` pointer is the same `WildPokemonInfo` pointer used by
`TIME_DAY`; do not emit a duplicate Pokemon array.

For every profile, day and night therefore have identical:

- encounter rate;
- active slots and slot order;
- species and source provenance;
- minimum and maximum authored levels; and
- land, Surf, Rock Smash, or fishing method data.

Morning and evening use the project's configured `TIME_DAY` fallback. The
audit resolves all four runtime periods and proves that they select the same
Sevii table. Do not depend on a missing-header fallback to create the night
behavior.

### Altering Cave and Tanoby chambers

Use `sSixIslandAlteringCave_FireRed` and
`sSixIslandAlteringCave_LeafGreen` as the only Altering Cave source pair. Do not
compile the numbered `_2` through `_9` event rotations into the Wayfarer Sevii
profile set. The map port leaves the selected wild-set variable at its default
value.

Retain the seven FRLG Tanoby chamber profiles and their existing Unown form
behavior. The Tanoby Key puzzle controls physical chamber availability through
the map port's local completion state, not through an encounter-table rewrite.
Once inside a chamber, ordinary encounters are available at every time of day.

### Wayfarer runtime behavior

Add a `POKEMON_WAYFARER` output product to
`game/tools/wild_encounters/wild_encounters_to_header.py`. It consumes the
Sevii manifest and emits the derived profiles under `IS_WAYFARER`, using the
existing generic FRLG map IDs selected by the map port. FireRed and LeafGreen
source rows keep their existing build guards.

The writer currently iterates only authored rows from
`wild_encounter_groups`. Add a synthetic-profile stage that resolves each
manifest pair, builds a normal in-memory encounter record, and appends it to
the generator's profile and header union as `POKEMON_WAYFARER`. Both array
emission and header emission must iterate that combined authored-plus-derived
sequence. Do not reclassify the original FireRed or LeafGreen labels and do not
write the synthetic records back into `wild_encounters.json`.

Update the generator's runtime header-ID calculation for the new product. The
generated union must preserve the existing product ordering and header IDs for
standalone Emerald, FireRed, LeafGreen, and HNS builds. Add Wayfarer population
and offset assertions so a profile cannot compile under the right guard but
resolve to the wrong header at runtime.

Ordinary Sevii encounters pass through the existing Trainer Rating projection,
predecessor resolution, species floors, Lures, ability attraction, randomizer
handoff, and Standard Rod mechanics. The derived authored table is the input to
those systems. No Sevii-specific rating offset, species floor, rod rule, Lure
rule, or special selection path is added.

Pokedex area data, DexNav, ambient species, and other readers that already use
the effective ordinary population must see these same profiles. A consumer may
not fall back to a FireRed-only row, a LeafGreen-only row, or an unrelated HNS
Kanto row.

Update Cartographer's encounter product union and source-to-projection join in
`devtools/tools/cartographer/src/catalog/encounters.ts` and
`devtools/tools/cartographer/src/catalog/types.ts`. Its tests must prove that
all 99 Wayfarer profiles join to the two frozen source rows and resolve the
same day and night table as the ROM generator.

### Generated audit

Extend the wild encounter balance audit with a separate `sevii` section. It
reports:

- all 58 encounter maps and 99 day profiles;
- one matching `DAY_ALIAS` for every profile and the same 99-profile night
  total;
- the 77 imported maps with no ordinary encounter method, derived as the 135
  imported maps minus the 58 maps with at least one profile;
- FireRed and LeafGreen source labels, source hashes, method, encounter rate,
  slots, species, levels, and counterpart groups;
- deterministic counterpart-resolution proofs and any omission;
- resolved day, morning, evening, and night pointers;
- authored and effective species at every integer Trainer Rating supported by
  production; and
- Altering Cave's default-only result and the seven Tanoby chamber results.

Generation fails on a missing source row, wrong map, source method mismatch,
source drift, wrong slot count, changed encounter rate, unproved counterpart
omission, non-FRLG species, authored night row, missing alias, duplicate target
profile, event Altering Cave row, a reader that resolves a different table, or
any Birth Island or Navel Rock encounter change.

### Delivery plan

1. Add the Sevii manifest parser and fixtures without changing generated ROM
   data.
2. Freeze and validate the 58-map, 99-profile topology against the current
   FRLG rows.
3. Reuse the Kanto counterpart resolver under the stricter no-additions policy
   and emit Wayfarer day profiles.
4. Add explicit night aliases and all-period identity checks.
5. Connect the profiles to the selected maps, Pokedex area data, DexNav, and
   the ordinary runtime scaling path.
6. Generate the audit, run mechanics tests, and sample every method in an
   emulator at low and high Trainer Rating.

### Validation

Add Sevii cases to `game/tools/wild_encounters/tests/test_scaling.py` for
identical and different version pairs, level-range selection, a proved
counterpart omission, default Altering Cave, Tanoby chambers, explicit aliases,
all-period pointer identity, deterministic report ordering, and rejection of a
non-FRLG species.

Required commands:

- `make -C game wild-encounter-scaling-test`
- `make -C game wild-encounter-balance-audit`
- `make -C game wayfarer-sevii-port-audit`
- `make -C game check`
- `make -C game wayfarer`
- `make -C game wayfarer release`

Runtime acceptance samples at least one land, Surf, Rock Smash, and fishing
profile on each island that provides the method, every Tanoby chamber, the
default Altering Cave, and a mixed FireRed/LeafGreen counterpart profile. Run
each sample at day and night and compare the pre-modifier profile identity.
Repeat representative profiles at the minimum and maximum Trainer Rating.

## References

- [Sevii exploration map port](sevii-exploration-map-port.md)
- [Kanto wild encounters](kanto-wild-encounters.md)
- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Wild encounter data](../../game/src/data/wild_encounters.json)
- [Wild encounter generator](../../game/tools/wild_encounters/wild_encounters_to_header.py)
- [Wild encounter runtime](../../game/src/wild_encounter.c)

# Native HM catch-window attachments

This package is attached to the approved [PRD](../../prds/native-hm-catch-windows.md)
and [technical specification](../../specs/native-hm-catch-windows.md). Keep the
files together in the repository. Links are the attachment mechanism; duplicate
copies in the PRD and spec directories are not required.

## Current selected revision

The [nearby-access revision](revisions/nearby-access/README.md) is the current
designer-selected distribution and encounter plan, following the user's request
to audit routes and select a design. It contains 121 species and 154 utility
roles, with code-traced acquisition areas and TR-by-TR simulations. Production
implementation remains pending.

Use its [proposal](revisions/nearby-access/proposal.json),
[effective roster](revisions/nearby-access/roster.md),
[catching locations](revisions/nearby-access/locations.md) and
[nearby itineraries](revisions/nearby-access/itineraries.md) together. Its
[README](revisions/nearby-access/README.md) links all revised machine-readable
reports and reproduction tools. Original files below remain historical evidence.
Whirl Islands are outside the revised access audit.

## Original approved snapshot and authority

The user approved the single-entry design and initial 117-species, 148-role
distribution. The feature is not implemented. Research files retain their
original proposal wording as a record of the investigation; approval status
and implementation requirements live in the PRD and spec.

Base commit: `479b0c83aea4ad90feb0af649e83ccb1a5916770`.
Assignment SHA-256: `8f520676447ebaed84a04a6f8ecee50d05172808b3a80868207ff91121226d86`.

The original effective roster is the approved initial distribution. The research
reports are evidence with stated limits, not proof of complete reachability.
The original Lilycove fishing replacements remain unselected alternatives;
the current revision selects nearby land catches instead. Exploratory
optimization does not override the selected assignments.

## Attachment inventory

| File | Purpose and status |
| --- | --- |
| [README.md](README.md) | Full findings, rationale, tradeoffs, old-design briefing and limitations |
| [roster.md](roster.md) | Every species/utility role, both learning levels and caught-level windows |
| [roster.csv](roster.csv) | Spreadsheet distribution, native/addition status and species-level regional TR windows |
| [roster.json](roster.json) | Effective ordered learnsets, assignments, windows and location inventory |
| [proposal.json](proposal.json) | Fixed authoring inputs; original native levels take precedence |
| [locations.md](locations.md) | Readable catching guide for all 117 species |
| [locations.csv](locations.csv) | Exhaustive 2,298 species/map/method/time records; presence is not a reachability guarantee |
| [baseline.json](baseline.json) | Original learnset and compatibility inventory for 1,333 species, with provenance |
| [coverage.json](coverage.json) | All 3,888 regional mode/utility/TR cells and selected witnesses; filtered table coverage only |
| [fixed_ports.json](fixed_ports.json) | Fixed-roster shore checks, probabilities, low-odds intervals, handovers and simulated Lilycove alternatives |
| [surf_ports.json](surf_ports.json) | Exploratory optimization results and scoped Lilycove search; not approved replacement assignments |
| [extract.py](extract.py) | Baseline extraction without production edits |
| [coverage.py](coverage.py) | Encounter projection, eligibility and devolution model |
| [analyze.py](analyze.py) | Distribution validation and roster/regional report generation |
| [make_locations.py](make_locations.py) | Readable location-guide generation |
| [validate_ports.py](validate_ports.py) | Fixed-proposal shore validation and in-memory encounter alternatives |
| [surf_ports.py](surf_ports.py) | Exploratory Surf level search using canonical insertion ordering |

## Original findings that must survive handoff

- No deleted original native entries, no newly repeated utility entries, and
  no selected species or audited evolution path above two authored utility types.
- All regional table cells covered in both modes, without a complete traversal
  graph or production C acceptance test.
- Modern Lilycove TR 0-1 is uncovered in the original fixed local fishing check;
  covered shore contexts can still have Old Rod carrier odds as low as 2%.
- Marill, Lotad and Psyduck alternatives close the measured Lilycove gap in
  simulation only. They do not resolve every low-odds interval elsewhere.
- Kirlia, Huntail and Gorebyss are not ordinary wild witnesses; Crawdaunt's
  special HNS placement does not certify ordinary regional coverage.
- Native preservation does not preserve every fresh catch's four original
  moves. Evolution preserves known moves but does not guarantee reminder access
  on an unlisted descendant.

## Reproduction and revisions

The [research README](README.md#validation-and-reproduction) gives commands and
model assumptions. When inputs have changed, reproduce in another task worktree
at the same repository-relative path: the scripts derive source paths from
their location and overwrite their generated files. Preserve this approved
snapshot and attach the resulting implementation evidence as a named revision
with its source commit and assignment hash. Retain original findings even when
a later approved change resolves them.

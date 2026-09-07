# Nearby utility acquisition: selected design

Status: selected design revision; production implementation remains pending.

Keep the single-entry distribution, permit short walking detours, and adjust
specific encounter slots where nearby sources cannot sustain useful odds.
Whirl Islands are outside this audit and its access guarantees.

The authoritative inputs are [proposal.json](proposal.json) and
[scenarios.json](scenarios.json). The complete revised distribution covers
121 species and 154 species/utility roles:

- [Every Pokemon, learning level and fresh-catch window](roster.md)
- [Spreadsheet distribution](roster.csv)
- [Catching locations](locations.md) and [exhaustive location rows](locations.csv)
- [Nearby catch itineraries by TR, mode and time](itineraries.md)
- [Probability summary](summary.csv) and [exact results with witnesses](results.json)

The [original research](../../README.md), its 117-species distribution and its
failed shore checks remain intact. This revision replaces those assignments
and selects particular encounter edits; it does not approve every explored
alternative. The [PRD](../../../../prds/native-hm-catch-windows.md) and
[specification](../../../../specs/native-hm-catch-windows.md) define the contract.

## Design decisions

Nearby means a usable grass patch or fishing bank reached without the utility
being sought. A map connection alone is insufficient. A detour through the next
town is acceptable, such as Route 118 west through Mauville to Route 117.
The simulation never adds probabilities from separate places together.

Use at least an 8% chance at one reachable source at every TR from 0 through
80, independently in modern/legacy mode and day/night. For fishing, this is
conditional on a hooked Old Rod encounter, equivalent to 2% per unmodified
cast at a 25% bite rate. For grass, it is conditional on a land encounter;
walking frequency is not modeled. The same 8% acquisition floor applies to
the required Whirlpool source before the Dragon's Den shrine.

The broader roster still has staggered catch windows. It preserves original
native entries and adds each assigned utility once. Species and audited
evolution paths stay within two authored utility types. No special moveset
injection, automatic retention or altered field-move authorization is needed.

### Mainland catches

Wingull gains Surf at 12 and Pelipper at 39 in both modes. Two Wingull slots
on Route 121 become Pelipper, preserving their authored levels and weights.
The eastern grass is reachable from Lilycove; Route 120 is not a safe substitute
because the intervening Aqua blockers may still be present.

Apply the same two-slot Wingull/Pelipper handover on Route 118's east bank.
On the west bank, use the dry Mauville-to-Route-117 detour. Marill learns Surf
at modern 7 / legacy 18; Azumarill gains it at 35 in both modes. One Route 117
Marill slot becomes Azumarill at authored level 20 instead of 13. That level
change closes a projected-level handover gap; it is an explicit exception,
not permission to change encounter levels generally.

### Coastal fishing

Use the existing Krabby/Kingler Surf family for the remaining HNS coastal
gaps. Kingler's Surf entry is modern 44 / legacy 30. The selected city slots
use authored levels 25-35 in Cinnabar and Vermilion, and 35-45 in Cianwood
and Olivine. Underlevel Kingler slots resolve to Krabby through the existing
devolution rule. Their exact zero-based slots and expected original species
are recorded in the proposal; rod weights remain unchanged.

Mossdeep and Pacifidlog each receive three fishing substitutions: slot 0
Magikarp becomes Sharpedo at 5-10, and Wailmer slots 4 and 6 become Kingler
at 10-30 and 30-35. These preserve every original species elsewhere in the
table, both Tentacool slots and three Wailmer slots. Underlevel Sharpedo
resolves to Carvanha. Wailmer itself keeps its native Whirlpool and Dive;
giving it Surf would break the two-role cap.

These island banks provide a new Surf carrier after arrival. They are not
evidence that the islands can first be reached without Surf. This design
assumes a rod and capture supplies; it does not add a guaranteed rescue after
the player deliberately loses every usable carrier.

### Selected encounter edits

Indices are zero-based. HNS rows apply to both day and night; Hoenn uses its
static profile. Every edit preserves the matching slot's rod or land weight.
Only the authored species and the explicitly listed levels change.

| Map | Method and slots | Original species | Replacement | Authored levels |
| --- | --- | --- | --- | --- |
| Mossdeep, Pacifidlog | Fishing 0 | Magikarp | Sharpedo | Preserve 5-10 |
| Mossdeep, Pacifidlog | Fishing 4 | Wailmer | Kingler | Preserve 10-30 |
| Mossdeep, Pacifidlog | Fishing 6 | Wailmer | Kingler | Preserve 30-35 |
| Blackthorn | Fishing 5, 6 | Poliwag | Poliwhirl | Preserve 40 |
| Route 121 | Land 8, 9 | Wingull | Pelipper | Preserve 26 and 27 respectively |
| Route 117 | Land 4 | Marill | Azumarill | Set 20 (original 13) |
| Route 118 | Land 8, 9 | Wingull | Pelipper | Preserve 26 |
| Cinnabar | Fishing 1 | Magikarp | Kingler | Set 25-35 |
| Vermilion | Fishing 1 | Magikarp | Kingler | Set 25-35 |
| Cianwood | Fishing 0 | Magikarp | Kingler | Set 35-45 |
| Olivine | Fishing 1 | Corsola by day; Krabby by night | Kingler | Set 35-45 |

These are 17 map/method/slot replacements. Cinnabar and Vermilion retain
daytime Gyarados in slot 0. Olivine retains its other Corsola slot; Cianwood
retains Magikarp in other slots. This is a limited ecology adjustment chosen
to keep learnsets clean, not a general rebalance of fishing populations.

### Blackthorn and the shrine

Poliwag learns Surf at 10 and gains Whirlpool at 31; Dratini gains Whirlpool
at 6, in both modes.
Poliwhirl learns Whirlpool at 27 in both modes; its Surf entry becomes modern
37 / legacy 30. Promote Blackthorn
fishing slots 5 and 6 from Poliwag to Poliwhirl, retaining authored level 40
and weights. The full roster records Poliwag's selected Surf entry as well.

Validate Surf acquisition outside the Den first. Then, with Surf available,
validate Whirlpool acquisition on the entrance side of the required obstacle.
The shrine is progression-critical: Clair's victory does not award the Rising
Badge; the Elder quiz does. Native Whirlpool can be used without that badge
or the HM item. Lance's HM reward is a fallback, not a required itinerary.
See the [code and terrain trace](blackthorn-routes.md).

Do not delay Whirlpool globally to control Whirl Islands. No island gate or
story timing is changed here. Do not claim that the Den is universally locked
until Clair either: the current entrance script's state checks are narrower.

## What was traced

Native subagents inspected map connections, warps, collision data, object and
coordinate scripts, encounter profiles, species resolution and initial-move
creation. The main designer selected and combined the revisions. A separate
read-only reviewer checked the resulting model and route claims.

| Scenario | Allowed useful sources |
| --- | --- |
| Cianwood Surf | City bank; Cliff Edge Cave land; Route 47 southern beach |
| Olivine Surf | City and outside-port banks; Route 40 beach |
| Vermilion Surf | City and outside-port banks; Route 6 pond |
| Cinnabar Surf | Local bank only |
| Lilycove Surf | City bank; Route 121 eastern grass and reachable bank |
| Mossdeep Surf | Local bank only |
| Pacifidlog Surf | Local bank only |
| Route 118 west Surf | West bank; Route 117 via Mauville |
| Route 118 east Surf | East bank and grass; southern Route 119 grass |
| Blackthorn Surf | City bank; conditional cleared-Ice-Path detour to Route 44 and onward |
| Dragon's Den Whirlpool | Pre-obstacle Den fishing with Surf; Blackthorn; conditional western detour |

The HNS ports' outside banks do not require ship tickets. Route 118's west
bank cannot borrow its east-bank grass. Cinnabar, Mossdeep and Pacifidlog
cannot borrow neighboring water-route encounters as Surf-free catches.

The extended Blackthorn detour needs the full Ice Path chain, not the apparent
direct Route 44 connection. A return trip needs the cleared boulder puzzle.
Cliff Edge Gate's sideways stairs are code-corroborated, but the terrain helper
does not fully reproduce diagonal forced movement. These qualifications remain
part of the route evidence even where a selected local catch makes the detour
unnecessary.

## Evidence and limits

The selected revision passes all 3,564 Old Rod scenario/mode/time/TR cells
at the 8% floor. All 10,692 cells across all three rods have a carrier.
Four Super Rod cells are below 8%: modern TR 35, day and night, at Lilycove
and Route 118 east (5%). This does not waive or lower the Old Rod floor;
it records a remaining rarity dip for the higher-quality rod.

| Starting area | Worst Old Rod or land chance across modes/times/TR | Required detour |
| --- | --- | --- |
| Cianwood | 22% | None |
| Olivine | 20% | None |
| Vermilion | 13% | None |
| Cinnabar | 11% | None |
| Lilycove | 8% | Route 121 at some Ratings |
| Mossdeep / Pacifidlog | 8.14% | None |
| Route 118 west | 10% | Route 117 through Mauville at some Ratings |
| Route 118 east | 8% | None |
| Blackthorn Surf | 9% | None |
| Dragon's Den Whirlpool | 8% using local sources only | None beyond Blackthorn/Den |

These are floors for a qualifying source, not every bank. Den's local daytime
floor is 11%; its nighttime floor is 8% modern and 9% legacy. The broader
scenario also records optional farther sources with better odds, but the
required local solution does not depend on cleared Ice Path or Route 44.

The numerical model enumerates 11 scenarios, two learnset modes, two clock
cases, three rods and 81 Ratings: 10,692 cells per distribution. Hoenn's static
day table is intentionally reused for both clock cases. All utilities are
inserted together before constructing each four-move initial moveset.

It uses exact fractions for uniform authored-level outcomes, production-derived
Rating projection, whole-slot eligibility, devolution and rod weights. RNG
modulo bias, lure/lead effects, capture success, battle safety and elapsed
travel time are not measured. Detour rank orders authored source choices; it
is not a distance in maps or minutes.

The [regional report](coverage.json) checks all 3,888 region/mode/utility/TR
cells. The [family audit](family-audit.json) checks explicit evolution branches
and the union of original and assigned utility types. Regional table coverage
does not prove that every utility is reachable in every story state.

This is static code tracing and an independent Python simulation. No emulator,
E2E playthrough or production C execution was performed for this revision.
Runtime implementation tests remain required by the specification. The full
world location guide also includes optional areas, including Whirl Islands;
listing a habitat does not bring its access timing into this audit's scope.

## Reproduce

Base source commit: `479b0c83aea4ad90feb0af649e83ccb1a5916770`.
The exact proposal and scenario hashes are embedded in `results.json`; the
itinerary header repeats the selected proposal hash. Encounter changes are
applied only to copied in-memory research profiles.

Run from the task repository root:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 .product/research/native-hm-windows/revisions/nearby-access/simulate.py
PYTHONDONTWRITEBYTECODE=1 python3 .product/research/native-hm-windows/revisions/nearby-access/export_revision.py
PYTHONDONTWRITEBYTECODE=1 python3 .product/research/native-hm-windows/revisions/nearby-access/summarize.py
PYTHONDONTWRITEBYTECODE=1 python3 .product/research/native-hm-windows/revisions/nearby-access/blackthorn_routes.py
PYTHONDONTWRITEBYTECODE=1 python3 .product/research/native-hm-windows/revisions/nearby-access/hoenn_routes.py --report
```

`simulate.py --full-profiles` additionally exports every source's full TR matrix.
The default keeps every best and nearest adequate witness, including species
and caught levels. Regenerate into this revision, not over the original snapshot.

## Supporting attachments

- [HNS coastal route trace](hns-coasts.md) and [terrain helper](hns_tiles.py)
- [Hoenn route trace](hoenn-routes.md), [probe](hoenn_routes.py) and [results](hoenn_routes.json)
- [Blackthorn route trace](blackthorn-routes.md) and [probe](blackthorn_routes.py)
- [Island search](island-options.json) and [search script](island_options.py)
- [Exploratory level search](level-options.json) and [script](optimize_levels.py)

Exploratory island probabilities use the earlier fixed Kingler entry. The
selected revision's final probabilities live in `results.json`. Neither search
output overrides the selected proposal. Native learnset preservation does not
mean preserving every fresh catch's original four moves: a new utility still
occupies an ordinary move slot.

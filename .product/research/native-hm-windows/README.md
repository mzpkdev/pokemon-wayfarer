# Native utility catch-window proposal

The design and initial distribution are now approved in the [PRD](../../prds/native-hm-catch-windows.md)
and [specification](../../specs/native-hm-catch-windows.md). This investigation is
preserved as their [attachment package](attachments.md); production implementation
and the documented coverage/access decisions remain outstanding.

Status: approved design, not implemented. Research based on Wayfarer commit `479b0c83aea4ad90feb0af649e83ccb1a5916770`.

## Recommendation

Adopt the wider roster and single-entry approach. Let a utility disappear naturally from a species' freshly generated wild movesets, then let another species cover that level range. Players who already caught and kept the move do not need to replace their Pokemon.

The proposed distribution has **117 species and 148 species/utility roles**. Of those species, 86 have one utility and 31 have two. The cap includes original native utilities, not just additions. This is a first balance proposal with measured coverage, not a claim that every assignment is essential.

Use these documents together:

- [Full distribution: every Pokemon, utility, learning level and catch window](roster.md)
- [Readable catching locations by Pokemon](locations.md)
- [Spreadsheet: distribution and species-level regional TR windows](roster.csv)
- [Exhaustive map/method/time inventory](locations.csv)
- [Editable level assignments](proposal.json)
- [Regional coverage evidence and witnesses for every TR](coverage.json)
- [Fixed-proposal shore-fishing evidence, odds and optional Lilycove changes](fixed_ports.json)

The distribution covers Wayfarer's combined HNS and Hoenn content. Kanto means the Kanto maps inside HNS, not the separate FireRed/LeafGreen build. Modern mode uses Generation 7 learnsets; legacy uses Generation 3. Fly remains outside this proposal.

## What the current design does

The existing feature concentrates additions in 19 HNS/Hoenn anchors, then repeats them whenever subsequent native moves would displace them from a fresh catch's four moves. The [existing specification](../../specs/native-hm-learnsets.md) contains all original schedules and anchor locations; its standalone FRLG rows are not the active Wayfarer Kanto roster.

| Content | Existing feature anchors and assigned roles |
| --- | --- |
| HNS | Gligar: Cut; Aipom: Cut/Rock Smash; Chinchou: Flash/Surf/Whirlpool; Mareep: Flash; Wooper: Surf/Waterfall; Snubbull: Strength; Miltank: Strength/Rock Smash; Marill: Waterfall; Mantine: Whirlpool |
| Hoenn | Corphish: Cut/Rock Smash; Sableye: Cut/Flash; Electrike: Flash; Lotad: Surf; Wailmer: Surf/Dive; Makuhita: Strength; Torkoal: Strength; Aron: Rock Smash; Barboach: Waterfall; Carvanha: Waterfall/Dive |

Those are feature assignments, not the entire original native utility inventory. Wailmer already has Whirlpool and Dive in its modern baseline, so assigning Surf gives it three utility types. Chinchou's existing modern injection also suppresses Charge at level 50. The research baseline removes the old feature additions and restores that Charge entry.

Normal wild creation considers entries up to the caught level, skips level-zero evolution entries, and keeps the last four distinct moves. Repeating a move while it is still known does not refresh its position. This is why retention must be simulated, not estimated as a fixed number of levels after learning.

Field use already distinguishes knowing a move from owning its HM. A known utility can qualify without its HM item. Once the HM is owned, compatible party Pokemon can qualify without spending a battle-move slot on it. Dive still has its separate authorization gate: Steven's grant in Hoenn and the HNS seventh-badge condition. This proposal does not change those rules.

## Distribution rules

1. Add each assigned utility once per learnset mode. Keep an existing native occurrence at its original level instead of adding another.
2. Keep every original native move, level and relative order. Insert a new utility after original moves at that level. For multiple new utilities at one level, use the move-section order in `proposal.json`.
3. Count all eight utility types toward a maximum of two authored native roles per species. Evolution paths must not accumulate a third role through inherited and successor-native moves.
4. Prefer compatible, ordinarily catchable species. Label special-area and evolution-only options instead of counting them as routine acquisition routes.
5. Give fresh catches overlapping utility windows across TR. Do not require each carrier to retain its utility forever.
6. Validate both learnset modes independently. Different native move cadences sometimes require different insertion levels, particularly for Surf.

Preserving native learnset entries does **not** preserve every fresh catch's original four moves. An ordinary utility move still occupies one of four slots and can push another move out of that particular generated moveset. The distinction is that the native move remains in the learnset, at its original level, and the utility is not repeatedly injected to keep displacing later moves.

The two-role cap concerns this authored distribution. It does not prohibit players from teaching, breeding or otherwise assembling more than two utilities themselves. The proposal introduces no runtime restriction on player movesets.

## Roster shape

Counts include preserved native roles. The same Pokemon can appear in two rows.

| Utility | Species | Distribution intent and examples |
| --- | ---: | --- |
| Cut | 22 | Early small cutters such as Rattata and Zigzagoon; middle windows on Paras, Sandshrew and Aipom; later Gligar, Sandslash, Scyther and Absol |
| Flash | 23 | Early Mareep, Ledyba and Ralts; middle Electrike, Magnemite and Voltorb; later Girafarig, Stantler, Magneton and Electrode |
| Surf | 27 | Land-catchable starters plus staggered shore fish; Tentacool, Staryu and the Carvanha family handle the constrained Hoenn fishing pools |
| Strength | 19 | Early Makuhita, Geodude and Machop; middle Snubbull, Miltank and Onix; later Graveler, Ursaring, Donphan and Torkoal |
| Rock Smash | 19 | Early Geodude, Phanpy, Aron and Poochyena; middle Teddiursa, Dunsparce and Heracross; later Sandshrew, Miltank and Hariyama |
| Waterfall | 10 | Shorter early windows on Remoraid and Barboach; middle Wooper and Whiscash; native Goldeen/Seaking and later Luvdisc |
| Whirlpool | 13 | Early Chinchou; intermediate Corsola, Qwilfish and Mantine; late Tentacool and Lanturn; preserve native Wailmer and clam-family occurrences |
| Dive | 15 | Early Goldeen; middle Psyduck, Barboach and Golduck; later Seaking, Wailmer, Whiscash and Spheal; preserve native specialists |

These examples describe learnset pacing, not a mandatory route order. Catch levels depend on the authored slot and TR. The full table supplies exact windows.

Some representative modern-mode rows show the intended behavior:

| Pokemon | Proposed learn level | Fresh catch knows the move at levels | Example source locations |
| --- | --- | --- | --- |
| Geodude | Rock Smash 5; Strength 12 | Rock Smash 5-11; Strength 12-23 | Johto Route 46/Dark Cave; Hoenn Granite Cave |
| Miltank | Strength 25; Rock Smash 45 | Strength 25-44; Rock Smash 45-100 | Johto Routes 38 and 39 |
| Psyduck | Surf 10; Dive 22 | Surf 10-21; Dive 22-33 | HNS fishing/water tables; see location guide for maps |
| Wooper | Waterfall 22; Surf 37 | Waterfall 22-36; Surf 37-100 | Johto Route 32 and Ruins of Alph |
| Tentacool | Surf 16; Whirlpool 45 | Surf 16-27; Whirlpool 45-100 | Hoenn coastal fishing, including Lilycove |
| Staryu | Surf 42 | Surf 42-100 | Lilycove; Cianwood night fishing |
| Goldeen | Dive 5; native Waterfall 32 | Dive 5-20; Waterfall 32-100 | Freshwater fishing across all three regions |
| Wailmer | Native Whirlpool 13; native Dive 41 | Whirlpool 13-24; Dive 41-100 | Lilycove, Mossdeep and Pacifidlog fishing |

Wailmer loses the feature's Surf assignment, not either native move. Chinchou keeps Surf and Whirlpool but loses the feature's Flash assignment. Electric and luminous species take over Flash. Barboach/Whiscash use Waterfall and Dive; Psyduck/Golduck use Surf and Dive. Keeping those family roles consistent avoids introducing a third utility through evolution.

## Measured coverage and its limits

The regional model finds at least one qualifying encounter profile for **every utility at every integer TR from 0 through 80**, separately in Johto, HNS Kanto and Hoenn, in both modes. That is 3,888 covered region/mode/move/TR cells.

This is an existence check over encounter tables, choosing the best available profile, time and rod. It is **not** proof that every carrier can be reached from every starting position before the relevant obstacle. The filter excludes several known optional/late locations, but it is not a traversal-graph audit. Surf witnesses exclude water encounters and Rock Smash tables; fishing still requires a rod and an accessible bank. Other utilities may have witnesses that require earlier traversal. Some Dive witnesses are deep in caves, which matters less while Dive itself remains story-authorized, but still needs route review.

TR is not caught level. Projection retains part of the authored level spread even at TR 80. Underlevel evolved slots can resolve to predecessors, so a low-TR Sharpedo slot can provide Carvanha. High-level base-species slots do not automatically evolve. The model uses those rules, rather than assuming every late encounter is a fully evolved Pokemon.

Three roster species have no ordinary modeled wild outcome: **Kirlia, Huntail and Gorebyss**. They are evolution/special options, not coverage witnesses. Crawdaunt has only special/unclassified HNS outcomes in this inventory, not an ordinary Johto/Kanto/Hoenn witness. The location guide labels these cases.

### Local Surf check

The fixed-proposal check covers twelve map/time contexts and three rod qualities at all 81 TR values: 2,916 cases per mode. It includes Cianwood, Cinnabar, Vermilion city, Vermilion port, Route 118, Lilycove, Mossdeep and Pacifidlog. HNS day/night profiles are separate; Hoenn's tables use their available day context.

| Scope | Modern mode | Legacy mode |
| --- | --- | --- |
| All tested HNS shore contexts, including Cianwood night | Covered at TR 0-80 | Covered at TR 0-80 |
| Route 118, Mossdeep and Pacifidlog fishing tables | Covered at TR 0-80 | Covered at TR 0-80 |
| Lilycove fishing table | Gap at TR 0-1; covered at TR 2-80 | Covered at TR 0-80 |

At Lilycove, the modern Surf handover is Tentacool, learned at 16 and present on catches at levels 16-27, followed by Staryu, learned at 42 and present at levels 42-100. In that particular fishing table their measured TR windows are **2-36** and **36-80**. Legacy uses Tentacool 6 and Staryu 33, with local TR windows **0-37** and **25-80**.

The exploratory search found at most 79 of 81 Lilycove TR values covered by varying those two Surf insertion levels with the other assignments fixed. That is a scoped result, not a proof that every possible compliant roster is impossible. The fixed proposal's gap is local to the Lilycove fishing table, not proof of an overall world softlock.

Existence alone is too weak as a shipping target. Some covered contexts drop to **2% of hooked encounters** with an Old Rod. Those are not per-cast odds, and casts can fail to hook anything. I recommend an initial tuning target of at least **8% conditional carrier chance at critical shores**, followed by playtesting. This proposal does not yet meet that target everywhere.

### Optional Lilycove encounter adjustment

No encounter changes are included in the main proposal. The validator separately tested replacing the third Lilycove fishing slot (zero-based slot 2), currently Magikarp at authored levels 10-30, while retaining its levels and rod weights.

| Replacement | Modern TR 0-1 Surf chance: Old / Good / Super Rod | Effect |
| --- | --- | --- |
| Marill | 7.62% / 9.14% / 8.38% | Closes the gap in both modes; just below the suggested 8% Old Rod target |
| Lotad | 10% / 12% / 11% | Closes the gap in both modes; reaches 8% at TR 0-1, but the species is a weaker coastal fit |
| Psyduck | 6.19% / 7.43% / 6.81% | Closes the gap in both modes; lower early coverage than the other options |

My preference is Marill if this is a coastal catch, with a small subsequent weight review. Lotad fits better if the accessible fishing point can plausibly be a freshwater pool. These changes solve the early gap, not all late-game rarity issues. The exact proposals and probabilities are in `fixed_ports.json`; all tests modify profiles in memory only.

## Evolution, relearning and player clarity

Evolution keeps already-known moves. Do not add blanket level-one utility copies to descendants just for reminder access; that recreates duplicate entries when the descendant has its own later utility level. Listed evolved species receive their own single assignment. Unlisted descendants retain their original learnsets, so an inherited utility can become unavailable to their Move Reminder after it is forgotten. If reminder continuity is desired, assign that descendant one deliberate level and recheck the family, rather than adding a second occurrence.

The cap audit examined explicit evolution paths, including unselected ancestors and successors. No path touching the selected roster accumulates more than two baseline/proposed utility types. Existing unrelated baseline duplicate utility entries elsewhere in the species inventory remain untouched; the guarantee is no newly introduced repeats, not a rewrite of all vanilla learnsets.

A player-facing habitat hint should name the move, place and relevant catch-level range. For example: "Young Tentacool here may know Surf. At higher levels, look for Staryu." Do not present an unqualified "Tentacool has Surf" tip. Exact local TR ranges are useful for internal validation, but need not become player-facing arithmetic.

## Validation and reproduction

All additions passed compatibility checks, the two-role cap, native-entry/order preservation, one occurrence per added utility, and a 40-entry learnset ceiling. The baseline contains 1,333 species; the encounter model contains 1,422 active HNS/Hoenn method/time/rod profiles. The proposal adds 131 entries in modern mode and 141 in legacy; the remaining roles already occur natively.

Compatibility comes from the repository's historical learnable-move union plus natural learnset moves. It is a species-compatibility criterion, not a claim that each new assignment was a Generation 3 TM. Production implementation should also verify generated runtime teachable tables wherever HM-item fallback is expected.

The scripts model production behavior in Python and reuse the encounter generator's projection/eligibility helpers. Probabilities are exact fractions under a uniform level-roll model; they omit small runtime modulo bias and all lure/lead modifiers. No C runtime test, ROM build or emulator playthrough was performed for this design-only task.

From the repository worktree:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 .product/research/native-hm-windows/extract.py
PYTHONDONTWRITEBYTECODE=1 python3 .product/research/native-hm-windows/analyze.py
PYTHONDONTWRITEBYTECODE=1 python3 .product/research/native-hm-windows/make_locations.py
PYTHONDONTWRITEBYTECODE=1 python3 .product/research/native-hm-windows/validate_ports.py
```

`surf_ports.py` is exploratory optimization, not the approved assignment source. `proposal.json` defines the fixed design; `fixed_ports.json` validates that file and records its SHA-256. Rerun the checks whenever a utility level, native move, evolution rule or encounter slot changes.

Before implementation acceptance, review critical banks/obstacles for actual reachability and tune their encounter odds. The approved specification replaces the old native-HM requirements and retains Standard Rod's 8% Old Rod accessibility minimum as an aggregate known-Surf-carrier shipping gate. Production work must replace the old tests, add production-path catch-window/TR tests, and playtest the crossings in both modes. Do not retain the old every-anchor-at-every-level assertion alongside this design; changing that requirement is the point of the proposal.

## Source references

- [Build content flags](../../../game/include/constants/global.h) and [active learnset generation](../../../game/include/config/pokemon.h)
- [Modern native learnsets](../../../game/src/data/pokemon/level_up_learnsets/gen_7.h), [legacy native learnsets](../../../game/src/data/pokemon/level_up_learnsets/gen_3.h), and [species compatibility](../../../game/src/data/pokemon/all_learnables.json)
- [Initial moveset creation and mode selection](../../../game/src/pokemon.c)
- [Wild encounter tables](../../../game/src/data/wild_encounters.json), [TR scaling data](../../../game/src/data/wild_encounter_scaling.json), and [runtime projection/devolution](../../../game/src/wild_encounter.c)
- [Field-move eligibility](../../../game/src/field_move.c), [Hoenn content authorization](../../specs/wayfarer-hoenn-content-port.md), and [Standard Rod rules](../../specs/standard-rod-fishing.md)

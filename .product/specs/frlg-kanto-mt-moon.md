# Mount Moon fossils on HNS

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)  
Dependencies: [FRLG Kanto independent story beats](../prds/frlg-kanto-independent-story-beats.md), [trainer party scaling](trainer-party-scaling.md), [trainer-only story encounters](trainer-only-story-encounters.md), [HNS open-world traversal](hns-open-world-region-traversal.md)  
Implemented: No

## Scope

Use only the selected HNS cave, `MAP_MT_MOON_CAVE_HNS`. Add the four authored
FRLG Rocket grunts, Super Nerd Miguel, and one local choice between the Helix
and Dome Fossils. This is a local cave adventure. It does not require or advance
Celadon, Silph, the Tower, a Gym, Kanto origin, a badge, or another Rocket story.
The four grunts are optional. Miguel is an optional side interaction and is the
only battle that opens the local fossil choice.

The imported scenes must leave every existing HNS cave exit open. They must also
leave Silver, all existing ambient Pokemon, the hidden Revive, the sign, the
existing inert fossil tableau, `MtMoon_Outside_hns`, and `MtMoon_Shop_hns`
unchanged. On transition, the cave continues to advance
`VAR_PEWTER_CITY_STATE` only from 0 to 1. Silver remains at `(9, 11)` with its
current Yes/No interaction: declining, losing, crossing the cave, and leaving
keep state 1 and Silver visible; only Silver's own win removes him and sets
state 2. The Mt. Moon adventure must neither run nor suppress that writer.

This does not import an FRLG map, layout, tileset, connection, warp, collision
change, or `map.bin`. `MtMoon_B2F_Frlg` remains source evidence only. Its
source coordinate trigger and its isolated topology do not apply to the HNS
cave.

## Placement and visual inspection

The HNS cave's current fossil tableau at `(15..17, 3..5)` is in a separate
83-tile collision component. It cannot host an accessible scene without
changing geometry, which is outside this milestone.

The following positions are in the main 689-tile collision component. They
were checked in the current E2E emulator ROM and by collision/elevation/route
audit. Porymap is preferred when available. With no Porymap executable, this
emulator and map-render inspection approves schema-valid manual `map.json`
event edits, because this milestone makes no tile, collision, elevation, or
`map.bin` change.

| Local ID | Actor | Object tile | Interaction or sight | Placement role |
| --- | --- | --- | --- | --- |
| `LOCALID_MTMOON_ROCKET_GRUNT_1` (2) | Rocket 1 | `(10, 14)` | faces down; sight 1; approach `(10, 15)` | West side encounter |
| `LOCALID_MTMOON_ROCKET_GRUNT_2` (3) | Rocket 2 | `(38, 15)` | faces down; sight 1; approach `(38, 16)` | East side encounter, outside Sandshrew's wander envelope |
| `LOCALID_MTMOON_ROCKET_GRUNT_3` (4) | Rocket 3 | `(44, 27)` | faces down; sight 1; approach `(44, 28)` | Southeast side encounter |
| `LOCALID_MTMOON_ROCKET_GRUNT_4` (5) | Rocket 4 | `(33, 15)` | faces down; sight 1; approach `(33, 16)` | Central-east side encounter |
| `LOCALID_MTMOON_MIGUEL` (6) | Miguel | `(25, 15)` | faces down; approach `(25, 16)` | Central fossil scene |
| `LOCALID_MTMOON_DOME_FOSSIL` (7) | Dome Fossil | `(24, 15)` | approach `(24, 16)` | Miguel's local choice |
| `LOCALID_MTMOON_HELIX_FOSSIL` (8) | Helix Fossil | `(26, 15)` | approach `(26, 16)` | Miguel's local choice |

All listed tiles and approaches are collision-passable at elevation zero. With
all seven object tiles treated as impassable, every listed approach and the
Route 4 exits `(46,31)` and `(4,12)` and both Mt. Moon Square exits `(24,10)`
and `(32,21)` remain in one component. Each new object has a route around it;
the imported side encounters do not form a cave gate. The rejected east spur
near `(46,16)` remains occupied by the existing Sandshrew, whose `(4,4)` wander
range overlaps its approaches; leave that HNS actor and the spur untouched.

The recorded inspection must confirm all seven object tiles, player approach
tiles, movement directions, sight lines, elevation, visual context, and a route
around every encounter. It must also prove that the Route 4 exits `(46,31)` and
`(4,12)`, the two Mt. Moon Square exits `(24,10)` and `(32,21)`, Silver's
interaction, and all existing map events remain reachable from each other. Do
not substitute a collision-only candidate or move the existing tableau. This is
a technical placement decision, not an unresolved product choice.

The cave already has 15 object-event templates, while `OBJECT_EVENTS_COUNT` is
16 and the player occupies one event slot. `TrySpawnObjectEvents` is viewport
driven and silently stops when no slot is free. For every camera viewport that
can contain a new object, the inspection and static test must count active,
unhidden, non-player templates and prove the count is at most 15. This count
includes Silver and every retained ambient actor under its actual time flags.
No placement may rely on template order to make an imported actor appear.

## Adventure behavior

Each placed Rocket object uses `OBJ_EVENT_GFX_ROCKET_M`, which the preceding
Celadon Hideout milestone already makes available in Wayfarer. It preserves the
four authored FRLG parties and the four encounter, defeat, and repeat dialogue
sets. The grunts have normal trainer sight only on optional side routes. They
never gate Miguel, fossils, a warp, or an exit. Defeating one records only that
ordinary Trainer's own defeat state and replaces its battle text with its
post-battle dialogue.

Miguel uses the available `OBJ_EVENT_GFX_SCIENTIST_M_HNS` overworld adaptation
and his authored FRLG Super Nerd party. He does not use the unreachable HNS
scientist/fossil tableau. Before he stages a battle, the canonical usable-party
predicate must refuse with `NP_FOSSIL` if the player has no non-Egg Pokemon with
HP. That refusal does not lock the player, start a battle, set a trainer flag,
reveal a fossil, or change a map state.

With a usable party, Miguel's interaction starts his battle. Only
`B_OUTCOME_WON` reveals the local choice. A loss, forfeit, or other non-win
cannot reveal either fossil, set a local choice state, remove an object, or run
the success dialogue. It follows the current ordinary battle recovery policy
until this caller has an explicitly approved field-return adapter. After normal
recovery, Miguel remains retryable. Winning leaves Miguel present for the local
choice and completed dialogue.

The local choice is independent of all four grunt wins. It offers exactly
`HELIX FOSSIL`, `DOME FOSSIL`, and `EXIT`, using a new appended
`MULTI_MT_MOON_FOSSIL_WAYFARER = 182` entry after the current menu ID 181. It must not
reuse `MULTI_FOSSIL_HNS`, which offers unrelated HNS fossils. Cancel or Exit
changes nothing.

The new menu constant, its three-choice list, and its `sMultichoiceLists`
record are all within `#if IS_WAYFARER`, following the existing Anne menu
pattern. Standalone HNS preprocessing and generated menu output remain
identical; this story-local menu is not linked into HNS just because the cave
has an HNS map source.

Selecting a fossil first checks whether the player already owns that exact item.
If not, it checks the normal Items pocket, adds the selected item, and verifies
that the add succeeded. A full Items pocket leaves both fossil objects and the
choice available. After either a successful new grant or an in-place
reconciliation of the selected pre-owned fossil, it hides both local fossil
objects and finalizes this local choice. Pre-owned Helix or Dome fossils alone
do not reveal fossils, defeat Miguel, or consume the local choice. They only
avoid a duplicate when the player makes this local selection. A fossil held from
the local choice does not change any future reward state.

The HNS Ruins of Alph rewards remain independent. Its Dome Fossil at `(21,4)`,
Helix Fossil at `(5,21)`, puzzle state, lab service, and flags remain unchanged.
The local Mt. Moon choice neither grants nor consumes those rewards. There is no
contract conflict: the story PRD requires a local one-of-two reward, while the
preserved HNS rewards have separate cause and state. The restored Cinnabar
revival service belongs to the later Cinnabar milestone and is not a prerequisite
for taking or preserving this reward.

## Persistent state

Use only these new Wayfarer-local flags in the reserved HNS content window:

| Flag | Name | Meaning |
| ---: | --- | --- |
| `0x4BF` | `FLAG_HIDE_MT_MOON_DOME_FOSSIL_HNS` | Hides the local Dome Fossil event. |
| `0x4C0` | `FLAG_HIDE_MT_MOON_HELIX_FOSSIL_HNS` | Hides the local Helix Fossil event. |
| `0x4C1` | `FLAG_GOT_FOSSIL_FROM_MT_MOON_HNS` | The player finalized this local choice. |

No new saved variable or save-block field is needed. On load, Wayfarer must hide
both fossil events until Miguel has actually been defeated and must restore their
visibility only while the choice is pending. A successful local selection hides
both events and sets `0x4C1` only after its selected-item transaction succeeds
or reconciles. Re-entry and save/reload must derive identical visibility from
Miguel's Trainer flag and these three flags.

On Miguel's winning battle outcome, clear both local hide flags and immediately
show both local fossil objects after the battle returns to the already-loaded
cave. A `clearflag` alone only affects a future map load because object spawning
has already occurred. On a successful selection, hide both live objects before
or with setting the corresponding hide flags and `0x4C1`. The immediate
post-battle test must interact with a visible fossil before any warp or reload.

Do not use, rename, or set `FLAG_HIDE_DOME_FOSSIL`,
`FLAG_HIDE_HELIX_FOSSIL`, `FLAG_GOT_DOME_FOSSIL`,
`FLAG_GOT_HELIX_FOSSIL`, `FLAG_GOT_FOSSIL_FROM_MT_MOON`, or
`VAR_MAP_SCENE_MT_MOON_B2F`: each source constant resolves to zero or belongs
to a different source build. Preserve existing HNS fossil variables
`VAR_FOSSIL_RESURRECTION_STATE` and `VAR_WHICH_FOSSIL_REVIVED` and all Ruins of
Alph flags.

## Trainers, data, and battle routing

Add five Wayfarer-only entries to `trainers_wayfarer.party`, preserving every
source roster, class, portrait, AI, music, and authored moves. Append compact
HNS slots 690 through 694 after the existing Hideout slot 689. With
`HNS_POSTOBC_TRAINER_ID`, they are runtime IDs 1544 through 1548.

| Compact slot / runtime ID | New symbol | Source trainer | Exact normal roster | Scaling policy |
| --- | --- | --- | --- | --- |
| 690 / 1544 | `TRAINER_MT_MOON_ROCKET_GRUNT_1_HNS` | `TRAINER_TEAM_ROCKET_GRUNT` (251) | Rattata 13, Zubat 13 | `ORDINARY` |
| 691 / 1545 | `TRAINER_MT_MOON_ROCKET_GRUNT_2_HNS` | `TRAINER_TEAM_ROCKET_GRUNT_2` (252) | Sandshrew 11, Rattata 11, Zubat 11 | `ORDINARY` |
| 692 / 1546 | `TRAINER_MT_MOON_ROCKET_GRUNT_3_HNS` | `TRAINER_TEAM_ROCKET_GRUNT_3` (253) | Zubat 11, Ekans 11 | `ORDINARY` |
| 693 / 1547 | `TRAINER_MT_MOON_ROCKET_GRUNT_4_HNS` | `TRAINER_TEAM_ROCKET_GRUNT_4` (254) | Rattata 13, Sandshrew 13 | `ORDINARY` |
| 694 / 1548 | `TRAINER_MT_MOON_MIGUEL_HNS` | `TRAINER_SUPER_NERD_MIGUEL` (79) | Grimer 12, Voltorb 12, Koffing 12 | `EXCLUDED` |

The Rocket grunts are eligible ordinary scripted Trainers. Miguel is a local
objective boss, so he remains authored and excluded from ordinary scaling.
Set `TRAINERS_COUNT_MT_MOON_WAYFARER` to 5 and extend the Wayfarer count to
1549:

```text
TRAINERS_COUNT_WAYFARER = TRAINERS_COUNT_HNS + TRAINERS_COUNT_EMERALD - 1
                        + TRAINERS_COUNT_SS_ANNE_WAYFARER
                        + TRAINERS_COUNT_CELADON_HIDEOUT_WAYFARER
                        + TRAINERS_COUNT_MT_MOON_WAYFARER
```

Standalone HNS retains `TRAINERS_COUNT_HNS = 661`. The compact slots use the
existing appended-HNS defeat storage and must not enter Hoenn's fixed 639 through
1492 ID bank. Extend the exact count/range assertions, trainer-state tests,
trainer source-party parity fixture, and the reviewed scaling manifest. The
FRLG Rocket class and portrait are already present from Hideout; Miguel's FRLG
trainer class and portrait are already global. No map graphics import is needed.

For direct grunt interaction, run the usable-party check before `trainerbattle`.
Because a normal-sight script probe stops at that check, extend the existing
narrow Wayfarer story-trainer resolver with only these four exact script-start to
trainerbattle-label pairs. Do not broaden the generic script parser. A defeated
sight grunt must remain suppressed, an undefeated sight grunt with no usable
party must not approach, and direct no-party talk must show the ordinary
refusal without an empty-party fallback. Miguel has no sight route and uses the
same predicate directly with `NP_FOSSIL`.

## Implementation and validation

All new map objects and event scripts must be `wayfarer_only` or enclosed by
`IS_WAYFARER`. Standalone HNS event output and the edited HNS scripts must
preprocess identically to `HEAD`; standalone FRLG continues to compile its
source B2F script and event data unchanged. Standalone HNS preprocessing and
generated output for both edited menu files must also be identical to `HEAD`.
Do not alter any map binary.

Once the placement inspection approves the five placements, static coverage must prove the exact
objects, graphics, scripts, flags, local IDs, collision and elevation of their
approach tiles, all four exits, Silver, the hidden Revive, sign, and retained
HNS object inventory. It must prove the HNS cave's `map.bin` hash stays intact,
the isolated tableau remains untouched, no Wayfarer objects leak into standalone
HNS output, and all three local flags have their Wayfarer values.

Native and generator coverage must prove all five numeric IDs, compact-to-runtime
translation, five exact parties, grunts' `ORDINARY` classification, Miguel's
`EXCLUDED` classification, and independent defeat flags. It must cover all four
resolver mappings, normal sight battle, completed sight suppression, sight and
direct no-party refusal, Miguel refusal, victory, non-win retry, save/reload,
full Items-pocket retry, pre-owned selected-fossil reconciliation, Exit, and
both local fossil choices. It must verify that neither local state nor pre-owned
fossils changes the Ruins of Alph rewards, flags, or fossil-lab interactions.

Emulator coverage must cross every HNS cave exit in both directions with none,
some, and all local encounters complete, then repeat Silver decline, loss, and
win. It must complete each grunt independently, reveal the choice only after
Miguel's win, prove both fossils are immediately visible and interactable before
any map transition, take each local fossil on separate saves, and prove no Gym,
badge, Celadon, Silph, Tower, or Kanto-origin state is required. Run the focused
map, trainer, and story tests plus Anne, Hideout, and Safari regressions. Obtain
a native critic review before the coordinated release build.

# Celadon Rocket Hideout on Wayfarer

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)

Dependencies: [Independent story beats](../prds/frlg-kanto-independent-story-beats.md), [trainer scaling](trainer-party-scaling.md), [story encounter policies](trainer-only-story-encounters.md), [runtime foundation](wayfarer-runtime-foundation.md)

Implemented: Yes

Implemented on `task/frlg-kanto-story-implementation` in [PR #86](https://github.com/mzpkdev/pokemon-wayfarer/pull/86); not yet merged or released. See the [milestone index](frlg-kanto-story-milestones.md) for the bounded delivery scope.

## Scope and integration

Import the five FRLG Rocket Hideout interiors: B1F, B2F, B3F, B4F and
Elevator. Their composite map IDs remain group 66, slots 42 through 46.
Select their maps and layouts explicitly using the existing Wayfarer opt-in.
Preserve authored interior geometry, arrows, barriers, ordinary rewards and
inter-room warps. Do not edit `map.bin`. Mahogany's HNS hideout remains separate.

Connect the adventure to the existing HNS Celadon Game Corner (group 17,
slot 9). Append a Wayfarer-only concealed wall-panel interaction at `(11, 1)`, reachable
from `(11, 2)`, and the source entrance grunt at the audited free position
`(10, 2)`. Preserve all existing NPCs, machine
interactions and shop services, including the Lass at `(12, 2)`. Add event
selection metadata to the generator so standalone HNS receives neither event.

The player's actual victory over the entrance grunt makes the concealed wall panel usable.
Interacting with the panel then opens the local entrance and offers direct entry at B1F
`(12, 2)`. Subsequent visits remain possible. The source B1F exterior exit
returns to the audited Game Corner approach tile `(11, 2)`. Its authored
`MB_UP_LEFT_STAIR_WARP` at `(12, 2)` exits on west input. Use a Wayfarer
warp override and dynamic destination, initialized on B1F load, rather than
importing the FRLG Game Corner or creating stairs by changing HNS tiles.
Validate exit, elevator travel and save/reload together because they share the
dynamic-warp facility.

## Local progression

No unrelated badge, Mt. Moon, Silph, Tower or rival state gates discovery,
entry, ordinary exploration or completion. Preserve these actual local causes:

| Cause | Result |
| --- | --- |
| Entrance Grunt 7 victory, then wall-panel interaction | Discover and open the entrance |
| B1F Grunt 12 victory | Open the B1F security barrier |
| B4F Grunt 18 victory | Reveal the Lift Key |
| Successful Lift Key handoff | Permit lift use |
| Both B4F Grunts 16 and 17 defeated | Open the paired-guard barrier |
| Giovanni victory | Reveal the Silph Scope |
| Successful Scope handoff | Obtain the local investigation reward |

Derive barrier state on map load from the corresponding real trainer flags.
Guard Giovanni's battle and reward with the real local security predicates;
a bypassed collision or fixture warp must not count as defeating guards.
Already earned partial victories persist across exits and losses. Do not
remove optional trainers or ordinary rewards when Giovanni is defeated.

Lift Key and Scope handoffs must check their local prerequisite, attempt the
item grant, and verify success before hiding the item or recording receipt.
A full bag leaves the reward available on interaction and reload. Prior item
ownership can avoid a duplicate only after the local prerequisite is met;
ownership cannot simulate a missing guard or boss victory. The lift requires
its local key handoff, not just an unrelated flag or a pre-existing item.

Allocate the following unique Wayfarer flags immediately after Anne. Source
FRLG names resolve to these values only in the Wayfarer branch; standalone HNS
and FRLG keep their existing definitions.

| Values | State |
| --- | --- |
| `0x4A4` through `0x4AF` | Escape Rope, Hyper Potion, X Speed, Moon Stone, TM12, Super Potion, Rare Candy, TM21, Black Glasses, TM49, Max Ether, and Calcium claims |
| `0x4B0` | Lift Key object visibility |
| `0x4B1` | Silph Scope object visibility |
| `0x4B2` through `0x4B5` | Hidden PP Up, Nugget, Nest Ball, and Net Ball claims |
| `0x4B6` | Game Corner entrance open after Grunt 7 is defeated |
| `0x4B7` | Lift enabled after the Key handoff or verified ownership |
| `0x4B8` | Giovanni defeated and hidden |
| `0x4B9` | Celadon Hideout world-map visit |
| `0x4BA` | Successful Lift Key receipt |
| `0x4BB` | Successful Silph Scope receipt |

The source object visibility flags and the item receipts are deliberately
different: `removeobject` changes the former. Before Grunt 18 or Giovanni
wins, B4F load hides the matching Key or Scope object. After the relevant win,
it restores that object only while its receipt flag remains clear. A successful
handoff sets its receipt and then hides its object. This prevents an unset
source hide flag from exposing either reward early without a save migration or
a global Rocket cleanup flag.

Append the 13 Trainer records at runtime IDs 1531 through 1543, backed by
compact HNS defeated-state slots 677 through 689. Define
`TRAINERS_COUNT_CELADON_HIDEOUT_WAYFARER = 13` and make the count:

```
TRAINERS_COUNT_WAYFARER = TRAINERS_COUNT_HNS
    + TRAINERS_COUNT_EMERALD - 1
    + TRAINERS_COUNT_SS_ANNE_WAYFARER
    + TRAINERS_COUNT_CELADON_HIDEOUT_WAYFARER
```

The resulting Wayfarer count is 1544. These IDs and flags cannot overlap HNS,
Hoenn, Anne, system, or partner state.

Allocate `VAR_CELADON_ROCKET_HIDEOUT_ELEVATOR_FLOOR` at `0x40D9`, after
Anne's `0x40D8`. The FRLG generic elevator variable aliases active HNS
Faraway Island state and must never be read or written by this adventure. A
Wayfarer-only resolver derives the dedicated floor from the incoming dynamic
warp when the elevator control panel is used; retain the existing renderer and menu
position helper, which read the supplied display floor and dynamic destination
without writing the aliased variable. Preserve B1F/B2F/B4F destinations, floor
labels, cancellation and dynamic return, including save/reload.

## Battles and dialogue

Preserve all 13 authored FRLG parties: entrance Grunt 7, Hideout Grunts 8
through 18, and Boss Giovanni. Retain original species, moves, levels, battle
classes and portraits. Append the 12 grunts to ordinary trainer scaling;
Giovanni retains the existing authored boss classification. Keep these IDs
isolated from HNS, Hoenn and Anne trainer defeat state.

Every deliberate battle entry checks the canonical usable-party predicate
before control locking or irreversible staging. Empty, fainted and Eggs-only
parties receive a repeatable hostile refusal with no battle, emergency recovery,
reward, departure or security change. Ordinary sight encounters keep the
existing usable-party suppression. Completed actors' non-battle dialogue remains
available without a usable party.

The existing generic sight probe remains authoritative for all normal trainers.
For one of the twelve Wayfarer Hideout grunt scripts, it stops at the
ordinary-party special before an unfought battle or at post-battle dialogue
after a completed battle. Only after that non-`trainerbattle` result, a
Wayfarer-only exact start-label resolver supplies the script's explicit
`trainerbattle` opcode. It recognizes no other script and excludes Giovanni.
The regular fought-flag check then suppresses completed grunt sight approaches,
while direct interaction still runs the local no-party refusal and post-battle
dialogue.

Check `GetBattleOutcome` explicitly before success continuations. Only a win
may remove a guard, reveal a reward or complete the boss. Retain current
blackout/recovery routing for these callers; the broader trainer-only core's
field-return behavior is not implemented by this milestone. If an existing
exception returns a loss to the field, stop the success continuation and release
control with retryable staging. Do not alter global battle recovery.

Adapt statements that assume Mt. Moon or another investigation was completed.
Giovanni's dialogue must work on a first encounter. The later Silph milestone
must add callbacks based on actual prior meetings without making either
investigation depend on the other. This milestone grants no badge, Silph rescue,
Tower/Fuji completion, Blue chapter advancement or Giovanni finale completion.

## Content and runtime closure

Preserve 14 visible rewards, including the two local key items, and four hidden
items:

| Floor | Visible rewards | Hidden rewards |
| --- | --- | --- |
| B1F | Escape Rope, Hyper Potion | PP Up |
| B2F | X Speed, Moon Stone, TM12, Super Potion | None |
| B3F | Rare Candy, TM21, Black Glasses | Nugget |
| B4F | Silph Scope, Lift Key, TM49, Max Ether, Calcium | Nest Ball, Net Ball |

Include the narrow BuildingFrlg primary and SilphCo secondary tileset
dependencies, including the latter's shared Condominiums graphics, door
animations, palettes and tile callbacks. Include only the required additional
FRLG overworld Rocket and Giovanni sprite assets; the FRLG battle portraits are
already available. Verify compiled tables rather than inferring inclusion from
source files.

Keep the existing Celadon `MAPSEC_ROCKET_HIDEOUT` separate from
`MAPSEC_ROCKET_HIDEOUT_HNS` used by Mahogany. The first HNS table already has a
Mahogany entry at `(15, 2, 1, 1)`, so Wayfarer must override its Celadon Hideout
entry to `(11, 6, 1, 1)` and add the valid combined Johto/Kanto entry at
Celadon's `(22, 5, 1, 1)`. Preserve Kanto classification, labels, existing
section IDs and standalone campaign behavior. Exercise actual Pokedex map
initialization.

## Validation and budget

Validate map selection and warp closure, event filtering, safe approach/return
paths, exact reward inventory, graphic/door dependencies, namespace isolation,
trainer party parity and appropriate scaling. Set a Faraway Island step-counter
sentinel before Hideout elevator use and prove it remains unchanged. Compare
standalone HNS and FRLG preprocessing for changed shared code and scripts;
the Hideout resolver's Wayfarer-only braces are semantically inert in the
standalone outputs.

Emulator coverage must exercise entrance refusal and discovery, exit/re-entry
through save/reload, empty/fainted/Eggs-only battle refusal, automatic ordinary
sight battle, defeated-grunt sight suppression, actual trainer and Giovanni
wins, loss followed by retry without unearned progress, the lift's key
requirement and floor travel, paired guards, full-bag reward retries, persistent
receipt and actual region-map initialization. Run relevant native state tests,
the exact harbor audit and focused Anne regression after shared generator edits.
Obtain native critic review before committing the completed milestone.

The Anne release baseline is 33,007,476 used bytes with 546,956 bytes physically
free and 22,668 bytes above the production reserve. Measure this milestone's
release delta. The user permits development below reserve; preserve and report
the normal production check. Physical ROM capacity still applies. Space recovery
and a rebase onto it remain merge prerequisites. Do not cut content or introduce
unrelated optimization to pass this milestone.

Validation on the completed implementation: all 14 Hideout emulator cases,
10 Anne regression cases, six native trainer-state tests, nine map-catalog
tests, 15 trainer-scaling tests, and 16 harbor-menu audit tests pass. The scaling
generator check also passes. Shared graphics, door and region source preprocessing
matches standalone HNS and FRLG; their trainer sight source differs only by
inert braces around the unchanged fallback assignment. E2E type checking,
lint and formatting pass. The production release links at 33,067,852 used bytes (`__rom_end =
0x09F8934C`), adding 60,376 bytes over Anne. EWRAM remains 248,557 bytes and
IWRAM remains 25,616 bytes. The unchanged production reserve check fails by
37,708 bytes; 486,580 bytes remain physically free. This is an accepted
development result under the user waiver, not a passing release-budget check.
Final native critic review is clear.

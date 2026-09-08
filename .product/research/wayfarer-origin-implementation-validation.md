# Starting-origin implementation validation

This records implementation evidence for the task based on merged PR #73.
The [specification](../specs/wayfarer-regional-start-choice.md) remains
`Implemented: No` until runtime is on main and its acceptance requirements
are met. Source checks and mechanics tests do not substitute for emulator
journeys.

## State and source-boundary audit

| Entry points | Previous ownership | Implementation ownership |
| --- | --- | --- |
| `new_game.c`, `data/scripts/new_game.inc` | Initial HNS warp preceded map-state reset; shared script aliases could depend on the active map | Common clears precede the explicitly HNS-sourced reset. The selected profile runs last, before its map callbacks. |
| `wayfarer_origin.c`, `wayfarer_persistence.c`, `save.c` | No persistent origin; Johto was unconditionally visited; invalid magic could reinitialize state | Stable saved origin and fallback recovery; actual visited bits; current-version invalid origin is corrupt data. Continue never invokes profile setup. |
| `data/scripts/wayfarer_hoenn_entry.inc` | Visitor entry was the only Hoenn initializer | Fixed Hoenn baseline is shared; visitor household suppression is a separate arrival routine. Native setup leaves household and truck scenes pending. Initialized commits last. |
| `Route101/scripts.inc`, `battle_setup.c` | Wayfarer always used the existing-party visitor rescue | Profile scene dispatch chooses native bag/first-battle or visitor rescue. Native delivery commits Hoenn slot and receipt after success; native loss returns to the authored healing continuation. |
| `LittlerootTown_ProfessorBirchsLab/scripts.inc` | Visitor choice and optional gift | Native acknowledgement bypasses a second grant; visitors retain independent choice and retryable gift. Shared equipment and Dex availability go through engine C helpers. |
| Littleroot houses, `clock.c`, `player_pc.c` | HNS compile-time paths could override Hoenn household behavior | Household policy selects family flow. Clock setup preserves global time initialization; PC location and shutdown follow active map source. |
| `NewBarkTown_Lab_hns/scripts.inc` | Shared Pokémon availability stood in for Elm choice | Candidate is temporary until confirmation. Dedicated Johto commitment and receipt flags preserve local quest choice independently from gift delivery. |
| New Bark houses and Silver scripts | Household triggers and rival branches assumed a New Bark player | Visitor household entries stop before movement or reset. Uncommitted Johto choice defers Silver scenes without closing traversal. |
| `PetalburgCity_Gym/scripts.inc` | Norman always claimed the player as his child | Native family dialogue remains; visitors receive neutral dialogue without campaign changes. |
| Olivine, Vermilion and Slateport Aqua attendants | Raw HNS maiden-voyage checks at all regular ports | Profile-backed C predicate reads the intended bank. Littleroot receives a retryable Ticket at Slateport; maiden-voyage state and rewards remain untouched. |
| `scrcmd.c`, `wild_encounter.c`, `trainer_see.c`, battle setup/special entry | Stock opening assumptions supplied a party | Actual usable-party checks suppress ordinary entry. Unexpected forced entry abandons its continuation and returns to safe local recovery. |

Hoenn generated operands reviewed separately from C accessors:

| State | Hoenn operand |
| --- | --- |
| Littleroot intro variable | `0x7092` |
| Birch lab variable | `0x7084` |
| Route 101 variable | `0x7060` |
| Local starter choice | `0x7023` |
| Successful starter receipt | `0x64FF` |
| Birch rescued | `0x6052` |
| Household clock set | `0x6051` |
| Local running-shoes receipt | `0x6112` |
| Local Pokémon / Dex / running availability | `0x6860` / `0x6861` / `0x68C0` |

The Hoenn-local system operands remain local. Native starter delivery and
shared equipment, shoes and Dex handoffs also set the HNS-engine availability
flags through C helpers. Hoenn starter C writes never target
`VAR_STARTER_MON`; the standalone callback retains its existing variable.
The Johto markers occupy Wayfarer-only HNS flags `0x930` and `0x931`.

`InsideOfTruck` is required content in the classification policy. The
generator retains the 518-map catalog, including existing IDs and the truck's
layout and event closure. No map binary was edited.

## Acceptance record

The following source checks passed during implementation:

- Hoenn content tests: 9; source-boundary tests: 19; regional campaign tests: 8.
- Aqua entry audit tests: 10; League script tests: 16.
- Origin script transaction and household tests: 20.
- Trainer scaling inventory audit: 1,491 populated IDs, no structural failures.
- Oak font-width audit: dialogue at most 157 pixels in the 216-pixel window.
- E2E TypeScript typecheck and 11 protocol unit tests.

Executed builds and mechanics:

- Wayfarer E2E ROM and symbols build successfully.
- Release-size build passes the enforced `0x09F80000` limit: 32,875,884
  bytes used, 678,548 bytes unused in the 32 MiB ROM.
- Standalone HNS, Emerald, FireRed and LeafGreen builds pass, run serially
  with explicit map versions.
- The selected Wayfarer mechanics suite passes all 70 tests, including
  flash save/reload, rejection of an unknown saved origin, custom-profile
  authored scripts, recovery, and existing Hoenn state checks. All four starter challenge tests pass, including monotype cache changes
  and randomized preview/delivery consistency. The suite also covers initial
  Dex catalog selection and the legacy Kanto visited flag.

Executed emulator checks use headless SkyEmu and an immutable copy of the
E2E ROM and symbols:

- Both Oak choices and save/Continue preserve the selected origin, initial
  map, visited region, home, empty party, and zero progression. Cancellation
  and confirmation input barriers pass (3 tests).
- All six native starter slots pass, including the female Hoenn appearance,
  persistence, independent regional choices, and no second lab grant.
- Losing the native Birch battle heals the same partner and resumes in the lab.
- A Hoenn visitor can postpone Elm's choice, commit Totodile, decline the
  gift, save/reload, and reclaim it without rewriting the Hoenn choice.
- Native Hoenn completes the Route 103 rival branch, Birch's shared Dex
  grant, Mom's running shoes, and save/reload. A Johto visitor enters via
  Aqua, rescues Birch with its existing Cyndaquil, declines Torchic, then
  saves/reloads and claims the optional gift (2 tests).
- Visual inspection confirms Oak and Marill remain visible and both the
  [question](assets/regional-start/oak-origin-question.png) and
  [Hoenn confirmation](assets/regional-start/oak-hoenn-confirmation.png)
  fit their windows.

- The complete Hoenn-origin Aqua loop passes Ticket receipt, all three legs,
  per-leg save/reload and local blackout, and all three visited-region bits.
  Existing Aqua traversal regressions pass all 13 tests, including the Johto
  maiden voyage.
- The visitor rescue loss/retry and post-Dex deferred gift pass in the rebuilt
  ROM: reclaiming the gift preserves lab progress and leaves the defeated
  Route 103 rival absent.
- Four special-recovery cases pass: both origins Teleport to their initial
  homes and preserve later Center recovery across travel/save; actual Hoenn
  field-poison whiteout uses Lavaridge, while Johto preserves its local Center
  despite the same Hoenn override flag.
- Both native household journeys pass. New Bark completes its clock and Mom
  sequence before Elm; Littleroot completes truck, clock, Mom, rival (local
  state 1), Route 101 rescue (local state 2), Birch's native starter, and
  save/reload. Each supports early-loss recovery at its selected home.
- Elm's visitor gift remains unreceived with a full six-mon party and all 420
  PC slots full, survives reload, and delivers the committed Totodile into
  the freed PC slot exactly once.
- Native Hoenn’s Start menu opens the actual Pokédex and Pokégear after their
  shared handoffs. Visual inspection records the Hoenn Pokédex with owned
  Torchic and the Pokégear screen in
  [the captured assets](assets/regional-start/hoenn-native-pokedex.png).
- A focused Hoenn-origin League journey passes all three League lobby/first
  battle entries, admission save/reload, and maiden-voyage isolation. Its
  badges and prior clears are fixtures; it is not evidence of a played
  24-badge campaign.

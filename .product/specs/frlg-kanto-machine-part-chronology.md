# FRLG Kanto Machine Part chronology

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)
Implemented: No

## Scope

This specification preserves HNS's Machine Part theft and Misty sequence in
Wayfarer while adapting two Route 24 passages that make claims about
Johto's Team Rocket story regardless of the player's actual progress. It covers only Wayfarer dialogue
selection after the Route 24 Rocket battle.

It does not choose a Power Plant layout, radio placement, or any radio service
change. It does not change the Machine Part state machine, Rocket battle,
rewards, map events, or standalone HNS dialogue.

## Behavior

### Retained quest sequence

`VAR_KANTO_ROCKET_STORY_STATE` retains its existing HNS meaning and transition
order:

1. The Power Plant manager starts the theft at state 0 and sets state 1.
2. The Route 10 officer moves the Cerulean lead to state 2 and exposes the Gym
   Rocket.
3. The Cerulean Gym scene moves the Rocket to Route 24 at state 3.
4. The Route 24 approach moves to state 4. Losing the Rocket battle leaves the
   state and Rocket available for retry. Winning keeps the existing battle,
   movement, hide flag, then sets state 5 and exposes the Gym pickup.
5. The Gym pickup grants `ITEM_MACHINE_PART`, then sets the existing hidden-item
   flag and state 6 only after possession succeeds.
6. The manager's existing retryable handoff grants or verifies `ITEM_TM_THUNDER`,
   removes the Machine Part, then sets state 7 and
   `FLAG_RETURNED_MACHINE_PART`. It also clears `FLAG_HIDE_ROUTE25_MISTY`, sets
   `VAR_CERULEAN_CITY_STATE` to 2, and hides the rear-exit engineer.
7. Route 25's existing Misty scene returns Misty and the Gym Trainers, then sets
   `VAR_CERULEAN_CITY_STATE` to 3. It does not award or require the Cascade
   Badge.

The retained flag continues to enable its existing Power Plant, Copycat, radio,
Underground Path, and Magnet Train consumers. This milestone does not alter
those consumers or the S.S. Aqua's independent eligibility.

### Route 24 chronology dialogue

`Route24_EventScript_Grunt` keeps its existing command stream, battle identity,
movement, flags, and state writes. The two existing text labels select their
strings with `#if IS_WAYFARER` and `#else`; the message calls themselves do not
change:

| Existing label | Required Wayfarer text | Reason |
| --- | --- | --- |
| `Route24_Text_RocketAfter` | `OK. Tell you mine secret will I?` / `MACHINE PART steal by me, hide it I did in GYM of the CERULEAN.` / `Inside water put it I did. Look for in water center of GYM at.` / `But you forget me not!` / `I find my own way from here.` | Reveals the hiding place without promising a Johto retaliation that may already be over or may still be pending. |
| `Route24_Text_RocketDisappears` | `…` / `You say what? I not welcome here anymore?` / `Oh, no! Should I do what now on from, me?` | Keeps the Rocket's exit without claiming that Team Rocket has disbanded. |

The shared hiding-place clue retains its original line and paragraph breaks.
New passages use the existing text window and explicit line breaks. They must not write a flag, alter a variable, choose
another battle result, or condition on Goldenrod, Giovanni, League, badge, or
origin state.

In non-Wayfarer builds, the `#else` branches retain the original strings under
the same labels exactly. The Johto revenge and Team Rocket disbandment exchange
remain the standalone HNS result.

## Validation

For this milestone, static validation must preprocess the Route 24 script for Wayfarer and HNS.
The Wayfarer output must contain the neutral strings and omit the Johto revenge
and disbandment claims. The HNS output must be byte-for-byte equivalent to the
preprocessed baseline at `7aa8db0557`. In both outputs, the executable command stream
between `Route24_EventScript_Grunt` and the next script label must be identical.

Run the existing HNS traversal tests that cover the Route 24 loss retry,
state-5 write, Machine Part pickup, manager handoff, Cape date, and Gym return.

## References

- [HNS open-world regional traversal](hns-open-world-region-traversal.md#magnet-train-restoration)
- [HNS Kanto story conflict inventory](../research/frlg-hns-kanto-story-conflicts.md#cross-region-follow-up)
- [Route 24 script](../../game/data/maps/Route24_hns/scripts.inc)
- [Power Plant handoff](../../game/data/maps/Route10_PowerPlantBackRoom_hns/scripts.inc)
- [Route 25 Misty scene](../../game/data/maps/Route25_hns/scripts.inc)

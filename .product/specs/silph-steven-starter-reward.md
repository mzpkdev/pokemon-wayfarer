# Silph Steven starter reward

Implemented: No

Status: Future draft, outside [PR #86's delivery scope](frlg-kanto-story-milestones.md).

## Approved product direction

Wayfarer retains the existing HNS Silph lobby interaction with Steven as a
universal, one-time Hoenn starter choice. It is independent of the local Silph
occupation and liberation story.

## Contract

The user approved the universal starter choice. The following describes the
intended ordinary flow; implementation remains deferred pending the capacity
decision below:

1. Every player may choose Treecko, Torchic, or Mudkip at level 5, or decline.
2. A completed gift uses the existing `Common_EventScript_GiftMon` flow,
   `FLAG_GOT_HOENN_STARTER`, Steven departure movement, and object removal.
3. The receipt remains clear after a decline. A successfully delivered choice
   must not duplicate on later visits.
4. Neither the player origin nor `FLAG_SILPH_LIBERATED_HNS` is read or written.
   A Hoenn-origin visitor may receive this separate HNS gift, by product
   decision.

## Capacity decision required

The inherited HNS script calls `Common_EventScript_GiftMon`, but when both
party and PC are full it can still set the receipt and remove Steven without
delivering the starter. The user has not selected whether to preserve that
exact inherited behavior or make failed delivery retryable with Steven and
the receipt unchanged. Resolve that choice before selecting an implementation
approach; the unmodified source script is not a success-only transaction.

PR #86 retains acknowledgement-only Steven dialogue and does not implement
this reward. Standalone HNS behavior must remain unchanged unless explicitly
included in a later approved scope.

## Boundaries

This draft does not alter the completed [President Master Ball](silph-president-master-ball.md).
It does not give an Earth Badge, change Giovanni, or create a Blue scene.

## Validation

Future E2E coverage must select a starter in both occupied and liberated Silph states,
verify the selected level-5 Pokemon and `FLAG_GOT_HOENN_STARTER`, preserve the
player origin, and confirm the removed Steven object and one-time receipt after
save/reload. Cover decline and the selected full-party plus full-PC behavior.
Choose source or Wayfarer handler coverage only after the capacity decision.

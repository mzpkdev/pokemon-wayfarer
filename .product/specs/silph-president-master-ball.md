# Silph President Master Ball

Dependencies: [Silph Co. liberation](frlg-kanto-silph-liberation.md), [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)

## Scope

After a Wayfarer player liberates Silph Co. by defeating its local Giovanni,
the President restores the FRLG Master Ball handoff at the next successful
interaction. This specification supersedes every Master-Ball-specific boundary,
state, President-handler, and validation statement in
`frlg-kanto-silph-liberation.md`.

The existing New Bark Lab Master Ball remains unchanged. A player can therefore
receive one local Silph Master Ball and the existing Elm Master Ball: two Master
Balls are intentional product behavior.

## Contract

Giovanni's successful local battle continues to set
`FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS`. The President on 11F owns the
pending handoff:

1. Before liberation, he retains the occupied-building line and gives nothing.
2. After liberation, a clear
   `FLAG_SILPH_MASTER_BALL_RECEIVED_HNS` (0x4F9, Wayfarer only) presents the
   original gendered FRLG thanks line, checks the Ball pocket for
   `ITEM_MASTER_BALL`, then gives exactly one Master Ball with the original
   receipt message and prototype explanation.
3. Set the Silph receipt and clear the pending marker only after the item is
   added. A full Ball pocket leaves the receipt clear and pending marker set,
   shows the original no-room message, and permits a later retry.
4. Once the receipt is set, the President repeats the original prototype
   explanation and never gives another Silph copy. A pre-owned Master Ball does
   not suppress the local Silph copy; its stack grows by one.

There is no Trainer Rating, badge, League, legendary-readiness, origin, or
other story threshold. The transaction must not read or write Elm's receipt,
`FLAG_GOT_MASTER_BALL_FROM_SILPH`, or any non-Wayfarer state.

This is an approved product override to the independent-story PRD's formerly
open Master Ball readiness question: Silph's Master Ball is immediate after
local liberation. It does not decide or alter readiness rules for Articuno,
Zapdos, Mewtwo, or any other legendary capture.

## Boundaries

Do not change the New Bark Lab script, its timing, or its Master Ball. Do not
award an Earth Badge, advance the Giovanni finale, affect Celebi, or change
Silph's staff services. This is a script and local-flag change only; no map,
tileset, collision, elevation, layout-version, or map.bin edit is authorized.

## Validation

Cover the occupied President, the initial post-liberation receipt, reload and
repeat interaction, a pre-owned Master Ball stack, and a full Ball pocket that
retries after one fixture slot is freed. Verify that a Giovanni loss leaves
both the pending and receipt flags clear. Retain source and native checks for
the isolated Wayfarer flag allocation and the set-after-add transaction.

Measure the release-ROM delta and remaining physical reserve after the
milestone. Obtain native critic review before acceptance.

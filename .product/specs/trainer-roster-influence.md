# Trainer roster influence

PRD: [Trainer roster influence](../prds/trainer-roster-influence.md)
Implemented: No
Design status: Mechanism defined; concrete interactions are future content.

## Scope

Own the gameplay hooks that act on a trainer's roster: flags that activate
filler-weight modifiers, evolution-item gift offers, and predefined trade offers
with their save records.
[Trainer world progression](trainer-world-progression.md#rosters) owns the
roster format, sizes, filler scoring and composition, the modifier table
format, evolution by line, traded-filler scoring and the `TRADED` move rule,
and snapshots; this spec does not restate them. Which interactions, gifts, and trades exist is future
content.

## Behavior

### Modifier hooks

A modifier is a row in the progression spec's
[modifier table](trainer-world-progression.md#filler-composition), keyed by a
gameplay flag. The content that grants an interaction (a conversation, quest
step, or favour) sets that flag in its own script. Flags are persistent save
flags; this feature never clears them. A set flag takes effect at the next
roster resolution and never changes a frozen Gym plan or league entry snapshot.
Modifiers only change filler scores: they never affect aces, `sizeFor`, the ace
allowance, standing, or arcs. A strong modifier may outrank a traded filler;
that is accepted.

### Evolution-item gifts

| Field | Contract |
| --- | --- |
| `giftId` | Stable ID, never reused. |
| `characterId` | Receiving trainer. |
| `member` | An `aceId` or `fillerId` in that trainer's roster. |
| `item` | The evolution item offered. |
| `form` | The form in the member's authored line that the item produces. |
| `flag` | One flag per offer, set when the player gives the item. |

Giving the item consumes it and sets the flag. While the flag is set, the
member is at least `form` whatever its level; later authored evolve levels
still apply. The gift is permanent for the save. Ace moves follow the authored
entry for the resulting form; filler moves follow `LEVEL_UP` for it.

### Trade offers

| Field | Contract |
| --- | --- |
| `offerId` | Stable ID, never reused. |
| `characterId` | Offering trainer. |
| `wantedSpecies` | Any individual of this species the player owns qualifies. |
| `givenFillerId` | A filler in that roster; never an ace. |
| `received` | The Pokémon the player receives, authored like existing in-game trades. |
| `tradedFillerId` | `fillerId` of the traded filler entry; unique in the roster, never reused. |
| `tradedLine` | Authored evolution line for the wanted species from its post-trade form. |
| `tradedOffset` | Traded filler level offset, −6..0, default −2. |
| `requiresFlag` | Optional flag that makes the offer available. |

At most one trade per trainer. After the trade, `givenFillerId` leaves the
trainer's eligible pool and the player's Pokémon joins it as an ordinary
[traded filler](trainer-world-progression.md#traded-fillers) with a score
boost, so the normal top-K rule includes it first. Because one filler leaves as
the traded filler arrives, the roster still reaches six at maximum size.

**Trade evolution.** A wanted species that evolves by trading evolves on the
trade under the main games' conditions; its post-trade form is where
`tradedLine` starts, and it never drops below that form.

**Moves.** Keep all, learn level-up only: the traded filler keeps every move
it had at the trade, taught moves included, and learns new moves by the
progression spec's [`TRADED` rule](trainer-world-progression.md#traded-fillers).
The trainer never teaches it TMs.

### Trade records

One fixed save record per trade slot, targeting about 32 bytes:

| Field | Purpose |
| --- | --- |
| `offerId` | Identifies trainer, given filler, traded filler ID, line, and offset. |
| Species/form at trade | Post-trade form. |
| Trade level | Base for move learning. |
| Moves at trade | Four move slots, oldest first. |
| Personality, OT ID | Identity, and shininess, gender, and nature where derived. |
| Nickname, OT name | Displayed identity. |
| Ability slot, ball, explicit shiny/nature bits | Identity not derived from personality. |

The record is written in the same logical save transaction as the Pokémon
exchange. Identity persists: nickname, shiny, gender, nature, ability slot,
ball, OT name and ID, and personality. Held items, IVs, EVs, and experience
are not kept; the traded filler uses the trainer's constructor defaults for
them.
Exact packing is an implementation choice measured against the save reserve;
the number of slots is a fixed budget. A record naming an unknown offer or
species is an invalid save, never a silent drop.

### Determinism

No feature here draws randomness. A roster's composition stays a pure function
of the root, flags, world point, trade records, and the pinned policy, so
reloads cannot reroll it. Gym plans read current flags and records at battle
setup; a league entry snapshot freezes composed teams, so gifts, trades, or
modifiers during retries do not change the field.

## Validation

- Offers and gifts reference existing trainers, members, items, and forms;
  trade offers never give an ace; one trade per trainer; stable, unique IDs.
- Every roster still reaches six at maximum size after each authored trade.
- Gift flags force the item form at every level below its evolve level and
  leave later evolutions intact.
- Trade records round-trip every identity field, hold within the budget, and
  reject unknown references; trade evolution fixtures cover each authored
  trade-evolution species.
- Traded-filler move fixtures as listed in the
  [progression validation](trainer-world-progression.md#validation).
- A flag, gift, or trade set during an active league retry leaves the frozen
  field unchanged, and applies to the next resolution.

## Open questions

- Concrete interactions, gifts, and offers (future content).
- The total trade-slot budget and the exact record packing.
- Whether a held item on the traded Pokémon returns to the player.

## References

- [Trainer world progression specification](trainer-world-progression.md)
- [Seeded league circuit](seeded-league-circuit.md)
- [Playthrough seed framework](playthrough-seed-framework.md)

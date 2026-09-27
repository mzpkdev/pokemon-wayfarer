# Trainer roster influence

PRD: [Trainer roster influence](../prds/trainer-roster-influence.md)
Implemented: No
Design status: Parked: not part of v0.

Design note for the Later extension path of notable trainer rosters. v0
rosters are one fixed ordered list of six roster slots, each flagged ace or
filler slot ([Notable trainers](notable-trainers.md#rosters)). This note keeps
that shape and lets filler slots be filled dynamically; nothing here changes
v0 resolution.

## Filler-slot pattern

The v0 roster becomes a pattern. Ace slots (`isAce`) stay fixed: authored
species, offset, and battle content, never replaced. Filler slots keep their
position, so join order and
[battle order](notable-trainers.md#rosters) are unchanged, but their Pokémon
come from the trainer's filler pool. Team size, team level, and trainer TR are
untouched.

## Weighted pools

Each trainer has one filler pool, which may hold more than six options.

- **Weight** = authored base weight + optional seeded variation (a per-save
  value per trainer and option from the
  [playthrough seed](playthrough-seed-framework.md)) + the sum of active
  modifier deltas.
- **Modifiers**: authored rows `flag → (characterId, fillerId, delta)`, active
  while the persistent save flag is set. A set flag applies at the next
  resolution and never alters a battle snapshot or a locked lineup.
- **Minimum TR** (optional per option): an option is ineligible while the
  trainer's TR is below it; the next-best eligible option fills meanwhile.
- **Fill**: with K filler slots in the current team, the K highest-weight
  eligible options fill them in pattern order (highest weight in the first
  filler slot; ties by pool order). A filled slot uses the option's species,
  level offset, and battle content.
- **Containment**: top-K is contained in top-(K+1), so with unchanged weights
  growth only adds fillers and never swaps one. Only modifiers, or an option
  reaching its minimum TR, change existing picks.

## Trades

- **Offers** are predefined per trainer: a wanted species (any individual
  qualifies) for one filler option, never an ace. The traded-away filler
  leaves the pool through one save flag per offer.
- **Pool entry**: the player's Pokémon joins the pool with weight
  `TRADE_BOOST`, high enough to take the first filler slot.
- **Trade record**: one fixed save record per trade (about 36 bytes), written
  in the same save transaction as the exchange: offer ID, species/form as
  arrived, personality (shiny, gender, nature), OT ID and OT name, nickname,
  four moves, trade level, ball, and ability slot. Held items, IVs, EVs, and
  experience do not persist. A record naming an unknown offer or species is an
  invalid save.
- **Evolution**: the same
  [downward rule](player-trainer-rating.md#evolution-stages) as everyone. The
  entry's authored stage is its line's final form (branching lines: the
  arrived stage unless the offer names a target). A **level floor** at the trade
  level means it never appears below what the player gave, not below the trade
  level and not below the arrived stage, and it evolves naturally as the
  trainer grows. A trade evolution happens on the trade, so the arrived stage
  already includes it.
- **Moves**: start from the record's four moves, oldest first. For each level
  above the trade level up to the member's level, add the current stage's
  level-up moves in learnset order, skipping known moves and otherwise filling
  an empty slot or replacing the oldest. The trainer never teaches TMs; the
  result is a pure function of the record and the level.

## Data shape

Sketch for reference; team building stays a pure function at battle start.

```text
ROM   NotableTrainer { characterId, startTR, peakTR, archetype, flags, slots[6] }
ROM   RosterSlot     { species, levelOffset, isAce, movePolicy, moves, item,
                       ability, nature, ... }                        // v0
Later FillerOption   { fillerId, species, levelOffset, baseWeight, minTR, ... }
Later FillerModifier { flag, characterId, fillerId, delta }
Save  TradeRecord    { offerId, species, personality, otId, otName, nickname,
                       moves[4], tradeLevel, ball, abilitySlot }     // ~36 bytes
```

Resolution then also reads save flags, the playthrough seed, and trade
records; it still draws no randomness at runtime, so reloads cannot change a
roster.

## Open questions

- Exact `TradeRecord` packing and the trade slot budget against the save
  reserve.
- Concrete pools, modifiers, and offers, and the `TRADE_BOOST` value.
- Whether a held item on the traded Pokémon returns to the player.
- Evolution gifts (an item that makes a filler appear evolved) as a further
  hook.

## References

- [Notable trainers](notable-trainers.md)
- [Player Trainer Rating](player-trainer-rating.md#evolution-stages)
- [Playthrough seed framework](playthrough-seed-framework.md)

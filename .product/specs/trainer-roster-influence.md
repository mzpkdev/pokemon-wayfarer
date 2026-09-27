# Trainer roster influence

PRD: [Trainer roster influence](../prds/trainer-roster-influence.md)
Implemented: No
Design status: Parked: not part of v0.

Design note only. v0 rosters are one fixed ordered list with no dynamic picks
([Well-known trainers](well-known-trainers.md#rosters)). Every
hook below depends on a future dynamic-roster rule (for example scored
supporting picks with an `ace` flag) that v0 does not have; restate the hooks
against that rule when it is designed.

## Hooks

- **Modifiers.** Authored rows `(modifierId, flag, characterId, member,
  delta)`, active while a persistent save flag is set, with deltas for the
  same member summing. They only reorder dynamic picks; they never change
  signature members, team size, or TR. A set flag applies at the next
  resolution and never alters a frozen battle or league snapshot.
- **Evolution gifts.** One offer per `(giftId, characterId, member, item,
  form, flag)`. Giving the item consumes it and sets the flag; while set, the
  member is at least `form` whatever its level. Permanent for the save.
- **Trades.** One offer per trainer: `wantedSpecies` (any individual
  qualifies) for a given non-signature member. The given member leaves the
  roster and the player's Pokémon joins as a strongly favoured member, so the
  roster still reaches six. A trade-evolving species evolves on the trade and
  never drops below that form.
- **Traded moves: keep all, learn level-up only.** Start from the four moves
  held at the trade, oldest first. For each level above the trade level up to
  the member's level, add that form's level-up moves in learnset order,
  skipping known moves and otherwise filling an empty slot or replacing the
  oldest. The trainer never teaches TMs; the result is a pure function of the
  trade record and level.

## Trade record

One fixed save record per trade slot, written in the same save transaction as
the exchange: offer ID, post-trade species/form, trade level, four moves,
personality and OT ID, nickname and OT name, ability slot, ball, and explicit
shiny/nature bits. Identity persists; held items, IVs, EVs, and experience do
not. A record naming an unknown offer or species is an invalid save. No
randomness is drawn, so reloads cannot change a roster.

## Open questions

- Record size: roughly 36 bytes per slot, against the save reserve; exact
  packing and the slot budget.
- Concrete interactions, gifts, and offers.
- Whether a held item on the traded Pokémon returns to the player.

## References

- [Well-known trainers](well-known-trainers.md)
- [Playthrough seed framework](playthrough-seed-framework.md)

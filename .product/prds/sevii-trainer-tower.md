# Sevii Trainer Tower

Status: Approved product design. Not implemented.

## Intent

Restore Trainer Tower as Sevii's repeatable timed battle facility. Players can
choose the original challenge formats, race to the roof, compare personal best
times, and earn the original prizes without tying the facility to Sevii's story
or Wayfarer's regional progression.

## Design

### Availability

Trainer Tower is available whenever the player can reach Seven Island. It does
not require a League clear, a Rainbow Pass, Celio's repair, the National Pokédex,
another Sevii objective, or a minimum Trainer Rating.

The lobby remains a safe service area with healing, its Mart, challenge
information, and the records board. Starting, abandoning, winning, or losing a
challenge never changes ferry destinations or another island's story.

### Challenge formats

Offer the four original formats:

| Format | Per-floor battle |
| --- | --- |
| Single | One Trainer in a Single Battle |
| Double | A paired team in one Double Battle |
| Knockout | Three consecutive Single Battles without a heal between them |
| Mixed | Each floor uses its authored Single, Double, or Knockout format |

All formats use eight floors and finish with the owner on the roof. Stairs and
floor navigation count toward the recorded time. The elevator is an exit, not a
shortcut for completing a run.

The receptionist heals the party immediately before the timer starts. There is
no automatic healing between floors or between opponents in a Knockout floor.
The player may use ordinary Bag items and menus where the source challenge
allows them; elapsed time continues according to the source timer rules.

A Double or Mixed challenge refuses to start unless the player has at least two
usable Pokémon. Other formats require at least one. This check occurs before
healing and before starting the timer.

### Opponents and balance

Use the repository's built-in FRLG Trainer Tower sets and their original floor,
format, speech, species, moves, held items, abilities, AI, and opponent graphics.
Do not require e-Reader data, Mystery Gift, external records, or a downloaded
Trainer set. Mixed mode selects from the same built-in format rows.

Trainer Tower remains a facility context. Opponents do not use ordinary-Trainer,
Gym, League, or story scaling and do not award experience or prize money. Retain
the source facility's party-level normalization against the player's highest
usable party level. The facility must never permanently modify the player's
party, opponent data, or Trainer Rating.

### Timing and records

Start the timer after the pre-run heal and final “Go” message. Pause only for
engine states the source facility excludes from timing; map movement, battles,
menus, and allowed item use otherwise count. Stop at the roof owner.

Store one best time for each of the four formats. A first clear establishes the
record; a faster clear replaces it. A slower clear leaves it unchanged. The
records board displays all established times and a clear empty state for modes
not yet completed.

Saving is unavailable during an active challenge. A reset or power loss abandons
the run and preserves earlier records. Leaving by elevator or an available exit
abandons the run after confirmation and returns the player to the lobby.

### Losses and recovery

Losing or drawing any facility battle ends the run. Heal the party, clear only
transient run state, and return the player to the lobby. Do not charge blackout
money, move the player to another Pokémon Center, alter an opponent's cleared
state permanently, or damage a saved record.

Winning records the current floor, opens the way upward, and retains current HP,
PP, status, held-item consumption, and Bag use for the rest of that run. Ending
the challenge restores the player's party and held items to the healed run-entry
snapshot captured immediately before the timer starts, while Bag items
deliberately consumed by the player remain consumed.
This prevents facility battle effects from leaking into exploration without
making item use free.

### Prizes

The roof owner offers the authored prize associated with the selected built-in
set. A full Bag closes the battle run, restores the party, and transfers the
exact prize to a persistent pending claim at the lobby receptionist. It does not
discard the reward or force the player to repeat the run. The player can make
room, save or leave, and claim it later.

The frozen local set awards Up-Grade for Single, Dragon Scale for Double, Metal
Coat for Knockout, and King's Rock for Mixed.

Prizes are repeatable on later successful runs. They are not gated by record
improvement and do not affect Trainer Rating, badges, League qualification, or
Sevii story state.

## Boundaries

- Restore the built-in local facility only. e-Reader transfer, remote Trainer
  data, record mixing, and downloadable sets remain out of scope.
- Do not add a rental-party mode, level-selection menu, streak system, battle
  points, shop currency, matchmaking, or new rewards.
- Preserve the four formats, eight-floor course, source parties, timing identity,
  records, and prize table.
- Trainer Tower opponents are not ordinary Sevii Trainers and never enter the
  ordinary scaling inventory or rematch system.
- The exterior's ordinary Trainers, wild encounters, items, and ferry geography
  remain owned by the exploration and Trainer-restoration contracts.

## Interactions

Challenge state is isolated from ordinary map Trainer flags and all story state.
The player's current challenge options still apply where they normally affect
battles, but Nuzlocke/hardcore consequences must not be inferred from facility
loss routing. The technical specification must inventory each active challenge
option and either preserve its established facility behavior or name an explicit
exception.

The facility uses its own opponent construction and battle type. It must not
enable the temporary empty-party trainer-only wild mechanic or use story battle
continuation handlers.

## Playtesting

- Enter before any Sevii story, inspect services and records, and start each of
  the four formats.
- Verify the usable-party checks, pre-run heal, eight floors, correct layouts,
  correct opponents, and roof completion for every format.
- Lose on every floor and every position of a Knockout set. Confirm lobby return,
  party restoration, no money loss, no false clear, and unchanged records.
- Improve and fail to improve each record. Save/reload after a completed run and
  confirm all four records.
- Fill the prize pocket before reaching the roof, make space, and claim the same
  pending reward without replaying the run.
- Abandon through the elevator and reset during a run. Preserve earlier records
  and all unrelated state.
- Confirm no experience, prize money, Trainer Rating, badge, League, ferry, or
  story change from any facility battle.

## References

- [Technical specification](../specs/sevii-trainer-tower.md)
- [Sevii content overlay](../specs/sevii-content-overlay.md)
- [Sevii exploration port](sevii-exploration-port.md)
- [Trainer Tower implementation](../../game/src/trainer_tower.c)
- [Built-in Trainer Tower sets](../../game/src/trainer_tower_sets.c)

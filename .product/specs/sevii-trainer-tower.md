# Sevii Trainer Tower

PRD: [Sevii Trainer Tower](../prds/sevii-trainer-tower.md)

Implemented: No

## Scope and authority

This specification restores the local built-in FRLG Trainer Tower in Wayfarer.
It owns the lobby, challenge selection, run state, floor layouts, opponent data,
battles, timing, records, loss routing, roof owner, and prizes. The
[Sevii content overlay](sevii-content-overlay.md) owns map-event selection,
script linkage, assets, and build isolation.

Do not make `IS_WAYFARER` behave as `IS_FRLG` globally. Select only the Trainer
Tower code, data, scripts, layouts, graphics, text, and save fields named here.

## Source inventory and build selection

Use the current source implementations as behavioral inputs:

- `game/src/trainer_tower.c` for challenge state, opponent construction,
  timing, records, and prizes;
- `game/src/trainer_tower_sets.c` for built-in challenge sets;
- `game/data/scripts/trainer_tower.inc` and the lobby/floor/roof scripts;
- `game/include/constants/trainer_tower.h` and `game/include/trainer_tower.h`;
- the lobby, elevator, roof, and 1F through 8F maps already present in the
  Sevii exploration catalog; and
- the Single, Double, and Knockout layout variants for all eight floors.

Add a focused `HAS_TRAINER_TOWER_CONTENT` capability enabled by standalone FRLG
and Wayfarer Sevii. Replace relevant `IS_FRLG` compilation guards with that
capability only after auditing each guarded symbol. `FREE_TRAINER_TOWER` remains
a build-size switch for products that do not ship the feature; the production
Wayfarer configuration must fail if the feature is specified as enabled while
the switch removes required data.

Do not select e-Reader read/write code, external Trainer Tower blocks, Mystery
Gift download paths, or record mixing. Use the repository's built-in source
sets directly; the shared content audit verifies that those local sources are
present and no external loader path is active.

## Content overlay

The `trainer_tower` domain restores:

- lobby nurse, Mart clerk, receptionist, records interaction, two flavor NPCs,
  counter trigger, map scripts, and elevator flow;
- the four possible floor actor slots and owner slot on every floor map;
- format-specific coordinate triggers and map scripts;
- the roof owner and completion interaction; and
- only the source scripts and dependencies reachable from those entries.

Every retained event uses exact source identity plus a Wayfarer-owned wrapper.
Wrappers may call shared Trainer Tower helpers selected by
`HAS_TRAINER_TOWER_CONTENT`; they must not include unrelated FRLG campaign
scripts. The existing Trainer Tower exterior remains part of the ordinary
Sevii map and Trainer specifications.

## Persistent and transient state

Add Wayfarer-owned persistent state rather than exposing standalone FRLG save
layout conditionally:

| Field | Meaning |
| --- | --- |
| `bestTime[4]` | Best completed time for Single, Double, Knockout, and Mixed; zero means no record |
| `completedMask` | One bit per format with at least one completed run |
| `pendingPrize` | Exact unclaimed item, or none |

Active run state is transient and contains format, built-in set identity,
current floor, Knockout opponent index, cleared-floor mask, elapsed timer,
status, and healed run-entry party snapshot. It is initialized only
after eligibility and healing succeed.

The run-entry party snapshot records party members, healed HP, restored PP,
cleared status, held items, and party order. Restore it after loss, draw,
abandonment, or successful prize resolution. Do not restore Bag quantities;
deliberately consumed Bag items stay
consumed. Do not save an active run. Loading a save always starts outside an
active challenge with records intact.

Validate `bestTime` bounds before display. Invalid current-version persistent
state follows Wayfarer's existing incompatible-save handling rather than
indexing or formatting unchecked data. Because Wayfarer is unreleased, bump its
save version if adding these fields changes the persistent layout; add no
prerelease migration.

## Challenge start

The receptionist follows this transaction:

1. show rules and the four formats plus Cancel;
2. require two usable non-Egg Pokémon for Double and Mixed, one for Single and
   Knockout;
3. ensure the party snapshot can be allocated;
4. heal the current party;
5. capture the healed run-entry snapshot and initialize the selected built-in
   set and run state;
6. show the final start message; and
7. start the timer and release the player toward 1F.

Failure before step 7 leaves no active challenge. Mixed is checked for two
usable Pokémon because any floor may select Double. Starting a new run never
overwrites a best time.

## Formats and floor initialization

Use `MAX_TRAINER_TOWER_FLOORS = 8`. For each floor, initialize the source row
for the selected built-in set and select the layout and actor arrangement:

- Single uses one opponent and the Single layout.
- Double uses its paired opponents and the Double layout.
- Knockout uses three ordered opponents and the Knockout layout.
- Mixed reads the authored format of that floor and then uses the corresponding
  layout and actor contract.

Freeze the local Mixed sequence as Mixed 1, Mixed 2, Mixed 3, Double 8, Mixed 5,
Knockout 8, Double 3, and Knockout 2. The four source prizes are Up-Grade for
Single, Dragon Scale for Double, Metal Coat for Knockout, and King's Rock for
Mixed.

Entering an uncleared floor stages only its format's actors and triggers.
Winning the required battle or sequence marks that floor cleared and opens the
stairs. Re-entering a cleared floor keeps opponents defeated and the upward path
open for the active run. Floor-cleared state is never copied into ordinary
Trainer defeat flags or a later run.

The elevator asks before abandoning. Confirmation restores the party snapshot,
clears active state, and returns to the lobby. Cancel leaves the run and timer
active.

## Opponent construction

Retain the built-in set's species, move tuple, held item, EVs, IVs, ability,
personality, nickname, friendship, facility class, graphics, speech, and floor
selection. Validate every species, move, item, ability, graphics ID, and Easy
Chat word in the Wayfarer build.

Construct opponent levels through the source Trainer Tower normalization using
the highest-level usable Pokémon in the captured player party. Clamp to legal
levels and apply the same normalized level consistently to all opponents created
for that floor. Do not apply Trainer Rating, ordinary-Trainer scaling, Gym or
League scaling, random roster overrides, rematches, or wild-encounter
predecessor resolution.

Set the facility battle type before party construction. Preserve no-EXP and
no-prize-money behavior, facility AI, facility item handling, opponent speech,
and battle transition. Double mode uses one legal two-opponent battle. Knockout
creates each opponent sequentially while retaining the player's current run HP,
PP, and status.

## Battle outcomes

| Outcome | Result |
| --- | --- |
| Win, more opponents on floor | Continue the same floor without healing |
| Win, floor complete | Mark the transient floor bit and open the stairs |
| Loss or draw | Restore the healed run-entry snapshot, mark the run lost, clear transient state, and warp to the lobby |
| Reset/power loss | Discard the unsaved active run on next boot; preserve prior records |

Facility loss does not deduct money, trigger an ordinary blackout warp, set an
ordinary Trainer flag, advance story, or authorize trainer-only wild mode.
Challenge-specific fainting and Nuzlocke hooks must use the repository's existing
facility exclusion. Focused mechanics tests cover these facility boundaries.

## Timer and records

Use the source Trainer Tower timer representation and frame cadence. Start after
the final start message and stop when the roof owner recognizes completion.
Count overworld traversal, battle time, allowed menus, and Bag use. Preserve only
source-authorized pauses for transitions where the engine cannot safely tick.

Display time in minutes, seconds, and hundredths using bounded arithmetic. On
roof completion:

1. stop and snapshot the final time;
2. compare it with the selected format's established best;
3. write a first or improved record atomically; and
4. retain the result across prize-capacity retries.

The records board lists formats in menu order and never exposes uninitialized or
external-set data. A slower run receives completion dialogue without replacing
the best time.

## Prize transaction and run completion

Resolve the prize from the frozen built-in set. Attempt the correct Bag pocket
before setting a received marker. Whether delivery succeeds or the pocket is
full, restore the healed run-entry snapshot and close the active run on the roof.
On success, give the item immediately. On failure, store its exact item ID in
`pendingPrize`; the lobby receptionist offers that claim before allowing a new
challenge. The player may make room, save, leave the Tower, reload, and retry.
Claiming clears `pendingPrize` only after the Bag transaction succeeds.

Only one pending prize may exist, so a player cannot start another run until it
is claimed. A later successful run may award its source prize again. Never save
a party snapshot, active run, format, or final time merely to preserve a prize.

## Content audit

`wayfarer-sevii-content-audit` retains the shared manifest, script closure, and
transaction checks. Its compact Trainer Tower check requires the local runtime
and built-in set sources, resolves their project-local includes, and rejects an
active external or e-Reader loader path. Compilation and focused mechanics tests
cover C initializer validity and runtime behavior; the audit does not duplicate
the compiler with a custom C parser or freeze per-opponent hashes.

## Validation

Keep focused unit and mechanics coverage for format/floor selection, level
normalization, records and timer bounds, party restoration, battle outcomes,
and pending-prize delivery. Prefer representative boundary cases over a full
cross-product of formats, floors, levels, and item pockets.

Keep one stable emulator journey that drives challenge start and confirmed
abandonment through ordinary lobby input. Focused mechanics tests cover party
restoration, completion gating, records, and pending-prize delivery. Shared
Sevii journeys cover travel and unrelated content; do not duplicate their
catalog coverage or add test-only runtime commands when player input can
exercise the behavior.

Run serial builds because map versions share generated files. The final
production-equivalent Wayfarer release must pass the active ROM reserve.

## References

- [Product requirements](../prds/sevii-trainer-tower.md)
- [Sevii content overlay](sevii-content-overlay.md)
- [Trainer Tower constants](../../game/include/constants/trainer_tower.h)
- [Trainer Tower scripts](../../game/data/scripts/trainer_tower.inc)
- [Trainer Tower implementation](../../game/src/trainer_tower.c)
- [Built-in sets](../../game/src/trainer_tower_sets.c)

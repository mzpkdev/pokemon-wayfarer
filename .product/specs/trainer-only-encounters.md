# Trainer-only wild encounters

PRD: [Trainer-only encounters](../prds/trainer-only-encounters.md)
Implemented: Yes

Status: Implemented for the supported scope below. Selected balance values are
not empirically validated; broader playtesting and excluded exceptional policies
remain open before a shipping-complete claim. See the
[implementation validation](../research/trainer-only-implementation-validation.md).

## Scope

Add a separate Wayfarer encounter mode when the player has no usable battling
Pokémon, whether the party is empty, fainted, Eggs-only, or a mixture without a
living non-Egg. Ordinary party defeat leaves the trainer in the overworld.
Retaliation against the trainer causes actual blackout through shared recovery.

Reuse selected Safari presentation, proximity, inventory and capture functions.
Keep a separate controller and rules; do not copy Safari or build a configurable
battle framework. HNS and Wayfarer Safari share the all-ball proximity correction.
The new mode and party-defeat continuation are Wayfarer-only. FRLG, Emerald and
standalone HNS defeat policies remain unchanged.

This specification owns wild encounters, shared loss/recovery routing and party
eligibility. The [companion story encounter specification](trainer-only-story-encounters.md)
owns ordinary trainer refusal, rival deferral, objective guards, public roadblocks
and their dialogue. Both implement the same PRD and canonical party predicate.

This feature does not create origins, grant equipment, guarantee reaching a
Center, or repair unrelated FRLG Safari behavior. Excluded authored defeat
policies remain explicit in the companion spec.

## Behavior

### Baseline, party state and entry

Source references were inspected at `56fb1e68b4`. This baseline guards battles
without a usable party. Preserve `WayfarerCanStartOrdinaryBattle()` in
[`wayfarer_origin.c`](../../game/src/wayfarer_origin.c): it means at least one
living non-Egg party Pokémon. New eligibility uses the absence of such a Pokémon,
not a literal-empty test. Check all slots through the canonical helper; do not
trust stale party counts or merely sum HP. Existing challenge eligibility still
applies, and a forbidden/dead Pokémon must not become protection through this mode.

| State/context | Required result |
| --- | --- |
| Usable party, ordinary wild | Existing normal battle |
| No usable party, eligible ordinary wild | Trainer-only mode |
| Usable party becomes exhausted in ordinary wild/trainer battle | Finish the loss and return to field; no automatic ordinary blackout |
| No usable party, ordinary trainer sight | No challenge or approach sequence |
| No usable party, talk to ordinary trainer | Refusal message; no battle or reward |
| No usable party, battle-only rival | Silent deferral under the companion spec; preserve chapter and restore later |
| No usable party, objective guard | Visible guard, specified refusal, objective closed and retreat available |
| No usable party, public travel lane | Preserve regional traversal policy; no story completion |
| No usable party, excluded special encounter | Explicit safe guard; no automatic conversion |
| Trainer-only retaliation | Centralized blackout and established recovery |

Eligible contexts are ordinary single-wild encounters: random land including
mass outbreaks, ordinary fishing, and Rock Smash/Sweet Scent when their field
actions are already legally available. This feature grants no field permissions.
Exclude Safari sessions, Bug Contest, facilities, tutorials, scripted/stationary
encounters, roamers, legendary battles, doubles, battle partners and special
capture modes such as unidentified Tower ghosts. Their entry/loss policies need
explicit handling and must not be inferred from party state alone.

Apply eligibility before incompatible encounter generation and again at battle
transition. `CreateBattleStartTask()` must admit only a deliberately initialized
eligible trainer-only context in addition to its existing contexts. Do not remove
the ordinary-party guard globally. Suppress ordinary trainer sight/talk startup
before the emergency `WayfarerAbortEmptyPartyBattle()` fallback: that existing
fallback warps to recovery and is not the intended refusal or bypass behavior.

Audit lead ability, held item, level, Repel and transition readers. With no usable
battler, do not use a fainted member or Egg as a substitute trainer Pokémon. Reuse
normal encounter availability rules where they apply, with explicit neutral
handling for unavailable active-battler modifiers. Preserve species, level, time,
Trainer Rating and encounter weights. Never dereference `PARTY_SIZE` as an index.

### Party defeat, trainer bypass and field continuation

Ordinary Wayfarer wild/trainer losses resolve as losses, not victories: retain
party HP/status, apply existing battle-loss money consequences once, preserve
appropriate challenge cleanup and end the battle. Return at the battle's field
location, with control unlocked and a short retreat/loss message. Do not heal,
withdraw a boxed Pokémon, create a replacement Pokémon, or warp merely because
the ordinary party has no usable member. Do not convert the same battle into a
trainer-only second attempt against its weakened opponent. The next eligible
wild encounter determines its mode from current party state.

The ordinary battle-loss script must stop saying the trainer blacked out. Money
is currently handled in that script before the callback; keep that obligation
separate from relocation. Both loss script and end callback require routing.
Check voluntary forfeits and authored exceptions explicitly; this change does
not convert every loss-like outcome into field continuation.

While there is no usable party, ordinary trainers ignore sight detection. Talking
to one uses the companion spec's small refusal pool with a stable per-trainer
assignment and authored overrides. Hostile optional trainers retain their tone.
This deliberate way to bypass ordinary trainer sight is part of sandbox play.
It does not set trainer-defeated flags, grant rewards, complete battles or open
a gate that requires victory. The trainer remains undefeated after a player loss.
When a usable Pokémon returns, ordinary challenge eligibility resumes. Do not
immediately replay a sight challenge from a still-running loss script; restore
normal field input and trigger evaluation after cleanup.

Trainer field-return routing is opt-in per battle caller, not the default for
every trainer battle type. Maintain an explicit supported-caller allowlist. A
caller enters it only with an explicit loss redirect/retreat handler selected
before the original event script can resume, and after its loss/abort path
releases control safely and all
victory-only continuations check a real winning outcome. Unclassified scripted
callers retain existing defeat routing. Low-level battle-type flags or shared
trainer IDs alone cannot authorize script resumption after a loss.

All unaudited Gym-map callers, including members, Gym Leaders and badge-awarding
callers, are excluded from field-return until
individually adapted, including rematches and every region. This includes current
Viridian Blue; its no-party refusal policy does not authorize new defeat routing.
For example, Morty and Falkner currently place badges/TMs/state writes after
`trainerbattle_no_intro`. A loss must never resume those writers. An adaptation
must gate badge flags, TM/item handoffs, TR/League credit, local story state and
reward dialogue on actual victory, preserving existing reward retry rules. Keep
existing Gym defeat recovery until that caller passes the required audit and tests.

The companion spec applies the completed scene audit to rivals, objective guards
and roadblocks. Implement its caller-specific startup and result handling before
enabling field-return loss for those story battles. Successful battle completion
is an explicit result; a returned or aborted battle call is insufficient. Reuse
this spec's central loss policy and money ownership rather than duplicating them
in map scripts. Multi-trainer/partner scenes, League, facilities and authored
exceptions retain existing loss policy unless the companion explicitly supports
them. Ordinary random encounters still allow trainer-only continuation outside
those exceptional contexts.

Party exhaustion caused in the overworld, including poison, must follow the same
ordinary Wayfarer distinction. Preserve poison damage/messages and challenge
faint cleanup, but do not let `FLDPSN_WHITEOUT` independently warp a player merely
because their last usable Pokémon fainted. Preserve facility and challenge
exceptions. No-usable-party state persists through field steps, menus and reload;
another eligibility check must not reinterpret it as a trainer blackout.

Audit both `field_poison.c` and `data/scripts/field_poison.inc`: the latter maps
`FLDPSN_WHITEOUT` to blackout text and `SetCB2WhiteOut`. Supported ordinary
exhaustion must resolve to `FLDPSN_NO_WHITEOUT` or an explicit non-blackout field
continuation, while `FLDPSN_FRONTIER_WHITEOUT` and challenge/contest exceptions
retain their own routing. Preserve script release and follower updates on the
new continuation; changing only a recovery callback is insufficient.

Existing encounter-immunity steps remain intact. Do not add guaranteed travel,
free healing, safe corridors or mandatory replenishment. The player may reach a
Center, recover a usable Pokémon, or fail and black out. Those are valid outcomes.

### PC and restoring protection

Allow reversible Wayfarer PC Deposit/Move-to-box operations that leave no usable
party, including zero members or fainted/Egg-only remainders. Add the exception
at those callers, not by removing the shared last-mon restriction: retain Release,
Mail, capacity, challenge/dead-Pokémon and non-Wayfarer rules. Trades/daycare are
not included. Safely place or restore a cursor-held Pokémon before PC exit, and
compact/recount after transfers; cancellation must never lose a Pokémon.

The Wayfarer start menu always exposes Pokémon, regardless of party contents
or the starter-received flag. An empty party opens the safe Cancel-only party
screen. This menu check does not set starter or chapter flags; non-Wayfarer
builds retain their existing flag-based visibility.

Outside encounters, existing legal Revives, Centers, healing and withdrawal can
restore protection. Center healing must finish and release field control with
an empty party or an Egg-only party excluded from the healing animation. Show
balls only for actual members included by the existing healing configuration;
fainted members still receive normal healing. Reevaluate the canonical
usable-party predicate after each
party change and on the next encounter. A capture follows normal party/PC delivery:
with six fainted Pokémon it may go to PC and leave the player unprotected. Do not
force a catch into a full party or promise that every capture restores protection.

Inside trainer-only Bag, allow legal non-berry recovery items with a real eligible
party target as well as balls/food. Reuse normal HP, status, revive and applicable
PP recovery effects, target selection and challenge rules. Never heal a
challenge-dead Pokémon through a new exception. A literal-empty party has no
recovery target. Selecting a standard berry feeds the wild Pokémon directly;
there is no Feed/Use submenu or party-medicine use for berries in this mode.
Feeding consumes one berry only when committed and never runs its medicine callback.
After Bag closes, play the existing Pokéblock throw animation unchanged, including
its projectile and wild eating motion. Apply food effects after the animation,
then resolve the single committed turn. Cancelling Bag plays no throw.
This approved choice supersedes the earlier feed-versus-party-use distinction.

Selected transition rule: if a legal recovery action creates a usable party
Pokémon, finish that item action and safely return to field without a capture,
victory or reward. End the trainer-only encounter rather than changing controllers
mid-battle. Revalidate real party state; an item that does not restore a usable
member continues the turn normally. This terminal recovery takes precedence over
anger/flee resolution, like a successful capture. This explicit implementation
choice makes emergency revival useful and avoids partially initialized battles.

Safe recovery also returns field control when an undefeated trainer can see the
return position. Defer those automatic sight approaches until the player moves
out of their sight area; talking to a trainer can still explicitly start a battle.
This transient protection applies only to successful trainer-only item recovery
and does not change trainer defeat flags, rewards or exceptional loss routing.

### Separate controller and state

Use a dedicated controller and transient context, for example
`trainer_only_encounter.c/.h` and `battle_controller_trainer_only.c`. State includes
mode identity, frozen soft cap/wild level, approach counter, initial/current catch
factors, approach escape factor, surviving rock count,
anger, food duration, escape attempts, completed-turn count, warning state and
pending outcome. No
separate trainer HP stat is added. All state is encounter-local and resets on
entry, abort, exit and reload; only normal inventory/party/progression persist.

Initialize an independent discriminator such as `IsTrainerOnlyEncounter()` before
`CreateBattleStartTask()` and battle initialization. It must survive initializers
and remain stable until teardown, even if recovery or capture changes the party.
Do not use `BATTLE_TYPE_SAFARI` as the new mode flag. Do not enter Safari mode,
grant balls, decrement `gNumSafariBalls`, suspend challenges, or install Safari
session callbacks/counters/warps.

Audit Safari-gated controller assignment, party selection, battle-mon initialization,
trainer presentation, healthboxes, send-out suppression, action scheduling, foe AI,
faint processing and end-turn logic. Extract only genuinely shared player-less
behavior. Do not copy an empty/fainted/Egg slot as an active battling Pokémon or
allow ordinary team-loss detection to auto-end the new mode. No regular enemy move
should execute against a nonexistent player Pokémon. The controller owns the first
selectable turn as well as later turns.

Share small functions for trainer/sprite presentation, inventory validation, ball
throwing, capture delivery and proximity adjustment. Keep fear, passive anger,
rocks, escape tuning, retaliation and party-defeat continuation separate from
Safari. Carry recovery cause into the central recovery context before battle
teardown clears encounter state; clear that cause after recovery/reset.

### Menu and inventory

| Position | Action |
| --- | --- |
| Upper left | Rock |
| Upper right | Bag |
| Lower left | Go Near |
| Lower right | Run |

Reuse the normal last-used-ball widget and configured R-button shortcut, including
its ball cycling and visibility setting. A valid shortcut throw consumes one
owned ball and enters the same capture path as Bag. A failed throw spends one
completed turn; unavailable or restricted throws spend neither a ball nor a turn.

Show trainer and wild Pokémon, retaining the wild HP bar. There is no player
Pokémon HP bar, Safari allowance/step readout, or visual fear/anger/proximity
meter. Use ordinary messages and animations for state feedback. Rock hits use
the native hit flash and gradual HP-bar drain before resolving damage. A
surviving hit adds a brief wild-sprite shake alongside the anger marks. Wild
fleeing and successful Run use the normal escape sound and message; like
ordinary wild fleeing, they do not add a sprite exit animation.

Bag reads real inventory. No ball grant, admission fee, throw limit or step limit
exists. Unsupported actions, missing items, invalid targets, capture restrictions
and cancellation spend neither item nor turn and perform no anger/flee/escape RNG.
No balls produces a clear message and returns to the menu; Run can still fail.

A committed item action consumes exactly one item. Existing Bag ball selection
already removes an allowed ball, while some direct throwing actions remove one
again. Choose one owner and prevent repeat callback consumption. Rocks are not
inventory items. Recovery items use their normal targeting/consumption owner.

Use an explicit feedable-berry table containing every standard berry from Cheri
through Maranga, including standard Enigma and excluding the e-reader Enigma item.
Do not infer eligibility from `battleUsage` or pocket membership. Every listed
berry shares the selected food effect, and selection feeds directly. Preserve
global Bag restrictions and normal challenge item rules: the current no-items
rule exempts Poké Balls, not feeding or Revives. Reuse storage/capture/Nuzlocke/type
checks and challenge-dependent PC delivery without Safari exceptions.

### Initial balance and TR scaling

These are selected initial tuning values, subject to measured playtesting. Resolve
`C = GetTrainerRatingSoftLevelCap()` and `L = wild level` once at encounter entry;
clamp valid runtime inputs to at least 1. Do not duplicate the cap table or use
regional badges/raw TR as an invented player Pokémon level. Current anchors are
TR 0 -> cap 15, TR 30 -> 30, TR 55 -> 60, TR 80 -> 100, with the existing resolver
interpolating. Relative danger is `L / C`.

| Parameter | Initial value |
| --- | --- |
| Rock damage fraction of maximum HP | `clamp(C / (8 * L), 1/20, 1/5)` |
| Rock HP damage | Round that fraction of maximum HP up; minimum 1, limited to remaining HP |
| Passive anger per committed nonterminal turn, T | `clamp(ceil(5 * L / C), 3, 15)` |
| Additional anger per surviving rock | 25 |
| Initial anger / retaliation threshold | 0 / 100 |
| Berry anger reduction | 20, floored at zero, before passive anger |
| Approach counter d | Starts at 0, increments up to 3; repeated actions at 3 still commit |
| Initial catch factor F0 | `max(1, floor(speciesCatchRate * 100 / 1275))`, as in HNS Safari |
| Current catch factor F | Starts at F0; each Go Near adds `[4, 3, 2, 1][d]` before incrementing d, capped at 20 |
| Approach escape factor E | Starts at 3; each Go Near adds 4, capped at 20 |
| Owned-ball approach adjustment | `F / F0`; see capture application below |
| Wild flee chance | `clamp(5*E + 5*r - food, 5, 75)` percent |
| r | Surviving rock hits this encounter, capped at 14 (enough to saturate fleeing at initial E with active food) |
| food | 10 while food effect is active, otherwise 0 |
| Food duration | 3 committed turns including feeding; another feeding refreshes to 3, never stacks flee reductions |

Evaluate rock damage using exact bounded rational arithmetic, then one upward
rounding; avoid floating point or intermediate percentage truncation. Use wide
unsigned intermediates and saturation for counters. Damage is based on maximum,
not current HP; rocks can knock out the Pokémon. No accuracy, randomness, STAB,
type effectiveness, critical hits, ability/item damage reactions or move secondary
effects apply. There is no extra rock catch bonus: lost HP affects normal catching.

| L/C | Rock fraction, approximately | Passive anger |
| --- | ---: | ---: |
| 0.5 | 20% | 3 |
| 1 | 12.5% | 5 |
| 1.5 | 8.33% | 8 |
| 2 | 6.25% | 10 |
| 3 | 5% | 15 |

For C=15, L=30, maximum HP=80, a rock deals 5 HP and a nonterminal turn adds 10
passive anger; a surviving rock adds 35 total. At C=30 with the same wild Pokémon,
a rock deals 10 HP and adds 30 total anger. The constants do not add a separate
TR capture penalty: ordinary species, HP, ball and global capture rules still apply.

Fear/flee state and anger are independent. Food lowers anger and temporary flee
risk, but does not erase proximity, rock-count fear or HP damage. Passive anger
applies even after peaceful approaches, failed balls, feeding and failed Run.
There is no time-based gain while reading menus and no automatic anger decay.
Enough food can manage anger at an inventory cost; that is allowed sandbox play.

### Proximity shared with Safari

Reuse HNS/Wayfarer Safari's existing Go Near factor update and action semantics.
The earlier fixed 1/1.5/2/2.5 multipliers and free action at stage 3 are superseded.
Initialize F0, F, E and d as above. Every Go Near first increases F and E, then
increments d if it is below 3. The first three actions use catch increments 4, 3
and 2; the fourth and every later action use 1. Catch and escape factors saturate
at 20 independently. Even with both factors saturated, the action still spends
a turn and receives normal surviving-turn resolution. No free invalid-action
path is introduced at the closest distance.

Retain Safari's existing closer/closest-distance messages and movement counter.
A closest-distance message does not mean that factors stop changing. Trainer-only
Go Near also adds its normal passive anger on every nonterminal attempt, advances
its completed-turn counter, and permits retaliation or fleeing. It adds no separate
rock anger. Berry feeding preserves F, E and d; its selected temporary fear and
anger effects still apply. The rock/berry overlay and 5..75% final flee bounds
belong to trainer-only mode; Safari retains its own actual flee calculation,
Pokéblock effects and visit rules. Sharing approach updates does not copy the
whole Safari encounter loop.

For HNS/Wayfarer Safari Balls during actual Safari visits, preserve the existing
factor-to-catch-rate conversion and rounding exactly, including repeated approaches.
Do not multiply those odds by another approach bonus. For other owned balls in
Safari, and all legal balls in trainer-only mode, use `floor(speciesCatchRate * F / F0)` as the effective species catch rate
before the existing flat ball bonus, minimum-rate clamp and HP/ball/status/global
odds calculation. Preserve that calculation's order and rounding. At F=F0 the
adjustment is exactly neutral. Use wide intermediates; do not cap the effective
rate at 255 or narrow it to an 8-bit species field. Clamp final capture odds to
the existing guaranteed-capture boundary. Guaranteed-capture early returns remain unchanged. A Safari Ball used
outside a Safari visit follows that mode's ordinary ball path, not Safari's
flattened species-rate path.

F0 remains the positive encounter-entry reference for this ratio. HNS Pokéblocks
alter escape behavior, not the catch factor; preserve that existing behavior.
FRLG bait/rock resets and factor rules remain outside this change. For example,
a species with F0=5 has factors 5,9,12,14,15 after zero through four approaches;
the owned-ball adjustment is 1,1.8,2.4,2.8,3 respectively. Factor saturation gives
a species-dependent maximum benefit, rather than the discarded universal 2.5x
cap. Existing Safari Ball quantization remains specific to its original path.

Ball effects needing an active player Pokémon skip only the unavailable comparison
and retain independent bonuses. Do not use fainted party stats, Eggs or C as a
fictional active Pokémon for Level/Love Ball or other player-dependent bonuses.
Independent badge/global rules remain in force. C is a reference for the
explicitly designed rock, anger and escape mechanics only.

### Capture context and ball turn count

Pass an explicit capture context through `ComputeBallData()` and
`ComputeCaptureOdds()`, including whether a valid active player battler exists.
Trainer-only mode sets `hasPlayerBattler=false`; player-dependent branches must
check it before indexing battle data. Do not pass a dummy index, create a fake
battler, or substitute the TR cap or fainted party stats. Existing normal battles
keep their real battler context and existing behavior.

| Modifier | Trainer-only behavior |
| --- | --- |
| Level Ball player-level comparison | Start from neutral (1x) for the absent comparison; retain the independent HNS wild-type bonus where applicable. |
| Love Ball player-dependent species/sex comparisons | Start from neutral (1x) for those conditions; retain the independent HNS wild-type bonus and never inspect an absent battler. |
| Gen 8 missing-badge malus requiring player level below wild level | Omit this comparison-dependent malus when no player battler exists. |
| Gen 9 missing-badge malus | Retain the existing badge-count/wild-level calculation, which needs no player battler. |
| Wild-level, species, HP/status, environment, Pokédex and other independent ball/capture effects | Retain their normal conditions, rounding and restrictions. |
| Guaranteed capture | Preserve existing permitted guaranteed-capture behavior. |

This explicitly narrows the earlier neutral-fallback rule: only effects requiring
missing player data are neutralized. It is not a general badge or challenge
exemption. The shared Safari proximity change does not opt Safari into these
trainer-only capture-context or turn-counter changes.

The trainer-only controller owns an encounter-local completed-turn counter,
initialized to zero before the first menu. Pass its value explicitly to the ball
calculation, or synchronize the existing capture counter through one owner; do
not also increment it through ordinary battle end-turn callbacks. Quick Ball uses
its normal first-turn bonus only while the counter is zero. Timer Ball uses the
existing configured formula and cap with this counter; do not invent new tuning.

A ball reads the counter before its action resolves. After each committed turn
that leaves the encounter ongoing, increment once before the next menu, with
saturation at 255 to prevent wraparound. Rocks, valid approaches, feeding, failed balls,
failed Run and legal recovery actions that leave the encounter ongoing all count.
Menus, cancellations and invalid actions never count. Terminal actions need no
further increment. Thus an immediate Quick Ball qualifies, Go Near followed by a
Quick Ball does not, and a Timer Ball after two completed actions reads two.
No actor animation or callback advances the counter a second time.

### Escape and warnings

Run attempts use the ordinary escape success/failure presentation. With entry
snapshots C and L clamped to at least 1, the success chance in percent is
`clamp(floor(50 * C / L) + 15 * priorFailedAttempts, 5, 95)`. Use wide unsigned
arithmetic. Prior failures start at zero and count only committed failed Run
attempts. There is no eventual guarantee: even at 95%, Run can fail. No added
terrain/trapping rules are in scope.

Audit `TryRunFromBattle()` and `IsRunningFromBattleImpossible()` separately: both
currently read player Pokémon state. Extract only shared escape result/attempt
handling. Preserve applicable global no-running flags and the existing
less-escapes challenge restriction. The selected ordinary formula grants no
challenge exemption. Do not call the normal Speed formula against zeroed data,
inherit active-Pokémon abilities/items, or invent player stats for a challenge.

A committed Run rolls escape once. Success ends immediately. Failure prints
"Can't escape!", increments the attempt history once and consumes a turn with
passive anger and the normal surviving-turn wild-flee check. Menus/cancellation
never advance attempts. Player escape and wild fleeing are distinct rolls; there
is never a second escape roll for a single selected Run.

Before returning to each action menu, compute the warning from current anger and
the largest possible next-turn increase, `25 + T`. Warn if anger is at least
`min(75, 100 - (25 + T))`. This ensures a menu and clear "ready to attack" message
occur before any nonterminal action can cross 100. Reissue the warning as needed
if feeding lowers danger and it later rises again. This is warning, not immunity:
Run or a ball may fail, allowing retaliation that turn. Do not expose numeric meters.

Retaliation is deterministic at the threshold for a surviving, still-active wild
Pokémon. Peaceful catching remains possible, not risk-free. The final warning
must not be phrased as though only another rock can trigger the attack.

### Turn resolution

Resolve a committed action once, in this order:

1. Validate action/item/target, then commit the action and inventory once.
2. Run rolls escape; a success ends. Otherwise apply rock damage, attempt capture,
   feed, approach or apply recovery as selected. A failed Run shows its failure text.
3. Successful capture, lethal rock or recovery of a usable party member ends
   immediately; no anger increase, retaliation or flee roll follows.
4. For a surviving rock, increment r and add 25 anger. Feeding subtracts 20 anger
   with floor zero and refreshes food to 3. Other action-specific state changes
   are already applied. Then add T passive anger, saturating at 100.
5. If anger reaches 100, play retaliation and enter centralized blackout. It takes
   priority over wild fleeing. The prior-menu warning invariant must already hold.
6. Otherwise perform exactly one wild-flee check with the updated state. If it
   succeeds, return to field.
7. If still active, decrement food duration once, emit appropriate feedback and
   warning, then return to action selection. Feeding's own check is the first of
   its three protected turns. Cancels and invalid actions never tick duration.

| Terminal outcome | Result |
| --- | --- |
| Capture | Normal challenge-aware party/PC delivery, then field return |
| Run success or wild flee | Field return at encounter location |
| Rock knockout | Field return; zero EXP, EVs, money, loot or wild-victory progression |
| Recovery item restores protection | Field return with actual item/party changes; no victory reward |
| Retaliation | Trainer attacked, then centralized blackout/respawn |

After every terminal outcome, mode for the next encounter depends on actual usable
party state. Captures sent to PC do not grant protection. Do not record a Pokémon
faint for retaliation or a trainer victory for a rock knockout. No Safari
out-of-balls outcome may leak into the new mode.

### Central loss and blackout policy

Separate party defeat from trainer blackout in the central loss/recovery routing.
Use explicit causes and an outcome context, including whether battle-loss money
and challenge cleanup already ran. Ordinary supported wild/trainer party defeat
returns to field; trainer retaliation invokes the shared blackout chain. Neither
path owns a second destination resolver, challenge implementation or money formula.

The existing recovery chain is `CB2_WhiteOut()` -> `DoWhiteOut()` ->
`SetWarpDestinationToLastHealLocation()` -> `WayfarerEnsureRecoveryDestination()`.
Retain saved heal-location selection, healer transforms and origin fallback before
the first Center visit. No nearest-Center search or mode-owned recovery destination
is added. A trainer blackout still recovers normally even after unsuccessful
attempts to walk to a Center. An origin needs a valid existing recovery destination;
travel success or guaranteed supply replenishment is not a release prerequisite.

Use cause-aware injury text and common cleanup/map loading. The party may actually
be fainted or empty: heal real eligible members through shared recovery, but avoid
claims about nonexistent members and empty-party nurse animations. Clear the
recovery cause after use so an ordinary defeat never inherits retaliation state.

Current money loss runs in battle script `getmoneyreward`, not `DoWhiteOut()`.
Retain that existing charge exactly once for ordinary battle losses even without
relocation. Never rerun it merely during field-return cleanup. Trainer retaliation
deducts no money, including when it follows an earlier ordinary party loss.
Central recovery must not invoke the ordinary loss charge for retaliation.

Challenge faint processing must remain active on field-return paths. In particular,
`NuzlockeDeleteFaintedPartyPokemon` occurs in battle teardown independently of
whiteout. Do not revive challenge-dead members, bypass death flags or silently
change hardcore loss conditions just by moving callback selection. Current actual
blackout can clear/reset a hardcore save or move a boxed mon/create a Rattata in
other Nuzlocke modes; current recovery also ends League runs. Their relationship
to continued exploration and actual trainer blackout must be explicitly approved
before those configurations ship. Until then preserve existing exceptional loss
policy rather than silently applying ordinary sandbox continuation there.

### Presentation and persistence

Use normal messages for nervousness, anger, feeding, distance, failed escape and
the impending attack. Rock animates a throw/impact and HP change, then a surviving
Pokémon reaction. The selected reaction uses two existing anger-mark effects
anchored to the real wild sprite, waiting for the animation before anger text and
turn resolution. Lethal rocks skip this reaction. Do not execute a move or its full
script with nonexistent Pokémon targets. Retaliation visibly attacks the trainer
before the shared fade. Validate animation anchors and localized text in-game.

Persist ordinary inventory, party and existing progression only. Save/reload while
unprotected must stay unprotected until real recovery; do not heal, teleport, grant
a starter or resume an expired encounter context. Clear contexts on all terminal
paths, load/reset and debug abort. No persistent fear/anger or save migration is
needed. Existing origins/supplies stay unchanged; new origin content remains out
of scope. Blackout is an intended fallback, not a failure of the feature.

## Integration and source evidence

These are existing integration surfaces, not claims that the new mode exists.

| Surface | Existing owner and required integration |
| --- | --- |
| Wild eligibility and generation | [`wild_encounter.c`](../../game/src/wild_encounter.c): Standard, Rock Smash, Sweet Scent and fishing readers; lead modifiers and double generation |
| Party guard and origin fallback | [`wayfarer_origin.c`](../../game/src/wayfarer_origin.c): retain `WayfarerCanStartOrdinaryBattle`; reuse recovery validation |
| Safe abort and transition | [`wayfarer_battle_gate.c`](../../game/src/wayfarer_battle_gate.c), [`battle_setup.c`](../../game/src/battle_setup.c): explicit eligible mode, clean aborts and separate callback |
| Trainer guards | [`trainer_see.c`](../../game/src/trainer_see.c), [`battle_special.c`](../../game/src/battle_special.c), [`scrcmd.c`](../../game/src/scrcmd.c): preserve excluded startup protection |
| Controller and init | [`battle_controller_safari.c`](../../game/src/battle_controller_safari.c), [`battle_controllers.c`](../../game/src/battle_controllers.c), [`battle_main.c`](../../game/src/battle_main.c): extract selected trainer presentation; guard party indexes |
| Item consumption | [`item_use.c`](../../game/src/item_use.c): `ItemUseInBattle_PokeBall` already removes an allowed ball; Safari Bag callbacks additionally decrement allowance, while `HandleAction_ThrowBall` in battle_util also removes an item |
| Catching and proximity | [`battle_script_commands.c`](../../game/src/battle_script_commands.c): `ComputeBallData`, `ComputeCaptureOdds`; [`battle_util.c`](../../game/src/battle_util.c): `HandleAction_GoNear`; [`battle_ai_main.c`](../../game/src/battle_ai_main.c): Safari flee logic |
| Safari-only limits | [`safari_zone.c`](../../game/src/safari_zone.c), [`battle_scripts_2.s`](../../game/data/battle_scripts_2.s): keep zero-allowance checks and entrance warps out of the new mode |
| Capture delivery | [`pokemon.c`](../../game/src/pokemon.c): `GiveCapturedMonToPlayer` already supports an empty slot and challenge-dependent PC delivery |
| Deliberate empty party | [`pokemon_storage_system.c`](../../game/src/pokemon_storage_system.c): scoped Deposit/Move-to-box exception; preserve shared last-mon Release guard |
| Berry data | [`items.h`](../../game/src/data/items.h): ordinary berries vary in medicine callbacks and battle usability; add explicit feeding eligibility |
| Animation primitives | [`battle_anim_scripts.s`](../../game/data/battle_anim_scripts.s): Rage and Leer need target/side-effect audit |
| Shared recovery | [`overworld.c`](../../game/src/overworld.c), [`field_screen_effect.c`](../../game/src/field_screen_effect.c), [`event_scripts.s`](../../game/data/event_scripts.s): cause-aware common recovery and presentation |
| Party loss callbacks/scripts | [`battle_setup.c`](../../game/src/battle_setup.c): `CB2_EndWildBattle`, `CB2_EndTrainerBattle`, `CB2_EndRematchBattle`; [`battle_scripts_1.s`](../../game/data/battle_scripts_1.s): `BattleScript_LocalBattleLost` money/text before callbacks; preserve victory-only flags and authored branches |
| Field exhaustion | [`field_poison.c`](../../game/src/field_poison.c): `AllMonsFainted`, `Task_TryFieldPoisonWhiteOut`; [`field_poison.inc`](../../game/data/scripts/field_poison.inc) and [`field_control_avatar.c`](../../game/src/field_control_avatar.c): preserve poison/challenge cleanup while routing ordinary exhaustion to continued field play |
| TR scaling | [`trainer_rating.c`](../../game/src/trainer_rating.c): `GetTrainerRatingSoftLevelCap`; snapshot rather than duplicate obedience-cap data |
| Escape | [`battle_util.c`](../../game/src/battle_util.c): `TryRunFromBattle` and `runTries`; [`battle_main.c`](../../game/src/battle_main.c): `IsRunningFromBattleImpossible`; adapt result/attempt handling without reading player battler stats |

## Validation

Use mechanics tests plus emulator journeys with both a genuinely empty party and
real fainted/Egg-only parties. Do not substitute a zeroed battling Pokémon fixture
for exercising the new controller and field-return routing.

| Area | Required cases |
| --- | --- |
| Eligibility | Empty, fully fainted, Eggs-only and mixed unusable parties enter eligible wild mode; one usable member restores normal battles; land/outbreak/fishing/legal field-action readers agree |
| Exclusions | Partner, double, scripted, ghost, facility and tutorial guards; no invalid initialization or accidental victory continuation |
| Party loss | Wild and ordinary trainer losses return to current field without healing/warp; one money charge; preserved faint state; no mid-battle trainer-only second chance |
| Field exhaustion | Last member faints to poison; challenge cleanup preserved; no ordinary poison-step blackout; subsequent encounters and reload remain unprotected |
| Trainers | No sight battle without protection; generic talk refusal without recovery warp; intentional bypass; no defeated flag/gate unlock; reenable after recovery; no replaying loss scene |
| Gym losses | Every Gym Leader/badge caller defaults to existing defeat routing; opt-in adapters prove loss/abort cannot grant badge, TM, TR/League credit or story progress, and victory/reward retry still works |
| Capture context | No player reads in trainer-only Level/Love and Gen8 malus paths; Gen9/independent bonuses retained; normal and Safari regressions; Quick first action and Timer after0/1/2 turns; cancellation invariance and no double counter increment |
| Story integration | Companion-spec rival restoration, guard dialogue, public lanes, victory-only writers and safe loss retry pass; excluded League/facility/partner outcomes retain explicit policy |
| Storage | Final usable/last member Deposit/Move-to-box; safe fainted/Egg remainders; cursor cancel/count/compaction; retain Release/Mail/capacity/challenge and non-Wayfarer rules |
| Engine | Mode survives initialization; no phantom send-out, automatic team defeat, invalid party index or regular move against trainer |
| Bag | All standard berries feed directly, including Enigma and excluding e-reader Enigma; no Feed/Use submenu; balls and legal non-berry HP/status/revive/PP recovery targeting; cancel/blocked/no-item states; exactly one consumption; no empty target or challenge revive bypass; no Safari counters |
| Scaling | Shared cap resolver across all anchors/intermediate TR; C/L frozen; damage clamping/rounding/minimum/lethal HP; anger ceil/clamp; wide arithmetic at extreme levels/HP |
| Balance examples | C15/L30/HP80 -> damage5/T10; C30/L30/HP80 -> damage10/T5; low-level cap20% and high-level floor5%; at-cap passive5 plus rock25 |
| Peaceful pressure | Failed balls, valid Go Near, feeding and failed Run add passive anger; menus/invalid actions do not; no rock prerequisite for retaliation |
| Warning | Largest-next-gain lookahead at all T values; +40 rock cannot skip prior warning; post-berry warning reentry; failed escape can trigger warned retaliation |
| Run | Approved formula at equal/lower/higher L/C; +15 per prior failure; bounds5..95 and no guarantee; cancellation invariance, one roll/history update, failure text/turn, success terminal; global/less-escapes restrictions and no active-Pokémon modifiers |
| Food/fear | -20 anger then T; feed may manage anger at item cost; -10 flee for3 checks incl feeding; refresh not stack; preserve damage/proximity/rock fear; flee bounds5..75 |
| Proximity | F0 floor/min1; increments4/3/2/1 then repeated1; E increments4; factor caps20; stage3 remains committed; Safari Ball legacy odds unchanged; owned-ball effective species rate F/F0 neutral at entry, floor before flat ball bonus, wide arithmetic/guarantee/no double bonus; F0=1/5/20 and repeated attempts after saturation |
| Outcome ordering | Capture, rock KO, successful Run and successful revival terminate before anger; retaliation before flee; at most one wild-flee check; no terminal double-resolution |
| Capture/revival | Slot/PC/count/Dex/nickname; full six-fainted catch toPC remains unprotected; challenge PC routing; valid revival safely ends encounter and next battle normal |
| Recovery | Retaliation uses existing current heal point/origin fallback/healer transform; true empty and fainted party text/healing; contexts cleared; retaliation deducts no money, ordinary loss charges once |
| Exceptions | Approved challenge/hardcore/money/League policy tested at party defeat and trainer blackout separately; no accidental deletion, replacement gift or exemption |
| Regression | Safari grants, allowance30/steps500/exit/retirement/Run/Pokéblocks and Go Near turn/RNG/factor behavior unchanged; only owned non-Safari balls gain the approach adjustment; Bug Contest; standalone HNS/FRLG/Emerald defeat policies |
| Persistence/UI | Unprotected reload, recovery then normal battle, Bag targets/animations, field controls and sprites; no meters or stale callbacks |

Build Wayfarer and HNS sequentially because map generation shares files. Compile
other affected conditional builds. Use existing mechanics and prebuilt-ROM emulator
harnesses and record revision/configuration. Numeric tests cannot validate animation
anchors, field return, trainer trigger behavior or recovery presentation alone.

## Open questions

1. **Exceptional defeat/recovery policy:** companion-spec excluded authored callers,
   challenge/hardcore consequences at party defeat versus trainer retaliation,
   Nuzlocke replacement behavior and League-run termination. Central ownership,
   ordinary field continuation and no retaliation money charge are settled; no
   silent challenge exemption is approved.
2. **Broader playtesting:** validation of the selected balance values. The
   implemented messages, anger marks and retaliation presentation are recorded
   with emulator evidence in the implementation validation report.

Rock/TR scaling, passive anger, warning lookahead, proximity, food/flee and Run
values above are selected. Berry eligibility and direct feeding are settled. Tune
them together using the acceptance examples and record any balance revision
explicitly. Unresolved items prevent claiming a
shipping-complete implementation, not updating this design or building isolated
infrastructure.

## References

- [Trainer-only story encounters](trainer-only-story-encounters.md): scene policy,
  no-party dialogue, objective gates and recoverable rival chapters.

- [Party progression and obedience](trainer-rating-party-progression.md): shared
  TR-derived soft cap; no duplicate cap table.
- [Regional start choice](wayfarer-regional-start-choice.md): origin fallback.
- [Runtime foundation](wayfarer-runtime-foundation.md): Wayfarer runtime scope.
- [Interregional League circuit](wayfarer-interregional-league-circuit.md):
  authored defeat and recovery interactions.

# Trainer-only wild encounters

Status: Supported implementation complete. Broader balance validation and the
exceptional challenge/League policies remain open. Future ports are separate work;
scripted-wild exclusions and their safe no-party handling are settled. See the
[implementation validation](../research/trainer-only-implementation-validation.md).

## Intent

Let a Wayfarer player with no usable party Pokémon encounter and catch a wild
Pokémon directly. The player can approach cautiously, offer food, throw a ball
immediately, or hurt the Pokémon with rocks. Catching without violence must remain
a complete strategy.

This supports both an empty-party opening and later sandbox play after depositing
the entire party, and continued exploration after the party faints. It does not
create a new origin or grant starter supplies.

## Design

### Eligibility and menu

An ordinary wild encounter, including fishing, uses this mode when the party
has no usable Pokémon under the shared battle-eligibility predicate. This includes
an empty party, a fully fainted party, and an Eggs-only party. Later encounters
return to normal battles when the party has a usable Pokémon. The encounter shows
the trainer and wild Pokémon without sending out an unusable party member.

Wayfarer's PC permits depositing or moving the final usable party Pokémon into a
box, including leaving fainted Pokémon or Eggs behind. Retain last-Pokémon release
protection, Mail and challenge restrictions. Cancelling a move safely restores
the Pokémon.

Losing an ordinary wild or trainer battle applies the existing loss penalty once
and returns the player to the field without healing or a recovery warp. It does
not convert the ongoing battle into this mode.
The next eligible wild encounter uses this mode if the party remains unusable.
Ordinary field poison that exhausts the party also leaves the trainer in the field;
party exhaustion alone must not trigger a delayed blackout on the next step.
Authored outcomes and challenge consequences need explicit treatment before this
field-return rule replaces their behavior.

Ordinary trainers ignore sight encounters while the player has no usable party.
Talking to them uses one of a small set of refusal lines, assigned consistently
to that trainer, with character-specific overrides. This intentional opportunity to walk past trainers grants no victory flags, rewards,
or gate unlocks. Trainers become available again when the party is usable.

### Rivals, objective guards and open travel

Encounter policy follows each scene's purpose. Rival battles can wait; local
rescues, keys, passwords and boss victories still need to be earned. Public
travel remains open wherever the regional independence and traversal designs
require it. Going without usable Pokémon grants no story completion.

Off-screen battle-only rivals do not spawn or trigger while the player has no
usable party. Persistent battle-only rivals are temporarily hidden, including
their collision. Suppression is silent. When the party recovers, pending chapters
become available at their existing locations under their narrative prerequisites
and the owning campaign's explicit retirement rules. No-party suppression itself
never advances or retires a chapter.
Completing another adventure, ending an occupation or departing on a ship must
not erase a pending rival chapter. Shared town-state variables need to be split
where they currently tie rival completion to that adventure.

A rival who also gives an independently available item or participates in a
non-battle scene keeps that interaction. If a visible rival's battle interaction
cannot start, use a short refusal appropriate to the character. Hiding every
appearance of a rival is not the policy.

Objective guards remain visible and protect their local objective. They refuse a
challenge when the player has no usable Pokémon and leave room to retreat. A
Rocket guarding a password or hostage can keep that objective unavailable; a
Rocket on a public road cannot reinstate a removed regional travel gate. Hostile guards
reuse suitable existing threats or dismissals, with short character-appropriate
variants where needed. They do not invite the player back for a fair battle or
give a key, reveal a password or imply victory. Friendly hosts retain readiness
refusals. Ordinary refusal does not attack or black out the
trainer.

This covers the audited Well, Mahogany, Radio Tower, Theater, Hoenn rescue and
villain encounters, plus local challenge rewards. The companion spec also records
future FRLG/Sevii rival and objective policies for those ports. Source availability
does not bring those maps into the current release. Gym and League objectives
retain their explicit entry and defeat rules. Gym losses keep existing recovery
until each badge-awarding caller is individually adapted and verified to grant
no badge, TM, progression or victory credit on defeat.

For supported story battles, loss returns control safely with the objective still
unresolved. Victory-only script continuations must check the actual result before
removing guards, handing over rewards or advancing story state. Authored tutorials,
chained/partner battles, League and challenge outcomes retain their existing loss
policy until a specific adaptation is approved.

The [story encounter specification](../specs/trainer-only-story-encounters.md)
owns the per-scene policy, no-party dialogue, safe retreat and recoverability
requirements. Its regional source mapping comes from the
[encounter gate audit](../research/trainer-only-story-encounter-gates.md).

### Encounter menu

The four actions occupy the familiar battle menu positions:

| Normal battle position | Trainer-only action |
| --- | --- |
| Move | Rock |
| Pokémon | Go Near |
| Bag | Bag |
| Run | Run |

There is no admission fee, granted ball supply, step limit, or throw allowance.
The player uses balls and berries actually owned in the Bag.

### Actions

| Action | Required behavior |
| --- | --- |
| Rock | Deal HP damage, increase flee risk, and build anger. Damage can knock out the Pokémon. There is no separate rock catch bonus: lower HP already improves catching. |
| Go Near | Use existing Safari approach behavior: increase catch and flee factors, including repeated attempts at the closest distance. Apply the catch benefit to every ball. |
| Bag: ball | Consume one owned, usable ball and attempt a normal capture with the proximity benefit. Preserve the ball's ordinary effects where applicable. |
| Bag: berry | Selecting any standard berry feeds it directly, consuming one to temporarily reduce flee risk and partially lower anger. Feeding does not heal rock damage or instantly reset anger. |
| Run | Attempt escape. Success depends on wild level relative to the Trainer Rating softcap and improves with repeated attempts. A failed attempt consumes a turn. |

Performing a valid action consumes a turn. Opening or cancelling the Bag,
selecting an unusable item, or attempting an unavailable action consumes neither
a turn nor an item. The Bag must clearly identify usable encounter items and
must safely target existing party members for applicable non-berry recovery items,
including HP/status medicine, revives and PP restoration,
without treating an empty party slot as a target. Restoring a usable party member
ends this encounter and returns to the field; later encounters use normal battles.
Trainers watching that return position allow the player to leave their sight area
before approaching again; the player can still talk to them to start a battle.
Every standard berry from Cheri through Maranga is feedable, including standard
Enigma and excluding the e-reader Enigma item. Use an explicit list. Berry selection
feeds directly without a Feed/Use submenu or party-medicine option in this mode.

With no balls left, the player can still run. Attempting to select a ball explains
that none are available without advancing the encounter. Food is optional; the
player does not need a Pokéblock Case or berries to attempt a catch.

Ball effects that need an active player Pokémon use a neutral fallback for that
modifier alone. Other applicable effects still work. The game must not invent a
substitute Pokémon level or treat an empty slot as a battling Pokémon. Level/Love
Ball comparisons use a neutral bonus. The Gen 8 badge penalty requiring a player
level comparison is omitted; independent badge rules remain. Quick and Timer
Balls count completed committed actions, starting at zero; menu navigation and
invalid actions do not consume their window.

### Fear and anger

Fear controls fleeing; anger controls retaliation. They are separate internal
values, with feedback through ordinary encounter messages and animations only.
Neither has a visible meter or numeric readout.

Approaching and rocks raise flee risk. Food temporarily lowers it. Anger builds
passively after every committed action that leaves the encounter ongoing,
including peaceful actions and failed escape attempts. Rocks add further anger.
Feeding lowers anger before that turn's passive increase. Catching without rocks
is supported, but remaining in an encounter is never free of retaliation risk.

Anger has a defined retaliation threshold. Messages warn before an action can
cross it, taking current anger and the possible action increases into account.
The warning must not promise safe escape: Run can fail. Reaching the threshold
triggers retaliation if the encounter is still ongoing and the Pokémon survives.
Successful capture, knockout and successful escape end the encounter without a
passive anger increase or subsequent retaliation.

### Outcomes

| Outcome | Result |
| --- | --- |
| Capture succeeds | Deliver the catch through the normal capture flow and return to the overworld. |
| Pokémon flees | Return to the encounter's overworld location. |
| Player successfully runs | Return to the encounter's overworld location. |
| Rock knocks out the Pokémon | End the encounter and return to the overworld, with no EXP or rewards. A knocked-out Pokémon cannot retaliate. |
| Surviving Pokémon retaliates | Show that it attacks the trainer, then enter the shared blackout and recovery flow. |

Retaliation uses the existing centralized logic for all faints, including its
established respawn destination and recovery rules. It must not calculate a
separate nearest Pokémon Center or maintain another respawn policy. Text explains
the trainer's defeat instead of claiming that their party caused this particular defeat.

## Boundaries

The trainer-only mode is Wayfarer-specific. Ordinary eligible wild encounters are
the initial scope. Trainer battles must safely refuse to start without a usable
party. Scripted and fixed encounters, tutorials, partner or multi-Pokémon battles,
Safari visits, and the Bug-Catching Contest do not automatically opt into this
mode. Random land encounters include outbreaks. Rock Smash and Sweet Scent
encounters qualify when their field actions are already legally available; this
feature grants no new field permissions. Excluded encounters' existing eligibility
or a deliberately specified guard must handle an unusable party safely.

Scripted-wild exclusion is a settled product rule, not a deferred conversion.
Route 120 Steven/Kecleon requires a usable party: without one, give the specified
safe, retryable refusal and grant neither the Scope nor bridge completion.
Mahogany Electrode and the future Tower Marowak/Lostelle scenes retain their
existing scripted-wild eligibility and completion rules. Their exclusion does
not promise that unported scenes are already available.

The feature does not bypass encounter availability, progression gates, normal
challenge restrictions, or capture restrictions. Walking past inactive ordinary
trainers is intentional, but does not satisfy any requirement to defeat them.
Safari-specific challenge exceptions belong to Safari visits.

Safari retains its action set, admission, ball grants, step limit, total throw
allowance including owned balls, and exit behavior. The intended shared change is
that Go Near benefits every ball in HNS/Wayfarer Safari encounters too. Safari
keeps its existing turn cost, factor updates and flee checks even at the closest
distance. Existing Safari Ball odds remain unchanged. This does
not redesign FRLG bait and rock mechanics or add anger and retaliation to Safari.

No new origin, starter gift, berry distribution, shop, encounter table, trainer HP
system, or save migration is included in this feature.

## Balance

Approaching offers a gradual, non-damaging way to improve capture odds. Rocks
trade damage for a higher chance of losing the encounter through fleeing,
knockout, or trainer defeat. Food spends inventory to keep the encounter going
and reduce anger without erasing the physical consequences of earlier rocks.

Species catch rates and ball effects remain meaningful. Go Near uses Safari's
existing factor limits, not a separate three-action or 2.5x cap. Every attempt
spends a turn, including attempts after the closest-distance message or factor
saturation. Trainer-only passive anger continues on those turns. Do not apply
the approach benefit twice to Safari Balls.

The following are selected initial playtest defaults. They are tuning values, not
claims that balance has been validated. Let `C` be the Trainer Rating soft level
cap and `L` the wild Pokémon's level, both fixed for the encounter at entry.

| Mechanic | Initial default |
| --- | --- |
| Rock HP damage | `max(1, ceil(maxHP × clamp(C / (8 × L), 0.05, 0.20)))` |
| Passive anger per ongoing turn | `clamp(ceil(5 × L / C), 3, 15)` |
| Extra anger from a rock | 25 |
| Retaliation threshold | 100 anger |
| Berry anger change | Reduce by 20, floored at zero, then apply passive anger |
| Base flee chance | 15% |
| Proximity flee change | Safari escape factor starts at 3 and gains 4 per attempt, capped at 20; contributes factor × 5 percentage points before trainer-only rock/food effects and final bounds |
| Rock flee change | +5 percentage points per rock |
| Food flee change | -10 percentage points for three turns; feeding refreshes duration without stacking |
| Final flee chance | Clamp to 5% through 75% |
| Proximity catch change | Safari catch factor gains +4, +3, +2, then +1 for every further attempt, capped at 20; owned-ball benefit scales by current/initial factor |

The flee defaults above belong to trainer-only encounters. Safari shares the
all-ball proximity catch benefit while retaining its visit rules. Run succeeds
with probability `clamp(floor(50 * C / L) + 15 * priorFailedAttempts, 5, 95)` percent,
using the entry softcap C and wild level L, each at least 1. Failed attempts consume
a turn; even repeated failures never guarantee escape. Preserve global no-running
and existing less-escapes challenge restrictions. Invalid actions and menu
cancellation advance none of these values.

## Presentation

A rock action shows the throw and impact, then updates the wild Pokémon's HP. A
survivor reacts with an existing animation suitable for anger and a short message.
Reusing an animation must not execute that move's gameplay effects. Animation
selection requires an in-game preview.

Messages distinguish getting nervous, getting angry, calming down, and being
ready to attack. The final warning must be understandable without exposing hidden
numbers. Reaching the closest allowed distance also gets a clear message.

Retaliation visibly connects the Pokémon's attack to the trainer's blackout,
followed by the normal fade and recovery presentation with appropriate text.

## Interactions

The ordinary battle guard still protects unsupported encounters. Eligible wild
encounters select this mode using the same no-usable-party predicate that prevents
ordinary trainer battles.

Capture delivery uses existing party and PC capacity rules. With six fainted
Pokémon, a catch can go to the PC and leave the party unusable. A successful catch
therefore does not promise immediate recovery. A usable catch added to the party,
or a successful revive, restores normal eligibility for later encounters.

Players are not guaranteed to reach a Pokémon Center or obtain repeatable balls
without risk. Retaliation and the established blackout fallback provide recovery
when needed. A valid centralized respawn destination must exist even before the
first Pokémon Center visit. Starter supplies, new origins, and economy changes are
outside this feature; repeatable ball supply is not a release prerequisite here.

Challenge handling remains centralized. If an existing rule assumes a usable
party member exists, resolve that case through the shared rule rather than
importing Safari exemptions. Ordinary party defeat and trainer retaliation are
distinct reasons within that policy: ordinary defeat returns to the field, while
retaliation invokes blackout and recovery, subject to the explicit challenge and
authored-outcome decisions below. Retaliation deducts no money. Ordinary supported
battle losses retain their existing money charge exactly once.

## Constraints

Implement a distinct encounter mode and controller with its own actions and
outcomes. Share selected Safari building blocks for trainer presentation, ball
selection and throwing, capture delivery, and proximity calculations. Do not
copy the Safari subsystem wholesale or build a configurable battle framework.
Admission, visit counters, ball grants, and entrance warps remain Safari-owned.

Blackout destination and recovery policy remain centralized for every faint
reason. The technical integration contract belongs in the linked specification.

## Playtesting

Acceptance examples:

- An empty-party player approaches and catches a Pokémon using an owned ball,
  without throwing rocks. Each unsuccessful committed action builds passive anger.
  If the catch enters the party usable, the next encounter uses normal battles.
- A player loses an ordinary battle and returns to the field with the party still
  fainted. The next eligible wild encounter uses the four-action trainer menu.
- A player deposits their final usable Pokémon, leaving an empty, fainted-only,
  or Eggs-only party. All three cases use this mode without a starter flag.
- Ordinary trainers ignore that player on sight and refuse a requested battle.
  Walking past them does not mark them defeated or unlock a victory-gated route.
  Reviving or withdrawing a usable Pokémon re-enables those trainers.
- A no-party player passes a suppressed rival without consuming the chapter,
  completes the host adventure, recovers a usable Pokémon and can return for that
  chapter at its original location.
- A no-party player can travel along an opened public lane but cannot collect a
  guarded password, rescue reward or boss prize. Talking to the guard gives the
  specified refusal, releases control and leaves a usable retreat path.
- Losing a supported story battle preserves the unresolved objective and permits
  retreat; its victory continuation never runs on loss or refused startup.
- A player throws rocks, sees HP fall and anger warnings, and attempts escape.
  Failure consumes a turn and can trigger retaliation. Success ends the encounter
  before passive anger or retaliation applies.
- A lethal rock ends the encounter without retaliation, EXP, or rewards.
- Feeding reduces anger before passive gain and temporarily lowers flee risk
  without restoring HP. Cancelling selection changes no encounter state or items.
- A player with no balls can attempt escape. Throwing the last ball invokes no
  Safari allowance or exit scripts.
- With six fainted Pokémon, a successful catch sent to the PC leaves the party
  unusable. Safely using a revive on a fainted party member ends the encounter and
  restores normal battle eligibility.
- Go Near improves owned-ball capture odds in both modes, while an actual Safari
  visit still enforces its own allowance and step limit. A fourth or later Go Near
  still consumes a turn in both modes; trainer-only mode also builds passive anger.
  Factor saturation never turns Go Near into a free action.
- Retaliation before visiting a Pokémon Center uses the established recovery
  destination and text explaining the trainer's defeat.

Playtesting must establish whether messages communicate escalating danger without
meters, whether the dynamic warning arrives in time to inform a choice, and
whether catching without rocks remains practical. Compare equal-level encounters
with Pokémon above and below the Trainer Rating softcap, including repeated failed
escapes, berry use near the anger threshold, and peaceful turns reaching it.

## Open questions

- Validation and adjustment of the selected initial numeric tuning, including
  warning timing and the three-turn food duration's turn-order behavior.
- Exact anger and retaliation animations and final encounter messages.
- Challenge consequences for ordinary party defeat versus retaliation: shared
  recovery currently includes
  hardcore Nuzlocke save clearing/reset and ordinary Nuzlocke box withdrawal or
  creation of a level-1 Rattata. Confirm how these apply to each defeat reason
  before shipping. Preserve challenge rules by default; any approved exception
  belongs in centralized recovery with the defeat reason, not a separate path.
- League-run consequences and other exceptional authored defeat outcomes remain
  open. Preserve their existing routing until approved. This does not reopen
  the settled scripted-wild exclusion or its no-party guards.
- Confirm the existing centralized respawn fallback is valid for every supported
  origin before its first Pokémon Center visit.

## References

- [Trainer-only wild encounters specification](../specs/trainer-only-encounters.md)
- [Trainer-only story encounter specification](../specs/trainer-only-story-encounters.md)
- [Story encounter gate audit](../research/trainer-only-story-encounter-gates.md)
- [Wayfarer regional start choice](wayfarer-regional-start-choice.md)

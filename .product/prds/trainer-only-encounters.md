# Trainer-only wild encounters

Status: Supported core mechanic. This PRD defines only the explicit, author-owned
wild-encounter mode; it does not define a global no-party, NPC, story, or recovery
policy.

## Intent

In a safe, human-authored Wayfarer scenario, let a player with no party Pokémon
approach and catch a wild Pokémon directly. The player can use an owned Ball or
Berry, approach cautiously, throw a Rock, or Run. Catching without violence must
remain a complete strategy.

The mode does not create an origin, grant starter supplies, provide items, or
change ordinary party, story, trainer, storage, or recovery rules.

## Design

### Eligibility and menu

The dedicated controller starts only when both conditions are true at wild-battle
admission:

1. A human-authored script has set `FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS`.
2. `CalculatePlayerPartyCount()` is exactly zero.

The flag aliases the map-local temporary `FLAG_TEMP_B`. Normal map loading clears
temporary flags, but authors must clear this flag before leaving the safe scenario
or entering unsupported content. No production scenario enables the flag.

This is a literal party-count rule. A nonempty party—including a fully fainted,
Egg-only, mixed, or otherwise unusable party—never enters trainer-only mode and
keeps native behavior. With the flag clear, an empty party also keeps native
behavior. The mode has no contextual exclusions or automatic map/NPC allowlist;
authors own the complete safety of each flagged scenario.

Trainer battles, scripted or fixed wild battles, tutorials, partner and special
battles, scene transitions, and other unsupported flows are not converted,
intercepted, refused, suppressed, rolled back, or given a recovery fallback by
this mechanic. They remain native. A safe scenario must not reach them while the
flag is set with an empty party.

Native storage protection remains in force: depositing or moving the last party
Pokémon into a box is prohibited. The mechanic does not permit leaving a party
empty by depositing its final member.

## Supported mechanic

An enabled ordinary wild encounter uses the dedicated trainer-only controller.
Existing ordinary wild sources that can start a wild battle retain their current
encounter availability and field permissions. The controller preserves its Ball,
Berry, Rock, Go Near, and Run actions.

### Encounter menu

The actions occupy the familiar battle menu positions:

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
a turn nor an item. The Bag exposes owned usable Balls and Berries only.
Every standard berry from Cheri through Maranga is feedable, including standard
Enigma and excluding the e-reader Enigma item. Use an explicit list. Berry selection
feeds directly without a Feed/Use submenu or party-medicine option in this mode.

With no balls left, the player can still run. Attempting to select a ball explains
that none are available without advancing the encounter. Food is optional; the
player does not need a Pokéblock Case or berries to attempt a catch.

Ball effects that need an active player Pokémon use a neutral modifier for that
effect alone. Other applicable effects still work. The game must not invent a
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

Retaliation completes normal battle teardown and uses native blackout and
recovery. It has no feature-specific injury message, regional recovery override,
special warp, money rule, or alternate loss policy. Ordinary party defeat and
field-poison exhaustion also remain native.

## Boundaries

This Wayfarer mechanic applies only to ordinary wild encounters admitted while
the explicit map-local flag is set and the party count is exactly zero. It grants
no new encounter availability, progression, field permission, capture
permission, or party/storage behavior. There are no contextual exclusions or
automatic map/NPC allowlists: authors own the safety of each flagged scenario.

Nonempty parties—including fully fainted, Egg-only, mixed, or otherwise unusable
parties—remain native, as does an empty party when the flag is clear. Trainer,
scripted, fixed, tutorial, partner, multi-Pokémon, scene, and other unsupported
flows remain native. This mechanic adds no conversion, interception, refusal,
suppression, rollback, or recovery fallback for them; authors must clear the
flag before reaching such content.

Safari retains its action set, admission, ball grants, step limit, total throw
allowance including owned balls, and exit behavior. The intended shared change is
that Go Near benefits every ball in HNS/Wayfarer Safari encounters too. Safari
keeps its existing turn cost, factor updates and flee checks even at the closest
distance. Existing Safari Ball odds remain unchanged. This does
not redesign FRLG bait and rock mechanics or add anger and retaliation to Safari.

No new origin, starter gift, berry distribution, shop, encounter table, trainer
HP system, save migration, NPC/story registry, medicine/revive flow, or custom
loss/recovery policy is included. Depositing the last party Pokémon remains
prohibited by native storage rules.

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
followed by the native fade and recovery presentation. It adds no feature-specific
blackout, recovery, or injury text.

## Native interactions

Capture delivery, party capacity, PC capacity, item ownership, field poisoning,
ordinary battle loss, blackout, recovery, challenge rules, and money handling
remain native. The mechanic adds no medicine, Revive, PP, custom loss, custom
poison, custom recovery, special warp, or fallback path. Native storage still
prohibits depositing the last party Pokémon.

Authors clear the flag before unsafe or unsupported interactions. No production
map or scenario enables it, and no NPC/story registry, interception, suppression,
refusal, or story continuation belongs to this mechanic.

## Constraints

Implement a distinct encounter mode and controller with its own actions and
outcomes. Share selected Safari building blocks for trainer presentation, ball
selection and throwing, capture delivery, and proximity calculations. Do not
copy the Safari subsystem wholesale or build a configurable battle framework.
Admission, visit counters, ball grants, and entrance warps remain Safari-owned.

Blackout destination and recovery policy remain native and centralized. The
technical integration contract belongs in the linked specification.

## Playtesting

Acceptance examples:

- An empty-party player approaches and catches a Pokémon using an owned ball,
  without throwing rocks. Each unsuccessful committed action builds passive anger.
  If the catch enters the party usable, the next encounter uses normal battles.
- The same empty party with the flag clear remains native. A nonempty party,
  including fully fainted, Egg-only, mixed, or otherwise unusable parties, also
  remains native.
- Depositing the last party Pokémon is still prohibited by native storage rules.
- The author clears the temporary flag before leaving the safe scenario or
  entering trainer, scripted, fixed, tutorial, partner, scene, or other
  unsupported content; those flows retain native behavior.
- A player throws rocks, sees HP fall and anger warnings, and attempts escape.
  Failure consumes a turn and can trigger retaliation. Success ends the encounter
  before passive anger or retaliation applies.
- A lethal rock ends the encounter without retaliation, EXP, or rewards.
- Feeding reduces anger before passive gain and temporarily lowers flee risk
  without restoring HP. Cancelling selection changes no encounter state or items.
- A player with no balls can attempt escape. Throwing the last ball invokes no
  Safari allowance or exit scripts.
- Go Near improves owned-ball capture odds in both modes, while an actual Safari
  visit still enforces its own allowance and step limit. A fourth or later Go Near
  still consumes a turn in both modes; trainer-only mode also builds passive anger.
  Factor saturation never turns Go Near into a free action.
- Retaliation, ordinary party defeat, and field-poison exhaustion all use their
  native teardown, blackout, and recovery behavior.

Playtesting must establish whether messages communicate escalating danger without
meters, whether the dynamic warning arrives in time to inform a choice, and
whether catching without rocks remains practical. Compare equal-level encounters
with Pokémon above and below the Trainer Rating softcap, including repeated failed
escapes, berry use near the anger threshold, and peaceful turns reaching it.

## No contextual exceptions

This PRD defines no challenge, trainer, NPC, story, tutorial, partner, scene,
loss, poisoning, medicine, revive, storage, or recovery exception. Existing
systems own those behaviors. A human author may enable the mode only where the
complete scenario is safe; unsupported flows remain native and are not converted
or intercepted by this feature.

## Open questions

- Validation and adjustment of the selected initial numeric tuning, including
  warning timing and the three-turn food duration's turn-order behavior.
- Exact anger and retaliation animations and final encounter messages.
- Playtest supported, human-authored scenarios and confirm authors clear the flag
  before every unsupported interaction.

## References

- [Trainer-only wild encounters specification](../specs/trainer-only-encounters.md)
- [Flag-only footprint research](../research/pr96-flag-only-footprint.md)

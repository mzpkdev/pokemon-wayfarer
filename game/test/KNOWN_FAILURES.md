# Deferred test failures

This document tracks tests marked with `TODO(nightly-failures)` and `KNOWN_FAILING`. It keeps the reason for each skip and the condition for restoring the test in one place, while the matching inline TODO remains the authoritative record beside the test.

The inventory covers 338 deferred tests across 112 source files. It does not include pre-existing `KNOWN_FAILING` tests that lack the nightly-failures tag.

## Returning to a deferred test

1. Open the linked source and confirm that the TODO still describes the current failure.
2. Run a focused check from `game/`:

   ```sh
   TEST=1 UNUSED_ERROR=1 DEPRECATED_ERROR=1 make check TESTS="<distinct part of the test title>"
   ```

3. Fix the underlying mechanics, AI behavior, event order, fixture, or layout. Do not change the expected result only to silence a real behavior difference.
4. Run the focused check again. A fixed test that still has `KNOWN_FAILING` reports `KNOWN_FAILING_PASS`.
5. Remove `KNOWN_FAILING`, its `TODO(nightly-failures)`, and the matching entry below.
6. Run the full Expansion Suite:

   ```sh
   MGBA_ROM_TEST_HYDRA_REPORT="$PWD/expansion-suite-report.ndjson" \
   TEST=1 UNUSED_ERROR=1 DEPRECATED_ERROR=1 \
   make -j"$(nproc)" check
   ```

## Summary

| Area | Deferred tests |
| --- | ---: |
| Battle abilities | 83 |
| Battle AI | 19 |
| Core battle behavior | 23 |
| Form changes | 17 |
| Battle gimmicks | 46 |
| Held item effects | 18 |
| Battle item effects | 5 |
| Move effects | 109 |
| Secondary move effects | 7 |
| Combined move effects | 4 |
| Starting battle status | 1 |
| Non-battle systems | 6 |
| **Total** | **338** |

## Inventory

### Battle abilities

#### [`battle/ability/aftermath.c`](battle/ability/aftermath.c)

- `Aftermath damages the attacker by 1/4th of its max HP if fainted by a contact move` ([source](battle/ability/aftermath.c#L4)): The contact-faint scene emits an unmatched pre-action message before the expected Aftermath sequence. Re-enable after its event ordering is aligned.

#### [`battle/ability/arena_trap.c`](battle/ability/arena_trap.c)

- `Arena Trap doesn't prevent switch outs if the Pokémon is switched in the same turn the opponent decided to switch out` ([source](battle/ability/arena_trap.c#L18)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Arena Trap doesn't prevent switch outs via Shed Shell` ([source](battle/ability/arena_trap.c#L69)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/beads_of_ruin.c`](battle/ability/beads_of_ruin.c)

- `Beads of Ruin's message displays correctly after all battlers fainted - Player` ([source](battle/ability/beads_of_ruin.c#L34)): Beads of Ruin activation after all battlers faint uses a different message sequence. Re-enable after its switch-in ordering is aligned.
- `Beads of Ruin's message displays correctly after all battlers fainted - Opponent` ([source](battle/ability/beads_of_ruin.c#L58)): Beads of Ruin activation after all battlers faint uses a different message sequence. Re-enable after its switch-in ordering is aligned.

#### [`battle/ability/clear_body.c`](battle/ability/clear_body.c)

- `Clear Body, Full Metal Body, and White Smoke don't prevent Topsy-Turvy` ([source](battle/ability/clear_body.c#L339)): Baton Pass and Topsy-Turvy use a different event/message sequence than this scene expects. Re-enable after that stat-transfer ordering is aligned.

#### [`battle/ability/color_change.c`](battle/ability/color_change.c)

- `Color Change changes the type to Normal when a Pokemon is hit by a forseen attack under the effect of Normalize` ([source](battle/ability/color_change.c#L137)): The Color Change battle log after Normalize does not match this game. Re-enable after the behavior and messages align.

#### [`battle/ability/comatose.c`](battle/ability/comatose.c)

- `Comatose prevents status-inducing moves` ([source](battle/ability/comatose.c#L4)): Comatose's drowsing message does not match this game. Re-enable after Comatose's status behavior and messages align.
- `Comatose may be suppressed if Pokémon transformed into a Pokémon with Comatose ability and was under the effects of Gastro Acid` ([source](battle/ability/comatose.c#L29)): The transformed Comatose and Gastro Acid scenario emits an unexpected battle log. Re-enable after its behavior and messages align.

#### [`battle/ability/commander.c`](battle/ability/commander.c)

- `Commander prevents Whirlwind from working against Dondozo or Tatsugiri while it's active` ([source](battle/ability/commander.c#L133)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/competitive.c`](battle/ability/competitive.c)

- `Competitive activates after Sticky Web lowers Speed` ([source](battle/ability/competitive.c#L117)): The Sticky Web switch-in log for Competitive does not match this game. Re-enable after its behavior and messages align.
- `Competitive doesn't activate after Sticky Web lowers Speed if Court Changed (gen8)` ([source](battle/ability/competitive.c#L142)): The Court Change and Sticky Web switch-in log does not match this game. Re-enable after its behavior and messages align.
- `Competitive correctly activates after Sticky Web lowers Speed if Court Changed (Gen8)` ([source](battle/ability/competitive.c#L171)): The Court Change and Sticky Web Competitive log does not match this game. Re-enable after its behavior and messages align.

#### [`battle/ability/costar.c`](battle/ability/costar.c)

- `Costar copies an ally's stat stages upon entering battle` ([source](battle/ability/costar.c#L4)): The expected battle message is not emitted by the current battle implementation. Re-enable after the message flow is corrected.
- `Costar's message displays correctly after all battlers fainted - Player` ([source](battle/ability/costar.c#L153)): The expected battle message is not emitted by the current battle implementation. Re-enable after the message flow is corrected.
- `Costar's message displays correctly after all battlers fainted - Opponent` ([source](battle/ability/costar.c#L184)): The expected battle message is not emitted by the current battle implementation. Re-enable after the message flow is corrected.

#### [`battle/ability/curious_medicine.c`](battle/ability/curious_medicine.c)

- `Curious Medicine resets ally's stat stages upon entering battle` ([source](battle/ability/curious_medicine.c#L4)): Curious Medicine's switch-in stat-reset sequence emits additional or reordered messages. Re-enable after its event ordering is aligned.

#### [`battle/ability/delta_stream.c`](battle/ability/delta_stream.c)

- `Delta Stream doesn't activate if there's already strong winds` ([source](battle/ability/delta_stream.c#L8)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Strong winds continue as long as there's a Pokémon with Delta Stream on the field` ([source](battle/ability/delta_stream.c#L33)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/desolate_land.c`](battle/ability/desolate_land.c)

- `Desolate Land blocks damaging Water-type moves and prints the message only once with moves hitting multiple targets` ([source](battle/ability/desolate_land.c#L32)): The multi-target Desolate Land battle log does not match this game. Re-enable after its blocking behavior and messages align.

#### [`battle/ability/fairy_aura.c`](battle/ability/fairy_aura.c)

- `Fairy Aura's effect doesn't stack multiple times` ([source](battle/ability/fairy_aura.c#L62)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/forecast.c`](battle/ability/forecast.c)

- `Forecast transforms Castform when Cloud Nine ability user leaves the field` ([source](battle/ability/forecast.c#L416)): Cloud Nine or Air Lock removal and Forecast transformation use a different message sequence. Re-enable after their switch-out ordering is aligned.

#### [`battle/ability/grim_neigh.c`](battle/ability/grim_neigh.c)

- `Grim Neigh raises Sp. Attack by one stage after directly causing a Pokemon to faint` ([source](battle/ability/grim_neigh.c#L4)): The direct-KO Grim Neigh battle log does not match this game. Re-enable after its KO handling and messages align.
- `Grim Neigh does not increase damage done by the same move that causes another Pokemon to faint` ([source](battle/ability/grim_neigh.c#L74)): The same-move KO Grim Neigh scenario emits an unexpected battle log. Re-enable after its behavior and messages align.

#### [`battle/ability/gulp_missile.c`](battle/ability/gulp_missile.c)

- `(Gulp Missile) Cramorant in Gorging damages an electric type without paralysing` ([source](battle/ability/gulp_missile.c#L233)): Gulp Missile paralyses an Electric-type target. Re-enable after the Gorging form's paralysis immunity is implemented.

#### [`battle/ability/intimidate.c`](battle/ability/intimidate.c)

- `Intimidate activates on an empty slot` ([source](battle/ability/intimidate.c#L134)): Intimidate activation against an empty opposing slot differs from this expected event sequence. Re-enable after that targeting behavior is aligned.

#### [`battle/ability/levitate.c`](battle/ability/levitate.c)

- `Levitate does not cause single remaining target to take higher damage` ([source](battle/ability/levitate.c#L64)): When Levitate leaves one valid Earthquake target, the target receives doubled damage. Re-enable after spread-target damage calculation is aligned.

#### [`battle/ability/lightning_rod.c`](battle/ability/lightning_rod.c)

- `Lightning Rod forces single-target Electric-type moves to target the Pokémon with this Ability.` ([source](battle/ability/lightning_rod.c#L44)): Lightning Rod's redirected-target animation does not match this game. Re-enable after targeting and animation behavior align.

#### [`battle/ability/magic_bounce.c`](battle/ability/magic_bounce.c)

- `Magic Bounce bounces back status moves` ([source](battle/ability/magic_bounce.c#L4)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Magic Bounce bounces back powder moves` ([source](battle/ability/magic_bounce.c#L65)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Magic Bounce cannot bounce back powder moves against Grass Types` ([source](battle/ability/magic_bounce.c#L87)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Magic Bounce bounces back moves hitting both foes at two foes` ([source](battle/ability/magic_bounce.c#L110)): The current battle emits an animation this scenario forbids. Re-enable after animation behavior is corrected.
- `Magic Bounce bounces back moves hitting foes field` ([source](battle/ability/magic_bounce.c#L139)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Magic Bounce bounced back status moves can not be bounced back by Magic Bounce` ([source](battle/ability/magic_bounce.c#L175)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/mimicry.c`](battle/ability/mimicry.c)

- `Mimicry changes the battler's type based on Terrain` ([source](battle/ability/mimicry.c#L12)): Mimicry's terrain type-change message does not match this game. Re-enable after terrain type changes and messages align.
- `Mimicry can trigger multiple times in a turn` ([source](battle/ability/mimicry.c#L80)): Mimicry's repeated terrain-trigger log does not match this game. Re-enable after repeated triggers and messages align.
- `Mimicry triggers after Skill Swap` ([source](battle/ability/mimicry.c#L107)): Mimicry after Skill Swap emits an unexpected battle log. Re-enable after its behavior and messages align.

#### [`battle/ability/moxie.c`](battle/ability/moxie.c)

- `Moxie/Chilling Neigh raises Attack by one stage after directly causing a Pokemon to faint` ([source](battle/ability/moxie.c#L4)): The direct-KO Moxie and Chilling Neigh log does not match this game. Re-enable after KO handling and messages align.
- `Moxie/Chilling Neigh does not increase damage done by the same move that causes another Pokemon to faint` ([source](battle/ability/moxie.c#L116)): The same-move KO Moxie and Chilling Neigh scenario emits an unexpected log. Re-enable after its behavior and messages align.

#### [`battle/ability/neutralizing_gas.c`](battle/ability/neutralizing_gas.c)

- `Neutralizing Gas leaving the field allows abilities to activate in turn order` ([source](battle/ability/neutralizing_gas.c#L164)): The post-KO Neutralizing Gas switch-in log does not match this game. Re-enable after activation order and messages align.
- `Neutralizing Gas only displays exiting message for the last user leaving the field` ([source](battle/ability/neutralizing_gas.c#L342)): Neutralizing Gas's final-user exit message does not match this game. Re-enable after exit handling and messages align.
- `Neutralizing Gas is active for the duration of a Spread Move even if Neutralizing Gas is no longer on the field` ([source](battle/ability/neutralizing_gas.c#L362)): Neutralizing Gas's spread-move exit scenario emits an unexpected battle log. Re-enable after its duration behavior and messages align.
- `Neutralizing Gas is active until the last Dragon Darts hit even if Neutralizing Gas is no longer on the field` ([source](battle/ability/neutralizing_gas.c#L385)): Neutralizing Gas's Dragon Darts exit scenario emits an unexpected battle log. Re-enable after its duration behavior and messages align.

#### [`battle/ability/parental_bond.c`](battle/ability/parental_bond.c)

- `Parental Bond converts Scratch into a two-strike move` ([source](battle/ability/parental_bond.c#L4)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Parental Bond does not convert a move with three or more strikes to a two-strike move` ([source](battle/ability/parental_bond.c#L30)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Parental Bond converts multi-target moves into a two-strike move in Single Battles` ([source](battle/ability/parental_bond.c#L56)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Parental Bond does not convert multi-target moves into a two-strike move in Double Battles, even if it only damages one` ([source](battle/ability/parental_bond.c#L87)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Parental Bond-converted moves only hit once on Lightning Rod/Storm Drain mons` ([source](battle/ability/parental_bond.c#L118)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Parental Bond has no affect on multi hit moves and they still hit twice 37.5/35% of the time` ([source](battle/ability/parental_bond.c#L153)): The Mega Evolution activation message sequence does not match this scenario. Re-enable after Mega Evolution messaging is aligned.
- `Parental Bond has no affect on multi hit moves and they still hit thrice 37.5/35% of the time` ([source](battle/ability/parental_bond.c#L185)): The Mega Evolution activation message sequence does not match this scenario. Re-enable after Mega Evolution messaging is aligned.
- `Parental Bond has no affect on multi hit moves and they still hit four times 12.5/15% of the time` ([source](battle/ability/parental_bond.c#L218)): The Mega Evolution activation message sequence does not match this scenario. Re-enable after Mega Evolution messaging is aligned.
- `Parental Bond Smack Down effect triggers after 2nd hit` ([source](battle/ability/parental_bond.c#L282)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Parental Bond Snore strikes twice while asleep` ([source](battle/ability/parental_bond.c#L309)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/pickpocket.c`](battle/ability/pickpocket.c)

- `Pickpocket activates after Sticky Barb transfers` ([source](battle/ability/pickpocket.c#L178)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/prankster.c`](battle/ability/prankster.c)

- `Prankster-affected moves called via Instruct do not affect Dark-type Pokémon` ([source](battle/ability/prankster.c#L84)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Moves called via Prankster-affected After you affect Dark-type Pokémon` ([source](battle/ability/prankster.c#L136)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Prankster-affected moves which are reflected by Magic Coat can affect Dark-type Pokémon, unless the Pokémon that bounced the move also has Prankster` ([source](battle/ability/prankster.c#L204)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/protosynthesis.c`](battle/ability/protosynthesis.c)

- `Protosynthesis prioritizes stats in the case of a tie in the following order: Atk, Def, Sp.Atk, Sp.Def, Speed` ([source](battle/ability/protosynthesis.c#L104)): Protosynthesis's tied-stat boost message does not match this game. Re-enable after stat selection and messages align.

#### [`battle/ability/quark_drive.c`](battle/ability/quark_drive.c)

- `Quark Drive prioritizes stats in the case of a tie in the following order: Atk, Def, Sp.Atk, Sp.Def, Speed` ([source](battle/ability/quark_drive.c#L171)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/rattled.c`](battle/ability/rattled.c)

- `Rattled boosts speed by 1 when hit by Bug, Dark or Ghost type move` ([source](battle/ability/rattled.c#L16)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Rattled triggers correctly when hit by U-Turn` ([source](battle/ability/rattled.c#L96)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/sheer_force.c`](battle/ability/sheer_force.c)

- `Sheer Force only boosts the damage of moves it's supposed to boost (Gen1)` ([source](battle/ability/sheer_force.c#L618)): The Gen 1 move set contains a Sheer Force classification or damage-rule mismatch. Re-enable after that move behavior is reconciled.
- `Sheer Force only boosts the damage of moves it's supposed to boost (Gen8)` ([source](battle/ability/sheer_force.c#L1236)): The Gen 8 move set contains a Sheer Force classification or damage-rule mismatch. Re-enable after that move behavior is reconciled.

#### [`battle/ability/sword_of_ruin.c`](battle/ability/sword_of_ruin.c)

- `Sword of Ruin's message displays correctly after all battlers fainted - Player` ([source](battle/ability/sword_of_ruin.c#L33)): Sword of Ruin's post-faint switch-in message does not match this game. Re-enable after switch-in behavior and messages align.
- `Sword of Ruin's message displays correctly after all battlers fainted - Opponent` ([source](battle/ability/sword_of_ruin.c#L57)): Sword of Ruin's opponent post-faint switch-in message does not match this game. Re-enable after switch-in behavior and messages align.

#### [`battle/ability/symbiosis.c`](battle/ability/symbiosis.c)

- `Symbiosis transfers its item to an ally after it consumes an item` ([source](battle/ability/symbiosis.c#L4)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Symbiosis triggers after partners berry eaten from bug bite` ([source](battle/ability/symbiosis.c#L35)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Symbiosis triggers after partner bestows its item` ([source](battle/ability/symbiosis.c#L66)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Symbiosis triggers after partner flings its item` ([source](battle/ability/symbiosis.c#L98)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/tablets_of_ruin.c`](battle/ability/tablets_of_ruin.c)

- `Tablets of Ruin's message displays correctly after all battlers fainted - Player` ([source](battle/ability/tablets_of_ruin.c#L33)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Tablets of Ruin's message displays correctly after all battlers fainted - Opponent` ([source](battle/ability/tablets_of_ruin.c#L58)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/ability/tera_shell.c`](battle/ability/tera_shell.c)

- `Tera Shell makes all hits of multi-hit moves against Terapagos not very effective` ([source](battle/ability/tera_shell.c#L33)): Tera Shell's multi-hit battle log does not match this game. Re-enable after multi-hit behavior and messages align.

#### [`battle/ability/vessel_of_ruin.c`](battle/ability/vessel_of_ruin.c)

- `Vessel of Ruin's message displays correctly after all battlers fainted - Player` ([source](battle/ability/vessel_of_ruin.c#L33)): Vessel of Ruin activation after all battlers faint uses a different message sequence. Re-enable after its switch-in ordering is aligned.
- `Vessel of Ruin's message displays correctly after all battlers fainted - Opponent` ([source](battle/ability/vessel_of_ruin.c#L57)): Vessel of Ruin activation after all battlers faint uses a different message sequence. Re-enable after its switch-in ordering is aligned.

#### [`battle/ability/weak_armor.c`](battle/ability/weak_armor.c)

- `Weak Armor still boosts Speed if Defense can't go any lower` ([source](battle/ability/weak_armor.c#L79)): The current battle emits an animation this scenario forbids. Re-enable after animation behavior is corrected.

#### [`battle/ability/wind_rider.c`](battle/ability/wind_rider.c)

- `Wind Rider activates when it's no longer effected by Neutralizing Gas` ([source](battle/ability/wind_rider.c#L85)): Wind Rider's Neutralizing Gas exit log does not match this game. Re-enable after activation behavior and messages align.
- `Tailwind does not trigger Wind Rider on an absent ally battler` ([source](battle/ability/wind_rider.c#L131)): The absent-battler Tailwind scenario emits an unexpected battle log. Re-enable after faint handling and messages align.

#### [`battle/ability/zen_mode.c`](battle/ability/zen_mode.c)

- `Zen Mode switches Darmanitan's form when HP is half or less at the end of the turn` ([source](battle/ability/zen_mode.c#L4)): Zen Mode form changes receive additional or reordered battle messages before this scene's expected actions. Re-enable after the form-change event sequence is aligned.
- `Zen Mode switches Darmanitan's form to Standard when swapped out` ([source](battle/ability/zen_mode.c#L36)): Zen Mode form changes receive additional or reordered battle messages before this scene's expected actions. Re-enable after the form-change event sequence is aligned.
- `Zen Mode switches Darmanitan's form when HP is healed above half` ([source](battle/ability/zen_mode.c#L77)): Zen Mode form changes receive additional or reordered battle messages before this scene's expected actions. Re-enable after the form-change event sequence is aligned.

#### [`battle/ability/zero_to_hero.c`](battle/ability/zero_to_hero.c)

- `Zero to Hero transforms Palafin when it switches out` ([source](battle/ability/zero_to_hero.c#L4)): Zero to Hero's switch-out transformation log does not match this game. Re-enable after transformation behavior and messages align.
- `Zero to Hero's message displays correctly after all battlers fainted - Player` ([source](battle/ability/zero_to_hero.c#L141)): Zero to Hero's player post-faint transformation log does not match this game. Re-enable after switch-in behavior and messages align.
- `Zero to Hero's message displays correctly after all battlers fainted - Opponent` ([source](battle/ability/zero_to_hero.c#L166)): Zero to Hero's opponent post-faint transformation log does not match this game. Re-enable after switch-in behavior and messages align.

### Battle AI

#### [`battle/ai/ai_doubles.c`](battle/ai/ai_doubles.c)

- `AI will not use a status move if partner already chose Helping Hand` ([source](battle/ai/ai_doubles.c#L265)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `AI recognizes Volt Absorb received from Trace` ([source](battle/ai/ai_doubles.c#L732)): The AI selects a move this scenario forbids. Re-enable after AI move scoring is corrected.

#### [`battle/ai/ai_flag_attacks_partner.c`](battle/ai/ai_flag_attacks_partner.c)

- `AI_FLAG_ATTACKS_PARTNER is willing to kill either the partner or the player` ([source](battle/ai/ai_flag_attacks_partner.c#L5)): The AI chooses BRUTAL SWING where this scenario expects OVERDRIVE. Re-enable after partner-target move scoring is aligned.

#### [`battle/ai/ai_flag_sequence_switching.c`](battle/ai/ai_flag_sequence_switching.c)

- `AI_FLAG_SEQUENCE_SWITCHING: AI will always switch after a KO in exactly party order` ([source](battle/ai/ai_flag_sequence_switching.c#L4)): AI post-KO replacement emits a different battle-message sequence than this party-order scene expects. Re-enable after replacement ordering is aligned.

#### [`battle/ai/ai_smart_tera.c`](battle/ai/ai_smart_tera.c)

- `AI_FLAG_SMART_TERA: AI will tera if it enables a ko` ([source](battle/ai/ai_smart_tera.c#L5)): The AI Smart Tera transformation message does not match this game. Re-enable after AI Terastallization behavior and messages align.

#### [`battle/ai/ai_switching.c`](battle/ai/ai_switching.c)

- `AI switches if Perish Song is about to kill` ([source](battle/ai/ai_switching.c#L80)): The AI does not switch before Perish Song causes a knockout. Re-enable after its Perish Song switch decision is aligned.
- `AI will not try to switch for the same Pokémon for 2 spots in a double battle (all bad moves)` ([source](battle/ai/ai_switching.c#L122)): Multi-slot switch reservation can select an ineligible player-party slot. Re-enable after switch ownership and reservation are aligned.
- `AI partner will not switch mid-turn into a player Pokémon (multi)` ([source](battle/ai/ai_switching.c#L154)): AI-partner switching can select a player-party slot. Re-enable after partner switch eligibility is aligned.
- `AI partner will not switch mid-turn into a player Pokémon (2v1)` ([source](battle/ai/ai_switching.c#L188)): AI-partner switching can select a player-party slot. Re-enable after partner switch eligibility is aligned.
- `AI partner will not switch into a player Pokémon after fainting (multi)` ([source](battle/ai/ai_switching.c#L222)): AI-partner replacement after a faint can select a player-party slot. Re-enable after replacement eligibility is aligned.
- `AI partner will not switch into a player Pokémon after fainting (2v1)` ([source](battle/ai/ai_switching.c#L254)): AI-partner replacement after a faint can select a player-party slot. Re-enable after replacement eligibility is aligned.
- `AI partner will not switch into a player Pokémon (multi)` ([source](battle/ai/ai_switching.c#L286)): AI-partner switching can select a player-party slot. Re-enable after partner switch eligibility is aligned.
- `AI partner will not switch into a player Pokémon (2v1)` ([source](battle/ai/ai_switching.c#L318)): AI-partner switching can select a player-party slot. Re-enable after partner switch eligibility is aligned.
- `AI will not try to switch for the same pokemon for 2 spots in a 2v1 battle (all bad moves)` ([source](battle/ai/ai_switching.c#L349)): Multi-slot switch reservation can select a duplicate or ineligible switch-in. Re-enable after switch reservation is aligned.
- `AI will not switch into a partner Pokémon in a 1v2 battle (all bad moves)` ([source](battle/ai/ai_switching.c#L381)): AI switch selection can cross party ownership in 1v2 battles. Re-enable after switch eligibility is aligned.
- `AI will not try to switch for the same Pokémon for 2 spots in a double battle (Wonder Guard)` ([source](battle/ai/ai_switching.c#L472)): Wonder Guard switching can reserve the same or an ineligible switch-in twice. Re-enable after switch reservation is aligned.
- `AI_FLAG_SMART_MON_CHOICES: Number of hits to KO calculation checks whether incoming damage is less than recurring healing to avoid an infinite loop` ([source](battle/ai/ai_switching.c#L557)): AI move scoring does not select a valid attack for this recurring-healing calculation. Re-enable after its KO evaluation is aligned.
- `AI_FLAG_SMART_MON_CHOICES: Number of hits to KO calculation checks whether incoming damage is zero to avoid an infinite loop` ([source](battle/ai/ai_switching.c#L578)): AI move scoring does not select a valid attack for this zero-damage calculation. Re-enable after its KO evaluation is aligned.
- `AI_FLAG_SMART_MON_CHOICES: AI will not switch in a Pokemon which is slower and gets 1HKOed after fainting` ([source](battle/ai/ai_switching.c#L614)): AI candidate scoring selects a slower switch-in that is immediately 1HKOed. Re-enable after post-KO switch scoring is aligned.

### Core battle behavior

#### [`battle/damage_formula.c`](battle/damage_formula.c)

- `Damage calculation matches Gen5+` ([source](battle/damage_formula.c#L6)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Damage calculation matches Gen6+ (Muscle Band, crit)` ([source](battle/damage_formula.c#L47)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `A spread move will do correct damage to the second mon if the first target faints from first hit of the spread move (double battle)` ([source](battle/damage_formula.c#L128)): After the first target faints, the remaining target receives unmodified spread-move damage. Re-enable after target-count damage calculation is aligned.
- `A spread move will do correct damage to the second mon if the first target faints from first hit of the spread move (multibattle)` ([source](battle/damage_formula.c#L166)): After the first target faints, the remaining target receives unmodified spread-move damage. Re-enable after target-count damage calculation is aligned.
- `A spread move will do correct damage to the second mon if the first target faints from first hit of the spread move (2v1)` ([source](battle/damage_formula.c#L204)): After the first target faints, the remaining target receives unmodified spread-move damage. Re-enable after target-count damage calculation is aligned.
- `A spread move will do correct damage to the second mon if the first target faints from first hit of the spread move (1v2)` ([source](battle/damage_formula.c#L242)): After the first target faints, the remaining target receives unmodified spread-move damage. Re-enable after target-count damage calculation is aligned.
- `Transistor Damage calculation` ([source](battle/damage_formula.c#L387)): The current battle does not emit this test's expected animation. Re-enable after animation behavior is corrected.

#### [`battle/hazards.c`](battle/hazards.c)

- `Hazards are applied based on order of set up` ([source](battle/hazards.c#L4)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Hazards are applied correctly after a battler faints` ([source](battle/hazards.c#L45)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Toxic Spikes can be removed after fainting to other hazards` ([source](battle/hazards.c#L71)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Hazards can trigger Emergency Exit and other hazards don't activate` ([source](battle/hazards.c#L108)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Hazards can trigger Emergency Exit and hazards still activate for other battlers` ([source](battle/hazards.c#L143)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/spread_moves.c`](battle/spread_moves.c)

- `Spread Moves: Spread move, Gem Boosted, vs Resist Berries` ([source](battle/spread_moves.c#L250)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Spread Moves: Explosion, Gem Boosted, vs Resist Berries` ([source](battle/spread_moves.c#L276)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Spread Moves: Spread move vs Eiscue and Mimikyu with 1 Eject Button` ([source](battle/spread_moves.c#L303)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Spread Moves: Spread move vs one protecting mon` ([source](battle/spread_moves.c#L347)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Spread Moves: Focus Sash activates correctly` ([source](battle/spread_moves.c#L477)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Spread Moves: AOE ground type move vs Levitate and Air Balloon` ([source](battle/spread_moves.c#L501)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/switch_in_abilities.c`](battle/switch_in_abilities.c)

- `Switch-in abilities trigger in Speed Order after post-KO switch - Single Battle` ([source](battle/switch_in_abilities.c#L62)): The single-battle post-KO switch-in log does not match this game. Re-enable after switch-in order and messages align.
- `Switch-in abilities trigger in Speed Order after post-KO switch - Double Battle` ([source](battle/switch_in_abilities.c#L91)): The double-battle post-KO switch-in log does not match this game. Re-enable after switch-in order and messages align.
- `Switch-in abilities trigger in Speed Order after post-KO switch - multibattle` ([source](battle/switch_in_abilities.c#L134)): The multi-battle post-KO switch-in log does not match this game. Re-enable after switch-in order and messages align.
- `Switch-in abilities trigger in Speed Order after post-KO switch - 2v1` ([source](battle/switch_in_abilities.c#L177)): The 2v1 post-KO switch-in log does not match this game. Re-enable after switch-in order and messages align.
- `Switch-in abilities trigger in Speed Order after post-KO switch - 1v2` ([source](battle/switch_in_abilities.c#L219)): The 1v2 post-KO switch-in log does not match this game. Re-enable after switch-in order and messages align.

### Form changes

#### [`battle/form_change/battle_after_move.c`](battle/form_change/battle_after_move.c)

- `Relic Song transforms once Meloetta in a double battle` ([source](battle/form_change/battle_after_move.c#L78)): Relic Song's double-battle event/message sequence differs from this expectation. Re-enable after its form-change sequence is aligned.

#### [`battle/form_change/end_battle.c`](battle/form_change/end_battle.c)

- `Palafin returns to Zero form upon battle end` ([source](battle/form_change/end_battle.c#L70)): Palafin's battle-end form reset log does not match this game. Re-enable after form reset behavior and messages align.

#### [`battle/form_change/mega_evolution.c`](battle/form_change/mega_evolution.c)

- `Venusaur can Mega Evolve holding Venusaurite` ([source](battle/form_change/mega_evolution.c#L4)): Mega Evolution's activation message sequence differs from this expectation. Re-enable after Mega Evolution messaging is aligned.
- `Mega Evolution's order is determined by Speed - opponent faster` ([source](battle/form_change/mega_evolution.c#L22)): Mega Evolution's speed-order message sequence differs from this expectation. Re-enable after Mega Evolution ordering is aligned.
- `Mega Evolution's order is determined by Speed - player faster` ([source](battle/form_change/mega_evolution.c#L43)): Mega Evolution's speed-order message sequence differs from this expectation. Re-enable after Mega Evolution ordering is aligned.
- `Rayquaza can Mega Evolve knowing Dragon Ascent` ([source](battle/form_change/mega_evolution.c#L64)): Rayquaza's fervent-wish Mega Evolution message sequence differs from this expectation. Re-enable after that activation sequence is aligned.
- `Mega Evolution happens after switching, but before Focus Punch-like Moves` ([source](battle/form_change/mega_evolution.c#L132)): Switching, Mega Evolution, and Focus Punch setup use a different event/message sequence. Re-enable after their ordering is aligned.
- `Regular Mega Evolution and Fervent Wish Mega Evolution can happen on the same turn` ([source](battle/form_change/mega_evolution.c#L162)): Mixed regular and fervent-wish Mega Evolution messages differ from this expectation. Re-enable after their same-turn sequence is aligned.

#### [`battle/form_change/primal_reversion.c`](battle/form_change/primal_reversion.c)

- `Primal Reversion happens after a mon is sent out after a mon is fainted` ([source](battle/form_change/primal_reversion.c#L120)): Faint replacement and Primal Reversion use a different event/message sequence. Re-enable after their ordering is aligned.
- `Primal Reversion happens after a switch-in caused by Eject Button` ([source](battle/form_change/primal_reversion.c#L158)): Eject Button switching and Primal Reversion use a different event/message sequence. Re-enable after their ordering is aligned.
- `Primal Reversion happens after a switch-in caused by Red Card` ([source](battle/form_change/primal_reversion.c#L181)): Red Card switching and Primal Reversion use a different event/message sequence. Re-enable after their ordering is aligned.
- `Primal Reversion happens after the entry hazards damage` ([source](battle/form_change/primal_reversion.c#L203)): Entry-hazard damage and Primal Reversion use a different event/message sequence. Re-enable after their ordering is aligned.

#### [`battle/form_change/ultra_burst.c`](battle/form_change/ultra_burst.c)

- `Dusk Mane Necrozma can Ultra Burst holding Ultranecrozium Z` ([source](battle/form_change/ultra_burst.c#L4)): Ultra Burst's activation message sequence differs from this expectation. Re-enable after Ultra Burst messaging is aligned.
- `Ultra Burst's order is determined by Speed - opponent faster` ([source](battle/form_change/ultra_burst.c#L22)): Ultra Burst's speed-order message sequence differs from this expectation. Re-enable after Ultra Burst ordering is aligned.
- `Ultra Burst's order is determined by Speed - player faster` ([source](battle/form_change/ultra_burst.c#L43)): Ultra Burst's speed-order message sequence differs from this expectation. Re-enable after Ultra Burst ordering is aligned.
- `Ultra Burst happens after switching, but before Focus Punch-like Moves` ([source](battle/form_change/ultra_burst.c#L80)): Switching, Ultra Burst, and Focus Punch setup use a different event/message sequence. Re-enable after their ordering is aligned.
- `Ultra Burst and Mega Evolution can happen on the same turn` ([source](battle/form_change/ultra_burst.c#L110)): Ultra Burst and Mega Evolution use a different same-turn message sequence. Re-enable after their ordering is aligned.

### Battle gimmicks

#### [`battle/gimmick/dynamax.c`](battle/gimmick/dynamax.c)

- `Dynamax: Dynamax expires when fainted` ([source](battle/gimmick/dynamax.c#L71)): Dynamax's faint-time reversion log does not match this game. Re-enable after reversion behavior and messages align.
- `Dynamax: Dynamaxed Pokemon cannot be flinched` ([source](battle/gimmick/dynamax.c#L187)): The Dynamax flinch-immunity scenario emits an unexpected battle log. Re-enable after the mechanic and messages align.
- `Dynamax: Dynamaxed Pokemon are affected by Grudge` ([source](battle/gimmick/dynamax.c#L236)): The Dynamax and Grudge scenario emits an unexpected battle log. Re-enable after the mechanic and messages align.
- `Dynamax: Dynamaxed Pokemon are not affected by phazing moves, but still take damage` ([source](battle/gimmick/dynamax.c#L253)): The Dynamax phazing scenario emits an unexpected battle log. Re-enable after phazing behavior and messages align.
- `Dynamax: Dynamaxed Pokemon are not affected by phazing moves but no block message is printed if they faint` ([source](battle/gimmick/dynamax.c#L277)): The fainting Dynamax phazing scenario emits an unexpected battle log. Re-enable after phazing behavior and messages align.
- `Dynamax: Dynamaxed Pokemon can be switched out by Eject Button` ([source](battle/gimmick/dynamax.c#L297)): The Dynamax Eject Button scenario emits an unexpected battle log. Re-enable after switch behavior and messages align.
- `Dynamax: Dynamaxed Pokemon can have their ability changed or suppressed` ([source](battle/gimmick/dynamax.c#L334)): The Dynamax ability-change scenario emits an unexpected battle log. Re-enable after ability behavior and messages align.
- `Dynamax: Dynamaxed Pokemon are not immune to Knock Off` ([source](battle/gimmick/dynamax.c#L403)): The Dynamax Knock Off scenario emits an unexpected battle log. Re-enable after item-removal behavior and messages align.
- `Dynamax: Dynamaxed Pokemon lose their substitutes` ([source](battle/gimmick/dynamax.c#L421)): The Dynamax Substitute scenario emits an unexpected battle log. Re-enable after Substitute behavior and messages align.
- `Dynamax: Feint bypasses Max Guard but doesn't break it` ([source](battle/gimmick/dynamax.c#L514)): The Feint and Max Guard scenario emits an unexpected battle log. Re-enable after guard behavior and messages align.
- `Dynamax: Dynamaxed Pokemon are immune to Instruct` ([source](battle/gimmick/dynamax.c#L537)): The Dynamax Instruct-immunity scenario emits an unexpected battle log. Re-enable after move-selection behavior and messages align.
- `Dynamax: Dynamaxed Pokemon are not affected by Choice items` ([source](battle/gimmick/dynamax.c#L557)): The Dynamax Choice item scenario emits an unexpected battle log. Re-enable after item behavior and messages align.
- `Dynamax: Dynamaxed Pokemon cannot use Max Guard while holding Assault Vest` ([source](battle/gimmick/dynamax.c#L580)): The Assault Vest and Max Guard scenario emits an unexpected battle log. Re-enable after move restrictions and messages align.
- `Dynamax: Max Knuckle raises both allies' attack` ([source](battle/gimmick/dynamax.c#L762)): Max Knuckle's ally stat-boost log does not match this game. Re-enable after stat changes and messages align.
- `Dynamax: Max Flare sets up sunlight` ([source](battle/gimmick/dynamax.c#L807)): Max Flare's weather-setting log does not match this game. Re-enable after weather behavior and messages align.
- `Dynamax: Max Geyser sets up heavy rain` ([source](battle/gimmick/dynamax.c#L825)): Max Geyser's weather-setting log does not match this game. Re-enable after weather behavior and messages align.
- `Dynamax: Max Hailstorm sets up hail` ([source](battle/gimmick/dynamax.c#L846)): Max Hailstorm's weather-setting log does not match this game. Re-enable after weather behavior and messages align.
- `Dynamax: Max Rockfall sets up a sandstorm` ([source](battle/gimmick/dynamax.c#L871)): Max Rockfall's weather-setting log does not match this game. Re-enable after weather behavior and messages align.
- `Dynamax: Max Overgrowth sets up Grassy Terrain` ([source](battle/gimmick/dynamax.c#L889)): Max Overgrowth's terrain-setting log does not match this game. Re-enable after terrain behavior and messages align.
- `Dynamax: Max Mindstorm sets up Psychic Terrain` ([source](battle/gimmick/dynamax.c#L912)): Max Mindstorm's terrain-setting log does not match this game. Re-enable after terrain behavior and messages align.
- `Dynamax: Max Lightning sets up Electric Terrain` ([source](battle/gimmick/dynamax.c#L931)): Max Lightning's terrain-setting log does not match this game. Re-enable after terrain behavior and messages align.
- `Dynamax: Max Starfall sets up Misty Terrain` ([source](battle/gimmick/dynamax.c#L948)): Max Starfall's terrain-setting log does not match this game. Re-enable after terrain behavior and messages align.
- `Dynamax: G-Max Stonesurge sets up Stealth Rocks` ([source](battle/gimmick/dynamax.c#L965)): G-Max Stonesurge's hazard-setting log does not match this game. Re-enable after hazard behavior and messages align.
- `Dynamax: G-Max Steelsurge sets up sharp steel` ([source](battle/gimmick/dynamax.c#L987)): G-Max Steelsurge's hazard-setting log does not match this game. Re-enable after hazard behavior and messages align.
- `Dynamax: G-Max Volt Crash paralyzes both opponents` ([source](battle/gimmick/dynamax.c#L1037)): G-Max Volt Crash's spread paralysis log does not match this game. Re-enable after status behavior and messages align.
- `Dynamax: G-Max Stun Shock paralyzes or poisons both opponents` ([source](battle/gimmick/dynamax.c#L1063)): G-Max Stun Shock's spread status log does not match this game. Re-enable after status behavior and messages align.
- `Dynamax: G-Max Befuddle paralyzes, poisons, or sleeps both opponents` ([source](battle/gimmick/dynamax.c#L1134)): G-Max Befuddle's spread status log does not match this game. Re-enable after status behavior and messages align.
- `Dynamax: G-Max Gold Rush confuses both opponents and generates money` ([source](battle/gimmick/dynamax.c#L1185)): G-Max Gold Rush's spread confusion log does not match this game. Re-enable after status behavior and messages align.
- `Dynamax: G-Max Smite confuses both opponents` ([source](battle/gimmick/dynamax.c#L1207)): G-Max Smite's spread confusion log does not match this game. Re-enable after status behavior and messages align.
- `Dynamax: G-Max Cuddle infatuates both opponents, if possible` ([source](battle/gimmick/dynamax.c#L1228)): G-Max Cuddle's spread infatuation log does not match this game. Re-enable after status behavior and messages align.
- `Dynamax: G-Max Terror traps both opponents` ([source](battle/gimmick/dynamax.c#L1251)): G-Max Terror's spread trapping log does not match this game. Re-enable after trapping behavior and messages align.
- `Dynamax: G-Max Meltdown torments both opponents for 3 turns` ([source](battle/gimmick/dynamax.c#L1289)): G-Max Meltdown's spread torment log does not match this game. Re-enable after status behavior and messages align.
- `Dynamax: G-Max Wildfire sets a field effect that damages non-Fire types` ([source](battle/gimmick/dynamax.c#L1327)): G-Max Wildfire's field-effect log does not match this game. Re-enable after residual damage behavior and messages align.
- `Dynamax: G-Max Replenish recycles allies' berries 50\% of the time` ([source](battle/gimmick/dynamax.c#L1375)): G-Max Replenish never produces the expected berry-recovery outcome. Re-enable after its recovery chance operates correctly.
- `Dynamax: G-Max Snooze makes only the target drowsy` ([source](battle/gimmick/dynamax.c#L1408)): G-Max Snooze never produces the expected drowsiness outcome. Re-enable after its status chance operates correctly.
- `Dynamax: G-Max Finale heals allies by 1/6 of their health` ([source](battle/gimmick/dynamax.c#L1434)): G-Max Finale's ally-healing log does not match this game. Re-enable after healing behavior and messages align.
- `Dynamax: G-Max Centiferno traps both opponents in Fire Spin` ([source](battle/gimmick/dynamax.c#L1484)): G-Max Centiferno's spread trapping log does not match this game. Re-enable after trapping behavior and messages align.
- `Dynamax: G-Max Chi Strike boosts allies' crit chance by 1 stage` ([source](battle/gimmick/dynamax.c#L1513)): G-Max Chi Strike's ally-boost log does not match this game. Re-enable after stat behavior and messages align.
- `Dynamax: G-Max Depletion takes away 2 PP from the target's last move` ([source](battle/gimmick/dynamax.c#L1548)): G-Max Depletion's PP-reduction log does not match this game. Re-enable after PP behavior and messages align.
- `Dynamax: G-Max One Blow bypasses Max Guard for full damage` ([source](battle/gimmick/dynamax.c#L1573)): G-Max One Blow's Max Guard scenario emits an unexpected battle log. Re-enable after guard behavior and messages align.
- `Dynamax: Moxie clones can be triggered by Max Moves fainting opponents` ([source](battle/gimmick/dynamax.c#L1636)): The Dynamax Moxie-clone KO log does not match this game. Re-enable after KO behavior and messages align.
- `Dynamax: G-Max Finale heals allies by 1/6 of their health, even if it faints the foe` ([source](battle/gimmick/dynamax.c#L1804)): G-Max Finale after a KO emits an unexpected battle log. Re-enable after healing behavior and messages align.
- `Dynamax: G-Max Replenish recycles allies' berries 50\% of the time, even if it faints the foe` ([source](battle/gimmick/dynamax.c#L1828)): G-Max Replenish after a KO never produces the expected berry-recovery outcome. Re-enable after its recovery chance operates correctly.
- `Dynamax: G-Max Volt Crash paralyzes other opponent even if its target faints` ([source](battle/gimmick/dynamax.c#L1860)): G-Max Volt Crash after a KO emits an unexpected battle log. Re-enable after spread status behavior and messages align.

#### [`battle/gimmick/zmove.c`](battle/gimmick/zmove.c)

- `(Z-MOVE) Genesis Supernova sets up psychic terrain` ([source](battle/gimmick/zmove.c#L586)): Genesis Supernova's terrain-blocking message sequence differs from this expectation. Re-enable after its terrain behavior is aligned.
- `(Z-MOVE) Genesis Supernova sets up psychic terrain when the target is behind a Substitute` ([source](battle/gimmick/zmove.c#L605)): Genesis Supernova, Substitute, and Psychic Terrain use a different message sequence. Re-enable after their interaction is aligned.

### Held item effects

#### [`battle/hold_effect/air_balloon.c`](battle/hold_effect/air_balloon.c)

- `Air Balloon prevents the holder from taking damage from ground type moves` ([source](battle/hold_effect/air_balloon.c#L12)): Air Balloon's immunity event/message sequence differs from this expectation. Re-enable after its battle behavior is aligned.
- `Air Balloon only displays entry message when user switches in` ([source](battle/hold_effect/air_balloon.c#L29)): Air Balloon's entry-message behavior differs from this expectation. Re-enable after its switch-in messaging is aligned.
- `Air Balloon pops when the holder is hit by a move that is not ground type` ([source](battle/hold_effect/air_balloon.c#L45)): Air Balloon's pop event/message sequence differs from this expectation. Re-enable after its hit handling is aligned.
- `Air Balloon no longer prevents the holder from taking damage from ground type moves once it has been popped` ([source](battle/hold_effect/air_balloon.c#L61)): Air Balloon popping and subsequent Ground-move handling differ from this expectation. Re-enable after that sequence is aligned.
- `Air Balloon can not be restored with Recycle after it has been popped` ([source](battle/hold_effect/air_balloon.c#L81)): Air Balloon consumption and Recycle use a different message sequence. Re-enable after their item-state behavior is aligned.
- `Air Balloon prevents the user from being healed by Grassy Terrain` ([source](battle/hold_effect/air_balloon.c#L102)): Air Balloon grounding and Grassy Terrain healing differ from this expectation. Re-enable after their interaction is aligned.
- `Air Balloon pops before it can be stolen with Magician` ([source](battle/hold_effect/air_balloon.c#L117)): Air Balloon popping and Magician's item handling differ from this expectation. Re-enable after their ordering is aligned.
- `Air Balloon pops before it can be stolen by Thief` ([source](battle/hold_effect/air_balloon.c#L133)): Air Balloon popping and Thief's item handling differ from this expectation. Re-enable after their ordering is aligned.

#### [`battle/hold_effect/booster_energy.c`](battle/hold_effect/booster_energy.c)

- `Booster Energy activates Quark Drive and increases highest stat` ([source](battle/hold_effect/booster_energy.c#L125)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/hold_effect/cure_status.c`](battle/hold_effect/cure_status.c)

- `Opponent Pokemon can be further poisoned with Toxic spikes after a status healing hold effect was previously used` ([source](battle/hold_effect/cure_status.c#L229)): The opponent Toxic Spikes and status-healing item log does not match this game. Re-enable after item and hazard behavior align.
- `Player Pokemon can be further poisoned with Toxic spikes after a status healing hold effect was previously used` ([source](battle/hold_effect/cure_status.c#L271)): The player Toxic Spikes and status-healing item log does not match this game. Re-enable after item and hazard behavior align.

#### [`battle/hold_effect/eject_pack.c`](battle/hold_effect/eject_pack.c)

- `Eject Pack does not cause the new Pokémon to lose HP due to it's held Life Orb` ([source](battle/hold_effect/eject_pack.c#L9)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Eject Pack is triggered by self-inflicting stat decreases` ([source](battle/hold_effect/eject_pack.c#L49)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/hold_effect/shed_shell.c`](battle/hold_effect/shed_shell.c)

- `Shed Shell allows switching out even when trapped by Mean Look` ([source](battle/hold_effect/shed_shell.c#L9)): Shed Shell and Mean Look use a different switching-message sequence. Re-enable after their trap override is aligned.
- `Shed Shell allows switching out even when trapped by Shadow Tag` ([source](battle/hold_effect/shed_shell.c#L27)): Shed Shell and Shadow Tag use a different switching-message sequence. Re-enable after their trap override is aligned.
- `Shed Shell allows switching out even when trapped by Arena Trap` ([source](battle/hold_effect/shed_shell.c#L43)): Shed Shell and Arena Trap use a different switching-message sequence. Re-enable after their trap override is aligned.

#### [`battle/hold_effect/shell_bell.c`](battle/hold_effect/shell_bell.c)

- `Shell Bell does not activate on Future Sight if the original user is on the field` ([source](battle/hold_effect/shell_bell.c#L175)): Future Sight and Shell Bell use a different delayed-attack message sequence. Re-enable after their interaction is aligned.

#### [`battle/hold_effect/terrain_seed.c`](battle/hold_effect/terrain_seed.c)

- `Electric Seed doesn't activate on existing Electric Terrain before user's ability changes the terrain` ([source](battle/hold_effect/terrain_seed.c#L184)): The Electric Seed terrain-change switch log does not match this game. Re-enable after item and terrain behavior align.

### Battle item effects

#### [`battle/item_effect/revive.c`](battle/item_effect/revive.c)

- `Revive restores a fainted battler's HP to half` ([source](battle/item_effect/revive.c#L4)): Revive's restoration message does not match this game. Re-enable after item behavior and messages align.
- `Max Revive restores a fainted battler's HP fully` ([source](battle/item_effect/revive.c#L24)): Max Revive's restoration message does not match this game. Re-enable after item behavior and messages align.
- `Revival Herb restores a fainted battler's HP fully` ([source](battle/item_effect/revive.c#L44)): Revival Herb's restoration message does not match this game. Re-enable after item behavior and messages align.
- `Max Honey restores a fainted battler's HP fully` ([source](battle/item_effect/revive.c#L64)): Max Honey's restoration message does not match this game. Re-enable after item behavior and messages align.
- `Revive works for a partner in a double battle` ([source](battle/item_effect/revive.c#L85)): The partner Revive scenario emits an unexpected battle log. Re-enable after revive behavior and messages align.

### Move effects

#### [`battle/move_effect/acupressure.c`](battle/move_effect/acupressure.c)

- `Acupressure fails on the user if it targeted its ally but switched positions via Ally Switch` ([source](battle/move_effect/acupressure.c#L11)): Acupressure and Ally Switch use a different target-resolution message sequence. Re-enable after their interaction is aligned.
- `Acupressure works on the ally if it targeted itself but switched positions via Ally Switch` ([source](battle/move_effect/acupressure.c#L35)): Acupressure and Ally Switch use a different target-resolution message sequence. Re-enable after their interaction is aligned.

#### [`battle/move_effect/belch.c`](battle/move_effect/belch.c)

- `Belch can still be used after switching out` ([source](battle/move_effect/belch.c#L56)): Belch after switching emits an unexpected battle log. Re-enable after switch behavior and messages align.
- `Belch can still be used after fainting` ([source](battle/move_effect/belch.c#L78)): Belch after fainting emits an unexpected battle log. Re-enable after faint handling and messages align.

#### [`battle/move_effect/conversion_2.c`](battle/move_effect/conversion_2.c)

- `Conversion 2 randomly changes the type of the user to a type that resists the last move that hit the user (Gen 1-4)` ([source](battle/move_effect/conversion_2.c#L6)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Conversion 2's type change considers Struggle to be Normal type (Gen 1-4)` ([source](battle/move_effect/conversion_2.c#L28)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Conversion 2 randomly changes the type of the user to a type that resists the last used target's move (Gen 5+)` ([source](battle/move_effect/conversion_2.c#L52)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Conversion 2's type change considers status moves (Gen 5+)` ([source](battle/move_effect/conversion_2.c#L74)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Conversion 2's type change considers the type of moves called by other moves` ([source](battle/move_effect/conversion_2.c#L97)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Conversion 2's type change considers dynamic type moves` ([source](battle/move_effect/conversion_2.c#L120)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Conversion 2's type change considers move types changed by Normalize and Electrify` ([source](battle/move_effect/conversion_2.c#L145)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Conversion 2's type change fails targeting Struggle (Gen 5+)` ([source](battle/move_effect/conversion_2.c#L179)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Conversion 2 fails if the move used is of typeless damage (Gen 5+)` ([source](battle/move_effect/conversion_2.c#L200)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Conversion 2 fails if the targeted move is Stellar Type` ([source](battle/move_effect/conversion_2.c#L224)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Conversion 2 fails if last hit by a Stellar-type move (Gen 1-4)` ([source](battle/move_effect/conversion_2.c#L258)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/court_change.c`](battle/move_effect/court_change.c)

- `Court Change swaps entry hazards used by the opponent` ([source](battle/move_effect/court_change.c#L9)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Court Change swaps entry hazards used by the player` ([source](battle/move_effect/court_change.c#L48)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Court Change used by the player swaps Mist, Safeguard, Aurora Veil, Reflect, Light Screen, Tailwind` ([source](battle/move_effect/court_change.c#L87)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Court Change used by the opponent swaps Mist, Safeguard, Aurora Veil, Reflect, Light Screen, Tailwind` ([source](battle/move_effect/court_change.c#L129)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Court Change used by the player swaps G-Max Steelsurge` ([source](battle/move_effect/court_change.c#L171)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Court Change used by the player swaps G-Max Vine Lash, G-Max Wildfire, G-Max Cannonade` ([source](battle/move_effect/court_change.c#L195)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/defog.c`](battle/move_effect/defog.c)

- `Defog fails if target has minimum evasion stat change behind Substitute (Gen4)` ([source](battle/move_effect/defog.c#L78)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Defog removes Reflect and Light Screen from target's side` ([source](battle/move_effect/defog.c#L173)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Defog removes Mist and Safeguard from target's side` ([source](battle/move_effect/defog.c#L241)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Defog removes Stealth Rock and Sticky Web from target's side` ([source](battle/move_effect/defog.c#L286)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Defog removes Stealth Rock and Sticky Web from user's side (Gen 6+)` ([source](battle/move_effect/defog.c#L340)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Defog removes Spikes from user's side (Gen 6+)` ([source](battle/move_effect/defog.c#L428)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Defog removes Toxic Spikes from target's side` ([source](battle/move_effect/defog.c#L515)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Defog removes Toxic Spikes from user's side (Gen 6+)` ([source](battle/move_effect/defog.c#L553)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Defog removes everything it can` ([source](battle/move_effect/defog.c#L677)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/echoed_voice.c`](battle/move_effect/echoed_voice.c)

- `Echoed Voice's power is increased even if it misses` ([source](battle/move_effect/echoed_voice.c#L117)): Echoed Voice miss handling uses a different battle-message sequence. Re-enable after its consecutive-use behavior is aligned.

#### [`battle/move_effect/embargo.c`](battle/move_effect/embargo.c)

- `Embargo doesn't prevent Mega Evolution` ([source](battle/move_effect/embargo.c#L340)): The Embargo and Mega Evolution battle log does not match this game. Re-enable after transformation behavior and messages align.

#### [`battle/move_effect/encore.c`](battle/move_effect/encore.c)

- `Encore forces the last move used while asleep` ([source](battle/move_effect/encore.c#L118)): Encore's asleep-target animation does not match this game. Re-enable after Encore behavior and animation align.
- `(DYNAMAX) Dynamaxed Pokemon are immune to Encore` ([source](battle/move_effect/encore.c#L140)): The Dynamax Encore-immunity log does not match this game. Re-enable after Encore behavior and messages align.
- `(DYNAMAX) Dynamaxed Pokemon can be encored immediately after reverting` ([source](battle/move_effect/encore.c#L158)): The post-Dynamax Encore scenario emits an unexpected battle log. Re-enable after reverting behavior and messages align.

#### [`battle/move_effect/fling.c`](battle/move_effect/fling.c)

- `Fling's thrown item can be regained with Recycle` ([source](battle/move_effect/fling.c#L115)): Fling and Recycle use a different item-recovery message sequence. Re-enable after their item-state behavior is aligned.
- `Fling - Item is lost even when there is no target` ([source](battle/move_effect/fling.c#L140)): Fling with no target uses a different item-loss message sequence. Re-enable after its targetless behavior is aligned.
- `Fling - Item is lost when target protects itself` ([source](battle/move_effect/fling.c#L167)): Fling and Protect use a different item-loss message sequence. Re-enable after their interaction is aligned.
- `Fling doesn't consume the item if the user is asleep/frozen/paralyzed` ([source](battle/move_effect/fling.c#L208)): Fling's status-prevention handling differs from this expectation. Re-enable after status and item-consumption behavior is aligned.
- `Fling applies special effects when throwing specific Items` ([source](battle/move_effect/fling.c#L261)): Fling's item-specific secondary-effect messages differ from this expectation. Re-enable after those effects are aligned.
- `Fling - thrown berry's effect activates for the target even if the trigger conditions are not met` ([source](battle/move_effect/fling.c#L404)): Fling's thrown-Berry activation sequence differs from this expectation. Re-enable after the target item effects are aligned.

#### [`battle/move_effect/focus_punch.c`](battle/move_effect/focus_punch.c)

- `Focus Punch does not activate when Focus Band/Focus Sash/Sturdy prevent getting one-shot by an attack` ([source](battle/move_effect/focus_punch.c#L140)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/grudge.c`](battle/move_effect/grudge.c)

- `Grudge's effect disappears if the user takes a new turn - Paralysis` ([source](battle/move_effect/grudge.c#L158)): The configured paralysis RNG never produces the required full-paralysis result. Re-enable after battle status RNG is aligned.
- `Grudge's effect disappears if the user takes a new turn - Flinching` ([source](battle/move_effect/grudge.c#L194)): The configured King's Rock RNG never produces the required flinch. Re-enable after held-item secondary-effect RNG is aligned.

#### [`battle/move_effect/hidden_power.c`](battle/move_effect/hidden_power.c)

- `Hidden Power always triggers Counter instead of Mirror Coat (Gen 1-3)` ([source](battle/move_effect/hidden_power.c#L139)): Gen 1-3 Hidden Power and counter-move handling use a different message sequence. Re-enable after that behavior is aligned.

#### [`battle/move_effect/hit_escape.c`](battle/move_effect/hit_escape.c)

- `Hit Escape: U-turn switches the user out after Ice Face activates` ([source](battle/move_effect/hit_escape.c#L98)): The U-turn and Ice Face scenario emits an unexpected battle log. Re-enable after switch behavior and messages align.

#### [`battle/move_effect/hit_switch_target.c`](battle/move_effect/hit_switch_target.c)

- `Dragon Tail switches the target after Rocky Helmet and Iron Barbs` ([source](battle/move_effect/hit_switch_target.c#L73)): The Rocky Helmet message does not match this scenario. Re-enable after the contact-damage message flow is corrected.

#### [`battle/move_effect/metronome.c`](battle/move_effect/metronome.c)

- `Metronome picks a random move` ([source](battle/move_effect/metronome.c#L9)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Metronome's called powder move fails against Grass Types` ([source](battle/move_effect/metronome.c#L28)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Metronome's called multi-hit move hits multiple times` ([source](battle/move_effect/metronome.c#L53)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/misty_terrain.c`](battle/move_effect/misty_terrain.c)

- `Misty Terrain protects grounded battlers from non-volatile status conditions` ([source](battle/move_effect/misty_terrain.c#L4)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Misty Terrain lasts for 5 turns` ([source](battle/move_effect/misty_terrain.c#L66)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/octolock.c`](battle/move_effect/octolock.c)

- `Octolock decreases Defense and Sp. Def by at the end of the turn` ([source](battle/move_effect/octolock.c#L4)): Octolock's end-of-turn stat-drop log does not match this game. Re-enable after stat behavior and messages align.
- `Octolock reduction is prevented by Clear Body, White Smoke and Full Metal Body` ([source](battle/move_effect/octolock.c#L23)): Octolock's prevention-ability log does not match this game. Re-enable after stat prevention and messages align.
- `Octolock will not decrease Defense and Sp. Def further then minus six` ([source](battle/move_effect/octolock.c#L113)): Octolock's minimum-stat-stage log does not match this game. Re-enable after stat behavior and messages align.
- `Octolock ends after user that set the lock switches out` ([source](battle/move_effect/octolock.c#L164)): Octolock's switch-out end condition emits an unexpected battle log. Re-enable after lock behavior and messages align.

#### [`battle/move_effect/protect.c`](battle/move_effect/protect.c)

- `Protect: Wide Guard can not fail on consecutive turns (Gen6+)` ([source](battle/move_effect/protect.c#L551)): Consecutive Wide Guard fails under the Gen6+ configuration. Re-enable after consecutive-guard handling is aligned.
- `Protect: Quick Guard can not fail on consecutive turns (Gen6+)` ([source](battle/move_effect/protect.c#L620)): Consecutive Quick Guard fails under the Gen6+ configuration. Re-enable after consecutive-guard handling is aligned.

#### [`battle/move_effect/psychic_terrain.c`](battle/move_effect/psychic_terrain.c)

- `Psychic Terrain protects grounded battlers from priority moves` ([source](battle/move_effect/psychic_terrain.c#L4)): Psychic Terrain priority blocking uses a different battle-message sequence. Re-enable after the terrain interaction is aligned.
- `Psychic Terrain increases power of Psychic-type moves by 30/50 percent` ([source](battle/move_effect/psychic_terrain.c#L23)): Psychic Terrain's power-boost message sequence differs from this expectation. Re-enable after the terrain interaction is aligned.
- `Psychic Terrain doesn't blocks priority moves that target the user` ([source](battle/move_effect/psychic_terrain.c#L48)): Psychic Terrain and self-targeting priority moves use a different message sequence. Re-enable after their interaction is aligned.
- `Psychic Terrain doesn't block priority moves that target all battlers` ([source](battle/move_effect/psychic_terrain.c#L65)): Psychic Terrain and all-battler priority moves use a different message sequence. Re-enable after their interaction is aligned.
- `Psychic Terrain doesn't block priority moves that target all opponents` ([source](battle/move_effect/psychic_terrain.c#L81)): Psychic Terrain and all-opponent priority moves use a different message sequence. Re-enable after their interaction is aligned.
- `Psychic Terrain doesn't block priority moves that target allies` ([source](battle/move_effect/psychic_terrain.c#L97)): Psychic Terrain and ally-targeting priority moves use a different message sequence. Re-enable after their interaction is aligned.
- `Psychic Terrain doesn't block priority field moves` ([source](battle/move_effect/psychic_terrain.c#L115)): Psychic Terrain and field-targeting priority moves use a different message sequence. Re-enable after their interaction is aligned.
- `Psychic Terrain doesn't block priority moves against semi-invulnerable targets` ([source](battle/move_effect/psychic_terrain.c#L131)): Psychic Terrain and semi-invulnerable targets use a different animation sequence. Re-enable after their priority interaction is aligned.
- `Psychic Terrain lasts for 5 turns` ([source](battle/move_effect/psychic_terrain.c#L168)): Psychic Terrain's turn-expiration message sequence differs from this expectation. Re-enable after terrain duration behavior is aligned.

#### [`battle/move_effect/pursuit.c`](battle/move_effect/pursuit.c)

- `Pursuited mon correctly switches out after it got hit and activated ability Tangling Hair` ([source](battle/move_effect/pursuit.c#L463)): Pursuit, Tangling Hair, and switching use a different event/message sequence. Re-enable after their ordering is aligned.
- `Pursuited mon correctly switches out after it got hit and activated ability Tangling Hair - Doubles` ([source](battle/move_effect/pursuit.c#L484)): Double-battle Pursuit, Tangling Hair, and switching use a different event/message sequence. Re-enable after their ordering is aligned.
- `Pursuited mon correctly switches out after it got hit and activated ability Tangling Hair - Mirror Armor` ([source](battle/move_effect/pursuit.c#L510)): Pursuit, Tangling Hair, and Mirror Armor use a different event/message sequence. Re-enable after their ordering is aligned.
- `Pursuited mon correctly switches out after it got hit and activated ability Cotton Down` ([source](battle/move_effect/pursuit.c#L530)): Pursuit, Cotton Down, and switching use a different event/message sequence. Re-enable after their ordering is aligned.
- `Pursuit becomes a locked move after being used on switch-out while holding a Choice Item` ([source](battle/move_effect/pursuit.c#L565)): Pursuit switching and Choice-lock behavior use a different message sequence. Re-enable after their interaction is aligned.

#### [`battle/move_effect/quash.c`](battle/move_effect/quash.c)

- `Quash calculates correct turn order if only one Pokémon is left on the opposing side` ([source](battle/move_effect/quash.c#L47)): Quash turn ordering after a faint differs from this expectation. Re-enable after the remaining-battler order is aligned.

#### [`battle/move_effect/rage_fist.c`](battle/move_effect/rage_fist.c)

- `Rage Fist base power is not lost if user switches out` ([source](battle/move_effect/rage_fist.c#L154)): Rage Fist after switching emits an unexpected battle log. Re-enable after switch behavior and messages align.

#### [`battle/move_effect/recoil_if_miss.c`](battle/move_effect/recoil_if_miss.c)

- `Recoil if miss: Jump Kick has 50% recoil on miss` ([source](battle/move_effect/recoil_if_miss.c#L9)): Jump Kick miss and crash-damage messaging differ from this expectation. Re-enable after crash handling is aligned.
- `Recoil if miss: Jump Kick's recoil happens after Spiky Shield damage and Pokemon can faint from either of these` ([source](battle/move_effect/recoil_if_miss.c#L59)): Jump Kick, Spiky Shield, and faint handling use a different event/message sequence. Re-enable after their ordering is aligned.
- `Recoil if miss: Disguise doesn't prevent crash damage from Jump Kick into ghost types` ([source](battle/move_effect/recoil_if_miss.c#L139)): Jump Kick, Ghost immunity, and Disguise use a different crash-damage sequence. Re-enable after their interaction is aligned.

#### [`battle/move_effect/retaliate.c`](battle/move_effect/retaliate.c)

- `Retaliate works with passive damage` ([source](battle/move_effect/retaliate.c#L47)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/revival_blessing.c`](battle/move_effect/revival_blessing.c)

- `AI will not revive a partner's party member with Revival Blessing` ([source](battle/move_effect/revival_blessing.c#L52)): The AI partner Revival Blessing log does not match this game. Re-enable after AI targeting behavior and messages align.
- `Revival Blessing doesn't prevent revived battlers from losing their turn` ([source](battle/move_effect/revival_blessing.c#L90)): The revived-battler turn-loss scenario emits an unexpected battle log. Re-enable after revive timing and messages align.
- `Revival Blessing correctly updates battler absent flags` ([source](battle/move_effect/revival_blessing.c#L111)): Revival Blessing's absent-battler update log does not match this game. Re-enable after revive state handling and messages align.

#### [`battle/move_effect/shed_tail.c`](battle/move_effect/shed_tail.c)

- `Shed Tail creates a Substitute at the cost of 1/2 users maximum HP and switches the user out` ([source](battle/move_effect/shed_tail.c#L9)): Shed Tail's Substitute and switch-out message sequence differs from this expectation. Re-enable after its pivot behavior is aligned.
- `Shed Tail's HP cost can trigger a berry before the user switches out` ([source](battle/move_effect/shed_tail.c#L46)): Shed Tail's HP cost, Berry activation, and switch-out use a different sequence. Re-enable after their ordering is aligned.

#### [`battle/move_effect/shell_trap.c`](battle/move_effect/shell_trap.c)

- `Shell Trap activates immediately after being hit on turn 1 and attacks both opponents` ([source](battle/move_effect/shell_trap.c#L98)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Shell Trap activates immediately after being hit on turn 2 and attacks both opponents` ([source](battle/move_effect/shell_trap.c#L125)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Shell Trap targets correctly if one of the opponents has fainted` ([source](battle/move_effect/shell_trap.c#L176)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Shell Trap does not trigger when hit into Substitute` ([source](battle/move_effect/shell_trap.c#L266)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/soak.c`](battle/move_effect/soak.c)

- `Soak/Magic Powder's type change is overwitten if the target changes form` ([source](battle/move_effect/soak.c#L12)): Soak or Magic Powder and Disguise form change use a different message sequence. Re-enable after type restoration is aligned.

#### [`battle/move_effect/solar_beam.c`](battle/move_effect/solar_beam.c)

- `Solar Beam does not need a charging turn if Sun is up` ([source](battle/move_effect/solar_beam.c#L10)): Solar Beam in sun emits an unexpected battle log. Re-enable after weather behavior and messages align.
- `Solar Beam does half damage if Sandstorm is up (Gen3+)` ([source](battle/move_effect/solar_beam.c#L38)): Solar Beam in sandstorm emits an unexpected battle log. Re-enable after weather damage behavior and messages align.

#### [`battle/move_effect/spicy_extract.c`](battle/move_effect/spicy_extract.c)

- `Spicy Extract Defense loss is prevented by Big Pecks` ([source](battle/move_effect/spicy_extract.c#L65)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Spicy Extract stat changes will be inverted by Contrary` ([source](battle/move_effect/spicy_extract.c#L124)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Spicy Extract against Clear Amulet and Contrary raises Defense only` ([source](battle/move_effect/spicy_extract.c#L149)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/stomping_tantrum.c`](battle/move_effect/stomping_tantrum.c)

- `Stomping Tantrum will not deal double if it missed` ([source](battle/move_effect/stomping_tantrum.c#L118)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Stomping Tantrum will deal double damage if user was immune to previous move` ([source](battle/move_effect/stomping_tantrum.c#L142)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/strength_sap.c`](battle/move_effect/strength_sap.c)

- `Strength Sap lowers Attack by 1 and restores HP based on target's Attack Stat` ([source](battle/move_effect/strength_sap.c#L9)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Strength Sap works exactly the same when attacker is behind substitute` ([source](battle/move_effect/strength_sap.c#L37)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Strength Sap lowers Attack by 1 and restores HP based on target's Attack Stat and stat Change` ([source](battle/move_effect/strength_sap.c#L68)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Strength Sap restores more HP if Big Root is held` ([source](battle/move_effect/strength_sap.c#L155)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Strength Sap will drain users HP if target has Liquid Ooze` ([source](battle/move_effect/strength_sap.c#L241)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect/tailwind.c`](battle/move_effect/tailwind.c)

- `Tailwind doesn't affect the partner on the same turn it's used (Gen4-7)` ([source](battle/move_effect/tailwind.c#L44)): Pre-Gen 8 Tailwind turn recalculation differs from this expectation. Re-enable after same-turn ordering is aligned.
- `Tailwind affects the partner on the same turn it's used (Gen8+)` ([source](battle/move_effect/tailwind.c#L64)): Gen 8 Tailwind turn recalculation differs from this expectation. Re-enable after same-turn ordering is aligned.

#### [`battle/move_effect/take_heart.c`](battle/move_effect/take_heart.c)

- `Take Heart cures sleep when used by Sleep Talk` ([source](battle/move_effect/take_heart.c#L50)): Take Heart through Sleep Talk emits an unexpected battle log. Re-enable after sleep-curing behavior and messages align.

#### [`battle/move_effect/toxic_spikes.c`](battle/move_effect/toxic_spikes.c)

- `Toxic Spikes fails after 2 layers` ([source](battle/move_effect/toxic_spikes.c#L50)): Toxic Spikes' maximum-layer log does not match this game. Re-enable after hazard behavior and messages align.
- `Toxic Spikes inflicts poison on switch in after Primal Reversed mon fainted` ([source](battle/move_effect/toxic_spikes.c#L209)): Toxic Spikes after Primal Reversion emits an unexpected switch-in log. Re-enable after hazard behavior and messages align.

#### [`battle/move_effect/toxic.c`](battle/move_effect/toxic.c)

- `Toxic cannot miss if used by a Poison-type (Gen6+)` ([source](battle/move_effect/toxic.c#L46)): The current battle does not emit this test's expected animation. Re-enable after animation behavior is corrected.

#### [`battle/move_effect/upper_hand.c`](battle/move_effect/upper_hand.c)

- `Upper Hand fails if the target has attempted to act even if previously successful` ([source](battle/move_effect/upper_hand.c#L125)): Upper Hand and Instruct use a different prior-action message sequence. Re-enable after their interaction is aligned.

#### [`battle/move_effect/wish.c`](battle/move_effect/wish.c)

- `Wish restores 50% of the user's HP when switching (Gen5+)` ([source](battle/move_effect/wish.c#L30)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.
- `Wish restores 50% of the recipient's HP when switching (Gen3-4)` ([source](battle/move_effect/wish.c#L53)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

### Secondary move effects

#### [`battle/move_effect_secondary/aromatherapy.c`](battle/move_effect_secondary/aromatherapy.c)

- `Sparkly Swirl cures the entire party of the user from primary status effects` ([source](battle/move_effect_secondary/aromatherapy.c#L9)): Sparkly Swirl's party-cure log does not match this game. Re-enable after status-curing behavior and messages align.

#### [`battle/move_effect_secondary/ion_deluge.c`](battle/move_effect_secondary/ion_deluge.c)

- `Ion Duldge turns normal moves into electric for the remainder of the current turn` ([source](battle/move_effect_secondary/ion_deluge.c#L9)): Ion Deluge's type-change log does not match this game. Re-enable after type-change behavior and messages align.
- `Plasma Fists turns normal moves into electric for the remainder of the current turn` ([source](battle/move_effect_secondary/ion_deluge.c#L32)): Plasma Fists' type-change log does not match this game. Re-enable after type-change behavior and messages align.
- `Plasma Fists type-changing effect is applied after Normalize` ([source](battle/move_effect_secondary/ion_deluge.c#L91)): Plasma Fists after Normalize emits an unexpected battle log. Re-enable after type-change behavior and messages align.
- `Plasma Fists turns normal type dynamax-moves into electric type moves` ([source](battle/move_effect_secondary/ion_deluge.c#L110)): Plasma Fists with Dynamax moves emits an unexpected battle log. Re-enable after type-change behavior and messages align.

#### [`battle/move_effect_secondary/psychic_noise.c`](battle/move_effect_secondary/psychic_noise.c)

- `Psychic Noise blocks healing moves for 2 turns` ([source](battle/move_effect_secondary/psychic_noise.c#L10)): The current battle does not emit this test's expected message. Re-enable after the message flow is corrected.

#### [`battle/move_effect_secondary/throat_chop.c`](battle/move_effect_secondary/throat_chop.c)

- `Throat Chop prevents the usage of sound moves` ([source](battle/move_effect_secondary/throat_chop.c#L9)): Throat Chop's sound-move prevention message differs from this expectation. Re-enable after its prevention behavior is aligned.

### Combined move effects

#### [`battle/move_effects_combined/axe_kick.c`](battle/move_effects_combined/axe_kick.c)

- `Axe Kick deals damage half the hp to user if it fails` ([source](battle/move_effects_combined/axe_kick.c#L41)): Axe Kick miss and crash-damage messaging differ from this expectation. Re-enable after crash handling is aligned.
- `Axe Kick still deals crash damage when boosted by Sheer Force` ([source](battle/move_effects_combined/axe_kick.c#L59)): Axe Kick and Sheer Force use a different crash-damage message sequence. Re-enable after their interaction is aligned.

#### [`battle/move_effects_combined/mind_blown.c`](battle/move_effects_combined/mind_blown.c)

- `Mind Blown causes everyone to faint in a double battle` ([source](battle/move_effects_combined/mind_blown.c#L87)): The all-faint Mind Blown scenario emits an unexpected battle log. Re-enable after faint handling and messages align.
- `Mind Blown's recoil only happens once, regardless of number of affected targets` ([source](battle/move_effects_combined/mind_blown.c#L127)): Mind Blown's multi-target recoil log does not match this game. Re-enable after recoil behavior and messages align.

### Starting battle status

#### [`battle/starting_status/terrain.c`](battle/starting_status/terrain.c)

- `Terrain started after the one which started the battle lasts only 5 turns` ([source](battle/starting_status/terrain.c#L63)): The replacement-terrain battle log does not match this game. Re-enable after terrain duration behavior and messages align.

### Non-battle systems

#### [`daycare.c`](daycare.c)

- `(Daycare) Pokémon with regional forms give the correct offspring` ([source](daycare.c#L90)): Regional-form breeding follows the current-region and Everstone policy instead of this expected offspring table. Re-enable after that policy is reconciled.

#### [`save.c`](save.c)

- `SaveBlock1 is backwards compatible` ([source](save.c#L12)): Save block layout no longer matches the compatibility baseline. Re-enable after existing saves are preserved or migrated.
- `SaveBlock3 is backwards compatible` ([source](save.c#L25)): Save block layout no longer matches the compatibility baseline. Re-enable after existing saves are preserved or migrated.

#### [`species.c`](species.c)

- `Form species ID tables are shared between all forms` ([source](species.c#L6)): Raticate-Alola points to a different form-species table. Re-enable after all forms share their table.

#### [`text.c`](text.c)

- `Item names fit on PC Storage (list)` ([source](text.c#L147)): ELECTRIC TERA SHARD is one pixel wider than the PC Storage list allowance. Re-enable after the list layout or item text fits.
- `Item descriptions fit on Bag and Shop Screen` ([source](text.c#L243)): The FIRE ELECTRIC FIGHTING TERA SHARD description exceeds the Bag and Shop text allowance. Re-enable after the UI or text fits.

# Runtime test failure inventory

This table is the historical PR #6 inventory, copied verbatim as a triage queue. It is not a current-HEAD result or a diagnosis. Its 1,842 table rows have deliberately not been regenerated or reclassified during the current verification.

## Run

The historical document says it used:

```sh
UNUSED_ERROR=1 DEPRECATED_ERROR=1 GAME_VERSION=EMERALD TEST=1 make -j$(nproc) check
```

It identifies the result range as log lines 39939 through 45666 and says that `origin/master` resolved to `1fa87a19fc1ff1c45e920f1790f4f5268cb34187` while the inventory was assembled. The raw log and exact runtime checkout or environment evidence are unavailable, so those provenance claims cannot be independently verified.

Current CI is the `expansion-suite` job in `.github/workflows/ci.yml`, which runs:

```sh
TEST=1 UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -j"$(nproc)" check
```

The completed [PR #7 CI run](https://github.com/mzpkdev/pokemon-wayfarer/actions/runs/32980328525) against `5993e0982b` is the latest comparable full-suite aggregate because the [current-head attempt](https://github.com/mzpkdev/pokemon-wayfarer/actions/runs/32984211294) was cancelled before any job acquired a hosted runner. It reported 5,113 total tests: 2,556 `PASS`, 1,884 `FAIL`, 28 `ASSUMPTION_FAIL`, 11 `KNOWN_FAILING`, 628 `TO_DO`, and 6 `EXPECT_FAILING`. The process exited 2 because tests failed. The job completed without hitting the runner timeout. Focused results recorded below may come from later worktree changes and do not rewrite that aggregate.

Hydra can write an uncapped `mgba-rom-test-hydra/v1` NDJSON ledger. A local current-head `-j22` run captured all 5,149 terminal results, 26 diagnostics, and a final summary. Its file-level assumptions repeat once per worker, so its aggregate is not directly comparable to CI. The ledger supports current prioritization, but it does not automatically reclassify any row in this historical table.

The battle runner now adds exact expected and observed message bytes to that ledger without changing matching or gameplay. A second 5,149-result local report contains pending-message context for 1,881 records. It retained observed messages for 1,762 records, although a four-entry bound overwrote older observations in 1,344 of them. Exact charmap decoding identified 268 records whose retained expected/observed pair differed only in canonical species, move, ability, or item capitalization. Updating the corresponding exact expectations changed 382 assertions across 113 test files. A complete local rerun moved 169 tests from `FAIL` to `PASS`, with no reverse transitions, and reduced the aggregate to 1,724 failures. Pending-message context fell to 1,716 records.

Reconstructing that ledger found 99 failed result records with a retained case-only pair, correcting an earlier count of 78. Iterating through newly exposed assertions changed 144 exact expectations across 53 test files, including generic stat labels already rendered by the game. Three complete reruns moved another 71 tests from `FAIL` to `PASS` with no reverse transitions. The latest complete aggregate is 2,803 `PASS`, 1,653 `FAIL`, 46 `ASSUMPTION_FAIL`, 12 `KNOWN_FAILING`, 629 `TO_DO`, and 6 `EXPECT_FAILING`. Pending-message context fell to 1,647 records. Its two remaining retained case-only pairs were aligned afterward, and both exact focused tests pass. These results do not automatically reclassify rows in this historical inventory.

The 4,676-line local log at `game/build/triage-evidence/full-strict-final.log` is earlier pre-fix evidence. That Emerald `-j22` run compiled and started Hydra, then stopped before a final summary. Its 3,898 result lines were 1,945 `PASS`, 1,416 `FAIL`, 49 `ASSUMPTION_FAIL`, 8 `KNOWN_FAILING`, 5 `EXPECTED_FAIL`, 474 `TO_DO`, and 1 intentional `CRASH`; no `INVALID` result was observed. These partial counts do not supersede the completed PR #7 aggregate.

## Counting and deduplication

The historical document reports 1,886 recognized non-pass records and 1,842 distinct rows, keyed by exact title and status, after removing 44 repeats. The stated distinct-status arithmetic is valid: 1,791 `FAIL` + 16 `INVALID` + 1 `CRASH` + 25 `ASSUMPTION_FAIL` + 9 `KNOWN_FAILING` = 1,842. The reported 1,886 records and 44 repeats remain unverified because the raw log is unavailable.

- `FAIL`: 1,791 distinct rows (1,795 records)
- `INVALID`: 16 distinct rows (16 records)
- `CRASH`: 1 distinct row (1 record)
- `ASSUMPTION_FAIL`: 25 distinct rows (65 records)
- `KNOWN_FAILING`: 9 distinct rows (9 records)

## Current verification and limitations

- Every historical title maps to current test source. Seven mappings are declaration-ambiguous, so the mapping only proves source presence, not the exact test body or behavior. All nine `KNOWN_FAILING` titles and the sole `CRASH` marker map to current source.
- `KNOWN_FAILING_GEN` is absent from current source and upstream. Do not use it as a current marker or infer support for it from this inventory.
- The strict object build now compiles. The historical `CheckTogepi` missing-flag error and the unused Frontier healthbox-helper error are no longer current blockers.
- The completed PR #7 CI run is the latest comparable full-suite aggregate. The current-head attempt did not acquire a hosted runner, so it contains no test evidence. The comparable run executed all 5,113 tests, exited 2 after reporting failures, and did not time out.
- The NDJSON ledger retains every result and diagnostic beyond Hydra's bounded console summary. It is per-result current evidence, not a replacement for focused reproduction or an automatic reclassification of the historical inventory.
- Exact message diagnostics preserve queue matching and result behavior. HnS commit `73c788a6b1` capitalized canonical data names without updating many older message expectations. The two evidence-backed expectation batches changed 240 tests from `FAIL` to `PASS` in complete comparisons without a status regression. The final two retained case-only tests pass focused checks. Keep exact matching and do not revert production casing because that would change visible gameplay.
- The 4,676-line, 378,565-byte local log predates the PR #7 fixes. It remains partial pre-fix evidence and has no final status or exact elapsed time.
- `ClearSaveBlocks` zeroes SaveBlock3, where zero enables the One Type challenge with `TYPE_NONE` instead of the disabled state. Species are then rejected and sent to the PC. The test runner now sets the disabled sentinel after clearing it. The strict focused `pokemon.c` run exited 0 with 26 `PASS` and 1 unrelated `KNOWN_FAILING`; all `givemon` paths passed. The focused `pokerus.c` run exited 0 with 19 `PASS`. Neither run emitted a timeout or `INVALID` result.
- The Daycare helper intentionally deposits party slot 0 twice. `StorePokemonInDaycare` compacts the party after the first deposit, moving the other parent into slot 0. The second deposit should not use slot 1. The focused `daycare.c` run exited 2 with 4 `PASS` and 1 `FAIL`: regional forms at `test/daycare.c:166`, `EXPECT_EQ(965,52)`. It had no timeout or `INVALID` result. The fixture is correct; production `DetermineEggSpeciesAndParentSlots` uses current-region, Everstone, and regional-form tables. This is a deferred gameplay-policy discrepancy. No production code or test expectation changed because either could alter or hide live breeding behavior.
- The Dazzling Z-status fixture has matching static Baby-Doll Eyes and Fairium Z data, but cleared SaveBlock3 disables Fairy types at runtime. With its test-local Fairy setting enabled, the exact focused run exited 0 with 1 `PASS` and no timeout or `INVALID` result. The committed Poké Ball runner exception allows Ball items to omit an explicit party index. The exact Light Metal Heavy Ball and Heavy Metal Heavy Ball runs each exited 0 with 1 `PASS` and no timeout or `INVALID` result.
- Three more tests explicitly require Fairy typing and now enable it inside their own fixtures. The exact Focus Punch AI and weakness-berry runs each changed from `ASSUMPTION_FAIL` to `PASS`. The exact Pickpocket run now reaches the mechanic and reports an unmatched message instead of aborting on its Fairy-type assumption. No expectation or battle behavior changed.
- Additional test-local Fairy, modern Sturdy, and modern Sitrus fixtures now reach their intended setup. Inverse Battle, AI Encore, both Whimsicott immunity cases, Aura Break, Filter, type-power items, and Z-Nature Power pass focused checks. Remaining ordinary failures in Roost, Mimicry, Sturdy, Sitrus, and related cases stay open; no expectation or production mechanic changed.
- It only includes the recognized `FAIL`, `INVALID`, `CRASH`, `ASSUMPTION_FAIL`, and `KNOWN_FAILING` result lines in the historical range. `PASS`, `EXPECTED_FAIL`, and `TO_DO` results are excluded from this table.
- Three titles contain non-UTF-8 game-text bytes. The table writes those bytes as `\xNN` so the source text remains identifiable in a UTF-8 Markdown file.
- A result status alone does not identify the root cause. Record classification and supporting evidence in the empty triage and notes fields during investigation.

## Inventory

| Original log line | Status | Test title | Triage | Notes |
| ---: | --- | --- | --- | --- |
| 39939 | FAIL | (Daycare) Pokémon with regional forms give the correct offspring |  |  |
| 39940 | FAIL | givemon respects perfectIVCount but does overwrite fixed IVs (1) |  |  |
| 39942 | FAIL | (Daycare) Pokémon offspring species is based off the mother's species |  |  |
| 39943 | FAIL | givemon respects FORM_CHANGE_ITEM_HOLD |  |  |
| 39946 | FAIL | (Pokerus) Test PartySpreadPokerus: Pokerus can spread to and from eggs |  |  |
| 39947 | FAIL | (Pokerus) Test UpdatePartyPokerusTime general behavior 7/1536 |  |  |
| 39948 | FAIL | (Daycare) Shellos' form is always based on the mother's form |  |  |
| 39949 | FAIL | (Daycare) Pokémon generate Eggs of the lowest member of the evolutionary family |  |  |
| 39951 | FAIL | CalculateMonStats |  |  |
| 39952 | FAIL | (Pokerus) Test IsPokerusInParty general behavior |  |  |
| 39953 | FAIL | givemon respects perfectIVCount |  |  |
| 39957 | FAIL | givemon [moves] |  |  |
| 39962 | FAIL | givemon respects perfectIVCount but does overwrite fixed IVs (2) |  |  |
| 39964 | FAIL | (Daycare) Pokémon can breed with Ditto if they don't belong to the Ditto or No Eggs Discovered group |  |  |
| 39965 | CRASH | Tests resume after CRASH |  |  |
| 39966 | FAIL | (Pokerus) No infection when POKERUS_ENABLED is false |  |  |
| 39967 | FAIL | (Pokerus) Test PartySpreadPokerus general behavior |  |  |
| 39970 | FAIL | givemon [simple] |  |  |
| 39972 | FAIL | givemon [moves (default)] |  |  |
| 39977 | FAIL | Item descriptions fit on Bag and Shop Screen: (20/20) Good for fast\xA4 FIRE\xA4 ELECTRIC\xA4 and FIGHTING POK\xA0MON\xA1 |  |  |
| 39984 | INVALID | Capture: when CRITICAL_CAPTURE_IF_OWNED is enabled, failed capture of owned pokemon does not appear critical |  |  |
| 39987 | FAIL | (Pokerus) Test PartySpreadPokerus using gen2 adjacency |  |  |
| 39990 | FAIL | (Pokerus) Eggs can only be infected if POKERUS_INFECT_EGG is TRUE |  |  |
| 39996 | FAIL | givemon [vars] |  |  |
| 40000 | FAIL | (Pokerus) Test PartySpreadPokerus: strain 0 can be spread to if POKERUS_WEAK_VARIANT is true |  |  |
| 40003 | FAIL | (Pokerus) Test PartySpreadPokerus when POKERUS_SPREAD_DAYS_LEFT is set to GEN2 |  |  |
| 40005 | FAIL | Item names fit on Pokemon Storage System: (727/727) FRESH\xB0START MOCHI |  |  |
| 40007 | INVALID | Capture: Missing badge malus apply correcly in gen 8 |  |  |
| 40011 | FAIL | (Pokerus) Test POKERUS_HERD_IMMUNITY config in RandomlyGivePartyPokerus 2/2 |  |  |
| 40016 | FAIL | SaveBlock2 is backwards compatible |  |  |
| 40028 | FAIL | Damage calculation matches Gen5+ 1/16 |  |  |
| 40029 | KNOWN_FAILING | Pokémon level up learnsets fit within MAX_LEVEL_UP_MOVES and MAX_RELEARNER_MOVES |  |  |
| 40032 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up by Hydration in the rain |  |  |
| 40035 | FAIL | Form species ID tables are shared between all forms: (283/283) ID:957 - RATICATE\xB0A |  |  |
| 40037 | FAIL | (Pokerus) No infection when POKERUS_INFECT_AGAIN is false and you already have active pokerus in party |  |  |
| 40038 | INVALID | Capture: Low level catch bonus apply correcly with all gen configs |  |  |
| 40042 | FAIL | SaveBlock1 is backwards compatible |  |  |
| 40049 | FAIL | givemon [all] |  |  |
| 40050 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up by gaining the ability Insomnia / Vital Spirit 1/2 |  |  |
| 40052 | FAIL | Hazards are applied based on order of set up |  |  |
| 40053 | FAIL | Spread Moves: Focus Sash activates correctly |  |  |
| 40057 | FAIL | SaveBlock3 is backwards compatible |  |  |
| 40059 | FAIL | Sleep Clause: G-Max Befuddle can only sleep one opposing mon if sleep clause is active |  |  |
| 40069 | FAIL | Sleep Clause: Psycho Shift'ing sleep will activate sleep clause |  |  |
| 40070 | FAIL | Battle Message: Send-in message depends on foe HP 1/4 |  |  |
| 40074 | FAIL | End Turn Effects: First Event Block is executed correctly (2v1) |  |  |
| 40076 | FAIL | Transistor Damage calculation 1/32 |  |  |
| 40078 | FAIL | Sleep Clause: Rest does not activate sleep clause |  |  |
| 40093 | FAIL | Adaptability increases same-type attack bonus from x1.5 to x2 1/2 |  |  |
| 40788 | INVALID | Pokemon gain experience after catching a Pokemon (Gen6+) |  |  |
| 40797 | FAIL | Trainer Slide: Doubles: Player Lands First STAB Hit |  |  |
| 40804 | FAIL | Spread Moves: Spread move, Gem Boosted, vs Resist Berries |  |  |
| 40806 | FAIL | Aftermath ability pop-up will be displayed correctly: opponent point of view |  |  |
| 40807 | FAIL | A spread move will do correct damage to the second mon if the first target faints from first hit of the spread move (2v1) |  |  |
| 40808 | FAIL | Bad Dreams faints both sleeping Pokemon on player side |  |  |
| 40809 | FAIL | Anger Point raises Attack stage to maximum after receiving a critical hit |  |  |
| 40811 | FAIL | Hazards can trigger Emergency Exit and hazards still activate for other battlers |  |  |
| 40812 | FAIL | Damage calculation matches Gen5+ (Marshadow vs Mawile) 1/16 |  |  |
| 40824 | FAIL | Hazards can trigger Emergency Exit and other hazards don't activate |  |  |
| 40826 | FAIL | Item names fit on PC Storage (list): (777/777) ELECTRIC TERA SHARD |  |  |
| 40827 | INVALID | Capture: Incapacitated catch bonus apply correcly with all gen configs |  |  |
| 40831 | FAIL | Beads of Ruin's message displays correctly after all battlers fainted - Player |  |  |
| 40832 | FAIL | A spread move will do correct damage to the second mon if the first target faints from first hit of the spread move (1v2) |  |  |
| 40835 | FAIL | End Turn Effects: First Event Block is executed correctly (multibattle) |  |  |
| 40836 | FAIL | A spread move will do correct damage to the second mon if the first target faints from first hit of the spread move (multibattle) |  |  |
| 40838 | FAIL | Beads of Ruin's message displays correctly after all battlers fainted - Opponent |  |  |
| 40839 | FAIL | Switch-in abilities trigger in Speed Order after post-KO switch - Double Battle 1/3 |  |  |
| 40840 | FAIL | Big Pecks doesn't prevent Spectral Thief from resetting positive Defense stage changes |  |  |
| 40842 | FAIL | Damage calculation matches Gen6+ (Muscle Band, crit) 1/16 |  |  |
| 40848 | FAIL | Berserk raises Sp.Atk by 1 |  |  |
| 40850 | FAIL | Higher leveled Pokemon give more exp 1/2 |  |  |
| 40851 | ASSUMPTION_FAIL | Inverse battle reverses type matchups |  |  |
| 40852 | FAIL | A spread move will do correct damage to the second mon if the first target faints from first hit of the spread move (double battle) |  |  |
| 40856 | FAIL | Clear Body, Full Metal Body, and White Smoke don't prevent Speed reduction from Iron Ball 1/6 |  |  |
| 40857 | FAIL | Sleep Clause: Pre-existing sleep condition doesn't activate sleep clause |  |  |
| 40858 | FAIL | Trainer Slide: Multi: Z Move |  |  |
| 40859 | FAIL | Clear Body, Full Metal Body, and White Smoke don't prevent Topsy-Turvy 1/3 |  |  |
| 40860 | FAIL | Competitive activates after Sticky Web lowers Speed |  |  |
| 40861 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up with G-Max Sweetness |  |  |
| 40864 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up by Flinging a held item 1/2 |  |  |
| 40866 | FAIL | Contrary raises Attack when Intimidated in a single battle 1/2 |  |  |
| 40867 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up by using a held item 1/2 |  |  |
| 40869 | FAIL | Trainer Slide: Doubles: Enemy Mon Unaffected |  |  |
| 40875 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is sent out, has Trace, and Traces Insomnia / Vital spirit 1/2 |  |  |
| 40878 | ASSUMPTION_FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon wakes up |  |  |
| 40881 | FAIL | Sleep Clause: Yawn will fail when sleep clause is active |  |  |
| 40883 | FAIL | Commander Tatsugiri takes no damage from multi-target damaging moves |  |  |
| 40884 | FAIL | End Turn Effects: First Event Block is executed correctly (1v2) |  |  |
| 40886 | FAIL | Move names fit on Contest Screen: (570/570) PARABOLIC CHARGE |  |  |
| 40887 | FAIL | Competitive activates for each stat that is lowered |  |  |
| 40888 | FAIL | Costar copies an ally's stat stages upon entering battle |  |  |
| 40889 | FAIL | Sleep Clause: Rest can still be used when sleep clause is active (Doubles) |  |  |
| 40890 | FAIL | Contrary raises a stat after using a move which would normally lower it: Growl 1/2 |  |  |
| 40891 | FAIL | Trainer Slide: Multi: Player Lands First Super Effective Hit |  |  |
| 40893 | FAIL | Cursed Body does not stop a multistrike move mid-execution |  |  |
| 40894 | FAIL | End Turn Effects: First Event Block is executed correctly (double battle) |  |  |
| 40897 | FAIL | Spread Moves: Spread move vs Wide Guard |  |  |
| 40905 | FAIL | Sleep Clause: Sleep moves used after being Encore'd are prevented when sleep clause is active |  |  |
| 40907 | FAIL | Dancer copies Lunar Dance after the original user faints, but before the replacement is sent out |  |  |
| 40908 | FAIL | Curious Medicine resets ally's stat stages upon entering battle 1/2 |  |  |
| 40909 | FAIL | Sleep Clause: Magic Bounce'ing a sleep move activates sleep clause, and fails if sleep clause is active |  |  |
| 40910 | FAIL | (TERA) Terastallizing into a different type with Adaptability gives 2.0x STAB 1/2 |  |  |
| 40912 | FAIL | Dancer still activate after Red Card even if blocked by Suction Cups |  |  |
| 40914 | FAIL | Large exp gains are supported 1/3 |  |  |
| 40915 | FAIL | Switch-in abilities trigger in Speed Order after post-KO switch - 1v2 1/3 |  |  |
| 40916 | FAIL | Sleep Clause: If both Pokémon on one side are Yawn'd at the same time, one will fall asleep and the other will not |  |  |
| 40923 | FAIL | Sleep Clause: Rest does not activate sleep clause (Doubles) |  |  |
| 40924 | FAIL | Sleep Clause: Waking up after Rest doesn't deactivate sleep clause |  |  |
| 40932 | FAIL | Aerilate doesn't affect Natural Gift's type 1/2 |  |  |
| 40933 | FAIL | Trainer Slide: Doubles: Z Move |  |  |
| 40936 | FAIL | Spread Moves: Explosion, Gem Boosted, vs Resist Berries |  |  |
| 40946 | INVALID | Capture: when CRITICAL_CAPTURE_IF_OWNED is enabled, capture of owned pokemon always appear critical |  |  |
| 40952 | FAIL | Trainer Slide: Singles: Player Lands First Down |  |  |
| 40957 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon faints |  |  |
| 40959 | FAIL | Dark Aura's effect doesn't stack multiple times |  |  |
| 40960 | FAIL | Lucky Egg boosts gained exp points by 50% 1/2 |  |  |
| 40962 | FAIL | Switch-in abilities trigger in Speed Order after post-KO switch - multibattle 1/3 |  |  |
| 40966 | FAIL | Sleep Clause: Sleep Clause does not prevent sleeping your partner Pokémon with Yawn |  |  |
| 40968 | FAIL | Red Card activates before Eject Pack |  |  |
| 40969 | FAIL | Dazzling, Queenly Majesty and Armor Tail protect the user from priority moves 1/3 |  |  |
| 40972 | FAIL | Big Pecks doesn't prevent receiving negative Defense stage changes from Baton Pass |  |  |
| 40974 | FAIL | Toxic Spikes can be removed after fainting to other hazards |  |  |
| 40976 | FAIL | Red Card activates but fails if the attacker has Suction Cups |  |  |
| 40977 | FAIL | Disguised Mimikyu blocks a move after getting Gastro Acid Batton Passed 1/2 |  |  |
| 40979 | FAIL | Beads of Ruin doesn't activate when dragged out by Mold Breaker attacker 2/2 |  |  |
| 40981 | FAIL | Beads of Ruin's Sp. Def reduction is ignored by Gastro Acid 1/2 |  |  |
| 40982 | FAIL | Sleep Clause: Reflection moves (ie. Magic Coat) that reflect Dark Void only sleep one opposing Pokémon |  |  |
| 40983 | FAIL | Clear Body, Full Metal Body, and White Smoke don't prevent Speed reduction from paralysis 1/3 |  |  |
| 40984 | INVALID | Comatose boosts Dream Ball's multiplier |  |  |
| 40987 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up by using Sleep Talk into a status curing move 1/5 |  |  |
| 40989 | FAIL | Sleep Clause: Yawn'd Pokémon who's partner is slept before Yawn triggers will not fall asleep due to sleep clause being activated |  |  |
| 40990 | FAIL | Commander prevents Red Card from working while Commander is active |  |  |
| 40998 | FAIL | Bulletproof makes ballistic moves fail against the ability user |  |  |
| 40999 | FAIL | Competitive doesn't activate after Sticky Web lowers Speed if Court Changed (gen8) |  |  |
| 41002 | FAIL | Rowap Berry is triggered even if berry user dies |  |  |
| 41003 | FAIL | Big Pecks prevents Defense stage reduction from moves |  |  |
| 41004 | FAIL | Sleep Clause: Opponent Spore'ing player's partner after partner was Yawn'd by player does not prevent Spore's effect from sleeping partner and activating clause |  |  |
| 41005 | FAIL | Dry Skin heals 1/8th Max HP in Rain |  |  |
| 41008 | FAIL | Contrary raises Attack when Intimidated in a double battle 1/4 |  |  |
| 41012 | FAIL | Shed Shell allows switching out even when trapped by Mean Look |  |  |
| 41018 | FAIL | Bad Dreams causes the sleeping enemy Pokemon to lose 1/8 of HP 2/2 |  |  |
| 41027 | FAIL | Sleep Clause: Sleep Clause does not prevent sleeping your partner Pokémon with sleep effects |  |  |
| 41028 | FAIL | Color Change changes the user to Electric type if hit by a move while the opponent is under the effect of Electrify |  |  |
| 41029 | FAIL | Sleep Clause: Sleep moves fail when sleep clause is active |  |  |
| 41031 | FAIL | Cursed Body disables the move that called another move instead of the called move (2/2) |  |  |
| 41032 | FAIL | Filter reduces damage to Super Effective moves by 0.75 1/2 |  |  |
| 41033 | FAIL | Sleep Clause: Reflection moves (ie. Magic Coat) that reflect a sleep move activate sleep clause |  |  |
| 41035 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up by Healer (2/2) |  |  |
| 41036 | FAIL | Apicot Berry raises the holder's Sp. Def by one stage when HP drops to 1/4 or below 2/2 |  |  |
| 41037 | FAIL | Trainer Slide: Multi: Player Lands First Down |  |  |
| 41039 | FAIL | Spread Moves: Not very effective Message on both opposing mons |  |  |
| 41041 | FAIL | Commander clears when Dondozo is replaced and Tatsugiri can be hit |  |  |
| 41043 | FAIL | Commander still blocks forced switch after swallowed Tatsugiri faints 1/2 |  |  |
| 41044 | FAIL | Sleep Clause: Mold Breaker Pokémon sleeping Vital Spirit / Insomnia activates sleep clause 1/2 |  |  |
| 41055 | FAIL | Sleep Clause: Rest can still be used when sleep clause is active |  |  |
| 41056 | FAIL | Big Pecks is ignored by Mold Breaker |  |  |
| 41057 | FAIL | Sleep Clause: Suppressing and then sleeping Vital Spirit / Insomnia and switching back in deactivates sleep clause 1/2 |  |  |
| 41060 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up by using an item |  |  |
| 41062 | FAIL | Forecast transforms Castform in weather from its own move 1/4 |  |  |
| 41063 | FAIL | Commander Tatsugiri avoids moves targetted towards it |  |  |
| 41065 | FAIL | Spread Moves: A spread move attack will activate both resist berries |  |  |
| 41066 | FAIL | Trainer Slide: Multi: Last Switchin |  |  |
| 41067 | FAIL | (TERA) Terastallizing into the same type with Adaptability gives 2.25x STAB 1/2 |  |  |
| 41068 | FAIL | Commander doesn't prevent Imposter from working on a Commander Tatsugiri |  |  |
| 41070 | FAIL | Dazzling, Queenly Majesty and Armor Tail protect users partner from priority moves 1/3 |  |  |
| 41071 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up by Shed Skin 1/3 (2/2) |  |  |
| 41072 | FAIL | White Herb restores stats after Attack was lowered by Intimidate in singles |  |  |
| 41074 | FAIL | Commander will activate once Dondozo switches in |  |  |
| 41075 | FAIL | Anger Shell lowers Def/Sp.Def by 1 and raises Atk/Sp.Atk/Spd by 1 |  |  |
| 41082 | FAIL | Big Malasada heals a battler from any primary status 1/7 |  |  |
| 41086 | FAIL | Aerilate doesn't affect Judgment / Techno Blast / Multi-Attack's type 1/3 |  |  |
| 41088 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up forcefully by a move from an opponent |  |  |
| 41090 | FAIL | Trainer Slide: Multi: Mega Evolution |  |  |
| 41091 | FAIL | Contrary lowers a stat after using a move which would normally raise it: Belly Drum 1/2 |  |  |
| 41093 | FAIL | Burn Heal heals a battler from being burned |  |  |
| 41096 | FAIL | Spread Moves: Spread move vs Eiscue and Mimikyu with 1 Eject Button |  |  |
| 41097 | FAIL | Color Change changes the type when a Pokemon is hit by Future Sight |  |  |
| 41100 | FAIL | Trainer Slide: Doubles: Player Lands First Down |  |  |
| 41116 | FAIL | Dire Hit increases a battler's critical hit chance by 2 stages 1/9 (2/2) |  |  |
| 41118 | FAIL | Comatose may be suppressed if Pokémon transformed into a Pokémon with Comatose ability and was under the effects of Gastro Acid 1/4 |  |  |
| 41119 | FAIL | Bad Dreams faints both sleeping Pokemon on opponent side |  |  |
| 41120 | INVALID | Ball Fetch causes the Pokémon to pick up the last failed Ball at the end of the turn |  |  |
| 41127 | FAIL | Ice Face form change persists after switching out |  |  |
| 41134 | FAIL | Ceaseless Edge fails to set up hazards if user faints |  |  |
| 41135 | FAIL | Illusion breaks if the target faints |  |  |
| 41137 | FAIL | Battle Bond transforms player's Greninja - Singles 1/4 |  |  |
| 41139 | FAIL | Switch-in abilities trigger in Speed Order after post-KO switch - 2v1 1/3 |  |  |
| 41140 | FAIL | Chilly Reception does not switch the user out if replacements fainted |  |  |
| 41149 | INVALID | Ball Fetch only picks up the first failed ball, once per battle |  |  |
| 41152 | FAIL | Arena Trap doesn't prevent switch outs if the Pokémon is switched in the same turn the opponent decided to switch out |  |  |
| 41158 | FAIL | Conversion 2's type change considers Struggle to be Normal type (Gen 1-4) |  |  |
| 41161 | FAIL | Trainer Slide: Doubles: Mega Evolution |  |  |
| 41163 | FAIL | Clear Body, Full Metal Body, and White Smoke prevent intimidate 1/3 |  |  |
| 41164 | FAIL | Competitive correctly activates after Sticky Web lowers Speed if Court Changed (Gen8) |  |  |
| 41165 | FAIL | Intimidate activates when it's no longer affected by Neutralizing Gas - opponent caused switches 1/4 |  |  |
| 41167 | FAIL | Forecast transforms Castform in weather from a partner's move 1/4 |  |  |
| 41168 | FAIL | Clear Body, Full Metal Body, and White Smoke don't prevent receiving negative stat changes from Baton Pass 1/3 |  |  |
| 41171 | FAIL | Contrary raises stats after using a move which would normally lower them: Overheat 1/2 |  |  |
| 41174 | FAIL | Beads of Ruin increases damage taken by physical moves in Wonder Room 1/4 |  |  |
| 41180 | FAIL | Color Change changes the type of a Pokemon being hit by a move if the type of the move and the Pokemon are different |  |  |
| 41181 | FAIL | Dancer-called moves can be reflected by Magic Bounce |  |  |
| 41183 | FAIL | Cotton Down drops speed by one of opposing battler if hit by a damaging move |  |  |
| 41185 | FAIL | Aerilate doesn't override Electrify |  |  |
| 41187 | FAIL | Dancer can copy Teeter Dance and confuse both opposing targets |  |  |
| 41192 | FAIL | Anger Point does not trigger when a substitute takes the hit (Gen5+) |  |  |
| 41194 | FAIL | Red Card is still consumed but cannot force out Dondozo after swallowed Tatsugiri faints |  |  |
| 41196 | FAIL | Instruct-called moves keep their priority, which is considered for Dazzling, Queenly Majesty and Armor Tail 1/3 |  |  |
| 41199 | FAIL | Court Change swaps entry hazards used by the opponent |  |  |
| 41200 | FAIL | Commander Tatsugiri is not damaged by a double target move if Dondozo faints |  |  |
| 41201 | ASSUMPTION_FAIL | Dark Void inflicts 1-3 turns of sleep |  |  |
| 41203 | FAIL | Dancer-called moves do not update move to be called by Mimic |  |  |
| 41205 | FAIL | Item names fit on Pokemon Summary Screen: (692/692) ELECTRIC TERA SHARD |  |  |
| 41206 | FAIL | Competitive activates before White Herb 1/2 |  |  |
| 41207 | INVALID | Capture: Missing badge malus apply correcly in gen 9 |  |  |
| 41211 | FAIL | Commander Tatsugiri is still affected by Haze while controlling Dondozo |  |  |
| 41212 | FAIL | Hospitality user restores 25% of ally's health 1/2 |  |  |
| 41213 | FAIL | Mirror Armor triggers even if the attacking Pokemon also has Mirror Armor ability |  |  |
| 41216 | FAIL | Tail Whip lowers Defense by 1 stage 2/2 |  |  |
| 41220 | FAIL | Corrosion can poison Poison/Steel types if the Pokémon uses Baneful Bunker 1/2 |  |  |
| 41222 | FAIL | Dauntless Shield raises Defense by one stage |  |  |
| 41224 | FAIL | Ice Face doesn't transform Eiscue if Cloud Nine/Air Lock is on the field |  |  |
| 41225 | FAIL | Disguised Mimikyu takes damage from secondary damage without breaking the disguise - Stealth Rock 1/2 |  |  |
| 41226 | FAIL | Commander Tatsugiri will still take residual damage from a field effect while inside Dondozo |  |  |
| 41227 | FAIL | Hazards are applied correctly after a battler faints |  |  |
| 41228 | FAIL | Defiant activates for each stat that is lowered |  |  |
| 41229 | FAIL | Dancer doesn't activate if the original move missed |  |  |
| 41231 | FAIL | Neutralizing Gas is active for the duration of a Spread Move even if Neutralizing Gas is no longer on the field |  |  |
| 41232 | FAIL | Defog doesn't remove Mist or Safeguard from the user's side 1/2 |  |  |
| 41235 | FAIL | Defiant activates after Sticky Web lowers Speed if Court Changed (Gen9) |  |  |
| 41236 | FAIL | Illusion breaks if the attacker faints |  |  |
| 41239 | FAIL | (DYNAMAX) Dynamaxed Pokemon are not affected by Destiny Bond |  |  |
| 41244 | FAIL | Defiant sharply raises player's Attack after Intimidate 1/4 |  |  |
| 41245 | FAIL | Color Change changes the type to Normal when a Pokemon is hit by a forseen attack under the effect of Normalize |  |  |
| 41246 | FAIL | Normalize doesn't affect Natural Gift's type 1/2 |  |  |
| 41247 | FAIL | Contrary does not invert stat changes that have been Baton-passed |  |  |
| 41248 | FAIL | Battle Bond transforms player's Greninja when fainting its Ally 1/4 |  |  |
| 41250 | FAIL | Early Bird wakes up if 1 sleep turn is preset |  |  |
| 41254 | FAIL | Desolate Land blocks damaging Water-type moves |  |  |
| 41256 | FAIL | Comatose isn't affected by Mold Breaker, Turboblaze or Teravolt 1/3 |  |  |
| 41257 | FAIL | Overcoat blocks damage from hail |  |  |
| 41258 | FAIL | Embody Aspect activates when it's no longer effected by Neutralizing Gas |  |  |
| 41261 | FAIL | Clear Body, Full Metal Body, and White Smoke prevent Sticky Web effect on switchin 1/3 |  |  |
| 41264 | FAIL | Download doesn't activate if target hasn't been sent out yet 1/2 |  |  |
| 41265 | FAIL | Intimidate activates when it's no longer affected by Neutralizing Gas - fainted |  |  |
| 41266 | FAIL | Parental Bond converts multi-target moves into a two-strike move in Single Battles 1/2 |  |  |
| 41268 | FAIL | Comatose prevents status-inducing moves 1/4 |  |  |
| 41270 | FAIL | Pickpocket cannot steal from Sticky Hold |  |  |
| 41271 | FAIL | Desolate Land does not block a move if Pokémon is asleep and uses a Water-type move |  |  |
| 41272 | FAIL | Competitive sharply raises player's Sp. Atk after Intimidate 1/4 |  |  |
| 41273 | FAIL | Intimidate (opponent) lowers player's attack after switch out 1/2 |  |  |
| 41276 | FAIL | Sleep Clause: Spore'ing opponent after Yawn'ing partner does not prevent Yawn's effect from sleeping partner |  |  |
| 41279 | FAIL | Flower Veil's stat reduction protection considers Contrary |  |  |
| 41283 | FAIL | Leaf Guard prevents non-volatile status conditions in sun 1/4 |  |  |
| 41285 | FAIL | Cotton Down drops speed by one of all other battlers on the field |  |  |
| 41287 | FAIL | Sleep Clause: Magic Bounce reflecting Dark Void only sleeps one opposing Pokémon |  |  |
| 41288 | FAIL | Dry Skin heals 25% when hit by water type moves |  |  |
| 41290 | FAIL | Cursed Body can trigger if the attacker is behind a Substitute |  |  |
| 41292 | FAIL | Poison Touch has a 30% chance to poison when attacking with contact moves (2/2) |  |  |
| 41294 | FAIL | Effect Spore only inflicts status on contact 1/2 |  |  |
| 41296 | FAIL | Sleep Clause: Reflection moves (ie. Magic Coat) fail if sleep clause is active and they reflect a sleep move |  |  |
| 41302 | FAIL | Burn Up fails if the user isn't a Fire-type |  |  |
| 41303 | FAIL | Forecast transforms Castform back to normal under Cloud Nine/Air Lock 1/2 |  |  |
| 41304 | FAIL | Mirror Armor doesn't lower the stats of an attacking Pokemon with the Clear Body ability |  |  |
| 41305 | FAIL | Protosynthesis recalculates the boosted stat after Neutralizing Gas leaves the field |  |  |
| 41308 | FAIL | Fairy Aura increases the power of all Fairy-type attacks by 33% |  |  |
| 41309 | INVALID | Dazzling, Queenly Majesty and Armor Tail do not block a move's Z-Status effect |  |  |
| 41315 | FAIL | Frisk triggers in a Single Battle |  |  |
| 41316 | FAIL | Protosynthesis boosts the highest stat |  |  |
| 41317 | FAIL | Flower Gift transforms Cherrim back to normal when weather changes |  |  |
| 41318 | FAIL | Costar's message displays correctly after all battlers fainted - Player |  |  |
| 41319 | FAIL | Flower Gift transforms Cherrim back to normal under Cloud Nine/Air Lock 1/2 |  |  |
| 41320 | FAIL | Quark Drive prioritizes stats in the case of a tie in the following order: Atk, Def, Sp.Atk, Sp.Def, Speed 1/4 |  |  |
| 41321 | FAIL | Gale Wings only grants priority to Flying-type moves 1/2 |  |  |
| 41323 | FAIL | Sleep Clause: Waking up after Rest doesn't deactivate sleep clause (Doubles) |  |  |
| 41327 | FAIL | Neutralizing Gas is active until the last Dragon Darts hit even if Neutralizing Gas is no longer on the field |  |  |
| 41332 | FAIL | Forecast transforms Castform back to normal when its ability is suppressed |  |  |
| 41333 | FAIL | Costar's message displays correctly after all battlers fainted - Opponent |  |  |
| 41334 | FAIL | Dauntless Shield activates when it's no longer effected by Neutralizing Gas |  |  |
| 41335 | FAIL | Fling - Item does not get blocked by Unnerve if it isn't a berry |  |  |
| 41338 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon faints (Doubles) |  |  |
| 41340 | FAIL | Forecast transforms all Castforms present in weather 1/4 |  |  |
| 41341 | FAIL | Cute Charm inflicts infatuation on contact 1/2 |  |  |
| 41345 | FAIL | Focus Punch activates when Focus Band/Focus Sash blocks OHKO move 1/3 |  |  |
| 41346 | FAIL | Forecast transforms Castform in primal weather 1/2 |  |  |
| 41351 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up by Natural Cure |  |  |
| 41352 | FAIL | Normalize doesn't affect Judgment / Techno Blast / Multi-Attack's type 1/3 |  |  |
| 41353 | FAIL | Schooling switches Level 20+ Wishiwashi's form when HP is healed above 25-percent 1/2 |  |  |
| 41356 | FAIL | Overcoat blocks Effect Spore's effect (Gen6+) 1/2 |  |  |
| 41359 | FAIL | Defiant activates after Sticky Web lowers Speed |  |  |
| 41360 | FAIL | Healer cures status condition before burn or poison damage is dealt 1/4 (2/2) |  |  |
| 41362 | FAIL | Sleep Clause: Dark Void can only sleep one opposing mon if sleep clause is active |  |  |
| 41365 | FAIL | Parental Bond does not convert multi-target moves into a two-strike move in Double Battles, even if it only damages one |  |  |
| 41369 | FAIL | Future Sight is not boosted by Life Orb is original user if not on the field |  |  |
| 41371 | FAIL | Sleep Clause: Psycho Shift'ing sleep will fail if sleep clause is active |  |  |
| 41374 | FAIL | Dazzling, Queenly Majesty and Armor Tail do not block high-priority moves called by other moves 1/3 |  |  |
| 41377 | FAIL | Lightning Rod absorbs Electric-type moves and increases the Sp. Attack 2/2 |  |  |
| 41378 | FAIL | Hospitality user restores 25% of ally's health on switch-in |  |  |
| 41379 | FAIL | If Glaive Rush is successful moves targeted at the user do not check accuracy (1/?) |  |  |
| 41380 | FAIL | Sleep Clause: Sleep moves fail when sleep clause is active (Doubles) |  |  |
| 41383 | FAIL | Flower Gift increases the attack of Cherrim and its allies by 1.5x 2/2 |  |  |
| 41387 | FAIL | Hydration cures non-volatile Status conditions if it is raining |  |  |
| 41389 | FAIL | Magic Bounce cannot bounce back powder moves against Grass Types |  |  |
| 41392 | FAIL | Spread Moves: AOE ground type move vs Levitate and Air Balloon |  |  |
| 41394 | FAIL | Forecast transforms Castform back when it switches out |  |  |
| 41395 | FAIL | Poison Touch only applies when using contact moves 1/2 |  |  |
| 41396 | FAIL | Delta Stream doesn't activate if there's already strong winds |  |  |
| 41397 | FAIL | Dry Skin causes 1/8th Max HP damage in Sun |  |  |
| 41399 | FAIL | Healing Wish effect activates even if the the switched Pokémon can't be healed (Gen4-7) |  |  |
| 41404 | FAIL | Forecast transforms Castform back to normal when weather expires |  |  |
| 41405 | FAIL | Ice Face is not restored if hail or snow and Eiscue are already out 1/2 |  |  |
| 41407 | FAIL | Mimicry can trigger multiple times in a turn |  |  |
| 41408 | FAIL | Download raises Attack if player has lower Def than Sp. Def 2/2 |  |  |
| 41409 | FAIL | Frisk triggers for opponent in a Double Battle after switching-in after fainting 1/2 |  |  |
| 41410 | FAIL | Electromorphosis triggers on each multistrike hit but Charge does not stack |  |  |
| 41411 | FAIL | Switch-in abilities trigger in Speed Order after post-KO switch - Single Battle 1/2 |  |  |
| 41414 | FAIL | Hospitality does not trigger if there is no ally on the field |  |  |
| 41416 | FAIL | Earth Eater heals 25% when hit by ground type moves |  |  |
| 41417 | FAIL | Mirror Armor raises the stat of an attacking Pokemon with Contrary |  |  |
| 41425 | FAIL | Strong winds continue as long as there's a Pokémon with Delta Stream on the field |  |  |
| 41427 | FAIL | Ice Body recovers 1/16th of Max HP in hail. 1/2 |  |  |
| 41429 | FAIL | Disguised Mimikyu takes damage from Rocky Helmet without breaking the disguise 1/2 |  |  |
| 41431 | FAIL | Flame Body inflicts burn on contact 1/2 |  |  |
| 41433 | FAIL | Mummy/Lingering Aroma replace the attacker's ability on contact 1/4 |  |  |
| 41434 | FAIL | Gulp Missile: Transformed Cramorant deal 1/4 of damage opposing mon if hit by a damaging move, Gulping also lowers defense |  |  |
| 41436 | FAIL | Flower Gift transforms Cherrim back when it switches out |  |  |
| 41443 | FAIL | Flower Veil prevents status on allied Grass-types - right target 1/5 |  |  |
| 41452 | FAIL | Intimidate (opponent) lowers player's attack after KO 1/2 |  |  |
| 41453 | FAIL | Hyper Cutter prevents intimidate |  |  |
| 41455 | FAIL | Earth Eater activates on status moves |  |  |
| 41458 | FAIL | Forecast transforms Castform in weather from an opponent's move 1/4 |  |  |
| 41464 | FAIL | Embody Aspect raises a stat depending on the users form by one stage 1/4 |  |  |
| 41466 | FAIL | Grim Neigh does not increase damage done by the same move that causes another Pokemon to faint 1/2 |  |  |
| 41468 | FAIL | Anger Point does not trigger when already at maximum Attack stage |  |  |
| 41473 | FAIL | Knock Off does knock off a Booster Energy from a non Paradox Pokemon |  |  |
| 41474 | FAIL | Forecast transforms Castform when Cloud Nine ability user leaves the field 1/2 |  |  |
| 41477 | FAIL | Knock Off does knock off other form-change hold items from Pokemon that don't actually use them |  |  |
| 41479 | FAIL | Intimidate and Eject Button don't force the opponent to Attack |  |  |
| 41482 | FAIL | Forewarn warns about the highest power move among all opposing battlers |  |  |
| 41489 | FAIL | Parental Bond does not trigger on OHKO moves |  |  |
| 41490 | INVALID | Ball Fetch doesn't trigger in Trainer Battles |  |  |
| 41493 | FAIL | Intrepid Sword raises Attack by one stage every time it switches in (Gen8) |  |  |
| 41495 | FAIL | Hyper Cutter doesn't prevent Attack stage reduction from moves used by the user |  |  |
| 41497 | FAIL | Hyper Cutter doesn't prevent receiving negative Attack stage changes from Baton Pass |  |  |
| 41498 | FAIL | Gale Wings only grants priority at full HP (Gen 7+) 1/4 |  |  |
| 41500 | FAIL | Clear Body, Full Metal Body, and White Smoke don't prevent Spectral Thief from resetting positive stat changes 1/3 |  |  |
| 41502 | FAIL | Pastel Veil prevents Toxic Spikes poison |  |  |
| 41503 | FAIL | Harvest can only restore the newest berry consumed that was transferred from another Pokémon instead of its original Berry |  |  |
| 41504 | FAIL | Liquid Ooze causes Absorb users to lose HP instead of heal |  |  |
| 41506 | FAIL | Innards Out does not damage Magic Guard Pokemon |  |  |
| 41507 | FAIL | Pickpocket steals Shell Bell after it heals the user |  |  |
| 41508 | FAIL | Ice Face is restored if Noice Face Eiscue is sent in while hail or snow is active 1/2 |  |  |
| 41509 | FAIL | Magic Bounce bounces back status moves |  |  |
| 41514 | FAIL | Healer cures adjacent ally's status condition 30% of the time 1/6 (2/2) |  |  |
| 41517 | FAIL | Intimidate can not further lower opponents Atk stat if it is at minimum stages |  |  |
| 41518 | FAIL | Commander doesn't prevent Transform from working on a Commander Tatsugiri |  |  |
| 41520 | FAIL | Mirror Armor lowers the Attack of Pokemon with Intimidate |  |  |
| 41521 | FAIL | Liquid Ooze will faint Matcha Gatcha users if it deals enough damage |  |  |
| 41522 | FAIL | Intrepid Sword and Dauntless Shield both can be Skill Swapped and active their effects on the Skill Swap user |  |  |
| 41524 | FAIL | Steel Beam causes the user & the target to faint when below 1/2 of its Max HP |  |  |
| 41525 | FAIL | Magic Bounce bounces back powder moves |  |  |
| 41526 | FAIL | Pixilate doesn't override Electrify |  |  |
| 41530 | FAIL | Moxie/Chilling Neigh raises Attack by one stage after directly causing a Pokemon to faint 1/3 |  |  |
| 41534 | FAIL | Prankster is blocked by Quick Guard in Gen5+ |  |  |
| 41535 | FAIL | Moxie/Chilling Neigh does not trigger when already at maximum Attack stage 1/3 |  |  |
| 41536 | FAIL | Neutralizing Gas leaving the field allows abilities to activate in turn order 1/6 |  |  |
| 41542 | FAIL | Own Tempo prevents Intimidate but no other stat down changes (Gen8+) |  |  |
| 41545 | FAIL | Mirror Armor doesn't lower the stats of an attacking Pokemon behind Substitute |  |  |
| 41548 | FAIL | Parental Bond-converted moves only hit once on Lightning Rod/Storm Drain mons 1/2 |  |  |
| 41556 | FAIL | Pickpocket activates after the final hit of a multi-strike move |  |  |
| 41558 | FAIL | Own Tempo cures confusion if it's obtained via Skill Swap |  |  |
| 41563 | FAIL | Oblivious doesn't prevent Intimidate (Gen3-7) |  |  |
| 41573 | FAIL | Pastel Veil prevents Toxic bad poison on partner - left target |  |  |
| 41575 | FAIL | Neutralizing Gas doesn't reactivate Beads of Ruin after Chi-Yu faints |  |  |
| 41576 | FAIL | Pickpocket steals Life Orb after it activates |  |  |
| 41578 | FAIL | Refrigerate doesn't affect Hidden Power's type |  |  |
| 41579 | FAIL | Poison Touch applies between multi-hit move hits |  |  |
| 41581 | FAIL | Parting Shot: Flower Veil prevents stat drops and switches (Gen6) |  |  |
| 41584 | FAIL | Prankster-affected moves can still be bounced back by a Dark-type with Magic Bounce |  |  |
| 41586 | FAIL | Parting Shot: Hyper Cutter blocks Attack drop but still switches |  |  |
| 41589 | FAIL | Disguised Mimikyu takes no damage from a confusion hit and changes to its busted form 1/2 |  |  |
| 41593 | FAIL | Dry Skin is only triggered once on multi strike moves |  |  |
| 41594 | FAIL | Normalize doesn't affect Terrain Pulse's type |  |  |
| 41595 | FAIL | Prankster-affected moves called via Instruct do not affect Dark-type Pokémon 1/2 |  |  |
| 41596 | FAIL | Poison Puppeteer confuses target if it was (badly) poisoned by a status move 1/2 |  |  |
| 41600 | FAIL | Moves called via Prankster-affected After you affect Dark-type Pokémon |  |  |
| 41604 | FAIL | Purifying Salt user can't be poisoned by Toxic Spikes |  |  |
| 41605 | FAIL | Primordial Sea does not block a move if Pokémon is asleep and uses a Fire-type move |  |  |
| 41607 | FAIL | Quick Draw does not activate 70% of the time (2/2) |  |  |
| 41612 | FAIL | Parental Bond Smack Down effect triggers after 2nd hit |  |  |
| 41613 | FAIL | Sand Spit triggers even if the user is knocked out by the hit |  |  |
| 41615 | FAIL | Sap Sipper blocks multi-hit grass type moves |  |  |
| 41617 | FAIL | Pastel Veil immediately cures Mold Breaker poison |  |  |
| 41618 | FAIL | Parental Bond has no affect on multi hit moves and they still hit thrice 37.5/35% of the time 1/2 (6/6) |  |  |
| 41620 | FAIL | Pickpocket activates after Magician steals an item |  |  |
| 41621 | FAIL | Powder doesn't prevent a Fire move from thawing its user out |  |  |
| 41625 | FAIL | Pickup restores an item that has been Flinged |  |  |
| 41627 | FAIL | Shield Dust does not block primary effects 1/4 |  |  |
| 41630 | FAIL | Pickup grants an item used by another Pokémon |  |  |
| 41632 | FAIL | Prankster-affected moves that are bounced back by Magic Bounce can affect Dark-type Pokémon |  |  |
| 41644 | FAIL | Poison Heal heals from (Toxic) Poison damage 1/2 |  |  |
| 41649 | FAIL | Stamina activates for every hit of a multi hit move |  |  |
| 41650 | FAIL | Protect: Quick Guard protects self and ally from priority moves 1/4 |  |  |
| 41651 | FAIL | Protean/Libero changes the type of the user only once per switch in (Gen9+) 1/2 |  |  |
| 41652 | FAIL | Prankster-affected moves which are reflected by Magic Coat can affect Dark-type Pokémon, unless the Pokémon that bounced the move also has Prankster 1/2 |  |  |
| 41655 | FAIL | Protosynthesis activates on switch-in |  |  |
| 41656 | FAIL | Psych Up copies the target's critical hit ratio (Gen6+) |  |  |
| 41659 | FAIL | Static triggers 1/3 times (Gen3) or 30% (Gen4+) of the time 1/2 (2/2) |  |  |
| 41669 | FAIL | Soundproof makes sound moves fail against the ability user |  |  |
| 41670 | FAIL | Psychic Terrain increases power of Psychic-type moves by 30/50 percent 1/2 |  |  |
| 41671 | FAIL | Quark Drive ability pop up activates only once during the duration of electric terrain |  |  |
| 41672 | FAIL | Sword of Ruin increases damage taken by special moves in Wonder Room 1/4 |  |  |
| 41675 | FAIL | Regenerator heals 1/3 of max HP upon switching out but doesn't surpass max HP 1/5 |  |  |
| 41676 | FAIL | Rattled does not boost speed by 1 when affected by Intimidate (Gen5-7) |  |  |
| 41678 | FAIL | Pursuit becomes a locked move after being used on switch-out while holding a Choice Item |  |  |
| 41686 | FAIL | Static inflicts paralysis on contact 1/2 |  |  |
| 41689 | FAIL | Pursuit user mega evolves before attacking a switching foe and hits twice if user has Parental Bond |  |  |
| 41691 | FAIL | Stench does not stack with King's Rock (2/2) |  |  |
| 41692 | FAIL | Toxic Chain inflicts bad poison when attacking (2/2) |  |  |
| 41693 | FAIL | Stamina activates correctly for every battler with the ability when hit by a multi target move 1/3 |  |  |
| 41694 | FAIL | Pursuit doubles in power if attacking while target switches out 1/2 |  |  |
| 41697 | FAIL | Stench doesn't trigger if partner uses a move |  |  |
| 41700 | FAIL | Storm Drain absorbs Water-type moves and increases the Sp. Attack (Gen5+) 2/2 |  |  |
| 41703 | FAIL | Supreme Overlord's message displays correctly after all battlers fainted - Player |  |  |
| 41707 | FAIL | Supreme Overlord's boost caps at a 1.5x multipler 1/2 |  |  |
| 41709 | FAIL | Symbiosis triggers after partner flings its item |  |  |
| 41712 | FAIL | Vessel of Ruin's message displays correctly after all battlers fainted - Opponent |  |  |
| 41714 | FAIL | Sword of Ruin's message displays correctly after all battlers fainted - Player |  |  |
| 41718 | FAIL | Speed Boost gradually boosts Speed |  |  |
| 41719 | FAIL | Reflect Type fails if the user is Terastallized |  |  |
| 41722 | FAIL | Symbiosis triggers after partners berry eaten from bug bite |  |  |
| 41723 | FAIL | Tera Shift can't be suppressed by Neutralizing Gas |  |  |
| 41726 | FAIL | Water Compaction raises Defense 2 stages on each hit of a multi-hit Water type move |  |  |
| 41731 | FAIL | Tablets of Ruin's message displays correctly after all battlers fainted - Player |  |  |
| 41733 | FAIL | Wind Power sets up Charge for player when hit by a wind move 2/2 |  |  |
| 41734 | FAIL | Tangling Hair drops opposing mon's speed if ability user got hit by a contact move 1/2 |  |  |
| 41739 | ASSUMPTION_FAIL | Roost prevents a Flying-type user from being protected by Delta Stream |  |  |
| 41740 | FAIL | Zero to Hero's message displays correctly after all battlers fainted - Opponent |  |  |
| 41743 | FAIL | Sleep Clause: Effect Spore causes sleep 11% (Gen5+) of the time with sleep clause active (100/100) |  |  |
| 41745 | FAIL | Rototiller fails if there are no valid targets |  |  |
| 41746 | FAIL | Zero to Hero can't be suppressed by Neutralizing Gas |  |  |
| 41747 | FAIL | Spread Moves: Spread move vs one protecting mon |  |  |
| 41753 | FAIL | Air Balloon is popped after Toxic Debris activates |  |  |
| 41754 | FAIL | Sturdy prevents OHKOs (Gen5+) 1/2 |  |  |
| 41761 | FAIL | Vessel of Ruin's message displays correctly after all battlers fainted - Player |  |  |
| 41769 | FAIL | Volt Absorb heals 25% when hit by electric type moves |  |  |
| 41777 | FAIL | Trainer Slide: Singles: Enemy Mon Unaffected |  |  |
| 41779 | KNOWN_FAILING | Sleep Clause: Sleep clause is deactivated when a sleeping mon is sent out and transforms into a mon with Insomnia / Vital spirit |  |  |
| 41781 | FAIL | Sky Drop does no damage to Flying type Pokémon |  |  |
| 41782 | ASSUMPTION_FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up with Aromatherapy / Heal Bell / Sparkly Swirl |  |  |
| 41783 | ASSUMPTION_FAIL | Hypnosis inflicts 1-3 turns of sleep |  |  |
| 41786 | FAIL | AI will choose Thunderbolt then Surf 2/3 times if the opposing mon has Volt Absorb (3/3) |  |  |
| 41793 | FAIL | CreateNPCTrainerPartyForTrainer generates customized Pokémon |  |  |
| 41796 | FAIL | Tailwind does not trigger Wind Rider on an absent ally battler |  |  |
| 41797 | FAIL | Snatch steals stat-boosting moves from the opponent |  |  |
| 41800 | FAIL | AI prefers moves which deal more damage instead of moves which are super-effective but deal less damage 1/2 |  |  |
| 41802 | FAIL | Tera Shell only makes the first hit against Terapagos from a multi-target move not very effective |  |  |
| 41804 | FAIL | Weak Armor doesn't interrupt multi hit moves if Speed can't go any higher |  |  |
| 41805 | FAIL | Wind Rider raises Attack by one stage if Tailwind is setup by its partner |  |  |
| 41806 | FAIL | Sleep Clause: Sleep from Effect Spore will not activate sleep clause (Doubles) (100/100) |  |  |
| 41807 | FAIL | Teraform Zero can be replaced |  |  |
| 41808 | FAIL | Trainer Slide: Singles: Player Lands First STAB Hit |  |  |
| 41815 | FAIL | Wind Power displays its message before fainting when triggered |  |  |
| 41816 | FAIL | Spread Moves: Not very effective message on both player mons |  |  |
| 41817 | FAIL | Aerilate doesn't affect Hidden Power's type |  |  |
| 41822 | KNOWN_FAILING | Anticipation treats Hidden Power as its dynamic type (Gen6+) |  |  |
| 41826 | FAIL | Spread Moves: If a spread move attack will activate a resist berries on one Pokémon, only the damage for that mon will be reduced |  |  |
| 41827 | FAIL | Wind Rider activates when it's no longer effected by Neutralizing Gas |  |  |
| 41829 | FAIL | Zero to Hero's message displays correctly after all battlers fainted - Player |  |  |
| 41831 | FAIL | Supersweet Syrup can not further lower opponents evasion if it is at minimum stages |  |  |
| 41832 | FAIL | Sturdy prevents OHKO moves |  |  |
| 41834 | FAIL | Sticky Web has correct interactions with Mirror Armor - the battler which set up Sticky Web has its Speed lowered instead 1/4 |  |  |
| 41838 | FAIL | Sleep Clause: Yawn'd Pokémon slept due to Effect Spore before Yawn triggers does not activate sleep clause (100/100) |  |  |
| 41840 | FAIL | Wonder Guard does not activate when targeted by moves super effective against it 1/5 |  |  |
| 41841 | FAIL | Supersweet Syrup lowers evasion of both opposing mon's in battle |  |  |
| 41842 | FAIL | AI will only use Dream Eater if target is asleep 2/2 |  |  |
| 41843 | FAIL | Stuff Cheeks raises Defense by 2 stages after consuming the berry and gaining its effect |  |  |
| 41844 | FAIL | Zero to Hero transforms Palafin when it switches out |  |  |
| 41850 | FAIL | Battle Bond increases Atk, SpAtk and Speed by 1 stage (Gen9+) |  |  |
| 41851 | FAIL | Battle Bond transforms opponent's Greninja - Singles 1/4 |  |  |
| 41857 | FAIL | Sleep Clause: Sleep clause is deactivated when a sleeping mon is woken up forcefully by Uproar |  |  |
| 41860 | FAIL | Supreme Overlord boosts Attack by an additive 10% per fainted mon on its side upon switch in 2/2 |  |  |
| 41862 | FAIL | Sword of Ruin's Defense reduction is ignored by Gastro Acid 1/2 |  |  |
| 41863 | FAIL | Teatime causes other Pokemon to consume their Berry even if the user doesn't have a Berry as its held item |  |  |
| 41866 | FAIL | Clear Body, Full Metal Body, and White Smoke prevent stat stage reduction from moves 1/21 |  |  |
| 41867 | FAIL | Trace copies opponents ability |  |  |
| 41868 | FAIL | Symbiosis transfers its item after Gem consumption, but before move execution (Gen6) |  |  |
| 41870 | FAIL | Telekinesis makes the target unable to avoid any attacks made against it |  |  |
| 41873 | FAIL | Trainer Slide: Singles: Mega Evolution |  |  |
| 41885 | FAIL | Stellar-type Tera Blast lowers both offensive stats |  |  |
| 41886 | FAIL | Sleep Clause: Sleep caused by Effect Spore does not prevent sleep clause from ever activating (100/100) |  |  |
| 41888 | FAIL | Teraform Zero cannot be copied |  |  |
| 41890 | FAIL | Tera Shell only makes the first hit of a double battle turn not very effective |  |  |
| 41891 | FAIL | Aftermath damages the attacker by 1/4th of its max HP if fainted by a contact move |  |  |
| 41895 | FAIL | Terrain Boost: Expanding Force's power increases by 50% if the user is affected by Psychic Terrain 1/2 |  |  |
| 41897 | FAIL | Sleep Clause: Moves with sleep effect chance will activate sleep clause (2/2) |  |  |
| 41900 | FAIL | Toxic Chain makes Lum/Pecha Berry trigger before being knocked off 1/2 |  |  |
| 41905 | FAIL | Spread Moves: Doesn't affect message on both opposing mons |  |  |
| 41913 | FAIL | Trace copies opponents ability randomly 1/2 (2/2) |  |  |
| 41914 | FAIL | Arena Trap doesn't prevent switch outs via moves that switch out 1/8 |  |  |
| 41916 | FAIL | Transform fails when the user is already transformed in Gen5+ 1/2 |  |  |
| 41920 | FAIL | Toxic Chain inflicts bad poison on any hit of a multi-hit move |  |  |
| 41922 | FAIL | Water Absorb is only triggered once on multi strike moves |  |  |
| 41925 | FAIL | Flower Gift transforms Cherrim back when it uses a move that forces it to switch out |  |  |
| 41926 | FAIL | Volt Absorb does not stop Electric Typed Explosion from damaging other Pokémon |  |  |
| 41927 | FAIL | Color Change changes the type when a Pokemon is hit by Doom Desire |  |  |
| 41928 | FAIL | Trick Room doesn't print its ending message twice when used again |  |  |
| 41932 | FAIL | Color Change does not change the type to Normal when a Pokemon is hit by Struggle |  |  |
| 41933 | FAIL | Weak Armor doesn't interrupt multi hit moves if Defense can't go any lower |  |  |
| 41934 | FAIL | Flower Veil prevents status on allied Grass-types - left target 1/5 |  |  |
| 41944 | FAIL | Wind Power activates correctly for every battler with the ability when hit by a 3 target move 1/3 |  |  |
| 41947 | FAIL | Weak Armor still boosts Speed if Defense can't go any lower 1/2 |  |  |
| 41950 | FAIL | Commander Attacker is kept (Dondozo Left Slot) |  |  |
| 41954 | FAIL | Transform doesn't apply the heroic transformation message when copying Palafin |  |  |
| 41955 | FAIL | Commander Tatsugiri does not get hit by Dragon Darts when a commanded Dondozo faints |  |  |
| 41956 | FAIL | AI will try to do damage on target instead of setting up hazards if target has a way to remove them |  |  |
| 41957 | FAIL | Commander Tatsugiri will still take poison damage if while inside Dondozo |  |  |
| 41963 | FAIL | Harvest can restore a Berry that was transferred from another Pokémon |  |  |
| 41968 | FAIL | AI_FLAG_SMART_TERA: AI will tera if it enables a ko |  |  |
| 41969 | FAIL | Trainer Slide: Multi: Player Lands First STAB Hit |  |  |
| 41970 | FAIL | Vessel of Ruin is still active if removed by Mold Breaker + Entrainment |  |  |
| 41976 | FAIL | Sparkly Swirl cures the entire party of the user from primary status effects 1/7 |  |  |
| 41978 | FAIL | Zen Mode switches Darmanitan's form when HP is healed above half 1/2 |  |  |
| 41979 | FAIL | Volt Absorb is only triggered once on multi strike moves |  |  |
| 41980 | FAIL | Commander prevents Whirlwind from working against Dondozo or Tatsugiri while it's active |  |  |
| 41981 | FAIL | Cotton Down drops speed by one for each multi hit |  |  |
| 41982 | FAIL | Hyper Cutter doesn't prevent Topsy-Turvy |  |  |
| 41983 | FAIL | AI will not use a status move if partner already chose Helping Hand 1/269 |  |  |
| 41985 | FAIL | Trainer Slide: Singles: Z Move |  |  |
| 41988 | FAIL | Water Absorb heals 25% when hit by water type moves |  |  |
| 41996 | FAIL | Competitive sharply raises opponent's Sp. Atk after Intimidate 1/4 |  |  |
| 41997 | FAIL | Cute Charm triggers 1/3 times (Gen3) or 30% (Gen 4+) of the time 1/2 (2/2) |  |  |
| 41998 | FAIL | Weak Armor does not trigger when brought in by Dragon Tail and taking Stealth Rock damage |  |  |
| 42002 | FAIL | Aftermath ability pop-up will be displayed correctly: player point of view |  |  |
| 42006 | FAIL | Defiant is activated by Cotton Down for non-ally Pokémon |  |  |
| 42008 | FAIL | Cotton Down correctly gets blocked by stat reduction preventing abilities |  |  |
| 42013 | FAIL | Wind Power sets up Charge for only one attack when hit by a wind move 2/2 |  |  |
| 42016 | FAIL | Arena Trap doesn't prevent switch outs via Shed Shell |  |  |
| 42017 | FAIL | Defiant sharply raises opponent's Attack after Intimidate 1/4 |  |  |
| 42021 | FAIL | Keen Eye, Gen9+ Illuminate & Minds Eye don't prevent Topsy-Turvy 1/3 |  |  |
| 42025 | FAIL | Bad Dreams activates for both sleeping Pokémon on the player side |  |  |
| 42026 | FAIL | Zen Mode switches Darmanitan's form to Standard when swapped out 1/2 |  |  |
| 42027 | FAIL | Limber prevents paralysis from Thunder Wave |  |  |
| 42028 | FAIL | Early Bird turns a 3-turn sleep into one missed turn |  |  |
| 42031 | FAIL | Liquid Ooze causes leech seed victim to faint before seeder 1/2 |  |  |
| 42032 | FAIL | Dancer still activates after Red Card |  |  |
| 42035 | FAIL | AI will choose either Rock Tomb or Bulldoze if Stat drop effect will activate and they kill with the same number of hits |  |  |
| 42041 | FAIL | Embody Aspect does not reactivate after Neutralizing Gas ends if it already activated this switch-in |  |  |
| 42045 | FAIL | Beads of Ruin reduces Sp. Def if opposing mon's ability doesn't match |  |  |
| 42047 | FAIL | AI prefers Water Gun over Bubble if it knows that foe has Contrary 1/2 |  |  |
| 42055 | FAIL | Pursuit doesn't attack a foe using Teleport / Baton Pass to switch out 1/2 |  |  |
| 42058 | FAIL | AI prefers Earthquake over Drill Run if both require the same number of hits to ko |  |  |
| 42060 | FAIL | Big Pecks doesn't prevent Topsy-Turvy |  |  |
| 42061 | FAIL | Flower Gift transforms Cherrim in harsh sunlight |  |  |
| 42075 | FAIL | Explosion causes the user & the target to faint |  |  |
| 42077 | FAIL | Mold Breaker, Teravolt, and Turboblaze ignore Clear Body and White Smoke, but not Full Metal Body 2/63 |  |  |
| 42079 | FAIL | Forecast transforms Castform on switch-in |  |  |
| 42080 | FAIL | AI sees increased base power of Wake Up Slap 2/2 |  |  |
| 42081 | FAIL | Defiant doesn't activate after Sticky Web lowers Speed if Court Changed (Gen8) |  |  |
| 42086 | FAIL | AI partner will not switch mid-turn into a player Pokémon (multi) 1/2 (1/?) |  |  |
| 42089 | FAIL | Starting Sticky Web lowers Speed on entry |  |  |
| 42090 | FAIL | Disguised Mimikyu's types revert back to Ghost/Fairy when Disguise is broken 1/2 |  |  |
| 42092 | FAIL | Forewarn randomly chooses between same-power moves on one opponent (3/3) |  |  |
| 42101 | KNOWN_FAILING | AI uses Helping Hand if the ally does notably more damage |  |  |
| 42104 | FAIL | Reflect Type copies a target's pure type |  |  |
| 42105 | FAIL | Sleep prevents the battler from using a move 1/3 |  |  |
| 42119 | FAIL | Mold Breaker does not prevent Own Tempo from curing confusion right after |  |  |
| 42127 | FAIL | Overcoat blocks damage from sandstorm |  |  |
| 42130 | FAIL | AI sees increased base power of Spit Up |  |  |
| 42137 | FAIL | Pastel Veil prevents Toxic bad poison on partner - right target |  |  |
| 42140 | FAIL | Parental Bond does not convert a move with three or more strikes to a two-strike move |  |  |
| 42141 | FAIL | Hyper Cutter doesn't prevent Spectral Thief from resetting positive Attack stage changes |  |  |
| 42143 | FAIL | Pickpocket activates after Knock Off, Bug Bite, or Pluck 1/3 |  |  |
| 42144 | FAIL | AI sees increased base power of Grav Apple 2/2 |  |  |
| 42147 | FAIL | Ice Face blocks physical moves, changing Eiscue into its Noice Face form |  |  |
| 42150 | FAIL | Roar fails to switch out target with Suction Cups |  |  |
| 42151 | FAIL | Pixilate doesn't affect Hidden Power's type |  |  |
| 42153 | FAIL | Pickpocket does not activate if the user faints |  |  |
| 42154 | FAIL | Poison Puppeteer confuses target if it was poisoned by a damaging move |  |  |
| 42157 | ASSUMPTION_FAIL | Roost's suppression does not prevent others who are Transforming into the user from copying its Flying-type |  |  |
| 42158 | ASSUMPTION_FAIL | Roost suppresses the user's not-yet-aquired Flying-type this turn |  |  |
| 42161 | FAIL | Innards Out uses correct damage amount for Future Sight |  |  |
| 42162 | FAIL | Flame Body triggers 1/3 times (Gen3) or 30% (Gen 4+) of the time 1/2 (2/2) |  |  |
| 42167 | FAIL | Primordial Sea blocks damaging Fire-type moves and prints the message only once with moves hitting multiple targets |  |  |
| 42168 | FAIL | Gale Wings doesn't increase priority of Flying-type Natural Gift, Judgment, Hidden Power, or Tera Blast 1/3 |  |  |
| 42170 | FAIL | Power Herb semi-invulnerable moves do not keep the user untargetable that turn |  |  |
| 42178 | INVALID | Power Construct Zygarde reverts to its original form upon catching |  |  |
| 42181 | FAIL | Intimidate is not going to trigger if a mon switches out through u-turn and the opposing field is empty |  |  |
| 42182 | FAIL | Gulp Missile: Transformed Cramorant Gulping lowers defense and still triggers other effects after 1/2 |  |  |
| 42183 | FAIL | Shed Tail creates a Substitute at the cost of 1/2 users maximum HP and switches the user out |  |  |
| 42188 | FAIL | Quark Drive boosts the highest stat |  |  |
| 42190 | FAIL | Shell Trap does not trigger when hit into Substitute |  |  |
| 42191 | FAIL | Keen Eye, Gen9+ Illuminate & Minds Eye don't prevent receiving negative Attack stage changes from Baton Pass 1/3 |  |  |
| 42194 | FAIL | Shell Trap activates only if hit by a physical move 1/3 |  |  |
| 42195 | FAIL | Ice Face is not restored if Eiscue changes into Noice Face form while there's already hail or snow 1/2 |  |  |
| 42198 | FAIL | Refrigerate doesn't affect Judgment / Techno Blast / Multi-Attack's type 1/3 |  |  |
| 42199 | FAIL | Lightning Rod forces single-target Electric-type moves to target the Pokémon with this Ability. 1/2 |  |  |
| 42202 | FAIL | Magic Bounce bounces back moves hitting both foes at two foes |  |  |
| 42211 | FAIL | TIE_BREAK_SCORE with SCORE_TIE_RANDOM randomizes AI move selection (Doubles) (4/4) |  |  |
| 42215 | FAIL | Mimicry triggers after Skill Swap |  |  |
| 42220 | FAIL | Mirror Armor doesn't lower the stat of the attacking Pokemon if it is already at -6 |  |  |
| 42221 | FAIL | Rattled triggers correctly when hit by U-Turn |  |  |
| 42225 | FAIL | Sleep Clause: Waking up after Effect Spore doesn't deactivate sleep clause (100/100) |  |  |
| 42226 | FAIL | Refrigerate doesn't affect Terrain Pulse's type |  |  |
| 42243 | FAIL | Sticky Web raises Speed by 1 for a Pokemon with Contrary |  |  |
| 42249 | FAIL | Spit Up and Swallow don't work if used without Stockpile 1/2 |  |  |
| 42251 | FAIL | AI prefers a weaker move over a one with a downside effect if both require the same number of hits to ko 1/2 |  |  |
| 42254 | FAIL | Soul Heart boosts Sp. Atk after opponent uses Memento |  |  |
| 42255 | FAIL | Strength Sap restores more HP if Big Root is held 1/2 |  |  |
| 42269 | FAIL | Teatime causes the user to consume its Berry, ignoring HP requirements, when not used by the Player |  |  |
| 42270 | FAIL | AI_FLAG_SEQUENCE_SWITCHING: Roar and Dragon Tail still force switch to random party member 1/2 (2/2) |  |  |
| 42282 | FAIL | Pastel Veil prevents Toxic Spikes poison on partner |  |  |
| 42283 | FAIL | Teleport forces the Pokémon to switch out in Trainer Battles (Gen 8+) |  |  |
| 42286 | FAIL | Pickpocket does not prevent King's Rock or Razor Fang flinches |  |  |
| 42287 | ASSUMPTION_FAIL | Pickpocket checks contact/effect per target for spread moves |  |  |
| 42290 | FAIL | Mirror Armor lowers a stat of the attacking Pokémon 1/6 |  |  |
| 42292 | FAIL | Sword of Ruin's message displays correctly after all battlers fainted - Opponent |  |  |
| 42296 | FAIL | Symbiosis triggers after partner bestows its item |  |  |
| 42299 | FAIL | Poison Point inflicts poison on contact 1/2 |  |  |
| 42300 | FAIL | Tablets of Ruin's message displays correctly after all battlers fainted - Opponent |  |  |
| 42301 | FAIL | Sleep Clause: Waking up after Effect Spore doesn't deactivate sleep clause (Doubles) (100/100) |  |  |
| 42307 | FAIL | Tangling Hair does not cause Rocky Helmet miss activation |  |  |
| 42308 | FAIL | Tera Shift transforms Terapagos into its Terastal form on switch in |  |  |
| 42309 | FAIL | AI partner will not switch into a player Pokémon after fainting (2v1) 1/2 (1/?) |  |  |
| 42313 | FAIL | AI can choose Counter or Mirror Coat if the predicted move split is correct and user doesn't faint 1/2 |  |  |
| 42314 | FAIL | AI prefers priority moves if it's slower and can kill target |  |  |
| 42319 | FAIL | Tera Blast has correct effectiveness for every Tera Type 9/18 |  |  |
| 42321 | FAIL | Spread Moves: Unless move hits every target user will not include partner in the target count |  |  |
| 42325 | FAIL | Parental Bond converts Scratch into a two-strike move |  |  |
| 42331 | FAIL | Recharge moves make the user unable to attack for exactly one turn |  |  |
| 42333 | FAIL | Pickpocket steals the attacker's item unless it already has one 1/2 |  |  |
| 42334 | FAIL | Refrigerate doesn't override Electrify |  |  |
| 42338 | FAIL | Salt Cure does not get applied if hitting a Substitute |  |  |
| 42341 | FAIL | Pixilate doesn't affect Terrain Pulse's type |  |  |
| 42350 | FAIL | Poison Heal heals from Toxic Poison damage are constant |  |  |
| 42353 | FAIL | AI_FLAG_SMART_MON_CHOICES: AI will not switch in a Pokemon which is slower and gets 1HKOed after fainting 1/3 |  |  |
| 42355 | FAIL | Vessel of Ruin reduces Sp. Atk if opposing mon's ability doesn't match |  |  |
| 42356 | FAIL | Axe Kick still deals crash damage when boosted by Sheer Force 1/2 |  |  |
| 42357 | FAIL | AI recognizes Volt Absorb received from Trace |  |  |
| 42359 | FAIL | Trainer Slide: Multi: Enemy Mon Unaffected |  |  |
| 42362 | FAIL | Mind Blown is blocked by Damp |  |  |
| 42367 | FAIL | AI switches if Perish Song is about to kill (1/?) |  |  |
| 42369 | FAIL | Relic Song is blocked by Throat Chop |  |  |
| 42377 | FAIL | TIE_BREAK_SCORE with SCORE_TIE_CHOSEN can control AI move selection when scores are tied (Doubles) 1/4 |  |  |
| 42383 | FAIL | Tailwind does not trigger Wind Power on an absent ally battler |  |  |
| 42385 | FAIL | Wind Rider absorbs Wind moves and raises Attack by one stage |  |  |
| 42386 | FAIL | Protosynthesis accounts for Sticky Web when determining the boosted stat |  |  |
| 42394 | FAIL | Explosion causes the user to faint |  |  |
| 42395 | FAIL | AI will not try to switch for the same Pokémon for 2 spots in a double battle (all bad moves) 1/2 (1/?) |  |  |
| 42396 | FAIL | Wind Rider raises Attack by one stage if it sets up Tailwind |  |  |
| 42399 | FAIL | Relic Song transforms Meloetta after Magician was activated |  |  |
| 42400 | FAIL | Quark Drive activates on switch-in |  |  |
| 42402 | FAIL | Shield Dust does not block self-targeting effects, primary or secondary 4/4 |  |  |
| 42404 | FAIL | Rattled boosts speed by 1 when affected by Intimidate (Gen8+) |  |  |
| 42405 | FAIL | Relic Song transforms Meloetta twice if used successfully |  |  |
| 42406 | FAIL | Starting Toxic Spikes badly poison the opposing switch-in |  |  |
| 42409 | FAIL | Refrigerate doesn't change Tera Blast's type when Terastallized |  |  |
| 42410 | FAIL | Freeze is thawed by user's Flame Wheel |  |  |
| 42412 | FAIL | Imposter doesn't apply the heroic transformation message when copying Palafin |  |  |
| 42414 | FAIL | Freeze is thawed by opponent's Fire-type attacks even if Sheer Force affected (Gen 3+) |  |  |
| 42419 | FAIL | Morpeko Hangry reverts to Full Belly Form upon battle end after changing forms at the end of the turn |  |  |
| 42421 | FAIL | Aura Break inverts Fairy Aura's effect |  |  |
| 42422 | INVALID | Ball Fetch doesn't trigger if the Pokémon is already holding an item |  |  |
| 42425 | FAIL | Stamina is not activated by users own Substitute |  |  |
| 42447 | FAIL | AI partner will not switch into a player Pokémon (2v1) 1/2 (1/?) |  |  |
| 42451 | FAIL | Shaymin retains Land form if it was frozen or frostbitten in battle |  |  |
| 42453 | FAIL | AI will choose a priority move if it is slower then the target and will be killed |  |  |
| 42455 | FAIL | TIE_BREAK_SCORE correctly controls AI move selection when scores are tied for all values in enum ScoreTieResolution (Doubles) 1/4 |  |  |
| 42458 | FAIL | Storm Drain forces single-target Water-type moves to target the Pokémon with this Ability 2/2 |  |  |
| 42459 | FAIL | Relic Song transforms once Meloetta in a double battle |  |  |
| 42462 | FAIL | Aegislash reverts to Shield Form upon fainting (start as Blade) |  |  |
| 42463 | FAIL | Supreme Overlord's message displays correctly after all battlers fainted - Opponent |  |  |
| 42464 | FAIL | Relic Song transforms Meloetta if used successfully |  |  |
| 42470 | FAIL | Sword of Ruin doesn't activate when dragged out by Mold Breaker attacker 2/2 |  |  |
| 42471 | FAIL | Commander Tatsugiri does not get hit by Dragon Darts when commanding Dondozo 1/2 |  |  |
| 42487 | FAIL | Rayquaza can Mega Evolve knowing Dragon Ascent |  |  |
| 42488 | FAIL | AI uses a guaranteed KO move instead of the move with the highest expected damage 2/2 |  |  |
| 42489 | FAIL | Tangling Hair Speed stat drop triggers defiant and keeps original attacker/target |  |  |
| 42490 | FAIL | Sleep Clause: Effect Spore causes sleep 11% (Gen5+) of the time with sleep clause active (Doubles) (100/100) |  |  |
| 42492 | FAIL | Tera Shell makes all moves against Terapagos not very effective when at full HP 1/2 |  |  |
| 42493 | FAIL | Competitive is activated by Cotton Down for non-ally pokemon |  |  |
| 42494 | FAIL | Battle Bond Greninja returns to base form upon battle end after knocking out an opponent |  |  |
| 42496 | FAIL | Spread Moves: Super Effective Message on both opposing mons |  |  |
| 42497 | FAIL | Abilities replaced by Mega Evolution do not affect turn order |  |  |
| 42500 | FAIL | Venusaur can Mega Evolve holding Venusaurite |  |  |
| 42501 | FAIL | Spread Moves: Ability and Item effects activate correctly after a multi target move |  |  |
| 42502 | FAIL | Contrary lowers a stat after using a move which would normally raise it: Swords Dance 1/2 |  |  |
| 42505 | FAIL | Primal Reversion happens after the entry hazards damage |  |  |
| 42506 | FAIL | Forced abilities activate on switch-in |  |  |
| 42509 | FAIL | Primal Reversion's order is determined by Speed - player faster |  |  |
| 42511 | FAIL | Ultra Burst and Mega Evolution can happen on the same turn |  |  |
| 42515 | FAIL | Ultra Burst affects turn order |  |  |
| 42517 | FAIL | Trainer Slide: Singles: Last Switchin |  |  |
| 42523 | FAIL | Cursed Body disables the base move of a status Z-Move |  |  |
| 42526 | FAIL | Aerilate doesn't change Tera Blast's type when Terastallized |  |  |
| 42528 | FAIL | Primal Reversion happens after a switch-in caused by Eject Button |  |  |
| 42532 | FAIL | Shaymin-Sky reverts to Shaymin-Land when frozen or frostbitten 1/5 |  |  |
| 42533 | FAIL | Wind Power sets up Charge for opponent when hit by a wind move 2/2 |  |  |
| 42539 | FAIL | Zero to Hero transforms both player and opponent |  |  |
| 42543 | FAIL | Battle Bond increases a Stat even if only one can be increased (Gen9+) |  |  |
| 42547 | FAIL | Effect Spore causes poison 3.3% (Gen3), 10% (Gen4) and 9% (Gen5+) of the time 1/3 (300/300) |  |  |
| 42550 | FAIL | Galvanize doesn't affect Hidden Power's type |  |  |
| 42552 | FAIL | Beast Boost doesn't trigger if user is fainted |  |  |
| 42554 | FAIL | Dynamax: G-Max Centiferno traps both opponents in Fire Spin |  |  |
| 42555 | FAIL | Desolate Land blocks damaging Water-type moves and prints the message only once with moves hitting multiple targets |  |  |
| 42556 | FAIL | (Gulp Missile) Cramorant in Gorging damages an electric type without paralysing |  |  |
| 42557 | FAIL | Big Pecks doesn't prevent Defense stage reduction from moves used by the user |  |  |
| 42562 | FAIL | Berserk Gene sharply raises attack at the start of a double battle 2/2 |  |  |
| 42566 | FAIL | Hunger Switch switches Morpeko's forms at the end of the turn 1/2 |  |  |
| 42568 | FAIL | Download raises Sp.Attack if enemies have lower total Sp. Def than Def 2/2 |  |  |
| 42569 | FAIL | AI won't use Solar Beam if there is no Sun up or the user is not holding Power Herb 1/2 |  |  |
| 42572 | FAIL | Dynamax: G-Max Replenish recycles allies' berries 50% of the time (2/2) |  |  |
| 42574 | FAIL | Dry Skin increases damage taken from Fire-type moves by 25% 1/2 |  |  |
| 42577 | FAIL | Booster Energy activates Quark Drive and increases highest stat 1/5 |  |  |
| 42578 | FAIL | Commander Attacker is kept (Dondozo Right Slot) |  |  |
| 42581 | FAIL | Commander Tatsugiri still avoids moves even when the attacker has No Guard |  |  |
| 42583 | FAIL | Dynamax: G-Max Volt Crash paralyzes both opponents |  |  |
| 42590 | FAIL | Fairy Aura's effect doesn't stack multiple times |  |  |
| 42591 | FAIL | Leaf Guard doesn't prevent status conditions from Flame Orb and Toxic Orb if Cloud Nine/Air Lock is on the field 1/4 |  |  |
| 42593 | FAIL | Dynamax: G-Max Stonesurge sets up Stealth Rocks |  |  |
| 42596 | FAIL | Dynamax: Dynamaxed Pokemon are not affected by phazing moves but no block message is printed if they faint |  |  |
| 42598 | FAIL | Custap Berry allows the holder to move first in its priority bracket when HP is below 1/4 |  |  |
| 42600 | FAIL | (TERA) Reflect Type fails if used by a Terastallized Pokemon |  |  |
| 42602 | FAIL | Cursed Body triggers 30% of the time (2/2) |  |  |
| 42603 | FAIL | Dynamax: Dynamax expires when fainted 1/2 |  |  |
| 42605 | FAIL | Flower Gift transforms Cherrim back to normal when its ability is suppressed |  |  |
| 42606 | FAIL | Dynamax: Max Mindstorm sets up Psychic Terrain |  |  |
| 42607 | FAIL | Ganlon Berry raises the holder's Defense by one stage when HP drops to 1/4 or below 2/2 |  |  |
| 42608 | FAIL | Dancer doesn't trigger if the original user flinches |  |  |
| 42613 | FAIL | (TERA) Status moves don't expend Stellar's one-time type boost |  |  |
| 42614 | FAIL | Dynamax: Max Flare sets up sunlight |  |  |
| 42615 | FAIL | Dauntless Shield raises Defense by one stage every time it switches in (Gen8) |  |  |
| 42617 | FAIL | (TERA) Revelation Dance uses a Stellar-type Pokemon's base type |  |  |
| 42619 | FAIL | (TERA) Terastallizing into a different type gives that type 1.5x STAB 1/2 |  |  |
| 42622 | FAIL | Dynamax: Pain Split uses a Pokemon's non-Dynamax HP 1/2 |  |  |
| 42623 | FAIL | (TERA) Reflect Type copies a Terastallized Pokemon's Tera Type |  |  |
| 42625 | FAIL | Moxie/Chilling Neigh does not increase damage done by the same move that causes another Pokemon to faint 1/3 |  |  |
| 42626 | FAIL | (TERA) Terastallization changes type effectiveness 1/2 |  |  |
| 42627 | FAIL | (TERA) Terastallizing into the same type gives that type 2x STAB 1/2 |  |  |
| 42630 | FAIL | (Z-MOVE) Genesis Supernova sets up psychic terrain when the target is behind a Substitute |  |  |
| 42634 | FAIL | Forecast transforms Castform in weather from an ability 1/3 |  |  |
| 42638 | FAIL | Download raises Sp.Attack if enemy has lower Sp. Def than Def 2/2 |  |  |
| 42641 | FAIL | Galvanize doesn't affect Judgment / Techno Blast / Multi-Attack's type 1/3 |  |  |
| 42642 | FAIL | Early Bird reduces Rest sleep to one turn |  |  |
| 42646 | FAIL | Dynamax: Dynamaxed Pokemon cannot be flinched |  |  |
| 42648 | FAIL | Electromorphosis sets up Charge when hit by any move 1/2 |  |  |
| 42649 | FAIL | TIE_BREAK_TARGET with TARGET_TIE_CHOSEN can correctly control AI target selection when scores are tied 1/12 |  |  |
| 42660 | FAIL | Red Card does not activate if holder is switched in mid-turn |  |  |
| 42664 | FAIL | Forecast transforms Castform when weather changes |  |  |
| 42666 | FAIL | Hospitality ignores Substitute |  |  |
| 42675 | FAIL | Sticky Barb gets transferred if its holder is hit by a contact move 1/3 |  |  |
| 42678 | FAIL | Hydration doesn't cure status conditions if Cloud Nine/Air Lock is on the field |  |  |
| 42679 | FAIL | Ability Shield on fainted ally does not block Receiver/Power of Alchemy 1/2 |  |  |
| 42682 | FAIL | Forewarn randomly chooses between opponents with same-power moves (4/4) |  |  |
| 42683 | FAIL | Ability Shield protects against Neutralizing Gas 1/2 |  |  |
| 42684 | FAIL | Eiscue Noice reverts to Ice Form upon battle end after being hit by a physical move in battle |  |  |
| 42685 | FAIL | AI_FLAG_ATTACKS_PARTNER is willing to kill either the partner or the player 3/6 |  |  |
| 42686 | FAIL | (TERA) Terastallizing into the Stellar-type provides a one-time 2.0x boost to STAB moves |  |  |
| 42691 | FAIL | (TERA) Revelation Dance uses a Terastallized Pokemon's Tera Type |  |  |
| 42692 | FAIL | Pewter Crunchies heals a battler from any primary status 1/7 |  |  |
| 42694 | FAIL | Palafin returns to Zero form upon battle end |  |  |
| 42699 | FAIL | (TERA) Terastallization changes the effect of Curse |  |  |
| 42700 | FAIL | Full Restore resets Toxic Counter |  |  |
| 42702 | FAIL | Parental Bond has no affect on multi hit moves and they still hit four times 12.5/15% of the time 1/2 (6/6) |  |  |
| 42703 | FAIL | Max Mushrooms raises battler's Speed stat 1/2 |  |  |
| 42704 | FAIL | (TERA) Terastallization's 60 BP floor occurs after Technician 1/2 |  |  |
| 42707 | FAIL | X Sp. Atk sharply raises battler's Sp. Attack stat 1/2 |  |  |
| 42711 | FAIL | Aegislash reverts to Shield Form upon fainting (start as Shield) |  |  |
| 42716 | FAIL | Player Pokemon can be further poisoned with Toxic spikes after a status healing hold effect was previously used 1/2 |  |  |
| 42718 | FAIL | Prankster-affected moves don't affect Dark-type Pokémon (Gen7+) 2/2 |  |  |
| 42722 | FAIL | Air Balloon pops before it can be stolen with Magician |  |  |
| 42730 | FAIL | Protosynthesis ability pop up activates only once during the duration of sunny day |  |  |
| 42731 | FAIL | Gem is consumed if the move type is changed |  |  |
| 42732 | FAIL | Eject Button prevents Volt Switch / U-Turn from activating |  |  |
| 42734 | FAIL | Innards Out doesn't trigger if Future Sight user is not on field |  |  |
| 42735 | FAIL | Regular Mega Evolution and Fervent Wish Mega Evolution can happen on the same turn |  |  |
| 42736 | FAIL | Liechi Berry raises the holder's Attack by one stage when HP drops to 1/4 or below 2/2 |  |  |
| 42738 | FAIL | Jaboca Berry is triggered even if berry user dies |  |  |
| 42743 | FAIL | Inner Focus doesn't prevent intimidate (Gen3-7) |  |  |
| 42747 | FAIL | Mega Evolution's order is determined by Speed - player faster |  |  |
| 42750 | FAIL | Intimidate activates when it's no longer effected by Neutralizing Gas - switching out |  |  |
| 42755 | FAIL | Mirror Herb does not trigger for Ally's Soul Heart's stat raise |  |  |
| 42758 | FAIL | Intimidate doesn't activate on an empty field in a double battle |  |  |
| 42761 | FAIL | Keen Eye & Gen9+ Illuminate don't prevent Spectral Thief from resetting positive accuracy stage changes 1/2 |  |  |
| 42762 | FAIL | Protective Pads doesn't reduce tough claws damage 1/2 |  |  |
| 42764 | FAIL | Booster Energy will activate Protosynthesis after harsh sunlight ends |  |  |
| 42766 | FAIL | Howl does not work on partner if it has Soundproof |  |  |
| 42768 | FAIL | Levitate activates when targeted by ground type moves |  |  |
| 42769 | FAIL | (Z-MOVE) Z-Nature Power transforms into different Z-Moves based on the current terrain 4/4 |  |  |
| 42774 | FAIL | Lightning Rod redirects an ally's attack |  |  |
| 42779 | FAIL | Primal Reversion happens after a mon is switched in |  |  |
| 42781 | FAIL | Red Card activates for only the fastest target |  |  |
| 42782 | FAIL | Red Card does not cause the dragged out mon to lose hp due to it's held Life Orb |  |  |
| 42788 | FAIL | Magic Bounce bounces back moves hitting foes field 1/2 |  |  |
| 42801 | FAIL | Safety Goggles block powder and spore moves |  |  |
| 42802 | FAIL | Effect Spore will check if it can inflict status onto attacker, not itself 1/3 (300/300) |  |  |
| 42804 | FAIL | Berserk Gene does not confuse on Misty Terrain but still raises attack sharply |  |  |
| 42807 | FAIL | Magician gets self-damage recoil after stealing Life Orb |  |  |
| 42809 | FAIL | Shield Dust does not prevent ability stat changes |  |  |
| 42810 | FAIL | Red Card switches the attacker with a random non-fainted replacement (2/2) |  |  |
| 42813 | FAIL | Custap Berry activates even if the opposing mon switches out |  |  |
| 42815 | KNOWN_FAILING | Mirror Armor lowers Speed of the partner Pokemon after Court Change was used by the opponent after it set up Sticky Web |  |  |
| 42816 | FAIL | Shell Bell does not activate on Future Sight if the original user is on the field |  |  |
| 42823 | FAIL | Ultra Burst's order is determined by Speed - opponent faster |  |  |
| 42834 | FAIL | Grim Neigh does not trigger if Pokemon faint to indirect damage or damage from other Pokemon 1/2 |  |  |
| 42837 | FAIL | Salac Berry raises Speed by one stage when HP drops to 1/4 or below if holder has Ripen |  |  |
| 42838 | FAIL | Booster Energy activates Protosynthesis and increases highest stat 1/5 |  |  |
| 42843 | FAIL | Moxie/Chilling Neigh does not trigger if Pokemon faint to indirect damage or damage from other Pokemon 1/3 |  |  |
| 42844 | FAIL | AI_FLAG_SMART_MON_CHOICES: Number of hits to KO calculation checks whether incoming damage is less than recurring healing to avoid an infinite loop |  |  |
| 42845 | FAIL | White Herb wont have time to activate if Magician steals it |  |  |
| 42849 | FAIL | Dynamax: G-Max Finale heals allies by 1/6 of their health, even if it faints the foe |  |  |
| 42850 | FAIL | Enigma Berry recovers 25% of HP if hit by super effective move |  |  |
| 42854 | FAIL | Grassy Seed raises the holder's Defense on Grassy Terrain 1/4 |  |  |
| 42862 | FAIL | AI will not try to switch for the same pokemon for 2 spots in a 2v1 battle (all bad moves) 1/2 (1/?) |  |  |
| 42864 | FAIL | Neutralizing Gas only displays exiting message for the last user leaving the field |  |  |
| 42866 | FAIL | Casteliacone heals a battler from any primary status 1/7 |  |  |
| 42870 | FAIL | Dynamax: Max Attacks prints a message when hitting into Max Guard |  |  |
| 42873 | FAIL | Normalize doesn't affect Hidden Power's type |  |  |
| 42879 | FAIL | Antidote heals a battler from being badly poisoned |  |  |
| 42885 | FAIL | Hyper Cutter is ignored by Mold Breaker |  |  |
| 42886 | FAIL | Oblivious prevents Taunt (Gen6+) 2/2 |  |  |
| 42888 | FAIL | Dynamax: G-Max One Blow bypasses Max Guard for full damage 1/2 |  |  |
| 42889 | FAIL | Oblivious prevents Infatuation |  |  |
| 42893 | FAIL | Illusion breaks when attacked behind a substitute |  |  |
| 42894 | FAIL | Stench has a 10% chance to flinch (2/2) |  |  |
| 42902 | FAIL | Own Tempo prevents confusion from moves by the opponent |  |  |
| 42904 | FAIL | X Speed sharply raises battler's Speed stat 1/2 |  |  |
| 42908 | FAIL | Symbiosis transfers its item after Gem consumption and move execution (Gen7+) |  |  |
| 42912 | FAIL | Items can restore a battler's HP by a percentage 2/2 |  |  |
| 42914 | FAIL | Innards Out does not trigger after Gastro Acid has been used |  |  |
| 42915 | FAIL | Coaching bypasses Protect |  |  |
| 42916 | FAIL | Pastel Veil cures partner's poison on initial switch in |  |  |
| 42917 | FAIL | Kings Rock does not increase flinch chance of a move that has the flinch effect (2/2) |  |  |
| 42918 | FAIL | Dynamax: G-Max Wildfire sets a field effect that damages non-Fire types |  |  |
| 42922 | FAIL | Conversion 2 fails if used by a Terastallized Pokemon |  |  |
| 42926 | FAIL | TIE_BREAK_TARGET with TARGET_TIE_RANDOM randomizes AI target (2/2) |  |  |
| 42927 | FAIL | Max Revive restores a fainted battler's HP fully |  |  |
| 42928 | FAIL | Corrosive Gas destroys foes and ally's items if they have one 1/8 |  |  |
| 42931 | FAIL | Teraform Zero cannot be swapped |  |  |
| 42932 | FAIL | Dynamax: G-Max Terror traps both opponents |  |  |
| 42933 | FAIL | Maranga Berry raises the holder's Sp. Def by two stages with Ripen when hit by a special move |  |  |
| 42934 | FAIL | AI sees Loaded Dice damage increase from multi hit moves |  |  |
| 42940 | FAIL | Parental Bond has no affect on multi hit moves and they still hit twice 37.5/35% of the time 1/2 (6/6) |  |  |
| 42942 | FAIL | Dynamax: G-Max Gold Rush confuses both opponents and generates money |  |  |
| 42943 | FAIL | Intrepid Sword activates when it's no longer effected by Neutralizing Gas |  |  |
| 42945 | FAIL | Toxic Chain can inflict bad poison on both foes |  |  |
| 42946 | FAIL | Micle Berry raises the holder's accuracy by 1.2 (2/2) |  |  |
| 42947 | FAIL | Poison Touch activates when user has Protective Pads, but not with Punching Glove 1/2 |  |  |
| 42951 | FAIL | Dynamax: G-Max Hydrosnipe has fixed power and ignores abilities 1/2 |  |  |
| 42954 | FAIL | Electric Terrain protects grounded battlers from falling asleep |  |  |
| 42955 | FAIL | Trace copies opponents ability on switch-in even if opponent switched in at the same time |  |  |
| 42956 | FAIL | Poison Point triggers 1/3 times (Gen3) or 30% (Gen 4+) of the time 1/2 (2/2) |  |  |
| 42958 | FAIL | Dynamax: Max Overgrowth sets up Grassy Terrain |  |  |
| 42959 | FAIL | Red Card does not activate if switched by Dragon Tail 2/2 |  |  |
| 42960 | FAIL | Embargo doesn't prevent Mega Evolution |  |  |
| 42962 | FAIL | Protean/Libero changes the type of the user to the move used every time (Gen6-8) 1/2 |  |  |
| 42963 | FAIL | Ally Switch does not update Future Sight target position when attacker side switches |  |  |
| 42966 | FAIL | Quark Drive uses Wonder Room swapped defenses when choosing boosted stat |  |  |
| 42970 | FAIL | Rain Dish recovers 1/16th of Max HP in Rain |  |  |
| 42972 | FAIL | Ally Switch - move fails if the target was ally which changed position 1/3 |  |  |
| 42974 | FAIL | Encore forces the last move used while asleep |  |  |
| 42976 | FAIL | Dynamax: Max Knuckle raises both allies' attack |  |  |
| 42981 | FAIL | Moody randomly lowers the user's Attack, Defense, Sp. Atk, Sp. Def, or Speed by one stage 1/2 (4/4) |  |  |
| 42982 | FAIL | Endure takes precedence over False Swipe (Gen 5+) |  |  |
| 42987 | FAIL | Dynamax: Endeavor uses a Pokemon's non-Dynamax HP 1/2 |  |  |
| 42989 | FAIL | Room Serive decreases the holder's seep by one stage |  |  |
| 42990 | FAIL | Regenerator heals 1/3 of max HP upon switching out 1/3 |  |  |
| 42991 | FAIL | Work Up raises Attack and Sp. Attack by 1 stage each 2/4 |  |  |
| 42998 | FAIL | Shed Shell allows switching out even when trapped by Arena Trap |  |  |
| 42999 | FAIL | Fling - White Herb effect should not remove the target's held item |  |  |
| 43000 | FAIL | Schooling switches Level 20+ Wishiwashi's form when HP is 25-percent or less at the end of the turn 1/2 |  |  |
| 43002 | FAIL | Dynamax: Dynamaxed Pokemon are not immune to Knock Off |  |  |
| 43005 | FAIL | Fling fails if Pokémon holds no item 1/2 |  |  |
| 43006 | FAIL | Wind Rider raises Attack by one stage if switched into Tailwind on its side of the field |  |  |
| 43007 | FAIL | Shell Bell doesn't restore HP for damage dealt by a foreseen move |  |  |
| 43011 | FAIL | Focus Punch failing occurs before flinching (Gen 5+) |  |  |
| 43015 | FAIL | Dynamax: Dynamaxed Pokemon can be switched out by Eject Button |  |  |
| 43017 | FAIL | Petaya Berry raises Sp. Atk by one stage when HP drops to 1/2 or below if holder has Gluttony |  |  |
| 43021 | FAIL | Parental Bond only triggers Dragon Tail's target switch out on the second hit |  |  |
| 43022 | FAIL | Electric Seed is consumed on Electric Terrain before other abilities change the terrain |  |  |
| 43023 | FAIL | Dynamax: Dynamax expires after three turns 1/2 |  |  |
| 43025 | FAIL | Pastel Veil prevents Toxic bad poison |  |  |
| 43029 | FAIL | Pickpocket activates after Focus Sash is consumed |  |  |
| 43031 | FAIL | Future Sight breaks Focus Sash and doesn't make the holder endure another move |  |  |
| 43033 | FAIL | White Herb is correctly displayed |  |  |
| 43039 | FAIL | White Herb wont have time to activate if it is knocked off or stolen by Thief 1/2 |  |  |
| 43041 | FAIL | Pixilate doesn't affect Judgment / Techno Blast / Multi-Attack's type 1/3 |  |  |
| 43043 | FAIL | Rage Candy Bar heals a battler from any primary status 1/7 |  |  |
| 43046 | FAIL | Grassy Terrain lasts for 5 turns |  |  |
| 43050 | FAIL | Antidote heals a battler from being poisoned |  |  |
| 43051 | FAIL | Belly Drum deducts HP if the user has Contrary and is at -6 |  |  |
| 43055 | FAIL | AI prefers moves with the best possible score, chosen randomly if tied |  |  |
| 43056 | FAIL | Prankster-affected moves called via Assist don't affect Dark-type Pokémon (Gen 7+) 2/2 |  |  |
| 43057 | FAIL | Full Restore heals a party member from any primary status 1/6 |  |  |
| 43061 | FAIL | Protosynthesis uses Wonder Room swapped defenses when choosing boosted stat |  |  |
| 43063 | FAIL | (TERA) Terastallization changes type effectiveness |  |  |
| 43066 | FAIL | Purifying Salt grants immunity to status effects 1/5 |  |  |
| 43067 | FAIL | Ability Shield protects against Sunsteel Strike (no message) 2/2 |  |  |
| 43069 | FAIL | (TERA) Terastallizing boosts moves of the same type to 60 BP 1/2 |  |  |
| 43073 | FAIL | Air Balloon no longer prevents the holder from taking damage from ground type moves once it has been popped |  |  |
| 43081 | FAIL | Berserk Gene does not cause an infinite loop |  |  |
| 43084 | FAIL | Sand Spit sets up sandstorm for 8 turns when hit with Smooth Rock |  |  |
| 43091 | FAIL | After You calculates correct turn order if only one Pokémon is left on the opposing side |  |  |
| 43092 | FAIL | Berserk Gene sharply raises attack at the start of a single battle 2/2 |  |  |
| 43093 | FAIL | Corrosive Gas destroys the target's item or fails if the target has no item 1/2 |  |  |
| 43097 | FAIL | Dragon Tail switches target out and incoming mon has Immunity negated by Mold Breaker |  |  |
| 43103 | FAIL | Defog removes Toxic Spikes from user's side (Gen 6+) 1/3 |  |  |
| 43106 | FAIL | Ally switch swaps opposing sky drop targets if partner is being held in the air |  |  |
| 43109 | FAIL | Scrappy prevents Intimidate (Gen8+) |  |  |
| 43111 | FAIL | Ingrain does not prevent switching out with Flip Turn |  |  |
| 43114 | FAIL | TIE_BREAK_SCORE with SCORE_TIE_RANDOM randomizes AI move selection (Singles) (4/4) |  |  |
| 43115 | FAIL | Ally Switch fails if there is no partner |  |  |
| 43116 | FAIL | AI sees increased base power of Facade 2/2 |  |  |
| 43118 | FAIL | Instructed move will be redirected by Rage Powder after instructed target loses Grass typing 1/2 |  |  |
| 43122 | FAIL | Shed Skin triggers 33% (Gen3, Gen5+) or 30% (Gen 4) of the time 1/3 (2/2) |  |  |
| 43123 | FAIL | Dragon Cheer increases critical hit ratio by 1 on non-Dragon types 2/18 (2/2) |  |  |
| 43124 | FAIL | Ion Deluge works the same way as always when used by a mon with Lightning Rod / Motor Drive 1/2 |  |  |
| 43125 | FAIL | Aromatic Mist raises Sp. Defense of a target ally by 1 stage |  |  |
| 43130 | FAIL | Lansat Berry raises the holder's critical-hit-ratio by two stages when HP drops to 1/4 or below 2/2 |  |  |
| 43131 | KNOWN_FAILING | Dream Eater works if the target is behind a Substitute (Gen 5+) |  |  |
| 43132 | FAIL | Sleep Clause: Sleep from Effect Spore will not activate sleep clause (100/100) |  |  |
| 43135 | FAIL | Attract fails if the target is already infatuated |  |  |
| 43140 | FAIL | Knock Off activates after Rocky Helmet and Weakness Policy 1/2 |  |  |
| 43145 | FAIL | Spread Moves: Super Effective Message on both player mons |  |  |
| 43146 | FAIL | Beak Blast's charging message is shown before other moves are used |  |  |
| 43147 | FAIL | Embargo disables the effect of the Memory items on the move Multi Attack 1/2 |  |  |
| 43148 | FAIL | Bide deals twice the taken damage over two turns |  |  |
| 43151 | FAIL | Nightmare affects Pokémon with Comatose |  |  |
| 43152 | FAIL | Attract fails if both the user and the target are genderless |  |  |
| 43154 | FAIL | Eject Button is activated before Emergency Exit |  |  |
| 43155 | FAIL | Octolock triggers Defiant for both stat reductions |  |  |
| 43159 | FAIL | Ceaseless Edge sets up hazards after hitting the target |  |  |
| 43160 | FAIL | Endure is not transferred to a mon that is switched in due to Eject Button |  |  |
| 43162 | FAIL | Steam Engine raises speed when hit by a Fire or Water move 1/2 |  |  |
| 43174 | FAIL | Chilly Reception switches the user out, even if the weather does not change |  |  |
| 43177 | FAIL | Parting Shot: Stat drop prevention by abilities/items switches (Gen6) 1/4 |  |  |
| 43178 | FAIL | Chilly Reception doesn't announce its move if it's called by a different move |  |  |
| 43181 | FAIL | Pledge move combo fails if ally fails to act - Flinch Right 1/6 |  |  |
| 43183 | FAIL | Coaching fails if there's no ally |  |  |
| 43190 | FAIL | Focus Punch will lose focus if damaged when used by selecting a different move and being Encored (Gen 3-4) |  |  |
| 43193 | FAIL | Teeter Dance confuses target |  |  |
| 43194 | FAIL | Focus Punch uses PP when losing focus (Gen 3-4) 1/2 |  |  |
| 43199 | FAIL | Magic Coat prints the correct message when bouncing back a move |  |  |
| 43201 | FAIL | Conversion 2's type change considers dynamic type moves |  |  |
| 43206 | FAIL | Life Orb activates if move connected but no damage was dealt |  |  |
| 43220 | FAIL | Metronome picks a random move |  |  |
| 43221 | FAIL | Grudge's effect doesn't trigger on indirect damage - Leech Seed |  |  |
| 43225 | FAIL | Harden raises Defense by 1 stage 2/2 |  |  |
| 43232 | FAIL | Court Change used by the player swaps G-Max Steelsurge |  |  |
| 43235 | FAIL | Aerilate doesn't affect Terrain Pulse's type |  |  |
| 43237 | FAIL | Protect: Burning Bulwark burns Pokémon for moves making contact 1/3 |  |  |
| 43240 | FAIL | Trace copies opponents ability on switch-in |  |  |
| 43241 | FAIL | Quick Claw activates 20% of the time (2/2) |  |  |
| 43242 | FAIL | Effect Spore causes sleep 3.3% (Gen3), 10% (Gen4) and 11% (Gen5+) of the time 1/3 (300/300) |  |  |
| 43244 | FAIL | Psychic Terrain doesn't block priority field moves |  |  |
| 43247 | FAIL | Heal Bell does not cure Soundproof partners (Gen 4, Gen 6+) 2/4 |  |  |
| 43254 | FAIL | Bad Dreams causes Pokémon with Comatose to lose 1/8 of HP |  |  |
| 43255 | FAIL | Destiny Bond does not fail if used repeatedly separated by other moves (Gen7+) |  |  |
| 43258 | FAIL | Hit Escape: Held items are consumed immediately after a mon switched in by U-turn: player side |  |  |
| 43259 | FAIL | Fling - Item is lost even when there is no target |  |  |
| 43266 | FAIL | Volt Absorb prevents Cell Battery from activating |  |  |
| 43268 | FAIL | Guard Dog raises Attack when intimidated 1/2 |  |  |
| 43269 | FAIL | Water Absorb activates on status moves |  |  |
| 43272 | FAIL | Pursuit only attacks a switching foe if user is alive |  |  |
| 43273 | FAIL | Weak Armor still lowers Defense if Speed can't go any higher |  |  |
| 43279 | FAIL | Wind Power activates correctly for every battler with the ability when hit by a 2/3 target move 1/3 |  |  |
| 43280 | FAIL | Electric Terrain lasts for 5 turns |  |  |
| 43281 | FAIL | Hyper Cutter prevents Attack stage reduction from moves |  |  |
| 43284 | FAIL | Effect Spore causes paralysis 3.3% (Gen3) and 10% (Gen4+) of the time 1/3 (300/300) |  |  |
| 43285 | FAIL | Echoed Voice's power is increased even if it misses |  |  |
| 43286 | FAIL | Rage volatile persists when using Rage again |  |  |
| 43288 | FAIL | Zero to Hero will activate if a switch move is used |  |  |
| 43289 | FAIL | Embargo doesn't prevent Primal Reversion |  |  |
| 43295 | FAIL | Life Orb activates if it hits a Substitute |  |  |
| 43298 | FAIL | Embargo doesn't block the effects of berries obtained through Bug Bite or Pluck |  |  |
| 43299 | FAIL | Embargo negates a held item's Speed reduction |  |  |
| 43304 | FAIL | Good as Gold protects from status moves |  |  |
| 43305 | FAIL | Mold Breaker ignores Inner Focus |  |  |
| 43312 | FAIL | Color Change changes the type to Electric when a Pokemon is hit by a forseen attack under the effect of Electrify |  |  |
| 43313 | FAIL | Burn Up user loses its Fire-type if enemy faints |  |  |
| 43314 | FAIL | Recoil if miss: Disguise doesn't prevent crash damage from Jump Kick into ghost types 1/2 |  |  |
| 43315 | FAIL | Gulp Missile: Power Herb does not prevent Cramaront from transforming |  |  |
| 43317 | FAIL | Micle Berry raises the holder's accuracy by 1.2 when HP drops to 1/4 or below 2/2 |  |  |
| 43336 | FAIL | Commander Tatsugiri does not attack if Dondozo faints the same turn it's switched in |  |  |
| 43337 | FAIL | Fling - thrown berry's effect activates for the target even if the trigger conditions are not met 1/18 |  |  |
| 43342 | FAIL | Red Card activates but fails if the attacker has Guard Dog |  |  |
| 43343 | FAIL | Reflect Type does not affect any of Silvally's forms 1/18 |  |  |
| 43344 | FAIL | Intimidate activates on an empty slot |  |  |
| 43354 | FAIL | Commander cannot affect a Dondozo that was previously affected by Commander until it faints and revived |  |  |
| 43355 | FAIL | Fling's thrown item can be regained with Recycle |  |  |
| 43356 | ASSUMPTION_FAIL | AI won't use status moves if the player's best attacking move is Focus Punch |  |  |
| 43360 | FAIL | Spicy Extract stat changes will be inverted by Contrary |  |  |
| 43366 | FAIL | Intrepid Sword raises Attack by one stage only once per battle (Gen9+) |  |  |
| 43369 | FAIL | Spikes damage on switch in 1/3 |  |  |
| 43370 | FAIL | Pledge move combo fails if ally fails to act - Paralyzed Both Left Faster |  |  |
| 43371 | FAIL | Restore HP Item effects do not miss timing 1/3 |  |  |
| 43373 | FAIL | Intrepid Sword raises Attack by one stage |  |  |
| 43376 | FAIL | Mirror Move copies the last used move by the target |  |  |
| 43379 | FAIL | Sticky Web can only be set up 1 time |  |  |
| 43380 | FAIL | Knock Off does not trigger the opposing ally's Symbiosis |  |  |
| 43381 | FAIL | Future Sight uses Sp. Atk stat of the original user without modifiers 1/2 |  |  |
| 43383 | INVALID | Light Metal doesn't affect Heavy Ball's multiplier |  |  |
| 43389 | FAIL | Levitate does not cause single remaining target to take higher damage |  |  |
| 43390 | INVALID | Heavy Metal doesn't affect Heavy Ball's multiplier |  |  |
| 43395 | FAIL | Apicot Berry raises Sp. Def by one stage when HP drops to 1/2 or below if holder has Gluttony |  |  |
| 43397 | FAIL | Stockpile's count can go up only to 3 |  |  |
| 43398 | FAIL | Liquid Ooze causes Matcha Gatcha users to lose HP instead of heal |  |  |
| 43399 | FAIL | Grudge does not activate for Struggle |  |  |
| 43401 | FAIL | Liquid Ooze causes Strength Sap users to lose HP instead of heal 1/2 |  |  |
| 43402 | FAIL | Last Resort works only when all of the known moves have been used - 4 moves |  |  |
| 43407 | FAIL | Psychic Seed raises the holder's Sp. Defense on Psychic Terrain 1/4 |  |  |
| 43409 | FAIL | Stone Axe can set up pointed stones only once |  |  |
| 43413 | FAIL | Dancer can still copy a move even if it's being forced into a different move - Encore |  |  |
| 43415 | FAIL | Healing Wish causes the user to faint and heals the replacement's HP and status (doubles) |  |  |
| 43416 | FAIL | White Herb restores stats after Attack was lowered by Intimidate in doubles |  |  |
| 43417 | FAIL | Mimicry changes the battler's type based on Terrain 1/4 |  |  |
| 43418 | FAIL | Strength Sap works exactly the same when attacker is behind substitute 1/2 |  |  |
| 43424 | FAIL | Jubilife Muffin heals a battler from any primary status 1/7 |  |  |
| 43428 | FAIL | Lunar Dance causes the user to faint and heals the replacement's HP, PP and status (singles) |  |  |
| 43430 | FAIL | Ice Heal heals a battler from being frozen or frostbite 1/2 |  |  |
| 43431 | FAIL | Teatime causes the user to consume its Berry, even in the pressence of Unnerve |  |  |
| 43432 | FAIL | Magnet Rise fails if the user is Grounded by Smack Down |  |  |
| 43433 | FAIL | Dancer-called moves doesn't update move to be called by Mirror Move |  |  |
| 43439 | FAIL | Me First fails if target uses a status move |  |  |
| 43440 | FAIL | Full Restore restores a party members HP and cures any primary status 1/7 |  |  |
| 43441 | FAIL | Dancer still triggers if another dancer flinches |  |  |
| 43443 | FAIL | Tera Starstorm remains Normal-type if used by Pokemon other than Terapagos |  |  |
| 43452 | FAIL | Dauntless Shield raises Defense by one stage only once per battle (Gen 9+) |  |  |
| 43455 | FAIL | Booster Energy's Quark Drive boost is preserved when terrain changes |  |  |
| 43459 | FAIL | Revive works for a partner in a double battle |  |  |
| 43461 | FAIL | Defiant doesn't display ability popup when already at Maximum Attack |  |  |
| 43463 | FAIL | Oblivious prevents Intimidate (Gen8+) |  |  |
| 43464 | FAIL | Protect: Wide Guard protects self and ally from multi-target moves 1/3 |  |  |
| 43468 | FAIL | Normalize turns a move into a Normal-type move 2/2 |  |  |
| 43469 | FAIL | Defiant activates before White Herb 1/2 |  |  |
| 43473 | FAIL | Toxic Spikes fails after 2 layers |  |  |
| 43474 | FAIL | Clear Amulet prevents Intimidate |  |  |
| 43476 | FAIL | Disguised Mimikyu takes damage from Rough Skin without breaking the disguise 1/2 |  |  |
| 43477 | FAIL | Transform fails on semi-invulnerable target in Gen2+ 1/1 |  |  |
| 43479 | FAIL | Covert Cloak does not prevent ability stat changes |  |  |
| 43483 | FAIL | Ally Switch updates attract battler |  |  |
| 43485 | FAIL | Lansat Berry raises the holder's critical-hit-ratio by two stages when HP drops to 1/2 or below |  |  |
| 43487 | FAIL | Drizzle summons rain 1/2 |  |  |
| 43491 | FAIL | Lum Berry properly cures a battler affected by a non-volatiles status and confusion 1/6 |  |  |
| 43495 | FAIL | AI partner will not switch into a player Pokémon after fainting (multi) 1/2 (1/?) |  |  |
| 43496 | FAIL | Electromorphosis displays its message before fainting when triggered |  |  |
| 43497 | FAIL | Parental Bond Snore strikes twice while asleep |  |  |
| 43503 | FAIL | Assist fails if there are no valid moves to choose from |  |  |
| 43511 | FAIL | Pickpocket activates after Sticky Barb transfers |  |  |
| 43516 | FAIL | Pursuit attacks a switching foe but not switching allies |  |  |
| 43517 | FAIL | Pickup restores an item that was used by Natural Gift |  |  |
| 43521 | FAIL | Pickup grants an item used by itself in wild battles (Gen9+) |  |  |
| 43531 | FAIL | Attract causes the target to become infatuated with the user if they have opposite genders |  |  |
| 43541 | FAIL | Rage volatile is cleared when using a different move |  |  |
| 43543 | FAIL | Pledge move combo fails if ally fails to act - Sleep Both Left Faster 1/3 |  |  |
| 43547 | FAIL | Crafty Shield does not protect against moves that target all battlers 1/2 |  |  |
| 43552 | FAIL | Rage Fist base power is not lost if user switches out |  |  |
| 43554 | FAIL | Rainbow flinch chance does not stack with Serene Grace (2/2) |  |  |
| 43557 | FAIL | Protect fails when the only slower battler is a fainted ally |  |  |
| 43558 | FAIL | Forewarn does not trigger if a mon switches in while the opposing field is empty |  |  |
| 43559 | FAIL | Steel Beam causes the user to faint when below 1/2 of its Max HP in a double battle |  |  |
| 43568 | FAIL | Belly Drum fails if the user's Attack is already at +6 |  |  |
| 43573 | FAIL | Recoil if miss: Jump Kick recoil happens after Spiky Shield damage |  |  |
| 43574 | FAIL | Prankster-affected moves don't affect Dark-type Pokémon after they switch-in |  |  |
| 43575 | FAIL | Frisk triggers for player in a Double Battle after switching-in after fainting 1/2 |  |  |
| 43577 | FAIL | Protect: King's Shield, Silk Trap and Obstruct protect from damaging moves and lower stats on contact 1/9 |  |  |
| 43578 | FAIL | Misty Terrain does not increase the power of Fairy-type moves 1/2 |  |  |
| 43584 | FAIL | Scale Shot is immune to Fairy types and will end the move correctly |  |  |
| 43587 | FAIL | Galvanize doesn't affect Natural Gift's type 1/2 |  |  |
| 43590 | FAIL | Psychic Terrain lasts for 5 turns |  |  |
| 43591 | FAIL | Chilly Reception changes the weather, even if the user cannot switch out |  |  |
| 43592 | FAIL | Sheer Force only boosts the damage of moves it's supposed to boost (Gen1) 80/109 |  |  |
| 43595 | FAIL | Coaching raises Attack and Defense of ally by 1 stage each |  |  |
| 43598 | FAIL | Psychic Terrain protects grounded battlers from priority moves |  |  |
| 43603 | FAIL | Quick Draw has a 30% chance of going first (2/2) |  |  |
| 43605 | FAIL | Octolock ends after user that set the lock switches out |  |  |
| 43609 | FAIL | Conversion 2 fails if the targeted move is Stellar Type |  |  |
| 43611 | FAIL | Conversion 2 randomly changes the type of the user to a type that resists the last used target's move (Gen 5+) |  |  |
| 43612 | FAIL | Sheer Cold doesn't affect Ice-type Pokémon (Gen7+) |  |  |
| 43614 | FAIL | Protect: Protect, Detect, Spiky Shield, Baneful Bunker and Burning Bulwark protect from all moves 1/15 |  |  |
| 43615 | FAIL | Refresh cures the user of burn, frostbite, poison, and paralysis 1/5 |  |  |
| 43616 | FAIL | Rapid Spin blows away all hazards |  |  |
| 43617 | FAIL | Items lost to Corrosive Gas cannot be restored by Recycle |  |  |
| 43620 | FAIL | Mega Evolution happens after switching, but before Focus Punch-like Moves |  |  |
| 43622 | FAIL | Pursuited mon correctly switches out after it got hit and activated ability Cotton Down |  |  |
| 43623 | FAIL | TIE_BREAK_SCORE correctly controls AI move selection when scores are tied for all values in enum ScoreTieResolution (Singles) 1/4 |  |  |
| 43628 | ASSUMPTION_FAIL | Roost's suppression prevents Reflect Type from copying any Flying typing |  |  |
| 43629 | ASSUMPTION_FAIL | Roost fails if the user is under the effects of Heal Block |  |  |
| 43631 | FAIL | Mega Evolution's order is determined by Speed - opponent faster |  |  |
| 43635 | FAIL | Court Change swaps entry hazards used by the player |  |  |
| 43636 | FAIL | Parting Shot: Mist prevents stat drops and switches (Gen6) |  |  |
| 43638 | FAIL | Pursuit attacks a switching foe but isn't affected by Follow Me |  |  |
| 43641 | FAIL | Parting Shot: Soundproof and Good as Gold block Parting Shot 1/2 |  |  |
| 43642 | FAIL | Seed Sower sets up Grassy Terrain even when the user faints from an attack |  |  |
| 43643 | FAIL | Primal Reversion happens immediately if it was brought in by U-turn |  |  |
| 43645 | FAIL | Defog removes Aurora Veil from target's side 1/2 |  |  |
| 43647 | FAIL | Pursuit doesn't attack switching foe if user already acted that turn |  |  |
| 43649 | FAIL | Ice Face is restored if hail or snow begins while Noice Face Eiscue is out 1/2 |  |  |
| 43650 | FAIL | Shed Tail creates a Substitute with 1/4 of user maximum health 1/2 |  |  |
| 43653 | FAIL | Primal Reversion happens after a mon is sent out after a mon is fainted |  |  |
| 43654 | FAIL | Defog lowers evasiveness by 1 stage |  |  |
| 43657 | FAIL | Defense Curl raises Defense by 1 stage 2/2 |  |  |
| 43660 | FAIL | Rage volatile behavior on Protect depends on generation 1/2 |  |  |
| 43661 | FAIL | Revival Blessing revives a chosen fainted party member for the player |  |  |
| 43663 | FAIL | Shell Trap fails if an other -3 or lower priority Move is used |  |  |
| 43668 | FAIL | Innards Out triggers if Future Sight user is back on the field |  |  |
| 43669 | FAIL | Dusk Mane Necrozma can Ultra Burst holding Ultranecrozium Z |  |  |
| 43671 | FAIL | Defog removes Mist and Safeguard from target's side 1/2 |  |  |
| 43683 | FAIL | Inner Focus prevents intimidate (Gen8+) |  |  |
| 43687 | FAIL | Mega Evolution affects turn order (Gen7+) |  |  |
| 43693 | FAIL | Dream Eater fails if Heal Block applies |  |  |
| 43696 | FAIL | Reflect Type defaults to Normal type for the user's 1st and 2nd types if the target only has a 3rd type |  |  |
| 43697 | FAIL | Triple Arrows makes the foe flinch 30% of the time 1/2 (2/2) |  |  |
| 43698 | FAIL | Max Mushrooms raises battler's Defense stat 1/2 |  |  |
| 43699 | FAIL | Intimidate activates when it's no longer affected by Neutralizing Gas - switching moves 1/3 |  |  |
| 43700 | FAIL | Crush Grip's damage is affected by the target's current HP 1/4 |  |  |
| 43701 | FAIL | Snatch fails when the only slower battler is a fainted ally |  |  |
| 43702 | FAIL | Embargo can be reflected by Magic Coat |  |  |
| 43711 | FAIL | Terrain started after the one which started the battle lasts only 5 turns 1/2 |  |  |
| 43712 | FAIL | Encore works even if the target's last move failed |  |  |
| 43713 | FAIL | Embargo makes Fling and Natural Gift fail 1/2 |  |  |
| 43716 | FAIL | Primal Reversion's order is determined by Speed - opponent faster |  |  |
| 43718 | FAIL | Solar Beam does half damage if Sandstorm is up (Gen3+) 1/3 |  |  |
| 43721 | FAIL | Psych Up does not copy the target's critical hit ratio (Gen5) |  |  |
| 43724 | FAIL | Encore forces the last move used before the target flinched |  |  |
| 43726 | FAIL | Ultra Burst happens after switching, but before Focus Punch-like Moves |  |  |
| 43730 | FAIL | Fillet Away's HP cost doesn't trigger effects that trigger on damage taken |  |  |
| 43731 | FAIL | Endure only lasts for one turn |  |  |
| 43735 | FAIL | Ultra Burst's order is determined by Speed - player faster |  |  |
| 43737 | FAIL | Shed Tail's HP cost doesn't trigger effects that trigger on damage taken |  |  |
| 43741 | FAIL | Psychic Terrain doesn't block priority moves against semi-invulnerable targets 2/2 |  |  |
| 43744 | FAIL | Dynamax: max move against semi-invulnerable target prints the correct message |  |  |
| 43746 | FAIL | Berserk Gene activates on switch in 2/2 |  |  |
| 43747 | FAIL | Sonic Boom doesn't affect ghost types (Gen2+) |  |  |
| 43749 | FAIL | Fling doesn't consume the item if the user is asleep/frozen/paralyzed 1/6 |  |  |
| 43750 | FAIL | Magic Bounce bounced back status moves can not be bounced back by Magic Bounce |  |  |
| 43758 | FAIL | Dynamax: Moxie clones can be triggered by Max Moves fainting opponents |  |  |
| 43759 | FAIL | Fling - Mental Herb effect should not remove the target's held item |  |  |
| 43761 | FAIL | Magician steals before switching with U-turn |  |  |
| 43765 | FAIL | Shell Trap targets correctly if one of the opponents has fainted |  |  |
| 43768 | FAIL | Dynamax: G-Max Depletion takes away 2 PP from the target's last move |  |  |
| 43776 | FAIL | Pursuit attacks a switching foe and takes Life Orb damage |  |  |
| 43780 | FAIL | Dynamax: G-Max Sweetness cures allies' status conditions |  |  |
| 43781 | FAIL | Grudge's effect disappears if the user takes a new turn - Move |  |  |
| 43784 | FAIL | Dynamax: Max Strike lowers both opponents' speed |  |  |
| 43787 | FAIL | Booster Energy will activate Quark Drive after Electric Terrain ends |  |  |
| 43788 | FAIL | Pursuit attacks a foe using Volt Switch / U-Turn / Parting Shot to switch out 1/3 |  |  |
| 43789 | FAIL | Sticky Web has correct interactions with Mirror Armor - no one has their Speed lowered if the set upper fainted 1/2 |  |  |
| 43791 | FAIL | Dynamax: G-Max Meltdown torments both opponents for 3 turns |  |  |
| 43792 | FAIL | Clear Amulet prevents stat reducing effects 1/7 |  |  |
| 43793 | FAIL | Dynamax: Dynamaxed Pokemon cannot use Max Guard while holding Assault Vest |  |  |
| 43795 | FAIL | Focus Punch will lose focus if damaged when encored into a different move and selected Focus Punch (Gen 5-6) |  |  |
| 43796 | FAIL | Healing Wish effect activates only if the switched Pokémon can be healed (Gen8+) 1/3 |  |  |
| 43797 | FAIL | Stomping Tantrum will not deal double damage if target protects |  |  |
| 43798 | FAIL | Ganlon Berry raises Defense by one stage when HP drops to 1/4 or below if holder has Ripen |  |  |
| 43800 | FAIL | Dynamax: G-Max Cuddle infatuates both opponents, if possible |  |  |
| 43802 | FAIL | Dynamax: Feint bypasses Max Guard but doesn't break it |  |  |
| 43804 | FAIL | Focus Punch failing occurs after flinching (Gen 3-4) |  |  |
| 43811 | FAIL | Dynamax: Dynamaxed Pokemon are immune to Torment |  |  |
| 43812 | FAIL | Swords Dance raises Attack by 2 stages 2/2 |  |  |
| 43813 | FAIL | Dynamax: G-Max Befuddle paralyzes, poisons, or sleeps both opponents 1/3 |  |  |
| 43815 | FAIL | Hold Hands is blocked by Crafty Shield |  |  |
| 43819 | FAIL | Stuff Cheeks can be used even if Unnerve is present |  |  |
| 43828 | FAIL | Teatime triggers Lightning Rod if it has been affected by Electrify or Plasma Fists, even when not holding a Berry 1/5 |  |  |
| 43829 | FAIL | Dynamax: Max Rockfall sets up a sandstorm |  |  |
| 43831 | FAIL | Dynamax: Dynamaxed Pokemon are not affected by phazing moves, but still take damage |  |  |
| 43833 | FAIL | Eject Pack does not activate if mon is switched in due to Eject Button |  |  |
| 43836 | FAIL | Dynamax: Dynamax Level increases HP and max HP multipliers by 0.05 for each level 1/12 |  |  |
| 43837 | FAIL | Synchronoise will fail if there is no corresponding typing on the field |  |  |
| 43841 | FAIL | Dynamax: Max Strike lowers single opponent's speed |  |  |
| 43845 | FAIL | Future Sight will miss timing if target faints by residual damage |  |  |
| 43847 | FAIL | Oblivious prevents Captivate |  |  |
| 43848 | FAIL | Dynamax: Dynamaxed Pokemon are not affected by Choice items 1/2 |  |  |
| 43849 | FAIL | Reflect Type copies a target's dual types |  |  |
| 43854 | FAIL | Teatime causes all Pokémon to consume their berry 1/4 |  |  |
| 43856 | FAIL | Grassy Terrain increases power of Grass-type moves by 30/50 percent 1/2 |  |  |
| 43859 | FAIL | Tidy Up removes Substitute |  |  |
| 43860 | FAIL | Dynamax: Dynamaxed Pokemon can have base moves disabled on their first turn |  |  |
| 43862 | FAIL | Opportunist copies the stat of each Pokémon that were raised at the same time |  |  |
| 43863 | FAIL | Toxic cannot miss if used by a Poison-type (Gen6+) 4/4 |  |  |
| 43865 | FAIL | Recycle cannot recover an item removed by Knock Off |  |  |
| 43866 | FAIL | Heal Bell cures a Soundproof user (Gen5, Gen8+) 1/4 |  |  |
| 43868 | FAIL | Dynamax: Dynamaxed Pokemon are affected by Grudge |  |  |
| 43871 | FAIL | Toxic Spikes inflicts poison on switch in after Primal Reversed mon fainted |  |  |
| 43873 | FAIL | Dynamax: Dynamax increases HP and max HP by 1.5x 1/2 |  |  |
| 43875 | FAIL | Overcoat blocks powder and spore moves (Gen6+) 2/2 |  |  |
| 43883 | FAIL | Hit Escape: Held items are consumed immediately after a mon switched in by U-turn: opposing side |  |  |
| 43884 | FAIL | Sheer Force only boosts the damage of moves it's supposed to boost (Gen8) 26/65 |  |  |
| 43886 | FAIL | Sky Attack needs a charging turn |  |  |
| 43891 | FAIL | Dragon Tail effect fails against target with Suction Cups |  |  |
| 43894 | FAIL | Revival Blessing correctly updates battler absent flags |  |  |
| 43897 | FAIL | Pastel Veil cures partner's poison on switch in |  |  |
| 43905 | FAIL | Roar fails against target with Guard Dog |  |  |
| 43906 | FAIL | Wish is blocked by Heal Block |  |  |
| 43909 | FAIL | Ability Shield activates a previously suppressed ability when obtained |  |  |
| 43911 | FAIL | AI sees increased base power of Smelling Salt 2/2 |  |  |
| 43913 | ASSUMPTION_FAIL | Roost does not suppress the ungrounded effect of Telekinesis |  |  |
| 43914 | ASSUMPTION_FAIL | Roost, if used by a Mystery/Flying type, treats the user as a Mystery/Mystery type until the end of the turn |  |  |
| 43918 | FAIL | Defog removes everything it can 1/2 |  |  |
| 43919 | FAIL | Ability Shield prevents Intimidate from reactivating after Neutralizing Gas ends |  |  |
| 43923 | FAIL | Pickpocket activates for the fastest itemless target when both are hit by a contact spread move |  |  |
| 43927 | FAIL | Air Balloon prevents the user from being healed by Grassy Terrain |  |  |
| 43928 | FAIL | Ion Deluge works the same way as always when used by a mon with Volt Absorb |  |  |
| 43929 | FAIL | Bug Bite eats the target's berry and immediately gains its effect 1/18 |  |  |
| 43933 | FAIL | Defog fails if target has minimum evasion stat change |  |  |
| 43941 | FAIL | Knock Off does not activate if user faints |  |  |
| 43942 | FAIL | Heal Powder heals a battler from any primary status 1/7 |  |  |
| 43943 | FAIL | Explosion causes the user to faint even if it misses |  |  |
| 43955 | FAIL | Knock Off does knock off Mega Stones from Pokemon that don't actually use them |  |  |
| 43958 | FAIL | Full Restore restores a battler's HP and cures confusion |  |  |
| 43961 | FAIL | Pixilate doesn't change Tera Blast's type when Terastallized |  |  |
| 43964 | FAIL | Max Mushrooms battler's Sp. Defense stat 1/2 |  |  |
| 43969 | FAIL | Knock Off knocks a healing berry before it has the chance to activate |  |  |
| 43971 | FAIL | X Defense sharply raises battler's Defense stat 1/2 |  |  |
| 43973 | FAIL | SetStartingStatus can start Spikes on the player side 1/3 |  |  |
| 43978 | FAIL | Thunder Wave doesn't print an effectiveness message |  |  |
| 43986 | FAIL | Prankster-affected moves can still be bounced back by Dark-types using Magic Coat |  |  |
| 43990 | FAIL | Teatime causes the user to consume its Berry, even under the effects of Magic Room |  |  |
| 43992 | FAIL | Teleport fails in Trainer Battles (Gen 1-7) |  |  |
| 43993 | FAIL | Scale Shot decreases defense and increases speed after the 4th hit of Loaded Dice (2/2) |  |  |
| 43995 | FAIL | Teleport does not fail if the user is trapped |  |  |
| 43996 | FAIL | Electric Terrain increases power of Electric-type moves by 30/50 percent 1/2 |  |  |
| 43998 | FAIL | Confusion adds a 50/33% chance to hit self with 40 power 1/2 (2/2) |  |  |
| 44000 | FAIL | Tera Blast changes from Normal-type to the user's Tera Type |  |  |
| 44006 | FAIL | Tera Blast becomes a physical move if the user is Terastallized and has a higher Attack stat 1/2 |  |  |
| 44007 | FAIL | Revive restores a fainted battler's HP to half |  |  |
| 44010 | FAIL | Embargo blocks the effect of an affected Pokémon's held item |  |  |
| 44014 | FAIL | OHKO moves can can be endured by Focus Sash |  |  |
| 44020 | FAIL | Flame Burst doesn't crash, opponent to player |  |  |
| 44022 | FAIL | (DYNAMAX) Dynamaxed Pokemon are immune to Encore |  |  |
| 44023 | FAIL | Torment works even if the target's last move failed |  |  |
| 44026 | FAIL | Air Balloon can not be restored with Recycle after it has been popped |  |  |
| 44031 | FAIL | Endure takes precedence over Sturdy (Gen 5+) |  |  |
| 44034 | FAIL | Lansat Berry raises the holder's critical-hit-ratio by 2 stages 1/9 (2/2) |  |  |
| 44036 | FAIL | Berserker Gene confusion can be healed with bag items 1/11 |  |  |
| 44040 | FAIL | Plasma Fists turns normal type dynamax-moves into electric type moves |  |  |
| 44043 | FAIL | Ally Switch does not update leech seed position |  |  |
| 44044 | FAIL | Sappy Seed can seed the target |  |  |
| 44045 | FAIL | Pledge move combo fails if ally fails to act - Sleep Both Left Wake Up 1/2 |  |  |
| 44048 | FAIL | Lunar Dance effect activates only if the switched Pokémon can be healed (Gen8+) 1/3 |  |  |
| 44050 | FAIL | Fling fails if Pokémon is under the effects of Embargo or Magic Room 1/3 |  |  |
| 44051 | FAIL | Custap Berry allows the holder to move first in its priority bracket when HP is below 1/2. If the holder has Gluttony |  |  |
| 44054 | FAIL | Psychic Noise is blocked by Soundproof |  |  |
| 44056 | FAIL | White Herb has correct interactions with Intimidate triggered Defiant and Competitive 1/2 |  |  |
| 44057 | FAIL | Toxic Spikes inflicts poison on subsequent switch ins |  |  |
| 44061 | FAIL | Ganlon Berry raises Defense by one stage when HP drops to 1/2 or below if holder has Gluttony |  |  |
| 44062 | FAIL | Grass and Water Pledge create a swamp on the user's side of the field for four turns |  |  |
| 44063 | FAIL | Ally Switch does not redirect moves done by Pokémon with Stalwart and Propeller Tail 1/3 |  |  |
| 44066 | FAIL | Lumiose Galette heals a battler from any primary status 1/7 |  |  |
| 44067 | FAIL | Smelling Salts get incread power vs. paralyzed targets 1/2 |  |  |
| 44073 | FAIL | Eject Button is not triggered after High Jump Kick crash damage |  |  |
| 44077 | FAIL | Antidote resets Toxic Counter |  |  |
| 44078 | FAIL | Mirror Move fails if no move was used before |  |  |
| 44080 | FAIL | Transform fails on transformed target in Gen2+ 1/2 |  |  |
| 44083 | FAIL | Sticky syrup will not decrease speed further then minus six |  |  |
| 44084 | FAIL | Poke Toy lets the player escape from a wild battle even if a move forbid them to |  |  |
| 44087 | FAIL | Rattled boosts speed by 1 when hit by Bug, Dark or Ghost type move 1/4 |  |  |
| 44091 | FAIL | Howl raises user's and partner's Attack by 1 stage 2/2 |  |  |
| 44104 | FAIL | AI partner will not switch mid-turn into a player Pokémon (2v1) 1/2 (1/?) |  |  |
| 44106 | FAIL | X Accuracy sharply raises battler's Accuracy stat (2/2) |  |  |
| 44108 | FAIL | Freeze is thawed by opponent's attack that can thaw the user (Gen 6+) |  |  |
| 44109 | FAIL | Upper Hand fails if the target has attempted to act even if previously successful |  |  |
| 44111 | FAIL | Gem boost is only applied once |  |  |
| 44112 | FAIL | Dynamax: Dynamaxed Pokemon are immune to Instruct |  |  |
| 44114 | FAIL | Octolock reduction is prevented by Clear Body, White Smoke and Full Metal Body 1/3 |  |  |
| 44116 | FAIL | Frostbite is healed if hit with a thawing move 1/5 |  |  |
| 44117 | FAIL | Schooling switches Level 20+ Wishiwashi's form when HP is over 25-percent before the first turn 1/4 |  |  |
| 44119 | FAIL | AI_FLAG_SEQUENCE_SWITCHING: AI will always switch after a KO in exactly party order 1/2 |  |  |
| 44125 | FAIL | Anticipation still triggers with Strong Winds active in Inverse Battle |  |  |
| 44126 | FAIL | Dynamax: Dynamaxed Pokemon lose their substitutes |  |  |
| 44134 | FAIL | Jaboca Berry causes the attacker to lose 1/8 of its max HP if a physical move was used 2/2 |  |  |
| 44135 | FAIL | Dynamax: Dynamaxed Pokemon cannot have their ability swapped to another Pokemon's |  |  |
| 44138 | FAIL | Revival Herb restores a fainted battler's HP fully |  |  |
| 44142 | FAIL | TIE_BREAK_SCORE with SCORE_TIE_CHOSEN can control AI move selection when scores are tied (Singles) 1/4 |  |  |
| 44144 | FAIL | Metronome Item counts charging turn of moves for its attacking turn 1/2 |  |  |
| 44148 | FAIL | Spread Move: Heals the correct amount from all Pokemon |  |  |
| 44149 | KNOWN_FAILING | AI uses Dynamax -- AI does not dynamax before using a utility move |  |  |
| 44150 | FAIL | AI_FLAG_SMART_TERA: AI might tera if it gets saved from a ko (2/2) |  |  |
| 44164 | FAIL | Pledge move combo fails if ally fails to act - Frozen Both Right Faster |  |  |
| 44167 | FAIL | Conversion 2 fails if the move used is of typeless damage (Gen 5+) |  |  |
| 44173 | FAIL | Corrosive Gas doesn't destroy the item of a Pokemon with the Sticky Hold ability |  |  |
| 44176 | FAIL | Red Card does not activate if stolen by a move 2/2 |  |  |
| 44177 | FAIL | Shield Dust does or does not block Sparkling Aria depending on number of targets hit 2/2 |  |  |
| 44178 | FAIL | Absorb does not play the draining message at full HP in Gen5+ 1/2 |  |  |
| 44181 | FAIL | If Salt Cure faints the target no status will be applied |  |  |
| 44187 | FAIL | Overheat drops Sp. Atk by 2 stages - doubles |  |  |
| 44188 | FAIL | Aegislash reverts to Shield Form upon switching out |  |  |
| 44189 | FAIL | Knock Off does knock off Orbs for Primal Reversion from Pokemon that don't actually use them |  |  |
| 44191 | FAIL | Rowap Berry causes the attacker to lose 1/8 of its max HP if a special move was used 1/2 |  |  |
| 44197 | FAIL | Defog doesn't remove Aurora Veil from the user's side 1/2 |  |  |
| 44206 | FAIL | Axe Kick deals damage half the hp to user if it fails |  |  |
| 44207 | FAIL | Charm lowers Attack by 2 stages 2/2 |  |  |
| 44211 | FAIL | Salac Berry raises the holder's Speed by one stage when HP drops to 1/4 or below 2/2 |  |  |
| 44213 | FAIL | Last Resort always fails if it's the only known move |  |  |
| 44217 | FAIL | Order Up increases a stat based on Tatsugiri's form 1/3 |  |  |
| 44221 | FAIL | Hail deals 1/16 damage per turn |  |  |
| 44225 | FAIL | Baton Pass used after Memento works correctly |  |  |
| 44227 | FAIL | Sandstorm deals 1/16 damage per turn |  |  |
| 44228 | FAIL | Triple Arrows can lower Defense and cause flinch at the time |  |  |
| 44230 | FAIL | AI chooses the safest option to faint the target, taking into account accuracy and move effect 1/5 |  |  |
| 44231 | FAIL | Anticipation still triggers with Strong Winds active |  |  |
| 44235 | FAIL | Belch can still be used after switching out |  |  |
| 44236 | FAIL | Sticky Hold prevents item theft |  |  |
| 44238 | FAIL | Thousand Arrows does neutral damage to non-grounded Flying types regardless of other typings 1/2 |  |  |
| 44245 | FAIL | Supersweet Syrup lowers evasion once per battle by one stage |  |  |
| 44251 | FAIL | Starting Toxic Spikes poison the opposing switch-in |  |  |
| 44254 | FAIL | Freeze is thawed by opponent's attack that can thaw the user if not Sheer Force affected (Gen 6+) |  |  |
| 44255 | FAIL | Coaching fails if all allies are is semi-invulnerable |  |  |
| 44259 | FAIL | Freeze is thawed by opponent's Fire-type attacks (Gen 3+) |  |  |
| 44260 | FAIL | Sword of Ruin's Defense reduction is not ignored by Mold Breaker 1/2 |  |  |
| 44262 | FAIL | Octolock Defense reduction is prevented by Big Pecks |  |  |
| 44265 | FAIL | Paralysis has a 25% chance of skipping the turn (2/2) |  |  |
| 44272 | FAIL | Primal Reversion happens for Kyogre only when holding Blue Orb 3/3 |  |  |
| 44275 | FAIL | Tera Shell makes all hits of multi-hit moves against Terapagos not very effective |  |  |
| 44278 | FAIL | Parting Shot: Switches if both stats are at minimum (Gen6) |  |  |
| 44279 | FAIL | Iron Defense raises Defense by 2 stages 2/2 |  |  |
| 44284 | FAIL | Defog does not lower evasiveness if target behind Substitute (Gen5+) 1/2 |  |  |
| 44286 | FAIL | Dynamax: G-Max Volt Crash paralyzes other opponent even if its target faints |  |  |
| 44290 | FAIL | Focus Punch doesn't use PP when losing focus (Gen 5+) 1/2 |  |  |
| 44294 | FAIL | Destiny Bond doesn't fail if used sequentially (Gen2-6) |  |  |
| 44296 | FAIL | Pursuit affected by Electrify fails against target with Volt Absorb |  |  |
| 44300 | FAIL | Growl lowers Attack by 1 stage 2/2 |  |  |
| 44303 | FAIL | Dynamax: Dynamax is reverted before switch out |  |  |
| 44308 | FAIL | Pursuit attacks a switching foe from fastest to slowest 1/2 |  |  |
| 44309 | FAIL | Future Sight will miss timing if target faints before it is about to get hit |  |  |
| 44312 | FAIL | Ability Shield doesn't reactivate an ability when receiving if user already had an Ability Shield |  |  |
| 44313 | FAIL | Dynamax: Max Moves don't execute effects on fainted battlers |  |  |
| 44316 | FAIL | Rage's volatile causes Attack to rise by 1 when hit by a damaging move |  |  |
| 44317 | FAIL | Grassy Terrain recovers 1/16th HP at end of turn |  |  |
| 44320 | FAIL | Dynamax: G-Max Finale heals allies by 1/6 of their health |  |  |
| 44322 | FAIL | Rage Fist base power is not increased if a substitute was hit |  |  |
| 44325 | FAIL | Grudge's effect doesn't trigger on indirect damage - Future Sight |  |  |
| 44327 | FAIL | Pledge move combo fails if ally fails to act - Paralyzed Both Right Faster |  |  |
| 44328 | FAIL | Dragon Cheer increases critical hit ratio by 2 on Dragon types 2/18 (2/2) |  |  |
| 44329 | FAIL | Liechi Berry raises Attack by one stage when HP drops to 1/2 or below if holder has Gluttony |  |  |
| 44331 | FAIL | Recoil if miss: Jump Kick's recoil happens after Spiky Shield damage and Pokemon can faint from either of these 1/3 |  |  |
| 44332 | FAIL | Belch can still be used after fainting |  |  |
| 44333 | FAIL | Pledge move combo fails if ally fails to act - Sleep Right 1/12 |  |  |
| 44334 | FAIL | Heal Bell cures inactive Soundproof Pokemon (Gen5+) 1/3 |  |  |
| 44335 | FAIL | Dynamax: G-Max Smite confuses both opponents |  |  |
| 44337 | FAIL | Berserk Gene does not confuse a Pokemon with Own Tempo but still raises attack sharply in a single battle 2/2 |  |  |
| 44340 | FAIL | Psych Up displays the correct battlers when used by the player |  |  |
| 44343 | FAIL | Psychic Terrain doesn't block priority moves that target all opponents |  |  |
| 44345 | FAIL | Dynamax: G-Max Stun Shock chooses statuses before considering immunities |  |  |
| 44346 | FAIL | Belly Drum maximizes the user's Attack stat 2/2 |  |  |
| 44349 | FAIL | Pursuit user faints to Life Orb and target still switches out |  |  |
| 44351 | FAIL | Attract fails when used by a genderless Pokémon |  |  |
| 44353 | FAIL | Volt Absorb activates on status moves |  |  |
| 44354 | FAIL | Roar fails if no replacements |  |  |
| 44355 | FAIL | Dynamax: G-Max Steelsurge sets up sharp steel |  |  |
| 44356 | FAIL | Pursuited mon correctly switches out after it got hit and activated ability Tangling Hair |  |  |
| 44357 | FAIL | Chilly Reception sets up snow and switches the user out |  |  |
| 44358 | FAIL | Water Bubble prevents burn from Will-o-Wisp |  |  |
| 44362 | ASSUMPTION_FAIL | Roost's effect is lifted after Grassy Terrain's healing |  |  |
| 44363 | ASSUMPTION_FAIL | Roost fails when user is at full HP |  |  |
| 44364 | FAIL | Dynamax: Max Starfall sets up Misty Terrain |  |  |
| 44365 | FAIL | Pursuit only attacks the first switching foe |  |  |
| 44370 | FAIL | (DYNAMAX) Dynamaxed Pokemon can have their base moves copied by Copycat |  |  |
| 44371 | FAIL | Dynamax: Max Hailstorm sets up hail |  |  |
| 44374 | FAIL | Dynamax: Heal Pulse heals based on a Pokemon's non-Dynamax HP 1/2 |  |  |
| 44378 | FAIL | Zen Mode switches Darmanitan's form when HP is half or less at the end of the turn 1/2 |  |  |
| 44381 | FAIL | Encore fails if target has active Shell Trap waiting |  |  |
| 44384 | FAIL | Dynamax: Dynamaxed Pokemon's Max Moves cannot be disabled |  |  |
| 44387 | FAIL | Type-enhancing items increase the base power of moves by 20% 36/36 |  |  |
| 44388 | FAIL | Dynamax: Dynamaxed Pokemon cannot be hit by OHKO moves |  |  |
| 44395 | FAIL | Eject Button will not activate under Substitute |  |  |
| 44397 | FAIL | Snore fails if not asleep 1/2 |  |  |
| 44398 | FAIL | Defog removes Stealth Rock and Sticky Web from user's side (Gen 6+) 1/3 |  |  |
| 44404 | FAIL | Destiny Bond fails if used sequentially (Gen7+) |  |  |
| 44407 | FAIL | Kee Berry raises the holder's Defense by one stage when hit by a physical move 2/2 |  |  |
| 44409 | FAIL | Refresh does not cure the user of Freeze (2/2) |  |  |
| 44413 | FAIL | Micle Berry raises the holder's accuracy by 1.2 when HP drops to 1/2 or below |  |  |
| 44414 | FAIL | AI will not revive a partner's party member with Revival Blessing 1/3 |  |  |
| 44416 | ASSUMPTION_FAIL | Roost does not suppress the ungrounded effect of Levitate |  |  |
| 44417 | ASSUMPTION_FAIL | Roost recovers 50% of the user's Max HP |  |  |
| 44423 | FAIL | Semi-invulnerable moves make the user semi-invulnerable turn 1, then strike turn 2 1/6 |  |  |
| 44426 | FAIL | Chloroblast does not cause the user to lose HP if there is no target |  |  |
| 44430 | FAIL | (TERA) Reflect Type copies a Stellar-type Pokemon's base type |  |  |
| 44431 | FAIL | Stockpile's Def and Sp. Def boost is lost after using Spit Up or Swallow 1/3 |  |  |
| 44433 | FAIL | Coaching bypasses Crafty Shield |  |  |
| 44435 | FAIL | Spicy Extract bypasses accuracy checks (1/?) |  |  |
| 44438 | FAIL | Conversion 2 fails if last hit by a Stellar-type move (Gen 1-4) |  |  |
| 44441 | FAIL | Shed Shell allows switching out even when trapped by Shadow Tag |  |  |
| 44442 | FAIL | Steel Roller will fail if there is no Terrain |  |  |
| 44447 | FAIL | Embargo doesn't block held item effects that affect effort values |  |  |
| 44451 | FAIL | (TERA) Terastallizing does not affect the power of non-STAB moves 1/2 |  |  |
| 44453 | FAIL | Defog is used on the correct side if opposing mon is behind a Substitute with Screen up 1/2 |  |  |
| 44457 | FAIL | Defog lowers evasiveness of target behind Substitute (Gen4) |  |  |
| 44460 | FAIL | Tailwind affects the partner on the same turn it's used (Gen8+) |  |  |
| 44461 | FAIL | Apicot Berry raises Sp. Def by one stage when HP drops to 1/4 or below if holder has Ripen |  |  |
| 44462 | FAIL | Stomping Tantrum will not deal double if it missed |  |  |
| 44463 | FAIL | Disable works even if the target's last move failed |  |  |
| 44465 | FAIL | (Z-MOVE) Genesis Supernova sets up psychic terrain |  |  |
| 44466 | FAIL | Teatime triggers Volt Absorb if it has been affected by Electrify or Plasma Fists, even when not holding a Berry 1/5 |  |  |
| 44467 | FAIL | Strength Sap will drain users HP if target has Liquid Ooze 1/2 |  |  |
| 44472 | ASSUMPTION_FAIL | AI_FLAG_SMART_SWITCHING: AI will stay in if Encore'd into super effective move |  |  |
| 44473 | FAIL | Substitute's HP cost can trigger a berry |  |  |
| 44483 | FAIL | Teatime triggers Motor Drive if it has been affected by Electrify or Plasma Fists, even when not holding a Berry 1/5 |  |  |
| 44489 | FAIL | White Herb restores stats after Attack was lowered by Intimidate while switching in |  |  |
| 44490 | FAIL | Focus Punch activation is based on Speed |  |  |
| 44497 | FAIL | Full Heal, Heal Powder and Local Specialties heal a battler from being confused 1/11 |  |  |
| 44500 | KNOWN_FAILING | Embargo blocks an affected Pokémon's trainer from using items |  |  |
| 44501 | FAIL | Ability Shield prevents Receiver/Power of Alchemy holder from copying ally's ability 1/2 |  |  |
| 44504 | FAIL | Max Mushrooms raises battler's Attack stat 1/2 |  |  |
| 44506 | FAIL | Upper Hand succeeds if the target is using a priority attacking move and causes it to flinch |  |  |
| 44508 | FAIL | AI will not try to switch for the same Pokémon for 2 spots in a double battle (Wonder Guard) (1/?) |  |  |
| 44509 | FAIL | Air Balloon pops when the holder is hit by a move that is not ground type |  |  |
| 44511 | FAIL | Sky Attack doesn't need to charge with Power Herb |  |  |
| 44512 | FAIL | AI revives the best fainted ally with Revival Blessing |  |  |
| 44513 | FAIL | Wish heals the user at the end of the next turn |  |  |
| 44518 | FAIL | (DYNAMAX) Dynamaxed Pokemon can be encored immediately after reverting |  |  |
| 44519 | FAIL | Razor Wind needs a charging turn |  |  |
| 44521 | FAIL | Grudge depletes all PP from a Max Move's base move |  |  |
| 44526 | FAIL | TIE_BREAK_TARGET correctly controls AI target selection when scores are tied for all values in enum TargetTieResolution 1/4 |  |  |
| 44527 | FAIL | Endure takes precedence over Focus Sash/Focus Band |  |  |
| 44535 | FAIL | Healing Wish causes the user to faint and heals the replacement's HP and status (singles) |  |  |
| 44540 | FAIL | Double Team raises Evasion by 1 stage (2/2) |  |  |
| 44543 | FAIL | Absorb fails if Heal Block applies |  |  |
| 44548 | FAIL | Covert Cloak does or does not block Sparkling Aria depending on number of targets hit 2/2 |  |  |
| 44549 | FAIL | After You makes the target move after user |  |  |
| 44551 | FAIL | Fling fails for Pokémon with Klutz ability (Gen5+) 1/3 |  |  |
| 44554 | FAIL | Hit Escape: U-turn switches the user out |  |  |
| 44555 | KNOWN_FAILING | AI uses Dynamax -- Max Moves are scored based on max move effects, not base effects |  |  |
| 44557 | FAIL | Opponent Pokemon can be further poisoned with Toxic spikes after a status healing hold effect was previously used 1/2 |  |  |
| 44559 | FAIL | Ally Switch works if ally used two-turn move like Dig |  |  |
| 44561 | FAIL | Psychic Noise heal block effect is blocked by partners Aroma Veil in doubles |  |  |
| 44564 | FAIL | Wake-Up Slap gets increased power against sleeping targets 1/2 |  |  |
| 44565 | FAIL | Aqua Ring's effect is passed by Baton Pass |  |  |
| 44566 | FAIL | Ingrain's effect is passed by Baton Pass |  |  |
| 44571 | FAIL | Assisted move triggers correct weakness berry 1/2 |  |  |
| 44576 | FAIL | Ingrain fails if already rooted |  |  |
| 44577 | FAIL | Sticky Syrup is removed when the user faints |  |  |
| 44580 | FAIL | Assurance doubles in power if the target has been damaged in the same turn - Life Orb |  |  |
| 44583 | FAIL | Spectral Thief steals opponents boost before attacking 1/2 |  |  |
| 44585 | FAIL | Throat Chop prevents the usage of sound moves |  |  |
| 44588 | FAIL | Attract ignores type immunity |  |  |
| 44593 | FAIL | Grassy Terrain heals the Pokémon on the field for the duration of the terrain, including last turn |  |  |
| 44595 | FAIL | Instruct causes the target to use its last used move again |  |  |
| 44597 | FAIL | Relic Song transformation is the last thing that happens after it hits |  |  |
| 44598 | FAIL | Eject Button has no chance to activate after Dragon Tail |  |  |
| 44599 | FAIL | Knock Off does knock off Ogerpon masks from Pokemon that aren't Ogerpon |  |  |
| 44600 | FAIL | Grudge depletes all PP of the move that fainted the target |  |  |
| 44601 | FAIL | Thunder, Ice and Fire Fang cause the opponent to flinch 10% of the time 1/3 (2/2) |  |  |
| 44607 | FAIL | Aromatherapy cures inactive Soundproof Pokemon regardless of config 1/2 |  |  |
| 44611 | FAIL | Soak/Magic Powder's type change is overwitten if the target changes form 1/2 |  |  |
| 44612 | FAIL | Make It Rain lowers special attack by one stage |  |  |
| 44613 | FAIL | Knock Off does not remove items through Substitute even if it breaks it |  |  |
| 44614 | FAIL | Belly Drum's HP cost doesn't trigger effects that trigger on damage taken |  |  |
| 44615 | FAIL | Hit Escape: U-turn triggers before Eject Pack |  |  |
| 44616 | FAIL | Eject Pack activates once intimidate mon switches in |  |  |
| 44619 | FAIL | Starting Stealth Rock damages the player's switch-in |  |  |
| 44620 | FAIL | Spicy Extract against Clear Amulet and Contrary raises Defense only |  |  |
| 44623 | FAIL | Dragon Tail switches target out and incoming mon has Levitate negated by Mold Breaker |  |  |
| 44624 | FAIL | Mind Blown causes the user to faint when below 1/2 of its Max HP in a double battle |  |  |
| 44626 | FAIL | Gem is consumed when it corresponds to the type of a move |  |  |
| 44629 | FAIL | Spikes fails after 3 layers |  |  |
| 44631 | FAIL | Meloetta returns to Aria form upon battle end after using Relic Song |  |  |
| 44635 | FAIL | Chilly Reception switches the user out even if it can't change the weather |  |  |
| 44637 | FAIL | Instruct message references the correct battlers |  |  |
| 44642 | FAIL | Causing a Forecast or Flower Gift Pokémon to faint should not cause a message 1/2 |  |  |
| 44644 | FAIL | Conversion 2's type change considers status moves (Gen 5+) |  |  |
| 44645 | FAIL | Ion Deluge makes Normal type moves Electric type |  |  |
| 44648 | FAIL | Mega Evolved Pokemon do not change abilities after fainting |  |  |
| 44649 | FAIL | Knock Off does not activate if the item was previously consumed |  |  |
| 44651 | FAIL | Mega Evolution doesn't affect turn order (Gen6) |  |  |
| 44656 | FAIL | Frostbite is healed when the user uses a thawing move 5/5 |  |  |
| 44660 | FAIL | Sticky Web lowers Speed by 1 in a double battle after Explosion fainting both mons |  |  |
| 44662 | FAIL | Court Change used by the player swaps Mist, Safeguard, Aurora Veil, Reflect, Light Screen, Tailwind |  |  |
| 44668 | FAIL | Last Resort works only when all of the known moves have been used - 2 moves |  |  |
| 44671 | FAIL | Red Card is consumed after dragged out replacement has its Speed lowered by Sticky Web |  |  |
| 44673 | FAIL | Defog removes Spikes from target's side 1/2 |  |  |
| 44674 | FAIL | Leech Seed doesn't affect Grass-type Pokémon (2/2) |  |  |
| 44681 | FAIL | Primal Reversion happens after a switch-in caused by Red Card |  |  |
| 44685 | FAIL | Strength Sap lowers Attack by 1 and restores HP based on target's Attack Stat and stat Change 1/12 |  |  |
| 44691 | FAIL | Primal Reversion happens for Groudon only when holding Red Orb 2/3 |  |  |
| 44692 | FAIL | Metronome's called powder move fails against Grass Types |  |  |
| 44699 | FAIL | Mirror Move's called powder move fails against Grass Types |  |  |
| 44703 | FAIL | Embargo doesn't stop an item flung at an affected target from activating |  |  |
| 44705 | FAIL | AI_FLAG_SMART_MON_CHOICES: Number of hits to KO calculation checks whether incoming damage is zero to avoid an infinite loop |  |  |
| 44707 | FAIL | Sleep Talk calls move and that move may be redirected by Storm Drain (2/2) |  |  |
| 44710 | FAIL | Octolock reduction is prevented by Clear Amulet |  |  |
| 44711 | FAIL | AI will not switch into a partner Pokémon in a 1v2 battle (all bad moves) 1/2 (1/?) |  |  |
| 44712 | FAIL | Solar Beam does not need a charging turn if Sun is up 1/2 |  |  |
| 44715 | FAIL | Sheer Cold can be endured by Focus Sash |  |  |
| 44716 | FAIL | Dynamax: G-Max Replenish recycles allies' berries 50% of the time, even if it faints the foe (2/2) |  |  |
| 44717 | FAIL | Burn Up fails if the user has Protean/Libero and is not a Fire-type |  |  |
| 44718 | FAIL | Spicy Extract Defense loss is prevented by Big Pecks |  |  |
| 44726 | FAIL | Fling applies special effects when throwing specific Items 1/6 |  |  |
| 44729 | FAIL | Parting Shot: Passes Substitute and switches the user out |  |  |
| 44730 | FAIL | Parting Shot: Switches if Contrary is at maximum stats (Gen6) |  |  |
| 44734 | FAIL | Dynamax: G-Max Chi Strike boosts allies' crit chance by 1 stage |  |  |
| 44738 | FAIL | Pledge move combo fails if ally fails to act - Flinch Both Left Faster |  |  |
| 44739 | FAIL | Pledge move combo fails if ally fails to act - Flinch Both Right Faster |  |  |
| 44742 | FAIL | Sticky Web setter has their speed lowered with Mirror Armor even after Ally Switch |  |  |
| 44743 | FAIL | Pledge move combo fails if ally fails to act - Sleep Left 1/12 |  |  |
| 44744 | FAIL | Pledge move combo fails if ally fails to act - Flinch Left 1/6 |  |  |
| 44745 | FAIL | Ion Duldge turns normal moves into electric for the remainder of the current turn |  |  |
| 44746 | FAIL | Stockpile temporarily raises Def and Sp. Def 1/2 |  |  |
| 44748 | FAIL | Psych Up displays the correct battlers when used by the opponent |  |  |
| 44749 | FAIL | Dynamax: G-Max Snooze makes only the target drowsy (2/2) |  |  |
| 44750 | FAIL | Eruption's damage is affected by the user's current HP 1/4 |  |  |
| 44755 | FAIL | Gravity cancels Fly and Sky Drop if they are in the air |  |  |
| 44756 | FAIL | Psychic Terrain doesn't block priority moves that target allies |  |  |
| 44758 | FAIL | Substitute creates a Substitute at the cost of 1/4 users maximum HP |  |  |
| 44761 | FAIL | Dynamax: G-Max Stun Shock paralyzes or poisons both opponents 1/2 |  |  |
| 44762 | FAIL | Tailwind doesn't affect the partner on the same turn it's used (Gen4-7) |  |  |
| 44763 | FAIL | Grudge's effect disappears if the user takes a new turn - Sleep |  |  |
| 44766 | FAIL | Dynamax: Max Lightning sets up Electric Terrain |  |  |
| 44768 | FAIL | Protect: Spiky Shield does 1/8 dmg of max hp of attackers making contact and may faint them 1/4 |  |  |
| 44769 | FAIL | Teatime does not affect Pokémon in the semi-invulnerable turn of a move |  |  |
| 44771 | FAIL | Dynamax: Max Geyser sets up heavy rain |  |  |
| 44772 | FAIL | Protect: Quick Guard can not fail on consecutive turns (Gen6+) 2/2 (50/50) |  |  |
| 44773 | FAIL | Pursuited mon correctly switches out after it got hit and activated ability Tangling Hair - Doubles |  |  |
| 44775 | FAIL | Flying-type Tera Blast does not have its priority boosted by Gale Wings |  |  |
| 44777 | FAIL | Dynamax: Sitrus Berries heal based on a Pokemon's non-Dynamax HP 1/2 |  |  |
| 44780 | FAIL | Axe Kick confuses the target |  |  |
| 44781 | FAIL | Pursuit only attacks a switching foe if foe is alive |  |  |
| 44782 | FAIL | Tera Starstorm changes from Normal-type to Stellar-type if used by Terapagos-Stellar |  |  |
| 44784 | FAIL | Pursuit attacks a switching foe but fails if user is asleep |  |  |
| 44785 | FAIL | Tidy Up raises Attack and Speed by one after clearing hazards on opposing field |  |  |
| 44788 | FAIL | Pursuit attacks a switching foe |  |  |
| 44789 | FAIL | Quash calculates correct turn order if only one Pokémon is left on the opposing side |  |  |
| 44790 | FAIL | Dynamax: Dynamaxed Pokemon cannot be hit by weight-based moves |  |  |
| 44793 | FAIL | Make It Rain lowers special attack by one stage if second target Protects |  |  |
| 44795 | FAIL | Rage builds Attack multiple times when hit multiple times |  |  |
| 44802 | FAIL | Hit Escape: U-turn does not switch the user out if Wimp Out activates |  |  |
| 44803 | FAIL | Mind Blown's recoil only happens once, regardless of number of affected targets |  |  |
| 44804 | FAIL | Recoil if miss: Supercell Slam causes recoil if it is absorbed |  |  |
| 44813 | FAIL | Reflect fails if already active |  |  |
| 44814 | FAIL | Dragon Tail switches the target with a random non-fainted replacement (2/2) |  |  |
| 44817 | FAIL | Uproar wakes up other pokemon on field |  |  |
| 44821 | ASSUMPTION_FAIL | Weakness berries decrease the base power of moves by half 34/36 |  |  |
| 44822 | FAIL | (TERA) Terastallizing into the Stellar-type provides a one-time 1.2x boost to non-STAB moves |  |  |
| 44824 | FAIL | Wish restores 50% of the recipient's HP when switching (Gen3-4) |  |  |
| 44829 | FAIL | Refresh does not cure sleep when used by Sleep Talk |  |  |
| 44830 | FAIL | Reflect Type fails if the target has no types |  |  |
| 44832 | FAIL | Howl raises user's Attack by 1 stage 2/2 |  |  |
| 44833 | FAIL | (TERA) Double Shock does not remove the user's Electric type while Terastallized, and changes STAB modifier depending on when it is used |  |  |
| 44840 | FAIL | Life Orb activates when users attack is succesful |  |  |
| 44842 | FAIL | Starting sharp steel damages the player's switch-in |  |  |
| 44843 | FAIL | (TERA) Roost does not remove the user's Flying type while Terastallized |  |  |
| 44844 | FAIL | Beak Blast doesn't burn when charging a two turn move 1/2 |  |  |
| 44846 | FAIL | (TERA) Terastallization's 60 BP floor does not apply to multi-hit moves 1/2 |  |  |
| 44848 | FAIL | Belly Drum fails if the user's Attack is already at +6, even with Contrary |  |  |
| 44849 | FAIL | SetStartingStatus can start Spikes on the opposing side 1/3 |  |  |
| 44850 | FAIL | Revival Blessing doesn't prevent revived battlers from losing their turn |  |  |
| 44854 | FAIL | Paralysis reduces Speed by 50% (Gen 7+) or 75% (Gen 1-6) 1/4 |  |  |
| 44855 | FAIL | Roar fails if replacements fainted |  |  |
| 44856 | FAIL | Red Card activates and is consumed but fails if the attacker is Dynamaxed |  |  |
| 44858 | ASSUMPTION_FAIL | Roost does not suppress the ungrounded effect of Air Balloon |  |  |
| 44859 | ASSUMPTION_FAIL | Roost suppresses the user's Flying-typing this turn, then restores it at the end of the turn |  |  |
| 44861 | FAIL | Knock Off does not prevent targets from receiving another item in Gen 5+ |  |  |
| 44863 | ASSUMPTION_FAIL | Roost does not suppress the ungrounded effect of Magnet Rise |  |  |
| 44864 | ASSUMPTION_FAIL | Roost, if used by a Flying/Flying type, treats the user as a Normal-type (or Typeless in Gen. 4) until the end of the turn |  |  |
| 44866 | FAIL | Red Card activates but fails if the attacker is rooted |  |  |
| 44873 | FAIL | Semi-invulnerable moves don't need to charge with Power Herb 1/6 |  |  |
| 44878 | FAIL | Stone Axe fails to set up hazards if user faints |  |  |
| 44879 | FAIL | Psychic Noise heal block effect is blocked by Aroma Veil |  |  |
| 44882 | FAIL | Red Card switches the target with a random non-battler, non-fainted replacement (2/2) |  |  |
| 44886 | FAIL | Cosmic Power increases the user's Defense and Sp. Defense by 1 stage each |  |  |
| 44889 | FAIL | Wake-Up Slap does not cure paralyzed pokemons behind substitutes or get increased power 1/2 |  |  |
| 44892 | FAIL | Ability Shield protects against Skill Swap 1/2 |  |  |
| 44894 | FAIL | Strength Sap fails if target is at -6 Atk |  |  |
| 44898 | FAIL | Defog removes Toxic Spikes from target's side 1/2 |  |  |
| 44900 | FAIL | Air Balloon prevents the holder from taking damage from ground type moves |  |  |
| 44903 | FAIL | Spite reduces the PP the last move used even while asleep |  |  |
| 44904 | FAIL | Salac Berry does not miss timing miss timing |  |  |
| 44905 | FAIL | Sticky Syrup is removed when the user switches out |  |  |
| 44907 | FAIL | Berserk Gene does not confuse when Safeguard is active |  |  |
| 44908 | FAIL | Stealth Rock damages the correct Pokémon when Eject Button is triggered |  |  |
| 44909 | FAIL | Misty Seed raises the holder's Sp. Defense on Misty Terrain 1/4 |  |  |
| 44913 | FAIL | White Herb restores stats when they're lowered |  |  |
| 44915 | FAIL | Shalour Sable heals a battler from any primary status 1/7 |  |  |
| 44921 | FAIL | Awakening heals a battler from being asleep |  |  |
| 44922 | FAIL | Covert Cloak does not block primary effects 1/4 |  |  |
| 44924 | FAIL | Taunt lasts for 3-5 turns (Gen 4) 1/3 (3/3) |  |  |
| 44926 | FAIL | Mind Blown does not cause the user to lose HP if there is no target |  |  |
| 44927 | FAIL | Full Restore restores a battler's HP and cures any primary status 1/7 |  |  |
| 44928 | FAIL | Stomping Tantrum will deal double damage if user was immune to previous move |  |  |
| 44931 | FAIL | Destiny Knot infatuates back when holder is targeted |  |  |
| 44932 | FAIL | Mind Blown causes the user to faint when below 1/2 of its Max HP |  |  |
| 44934 | FAIL | Max Honey restores a fainted battler's HP fully |  |  |
| 44935 | FAIL | Substitute's HP cost doesn't trigger effects that trigger on damage taken |  |  |
| 44941 | FAIL | Eject Pack is triggered by self-inflicting stat decreases |  |  |
| 44945 | FAIL | Encore has no effect if no previous move |  |  |
| 44946 | FAIL | Kee Berry raises the holder's Defense by two stages with Ripen when hit by a physical move |  |  |
| 44948 | FAIL | Endure does not prevent multiple hits and stat changes occur at the end of the turn |  |  |
| 44954 | FAIL | Double Shock user loses its Electric-type if enemy faints |  |  |
| 44960 | FAIL | Tera Starstorm targets both opponents in a double battle if used by Terapagos-Stellar |  |  |
| 44962 | FAIL | Red Card activates and overrides U-turn |  |  |
| 44966 | FAIL | Red Card activates after the last hit of a multi-hit move |  |  |
| 44967 | FAIL | Torment prevents consecutive move uses |  |  |
| 44968 | FAIL | Misty Terrain decreases power of Dragon-type moves by 50 percent 1/2 |  |  |
| 44975 | FAIL | Toxic Spikes inflicts poison on switch in |  |  |
| 44977 | FAIL | Scale Shot decreases defense and increases speed after killing opposing with less then 4 hits 1/2 |  |  |
| 44978 | FAIL | Sand Attack lowers Accuracy by 1 stage (2/2) |  |  |
| 44979 | FAIL | Focus Punch's initial message is not shown if the user selected a different move and was Encored into using Focus Punch |  |  |
| 44982 | FAIL | Shed Shell does not allow Teleport when trapped |  |  |
| 44987 | FAIL | Focus Punch activates when the user's Substitute is hit |  |  |
| 44991 | FAIL | Shell Bell does not activate on Future Sight if the original user is not on the field |  |  |
| 44994 | FAIL | Solar Beam and Solar Blade can be used instantly in Sunlight 1/4 |  |  |
| 44995 | FAIL | Petaya Berry raises Sp. Atk by one stage when HP drops to 1/4 or below if holder has Ripen |  |  |
| 45000 | FAIL | Electric Seed doesn't activate on existing Electric Terrain before user's ability changes the terrain |  |  |
| 45001 | FAIL | Hone Claws increases Attack and Accuracy by one stage each |  |  |
| 45005 | FAIL | Salt Cure is removed when the afflicted Pokémon is switched out |  |  |
| 45010 | FAIL | Overheat drops Sp. Atk by 2 stages - singles |  |  |
| 45013 | FAIL | Grudge's effect doesn't trigger on indirect damage - Sandstorm |  |  |
| 45017 | FAIL | Pledge move combo fails if ally fails to act - Sleep Both Right Wake Up 1/2 |  |  |
| 45018 | FAIL | Axe Kick deals damage half the hp to user if def battler protected |  |  |
| 45030 | FAIL | Full Heal heals a battler from any primary status 1/7 |  |  |
| 45031 | FAIL | Sparkling Aria cures burns from all Pokemon on the field and behind substitutes |  |  |
| 45032 | FAIL | Relic Song is prevented by Soundproof |  |  |
| 45035 | FAIL | Water and Fire Pledge create a rainbow on the user's side of the field for four turns |  |  |
| 45037 | FAIL | Max Mushrooms raises battler's Sp. Attack stat 1/2 |  |  |
| 45038 | FAIL | Salt Cure inflicts 1/8 of the target's maximum HP as damage per turn |  |  |
| 45039 | FAIL | X Attack sharply raises battler's Attack stat 1/2 |  |  |
| 45040 | FAIL | Spectral Thief can't steal opponent's boost if target is immune |  |  |
| 45041 | FAIL | Ceaseless Edge can set up to 3 layers of Spikes |  |  |
| 45046 | FAIL | Chilly Reception does not switch the user out if no replacements |  |  |
| 45051 | FAIL | Guard Spec. sets Mist effect on the battlers side |  |  |
| 45054 | FAIL | Make It Rain lowers special attack by one stage if it hits both targets |  |  |
| 45056 | FAIL | Conversion 2 randomly changes the type of the user to a type that resists the last move that hit the user (Gen 1-4) |  |  |
| 45057 | FAIL | Mind Blown causes the user & the target to faint when below 1/2 of its Max HP |  |  |
| 45058 | FAIL | Acupressure fails on the user if it targeted its ally but switched positions via Ally Switch |  |  |
| 45062 | FAIL | After You fails if the turn order remains the same after After You (Gen5-7) |  |  |
| 45065 | FAIL | Crafty Shield protects self and ally from opposing status moves 2/4 |  |  |
| 45066 | FAIL | Screech lowers Defense by 2 stages 2/2 |  |  |
| 45069 | FAIL | Defog removes Reflect and Light Screen from target's side 1/2 |  |  |
| 45070 | FAIL | Ally Switch changes the position of battlers |  |  |
| 45071 | FAIL | Aromatic Mist fails in Single Battles |  |  |
| 45074 | FAIL | Destiny Bond does not fail if used after failing (Gen7+) |  |  |
| 45077 | FAIL | Attract fails when used on a Pokémon of the same gender |  |  |
| 45078 | FAIL | Pursuit attacks a switching foe and switchin is correctly stored 1/4 |  |  |
| 45079 | FAIL | Aura Wheel raises Speed; fails if the user is not Morpeko 1/2 |  |  |
| 45085 | FAIL | Beak Blast burns all who make contact with the Pokémon |  |  |
| 45086 | FAIL | Dream Eater fails on awake targets |  |  |
| 45092 | FAIL | Belly Drum maximizes the user's Attack stat, even when below 0 2/2 |  |  |
| 45104 | FAIL | Dynamax: Super Fang uses a Pokemon's non-Dynamax HP 1/2 |  |  |
| 45106 | FAIL | Burn Up user loses its Fire-type |  |  |
| 45107 | FAIL | Reflect Type succeeds against a Terastallized target and copies its Tera type |  |  |
| 45108 | FAIL | Fillet Away sharply raises Attack, Sp. Atk, and Speed |  |  |
| 45109 | FAIL | Chloroblast causes the user to faint when below 1/2 of its Max HP |  |  |
| 45116 | FAIL | Dynamax: Dynamaxed Pokemon can have their ability changed or suppressed |  |  |
| 45118 | FAIL | Fling - Item is lost when target protects itself |  |  |
| 45119 | FAIL | Conversion 2's type change considers move types changed by Normalize and Electrify |  |  |
| 45121 | FAIL | Last Resort works with Sleep Talk |  |  |
| 45123 | FAIL | Retaliate works with passive damage 1/9 |  |  |
| 45126 | FAIL | Court Change used by the player swaps G-Max Vine Lash, G-Max Wildfire, G-Max Cannonade 1/3 |  |  |
| 45130 | FAIL | Defog removes Stealth Rock and Sticky Web from target's side 1/2 |  |  |
| 45133 | FAIL | Shell Trap does not activate if attacker's Sheer Force applied 1/2 |  |  |
| 45143 | FAIL | Snatch does not steal non-snatchable moves |  |  |
| 45144 | FAIL | Mimic copies the last move used even while asleep |  |  |
| 45146 | FAIL | (TERA) Stellar type's one-time boost factors in dynamically-typed moves |  |  |
| 45148 | FAIL | Mist's protection considers Contrary |  |  |
| 45149 | FAIL | (TERA) Roost does not remove Flying-type ground immunity when Terastallized into the Stellar type |  |  |
| 45152 | FAIL | (TERA) Synchronoise uses a Terastallized Pokemon's Tera Type |  |  |
| 45154 | FAIL | Tail Glow drastically raises Special Attack 2/2 |  |  |
| 45155 | FAIL | Embargo disables the effect of the Plate items on the move Judgment 1/2 |  |  |
| 45156 | FAIL | (TERA) Terastallization persists across switches |  |  |
| 45159 | FAIL | Spikes damage on subsequent switch ins |  |  |
| 45162 | FAIL | Double Shock user loses its Electric-type |  |  |
| 45172 | FAIL | Sticky Web has correct interactions with Mirror Armor - no one has their Speed lowered if the set upper switched 1/2 |  |  |
| 45173 | FAIL | Focus Punch activates when Disguise block a OHKO move (Gen8+) 1/2 |  |  |
| 45181 | FAIL | Air Balloon pops before it can be stolen by Thief |  |  |
| 45182 | FAIL | Liechi Berry raises Attack by one stage when HP drops to 1/4 or below if holder has Ripen |  |  |
| 45184 | FAIL | Parting Shot: Does not switch if Contrary is at maximum stats (Gen7+) |  |  |
| 45188 | FAIL | Grudge's effect disappears if the user takes a new turn - Flinching (2/2) |  |  |
| 45189 | FAIL | Berserk Gene does not confuse a Pokemon with Own Tempo but still raises attack sharply in a double battle 2/3 |  |  |
| 45190 | FAIL | Pledge move combo fails if ally fails to act - Frozen Both Left Faster |  |  |
| 45196 | FAIL | Hit Escape: U-turn switches the user out after Ice Face activates |  |  |
| 45198 | FAIL | Teatime causes other Pokemon to consume their Berry even if the user doesn't have a Berry as its held item, when not used by the Player |  |  |
| 45201 | FAIL | Telekinesis ends after 3 turns |  |  |
| 45202 | FAIL | Booster Energy's Protosynthesis boost is preserved when weather changes |  |  |
| 45203 | FAIL | Stellar-type Tera Blast has 100 BP and a one-time 1.2x boost |  |  |
| 45205 | FAIL | Eject Button activation will not trigger an attack from the incoming mon |  |  |
| 45209 | FAIL | Tidy Up raises Attack and Speed by one |  |  |
| 45219 | FAIL | Crafty Shield protects self and ally from Confide and Decorate 1/2 |  |  |
| 45223 | FAIL | Eject Button is not blocked by trapping abilities or moves |  |  |
| 45225 | FAIL | Jaboca Berry triggers before Bug Bite can steal it |  |  |
| 45227 | FAIL | Eject Pack does not cause the new Pokémon to lose HP due to it's held Life Orb |  |  |
| 45228 | FAIL | Leftovers recovers 1/16th HP at end of turn |  |  |
| 45230 | FAIL | Protect: Baneful Bunker poisons Pokémon for moves making contact 1/3 |  |  |
| 45232 | FAIL | Psychic Terrain doesn't block priority moves that target all battlers |  |  |
| 45235 | FAIL | Kings Rock holder will flinch the target 10% of the time (2/2) |  |  |
| 45238 | FAIL | Pursuit user gets forced out by Red Card and target still switches out |  |  |
| 45240 | FAIL | Lunar Dance causes the user to faint and heals the replacement's HP, PP and status (doubles) |  |  |
| 45242 | FAIL | Protective Pads protected moves still make direct contact 1/2 |  |  |
| 45243 | FAIL | Razor Wind doesn't need to charge with Power Herb |  |  |
| 45248 | FAIL | Pursuit affected by Electrify fails against immune target |  |  |
| 45249 | FAIL | Maranga Berry raises the holder's Sp. Def by one stage when hit by a special move 2/2 |  |  |
| 45251 | FAIL | Me First fails if target moves first |  |  |
| 45253 | FAIL | Red Card does not activate if stolen by Magician 2/2 |  |  |
| 45254 | FAIL | Pursuit attacks switching foes even if not targetting them (Gen 4+) |  |  |
| 45263 | FAIL | Starf Berry randomly raises the holder's Attack, Defense, Sp. Atk, Sp. Def, or Speed by two stages (5/5) |  |  |
| 45264 | FAIL | Rage Fist base power is not increased by a confusion hit |  |  |
| 45266 | FAIL | Plasma Fists turns normal moves into electric for the remainder of the current turn |  |  |
| 45267 | FAIL | Salac Berry raises Speed by one stage when HP drops to 1/2 or below if holder has Gluttony |  |  |
| 45268 | FAIL | Rapid Spin: Mortal Spin blows away Wrap, hazards and poisons foe |  |  |
| 45270 | FAIL | Order Up increases a stat based on Tatsugiri's form even if Tatsugiri fainted inside Dondozo 1/3 |  |  |
| 45272 | FAIL | Electric Seed raises the holder's Defense on Electric Terrain 1/4 |  |  |
| 45276 | FAIL | Old Gateu heals a battler from any primary status 1/7 |  |  |
| 45284 | FAIL | Syrup Bomb covers the foe in sticky syrup for 3 turns |  |  |
| 45288 | FAIL | Petaya Berry raises the holder's Sp. Atk by one stage when HP drops to 1/4 or below 2/2 |  |  |
| 45290 | FAIL | Parting Shot: Does not switch if both stats are at minimum (Gen7+) |  |  |
| 45292 | FAIL | X Sp. Def sharply raises battler's Sp. Defense stat 1/2 |  |  |
| 45301 | FAIL | White Herb restores stats after all hits of a multi hit move happened 1/2 |  |  |
| 45302 | FAIL | Pledge move combo fails if ally fails to act - Sleep Both Right Faster 1/3 |  |  |
| 45303 | FAIL | Revival Blessing fails if no party members are fainted |  |  |
| 45304 | FAIL | Lava Cookies heals a battler from any primary status 1/7 |  |  |
| 45305 | FAIL | Paralyze Heal heals a battler from being paralyzed |  |  |
| 45306 | FAIL | Full Restore heals a battler from any primary status 1/6 |  |  |
| 45307 | FAIL | Fire and Grass Pledge summons Sea Of Fire for four turns that damages the opponent |  |  |
| 45308 | FAIL | Roar switches the target with a random non-battler, non-fainted replacement (2/2) |  |  |
| 45325 | FAIL | Shed Tail fails if there are no usable Pokémon left |  |  |
| 45328 | FAIL | Ally Switch does not update Future Sight target position |  |  |
| 45332 | FAIL | Starting Stealth Rock damages the opposing switch-in |  |  |
| 45333 | FAIL | After You does nothing if the target has already moved |  |  |
| 45334 | FAIL | Shell Trap activates immediately after being hit on turn 3 and attacks both opponents |  |  |
| 45335 | FAIL | Ally Switch has no effect on partner's chosen move 1/4 |  |  |
| 45338 | FAIL | Protect: Multi-hit moves don't hit a protected target and fail only once 1/7 |  |  |
| 45341 | FAIL | Ally switch swaps sky drop targets if being used by partner |  |  |
| 45342 | FAIL | Meditate raises Attack by 1 stage 2/2 |  |  |
| 45343 | FAIL | Ally Switch fails in a single battle |  |  |
| 45347 | FAIL | Pursuit user mega evolves before attacking a switching foe and others mega evolve after switch |  |  |
| 45356 | FAIL | Pursuit ignores accuracy checks when attacking a switching target (1/?) |  |  |
| 45357 | FAIL | Attract bypasses Substitute |  |  |
| 45358 | FAIL | Beat Up lists each party member's name |  |  |
| 45363 | FAIL | Sleep Talk calls move and that move may be redirected by Lightning Rod (2/2) |  |  |
| 45366 | FAIL | Rapid Spin activates after Toxic Debris |  |  |
| 45375 | FAIL | Confide lowers Special Attack 2/2 |  |  |
| 45376 | FAIL | Belly Drum minimizes the user's Attack stat with Contrary 2/2 |  |  |
| 45378 | FAIL | Spicy Extract is prevented by target's ability if it's Attack stat is maxed out 1/2 |  |  |
| 45382 | FAIL | Captivate decreases the target's Sp. Attack if they're opposite gender from the user |  |  |
| 45383 | FAIL | Chilly Reception fails if it can't switch the user out or change the weather |  |  |
| 45386 | FAIL | Curse lowers Speed, raises Attack, and raises Defense when used by non-Ghost-types |  |  |
| 45388 | FAIL | (TERA) Conversion fails if used by a Terastallized Pokemon |  |  |
| 45391 | ASSUMPTION_FAIL | Roost does not undo other type-changing effects at the end of the turn |  |  |
| 45392 | FAIL | Conversion 2's type change considers the type of moves called by other moves |  |  |
| 45394 | FAIL | Cotton Guard raises Defense by 3 stages 2/2 |  |  |
| 45395 | FAIL | Rototiller doesn't affect Pokémon that are semi-invulnerable |  |  |
| 45397 | FAIL | Defog fails if target has minimum evasion stat change behind Substitute (Gen4) |  |  |
| 45402 | FAIL | Destiny Bond faints the opposing mon if it fainted from the attack |  |  |
| 45404 | FAIL | Shell Trap activates immediately after being hit on turn 1 and attacks both opponents |  |  |
| 45407 | FAIL | Court Change used by the opponent swaps Mist, Safeguard, Aurora Veil, Reflect, Light Screen, Tailwind |  |  |
| 45412 | FAIL | Sky Drop is cancelled if Gravity activated |  |  |
| 45414 | FAIL | Defog removes Spikes from user's side (Gen 6+) 1/3 |  |  |
| 45418 | FAIL | Synchronoise will fail for a typeless user even if a target is typeless |  |  |
| 45419 | FAIL | Doodle gives the target's ability to user and ally |  |  |
| 45420 | FAIL | Snatch steals from the correct target when multiple snatchable moves are used |  |  |
| 45424 | FAIL | Tailwind applies for 3 turns (Gen4) or 4 turns (Gen5+) 1/2 |  |  |
| 45427 | FAIL | Stellar-type Tera Blast activates a Stellar-type Pokemon's Weakness Policy |  |  |
| 45429 | FAIL | Embargo doesn't block held item effects that affect experience gain 1/2 |  |  |
| 45440 | FAIL | Echoed Voice's power increase is reset when no battler uses it successfully during a turn |  |  |
| 45446 | FAIL | Baton Pass passes Embargo's effect |  |  |
| 45448 | FAIL | Sticky Web is placed on the correct side after Explosion |  |  |
| 45451 | FAIL | Focus Punch activates only if not damaged 1/3 |  |  |
| 45454 | FAIL | Trick fails against Sticky Hold |  |  |
| 45458 | FAIL | Spit Up's power raises depending on Stockpile's count 1/3 |  |  |
| 45463 | FAIL | Stuff Cheeks can be used even if Magic Room is active |  |  |
| 45464 | FAIL | Electro Shot doesn't need to charge with Power Herb |  |  |
| 45465 | FAIL | Grudge does not deplete PP of a Z-Move |  |  |
| 45468 | FAIL | Skull Bash doesn't need to charge with Power Herb |  |  |
| 45471 | FAIL | Fling's secondary effects are blocked by Shield Dust 1/6 |  |  |
| 45475 | FAIL | Uproar status causes sleeping Pokémon to wake up during an attack (2/2) |  |  |
| 45476 | FAIL | Take Heart cures the user of all status conditions 1/6 |  |  |
| 45477 | FAIL | Hidden Power always triggers Counter instead of Mirror Coat (Gen 1-3) 1/16 |  |  |
| 45479 | FAIL | Foresight always hits unless the target is semi-invulnerable 2/2 |  |  |
| 45481 | FAIL | Toxic Spikes print normal poison for 1 layer |  |  |
| 45482 | FAIL | Magic Coat fails when the only slower battler is a fainted ally |  |  |
| 45483 | FAIL | Flame Burst Substitute |  |  |
| 45487 | FAIL | (TERA) Transform does not copy the target's Tera Type, and if the user is Terastallized it keeps its own Tera Type 1/2 |  |  |
| 45489 | FAIL | Steel Beam causes the user to faint when below 1/2 of its Max HP |  |  |
| 45490 | FAIL | Plasma Fists type-changing effect is applied after Normalize |  |  |
| 45491 | FAIL | Electro Shot needs a charging Turn |  |  |
| 45492 | FAIL | Metronome's called multi-hit move hits multiple times |  |  |
| 45494 | FAIL | Grudge's effect disappears if the user takes a new turn - Paralysis (2/2) |  |  |
| 45495 | FAIL | Mirror Move's called multi-hit move hits multiple times |  |  |
| 45496 | FAIL | Razor Wind successfully KOs both opponents |  |  |
| 45498 | FAIL | Smelling Salts does not cure paralyzed pokemons behind substitutes or get increased power 1/2 |  |  |
| 45499 | FAIL | Misty Terrain protects grounded battlers from non-volatile status conditions |  |  |
| 45503 | FAIL | Sticky Syrup speed reduction is prevented by Clear Amulet |  |  |
| 45504 | FAIL | Scale Shot decreases defense and increases speed after final hit |  |  |
| 45505 | FAIL | Wish restores 50% of the user's HP when not switching 1/2 |  |  |
| 45508 | FAIL | Nightmare damages sleeping targets at end of turn |  |  |
| 45509 | FAIL | Alluring Voice confuses the target if the target raised a stat this turn 2/2 |  |  |
| 45510 | FAIL | Hit Escape: U-turn switches the user out if Wimp Out fails to activate |  |  |
| 45512 | FAIL | Octolock will not decrease Defense and Sp. Def further then minus six |  |  |
| 45513 | FAIL | Plasma Fists type-changing effect does not override Pixilate |  |  |
| 45515 | FAIL | Dragon Tail switches the target with a random non-battler, non-fainted replacement (2/2) |  |  |
| 45516 | FAIL | Order Up is boosted by Sheer Force without removing the stat boosting effect |  |  |
| 45519 | FAIL | Psychic Noise blocks healing moves for 2 turns |  |  |
| 45521 | FAIL | Explosion boosted by Galvanize is correctly blocked by Volt Absorb |  |  |
| 45522 | FAIL | Explosion is blocked by Ability Damp |  |  |
| 45529 | FAIL | Starting Sticky Web lowers Speed on player's entry |  |  |
| 45530 | FAIL | Knock Off does knock off begin-battle form-change hold items from Pokemon that don't actually use them |  |  |
| 45531 | FAIL | Syrup Bomb is prevented by Bulletproof |  |  |
| 45532 | FAIL | Knock Off triggers Unburden |  |  |
| 45536 | FAIL | Last Resort works only when all of the known moves have been used - 3 moves |  |  |
| 45537 | FAIL | Freeze is thawed by opponent's Tri Attack 1/3 of the time (Gen 1-2) (3/3) |  |  |
| 45539 | FAIL | Confusion damage activates Focus Sash |  |  |
| 45543 | FAIL | Life Dew recovers 25% of hp for both user and partner |  |  |
| 45548 | FAIL | Psych Up ignores Spiky Shield and Baneful Bunker but fails against Crafty Shield 1/3 |  |  |
| 45549 | FAIL | Strong winds prevent Weakness Policy from activating on Flying-type weaknesses |  |  |
| 45551 | FAIL | Magic Coat reflection doesn't activate Protean/Libero 1/2 |  |  |
| 45552 | FAIL | Pursuited mon correctly switches out after it got hit and activated ability Tangling Hair - Mirror Armor |  |  |
| 45558 | FAIL | Steel Beam does not cause the user to lose HP if there is no target |  |  |
| 45559 | FAIL | Explosion causes the user to faint even if it has no effect |  |  |
| 45561 | FAIL | Pursuit attacks the second switching foe if the first faints from pursuit |  |  |
| 45564 | FAIL | Misty Terrain lasts for 5 turns |  |  |
| 45567 | FAIL | Starting Toxic Spikes poison the player's switch-in |  |  |
| 45568 | FAIL | Rage Fist base power is not increased if move had no affect |  |  |
| 45569 | FAIL | Multi Hit moves will not disrupt Destiny Bond flag 1/2 |  |  |
| 45581 | FAIL | Parting Shot: Mirror Armor switches the user even if reflected drops fail 1/4 |  |  |
| 45582 | FAIL | Reflect Type does not affect any of Arceus' forms 1/18 |  |  |
| 45587 | FAIL | Spicy Extract will fail if target is in a semi-invulnerability state |  |  |
| 45589 | FAIL | Stealth Rock damages the correct Pokémon when Eject Button is triggered in double battle |  |  |
| 45590 | FAIL | Rainbow doubles the chance of secondary move effects (2/2) |  |  |
| 45593 | FAIL | Sticky Web lowers Speed by 1 on switch-in |  |  |
| 45597 | FAIL | Stone Axe sets up hazards after hitting the target |  |  |
| 45599 | FAIL | Strength Sap lowers Attack by 1 and restores HP based on target's Attack Stat 1/2 |  |  |
| 45601 | FAIL | Protect: Recoil damage is not applied if target was protected 1/28 |  |  |
| 45603 | FAIL | Teatime causes the user to consume its Berry, ignoring HP requirements |  |  |
| 45604 | FAIL | Shell Trap does not activate if battler faints before being able to activate it |  |  |
| 45607 | FAIL | Tera Starstorm becomes a physical move if the user is Terapagos-Stellar, is Terastallized, and has a higher Attack stat |  |  |
| 45614 | FAIL | Snatch does not steal a move that was already snatched this turn (Gen 5+) |  |  |
| 45617 | FAIL | Toxic Spikes inflicts bad poison on switch in |  |  |
| 45625 | FAIL | Flame Burst doesn't crash, player to opponent |  |  |
| 45626 | FAIL | Headbutt flinches the target if attacker is faster 1/2 |  |  |
| 45630 | FAIL | Spin Out lowers speed by 2 stages |  |  |
| 45634 | FAIL | Mind Blown causes everyone to faint in a double battle |  |  |
| 45637 | FAIL | Tidy Up removes hazards and raises Stats |  |  |
| 45638 | FAIL | Triple Arrows may lower Defense by one stage 1/2 (2/2) |  |  |
| 45640 | FAIL | Transform fails on target behind substitute in Gen5+ 1/2 |  |  |
| 45648 | FAIL | Clanging Scales lowers defense by one stage if it hits both targets |  |  |
| 45649 | FAIL | Protect always works when used after flinching |  |  |
| 45651 | FAIL | Plasma Fists does not set up Ion Deluge if it does not connect |  |  |
| 45654 | FAIL | If Salt Cure faints the target, messages will be applied in the correct order |  |  |
| 45656 | FAIL | Sticky Syrup isn't applied again if the target is already covered |  |  |
| 45658 | FAIL | Powder moves are blocked by Grass-type Pokémon (Gen6+) 2/2 |  |  |
| 45659 | FAIL | Starting sharp steel damages the opposing switch-in |  |  |
| 45661 | FAIL | Protect: Wide Guard can not fail on consecutive turns (Gen6+) 2/2 (50/50) |  |  |
| 45662 | FAIL | Thunder Wave doesn't affect Electric types (Gen6+) 1/2 |  |  |
| 45666 | FAIL | Strong winds remove Flying-type weaknesses of all battlers 1/6 |  |  |

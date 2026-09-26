# Viridian Giovanni finale implementation evidence

This records the Wayfarer implementation of the [Viridian finale specification](../specs/frlg-kanto-viridian-finale.md), based on main `a646c8a7b1e8a18734b377bbb038ca93332484cb` (PR #132). It covers the FRLG Viridian Gym selected behind the HNS city doorway, Giovanni's local prerequisite/reward/departure sequence, and Blue's Saffron Dojo unlock at a committed first Indigo victory. The shared FRLG/Blue Indigo League is owned by PR #136; see the integration section below.

## Runtime and acceptance

The selected FRLG Gym retains its 20×24 maze, guide, eight ordinary Trainers, and five-member Giovanni party. The HNS city doorway enters it; all three Gym exits return to the city. The exit handler has an exact Gym-only fallback because the two side exit tiles have ordinary metatile behavior; the center has a south-arrow warp behavior. No `map.bin` was edited. Ordinary Trainer sight and persistent victories use Wayfarer's existing scaling and defeat state. Giovanni uses Gym scaling on the source-ordered Rhyhorn, Dugtrio, Nidoqueen, Nidoking, Rhyhorn party.

The final 21-case [r7 SkyEmu finale journey](../../e2e/src/journeys/wayfarer-viridian-finale.e2e.ts) passed on the frozen r7 E2E ROM. It walks from the HNS doorway to Giovanni and probes all 12 spinner lanes to their actual stop tiles. It enters and returns through each exit before and after departure, exercises the guide, early entry, and save/reload. Both Hideout→Silph and Silph→Hideout prerequisite orders keep Giovanni locked until both local resolutions; a battle loss leaves him retryable without reward. All eight ordinary Trainers are beaten once and remain defeated after reload, including a sight-started battle.

The E2E harness arranges local prerequisite and badge fixtures, starts the real battles, then injects win or loss outcomes to exercise native battle completion and field scripts; it does not play every turn. Native tests separately verify construction and scaling of Giovanni's full five-slot party.

An actual Giovanni win awards one Earth Badge and +4 Trainer Rating, gives the Earthquake TM, then removes Giovanni permanently. The journey checks that unrelated Rocket/Tower/Snorlax/Silph states stay independent, the Gym and guide remain usable, all three exits still work, and the separate hidden Macho Brace survives reload without aliasing Sprout Tower state. A full TM pocket leaves Giovanni present with the badge awarded; after reload and a free slot, the TM is delivered without another badge or rating award and only then does he depart. Prior ownership produces exactly one additional Earthquake TM, with no replay delivery. Viridian and Cinnabar Blue invitation paths are removed for Wayfarer while standalone HNS branches remain guarded.

The Dojo integration was exercised in both actual story orders on continuing saves: Giovanni→Indigo→Dojo and Indigo→Dojo→Giovanni. The first real Kanto-stage League commit reveals Blue; Giovanni, a fixture's projected regional clear, Champion battle start, and Champion loss do not. The existing eight-global-badge Indigo admission remained available without the Earth Badge. Dojo decline and loss leave Battle Points at 0; the authored repeatable battle grants 10 BP on the first win and 10 more on a repeat Dojo win, retained at 10 after reload and 20 after later Giovanni. The three changed League journeys passed on r6 after formatting. Twelve unchanged League journey cases had already passed in the earlier full run; its three test-driver failures were corrected and passed in the focused run.

The compact [evidence summary](assets/viridian-finale/evidence-summary.json) records frozen source/build hashes and test results; raw development logs and emulator captures are retained outside the repository.

## Validation identity and limits

The r7 source manifest was fingerprinted after both E2E and strict production builds. The immutable r7 E2E ROM SHA256 is `4f57fcda8804ee95cd6089bd4a417c9a2388b518e1cbe55d480b3886cc72ed66`; its symbols SHA256 is `9bdc74d15ae94cb40df286b0708d77d04298fd1270909b23ed6dce8c300b3b99`. The strict Wayfarer release build passed with 31,677,828 bytes used and 1,876,604 bytes spare, beyond the enforced 512 KiB reserve. The FRLG Gym layout binary SHA256 `65964beff473668a3b4490be8905b9f8882243931b2abbaf210ed8d119451406` matches the base; tracked `map.bin` diff is empty.

Native checks passed 54/54 on r6 source: Viridian 5, Dojo 1, League 26, trainer scaling 22. Static trainer checks passed 41+5 on r7, with 1,790 populated records, no structural failures, all 15 League rosters and 81 rating inventories. Existing Hideout, Silph, independence, Kanto traversal, and origin/League E2E regressions passed 54/54 on r6; the 23 Kanto traversal/admission cases affected by the exit-path change were rerun and passed on r7. The r6 native and Dojo evidence remains applicable because the only r6→r7 runtime edit was the exact Gym exit handler plus its focused static bounds test; party, state, and Dojo inputs did not change. The r7 finale journey separately validates the changed navigation live.

Reproduction commands (from the repository root, with serialized map-version builds):

```sh
make -C game -j8 CXX=g++ BUILD=wayfarer UNUSED_ERROR=1 DEPRECATED_ERROR=1 release
make -C game trainer-party-scaling-test trainer-party-scaling-audit
SKYEMU_ROM=/path/to/pokemon-wayfarer-e2e.gba SKYEMU_SYMS=/path/to/pokemon-wayfarer-e2e.sym pnpm --dir e2e exec wa test src/journeys/wayfarer-viridian-finale.e2e.ts
```

Independent review of the source and test logs found no actionable gameplay or source findings; the reviewer did not run a separate emulator session. Draft-PR CI is tracked separately and is not asserted here.

## Shared League integration

The shared FRLG/Blue Indigo League landed in PR #136 (save format version 10). This change rebased onto it and uses its `HasCommittedFirstIndigoVictory()` seam, which reads `CIRCUIT_CLEAR_INDIGO` (bit 2 of `wayfarerHoenn.leagueFlags`). The interim stand-in and its duplicate native cases were removed; #136 owns committed-victory coverage.

Dojo visibility lives outside the League code. `SyncBlueDojoVisibility()` in `wayfarer_blue_dojo.c` runs on Dojo transition and projects `FLAG_HIDE_DOJO_BLUE` from the seam alone, so League commit and #136's Hall of Fame rollback never touch the flag. `test/viridian_finale.c` asserts bit 2 directly as a storage tripwire and drives a real Indigo commit followed by `RollbackIndigoHallOfFameCommit()` to confirm Blue stays hidden.

The Viridian Gym Trainer IDs follow #136's Indigo range: Indigo keeps 1795-1799 and the Viridian Gym uses 1800-1808, so `TRAINERS_COUNT_WAYFARER` is 1809. The Gym's defeat flags stay at trainer-flag slots 703-711.

The Dojo steps in `wayfarer-league-circuit.e2e.ts` run on #136's Indigo helpers (`admitIndigo`, `finishRoomChain`, `clears.indigo`). The losing-Champion case plays the first four rooms and loses to Blue, because the Champion room validates the earlier defeats.

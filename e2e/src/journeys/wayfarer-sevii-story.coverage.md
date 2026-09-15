# Sevii story emulator coverage

`wayfarer-sevii-story.e2e.ts` drives the restored map actors and scripts in the
SkyEmu test ROM. It covers:

- Bill's full-Key-Items failure, successful retry, and persisted Meteorite;
- Lostelle's local start without Bill, refusal of an out-of-order biker,
  objective-battle loss/blackout/re-entry, all four ordered wins, persisted
  biker completion, and Hypno loss/reload/retry through rescue; and
- Moltres at TR 54 and 55, loss/re-entry, knockout, and persisted resolution.

The existing `wayfarer-sevii-exploration.e2e.ts` remains the emulator coverage
for the ungated ferry, the frozen 135-map catalog, and unchanged Birth Island
and Navel Rock predicates.

The current E2E ABI cannot directly force a scripted wild battle's catch or run
outcome, set a Wayfarer Sevii custom-bank flag as a fixture, set lead friendship,
or fill the Berry pocket. Consequently this journey does not claim Moltres or
Hypno catch/run coverage, arbitrary mid-objective gem/Warehouse permutations,
Selphy request seeding, the friendship-qualified Egg offer, or Iapapa capacity
failure. Those cases require mechanics tests and generated objective/transaction
audits unless the E2E ABI is deliberately extended in a later change.

# Phone rematches

PRDs: [Hoenn phone rematches](../prds/hoenn-phone-rematches.md),
[Sevii and Kanto coast phone rematches](../prds/sevii-coast-phone-rematches.md),
[Daily world slots](../prds/daily-world-slots.md#phone-numbers-and-rematches)
Implemented: No

## Scope

One phone system for every route trainer with a number: HNS's Johto and Kanto
trainers as today, Emerald's 64 Hoenn rematch trainers, and 41 FRLG families
on Sevii and the Kanto coast. It covers registration, the contact list, calls,
readiness, rematch teams, the save state and every new or converted call
text. `IS_WAYFARER` only.

## Contacts

| Group | Trainers | Rematch teams | Source of call texts |
| --- | ---: | --- | --- |
| HNS (Johto, Kanto) | as today | HNS rematch table | `game/data/text/match_call_hns.inc`, unchanged apart from the Battle text rule below |
| Hoenn | 64 | Emerald's rematch table rows | Converted Emerald Match Call texts, [below](#hoenn-call-texts) |
| Sevii | 32 families | Sevii rematch registry | New, [below](#sevii-and-coast-call-texts) |
| Kanto coast | 9 families | Coast rematch registry | New, [below](#sevii-and-coast-call-texts) |

Pairs share one contact and battle together.

## Rematch table and save state

- **Hoenn rows.** Wayfarer compiles HNS's `gRematchTable`, which reuses
  Emerald's entry names for different people (Emerald's Rose slot holds Joey).
  Hoenn trainers get new entries and never reuse an HNS index. Indexes 28–63
  are empty and take the first 36; the table grows past its 90 entries for the
  other 28. Entries are appended, never inserted, so existing indexes keep
  their meaning. `REMATCH_SPECIAL_TRAINER_START` moves past the new rows, and
  every loop keyed on it is checked.
- **Sevii and coast families** stay in their own registries. Their stage and
  ready bits already live in Wayfarer's Sevii and coast save state, not in the
  shared rematch array.
- **Registration flags.** One per contact. HNS and Hoenn contacts use the
  registered-flag block, which moves from `0x310` to a free range large enough
  for the grown table, because the current block ends flush against
  `HNS_EXTENDED_CONTENT_START`. Sevii and coast contacts use 41 new Sevii bank
  flags.
- **Rematch progress.** `trainerRematches` grows from 100 to the table size.
  This changes the SaveBlock1 layout; Wayfarer has no released saves, so no
  migration is needed (`AGENTS.md`).
- **Contact list.** `matchCallEntries` grows from 99 to cover every contact plus
  the special headers, with a bounds check. The list also reads the Sevii and
  coast registries.

## Getting a number

After you win against a trainer who has a number and isn't registered, they
offer it with a yes/no prompt. Yes registers them and shows the "Registered …
in the POKéGEAR" message. No leaves them unregistered, and they ask again
after your next win against them, home or wandering. This replaces the
automatic registration in `RegisterTrainerInMatchCall` for Wayfarer.

## Readiness and teams

- **Readiness.** HNS makes a trainer ready after 255 steps on their home map.
  Wayfarer counts steps anywhere in the trainer's region instead: every 255
  steps in a region, one registered, already-fought trainer of that region
  who isn't ready becomes ready (chosen by `roll`, as in
  [daily world slots](daily-world-slots.md#deterministic-draws)), and calls.
- **Teams.** A ready trainer's next battle uses their next team: the next
  unbeaten tier in the rematch table, or the next stage in the Sevii or coast
  registry. After the last, they keep it. Winning clears readiness. Levels
  come from [regular trainer scaling](trainer-party-scaling.md); the HNS
  badge-level ceiling no longer applies in Wayfarer.
- **Placement.** A ready trainer always has a spot that day
  ([daily world slots](daily-world-slots.md#who-stands-where-today)). If their
  group has no rotating spot, the call waits for the next day.

## Calls

- **Rate.** The total call rate stays HNS's: the random-call timer and chance
  are unchanged, and the caller is chosen among registered contacts. More
  contacts means each one calls less often.
- **Text selection.** A ready trainer's call uses their Battle text; otherwise
  a random General text, as in HNS.
- **Today's place.** Before a call, `{STR_VAR_2}` is set to the map name from
  `WhereIsTrainerToday`. Every Battle text names it; HNS's existing Battle
  texts gain a line that does.
- **Text format.** Every contact has `General1`–`General3` and `Battle`, in
  the `sHnsMatchCallTrainers` shape. A contact without texts fails the build
  rather than showing an empty box.

## Vs. Seeker

Retired. Its giver isn't compiled into Wayfarer. Remove its readiness paths for
the Sevii and coast registries and stop it from suppressing step-based
rematches; the item itself is left unobtainable.

## Validation

- Every Hoenn, Sevii and coast contact registers, appears in the list, calls
  with each of its texts, and rematches through every team in order.
- Existing HNS contacts keep their indexes, texts, gifts and team order.
- No flag overlaps, and the contact list never overflows.
- Every Battle text contains `{STR_VAR_2}`, and every text fits the text box.

## Hoenn call texts

Pending: converted texts are being prepared.

## Sevii and coast call texts

Pending: new texts are being written.

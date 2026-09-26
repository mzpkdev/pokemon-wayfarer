# Wayfarer interregional League circuit

Implemented: Outdated

Proposed successor: [Seeded Trainer Circuit](seeded-trainer-circuit.md) replaces
the fixed order and lineups with a seeded itinerary and rated trainer pool.
That draft separates confirmed requirements from proposed defaults. This document
remains the approved baseline until the successor is adopted.

The current runtime still treats Kanto and Johto as separate League clears at
the shared Indigo venue. The approved design below replaces that model with one
shared Indigo League and a separate Sevii Masters Challenge.

## Intent

Give Wayfarer's open world a clear long-term arc without requiring the player
to finish one region before exploring another. Badges earned in Kanto, Johto,
and Hoenn advance the same career. Three fixed challenge destinations turn each
eight-badge interval into a journey while the player remains free to choose
their regions, available Gyms, party, and timing.

## Circuit

The Trainer Card presents this itinerary:

| Career tier | Qualification | Destination |
| --- | --- | --- |
| Tier 1 | At least 8 total badges | Shared Kanto-Johto Indigo League |
| Tier 2 | Indigo cleared and at least 16 total badges | Sevii Masters Challenge |
| Tier 3 | Masters cleared and all 24 badges | Hoenn League |
| Mastery | Indigo, Masters, and Hoenn cleared | Red at Mt. Silver |

Any sanctioned Gym Badge from Kanto, Johto, or Hoenn counts. Badge origin does
not change its value for circuit qualification. Badges are never spent or
reset, and challenge participation never gates earning another badge. A player
may collect all twenty-four badges before attempting any circuit stage, then
clear the three stages consecutively.

The existing Johto opening remains the sole new-game start for the current
release. Kanto and Hoenn openings are separately scoped future features and do
not change the circuit.

### Indigo League

Indigo Plateau is the single League authority shared by Kanto and Johto. It
uses the FRLG room chain and this authored Tier 1 lineup:

1. Lorelei
2. Bruno
3. Agatha
4. Lance
5. Blue

Blue is the current Indigo Champion. Clearing Indigo grants the one Champion
title recognized by both Kanto and Johto. The clear performs one Hall of Fame
registration and one first-clear reward; it is not two circuit clears merely
because two regions recognize the title.

For the current Johto-origin release, Blue uses the existing FRLG first-clear
Blastoise roster. Do not infer his team from the Johto starter. A future Kanto
opening may select among the three FRLG starter-dependent variants from that
opening's own origin state.

The future FRLG Kanto story port owns Blue's origin-dependent rival dialogue
and his authored party selection. Giovanni owns the initial Earth Badge through
that port's finale. Neither decision changes Indigo's circuit position.
The [Viridian finale specification](../specs/frlg-kanto-viridian-finale.md)
also makes Blue's Saffron Dojo battle available after the first committed
Indigo victory, independently of Giovanni's badge.

### Sevii Masters Challenge

Tier 2 is an invitational challenge rather than another regional League. It
uses Seven Island's existing two-room battle house as the public entrance. The
elderly former Trainer operates the house and reveals its hidden basement when
the player qualifies. The basement leads into the existing HNS room chain:

1. Will
2. Koga
3. Bruno
4. Karen
5. Lance

Lance is the final invited Master and may be presented as a former Champion.
Blue remains the current Indigo Champion. Repeated opponents are intentional:
the Masters Challenge is a stronger invitational appearance, not a simultaneous
second Elite Four appointment.

The HNS Hall of Fame room becomes the Masters Gallery. Clearing it records a
Masters result and the Tier 2 first-clear reward. It does not grant a regional
Champion title, set a regional game-clear result, award a Champion Ribbon, or
perform Hall of Fame registration.

Sevii travel and local adventures remain independent. The Masters Challenge
does not require Celio's repair, Lorelei's Icefall adventure, Trainer Tower,
the Rainbow Pass, a local quest, or another Sevii completion fact.

### Hoenn League and Red

The Hoenn League remains Tier 3 and retains its Emerald rooms, roster, Hall of
Fame, regional Champion state, and cleanup. Its first clear completes the
circuit and runs the full completion credits.

Red remains at the Mt. Silver summit. Defeating every Gym is already implied by
Hoenn admission; Red becomes available after the player has also cleared
Indigo, Masters, and Hoenn. Red is a final mastery encounter, not a fourth
League stage, and awards no additional circuit Trainer Rating.

## State and titles

Circuit stages are not regions. Store and dispatch the three first-clear facts
as Indigo, Masters, and Hoenn identities. In particular, do not represent the
Masters Challenge as `REGION_JOHTO` merely because it uses HNS rooms and
opponents. Its maps remain Sevii/Kanto for ordinary map, Pokédex, healing,
blackout, and story systems.

Indigo clear is the shared source of Kanto and Johto Champion recognition.
Implementation may project that result into the existing regional helpers, but
qualification and rewards must read one canonical Indigo clear so the result
cannot count twice. Masters has dedicated circuit state and no Champion
meaning. Hoenn retains its isolated regional Champion and game-clear state.

All first-clear writes are atomic with their ceremony and reward handoff.
Repeated completion callbacks cannot duplicate rewards, change another
regional state, or advance the circuit twice.

## Difficulty and replays

Each stage has one authored roster for its fixed circuit tier. League scaling
captures Trainer Rating at admission and applies that snapshot for the entire
run. It changes effective levels only; authored species, party sizes, moves,
items, abilities, AI, and battle order remain intact.

Cleared stages remain replayable. A replay captures a fresh rating snapshot and
uses the same authored tier roster, but it grants no additional circuit state,
Trainer Rating, Hall of Fame registration, Champion Ribbon, credits, or
first-clear unlock. Ordinary battle rewards remain unchanged.

## Trainer Rating

The existing badge contribution remains unchanged. First clears contribute:

| Clear | Trainer Rating |
| --- | ---: |
| Indigo League | +8 |
| Sevii Masters Challenge | +8 |
| Hoenn League | +8 |

The total circuit contribution is +24. Individual opponent victories, losses,
replays, repeated ceremony callbacks, Red, and Hall of Fame revisits add
nothing.

Taking each stage at its minimum badge requirement produces:

| Progress | Trainer Rating |
| --- | ---: |
| New game | 0 |
| 4 badges | 16 |
| 8 badges | 40 |
| Indigo cleared | 48 |
| 16 badges and Indigo cleared | 56 |
| Masters cleared | 64 |
| 24 badges and both earlier stages cleared | 72 |
| Hoenn cleared | 80 |

Collecting all badges first remains valid:

| With 24 badges | Trainer Rating | Soft level cap |
| --- | ---: | ---: |
| No circuit clears | 56 | 62 |
| Indigo cleared | 64 | 78 |
| Indigo and Masters cleared | 72 | 89 |
| Circuit complete | 80 | 100 |

Trainer Rating remains a high-water mark. It drives the existing wild,
ordinary-Trainer, Gym, soft-cap, and obedience consumers. Circuit opponents use
their saved run-entry snapshot through League scaling.

## Travel and discovery

All twenty-four badges remain obtainable before any circuit clear. Regional
stories may retain coherent local prerequisites, but an initial Gym challenge
must not require a League or Masters clear.

Every eligible stage must be reachable from every possible threshold-badge
location without requiring another badge or the clear being pursued. The
existing Johto opening and S.S. Aqua maiden voyage provide the route into the
interregional transport network. Numbered Sevii islands remain independently
available from Vermilion.

The Trainer Card is the complete circuit overview. It shows the badge total out
of twenty-four and every stage's locked, available, or cleared state. The game
does not interrupt play with qualification announcements. The Seven Island
caretaker may state the immediate Masters admission requirement, and each
venue may state its own unmet requirement, but NPCs do not repeat the entire
itinerary.

Before Tier 2 qualification, the battle-house box remains closed. Once the
player has 16 badges and an Indigo clear, the caretaker reveals the basement.
The existing second room serves as an antechamber to the HNS challenge chain.
A loss or voluntary exit returns the player to the battle house; a successful
ceremony returns the player with access to Seven Island's harbor and the wider
travel network.

## Presentation

Indigo uses its FRLG Hall of Fame and Champion presentation but does not run the
full completion credits. The Masters Gallery recognizes the winner without
calling them a regional Champion. Hoenn's first clear runs the full completion
credits and leaves the completed itinerary available on the Trainer Card.

Challenge names shown to the player are `Indigo League`, `Sevii Masters`, and
`Hoenn League`. The Seven Island building may be called `Masters House`; its
underground competition space is the `Masters Hall`.

## Constraints

- Preserve Kanto, Johto, and Hoenn badge identity and storage. Global badge
  count remains a derived total from the twenty-four regional badges.
- Replace the old separate Kanto/Johto League-clear assumption wherever it
  controls circuit order, Trainer Rating, status text, scaling, Hall of Fame,
  transport cleanup, or regional postgame checks.
- Preserve Sevii's independent ferry access, stories, ordinary Trainers,
  encounters, services, and Trainer Tower.
- Reuse the registered Seven Island battle house and HNS League rooms. Do not
  add the excluded `SevenIsland_UnusedHouse` merely to create another entrance.
- Standalone Emerald, FireRed, LeafGreen, and HNS behavior remains unchanged.
- Prerelease save compatibility is not required.
- The finished implementation must fit the standard 32 MiB ROM and active
  reserve policy.

## Playtesting

- Reach Indigo with mixed sets of eight badges, clear the FRLG roster, verify
  one Hall of Fame registration, shared Kanto/Johto Champion recognition, +8
  Rating, wider-world return, and no full credits.
- Reach Seven Island before qualification and verify that the ordinary island,
  battle house, harbor, local stories, and Trainer Tower remain available while
  the basement challenge stays closed.
- Qualify with 16 badges and Indigo clear, traverse the battle house into every
  HNS room, clear the authored roster, and verify the Masters Gallery, +8
  Rating, dedicated clear, and absence of Champion/Hall of Fame side effects.
- Reach Hoenn after all 24 badges and both prior clears, complete its native
  League and Hall of Fame, verify Rating 80 and full credits, then reach Red at
  Mt. Silver.
- Exercise early and postponed attempts, all-badges-first consecutive clears,
  save/load in every room, losses at every opponent, voluntary exits, invalid
  run recovery, repeated ceremony callbacks, and replays of all three stages.
- Confirm replays use fresh scaling snapshots without another clear reward or
  first-clear side effect.
- Confirm no opening, badge award, Hall of Fame return, or unrelated Sevii
  interaction produces an automatic circuit announcement.

## References

- [Technical specification](../specs/wayfarer-interregional-league-circuit.md)
- [League scaling](league-scaling.md)
- [Runtime foundation](../specs/wayfarer-runtime-foundation.md)
- [Sevii exploration](sevii-exploration-port.md)
- [Sevii Trainer Tower](sevii-trainer-tower.md)
- [FRLG Kanto story port](frlg-kanto-story-on-hns-maps.md)

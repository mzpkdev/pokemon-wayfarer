# Trainer scaling inventory and balance review

This report checks authored source records and projected parties. It does not establish playable balance or report emulator playtesting.

| Policy | Populated IDs |
| --- | ---: |
| EXCLUDED | 181 |
| GYM_LEADER | 30 |
| GYM_MEMBER | 104 |
| ORDINARY | 1176 |

| Region | Populated IDs |
| --- | ---: |
| Alola | 20 |
| HNS unplaced | 224 |
| Hoenn | 854 |
| Johto | 235 |
| Kanto | 150 |
| Sinjoh | 8 |

HNS unplaced means a compiled roster lacks a direct regional map reference. The manifest preserves that uncertainty instead of assigning a region from its Trainer name.

All eligible slots use generated level-up moves unless listed in the reviewed exception manifest. Bosses, rivals, facility heads, the eight Kimono story opponents, and four Sinjoh Plate gates retain authored construction.

## Level and reward inputs

Baseline anchors: 0:7, 4:8, 8:10, 16:15, 30:22, 40:34, 55:52, 65:72, 80:92. Each slot adds its bounded authored-level adjustment; Gym members add two levels. Final levels stop at 100. The player soft cap does not clamp opponents.

Battle XP reads effective species and levels; prize money retains authored party levels and class multiplier.

The audit evaluated 439,668 slot, Rating, and learnset-mode combinations with 0 structural failures.

## Highest early parties

| Trainer | Party size | Rating 0 species and levels |
| --- | ---: | --- |
| TRAINER_ALICE_HNS | 3 | VILEPLUME 17, EKANS 17, VILEPLUME 17 |
| TRAINER_BARRY_HNS | 2 | TENTACOOL 17, NIDOKING 17 |
| TRAINER_BRIANA_HNS | 3 | GOLDEEN 17, MARILL 17, GOLDEEN 17 |
| TRAINER_CINDY_HNS | 2 | TENTACOOL 17, NIDOQUEEN 17 |
| TRAINER_CODY_HNS | 4 | HORSEA 16, DRATINI 16, PSYDUCK 17, DRATINI 16 |
| TRAINER_DIANA_HNS | 3 | PSYDUCK 17, CLOYSTER 17, CORSOLA 17 |
| TRAINER_DORIS_HNS | 3 | HOOTHOOT 17, SLOWPOKE 17, NATU 17 |
| TRAINER_FRANKLIN_HNS | 1 | KADABRA 17 |
| TRAINER_GREGORY_HNS | 2 | PIKACHU 17, FLAAFFY 17 |
| TRAINER_HORTON_HNS | 4 | VOLTORB 17, VOLTORB 17, VOLTORB 17, VOLTORB 17 |

## Largest early parties

| Trainer | Party size | Rating 0 species and levels |
| --- | ---: | --- |
| TRAINER_COLTON | 6 | SKITTY 10, SKITTY 13, SKITTY 14, SKITTY 8, SKITTY 12, DELCATTY 14 |
| TRAINER_GABRIELLE_1 | 6 | SKITTY 11, POOCHYENA 11, ZIGZAGOON 11, LOTAD 11, SEEDOT 11, TAILLOW 11 |
| TRAINER_GABRIELLE_2 | 6 | SKITTY 12, POOCHYENA 12, ZIGZAGOON 12, LOTAD 12, SEEDOT 12, TAILLOW 12 |
| TRAINER_GABRIELLE_3 | 6 | SKITTY 13, POOCHYENA 13, ZIGZAGOON 13, LOTAD 13, SEEDOT 13, TAILLOW 13 |
| TRAINER_GABRIELLE_4 | 6 | DELCATTY 13, POOCHYENA 13, ZIGZAGOON 13, LOTAD 13, SEEDOT 13, TAILLOW 13 |
| TRAINER_GABRIELLE_5 | 6 | DELCATTY 13, POOCHYENA 13, ZIGZAGOON 13, LUDICOLO 13, SHIFTRY 13, TAILLOW 13 |
| TRAINER_ISAAC_1 | 6 | WHISMUR 8, ZIGZAGOON 8, ARON 8, POOCHYENA 8, TAILLOW 8, MAKUHITA 8 |
| TRAINER_ISAAC_2 | 6 | WHISMUR 10, ZIGZAGOON 10, ARON 10, POOCHYENA 10, TAILLOW 10, MAKUHITA 10 |
| TRAINER_ISAAC_3 | 6 | WHISMUR 11, ZIGZAGOON 11, ARON 11, POOCHYENA 11, TAILLOW 11, MAKUHITA 11 |
| TRAINER_ISAAC_4 | 6 | WHISMUR 12, ZIGZAGOON 12, ARON 12, POOCHYENA 12, TAILLOW 12, MAKUHITA 12 |

## Species and retained-field observations

- Custom authored moves replaced: 2473 distinct projected outcomes. Exact affected IDs, variants, slots, and Rating intervals are indexed in inventory.json.
- Authored ability requires fallback: 1 distinct projected outcomes. Exact affected IDs, variants, slots, and Rating intervals are indexed in inventory.json.
- Authored gender requires adjustment: 0 distinct projected outcomes. Exact affected IDs, variants, slots, and Rating intervals are indexed in inventory.json.
- Incompatible gimmick suppressed: 0 distinct projected outcomes. Exact affected IDs, variants, slots, and Rating intervals are indexed in inventory.json.
- Held item retained after species reversal: 141 distinct projected outcomes. Exact affected IDs, variants, slots, and Rating intervals are indexed in inventory.json.
- Opponent above player soft cap: 853 distinct projected outcomes. Exact affected IDs, variants, slots, and Rating intervals are indexed in inventory.json.
- High-stat species without numeric predecessor: 1225 distinct projected outcomes. Exact affected IDs, variants, slots, and Rating intervals are indexed in inventory.json.

## Representative parties

The machine-readable inventory contains full normal and legacy moves, abilities, XP species inputs, authored money inputs, and every pool candidate for representative parties at Ratings 0, 4, 8, 16, 30, 40, 55, 63, 65, 68, 76, and 80. Its slot and projection tables cover all other eligible source slots.

## Remaining validation

Playtest early, middle, and late careers in every included region, including Gym doubles, utility-heavy learnsets, powerful species without numeric predecessors, and the largest early parties above. Passing structural validation is not a balance approval.

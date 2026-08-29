# Authored under-level ordinary wild encounters

Related PRD: [Trainer Rating wild encounter scaling](../prds/trainer-rating-wild-encounter-scaling.md)

Related spec: [Trainer Rating wild encounter scaling](../specs/trainer-rating-wild-encounter-scaling.md)

## Question

Which ordinary random encounter tables deliberately place an evolved Pokemon below its numeric level-evolution threshold, and which of those entries should affect Trainer Rating scaling policy?

## Scope

This audit covers active runtime slots in `gWildMonHeaders` for Emerald, FireRed, LeafGreen, and HNS. It includes ordinary land, water, Rock Smash, and fishing profiles.

It excludes fixed and scripted encounters, hidden DexNav entries, roamers, outbreaks, Feebas, Battle Pike, and Battle Pyramid. The scripted Red Gyarados is excluded. The separate ordinary Lake of Rage water table is included.

Duplicate slots and profiles are grouped where that does not hide a build, method, authored range, or evolution threshold. HNS compact rows may combine maps with identical species, range, and method signatures; their locations are summarized in the location cell. Day and night variants with identical results are counted once.

## Current scaling rule

The runtime preserves a species that the source table already authors below its numeric evolution threshold. It only resolves an evolved species to its predecessor when both conditions are true:

1. The rolled authored level met or exceeded the numeric evolution threshold.
2. Trainer Rating projection lowered the result below that threshold.

For example, an authored level 25 Gyarados projected to level 18 becomes Magikarp. An authored level 15 Gyarados remains Gyarados because the source table already made that exception.

Some ranged slots cross an evolution threshold. In those slots, a low authored roll remains the evolved species while a higher authored roll can resolve to its predecessor if scaling pushes the result below the threshold.

## Summary

| Build | Evolved species | Active raw slots | Consolidated groups |
| --- | ---: | ---: | ---: |
| Emerald | 10 | 17 | 17 |
| FRLG combined | 16 | 233 | 227 |
| HNS | 26 | 153 | 82 |

FireRed contributes 111 raw slots and 109 consolidated groups. LeafGreen contributes 122 raw slots and 118 consolidated groups. HNS has 54 compact cross-map species, range, and method signatures after equivalent profiles are combined; preserving maps as separate groups produces the 82 shown in the table. No qualifying HNS Kanto entries were found.

## Emerald findings

| Evolved species | Predecessor and threshold | Authored source | Location and method |
| --- | --- | --- | --- |
| Silcoon | Wurmple at 7 | 5 | Petalburg Woods land |
| Cascoon | Wurmple at 7 | 5 | Petalburg Woods land |
| Tentacruel | Tentacool at 30 | 20-25 and 25-30; the latter crosses 30 | Abandoned Ship Rooms B1F and Hidden Floor Super Rod |
| Electrode | Voltorb at 30 | 26 | New Mauville land |
| Magneton | Magnemite at 30 | 26 | New Mauville land |
| Wailord | Wailmer at 40 | 25-30 | Route 129 water |
| Seaking | Goldeen at 33 | 25-30 and 30-35; the latter crosses 33 | Safari Zone Southwest and Northwest Super Rod |
| Dodrio | Doduo at 31 | 29 | Safari Zone Northwest land |
| Golduck | Psyduck at 33 | 30-35 and 25-40; both cross 33 | Safari Zone Northwest water |
| Gyarados | Magikarp at 20 | 5-45; crosses 20 | Sootopolis City Super Rod |

## FireRed and LeafGreen findings

| Evolved species | Predecessor and threshold | Build and authored source | Locations and methods |
| --- | --- | --- | --- |
| Metapod | Caterpie at 7 | FireRed 5; LeafGreen 4, 5, 6 | Viridian Forest land |
| Kakuna | Weedle at 7 | FireRed 4, 5, 6; LeafGreen 5 | Viridian Forest land |
| Gyarados | Magikarp at 20 | Both builds 15-25; crosses 20 | Super Rod on S.S. Anne exterior; Routes 4, 6, 10-13, and 19-25; Kanto city waters; Cerulean Cave; Seafoam; and many Sevii routes and islands |
| Weezing | Koffing at 35 | FireRed 32 and 34 | Pokemon Mansion land |
| Muk | Grimer at 38 | LeafGreen 32 and 34 | Pokemon Mansion land |
| Seaking | Goldeen at 33 | Both builds 20-30 | Safari Zone, Berry Forest, and Fuchsia Super Rod |
| Dragonair | Dratini at 30 | Both builds 25-35; crosses 30 | Safari Zone fishing |
| Poliwhirl | Poliwag at 25 | Both builds 20-30; crosses 25 | Cerulean Cave, Icefall entrance, Cape Brink, Ruin Valley, Routes 6, 22, 23, and 25, Viridian City, and Four Island fishing |
| Slowbro | Slowpoke at 37 | LeafGreen land 32-35, water 35-40 crossing 37, and fishing 25-35 | Seafoam, Berry Forest, Cape Brink, and Cinnabar |
| Golduck | Psyduck at 33 | FireRed 32 | Seafoam land |
| Dewgong | Seel at 34 | Both builds 32 | Seafoam B3F land |
| Haunter | Gastly at 25 | Both builds 20, 21, and 23 | Pokemon Tower land |
| Rapidash | Ponyta at 40 | Both builds 37 and 39 | Kindle Road and Mt. Ember land |
| Magcargo | Slugma at 38 | Both builds 25-35 and 35-45; the latter crosses 38 | Mt. Ember Ruby Path Rock Smash |
| Seadra | Horsea at 32 | FireRed 25-35; crosses 32 | Coastal Kanto and Sevii fishing profiles |
| Kingler | Krabby at 28 | LeafGreen 25-35; crosses 28 | Coastal Kanto and Sevii fishing profiles |

The repeated coastal and Sevii Gyarados, Seadra, and Kingler profiles account for most of the FRLG raw slot count.

## HNS findings

| Evolved species | Predecessor and threshold | Authored source | Locations and methods |
| --- | --- | --- | --- |
| Arbok | Ekans at 22 | 19-26; crosses 22 | Safari Top Mid land |
| Ariados | Spinarak at 22 | 18 | Route 37 night land |
| Banette | Shuppet at 37 | 30-45; crosses 37 | New Sinjoh land |
| Dewgong | Seel at 34 | 27-29 | Whirl Islands land |
| Dragonair | Dratini at 30 | 10 and 19-26 | Route 45 Super Rod and Safari Low Right Super Rod |
| Fearow | Spearow at 20 | 19-26; crosses 20 | Safari Low Left land |
| Golduck | Psyduck at 33 | 15-19, 19-26, 20-24, and 28-29 | Ilex water, Route 35 water, Safari Low Right land, and Whirl Islands land |
| Graveler | Geodude at 25 | 19-26 and 23-25; both cross 25 | Safari Low Mid and Cliff Edge Cave land |
| Gyarados | Magikarp at 20 | 10-13 and 19-26; the latter crosses 20 | Ordinary Lake of Rage water slot 4 and Safari Top Right water |
| Haunter | Gastly at 25 | 19-26; crosses 25 | Safari Top Mid land |
| Kingler | Krabby at 28 | 19-26 and 23-25 | Several Safari land and Super Rod profiles, plus Cliff Edge Cave land |
| Ledian | Ledyba at 18 | 16-18; crosses 18 | Route 37 day land |
| Machoke | Machop at 28 | 23-25 | Cliff Edge Cave land |
| Magmar | Magby at 30 | 16, 17, and 18 | Burned Tower B1F land |
| Magneton | Magnemite at 30 | 19-26 | Safari Low Mid land |
| Noctowl | Hoothoot at 20 | 19 | Route 37 night land |
| Poliwhirl | Poliwag at 25 | 19-26 crossing 25, and 20-24 | Safari water profiles; Routes 30 and 31; Violet City; and Ecruteak water |
| Quagsire | Wooper at 20 | 15-24 and 19-26; both cross 20 | Ruins outside, Union Cave water, and Safari Low Right land and Super Rod |
| Raticate | Rattata at 20 | 15, 19, and 19-26 crossing 20 | Burned Tower, Route 38, and Safari Top Right land |
| Sandslash | Sandshrew at 22 | 19-26; crosses 22 | Safari Low Left land |
| Seadra | Horsea at 32 | 19-26 and 30 | Safari Top Right water and Super Rod, and Dragon's Den Super Rod |
| Seaking | Goldeen at 33 | 19-29 | Safari Super Rod, Mt. Mortar water, Tohjo Falls water and fishing, and Route 42 water |
| Slowbro | Slowpoke at 37 | 10-24 and 26-28 | Slowpoke Well water and Whirl Islands land |
| Tentacruel | Tentacool at 30 | 20-28 | Routes 27, 32, 34, and 40; Union Cave; Whirl Islands; Cherrygrove; Olivine; Olivine Port; and Cianwood water, plus Whirl Islands land |
| Venomoth | Venonat at 31 | 23 | Route 43 night land |
| Weepinbell | Bellsprout at 21 | 19-26; crosses 21 | Safari Top Mid land |

Only the first five authored Lake of Rage water rows are active under the shared water weight table. This makes the level 10-13 Gyarados row the only active ordinary Gyarados slot there. The fixed Red Gyarados encounter remains outside this audit and outside Trainer Rating scaling.

## Interpretation

These entries are not malformed data. Pokemon games frequently use under-level evolved forms to give a place or encounter method a distinct population. Viridian Forest Metapod and Kakuna, Pokemon Tower Haunter, Seafoam Dewgong, Route 45 Dragonair, Slowpoke Well Slowbro, and ordinary fishing Gyarados are examples of that pattern.

Applying a universal natural-evolution rule would change all of these source-authored exceptions. It would also change mixed ranged slots differently depending on the rolled authored level. The current implementation instead preserves authored exceptions while preventing Trainer Rating projection from creating new ones.

## Separate species-floor review

The current explicit ordinary-wild floors are:

| Species | Floor |
| --- | ---: |
| Kecleon | 20 |
| Skarmory | 18 |

These are single-stage exceptions rather than under-level evolved forms, so they do not appear in the lists above.

The cross-region strength audit found three additional candidates:

| Species | Candidate floor | Evidence at Rating 10 |
| --- | ---: | --- |
| Aerodactyl | 20 | HNS Sinjoh Ruins Temple authors level 5, projected to 9 |
| Heracross | 15 | HNS Azalea Rock Smash authors level 10, projected to 11 |
| Bagon | 25 | Emerald Meteor Falls authors 25, 30, and 35, projected to 19, 21, and 24 |

Anorith and Lileep also appear at level 5 in Sinjoh Ruins Temple and project to 9 at Rating 10. They are first-stage fossils rather than strong evolved forms. The audit recommends leaving them unchanged unless the design adopts a general minimum for fossil species.

No Gyarados floor is recommended under the current policy. A simple floor would reject an entire ranged slot as soon as one possible Gyarados outcome fell below 20, including rolls that could otherwise resolve correctly from authored levels at or above 20. Preserving the source-authored exception is consistent with Dragonair, Haunter, Dewgong, Seaking, and the other entries in this research.

## Decision options

### Preserve authored exceptions

Keep the current rule. Trainer Rating may reverse a species only when projection creates a new under-level outcome. This preserves every ordinary source table listed above.

### Enforce natural evolution levels globally

Resolve every evolved species below its numeric level threshold, even when the source table authored it that way. This would affect common and intentional encounters across every build and would require a policy for ranged slots.

### Add curated species floors

Keep authored evolved forms, but add floors for exceptional single-stage or first-stage species whose strength or identity needs a minimum level. Aerodactyl, Heracross, and Bagon are the current candidates.

## Sources

- [Ordinary encounter data](../../game/src/data/wild_encounters.json)
- [Species floor configuration](../../game/src/data/wild_encounter_species.json)
- [Runtime encounter scaling](../../game/src/wild_encounter.c)
- [Encounter generator and audit](../../game/tools/wild_encounters/wild_encounters_to_header.py)

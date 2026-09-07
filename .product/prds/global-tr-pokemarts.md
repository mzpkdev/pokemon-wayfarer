# Poké Marts across the three regions

Status: Design for implementation; runtime changes are not part of this document PR.
Specification: [Global TR Poké Marts](../specs/global-tr-pokemarts.md)

## Intent

Keep ordinary Poké Marts useful throughout a Wayfarer journey across Kanto,
Johto and Hoenn. A player returning to an early town should find supplies
appropriate to their current Trainer Rating (TR). Local stock should also
make towns recognizable and give players reasons to visit different shops.

Today, shared Johto/Kanto clerk scripts select inventory using their legacy
badge flags, while Hoenn marts largely use fixed lists and local story gates.
Neither provides consistent resupply across the combined adventure.

## Design

Every ordinary mart location carries a shared essentials catalog based on
global TR, plus a permanent local selection. Lilycove's two existing 2F
counters supply that catalog together: balls and status cures on the left,
HP medicine and repels on the right. All other converted locations have a
full-service counter. Stock expands when the player reaches a
threshold, keeps all earlier items, and remains available at every higher TR.
The source of the player's badges or League clears does not affect stock at
the same TR.

| Minimum TR | Additions to essentials |
| --- | --- |
| 0 | Poké Ball, Potion, Antidote, Paralyze Heal, Awakening, Burn Heal, Ice Heal, Repel, Escape Rope |
| 4 | Great Ball, Super Potion |
| 16 | Super Repel |
| 30 | Ultra Ball, Hyper Potion, Revive |
| 40 | Full Heal, Max Repel |
| 55 | Max Potion, Full Restore |

These are the selected v1 thresholds. TR 55 through 80 shares the complete
catalog. An ordinary mart never sells a later-tier essential through its
local selection. Department-store essentials use these same thresholds,
including both halves of Lilycove's split catalog.

For example, a TR 4 visitor to Cherrygrove and Mossdeep can buy Great Balls
and Super Potions in both towns. Their specialty stock differs. On returning
to Cherrygrove at TR 55, the player can buy Full Restores there too.

### Local character

Ordinary towns carry two to four specialty items, with deliberate overlap
between similar places and different combinations per town. Preserve authored
identities such as Mossdeep's Net/Dive Balls, Verdanturf's Nest Balls and Fluffy
Tails, and Rustboro's Timer/Repeat Balls. The specification supplies the complete
map-by-map selection for all three regions, including inherited battle items
and mail that remain available in addition to these signatures.

Situational balls, battle consumables and souvenirs provide local identity.
Supplies needed for routine travel, healing and ordinary catching belong in
the shared catalog. Do not turn a frequently consumed essential into a
single-town exclusive. Exclusivity means a distinctive shop role or limited
distribution, not removing existing gifts, pickups or other acquisition routes.

### Specialist shops and department stores

Goldenrod's flower shop retains mints and Mirror Herb. Mahogany retains its
evolution-item shop. Kurt keeps his Apricorn exchange. Mt. Moon keeps its
gift-shop stock. Herb shops, TM counters, vitamin sellers, décor sellers,
vending machines, BP shops and other special currencies retain their authored
services, prices and availability conditions.

Celadon, Goldenrod and Lilycove combine the shared essentials progression with
their existing specialist floors. General-counter evolution stones and other
nonessential goods remain available. A department store earns its appeal
through breadth and convenience.

Cash marts at League venues, Trainer Hill and the Battle Frontier also receive
TR essentials where the specification identifies an existing resupply clerk.
Specialist counters in the same building keep their separate services.

### Story and access

Regional story flags do not restrict ordinary mart essentials or the permanent
local signatures. Remove the opening-stock restrictions at Cherrygrove and
Oldale and the inventory branches at Petalburg and Rustboro for Wayfarer.
Retain the associated story events, rewards and progression state.

Mahogany must offer essentials even while its specialist shop is closed. Use
an existing, permanently available town NPC for the separate resupply service;
the specification names the binding. The evolution shop can retain its story
gate. Specialist access gates, including the flower shop's specific badge
requirement, remain authored exceptions.

This feature upgrades existing resupply locations and covers Mahogany's known
closure. It does not add a mart to every settlement: towns without an existing
mart, such as the starting towns, remain as authored.

### Prices and availability

Stock is unlimited and predictable, with no daily rotation. Keep existing
item prices and purchase rules. Do not introduce regional price differences,
discounts, new reward flags or new story-bonus catalogs in v1. Existing shop
events and engine pricing modifiers, including PokéNews sales and challenge
multipliers, remain intact at converted and specialist services alike.

Normal play does not gain universal purchasable PP recovery. The Pokémon Center
challenge retains an expanded PP-recovery selection, mapped to TR in the
specification. Existing restrictions on using items still apply.

## Boundaries

Apply this feature only to the combined Wayfarer build, including HNS Kanto,
Johto and imported Hoenn. Standalone HNS, Emerald, FireRed and LeafGreen retain
their behavior. FRLG Kanto/Sevii map files are not Wayfarer's Kanto coverage.
Alola, Sinjoh and other outlying specialist services are outside the ordinary
three-region catalog unless explicitly listed in the specification.

Preserve iconic Pokémon species, authored Trainer teams, hand-authored
movesets, learnsets, encounters, Gym and League battles. This feature also
does not change TR calculation, item effects, field-move requirements, bag
capacity, travel routes or facility battle rules. Do not edit map tiles.

## Balance

The essentials thresholds follow the existing TR progression: stronger
healing and balls become broadly available as the party's level range rises.
All essential supplies unlock by TR 55, so late-game travel remains convenient.
Reaching a late town early no longer bypasses those tiers at an ordinary mart;
existing specialist remedies and pickups remain intentional alternatives.

Keep earlier items because lower cost and different efficiency can still make
them useful. Permanent local selections favor situational tools; v1 does not
add purchasable rare held items, PP berries or new common-berry vendors.

## Presentation

Retain the familiar Buy/Sell/Quit interaction and item descriptions. List balls,
medicine and travel essentials in consistent order, then local goods. Keep
department-store specialist counters separate. No new unlock announcements or
TR display are required. Update stock dialogue that becomes false, including
claims that ordinary Poké Balls are sold out until a story event.

## Playtesting

- Revisit an early town at TR 55 and buy the same essentials available in a late town.
- Enter each region at low TR and check that ordinary counters follow the same tiers.
- Check whether each town's signature goods are useful and recognizable without
  requiring frequent travel solely for everyday supplies.
- Try the new tier thresholds with typical party HP and available money; evaluate
  the effects of early access to situational balls and inherited specialist remedies.
- Check Mahogany resupply before, during and after its Rocket story, and challenge
  play without Pokémon Center healing.

## References

- [Implementation specification and complete stock tables](../specs/global-tr-pokemarts.md)
- [Trainer Rating and party progression](../specs/trainer-rating-party-progression.md)
- [Interregional League circuit](../specs/wayfarer-interregional-league-circuit.md)

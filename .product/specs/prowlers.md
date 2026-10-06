# Prowlers

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: v0 approved. It holds only the agreed rules. The reward lists cover
every included generation: Generations I, II, III, V, VI, VII and VIII, with
their regional forms. Generation IV's list waits for a future Sinnoh, apart
from the rows Sinjoh uses.

## Scope

This spec defines prowlers and lists the reward Pokémon for every included
generation, with their regional forms. It covers what a reward Pokémon is,
where prowlers appear, and how their level is set.

It doesn't cover:

- reach bonuses or dungeon difficulty;
- which map tables include which reward Pokémon, which belongs to each
  region's encounter spec;
- legendaries and mythicals, which roam or live in lairs.

## Behavior

### Reward Pokémon

A reward Pokémon is a rare species worth remembering when you find it. Each
generation has a single list. A species earns its place for one or more of
these reasons:

| Reason | Meaning |
| --- | --- |
| **Power** | Strong for a wild Pokémon, judged by its base stat total |
| **Rarity** | Rare in its original games |
| **Prestige** | A starter, a pseudo-legendary line, or a fan favourite with a special status |
| **Utility** | Valued for what it does rather than its stats |
| **Evolution** | A final stage reached by a non-level evolution (item, trade, friendship, move or location), which stage caps keep to Outlands and deeper floors |

Evolution rewards appear as later stages wherever stage caps allow them, in
Outlands and on a dungeon's [deeper floors](reach-assignments.md#dungeon-floors). Every other reward is a
prowler.

### Prowlers

A prowler is a reward Pokémon met in the wild. Prowlers are usually rare finds
in a table's rare slots, but some lead or hold common slots, such as Dratini at
Dragon's Den, Lunatone and Solrock at Meteor Falls, Torkoal on Fiery Path and
Clamperl at Sootopolis. Each prowler has a **temperament**, and each reach allows
certain temperaments:

| Temperament | What it is | Allowed in |
| --- | --- | --- |
| **Harmless** | Rare, but no threat | Every reach, including Roads |
| **Fierce** | Strong, and above the area's level early on | Wilds, Outlands and dungeons |
| **Dangerous** | The top tier | Outlands and a dungeon's deeper floors |

A reward's temperament follows from why it is a reward:

- **Harmless:** rarity, utility or prestige with modest stats, including
  starters.
- **Fierce:** power.
- **Dangerous:** a base stat total of 515 or more, a pseudo-legendary line, or
  another top-tier line such as Axew's and Larvesta's.

What each reach feels like:

- **Road:** you might spot something rare, but nothing that can hurt you.
- **Wilds:** where prowlers turn fierce.
- **Outlands:** the most dangerous finds.
- **Dungeons:** temperaments follow depth. Harmless and fierce near the
  entrance. Dangerous prowlers appear only in Outlands and on a dungeon's
  [deeper floors](reach-assignments.md#dungeon-floors): every step after the
  first, with single-floor and flat dungeons counting as deeper.

Each prowler has a **minimum level**, set by its temperament and base stat
total in [Wild level scaling](wild-level-scaling.md#prowler-minimum-levels).
It appears at the higher of the area's wild level and its minimum level, in
any slot, common or rare. Early in the game that makes a fierce or
dangerous prowler a threat and a tricky catch. Later the area's level passes
its minimum, and it is simply a rare find.

A prowler looks like any other wild encounter. There is no special intro,
cry or effect, so players only notice it by what appears and how strong it is.

A caught prowler keeps its level, and the existing obedience rules apply.

### Generation I rewards

| Species | Why | Temperament | Base stat total | Notes |
| --- | --- | --- | --- | --- |
| Bulbasaur line | Prestige | Harmless | 318 | Starter |
| Charmander line | Prestige | Harmless | 309 | Starter |
| Squirtle line | Prestige | Harmless | 314 | Starter |
| Ditto | Utility, rarity | Harmless | 288 | A breeding partner for almost every species |
| Abra | Rarity | Harmless | 310 | Teleports away; a tricky catch |
| Clefairy | Rarity, prestige | Harmless | 323 | Mt. Moon's rarity |
| Eevee | Prestige, rarity | Harmless | 325 | A single gift in the originals |
| Farfetch'd | Rarity | Harmless | 377 | Trade-only in the originals |
| Porygon | Rarity, prestige | Harmless | 395 | A Game Corner prize in the originals |
| Dratini line | Prestige | Dangerous | — | Pseudo-legendary, up to Dragonite |
| Wigglytuff | Evolution | — | 435 | Moon Stone |
| Chansey | Power, rarity | Fierce | 450 | A Safari Zone rarity in the originals |
| Hitmonlee | Power, rarity | Fierce | 455 | A single gift in the originals |
| Hitmonchan | Power, rarity | Fierce | 455 | A single gift in the originals |
| Jynx | Power, rarity | Fierce | 455 | |
| Mr. Mime | Power, rarity | Fierce | 460 | Trade-only in FireRed and LeafGreen |
| Clefable | Evolution | — | 483 | Moon Stone |
| Raichu | Evolution | — | 485 | Thunder Stone |
| Kangaskhan | Power, rarity | Fierce | 490 | A Safari Zone rarity in the originals |
| Tauros | Power, rarity | Fierce | 490 | A Safari Zone rarity in the originals |
| Electabuzz | Power | Fierce | 490 | |
| Vileplume | Evolution | — | 490 | Leaf Stone |
| Victreebel | Evolution | — | 490 | Leaf Stone |
| Magmar | Power | Fierce | 495 | |
| Golem | Evolution | — | 495 | Trade |
| Scyther | Power, rarity | Fierce | 500 | |
| Pinsir | Power, rarity | Fierce | 500 | |
| Alakazam | Evolution | — | 500 | Trade |
| Gengar | Evolution | — | 500 | Trade |
| Nidoqueen | Evolution | — | 505 | Moon Stone |
| Nidoking | Evolution | — | 505 | Moon Stone |
| Ninetales | Evolution | — | 505 | Fire Stone |
| Machamp | Evolution | — | 505 | Trade |
| Poliwrath | Evolution | — | 510 | Water Stone |
| Aerodactyl | Power, prestige | Dangerous | 515 | Also a fossil |
| Starmie | Evolution | — | 520 | Water Stone |
| Cloyster | Evolution | — | 525 | Water Stone |
| Vaporeon | Evolution | — | 525 | Water Stone |
| Jolteon | Evolution | — | 525 | Thunder Stone |
| Flareon | Evolution | — | 525 | Fire Stone |
| Sylveon | Evolution | — | 525 | Shiny Stone. Extension of Eevee |
| Exeggutor | Evolution | — | 530 | Leaf Stone |
| Lapras | Power, prestige | Dangerous | 535 | A single gift in the originals |
| Annihilape | Evolution | — | 535 | Use Rage Fist 20 times. Extension of Primeape |
| Snorlax | Power, prestige | Dangerous | 540 | A static encounter in the originals |
| Arcanine | Evolution | — | 555 | Fire Stone |

**Considered and excluded:**

- Lickitung and Tangela, which are neither strong nor rare enough.
- Omanyte and Kabuto, which are fossils.

### Generation II rewards

| Species | Why | Temperament | Base stat total | Notes |
| --- | --- | --- | --- | --- |
| Chikorita line | Prestige | Harmless | 318 | Starter |
| Cyndaquil line | Prestige | Harmless | 309 | Starter |
| Totodile line | Prestige | Harmless | 314 | Starter |
| Smeargle | Utility, rarity | Harmless | 250 | Learns any move through Sketch |
| Houndour | Prestige | Harmless | 330 | |
| Yanma | Rarity | Harmless | 390 | A swarm species in the originals |
| Togetic | Evolution | — | 405 | Friendship |
| Murkrow | Rarity | Harmless | 405 | |
| Wobbuffet | Rarity | Fierce | 405 | Shadow Tag traps the player, so it is not harmless |
| Dunsparce | Rarity | Harmless | 415 | A rare cave find |
| Sunflora | Evolution | — | 425 | Sun Stone |
| Sneasel | Rarity, prestige | Harmless | 430 | |
| Gligar | Rarity | Harmless | 430 | |
| Misdreavus | Rarity | Harmless | 435 | |
| Larvitar line | Prestige | Dangerous | — | Pseudo-legendary, up to Tyranitar |
| Girafarig | Power | Fierce | 455 | |
| Hitmontop | Power, rarity | Fierce | 455 | |
| Skarmory | Power, rarity | Fierce | 465 | |
| Stantler | Power | Fierce | 465 | |
| Mantine | Power | Fierce | 485 | A native Whirlpool carrier |
| Miltank | Power | Fierce | 490 | |
| Bellossom | Evolution | — | 490 | Sun Stone |
| Slowking | Evolution | — | 490 | Trade |
| Heracross | Power, rarity | Fierce | 500 | A rare Headbutt find |
| Politoed | Evolution | — | 500 | Trade |
| Scizor | Evolution | — | 500 | Trade |
| Shuckle | Power, rarity | Fierce | 505 | |
| Steelix | Evolution | — | 510 | Trade |
| Porygon2 | Evolution | — | 515 | Trade |
| Farigiraf | Evolution | — | 520 | Levels up knowing Twin Beam. Extension of Girafarig |
| Dudunsparce | Evolution | — | 520 | Levels up knowing Hyper Drill. Extension of Dunsparce |
| Espeon | Evolution | — | 525 | Friendship |
| Umbreon | Evolution | — | 525 | Friendship |
| Crobat | Evolution | — | 535 | Friendship |
| Kingdra | Evolution | — | 540 | Trade |
| Blissey | Evolution | — | 540 | Friendship |

**Considered and excluded:** Qwilfish, Corsola, Teddiursa, Phanpy and Delibird,
which are neither strong nor special enough. Sudowoodo and Togepi have their
own static and gift sources.

### Generation III rewards

| Species | Why | Temperament | Base stat total | Notes |
| --- | --- | --- | --- | --- |
| Ralts | Rarity, prestige | Harmless | 198 | A rare find on an early route in the originals |
| Feebas | Rarity, prestige | Harmless | 200 | Hidden on a few fishing tiles in the originals |
| Slakoth | Rarity | Harmless | 280 | The Slaking line |
| Snorunt | Rarity | Harmless | 300 | A rare Shoal Cave find |
| Bagon line | Prestige | Dangerous | — | Pseudo-legendary, up to Salamence |
| Beldum line | Prestige | Dangerous | — | Pseudo-legendary, up to Metagross; a single gift in the originals |
| Treecko line | Prestige | Harmless | 310 | Starter |
| Torchic line | Prestige | Harmless | 310 | Starter |
| Mudkip line | Prestige | Harmless | 310 | Starter |
| Clamperl | Rarity | Harmless | 345 | Found underwater |
| Delcatty | Evolution | — | 400 | Moon Stone |
| Castform | Rarity, utility | Harmless | 420 | A single gift in the originals; changes form with the weather |
| Kecleon | Rarity | Harmless | 440 | Invisible until revealed in the originals |
| Chimecho | Power, rarity | Fierce | 455 | A rare find at Mt. Pyre's summit |
| Zangoose | Power | Fierce | 458 | |
| Seviper | Power | Fierce | 458 | |
| Tropius | Power, rarity | Fierce | 460 | |
| Lunatone | Power | Fierce | 460 | |
| Solrock | Power | Fierce | 460 | |
| Absol | Power, rarity, prestige | Fierce | 465 | |
| Torkoal | Power | Fierce | 470 | |
| Ludicolo | Evolution | — | 480 | Water Stone |
| Shiftry | Evolution | — | 480 | Leaf Stone |
| Relicanth | Power, rarity | Fierce | 485 | Found underwater |
| Huntail | Evolution | — | 485 | Trade with an item |
| Gorebyss | Evolution | — | 485 | Trade with an item |
| Milotic | Evolution | — | 540 | Beauty or trade with an item |

**Considered and excluded:**

- Sableye, Mawile, Spinda, Plusle, Minun, Volbeat, Illumise, Roselia and
  Trapinch, which are neither strong nor rare enough.
- Lileep and Anorith, which are fossils.
- Shedinja, which comes only from evolving Nincada.
- Gen IV extensions of Gen III lines, such as Gallade, Froslass, Probopass,
  Dusknoir and Roserade, which belong to a Gen IV list.

### Generation IV and Hisuian rewards

Generation IV is reserved for a future Sinnoh region, so its Sinnoh-only rows
(the three starters, Rotom, the Gible line, Carnivine and Spiritomb) are
reserved: they record temperaments for Sinnoh and generate no minimum levels
until it exists.
Sinjoh's natives are the Hisuian part of Generation VIII and the Hisuian forms
of older species, and they use the Hisuian rows. At home in Sinjoh, its
Hisuian natives, with Stantler and Scyther, are residents: they hold common
slots and have no minimum level there, as the [Sinjoh encounters spec](sinjoh-encounters.md) defines. Sinjoh's maps count as the
Hisui region, so evolutions that need Hisui work there.

Generation IV species that extend a Generation I–III line are marked
**Extension**. They appear with their line in its home region, not in Sinjoh.
Sinjoh's blend holds nine Generation IV species of its own: Snover, Riolu,
Bronzor, Drifloon, Chingling, Croagunk, Buizel, Shellos and Finneon. Their rows
and the Extension rows apply today.

| Species | Why | Temperament | Base stat total | Notes |
| --- | --- | --- | --- | --- |
| Chimchar line | Prestige | Harmless | 309 | Starter |
| Piplup line | Prestige | Harmless | 314 | Starter |
| Turtwig line | Prestige | Harmless | 318 | Starter |
| Voltorb (Hisuian) | Rarity | Harmless | 330 | |
| Zorua (Hisuian) | Prestige, rarity | Harmless | 330 | |
| Drifloon | Rarity | Harmless | 348 | A rare find in the originals |
| Growlithe (Hisuian) | Prestige, rarity | Harmless | 350 | |
| Sneasel (Hisuian) | Rarity | Harmless | 430 | |
| Rotom | Rarity, prestige | Harmless | 440 | Takes over appliances to change form |
| Qwilfish (Hisuian) | Rarity | Harmless | 440 | |
| Gible line | Prestige | Dangerous | — | Pseudo-legendary, up to Garchomp |
| Carnivine | Power, rarity | Fierce | 454 | |
| Basculin (White-Striped) | Power, rarity | Fierce | 460 | The Basculegion line |
| Lopunny | Evolution | — | 480 | Friendship |
| Lilligant (Hisuian) | Evolution | — | 480 | Sun Stone, in Sinjoh |
| Froslass | Evolution | — | 480 | Dawn Stone. Extension of Snorunt |
| Ambipom | Evolution | — | 482 | Levels up knowing Double Hit. Extension of Aipom |
| Spiritomb | Power, prestige | Fierce | 485 | |
| Electrode (Hisuian) | Evolution | — | 490 | Leaf Stone |
| Mismagius | Evolution | — | 495 | Dusk Stone. Extension of Misdreavus |
| Kleavor | Evolution | — | 500 | Black Augurite |
| Honchkrow | Evolution | — | 505 | Dusk Stone. Extension of Murkrow |
| Braviary (Hisuian) | Evolution | — | 510 | Levels up at 54 in Sinjoh |
| Sneasler | Evolution | — | 510 | Razor Claw, by day |
| Overqwil | Evolution | — | 510 | Levels up knowing Barb Barrage in Sinjoh |
| Weavile | Evolution | — | 510 | Razor Claw, at night. Extension of Sneasel |
| Gliscor | Evolution | — | 510 | Razor Fang, at night. Extension of Gligar |
| Yanmega | Evolution | — | 515 | Levels up knowing Ancient Power. Extension of Yanma |
| Lickilicky | Evolution | — | 515 | Levels up knowing Rollout. Extension of Lickitung |
| Roserade | Evolution | — | 515 | Shiny Stone. Extension of Roselia |
| Gallade | Evolution | — | 518 | Dawn Stone. Extension of Kirlia |
| Lucario | Evolution | — | 525 | Friendship, by day, from Riolu, a rare baby find in Sinjoh |
| Wyrdeer | Evolution | — | 525 | Use Psyshield Bash 20 times |
| Leafeon | Evolution | — | 525 | Leaf Stone, or levels up in Ilex Forest. Extension of Eevee |
| Glaceon | Evolution | — | 525 | Ice Stone, or levels up in Ice Path. Extension of Eevee |
| Probopass | Evolution | — | 525 | Levels up on Route 10 or in the Power Plant. Extension of Nosepass |
| Dusknoir | Evolution | — | 525 | Trade. Extension of Dusclops |
| Samurott (Hisuian) | Evolution | — | 528 | Levels up at 36 in Sinjoh |
| Basculegion | Evolution | — | 530 | Takes enough recoil damage |
| Decidueye (Hisuian) | Evolution | — | 530 | Levels up at 36 in Sinjoh |
| Mamoswine | Evolution | — | 530 | Levels up knowing Ancient Power. Extension of Piloswine |
| Typhlosion (Hisuian) | Evolution | — | 534 | Levels up at 36 in Sinjoh |
| Magnezone | Evolution | — | 535 | Levels up on Route 10 or in the Power Plant. Extension of Magneton |
| Rhyperior | Evolution | — | 535 | Trade. Extension of Rhydon |
| Tangrowth | Evolution | — | 535 | Levels up knowing Ancient Power. Extension of Tangela |
| Porygon-Z | Evolution | — | 535 | Trade. Extension of Porygon2 |
| Electivire | Evolution | — | 540 | Trade. Extension of Electabuzz |
| Magmortar | Evolution | — | 540 | Trade. Extension of Magmar |
| Togekiss | Evolution | — | 545 | Shiny Stone. Extension of Togetic |
| Ursaluna | Evolution | — | 550 | Peat Block, at night. In Sinjoh or at Mt. Moon it gives Bloodmoon Ursaluna |
| Arcanine (Hisuian) | Evolution | — | 555 | Fire Stone |

**Considered and excluded:**

- Uxie, Mesprit, Azelf, Dialga, Palkia, Giratina, Heatran, Regigigas,
  Cresselia, Phione, Manaphy, Darkrai, Shaymin, Arceus and Enamorus, which are
  legendary or mythical.
- Cranidos and Shieldon, which are fossils.
- Level-evolved final stages such as Luxray, Staraptor, Drapion, Toxicroak,
  Hippowdon, Abomasnow, Bronzong, Drifblim and Hisuian Zoroark, which arrive
  through ordinary lines.
- Budew, Chingling, Bonsly, Mime Jr., Happiny, Munchlax and Mantyke, which are
  babies. Babies appear as rare finds named directly in tables, not as reward
  Pokémon.
- Hisuian Sliggoo and Avalugg, which evolve from Kalos's Goomy and Bergmite
  when they evolve in Sinjoh, and Hisuian Goodra, which follows from Hisuian
  Sliggoo outside Sinjoh.

### Generation VI rewards

Generation VI (Kalos) lives in the Safari Zones' reserve. There these species
are residents rather than prowlers: they have no minimum level, never lead a
table, and take at most 10% of any one table, as the
[Safari Zones spec](safari-zones.md) defines. Safari mode has no battles to
lose, so temperament doesn't limit where they appear in a Safari Zone. It still
applies anywhere else.

| Species | Why | Temperament | Base stat total | Notes |
| --- | --- | --- | --- | --- |
| Fennekin line | Prestige | Harmless | 307 | Starter |
| Chespin line | Prestige | Harmless | 313 | Starter |
| Froakie line | Prestige | Harmless | 314 | Starter |
| Honedge | Prestige | Harmless | 325 | The Aegislash line |
| Dedenne | Rarity | Harmless | 431 | |
| Klefki | Power, rarity | Fierce | 470 | |
| Furfrou | Power, rarity | Fierce | 472 | |
| Hawlucha | Power, rarity | Fierce | 500 | |
| Carbink | Power, rarity | Fierce | 500 | Lives in the Kanto Safari's cave |
| Noibat line | Power | Fierce | — | Up to Noivern |
| Goomy line | Prestige | Dangerous | — | Pseudo-legendary, up to Goodra |
| Aromatisse | Evolution | — | 462 | Shiny Stone |
| Trevenant | Evolution | — | 474 | Trade |
| Slurpuff | Evolution | — | 480 | Moon Stone |
| Heliolisk | Evolution | — | 481 | Sun Stone |
| Gourgeist | Evolution | — | 494 | Trade |
| Aegislash | Evolution | — | 500 | Dusk Stone |
| Florges | Evolution | — | 552 | Shiny Stone |

**Considered and excluded:**

- Xerneas, Yveltal, Zygarde, Diancie, Hoopa and Volcanion, which are legendary
  or mythical.
- Tyrunt and Amaura, which come from fossils.
- Sylveon, which evolves from Eevee in Eevee's home regions.

### Generation V rewards

Generation V is Hoenn's second generation. Its Galarian and Hisuian forms live
in Sevii and Sinjoh and are listed there.

| Species | Why | Temperament | Base stat total | Notes |
| --- | --- | --- | --- | --- |
| Litwick | Prestige | Harmless | 275 | The Chandelure line |
| Snivy line | Prestige | Harmless | 308 | Starter |
| Tepig line | Prestige | Harmless | 308 | Starter |
| Oshawott line | Prestige | Harmless | 308 | Starter |
| Axew line | Prestige | Dangerous | 320 | Up to Haxorus. A top-tier line |
| Zorua | Prestige, rarity | Harmless | 330 | The Zoroark line |
| Rufflet | Rarity | Harmless | 350 | A version exclusive in the originals |
| Larvesta line | Prestige, rarity | Dangerous | 360 | Up to Volcarona. A top-tier line |
| Vullaby | Rarity | Harmless | 370 | A version exclusive in the originals |
| Audino | Utility, rarity | Harmless | 445 | Found in rustling grass in the originals; a rich source of experience |
| Deino line | Prestige | Dangerous | — | Pseudo-legendary, up to Hydreigon |
| Swoobat | Evolution | — | 425 | Friendship |
| Maractus | Power | Fierce | 461 | |
| Throh | Power, rarity | Fierce | 465 | A version exclusive in the originals |
| Sawk | Power, rarity | Fierce | 465 | A version exclusive in the originals |
| Alomomola | Power, rarity | Fierce | 470 | |
| Cinccino | Evolution | — | 470 | Shiny Stone |
| Stunfisk | Power | Fierce | 471 | |
| Lilligant | Evolution | — | 480 | Sun Stone |
| Whimsicott | Evolution | — | 480 | Sun Stone |
| Heatmor | Power, rarity | Fierce | 484 | A version exclusive in the originals |
| Durant | Power, rarity | Fierce | 484 | A version exclusive in the originals |
| Druddigon | Power, rarity | Fierce | 485 | |
| Musharna | Evolution | — | 487 | Moon Stone |
| Sigilyph | Power, rarity | Fierce | 490 | |
| Bouffalant | Power | Fierce | 490 | |
| Escavalier | Evolution | — | 495 | Metal Coat |
| Accelgor | Evolution | — | 495 | Friendship |
| Simisage | Evolution | — | 498 | Leaf Stone |
| Simisear | Evolution | — | 498 | Fire Stone |
| Simipour | Evolution | — | 498 | Water Stone |
| Leavanny | Evolution | — | 500 | Friendship |
| Conkeldurr | Evolution | — | 505 | Protector |
| Cryogonal | Power, rarity | Dangerous | 515 | |
| Gigalith | Evolution | — | 515 | Black Augurite |
| Eelektross | Evolution | — | 515 | Thunder Stone |
| Chandelure | Evolution | — | 520 | Dusk Stone |
| Kingambit | Evolution | — | 550 | King's Rock. Extension of Bisharp |

**Considered and excluded:**

- Victini, Cobalion, Terrakion, Virizion, Tornadus, Thundurus, Reshiram,
  Zekrom, Landorus, Kyurem, Keldeo, Meloetta and Genesect, which are legendary
  or mythical.
- Tirtouga and Archen, which are fossils.
- Level-evolved final stages such as Excadrill, Krookodile, Seismitoad,
  Galvantula, Ferrothorn, Klinklang, Reuniclus, Gothitelle, Golurk, Bisharp,
  Braviary, Mandibuzz and Zoroark, which arrive through ordinary lines.
- Basculin, which is a common fish, and Emolga, Karrablast and Shelmet, which
  are neither strong nor rare enough.

### Generation VII and Alolan form rewards

Alola's natives are Generation VII and the Alolan forms of older species.

| Species | Why | Temperament | Base stat total | Notes |
| --- | --- | --- | --- | --- |
| Rockruff | Prestige | Harmless | 280 | The Lycanroc line |
| Vulpix (Alolan) | Prestige, rarity | Harmless | 299 | A version exclusive in the originals |
| Sandshrew (Alolan) | Rarity | Harmless | 300 | A version exclusive in the originals |
| Rowlet line | Prestige | Harmless | 320 | Starter |
| Litten line | Prestige | Harmless | 320 | Starter |
| Popplio line | Prestige | Harmless | 320 | Starter |
| Minior | Rarity | Harmless | 440 | Falls from the sky in many colours |
| Jangmo-o line | Prestige | Dangerous | — | Pseudo-legendary, up to Kommo-o |
| Persian (Alolan) | Evolution | — | 440 | Friendship |
| Sandslash (Alolan) | Evolution | — | 450 | Ice Stone |
| Bruxish | Power | Fierce | 475 | |
| Oricorio | Power, rarity | Fierce | 476 | Changes style with each island's nectar |
| Mimikyu | Power, prestige | Fierce | 476 | |
| Crabominable | Evolution | — | 478 | Levels up at an icy location |
| Komala | Power | Fierce | 480 | |
| Comfey | Power, rarity | Fierce | 485 | |
| Turtonator | Power, rarity | Fierce | 485 | A version exclusive in the originals |
| Drampa | Power, rarity | Fierce | 485 | A version exclusive in the originals |
| Raichu (Alolan) | Evolution | — | 485 | Thunder Stone |
| Oranguru | Power, rarity | Fierce | 490 | A version exclusive in the originals |
| Passimian | Power, rarity | Fierce | 490 | A version exclusive in the originals |
| Golem (Alolan) | Evolution | — | 495 | Trade |
| Vikavolt | Evolution | — | 500 | Levels up in a magnetic field |
| Ninetales (Alolan) | Evolution | — | 505 | Ice Stone |
| Tsareena | Evolution | — | 510 | Levels up knowing Stomp |
| Dhelmise | Power, rarity | Dangerous | 517 | |
| Exeggutor (Alolan) | Evolution | — | 530 | Leaf Stone |

**Considered and excluded:**

- Type: Null and Silvally, Cosmog's line, the Tapus, the Ultra Beasts, Necrozma
  and the mythicals, which are legendary, gifts or lair Pokémon.
- Level-evolved final stages such as Golisopod, Toxapex, Bewear, Lycanroc,
  Salazzle and Alolan Muk, which arrive through ordinary lines.
- Togedemaru, Pyukumuku and Mudbray, which are neither strong nor rare enough.
- Wishiwashi, Alola's commonest fish, as in the original games. Its strong
  school form appears only in battle.

### Generation VIII (Galar) and Galarian form rewards

Sevii's natives are Galar's Generation VIII species and the Galarian forms of
older species. The Hisuian part of Generation VIII belongs to Sinjoh and is
listed separately.

| Species | Why | Temperament | Base stat total | Notes |
| --- | --- | --- | --- | --- |
| Snom | Rarity, prestige | Harmless | 185 | |
| Applin | Prestige | Harmless | 260 | The Flapple and Appletun line |
| Milcery | Prestige | Harmless | 270 | The Alcremie line |
| Dreepy line | Prestige | Dangerous | — | Pseudo-legendary, up to Dragapult |
| Sinistea | Rarity | Harmless | 308 | The Polteageist line |
| Grookey line | Prestige | Harmless | 310 | Starter |
| Scorbunny line | Prestige | Harmless | 310 | Starter |
| Sobble line | Prestige | Harmless | 310 | Starter |
| Slowpoke (Galarian) | Prestige, rarity | Harmless | 315 | |
| Darumaka (Galarian) | Rarity | Harmless | 315 | A version exclusive in the originals |
| Farfetch'd (Galarian) | Rarity | Harmless | 377 | A version exclusive in the originals |
| Ponyta (Galarian) | Prestige, rarity | Harmless | 410 | A version exclusive in the originals |
| Corsola (Galarian) | Rarity | Harmless | 410 | A version exclusive in the originals |
| Mr. Mime (Galarian) | Power, rarity | Fierce | 460 | |
| Falinks | Power | Fierce | 470 | |
| Stonjourner | Power, rarity | Fierce | 470 | A version exclusive in the originals |
| Eiscue | Power, rarity | Fierce | 470 | A version exclusive in the originals |
| Stunfisk (Galarian) | Power | Fierce | 471 | |
| Frosmoth | Evolution | — | 475 | Friendship at night |
| Indeedee | Power, rarity | Fierce | 475 | |
| Cramorant | Power, prestige | Fierce | 475 | |
| Grapploct | Evolution | — | 480 | Levels up knowing Taunt |
| Darmanitan (Galarian) | Evolution | — | 480 | Ice Stone |
| Runerigus | Evolution | — | 483 | Peat Block |
| Flapple | Evolution | — | 485 | Leaf Stone |
| Appletun | Evolution | — | 485 | Sun Stone |
| Slowbro (Galarian) | Evolution | — | 490 | Galarica Cuff |
| Slowking (Galarian) | Evolution | — | 490 | Galarica Wreath |
| Alcremie | Evolution | — | 495 | Shiny Stone |
| Sirfetch'd | Evolution | — | 507 | Three critical hits in one battle |
| Polteageist | Evolution | — | 508 | Dusk Stone |
| Duraludon | Power, rarity | Dangerous | 535 | |

**Considered and excluded:**

- Zacian, Zamazenta, Eternatus, Kubfu and Urshifu, Zarude, Regieleki,
  Regidrago, Glastrier, Spectrier, Calyrex and the Galarian legendary birds,
  which are legendary or mythical.
- Dracozolt, Arctozolt, Dracovish and Arctovish, which are fossils.
- Level-evolved final stages such as Corviknight, Cursola, Mr. Rime,
  Obstagoon, Perrserker, Galarian Rapidash and Galarian Weezing, which arrive
  through ordinary lines.
- Toxel, which is a baby, and Morpeko and Pincurchin, which are neither strong
  nor rare enough.

### Data

This roster replaces the minimum-level list in
`game/src/data/wild_encounter_species.json`, so every species with a minimum
level is a reward, and takes it only where it is a prowler. The change itself
belongs to the implementation.


# Sinjoh encounters

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. It sets the rules for Sinjoh's encounter tables. The
tables themselves are in the [Sinjoh table spec](sinjoh-encounter-tables.md).
Every share below is a placeholder for playtesting.

## Scope

This spec defines how Sinjoh's tables are built:

- Sinjoh's natives, the Hisuian lines born there;
- the Hisuian forms that come only from evolving in Sinjoh;
- Sinjoh's blend, including the few Generation IV species that fit;
- the rules for reach, rewards, water, and day and night.

It doesn't cover:

- the per-map tables, which belong to the Sinjoh table spec;
- Enamorus, the Regi rooms' legendaries and Arceus's events, which belong to
  the legendaries spec;
- how items such as the Black Augurite and the Peat Block are obtained;
- reach values, which belong to [Reach assignments](reach-assignments.md).

## Behavior

### Hisui in miniature

Sinjoh lies beyond Mt. Silver, and its maps count as the Hisui region. It is
ancient Sinnoh on Johto's doorstep: snowfields, an old temple and hot springs.
Its Hisuian content falls into three groups:

| Group | Species | How you get them |
| --- | --- | --- |
| Born Hisuian (natives) | Hisuian Growlithe, Voltorb, Sneasel, Qwilfish and Zorua; white-striped Basculin | Wild in Sinjoh, and only there |
| Hisui's new species | Wyrdeer, Kleavor, Ursaluna, Sneasler, Overqwil, Basculegion | Evolve from Stantler, Scyther, Ursaring, Hisuian Sneasel, Hisuian Qwilfish and white-striped Basculin. A few appear wild as rare showpieces |
| Made by evolving in Sinjoh | Hisuian Typhlosion, Samurott, Decidueye, Lilligant, Braviary, Sliggoo and Goodra, Avalugg | Only by bringing your own Cyndaquil, Oshawott, Rowlet, Petilil, Rufflet, Goomy or Bergmite to Sinjoh and evolving it there |

- **Evolving in Sinjoh is its own reward.** The third group never appears wild,
  and neither do the lines it evolves from, so travelling to Sinjoh with your
  own Pokémon is the only way to these forms.
- **Plain forms stay out.** Ordinary Growlithe, Voltorb, Sneasel, Qwilfish,
  Zorua and red- or blue-striped Basculin don't appear in Sinjoh, so its
  Hisuian forms stay distinct.
- **The natives are residents at home.** They hold Sinjoh's common slots and
  have no prowler minimum level there, though the prowlers spec lists them as
  rewards.

### Rare showpieces

Hisui's new species appear wild only where their stages are allowed:

- On Route 50, which is Outlands: Wyrdeer, Kleavor and Sneasler at 1–4%, and
  Hisuian Arcanine at 1%.
- In the Hot Springs dungeon: Ursaluna at 1–4%, and Hisuian Arcanine.
- In the Ruins chambers: Hisuian Electrode and Sneasler.
- Overqwil and Basculegion come only from evolving: Sinjoh's only water is on
  Route 49, which is Wilds.

### The blend

Hisui's own lines can't fill eight maps, so Sinjoh's blend is larger than
Alola's or Sevii's: about 30–55% of the region's slot weight. Its species come
from Legends: Arceus's Hisui and fit Sinjoh's places.

- **They never lead a table.**
- **They come from included generations** (I, II, III and V), each already
  catchable in its own region, **plus nine Generation IV species that fit:**
  Snover, Riolu, Bronzor, Drifloon, Chingling, Croagunk, Buizel, Shellos and
  Finneon. Generation IV is otherwise reserved for Sinnoh, and Sinnoh's
  signatures, such as its starters, Gible, Shinx and Rotom, stay out.
- **Fierce rewards may appear where the reach allows,** unlike in Alola's and
  Sevii's light blends. Stantler and Scyther, the bases of Wyrdeer and Kleavor,
  are fierce rewards.
- **Kalos stays in the Safari Zones,** so Goomy and Bergmite never appear here.

| Place | Natives | Blend |
| --- | --- | --- |
| Route 49, the way in | Hisuian Growlithe, Voltorb and Sneasel; Hisuian Zorua at night | Stantler, Teddiursa, Scyther, Ponyta, Abra, Yanma; Drifloon at night |
| Route 49's water | Hisuian Qwilfish, white-striped Basculin | Buizel, Shellos, Finneon at night, Remoraid, Tentacool, Magikarp |
| Snowswept Cavern | Hisuian Sneasel and Zorua | Swinub, Snorunt, Snover, Spheal, Zubat, Geodude, Onix, Riolu; Misdreavus at night |
| Sinjoh Ruins | Hisuian Voltorb, Growlithe and Zorua | Bronzor, Chimecho, Chingling, Nosepass, Clefairy, Stantler; Misdreavus, Gastly, Duskull and Drifloon at night |
| Route 50, the snowy edge | Hisuian Sneasel, Growlithe and Zorua; Hisuian Voltorb in the trees | Snover, Stantler, Ursaring, Swinub, Scyther, Machop, Riolu; Snorunt and Drifloon at night |
| The Hot Springs | Hisuian Growlithe | Magmar and Magby, Ponyta, Rhyhorn, Croagunk, Geodude, Psyduck |
| The Ruins chambers | Hisuian Zorua and Voltorb; Hisuian Sneasel in the ice room | Unown, Baltoy, Bronzor, Chimecho, Chingling, Nosepass, Geodude, Snorunt, Snover; Gastly and Misdreavus at night |

### Trees and rocks

Route 50's Headbutt trees drop Hisuian Voltorb, which looks like an old
Apricorn ball, and Hisuian Sneasel, with Teddiursa, Scyther and Drifloon.
Snowswept Cavern's Rock Smash rocks hide Hisuian Zorua and Sneasel, with
Geodude, Nosepass and Onix. The Sinjoh Ruins have no breakable rocks, so they
have no table.

### Water

Route 49 is Sinjoh's only water, a cold coast. Hisuian Qwilfish and
white-striped Basculin lead it, with Buizel, Shellos and Finneon from the
blend.

### Day and night

- Every map has its own night table.
- Outdoors, at least 30% of a night table's slot weight goes to species that
  don't appear in that map's day table, such as Hisuian Zorua, Drifloon,
  Misdreavus, Gastly and Duskull.
- Caves, the Hot Springs and the chambers change more lightly.

### Evolution items and Unown

- **Kleavor's Black Augurite and Ursaluna's Peat Block** come from Mahogany
  Town's evolution-item shop, which also sells the Razor Claw that Sneasler
  needs. Any one-off finds of these items in Sinjoh belong to the
  obtainability spec.
- **Unown letters are random.** A wild Unown takes any of its 28 letters, as
  the engine does everywhere, so the chambers don't spell words.

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
- **Some forms finish evolving elsewhere.** Goomy becomes Hisuian Sliggoo at
  level 40 in Sinjoh, but Hisuian Sliggoo needs rain or fog to become
  Hisuian Goodra, and Sinjoh's weather is snow. It finishes evolving outside.
- **Ursaring's Peat Block depends on place and time.** At night in Sinjoh, or
  at Mt. Moon, it gives Bloodmoon Ursaluna. At night anywhere else, it gives
  Ursaluna. Wild Ursaluna in the Hot Springs is the ordinary form.

### Residents and rewards

- **Residents hold the common slots.** Sinjoh's natives, plus Stantler and
  Scyther, the bases of Wyrdeer and Kleavor, are residents at home: they may
  lead or fill common slots and have no prowler minimum level there, though
  the prowlers spec lists them as rewards.
- **Other rewards are rare.** Any other reward species takes only rare slots
  and at most 5% of a table, and a map holds at most three of them. In the
  blend that means Snorunt, Misdreavus, Drifloon, Chimecho, Clefairy, Magmar
  and Murkrow.

### Rare showpieces

Hisui's new species appear wild only where their stages are allowed:

- On Route 50, which is Outlands: Wyrdeer and Kleavor at 4%, Sneasler and
  Hisuian Arcanine at 1%.
- In the Hot Springs dungeon: Ursaluna at 4% and Hisuian Arcanine at 1%.
- In the Ruins chambers: Hisuian Electrode and Sneasler.
- Overqwil and Basculegion come only from evolving: Sinjoh's only water is on
  Route 49, which is Wilds.

### The blend

Hisui's own lines can't fill eight maps, so Sinjoh's blend is larger than
Alola's or Sevii's: about 30–55% of the region's slot weight, measured across
all of Sinjoh's tables. Its species come from Legends: Arceus's Hisui and fit
Sinjoh's places.

- **They never lead a table.**
- **They come from included generations** (I, II, III and V), each already
  catchable in its own region, **plus nine Generation IV species that fit:**
  Snover, Riolu, Bronzor, Drifloon, Chingling, Croagunk, Buizel, Shellos and
  Finneon. Generation IV is otherwise reserved for Sinnoh, and Sinnoh's
  signatures, such as its starters, Gible, Shinx and Rotom, stay out.
- **Fierce rewards may appear where the reach allows,** unlike in Alola's and
  Sevii's light blends.
- **Kalos stays in the Safari Zones,** so Goomy and Bergmite never appear here.

| Place | Natives | Blend |
| --- | --- | --- |
| Route 49, the way in | Hisuian Sneasel, Growlithe and Voltorb; Hisuian Zorua at night | Stantler, Swinub, Snover, Scyther, Teddiursa, Abra; Duskull and Drifloon at night |
| Route 49's lake | White-striped Basculin, Hisuian Qwilfish | Buizel, Psyduck, Barboach, Remoraid, Magikarp; Shellos and Finneon at night |
| Snowswept Cavern | Hisuian Sneasel and Zorua | Swinub, Snover, Snorunt, Spheal, Zubat, Geodude, Onix, Riolu; Misdreavus at night |
| Sinjoh Ruins | Hisuian Voltorb and Growlithe; Hisuian Zorua at night | Bronzor, Nosepass, Geodude, Stantler, Chingling; Gastly, Duskull, Misdreavus, Drifloon and Clefairy at night |
| Route 50, the snowy edge | Hisuian Sneasel, Growlithe and Zorua; Hisuian Voltorb in the trees | Snover, Stantler, Teddiursa, Swinub, Scyther, Machop, Riolu; Duskull, Drifloon, Snorunt and Murkrow at night |
| The Hot Springs | Hisuian Growlithe; Hisuian Zorua at night | Croagunk, Ponyta, Rhyhorn, Swinub, Geodude, Psyduck, Magmar, Magby; Teddiursa as Ursaluna |
| The Ruins chambers | Hisuian Zorua and Voltorb; Hisuian Sneasel in the ice room | Unown, Bronzor, Nosepass, Geodude, Onix, Snover, Snorunt, Spheal, Chimecho, Chingling; Gastly, Duskull and Misdreavus at night |

### Trees and rocks

Route 50's Headbutt trees drop Hisuian Voltorb, which looks like an old
Apricorn ball, and Hisuian Sneasel, with Teddiursa, Scyther and Drifloon, and
Murkrow at night. Snowswept Cavern's Rock Smash rocks hide Hisuian Zorua and
Sneasel, with Geodude, Nosepass and Onix. The Sinjoh Ruins have no breakable
rocks, so they have no table.

### Water

Route 49's water is a lake fed by a stream and waterfalls, so it is ponds and
rivers. White-striped Basculin, a river fish, leads it. Hisuian Qwilfish, a sea
species, lives there too as a recorded exception, since it has no other water
in Sinjoh, and so do Shellos and Finneon at night.

### Day and night

- Every map has its own night table.
- Outdoors, at least 30% of a land table's night weight goes to species that
  don't appear in that map's day table, such as Hisuian Zorua, Duskull,
  Drifloon, Misdreavus and Gastly.
- Caves, the Hot Springs and the chambers change more lightly, and so do
  surfing, fishing, trees and rocks: each night table there has at least one
  species that appears only at night or is much more common then.

### Variety

No family is one of a table's two most common land slots on more than five of
Sinjoh's eight maps. Hisuian Zorua leads only the caves and chambers at night.

### Native HM sources

Sinjoh needs one field move: Rock Smash. Snowswept Cavern's rocks are the only
way in from Mt. Silver and the only way back, and smashed rocks return when you
re-enter. Route 49's lake and waterfalls and the cavern's two Strength boulders
open nothing new, so Surf, Strength and Waterfall stay outside Sinjoh's native
HM guarantee.

The [catch-window audit's v2 revision](../research/native-hm-windows/revisions/wild-encounters-v2/README.md)
checks Rock Smash across Sinjoh, and on both sides of the rocks: in Mt. Silver's
waterfall room and the cavern's mouth, and in Sinjoh beyond them. Graveler,
which learns Rock Smash at 26 in that revision, carries it: a Graveler slot
stays below Golem's level, so it keeps the move. Hisuian Sneasel and
Teddiursa carry it at lower levels. Any change to those slots must re-run the
audit.

### Evolution items and Unown

- **Kleavor's Black Augurite and Ursaluna's Peat Block** come from Mahogany
  Town's evolution-item shop, which also sells the Razor Claw that Sneasler
  needs. Any one-off finds of these items in Sinjoh belong to the
  obtainability spec.
- **Unown letters are random.** A wild Unown takes any of its 28 letters, as
  the engine does everywhere, so the chambers don't spell words.

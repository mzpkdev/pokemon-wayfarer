# Home Pokédex lists and the National upgrade

Status: Agreed product design, 2026-11-07. Implemented in #220, #221 and
#222. It revises
[Regional and expanding National Pokédex](regional-national-pokedex.md): the
three original regional lists become two home lists matched to Wayfarer's
lands, the National upgrade shows every species, and the professors give that
upgrade on a traveller's first meeting.
Specification: [Home Pokédex lists](../specs/home-pokedex-lists.md)

## Intent

The Pokédex should show what the player meets. Wayfarer's regions are not the
original games' regions: Kanto and Johto are one land where Gen I and Gen II
live, Hoenn holds Gen III and Gen V, the Safari Zones hold Kalos, Sevii holds
Galar, Sinjoh holds Hisui, and later evolutions live with their lines
([Wild encounters v2](wild-encounters-v2.md#natives)). The Pokédex today keeps
the original Kanto, Johto, and Hoenn lists, and only a list's members are ever
shown. A Hoenn-origin player who catches a Gen V Pokémon at home never sees it;
a Johto-origin player never sees anything caught in Hoenn; nobody sees a Safari
Zone Kalos Pokémon. Only Kanto is ever added to anyone's National list.

This design gives every player a home list that matches their home land, and a
National upgrade that shows everything, given by the first professor they meet
in the other land.

## Design

### Two home lists

There are two lands and two home lists:

| Home list | Members | Who starts with it |
| --- | --- | --- |
| Kanto–Johto | Every Gen I and Gen II species | Pallet and New Bark origins |
| Hoenn | Every Gen III and Gen V species | Littleroot origin |

The origin chosen in Oak's introduction selects the home list. It never changes
afterwards.

A home list holds whole families. A species is on it when its evolutionary
family has a member of one of the home generations, which is the same rule
that decides where families live in the world. So the Hoenn list has Pawniard
and its later evolution Kingambit, Roselia and its baby Budew, and the
Kanto–Johto list has Magnemite with Magnezone and Snorlax with Munchlax. A
family that spans both lands, such as Marill's with its Gen III baby Azurill,
is on both lists. An evolution that exists only for a regional form, such as
Obstagoon from Galarian Linoone, is not part of the family: it is National-only,
because its form lives in Sevii or Sinjoh and a home list must be completable
at home. Everything that roams the home land is on its list, whichever origin
the player chose within that land.

Both lists are in National order: the home generations first, then the family
members from other generations. Bulbasaur opens the Kanto–Johto list and
Treecko opens Hoenn's, with Victini after Deoxys.

The lists hold species, not forms. Regional forms have their own entries and
appear in National mode only, as today.

Today's three lists are the original games' regional lists, with later
evolutions added: Kanto's 151, Johto's 256, and Hoenn's 202 species. The two
home lists are about 251 and 291 species plus their families' members from
other generations; the exact counts are recorded when the lists are built.

### The National upgrade shows everything

The Pokédex has the same two modes as today. Regional mode shows the home list.
National mode, once the upgrade is held, shows every species in the game: the
other land's generations, the Safari Zones' Kalos, Sevii's Galar, Sinjoh's
Hisui, Alola's, the Sinnoh species the world holds, and every later evolution.
Nothing is added region by region any more.

Seen and caught records stay global, as today. A Pokémon caught before it can
be shown is remembered and appears, already marked, the moment the player can
see it.

### The professors give the upgrade on first meeting

The upgrade comes from the first professor the player meets in the other land:

| Origin | Who upgrades the Pokédex | When |
| --- | --- | --- |
| Pallet or New Bark | Professor Birch | In his lab, right after the Route 101 rescue, when he thanks the player |
| Littleroot | Professor Elm or Professor Oak, whichever comes first | On the first talk in the New Bark lab or the Pallet lab |

No badge, League, or Hall of Fame is involved. A traveller's Pokédex catches up
the moment they meet the local professor, and everything they caught on the way
appears at once.

Each land's own professor still gives the upgrade at today's milestone, for a
player who never leaves home: Oak in Pallet at eight Kanto badges for a New
Bark origin, Oak at the Olivine port after the Johto Elite Four for a Pallet or
New Bark origin, and Birch after the Hoenn Hall of Fame for a Littleroot
origin. A professor who finds the Pokédex already upgraded says so briefly and
does not upgrade it again; the rest of his scene plays as today.

### Completion and rewards

Completion of the home list means every species on it that the game requires
today, with the same exceptions for mythical Pokémon. A species the game offers
no way to obtain is not required, under today's rule for such species. Before
this ships, every species is checked against what the world actually offers,
for the home lists and for the National list alike, since completing National
earns the Shiny Charm. Gen IV is already marked not required; thirteen Gen V
species have no known source yet (the Tirtouga and Archen fossil lines,
Cobalion, Terrakion, Virizion, Tornadus, Thundurus, Landorus, Reshiram, Zekrom,
and Kyurem), and the later generations' legendaries are unchecked.

The rewards for completing a home list stay where they are: Birch's Johto
starter for completing the Hoenn list, and the Kanto–Johto diplomas and scenes
for completing that list. The Hoenn list is longer than Emerald's, so Birch's
reward asks for Gen V too; that is the point of a list that matches the land.

Pokédex ratings keep their shape. In person, a professor rates the home list
and, once the Pokédex is National, also reports the National counts. By phone,
the home list is rated until it is complete, then the National count is. The
rating's wording stretches to the longer lists. The Trainer Card shows the same
counts the Pokédex shows.

### Critical captures

Critical-capture odds use the home list only, before and after the upgrade: the
number of home-list species caught over the home list's size. Travelling never
lowers a player's odds, and completing the home list reaches the best odds.

## Boundaries

- This design does not change which Pokémon live where, the encounter tables,
  trades, or the Pokédex screens' layout.
- It does not add a third home list. Sevii, Sinjoh, and the Safari Zones'
  Kalos are National-only, because nobody starts there. A later region with
  its own origin registers its own home list.
- It does not move the home professors' existing upgrade scenes; it makes them
  idempotent and adds the traveller's first-meeting upgrade. The Hoenn story
  beats' rival revision, which moves Birch's thanks for a traveller into his
  lab, lands first.
- Saves made before this design keep their origin, their National upgrade, and
  all progress. A saved Kanto or Johto list becomes the Kanto–Johto home list;
  a saved Hoenn list becomes the Hoenn home list. Region grants recorded in an
  old save are ignored, since the upgrade now shows everything.

## Presentation

The Pokédex calls the two modes what it calls them today. The regional mode's
heading names the home land: "KANTO–JOHTO" or "HOENN". Numbers in regional mode
run from 1 for the first species of the home list; National mode uses National
numbers.

Birch's upgrade uses his lab's existing "upgraded to the National mode" line,
at the end of his thanks after the rescue. Elm's and Oak's are short new lines
in their own voices, at the end of the traveller's first talk in their labs,
after whatever else that talk had to say. A traveller who has no Pokédex yet
gets the upgrade on a later talk. In-game text writes the home land as
"KANTO-JOHTO" with a hyphen.

## Playtesting

- A Hoenn-origin player catches a Gen V Pokémon in Hoenn before any travel and
  sees it in regional mode at once.
- A Johto-origin player sails to Hoenn, catches ten Hoenn Pokémon, then rescues
  Birch: all ten appear in National mode, marked caught, right after his thanks.
- A Hoenn-origin player visits New Bark first and Pallet second: Elm upgrades,
  Oak notices it is done.
- A player who never leaves home still gets the upgrade from their own
  professor at today's milestone.
- A Kanto-origin save made before this design loads with the Kanto–Johto list
  and its progress intact.
- Critical-capture odds do not change when the National upgrade arrives.

## Settled questions

1. **Why two lists and not three?** Gen I and Gen II roam both Kanto and Johto,
   so a Kanto-only list of 151 would hide half of what a Kanto-origin player
   meets at home. One list for the land matches the world.
2. **Why not grant regions one by one?** Most of what a traveller meets is in
   no region's list at all (the Safari Zones' Kalos, Sevii's Galar, later
   evolutions). Only "everything" covers it.
3. **Why the professor and not arrival?** Meeting the professor is a scene the
   player can find and remember, it keeps today's upgrade lines, and the Route
   101 rescue already brings every traveller to Birch.
4. **Why keep the home professors' milestones?** A player who never travels
   still deserves a National mode for the Safari Zones and later evolutions.

## References

- [Regional and expanding National Pokédex](regional-national-pokedex.md), the
  design this revises
- [Wild encounters v2](wild-encounters-v2.md#natives), which decides what lives
  where
- [Johto, Hoenn, and Kanto starting regions](wayfarer-regional-start-choice.md)
- [Hoenn independent story beats](hoenn-independent-story-beats.md), whose
  rival revision moves Birch's thanks for a traveller into his lab

# Regional starter gifts

Status: Draft product design, 2026-11-07, for the owner's review. Not implemented.
It revises the starter gifts in [Johto, Hoenn, and Kanto starting regions](wayfarer-regional-start-choice.md),
[Johto independent story beats](johto-independent-story-beats.md#who-plays-which-story),
[Hoenn independent story beats](hoenn-independent-story-beats.md) and the Hoenn-list
reward in [Home Pokédex lists](home-pokedex-lists.md#completion-and-rewards).

## Intent

Every land in Wayfarer should have someone who gives its starters. Today the
gifts depend on where you began. A Pallet player collects one starter from each
of Kanto, Johto and Hoenn. A New Bark or Littleroot player never receives a
Kanto starter, and instead gets a second one from their own region. The
Safari Zones' Kalos, Alola and Sevii's Galar have no starter gift at all.

This design gives each land one gifter, a person who already lives there. Each
gift is earned: a local story, a sidequest or an accomplishment that belongs to
that person, so a starter becomes a memento of something you did in that land.

## Design

### One gifter per land

| Land | Gifter | Trio | Earned by |
| --- | --- | --- | --- |
| Kanto | Professor Oak, Pallet lab | Bulbasaur, Charmander, Squirtle | Beating his grandson Blue in any battle |
| Johto | Professor Elm, New Bark lab | Chikorita, Cyndaquil, Totodile | Hatching an Egg and showing it to him |
| Hoenn | Professor Birch, Littleroot lab | Treecko, Torchic, Mudkip | Beating his friend Norman at the Petalburg Gym |
| Hoenn (Unova lives here) | Professor Cozmo, Fallarbor | Snivy, Tepig, Oshawott | Returning his Meteorite |
| Safari Zones (Kalos) | Baoba, Safari Zone Gate | Chespin, Fennekin, Froakie | Finishing his Safari challenge |
| Alola | Samson Oak, Melemele Isle | Rowlet, Litten, Popplio | Defeating the Melemele totem |
| Sevii (Galar) | Celio, One Island | Grookey, Scorbunny, Sobble | Repairing his Network Machine |

Each gifter offers a choice of one Pokémon from the trio, once per game. The
offer waits until it is taken: a player who declines, or has no room, can come
back later.

### Who receives which gift

The starter you choose in your origin's opening stays free and unchanged. The
professor who gave it does not give a second one from the same trio:

- A Pallet player already has Oak's starter, so Oak's gift is the opening itself.
- A New Bark player already has Elm's.
- A Littleroot player already has Birch's.

Every other gift is open to every origin. Kanto and Johto are one land with two
professors, so a Pallet player can earn Elm's gift and a New Bark player can
earn Oak's. Hoenn's Cozmo is a separate gifter, so a Littleroot player can earn
the Unova trio as well. Each player can end with one starter from every trio.

### The requirements

- **Oak: beat Blue.** Blue is a notable trainer who travels: he fights in
  leagues, visits haunts such as the Pewter Museum, and waits at the Saffron
  Fighting Dojo after your first Indigo win. Any win against him counts. Oak
  is proud of his grandson, and curious about whoever beat him.
- **Elm: hatch an Egg.** Elm studies Eggs, and Johto's Day Care is on Route 34,
  south of Goldenrod. Hatch any Egg and bring the hatched Pokémon to him.
- **Birch: beat Norman.** Birch's friend Norman leads the Petalburg Gym.
  Win his badge and Birch rewards you. This mirrors Oak and Blue: each land's
  professor rewards the visitor who beats someone close to him.
- **Cozmo: return the Meteorite.** This is Hoenn's existing adventure: Team
  Magma takes Cozmo's Meteorite at Meteor Falls, you win it back on Mt. Chimney,
  and Cozmo thanks you in Fallarbor. He gives the starter along with TM Return.
  He explains the Unova Pokémon in his own words, as specimens from a far-off
  region he corresponds with.
- **Baoba: finish the Safari challenge.** Baoba already asks you to show him an
  exotic Pokémon from each Safari area in turn. When the last one is done and
  the Safari Zone is "thriving", he adds the Kalos starter to his thanks.
- **Samson Oak: defeat the totem.** Melemele's totem battle (Totem Alolan
  Raticate, or Totem Leafeon by day) already involves Samson Oak. Win it and he
  offers the Alola trio.
- **Celio: repair the Network Machine.** The Sevii gem investigation ends when
  Celio has both the Ruby and the Sapphire. His thanks for the finished repair
  now include the Galar starter.

No requirement depends on another land's story, and none adds a badge,
league or Hall of Fame condition beyond what its own quest already has.

### Steven no longer gives starters

Steven's Treecko, Torchic or Mudkip gift in the Silph Co. lobby is removed, and
Steven leaves the lobby.
HeartGold and SoulSilver held it back until Red was beaten; Heart & Soul kept
the gift and dropped the condition, so it became the easiest starter in the
game and a second Hoenn starter for Littleroot players. Birch's gift replaces it.

## Boundaries

- The opening starter choice, the rival who follows it, and each origin's
  opening scenes do not change.
- This design does not change where wild starters live. Every Gen I–III and
  V–VIII starter's basic form is already catchable in the wild at a low Trainer
  Rating, and breedable later, so these gifts are rewards, not the only source
  ([Pokédex obtainability audit](../research/pokedex-obtainability.md#starters)).
- No gifter is added for Sinnoh or Sinjoh. Gen IV is not in the world, and
  Sinjoh's Hisuian forms come from evolving existing starters there.
- The gifters' existing quests keep their steps and their other rewards; only
  the starter is added.
- Choosing a gift starter never changes a rival's team or the player's origin.

## Interactions

- **Johto visitors.** The Johto story revision gave visitors no Elm errand, no
  starter choice and no gift. This design keeps the first two and the Mystery
  Egg and Silver story for New Bark players only, but gives visitors Elm's
  starter gift once they hatch an Egg.
- **Hoenn visitors.** Birch's rescue scene still thanks a visitor and rates
  their Pokédex with no gift. His starter gift comes later, after Norman.
- **Birch's old Johto-starter reward is removed.** Heart & Soul has Birch offer
  a Johto starter for completing the Hoenn list. Under this design each trio
  comes only from its own land's gifter, so Elm gives the Johto trio and Birch
  gives only the Hoenn trio. Completing the Hoenn list keeps Birch's Pokédex
  rating but no longer earns a starter. This revises the Hoenn reward named in
  [Home Pokédex lists](home-pokedex-lists.md#completion-and-rewards).
- **Pokédex.** A gift starter records as seen and caught like any received
  Pokémon.
- **Randomizer.** Gift starters follow the existing "starter and gift
  Pokémon" randomizer option, as today's gifts do.
- **Full party.** A gift goes to the PC when the party is full, like other
  gifts; the offer can also wait.

## Balance

The gifts spread across the game. Cozmo's Meteorite and Elm's Egg come early;
Norman, Baoba's challenge and Celio's repair sit mid-game; Blue and the level-60
Melemele totem come late. A dedicated traveller ends with seven starters from
seven trios, in addition to the opening one.

Every gift starter arrives at level 5, as today's gifts do. A late gift is a
fresh partner to raise, not an instant team member.

## Presentation

Each gifter offers the trio the way Birch and Steven do today: the three Poké
Balls or a short choice, the Pokémon's picture and cry, a confirmation, and the
nickname prompt. The thanks line names what you did ("You beat my grandson!",
"The SAFARI ZONE is thriving!").

## Playtesting

- Does each gift feel like a reward for the deed it follows, rather than a
  handout?
- Do players notice the gifters without a hint, or does each land need one line
  of foreshadowing (an aide, a sign, a phone call)?
- Do players raise a level-5 gift that arrives late, after Blue or the totem,
  or leave it in the PC?
- Does Cozmo's explanation for having Unova Pokémon read naturally?

## References

- [Johto, Hoenn, and Kanto starting regions](wayfarer-regional-start-choice.md)
- [Johto independent story beats](johto-independent-story-beats.md#who-plays-which-story)
- [Hoenn independent story beats](hoenn-independent-story-beats.md)
- [Sevii independent story beats](sevii-independent-story-beats.md)
- [Notable trainers](notable-trainers.md) and [Notable haunts](notable-haunts.md), for Blue
- [Kanto Viridian finale](../specs/frlg-kanto-viridian-finale.md), which removed Blue from the Viridian Gym
- [Home Pokédex lists](home-pokedex-lists.md#completion-and-rewards), whose Hoenn-list reward this revises
- [Pokédex obtainability audit](../research/pokedex-obtainability.md)

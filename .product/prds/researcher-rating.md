# Researcher Rating

Implemented: No.
Design status: Draft. The story, the Field Notebook and the 24 researchers and
their challenges are agreed. The curve, thresholds and rewards are placeholders.

## Intent

Trainer Rating rewards the trainer who wins badges. Researcher Rating (RR)
rewards the trainer who studies Pokémon in the field. RR is a second track
beside TR. You earn it by helping one researcher in each of the 24 Gym towns,
and it ends in a story that leads to Mew. RR never changes how strong the world
is. It changes how well you catch, which Pokémon obey you, and what you learn
about rare Pokémon.

## Design

### The origins study

The three regional professors run one shared study: where did Pokémon come
from? Each field researcher studies one piece of it.

- **The start.** Your origin professor (Elm, Oak or Birch) explains the study
  and gives you the Field Notebook. They ask you to help the researchers and
  bring their findings back. A professor you meet later knows the study and
  refers to it, but does not set it up again.
- **The midpoint.** After a set number of seals, your professor reads the
  notebook and sees a pattern: the findings point to one common ancestor of
  all Pokémon. They send you to the Pokémon Mansion's diaries on Cinnabar,
  which record the discovery of Mew.
- **The payoff.** When the study is complete enough, the professors give you
  the Old Sea Map. It leads to Faraway Island, where Mew lives.

### The Field Notebook and Research Seals

The Field Notebook is a key item. It has one page for each of the 24
researchers, showing who they are, what they study and whether you have their
seal. When you complete a researcher's challenge, they stamp a Research Seal
into your notebook and add a one-line note in their own voice.

**Finding researchers.** From the start, the Notebook holds a page for each of
the 24 Gym towns, grouped by land. A page shows only its town until you meet
that town's researcher, so the empty pages tell you where to look without
naming anyone.

**What a page shows.**

| State | The page shows |
| --- | --- |
| Not met | The town's name only |
| Met | The researcher, their subject and how they want the proof handed in |
| Accepted | The task in plain words, and progress for a study with several parts |
| Sealed | The Research Seal and the researcher's note |

The Notebook's cover shows how many seals you hold. You open the Notebook from
the key items pocket.

Seals work like badges for other content:

- A beat can require a number of seals, such as the professor's midpoint scene
  or the Old Sea Map.
- A beat can require a specific seal for a local follow-up. For example, Kurt
  could make his rarest ball only after he has stamped your notebook.

### How you earn RR

Each seal raises RR, in any order and from any land. RR uses the same shape as
TR: the first eight seals each give a big step, and every seal after that gives
a smaller one. RR is hidden, like TR. You feel it through the world, and the
professors' Pokédex ratings comment on your standing as a researcher. RR never
decreases. A new game starts at 0.

### What RR drives

- **Catching.** RR sets the chance of a critical capture, the one-shake catch.
  Today that chance grows with the number of species you have caught. RR
  replaces that count, so catching many species alone no longer raises it.
- **Obedience.** Today Pokémon obey you up to TR's soft level cap. RR adds a
  modest margin on top of that cap, about 5 to 10 levels at full RR (a
  placeholder). TR stays the main control. The margin matters most for
  Pokémon you did not raise yourself, such as legendaries, gifts and trades.
  Experience still follows TR's level cap.
- **Poké Mart stock.** RR unlocks its own shelf in ordinary marts and shares
  the field supplies with TR. See [Poké Mart stock](#poké-mart-stock).
- **Rare encounters.** RR works like a weak Lure that is always on. On every
  wild encounter, a small chance grows with RR. When it hits, the slot is read
  from the far end of the table, so the rarest slots take the odds of the most
  common ones. This uses the Lure's own mechanism. A Lure still stacks on top
  and stays stronger. It applies to every encounter method, including fishing
  and surfing. Prowlers in rare slots get the same small boost.
- **A DexNav.** An early RR reward that grows with every seal. See
  [The DexNav](#the-dexnav).
- **Roamer hints.** Researchers and professors share sightings of roaming
  Pokémon, and higher RR gives sharper hints. A future roamer design owns the
  details.

### The researchers

Each researcher gives one challenge, which earns their seal. A researcher must
be someone whose recognition means something. Where a town had no such person,
a new researcher is added. Each challenge stays within its Gym town and the
area around it, and never needs a trip to another town or land.

| Town | Researcher | Challenge |
| --- | --- | --- |
| Violet | The head scientist at the Ruins of Alph Research Center | Show them 10 different Unown letters |
| Azalea | Kurt | Catch a Pichu in Ilex Forest with a Friend Ball he made |
| Goldenrod | Bill, at home | Breed the Eevee he gave you with a Route 34 Ditto at the Route 34 Day-Care, then give him the hatchling |
| Ecruteak | Sage C, the Sage Office | Catch a Magby-line Pokémon in the Burned Tower, born of the fire that burned it |
| Cianwood | A new marine biologist | Catch a Mantyke and evolve it into Mantine with a Remoraid in your party |
| Olivine | The Fishing Guru, House 3 | Hook a Qwilfish in Olivine's harbour |
| Mahogany | A new lake researcher | Investigate the red Gyarados at the Lake of Rage, by catching it or knocking it out |
| Blackthorn | A new cold-cave scholar | Survey the Ice Path: catch a Swinub, Delibird, Smoochum and Sneasel |
| Pewter | The fossil researcher at the Museum, 1F | Bring the Dome or Helix Fossil from Mt. Moon, have it revived at the Museum, and lend her the Kabuto or Omanyte to display for a day |
| Cerulean | A new entomologist | Catch both a Scyther and a Pinsir on Route 25 |
| Vermilion | The Fishing Dude, elder of the Fishing Bros, House 1 | Hook a Magikarp in Vermilion's harbour that beats his size record |
| Celadon | The Game Designer, Apartments 3F | Catch a Murkrow or Houndour on Route 7, where they only come out after dark |
| Fuchsia | Baoba, the Safari Warden, House 2 | Find a Carbink in the Safari Zone's Mountain or Cave area |
| Saffron | Mr. Psychic | Catch an Abra before it teleports away |
| Cinnabar | The scientist at the Pokémon Lab entrance | Catch a Porygon in the Pokémon Mansion's basement and lend it to the Lab overnight |
| Viridian | A new forest botanist | Level up a Route 22 Eevee inside Viridian Forest so it becomes Leafeon |
| Rustboro | The Poké Ball scientist at Devon Corp, 2F | Evolve a Route 116 Nincada so a Shedinja appears beside Ninjask |
| Dewford | A new marine researcher | Hook an Eelektrik off Dewford after dark |
| Mauville | The Rock Smash Dude, House 1 | Smash Route 111's rocks after dark and find an Aron-line Pokémon |
| Lavaridge | The Herb Shop owner | Catch a Torkoal carrying Charcoal on Fiery Path or Jagged Pass |
| Petalburg | A new Psychic-type researcher | Catch a Ralts on Route 102, where Wally caught his |
| Fortree | A Weather Institute researcher, 1F, on Route 119 | Find Feebas in Route 119's river |
| Mossdeep | The Space Center scientist, 2F | Catch a Snorunt in Shoal Cave at low tide |
| Sootopolis | The former sea researcher in House 5 | Catch a Relicanth on Route 126 |

### Challenge rules

- **Any member of a line counts.** TR decides which stage of a line you meet in
  the wild, so a challenge accepts the whole line.
- **Proof must be local.** Where a challenge names a place, the Pokémon you show
  must have been met there. One caught elsewhere or traded in does not count.
  The game records where a Pokémon was met by map section, not by floor or
  time. A clock or floor condition therefore holds only where the target
  exists in that map section under that condition alone.
- **What a check cannot tell.** The game records the ball a Pokémon is in, not
  who made it, so any Friend Ball counts for Kurt. Friend Balls are also given
  in Alola. It does not record an Egg's parents, so any hatched Eevee counts
  for Bill. It does not record how an Eevee evolved, so a Route 22 Eevee
  evolved with a Leaf Stone also counts for Viridian.
- **Pewter:** only a Kabuto or Omanyte revived at the Pewter Museum counts. The
  Root and Claw Fossils on the Museum counter do not.
- **Mahogany:** the proof is that the lake story's red Gyarados encounter is
  resolved, whether you caught it or knocked it out. Holding the Red Scale is
  not required, because Mr. Pokémon trades it away for an Exp. Share.
- **Some targets must be listed as themselves.** TR only ever steps a wild
  Pokémon down to an earlier form, never forward and never into a baby. A
  challenge that needs an early stage must have that stage listed in its
  table. Route 116 lists Nincada, because a Ninjask can never give a Shedinja.
  A route next to Saffron lists Abra, because Kadabra does not teleport away.
  Burned Tower 1F already lists Magby, and Magmar on B1F counts as the line.
- **Researchers with other roles keep them.** A challenge opens only after an
  existing role that comes first, such as Bill's Eevee gift, a rod handout,
  Baoba's Gold Teeth quest or the Rock Smash HM.

### How you hand in the proof

Each researcher uses exactly one way, and says which in their first
conversation. The Yes/No prompt confirms it before anything is taken.

- **Show:** you show the Pokémon and keep it. Hard-won catches always use show.
- **Log:** each find is recorded as you show it, so you need not keep them all.
- **Give:** the researcher keeps the Pokémon for their study, and their reward
  makes up for it.
- **Give item:** you hand over an item the Pokémon brought, and keep the
  Pokémon.
- **Loan:** the researcher keeps it for a day, then hands it back, sometimes
  with something extra. The seal is stamped when you hand it over.
- **Trade:** the researcher swaps you one of their own, always worth at least
  what you give.
- **Release:** you let the Pokémon go where it belongs, in a short scene. Its
  held item returns to your bag.
- **Battle:** the researcher battles your Pokémon to watch it at work. Losing
  costs nothing, and you can try again.

Give and release may only ask for a Pokémon that is easy to get again. Before
anything leaves your party, the game shows its summary and asks you to
confirm, and it never takes your last Pokémon.

| Town | Way | What happens |
| --- | --- | --- |
| Violet | Log | Each Unown letter is noted as you show it |
| Azalea | Show | Kurt inspects his ball's catch |
| Goldenrod | Give | Bill keeps the hatched Eevee; you can breed another |
| Ecruteak | Release | You return the fire-born Magby-line Pokémon to the tower's ashes |
| Cianwood | Show | Your Mantine, with a Remoraid beside it |
| Olivine | Trade | The Guru swaps your Qwilfish for one of his Remoraid, which also helps Cianwood's challenge without being required |
| Mahogany | Story | The lake story's red Gyarados encounter is resolved |
| Blackthorn | Log | The four Ice Path residents, one at a time |
| Pewter | Loan | The Museum displays your Kabuto or Omanyte for a day |
| Cerulean | Battle | The entomologist watches your Scyther and Pinsir fight |
| Vermilion | Show | The Fishing Dude measures your Magikarp against his record |
| Celadon | Loan | The Game Designer sketches your night Pokémon overnight |
| Fuchsia | Trade | Baoba swaps your Carbink for a revived Tyrunt by day or Amaura by night, "a specimen from my Kalos colleagues". Neither is obtainable anywhere else today |
| Saffron | Trade | Mr. Psychic swaps your Abra for his own: an Abra with the hidden ability Magic Guard, holding a TwistedSpoon |
| Cinnabar | Loan | The Lab studies your Porygon overnight and returns it holding an Up-Grade. Porygon evolves by using the Up-Grade like a stone. Silph Co. also gives one, and the Mahogany shop sells it |
| Viridian | Show | Your Leafeon |
| Rustboro | Show | Your Shedinja |
| Dewford | Release | The researcher tags your Eelektrik and you let it back into the sea |
| Mauville | Battle | The Rock Smash Dude battles your Aron-line Pokémon |
| Lavaridge | Give item | You hand over the Charcoal your Torkoal carried |
| Petalburg | Show | Your Ralts |
| Fortree | Show | Your Feebas |
| Mossdeep | Show | Your Snorunt |
| Sootopolis | Show | Your Relicanth |

A Pokémon received in a trade runs the usual trade evolution check, so a
traded Pokémon that evolves by trade evolves on arrival.

## Boundaries

- RR never changes wild Pokémon levels, trainer levels, the level cap or
  experience. TR alone drives those. RR only adds to shop stock; it never
  removes anything TR opens.
- RR and TR never read each other.
- The challenge NPCs' existing quests, trades and services stay as they are.
- Rival-style research characters are not part of this design.

## Balance

Each challenge should be a short hunt, not a grind. Land slots weigh 20, 20,
10, 10, 10, 10, 5, 5, 4, 4, 1 and 1 per cent. Rock Smash slots weigh 60, 30, 5,
4 and 1 per cent, and on Route 111 a smash starts a battle about a third of the
time. Fishing slots depend on the rod.

Most targets sit at 4–15%, or in a fishing slot near 9% with the Super Rod.
Four needed attention:

| Town | Target | Odds today | Decision |
| --- | --- | --- | --- |
| Mauville | Aron line from Route 111's rocks | 1% of rock encounters, 2 rocks per visit | Move it to the 30% night rock slot: about 5 visits |
| Viridian | Route 22 Eevee | 1% day and night | Move it to a 5% slot |
| Petalburg | Route 102 Ralts | 2% by day, 1% at night | Move it to a 5% day slot |
| Lavaridge | Torkoal holding Charcoal | About 0.5–0.7% per encounter | Keep the odds. The hint points to a Compound Eyes lead, which raises the held-item chance from 5% to 20% (about 35 encounters). Route 116 Nincada has Compound Eyes |

### The DexNav

The DexNav comes from Omega Ruby and Alpha Sapphire. Wayfarer's engine ships a
port of it (`src/dexnav.c`, `include/config/dexnav.h`), which is off today
(`DEXNAV_ENABLED FALSE`). RR gives it to the player early, and every seal makes
it better along three lines.

**A Pokégear app.** Every origin carries the same Pokégear: Emerald's PokéNav,
dressed and named as Heart & Soul's Pokégear (`WayfarerGrantSharedEquipment`
in `src/wayfarer_origin.c`). The DexNav is a new app in it, as it was on the
PokéNav Plus in Omega Ruby and Alpha Sapphire, and keeps the name DexNav. It
appears in the Pokégear's menu once unlocked, next to the map and the phone.
In the field, the R button searches for the species registered in the DexNav,
as the port already does. The DexNav has no Start menu entry of its own. The
Field Notebook stays a separate key item that holds the research pages and
seals.

**What it shows.**

| RR | The area view shows |
| --- | --- |
| From the start | Every species in the area for the current method. Caught species show their icon, seen ones a silhouette, and ones never met a "?" |
| Low | Species that appear only at the other time of day |
| Mid | Each species' rarity, and whether it can carry a held item |
| High | The names behind "?" |

**What you can search for.**

| RR | Search |
| --- | --- |
| From the start | Caught species only, with a short timeout and a close approach |
| Low | A longer timeout and approach |
| Mid | Species you have seen but not caught |
| High | Detector mode: hidden Pokémon ping you as you walk |
| Top | Chaining, which raises shiny odds |

Species you have never seen cannot be searched for, so discovering them still
takes exploring.

**How good the finds are.** RR stands in for the port's search level. ORAS
keeps one search level per species, which would cost a byte per species, so RR
works as one global level instead. Each tier uses one of the port's search
levels and its values:

| Seals | Port search level | Shown before the battle | Egg move | Held item |
| --- | --- | --- | --- | --- |
| 0 | 0 | Level | 0% | 0% |
| 4 | 5 | One move as well | 21% | 0% |
| 8 | 10 | Whether it holds an item | 46% | 1% |
| 12 | 25 | Its ability | 58% | 7% |
| 16 | 50 | Its potential stars | 63% | 6% |
| 24 | 100 | Everything | 83% | 12% |

The seal thresholds are placeholders. The port also raises potential, its IV
stars, and the chance of a hidden ability (up to 23%).

**Wayfarer's encounter rules still apply.** A DexNav find gets the same level
and stage as an ordinary encounter, from TR, the reach and the downward rule. A
prowler keeps its minimum level. A rarer species is harder to find with the
DexNav than a common one, so it never ignores rarity. The port has to read
Wayfarer's encounter data rather than the tables it was written for.

### Poké Mart stock

Ordinary marts sell unlimited stock in tiers, and each tier stays open once
reached ([Poké Marts](global-tr-pokemarts.md)). RR splits that stock with TR by
theme:

| Shelf | Opened by | Items |
| --- | --- | --- |
| Battle supplies | TR, as today | Super, Hyper and Max Potion, Full Restore, Revive, Full Heal |
| Field supplies | Whichever rating reaches the tier first | Great and Ultra Balls, Super and Max Repel |
| Research supplies | RR | Lure, then Super Lure, then Max Lure |

A pure researcher can buy Ultra Balls without seven badges, and a pure battler
without seals. The research shelf is new to ordinary marts. Today a Lure is
only found on Route 49, and only New Sinjoh's shop sells the Max Lure. The seal
counts for each tier are placeholders.

**Specialist shops and town signatures stay as they are.** Each town's own
items, such as Mossdeep's Net and Dive Balls or Verdanturf's Nest Balls, and the
specialist shops, such as Kurt's balls, Mahogany's evolution items, Goldenrod's
flower shop and the department stores' specialist floors, keep their stock,
prices and access for every player. Travelling to them for items found
nowhere else is part of the game for battlers and researchers alike.

### Rare-encounter odds

Placeholder values. On land, the 1% slots trade places with the 20% ones and the
4% slots with the 10% ones.

| RR | Mirror chance | A 1% land slot | A 4% land slot |
| --- | --- | --- | --- |
| 0 | 0% | 1% | 4% |
| After 8 seals | 5% | about 2% | about 4.3% |
| After 16 seals | 10% | about 2.9% | about 4.6% |
| After 24 seals | 15% | about 3.9% | about 4.9% |
| A Lure, for comparison | 20% | about 4.8% | about 5.2% |

The researcher challenges must still be fair at RR 0, because the first
researcher meets a player with no seals. The boost makes later hunts smoother,
not possible.

## Content

- **New key item:** the Field Notebook.
- **Engine feature:** the DexNav port, turned on as a Pokégear app and adapted
  to Wayfarer's encounters.
- **New researchers:** seven new NPCs, in Cianwood, Mahogany, Blackthorn,
  Cerulean, Viridian, Dewford and Petalburg.
- **New dialogue:** the researcher conversations for all 24, the professors'
  introduction, midpoint and payoff scenes, and the later professors' nods to
  the study.
- **New trade Pokémon:** Mr. Psychic's Abra, and Baoba's Tyrunt and Amaura.
- **Encounter edits:** Nincada listed on Route 116, Abra listed on a route next
  to Saffron, the Aron line in Route 111's 30% night rock slot, Route 22 Eevee
  and Route 102 Ralts (day) moved to 5% slots.
- **Removed:** the free Old Sea Map in Vermilion.
- **Changed lines:** the Vermilion Fishing Dude's pointer to the Lake of Rage.

## Presentation

### Researcher conversations

Every researcher follows the same pattern, so the player learns it once.

1. **Introduction.** When you talk to them with the Field Notebook, they
   recognise it ("Ah, PROF. ELM's Field Notebook!") and link their work to the
   origins study in a line or two. Then they state the challenge plainly: what
   to catch, evolve or show, where, when if a clock window applies, and that
   you bring the proof to them. The player should never have to guess what is
   being asked.
2. **Accepting.** A Yes/No prompt asks whether you take on their field study.
   Yes writes the task into the researcher's Notebook page, where you can
   reread it. No ends politely, and the offer stays open.
3. **While the study is open.** Talking again restates the task in one line and
   adds one hint, such as a field technique, a better time or a better spot.
   The hint helps without solving the challenge. The first conversation gives
   no hint.
4. **Turn-in.** The researcher checks for the proof when you talk to them,
   asking you to pick from your party where needed. If you already have the
   proof when you accept, the study completes at once. Completing it plays the
   seal stamp, adds the researcher's note to the Notebook and gives their local
   reward.
5. **Afterwards.** A short line ties their finding back to the study.

### Shared and per-researcher parts

**Shared by every researcher:** the Notebook greeting with the origin
professor's name, the Yes/No prompt, writing the task to the Notebook, the
proof check on accepting, and the seal stamp with its jingle. The seven new
researchers and the plainly spoken scientists also share the accept and decline
lines: the Ruins of Alph Lab, the Cinnabar Lab, Devon, the Weather Institute,
the Space Center, the Pewter Museum and the Sootopolis ex-researcher.

**Written for each researcher:** their subject line, the challenge statement,
the hint, the note they stamp into the Notebook and their closing line.

**Accept and decline lines in their own voice:** Kurt, Bill, Mr. Psychic, the
Rock Smash Dude, the Fishing Dude, the Fishing Guru, Baoba, the Game Designer
and Sage C.

**Placed after an existing role:**

| Researcher | Comes after or around |
| --- | --- |
| Kurt | The Slowpoke Well, the GS Ball and his ball service |
| Bill | His Eevee gift and his first-meeting state |
| The Fishing Guru | His rod handouts |
| Baoba | The Gold Teeth quest |
| The Rock Smash Dude | The HM Rock Smash gift |
| The Herb Shop owner | His Charcoal gift |
| The Game Designer | The Pokédex diploma |
| Mr. Psychic | His TM |
| The Pewter Museum researcher | Her free counter fossils |
| Devon's Poké Ball scientist | His script also advances fossil revival |
| The Weather Institute 1F worker | Hidden while Team Aqua holds the Institute |
| The Space Center 2F scientist | Speaks during Team Magma's raid |

The Ruins of Alph Lab has no head scientist today. The researcher there is the
scientist who calls the wall patterns "the keys for unraveling the RUINS'
mystery".

**Proof checks that differ from the common one.** The common check has you
pick a party Pokémon, then checks its line and where it was met.

| Researcher | What it adds |
| --- | --- |
| Violet, Blackthorn | Several parts, with progress saved between visits |
| Cerulean | A Scyther-line Pokémon and a Pinsir, both in your party for the battle |
| Azalea | The ball it was caught in: a Friend Ball |
| Lavaridge | Its held item: Charcoal |
| Vermilion | The existing Magikarp size-record comparison |
| Goldenrod | A hatched Eevee: met level 0, marked as an Egg |
| Mahogany | The lake story's flag instead of a Pokémon |

## Interactions

- **Old Sea Map.** Today it lies as a free item in Vermilion. RR makes it the
  payoff of the origins study, so the Vermilion item goes away.
- **Vermilion.** The Fishing Dude's line stops sending you to the Lake of Rage
  Fishing Guru. The Lake of Rage Guru stays a separate buyer of big Magikarp.
- **Fossil revival.** The Pewter Museum, the Ruins of Alph Lab and Devon in
  Rustboro share one revival slot. A fossil handed in at one can be collected
  at another. Devon only handles Lileep and Anorith. The Pewter revival
  scientist also borrows the Ruins of Alph local id, which only matches by
  object order.
- **Poké Marts.** The Mart design gains RR thresholds for the field supplies
  and the new research shelf. Its tiers, town signatures and specialist shops
  otherwise stay as designed ([Poké Marts](global-tr-pokemarts.md)).
- **Lures.** The RR mirror chance and a Lure's both read the slot from the far
  end of the table. Their combined effect needs a cap, so that a Lure on top of
  full RR does not make rare slots common.
- **Pokédex obtainability.** Baoba's trade makes Tyrunt and Amaura obtainable,
  along with Tyrantrum and Aurorus. They lose `dexNotRequired`, and the pinned
  obtainability record changes with them
  ([obtainability audit](../research/pokedex-obtainability.md)).
- **Abra.** A player with high TR meets Kadabra instead of Abra. The line
  counts, but only Abra teleports away.

## Constraints

- **Save data.** Each researcher needs met, accepted and sealed states. Violet
  needs one bit per Unown letter, and Blackthorn one per Ice Path species. A
  loaned Pokémon must be stored whole while it is away, as the Day-Care does.
  Only one loan runs at a time, so one stored Pokémon is enough. New save data
  follows the repository's save rules, and prerelease saves need no migration.
- **Hints need no saved state.** Every conversation after the first gives the
  hint.

## Playtesting

- Does each challenge feel like a short expedition rather than a checklist or a
  grind?
- Do players notice the Notebook's empty pages and go looking for researchers?
- Is the challenge always clear from the first conversation, and is the hint
  enough without solving it?
- Do the midpoint scene and the Old Sea Map arrive at a satisfying point in a
  playthrough?
- Do release and give feel fair, and do the trades feel like a treat?
- Is the critical capture and obedience boost noticeable without feeling like
  power?

## Open questions

- Does a researcher accept you before your professor has introduced the study?
- How many seals trigger the midpoint scene, and how many earn the Old Sea Map?
- Which professor plays the midpoint and the payoff for a player who has met
  more than one?
- The RR curve values, and how far it raises critical captures and obedience.
- **Mart thresholds:** how many seals open the field supplies and each Lure.
- **DexNav unlock:** does the origin professor install the app when the
  origins study begins, or does the first Research Seal unlock it?
- **DexNav finds:** may they have hidden abilities and higher potential, or do
  those stay at 0 so the DexNav only gives knowledge?
- **DexNav save cost:** the flags and variables it needs, and confirming that no
  per-species search levels are stored.
- The final mirror chances, and how the RR chance combines with a Lure's: added
  together, or rolled separately.
- What each researcher gives as their local reward alongside the seal.
- Names, sprites and places for the seven new researchers.
- What each researcher's hint says.
- **Battles:** which team does each researcher use, and must the target
  Pokémon take part? Their levels follow TR, like any trainer.
- **Loans:** can you collect a loaned Pokémon before the day is over, and what
  happens when it is your only Pokémon or another loan is running?
- **Traded Pokémon:** what level are Mr. Psychic's Abra and Baoba's Tyrunt and
  Amaura, and do they obey you like your own?
- **Mew:** what level is Mew on Faraway Island, and how does the Old Sea Map's
  ship route fit the payoff?
- **Elm's rating:** Birch and Oak rate your Pokédex. Does Elm need a rating
  script to comment on RR?
- **Phone:** do researchers register in the Pokégear, for the roamer hints and
  the professor's midpoint call?
- **Shedinja:** does a Shedinja keep its Nincada's met location, so it counts as
  met on Route 116?

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Regional starter gifts](regional-starter-gifts.md)
- [FRLG Cinnabar, Seafoam Islands, and coastal routes port](frlg-cinnabar-seafoam-port.md)
- [S.S. ferries](ss-ferries.md)
- [Safari Zones](../specs/safari-zones.md)
- [Kanto Safari and the Gold Teeth](../specs/frlg-kanto-safari-teeth.md)
- [Home Pokédex lists](../specs/home-pokedex-lists.md)
- [Pokédex obtainability audit](../research/pokedex-obtainability.md)

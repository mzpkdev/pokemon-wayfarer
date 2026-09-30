# World news

Implemented: No
Specification: [World news specification](../specs/world-news.md)
Design status: v0 draft for review: one **news feed**, generated from world
state and never stored, is read by four **channels**: notable friends,
townsfolk, TV, and radio. Every channel picks from one shared pool of
neutral third-person **templates** and adds only its own framing. Picks are
deterministic. Emerald's TV system (its shows, PokéNews, and their record
mixing) is retired and its save space freed; swarms, market sales, roamer
sightings, and Gabby & Ty return later as World news events in every
region. Templates and wrapper lines are draft content. Terms follow the
[glossary](player-trainer-rating.md#glossary).

## Intent

The world should talk about itself and about the player. Who holds which
league title, where a Gym Leader spends their days, how far the player has
come: people should know these things and pass them on. One news feed holds
what there is to know, and many mouths repeat it: a friend over a chat, a
stranger in town, the evening news on TV, a DJ on the radio. Since the feed
is built from what is true right now, the news is never stale and never
contradicts the world.

## Design

### The feed

A **news item** is one fact about the world, built on demand from world
state. Nothing about the feed is saved. Items come from three sources:

- **League results:** who holds each league's title, who took the title in
  the latest event (including events the player declined), which notable
  trainers are Masters, and which titles have no holder yet
  ([Leagues](leagues.md)).
- **Whereabouts:** where notable trainers are placed at haunts, and which
  trainers are rising fast right now ([Notable haunts](notable-haunts.md)).
- **The player's fame:** the player's badges, their league titles, and being
  a Master. These only appear once the player's **fame** reaches the
  speaker: for a notable trainer, the haunts' fame rule; for everyone else,
  its global part (TR 80, a league title, or being a Master).

When items need an order, such as "the latest" league event, existing saved
data provides it: the leagues' call counter orders their events.

### Templates

Every item is written with one template from a single shared pool. A
template is a neutral fact in the third person with slots, such as "{NAME}
holds the {LEAGUE} title." Each template is tagged by its topic, its scope
(**local** to a region, or **global**), and its weight (**everyday** or
**headline**).

**Writing rule:** a template states the fact in the third person and never
assumes who says it. "I hear" or "our town" belongs to no template, so the
same line works from a friend, a stranger, an anchor, or a DJ. v0 has 15
templates.

### Channels

A channel decides which items it may use and how to frame them:

- **Notable friends** pass on one item as their gossip, framed by their
  `NEWS` voice bit, from Friend stage up. Whereabouts come first: where
  another trainer the player knows hangs out
  ([gossip](../specs/notable-haunts.md#gossip)).
- **Townsfolk** are existing NPCs chosen as **gossips**. They open with one
  of a few shared lead-ins and tell local news: items about their own
  region.
- **TV** is the World news program: an anchor's intro and outro around
  headlines only. Every TV set in the game runs it, and its screen lights up
  when there is news to air.
- **Radio** airs the same headlines as TV on a new World news station on the
  PokéNav radio, with a DJ's intro and outro.

### Rotation

What a speaker says is fixed by where they stand and how far the world has
come. Two gossips in one town say different things, and both move on to new
news as world progress rises. TV and radio walk the headlines in order, a
few per broadcast. Once the in-game clock exists, news can also rotate
daily.

### Retiring Emerald TV

Emerald's TV system goes: its shows, PokéNews, their texts, and their part
in record mixing. Its hooks stay in place and do nothing, so the systems
that fed it (contests, secret bases, the Battle Tower, the Game Corner, and
more) keep working as they do; only their TV coverage goes. The freed save
space is 964 bytes. Prerelease saves don't carry over.

Four things TV used to start are kept and come back later as World news
events in all regions: swarms, market sales, roamer news, and Gabby & Ty.
Until then, retiring TV turns off swarms and PokéNews sales, which only TV
could start. Gabby & Ty keep battling on their Hoenn routes.

## Sample playthrough

1. With three Kanto badges, the player talks to a gossip in Pewter City.
   Their fame hasn't spread yet (TR 30), so the gossip tells local news,
   such as "Word around here is... LT. SURGE is back home, at the
   VERMILION harbour." Another gossip in the same town says something
   else: "Have you heard? No one holds the INDIGO LEAGUE title yet."
2. In a house in Cerulean City, the TV screen is lit. The anchor opens
   World news and reads up to three headlines, such as "MISTY is on the
   rise, getting stronger by the day." The next time the player watches,
   the program moves on to the next headlines.
3. Brock is a friend. At his haunt he says his `NEWS` bit and passes on
   where Misty is.
4. Much later the player declines an Indigo invitation, and Lance takes the
   title. Now Kanto and Johto gossips and TVs say "LANCE took the INDIGO
   LEAGUE title in the latest event." Once the player wins the Hoenn
   League, Hoenn TVs say "{PLAYER} holds the HOENN LEAGUE title."

## Boundaries

In: the feed and its sources, the template pool and its tags, the four
channels and their wrapper lines, rotation, the gossip catalog, the World
news TV script and screen rule, the radio station, retiring Emerald's TV
system and PokéNews, and the dependency decisions.

Unchanged: the haunts keep the notable channel's rules (stage, the
whereabouts pick, and the `NEWS` bit:
[Notable haunts](../specs/notable-haunts.md#gossip)); league results come
from [Leagues](leagues.md); contests, secret bases, the Battle Tower, the
Dewford trend, the Game Corner, and the Trainer Fan Club keep working
without TV coverage.

Out of scope for v0: swarms, market sales, roamer sightings, Gabby & Ty
interviews on air, phone calls with news, and daily rotation.

## Presentation

- Townsfolk gossips speak in their usual text box: a lead-in, then the
  fact.
- TV shows the anchor's lines in a sign-style box with the screen lit, then
  turns the screen off.
- The radio station prints the DJ's lines and headlines in the PokéNav radio
  window, one short line at a time.

## Interactions

- **Notable haunts.** World news owns the feed and templates; the haunts
  keep the notable channel's rules.
- **Leagues.** Titles, reign records, and the call counter feed league
  items; no league state changes.
- **Poké Marts.** Today's PokéNews half-price sale stops; market sales come
  back later as a World news event tied to the
  [global marts](global-tr-pokemarts.md).
- **Emerald Lati roaming.** The Littleroot TV stops replaying the Latias or
  Latios news flash; the return-home scene keeps it
  ([Lati roaming](emerald-lati-roaming.md)).
- **Mauville's Old Man.** Watching TV still counts for his stories;
  interviews no longer do until Gabby & Ty's interviews return.

## Open risks

- Townsfolk and TV only show the player's fame from TR 80, so before then
  the player never hears about themselves outside their friends.
- The radio station is only on the PokéNav radio, which Wayfarer gives out
  at Goldenrod's Radio Tower; players who start elsewhere may meet it late.

## Specifications

- [World news specification](../specs/world-news.md): the feed and its
  items, templates and tags, channels and wrapper lines, rotation, the TV
  and radio hooks, retiring Emerald TV and its dependencies, saved state,
  load validation, and acceptance.

## Later

- **Swarms:** a news item plus a real outbreak on that route, in any region.
- **Market sales:** a news item plus a real discount at that shop.
- **Roamer sightings:** a hint about where a roamer is now.
- **Gabby & Ty:** roaming reporters in every region who interview the player
  after big moments; the interview airs.
- **Phone calls:** Close friends call with news from the feed.
- **Daily rotation** once the in-game clock exists.

## References

- [Notable haunts](notable-haunts.md)
- [Notable trainers](notable-trainers.md)
- [Leagues](leagues.md)
- [Sevii Masters](sevii-masters.md)
- [Poké Marts across the three regions](global-tr-pokemarts.md)
- [Emerald Lati roaming](emerald-lati-roaming.md)

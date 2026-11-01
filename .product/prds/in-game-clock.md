# In-game clock

Specifications: [In-game clock](../specs/in-game-clock.md)

Design status: agreed. The 90-minute day with the engine's hours shipped first;
the 2-hour day with Wayfarer's own times of day below is agreed and replaces
it. Both lengths are placeholders for playtesting.

## Intent

Wayfarer's days drive a lot of the world: the daily world slots, phone
rematches, berries, day and night, and soon league invitations. Today those
days follow the cartridge's real-time clock (RTC), so the world changes while
the game is off, a player who plays one long session sees one day, and anyone
can skip days by setting a clock. Wayfarer instead keeps its own clock that
runs only while you play. A day passes every 2 hours of play, so a long
session sees the world change a few times, and every player gets the same
pace. The day has its own shape: a long bright day framed by a morning, an
evening and a night of equal length, so each time of day lasts long enough
to notice and to plan around.

## Design

**The clock runs with play.** The game keeps an in-game date and time in the
save. It moves forward only while the game is running: 12 in-game seconds for
every real second, so one in-game day lasts **2 hours of play** and an in-game
hour lasts **5 minutes**. It never runs while the game is off, and nothing in
the game reads the cartridge's clock.

**What counts as play.** The clock runs on the same counter as the play time
on the trainer card and the save screen, so it runs everywhere that counter
does: on the field, in battles, menus, the bag, the PC, the Pokédex, and in
cutscenes. It stops on the title screen and its menus, and while the game is
off. Exactly which screens stop it is in the
[spec](../specs/in-game-clock.md#what-play-time-counts).

**Times of day.** Wayfarer sets its own hours instead of the engine's, whose
evening lasted under 4 minutes of play and whose night lasted 41:

| Time of day | Hours | Play time |
| --- | --- | --- |
| Morning | 4:00–8:00 | 20 minutes |
| Day | 8:00–20:00 | 1 hour |
| Evening | 20:00–24:00 | 20 minutes |
| Night | 0:00–4:00 | 20 minutes |

Half of every day is bright day. Morning, evening and night each last
20 minutes: long enough to see the light change, catch the Pokémon that only
come out then, or evolve one that needs that time.

Wild Pokémon keep two populations: morning and day bring out the day
Pokémon, evening and night the night ones, so night Pokémon are about from
20:00 to 4:00. The Pokémon you can see walking around follow the same rule as
the ones you meet in the grass; before this, evening showed day ones on the
map while the grass held night ones.

The day also splits into two equal halves that are easy to read on a clock:
**by day** is the Day period (8:00–20:00) and **by night** is evening, night
and morning together (20:00–8:00), an hour of play each. Systems that only
need "day or night", like the [S.S. ferries](#later), use these halves.

**A new game starts by day.** The player's first day starts at 8:00 on
day 1, the start of the day period, with an hour of daylight ahead.

**No setting or skipping time.** The player can't set, turn back or skip the
clock in v0. Wayfarer drops the options that did:

- the challenge menu's real-time-clock / in-game-clock choice (the in-game
  clock is the only one);
- the hidden screen that resets the cartridge clock; and
- setting the clock in the player's house: the openings still stop at the
  bedroom clock, but only to look at it.

**Days and times of day.** Everything that happens "each day" or "at night"
follows the in-game clock: daily world slots, phone rematch readiness and
gifts, berries and apricorns, daily flags and events, the lottery, Mirage
Island, the Shoal Cave tides, day-and-night wild Pokémon, lighting, evolutions
and forms that need a time of day, and the time shown in menus. A new day
starts the same way it always has; only the clock that says when it starts
changed.

**Days only go forward.** With no way to set the clock, the day count only
ever increases. Systems that count days (the daily update, league
invitations) still guard against a day that goes backwards, but in v0 it
can't happen.

## Boundaries

- Only the Wayfarer build. The standalone Emerald, FRLG and HNS builds are
  not kept working with their old clocks.
- No time skipping in v0 (no sleeping, no waiting at a clock), except sleeping
  through a voyage in an S.S. ferry cabin ([S.S. ferries](ss-ferries.md)).
- The times of day keep their names and their meaning to every system that
  reads them; only their hours change. No system gains or loses a time of day.
- The notable world simulation keeps running on map changes; the clock does
  not drive it yet ([notable world simulation](../specs/notable-world-simulation.md#later)).
- No save migration: prerelease saves are not carried over.

## Balance

2 hours a day is a placeholder. It should feel like a session sees a day or
two pass, so daily slots and rematches refresh often enough to matter, while a
day still lasts long enough to clear a route or two and the times of day are
long enough to plan around.

Going from 90 minutes to 2 hours makes each day a third longer, so every daily
thing comes a quarter less often per hour of play: daily world slots, phone rematches and gifts,
berries, and league invitations (a 7-day invitation interval goes from about
10.5 to 14 hours of play). Things counted in in-game hours slow the same way:
a phone call chance every 15 minutes of play instead of about 11, and each
Shoal Cave tide lasts 30 minutes instead of 22.5.

## Playtesting

- Does a 2-hour day feel right for daily slots and rematches?
- Are 20-minute mornings, evenings and nights long enough to notice and use?
- Should league invitations come after fewer days, now that a day is longer?

## Later

- **S.S. ferries.** The S.S. Aqua, Anne and Tidal will sail between regions,
  and ports shared by two ships will host one by day and the other by night
  ([S.S. ferries](ss-ferries.md)). Their design builds on this clock's halves.

## References

- [Daily world slots](daily-world-slots.md)
- [Leagues](leagues.md), whose invitations count in-game days
- [Hoenn phone rematches](hoenn-phone-rematches.md)

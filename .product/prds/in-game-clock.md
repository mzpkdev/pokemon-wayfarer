# In-game clock

Specifications: [In-game clock](../specs/in-game-clock.md)

Design status: agreed. The day length is a placeholder for playtesting.

## Intent

Wayfarer's days drive a lot of the world: the daily world slots, phone
rematches, berries, day and night, and soon league invitations. Today those
days follow the cartridge's real-time clock (RTC), so the world changes while
the game is off, a player who plays one long session sees one day, and anyone
can skip days by setting a clock. Wayfarer instead keeps its own clock that
runs only while you play. A day passes every 90 minutes of play, so a long
session sees the world change several times, and every player gets the same
pace.

## Design

**The clock runs with play.** The game keeps an in-game date and time in the
save. It moves forward only while the game is running: 16 in-game seconds for
every real second, so one in-game day lasts **90 minutes of play** (an in-game
hour lasts 3 minutes 45 seconds). It never runs while the game is off, and
nothing in the game reads the cartridge's clock.

**What counts as play.** The clock runs on the same counter as the play time
on the trainer card and the save screen, so it runs everywhere that counter
does: on the field, in battles, menus, the bag, the PC, the Pokédex, and in
cutscenes. It stops on the title screen and its menus, and while the game is
off. Exactly which screens stop it is in the
[spec](../specs/in-game-clock.md#what-play-time-counts).

**A new game starts by day.** The player's first day starts at 10:00 on
day 1, the start of the day period.

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
- No time skipping in v0 (no sleeping, no waiting at a clock). A later design
  may add one.
- The notable world simulation keeps running on map changes; the clock does
  not drive it yet ([notable world simulation](../specs/notable-world-simulation.md#later)).
- No save migration: prerelease saves are not carried over.

## Balance

90 minutes a day is a placeholder. It should feel like a session sees a few
days pass, so daily slots and rematches refresh often enough to matter, while
a day still lasts long enough to clear a route or two.

## Playtesting

- Does a 90-minute day feel right for daily slots and rematches?
- Is night (about 41 minutes of each day with the current hours) too long?

## References

- [Daily world slots](daily-world-slots.md)
- [Leagues](leagues.md), whose invitations count in-game days
- [Hoenn phone rematches](hoenn-phone-rematches.md)

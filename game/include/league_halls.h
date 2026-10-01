#ifndef GUARD_LEAGUE_HALLS_H
#define GUARD_LEAGUE_HALLS_H

#include "global.h"
#include "league_selection.h"

struct StartingStatuses;

enum LeagueHallCondition
{
    LEAGUE_HALL_NEUTRAL,
    LEAGUE_HALL_SNOW,
    LEAGUE_HALL_SANDSTORM,
    LEAGUE_HALL_TRICK_ROOM,
    LEAGUE_HALL_MAGIC_ROOM,
    LEAGUE_HALL_WONDER_ROOM,
    LEAGUE_HALL_PSYCHIC_TERRAIN,
    LEAGUE_HALL_GRASSY_TERRAIN,
    LEAGUE_HALL_MISTY_TERRAIN,
    LEAGUE_HALL_TAILWIND,
    LEAGUE_HALL_SEA_OF_FIRE,
    LEAGUE_HALL_STICKY_WEB,
    LEAGUE_HALL_STEALTH_ROCK,
    LEAGUE_HALL_CONDITION_COUNT,
};

struct LeagueHall
{
    u16 room;
    const u8 *name; // NULL for the neutral Champion's Room.
    u16 honours; // Canonical notable character ID; 0 for the Champion's Room.
    enum LeagueHallCondition condition;
    u8 weather; // Map weather, not a battle-side weather override.
    const u8 *conditionName;
};

const struct LeagueHall *GetLeagueHall(enum LeagueId league, u8 match);
bool32 BuildLeagueHallStartingStatuses(const struct LeagueHall *hall, struct StartingStatuses *out);
bool32 ValidateLeagueHallRegistry(void);

#endif // GUARD_LEAGUE_HALLS_H

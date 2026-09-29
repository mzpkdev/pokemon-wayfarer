#include "global.h"
#include "data.h"
#include "league_circuit.h"
#include "league_halls.h"
#include "constants/maps.h"
#include "constants/notable_trainers.h"
#include "constants/weather.h"

// Each row follows the corresponding five-room circuit chain. Conditions
// belong to these rooms, regardless of which selected trainer fights there.
static const struct LeagueHall sLeagueHalls[3][LEAGUE_LINEUP_SIZE] =
{
    [LEAGUE_ID_INDIGO - 1] = {
        {MAP_POKEMON_LEAGUE_LORELEIS_ROOM, COMPOUND_STRING("Lorelei's Hall"), NOTABLE_TRAINER_LORELEI, LEAGUE_HALL_SNOW, WEATHER_SNOW, COMPOUND_STRING("Snow")},
        {MAP_POKEMON_LEAGUE_BRUNOS_ROOM, COMPOUND_STRING("Bruno's Hall"), NOTABLE_TRAINER_BRUNO, LEAGUE_HALL_SANDSTORM, WEATHER_SANDSTORM, COMPOUND_STRING("Sandstorm")},
        {MAP_POKEMON_LEAGUE_AGATHAS_ROOM, COMPOUND_STRING("Agatha's Hall"), NOTABLE_TRAINER_AGATHA, LEAGUE_HALL_TRICK_ROOM, WEATHER_NONE, COMPOUND_STRING("Trick Room")},
        {MAP_POKEMON_LEAGUE_LANCES_ROOM, COMPOUND_STRING("Lance's Hall"), NOTABLE_TRAINER_LANCE, LEAGUE_HALL_TAILWIND, WEATHER_NONE, COMPOUND_STRING("Tailwind (both sides)")},
        {MAP_POKEMON_LEAGUE_CHAMPIONS_ROOM, NULL, NOTABLE_TRAINER_NONE, LEAGUE_HALL_NEUTRAL, WEATHER_NONE, COMPOUND_STRING("Neutral")},
    },
    [LEAGUE_ID_MASTERS - 1] = {
        {MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS, COMPOUND_STRING("Will's Hall"), NOTABLE_TRAINER_WILL, LEAGUE_HALL_PSYCHIC_TERRAIN, WEATHER_NONE, COMPOUND_STRING("Psychic Terrain")},
        {MAP_POKEMON_LEAGUE_KOGAS_ROOM_HNS, COMPOUND_STRING("Koga's Hall"), NOTABLE_TRAINER_KOGA, LEAGUE_HALL_GRASSY_TERRAIN, WEATHER_NONE, COMPOUND_STRING("Grassy Terrain")},
        {MAP_POKEMON_LEAGUE_BRUNOS_ROOM_HNS, COMPOUND_STRING("Bruno's Hall"), NOTABLE_TRAINER_BRUNO, LEAGUE_HALL_STEALTH_ROCK, WEATHER_NONE, COMPOUND_STRING("Stealth Rock (both sides)")},
        {MAP_POKEMON_LEAGUE_KARENS_ROOM_HNS, COMPOUND_STRING("Karen's Hall"), NOTABLE_TRAINER_KAREN, LEAGUE_HALL_WONDER_ROOM, WEATHER_NONE, COMPOUND_STRING("Wonder Room")},
        {MAP_POKEMON_LEAGUE_CHAMPIONS_ROOM_HNS, NULL, NOTABLE_TRAINER_NONE, LEAGUE_HALL_NEUTRAL, WEATHER_NONE, COMPOUND_STRING("Neutral")},
    },
    [LEAGUE_ID_HOENN - 1] = {
        {MAP_EVER_GRANDE_CITY_SIDNEYS_ROOM, COMPOUND_STRING("Sidney's Hall"), NOTABLE_TRAINER_SIDNEY, LEAGUE_HALL_MAGIC_ROOM, WEATHER_NONE, COMPOUND_STRING("Magic Room")},
        {MAP_EVER_GRANDE_CITY_PHOEBES_ROOM, COMPOUND_STRING("Phoebe's Hall"), NOTABLE_TRAINER_PHOEBE, LEAGUE_HALL_STICKY_WEB, WEATHER_NONE, COMPOUND_STRING("Sticky Web (both sides)")},
        {MAP_EVER_GRANDE_CITY_GLACIAS_ROOM, COMPOUND_STRING("Glacia's Hall"), NOTABLE_TRAINER_GLACIA, LEAGUE_HALL_MISTY_TERRAIN, WEATHER_NONE, COMPOUND_STRING("Misty Terrain")},
        {MAP_EVER_GRANDE_CITY_DRAKES_ROOM, COMPOUND_STRING("Drake's Hall"), NOTABLE_TRAINER_DRAKE, LEAGUE_HALL_SEA_OF_FIRE, WEATHER_NONE, COMPOUND_STRING("Sea of Fire (both sides)")},
        {MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM, NULL, NOTABLE_TRAINER_NONE, LEAGUE_HALL_NEUTRAL, WEATHER_NONE, COMPOUND_STRING("Neutral")},
    },
};

const struct LeagueHall *GetLeagueHall(enum LeagueId league, u8 match)
{
    if (league < LEAGUE_ID_INDIGO || league > LEAGUE_ID_HOENN || match >= LEAGUE_LINEUP_SIZE)
        return NULL;
    return &sLeagueHalls[league - 1][match];
}

bool32 BuildLeagueHallStartingStatuses(const struct LeagueHall *hall, struct StartingStatuses *out)
{
    if (hall == NULL || out == NULL || hall->condition >= LEAGUE_HALL_CONDITION_COUNT)
        return FALSE;
    *out = (struct StartingStatuses){0};
    switch (hall->condition)
    {
    case LEAGUE_HALL_NEUTRAL:
    case LEAGUE_HALL_SNOW:
    case LEAGUE_HALL_SANDSTORM:
        break; // Weather comes from the room map, not a starting status.
    case LEAGUE_HALL_TRICK_ROOM:
        out->trickRoomTemporary = TRUE;
        break;
    case LEAGUE_HALL_MAGIC_ROOM:
        out->magicRoomTemporary = TRUE;
        break;
    case LEAGUE_HALL_WONDER_ROOM:
        out->wonderRoomTemporary = TRUE;
        break;
    case LEAGUE_HALL_PSYCHIC_TERRAIN:
        out->psychicTerrainTemporary = TRUE;
        break;
    case LEAGUE_HALL_GRASSY_TERRAIN:
        out->grassyTerrainTemporary = TRUE;
        break;
    case LEAGUE_HALL_MISTY_TERRAIN:
        out->mistyTerrainTemporary = TRUE;
        break;
    case LEAGUE_HALL_TAILWIND:
        out->tailwindPlayerTemporary = TRUE;
        out->tailwindOpponentTemporary = TRUE;
        break;
    case LEAGUE_HALL_SEA_OF_FIRE:
        out->seaOfFirePlayerTemporary = TRUE;
        out->seaOfFireOpponentTemporary = TRUE;
        break;
    case LEAGUE_HALL_STICKY_WEB:
        out->stickyWebPlayer = TRUE;
        out->stickyWebOpponent = TRUE;
        break;
    case LEAGUE_HALL_STEALTH_ROCK:
        out->stealthRockPlayer = TRUE;
        out->stealthRockOpponent = TRUE;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

bool32 ValidateLeagueHallRegistry(void)
{
    u16 seenConditions = 0;
    u32 league, match;
    for (league = LEAGUE_ID_INDIGO; league <= LEAGUE_ID_HOENN; league++)
    {
        u64 seenHonours = 0;
        for (match = 0; match < LEAGUE_LINEUP_SIZE; match++)
        {
            const struct LeagueHall *hall = GetLeagueHall(league, match);
            u8 requiredWeather;
            if (hall == NULL || hall->condition >= LEAGUE_HALL_CONDITION_COUNT
             || hall->conditionName == NULL || hall->room == MAP_UNDEFINED)
                return FALSE;
#if IS_WAYFARER
            enum CircuitStage stage = league == LEAGUE_ID_INDIGO ? CIRCUIT_STAGE_INDIGO
                                    : league == LEAGUE_ID_MASTERS ? CIRCUIT_STAGE_MASTERS
                                    : CIRCUIT_STAGE_HOENN;
            if (hall->room != GetCircuitStageRoom(stage, match))
                return FALSE;
#endif
            requiredWeather = hall->condition == LEAGUE_HALL_SNOW ? WEATHER_SNOW
                            : hall->condition == LEAGUE_HALL_SANDSTORM ? WEATHER_SANDSTORM : WEATHER_NONE;
            if (hall->weather != requiredWeather)
                return FALSE;
            if (match == LEAGUE_LINEUP_SIZE - 1)
            {
                if (hall->condition != LEAGUE_HALL_NEUTRAL || hall->name != NULL
                 || hall->honours != NOTABLE_TRAINER_NONE)
                    return FALSE;
            }
            else
            {
                if (hall->condition == LEAGUE_HALL_NEUTRAL || hall->name == NULL
                 || hall->honours == NOTABLE_TRAINER_NONE || hall->honours > NOTABLE_TRAINER_COUNT
                 || (seenHonours & (1ull << hall->honours))
                 || (seenConditions & (1 << hall->condition)))
                    return FALSE;
                seenHonours |= 1ull << hall->honours;
                seenConditions |= 1 << hall->condition;
            }
        }
    }
    return seenConditions == ((1 << LEAGUE_HALL_CONDITION_COUNT) - 2);
}

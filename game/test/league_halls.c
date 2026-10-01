#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "debug.h"
#include "event_data.h"
#include "league_circuit.h"
#include "league_event_battle.h"
#include "league_events.h"
#include "league_halls.h"
#include "league_run_helpers.h"
#include "overworld.h"
#include "string_util.h"
#include "test/test.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "config/league_circuit.h"
#include "constants/battle_setup.h"
#include "constants/maps.h"
#include "constants/notable_trainers.h"
#include "constants/opponents.h"
#include "constants/weather.h"
#include "constants/wayfarer_indigo_trainers.h"

#if WAYFARER_LEAGUE_EVENTS

struct ExpectedHall
{
    u16 room;
    u16 honours;
    enum LeagueHallCondition condition;
    u8 weather;
    const u8 *name;
    const u8 *conditionName;
};

static const struct ExpectedHall sExpectedHalls[3][LEAGUE_LINEUP_SIZE] =
{
    [LEAGUE_ID_INDIGO - 1] = {
        {MAP_POKEMON_LEAGUE_LORELEIS_ROOM, NOTABLE_TRAINER_LORELEI, LEAGUE_HALL_SNOW, WEATHER_SNOW, COMPOUND_STRING("Lorelei's Hall"), COMPOUND_STRING("Snow")},
        {MAP_POKEMON_LEAGUE_BRUNOS_ROOM, NOTABLE_TRAINER_BRUNO, LEAGUE_HALL_SANDSTORM, WEATHER_SANDSTORM, COMPOUND_STRING("Bruno's Hall"), COMPOUND_STRING("Sandstorm")},
        {MAP_POKEMON_LEAGUE_AGATHAS_ROOM, NOTABLE_TRAINER_AGATHA, LEAGUE_HALL_TRICK_ROOM, WEATHER_NONE, COMPOUND_STRING("Agatha's Hall"), COMPOUND_STRING("Trick Room")},
        {MAP_POKEMON_LEAGUE_LANCES_ROOM, NOTABLE_TRAINER_LANCE, LEAGUE_HALL_TAILWIND, WEATHER_NONE, COMPOUND_STRING("Lance's Hall"), COMPOUND_STRING("Tailwind (both sides)")},
        {MAP_POKEMON_LEAGUE_CHAMPIONS_ROOM, NOTABLE_TRAINER_NONE, LEAGUE_HALL_NEUTRAL, WEATHER_NONE, NULL, COMPOUND_STRING("Neutral")},
    },
    [LEAGUE_ID_MASTERS - 1] = {
        {MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS, NOTABLE_TRAINER_WILL, LEAGUE_HALL_PSYCHIC_TERRAIN, WEATHER_NONE, COMPOUND_STRING("Will's Hall"), COMPOUND_STRING("Psychic Terrain")},
        {MAP_POKEMON_LEAGUE_KOGAS_ROOM_HNS, NOTABLE_TRAINER_KOGA, LEAGUE_HALL_GRASSY_TERRAIN, WEATHER_NONE, COMPOUND_STRING("Koga's Hall"), COMPOUND_STRING("Grassy Terrain")},
        {MAP_POKEMON_LEAGUE_BRUNOS_ROOM_HNS, NOTABLE_TRAINER_BRUNO, LEAGUE_HALL_STEALTH_ROCK, WEATHER_NONE, COMPOUND_STRING("Bruno's Hall"), COMPOUND_STRING("Stealth Rock (both sides)")},
        {MAP_POKEMON_LEAGUE_KARENS_ROOM_HNS, NOTABLE_TRAINER_KAREN, LEAGUE_HALL_WONDER_ROOM, WEATHER_NONE, COMPOUND_STRING("Karen's Hall"), COMPOUND_STRING("Wonder Room")},
        {MAP_POKEMON_LEAGUE_CHAMPIONS_ROOM_HNS, NOTABLE_TRAINER_NONE, LEAGUE_HALL_NEUTRAL, WEATHER_NONE, NULL, COMPOUND_STRING("Neutral")},
    },
    [LEAGUE_ID_HOENN - 1] = {
        {MAP_EVER_GRANDE_CITY_SIDNEYS_ROOM, NOTABLE_TRAINER_SIDNEY, LEAGUE_HALL_MAGIC_ROOM, WEATHER_NONE, COMPOUND_STRING("Sidney's Hall"), COMPOUND_STRING("Magic Room")},
        {MAP_EVER_GRANDE_CITY_PHOEBES_ROOM, NOTABLE_TRAINER_PHOEBE, LEAGUE_HALL_STICKY_WEB, WEATHER_NONE, COMPOUND_STRING("Phoebe's Hall"), COMPOUND_STRING("Sticky Web (both sides)")},
        {MAP_EVER_GRANDE_CITY_GLACIAS_ROOM, NOTABLE_TRAINER_GLACIA, LEAGUE_HALL_MISTY_TERRAIN, WEATHER_NONE, COMPOUND_STRING("Glacia's Hall"), COMPOUND_STRING("Misty Terrain")},
        {MAP_EVER_GRANDE_CITY_DRAKES_ROOM, NOTABLE_TRAINER_DRAKE, LEAGUE_HALL_SEA_OF_FIRE, WEATHER_NONE, COMPOUND_STRING("Drake's Hall"), COMPOUND_STRING("Sea of Fire (both sides)")},
        {MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM, NOTABLE_TRAINER_NONE, LEAGUE_HALL_NEUTRAL, WEATHER_NONE, NULL, COMPOUND_STRING("Neutral")},
    },
};

static const u16 sRoomCarriers[3][LEAGUE_LINEUP_SIZE] =
{
    [LEAGUE_ID_INDIGO - 1] = {
        TRAINER_WAYFARER_INDIGO_LORELEI, TRAINER_WAYFARER_INDIGO_BRUNO,
        TRAINER_WAYFARER_INDIGO_AGATHA, TRAINER_WAYFARER_INDIGO_LANCE,
        TRAINER_WAYFARER_INDIGO_BLUE,
    },
    [LEAGUE_ID_MASTERS - 1] = {
        TRAINER_WILL_2_HNS, TRAINER_KOGA_2_HNS, TRAINER_BRUNO_2_HNS,
        TRAINER_KAREN_2_HNS, TRAINER_LANCE_2_HNS,
    },
    [LEAGUE_ID_HOENN - 1] = {
        TRAINER_SIDNEY, TRAINER_PHOEBE, TRAINER_GLACIA, TRAINER_DRAKE,
        TRAINER_WALLACE,
    },
};

TEST("All 15 league halls follow their real room chains and map weather")
{
    enum LeagueId league;
    u8 match;
    u16 conditions = 0;

    EXPECT(ValidateLeagueHallRegistry());
    EXPECT(GetLeagueHall(LEAGUE_ID_NONE, 0) == NULL);
    EXPECT(GetLeagueHall(LEAGUE_ID_INDIGO, LEAGUE_LINEUP_SIZE) == NULL);
    for (league = LEAGUE_ID_INDIGO; league <= LEAGUE_ID_HOENN; league++)
        for (match = 0; match < LEAGUE_LINEUP_SIZE; match++)
        {
            const struct ExpectedHall *expected = &sExpectedHalls[league - 1][match];
            const struct LeagueHall *hall = GetLeagueHall(league, match);
            const struct MapHeader *map = Overworld_GetMapHeaderByGroupAndId(
                MAP_GROUP(expected->room), MAP_NUM(expected->room));

            EXPECT(hall != NULL);
            EXPECT_EQ(hall->room, expected->room);
            EXPECT_EQ(hall->honours, expected->honours);
            EXPECT_EQ(hall->condition, expected->condition);
            EXPECT_EQ(hall->weather, expected->weather);
            EXPECT_EQ(map->weather, expected->weather);
            EXPECT_EQ(StringCompare(hall->conditionName, expected->conditionName), 0);
            if (expected->name == NULL)
                EXPECT(hall->name == NULL);
            else
                EXPECT_EQ(StringCompare(hall->name, expected->name), 0);
            if (hall->condition != LEAGUE_HALL_NEUTRAL)
            {
                EXPECT(!(conditions & (1 << hall->condition)));
                conditions |= 1 << hall->condition;
            }
        }
    EXPECT_EQ(conditions, ((1 << LEAGUE_HALL_CONDITION_COUNT) - 2));
}

TEST("Hall starting statuses are temporary where required and hit both sides")
{
    enum LeagueId league;
    u8 match;
    struct StartingStatuses status;

    for (league = LEAGUE_ID_INDIGO; league <= LEAGUE_ID_HOENN; league++)
        for (match = 0; match < LEAGUE_LINEUP_SIZE; match++)
        {
            const struct LeagueHall *hall = GetLeagueHall(league, match);
            EXPECT(BuildLeagueHallStartingStatuses(hall, &status));
            switch (hall->condition)
            {
            case LEAGUE_HALL_NEUTRAL:
            case LEAGUE_HALL_SNOW:
            case LEAGUE_HALL_SANDSTORM:
                EXPECT_EQ(memcmp(&status, &(struct StartingStatuses){0}, sizeof(status)), 0);
                break;
            case LEAGUE_HALL_TRICK_ROOM: EXPECT(status.trickRoomTemporary && !status.trickRoom); break;
            case LEAGUE_HALL_MAGIC_ROOM: EXPECT(status.magicRoomTemporary && !status.magicRoom); break;
            case LEAGUE_HALL_WONDER_ROOM: EXPECT(status.wonderRoomTemporary && !status.wonderRoom); break;
            case LEAGUE_HALL_PSYCHIC_TERRAIN: EXPECT(status.psychicTerrainTemporary && !status.psychicTerrain); break;
            case LEAGUE_HALL_GRASSY_TERRAIN: EXPECT(status.grassyTerrainTemporary && !status.grassyTerrain); break;
            case LEAGUE_HALL_MISTY_TERRAIN: EXPECT(status.mistyTerrainTemporary && !status.mistyTerrain); break;
            case LEAGUE_HALL_TAILWIND:
                EXPECT(status.tailwindPlayerTemporary && status.tailwindOpponentTemporary);
                EXPECT(!status.tailwindPlayer && !status.tailwindOpponent);
                break;
            case LEAGUE_HALL_SEA_OF_FIRE:
                EXPECT(status.seaOfFirePlayerTemporary && status.seaOfFireOpponentTemporary);
                EXPECT(!status.seaOfFirePlayer && !status.seaOfFireOpponent);
                break;
            case LEAGUE_HALL_STICKY_WEB: EXPECT(status.stickyWebPlayer && status.stickyWebOpponent); break;
            case LEAGUE_HALL_STEALTH_ROCK: EXPECT(status.stealthRockPlayer && status.stealthRockOpponent); break;
            default: EXPECT(FALSE); break;
            }
        }
    EXPECT(!BuildLeagueHallStartingStatuses(NULL, &status));
    EXPECT(!BuildLeagueHallStartingStatuses(GetLeagueHall(LEAGUE_ID_INDIGO, 0), NULL));
}

TEST("Every accepted league room binds its own hall before the selected opponent battle")
{
    static const enum CircuitStage stages[] = {
        CIRCUIT_STAGE_INDIGO, CIRCUIT_STAGE_HOENN, CIRCUIT_STAGE_MASTERS,
    };
    struct StartingStatuses expected, prepared;
    const struct LeagueSavedTeam *team;
    const struct LeagueHall *hall;
    u8 stageIndex, match, badge;
    bool32 selectedDiffersFromHonouree = FALSE;

    WayfarerInitPersistentState();
    for (badge = 0; badge < 8; badge++)
    {
        SetBadgeStateForRegion(REGION_KANTO, badge, TRUE);
        SetBadgeStateForRegion(REGION_HOENN, badge, TRUE);
    }
    SetTrainerRating(160);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gIsDebugBattle = FALSE;

    for (stageIndex = 0; stageIndex < ARRAY_COUNT(stages); stageIndex++)
    {
        enum CircuitStage stage = stages[stageIndex];
        EXPECT(Test_AdmitCircuitRun(stage));
        for (match = 0; match < LEAGUE_LINEUP_SIZE; match++)
        {
            hall = GetLeagueHall((enum LeagueId)stage, match);
            Test_SetLeagueMap(&gSaveBlock1Ptr->location, hall->room);
            if (stageIndex == 0 && match == 0)
            {
                // The same room cannot grant a hall effect to a debug or
                // unrelated story battle.
                gIsDebugBattle = TRUE;
                InitTrainerBattleParameter();
                TRAINER_BATTLE_PARAM.mode = TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT;
                TRAINER_BATTLE_PARAM.opponentA = sRoomCarriers[stage - 1][match];
                BattleSetup_ConfigureTrainerBattle(NULL);
                EXPECT(!GetPreparedLeagueHallStartingStatuses(&prepared));
                gIsDebugBattle = FALSE;
                InitTrainerBattleParameter();
                TRAINER_BATTLE_PARAM.mode = TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT;
                TRAINER_BATTLE_PARAM.opponentA = TRAINER_BROCK_HNS;
                BattleSetup_ConfigureTrainerBattle(NULL);
                EXPECT(!GetPreparedLeagueHallStartingStatuses(&prepared));
            }
            InitTrainerBattleParameter();
            EXPECT(!GetPreparedLeagueHallStartingStatuses(&prepared));
            TRAINER_BATTLE_PARAM.mode = TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT;
            TRAINER_BATTLE_PARAM.opponentA = sRoomCarriers[stage - 1][match];
            BattleSetup_ConfigureTrainerBattle(NULL);
            EXPECT(IsLeagueEventBattleInProgress());
            EXPECT(GetAcceptedLeagueEventMember(match, &team));
            EXPECT_EQ(TRAINER_BATTLE_PARAM.opponentA, team->sourceTrainerId);
            EXPECT(GetPreparedLeagueHallStartingStatuses(&prepared));
            EXPECT(BuildLeagueHallStartingStatuses(hall, &expected));
            EXPECT_EQ(memcmp(&prepared, &expected, sizeof(prepared)), 0);
            if (hall->honours != NOTABLE_TRAINER_NONE
             && team->characterId != hall->honours)
                selectedDiffersFromHonouree = TRUE;

            ResetLeagueEventBattleProof();
            SetLeagueEventBattleVictoryForTesting(GetAcceptedLeagueEventId(), match);
            EXPECT(RecordCircuitRoomVictory(stage, match));
        }
        Test_SetLeagueMap(&gSaveBlock1Ptr->location, stage == CIRCUIT_STAGE_INDIGO
            ? MAP_POKEMON_LEAGUE_HALL_OF_FAME : stage == CIRCUIT_STAGE_HOENN
            ? MAP_EVER_GRANDE_CITY_HALL_OF_FAME : MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS);
        EXPECT(CanCompleteCircuitRun(stage));
        EXPECT_EQ(CommitCircuitRun(stage), CIRCUIT_COMMIT_FIRST_CLEAR);
    }
    EXPECT(selectedDiffersFromHonouree);
    EXPECT(!GetPreparedLeagueHallStartingStatuses(&prepared));
}

#endif

#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "data.h"
#include "debug.h"
#include "difficulty.h"
#include "event_data.h"
#include "pokemon.h"
#include "test/test.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "constants/maps.h"

#if IS_WAYFARER

TEST("League tiers create the same authored Pokemon across circuit facts and difficulty")
{
    static const struct
    {
        u16 trainer;
        u8 levels[PARTY_SIZE];
    } sLeagueParties[] =
    {
        { TRAINER_WILL_1_HNS, { 48, 49, 48, 49, 50 } },
        { TRAINER_KOGA_1_HNS, { 50, 51, 50, 51, 52 } },
        { TRAINER_BRUNO_1_HNS, { 53, 52, 53, 53, 54 } },
        { TRAINER_KAREN_1_HNS, { 54, 53, 53, 53, 55 } },
        { TRAINER_LANCE_1_HNS, { 54, 55, 54, 55, 54, 56 } },
        { TRAINER_WILL_2_HNS, { 66, 67, 67, 67, 66, 68 } },
        { TRAINER_KOGA_2_HNS, { 67, 67, 67, 67, 67, 68 } },
        { TRAINER_BRUNO_2_HNS, { 67, 68, 67, 68, 67, 68 } },
        { TRAINER_KAREN_2_HNS, { 68, 67, 68, 68, 67, 69 } },
        { TRAINER_LANCE_2_HNS, { 69, 68, 69, 68, 69, 70 } },
        { TRAINER_SIDNEY, { 86, 86, 86, 86, 87 } },
        { TRAINER_PHOEBE, { 87, 87, 88, 87, 88 } },
        { TRAINER_GLACIA, { 88, 88, 89, 89, 89 } },
        { TRAINER_DRAKE, { 89, 90, 89, 90, 90 } },
        { TRAINER_WALLACE, { 91, 90, 91, 91, 91, 92 } },
    };
    static const struct
    {
        u8 badges[3];
        u8 clears;
        u8 rating;
        u8 mapGroup;
        u8 mapNum;
    } sFacts[] =
    {
        { { 0, 0, 0 }, 0, 0, MAP_GROUP(MAP_PALLET_TOWN_HNS), MAP_NUM(MAP_PALLET_TOWN_HNS) },
        { { 0, 4, 4 }, 0, 40, MAP_GROUP(MAP_NEW_BARK_TOWN_HNS), MAP_NUM(MAP_NEW_BARK_TOWN_HNS) },
        { { 8, 0, 8 }, 1, 63, MAP_GROUP(MAP_SLATEPORT_CITY), MAP_NUM(MAP_SLATEPORT_CITY) },
        { { 8, 8, 8 }, 3, 76, MAP_GROUP(MAP_PALLET_TOWN_HNS), MAP_NUM(MAP_PALLET_TOWN_HNS) },
        { { 0, 0, 0 }, 7, 80, MAP_GROUP(MAP_NEW_BARK_TOWN_HNS), MAP_NUM(MAP_NEW_BARK_TOWN_HNS) },
    };
    u32 fact, difficulty, row, region, badge, mon, move;

    gIsDebugBattle = FALSE;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    for (fact = 0; fact < ARRAY_COUNT(sFacts); fact++)
    {
        for (region = 0; region < 3; region++)
        {
            for (badge = 0; badge < 8; badge++)
                SetBadgeStateForRegion(REGION_KANTO + region, badge, badge < sFacts[fact].badges[region]);
            SetChampionStateForRegion(REGION_KANTO + region, (sFacts[fact].clears & (1 << region)) != 0);
        }
        SetTrainerRating(sFacts[fact].rating);
        gSaveBlock1Ptr->location.mapGroup = sFacts[fact].mapGroup;
        gSaveBlock1Ptr->location.mapNum = sFacts[fact].mapNum;
        for (difficulty = DIFFICULTY_MIN; difficulty <= DIFFICULTY_MAX; difficulty++)
        {
            SetCurrentDifficultyLevel(difficulty);
            for (row = 0; row < ARRAY_COUNT(sLeagueParties); row++)
            {
                const struct Trainer *trainer = GetTrainerStructFromId(sLeagueParties[row].trainer);
                EXPECT_EQ(GetTrainerDifficultyLevel(sLeagueParties[row].trainer), DIFFICULTY_NORMAL);
                EXPECT(trainer == &gTrainers[DIFFICULTY_NORMAL][sLeagueParties[row].trainer]);
                EXPECT(trainer->party != NULL);
                EXPECT_EQ(trainer->partySize, sLeagueParties[row].levels[5] == 0 ? 5 : 6);
                EXPECT_EQ(CreateNPCTrainerPartyFromTrainer(gEnemyParty, trainer, TRUE, BATTLE_TYPE_TRAINER), trainer->partySize);
                for (mon = 0; mon < trainer->partySize; mon++)
                {
                    EXPECT_EQ(GetMonData(&gEnemyParty[mon], MON_DATA_LEVEL), sLeagueParties[row].levels[mon]);
                    EXPECT_EQ(GetMonData(&gEnemyParty[mon], MON_DATA_SPECIES), trainer->party[mon].species);
                    EXPECT_EQ(GetMonData(&gEnemyParty[mon], MON_DATA_HELD_ITEM), trainer->party[mon].heldItem);
                    for (move = 0; move < MAX_MON_MOVES; move++)
                        EXPECT_EQ(GetMonData(&gEnemyParty[mon], MON_DATA_MOVE1 + move), trainer->party[mon].moves[move]);
                }
            }
        }
    }
}

#endif

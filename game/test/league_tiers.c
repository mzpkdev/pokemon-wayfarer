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
#include "trainer_party_scaling.h"
#include "league_circuit.h"
#include "malloc.h"
#include "random.h"
#include "randomizer.h"
#include "wayfarer_persistence.h"
#include "constants/maps.h"

#if IS_WAYFARER

TEST("League tiers keep authored fallback without an admitted run across difficulty")
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

    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = FALSE;
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

static const u16 sLeagueIds[] = {
    TRAINER_WILL_1_HNS, TRAINER_KOGA_1_HNS, TRAINER_BRUNO_1_HNS, TRAINER_KAREN_1_HNS, TRAINER_LANCE_1_HNS,
    TRAINER_WILL_2_HNS, TRAINER_KOGA_2_HNS, TRAINER_BRUNO_2_HNS, TRAINER_KAREN_2_HNS, TRAINER_LANCE_2_HNS,
    TRAINER_SIDNEY, TRAINER_PHOEBE, TRAINER_GLACIA, TRAINER_DRAKE, TRAINER_WALLACE,
};

static const u16 sLeagueRoomMaps[][5] = {
    { MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS, MAP_POKEMON_LEAGUE_KOGAS_ROOM_HNS,
      MAP_POKEMON_LEAGUE_BRUNOS_ROOM_HNS, MAP_POKEMON_LEAGUE_KARENS_ROOM_HNS, MAP_POKEMON_LEAGUE_CHAMPIONS_ROOM_HNS },
    { MAP_EVER_GRANDE_CITY_SIDNEYS_ROOM, MAP_EVER_GRANDE_CITY_PHOEBES_ROOM,
      MAP_EVER_GRANDE_CITY_GLACIAS_ROOM, MAP_EVER_GRANDE_CITY_DRAKES_ROOM, MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM },
};

static void PrepareLeagueConstruction(u32 row, u32 rating)
{
    u32 region = REGION_KANTO + row / 5;
    u32 room = row % 5;
    u16 map = sLeagueRoomMaps[region == REGION_HOENN][room];
    for (u32 i = REGION_KANTO; i <= REGION_HOENN; i++)
        SetChampionStateForRegion(i, i < region);
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = TRUE;
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.region = region;
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry = rating;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(map);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(map);
    VarSet(VAR_LEAGUE_STATE, room + 1);
    VarSet(HOENN_VAR_ID(0x409C), min(room + 1, 4));
    for (u32 i = 0; i < 4; i++)
    {
        if (i < room)
            FlagSet(HOENN_FLAG_ID(0x4FB) + i);
        else
            FlagClear(HOENN_FLAG_ID(0x4FB) + i);
    }
    gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingIVs = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingEVs = FALSE;
    gIsDebugBattle = FALSE;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    ResetTrainerScalingSnapshot();
}

TEST("League scaling uses its own exact rounded curve and clamps after both offsets")
{
    static const u8 oracle[] = {
        15,15,16,16,16,17,17,18,18,19,19,20,21,21,22,22,23,
        24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,32,
        34,35,36,37,38,40,41,42,43,44,46,47,48,49,50,52,
        53,54,55,56,58,59,60,62,64,66,68,70,72,74,76,78,
        80,81,83,84,85,87,88,89,91,92,93,95,96,97,99,100,
    };
    EXPECT_EQ(ARRAY_COUNT(oracle), 81);
    for (u32 rating = 0; rating <= 80; rating++)
    {
        EXPECT_EQ(GetLeagueScalingBaseline(rating), oracle[rating]);
        for (u32 row = 0; row < ARRAY_COUNT(sLeagueIds); row++)
        {
            const struct LeagueScalingRoster *roster = GetLeagueScalingRoster(sLeagueIds[row], sLeagueIds[row], DIFFICULTY_NORMAL);
            EXPECT(roster != NULL);
            EXPECT_EQ(GetTrainerScalingPolicy(sLeagueIds[row]), TRAINER_SCALING_LEAGUE);
            for (u32 slot = 0; slot < roster->count; slot++)
            {
                s32 expected = oracle[rating] + roster->encounterOffset + roster->offsets[slot];
                EXPECT_EQ(GetLeagueScalingLevel(rating, roster->encounterOffset, roster->offsets[slot]), min(max(expected, 1), 100));
                if (rating)
                    EXPECT(GetLeagueScalingLevel(rating, roster->encounterOffset, roster->offsets[slot]) >= GetLeagueScalingLevel(rating - 1, roster->encounterOffset, roster->offsets[slot]));
            }
        }
    }
    EXPECT_EQ(GetLeagueScalingBaseline(UINT_MAX), 100);
    EXPECT_EQ(GetLeagueScalingLevel(80, 1, -1), 100);
    EXPECT_EQ(GetLeagueScalingLevel(0, -100, -100), 1);
    EXPECT(GetLeagueScalingRoster(TRAINER_LANCE_2_HNS, TRAINER_LANCE_1_HNS, DIFFICULTY_NORMAL) == NULL);
    EXPECT(GetLeagueScalingRoster(TRAINER_LANCE_2_HNS, TRAINER_LANCE_2_HNS, DIFFICULTY_HARD) == NULL);
}

TEST("League construction and reconstruction preserve authored identity at every TR and difficulty")
{
    u32 difficulty = DIFFICULTY_NORMAL, row = 0, firstRating = 0;
    for (u32 d = DIFFICULTY_MIN; d <= DIFFICULTY_MAX; d++)
        for (u32 r = 0; r < ARRAY_COUNT(sLeagueIds); r++)
            for (u32 first = 0; first <= 80; first += 9)
                PARAMETRIZE { difficulty = d; row = r; firstRating = first; }
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    struct Pokemon *authored = AllocZeroed(PARTY_SIZE * sizeof(*authored));
    static const u8 fields[] = {
        MON_DATA_SPECIES, MON_DATA_HELD_ITEM, MON_DATA_PERSONALITY,
        MON_DATA_IVS, MON_DATA_ABILITY_NUM, MON_DATA_FRIENDSHIP, MON_DATA_POKEBALL,
        MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV, MON_DATA_SPEED_EV, MON_DATA_SPATK_EV, MON_DATA_SPDEF_EV,
        MON_DATA_DYNAMAX_LEVEL, MON_DATA_GIGANTAMAX_FACTOR, MON_DATA_TERA_TYPE,
    };
    {
        SetCurrentDifficultyLevel(difficulty);
        const struct Trainer *trainer = GetTrainerStructFromId(sLeagueIds[row]);
        const struct LeagueScalingRoster *roster = GetLeagueScalingRoster(sLeagueIds[row], sLeagueIds[row], GetTrainerDifficultyLevel(sLeagueIds[row]));
        EXPECT_EQ(GetTrainerDifficultyLevel(sLeagueIds[row]), DIFFICULTY_NORMAL);
        EXPECT(IsLeagueScalingRosterValid(roster, trainer->party, trainer->partySize));
        for (u32 rating = firstRating; rating < firstRating + 9; rating++)
        {
            PrepareLeagueConstruction(row, rating);
            EXPECT(ValidateActiveLeagueRun());
            SeedRng(123);
            CreateNPCTrainerPartyFromTrainer(authored, trainer, TRUE, BATTLE_TYPE_TRAINER);
            SetTrainerRating(80 - rating);
            u32 liveRating = GetTrainerRating();
            for (u32 reconstruction = 0; reconstruction < 2; reconstruction++)
            {
                ResetTrainerScalingSnapshot();
                SeedRng(123);
                EXPECT_EQ(CreateNPCTrainerPartyForOpponent(party, sLeagueIds[row], TRUE, BATTLE_TYPE_TRAINER), trainer->partySize);
                EXPECT_EQ(GetTrainerRating(), liveRating);
                for (u32 slot = 0; slot < trainer->partySize; slot++)
                {
                    u32 expected = B_LEAGUE_SCALING ? GetLeagueScalingLevel(rating, roster->encounterOffset, roster->offsets[slot]) : trainer->party[slot].lvl;
                    EXPECT_EQ(GetMonData(&party[slot], MON_DATA_LEVEL), expected);
                    for (u32 field = 0; field < ARRAY_COUNT(fields); field++)
                        EXPECT_EQ(GetMonData(&party[slot], fields[field]), GetMonData(&authored[slot], fields[field]));
                    for (u32 move = 0; move < MAX_MON_MOVES; move++)
                        EXPECT_EQ(GetMonData(&party[slot], MON_DATA_MOVE1 + move), trainer->party[slot].moves[move]);
                }
            }
        }
    }
    Free(authored);
    Free(party);
}

TEST("League scaling rejects invalid roster metadata and run context")
{
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    const struct Trainer *trainer = GetTrainerStructFromId(TRAINER_WILL_1_HNS);
    const struct LeagueScalingRoster *roster = GetLeagueScalingRoster(TRAINER_WILL_1_HNS, TRAINER_WILL_1_HNS, DIFFICULTY_NORMAL);
    struct TrainerMon changed[PARTY_SIZE];
    memcpy(changed, trainer->party, trainer->partySize * sizeof(*changed));
    EXPECT(!IsLeagueScalingRosterValid(roster, changed, trainer->partySize - 1));
    changed[0].moves[0] = MOVE_SPLASH;
    EXPECT(!IsLeagueScalingRosterValid(roster, changed, trainer->partySize));
    for (u32 context = 0; context < 3; context++)
    {
        PrepareLeagueConstruction(0, 0);
        if (context == 0)
            gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = FALSE;
        else if (context == 1)
            gIsDebugBattle = TRUE;
        else
            VarSet(VAR_LEAGUE_STATE, 3);
        trainer = GetTrainerStructFromId(TRAINER_WILL_1_HNS);
        EXPECT_EQ(CreateNPCTrainerPartyForOpponent(party, TRAINER_WILL_1_HNS, TRUE, BATTLE_TYPE_TRAINER), trainer->partySize);
        EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), trainer->party[0].lvl);
    }
    Free(party);
}

TEST("League species randomizer preserves the complete authored constructor bypass")
{
#if RANDOMIZER_AVAILABLE
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    struct Pokemon *authored = AllocZeroed(PARTY_SIZE * sizeof(*authored));
    for (u32 row = 0; row < ARRAY_COUNT(sLeagueIds); row++)
    {
        PrepareLeagueConstruction(row, 0);
        gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = TRUE;
        const struct Trainer *trainer = GetTrainerStructFromId(sLeagueIds[row]);
        SeedRng(123);
        CreateNPCTrainerPartyFromTrainer(authored, trainer, TRUE, BATTLE_TYPE_TRAINER);
        SeedRng(123);
        EXPECT_EQ(CreateNPCTrainerPartyForOpponent(party, sLeagueIds[row], TRUE, BATTLE_TYPE_TRAINER), trainer->partySize);
        for (u32 slot = 0; slot < trainer->partySize; slot++)
        {
            EXPECT_EQ(GetMonData(&party[slot], MON_DATA_SPECIES), GetMonData(&authored[slot], MON_DATA_SPECIES));
            EXPECT_EQ(GetMonData(&party[slot], MON_DATA_LEVEL), trainer->party[slot].lvl);
            for (u32 move = 0; move < MAX_MON_MOVES; move++)
                EXPECT_EQ(GetMonData(&party[slot], MON_DATA_MOVE1 + move), GetMonData(&authored[slot], MON_DATA_MOVE1 + move));
        }
    }
    Free(authored);
    Free(party);
#endif
}

#elif !IS_FRLG

TEST("Standalone League constructor keeps authored parties at every difficulty")
{
#if IS_HNS
    static const u16 ids[] = {
        TRAINER_WILL_1_HNS, TRAINER_KOGA_1_HNS, TRAINER_BRUNO_1_HNS, TRAINER_KAREN_1_HNS, TRAINER_LANCE_1_HNS,
        TRAINER_WILL_2_HNS, TRAINER_KOGA_2_HNS, TRAINER_BRUNO_2_HNS, TRAINER_KAREN_2_HNS, TRAINER_LANCE_2_HNS,
    };
#else
    static const u16 ids[] = {TRAINER_SIDNEY, TRAINER_PHOEBE, TRAINER_GLACIA, TRAINER_DRAKE, TRAINER_WALLACE};
#endif
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    struct Pokemon *authored = AllocZeroed(PARTY_SIZE * sizeof(*authored));
    gIsDebugBattle = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingIVs = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingEVs = FALSE;
    for (u32 difficulty = DIFFICULTY_MIN; difficulty <= DIFFICULTY_MAX; difficulty++)
    {
        SetCurrentDifficultyLevel(difficulty);
        for (u32 row = 0; row < ARRAY_COUNT(ids); row++)
        {
            const struct Trainer *trainer = GetTrainerStructFromId(ids[row]);
            EXPECT(trainer->party != NULL);
            EXPECT_EQ(GetTrainerScalingPolicy(ids[row]), TRAINER_SCALING_EXCLUDED);
            SeedRng(123);
            CreateNPCTrainerPartyFromTrainer(authored, trainer, TRUE, BATTLE_TYPE_TRAINER);
            SeedRng(123);
            EXPECT_EQ(CreateNPCTrainerPartyForOpponent(party, ids[row], TRUE, BATTLE_TYPE_TRAINER), trainer->partySize);
            for (u32 slot = 0; slot < trainer->partySize; slot++)
            {
                EXPECT_EQ(GetMonData(&party[slot], MON_DATA_LEVEL), trainer->party[slot].lvl);
                EXPECT_EQ(GetMonData(&party[slot], MON_DATA_SPECIES), trainer->party[slot].species);
                EXPECT_EQ(GetMonData(&party[slot], MON_DATA_HELD_ITEM), trainer->party[slot].heldItem);
                EXPECT_EQ(GetMonData(&party[slot], MON_DATA_PERSONALITY), GetMonData(&authored[slot], MON_DATA_PERSONALITY));
                EXPECT_EQ(GetMonData(&party[slot], MON_DATA_IVS), GetMonData(&authored[slot], MON_DATA_IVS));
                EXPECT_EQ(GetMonData(&party[slot], MON_DATA_ABILITY_NUM), GetMonData(&authored[slot], MON_DATA_ABILITY_NUM));
                for (u32 move = 0; move < MAX_MON_MOVES; move++)
                    EXPECT_EQ(GetMonData(&party[slot], MON_DATA_MOVE1 + move), GetMonData(&authored[slot], MON_DATA_MOVE1 + move));
            }
        }
    }
    Free(authored);
    Free(party);
}

#endif

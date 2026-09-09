#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "constants/flags.h"
#include "test/test.h"
#include "constants/opponents.h"

TEST("Trainer defeat helpers reject sentinel and out-of-range IDs")
{
    SetTrainerFlag(TRAINER_NONE);
    SetTrainerFlag(0xFFFF);
    SetTrainerFlag(TRAINERS_COUNT);

    EXPECT(!HasTrainerBeenFought(TRAINER_NONE));
    EXPECT(!HasTrainerBeenFought(0xFFFF));
    EXPECT(!HasTrainerBeenFought(TRAINERS_COUNT));

    ClearTrainerFlag(TRAINER_NONE);
    ClearTrainerFlag(0xFFFF);
    ClearTrainerFlag(TRAINERS_COUNT);
}

#if IS_WAYFARER

TEST("Wayfarer Trainer IDs keep HNS stable and map Hoenn after it")
{
    EXPECT_EQ(TRAINER_BEVERLY_5_HNS, 630);
    EXPECT_EQ(TRAINER_SAWYER_1, 639);
    EXPECT_EQ(TRAINER_MAY_PLACEHOLDER, 1492);
    EXPECT_EQ(TRAINERS_COUNT, TRAINERS_COUNT_WAYFARER);
    EXPECT_EQ(TRAINER_FALKNER_POSTOBC_HNS, 1493);
    EXPECT_EQ(TRAINER_ERIKA_POSTOBC_HNS, 1514);
    EXPECT_EQ(TRAINER_SS_ANNE_YOUNGSTER_TYLER_HNS, 1515);
    EXPECT_EQ(TRAINER_SS_ANNE_GENTLEMAN_LAMAR_HNS, 1530);
    EXPECT_EQ(TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS, 1531);
    EXPECT_EQ(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS, 1543);
    EXPECT_EQ(TRAINER_MT_MOON_ROCKET_GRUNT_1_HNS, 1544);
    EXPECT_EQ(TRAINER_MT_MOON_MIGUEL_HNS, 1548);
    EXPECT_EQ(TRAINER_CERULEAN_BURGLARY_GRUNT_HNS, 1549);
    EXPECT_EQ(TRAINER_NUGGET_BRIDGE_CALE_HNS, 1550);
    EXPECT_EQ(TRAINER_NUGGET_BRIDGE_ROCKET_HNS, 1555);
    EXPECT_EQ(TRAINER_SILPH_GRUNT_23_HNS, 1556);
    EXPECT_EQ(TRAINER_SILPH_GIOVANNI_HNS, 1586);
    EXPECT_EQ(TRAINERS_COUNT_WAYFARER, 1587);
    EXPECT_EQ(MAX_TRAINERS_COUNT, MAX_TRAINERS_COUNT_WAYFARER);
    EXPECT_EQ(TRAINER_PARTNER(PARTNER_NONE), 2048);
}

TEST("Wayfarer HNS and Hoenn Trainer defeat state is isolated")
{
    ClearTrainerFlag(TRAINER_BEVERLY_5_HNS);
    ClearTrainerFlag(TRAINER_SAWYER_1);

    SetTrainerFlag(TRAINER_BEVERLY_5_HNS);
    EXPECT(HasTrainerBeenFought(TRAINER_BEVERLY_5_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_SAWYER_1));

    SetTrainerFlag(TRAINER_SAWYER_1);
    EXPECT(HasTrainerBeenFought(TRAINER_BEVERLY_5_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_SAWYER_1));

    ClearTrainerFlag(TRAINER_BEVERLY_5_HNS);
    EXPECT(!HasTrainerBeenFought(TRAINER_BEVERLY_5_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_SAWYER_1));
}

TEST("Wayfarer appended HNS rematch defeat flags are isolated from Hoenn")
{
    ClearTrainerFlag(TRAINER_FALKNER_POSTOBC_HNS);
    ClearTrainerFlag(TRAINER_ERIKA_POSTOBC_HNS);
    ClearTrainerFlag(TRAINER_SAWYER_1);
    ClearTrainerFlag(TRAINER_MAY_PLACEHOLDER);
    ClearTrainerFlag(TRAINER_BLUE_DOJO_HNS);

    SetTrainerFlag(TRAINER_FALKNER_POSTOBC_HNS);
    SetTrainerFlag(TRAINER_ERIKA_POSTOBC_HNS);
    EXPECT(HasTrainerBeenFought(TRAINER_FALKNER_POSTOBC_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_ERIKA_POSTOBC_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_SAWYER_1));
    EXPECT(!HasTrainerBeenFought(TRAINER_MAY_PLACEHOLDER));
    EXPECT(!HasTrainerBeenFought(TRAINER_BLUE_DOJO_HNS));

    SetTrainerFlag(TRAINER_SAWYER_1);
    SetTrainerFlag(TRAINER_MAY_PLACEHOLDER);
    ClearTrainerFlag(TRAINER_FALKNER_POSTOBC_HNS);
    EXPECT(!HasTrainerBeenFought(TRAINER_FALKNER_POSTOBC_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_ERIKA_POSTOBC_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_SAWYER_1));
    EXPECT(HasTrainerBeenFought(TRAINER_MAY_PLACEHOLDER));
    ClearTrainerFlag(TRAINER_ERIKA_POSTOBC_HNS);
    EXPECT(!HasTrainerBeenFought(TRAINER_ERIKA_POSTOBC_HNS));
}

TEST("Wayfarer S.S. Anne trainer defeat flags are compact HNS flags")
{
    static const u16 sAnneTrainers[] =
    {
        TRAINER_SS_ANNE_YOUNGSTER_TYLER_HNS,
        TRAINER_SS_ANNE_LASS_ANN_HNS,
        TRAINER_SS_ANNE_LASS_DAWN_HNS,
        TRAINER_SS_ANNE_SAILOR_EDMOND_HNS,
        TRAINER_SS_ANNE_SAILOR_TREVOR_HNS,
        TRAINER_SS_ANNE_SAILOR_LEONARD_HNS,
        TRAINER_SS_ANNE_SAILOR_DUNCAN_HNS,
        TRAINER_SS_ANNE_SAILOR_HUEY_HNS,
        TRAINER_SS_ANNE_SAILOR_DYLAN_HNS,
        TRAINER_SS_ANNE_SAILOR_PHILLIP_HNS,
        TRAINER_SS_ANNE_FISHERMAN_DALE_HNS,
        TRAINER_SS_ANNE_FISHERMAN_BARNY_HNS,
        TRAINER_SS_ANNE_GENTLEMAN_THOMAS_HNS,
        TRAINER_SS_ANNE_GENTLEMAN_ARTHUR_HNS,
        TRAINER_SS_ANNE_GENTLEMAN_BROOKS_HNS,
        TRAINER_SS_ANNE_GENTLEMAN_LAMAR_HNS,
    };
    u32 i;
    u32 j;

    ClearTrainerFlag(TRAINER_SAWYER_1);
    for (i = 0; i < ARRAY_COUNT(sAnneTrainers); i++)
    {
        EXPECT_EQ(sAnneTrainers[i], 1515 + i);
        ClearTrainerFlag(sAnneTrainers[i]);
    }

    // Every imported Anne team maps to a distinct compact HNS defeated flag.
    // The fixed Hoenn range must remain independent of each of those flags.
    for (i = 0; i < ARRAY_COUNT(sAnneTrainers); i++)
    {
        SetTrainerFlag(sAnneTrainers[i]);
        for (j = 0; j < ARRAY_COUNT(sAnneTrainers); j++)
            EXPECT_EQ(HasTrainerBeenFought(sAnneTrainers[j]), i == j);
        EXPECT(!HasTrainerBeenFought(TRAINER_SAWYER_1));
        ClearTrainerFlag(sAnneTrainers[i]);
    }

    SetTrainerFlag(TRAINER_SAWYER_1);
    for (i = 0; i < ARRAY_COUNT(sAnneTrainers); i++)
        EXPECT(!HasTrainerBeenFought(sAnneTrainers[i]));
    EXPECT(HasTrainerBeenFought(TRAINER_SAWYER_1));
    ClearTrainerFlag(TRAINER_SAWYER_1);
}

TEST("Wayfarer Celadon Hideout trainer defeat flags are compact HNS flags")
{
    static const u16 sCeladonHideoutTrainers[] =
    {
        TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_8_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_9_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_10_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_11_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_12_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_13_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_14_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_15_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_16_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_17_HNS,
        TRAINER_CELADON_HIDEOUT_GRUNT_18_HNS,
        TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS,
    };
    u32 i;
    u32 j;

    ClearTrainerFlag(TRAINER_SAWYER_1);
    ClearTrainerFlag(TRAINER_SS_ANNE_YOUNGSTER_TYLER_HNS);
    for (i = 0; i < ARRAY_COUNT(sCeladonHideoutTrainers); i++)
    {
        EXPECT_EQ(sCeladonHideoutTrainers[i], 1531 + i);
        ClearTrainerFlag(sCeladonHideoutTrainers[i]);
    }

    // Every imported Hideout team maps to a distinct compact HNS defeated
    // flag while Hoenn and the preceding Anne range remain independent.
    for (i = 0; i < ARRAY_COUNT(sCeladonHideoutTrainers); i++)
    {
        SetTrainerFlag(sCeladonHideoutTrainers[i]);
        for (j = 0; j < ARRAY_COUNT(sCeladonHideoutTrainers); j++)
            EXPECT_EQ(HasTrainerBeenFought(sCeladonHideoutTrainers[j]), i == j);
        EXPECT(!HasTrainerBeenFought(TRAINER_SS_ANNE_YOUNGSTER_TYLER_HNS));
        EXPECT(!HasTrainerBeenFought(TRAINER_SAWYER_1));
        ClearTrainerFlag(sCeladonHideoutTrainers[i]);
    }

    SetTrainerFlag(TRAINER_SAWYER_1);
    SetTrainerFlag(TRAINER_SS_ANNE_YOUNGSTER_TYLER_HNS);
    for (i = 0; i < ARRAY_COUNT(sCeladonHideoutTrainers); i++)
        EXPECT(!HasTrainerBeenFought(sCeladonHideoutTrainers[i]));
    EXPECT(HasTrainerBeenFought(TRAINER_SAWYER_1));
    EXPECT(HasTrainerBeenFought(TRAINER_SS_ANNE_YOUNGSTER_TYLER_HNS));
    ClearTrainerFlag(TRAINER_SAWYER_1);
    ClearTrainerFlag(TRAINER_SS_ANNE_YOUNGSTER_TYLER_HNS);
}

TEST("Wayfarer Cerulean burglary defeat state is independent")
{
    ClearTrainerFlag(TRAINER_CERULEAN_BURGLARY_GRUNT_HNS);
    ClearTrainerFlag(TRAINER_MT_MOON_MIGUEL_HNS);
    ClearTrainerFlag(TRAINER_SAWYER_1);
    SetTrainerFlag(TRAINER_CERULEAN_BURGLARY_GRUNT_HNS);
    EXPECT(HasTrainerBeenFought(TRAINER_CERULEAN_BURGLARY_GRUNT_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_MT_MOON_MIGUEL_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_SAWYER_1));
    ClearTrainerFlag(TRAINER_CERULEAN_BURGLARY_GRUNT_HNS);
    SetTrainerFlag(TRAINER_MT_MOON_MIGUEL_HNS);
    SetTrainerFlag(TRAINER_SAWYER_1);
    EXPECT(!HasTrainerBeenFought(TRAINER_CERULEAN_BURGLARY_GRUNT_HNS));
    ClearTrainerFlag(TRAINER_MT_MOON_MIGUEL_HNS);
    ClearTrainerFlag(TRAINER_SAWYER_1);
}

TEST("Wayfarer Nugget Bridge defeats are ordered independent trainer flags")
{
    static const u16 trainers[] = {
        TRAINER_NUGGET_BRIDGE_CALE_HNS, TRAINER_NUGGET_BRIDGE_ALI_HNS,
        TRAINER_NUGGET_BRIDGE_TIMMY_HNS, TRAINER_NUGGET_BRIDGE_RELI_HNS,
        TRAINER_NUGGET_BRIDGE_ETHAN_HNS, TRAINER_NUGGET_BRIDGE_ROCKET_HNS,
    };
    u32 i, j;
    FlagClear(FLAG_NUGGET_BRIDGE_NUGGET_RECEIVED_HNS);
    ClearTrainerFlag(TRAINER_CERULEAN_BURGLARY_GRUNT_HNS);
    ClearTrainerFlag(TRAINER_GRUNT_31_HNS);
    ClearTrainerFlag(TRAINER_SAWYER_1);
    for (i = 0; i < ARRAY_COUNT(trainers); i++)
    {
        EXPECT_EQ(trainers[i], 1550 + i);
        ClearTrainerFlag(trainers[i]);
    }
    for (i = 0; i < ARRAY_COUNT(trainers); i++)
    {
        SetTrainerFlag(trainers[i]);
        EXPECT(FlagGet(TRAINER_FLAGS_START + 696 + i));
        EXPECT(!FlagGet(FLAG_NUGGET_BRIDGE_NUGGET_RECEIVED_HNS));
        for (j = 0; j < ARRAY_COUNT(trainers); j++)
            EXPECT_EQ(HasTrainerBeenFought(trainers[j]), i == j);
        EXPECT(!HasTrainerBeenFought(TRAINER_CERULEAN_BURGLARY_GRUNT_HNS));
        EXPECT(!HasTrainerBeenFought(TRAINER_GRUNT_31_HNS));
        EXPECT(!HasTrainerBeenFought(TRAINER_SAWYER_1));
        ClearTrainerFlag(trainers[i]);
    }
}

TEST("Wayfarer Silph defeats and liberation receipts are isolated")
{
    u32 i, j;
    FlagClear(FLAG_SILPH_LIBERATED_HNS);
    FlagClear(FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS);
    FlagClear(FLAG_SILPH_MASTER_BALL_RECEIVED_HNS);
    ClearTrainerFlag(TRAINER_NUGGET_BRIDGE_ROCKET_HNS);
    for (i = 1556; i <= 1586; i++)
        ClearTrainerFlag(i);
    for (i = 1556; i <= 1586; i++)
    {
        SetTrainerFlag(i);
        EXPECT(FlagGet(TRAINER_FLAGS_START + 702 + i - 1556));
        for (j = 1556; j <= 1586; j++)
            EXPECT_EQ(HasTrainerBeenFought(j), i == j);
        EXPECT(!FlagGet(FLAG_SILPH_LIBERATED_HNS));
        EXPECT(!FlagGet(FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS));
        EXPECT(!FlagGet(FLAG_SILPH_MASTER_BALL_RECEIVED_HNS));
        EXPECT(!HasTrainerBeenFought(TRAINER_NUGGET_BRIDGE_ROCKET_HNS));
        ClearTrainerFlag(i);
    }
}

#endif

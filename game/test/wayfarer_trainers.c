#include "global.h"
#include "battle_setup.h"
#include "test/test.h"
#include "constants/opponents.h"
#include "wayfarer_sevii_trainer_defeats.h"

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
    EXPECT_EQ(TRAINER_WAYFARER_COAST_FIRST, 1651);
    EXPECT_EQ(TRAINERS_COUNT_WAYFARER, 1707);
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

TEST("Wayfarer Sevii Trainer defeat routing uses frozen allocation slots")
{
    EXPECT_EQ(WayfarerSeviiTrainerGetDefeatSlot(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_JOCELYN), 0);
    EXPECT_EQ(WayfarerSeviiTrainerGetDefeatSlot(TRAINER_WAYFARER_SEVII_LADY_SELPHY), 135);
    EXPECT_EQ(WayfarerSeviiTrainerGetDefeatSlot(TRAINER_WAYFARER_SEVII_FIRST - 1), WAYFARER_SEVII_TRAINER_DEFEAT_SLOT_NONE);
    EXPECT_EQ(WayfarerSeviiTrainerGetDefeatSlot(TRAINER_WAYFARER_SEVII_LAST + 1), WAYFARER_SEVII_TRAINER_DEFEAT_SLOT_NONE);

    ClearTrainerFlag(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_JOCELYN);
    ClearTrainerFlag(TRAINER_WAYFARER_SEVII_LADY_SELPHY);
    ClearTrainerFlag(TRAINER_BEVERLY_5_HNS);
    ClearTrainerFlag(TRAINER_SAWYER_1);
    SetTrainerFlag(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_JOCELYN);
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_JOCELYN));
    EXPECT(!HasTrainerBeenFought(TRAINER_WAYFARER_SEVII_LADY_SELPHY));
    EXPECT(!HasTrainerBeenFought(TRAINER_BEVERLY_5_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_SAWYER_1));

    SetTrainerFlag(TRAINER_WAYFARER_SEVII_LADY_SELPHY);
    SetTrainerFlag(TRAINER_BEVERLY_5_HNS);
    SetTrainerFlag(TRAINER_SAWYER_1);
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_SEVII_LADY_SELPHY));
    EXPECT(HasTrainerBeenFought(TRAINER_BEVERLY_5_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_SAWYER_1));
    ClearTrainerFlag(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_JOCELYN);
    EXPECT(!HasTrainerBeenFought(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_JOCELYN));
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_SEVII_LADY_SELPHY));
    EXPECT(HasTrainerBeenFought(TRAINER_BEVERLY_5_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_SAWYER_1));
}

TEST("Wayfarer Sevii rematch parties share their base Trainer defeat state")
{
    ClearTrainerFlag(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON);
    SetTrainerFlag(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON_2);
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON));
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON_2));

    ClearTrainerFlag(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON);
    EXPECT(!HasTrainerBeenFought(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON));
    EXPECT(!HasTrainerBeenFought(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON_2));
}

#endif

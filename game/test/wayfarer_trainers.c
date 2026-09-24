#include "global.h"
#include "battle_setup.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/opponents.h"
#include "wayfarer_ss_anne_trainer_defeats.h"
#include "wayfarer_sevii_trainer_defeats.h"
#include "wayfarer_celadon_hideout.h"
#include "constants/flags.h"
#include "constants/species.h"

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
    EXPECT_EQ(TRAINER_WAYFARER_SS_ANNE_FIRST, 1707);
    EXPECT_EQ(TRAINER_WAYFARER_SS_ANNE_LAST, 1722);
    EXPECT_EQ(TRAINER_WAYFARER_TOWER_FIRST, 1779);
    EXPECT_EQ(TRAINER_WAYFARER_TOWER_LAST, 1794);
    EXPECT_EQ(TRAINERS_COUNT_WAYFARER, 1795);
    EXPECT_EQ(TRAINER_CELADON_HIDEOUT_FIRST, 1723);
    EXPECT_EQ(TRAINER_CELADON_HIDEOUT_LAST, 1735);
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

TEST("Wayfarer S.S. Anne Trainer defeats use their isolated one-time flags")
{
    ClearTrainerFlag(TRAINER_WAYFARER_SS_ANNE_YOUNGSTER_TYLER);
    ClearTrainerFlag(TRAINER_WAYFARER_SS_ANNE_SAILOR_TREVOR);
    ClearTrainerFlag(TRAINER_BEVERLY_5_HNS);
    ClearTrainerFlag(TRAINER_SAWYER_1);

    EXPECT_EQ(WayfarerSSAnneTrainerGetDefeatSlot(TRAINER_WAYFARER_SS_ANNE_YOUNGSTER_TYLER), 0);
    EXPECT_EQ(WayfarerSSAnneTrainerGetDefeatSlot(TRAINER_WAYFARER_SS_ANNE_SAILOR_TREVOR), 15);
    EXPECT_EQ(WayfarerSSAnneTrainerGetDefeatSlot(TRAINER_WAYFARER_SS_ANNE_FIRST - 1), WAYFARER_SS_ANNE_TRAINER_DEFEAT_SLOT_NONE);
    EXPECT_EQ(WayfarerSSAnneTrainerGetDefeatSlot(TRAINER_WAYFARER_SS_ANNE_LAST + 1), WAYFARER_SS_ANNE_TRAINER_DEFEAT_SLOT_NONE);

    SetTrainerFlag(TRAINER_WAYFARER_SS_ANNE_YOUNGSTER_TYLER);
    SetTrainerFlag(TRAINER_WAYFARER_SS_ANNE_SAILOR_TREVOR);
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_SS_ANNE_YOUNGSTER_TYLER));
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_SS_ANNE_SAILOR_TREVOR));
    EXPECT(!HasTrainerBeenFought(TRAINER_BEVERLY_5_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_SAWYER_1));

    ClearTrainerFlag(TRAINER_WAYFARER_SS_ANNE_YOUNGSTER_TYLER);
    EXPECT(!HasTrainerBeenFought(TRAINER_WAYFARER_SS_ANNE_YOUNGSTER_TYLER));
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_SS_ANNE_SAILOR_TREVOR));
}

TEST("Pokemon Tower Trainer defeats use independent SaveBlock3 bits")
{
    gSaveBlock3Ptr->wayfarerTowerTrainerDefeats = 0;
    ClearTrainerFlag(TRAINER_WAYFARER_SS_ANNE_SAILOR_TREVOR);

    SetTrainerFlag(TRAINER_WAYFARER_TOWER_CHANNELER_PATRICIA);
    SetTrainerFlag(TRAINER_WAYFARER_TOWER_ROCKET_GRUNT_21);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerTowerTrainerDefeats, 0x8001);
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_TOWER_CHANNELER_PATRICIA));
    EXPECT(!HasTrainerBeenFought(TRAINER_WAYFARER_TOWER_CHANNELER_CARLY));
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_TOWER_ROCKET_GRUNT_21));
    EXPECT(!HasTrainerBeenFought(TRAINER_WAYFARER_SS_ANNE_SAILOR_TREVOR));

    ClearTrainerFlag(TRAINER_WAYFARER_TOWER_CHANNELER_PATRICIA);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerTowerTrainerDefeats, 0x8000);
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_TOWER_ROCKET_GRUNT_21));
    ClearTrainerFlag(TRAINER_WAYFARER_TOWER_ROCKET_GRUNT_21);
}

TEST("Wayfarer Celadon Hideout Trainer defeats use isolated local flags")
{
    ClearTrainerFlag(TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS);
    ClearTrainerFlag(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS);
    ClearTrainerFlag(TRAINER_WAYFARER_SS_ANNE_SAILOR_TREVOR);
    ClearTrainerFlag(TRAINER_SAWYER_1);

    EXPECT_EQ(WayfarerCeladonHideoutTrainerDefeatFlag(TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS), FLAG_WAYFARER_CELADON_HIDEOUT_TRAINER_GRUNT_7);
    EXPECT_EQ(WayfarerCeladonHideoutTrainerDefeatFlag(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS), FLAG_WAYFARER_CELADON_HIDEOUT_TRAINER_GIOVANNI);
    EXPECT_EQ(WayfarerCeladonHideoutTrainerDefeatFlag(TRAINER_CELADON_HIDEOUT_FIRST - 1), 0);
    EXPECT_EQ(WayfarerCeladonHideoutTrainerDefeatFlag(TRAINER_CELADON_HIDEOUT_LAST + 1), 0);

    SetTrainerFlag(TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS);
    SetTrainerFlag(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS);
    EXPECT(HasTrainerBeenFought(TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_WAYFARER_SS_ANNE_SAILOR_TREVOR));
    EXPECT(!HasTrainerBeenFought(TRAINER_SAWYER_1));

    ClearTrainerFlag(TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS);
    EXPECT(!HasTrainerBeenFought(TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS));
}

TEST("Wayfarer Celadon Hideout battle gate needs a conscious non-Egg party member")
{
    u16 hp = 0;
    bool8 egg = TRUE;

    ZeroPlayerPartyMons();
    EXPECT(!WayfarerCanStartOrdinaryBattleForScript());

    CreateMon(&gPlayerParty[0], SPECIES_RATTATA, 5, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&gPlayerParty[0]);
    EXPECT(WayfarerCanStartOrdinaryBattleForScript());
    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    EXPECT(!WayfarerCanStartOrdinaryBattleForScript());
    SetMonData(&gPlayerParty[0], MON_DATA_IS_EGG, &egg);
    hp = 1;
    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    EXPECT(!WayfarerCanStartOrdinaryBattleForScript());

    CreateMon(&gPlayerParty[1], SPECIES_PIDGEY, 5, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&gPlayerParty[1]);
    EXPECT(WayfarerCanStartOrdinaryBattleForScript());
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

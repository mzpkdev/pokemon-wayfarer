#include "global.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "trainer_tower.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/vars.h"

#if IS_WAYFARER
static void StartTowerRun(u8 challengeType)
{
    gSpecialVar_0x8004 = TRAINER_TOWER_FUNC_START_CHALLENGE;
    gSpecialVar_0x8005 = challengeType;
    CallTrainerTowerFunc();
}

static void CallTowerFunction(u8 functionId)
{
    gSpecialVar_0x8004 = functionId;
    CallTrainerTowerFunc();
}

static void SetUpUsableTowerParty(void)
{
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_PIKACHU, 31, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&gPlayerParty[0]);
    gPlayerPartyCount = 1;
}

static void ClearEveryTowerFloor(void)
{
    u8 floor;

    for (floor = 0; floor < MAX_TRAINER_TOWER_FLOORS; floor++)
    {
        gMapHeader.mapLayoutId = LAYOUT_TRAINER_TOWER_1F + floor;
        CallTowerFunction(TRAINER_TOWER_FUNC_INIT_FLOOR);
        CallTowerFunction(TRAINER_TOWER_FUNC_CLEARED_FLOOR);
    }
}

TEST("Wayfarer Trainer Tower clamps levels and maps source prizes")
{
    EXPECT_EQ(WayfarerTrainerTowerNormalizeLevel(0), 1);
    EXPECT_EQ(WayfarerTrainerTowerNormalizeLevel(1), 1);
    EXPECT_EQ(WayfarerTrainerTowerNormalizeLevel(100), 100);
    EXPECT_EQ(WayfarerTrainerTowerNormalizeLevel(255), 100);
    EXPECT_EQ(WayfarerTrainerTowerGetPrize(CHALLENGE_TYPE_SINGLE), ITEM_UP_GRADE);
    EXPECT_EQ(WayfarerTrainerTowerGetPrize(CHALLENGE_TYPE_DOUBLE), ITEM_DRAGON_SCALE);
    EXPECT_EQ(WayfarerTrainerTowerGetPrize(CHALLENGE_TYPE_KNOCKOUT), ITEM_METAL_COAT);
    EXPECT_EQ(WayfarerTrainerTowerGetPrize(CHALLENGE_TYPE_MIXED), ITEM_KINGS_ROCK);
}

TEST("Wayfarer Trainer Tower formats bounded source timer frames")
{
    u16 minutes;
    u8 seconds;
    u8 centiseconds;

    WayfarerTrainerTowerFormatTime(0, &minutes, &seconds, &centiseconds);
    EXPECT_EQ(minutes, 0);
    EXPECT_EQ(seconds, 0);
    EXPECT_EQ(centiseconds, 0);
    WayfarerTrainerTowerFormatTime(60 * 60 + 61, &minutes, &seconds, &centiseconds);
    EXPECT_EQ(minutes, 1);
    EXPECT_EQ(seconds, 1);
    EXPECT_EQ(centiseconds, 1);
    WayfarerTrainerTowerFormatTime(TRAINER_TOWER_MAX_TIME + 1, &minutes, &seconds, &centiseconds);
    EXPECT_EQ(minutes, 59);
    EXPECT_EQ(seconds, 59);
    EXPECT_EQ(centiseconds, 99);
}

TEST("Wayfarer Trainer Tower mixed format selects the frozen floor variants")
{
    static const u8 expected[MAX_TRAINER_TOWER_FLOORS] =
    {
        CHALLENGE_TYPE_SINGLE, CHALLENGE_TYPE_SINGLE, CHALLENGE_TYPE_SINGLE, CHALLENGE_TYPE_DOUBLE,
        CHALLENGE_TYPE_DOUBLE, CHALLENGE_TYPE_KNOCKOUT, CHALLENGE_TYPE_DOUBLE, CHALLENGE_TYPE_KNOCKOUT,
    };
    u8 floor;

    for (floor = 0; floor < MAX_TRAINER_TOWER_FLOORS; floor++)
        EXPECT_EQ(WayfarerTrainerTowerGetFloorChallengeType(CHALLENGE_TYPE_MIXED, floor), expected[floor]);
    EXPECT_EQ(WayfarerTrainerTowerGetFloorChallengeType(CHALLENGE_TYPE_MIXED, MAX_TRAINER_TOWER_FLOORS), NUM_TOWER_CHALLENGE_TYPES);
}

TEST("Wayfarer Trainer Tower retains an independent best record for every format")
{
    struct WayfarerSeviiTrainerTowerRecords records = {0};
    u8 format;

    for (format = 0; format < NUM_TOWER_CHALLENGE_TYPES; format++)
    {
        EXPECT(WayfarerTrainerTowerRecordTime(&records, format, 600 + format));
        EXPECT_EQ(records.bestTime[format], 600 + format);
        EXPECT(!WayfarerTrainerTowerRecordTime(&records, format, 601 + format));
        EXPECT(WayfarerTrainerTowerRecordTime(&records, format, 599 + format));
    }
    EXPECT_EQ(records.completedMask, (1 << NUM_TOWER_CHALLENGE_TYPES) - 1);
}

TEST("Wayfarer Trainer Tower restores its healed entry snapshot for loss and abandonment")
{
    u16 heldItem = ITEM_ORAN_BERRY;
    u16 noItem = ITEM_NONE;
    u16 hp = 1;

    WayfarerSeviiInitPersistentState();
    SetUpUsableTowerParty();
    SetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM, &heldItem);
    StartTowerRun(CHALLENGE_TYPE_SINGLE);
    EXPECT(!WayfarerTrainerTowerIsSaveAllowed());
    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    SetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM, &noItem);
    CallTowerFunction(TRAINER_TOWER_FUNC_SET_LOST);
    EXPECT(!WayfarerTrainerTowerIsChallengeActive());
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM), heldItem);
    EXPECT(GetMonData(&gPlayerParty[0], MON_DATA_HP) > hp);

    StartTowerRun(CHALLENGE_TYPE_SINGLE);
    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    SetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM, &noItem);
    CallTowerFunction(TRAINER_TOWER_FUNC_ABANDON_CHALLENGE);
    EXPECT(!WayfarerTrainerTowerIsChallengeActive());
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM), heldItem);
    EXPECT(GetMonData(&gPlayerParty[0], MON_DATA_HP) > hp);
}

TEST("Wayfarer Trainer Tower discards an unsaved run when transient state resets")
{
    u16 hp = 1;

    WayfarerSeviiInitPersistentState();
    SetUpUsableTowerParty();
    StartTowerRun(CHALLENGE_TYPE_SINGLE);
    EXPECT(WayfarerTrainerTowerIsChallengeActive());
    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);

    WayfarerTrainerTowerResetTransientState();

    EXPECT(!WayfarerTrainerTowerIsChallengeActive());
    EXPECT(WayfarerTrainerTowerIsSaveAllowed());
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_HP), hp);
}

TEST("Wayfarer Trainer Tower claim finalizer clears only an existing pending prize")
{
    struct WayfarerSeviiTrainerTowerRecords *records;

    WayfarerSeviiInitPersistentState();
    records = WayfarerSevii_GetTrainerTowerRecords();
    records->pendingPrize = ITEM_METAL_COAT;
    CallTowerFunction(TRAINER_TOWER_FUNC_CHECK_PENDING_PRIZE);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(VarGet(VAR_WAYFARER_SEVII_TRAINER_TOWER_PENDING_PRIZE), ITEM_METAL_COAT);
    CallTowerFunction(TRAINER_TOWER_FUNC_CLAIM_PENDING_PRIZE);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(records->pendingPrize, ITEM_NONE);
    EXPECT_EQ(VarGet(VAR_WAYFARER_SEVII_TRAINER_TOWER_PENDING_PRIZE), ITEM_NONE);
    EXPECT(!CheckBagHasItem(ITEM_METAL_COAT, 1));
    CallTowerFunction(TRAINER_TOWER_FUNC_CLAIM_PENDING_PRIZE);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
}

TEST("Wayfarer Trainer Tower successful roof delivery restores the entry snapshot")
{
    struct WayfarerSeviiTrainerTowerRecords *records;
    u16 hp = 1;

    WayfarerSeviiInitPersistentState();
    records = WayfarerSevii_GetTrainerTowerRecords();
    ClearBag();
    SetUpUsableTowerParty();
    StartTowerRun(CHALLENGE_TYPE_SINGLE);
    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    CallTowerFunction(TRAINER_TOWER_FUNC_GIVE_PRIZE);
    EXPECT_EQ(gSpecialVar_Result, 2);
    EXPECT(WayfarerTrainerTowerIsChallengeActive());
    EXPECT(!CheckBagHasItem(ITEM_UP_GRADE, 1));
    ClearEveryTowerFloor();
    CallTowerFunction(TRAINER_TOWER_FUNC_GIVE_PRIZE);
    EXPECT_EQ(gSpecialVar_Result, 0);
    EXPECT(!WayfarerTrainerTowerIsChallengeActive());
    EXPECT(GetMonData(&gPlayerParty[0], MON_DATA_HP) > hp);
    EXPECT_EQ(records->pendingPrize, ITEM_NONE);
    EXPECT(CheckBagHasItem(ITEM_UP_GRADE, 1));
}
#endif

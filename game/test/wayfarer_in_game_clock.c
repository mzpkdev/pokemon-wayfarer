#include "global.h"
#include "clock.h"
#include "event_data.h"
#include "fake_rtc.h"
#include "play_time.h"
#include "rtc.h"
#include "wayfarer_daily_slots.h"
#include "test/test.h"
#include "constants/map_groups.h"
#include "constants/wayfarer_persistence.h"

#if IS_WAYFARER

// The in-game clock (.product/specs/in-game-clock.md): the clock runs with play at 16 in-game
// seconds a second, a new game starts on day 1 at 10:00, and every daily system reads one day.

// One daily flag of each bank the daily update clears: the shared one and Hoenn's.
#define TEST_DAILY_FLAG       FLAG_DAILY_PICKED_LOTO_TICKET
#define TEST_HOENN_DAILY_FLAG HOENN_FLAG_ID(WAYFARER_HOENN_DAILY_FLAGS_START + 2)

struct ClockSnapshot
{
    struct SiiRtcInfo clock;
    struct Time offset;
    struct Time lastBerryTreeUpdate;
    struct WarpData location;
    u16 days;
    bool8 clockSet;
    bool8 dailyFlag, hoennDailyFlag;
    u8 debugFlags;
    u16 playTimeHours;
    u8 playTimeMinutes, playTimeSeconds, playTimeVBlanks;
};

static void TakeSnapshot(struct ClockSnapshot *snapshot)
{
    snapshot->clock = gSaveBlock3Ptr->fakeRTC;
    snapshot->offset = gSaveBlock2Ptr->localTimeOffset;
    snapshot->lastBerryTreeUpdate = gSaveBlock2Ptr->lastBerryTreeUpdate;
    snapshot->location = gSaveBlock1Ptr->location;
    snapshot->days = VarGet(VAR_DAYS);
    snapshot->clockSet = FlagGet(FLAG_SYS_CLOCK_SET);
    snapshot->dailyFlag = FlagGet(TEST_DAILY_FLAG);
    snapshot->hoennDailyFlag = FlagGet(TEST_HOENN_DAILY_FLAG);
    snapshot->debugFlags = gDailySlotsDebugFlags;
    snapshot->playTimeHours = gSaveBlock2Ptr->playTimeHours;
    snapshot->playTimeMinutes = gSaveBlock2Ptr->playTimeMinutes;
    snapshot->playTimeSeconds = gSaveBlock2Ptr->playTimeSeconds;
    snapshot->playTimeVBlanks = gSaveBlock2Ptr->playTimeVBlanks;
    // The daily slots read the clock, not a pinned day.
    gDailySlotsDebugFlags = 0;
}

static void RestoreSnapshot(const struct ClockSnapshot *snapshot)
{
    PlayTimeCounter_Stop();
    gSaveBlock3Ptr->fakeRTC = snapshot->clock;
    gSaveBlock2Ptr->localTimeOffset = snapshot->offset;
    gSaveBlock2Ptr->lastBerryTreeUpdate = snapshot->lastBerryTreeUpdate;
    gSaveBlock1Ptr->location = snapshot->location;
    VarSet(VAR_DAYS, snapshot->days);
    if (snapshot->clockSet)
        FlagSet(FLAG_SYS_CLOCK_SET);
    else
        FlagClear(FLAG_SYS_CLOCK_SET);
    if (snapshot->dailyFlag)
        FlagSet(TEST_DAILY_FLAG);
    else
        FlagClear(TEST_DAILY_FLAG);
    if (snapshot->hoennDailyFlag)
        FlagSet(TEST_HOENN_DAILY_FLAG);
    else
        FlagClear(TEST_HOENN_DAILY_FLAG);
    gDailySlotsDebugFlags = snapshot->debugFlags;
    gSaveBlock2Ptr->playTimeHours = snapshot->playTimeHours;
    gSaveBlock2Ptr->playTimeMinutes = snapshot->playTimeMinutes;
    gSaveBlock2Ptr->playTimeSeconds = snapshot->playTimeSeconds;
    gSaveBlock2Ptr->playTimeVBlanks = snapshot->playTimeVBlanks;
    RtcCalcLocalTime();
}

static void StandOn(u32 mapGroup, u32 mapNum)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
}

// Seconds since the clock's day 0 at 00:00:00, read the way every reader does.
static u32 ClockSeconds(void)
{
    RtcCalcLocalTime();
    return ((gLocalTime.days * HOURS_PER_DAY + gLocalTime.hours) * MINUTES_PER_HOUR + gLocalTime.minutes) * SECONDS_PER_MINUTE
         + gLocalTime.seconds;
}

TEST("In-game clock: the clock is always on and can't fail")
{
    EXPECT(UseFakeRtc());
    EXPECT_EQ(RtcGetErrorStatus(), 0);
    EXPECT_EQ(FakeRtc_GetSecondsRatio(), 16);
}

TEST("In-game clock: a new game starts on day 1 at 10:00 with every daily system on the same day")
{
    struct ClockSnapshot snapshot;

    TakeSnapshot(&snapshot);
    // Leftovers from an earlier file are overwritten, not added to.
    gSaveBlock2Ptr->localTimeOffset.days = 3;
    gSaveBlock2Ptr->localTimeOffset.hours = 5;
    FakeRtc_AdvanceTimeBy(40, 7, 0, 0);
    FlagClear(FLAG_SYS_CLOCK_SET);
    InGameClock_StartNewGame();

    RtcCalcLocalTime();
    EXPECT_EQ(gLocalTime.days, 1);
    EXPECT_EQ(gLocalTime.hours, 10);
    EXPECT_EQ(gLocalTime.minutes, 0);
    EXPECT_EQ(gLocalTime.seconds, 0);
    EXPECT_EQ(gSaveBlock2Ptr->localTimeOffset.days, 0);
    EXPECT_EQ(gSaveBlock2Ptr->localTimeOffset.hours, 0);
    EXPECT_EQ(gSaveBlock2Ptr->localTimeOffset.minutes, 0);
    EXPECT_EQ(gSaveBlock2Ptr->localTimeOffset.seconds, 0);
    EXPECT(FlagGet(FLAG_SYS_CLOCK_SET));
    EXPECT_EQ(VarGet(VAR_DAYS), 1);
    EXPECT_EQ(RtcGetLocalDayCount(), 1);
    EXPECT_EQ(DailySlots_GetDay(), 1);
    EXPECT_EQ(GetTimeOfDay(), TIME_DAY);
    // Every date reader sees the clock's own date: day 1 is Saturday 1 January 2000.
    EXPECT_EQ(GetDayOfWeek(), WEEKDAY_SAT);
    EXPECT_EQ(GetDay(), 1);
    EXPECT_EQ(GetMonth(), MONTH_JAN);
    EXPECT_EQ(GetFullYear(), 2000);
    EXPECT_EQ(GetDayOfWeek(), gSaveBlock3Ptr->fakeRTC.dayOfWeek);
    RestoreSnapshot(&snapshot);
}

TEST("In-game clock: a second of play moves the clock 16 seconds, so a day lasts 90 minutes of play")
{
    struct ClockSnapshot snapshot;
    u32 start, i;

    TakeSnapshot(&snapshot);
    InGameClock_StartNewGame();
    start = ClockSeconds();

    // 59 frames are less than a second of play: the clock waits for the 60th.
    PlayTimeCounter_Start();
    for (i = 0; i < 59; i++)
        PlayTimeCounter_Update();
    EXPECT_EQ(ClockSeconds(), start);
    PlayTimeCounter_Update();
    EXPECT_EQ(ClockSeconds(), start + 16);
    PlayTimeCounter_Stop();
    for (i = 0; i < 60; i++)
        PlayTimeCounter_Update();
    EXPECT_EQ(ClockSeconds(), start + 16);

    // 90 minutes of play are one in-game day to the second.
    for (i = 1; i < 90 * 60; i++)
        FakeRtc_TickTimeForward();
    EXPECT_EQ(ClockSeconds(), start + HOURS_PER_DAY * MINUTES_PER_HOUR * SECONDS_PER_MINUTE);
    EXPECT_EQ(gLocalTime.days, 2);
    EXPECT_EQ(gLocalTime.hours, 10);
    RestoreSnapshot(&snapshot);
}

TEST("In-game clock: passing midnight on the field starts the next day for the daily update and the daily slots")
{
    struct ClockSnapshot snapshot;

    TakeSnapshot(&snapshot);
    InGameClock_StartNewGame();
    StandOn(MAP_GROUP(MAP_ROUTE1_HNS), MAP_NUM(MAP_ROUTE1_HNS));
    FlagSet(TEST_DAILY_FLAG);
    FlagSet(TEST_HOENN_DAILY_FLAG);

    // 23:59:50 is still day 1.
    FakeRtc_AdvanceTimeBy(0, 13, 59, 50);
    DoTimeBasedEvents();
    EXPECT_EQ(gLocalTime.hours, 23);
    EXPECT_EQ(VarGet(VAR_DAYS), 1);
    EXPECT(FlagGet(TEST_DAILY_FLAG));
    EXPECT(FlagGet(TEST_HOENN_DAILY_FLAG));
    EXPECT_EQ(DailySlots_GetDay(), 1);
    EXPECT_EQ(GetTimeOfDay(), TIME_NIGHT);

    // One second of play later it is 00:00:06 on day 2.
    FakeRtc_TickTimeForward();
    DoTimeBasedEvents();
    EXPECT_EQ(gLocalTime.days, 2);
    EXPECT_EQ(gLocalTime.hours, 0);
    EXPECT_EQ(gLocalTime.seconds, 6);
    EXPECT_EQ(VarGet(VAR_DAYS), 2);
    EXPECT(!FlagGet(TEST_DAILY_FLAG));
    EXPECT(!FlagGet(TEST_HOENN_DAILY_FLAG));
    EXPECT_EQ(DailySlots_GetDay(), 2);
    EXPECT_EQ(RtcGetLocalDayCount(), VarGet(VAR_DAYS));
    RestoreSnapshot(&snapshot);
}

TEST("In-game clock: in a Pokemon Center the daily update waits for the player to leave, then counts every day")
{
    struct ClockSnapshot snapshot;

    TakeSnapshot(&snapshot);
    InGameClock_StartNewGame();
    StandOn(MAP_GROUP(MAP_OLDALE_TOWN_POKEMON_CENTER_1F), MAP_NUM(MAP_OLDALE_TOWN_POKEMON_CENTER_1F));
    FlagSet(TEST_DAILY_FLAG);
    FlagSet(TEST_HOENN_DAILY_FLAG);

    FakeRtc_AdvanceTimeBy(2, 0, 0, 0);
    DoTimeBasedEvents();
    EXPECT_EQ(VarGet(VAR_DAYS), 1);
    EXPECT(FlagGet(TEST_DAILY_FLAG));
    EXPECT(FlagGet(TEST_HOENN_DAILY_FLAG));
    // The daily slots don't wait: their day is the clock's.
    EXPECT_EQ(DailySlots_GetDay(), 3);

    StandOn(MAP_GROUP(MAP_ROUTE1_HNS), MAP_NUM(MAP_ROUTE1_HNS));
    DoTimeBasedEvents();
    EXPECT_EQ(VarGet(VAR_DAYS), 3);
    EXPECT(!FlagGet(TEST_DAILY_FLAG));
    EXPECT(!FlagGet(TEST_HOENN_DAILY_FLAG));
    RestoreSnapshot(&snapshot);
}

TEST("In-game clock: a Pokemon Center outside Emerald's list is an ordinary map for the daily update")
{
    struct ClockSnapshot snapshot;

    TakeSnapshot(&snapshot);
    InGameClock_StartNewGame();
    StandOn(MAP_GROUP(MAP_CHERRYGROVE_CITY_POKEMON_CENTER_HNS), MAP_NUM(MAP_CHERRYGROVE_CITY_POKEMON_CENTER_HNS));
    FlagSet(TEST_DAILY_FLAG);

    FakeRtc_AdvanceTimeBy(1, 0, 0, 0);
    DoTimeBasedEvents();
    EXPECT_EQ(VarGet(VAR_DAYS), 2);
    EXPECT(!FlagGet(TEST_DAILY_FLAG));
    RestoreSnapshot(&snapshot);
}

TEST("In-game clock: the day count only goes forward, one day at a time, across months and years")
{
    struct ClockSnapshot snapshot;
    u32 day, weekday;

    TakeSnapshot(&snapshot);
    InGameClock_StartNewGame();
    StandOn(MAP_GROUP(MAP_ROUTE1_HNS), MAP_NUM(MAP_ROUTE1_HNS));
    weekday = GetDayOfWeek();
    // Two years and a bit: the first year (2000) is a leap year.
    for (day = 2; day <= 800; day++)
    {
        FakeRtc_AdvanceTimeBy(1, 0, 0, 0);
        DoTimeBasedEvents();
        EXPECT_EQ(gLocalTime.days, day);
        EXPECT_EQ(gLocalTime.hours, 10);
        EXPECT_EQ(RtcGetLocalDayCount(), day);
        EXPECT_EQ(DailySlots_GetDay(), day);
        EXPECT_EQ(VarGet(VAR_DAYS), day);
        weekday = (weekday + 1) % WEEKDAY_COUNT;
        EXPECT_EQ(GetDayOfWeek(), weekday);
        EXPECT_EQ(GetDayOfWeek(), gSaveBlock3Ptr->fakeRTC.dayOfWeek);
    }
    RestoreSnapshot(&snapshot);
}

#endif // IS_WAYFARER

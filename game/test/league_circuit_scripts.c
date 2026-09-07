#include "global.h"
#include "event_data.h"
#include "league_circuit.h"
#include "string_util.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "config/league_circuit.h"

#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
extern const u8 LeagueCircuit_Text_NeedEightBadges[];
extern const u8 LeagueCircuit_Text_NeedKantoClear[];
extern const u8 LeagueCircuit_Text_NeedSixteenBadges[];
extern const u8 LeagueCircuit_Text_NeedJohtoClear[];
extern const u8 LeagueCircuit_Text_NeedTwentyFourBadges[];
extern const u8 LeagueCircuit_Text_Unavailable[];
extern u16 LeagueCircuit_GetRequiredRegion(void);
extern u16 LeagueCircuit_IsEligible(void);
extern u16 LeagueCircuit_RecordClear(void);
extern u16 LeagueCircuit_BufferAdmissionDenial(void);

static void SetTotalBadges(u8 count)
{
    u8 i;

    for (i = 0; i < 24; i++)
        SetBadgeStateForRegion(REGION_KANTO + i / 8, i % 8, i < count);
}

static void ExpectAdmissionDenial(enum Region region, enum LeagueAdmissionRequirement requirement, const u8 *text)
{
    gSpecialVar_0x8004 = region;
    EXPECT_EQ(LeagueCircuit_BufferAdmissionDenial(), requirement);
    EXPECT_EQ(StringCompare(gStringVar4, text), 0);
}

TEST("League circuit script specials expose ordered exact-threshold eligibility")
{
    SetTotalBadges(24);
    EXPECT_EQ(LeagueCircuit_GetRequiredRegion(), REGION_KANTO);

    gSpecialVar_0x8004 = REGION_JOHTO;
    EXPECT_EQ(LeagueCircuit_IsEligible(), FALSE);
    EXPECT_EQ(LeagueCircuit_RecordClear(), FALSE);

    gSpecialVar_0x8004 = REGION_KANTO;
    EXPECT_EQ(LeagueCircuit_IsEligible(), TRUE);
    EXPECT_EQ(LeagueCircuit_RecordClear(), TRUE);
    EXPECT_EQ(LeagueCircuit_GetRequiredRegion(), REGION_JOHTO);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_KANTO);

    gSpecialVar_0x8004 = REGION_JOHTO;
    EXPECT_EQ(LeagueCircuit_IsEligible(), TRUE);
    EXPECT_EQ(LeagueCircuit_RecordClear(), TRUE);
    EXPECT_EQ(LeagueCircuit_GetRequiredRegion(), REGION_HOENN);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_JOHTO);

    gSpecialVar_0x8004 = REGION_HOENN;
    EXPECT_EQ(LeagueCircuit_IsEligible(), TRUE);
    EXPECT_EQ(LeagueCircuit_RecordClear(), TRUE);
    EXPECT_EQ(LeagueCircuit_GetRequiredRegion(), REGION_NONE);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_HOENN);
}

TEST("League admission denial reports only the immediate missing requirement")
{
    SetTotalBadges(7);
    ExpectAdmissionDenial(REGION_KANTO, LEAGUE_ADMISSION_NEEDS_8_BADGES, LeagueCircuit_Text_NeedEightBadges);
    ExpectAdmissionDenial(REGION_JOHTO, LEAGUE_ADMISSION_NEEDS_KANTO_CLEAR, LeagueCircuit_Text_NeedKantoClear);
    ExpectAdmissionDenial(REGION_HOENN, LEAGUE_ADMISSION_NEEDS_KANTO_CLEAR, LeagueCircuit_Text_NeedKantoClear);

    SetGameClearStateForRegion(REGION_KANTO, TRUE);
    ExpectAdmissionDenial(REGION_JOHTO, LEAGUE_ADMISSION_NEEDS_16_BADGES, LeagueCircuit_Text_NeedSixteenBadges);
    ExpectAdmissionDenial(REGION_HOENN, LEAGUE_ADMISSION_NEEDS_JOHTO_CLEAR, LeagueCircuit_Text_NeedJohtoClear);

    SetTotalBadges(16);
    SetGameClearStateForRegion(REGION_JOHTO, TRUE);
    ExpectAdmissionDenial(REGION_HOENN, LEAGUE_ADMISSION_NEEDS_24_BADGES, LeagueCircuit_Text_NeedTwentyFourBadges);

    SetTotalBadges(24);
    SetGameClearStateForRegion(REGION_HOENN, TRUE);
    ExpectAdmissionDenial(REGION_HOENN, LEAGUE_ADMISSION_UNAVAILABLE, LeagueCircuit_Text_Unavailable);
    ExpectAdmissionDenial(REGION_NONE, LEAGUE_ADMISSION_UNAVAILABLE, LeagueCircuit_Text_Unavailable);
}
#endif

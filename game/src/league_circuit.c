#include "global.h"
#include "league_circuit.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"

#if IS_WAYFARER
EWRAM_DATA static enum Region sRecordedLeagueClearRegion = REGION_NONE;

u8 GetGlobalBadgeCount(void)
{
    return GetBadgeCountForRegion(REGION_KANTO)
         + GetBadgeCountForRegion(REGION_JOHTO)
         + GetBadgeCountForRegion(REGION_HOENN);
}

enum Region GetRequiredLeagueRegion(void)
{
    if (!GetChampionStateForRegion(REGION_KANTO))
        return REGION_KANTO;
    if (!GetChampionStateForRegion(REGION_JOHTO))
        return REGION_JOHTO;
    if (!GetChampionStateForRegion(REGION_HOENN))
        return REGION_HOENN;
    return REGION_NONE;
}

u8 GetBadgeCertificationCap(void)
{
    switch (GetRequiredLeagueRegion())
    {
    case REGION_KANTO:
        return 8;
    case REGION_JOHTO:
        return 16;
    default:
        return 24;
    }
}

bool8 IsEligibleForLeague(enum Region region)
{
    return region != REGION_NONE
        && region == GetRequiredLeagueRegion()
        && GetGlobalBadgeCount() >= GetBadgeCertificationCap();
}

bool8 CanChallengeGymForBadge(enum Region region, u8 badgeIndex)
{
    if ((region != REGION_KANTO && region != REGION_JOHTO && region != REGION_HOENN)
     || badgeIndex >= 8)
        return FALSE;
    return GetBadgeStateForRegion(region, badgeIndex)
        || GetGlobalBadgeCount() < GetBadgeCertificationCap();
}

bool8 TryRecordLeagueClear(enum Region region)
{
    if (!IsEligibleForLeague(region))
        return FALSE;
    SetGameClearStateForRegion(region, TRUE);
    GetTrainerRating();
    sRecordedLeagueClearRegion = region;
    return TRUE;
}

enum Region ConsumeRecordedLeagueClearRegion(void)
{
    enum Region region = sRecordedLeagueClearRegion;
    sRecordedLeagueClearRegion = REGION_NONE;
    return region;
}

u8 CalculateLeagueCircuitTrainerRating(void)
{
    u8 badges = GetGlobalBadgeCount();
    u8 rating;

    if (badges <= 4)
        rating = 4 * badges;
    else if (badges <= 8)
        rating = 16 + 6 * (badges - 4);
    else
        rating = 40 + badges - 8;

    if (GetChampionStateForRegion(REGION_KANTO))
        rating += 15;
    if (GetChampionStateForRegion(REGION_JOHTO))
        rating += 5;
    if (GetChampionStateForRegion(REGION_HOENN))
        rating += 4;
    return ClampTrainerRating(rating);
}
#endif

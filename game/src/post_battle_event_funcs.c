#include "global.h"
#include "main.h"
#include "credits.h"
#include "event_data.h"
#include "hall_of_fame.h"
#include "hall_of_fame_frlg.h"
#include "load_save.h"
#include "malloc.h"
#include "league_circuit.h"
#include "overworld.h"
#include "regions.h"
#include "script_pokemon_util.h"
#include "tv.h"
#include "wayfarer_persistence.h"
#include "constants/heal_locations.h"
#include "config/league_circuit.h"

#if IS_WAYFARER
extern void GivePartyMonChampionRibbon(void);
#endif

#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
static bool8 sUseNonCircuitGameClear;
static bool8 sIndigoHallOfFameCommitPending;
static bool8 sIndigoHallOfFameSaveTransaction;
static bool8 sIndigoHallOfFameSaveCountIncremented;
static u8 sIndigoHallOfFameRatingAtEntry;
static u8 sIndigoHallOfFameStoredRatingBefore;
// The rollback snapshot is too large for IWRAM, where it would shrink the
// shared stack, so it lives on the heap only while a transaction is open.
static struct IndigoHallOfFameSnapshot
{
    struct Pokemon party[PARTY_SIZE];
    TVShow tvShows[TV_SHOWS_COUNT];
} *sIndigoHallOfFameSnapshot;
static struct WarpData sIndigoHallOfFameContinueWarpBefore;
static u32 sIndigoHallOfFameCountBefore;
static u32 sIndigoHallOfFameFirstPlayTimeBefore;
static u32 sIndigoHallOfFameRibbonCountBefore;
static u8 sIndigoHallOfFameSpecialWarpFlagsBefore;
static bool8 sIndigoHallOfFameRibbonFlagBefore;
static bool8 sIndigoHallOfFameGlobalClearBefore;
static bool8 sIndigoHallOfFameKantoClearBefore;
static bool8 sIndigoHallOfFameJohtoClearBefore;
#endif

u16 LeagueCircuit_CommitAndRegisterIndigo(void)
{
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    if (!CanCompleteCircuitRun(CIRCUIT_STAGE_INDIGO)
     || IsActiveLeagueRunReplay())
        return FALSE;

    if (sIndigoHallOfFameSnapshot == NULL)
        sIndigoHallOfFameSnapshot = Alloc(sizeof(*sIndigoHallOfFameSnapshot));
    if (sIndigoHallOfFameSnapshot == NULL)
        return FALSE;

    memcpy(sIndigoHallOfFameSnapshot->party, gPlayerParty, sizeof(gPlayerParty));
    memcpy(sIndigoHallOfFameSnapshot->tvShows, gSaveBlock1Ptr->tvShows, sizeof(gSaveBlock1Ptr->tvShows));
    sIndigoHallOfFameContinueWarpBefore = gSaveBlock1Ptr->continueGameWarp;
    sIndigoHallOfFameCountBefore = GetGameStat(GAME_STAT_ENTERED_HOF);
    sIndigoHallOfFameFirstPlayTimeBefore = GetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME);
    sIndigoHallOfFameRibbonCountBefore = GetGameStat(GAME_STAT_RECEIVED_RIBBONS);
    sIndigoHallOfFameSpecialWarpFlagsBefore = gSaveBlock2Ptr->specialSaveWarpFlags;
    sIndigoHallOfFameRibbonFlagBefore = FlagGet(FLAG_SYS_RIBBON_GET);
    sIndigoHallOfFameGlobalClearBefore = FlagGet(FLAG_SYS_GAME_CLEAR);
    sIndigoHallOfFameKantoClearBefore = GetGameClearStateForRegion(REGION_KANTO);
    sIndigoHallOfFameJohtoClearBefore = GetGameClearStateForRegion(REGION_JOHTO);
    HealPlayerParty();
    GivePartyMonChampionRibbon();
    gHasHallOfFameRecords = GetGameStat(GAME_STAT_ENTERED_HOF) != 0;
    if (GetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME) == 0)
        SetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME,
                    (gSaveBlock2Ptr->playTimeHours << 16)
                  | (gSaveBlock2Ptr->playTimeMinutes << 8)
                  | gSaveBlock2Ptr->playTimeSeconds);
    SetContinueGameWarpStatus();
    SetContinueGameWarpToHealLocation(HEAL_LOCATION_INDIGO_PLATEAU_HNS);
    // Defer the canonical clear until the FRLG Hall of Fame has prepared its
    // team buffer. The clear and its derived Rating then enter the same save
    // operation as the required Hall of Fame registration.
    sIndigoHallOfFameCommitPending = TRUE;
    sIndigoHallOfFameSaveTransaction = TRUE;
    sIndigoHallOfFameSaveCountIncremented = FALSE;
    sIndigoHallOfFameRatingAtEntry = gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry;
    sIndigoHallOfFameStoredRatingBefore = VarGet(VAR_TRAINER_RATING);
    SetMainCallback2(CB2_DoHallOfFameScreenFrlg);
    return TRUE;
#else
    return FALSE;
#endif
}

bool8 IsIndigoHallOfFameSaveTransactionActive(void)
{
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    return sIndigoHallOfFameSaveTransaction;
#else
    return FALSE;
#endif
}

void FinishIndigoHallOfFameSaveTransaction(bool8 success)
{
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    if (!sIndigoHallOfFameSaveTransaction)
        return;
    sIndigoHallOfFameSaveTransaction = FALSE;
    if (!success)
    {
        RollbackIndigoHallOfFameCommit(sIndigoHallOfFameRatingAtEntry,
                                      sIndigoHallOfFameStoredRatingBefore);
        SetGameClearStateForRegion(REGION_KANTO, sIndigoHallOfFameKantoClearBefore);
        SetGameClearStateForRegion(REGION_JOHTO, sIndigoHallOfFameJohtoClearBefore);
        if (sIndigoHallOfFameGlobalClearBefore)
            FlagSet(FLAG_SYS_GAME_CLEAR);
        else
            FlagClear(FLAG_SYS_GAME_CLEAR);
        memcpy(gPlayerParty, sIndigoHallOfFameSnapshot->party, sizeof(gPlayerParty));
        memcpy(gSaveBlock1Ptr->tvShows, sIndigoHallOfFameSnapshot->tvShows, sizeof(gSaveBlock1Ptr->tvShows));
        gSaveBlock1Ptr->continueGameWarp = sIndigoHallOfFameContinueWarpBefore;
        gSaveBlock2Ptr->specialSaveWarpFlags = sIndigoHallOfFameSpecialWarpFlagsBefore;
        SetGameStat(GAME_STAT_ENTERED_HOF, sIndigoHallOfFameCountBefore);
        SetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME, sIndigoHallOfFameFirstPlayTimeBefore);
        SetGameStat(GAME_STAT_RECEIVED_RIBBONS, sIndigoHallOfFameRibbonCountBefore);
        if (sIndigoHallOfFameRibbonFlagBefore)
            FlagSet(FLAG_SYS_RIBBON_GET);
        else
            FlagClear(FLAG_SYS_RIBBON_GET);
    }
    FREE_AND_SET_NULL(sIndigoHallOfFameSnapshot);
#endif
}

void ResolveIndigoHallOfFameSaveAttempt(bool8 success, bool8 retryPending)
{
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    if (success)
        FinishIndigoHallOfFameSaveTransaction(TRUE);
    else if (!retryPending)
        FinishIndigoHallOfFameSaveTransaction(FALSE);
#endif
}

bool8 TryIncrementIndigoHallOfFameSaveCount(void)
{
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    if (!sIndigoHallOfFameSaveTransaction)
        return FALSE;
    if (!sIndigoHallOfFameSaveCountIncremented)
    {
        if (GetGameStat(GAME_STAT_ENTERED_HOF) < 999)
            IncrementGameStat(GAME_STAT_ENTERED_HOF);
        sIndigoHallOfFameSaveCountIncremented = TRUE;
    }
    return TRUE;
#else
    return FALSE;
#endif
}

bool8 CommitPendingIndigoHallOfFame(void)
{
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    if (!sIndigoHallOfFameCommitPending)
        return FALSE;
    sIndigoHallOfFameCommitPending = FALSE;
    return CommitCircuitRun(CIRCUIT_STAGE_INDIGO) == CIRCUIT_COMMIT_FIRST_CLEAR;
#else
    return FALSE;
#endif
}

int GameClear(void)
{
    int i;
    bool32 ribbonGet;
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    bool8 nonCircuitClear = sUseNonCircuitGameClear;
    enum Region currentRegion;
    sUseNonCircuitGameClear = FALSE;
    currentRegion = nonCircuitClear
        ? WayfarerGetCurrentMapRegion()
        : ConsumeRecordedLeagueClearRegion();
#elif IS_WAYFARER
    enum Region currentRegion = WayfarerGetCurrentMapRegion();
#endif
    struct RibbonCounter {
        u8 partyIndex;
        u8 count;
    } ribbonCounts[6];

#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    if (currentRegion == REGION_NONE)
        return 0;
#endif
    HealPlayerParty();

#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    if (nonCircuitClear)
        gHasHallOfFameRecords = GetGameClearStateForRegion(currentRegion);
    else
        gHasHallOfFameRecords = GetGameStat(GAME_STAT_ENTERED_HOF) != 0;
#else
#if IS_WAYFARER
    if (GetGameClearStateForRegion(currentRegion) == TRUE)
#else
    if (FlagGet(FLAG_SYS_GAME_CLEAR) == TRUE)
#endif
    {
        gHasHallOfFameRecords = TRUE;
    }
    else
    {
        gHasHallOfFameRecords = FALSE;
#if IS_WAYFARER
        SetGameClearStateForRegion(currentRegion, TRUE);
#else
        FlagSet(FLAG_SYS_GAME_CLEAR);
#endif
    }
#endif

    if (GetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME) == 0)
        SetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME, (gSaveBlock2Ptr->playTimeHours << 16) | (gSaveBlock2Ptr->playTimeMinutes << 8) | gSaveBlock2Ptr->playTimeSeconds);

    SetContinueGameWarpStatus();

#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    if (nonCircuitClear)
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_NEW_BARK_TOWN_HNS);
    else if (currentRegion == REGION_HOENN)
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_EVER_GRANDE_CITY_POKEMON_LEAGUE);
    else
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_INDIGO_PLATEAU_HNS);
#elif IS_WAYFARER
    if (currentRegion != REGION_HOENN)
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_NEW_BARK_TOWN_HNS);
    else if (gSaveBlock2Ptr->playerGender == MALE)
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F);
    else
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F);
#else
    if (IS_HNS)
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_NEW_BARK_TOWN_HNS);
    else if (gSaveBlock2Ptr->playerGender == MALE)
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F);
    else
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F);
#endif

    ribbonGet = FALSE;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];

        ribbonCounts[i].partyIndex = i;
        ribbonCounts[i].count = 0;

        if (GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES)
         && !GetMonData(mon, MON_DATA_SANITY_IS_EGG)
         && !GetMonData(mon, MON_DATA_CHAMPION_RIBBON))
        {
            u8 val[1] = {TRUE};
            SetMonData(mon, MON_DATA_CHAMPION_RIBBON, val);
            ribbonCounts[i].count = GetRibbonCount(mon);
            ribbonGet = TRUE;
        }
    }

    if (ribbonGet == TRUE)
    {
        IncrementGameStat(GAME_STAT_RECEIVED_RIBBONS);
        FlagSet(FLAG_SYS_RIBBON_GET);

        for (i = 1; i < 6; i++)
        {
            if (ribbonCounts[i].count > ribbonCounts[0].count)
            {
                struct RibbonCounter prevBest = ribbonCounts[0];
                ribbonCounts[0] = ribbonCounts[i];
                ribbonCounts[i] = prevBest;
            }
        }

        if (ribbonCounts[0].count > NUM_CUTIES_RIBBONS)
        {
            TryPutSpotTheCutiesOnAir(&gPlayerParty[ribbonCounts[0].partyIndex], MON_DATA_CHAMPION_RIBBON);
        }
    }

    SetMainCallback2(CB2_DoHallOfFameScreen);
    return 0;
}

#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
int WayfarerRedGameClear(void)
{
    // Red retains the authored HNS completion presentation, but is not a
    // circuit stage and therefore must not consume or create a circuit clear.
    sUseNonCircuitGameClear = TRUE;
    return GameClear();
}
#endif

bool8 SetCB2WhiteOut(void)
{
    SetMainCallback2(CB2_WhiteOut);
    return FALSE;
}

bool8 EnterHallOfFame(void)
{
    bool8 ribbonState;
    bool8 *r7;
    int i;
    bool8 gaveAtLeastOneRibbon;
    HealPlayerParty();
    if (FlagGet(FLAG_SYS_GAME_CLEAR) == TRUE)
    {
        gHasHallOfFameRecords = TRUE;
    }
    else
    {
        gHasHallOfFameRecords = FALSE;
        FlagSet(FLAG_SYS_GAME_CLEAR);
    }
    if (GetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME) == 0)
    {
        SetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME, (gSaveBlock2Ptr->playTimeHours << 16) | (gSaveBlock2Ptr->playTimeMinutes << 8) | gSaveBlock2Ptr->playTimeSeconds);
    }
    SetContinueGameWarpStatus();
    if (IS_HNS)
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_NEW_BARK_TOWN_HNS);
    else
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_PALLET_TOWN);
    gaveAtLeastOneRibbon = FALSE;
    for (i = 0, r7 = &ribbonState; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES) && !GetMonData(&gPlayerParty[i], MON_DATA_SANITY_IS_EGG))
        {
            if (!GetMonData(&gPlayerParty[i], MON_DATA_CHAMPION_RIBBON))
            {
                *r7 = TRUE;
                SetMonData(&gPlayerParty[i], MON_DATA_CHAMPION_RIBBON, &ribbonState);
                gaveAtLeastOneRibbon = TRUE;
            }
        }
    }
    if (gaveAtLeastOneRibbon == TRUE)
    {
        IncrementGameStat(GAME_STAT_RECEIVED_RIBBONS);
        FlagSet(FLAG_SYS_RIBBON_GET);
    }
    SetMainCallback2(CB2_DoHallOfFameScreenFrlg);
    return FALSE;
}

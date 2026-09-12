#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "test/test.h"
#include "trainer_only_encounter.h"
#include "wild_encounter.h"
#include "constants/items.h"
#include "constants/metatile_behaviors.h"

#if IS_WAYFARER

enum EncounterSource { SOURCE_LAND, SOURCE_OUTBREAK, SOURCE_FISHING, SOURCE_ROCK_SMASH, SOURCE_SWEET_SCENT };

// Function-test assertions return to the runner immediately. Keep this small
// integration harness self-contained even when an expectation fails, since its
// TESTING interception intentionally changes process-wide wild-battle state.
struct EncounterSourceTestSnapshot
{
    struct MapHeader mapHeader;
    struct BackupMapLayout backupMapLayout;
    struct PlayerAvatar playerAvatar;
    struct ObjectEvent playerObjectEvent;
    struct WarpData location;
    struct Pokemon playerParty[PARTY_SIZE];
    struct Pokemon enemyParty[PARTY_SIZE];
    u16 repelSteps;
    u16 specialResult;
    u8 playerPartyCount;
    u8 enemyPartyCount;
    bool8 isFishingEncounter;
};

static void SnapshotEncounterSourceTestState(struct EncounterSourceTestSnapshot *snapshot)
{
    snapshot->mapHeader = gMapHeader;
    snapshot->backupMapLayout = gBackupMapLayout;
    snapshot->playerAvatar = gPlayerAvatar;
    snapshot->playerObjectEvent = gObjectEvents[0];
    snapshot->location = gSaveBlock1Ptr->location;
    memcpy(snapshot->playerParty, gPlayerParty, sizeof(snapshot->playerParty));
    memcpy(snapshot->enemyParty, gEnemyParty, sizeof(snapshot->enemyParty));
    snapshot->repelSteps = VarGet(VAR_REPEL_STEP_COUNT);
    snapshot->specialResult = gSpecialVar_Result;
    snapshot->playerPartyCount = gPlayerPartyCount;
    snapshot->enemyPartyCount = gEnemyPartyCount;
    snapshot->isFishingEncounter = gIsFishingEncounter;
}

static void RestoreEncounterSourceTestState(const struct EncounterSourceTestSnapshot *snapshot)
{
    // Disable the testing hook before anything can return to the next test.
    SetWildStartInterceptionForTesting(FALSE);
    TrainerOnlyResetEncounter();
    FlagClear(FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS);
    gMapHeader = snapshot->mapHeader;
    gBackupMapLayout = snapshot->backupMapLayout;
    gPlayerAvatar = snapshot->playerAvatar;
    gObjectEvents[0] = snapshot->playerObjectEvent;
    gSaveBlock1Ptr->location = snapshot->location;
    memcpy(gPlayerParty, snapshot->playerParty, sizeof(snapshot->playerParty));
    memcpy(gEnemyParty, snapshot->enemyParty, sizeof(snapshot->enemyParty));
    VarSet(VAR_REPEL_STEP_COUNT, snapshot->repelSteps);
    gSpecialVar_Result = snapshot->specialResult;
    gPlayerPartyCount = snapshot->playerPartyCount;
    gEnemyPartyCount = snapshot->enemyPartyCount;
    gIsFishingEncounter = snapshot->isFishingEncounter;
}

static void SetSourceParty(void)
{
    ZeroPlayerPartyMons();
    gPlayerPartyCount = 0;
}

static bool32 SelectGrassFromActualLayout(void)
{
    u32 x, y;
    const struct MapLayout *layout = gMapHeader.mapLayout;

    // Read the real map without executing its field scripts or loading graphics.
    gBackupMapLayout.width = layout->width;
    gBackupMapLayout.height = layout->height;
    gBackupMapLayout.map = (u16 *)layout->map;
    gPlayerAvatar.objectEventId = 0;
    for (y = 0; y < layout->height; y++)
        for (x = 0; x < layout->width; x++)
            if (MetatileBehavior_IsLandWildEncounter(MapGridGetMetatileBehaviorAt(x, y)))
            {
                gObjectEvents[0].currentCoords.x = x;
                gObjectEvents[0].currentCoords.y = y;
                return TRUE;
            }
    return FALSE;
}

static void PrepareSource(u32 source)
{
    SetSourceParty();
    TrainerOnlyResetEncounter();
    FlagSet(FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS);
    ZeroEnemyPartyMons();
    DisableWildEncounters(FALSE);
    gPlayerAvatar.flags = 0;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ROUTE30_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ROUTE30_HNS);
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_ROUTE30_HNS), MAP_NUM(MAP_ROUTE30_HNS));
    VarSet(VAR_REPEL_STEP_COUNT, 100);
    gSaveBlock1Ptr->outbreakPokemonSpecies = source == SOURCE_OUTBREAK ? SPECIES_DITTO : SPECIES_NONE;
    gSaveBlock1Ptr->outbreakLocationMapGroup = MAP_GROUP(MAP_ROUTE30_HNS);
    gSaveBlock1Ptr->outbreakLocationMapNum = MAP_NUM(MAP_ROUTE30_HNS);
    gSaveBlock1Ptr->outbreakPokemonLevel = 7;
    gSaveBlock1Ptr->outbreakPokemonProbability = 100;
    gSaveBlock1Ptr->outbreakPokemonMoves[0] = MOVE_TRANSFORM;
    for (u32 i = 1; i < MAX_MON_MOVES; i++)
        gSaveBlock1Ptr->outbreakPokemonMoves[i] = MOVE_NONE;
    SetWildStartInterceptionForTesting(TRUE);
}

static bool32 TrySource(u32 source)
{
    switch (source)
    {
    case SOURCE_LAND:
    case SOURCE_OUTBREAK:
        return StandardWildEncounter(MB_TALL_GRASS, MB_TALL_GRASS);
    case SOURCE_FISHING:
        FishingWildEncounter(OLD_ROD);
        return GetWildStartsForTesting() != 0;
    case SOURCE_ROCK_SMASH:
        RockSmashWildEncounter();
        return gSpecialVar_Result;
    case SOURCE_SWEET_SCENT:
        return SweetScentWildEncounter();
    }
    return FALSE;
}

TEST("Trainer-only native encounter sources generate and initialize with an empty party")
{
    u32 source = SOURCE_LAND;
    for (u32 i = SOURCE_LAND; i <= SOURCE_SWEET_SCENT; i++)
        PARAMETRIZE(source = i);

    struct EncounterSourceTestSnapshot snapshot;
    u32 seed;
    bool32 started = FALSE;
    bool32 passed = TRUE;

    SnapshotEncounterSourceTestState(&snapshot);

    PrepareSource(source);
    passed &= GetCurrentMapWildMonHeaderId() != HEADER_NONE;
    passed &= SelectGrassFromActualLayout();
    for (seed = 0; seed < 256 && !started; seed++)
    {
        SeedRng(seed);
        started = TrySource(source);
    }
    passed &= started;
    passed &= GetWildStartsForTesting() == 1;
    passed &= IsTrainerOnlyEncounter();
    passed &= GetMonData(&gEnemyParty[0], MON_DATA_SPECIES) != SPECIES_NONE;
    passed &= GetMonData(&gEnemyParty[0], MON_DATA_HP) > 0;
    passed &= gPlayerPartyCount == 0;
    if (source == SOURCE_OUTBREAK)
    {
        passed &= GetMonData(&gEnemyParty[0], MON_DATA_SPECIES) == SPECIES_DITTO;
        passed &= GetMonData(&gEnemyParty[0], MON_DATA_LEVEL) == 7;
        passed &= GetMonData(&gEnemyParty[0], MON_DATA_MOVE1) == MOVE_TRANSFORM;
    }
    if (source == SOURCE_FISHING)
        passed &= gIsFishingEncounter;
    RestoreEncounterSourceTestState(&snapshot);
    EXPECT(passed);
}

#endif

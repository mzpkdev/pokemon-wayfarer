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

static void SetSourceParty(u32 variant)
{
    u16 hp = 0;
    u16 heldItem = ITEM_CLEANSE_TAG;
    bool8 egg = TRUE;

    ZeroPlayerPartyMons();
    gPlayerPartyCount = 0;
    if (variant != 0)
    {
        CreateMon(&gPlayerParty[0], SPECIES_ABRA, 100, 0, OTID_STRUCT_PLAYER_ID);
        SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
        SetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM, &heldItem);
        if (variant == 2)
            SetMonData(&gPlayerParty[0], MON_DATA_IS_EGG, &egg);
        gPlayerPartyCount = 1;
    }
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

static void PrepareSource(u32 source, u32 variant)
{
    SetSourceParty(variant);
    TrainerOnlyResetEncounter();
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

TEST("Trainer-only native encounter sources generate and initialize without a usable lead")
{
    u32 source = SOURCE_LAND, variant = 0;
    for (u32 i = SOURCE_LAND; i <= SOURCE_SWEET_SCENT; i++)
        for (u32 j = 0; j < 3; j++)
            PARAMETRIZE(source = i, variant = j);

    struct EncounterSourceTestSnapshot snapshot;
    u32 seed;
    bool32 started = FALSE;
    bool32 passed = TRUE;

    SnapshotEncounterSourceTestState(&snapshot);

    PrepareSource(source, variant);
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
    passed &= gPlayerPartyCount == (variant == 0 ? 0 : 1);
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

TEST("Trainer-only native sources ignore fainted and Egg lead stats items and Repel comparisons")
{
    u32 source = SOURCE_LAND;
    for (u32 i = SOURCE_LAND; i <= SOURCE_SWEET_SCENT; i++)
        PARAMETRIZE(source = i);

    struct EncounterSourceTestSnapshot snapshot;
    bool32 passed = TRUE;

    SnapshotEncounterSourceTestState(&snapshot);
    for (u32 seed = 0; seed < 8; seed++)
    {
        u32 personality = 0, species = 0, level = 0;
        bool32 emptyStarted = FALSE;
        for (u32 variant = 0; variant < 3; variant++)
        {
            bool32 started;

            PrepareSource(source, variant);
            passed &= SelectGrassFromActualLayout();
            SeedRng(seed);
            started = TrySource(source);
            if (variant == 0)
            {
                emptyStarted = started;
                personality = GetMonData(&gEnemyParty[0], MON_DATA_PERSONALITY);
                species = GetMonData(&gEnemyParty[0], MON_DATA_SPECIES);
                level = GetMonData(&gEnemyParty[0], MON_DATA_LEVEL);
            }
            else
            {
                u32 actualPersonality = GetMonData(&gEnemyParty[0], MON_DATA_PERSONALITY);
                u32 actualSpecies = GetMonData(&gEnemyParty[0], MON_DATA_SPECIES);
                u32 actualLevel = GetMonData(&gEnemyParty[0], MON_DATA_LEVEL);

                if (started != emptyStarted
                 || actualPersonality != personality
                 || actualSpecies != species
                 || actualLevel != level)
                {
                    Test_MgbaPrintf("trainer-only source mismatch: source %d seed %d variant %d; started %d/%d personality %d/%d species %d/%d level %d/%d",
                                    source, seed, variant, started, emptyStarted,
                                    actualPersonality, personality, actualSpecies, species, actualLevel, level);
                    passed = FALSE;
                }
            }
        }
    }
    RestoreEncounterSourceTestState(&snapshot);
    EXPECT(passed);
}
#endif

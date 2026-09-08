#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "trainer_see.h"
#include "wild_encounter.h"
#include "wayfarer_origin.h"
#include "trainer_only_encounter.h"
#include "random.h"
#include "test/test.h"
#include "constants/species.h"

#if IS_WAYFARER
TEST("Wayfarer empty party admits trainer-only wild encounters without enabling trainers")
{
    ZeroPlayerPartyMons();
    gPlayerPartyCount = 0;
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_JOHTO_STARTER_RECEIVED);
    FlagSet(FLAG_HOENN_STARTER_RECEIVED);

    EXPECT(!WayfarerCanStartOrdinaryBattle());
    EXPECT(!StandardWildEncounter(0, 0));
    EXPECT(TrainerOnlyCanEnterWildEncounter());
    EXPECT(!CheckForTrainersWantingBattle());
}

TEST("Wayfarer battle admission uses a usable party without stock origin milestones")
{
    u16 hp = 0;
    bool8 egg = TRUE;

    ZeroPlayerPartyMons();
    FlagClear(FLAG_SYS_POKEMON_GET);
    FlagClear(FLAG_JOHTO_STARTER_RECEIVED);
    FlagClear(FLAG_HOENN_STARTER_RECEIVED);
    CreateMon(&gPlayerParty[0], SPECIES_RATTATA, 5, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&gPlayerParty[0]);
    gPlayerPartyCount = 1;
    EXPECT(WayfarerCanStartOrdinaryBattle());
    EXPECT(!TrainerOnlyCanEnterWildEncounter());

    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    EXPECT(!WayfarerCanStartOrdinaryBattle());
    EXPECT(TrainerOnlyCanEnterWildEncounter());
    HealPlayerParty();
    SetMonData(&gPlayerParty[0], MON_DATA_IS_EGG, &egg);
    EXPECT(!WayfarerCanStartOrdinaryBattle());
    EXPECT(TrainerOnlyCanEnterWildEncounter());
}
TEST("Trainer-only eligibility preserves Safari and Bug Contest exclusions")
{
    ZeroPlayerPartyMons();
    FlagSet(FLAG_SYS_SAFARI_MODE);
    EXPECT(!TrainerOnlyCanEnterWildEncounter());
    FlagClear(FLAG_SYS_SAFARI_MODE);
    FlagSet(FLAG_SYS_BUG_CONTEST_MODE);
    EXPECT(!TrainerOnlyCanEnterWildEncounter());
    FlagClear(FLAG_SYS_BUG_CONTEST_MODE);
    EXPECT(TrainerOnlyCanEnterWildEncounter());
}

TEST("Trainer-only wild generation ignores an unusable Synchronize lead")
{
    u16 hp = 0;
    u32 personality;
    u32 seed;

    for (seed = 0; seed < 16; seed++)
    {
        ZeroPlayerPartyMons();
        SeedRng(seed);
        CreateWildMon(SPECIES_RATTATA, 5);
        personality = GetMonData(&gEnemyParty[0], MON_DATA_PERSONALITY);

        CreateMon(&gPlayerParty[0], SPECIES_ABRA, 50, 0, OTID_STRUCT_PLAYER_ID);
        SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
        SeedRng(seed);
        CreateWildMon(SPECIES_RATTATA, 5);
        EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_PERSONALITY), personality);
    }
}
#endif

#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "trainer_only_encounter.h"
#include "test/test.h"
#include "constants/species.h"

#if IS_WAYFARER
TEST("Trainer-only admission requires the map-local opt-in flag and clears it on map reset")
{
    ZeroPlayerPartyMons();
    gPlayerPartyCount = 0;
    FlagClear(FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS);
    EXPECT(!TrainerOnlyCanEnterWildEncounter());
    FlagSet(FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS);
    EXPECT(TrainerOnlyCanEnterWildEncounter());
    ClearTempFieldEventData();
    EXPECT(!TrainerOnlyCanEnterWildEncounter());
}

TEST("Trainer-only admission rejects every nonempty party")
{
    u16 hp = 0;
    bool8 egg = TRUE;

    ZeroPlayerPartyMons();
    FlagSet(FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS);
    CreateMon(&gPlayerParty[0], SPECIES_RATTATA, 5, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&gPlayerParty[0]);
    gPlayerPartyCount = 1;
    EXPECT(!TrainerOnlyCanEnterWildEncounter());

    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    EXPECT(!TrainerOnlyCanEnterWildEncounter());
    SetMonData(&gPlayerParty[0], MON_DATA_IS_EGG, &egg);
    EXPECT(!TrainerOnlyCanEnterWildEncounter());
}

#endif

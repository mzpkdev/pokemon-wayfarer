#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "trainer_see.h"
#include "wild_encounter.h"
#include "wayfarer_origin.h"
#include "test/test.h"
#include "constants/species.h"

#if IS_WAYFARER
TEST("Wayfarer empty party blocks encounters despite saved starter receipt flags")
{
    ZeroPlayerPartyMons();
    gPlayerPartyCount = 0;
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_JOHTO_STARTER_RECEIVED);
    FlagSet(FLAG_HOENN_STARTER_RECEIVED);

    EXPECT(!WayfarerCanStartOrdinaryBattle());
    EXPECT(!StandardWildEncounter(0, 0));
    EXPECT(!SweetScentWildEncounter());
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

    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    EXPECT(!WayfarerCanStartOrdinaryBattle());
    HealPlayerParty();
    SetMonData(&gPlayerParty[0], MON_DATA_IS_EGG, &egg);
    EXPECT(!WayfarerCanStartOrdinaryBattle());
}
#endif

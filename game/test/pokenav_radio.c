#include "global.h"
#include "event_data.h"
#include "test/test.h"
#include "trainer_rating.h"
#include "wild_encounter.h"

#if IS_HNS
bool8 PickOakPokemonTalkSpeciesForTesting(u16 headerId, enum TimeOfDay timeOfDay, u8 firstSlot, u16 *species);

static bool8 FindWildHeaderForMap(u16 map, u16 *headerId)
{
    u16 candidate;

    for (candidate = 0; gWildMonHeaders[candidate].mapGroup != MAP_GROUP(MAP_UNDEFINED); candidate++)
    {
        u16 headerMap = (gWildMonHeaders[candidate].mapGroup << 8) | gWildMonHeaders[candidate].mapNum;

        if (headerMap == map)
        {
            *headerId = candidate;
            return TRUE;
        }
    }

    return FALSE;
}

TEST("Oak's Pokemon Talk announces an eligible effective species from a rebalanced Johto slot")
{
    struct WildEncounterProfileContext context;
    struct WildEncounterProfileView view;
    const struct WildPokemon *entry;
    u16 headerId = HEADER_NONE;
    u16 species = SPECIES_NONE;

    VarSet(VAR_TRAINER_RATING, 10);
    ASSUME(GetTrainerRating() == 10);
    ASSUME(FindWildHeaderForMap(MAP_ROUTE30_HNS, &headerId));

    context.headerId = headerId;
    context.timeOfDay = TIME_DAY;
    context.area = WILD_AREA_LAND;
    context.fishingRod = WILD_ENCOUNTER_FISHING_ROD_NONE;
    ASSUME(GetWildEncounterProfileView(&context, &view));
    ASSUME(GetWildEncounterProfileEntry(&view, 2, &entry));

    EXPECT_EQ(entry->species, SPECIES_LEDYBA);
    EXPECT(IsWildEncounterProfileSlotEligible(&view, 2, 10, FALSE));
    EXPECT(PickOakPokemonTalkSpeciesForTesting(headerId, TIME_DAY, 2, &species));
    EXPECT_EQ(species, SPECIES_LEDYBA);
}
#endif

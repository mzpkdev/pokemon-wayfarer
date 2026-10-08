#include "global.h"
#include "event_data.h"
#include "test/test.h"
#include "trainer_rating.h"
#include "wild_encounter.h"
#include "constants/maps.h"

#if IS_HNS
bool8 PickOakPokemonTalkSpeciesForTesting(u16 headerId, enum TimeOfDay timeOfDay, u8 firstSlot, u16 *species);

TEST("Oak's Pokemon Talk names the likeliest species of its slot from the day or night distribution")
{
    static const u16 sRatings[] = { 0, 40, 80, 160 };
    static const enum TimeOfDay sTimes[] = { TIME_MORNING, TIME_DAY, TIME_EVENING, TIME_NIGHT };
    u32 header = GetWildMonHeaderIdForMap(MAP_GROUP(MAP_ROUTE30_HNS), MAP_NUM(MAP_ROUTE30_HNS));
    u32 r, t, slot, i;

    EXPECT_NE(header, HEADER_NONE);
    for (r = 0; r < ARRAY_COUNT(sRatings); r++)
    {
        SetTrainerRating(sRatings[r]);
        for (t = 0; t < ARRAY_COUNT(sTimes); t++)
        {
            struct WildEncounterProfileContext context = { header, sTimes[t], WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE };
            struct WildEncounterProfileView view;

            EXPECT(GetWildEncounterProfileView(&context, &view));
            for (slot = 2; slot <= 4; slot++)
            {
                struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
                u32 count = GetCurrentWildEncounterSlotOutcomes(&view, slot, outcomes);
                u16 species = SPECIES_NONE;
                bool32 inDistribution = FALSE;

                EXPECT(PickOakPokemonTalkSpeciesForTesting(header, sTimes[t], slot, &species));
                for (i = 0; i < count; i++)
                    inDistribution |= outcomes[i].species == species;
                EXPECT(inDistribution);
                EXPECT_EQ(species, GetCurrentWildEncounterSlotLikelySpecies(&view, slot));
            }
        }
    }
}
#endif

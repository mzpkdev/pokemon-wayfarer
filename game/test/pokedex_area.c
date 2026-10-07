#include "global.h"
#include "pokedex_area_screen.h"
#include "test/test.h"
#include "trainer_rating.h"
#include "wild_encounter.h"
#include "constants/maps.h"

static const struct WildPokemon sPokedexFishingMons[FISH_WILD_COUNT] =
{
    { 50, 50, SPECIES_MAGIKARP }, { 50, 50, SPECIES_MAGIKARP },
    { 50, 50, SPECIES_TENTACOOL }, { 50, 50, SPECIES_TENTACOOL },
    { 50, 50, SPECIES_TENTACOOL }, { 50, 50, SPECIES_WAILMER },
    { 50, 50, SPECIES_WAILMER }, { 50, 50, SPECIES_WAILMER },
    { 50, 50, SPECIES_WAILMER }, { 50, 50, SPECIES_NONE },
};

static const struct WildPokemonInfo sPokedexFishingInfo =
{
    .encounterRate = 1,
    .wildPokemon = sPokedexFishingMons,
};

TEST("Pokedex area: fishing population includes every slot for every quality")
{
    struct WildEncounterProfileView view =
    {
        .wildMonsInfo = &sPokedexFishingInfo,
        .headerId = 0, // a real place: the shown species come from its level
        .timeOfDay = TIME_OF_DAY_DEFAULT,
        .area = WILD_AREA_FISHING,
        .entryStart = 0,
        .entryCount = FISH_WILD_COUNT,
    };

    for (u8 rod = WILD_ENCOUNTER_FISHING_ROD_OLD; rod <= WILD_ENCOUNTER_FISHING_ROD_SUPER; rod++)
    {
        view.fishingRod = rod;
        view.weights = gStandardRodFishingWeights[rod];
        EXPECT(PokedexArea_ProfileViewHasSpeciesForTesting(&view, SPECIES_MAGIKARP));
        EXPECT(PokedexArea_ProfileViewHasSpeciesForTesting(&view, SPECIES_TENTACOOL));
        EXPECT(PokedexArea_ProfileViewHasSpeciesForTesting(&view, SPECIES_WAILMER));
        EXPECT(!PokedexArea_ProfileViewHasSpeciesForTesting(&view, SPECIES_NONE));
    }
}

#if IS_WAYFARER
TEST("Pokedex area: a species shows where the slot distribution holds it at the current Trainer Rating")
{
    static const u16 sRatings[] = { 0, 40, 60, 160 };
    static const u8 weights[] = { 100 };
    u32 rating = 0, parameter, header, slot, time;

    for (parameter = 0; parameter < ARRAY_COUNT(sRatings); parameter++)
        PARAMETRIZE_LABEL("TR %d", sRatings[parameter]) { rating = sRatings[parameter]; }

    SetTrainerRating(rating);
    for (header = 0; gWildMonHeaders[header].mapGroup != MAP_GROUP(MAP_UNDEFINED); header++)
    {
        for (time = TIME_DAY; time <= TIME_NIGHT; time += TIME_NIGHT - TIME_DAY)
        {
            struct WildEncounterProfileContext context = { header, time, WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE };
            struct WildEncounterProfileView view;

            if (!GetWildEncounterProfileView(&context, &view))
                continue;
            for (slot = 0; slot < LAND_WILD_COUNT; slot++)
            {
                struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
                const struct WildPokemon *entry;
                struct WildPokemon mon;
                struct WildPokemonInfo info = { 1, &mon };
                struct WildEncounterProfileView single = view;
                u32 count = GetCurrentWildEncounterSlotOutcomes(&view, slot, outcomes);
                bool32 capShown = FALSE;
                u32 i;

                EXPECT(GetWildEncounterProfileEntry(&view, slot, &entry));
                mon = *entry;
                for (i = 0; i < count; i++)
                    capShown |= outcomes[i].species == entry->species;
                // A single-slot view of the same place shows the slot's species exactly when an
                // outcome holds it (a stepped-down Raichu slot shows Pikachu, not Raichu), and shows
                // the species of its first outcome.
                single.wildMonsInfo = &info;
                single.weights = weights;
                single.entryCount = 1;
                EXPECT_EQ(PokedexArea_ProfileViewHasSpeciesForTesting(&single, entry->species), capShown);
                if (count != 0)
                    EXPECT(PokedexArea_ProfileViewHasSpeciesForTesting(&single, outcomes[0].species));
            }
        }
    }
}
#endif

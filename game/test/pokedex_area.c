#include "global.h"
#include "pokedex_area_screen.h"
#include "test/test.h"
#include "wild_encounter.h"

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

TEST("Standard Rod: Pokedex fishing population includes every former rarity band for every quality")
{
    struct WildEncounterProfileView view =
    {
        .wildMonsInfo = &sPokedexFishingInfo,
        .headerId = HEADER_NONE,
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

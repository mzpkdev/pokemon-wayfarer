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

TEST("Pokedex area: morning shows the day tables and evening the night tables")
{
    EXPECT_EQ(PokedexArea_GetTableTimeOfDayForTesting(TIME_MORNING), TIME_DAY);
    EXPECT_EQ(PokedexArea_GetTableTimeOfDayForTesting(TIME_DAY), TIME_DAY);
    EXPECT_EQ(PokedexArea_GetTableTimeOfDayForTesting(TIME_EVENING), TIME_NIGHT);
    EXPECT_EQ(PokedexArea_GetTableTimeOfDayForTesting(TIME_NIGHT), TIME_NIGHT);
}

TEST("Pokedex area: the Bug-Catching Contest map shows a species held by any of its three day tables")
{
    u32 first = GetWildMonHeaderIdForMap(MAP_GROUP(MAP_NATIONAL_PARK_BUG_CONTEST_HNS), MAP_NUM(MAP_NATIONAL_PARK_BUG_CONTEST_HNS));
    u32 day, slot, other, found = 0;

    SetTrainerRating(40);
    EXPECT_NE(first, HEADER_NONE);
    for (day = 0; day < 3; day++)
    {
        struct WildEncounterProfileContext context = { first + day, TIME_DAY, WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE };
        struct WildEncounterProfileView view;

        EXPECT(GetWildEncounterProfileView(&context, &view));
        for (slot = 0; slot < LAND_WILD_COUNT; slot++)
        {
            struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
            u32 count = GetCurrentWildEncounterSlotOutcomes(&view, slot, outcomes);
            bool32 inOther = FALSE;

            if (count == 0)
                continue;
            // A species of one day's table that no other day's table holds.
            for (other = 0; other < 3; other++)
            {
                struct WildEncounterProfileContext otherContext = { first + other, TIME_DAY, WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE };
                struct WildEncounterProfileView otherView;

                if (other != day && GetWildEncounterProfileView(&otherContext, &otherView))
                    inOther |= PokedexArea_ProfileViewHasSpeciesForTesting(&otherView, outcomes[0].species);
            }
            if (!inOther)
            {
                found |= 1 << day;
                EXPECT(PokedexArea_SpeciesShownOnMapForTesting(outcomes[0].species, MAP_GROUP(MAP_NATIONAL_PARK_BUG_CONTEST_HNS), MAP_NUM(MAP_NATIONAL_PARK_BUG_CONTEST_HNS), TIME_DAY));
                break;
            }
        }
    }
    EXPECT_NE(found, 0);
    // A species no table of the map holds is not marked.
    EXPECT(!PokedexArea_SpeciesShownOnMapForTesting(SPECIES_MEWTWO, MAP_GROUP(MAP_NATIONAL_PARK_BUG_CONTEST_HNS), MAP_NUM(MAP_NATIONAL_PARK_BUG_CONTEST_HNS), TIME_DAY));
}

TEST("Pokedex area: the time of day picks the table the area search reads")
{
    u32 header, slot;
    bool32 differs = FALSE;

    SetTrainerRating(80);
    for (header = 0; header < gWildMonHeaderCount && !differs; header++)
    {
        struct WildEncounterProfileContext day = { header, TIME_DAY, WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE };
        struct WildEncounterProfileContext night = { header, TIME_NIGHT, WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE };
        struct WildEncounterProfileView dayView, nightView;

        if (gWildMonHeaders[header].mapGroup != MAP_GROUP(MAP_NEW_BARK_TOWN_HNS))
            continue;
        if (!GetWildEncounterProfileView(&day, &dayView) || !GetWildEncounterProfileView(&night, &nightView))
            continue;
        for (slot = 0; slot < LAND_WILD_COUNT && !differs; slot++)
        {
            const struct WildPokemon *dayEntry, *nightEntry;

            if (!GetWildEncounterProfileEntry(&dayView, slot, &dayEntry) || !GetWildEncounterProfileEntry(&nightView, slot, &nightEntry)
             || dayEntry->species == nightEntry->species
             || !PokedexArea_ProfileViewHasSpeciesForTesting(&dayView, dayEntry->species)
             || !PokedexArea_ProfileViewHasSpeciesForTesting(&nightView, nightEntry->species)
             || PokedexArea_ProfileViewHasSpeciesForTesting(&nightView, dayEntry->species)
             || PokedexArea_ProfileViewHasSpeciesForTesting(&dayView, nightEntry->species))
                continue;
            // A map's day-only species is read from the day table in the morning as by day, and its
            // night-only species from the night table in the evening as at night. The highlight list
            // is bounded, so compare the readings of one species under the two names of a table.
            u8 group = gWildMonHeaders[header].mapGroup, num = gWildMonHeaders[header].mapNum;

            if (!PokedexArea_SpeciesShownOnMapForTesting(dayEntry->species, group, num, TIME_DAY)
             || !PokedexArea_SpeciesShownOnMapForTesting(nightEntry->species, group, num, TIME_NIGHT))
                continue;
            differs = TRUE;
            EXPECT(PokedexArea_SpeciesShownOnMapForTesting(dayEntry->species, group, num, TIME_MORNING));
            EXPECT(PokedexArea_SpeciesShownOnMapForTesting(nightEntry->species, group, num, TIME_EVENING));
        }
    }
    EXPECT(differs);
}
#endif

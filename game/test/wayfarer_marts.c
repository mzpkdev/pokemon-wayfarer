#include "global.h"
#include "config/wayfarer_marts.h"
#include "event_data.h"
#include "test/test.h"
#include "trainer_rating.h"
#include "wayfarer_marts.h"
#include "constants/items.h"
#include "constants/maps.h"

#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED

struct ExpectedMartItem
{
    u8 minimumTrainerRating;
    u8 category;
    u16 item;
};

struct ExpectedProfile
{
    u16 id;
    u8 commonCategoryMask;
    bool8 supportsPpRecovery;
    u8 catalogCategoryMask;
    const u16 *signature;
    u8 signatureCount;
    const u16 *retained;
    u8 retainedCount;
};

static const struct ExpectedMartItem sExpectedCommonItems[] =
{
    {  0, MART_COMMON_BALLS,       ITEM_POKE_BALL },
    {  4, MART_COMMON_BALLS,       ITEM_GREAT_BALL },
    { 30, MART_COMMON_BALLS,       ITEM_ULTRA_BALL },
    {  0, MART_COMMON_HP_MEDICINE, ITEM_POTION },
    {  4, MART_COMMON_HP_MEDICINE, ITEM_SUPER_POTION },
    { 30, MART_COMMON_HP_MEDICINE, ITEM_HYPER_POTION },
    { 55, MART_COMMON_HP_MEDICINE, ITEM_MAX_POTION },
    { 55, MART_COMMON_HP_MEDICINE, ITEM_FULL_RESTORE },
    {  0, MART_COMMON_STATUS_CURE, ITEM_ANTIDOTE },
    {  0, MART_COMMON_STATUS_CURE, ITEM_PARALYZE_HEAL },
    {  0, MART_COMMON_STATUS_CURE, ITEM_AWAKENING },
    {  0, MART_COMMON_STATUS_CURE, ITEM_BURN_HEAL },
    {  0, MART_COMMON_STATUS_CURE, ITEM_ICE_HEAL },
    { 40, MART_COMMON_STATUS_CURE, ITEM_FULL_HEAL },
    { 30, MART_COMMON_REVIVE,      ITEM_REVIVE },
    {  0, MART_COMMON_REPELS,      ITEM_REPEL },
    { 16, MART_COMMON_REPELS,      ITEM_SUPER_REPEL },
    { 40, MART_COMMON_REPELS,      ITEM_MAX_REPEL },
    {  0, MART_COMMON_ESCAPE_ROPE, ITEM_ESCAPE_ROPE },
};

static const struct ExpectedMartItem sExpectedPpItems[] =
{
    {  0, 0, ITEM_ETHER },
    { 16, 0, ITEM_ELIXIR },
    { 30, 0, ITEM_MAX_ETHER },
    { 55, 0, ITEM_MAX_ELIXIR },
};

static const u16 sCherrygrove[] = { ITEM_HEAL_BALL, ITEM_POKE_DOLL };
static const u16 sViolet[] = { ITEM_NEST_BALL, ITEM_X_ACCURACY };
static const u16 sAzalea[] = { ITEM_NET_BALL, ITEM_NEST_BALL, ITEM_WOOD_MAIL };
static const u16 sGoldenrod[] = { ITEM_FIRE_STONE, ITEM_WATER_STONE, ITEM_THUNDER_STONE, ITEM_LUXURY_BALL };
static const u16 sEcruteak[] = { ITEM_DUSK_BALL, ITEM_POKE_DOLL, ITEM_RETRO_MAIL };
static const u16 sOlivine[] = { ITEM_NET_BALL, ITEM_HEAL_BALL, ITEM_HARBOR_MAIL };
static const u16 sBlackthorn[] = { ITEM_DUSK_BALL, ITEM_TIMER_BALL, ITEM_GUARD_SPEC };
static const u16 sMahogany[] = { ITEM_NET_BALL, ITEM_DUSK_BALL };
static const u16 sViridian[] = { ITEM_NEST_BALL, ITEM_POKE_DOLL, ITEM_ORANGE_MAIL };
static const u16 sPewter[] = { ITEM_DUSK_BALL, ITEM_X_DEFENSE };
static const u16 sCerulean[] = { ITEM_NET_BALL, ITEM_HEAL_BALL, ITEM_X_SPEED };
static const u16 sVermilion[] = { ITEM_NET_BALL, ITEM_DIVE_BALL, ITEM_HARBOR_MAIL };
static const u16 sLavender[] = { ITEM_DUSK_BALL, ITEM_HEAL_BALL, ITEM_SHADOW_MAIL };
static const u16 sCeladon[] = { ITEM_LUXURY_BALL, ITEM_POKE_DOLL, ITEM_RETRO_MAIL };
static const u16 sSaffron[] = { ITEM_X_SP_ATK, ITEM_X_SP_DEF, ITEM_GUARD_SPEC };
static const u16 sFuchsia[] = { ITEM_NET_BALL, ITEM_NEST_BALL, ITEM_FLUFFY_TAIL };
static const u16 sOldale[] = { ITEM_HEAL_BALL, ITEM_NEST_BALL };
static const u16 sPetalburg[] = { ITEM_NEST_BALL, ITEM_X_DEFENSE, ITEM_ORANGE_MAIL };
static const u16 sRustboro[] = { ITEM_TIMER_BALL, ITEM_REPEAT_BALL };
static const u16 sSlateport[] = { ITEM_NET_BALL, ITEM_DIVE_BALL, ITEM_HARBOR_MAIL, ITEM_LUXURY_BALL };
static const u16 sMauville[] = { ITEM_X_SPEED, ITEM_X_SP_ATK, ITEM_X_ACCURACY, ITEM_MECH_MAIL };
static const u16 sVerdanturf[] = { ITEM_NEST_BALL, ITEM_FLUFFY_TAIL };
static const u16 sFallarbor[] = { ITEM_DUSK_BALL, ITEM_DIRE_HIT, ITEM_X_DEFENSE };
static const u16 sLavaridge[] = { ITEM_HEAL_BALL, ITEM_GUARD_SPEC, ITEM_X_SP_DEF };
static const u16 sFortree[] = { ITEM_NEST_BALL, ITEM_NET_BALL, ITEM_WOOD_MAIL, ITEM_X_SPEED };
static const u16 sMossdeep[] = { ITEM_NET_BALL, ITEM_DIVE_BALL };
static const u16 sSootopolis[] = { ITEM_DIVE_BALL, ITEM_DUSK_BALL, ITEM_SHADOW_MAIL };
static const u16 sLilycoveLeft[] = { ITEM_LUXURY_BALL, ITEM_FLUFFY_TAIL };
static const u16 sLilycoveRight[] = { ITEM_WAVE_MAIL, ITEM_MECH_MAIL };

static const u16 sGoldenrodRetained[] = { ITEM_FIRE_STONE, ITEM_WATER_STONE, ITEM_THUNDER_STONE };
static const u16 sPetalburgRetained[] = { ITEM_X_SPEED, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_ORANGE_MAIL };
static const u16 sRustboroRetained[] = { ITEM_X_SPEED, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_TIMER_BALL, ITEM_REPEAT_BALL };
static const u16 sSlateportRetained[] = { ITEM_HARBOR_MAIL };
static const u16 sMauvilleRetained[] = { ITEM_X_SPEED, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_GUARD_SPEC, ITEM_DIRE_HIT, ITEM_X_ACCURACY };
static const u16 sVerdanturfRetained[] = { ITEM_NEST_BALL, ITEM_X_SP_ATK, ITEM_FLUFFY_TAIL };
static const u16 sFallarborRetained[] = { ITEM_X_SP_ATK, ITEM_X_SPEED, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_DIRE_HIT, ITEM_GUARD_SPEC };
static const u16 sLavaridgeRetained[] = { ITEM_X_SPEED };
static const u16 sFortreeRetained[] = { ITEM_WOOD_MAIL };
static const u16 sMossdeepRetained[] = { ITEM_NET_BALL, ITEM_DIVE_BALL, ITEM_X_ATTACK, ITEM_X_DEFENSE };
static const u16 sSootopolisRetained[] = { ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_SHADOW_MAIL };
static const u16 sLilycoveLeftRetained[] = { ITEM_FLUFFY_TAIL };
static const u16 sLilycoveRightRetained[] = { ITEM_WAVE_MAIL, ITEM_MECH_MAIL };
static const u16 sVitamins[] = { ITEM_PROTEIN, ITEM_CALCIUM, ITEM_IRON, ITEM_ZINC, ITEM_CARBOS, ITEM_HP_UP };
static const u16 sTrainerHillRetained[] = { ITEM_X_SPEED, ITEM_X_SP_ATK, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_DIRE_HIT, ITEM_GUARD_SPEC, ITEM_X_ACCURACY };

#define PROFILE(id, common, pp, category, signature, retained) \
    { id, common, pp, category, signature, ARRAY_COUNT(signature), retained, ARRAY_COUNT(retained) }
#define PROFILE_NO_RETAINED(id, common, pp, category, signature) \
    { id, common, pp, category, signature, ARRAY_COUNT(signature), NULL, 0 }
#define PROFILE_FACILITY(id, category, retained) \
    { id, MART_COMMON_ALL, TRUE, category, NULL, 0, retained, ARRAY_COUNT(retained) }
#define PROFILE_EMPTY_FACILITY(id, category) \
    { id, MART_COMMON_ALL, TRUE, category, NULL, 0, NULL, 0 }

static const struct ExpectedProfile sExpectedProfiles[] =
{
    PROFILE_NO_RETAINED(MART_PROFILE_CHERRYGROVE, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sCherrygrove),
    PROFILE_NO_RETAINED(MART_PROFILE_VIOLET, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sViolet),
    PROFILE_NO_RETAINED(MART_PROFILE_AZALEA, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sAzalea),
    PROFILE(MART_PROFILE_GOLDENROD_2F, MART_COMMON_ALL, TRUE, MART_CATEGORY_DEPT, sGoldenrod, sGoldenrodRetained),
    PROFILE_NO_RETAINED(MART_PROFILE_ECRUTEAK, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sEcruteak),
    PROFILE_NO_RETAINED(MART_PROFILE_OLIVINE, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sOlivine),
    PROFILE_NO_RETAINED(MART_PROFILE_BLACKTHORN, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sBlackthorn),
    PROFILE_NO_RETAINED(MART_PROFILE_MAHOGANY, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sMahogany),
    PROFILE_NO_RETAINED(MART_PROFILE_VIRIDIAN, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sViridian),
    PROFILE_NO_RETAINED(MART_PROFILE_PEWTER, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sPewter),
    PROFILE_NO_RETAINED(MART_PROFILE_CERULEAN, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sCerulean),
    PROFILE_NO_RETAINED(MART_PROFILE_VERMILION, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sVermilion),
    PROFILE_NO_RETAINED(MART_PROFILE_LAVENDER, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sLavender),
    PROFILE_NO_RETAINED(MART_PROFILE_CELADON_2F, MART_COMMON_ALL, TRUE, MART_CATEGORY_DEPT, sCeladon),
    PROFILE_NO_RETAINED(MART_PROFILE_SAFFRON, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sSaffron),
    PROFILE_NO_RETAINED(MART_PROFILE_FUCHSIA, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sFuchsia),
    PROFILE_NO_RETAINED(MART_PROFILE_OLDALE, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sOldale),
    PROFILE(MART_PROFILE_PETALBURG, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sPetalburg, sPetalburgRetained),
    PROFILE(MART_PROFILE_RUSTBORO, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sRustboro, sRustboroRetained),
    PROFILE(MART_PROFILE_SLATEPORT, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sSlateport, sSlateportRetained),
    PROFILE(MART_PROFILE_MAUVILLE, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sMauville, sMauvilleRetained),
    PROFILE(MART_PROFILE_VERDANTURF, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sVerdanturf, sVerdanturfRetained),
    PROFILE(MART_PROFILE_FALLARBOR, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sFallarbor, sFallarborRetained),
    PROFILE(MART_PROFILE_LAVARIDGE, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sLavaridge, sLavaridgeRetained),
    PROFILE(MART_PROFILE_FORTREE, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sFortree, sFortreeRetained),
    PROFILE(MART_PROFILE_MOSSDEEP, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sMossdeep, sMossdeepRetained),
    PROFILE(MART_PROFILE_SOOTOPOLIS, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN, sSootopolis, sSootopolisRetained),
    PROFILE(MART_PROFILE_LILYCOVE_2F_LEFT, MART_COMMON_BALLS | MART_COMMON_STATUS_CURE | MART_COMMON_ESCAPE_ROPE, FALSE, MART_CATEGORY_DEPT, sLilycoveLeft, sLilycoveLeftRetained),
    PROFILE(MART_PROFILE_LILYCOVE_2F_RIGHT, MART_COMMON_HP_MEDICINE | MART_COMMON_REVIVE | MART_COMMON_REPELS, TRUE, MART_CATEGORY_DEPT, sLilycoveRight, sLilycoveRightRetained),
    PROFILE_FACILITY(MART_PROFILE_INDIGO_PLATEAU, MART_CATEGORY_LEAGUE, sVitamins),
    PROFILE_EMPTY_FACILITY(MART_PROFILE_EVER_GRANDE_LEAGUE, MART_CATEGORY_LEAGUE),
    PROFILE_FACILITY(MART_PROFILE_BATTLE_FRONTIER_HNS, MART_CATEGORY_FACILITY, sVitamins),
    PROFILE_FACILITY(MART_PROFILE_BATTLE_FRONTIER, MART_CATEGORY_FACILITY, sVitamins),
    PROFILE_EMPTY_FACILITY(MART_PROFILE_TRAINER_HILL_HNS, MART_CATEGORY_FACILITY),
    PROFILE_FACILITY(MART_PROFILE_TRAINER_HILL, MART_CATEGORY_FACILITY, sTrainerHillRetained),
};

#undef PROFILE
#undef PROFILE_NO_RETAINED
#undef PROFILE_FACILITY
#undef PROFILE_EMPTY_FACILITY

static void AppendExpectedItem(u16 *items, u8 *count, u16 item)
{
    u8 i;

    for (i = 0; i < *count; i++)
    {
        if (items[i] == item)
            return;
    }
    items[*count] = item;
    (*count)++;
}

static u8 BuildExpectedCatalog(const struct ExpectedProfile *profile, u8 rating, bool8 challenge, u16 *items)
{
    u8 i;
    u8 count = 0;

    for (i = 0; i < ARRAY_COUNT(sExpectedCommonItems); i++)
    {
        const struct ExpectedMartItem *entry = &sExpectedCommonItems[i];

        if (rating >= entry->minimumTrainerRating && (profile->commonCategoryMask & entry->category) != 0)
            AppendExpectedItem(items, &count, entry->item);
    }
    if (challenge && profile->supportsPpRecovery)
    {
        for (i = 0; i < ARRAY_COUNT(sExpectedPpItems); i++)
        {
            if (rating >= sExpectedPpItems[i].minimumTrainerRating)
                AppendExpectedItem(items, &count, sExpectedPpItems[i].item);
        }
    }
    for (i = 0; i < profile->signatureCount; i++)
        AppendExpectedItem(items, &count, profile->signature[i]);
    for (i = 0; i < profile->retainedCount; i++)
        AppendExpectedItem(items, &count, profile->retained[i]);
    items[count] = ITEM_NONE;
    return count;
}

static bool8 CatalogContains(const u16 *items, u16 item)
{
    u8 i;

    for (i = 0; items[i] != ITEM_NONE; i++)
    {
        if (items[i] == item)
            return TRUE;
    }
    return FALSE;
}

static void ExpectSameItems(const u16 *actual, const u16 *expected)
{
    u8 i;

    for (i = 0; i < WAYFARER_MART_CATALOG_CAPACITY; i++)
    {
        EXPECT_EQ(actual[i], expected[i]);
        if (expected[i] == ITEM_NONE)
            return;
    }
    EXPECT(FALSE);
}

TEST("Wayfarer mart profiles resolve every rating, tier boundary, and challenge state")
{
    u8 profileIndex;

    EXPECT_EQ(ARRAY_COUNT(sExpectedProfiles), MART_PROFILE_COUNT - 1);
    for (profileIndex = 0; profileIndex < ARRAY_COUNT(sExpectedProfiles); profileIndex++)
    {
        const struct ExpectedProfile *expectedProfile = &sExpectedProfiles[profileIndex];
        const struct WayfarerMartProfile *actualProfile = WayfarerGetMartProfile(expectedProfile->id);
        u8 rating;

        EXPECT(actualProfile != NULL);
        if (actualProfile == NULL)
            continue;
        EXPECT_EQ(actualProfile->commonCategoryMask, expectedProfile->commonCategoryMask);
        EXPECT_EQ(actualProfile->supportsPpRecovery, expectedProfile->supportsPpRecovery);
        EXPECT_EQ(actualProfile->catalogCategoryMask, expectedProfile->catalogCategoryMask);
        EXPECT_EQ(actualProfile->signatureItemCount, expectedProfile->signatureCount);
        EXPECT_EQ(actualProfile->retainedItemCount, expectedProfile->retainedCount);
        for (rating = 0; rating < expectedProfile->signatureCount; rating++)
            EXPECT_EQ(actualProfile->signatureItems[rating], expectedProfile->signature[rating]);
        for (rating = 0; rating < expectedProfile->retainedCount; rating++)
            EXPECT_EQ(actualProfile->retainedItems[rating], expectedProfile->retained[rating]);

        for (rating = 0; rating <= TRAINER_RATING_MAX; rating++)
        {
            u8 challenge;
            for (challenge = FALSE; challenge <= TRUE; challenge++)
            {
                u16 actual[WAYFARER_MART_CATALOG_CAPACITY];
                u16 expected[WAYFARER_MART_CATALOG_CAPACITY];
                u8 item;

                EXPECT(WayfarerResolveMartProfile(expectedProfile->id, rating, challenge, actual, ARRAY_COUNT(actual)));
                BuildExpectedCatalog(expectedProfile, rating, challenge, expected);
                ExpectSameItems(actual, expected);
                for (item = 0; actual[item] != ITEM_NONE; item++)
                {
                    u8 previous;
                    for (previous = 0; previous < item; previous++)
                        EXPECT_NE(actual[item], actual[previous]);
                }
            }
        }
    }
}

TEST("Wayfarer town signatures are distinct and two to four items at every tier")
{
    u8 first;
    u8 townCount = 0;

    for (first = 0; first < ARRAY_COUNT(sExpectedProfiles); first++)
    {
        const struct ExpectedProfile *profile = &sExpectedProfiles[first];
        if ((profile->catalogCategoryMask & (MART_CATEGORY_TOWN | MART_CATEGORY_DEPT)) == 0)
            continue;

        townCount++;
        EXPECT_GE(profile->signatureCount, 2);
        EXPECT_LE(profile->signatureCount, 4);
        for (u8 second = first + 1; second < ARRAY_COUNT(sExpectedProfiles); second++)
        {
            const struct ExpectedProfile *other = &sExpectedProfiles[second];
            bool8 same = profile->signatureCount == other->signatureCount;

            if ((other->catalogCategoryMask & (MART_CATEGORY_TOWN | MART_CATEGORY_DEPT)) == 0)
                continue;
            for (u8 item = 0; same && item < profile->signatureCount; item++)
                same = profile->signature[item] == other->signature[item];
            EXPECT(!same);
        }
    }
    EXPECT_EQ(townCount, 29);
}

TEST("Wayfarer mart resolver keeps PP supplements and Lilycove's common split exact")
{
    u16 left[WAYFARER_MART_CATALOG_CAPACITY];
    u16 right[WAYFARER_MART_CATALOG_CAPACITY];
    u8 i;

    EXPECT(WayfarerResolveMartProfile(MART_PROFILE_LILYCOVE_2F_LEFT, 55, TRUE, left, ARRAY_COUNT(left)));
    EXPECT(WayfarerResolveMartProfile(MART_PROFILE_LILYCOVE_2F_RIGHT, 55, TRUE, right, ARRAY_COUNT(right)));
    for (i = 0; i < ARRAY_COUNT(sExpectedCommonItems); i++)
    {
        const struct ExpectedMartItem *entry = &sExpectedCommonItems[i];
        bool8 inLeft = CatalogContains(left, entry->item);
        bool8 inRight = CatalogContains(right, entry->item);

        EXPECT(inLeft != inRight);
        EXPECT(inLeft || inRight);
    }
    EXPECT(!CatalogContains(left, ITEM_ETHER));
    EXPECT(CatalogContains(right, ITEM_ETHER));
    EXPECT(CatalogContains(right, ITEM_ELIXIR));
    EXPECT(CatalogContains(right, ITEM_MAX_ETHER));
    EXPECT(CatalogContains(right, ITEM_MAX_ELIXIR));
}

TEST("Wayfarer mart resolver rejects invalid profiles and capacity without changing output")
{
    u16 small[31];
    u16 full[WAYFARER_MART_CATALOG_CAPACITY];
    u16 clamped[WAYFARER_MART_CATALOG_CAPACITY];
    u8 i;

    for (i = 0; i < ARRAY_COUNT(small); i++)
        small[i] = 0xBEEF;
    EXPECT(!WayfarerResolveMartProfile(MART_PROFILE_MAUVILLE, 55, TRUE, small, ARRAY_COUNT(small)));
    for (i = 0; i < ARRAY_COUNT(small); i++)
        EXPECT_EQ(small[i], 0xBEEF);
    EXPECT(!WayfarerResolveMartProfile(MART_PROFILE_NONE, 55, FALSE, full, ARRAY_COUNT(full)));
    EXPECT(!WayfarerResolveMartProfile(MART_PROFILE_COUNT, 55, FALSE, full, ARRAY_COUNT(full)));
    EXPECT(WayfarerResolveMartProfile(MART_PROFILE_CHERRYGROVE, TRAINER_RATING_MAX, FALSE, full, ARRAY_COUNT(full)));
    EXPECT(WayfarerResolveMartProfile(MART_PROFILE_CHERRYGROVE, 255, FALSE, clamped, ARRAY_COUNT(clamped)));
    ExpectSameItems(clamped, full);
}

TEST("Wayfarer shared-clerk lookup keys only on clerk family and compiled map")
{
    static const struct
    {
        u16 family;
        u8 mapGroup;
        u8 mapNum;
        u16 profile;
    } sKnownBindings[] =
    {
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_CHERRYGROVE_CITY_MART_HNS), MAP_NUM(MAP_CHERRYGROVE_CITY_MART_HNS), MART_PROFILE_CHERRYGROVE },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_AZALEA_TOWN_MART_HNS), MAP_NUM(MAP_AZALEA_TOWN_MART_HNS), MART_PROFILE_AZALEA },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_ECRUTEAK_CITY_MART_HNS), MAP_NUM(MAP_ECRUTEAK_CITY_MART_HNS), MART_PROFILE_ECRUTEAK },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_OLIVINE_CITY_MART_HNS), MAP_NUM(MAP_OLIVINE_CITY_MART_HNS), MART_PROFILE_OLIVINE },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_BLACKTHORN_CITY_MART_HNS), MAP_NUM(MAP_BLACKTHORN_CITY_MART_HNS), MART_PROFILE_BLACKTHORN },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_VIRIDIAN_CITY_MART_HNS), MAP_NUM(MAP_VIRIDIAN_CITY_MART_HNS), MART_PROFILE_VIRIDIAN },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_PEWTER_CITY_MART_HNS), MAP_NUM(MAP_PEWTER_CITY_MART_HNS), MART_PROFILE_PEWTER },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_CERULEAN_CITY_MART_HNS), MAP_NUM(MAP_CERULEAN_CITY_MART_HNS), MART_PROFILE_CERULEAN },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_VERMILION_CITY_MART_HNS), MAP_NUM(MAP_VERMILION_CITY_MART_HNS), MART_PROFILE_VERMILION },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_LAVENDER_TOWN_MART_HNS), MAP_NUM(MAP_LAVENDER_TOWN_MART_HNS), MART_PROFILE_LAVENDER },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_SAFFRON_CITY_MART_HNS), MAP_NUM(MAP_SAFFRON_CITY_MART_HNS), MART_PROFILE_SAFFRON },
        { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_FUCHSIA_CITY_MART_HNS), MAP_NUM(MAP_FUCHSIA_CITY_MART_HNS), MART_PROFILE_FUCHSIA },
        { MART_CLERK_FAMILY_VIOLET, MAP_GROUP(MAP_VIOLET_CITY_MART_HNS), MAP_NUM(MAP_VIOLET_CITY_MART_HNS), MART_PROFILE_VIOLET },
        { MART_CLERK_FAMILY_VIOLET, MAP_GROUP(MAP_CELADON_CITY_DEPARTMENT_STORE_2F_HNS), MAP_NUM(MAP_CELADON_CITY_DEPARTMENT_STORE_2F_HNS), MART_PROFILE_CELADON_2F },
        { MART_CLERK_FAMILY_FRONTIER, MAP_GROUP(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS), MAP_NUM(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS), MART_PROFILE_INDIGO_PLATEAU },
        { MART_CLERK_FAMILY_FRONTIER, MAP_GROUP(MAP_BATTLE_FRONTIER_MART_HNS), MAP_NUM(MAP_BATTLE_FRONTIER_MART_HNS), MART_PROFILE_BATTLE_FRONTIER_HNS },
        { MART_CLERK_FAMILY_FRONTIER, MAP_GROUP(MAP_BATTLE_FRONTIER_MART), MAP_NUM(MAP_BATTLE_FRONTIER_MART), MART_PROFILE_BATTLE_FRONTIER },
    };
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sKnownBindings); i++)
    {
        gSpecialVar_0x8005 = sKnownBindings[i].family;
        gSaveBlock1Ptr->location.mapGroup = sKnownBindings[i].mapGroup;
        gSaveBlock1Ptr->location.mapNum = sKnownBindings[i].mapNum;
        EXPECT_EQ(WayfarerLookupMartProfileForSharedClerk(), sKnownBindings[i].profile);
    }
    gSpecialVar_0x8005 = MART_CLERK_FAMILY_CHERRYGROVE;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_OLDALE_TOWN_MART);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_OLDALE_TOWN_MART);
    EXPECT_EQ(WayfarerLookupMartProfileForSharedClerk(), MART_PROFILE_NONE);
}

#endif

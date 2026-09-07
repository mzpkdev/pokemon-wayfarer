#include "global.h"
#include "config/wayfarer_marts.h"
#include "event_data.h"
#include "gba/isagbprint.h"
#include "item.h"
#include "shop.h"
#include "trainer_rating.h"
#include "wayfarer_marts.h"
#include "constants/items.h"
#include "constants/maps.h"

struct WayfarerMartCommonItem
{
    u8 minimumTrainerRating;
    u8 category;
    u16 item;
};

struct WayfarerMartPpItem
{
    u8 minimumTrainerRating;
    u16 item;
};

struct WayfarerMartSharedClerkBinding
{
    u16 clerkFamily;
    u8 mapGroup;
    u8 mapNum;
    u16 profileId;
};

#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
#include "data/wayfarer_marts.h"
#endif

#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
static EWRAM_DATA u16 sWayfarerMartCatalog[WAYFARER_MART_CATALOG_CAPACITY];

static bool8 IsValidMartItem(u16 item)
{
    return item != ITEM_NONE
        && item < ITEMS_COUNT
        && GetItemPrice(item) != 0
        && GetItemImportance(item) == 0;
}

static bool8 AppendUniqueMartItem(u16 *items, u8 capacity, u8 *count, u16 item)
{
    u8 i;

    if (!IsValidMartItem(item))
        return FALSE;

    for (i = 0; i < *count; i++)
    {
        if (items[i] == item)
            return TRUE;
    }

    // One slot is always reserved for ITEM_NONE.
    if (*count >= capacity - 1)
        return FALSE;

    items[*count] = item;
    (*count)++;
    return TRUE;
}

static bool8 AppendMartItems(u16 *items, u8 capacity, u8 *count, const u16 *source, u8 sourceCount)
{
    u8 i;

    if (sourceCount != 0 && source == NULL)
        return FALSE;

    for (i = 0; i < sourceCount; i++)
    {
        if (!AppendUniqueMartItem(items, capacity, count, source[i]))
            return FALSE;
    }
    return TRUE;
}

static bool8 IsProfileStructValid(const struct WayfarerMartProfile *profile)
{
    if (profile == NULL)
        return FALSE;
    if (profile->catalogCategoryMask == 0)
        return FALSE;
    if ((profile->commonCategoryMask & ~MART_COMMON_ALL) != 0)
        return FALSE;
    if (profile->signatureItemCount != 0 && profile->signatureItems == NULL)
        return FALSE;
    if (profile->retainedItemCount != 0 && profile->retainedItems == NULL)
        return FALSE;
    return TRUE;
}

static bool8 ResolveMartCatalog(const struct WayfarerMartProfile *profile, u8 trainerRating, bool8 challengeEnabled, u16 *items, u8 capacity)
{
    u16 draft[WAYFARER_MART_CATALOG_CAPACITY];
    u8 count = 0;
    u8 i;

    if (items == NULL || capacity < 2 || capacity > ARRAY_COUNT(draft) || !IsProfileStructValid(profile))
        return FALSE;

    // The public getter is already clamped, but this keeps the resolver's
    // behavior defined for host callers and corrupted script inputs as well.
    if (trainerRating > TRAINER_RATING_MAX)
        trainerRating = TRAINER_RATING_MAX;

    for (i = 0; i < ARRAY_COUNT(sWayfarerMartCommonItems); i++)
    {
        const struct WayfarerMartCommonItem *entry = &sWayfarerMartCommonItems[i];

        if (trainerRating >= entry->minimumTrainerRating
         && (profile->commonCategoryMask & entry->category) != 0
         && !AppendUniqueMartItem(draft, capacity, &count, entry->item))
            return FALSE;
    }

    if (challengeEnabled && profile->supportsPpRecovery)
    {
        for (i = 0; i < ARRAY_COUNT(sWayfarerMartPpItems); i++)
        {
            const struct WayfarerMartPpItem *entry = &sWayfarerMartPpItems[i];

            if (trainerRating >= entry->minimumTrainerRating
             && !AppendUniqueMartItem(draft, capacity, &count, entry->item))
                return FALSE;
        }
    }

    if (!AppendMartItems(draft, capacity, &count, profile->signatureItems, profile->signatureItemCount)
     || !AppendMartItems(draft, capacity, &count, profile->retainedItems, profile->retainedItemCount))
        return FALSE;

    draft[count] = ITEM_NONE;
    for (i = 0; i <= count; i++)
        items[i] = draft[i];
    return TRUE;
}

static bool8 ResolveFallbackMartCatalog(u8 trainerRating, bool8 challengeEnabled, u16 *items, u8 capacity)
{
    static const struct WayfarerMartProfile sFallbackProfile =
    {
        .signatureItems = NULL,
        .retainedItems = NULL,
        .signatureItemCount = 0,
        .retainedItemCount = 0,
        .commonCategoryMask = MART_COMMON_ALL,
        .supportsPpRecovery = TRUE,
        .catalogCategoryMask = MART_CATEGORY_TOWN,
    };

    return ResolveMartCatalog(&sFallbackProfile, trainerRating, challengeEnabled, items, capacity);
}
#endif

const struct WayfarerMartProfile *WayfarerGetMartProfile(u16 profileId)
{
#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
    const struct WayfarerMartProfile *profile;

    if (profileId == MART_PROFILE_NONE || profileId >= MART_PROFILE_COUNT)
        return NULL;

    profile = &sWayfarerMartProfiles[profileId];
    return IsProfileStructValid(profile) ? profile : NULL;
#else
    (void)profileId;
    return NULL;
#endif
}

bool8 WayfarerResolveMartProfile(u16 profileId, u8 trainerRating, bool8 challengeEnabled, u16 *items, u8 capacity)
{
#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
    return ResolveMartCatalog(WayfarerGetMartProfile(profileId), trainerRating, challengeEnabled, items, capacity);
#else
    (void)profileId;
    (void)trainerRating;
    (void)challengeEnabled;
    (void)items;
    (void)capacity;
    return FALSE;
#endif
}

void WayfarerOpenMartProfile(void)
{
#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
    const u16 profileId = gSpecialVar_0x8004;
    const u8 trainerRating = GetTrainerRating();
    const bool8 challengeEnabled = gSaveBlock3Ptr->challengeSettings.tx_Challenges_PkmnCenter != 0;

    if (!WayfarerResolveMartProfile(profileId, trainerRating, challengeEnabled, sWayfarerMartCatalog, ARRAY_COUNT(sWayfarerMartCatalog)))
    {
        DebugPrintf("Wayfarer mart fallback: profile %u", profileId);
        if (!ResolveFallbackMartCatalog(trainerRating, challengeEnabled, sWayfarerMartCatalog, ARRAY_COUNT(sWayfarerMartCatalog)))
        {
            // The fallback only contains audited normal-mart essentials. This
            // is an impossible final guard that still gives the UI a valid
            // static list if compiled item data has been corrupted.
            sWayfarerMartCatalog[0] = ITEM_POKE_BALL;
            sWayfarerMartCatalog[1] = ITEM_NONE;
        }
    }
    CreatePokemartMenu(sWayfarerMartCatalog);
#endif
}

u16 WayfarerLookupMartProfileForSharedClerk(void)
{
#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
    u8 i;
    const u16 clerkFamily = gSpecialVar_0x8005;
    const u8 mapGroup = gSaveBlock1Ptr->location.mapGroup;
    const u8 mapNum = gSaveBlock1Ptr->location.mapNum;

    for (i = 0; i < ARRAY_COUNT(sWayfarerMartSharedClerkBindings); i++)
    {
        const struct WayfarerMartSharedClerkBinding *binding = &sWayfarerMartSharedClerkBindings[i];

        if (binding->clerkFamily == clerkFamily
         && binding->mapGroup == mapGroup
         && binding->mapNum == mapNum)
        {
            return binding->profileId;
        }
    }

    return MART_PROFILE_NONE;
#else
    return MART_PROFILE_NONE;
#endif
}

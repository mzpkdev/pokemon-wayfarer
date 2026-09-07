#ifndef GUARD_WAYFARER_MARTS_H
#define GUARD_WAYFARER_MARTS_H

#include "constants/wayfarer_marts.h"

#define WAYFARER_MART_CATALOG_CAPACITY 64

enum WayfarerMartCommonCategory
{
    MART_COMMON_BALLS       = (1 << 0),
    MART_COMMON_HP_MEDICINE = (1 << 1),
    MART_COMMON_STATUS_CURE = (1 << 2),
    MART_COMMON_REVIVE      = (1 << 3),
    MART_COMMON_REPELS      = (1 << 4),
    MART_COMMON_ESCAPE_ROPE = (1 << 5),
    MART_COMMON_ALL = MART_COMMON_BALLS
                    | MART_COMMON_HP_MEDICINE
                    | MART_COMMON_STATUS_CURE
                    | MART_COMMON_REVIVE
                    | MART_COMMON_REPELS
                    | MART_COMMON_ESCAPE_ROPE,
};

enum WayfarerMartCategory
{
    MART_CATEGORY_TOWN     = (1 << 0),
    MART_CATEGORY_DEPT     = (1 << 1),
    MART_CATEGORY_LEAGUE   = (1 << 2),
    MART_CATEGORY_FACILITY = (1 << 3),
};

struct WayfarerMartProfile
{
    const u16 *signatureItems;
    const u16 *retainedItems;
    u8 signatureItemCount;
    u8 retainedItemCount;
    u8 commonCategoryMask;
    bool8 supportsPpRecovery;
    u8 catalogCategoryMask;
};

const struct WayfarerMartProfile *WayfarerGetMartProfile(u16 profileId);
bool8 WayfarerResolveMartProfile(u16 profileId, u8 trainerRating, bool8 challengeEnabled, u16 *items, u8 capacity);
void WayfarerOpenMartProfile(void);
u16 WayfarerLookupMartProfileForSharedClerk(void);

#endif // GUARD_WAYFARER_MARTS_H

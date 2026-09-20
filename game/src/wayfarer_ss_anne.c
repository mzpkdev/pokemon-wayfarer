#include "global.h"
#include "constants/flags.h"
#include "event_data.h"
#include "wayfarer_ss_anne.h"

#if IS_WAYFARER

static const u16 sWayfarerSSAnneTrainerFlags[] =
{
    FLAG_WAYFARER_SS_ANNE_TRAINER_TYLER,
    FLAG_WAYFARER_SS_ANNE_TRAINER_ANN,
    FLAG_WAYFARER_SS_ANNE_TRAINER_ARTHUR,
    FLAG_WAYFARER_SS_ANNE_TRAINER_THOMAS,
    FLAG_WAYFARER_SS_ANNE_TRAINER_DALE,
    FLAG_WAYFARER_SS_ANNE_TRAINER_BROOKS,
    FLAG_WAYFARER_SS_ANNE_TRAINER_LAMAR,
    FLAG_WAYFARER_SS_ANNE_TRAINER_DAWN,
    FLAG_WAYFARER_SS_ANNE_TRAINER_BARNY,
    FLAG_WAYFARER_SS_ANNE_TRAINER_PHILLIP,
    FLAG_WAYFARER_SS_ANNE_TRAINER_HUEY,
    FLAG_WAYFARER_SS_ANNE_TRAINER_DYLAN,
    FLAG_WAYFARER_SS_ANNE_TRAINER_LEONARD,
    FLAG_WAYFARER_SS_ANNE_TRAINER_DUNCAN,
    FLAG_WAYFARER_SS_ANNE_TRAINER_EDMOND,
    FLAG_WAYFARER_SS_ANNE_TRAINER_TREVOR,
};

static const u16 sWayfarerSSAnneReceipts[] =
{
    FLAG_WAYFARER_SS_ANNE_ITEM_TM31, FLAG_WAYFARER_SS_ANNE_ITEM_STARDUST,
    FLAG_WAYFARER_SS_ANNE_ITEM_X_ATTACK, FLAG_WAYFARER_SS_ANNE_ITEM_TM44,
    FLAG_WAYFARER_SS_ANNE_ITEM_ETHER, FLAG_WAYFARER_SS_ANNE_ITEM_SUPER_POTION,
    FLAG_WAYFARER_SS_ANNE_ITEM_GREAT_BALL, FLAG_WAYFARER_SS_ANNE_ITEM_HYPER_POTION,
    FLAG_WAYFARER_SS_ANNE_ITEM_CHESTO_BERRY, FLAG_WAYFARER_SS_ANNE_ITEM_PECHA_BERRY,
    FLAG_WAYFARER_SS_ANNE_ITEM_CHERI_BERRY, FLAG_WAYFARER_SS_ANNE_BLUE_MET,
    FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED,
};

bool32 WayfarerSSAnneTrainerDefeatGet(u16 slot)
{
    return slot < ARRAY_COUNT(sWayfarerSSAnneTrainerFlags) && FlagGet(sWayfarerSSAnneTrainerFlags[slot]);
}

void WayfarerSSAnneTrainerDefeatSet(u16 slot)
{
    if (slot < ARRAY_COUNT(sWayfarerSSAnneTrainerFlags))
        FlagSet(sWayfarerSSAnneTrainerFlags[slot]);
}

void WayfarerSSAnneTrainerDefeatClear(u16 slot)
{
    if (slot < ARRAY_COUNT(sWayfarerSSAnneTrainerFlags))
        FlagClear(sWayfarerSSAnneTrainerFlags[slot]);
}

bool8 WayfarerSSAnneHasUnfinishedContent(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sWayfarerSSAnneTrainerFlags); i++)
        if (!FlagGet(sWayfarerSSAnneTrainerFlags[i]))
            return TRUE;
    for (i = 0; i < ARRAY_COUNT(sWayfarerSSAnneReceipts); i++)
        if (!FlagGet(sWayfarerSSAnneReceipts[i]))
            return TRUE;
    return FALSE;
}

u16 WayfarerSSAnne_HasUnfinishedContent(void)
{
    return WayfarerSSAnneHasUnfinishedContent();
}

#else

bool8 WayfarerSSAnneHasUnfinishedContent(void) { return FALSE; }
u16 WayfarerSSAnne_HasUnfinishedContent(void) { return FALSE; }
bool32 WayfarerSSAnneTrainerDefeatGet(u16 slot) { return FALSE; }
void WayfarerSSAnneTrainerDefeatSet(u16 slot) {}
void WayfarerSSAnneTrainerDefeatClear(u16 slot) {}

#endif

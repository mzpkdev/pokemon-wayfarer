#include "global.h"
#include "wayfarer_coast_trainers.h"

#if IS_WAYFARER

#include "constants/wayfarer_coast_trainers.h"

#define WAYFARER_COAST_REMATCH_STAGES 3

struct WayfarerCoastRematchFamily
{
    u16 trainerIds[WAYFARER_COAST_REMATCH_STAGES];
};

static const struct WayfarerCoastRematchFamily sWayfarerCoastRematchFamilies[WAYFARER_COAST_REMATCH_FAMILY_COUNT] =
{
    {{TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_ALICE, TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_ALICE_2, TRAINER_NONE}},
    {{TRAINER_WAYFARER_COAST_SWIMMER_MALE_DARRIN, TRAINER_WAYFARER_COAST_SWIMMER_MALE_DARRIN_2, TRAINER_NONE}},
    {{TRAINER_WAYFARER_COAST_PICNICKER_MISSY, TRAINER_WAYFARER_COAST_PICNICKER_MISSY_2, TRAINER_WAYFARER_COAST_PICNICKER_MISSY_3}},
    {{TRAINER_WAYFARER_COAST_FISHERMAN_WADE, TRAINER_WAYFARER_COAST_FISHERMAN_WADE_2, TRAINER_NONE}},
    {{TRAINER_WAYFARER_COAST_SWIMMER_MALE_JACK, TRAINER_WAYFARER_COAST_SWIMMER_MALE_JACK_2, TRAINER_NONE}},
    {{TRAINER_WAYFARER_COAST_SIS_AND_BRO_LIL_IAN, TRAINER_WAYFARER_COAST_SIS_AND_BRO_LIL_IAN_2, TRAINER_WAYFARER_COAST_SIS_AND_BRO_LIL_IAN_3}},
    {{TRAINER_WAYFARER_COAST_SWIMMER_MALE_MATTHEW, TRAINER_WAYFARER_COAST_SWIMMER_MALE_MATTHEW_2, TRAINER_NONE}},
    {{TRAINER_WAYFARER_COAST_SWIMMER_MALE_TONY, TRAINER_WAYFARER_COAST_SWIMMER_MALE_TONY_2, TRAINER_NONE}},
    {{TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_MELISSA, TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_MELISSA_2, TRAINER_NONE}},
};

static bool8 CoastFlagGet(u16 slot)
{
    return (gSaveBlock3Ptr->wayfarerCoast.flags[slot / 8] >> (slot & 7)) & 1;
}

static void CoastFlagSet(u16 slot, bool8 value)
{
    if (value)
        gSaveBlock3Ptr->wayfarerCoast.flags[slot / 8] |= 1 << (slot & 7);
    else
        gSaveBlock3Ptr->wayfarerCoast.flags[slot / 8] &= ~(1 << (slot & 7));
}

bool32 WayfarerCoastTrainerDefeatGet(u16 slot)
{
    if (slot >= WAYFARER_COAST_TRAINER_DEFEAT_COUNT)
        return FALSE;
    return CoastFlagGet(WAYFARER_COAST_TRAINER_DEFEAT_SLOT_FIRST + slot);
}

void WayfarerCoastTrainerDefeatSet(u16 slot)
{
    if (slot < WAYFARER_COAST_TRAINER_DEFEAT_COUNT)
        CoastFlagSet(WAYFARER_COAST_TRAINER_DEFEAT_SLOT_FIRST + slot, TRUE);
}

void WayfarerCoastTrainerDefeatClear(u16 slot)
{
    if (slot < WAYFARER_COAST_TRAINER_DEFEAT_COUNT)
        CoastFlagSet(WAYFARER_COAST_TRAINER_DEFEAT_SLOT_FIRST + slot, FALSE);
}

s32 WayfarerCoastRematchFamilyIndex(u16 trainerId)
{
    s32 family;
    s32 stage;

    if (trainerId == TRAINER_NONE)
        return -1;
    for (family = 0; family < WAYFARER_COAST_REMATCH_FAMILY_COUNT; family++)
        for (stage = 0; stage < WAYFARER_COAST_REMATCH_STAGES; stage++)
            if (sWayfarerCoastRematchFamilies[family].trainerIds[stage] == trainerId)
                return family;
    return -1;
}

static u8 GetStage(s32 family)
{
    u16 packed = family < 8
        ? gSaveBlock3Ptr->wayfarerCoast.vars[WAYFARER_COAST_REMATCH_STAGE_VAR_LOW]
        : gSaveBlock3Ptr->wayfarerCoast.vars[WAYFARER_COAST_REMATCH_STAGE_VAR_HIGH];
    return (packed >> ((family & 7) * 2)) & 3;
}

static void SetStage(s32 family, u8 stage)
{
    u16 *packed = family < 8
        ? &gSaveBlock3Ptr->wayfarerCoast.vars[WAYFARER_COAST_REMATCH_STAGE_VAR_LOW]
        : &gSaveBlock3Ptr->wayfarerCoast.vars[WAYFARER_COAST_REMATCH_STAGE_VAR_HIGH];
    u8 shift = (family & 7) * 2;
    *packed = (*packed & ~(3 << shift)) | ((stage & 3) << shift);
}

bool8 WayfarerCoastRematchHasFamily(u16 trainerId)
{
    return WayfarerCoastRematchFamilyIndex(trainerId) != -1;
}

bool8 WayfarerCoastRematchIsReady(u16 trainerId)
{
    s32 family = WayfarerCoastRematchFamilyIndex(trainerId);
    return family != -1 && CoastFlagGet(WAYFARER_COAST_REMATCH_PENDING_SLOT_FIRST + family);
}

void WayfarerCoastRematchSetReady(u16 trainerId)
{
    s32 family = WayfarerCoastRematchFamilyIndex(trainerId);
    if (family != -1)
        CoastFlagSet(WAYFARER_COAST_REMATCH_PENDING_SLOT_FIRST + family, TRUE);
}

void WayfarerCoastRematchClearReady(u16 trainerId)
{
    s32 family = WayfarerCoastRematchFamilyIndex(trainerId);
    if (family != -1)
        CoastFlagSet(WAYFARER_COAST_REMATCH_PENDING_SLOT_FIRST + family, FALSE);
}

void WayfarerCoastRematchClearAllReady(void)
{
    s32 family;
    for (family = 0; family < WAYFARER_COAST_REMATCH_FAMILY_COUNT; family++)
        CoastFlagSet(WAYFARER_COAST_REMATCH_PENDING_SLOT_FIRST + family, FALSE);
}

bool8 WayfarerCoastRematchCompleteIfReady(u16 trainerId)
{
    if (!WayfarerCoastRematchIsReady(trainerId))
        return FALSE;
    WayfarerCoastRematchClearReady(trainerId);
    WayfarerCoastRematchAdvance(trainerId);
    return TRUE;
}

u16 WayfarerCoastRematchGetOpponent(u16 trainerId)
{
    s32 family = WayfarerCoastRematchFamilyIndex(trainerId);
    u8 stage;

    if (family == -1)
        return trainerId;
    stage = GetStage(family);
    if (stage >= WAYFARER_COAST_REMATCH_STAGES)
        stage = 0;
    if (stage >= WAYFARER_COAST_REMATCH_STAGES - 1
     || sWayfarerCoastRematchFamilies[family].trainerIds[stage + 1] == TRAINER_NONE)
        return sWayfarerCoastRematchFamilies[family].trainerIds[stage];
    return sWayfarerCoastRematchFamilies[family].trainerIds[stage + 1];
}

void WayfarerCoastRematchAdvance(u16 trainerId)
{
    s32 family = WayfarerCoastRematchFamilyIndex(trainerId);
    u8 stage;

    if (family == -1)
        return;
    stage = GetStage(family);
    if (stage + 2 < WAYFARER_COAST_REMATCH_STAGES
     && sWayfarerCoastRematchFamilies[family].trainerIds[stage + 2] != TRAINER_NONE)
        SetStage(family, stage + 1);
}

#endif  // IS_WAYFARER

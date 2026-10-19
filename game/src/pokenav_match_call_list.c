#include "global.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "gym_leader_rematch.h"
#include "international_string_util.h"
#include "main.h"
#include "match_call.h"
#include "overworld.h"
#include "pokemon.h"
#include "region_map.h"
#include "pokenav.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "wayfarer_daily_slots.h"
#include "wayfarer_phone.h"
#include "constants/region_map_sections.h"
#include "constants/songs.h"

#if IS_WAYFARER
#define MATCH_CALL_LAST_CONTACT DAILY_CONTACT_LAST
STATIC_ASSERT(MATCH_CALL_ENTRIES_MAX >= MC_HEADER_COUNT + DAILY_CONTACT_LAST + 1, MatchCallListFitsEveryContact);
#else
#define MATCH_CALL_LAST_CONTACT (REMATCH_TABLE_ENTRIES - 1)
#endif

struct Pokenav_MatchCallMenu
{
    u16 optionCursorPos;
    u16 maxOptionId;
    const u8 *matchCallOptions;
    u16 headerId;
    u16 numRegistered;
    u16 numSpecialTrainers;
    bool32 initFinished;
    u32 loopedTaskId;
    u32 (*callback)(struct Pokenav_MatchCallMenu *);
    struct PokenavMatchCallEntry matchCallEntries[MATCH_CALL_ENTRIES_MAX];
};

static u32 CB2_HandleMatchCallInput(struct Pokenav_MatchCallMenu *);
static u32 GetExitMatchCallMenuId(struct Pokenav_MatchCallMenu *);
static u32 CB2_HandleMatchCallOptionsInput(struct Pokenav_MatchCallMenu *);
static u32 CB2_HandleCheckPageInput(struct Pokenav_MatchCallMenu *);
static u32 CB2_HandleCallExitInput(struct Pokenav_MatchCallMenu *);
static u32 LoopedTask_BuildMatchCallList(s32);
static bool32 ShouldDoNearbyMessage(void);

static const u8 gText_CallCantBeMadeHere[] = _("A call can't be made from here.");

#include "data/text/match_call_messages.h"

static const u8 sMatchCallOptionsNoCheckPage[] =
{
    MATCH_CALL_OPTION_CALL,
    MATCH_CALL_OPTION_CANCEL
};

static const u8 sMatchCallOptionsHasCheckPage[] =
{
    MATCH_CALL_OPTION_CALL,
    MATCH_CALL_OPTION_CHECK,
    MATCH_CALL_OPTION_CANCEL
};

bool32 PokenavCallback_Init_MatchCall(void)
{
    struct Pokenav_MatchCallMenu *state = AllocSubstruct(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN, sizeof(struct Pokenav_MatchCallMenu));
    if (!state)
        return FALSE;

    state->callback = CB2_HandleMatchCallInput;
    state->headerId = 0;
    state->initFinished = FALSE;
    state->loopedTaskId = CreateLoopedTask(LoopedTask_BuildMatchCallList, 1);
    return TRUE;
}

u32 GetMatchCallCallback(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->callback(state);
}

void FreeMatchCallSubstruct1(void)
{
    FreePokenavSubstruct(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
}

static u32 CB2_HandleMatchCallInput(struct Pokenav_MatchCallMenu *state)
{
    int selection;

    if (JOY_REPEAT(DPAD_UP))
        return POKENAV_MC_FUNC_UP;
    if (JOY_REPEAT(DPAD_DOWN))
        return POKENAV_MC_FUNC_DOWN;
    if (JOY_REPEAT(DPAD_LEFT))
        return POKENAV_MC_FUNC_PG_UP;
    if (JOY_REPEAT(DPAD_RIGHT))
        return POKENAV_MC_FUNC_PG_DOWN;

    if (JOY_NEW(A_BUTTON))
    {
        state->callback = CB2_HandleMatchCallOptionsInput;
        state->optionCursorPos = 0;
        selection = PokenavList_GetSelectedIndex();

        if (MatchCall_HasCheckPage(state->matchCallEntries[selection].headerId))
        {
            state->matchCallOptions = sMatchCallOptionsHasCheckPage;
            state->maxOptionId = ARRAY_COUNT(sMatchCallOptionsHasCheckPage) - 1;
        }
        else
        {
            state->matchCallOptions = sMatchCallOptionsNoCheckPage;
            state->maxOptionId = ARRAY_COUNT(sMatchCallOptionsNoCheckPage) - 1;
        }

        return POKENAV_MC_FUNC_SELECT;
    }

    if (JOY_NEW(B_BUTTON))
    {
        if (GetPokenavMode() != POKENAV_MODE_FORCE_CALL_READY)
        {
            state->callback = GetExitMatchCallMenuId;
            return POKENAV_MC_FUNC_EXIT;
        }
        else
        {
            // Cant exit Match Call menu before calling Mr Stone during tutorial
            PlaySE(SE_FAILURE);
        }
    }

    return POKENAV_MC_FUNC_NONE;
}

static u32 GetExitMatchCallMenuId(struct Pokenav_MatchCallMenu *state)
{
    return POKENAV_MAIN_MENU_CURSOR_ON_MATCH_CALL;
}

static u32 CB2_HandleMatchCallOptionsInput(struct Pokenav_MatchCallMenu *state)
{
    if (JOY_NEW(DPAD_UP) && state->optionCursorPos)
    {
        state->optionCursorPos--;
        return POKENAV_MC_FUNC_MOVE_OPTIONS_CURSOR;
    }

    if (JOY_NEW(DPAD_DOWN) && state->optionCursorPos < state->maxOptionId)
    {
        state->optionCursorPos++;
        return POKENAV_MC_FUNC_MOVE_OPTIONS_CURSOR;
    }

    if (JOY_NEW(A_BUTTON))
    {
        switch (state->matchCallOptions[state->optionCursorPos])
        {
        case MATCH_CALL_OPTION_CANCEL:
            state->callback = CB2_HandleMatchCallInput;
            return POKENAV_MC_FUNC_CANCEL;
        case MATCH_CALL_OPTION_CALL:
            if (GetPokenavMode() == POKENAV_MODE_FORCE_CALL_READY)
                SetPokenavMode(POKENAV_MODE_FORCE_CALL_EXIT);

            state->callback = CB2_HandleCallExitInput;
            if (ShouldDoNearbyMessage())
                return POKENAV_MC_FUNC_NEARBY_MSG;

            return POKENAV_MC_FUNC_CALL_MSG;
        case MATCH_CALL_OPTION_CHECK:
            state->callback = CB2_HandleCheckPageInput;
            return POKENAV_MC_FUNC_SHOW_CHECK_PAGE;
        }
    }

    if (JOY_NEW(B_BUTTON))
    {
        state->callback = CB2_HandleMatchCallInput;
        return POKENAV_MC_FUNC_CANCEL;
    }

    return POKENAV_MC_FUNC_NONE;
}

static u32 CB2_HandleCheckPageInput(struct Pokenav_MatchCallMenu *state)
{
    if (JOY_REPEAT(DPAD_UP))
        return POKENAV_MC_FUNC_CHECK_PAGE_UP;
    if (JOY_REPEAT(DPAD_DOWN))
        return POKENAV_MC_FUNC_CHECK_PAGE_DOWN;

    if (JOY_NEW(B_BUTTON))
    {
        state->callback = CB2_HandleMatchCallInput;
        return POKENAV_MC_FUNC_EXIT_CHECK_PAGE;
    }

    return POKENAV_MC_FUNC_NONE;
}

static u32 CB2_HandleCallExitInput(struct Pokenav_MatchCallMenu *state)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        state->callback = CB2_HandleMatchCallInput;
        return POKENAV_MC_FUNC_EXIT_CALL;
    }

    return POKENAV_MC_FUNC_NONE;
}

static u32 LoopedTask_BuildMatchCallList(s32 taskState)
{
    int i, j;
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    switch (taskState)
    {
    case 0:
        state->headerId = 0;
        state->numRegistered = 0;
        return LT_INC_AND_CONTINUE;
    case 1:
        // Load special trainers (e.g. Rival, gym leaders)
        for (i = 0, j = state->headerId; i < 30; i++, j++)
        {
            if (MatchCall_GetEnabled(j) && state->numRegistered < MATCH_CALL_ENTRIES_MAX)
            {
                state->matchCallEntries[state->numRegistered].headerId = j;
                state->matchCallEntries[state->numRegistered].isSpecialTrainer = TRUE;
                state->matchCallEntries[state->numRegistered].mapSec = MatchCall_GetMapSec(j);
                state->numRegistered++;
            }

            if (++state->headerId >= MC_HEADER_COUNT)
            {
                state->numSpecialTrainers = state->headerId;
                state->headerId = 0;
                return LT_INC_AND_CONTINUE;
            }
        }

        return LT_CONTINUE;
    case 2:
        // Load normal trainers
        for (i = 0, j = state->headerId; i < 30; i++, j++)
        {
            if (!MatchCall_HasRematchId(state->headerId) && IsRematchEntryRegistered(state->headerId)
             && state->numRegistered < MATCH_CALL_ENTRIES_MAX)
            {
                state->matchCallEntries[state->numRegistered].headerId = state->headerId;
                state->matchCallEntries[state->numRegistered].isSpecialTrainer = FALSE;
                state->matchCallEntries[state->numRegistered].mapSec = GetMatchTableMapSectionId(j);
                state->numRegistered++;
            }

            if (++state->headerId > MATCH_CALL_LAST_CONTACT)
                return LT_INC_AND_CONTINUE;
        }

        return LT_CONTINUE;
    case 3:
        state->initFinished = TRUE;
        break;
    }

    return LT_FINISH;
}

bool32 IsRematchEntryRegistered(int rematchIndex)
{
#if IS_WAYFARER
    return rematchIndex >= 0 && rematchIndex <= MATCH_CALL_LAST_CONTACT && PhoneContact_IsRegistered(rematchIndex);
#else
    if (rematchIndex < REMATCH_TABLE_ENTRIES)
        return FlagGet(PhoneContact_RegisteredFlag(rematchIndex));

    return FALSE;
#endif
}

// Whether a list entry's trainer is ready for a rematch: a rematch table row, or a Sevii or coast family.
static bool32 IsRematchReady(int index)
{
#if IS_WAYFARER
    if (index >= REMATCH_TABLE_ENTRIES)
        return PhoneContact_IsReady(index);
#endif
    return gSaveBlock1Ptr->trainerRematches[index] != 0;
}

int IsMatchCallListInitFinished(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->initFinished;
}

int GetNumberRegistered(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->numRegistered;
}

static int UNUSED GetNumSpecialTrainers(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->numSpecialTrainers;
}

static int UNUSED GetNumNormalTrainers(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->numRegistered - state->numSpecialTrainers;
}

static int UNUSED GetNormalTrainerHeaderId(int index)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    index += state->numSpecialTrainers;
    if (index >= state->numRegistered)
        return REMATCH_TABLE_ENTRIES;

    return state->matchCallEntries[index].headerId;
}

struct PokenavMatchCallEntry *GetMatchCallList(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->matchCallEntries;
}

#if IS_WAYFARER
// The place a contact calls home, named from its home map: section ids are only meaningful within one region.
bool32 GetMatchCallContactPlaceName(int index, u8 *dest)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    u8 mapGroup, mapNum;

    if (state->matchCallEntries[index].isSpecialTrainer
     || !DailyContacts_GetHome(state->matchCallEntries[index].headerId, &mapGroup, &mapNum))
        return FALSE;
    WayfarerGetPlaceNameOfMap(dest, mapGroup, mapNum);
    return TRUE;
}
#endif

mapsec_u16_t GetMatchCallMapSec(int index)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->matchCallEntries[index].mapSec;
}

bool32 ShouldDrawRematchPokeballIcon(int index)
{
#if FREE_MATCH_CALL == FALSE
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    if (!state->matchCallEntries[index].isSpecialTrainer)
    {
        // A contact number: for the Sevii and coast families it reaches past REMATCH_TABLE_ENTRIES.
        index = state->matchCallEntries[index].headerId;
    }
    else
    {
        index = MatchCall_GetRematchTableIdx(state->matchCallEntries[index].headerId);
        if (index == REMATCH_TABLE_ENTRIES)
            return FALSE;
    }

    return IsRematchReady(index);
#else
    return FALSE;
#endif //FREE_MATCH_CALL
}

enum TrainerPicID GetMatchCallTrainerPic(int index)
{
    int headerId;
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    if (!state->matchCallEntries[index].isSpecialTrainer)
    {
        index = GetTrainerIdxByRematchIdx(state->matchCallEntries[index].headerId);
        return GetTrainerPicFromId(index);
    }

    headerId = state->matchCallEntries[index].headerId;
    index = MatchCall_GetRematchTableIdx(headerId);
    if (index != REMATCH_TABLE_ENTRIES)
    {
        index = GetTrainerIdxByRematchIdx(index);
        return GetTrainerPicFromId(index);
    }

    index = MatchCall_GetOverrideFacilityClass(headerId);
    return gFacilityClassToPicIndex[index];
}

const u8 *GetMatchCallMessageText(int index, bool8 *newRematchRequest)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    *newRematchRequest = FALSE;
    if (!Overworld_MapTypeAllowsTeleportAndFly(gMapHeader.mapType))
        return gText_CallCantBeMadeHere;

    if (!state->matchCallEntries[index].isSpecialTrainer)
        *newRematchRequest = SelectMatchCallMessage(GetTrainerIdxByRematchIdx(state->matchCallEntries[index].headerId), gStringVar4);
    else
        MatchCall_GetMessage(state->matchCallEntries[index].headerId, gStringVar4);

    return gStringVar4;
}

const u8 *GetMatchCallFlavorText(int index, int checkPageEntry)
{
    int rematchId;
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    if (state->matchCallEntries[index].isSpecialTrainer)
    {
        rematchId = MatchCall_GetRematchTableIdx(state->matchCallEntries[index].headerId);
        if (rematchId == REMATCH_TABLE_ENTRIES)
            return MatchCall_GetOverrideFlavorText(state->matchCallEntries[index].headerId, checkPageEntry);
    }
    else
    {
        rematchId = state->matchCallEntries[index].headerId;
    }

    return gMatchCallFlavorTexts[rematchId][checkPageEntry];
}

u16 GetMatchCallOptionCursorPos(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->optionCursorPos;
}

u16 GetMatchCallOptionId(int optionId)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    if (state->maxOptionId < optionId)
        return MATCH_CALL_OPTION_COUNT;

    return state->matchCallOptions[optionId];
}

void BufferMatchCallNameAndDesc(struct PokenavMatchCallEntry *matchCallEntry, u8 *str)
{
    const u8 *trainerName;
    const u8 *className;
    if (!matchCallEntry->isSpecialTrainer)
    {
        int index = GetTrainerIdxByRematchIdx(matchCallEntry->headerId);
        const struct Trainer *trainer = GetTrainerStructFromId(index);
        enum TrainerClassID class = trainer->trainerClass;
        className = gTrainerClasses[class].name;
        trainerName = trainer->trainerName;
    }
    else
    {
        MatchCall_GetNameAndDesc(matchCallEntry->headerId, &className, &trainerName);
    }

    if (className && trainerName)
    {
        u8 *str2 = GetStringClearToWidth(str, FONT_NARROW, className, 69);
        GetStringClearToWidth(str2, FONT_NARROW, trainerName, 51);
    }
    else
    {
        GetStringClearToWidth(str, FONT_NARROW, NULL, 120);
    }
}

mapsec_u8_t GetMatchTableMapSectionId(int rematchIndex)
{
#if IS_WAYFARER
    if (rematchIndex >= REMATCH_TABLE_ENTRIES)
    {
        u8 homeGroup, homeNum;

        // A Sevii or coast family has no rematch table row: its home is where its first trainer lives.
        if (!DailyContacts_GetHome(rematchIndex, &homeGroup, &homeNum))
            return MAPSEC_NONE;
        return Overworld_GetMapHeaderByGroupAndId(homeGroup, homeNum)->regionMapSectionId;
    }
#endif
    int mapGroup = gRematchTable[rematchIndex].mapGroup;
    int mapNum = gRematchTable[rematchIndex].mapNum;
    return Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum)->regionMapSectionId;
}

int GetIndexDeltaOfNextCheckPageDown(int index)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    int count = 1;
    while (++index < state->numRegistered)
    {
        if (MatchCall_HasCheckPage(state->matchCallEntries[index].headerId))
            return count;

        count++;
    }

    return 0;
}

int GetIndexDeltaOfNextCheckPageUp(int index)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    int count = -1;
    while (--index >= 0)
    {
        if (MatchCall_HasCheckPage(state->matchCallEntries[index].headerId))
            return count;

        count--;
    }

    return 0;
}

static bool32 UNUSED HasRematchEntry(void)
{
#if FREE_MATCH_CALL == FALSE
    int i;

    for (i = 0; i < REMATCH_TABLE_ENTRIES; i++)
    {
        if (IsRematchEntryRegistered(i) && gSaveBlock1Ptr->trainerRematches[i])
            return TRUE;
    }

    for (i = 0; i < MC_HEADER_COUNT; i++)
    {
        if (MatchCall_GetEnabled(i))
        {
            int index = MatchCall_GetRematchTableIdx(i);
            if (gSaveBlock1Ptr->trainerRematches[index])
                return TRUE;
        }
    }
#endif //FREE_MATCH_CALL

    return FALSE;
}

static bool32 ShouldDoNearbyMessage(void)
{
#if FREE_MATCH_CALL == FALSE
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    int selection = PokenavList_GetSelectedIndex();
    if (!state->matchCallEntries[selection].isSpecialTrainer)
    {
#if IS_WAYFARER
        if (PhoneContact_IsInHomeSection(state->matchCallEntries[selection].headerId))
#else
        if (GetMatchCallMapSec(selection) == gMapHeader.regionMapSectionId)
#endif
        {
            if (!IsRematchReady(state->matchCallEntries[selection].headerId))
                return TRUE;
        }
    }
    else
    {
        if (state->matchCallEntries[selection].headerId == MC_HEADER_WATTSON)
        {
            if (GetMatchCallMapSec(selection) == gMapHeader.regionMapSectionId
             && FlagGet(FLAG_BADGE05_GET) == TRUE)
            {
                if (!FlagGet(FLAG_WATTSON_REMATCH_AVAILABLE))
                    return TRUE;
            }
        }
    }
#endif //FREE_MATCH_CALL
    return FALSE;
}

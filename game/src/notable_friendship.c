#include "global.h"
#include "notable_friendship.h"

#if IS_WAYFARER
#include "data.h"
#include "event_data.h"
#include "league_selection.h"
#include "notable_trainers.h"
#include "script.h"
#include "string_util.h"
#include "text.h"
#include "constants/notable_trainers.h"

extern const u8 NotableFriendship_EventScript_NumberRegistered[];

static bool8 IsCharacterId(u8 characterId)
{
    return characterId >= 1 && characterId <= NOTABLE_FRIENDSHIP_COUNT;
}

static bool8 GetBit(const u8 *bits, u8 characterId)
{
    return (bits[(characterId - 1) / 8] >> ((characterId - 1) % 8)) & 1;
}

static void SetBit(u8 *bits, u8 characterId)
{
    bits[(characterId - 1) / 8] |= 1 << ((characterId - 1) % 8);
}

void NotableFriendship_Reset(void)
{
    memset(&gSaveBlock3Ptr->notableFriendship, 0, sizeof(gSaveBlock3Ptr->notableFriendship));
}

static bool8 GivesNumber(u8 characterId)
{
    return characterId != NOTABLE_TRAINER_TATE_LIZA;
}

bool8 NotableFriendship_IsValid(void)
{
    const struct NotableFriendshipState *state = &gSaveBlock3Ptr->notableFriendship;
    u8 characterId;

    for (characterId = 1; characterId <= NOTABLE_FRIENDSHIP_COUNT; characterId++)
    {
        if (GetBit(state->firstWin, characterId) && state->score[characterId - 1] < NOTABLE_FRIENDSHIP_MET)
            return FALSE;
        // The handover's line is shown only to a trainer who gave their number, so at Friend or above.
        if (GetBit(state->numberShown, characterId)
         && (!GivesNumber(characterId) || state->score[characterId - 1] < NOTABLE_FRIENDSHIP_FRIEND))
            return FALSE;
    }
    // The bits past the last character are padding and stay clear.
    return (state->firstWin[sizeof(state->firstWin) - 1] >> (NOTABLE_FRIENDSHIP_COUNT % 8)) == 0
        && (state->numberShown[sizeof(state->numberShown) - 1] >> (NOTABLE_FRIENDSHIP_COUNT % 8)) == 0;
}

u8 NotableFriendship_GetScore(u8 characterId)
{
    return IsCharacterId(characterId) ? gSaveBlock3Ptr->notableFriendship.score[characterId - 1] : 0;
}

enum NotableFriendshipStage NotableFriendship_GetStage(u8 characterId)
{
    u8 score = NotableFriendship_GetScore(characterId);

    if (score >= NOTABLE_FRIENDSHIP_CLOSE)
        return NOTABLE_STAGE_CLOSE;
    if (score >= NOTABLE_FRIENDSHIP_FRIEND)
        return NOTABLE_STAGE_FRIEND;
    if (score >= NOTABLE_FRIENDSHIP_MET)
        return NOTABLE_STAGE_MET;
    return NOTABLE_STAGE_STRANGER;
}

static void AddPoints(u8 characterId, u8 points)
{
    u8 *score;
    u8 before;

    if (!IsCharacterId(characterId))
        return;
    score = &gSaveBlock3Ptr->notableFriendship.score[characterId - 1];
    before = *score;
    // Crossing Friend makes the trainer a contact (NotablePhone_IsContact reads the stage); the field then owes
    // the handover's line until it is shown, which the saved number-shown bit records.
    *score = points > 255 - before ? 255 : before + points;
}

void NotableFriendship_AddQuestPoints(u8 characterId)
{
    AddPoints(characterId, NOTABLE_FRIENDSHIP_QUEST_POINTS);
}

bool8 NotableFriendship_CountFirstWin(u8 characterId)
{
    u8 *bits = gSaveBlock3Ptr->notableFriendship.firstWin;

    if (!IsCharacterId(characterId) || GetBit(bits, characterId))
        return FALSE;
    SetBit(bits, characterId);
    AddPoints(characterId, NOTABLE_FRIENDSHIP_FIRST_WIN_POINTS);
    return TRUE;
}

void NotableFriendship_CountTrainerWin(u16 trainerId)
{
    const struct NotableTrainer *trainer = GetNotableTrainerForEncounter(trainerId);

    if (trainer != NULL)
        NotableFriendship_CountFirstWin(trainer->characterId);
}

// The first character whose number is owed: at Friend or above, giving a number, and the line not shown yet.
// The field polls this every frame it has control; it reads 38 saved bytes at most, so it needs no RAM cache,
// and a save anywhere between the crossing and the line keeps the line owed.
static u8 NextOwedNumber(void)
{
    const struct NotableFriendshipState *state = &gSaveBlock3Ptr->notableFriendship;
    u8 characterId;

    for (characterId = 1; characterId <= NOTABLE_FRIENDSHIP_COUNT; characterId++)
    {
        if (state->score[characterId - 1] >= NOTABLE_FRIENDSHIP_FRIEND && GivesNumber(characterId)
         && !GetBit(state->numberShown, characterId))
            return characterId;
    }
    return NOTABLE_TRAINER_NONE;
}

bool8 NotableFriendship_HasPendingNumber(void)
{
    return NextOwedNumber() != NOTABLE_TRAINER_NONE;
}

bool8 NotableFriendship_TryStartPendingNumberScript(void)
{
    u8 characterId = NextOwedNumber();

    if (characterId == NOTABLE_TRAINER_NONE)
        return FALSE;
    gSpecialVar_0x8004 = characterId;
    ScriptContext_SetupScript(NotableFriendship_EventScript_NumberRegistered);
    return TRUE;
}

// Special: the handover's line has played for the character in VAR_0x8004.
void NotableFriendship_MarkNumberShown(void)
{
    if (IsCharacterId(gSpecialVar_0x8004) && GivesNumber(gSpecialVar_0x8004)
     && NotableFriendship_GetScore(gSpecialVar_0x8004) >= NOTABLE_FRIENDSHIP_FRIEND)
        SetBit(gSaveBlock3Ptr->notableFriendship.numberShown, gSpecialVar_0x8004);
}

void NotableFriendship_BufferTrainerName(void)
{
    const struct LeagueTrainer *trainer = GetLeagueTrainer(gSpecialVar_0x8004);

    // The league registry names each character's own encounter, whatever alias fought the player.
    if (trainer == NULL)
        gStringVar1[0] = EOS;
    else
        StringCopy(gStringVar1, GetTrainerNameFromId(trainer->sourceTrainerId));
}

#endif // IS_WAYFARER

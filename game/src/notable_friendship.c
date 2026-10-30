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

// Handovers waiting for the field, one bit per character. RAM only: the score is the saved fact, and a
// crossing is shown at the next frame the field has control. A script that saves before handing control
// back (the Hall of Fame after a champion battle) loses the line, never the number.
static EWRAM_DATA u8 sPendingNumber[(NOTABLE_FRIENDSHIP_COUNT + 7) / 8] = {0};
// True while any bit above is set: the field polls every frame, so the common case must not scan the bits.
static EWRAM_DATA bool8 sAnyPendingNumber = FALSE;

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
    memset(sPendingNumber, 0, sizeof(sPendingNumber));
    sAnyPendingNumber = FALSE;
}

bool8 NotableFriendship_IsValid(void)
{
    const struct NotableFriendshipState *state = &gSaveBlock3Ptr->notableFriendship;
    u8 characterId;

    for (characterId = 1; characterId <= NOTABLE_FRIENDSHIP_COUNT; characterId++)
    {
        if (GetBit(state->firstWin, characterId) && state->score[characterId - 1] < NOTABLE_FRIENDSHIP_MET)
            return FALSE;
    }
    // The bits past the last character are padding and stay clear.
    return (state->firstWin[sizeof(state->firstWin) - 1] >> (NOTABLE_FRIENDSHIP_COUNT % 8)) == 0;
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
    *score = points > 255 - before ? 255 : before + points;
    // The number is handed over once, when the score first reaches Friend; Tate & Liza give none.
    if (before < NOTABLE_FRIENDSHIP_FRIEND && *score >= NOTABLE_FRIENDSHIP_FRIEND
     && characterId != NOTABLE_TRAINER_TATE_LIZA)
    {
        SetBit(sPendingNumber, characterId);
        sAnyPendingNumber = TRUE;
    }
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

static u8 NextPendingNumber(void)
{
    u8 characterId;

    for (characterId = 1; characterId <= NOTABLE_FRIENDSHIP_COUNT; characterId++)
    {
        if (GetBit(sPendingNumber, characterId))
            return characterId;
    }
    return NOTABLE_TRAINER_NONE;
}

bool8 NotableFriendship_HasPendingNumber(void)
{
    return NextPendingNumber() != NOTABLE_TRAINER_NONE;
}

bool8 NotableFriendship_TryStartPendingNumberScript(void)
{
    u8 characterId;

    if (!sAnyPendingNumber)
        return FALSE;
    characterId = NextPendingNumber();
    if (characterId == NOTABLE_TRAINER_NONE)
    {
        sAnyPendingNumber = FALSE;
        return FALSE;
    }
    sPendingNumber[(characterId - 1) / 8] &= ~(1 << ((characterId - 1) % 8));
    gSpecialVar_0x8004 = characterId;
    ScriptContext_SetupScript(NotableFriendship_EventScript_NumberRegistered);
    return TRUE;
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

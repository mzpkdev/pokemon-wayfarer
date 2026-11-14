#ifndef GUARD_NOTABLE_FRIENDSHIP_H
#define GUARD_NOTABLE_FRIENDSHIP_H

// Notable trainer friendship and the talk flow built on it.
// Specs: .product/specs/notable-trainers.md#friendship (score, stages, events),
// notable-haunts.md#talk-flow (greeting choice), notable-spots.md#talk-flow (activity lines),
// notable-world-simulation.md#interactions (talking to a walker).

#include "global.h"
#include "constants/notable_story_beats.h"

#if IS_WAYFARER

enum NotableFriendshipStage
{
    NOTABLE_STAGE_STRANGER,
    NOTABLE_STAGE_MET,
    NOTABLE_STAGE_FRIEND,
    NOTABLE_STAGE_CLOSE,
};

// Placeholder thresholds and weights (the spec's tables).
#define NOTABLE_FRIENDSHIP_MET      10
#define NOTABLE_FRIENDSHIP_FRIEND   20
#define NOTABLE_FRIENDSHIP_CLOSE    60
#define NOTABLE_FRIENDSHIP_FIRST_WIN_POINTS 10
#define NOTABLE_FRIENDSHIP_QUEST_POINTS     10
#define NOTABLE_FRIENDSHIP_STORY_BEAT_POINTS 10

// New Game: every score 0, every first-win bit clear, no handover waiting.
void NotableFriendship_Reset(void);
// A first-win bit needs a score of at least Met, and no bit past the last character may be set.
bool8 NotableFriendship_IsValid(void);

u8 NotableFriendship_GetScore(u8 characterId);
enum NotableFriendshipStage NotableFriendship_GetStage(u8 characterId);

// A completed haunt quest (+10, capped at 255). Haunts call it once per completed quest, for the
// sender and the recipient of a Courier quest alike. Tate & Liza keep a score like the rest.
void NotableFriendship_AddQuestPoints(u8 characterId);
// Acquaintance (notable-story-continuity.md#1-acquaintance): one saved bit per character, never cleared. It means
// the player and the trainer know each other's faces. It adds no points, changes no stage and gives no number.
bool8 NotableFriendship_IsIntroduced(u8 characterId);
void NotableFriendship_MarkIntroduced(u8 characterId);
// Any battle against the trainer, won or lost, introduces them: by battle trainer id (any alias); ids that belong
// to no notable are ignored. A partner beside the player is an acquaintance too.
void NotableFriendship_MarkTrainerEncountered(u16 trainerId);
// A story beat (constants/notable_story_beats.h) pays NOTABLE_FRIENDSHIP_STORY_BEAT_POINTS to its character once
// and introduces them; returns TRUE when it paid. The beat's character is the caller's (src/notable_story.c).
bool8 NotableFriendship_CompleteStoryBeat(u8 characterId, u8 beatId);
bool8 NotableFriendship_IsStoryBeatDone(u8 beatId);
// The first win over a character counts +10 once; later wins add nothing. Returns TRUE when it counted.
bool8 NotableFriendship_CountFirstWin(u8 characterId);
// Credits a won trainer battle's opponent by battle trainer id (any alias of the character); ids that
// belong to no notable are ignored. Tag matches call it once per opponent.
void NotableFriendship_CountTrainerWin(u16 trainerId);

// The phone number is handed over once, when the score first reaches the Friend threshold (Tate & Liza
// give none). The field shows its system line when it has control; until the line has played (the saved
// number-shown bit), the line stays owed, across saves and reloads.
bool8 NotableFriendship_HasPendingNumber(void);
// Starts the script for the next owed number and returns TRUE, or returns FALSE with none owed or before the
// player has the Pokégear's phone (FLAG_HAS_MATCH_CALL).
bool8 NotableFriendship_TryStartPendingNumberScript(void);
// Special: records that the handover's line played for the character in VAR_0x8004.
void NotableFriendship_MarkNumberShown(void);
// Special: buffers the trainer's name for the system line (VAR_0x8004 = character id).
void NotableFriendship_BufferTrainerName(void);

#endif // IS_WAYFARER
#endif // GUARD_NOTABLE_FRIENDSHIP_H

#ifndef GUARD_NOTABLE_TALK_H
#define GUARD_NOTABLE_TALK_H

// The talk flow of a notable trainer outside a haunt: greeting by friendship stage, then a follow-up or the
// activity line, then the farewell (notable-spots.md#talk-flow). Talking adds no friendship.
//
// Scripts (data/scripts/wayfarer_notable_talk.inc), all taking the talked-to character in VAR_0x8004
// (NOTABLE_TRAINER_*) and used with `call`:
//   NotableTalk_EventScript_Greeting   shows the stage greeting (MEET or HEARD, AGAIN, HELLO, CLOSE)
//   NotableTalk_EventScript_Farewell   shows BYE and closes the message
// A Gym Leader's own Gym script, once nothing is owed, does
//   setvar VAR_0x8004, NOTABLE_TRAINER_<LEADER>
//   call NotableTalk_EventScript_Greeting
//   msgbox <the Gym's existing post-battle line>, MSGBOX_DEFAULT
//   call NotableTalk_EventScript_Farewell
// A walker's talk script, NotableTalk_EventScript_WalkerTalk, is attached to walker actors by
// field_control_avatar.c and does the same with the walker's activity line.

#include "global.h"

#if IS_WAYFARER

#include "notable_friendship.h"

enum NotableGreeting
{
    NOTABLE_GREETING_MEET,
    NOTABLE_GREETING_HEARD,
    NOTABLE_GREETING_AGAIN,
    NOTABLE_GREETING_HELLO,
    NOTABLE_GREETING_CLOSE,
};

// One shared line per activity the trainer is doing (notable-spots.md#talk-flow, activity lines).
enum NotableActivityLine
{
    NOTABLE_LINE_CENTER,
    NOTABLE_LINE_STORE,
    NOTABLE_LINE_GAME_CORNER,
    NOTABLE_LINE_GYM,
    NOTABLE_LINE_GRASS,
    NOTABLE_LINE_WATER,
    NOTABLE_LINE_SQUARE,
    NOTABLE_LINE_CHAT,
    NOTABLE_LINE_STUDY,
    NOTABLE_LINE_SIGHTSEE,
    NOTABLE_LINE_LIE_LOW,
    NOTABLE_LINE_HOME,
    NOTABLE_LINE_COUNT,
};

// The player's fame reaches a trainer when the player's TR is at least min(80, trainerTR - 10), or the player
// reigns as champion at any league or is a Master.
bool8 NotableTalk_FameReaches(u32 playerTR, u32 trainerTR, bool8 reignsOrMaster);
// Whether fame reaches this character now (reads TR, the leagues and the circuit clears).
bool8 NotableTalk_CharacterIsFamous(u8 characterId);
enum NotableGreeting NotableTalk_GreetingFor(enum NotableFriendshipStage stage, bool8 famous);
enum NotableGreeting NotableTalk_GetGreeting(u8 characterId);

// The activity line for a record's destination and activity (a travelling walker names where it is heading).
enum NotableActivityLine NotableTalk_ActivityLineFor(u8 destKind, u16 destId, u8 activity);

// Messages with placeholders expanded, in gStringVar4. NULL-safe: an unknown character gives an empty string.
void NotableTalk_BufferGreetingText(u8 characterId);
void NotableTalk_BufferFarewellText(u8 characterId);
void NotableTalk_BufferActivityLineText(enum NotableActivityLine line);

// Specials. All but PrepareWalker read VAR_0x8004; PrepareWalker writes it.
u32 NotableTalk_BufferGreeting(void);
u32 NotableTalk_BufferFarewell(void);
// The selected object is a walker with a world record: VAR_0x8004 = its character. Returns FALSE otherwise.
u32 NotableTalk_PrepareWalker(void);
// Buffers the selected walker's activity line.
void NotableTalk_BufferWalkerActivityLine(void);

// For field_control_avatar.c: the script of a walker actor.
extern const u8 NotableTalk_EventScript_WalkerTalk[];

#endif // IS_WAYFARER
#endif // GUARD_NOTABLE_TALK_H

#include "global.h"
#include "notable_talk.h"

#if IS_WAYFARER
#include "event_data.h"
#include "event_object_movement.h"
#include "league_circuit.h"
#include "league_events.h"
#include "league_selection.h"
#include "notable_moves.h"
#include "notable_trainers.h"
#include "pokemon.h"
#include "string_util.h"
#include "text.h"
#include "trainer_rating.h"
#include "wayfarer_walker_beats.h"
#include "wayfarer_world.h"
#include "wayfarer_world_data.h"
#include "constants/notable_trainers.h"
#include "constants/wayfarer_world.h"

#include "data/notable_trainers/voices.h"

// The activity lines (notable-spots.md#talk-flow). Placeholders for review.
static const u8 sActivityLine_Center[] = _("Just letting my team rest up.");
static const u8 sActivityLine_Store[] = _("Just stocking up on supplies.");
static const u8 sActivityLine_GameCorner[] = _("Trying my luck for a while.");
static const u8 sActivityLine_Gym[] = _("Just dropped by to see the GYM.");
static const u8 sActivityLine_Grass[] = _("Seeing what turns up in this\ngrass.");
static const u8 sActivityLine_Water[] = _("Waiting for a bite.");
static const u8 sActivityLine_Square[] = _("Taking a breather.");
static const u8 sActivityLine_Chat[] = _("Just catching up with someone\nhere.");
static const u8 sActivityLine_Study[] = _("Just reading up on a few things.");
static const u8 sActivityLine_Sightsee[] = _("Just taking in the view.");
static const u8 sActivityLine_LieLow[] = _("Keeping my head down for a while.");
static const u8 sActivityLine_Home[] = _("Just spending some time at home.");

static const u8 *const sActivityLines[NOTABLE_LINE_COUNT] =
{
    [NOTABLE_LINE_CENTER] = sActivityLine_Center,
    [NOTABLE_LINE_STORE] = sActivityLine_Store,
    [NOTABLE_LINE_GAME_CORNER] = sActivityLine_GameCorner,
    [NOTABLE_LINE_GYM] = sActivityLine_Gym,
    [NOTABLE_LINE_GRASS] = sActivityLine_Grass,
    [NOTABLE_LINE_WATER] = sActivityLine_Water,
    [NOTABLE_LINE_SQUARE] = sActivityLine_Square,
    [NOTABLE_LINE_CHAT] = sActivityLine_Chat,
    [NOTABLE_LINE_STUDY] = sActivityLine_Study,
    [NOTABLE_LINE_SIGHTSEE] = sActivityLine_Sightsee,
    [NOTABLE_LINE_LIE_LOW] = sActivityLine_LieLow,
    [NOTABLE_LINE_HOME] = sActivityLine_Home,
};

#define FAME_TR_CEILING 80  // where every league knows the player (leagues.md#qualification)
#define FAME_TR_MARGIN  10

bool8 NotableTalk_FameReaches(u32 playerTR, u32 trainerTR, bool8 reignsOrMaster)
{
    s32 threshold = (s32)trainerTR - FAME_TR_MARGIN;

    if (reignsOrMaster)
        return TRUE;
    if (threshold > FAME_TR_CEILING)
        threshold = FAME_TR_CEILING;
    return (s32)playerTR >= threshold;
}

static bool8 PlayerReignsOrIsMaster(void)
{
    return GetLeagueReigningChampion(LEAGUE_ID_INDIGO) == LEAGUE_EVENT_PLAYER_CHAMPION
        || GetLeagueReigningChampion(LEAGUE_ID_MASTERS) == LEAGUE_EVENT_PLAYER_CHAMPION
        || GetLeagueReigningChampion(LEAGUE_ID_HOENN) == LEAGUE_EVENT_PLAYER_CHAMPION
        || (HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO) && HasClearedCircuitStage(CIRCUIT_STAGE_HOENN));
}

bool8 NotableTalk_CharacterIsFamous(u8 characterId)
{
    const struct NotableTrainer *trainer = GetNotableTrainerById(characterId);
    u32 playerTR = GetTrainerRating();

    if (trainer == NULL)
        return FALSE;
    return NotableTalk_FameReaches(playerTR, GetNotableTrainerRating(trainer, playerTR), PlayerReignsOrIsMaster());
}

enum NotableGreeting NotableTalk_GreetingFor(enum NotableFriendshipStage stage, bool8 famous)
{
    switch (stage)
    {
    case NOTABLE_STAGE_MET:
        return NOTABLE_GREETING_AGAIN;
    case NOTABLE_STAGE_FRIEND:
        return NOTABLE_GREETING_HELLO;
    case NOTABLE_STAGE_CLOSE:
        return NOTABLE_GREETING_CLOSE;
    default:
        return famous ? NOTABLE_GREETING_HEARD : NOTABLE_GREETING_MEET;
    }
}

enum NotableGreeting NotableTalk_GetGreeting(u8 characterId)
{
    enum NotableFriendshipStage stage = NotableFriendship_GetStage(characterId);

    return NotableTalk_GreetingFor(stage, stage == NOTABLE_STAGE_STRANGER && NotableTalk_CharacterIsFamous(characterId));
}

enum NotableActivityLine NotableTalk_ActivityLineFor(u8 destKind, u16 destId, u8 activity)
{
    if (destKind == WORLD_DEST_SPOT && destId < gWayfarerWorldSpotCount)
    {
        switch (gWayfarerWorldSpots[destId].kind)
        {
        case WORLD_SPOT_CENTER_COUNTER:
        case WORLD_SPOT_CENTER_SIDE:
            return NOTABLE_LINE_CENTER;
        case WORLD_SPOT_STORE:
            return NOTABLE_LINE_STORE;
        case WORLD_SPOT_GAME_CORNER:
            return NOTABLE_LINE_GAME_CORNER;
        case WORLD_SPOT_GYM:
            return NOTABLE_LINE_GYM;
        case WORLD_SPOT_TALL_GRASS:
            return NOTABLE_LINE_GRASS;
        case WORLD_SPOT_WATER_EDGE:
            return NOTABLE_LINE_WATER;
        case WORLD_SPOT_SQUARE:
        case WORLD_SPOT_BENCH:
            return NOTABLE_LINE_SQUARE;
        case WORLD_SPOT_NPC_CHAT:
            return NOTABLE_LINE_CHAT;
        default: // a named spot says the line of the activity done there
            break;
        }
    }
    switch (activity)
    {
    case WORLD_ACTIVITY_CARE:
        return NOTABLE_LINE_CENTER;
    case WORLD_ACTIVITY_SHOP:
        return NOTABLE_LINE_STORE;
    case WORLD_ACTIVITY_GAMBLE:
        return NOTABLE_LINE_GAME_CORNER;
    case WORLD_ACTIVITY_TRAIN:
        return NOTABLE_LINE_GRASS;
    case WORLD_ACTIVITY_FISH:
        return NOTABLE_LINE_WATER;
    case WORLD_ACTIVITY_VISIT:
        return NOTABLE_LINE_CHAT;
    case WORLD_ACTIVITY_STUDY:
        return NOTABLE_LINE_STUDY;
    case WORLD_ACTIVITY_SIGHTSEE:
        return NOTABLE_LINE_SIGHTSEE;
    case WORLD_ACTIVITY_LIE_LOW:
        return NOTABLE_LINE_LIE_LOW;
    case WORLD_ACTIVITY_HOME:
        return NOTABLE_LINE_HOME;
    default: // relax, and anything unrecognised
        return NOTABLE_LINE_SQUARE;
    }
}

// {BUDDY}: the buddy slot's species after the downward rule, at the member level for the current world progress.
static void BufferBuddySpecies(const struct NotableTrainer *trainer)
{
    u32 slot, level;
    s32 memberLevel;

    gStringVar1[0] = EOS;
    if (trainer == NULL || trainer->buddySlot < 1 || trainer->buddySlot > PARTY_SIZE)
        return;
    slot = trainer->buddySlot - 1;
    level = GetNotableTrainerTeamLevel(GetNotableTrainerRating(trainer, GetTrainerRating()));
    memberLevel = (s32)level + trainer->levelOffsets[slot];
    if (memberLevel < 1)
        memberLevel = 1;
    if (memberLevel > 100)
        memberLevel = 100;
    StringCopy(gStringVar1, GetSpeciesName(StepDownSpeciesToLevel(trainer->roster[slot].species, memberLevel)));
}

static void BufferVoice(u8 characterId, const u8 *text)
{
    BufferBuddySpecies(GetNotableTrainerById(characterId));
    if (text == NULL)
        gStringVar4[0] = EOS;
    else
        StringExpandPlaceholders(gStringVar4, text);
}

void NotableTalk_BufferGreetingText(u8 characterId)
{
    const struct NotableVoice *voice = GetNotableTrainerById(characterId) != NULL ? &gNotableVoices[characterId - 1] : NULL;
    const u8 *text = NULL;

    if (voice != NULL)
    {
        switch (NotableTalk_GetGreeting(characterId))
        {
        case NOTABLE_GREETING_MEET: text = voice->meet; break;
        case NOTABLE_GREETING_HEARD: text = voice->heard; break;
        case NOTABLE_GREETING_AGAIN: text = voice->again; break;
        case NOTABLE_GREETING_HELLO: text = voice->hello; break;
        case NOTABLE_GREETING_CLOSE: text = voice->close; break;
        }
    }
    BufferVoice(characterId, text);
}

void NotableTalk_BufferFarewellText(u8 characterId)
{
    BufferVoice(characterId, GetNotableTrainerById(characterId) != NULL ? gNotableVoices[characterId - 1].bye : NULL);
}

void NotableTalk_BufferActivityLineText(enum NotableActivityLine line)
{
    StringCopy(gStringVar4, line < NOTABLE_LINE_COUNT ? sActivityLines[line] : sActivityLine_Square);
}

u32 NotableTalk_BufferGreeting(void)
{
    NotableTalk_BufferGreetingText(gSpecialVar_0x8004);
    return gStringVar4[0] != EOS;
}

u32 NotableTalk_BufferFarewell(void)
{
    NotableTalk_BufferFarewellText(gSpecialVar_0x8004);
    return gStringVar4[0] != EOS;
}

static const struct WayfarerWorldRecord *SelectedWalkerRecord(u8 *characterId)
{
    u8 slot;

    if (gSelectedObjectEvent >= OBJECT_EVENTS_COUNT)
        return NULL;
    slot = WayfarerWalkers_ObjectSlot(&gObjectEvents[gSelectedObjectEvent]);
    if (slot >= WORLD_SIM_TRAINER_COUNT)
        return NULL;
    *characterId = gWayfarerWorldTrainers[slot].characterId;
    return &WayfarerWorld_GetState()->records[slot];
}

u32 NotableTalk_PrepareWalker(void)
{
    u8 characterId;

    if (SelectedWalkerRecord(&characterId) == NULL)
        return FALSE;
    gSpecialVar_0x8004 = characterId;
    return TRUE;
}

void NotableTalk_BufferWalkerActivityLine(void)
{
    u8 characterId;
    const struct WayfarerWorldRecord *record = SelectedWalkerRecord(&characterId);

    NotableTalk_BufferActivityLineText(record != NULL
        ? NotableTalk_ActivityLineFor(record->destKind, record->destId, record->activity)
        : NOTABLE_LINE_SQUARE);
}

#endif // IS_WAYFARER

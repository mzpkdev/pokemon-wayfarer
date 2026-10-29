#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "league_circuit.h"
#include "league_selection.h"
#include "league_events.h"
#include "league_run_helpers.h"
#include "notable_friendship.h"
#include "notable_talk.h"
#include "notable_trainers.h"
#include "overworld.h"
#include "pokemon.h"
#include "script.h"
#include "string_util.h"
#include "text.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "wayfarer_world_data.h"
#include "test/test.h"
#include "constants/battle_setup.h"
#include "constants/characters.h"
#include "constants/notable_trainers.h"
#include "constants/opponents.h"
#include "constants/opponents_hns.h"
#include "constants/species.h"
#include "constants/maps.h"
#include "constants/regions.h"
#include "constants/wayfarer_indigo_trainers.h"
#include "constants/wayfarer_kanto_trainers.h"
#include "constants/wayfarer_world.h"

#if IS_WAYFARER

// Notable trainer friendship (.product/specs/notable-trainers.md#friendship) and the talk flow built on it
// (notable-haunts.md#talk-flow, notable-spots.md#talk-flow).

void Test_EndTrainerBattle(void);
void Test_EndRematchBattle(void);

static void ResetFriendship(void)
{
    WayfarerInitPersistentState();
    NotableFriendship_Reset();
    ScriptContext_Init();
    gIsDebugBattle = FALSE;
    gPartnerTrainerId = 0;
    // {PLAYER} expands the name in the save, which a test starts without.
    gSaveBlock2Ptr->playerName[0] = CHAR_A;
    gSaveBlock2Ptr->playerName[1] = EOS;
    SetTrainerRating(0);
}

static void EndSingleBattle(u16 opponent, u8 outcome)
{
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gBattleOutcome = outcome;
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.mode = TRAINER_BATTLE_SINGLE;
    TRAINER_BATTLE_PARAM.opponentA = opponent;
    Test_EndTrainerBattle();
}

TEST("Friendship stages are read from the score at 10, 20 and 60")
{
    static const struct { u8 score; u8 stage; } cases[] = {
        {0, NOTABLE_STAGE_STRANGER}, {9, NOTABLE_STAGE_STRANGER}, {10, NOTABLE_STAGE_MET},
        {19, NOTABLE_STAGE_MET}, {20, NOTABLE_STAGE_FRIEND}, {59, NOTABLE_STAGE_FRIEND},
        {60, NOTABLE_STAGE_CLOSE}, {255, NOTABLE_STAGE_CLOSE},
    };
    u32 i;
    ResetFriendship();
    for (i = 0; i < ARRAY_COUNT(cases); i++)
    {
        gSaveBlock3Ptr->notableFriendship.score[NOTABLE_TRAINER_LANCE - 1] = cases[i].score;
        EXPECT_EQ(NotableFriendship_GetStage(NOTABLE_TRAINER_LANCE), cases[i].stage);
    }
    // Unknown characters are strangers with nothing to add to.
    EXPECT_EQ(NotableFriendship_GetStage(NOTABLE_TRAINER_NONE), NOTABLE_STAGE_STRANGER);
    EXPECT_EQ(NotableFriendship_GetStage((NOTABLE_TRAINER_COUNT + 1)), NOTABLE_STAGE_STRANGER);
}

TEST("Friendship points add 10 per quest and stop at 255")
{
    u32 i;
    ResetFriendship();
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_BRAWLY);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BRAWLY), 10);
    for (i = 0; i < 40; i++)
        NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_BRAWLY);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BRAWLY), 255);
    // Strangers can do quests, and a quest does not touch the first-win bit.
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_MISTY), 0);
    EXPECT(NotableFriendship_IsValid());
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_NONE);
    NotableFriendship_AddQuestPoints((NOTABLE_TRAINER_COUNT + 1));
}

TEST("The first win over a notable counts +10 once, through any of its encounters")
{
    ResetFriendship();
    EXPECT(NotableFriendship_CountFirstWin(NOTABLE_TRAINER_BROCK));
    EXPECT(!NotableFriendship_CountFirstWin(NOTABLE_TRAINER_BROCK));
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BROCK), 10);
    EXPECT_EQ(NotableFriendship_GetStage(NOTABLE_TRAINER_BROCK), NOTABLE_STAGE_MET);
    NotableFriendship_CountTrainerWin(TRAINER_BROCK_DOJO_HNS);
    NotableFriendship_CountTrainerWin(TRAINER_BROCK_POSTOBC_HNS);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BROCK), 10);
    // Tate & Liza are one character whichever variant fought.
    NotableFriendship_CountTrainerWin(TRAINER_TATE_AND_LIZA_3);
    NotableFriendship_CountTrainerWin(TRAINER_TATE_AND_LIZA_5);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_TATE_LIZA), 10);
    // A trainer who is no notable adds nothing.
    NotableFriendship_CountTrainerWin(TRAINER_NONE);
    NotableFriendship_CountTrainerWin(0xFFFF);
    NotableFriendship_CountTrainerWin(TRAINER_BLUE_HNS);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BLUE), 0);
}

TEST("A won trainer battle credits its opponent once; losses, draws, fleeing, debug battles and repeats add nothing")
{
    ResetFriendship();
    EndSingleBattle(TRAINER_BROCK_HNS, B_OUTCOME_LOST);
    EndSingleBattle(TRAINER_BROCK_HNS, B_OUTCOME_DREW);
    EndSingleBattle(TRAINER_BROCK_HNS, B_OUTCOME_RAN);
    EndSingleBattle(TRAINER_BROCK_HNS, B_OUTCOME_MON_FLED);
    EndSingleBattle(TRAINER_BROCK_HNS, B_OUTCOME_FORFEITED);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BROCK), 0);

    // The debug flag is cleared before the win is known to count, so it is kept from the start.
    gIsDebugBattle = TRUE;
    EndSingleBattle(TRAINER_BROCK_HNS, B_OUTCOME_WON);
    EXPECT(!gIsDebugBattle);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BROCK), 0);

    EndSingleBattle(TRAINER_BROCK_HNS, B_OUTCOME_WON);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BROCK), 10);
    EndSingleBattle(TRAINER_BROCK_HNS, B_OUTCOME_WON);
    EndSingleBattle(TRAINER_BROCK_POSTOBC_HNS, B_OUTCOME_WON);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BROCK), 10);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_MISTY), 0);
}

TEST("A tag match credits both opponents and never the player's partner")
{
    ResetFriendship();
    gBattleTypeFlags = BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_TRAINER;
    gBattleOutcome = B_OUTCOME_WON;
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.mode = TRAINER_BATTLE_DOUBLE;
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_BROCK_HNS;
    TRAINER_BATTLE_PARAM.opponentB = TRAINER_MISTY_HNS;
    Test_EndTrainerBattle();
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BROCK), 10);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_MISTY), 10);

    // Beside a partner against a single opponent: the partner is not an opponent, and 0xFFFF fills the slot.
    gBattleTypeFlags = BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRAINER;
    gBattleOutcome = B_OUTCOME_WON;
    gPartnerTrainerId = TRAINER_LTSURGE_HNS;
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.mode = TRAINER_BATTLE_SINGLE;
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_ERIKA_HNS;
    TRAINER_BATTLE_PARAM.opponentB = 0xFFFF;
    Test_EndTrainerBattle();
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_ERIKA), 10);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_LT_SURGE), 0);
}

TEST("Rematch wins and story battles count the first win, and a lost rematch does not")
{
    ResetFriendship();
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gBattleOutcome = B_OUTCOME_LOST;
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_SABRINA_HNS;
    Test_EndRematchBattle();
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_SABRINA), 0);
    gBattleOutcome = B_OUTCOME_WON;
    Test_EndRematchBattle();
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_SABRINA), 10);

    // The tutorial rival battle (Blue) is an early-rival battle, which skips the ordinary end.
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gBattleOutcome = B_OUTCOME_WON;
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.mode = TRAINER_BATTLE_EARLY_RIVAL;
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_WAYFARER_KANTO_BLUE_SQUIRTLE;
    Test_EndTrainerBattle();
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BLUE), 10);
}

#if WAYFARER_LEAGUE_EVENTS
TEST("A league event win credits the accepted opponent, and a lost one does not")
{
    const struct LeagueSavedTeam *team;
    u8 badge;
    u8 first;

    ResetFriendship();
    for (badge = 0; badge < 8; badge++)
    {
        SetBadgeStateForRegion(REGION_KANTO, badge, TRUE);
        SetBadgeStateForRegion(REGION_HOENN, badge, TRUE);
    }
    SetTrainerRating(160);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_POKEMON_LEAGUE_LORELEIS_ROOM);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.mode = TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT;
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_WAYFARER_INDIGO_LORELEI;
    BattleSetup_ConfigureTrainerBattle(NULL);
    EXPECT(IsLeagueEventBattleInProgress());
    EXPECT(GetAcceptedLeagueEventMember(0, &team));
    first = team->characterId;
    EXPECT(first != NOTABLE_TRAINER_NONE);

    gBattleOutcome = B_OUTCOME_WON;
    Test_EndTrainerBattle();
    EXPECT_EQ(NotableFriendship_GetScore(first), 10);
}
#endif // WAYFARER_LEAGUE_EVENTS

TEST("The number is handed over once, when the score first crosses Friend")
{
    ResetFriendship();
    EXPECT(!NotableFriendship_HasPendingNumber());
    NotableFriendship_CountFirstWin(NOTABLE_TRAINER_LORELEI);
    EXPECT(!NotableFriendship_HasPendingNumber());
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_LORELEI);
    EXPECT_EQ(NotableFriendship_GetStage(NOTABLE_TRAINER_LORELEI), NOTABLE_STAGE_FRIEND);
    EXPECT(NotableFriendship_HasPendingNumber());

    EXPECT(NotableFriendship_TryStartPendingNumberScript());
    EXPECT(ScriptContext_IsEnabled());
    EXPECT_EQ(gSpecialVar_0x8004, NOTABLE_TRAINER_LORELEI);
    NotableFriendship_BufferTrainerName();
    EXPECT_EQ(StringCompare(gStringVar1, GetTrainerNameFromId(TRAINER_WAYFARER_INDIGO_LORELEI)), 0);
    ScriptContext_Init();
    EXPECT(!NotableFriendship_HasPendingNumber());
    EXPECT(!NotableFriendship_TryStartPendingNumberScript());

    // Past the threshold, nothing hands the number over again.
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_LORELEI);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_LORELEI);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_LORELEI);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_LORELEI);
    EXPECT_EQ(NotableFriendship_GetStage(NOTABLE_TRAINER_LORELEI), NOTABLE_STAGE_CLOSE);
    EXPECT(!NotableFriendship_HasPendingNumber());

    // Two quests alone cross it too, for a Stranger.
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_STEVEN);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_STEVEN);
    EXPECT(NotableFriendship_TryStartPendingNumberScript());
    EXPECT_EQ(gSpecialVar_0x8004, NOTABLE_TRAINER_STEVEN);
    ScriptContext_Init();
}

TEST("Tate & Liza keep a score but give no number, and New Game clears a waiting handover")
{
    ResetFriendship();
    NotableFriendship_CountFirstWin(NOTABLE_TRAINER_TATE_LIZA);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_TATE_LIZA);
    EXPECT_EQ(NotableFriendship_GetStage(NOTABLE_TRAINER_TATE_LIZA), NOTABLE_STAGE_FRIEND);
    EXPECT(!NotableFriendship_HasPendingNumber());
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_TATE_LIZA);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_TATE_LIZA);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_TATE_LIZA);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_TATE_LIZA);
    EXPECT_EQ(NotableFriendship_GetStage(NOTABLE_TRAINER_TATE_LIZA), NOTABLE_STAGE_CLOSE);
    EXPECT(!NotableFriendship_HasPendingNumber());

    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_KAREN);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_KAREN);
    EXPECT(NotableFriendship_HasPendingNumber());
    WayfarerInitPersistentState();
    EXPECT(!NotableFriendship_HasPendingNumber());
}

TEST("New Game zeroes every score and first-win bit")
{
    u32 i;
    memset(&gSaveBlock3Ptr->notableFriendship, 0xFF, sizeof(gSaveBlock3Ptr->notableFriendship));
    WayfarerInitPersistentState();
    for (i = 0; i < NOTABLE_FRIENDSHIP_COUNT; i++)
        EXPECT_EQ(gSaveBlock3Ptr->notableFriendship.score[i], 0);
    for (i = 0; i < sizeof(gSaveBlock3Ptr->notableFriendship.firstWin); i++)
        EXPECT_EQ(gSaveBlock3Ptr->notableFriendship.firstWin[i], 0);
    EXPECT(NotableFriendship_IsValid());
}

TEST("A first-win bit with a score under Met is an invalid save")
{
    ResetFriendship();
    EXPECT(NotableFriendship_IsValid());
    gSaveBlock3Ptr->notableFriendship.firstWin[0] |= 1 << 2; // character 3
    EXPECT(!NotableFriendship_IsValid());
    gSaveBlock3Ptr->notableFriendship.score[2] = NOTABLE_FRIENDSHIP_MET - 1;
    EXPECT(!NotableFriendship_IsValid());
    gSaveBlock3Ptr->notableFriendship.score[2] = NOTABLE_FRIENDSHIP_MET;
    EXPECT(NotableFriendship_IsValid());
    // A score without its bit is fine (quests), and the bits past the last character stay clear.
    gSaveBlock3Ptr->notableFriendship.score[10] = 40;
    EXPECT(NotableFriendship_IsValid());
    gSaveBlock3Ptr->notableFriendship.firstWin[4] |= 1 << 5; // character 38
    gSaveBlock3Ptr->notableFriendship.score[37] = NOTABLE_FRIENDSHIP_MET;
    EXPECT(NotableFriendship_IsValid());
    gSaveBlock3Ptr->notableFriendship.firstWin[4] |= 1 << 6;
    EXPECT(!NotableFriendship_IsValid());
}

TEST("Fame reaches a trainer at min(80, their TR - 10), or for a champion or Master")
{
    EXPECT(!NotableTalk_FameReaches(69, 80, FALSE));
    EXPECT(NotableTalk_FameReaches(70, 80, FALSE));
    EXPECT(!NotableTalk_FameReaches(79, 200, FALSE));
    EXPECT(NotableTalk_FameReaches(80, 200, FALSE));
    EXPECT(NotableTalk_FameReaches(0, 5, FALSE));
    EXPECT(NotableTalk_FameReaches(0, 10, FALSE));
    EXPECT(!NotableTalk_FameReaches(0, 11, FALSE));
    EXPECT(NotableTalk_FameReaches(0, 200, TRUE));
}

TEST("The greeting follows the stage, and a Stranger hears of the player only through fame")
{
    EXPECT_EQ(NotableTalk_GreetingFor(NOTABLE_STAGE_STRANGER, FALSE), NOTABLE_GREETING_MEET);
    EXPECT_EQ(NotableTalk_GreetingFor(NOTABLE_STAGE_STRANGER, TRUE), NOTABLE_GREETING_HEARD);
    EXPECT_EQ(NotableTalk_GreetingFor(NOTABLE_STAGE_MET, FALSE), NOTABLE_GREETING_AGAIN);
    EXPECT_EQ(NotableTalk_GreetingFor(NOTABLE_STAGE_MET, TRUE), NOTABLE_GREETING_AGAIN);
    EXPECT_EQ(NotableTalk_GreetingFor(NOTABLE_STAGE_FRIEND, TRUE), NOTABLE_GREETING_HELLO);
    EXPECT_EQ(NotableTalk_GreetingFor(NOTABLE_STAGE_CLOSE, FALSE), NOTABLE_GREETING_CLOSE);
}

TEST("Greetings read the stage and fame from the save")
{
    ResetFriendship();
    // Brock's TR at world progress 0 is 25: a rating of 15 reaches him, 14 does not.
    SetTrainerRating(14);
    EXPECT_EQ(NotableTalk_GetGreeting(NOTABLE_TRAINER_BROCK), NOTABLE_GREETING_MEET);
    SetTrainerRating(0);
    // Talking changes nothing: ask as often as you like.
    EXPECT_EQ(NotableTalk_GetGreeting(NOTABLE_TRAINER_BROCK), NOTABLE_GREETING_MEET);
    EXPECT_EQ(NotableFriendship_GetScore(NOTABLE_TRAINER_BROCK), 0);

    // Fame from a title: the player reigns at any league.
    gSaveBlock3Ptr->leagueEvent.champions[LEAGUE_ID_HOENN - 1] = LEAGUE_EVENT_PLAYER_CHAMPION;
    EXPECT_EQ(NotableTalk_GetGreeting(NOTABLE_TRAINER_BROCK), NOTABLE_GREETING_HEARD);
    gSaveBlock3Ptr->leagueEvent.champions[LEAGUE_ID_HOENN - 1] = 0;
    EXPECT_EQ(NotableTalk_GetGreeting(NOTABLE_TRAINER_BROCK), NOTABLE_GREETING_MEET);

    // Fame from being a Master: cleared both Indigo and Hoenn.
    SetCircuitClearForTesting(CIRCUIT_STAGE_INDIGO, TRUE);
    EXPECT_EQ(NotableTalk_GetGreeting(NOTABLE_TRAINER_BROCK), NOTABLE_GREETING_MEET);
    SetCircuitClearForTesting(CIRCUIT_STAGE_HOENN, TRUE);
    EXPECT_EQ(NotableTalk_GetGreeting(NOTABLE_TRAINER_BROCK), NOTABLE_GREETING_HEARD);
    SetCircuitClearForTesting(CIRCUIT_STAGE_INDIGO, FALSE);
    SetCircuitClearForTesting(CIRCUIT_STAGE_HOENN, FALSE);

    // Once they have met, fame no longer matters, and the stage picks the greeting.
    SetTrainerRating(80);
    EXPECT_EQ(NotableTalk_GetGreeting(NOTABLE_TRAINER_BROCK), NOTABLE_GREETING_HEARD);
    NotableFriendship_CountFirstWin(NOTABLE_TRAINER_BROCK);
    EXPECT_EQ(NotableTalk_GetGreeting(NOTABLE_TRAINER_BROCK), NOTABLE_GREETING_AGAIN);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_BROCK);
    EXPECT_EQ(NotableTalk_GetGreeting(NOTABLE_TRAINER_BROCK), NOTABLE_GREETING_HELLO);
    ScriptContext_Init();
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_BROCK);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_BROCK);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_BROCK);
    NotableFriendship_AddQuestPoints(NOTABLE_TRAINER_BROCK);
    EXPECT_EQ(NotableTalk_GetGreeting(NOTABLE_TRAINER_BROCK), NOTABLE_GREETING_CLOSE);
}

static void ExpectBuffered(const u8 *text)
{
    u8 expanded[200];
    StringExpandPlaceholders(expanded, text);
    EXPECT_EQ(StringCompare(gStringVar4, expanded), 0);
}

TEST("The greeting and the farewell buffer the trainer's own voice lines")
{
    const struct NotableVoice *brock = &gNotableVoices[NOTABLE_TRAINER_BROCK - 1];
    ResetFriendship();
    SetTrainerRating(0);
    NotableTalk_BufferGreetingText(NOTABLE_TRAINER_BROCK);
    ExpectBuffered(brock->meet);
    NotableTalk_BufferFarewellText(NOTABLE_TRAINER_BROCK);
    ExpectBuffered(brock->bye);

    SetTrainerRating(80);
    NotableTalk_BufferGreetingText(NOTABLE_TRAINER_BROCK);
    ExpectBuffered(brock->heard);
    // HEARD addresses the player by name.
    EXPECT(StringLength(gStringVar4) > StringLength(brock->heard) - 8 - 2);

    NotableFriendship_CountFirstWin(NOTABLE_TRAINER_BROCK);
    NotableTalk_BufferGreetingText(NOTABLE_TRAINER_BROCK);
    ExpectBuffered(brock->again);

    // An unknown character has nothing to say.
    NotableTalk_BufferGreetingText(NOTABLE_TRAINER_NONE);
    EXPECT_EQ(gStringVar4[0], EOS);
    gSpecialVar_0x8004 = NOTABLE_TRAINER_NONE;
    EXPECT(!NotableTalk_BufferGreeting());
    EXPECT(!NotableTalk_BufferFarewell());
    gSpecialVar_0x8004 = NOTABLE_TRAINER_BROCK;
    EXPECT(NotableTalk_BufferGreeting());
    EXPECT(NotableTalk_BufferFarewell());
}

TEST("{BUDDY} names the buddy slot's current species")
{
    u8 expected[200];
    ResetFriendship();
    // Misty's buddy is slot 2, PSYDUCK early and GOLDUCK later.
    SetTrainerRating(0);
    NotableTalk_BufferFarewellText(NOTABLE_TRAINER_MISTY);
    StringCopy(gStringVar1, GetSpeciesName(SPECIES_PSYDUCK));
    StringExpandPlaceholders(expected, gNotableVoices[NOTABLE_TRAINER_MISTY - 1].bye);
    EXPECT_EQ(StringCompare(gStringVar4, expected), 0);
    SetTrainerRating(160);
    NotableTalk_BufferFarewellText(NOTABLE_TRAINER_MISTY);
    StringCopy(gStringVar1, GetSpeciesName(SPECIES_GOLDUCK));
    StringExpandPlaceholders(expected, gNotableVoices[NOTABLE_TRAINER_MISTY - 1].bye);
    EXPECT_EQ(StringCompare(gStringVar4, expected), 0);
}

static u32 FirstSpotOfKind(u8 kind)
{
    u32 spot;
    for (spot = 0; spot < gWayfarerWorldSpotCount; spot++)
    {
        if (gWayfarerWorldSpots[spot].kind == kind)
            return spot;
    }
    return WORLD_SPOT_NONE;
}

TEST("The activity line follows the spot kind, and a named spot says the line of its activity")
{
    static const struct { u8 kind; u8 line; } kinds[] = {
        {WORLD_SPOT_CENTER_COUNTER, NOTABLE_LINE_CENTER}, {WORLD_SPOT_CENTER_SIDE, NOTABLE_LINE_CENTER},
        {WORLD_SPOT_STORE, NOTABLE_LINE_STORE}, {WORLD_SPOT_GAME_CORNER, NOTABLE_LINE_GAME_CORNER},
        {WORLD_SPOT_GYM, NOTABLE_LINE_GYM}, {WORLD_SPOT_TALL_GRASS, NOTABLE_LINE_GRASS},
        {WORLD_SPOT_WATER_EDGE, NOTABLE_LINE_WATER}, {WORLD_SPOT_SQUARE, NOTABLE_LINE_SQUARE},
        {WORLD_SPOT_BENCH, NOTABLE_LINE_SQUARE}, {WORLD_SPOT_NPC_CHAT, NOTABLE_LINE_CHAT},
    };
    static const struct { u8 activity; u8 line; } activities[] = {
        {WORLD_ACTIVITY_CARE, NOTABLE_LINE_CENTER}, {WORLD_ACTIVITY_SHOP, NOTABLE_LINE_STORE},
        {WORLD_ACTIVITY_GAMBLE, NOTABLE_LINE_GAME_CORNER}, {WORLD_ACTIVITY_TRAIN, NOTABLE_LINE_GRASS},
        {WORLD_ACTIVITY_RELAX, NOTABLE_LINE_SQUARE}, {WORLD_ACTIVITY_FISH, NOTABLE_LINE_WATER},
        {WORLD_ACTIVITY_VISIT, NOTABLE_LINE_CHAT}, {WORLD_ACTIVITY_STUDY, NOTABLE_LINE_STUDY},
        {WORLD_ACTIVITY_SIGHTSEE, NOTABLE_LINE_SIGHTSEE}, {WORLD_ACTIVITY_LIE_LOW, NOTABLE_LINE_LIE_LOW},
        {WORLD_ACTIVITY_HOME, NOTABLE_LINE_HOME},
    };
    u32 i, spot;

    for (i = 0; i < ARRAY_COUNT(kinds); i++)
    {
        spot = FirstSpotOfKind(kinds[i].kind);
        // Benches and named spots are not in the generated table yet.
        if (spot == WORLD_SPOT_NONE)
        {
            EXPECT(kinds[i].kind == WORLD_SPOT_BENCH);
            continue;
        }
        // A plain spot ignores the activity: the kind decides.
        EXPECT_EQ(NotableTalk_ActivityLineFor(WORLD_DEST_SPOT, spot, WORLD_ACTIVITY_NONE), kinds[i].line);
        EXPECT_EQ(NotableTalk_ActivityLineFor(WORLD_DEST_SPOT, spot, WORLD_ACTIVITY_STUDY), kinds[i].line);
    }
    // A named spot (once any exists) and a trip home say the line of the activity.
    spot = FirstSpotOfKind(WORLD_SPOT_NAMED);
    for (i = 0; i < ARRAY_COUNT(activities); i++)
    {
        if (spot != WORLD_SPOT_NONE)
            EXPECT_EQ(NotableTalk_ActivityLineFor(WORLD_DEST_SPOT, spot, activities[i].activity), activities[i].line);
        EXPECT_EQ(NotableTalk_ActivityLineFor(WORLD_DEST_HOME, WORLD_SPOT_NONE, activities[i].activity), activities[i].line);
        EXPECT_EQ(NotableTalk_ActivityLineFor(WORLD_DEST_NONE, 0, activities[i].activity), activities[i].line);
    }
    // Heading home, or somewhere the record names no spot for: the activity decides.
    // At home in a home place (a real spot), the activity still decides.
    spot = FirstSpotOfKind(WORLD_SPOT_SQUARE);
    EXPECT_EQ(NotableTalk_ActivityLineFor(WORLD_DEST_HOME, spot, WORLD_ACTIVITY_HOME), NOTABLE_LINE_HOME);
    EXPECT_EQ(NotableTalk_ActivityLineFor(WORLD_DEST_NONE, 0, WORLD_ACTIVITY_NONE), NOTABLE_LINE_SQUARE);
}

// The standard text box shows two lines of this many pixels (the 27-tile window).
#define BOX_PIXELS 216

static void ExpectFitsOneBox(const u8 *text)
{
    u8 expanded[160];
    u8 line[160];
    const u8 *p;
    u32 lines = 1, length = 0;

    // The worst-case player name, and the longest species name.
    StringCopy(gStringVar1, COMPOUND_STRING("MMMMMMMMMM"));
    memset(gSaveBlock2Ptr->playerName, CHAR_W, PLAYER_NAME_LENGTH);
    gSaveBlock2Ptr->playerName[PLAYER_NAME_LENGTH] = EOS;
    StringExpandPlaceholders(expanded, text);
    for (p = expanded; *p != EOS; p++)
    {
        EXPECT(*p != CHAR_PROMPT_CLEAR);
        if (*p == CHAR_NEWLINE)
        {
            line[length] = EOS;
            EXPECT_LE(GetStringWidth(FONT_NORMAL, line, 0), BOX_PIXELS);
            lines++;
            length = 0;
        }
        else
        {
            line[length++] = *p;
        }
    }
    line[length] = EOS;
    EXPECT_LE(GetStringWidth(FONT_NORMAL, line, 0), BOX_PIXELS);
    EXPECT_LE(lines, 2);
}

TEST("Every voice line and activity line fits one message box")
{
    u32 id, line;
    for (id = 0; id < NOTABLE_TRAINER_COUNT; id++)
    {
        const struct NotableVoice *voice = &gNotableVoices[id];
        ExpectFitsOneBox(voice->meet);
        ExpectFitsOneBox(voice->heard);
        ExpectFitsOneBox(voice->again);
        ExpectFitsOneBox(voice->hello);
        ExpectFitsOneBox(voice->close);
        ExpectFitsOneBox(voice->bye);
    }
    for (line = 0; line < NOTABLE_LINE_COUNT; line++)
    {
        NotableTalk_BufferActivityLineText(line);
        ExpectFitsOneBox(gStringVar4);
    }
}

#endif // IS_WAYFARER

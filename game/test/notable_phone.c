#include "global.h"
#include "data.h"
#include "event_data.h"
#include "league_circuit.h"
#include "league_events.h"
#include "league_run_helpers.h"
#include "league_selection.h"
#include "malloc.h"
#include "notable_friendship.h"
#include "notable_phone.h"
#include "notable_talk.h"
#include "notable_trainers.h"
#include "overworld.h"
#include "pokenav.h"
#include "region_map.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer_rating.h"
#include "wayfarer_daily_slots.h"
#include "wayfarer_persistence.h"
#include "wayfarer_phone.h"
#include "wayfarer_world.h"
#include "wayfarer_world_data.h"
#include "wayfarer_world_sim.h"
#include "test/test.h"
#include "constants/characters.h"
#include "constants/maps.h"
#include "constants/notable_trainers.h"
#include "constants/regions.h"
#include "constants/wayfarer_world.h"

#if IS_WAYFARER

// Notable trainers as phone contacts (.product/specs/notable-trainers.md#phone-contacts). Nobody reaches Friend
// until haunts ship, so these tests set the friendship score directly.

#define PHONE_LINE_PIXELS 192  // the call box: 28 tiles, text 32 pixels in (pokenav_match_call_gfx.c)

static const u8 sNext[] = _("\p");
static const u8 sEmpty[] = _("");
static const u8 sWhereAround[] = _("I'm around {STR_VAR_2}\nright now.");
static const u8 sWhereTravelling[] = _("I'm on the road right now,\npassing through {STR_VAR_2}.");
static const u8 sWhereOwnGym[] = _("I'm at my GYM in\n{STR_VAR_2} right now.");
static const u8 sWhereLeagueHall[] = _("I'm at the POKéMON LEAGUE in\n{STR_VAR_2}.");
static const u8 sWhereLeague[] = _("I'm at the POKéMON LEAGUE,\ngetting ready for you!");
static const u8 sWherePartner[] = _("I'm right here with you,\nremember?");
static const u8 sWhereUnknown[] = _("I'm out and about right now.");
static const u8 sCenterLine[] = _("Just letting my team rest up.");
static const u8 sBrockHello[] = _("Hey, A! BROCK here. Have you\nbeen eating well?");
static const u8 sBrockClose[] = _("A! Always glad to hear from\nyou, friend!");
static const u8 sBrockBye[] = _("Talk soon! Keep your POKéMON well\nfed!");

static void ResetPhone(void)
{
    WayfarerInitPersistentState();
    NotableFriendship_Reset();
    WayfarerWorld_InitNewGame();
    // {PLAYER} expands the name in the save, which a test starts without.
    gSaveBlock2Ptr->playerName[0] = CHAR_A;
    gSaveBlock2Ptr->playerName[1] = EOS;
}

static void SetScore(u8 characterId, u8 score)
{
    gSaveBlock3Ptr->notableFriendship.score[characterId - 1] = score;
}

static struct WayfarerWorldRecord *Record(u8 characterId)
{
    u8 slot = WorldSim_SlotForCharacter(characterId);

    ASSUME(slot < WORLD_SIM_TRAINER_COUNT);
    return &WayfarerWorld_GetState()->records[slot];
}

static const struct WayfarerWorldTrainer *WorldTrainer(u8 characterId)
{
    return &gWayfarerWorldTrainers[WorldSim_SlotForCharacter(characterId)];
}

static void PlaceOfNode(u8 *dest, u16 node)
{
    u16 map = WorldSim_NodeMap(node);

    ASSUME(map != 0xFFFF);
    WayfarerGetPlaceNameOfMap(dest, map >> 8, map & 0xFF);
}

static void PlaceOfMap(u8 *dest, u16 map)
{
    WayfarerGetPlaceNameOfMap(dest, map >> 8, map & 0xFF);
}

// The call is the greeting, the where line ({STR_VAR_2} = place), the activity line if any, and the farewell,
// one message box each.
static bool32 CallIs(u8 characterId, const u8 *greeting, const u8 *where, const u8 *place, const u8 *activity,
                     const u8 *bye)
{
    u8 *actual = Alloc(1000), *expected = Alloc(1000), *end;
    bool32 same;

    ASSUME(actual != NULL && expected != NULL);
    NotablePhone_BufferCall(characterId, actual);
    StringCopy(gStringVar2, place != NULL ? place : sEmpty);
    NotableTalk_BufferBuddy(characterId);
    end = StringExpandPlaceholders(expected, greeting);
    end = StringCopy(end, sNext);
    end = StringExpandPlaceholders(end, where);
    if (activity != NULL)
    {
        end = StringCopy(end, sNext);
        end = StringCopy(end, activity);
    }
    end = StringCopy(end, sNext);
    StringExpandPlaceholders(end, bye);
    same = StringCompare(actual, expected) == 0;
    Free(actual);
    Free(expected);
    return same;
}

static const u8 *CallHello(u8 characterId) { return gNotableVoices[characterId - 1].callHello; }
static const u8 *CallBye(u8 characterId) { return gNotableVoices[characterId - 1].callBye; }

// The widest line of a message, split at line and box breaks.
static s32 WidestLine(const u8 *text)
{
    u8 line[128];
    s32 widest = 0, width;
    u32 i, n = 0;

    for (i = 0;; i++)
    {
        if (text[i] == CHAR_NEWLINE || text[i] == CHAR_PROMPT_CLEAR || text[i] == EOS)
        {
            line[n] = EOS;
            width = GetStringWidth(FONT_NORMAL, line, 0);
            if (width > widest)
                widest = width;
            n = 0;
            if (text[i] == EOS)
                return widest;
        }
        else if (n < sizeof(line) - 1)
        {
            line[n++] = text[i];
        }
    }
}

TEST("A notable trainer is a contact from Friend on, and Tate & Liza never are")
{
    ResetPhone();
    EXPECT_EQ(NotablePhone_CountContacts(), 0);
    SetScore(NOTABLE_TRAINER_BROCK, NOTABLE_FRIENDSHIP_FRIEND - 1);
    EXPECT(!NotablePhone_IsContact(NOTABLE_TRAINER_BROCK));
    SetScore(NOTABLE_TRAINER_BROCK, NOTABLE_FRIENDSHIP_FRIEND);
    EXPECT(NotablePhone_IsContact(NOTABLE_TRAINER_BROCK));
    SetScore(NOTABLE_TRAINER_BROCK, 255);
    EXPECT(NotablePhone_IsContact(NOTABLE_TRAINER_BROCK));
    SetScore(NOTABLE_TRAINER_TATE_LIZA, 255);
    EXPECT(!NotablePhone_IsContact(NOTABLE_TRAINER_TATE_LIZA));
    EXPECT_EQ(NotablePhone_CountContacts(), 1);
    EXPECT(!NotablePhone_IsContact(NOTABLE_TRAINER_NONE));
    EXPECT(!NotablePhone_IsContact(NOTABLE_TRAINER_COUNT + 1));
    // Not a contact: the call is empty.
    NotablePhone_BufferCall(NOTABLE_TRAINER_TATE_LIZA, gStringVar4);
    EXPECT_EQ(gStringVar4[0], EOS);
}

TEST("Every possible contact has phone lines, a trainer, a home, and a place to be")
{
    u8 characterId, mapGroup, mapNum;
    struct NotablePhoneLocation location;

    ResetPhone();
    for (characterId = 1; characterId <= NOTABLE_TRAINER_COUNT; characterId++)
    {
        const struct NotableVoice *voice = &gNotableVoices[characterId - 1];

        if (characterId == NOTABLE_TRAINER_TATE_LIZA)
        {
            EXPECT(voice->callHello == NULL && voice->callClose == NULL && voice->callBye == NULL);
            continue;
        }
        EXPECT(voice->callHello != NULL && voice->callClose != NULL && voice->callBye != NULL);
        EXPECT(NotablePhone_GetTrainerId(characterId) != 0);
        EXPECT(NotablePhone_GetHome(characterId, &mapGroup, &mapNum));
        // A world record (the simulated trainers) or a fixed place (the face-only ones) always names a place.
        NotablePhone_GetLocation(characterId, &location);
        EXPECT(location.where != NOTABLE_WHERE_UNKNOWN);
        WayfarerGetPlaceNameOfMap(gStringVar2, location.mapGroup, location.mapNum);
        EXPECT(gStringVar2[0] != EOS);
    }
}

TEST("A call is the stage's phone greeting, where they are, the activity, and the farewell")
{
    struct WayfarerWorldRecord *record;
    u8 place[32];
    u16 spot;

    ResetPhone();
    for (spot = 0; spot < gWayfarerWorldSpotCount; spot++)
    {
        if (gWayfarerWorldSpots[spot].kind == WORLD_SPOT_CENTER_COUNTER)
            break;
    }
    ASSUME(spot < gWayfarerWorldSpotCount);
    record = Record(NOTABLE_TRAINER_BROCK);
    record->state = WORLD_STATE_DWELLING;
    record->node = gWayfarerWorldSpots[spot].node;
    record->destKind = WORLD_DEST_SPOT;
    record->destId = spot;
    record->activity = WORLD_ACTIVITY_CARE;
    PlaceOfNode(place, record->node);

    SetScore(NOTABLE_TRAINER_BROCK, NOTABLE_FRIENDSHIP_FRIEND);
    EXPECT(CallIs(NOTABLE_TRAINER_BROCK, sBrockHello, sWhereAround, place, sCenterLine, sBrockBye));
    // Close greets with its own line; the place and farewell don't change.
    SetScore(NOTABLE_TRAINER_BROCK, NOTABLE_FRIENDSHIP_CLOSE);
    EXPECT(CallIs(NOTABLE_TRAINER_BROCK, sBrockClose, sWhereAround, place, sCenterLine, sBrockBye));
    // Calling reads the record and never changes it.
    EXPECT_EQ((u32)record->state, WORLD_STATE_DWELLING);
    EXPECT_EQ((u32)record->destId, spot);
}

TEST("On the road, a contact names the map they are passing through")
{
    struct WayfarerWorldRecord *record;
    u8 place[32];

    ResetPhone();
    SetScore(NOTABLE_TRAINER_BLUE, NOTABLE_FRIENDSHIP_FRIEND);
    record = Record(NOTABLE_TRAINER_BLUE);
    record->state = WORLD_STATE_TRAVELLING;
    record->node = WorldTrainer(NOTABLE_TRAINER_BLUE)->homeNode;
    PlaceOfNode(place, record->node);
    EXPECT(CallIs(NOTABLE_TRAINER_BLUE, CallHello(NOTABLE_TRAINER_BLUE), sWhereTravelling, place, NULL,
                  CallBye(NOTABLE_TRAINER_BLUE)));
}

TEST("A Gym Leader in their own Gym says so, before and after the badge")
{
    static const u8 sCerulean[] = _("CERULEAN CITY");
    struct WayfarerWorldRecord *record;
    const struct WayfarerWorldTrainer *misty;
    u8 place[32];

    ResetPhone();
    SetScore(NOTABLE_TRAINER_MISTY, NOTABLE_FRIENDSHIP_FRIEND);
    misty = WorldTrainer(NOTABLE_TRAINER_MISTY);
    record = Record(NOTABLE_TRAINER_MISTY);
    PlaceOfNode(place, misty->gymNode);
    EXPECT_EQ(StringCompare(place, sCerulean), 0);
    record->state = WORLD_STATE_HOME_LOCKED;
    EXPECT(CallIs(NOTABLE_TRAINER_MISTY, CallHello(NOTABLE_TRAINER_MISTY), sWhereOwnGym, place, NULL,
                  CallBye(NOTABLE_TRAINER_MISTY)));
    // Dwelling at home is their Gym for a leader.
    record->state = WORLD_STATE_DWELLING;
    record->destKind = WORLD_DEST_HOME;
    record->node = misty->gymNode;
    record->activity = WORLD_ACTIVITY_HOME;
    EXPECT(CallIs(NOTABLE_TRAINER_MISTY, CallHello(NOTABLE_TRAINER_MISTY), sWhereOwnGym, place, NULL,
                  CallBye(NOTABLE_TRAINER_MISTY)));
}

TEST("Back from a league, pinned at a haunt, or away as the partner, a contact still says where")
{
    struct WayfarerWorldRecord *record;
    u8 place[32];

    ResetPhone();
    SetScore(NOTABLE_TRAINER_LANCE, NOTABLE_FRIENDSHIP_FRIEND);
    record = Record(NOTABLE_TRAINER_LANCE);
    // The event is over (no accepted lineup) but the heartbeat hasn't brought them home yet: they say home.
    record->state = WORLD_STATE_AWAY_LEAGUE;
    PlaceOfMap(place, WorldTrainer(NOTABLE_TRAINER_LANCE)->homeMap);
    EXPECT(CallIs(NOTABLE_TRAINER_LANCE, CallHello(NOTABLE_TRAINER_LANCE), sWhereAround, place, NULL,
                  CallBye(NOTABLE_TRAINER_LANCE)));
    record->state = WORLD_STATE_PINNED;
    record->node = WorldTrainer(NOTABLE_TRAINER_LANCE)->homeNode;
    PlaceOfNode(place, record->node);
    EXPECT(CallIs(NOTABLE_TRAINER_LANCE, CallHello(NOTABLE_TRAINER_LANCE), sWhereAround, place, NULL,
                  CallBye(NOTABLE_TRAINER_LANCE)));
    record->state = WORLD_STATE_AWAY_PARTNER;
    EXPECT(CallIs(NOTABLE_TRAINER_LANCE, CallHello(NOTABLE_TRAINER_LANCE), sWherePartner, NULL, NULL,
                  CallBye(NOTABLE_TRAINER_LANCE)));
    // A node outside the graph names no place.
    record->state = WORLD_STATE_TRAVELLING;
    record->node = WORLD_NODE_NONE;
    EXPECT(CallIs(NOTABLE_TRAINER_LANCE, CallHello(NOTABLE_TRAINER_LANCE), sWhereUnknown, NULL, NULL,
                  CallBye(NOTABLE_TRAINER_LANCE)));
}

TEST("The face-only contacts are at their fixed place: their Gym, or their league")
{
    static const u8 sRustboro[] = _("RUSTBORO CITY");
    static const u8 sEverGrande[] = _("EVER GRANDE CITY");
    static const u8 sIndigo[] = _("INDIGO PLATEAU");

    ResetPhone();
    SetScore(NOTABLE_TRAINER_ROXANNE, NOTABLE_FRIENDSHIP_FRIEND);
    SetScore(NOTABLE_TRAINER_SIDNEY, NOTABLE_FRIENDSHIP_FRIEND);
    SetScore(NOTABLE_TRAINER_BRUNO, NOTABLE_FRIENDSHIP_FRIEND);
    SetScore(NOTABLE_TRAINER_KOGA, NOTABLE_FRIENDSHIP_FRIEND);
    EXPECT(CallIs(NOTABLE_TRAINER_ROXANNE, CallHello(NOTABLE_TRAINER_ROXANNE), sWhereOwnGym, sRustboro, NULL,
                  CallBye(NOTABLE_TRAINER_ROXANNE)));
    EXPECT(CallIs(NOTABLE_TRAINER_SIDNEY, CallHello(NOTABLE_TRAINER_SIDNEY), sWhereLeagueHall, sEverGrande, NULL,
                  CallBye(NOTABLE_TRAINER_SIDNEY)));
    EXPECT(CallIs(NOTABLE_TRAINER_BRUNO, CallHello(NOTABLE_TRAINER_BRUNO), sWhereLeagueHall, sIndigo, NULL,
                  CallBye(NOTABLE_TRAINER_BRUNO)));
    EXPECT(CallIs(NOTABLE_TRAINER_KOGA, CallHello(NOTABLE_TRAINER_KOGA), sWhereLeagueHall, sIndigo, NULL,
                  CallBye(NOTABLE_TRAINER_KOGA)));
}

#if WAYFARER_LEAGUE_EVENTS
TEST("A contact in the accepted league event's lineup is at the league")
{
    const struct LeagueSavedTeam *team;
    u8 badge;

    ResetPhone();
    for (badge = 0; badge < 8; badge++)
    {
        SetBadgeStateForRegion(REGION_KANTO, badge, TRUE);
        SetBadgeStateForRegion(REGION_HOENN, badge, TRUE);
    }
    SetTrainerRating(160);
    ASSUME(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    ASSUME(GetAcceptedLeagueEventMember(0, &team));
    ASSUME(team->characterId != NOTABLE_TRAINER_TATE_LIZA);
    SetScore(team->characterId, NOTABLE_FRIENDSHIP_FRIEND);
    EXPECT(CallIs(team->characterId, CallHello(team->characterId), sWhereLeague, NULL, NULL,
                  CallBye(team->characterId)));
}
#endif // WAYFARER_LEAGUE_EVENTS

TEST("Every line of a call fits the phone's call box")
{
    static const u8 sWidePlace[] = _("WWWWWWWWWWWWWWWW");
    static const u8 *const sWhereLines[] = {
        sWhereAround, sWhereTravelling, sWhereOwnGym, sWhereLeagueHall, sWhereLeague, sWherePartner, sWhereUnknown,
    };
    u32 i;
    u8 characterId;

    ResetPhone();
    // The widest place a {STR_VAR_2} can hold (16 characters).
    StringCopy(gStringVar2, sWidePlace);
    for (i = 0; i < ARRAY_COUNT(sWhereLines); i++)
    {
        StringExpandPlaceholders(gStringVar4, sWhereLines[i]);
        EXPECT_LE(WidestLine(gStringVar4), PHONE_LINE_PIXELS);
    }
    for (i = 0; i < NOTABLE_LINE_COUNT; i++)
        EXPECT_LE(WidestLine(NotableTalk_GetActivityLine(i)), PHONE_LINE_PIXELS);
    // Every contact's whole call as it stands at New Game, with a seven-letter name of the widest letter.
    for (i = 0; i < PLAYER_NAME_LENGTH; i++)
        gSaveBlock2Ptr->playerName[i] = CHAR_W;
    gSaveBlock2Ptr->playerName[PLAYER_NAME_LENGTH] = EOS;
    for (characterId = 1; characterId <= NOTABLE_TRAINER_COUNT; characterId++)
    {
        if (characterId == NOTABLE_TRAINER_TATE_LIZA)
            continue;
        SetScore(characterId, NOTABLE_FRIENDSHIP_FRIEND);
        NotablePhone_BufferCall(characterId, gStringVar4);
        EXPECT_LE(WidestLine(gStringVar4), PHONE_LINE_PIXELS);
        SetScore(characterId, NOTABLE_FRIENDSHIP_CLOSE);
        NotablePhone_BufferCall(characterId, gStringVar4);
        EXPECT_LE(WidestLine(gStringVar4), PHONE_LINE_PIXELS);
    }
}

TEST("Becoming a contact registers no route contact, so notables never make a random call")
{
    u8 characterId, contact;

    ResetPhone();
    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
    {
        if (PhoneContact_IsContact(contact))
            FlagClear(PhoneContact_RegisteredFlag(contact));
    }
    for (characterId = 1; characterId <= NOTABLE_TRAINER_COUNT; characterId++)
        SetScore(characterId, 255);
    EXPECT_EQ(NotablePhone_CountContacts(), NOTABLE_TRAINER_COUNT - 1);
    // Random calls choose only among registered route contacts (match_call.c, SelectMatchCallTrainer).
    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
        EXPECT(!PhoneContact_IsRegistered(contact));
}

static bool32 Contains(const u8 *haystack, const u8 *needle)
{
    u32 i, j;

    for (i = 0; haystack[i] != EOS; i++)
    {
        for (j = 0; needle[j] != EOS && haystack[i + j] == needle[j]; j++)
            ;
        if (needle[j] == EOS)
            return TRUE;
    }
    return FALSE;
}

// The Match Call list as the Pokénav builds it, run to its end. The resources struct is private to pokenav.c, so
// this is its layout (as in test/wayfarer_phone.c).
struct PokenavResourcesView
{
    u32 (*currentMenuCb1)(void);
    u32 currentMenuIndex;
    u16 mode;
    u16 conditionSearchId;
    bool32 hasAnyRibbons;
    void *substructPtrs[POKENAV_SUBSTRUCT_COUNT];
};
extern struct PokenavResourcesView *gPokenavResources;

static u32 BuildMatchCallList(void)
{
    u32 frames;

    gPokenavResources = AllocZeroed(sizeof(*gPokenavResources));
    ASSUME(gPokenavResources != NULL);
    ASSUME(PokenavCallback_Init_MatchCall());
    for (frames = 0; frames < 600 && !IsMatchCallListInitFinished(); frames++)
        RunTasks();
    return IsMatchCallListInitFinished();
}

static void FreeMatchCallList(void)
{
    FreePokenavSubstruct(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    Free(gPokenavResources);
    gPokenavResources = NULL;
}

TEST("Notable contacts are listed once each, after the headers and before the route contacts")
{
    struct PokenavMatchCallEntry *list;
    u8 characterId, contact;
    u32 i, entries, routeContacts = 0, firstRoute, lastNotable = 0;
    struct MapHeader header = gMapHeader;
    u8 *call = Alloc(1000);

    ASSUME(call != NULL);
    ResetPhone();
    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
    {
        if (!PhoneContact_IsContact(contact))
            continue;
        FlagSet(PhoneContact_RegisteredFlag(contact));
        routeContacts++;
    }
    for (characterId = 1; characterId <= NOTABLE_TRAINER_COUNT; characterId++)
        SetScore(characterId, NOTABLE_FRIENDSHIP_CLOSE);
    // An entry is still the size of a Condition or Ribbons list item (they share one list item type).
    EXPECT_EQ(sizeof(struct PokenavMatchCallEntry), sizeof(struct PokenavMonListItem));
    // With everyone, the list still has room.
    EXPECT(MATCH_CALL_ENTRIES_MAX >= MC_HEADER_COUNT + (NOTABLE_TRAINER_COUNT - 1) + routeContacts);
    ASSUME(BuildMatchCallList());
    entries = GetNumberRegistered();
    list = GetMatchCallList();
    gMapHeader.mapType = MAP_TYPE_TOWN;  // calls can be made here
    for (characterId = 1; characterId <= NOTABLE_TRAINER_COUNT; characterId++)
    {
        u32 found = 0;

        for (i = 0; i < entries; i++)
        {
            bool8 rematch;

            if (list[i].isSpecialTrainer || list[i].headerId != MATCH_CALL_NOTABLE_HEADER(characterId))
                continue;
            found++;
            lastNotable = i;
            EXPECT(!list[i].isSpecialTrainer);
            EXPECT(!ShouldDrawRematchPokeballIcon(i));
            EXPECT_EQ(GetMatchCallTrainerPic(i), GetTrainerPicFromId(NotablePhone_GetTrainerId(characterId)));
            EXPECT(GetMatchCallContactPlaceName(i, gStringVar1));
            EXPECT(gStringVar1[0] != EOS);
            // The name line is the character's own trainer, and there is no CHECK page text to read.
            BufferMatchCallNameAndDesc(&list[i], call);
            EXPECT(Contains(call, GetTrainerNameFromId(NotablePhone_GetTrainerId(characterId))));
            EXPECT_EQ(GetMatchCallFlavorText(i, 0)[0], EOS);
            GetMatchCallMessageText(i, &rematch);
            EXPECT(!rematch);
            StringCopy(call, gStringVar4);
            NotablePhone_BufferCall(characterId, gStringVar4);
            EXPECT_EQ(StringCompare(call, gStringVar4), 0);
        }
        EXPECT_EQ(found, characterId == NOTABLE_TRAINER_TATE_LIZA ? 0 : 1);
    }
    // Route contacts follow the notables, every one still listed.
    for (firstRoute = 0; firstRoute < entries; firstRoute++)
    {
        if (!list[firstRoute].isSpecialTrainer && list[firstRoute].headerId < MATCH_CALL_NOTABLE_HEADER_BASE)
            break;
    }
    EXPECT_LT(lastNotable, firstRoute);
    EXPECT_EQ(entries - firstRoute, routeContacts);
    for (i = 0; i < entries; i++)
    {
        if (list[i].isSpecialTrainer)
            EXPECT_LT(i, firstRoute);
    }
    FreeMatchCallList();
    Free(call);
    gMapHeader = header;
    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
    {
        if (PhoneContact_IsContact(contact))
            FlagClear(PhoneContact_RegisteredFlag(contact));
    }
}

#endif // IS_WAYFARER

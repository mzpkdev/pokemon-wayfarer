#include "global.h"
#include "notable_phone.h"

#if IS_WAYFARER
#include "league_events.h"
#include "league_selection.h"
#include "notable_friendship.h"
#include "notable_talk.h"
#include "notable_trainers.h"
#include "region_map.h"
#include "string_util.h"
#include "wayfarer_world.h"
#include "wayfarer_world_data.h"
#include "wayfarer_world_sim.h"
#include "constants/characters.h"
#include "constants/maps.h"
#include "constants/notable_trainers.h"
#include "constants/wayfarer_world.h"

// Where a contact says they are (notable-trainers.md#phone-contacts). Shared, neutral lines that only describe
// the place, so every trainer can say them; personality stays in the greeting and farewell. Placeholders for
// review. {STR_VAR_2} is the place, at most 16 characters (WayfarerGetPlaceNameOfMap).
static const u8 sWhere_Unknown[] = _("I'm out and about right now.");
static const u8 sWhere_League[] = _("I'm at the POKéMON LEAGUE,\ngetting ready for you!");
static const u8 sWhere_Partner[] = _("I'm right here with you,\nremember?");
static const u8 sWhere_OwnGym[] = _("I'm at my GYM in\n{STR_VAR_2} right now.");
static const u8 sWhere_LeagueHall[] = _("I'm at the POKéMON LEAGUE in\n{STR_VAR_2}.");
static const u8 sWhere_Travelling[] = _("I'm on the road right now,\npassing through {STR_VAR_2}.");
static const u8 sWhere_Around[] = _("I'm around {STR_VAR_2}\nright now.");

static const u8 *const sWhereLines[NOTABLE_WHERE_COUNT] =
{
    [NOTABLE_WHERE_UNKNOWN] = sWhere_Unknown,
    [NOTABLE_WHERE_LEAGUE] = sWhere_League,
    [NOTABLE_WHERE_PARTNER] = sWhere_Partner,
    [NOTABLE_WHERE_OWN_GYM] = sWhere_OwnGym,
    [NOTABLE_WHERE_LEAGUE_HALL] = sWhere_LeagueHall,
    [NOTABLE_WHERE_TRAVELLING] = sWhere_Travelling,
    [NOTABLE_WHERE_DWELLING] = sWhere_Around,
    [NOTABLE_WHERE_AROUND] = sWhere_Around,
};

static const u8 sText_NextBox[] = _("\p");

// The trainers with no world record (a face-only sprite: notable-world-simulation.md#known-limitation-walking-sprites)
// stand at a fixed place: a Hoenn Gym Leader in their Gym, an Elite Four member at their league.
struct NotableFixedPlace
{
    u8 characterId;
    u8 where;  // NOTABLE_WHERE_OWN_GYM or NOTABLE_WHERE_LEAGUE_HALL
    u16 map;   // MAP_*: the town or city, so the place reads as its name
};

static const struct NotableFixedPlace sFixedPlaces[] =
{
    {NOTABLE_TRAINER_ROXANNE, NOTABLE_WHERE_OWN_GYM, MAP_RUSTBORO_CITY},
    {NOTABLE_TRAINER_BRAWLY, NOTABLE_WHERE_OWN_GYM, MAP_DEWFORD_TOWN},
    {NOTABLE_TRAINER_WATTSON, NOTABLE_WHERE_OWN_GYM, MAP_MAUVILLE_CITY},
    {NOTABLE_TRAINER_FLANNERY, NOTABLE_WHERE_OWN_GYM, MAP_LAVARIDGE_TOWN},
    {NOTABLE_TRAINER_WINONA, NOTABLE_WHERE_OWN_GYM, MAP_FORTREE_CITY},
    {NOTABLE_TRAINER_SIDNEY, NOTABLE_WHERE_LEAGUE_HALL, MAP_EVER_GRANDE_CITY},
    {NOTABLE_TRAINER_PHOEBE, NOTABLE_WHERE_LEAGUE_HALL, MAP_EVER_GRANDE_CITY},
    {NOTABLE_TRAINER_GLACIA, NOTABLE_WHERE_LEAGUE_HALL, MAP_EVER_GRANDE_CITY},
    {NOTABLE_TRAINER_DRAKE, NOTABLE_WHERE_LEAGUE_HALL, MAP_EVER_GRANDE_CITY},
    {NOTABLE_TRAINER_AGATHA, NOTABLE_WHERE_LEAGUE_HALL, MAP_INDIGO_PLATEAU_HNS},
    {NOTABLE_TRAINER_BRUNO, NOTABLE_WHERE_LEAGUE_HALL, MAP_INDIGO_PLATEAU_HNS},
    {NOTABLE_TRAINER_KOGA, NOTABLE_WHERE_LEAGUE_HALL, MAP_INDIGO_PLATEAU_HNS},
};

static const struct NotableFixedPlace *GetFixedPlace(u8 characterId)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sFixedPlaces); i++)
    {
        if (sFixedPlaces[i].characterId == characterId)
            return &sFixedPlaces[i];
    }
    return NULL;
}

bool8 NotablePhone_IsContact(u8 characterId)
{
    return GetNotableTrainerById(characterId) != NULL && characterId != NOTABLE_TRAINER_TATE_LIZA
        && NotableFriendship_GetStage(characterId) >= NOTABLE_STAGE_FRIEND;
}

u8 NotablePhone_CountContacts(void)
{
    u8 characterId, count = 0;

    for (characterId = 1; characterId <= NOTABLE_TRAINER_COUNT; characterId++)
    {
        if (NotablePhone_IsContact(characterId))
            count++;
    }
    return count;
}

u16 NotablePhone_GetTrainerId(u8 characterId)
{
    const struct LeagueTrainer *trainer = GetLeagueTrainer(characterId);

    return trainer != NULL ? trainer->sourceTrainerId : 0;
}

static bool8 InAcceptedLineup(u8 characterId)
{
    const struct LeagueSavedTeam *team;
    u8 match;

    for (match = 0; match < LEAGUE_EVENT_LINEUP_SIZE; match++)
    {
        if (GetAcceptedLeagueEventMember(match, &team) && team->characterId == characterId)
            return TRUE;
    }
    return FALSE;
}

static void SetMap(struct NotablePhoneLocation *location, u8 where, u16 map)
{
    location->where = where;
    location->mapGroup = map >> 8;
    location->mapNum = map & 0xFF;
}

static void SetNode(struct NotablePhoneLocation *location, u8 where, u16 node)
{
    u16 map = WorldSim_NodeMap(node);

    if (map == 0xFFFF)
        location->where = NOTABLE_WHERE_UNKNOWN;
    else
        SetMap(location, where, map);
}

// The record as it stands. A heartbeat still pending after a map change hasn't moved anyone yet, so the record
// still says where they are; the call only reads it and never finishes or writes anything.
static void LocateSimulated(u8 slot, struct NotablePhoneLocation *location)
{
    const struct WayfarerWorldRecord *record = &WayfarerWorld_GetState()->records[slot];
    const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[slot];
    bool8 leader = (trainer->flags & WORLD_TRAINER_FLAG_GYM_LEADER) != 0;

    switch (record->state)
    {
    case WORLD_STATE_AWAY_PARTNER:
        location->where = NOTABLE_WHERE_PARTNER;
        break;
    case WORLD_STATE_HOME_LOCKED:
        SetNode(location, NOTABLE_WHERE_OWN_GYM, trainer->gymNode);
        break;
    case WORLD_STATE_AWAY_LEAGUE:
        // The event is over (InAcceptedLineup said no) and they reappear at home at the next heartbeat.
        SetMap(location, NOTABLE_WHERE_AROUND, trainer->homeMap);
        break;
    case WORLD_STATE_TRAVELLING:
        SetNode(location, NOTABLE_WHERE_TRAVELLING, record->node);
        break;
    case WORLD_STATE_DWELLING:
        if (leader && record->destKind == WORLD_DEST_HOME && record->node == trainer->gymNode)
        {
            SetNode(location, NOTABLE_WHERE_OWN_GYM, record->node);
        }
        else
        {
            SetNode(location, NOTABLE_WHERE_DWELLING, record->node);
            location->activity = NotableTalk_ActivityLineFor(record->destKind, record->destId, record->activity);
        }
        break;
    case WORLD_STATE_PINNED:
    default:
        SetNode(location, NOTABLE_WHERE_AROUND, record->node);
        break;
    }
}

void NotablePhone_GetLocation(u8 characterId, struct NotablePhoneLocation *location)
{
    const struct NotableFixedPlace *fixed;
    u8 slot;

    location->where = NOTABLE_WHERE_UNKNOWN;
    location->mapGroup = 0;
    location->mapNum = 0;
    location->activity = NOTABLE_LINE_SQUARE;
    if (GetNotableTrainerById(characterId) == NULL)
        return;
    // From acceptance until the event ends, live, for every trainer (the record follows at the next heartbeat).
    if (InAcceptedLineup(characterId))
    {
        location->where = NOTABLE_WHERE_LEAGUE;
        return;
    }
    slot = WorldSim_SlotForCharacter(characterId);
    if (slot < WORLD_SIM_TRAINER_COUNT)
    {
        LocateSimulated(slot, location);
        return;
    }
    fixed = GetFixedPlace(characterId);
    if (fixed != NULL)
        SetMap(location, fixed->where, fixed->map);
}

bool8 NotablePhone_GetHome(u8 characterId, u8 *mapGroup, u8 *mapNum)
{
    const struct NotableFixedPlace *fixed;
    u8 slot = WorldSim_SlotForCharacter(characterId);
    u16 map;

    if (slot < WORLD_SIM_TRAINER_COUNT)
    {
        map = gWayfarerWorldTrainers[slot].homeMap;
    }
    else
    {
        fixed = GetFixedPlace(characterId);
        if (fixed == NULL)
            return FALSE;
        map = fixed->map;
    }
    *mapGroup = map >> 8;
    *mapNum = map & 0xFF;
    return TRUE;
}

static const u8 *CallGreeting(u8 characterId)
{
    const struct NotableVoice *voice = &gNotableVoices[characterId - 1];

    return NotableFriendship_GetStage(characterId) >= NOTABLE_STAGE_CLOSE ? voice->callClose : voice->callHello;
}

void NotablePhone_BufferCall(u8 characterId, u8 *dest)
{
    struct NotablePhoneLocation location;
    const struct NotableVoice *voice;
    const u8 *where;
    u8 *end = dest;

    *dest = EOS;
    if (!NotablePhone_IsContact(characterId))
        return;
    voice = &gNotableVoices[characterId - 1];
    if (voice->callHello == NULL || voice->callClose == NULL || voice->callBye == NULL)
        return;

    NotablePhone_GetLocation(characterId, &location);
    gStringVar2[0] = EOS;
    if (location.where != NOTABLE_WHERE_UNKNOWN && location.where != NOTABLE_WHERE_LEAGUE
     && location.where != NOTABLE_WHERE_PARTNER)
    {
        WayfarerGetPlaceNameOfMap(gStringVar2, location.mapGroup, location.mapNum);
        if (gStringVar2[0] == EOS)
            location.where = NOTABLE_WHERE_UNKNOWN;
    }
    where = location.where < NOTABLE_WHERE_COUNT ? sWhereLines[location.where] : sWhere_Unknown;

    // {BUDDY} in a voice line is the buddy species, in gStringVar1.
    NotableTalk_BufferBuddy(characterId);
    end = StringExpandPlaceholders(end, CallGreeting(characterId));
    end = StringCopy(end, sText_NextBox);
    end = StringExpandPlaceholders(end, where);
    if (location.where == NOTABLE_WHERE_DWELLING)
    {
        end = StringCopy(end, sText_NextBox);
        end = StringCopy(end, NotableTalk_GetActivityLine(location.activity));
    }
    end = StringCopy(end, sText_NextBox);
    StringExpandPlaceholders(end, voice->callBye);
}

#endif // IS_WAYFARER

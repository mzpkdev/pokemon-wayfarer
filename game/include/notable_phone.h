#ifndef GUARD_NOTABLE_PHONE_H
#define GUARD_NOTABLE_PHONE_H

// Notable trainers as phone contacts (notable-trainers.md#phone-contacts).
//
// A notable trainer other than Tate & Liza is a contact while their friendship stage is Friend or above. Nothing
// registers them: the saved score is the fact, and friendship never decreases, so a contact stays one. They are
// listed in the phone's contact list beside the route contacts, never make a random call, and have no rematch,
// gift or check page. Calling one plays three parts: the phone greeting of their stage, where they are right now
// (read from their world record, never written), and the phone farewell.

#include "global.h"

#if IS_WAYFARER

// Where a contact says they are (notable-trainers.md#phone-contacts, where they are).
enum NotablePhoneWhere
{
    NOTABLE_WHERE_UNKNOWN,     // no place to name
    NOTABLE_WHERE_LEAGUE,      // in the accepted league event's lineup
    NOTABLE_WHERE_PARTNER,     // away as the Masters partner
    NOTABLE_WHERE_OWN_GYM,     // a Gym Leader in their own Gym
    NOTABLE_WHERE_LEAGUE_HALL, // an Elite Four member at their fixed spot in a league
    NOTABLE_WHERE_TRAVELLING,  // on the road, passing through a map
    NOTABLE_WHERE_DWELLING,    // at a spot, doing an activity
    NOTABLE_WHERE_AROUND,      // on a map, with no activity to tell (a haunt, or back from away)
    NOTABLE_WHERE_COUNT,
};

struct NotablePhoneLocation
{
    u8 where;     // enum NotablePhoneWhere
    u8 mapGroup;  // the map whose place is named (unused for UNKNOWN, LEAGUE and PARTNER)
    u8 mapNum;
    u8 activity;  // enum NotableActivityLine, for DWELLING
};

// Friend or above, and gives a number (not Tate & Liza).
bool8 NotablePhone_IsContact(u8 characterId);
u8 NotablePhone_CountContacts(void);

// Where the contact is right now, from their world record (simulated trainers) or their fixed place (the
// face-only trainers), with an accepted league lineup first.
void NotablePhone_GetLocation(u8 characterId, struct NotablePhoneLocation *location);
// The place the contact list shows for them: their home base, or their fixed place.
bool8 NotablePhone_GetHome(u8 characterId, u8 *mapGroup, u8 *mapNum);
// The trainer whose class, name and picture the list shows (the character's league source encounter).
u16 NotablePhone_GetTrainerId(u8 characterId);

// The whole call, placeholders expanded, into dest (gStringVar4-sized). Uses gStringVar1 and gStringVar2.
void NotablePhone_BufferCall(u8 characterId, u8 *dest);

#endif // IS_WAYFARER
#endif // GUARD_NOTABLE_PHONE_H

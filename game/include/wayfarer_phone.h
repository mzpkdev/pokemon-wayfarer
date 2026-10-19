#ifndef GUARD_WAYFARER_PHONE_H
#define GUARD_WAYFARER_PHONE_H

// Phone rematches (Wayfarer only): the registered flag of every contact, how a registered contact's next team and an
// unregistered one's team are chosen, the region step that makes a contact ready, and the number offer after a win.
// Spec: .product/specs/phone-rematches.md.
//
// A contact is a number in the index space of wayfarer_daily_slots.h: the normal rematch table rows (HNS, Hoenn and
// the appended Hoenn rows), then the Sevii families that carry a stronger team, then the Kanto coast families.

#include "global.h"
#include "constants/flags.h"
#include "constants/rematches.h"

struct ScriptContext;

#if IS_WAYFARER

enum PhoneRegion
{
    PHONE_REGION_OTHER,
    PHONE_REGION_KANTO,
    PHONE_REGION_SEVII,
    PHONE_REGION_JOHTO,
    PHONE_REGION_HOENN,
};

// Trainer Rating at which a rematch team unlocks for a contact without a registered number: team 2 to 5.
#define PHONE_RATING_TIERS 4
extern const u8 gPhoneRatingTierRatings[PHONE_RATING_TIERS];

// The one place that maps a contact to its registered flag (0 for a number that is not a contact).
u16 PhoneContact_RegisteredFlag(u32 contact);
bool8 PhoneContact_IsContact(u8 contact);
bool8 PhoneContact_IsRegistered(u8 contact);
bool8 PhoneContact_IsReady(u8 contact);
void PhoneContact_SetReady(u8 contact);

// Team tiers are indexes into the contact's authored teams (0 is the team met first).
u8 PhoneContact_FoughtTier(u8 contact);
u8 PhoneContact_RatingTier(u8 contact);
u16 PhoneContact_LatestTeam(u8 contact);
u16 PhoneContact_NextTeam(u8 contact);
u16 PhoneContact_RatingTeam(u8 contact);

enum PhoneRegion PhoneRegion_OfMap(u8 mapGroup, u8 mapNum);
enum PhoneRegion PhoneContact_Region(u8 contact);
enum PhoneRegion PhoneContact_HomeRegion(u8 contact);  // the same, without a home lookup (the step event)
bool8 PhoneContact_IsInHomeSection(u8 contact);
void PhoneReadiness_OnRegionSteps(void);

// The number offer after a win.
bool8 PhoneOffer_IsDue(u16 trainerId);
bool8 PhoneOffer_IsDueOnTalk(u16 trainerId);
const u8 *PhoneOffer_BeginTalkOffer(const u8 *resume);
void PhoneOffer_Resume(struct ScriptContext *ctx);
void PhoneOffer_StartInteraction(void);
u32 PhoneOffer_CanOffer(void);
void PhoneOffer_BufferTarget(void);
void PhoneOffer_Accept(void);
extern const u8 PhoneOffer_EventScript_AfterBattle[];
extern const u8 PhoneOffer_EventScript_TalkAgain[];

#else

static inline u16 PhoneContact_RegisteredFlag(u32 contact)
{
    return TRAINER_REGISTERED_FLAGS_START + contact;
}

#endif // IS_WAYFARER

#endif // GUARD_WAYFARER_PHONE_H

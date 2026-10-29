// Phone rematches (Wayfarer only). Spec: .product/specs/phone-rematches.md (Rematch table and save state, Getting a
// number, Readiness and teams).
//
// What lives here is the part of the phone that is not text: which flag registers a contact, which team a contact
// brings, which contact a region's steps make ready, and whether a win leads to a number offer. The calls themselves
// are in match_call.c, the contact index space and the day-start snapshots in wayfarer_daily_trainers.c.

#include "global.h"
#include "wayfarer_phone.h"

#if IS_WAYFARER
#include "battle.h"
#include "battle_pyramid.h"
#include "battle_setup.h"
#include "trainer_hill.h"
#include "event_data.h"
#include "match_call.h"
#include "overworld.h"
#include "regions.h"
#include "script.h"
#include "trainer_see.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "wayfarer_coast_trainers.h"
#include "wayfarer_daily_slots.h"
#include "wayfarer_sevii_rematches.h"
#include "constants/flags.h"
#include "constants/game_stat.h"
#include "constants/opponents.h"

// The Sevii and coast contacts own one Sevii bank flag each, in contact order.
STATIC_ASSERT(DAILY_CONTACT_LAST - DAILY_CONTACT_SEVII_FIRST + 1 == WAYFARER_SEVII_PHONE_FLAG_COUNT, PhoneSeviiFlagCount);
STATIC_ASSERT(REMATCH_TABLE_ENTRIES - REMATCH_APPENDED_START == TRAINER_REGISTERED_APPENDED_FLAGS_COUNT, PhoneAppendedFlagCount);
// Registered flags of the 0x310 block must stay below the decoration flags that follow it.
STATIC_ASSERT(TRAINER_REGISTERED_FLAGS_START + REMATCH_APPENDED_START - 1 < HNS_EXTENDED_CONTENT_START, PhoneRegisteredBlockEnd);

const u8 gPhoneRatingTierRatings[PHONE_RATING_TIERS] = {40, 80, 120, 160};

// ---------- registration ----------
u16 PhoneContact_RegisteredFlag(u32 contact)
{
    if (contact < REMATCH_APPENDED_START)
        return TRAINER_REGISTERED_FLAGS_START + contact;
    if (contact < REMATCH_TABLE_ENTRIES)
        return TRAINER_REGISTERED_APPENDED_FLAGS_START + (contact - REMATCH_APPENDED_START);
    if (contact >= DAILY_CONTACT_SEVII_FIRST && contact <= DAILY_CONTACT_LAST)
        return FLAG_WAYFARER_SEVII_PHONE_FIRST + (contact - DAILY_CONTACT_SEVII_FIRST);
    return 0;
}

bool8 PhoneContact_IsContact(u8 contact)
{
    return contact < DAILY_CONTACT_COUNT && DailyContacts_FirstTrainer(contact) != TRAINER_NONE;
}

bool8 PhoneContact_IsRegistered(u8 contact)
{
    return PhoneContact_IsContact(contact) && FlagGet(PhoneContact_RegisteredFlag(contact));
}

// ---------- readiness ----------
bool8 PhoneContact_IsReady(u8 contact)
{
    u16 first = DailyContacts_FirstTrainer(contact);

    if (first == TRAINER_NONE)
        return FALSE;
    if (contact >= DAILY_CONTACT_COAST_FIRST)
        return WayfarerCoastRematchFamilyIsReady(contact - DAILY_CONTACT_COAST_FIRST);
    if (contact >= DAILY_CONTACT_SEVII_FIRST)
        return WayfarerSeviiRematchIsReady(first);
    return gSaveBlock1Ptr->trainerRematches[contact] != 0;
}

void PhoneContact_SetReady(u8 contact)
{
    u16 first = DailyContacts_FirstTrainer(contact);

    if (first == TRAINER_NONE)
        return;
    if (contact >= DAILY_CONTACT_COAST_FIRST)
        WayfarerCoastRematchSetReady(first);
    else if (contact >= DAILY_CONTACT_SEVII_FIRST)
        WayfarerSeviiRematchSetReady(first);
    else
        UpdateRematchIfDefeated(contact);
}

// ---------- teams ----------
static u8 TeamIndex(const u16 *ids, u8 count, u16 id)
{
    u8 i;

    for (i = 0; i < count; i++)
    {
        if (ids[i] == id)
            return i;
    }
    return 0;
}

// The highest team of the contact the player has fought. A rematch table row's teams each have a defeat flag, and a win
// at a team the rating chose counts. A Sevii or coast family's teams share the defeat flag of its first team, so those
// count the rematches the family's registry has recorded instead.
u8 PhoneContact_FoughtTier(u8 contact)
{
    u8 count, tier = 0, i;
    const u16 *ids = DailyContacts_Teams(contact, &count);

    if (ids == NULL)
        return 0;
    if (contact >= DAILY_CONTACT_SEVII_FIRST)
    {
        u16 first = DailyContacts_FirstTrainer(contact);
        u16 latest = contact >= DAILY_CONTACT_COAST_FIRST ? WayfarerCoastRematchGetLatest(first) : WayfarerSeviiRematchGetLatest(first);

        return TeamIndex(ids, count, latest);
    }
    for (i = count - 1; i > 0; i--)
    {
        if (HasTrainerBeenFought(ids[i]))
        {
            tier = i;
            break;
        }
    }
    return tier;
}

// The highest team the player's Trainer Rating unlocks, counted in the contact's authored order.
u8 PhoneContact_RatingTier(u8 contact)
{
    u8 count, tier = 0, i;
    u32 rating;

    if (DailyContacts_Teams(contact, &count) == NULL)
        return 0;
    rating = GetTrainerRating();
    for (i = 0; i < PHONE_RATING_TIERS; i++)
    {
        if (rating >= gPhoneRatingTierRatings[i])
            tier++;
    }
    return tier < count ? tier : count - 1;
}

// The team a wanderer with a registered number brings when it is not ready: the latest it has fought, or the team the
// rating unlocks if that is further on, so registering never lowers its everyday team.
u16 PhoneContact_LatestTeam(u8 contact)
{
    u8 count, tier;
    const u16 *ids = DailyContacts_Teams(contact, &count);

    if (ids == NULL)
        return TRAINER_NONE;
    tier = PhoneContact_FoughtTier(contact);
    return ids[max(tier, PhoneContact_RatingTier(contact))];
}

// A ready contact's next battle: the team after the higher of the highest fought and the highest the rating unlocks.
// After the last team, the contact keeps it.
u16 PhoneContact_NextTeam(u8 contact)
{
    u8 count, tier;
    const u16 *ids = DailyContacts_Teams(contact, &count);

    if (ids == NULL)
        return TRAINER_NONE;
    tier = PhoneContact_FoughtTier(contact);
    if (PhoneContact_RatingTier(contact) > tier)
        tier = PhoneContact_RatingTier(contact);
    return ids[tier + 1 < count ? tier + 1 : count - 1];
}

// The team of a contact without a registered number: whatever the rating unlocks right now. No save state.
u16 PhoneContact_RatingTeam(u8 contact)
{
    u8 count;
    const u16 *ids = DailyContacts_Teams(contact, &count);

    return ids == NULL ? TRAINER_NONE : ids[PhoneContact_RatingTier(contact)];
}

// ---------- regions ----------
// The region a map is in. Hoenn's map headers keep Emerald's numeric section ids, which mean something else to the HNS
// section tables, so Hoenn is told by the map's source; the Sevii sections follow the Johto ones and are told by their
// own lists before the section ranges decide Kanto, Johto and the rest.
enum PhoneRegion PhoneRegion_OfMap(u8 mapGroup, u8 mapNum)
{
    u32 mapSec;

    if (WayfarerIsMapHoennSource(mapGroup, mapNum))
        return PHONE_REGION_HOENN;
    mapSec = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum)->regionMapSectionId;
    if (GetKantoSubregion(mapSec) != KANTO_SUBREGION_KANTO)
        return PHONE_REGION_SEVII;
    switch (GetRegionForSectionId(mapSec))
    {
    case REGION_KANTO:
        return PHONE_REGION_KANTO;
    case REGION_JOHTO:
        return PHONE_REGION_JOHTO;
    default:
        return PHONE_REGION_OTHER;
    }
}

// The region of a contact's home map.
enum PhoneRegion PhoneContact_Region(u8 contact)
{
    u8 mapGroup, mapNum;

    if (!DailyContacts_GetHome(contact, &mapGroup, &mapNum))
        return PHONE_REGION_OTHER;
    return PhoneRegion_OfMap(mapGroup, mapNum);
}

// The same region without a home lookup, for the step event: a table row's map is the contact's home, a Sevii family lives
// on Sevii and a coast family on the Kanto coast. A test holds it equal to PhoneContact_Region for every contact.
enum PhoneRegion PhoneContact_HomeRegion(u8 contact)
{
    if (contact < DAILY_CONTACT_SEVII_FIRST)
        return PhoneRegion_OfMap(gRematchTable[contact].mapGroup, gRematchTable[contact].mapNum);
    if (contact < DAILY_CONTACT_COAST_FIRST)
        return PHONE_REGION_SEVII;
    return PHONE_REGION_KANTO;
}

// Whether the player stands in the map section a contact calls home. Hoenn's section ids are Emerald's, so they only mean
// the same section to a player who is in Hoenn too.
bool8 PhoneContact_IsInHomeSection(u8 contact)
{
    u8 mapGroup, mapNum;

    return DailyContacts_GetHome(contact, &mapGroup, &mapNum)
        && WayfarerIsMapHoennSource(mapGroup, mapNum) == WayfarerIsCurrentMapHoennSource()
        && Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum)->regionMapSectionId == gMapHeader.regionMapSectionId;
}

// A registered contact the player has met, who isn't ready yet, in the region. The registered flag and the region go
// first: late in the game most contacts have a number, and the region (no home lookup) leaves the first-trainer and gift
// lookups to the contacts of one region.
static bool8 IsReadinessCandidate(u8 contact, enum PhoneRegion region)
{
    u16 first;

    if (contact < REMATCH_TABLE_ENTRIES && !REMATCH_IS_NORMAL(contact))
        return FALSE;
    if (!FlagGet(PhoneContact_RegisteredFlag(contact)))
        return FALSE;
    if (PhoneContact_HomeRegion(contact) != region)
        return FALSE;
    first = DailyContacts_FirstTrainer(contact);
    return first != TRAINER_NONE
        && HasTrainerBeenFought(first)
        && !PhoneContact_IsReady(contact)
        && !MatchCall_HnsTrainerHasGift(first);  // a gift is not rolled again
}

// Every 255 steps (match_call.c): one registered, already-fought contact of the region the player is in who isn't ready
// becomes ready, picked by the day's roll. A gift contact may find a gift instead (the roll keeps HNS's 1 in 5). There
// is no call now: the contact is placed, and calls, from the next day.
void PhoneReadiness_OnRegionSteps(void)
{
    enum PhoneRegion region = PhoneRegion_OfMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    u32 steps = GetGameStat(GAME_STAT_STEPS);
    u32 count = 0;
    u8 candidates[DAILY_CONTACT_LAST + 1];
    u8 contact;

    if (region == PHONE_REGION_OTHER)
        return;
    // One pass, in contact order, so the pick is the same as counting first and walking again.
    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
    {
        if (IsReadinessCandidate(contact, region))
            candidates[count++] = contact;
    }
    if (count == 0)
        return;
    contact = candidates[DailySlots_Roll(DAILY_ROLL_PHONE_PICK, steps, 0) % count];
    if (!MatchCall_HnsRollGift(DailyContacts_FirstTrainer(contact), DailySlots_Roll(DAILY_ROLL_PHONE_GIFT, steps, contact),
                               DailySlots_Roll(DAILY_ROLL_PHONE_ITEM, steps, contact)))
        PhoneContact_SetReady(contact);
}

// ---------- the number offer ----------
// Where the trainer's own script goes on after a talk-again offer, and the contact whose number was just offered in this
// interaction (so the script's own offer does not ask a second time).
static EWRAM_DATA const u8 *sTalkOfferResume = NULL;
static EWRAM_DATA u8 sOfferedContactPlusOne = 0;

void PhoneOffer_StartInteraction(void)
{
    sOfferedContactPlusOne = 0;
}

static bool8 PhoneOffer_IsDueIn(u16 trainerId, bool8 afterBattle);

// A contact with no daily slot never gets a later win, so talking to it again is how it asks again.
bool8 PhoneOffer_IsDueOnTalk(u16 trainerId)
{
    u8 contact = DailyContacts_IndexOf(trainerId);

    // No battle has run in this interaction, so the last battle's type says nothing about this trainer.
    return PhoneOffer_IsDueIn(trainerId, FALSE) && DailyContacts_IsFixed(contact);
}

const u8 *PhoneOffer_BeginTalkOffer(const u8 *resume)
{
    sTalkOfferResume = resume;
    gWhichTrainerToFaceAfterBattle = 0;
    return PhoneOffer_EventScript_TalkAgain;
}

// The end of the talk-again offer: carry on with the rest of the trainer's script.
void PhoneOffer_Resume(struct ScriptContext *ctx)
{
    sOfferedContactPlusOne = DailyContacts_IndexOf(TRAINER_BATTLE_PARAM.opponentA) + 1;
    if (sTalkOfferResume != NULL)
        ctx->scriptPtr = sTalkOfferResume;
    sTalkOfferResume = NULL;
}

// A contact without a registered number, with the Pokégear in hand. After a battle, not in a facility: the Battle Pyramid,
// Trainer Hill (Hoenn's sets its own copy of the challenge var, so the battle type is checked too) and the Frontier battle
// with indexes that are not trainer ids, which can equal a contact's.
static bool8 PhoneOffer_IsDueIn(u16 trainerId, bool8 afterBattle)
{
    u8 contact;

    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE || InTrainerHillChallenge())
        return FALSE;
    if (afterBattle && (gBattleTypeFlags & (BATTLE_TYPE_FRONTIER | BATTLE_TYPE_TRAINER_HILL)))
        return FALSE;
    contact = DailyContacts_IndexOf(trainerId);
    return contact != DAILY_NO_CONTACT && FlagGet(FLAG_HAS_MATCH_CALL) && !PhoneContact_IsRegistered(contact);
}

// After a win against a contact without a registered number.
bool8 PhoneOffer_IsDue(u16 trainerId)
{
    return PhoneOffer_IsDueIn(trainerId, TRUE);
}

// For the offer script: VAR_0x8000 holds the trainer (any team) of the contact offering its number. Every way into the
// script (the after-battle choice, talking again, a trainer's own script) has already ruled out facility battles, and a
// talk has no battle of its own, so the last battle's type is not checked here.
u32 PhoneOffer_CanOffer(void)
{
    return PhoneOffer_IsDueIn(gSpecialVar_0x8000, FALSE)
        && DailyContacts_IndexOf(gSpecialVar_0x8000) + 1 != sOfferedContactPlusOne;
}

// For the shared after-battle offer: the contact of the trainer just fought, by its first team.
void PhoneOffer_BufferTarget(void)
{
    u8 contact = DailyContacts_IndexOf(gWhichTrainerToFaceAfterBattle == 1 ? TRAINER_BATTLE_PARAM.opponentB : TRAINER_BATTLE_PARAM.opponentA);

    gSpecialVar_0x8000 = contact == DAILY_NO_CONTACT ? TRAINER_NONE : DailyContacts_FirstTrainer(contact);
}

void PhoneOffer_Accept(void)
{
    u8 contact = DailyContacts_IndexOf(gSpecialVar_0x8000);

    if (contact != DAILY_NO_CONTACT)
        FlagSet(PhoneContact_RegisteredFlag(contact));
}

#endif // IS_WAYFARER

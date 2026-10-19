#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "constants/battle_setup.h"
#include "event_data.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "malloc.h"
#include "match_call.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "pokenav.h"
#include "region_map.h"
#include "task.h"
#include "script.h"
#include "string_util.h"
#include "strings.h"
#include "trainer_rating.h"
#include "wayfarer_coast_trainers.h"
#include "wayfarer_daily_slots.h"
#include "wayfarer_phone.h"
#include "wayfarer_sevii_rematches.h"
#include "wayfarer_sevii_state.h"
#include "test/test.h"
#include "constants/characters.h"
#include "constants/flags.h"
#include "constants/game_stat.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/opponents.h"
#include "constants/region_map_sections.h"
#include "constants/vars.h"
#include "constants/wayfarer_persistence.h"

#if IS_WAYFARER

// Phone rematches (.product/specs/phone-rematches.md): the contact index space, registration flags, team tiers,
// readiness by region, call text selection and the number offer.

#define TEST_SEED   0x5EEDF00D

// Specials the scripts call, defined in field_specials.c and pokenav_match_call_data.c.
bool32 IsTrainerRegistered(void);
void SetMatchCallRegisteredFlag(void);

// The Hoenn, Sevii and coast call texts live in the data files; a test names the ones it compares against.
extern const u8 MatchCall_HOENN_Rose_Battle[];
extern const u8 MatchCall_HOENN_Lao_Battle[];
extern const u8 MatchCall_HOENN_Lao_General1[];
extern const u8 MatchCall_HOENN_Lao_General2[];
extern const u8 MatchCall_HOENN_Lao_General3[];
extern const u8 MatchCall_SEVII_Finn_Battle[];
extern const u8 MatchCall_COAST_Alice_Battle[];

struct Snapshot
{
    struct MapHeader header;
    struct WarpData location;
    struct WayfarerDailySlots daily;
    u8 flags[sizeof(gSaveBlock1Ptr->flags)];
    u8 rematches[MAX_REMATCH_ENTRIES];
    u16 vars[VARS_COUNT];
    u32 debugDay, debugSeed;
    u8 debugFlags;
    u8 selected;
    struct ObjectEvent object;
    struct SaveBlock3 sb3;
};

static struct Snapshot *TakeSnapshot(void)
{
    struct Snapshot *snapshot = Alloc(sizeof(*snapshot));

    ASSUME(snapshot != NULL);
    snapshot->header = gMapHeader;
    snapshot->location = gSaveBlock1Ptr->location;
    snapshot->daily = gPokemonStoragePtr->dailySlots;
    memcpy(snapshot->flags, gSaveBlock1Ptr->flags, sizeof(snapshot->flags));
    memcpy(snapshot->rematches, gSaveBlock1Ptr->trainerRematches, sizeof(snapshot->rematches));
    memcpy(snapshot->vars, gSaveBlock1Ptr->vars, sizeof(snapshot->vars));
    snapshot->debugDay = gDailySlotsDebugDay;
    snapshot->debugSeed = gDailySlotsDebugSeed;
    snapshot->debugFlags = gDailySlotsDebugFlags;
    snapshot->selected = gSelectedObjectEvent;
    snapshot->object = gObjectEvents[0];
    snapshot->sb3 = *gSaveBlock3Ptr;
    return snapshot;
}

static void RestoreSnapshot(struct Snapshot *snapshot)
{
    gMapHeader = snapshot->header;
    gSaveBlock1Ptr->location = snapshot->location;
    gPokemonStoragePtr->dailySlots = snapshot->daily;
    memcpy(gSaveBlock1Ptr->flags, snapshot->flags, sizeof(snapshot->flags));
    memcpy(gSaveBlock1Ptr->trainerRematches, snapshot->rematches, sizeof(snapshot->rematches));
    memcpy(gSaveBlock1Ptr->vars, snapshot->vars, sizeof(snapshot->vars));
    gDailySlotsDebugDay = snapshot->debugDay;
    gDailySlotsDebugSeed = snapshot->debugSeed;
    gDailySlotsDebugFlags = snapshot->debugFlags;
    gSelectedObjectEvent = snapshot->selected;
    gObjectEvents[0] = snapshot->object;
    *gSaveBlock3Ptr = snapshot->sb3;
    Free(snapshot);
}

static void StartDay(u32 day)
{
    gDailySlotsDebugFlags = DAILY_DEBUG_DAY | DAILY_DEBUG_SEED | DAILY_DEBUG_NO_EMPTY;
    gDailySlotsDebugDay = day;
    gDailySlotsDebugSeed = TEST_SEED;
    DailySlots_OnMapLoad();
}

static void EnterMap(u8 mapGroup, u8 mapNum)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    LoadObjEventTemplatesFromHeader();
}

static void ClearPhoneState(void)
{
    u8 contact;

    // {PLAYER} expands the name in the save, which a test starts without.
    gSaveBlock2Ptr->playerName[0] = CHAR_A;
    gSaveBlock2Ptr->playerName[1] = EOS;

    memset(gSaveBlock1Ptr->trainerRematches, 0, sizeof(gSaveBlock1Ptr->trainerRematches));
    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
    {
        if (PhoneContact_IsContact(contact))
            FlagClear(PhoneContact_RegisteredFlag(contact));
    }
    WayfarerSeviiRematchClearAllReady();
    WayfarerCoastRematchClearAllReady();
    DailySlots_InitNewGame();
}

static u8 ContactOf(u16 trainerId)
{
    u8 contact = DailyContacts_IndexOf(trainerId);

    ASSUME(contact != DAILY_NO_CONTACT);
    return contact;
}

// ---------- the contact index space and the registered flags ----------
TEST("Phone: the contacts are the normal rematch rows, the Sevii families that carry a stronger team and the coast")
{
    u8 contact;
    u32 table = 0, sevii = 0, coast = 0;

    EXPECT_EQ(REMATCH_TABLE_ENTRIES, MAX_REMATCH_ENTRIES);
    EXPECT_EQ(REMATCH_APPENDED_START, 90);
    EXPECT_EQ(REMATCH_TABLE_ENTRIES, 118);
    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
    {
        if (!PhoneContact_IsContact(contact))
            continue;
        if (contact < REMATCH_TABLE_ENTRIES)
            table++;
        else if (contact < DAILY_CONTACT_COAST_FIRST)
            sevii++;
        else
            coast++;
        // Every contact is found again by its first trainer and has every text a call can ask for.
        EXPECT_EQ(DailyContacts_IndexOf(DailyContacts_FirstTrainer(contact)), contact);
        EXPECT(MatchCall_ContactHasTexts(contact));
        // Every contact stands somewhere (a daily slot or a fixed home), so a call can name its place.
        {
            u8 homeGroup, homeNum, homeLocal;

            EXPECT(DailyTrainers_WhereIsTrainerToday(DailyContacts_FirstTrainer(contact), &homeGroup, &homeNum, &homeLocal));
        }
    }
    // HNS's 39 (Nicole is compiled out) and Hoenn's 64; Sevii's 32 and the coast's 9.
    EXPECT_EQ(table, 39 + 64);
    EXPECT_EQ(sevii, 32);
    EXPECT_EQ(coast, 9);
    // Nicole's row is empty in Wayfarer: not a contact, never offered or registered.
    EXPECT(!PhoneContact_IsContact(REMATCH_NICOLE_HNS));
    EXPECT_EQ(DailyContacts_IndexOf(TRAINER_NICOLE_HNS), DAILY_NO_CONTACT);
    FlagSet(FLAG_HAS_MATCH_CALL);
    EXPECT(!PhoneOffer_IsDue(TRAINER_NICOLE_HNS));
    EXPECT(!PhoneOffer_IsDueOnTalk(TRAINER_NICOLE_HNS));
    // Special rows (Wally, gym leaders, Elite Four) are not contacts.
    EXPECT(!PhoneContact_IsContact(REMATCH_WALLY_VR));
    EXPECT(!PhoneContact_IsContact(REMATCH_ROXANNE));
    EXPECT(!PhoneContact_IsContact(REMATCH_WALLACE));
}

TEST("Phone: Hoenn takes the empty indexes 28-63 and appends the rest, and HNS keeps its indexes")
{
    // HNS rows keep their index and meaning.
    EXPECT_EQ(gRematchTable[REMATCH_ROSE].trainerIds[0], TRAINER_JOEY_HNS);
    EXPECT_EQ(gRematchTable[REMATCH_ANDRES].trainerIds[0], TRAINER_WADE_HNS);
    EXPECT_EQ(gRematchTable[REMATCH_KAREN].trainerIds[0], TRAINER_REENA_HNS);
    EXPECT_EQ(gRematchTable[REMATCH_ALEX_HNS].trainerIds[0], TRAINER_ALEX_HNS);
    EXPECT_EQ(gRematchTable[REMATCH_CHARLES_HNS].trainerIds[0], TRAINER_CHARLES_HNS);
    // Hoenn's rows: 36 in the gap, 28 past the Elite Four rows.
    EXPECT_EQ(gRematchTable[28].trainerIds[0], TRAINER_JERRY_1);
    EXPECT_EQ(gRematchTable[63].trainerIds[0], TRAINER_KIRA_AND_DAN_1);
    EXPECT_EQ(gRematchTable[90].trainerIds[0], TRAINER_ROSE_1);
    EXPECT_EQ(gRematchTable[117].trainerIds[0], TRAINER_KAREN_1);
    EXPECT_EQ(REMATCH_ROSE_HOENN, REMATCH_APPENDED_START);
    EXPECT_EQ(REMATCH_KAREN_HOENN, REMATCH_TABLE_ENTRIES - 1);
    // The special rows and the Elite Four keep their constants.
    EXPECT_EQ(REMATCH_WALLY_VR, 76);
    EXPECT_EQ(REMATCH_SIDNEY, 85);
    EXPECT_EQ(gRematchTable[REMATCH_SIDNEY].trainerIds[0], 0);
    // A trainer is in one row only, so no id is reused between HNS and Hoenn.
    {
        u32 i, j;

        for (i = 0; i < REMATCH_TABLE_ENTRIES; i++)
        {
            if (gRematchTable[i].trainerIds[0] == 0)
                continue;
            for (j = i + 1; j < REMATCH_TABLE_ENTRIES; j++)
                EXPECT(gRematchTable[i].trainerIds[0] != gRematchTable[j].trainerIds[0]);
        }
    }
    // Normal rows are the ones before Wally and the appended range.
    EXPECT(REMATCH_IS_NORMAL(0));
    EXPECT(REMATCH_IS_NORMAL(75));
    EXPECT(!REMATCH_IS_NORMAL(76));
    EXPECT(!REMATCH_IS_NORMAL(89));
    EXPECT(REMATCH_IS_NORMAL(90));
    EXPECT(REMATCH_IS_NORMAL(117));
    EXPECT(!REMATCH_IS_NORMAL(118));
    EXPECT(REMATCH_IS_ELITE_FOUR(85));
    EXPECT(!REMATCH_IS_ELITE_FOUR(90));
    EXPECT_EQ(REMATCH_NORMAL_ENTRIES, 76 + 28);
}

TEST("Phone: every contact has its own registered flag and no flag overlaps another")
{
    u8 contact;
    u32 i;
    u16 lowestAppended = 0xFFFF, highestAppended = 0, highestBlock = 0, lowestSevii = 0xFFFF, highestSevii = 0;

    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
    {
        u16 flag = PhoneContact_RegisteredFlag(contact);

        if (!PhoneContact_IsContact(contact))
            continue;
        EXPECT(flag != 0);
        for (i = contact + 1; i <= DAILY_CONTACT_LAST; i++)
        {
            if (PhoneContact_IsContact(i))
                EXPECT(PhoneContact_RegisteredFlag(i) != flag);
        }
        if (contact < REMATCH_APPENDED_START)
        {
            EXPECT_EQ(flag, TRAINER_REGISTERED_FLAGS_START + contact);
            highestBlock = max(highestBlock, flag);
        }
        else if (contact < REMATCH_TABLE_ENTRIES)
        {
            EXPECT_EQ(flag, 0x8BE + (contact - 90));
            lowestAppended = min(lowestAppended, flag);
            highestAppended = max(highestAppended, flag);
        }
        else
        {
            EXPECT(IS_WAYFARER_SEVII_FLAG_ID(flag));
            lowestSevii = min(lowestSevii, WAYFARER_SEVII_ID_INDEX(flag));
            highestSevii = max(highestSevii, WAYFARER_SEVII_ID_INDEX(flag));
        }
    }
    // The 0x310 block ends below the decoration flags.
    EXPECT_EQ(TRAINER_REGISTERED_FLAGS_START, 0x310);
    EXPECT(highestBlock < HNS_EXTENDED_CONTENT_START);
    EXPECT_EQ(HNS_EXTENDED_CONTENT_START, 0x36A);
    // The appended window is 0x8BE-0x8D9, between the Battle Frontier symbols and the Tower flags.
    EXPECT_EQ(lowestAppended, 0x8BE);
    EXPECT_EQ(highestAppended, 0x8D9);
    EXPECT(lowestAppended > FLAG_SYS_PYRAMID_GOLD);
    EXPECT(highestAppended < FLAG_WAYFARER_TOWER_MAROWAK_CALMED);
    // The 41 Sevii and coast flags sit past the 97 exploration pickups (slots 61-157) in the Sevii bank.
    EXPECT_EQ(lowestSevii, 158);
    EXPECT_EQ(highestSevii, 198);
    EXPECT(lowestSevii > WAYFARER_SEVII_ID_INDEX(FLAG_WAYFARER_SEVII_HIDDEN_SEVEN_ISLAND_TANOBY_RUINS_3));
    EXPECT(highestSevii < WAYFARER_SEVII_FLAG_COUNT);
}

TEST("Phone: registering an appended contact sets a flag in 0x8BE-0x8D9 and nothing else")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 contact = ContactOf(TRAINER_ROSE_1);
    u8 before[sizeof(gSaveBlock1Ptr->flags)];
    u32 changed = 0, i;

    ASSUME(contact >= REMATCH_APPENDED_START);
    ClearPhoneState();
    FlagSet(FLAG_HAS_MATCH_CALL);
    memcpy(before, gSaveBlock1Ptr->flags, sizeof(before));
    EXPECT(!PhoneContact_IsRegistered(contact));
    gSpecialVar_0x8000 = TRAINER_ROSE_1;
    PhoneOffer_Accept();
    EXPECT(PhoneContact_IsRegistered(contact));
    EXPECT(FlagGet(0x8BE + (contact - 90)));
    // The three places that read registration agree.
    EXPECT(TrainerIsMatchCallRegistered(contact));
    EXPECT(IsRematchEntryRegistered(contact));
    gSpecialVar_0x8004 = TRAINER_ROSE_1;
    EXPECT(IsTrainerRegistered());
    // Exactly one flag changed, and the decoration flags from 0x36A on are untouched.
    for (i = 0; i < sizeof(before); i++)
    {
        if (before[i] != gSaveBlock1Ptr->flags[i])
        {
            changed++;
            EXPECT_EQ(i, (0x8BE + (contact - 90)) / 8);
        }
    }
    EXPECT_EQ(changed, 1);
    for (i = HNS_EXTENDED_CONTENT_START / 8; i <= (HNS_EXTENDED_CONTENT_START + 299) / 8; i++)
        EXPECT_EQ(before[i], gSaveBlock1Ptr->flags[i]);
    // The old special never registers: Wayfarer registers only what the player accepts.
    FlagClear(PhoneContact_RegisteredFlag(contact));
    gSpecialVar_0x8004 = TRAINER_ROSE_1;
    SetMatchCallRegisteredFlag();
    EXPECT(!PhoneContact_IsRegistered(contact));

    RestoreSnapshot(snapshot);
}

TEST("Phone: Sevii and coast contacts register in their own bank slots")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 sevii = ContactOf(TRAINER_WAYFARER_SEVII_SWIMMER_MALE_FINN);
    u8 coast = ContactOf(TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_ALICE);

    ClearPhoneState();
    EXPECT(sevii >= DAILY_CONTACT_SEVII_FIRST && sevii < DAILY_CONTACT_COAST_FIRST);
    EXPECT(coast >= DAILY_CONTACT_COAST_FIRST && coast <= DAILY_CONTACT_LAST);
    gSpecialVar_0x8000 = TRAINER_WAYFARER_SEVII_SWIMMER_MALE_FINN_2;   // any team of the family offers the same number
    PhoneOffer_Accept();
    EXPECT(PhoneContact_IsRegistered(sevii));
    EXPECT(!PhoneContact_IsRegistered(coast));
    EXPECT(FlagGet(WAYFARER_SEVII_FLAG_ID(158 + sevii - DAILY_CONTACT_SEVII_FIRST)));
    gSpecialVar_0x8000 = TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_ALICE;
    PhoneOffer_Accept();
    EXPECT(PhoneContact_IsRegistered(coast));
    EXPECT(IsRematchEntryRegistered(coast));
    EXPECT(FlagGet(WAYFARER_SEVII_FLAG_ID(158 + coast - DAILY_CONTACT_SEVII_FIRST)));

    RestoreSnapshot(snapshot);
}

// ---------- team tiers ----------
// Fights teams 1..tier of a contact (marks their defeat flags), none above.
static void FightUpTo(u8 contact, u8 tier)
{
    u8 count, i;
    const u16 *ids = DailyContacts_Teams(contact, &count);

    for (i = 0; i < count; i++)
    {
        if (i <= tier)
            SetTrainerFlag(ids[i]);
        else
            ClearTrainerFlag(ids[i]);
    }
}

TEST("Phone: a registered contact's next team follows the higher of the teams fought and the teams its rating unlocks")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 contact = ContactOf(TRAINER_WADE_HNS);
    u8 count;
    const u16 *ids = DailyContacts_Teams(contact, &count);
    static const struct { u8 fought; u8 rating; u8 next; } cases[] =
    {
        // fought, Trainer Rating, expected next team index
        { 0,   0, 1 },  // the first rematch
        { 1,   0, 2 },  // each win moves on
        { 2,   0, 3 },
        { 3,   0, 4 },
        { 4,   0, 4 },  // after the last, they keep it
        { 0,  40, 2 },  // rating 40 unlocks team 2, so the next is team 3
        { 0,  79, 2 },
        { 0,  80, 3 },
        { 0, 120, 4 },
        { 0, 160, 4 },
        { 1,  80, 3 },  // the higher of the two wins
        { 3,  40, 4 },
        { 4, 160, 4 },
    };
    u32 i;

    ASSUME(count == 5);
    for (i = 0; i < ARRAY_COUNT(cases); i++)
    {
        FightUpTo(contact, cases[i].fought);
        SetTrainerRating(cases[i].rating);
        EXPECT_EQ(PhoneContact_NextTeam(contact), ids[cases[i].next]);
        EXPECT_EQ(PhoneContact_FoughtTier(contact), cases[i].fought);
    }
    // Registering later never lowers it: the teams fought as a wanderer count.
    FightUpTo(contact, 3);
    SetTrainerRating(0);
    EXPECT_EQ(PhoneContact_NextTeam(contact), ids[4]);

    RestoreSnapshot(snapshot);
}

TEST("Phone: a trainer without a registered number unlocks teams 2 to 5 at Trainer Rating 40, 80, 120 and 160")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 five = ContactOf(TRAINER_WADE_HNS);
    u8 four = ContactOf(TRAINER_TULLY_HNS);
    u8 two = ContactOf(TRAINER_KENJI_HNS);
    u8 count;
    const u16 *ids;
    static const struct { u8 rating; u8 tier; } cases[] =
    {
        { 0, 0 }, { 39, 0 }, { 40, 1 }, { 79, 1 }, { 80, 2 }, { 119, 2 }, { 120, 3 }, { 159, 3 }, { 160, 4 },
    };
    u32 i;

    ClearPhoneState();
    EXPECT_EQ(gPhoneRatingTierRatings[0], 40);
    EXPECT_EQ(gPhoneRatingTierRatings[3], 160);
    for (i = 0; i < ARRAY_COUNT(cases); i++)
    {
        SetTrainerRating(cases[i].rating);
        ids = DailyContacts_Teams(five, &count);
        ASSUME(count == 5);
        EXPECT_EQ(PhoneContact_RatingTier(five), cases[i].tier);
        EXPECT_EQ(PhoneContact_RatingTeam(five), ids[cases[i].tier]);
        // A trainer with fewer teams uses the lowest steps and stops at its last team.
        ids = DailyContacts_Teams(four, &count);
        ASSUME(count == 4);
        EXPECT_EQ(PhoneContact_RatingTeam(four), ids[min(cases[i].tier, 3)]);
        ids = DailyContacts_Teams(two, &count);
        ASSUME(count == 2);
        EXPECT_EQ(PhoneContact_RatingTeam(two), ids[min(cases[i].tier, 1)]);
    }
    // The wanderer's team is derived when the battle starts: an unregistered contact brings the rating's team, a
    // registered one the latest it fought.
    SetTrainerRating(120);
    ids = DailyContacts_Teams(five, &count);
    EXPECT_EQ(DailyTrainers_LatestTeam(TRAINER_WADE_HNS), ids[3]);
    FightUpTo(five, 1);
    FlagSet(PhoneContact_RegisteredFlag(five));
    // Registering never lowers it: the rating's team (tier 3) is further on than the latest fought (tier 1).
    EXPECT_EQ(DailyTrainers_LatestTeam(TRAINER_WADE_HNS), ids[3]);
    FightUpTo(five, 4);
    EXPECT_EQ(DailyTrainers_LatestTeam(TRAINER_WADE_HNS), ids[4]);
    SetTrainerRating(0);
    FightUpTo(five, 1);
    EXPECT_EQ(DailyTrainers_LatestTeam(TRAINER_WADE_HNS), ids[1]);
    SetTrainerRating(120);
    // A contact with a single team brings it whatever the rating.
    SetTrainerRating(160);
    EXPECT_EQ(PhoneContact_RatingTier(ContactOf(TRAINER_ROSE_1)), 4);
    EXPECT(PhoneContact_RatingTeam(ContactOf(TRAINER_ROSE_1)) != 0);

    RestoreSnapshot(snapshot);
}

TEST("Phone: Sevii and coast contacts choose their teams from the same tiers, and their registries count as fought")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 coast = ContactOf(TRAINER_WAYFARER_COAST_PICNICKER_MISSY);
    u8 sevii = ContactOf(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON);
    u8 count;
    const u16 *ids;

    ClearPhoneState();
    SetTrainerRating(0);
    gSaveBlock3Ptr->wayfarerCoast.vars[WAYFARER_COAST_REMATCH_STAGE_VAR_LOW] = 0;
    gSaveBlock3Ptr->wayfarerCoast.vars[WAYFARER_COAST_REMATCH_STAGE_VAR_HIGH] = 0;
    WayfarerSeviiRematchStageSet(WayfarerSeviiRematchFamilyIndex(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON), 0);
    ids = DailyContacts_Teams(coast, &count);
    EXPECT_EQ(count, 3);
    FightUpTo(coast, 0);
    EXPECT_EQ(PhoneContact_NextTeam(coast), ids[1]);
    // A rematch won advances the registry's stage, which is as good as the defeat flag.
    ClearTrainerFlag(ids[1]);
    WayfarerCoastRematchAdvance(TRAINER_WAYFARER_COAST_PICNICKER_MISSY);
    EXPECT_EQ(PhoneContact_FoughtTier(coast), 1);
    EXPECT_EQ(PhoneContact_NextTeam(coast), ids[2]);
    EXPECT_EQ(PhoneContact_LatestTeam(coast), ids[1]);
    // A high rating on a three-team family stops at its last team.
    SetTrainerRating(160);
    EXPECT_EQ(PhoneContact_RatingTier(coast), 2);
    EXPECT_EQ(PhoneContact_NextTeam(coast), ids[2]);

    SetTrainerRating(0);
    ids = DailyContacts_Teams(sevii, &count);
    EXPECT_EQ(count, 3);
    FightUpTo(sevii, 0);
    EXPECT_EQ(PhoneContact_NextTeam(sevii), ids[1]);
    WayfarerSeviiRematchAdvance(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON);
    ClearTrainerFlag(ids[1]);
    EXPECT_EQ(PhoneContact_FoughtTier(sevii), 1);
    EXPECT_EQ(PhoneContact_NextTeam(sevii), ids[2]);

    RestoreSnapshot(snapshot);
}

// ---------- readiness by steps ----------
// Puts the player on the home map of a contact, so the player stands in that contact's region.
static void StandWith(u8 contact)
{
    u8 mapGroup, mapNum;

    ASSUME(DailyContacts_GetHome(contact, &mapGroup, &mapNum));
    EnterMap(mapGroup, mapNum);
}

static void MeetAndRegister(u8 contact)
{
    SetTrainerFlag(DailyContacts_FirstTrainer(contact));
    FlagSet(PhoneContact_RegisteredFlag(contact));
}

TEST("Phone: contacts belong to their home's region")
{
    u8 contact;

    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
    {
        enum PhoneRegion region;

        if (!PhoneContact_IsContact(contact))
            continue;
        region = PhoneContact_Region(contact);
        EXPECT(region != PHONE_REGION_OTHER);
        if (contact >= DAILY_CONTACT_COAST_FIRST)
            EXPECT_EQ(region, PHONE_REGION_KANTO);
        else if (contact >= DAILY_CONTACT_SEVII_FIRST)
            EXPECT_EQ(region, PHONE_REGION_SEVII);
        else if (contact >= REMATCH_APPENDED_START || (contact >= 28 && contact < REMATCH_ALEX_HNS))
            EXPECT_EQ(region, PHONE_REGION_HOENN);
        else if (contact >= REMATCH_ALEX_HNS)
            EXPECT_EQ(region, PHONE_REGION_KANTO);
        else
            EXPECT_EQ(region, PHONE_REGION_JOHTO);
    }
}

TEST("Phone: 255 steps in a region make one registered, fought, not-ready contact of that region ready")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 johtoOne = ContactOf(TRAINER_WADE_HNS), johtoTwo = ContactOf(TRAINER_RALPH_HNS);
    u8 kanto = ContactOf(TRAINER_ALEX_HNS), hoenn = ContactOf(TRAINER_ROSE_1);
    u8 sevii = ContactOf(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON), coast = ContactOf(TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_ALICE);
    u8 unregistered = ContactOf(TRAINER_LIZ_HNS), unmet = ContactOf(TRAINER_ANTHONY_HNS);
    u8 ready, i;

    ClearPhoneState();
    StartDay(5000);
    MeetAndRegister(johtoOne);
    MeetAndRegister(johtoTwo);
    MeetAndRegister(kanto);
    MeetAndRegister(hoenn);
    MeetAndRegister(sevii);
    MeetAndRegister(coast);
    SetTrainerFlag(TRAINER_LIZ_HNS);                // met, no number
    FlagSet(PhoneContact_RegisteredFlag(unmet));    // a number, but never met (cannot happen in play; the step skips it)
    ClearTrainerFlag(TRAINER_ANTHONY_HNS);

    // Steps in Johto make one of the two Johto contacts ready, wherever the player stands in Johto.
    StandWith(johtoOne);
    PhoneReadiness_OnRegionSteps();
    ready = PhoneContact_IsReady(johtoOne) + PhoneContact_IsReady(johtoTwo);
    EXPECT_EQ(ready, 1);
    // Nobody from another region, nobody without a number, nobody unmet.
    EXPECT(!PhoneContact_IsReady(kanto));
    EXPECT(!PhoneContact_IsReady(hoenn));
    EXPECT(!PhoneContact_IsReady(sevii));
    EXPECT(!PhoneContact_IsReady(coast));
    EXPECT(!PhoneContact_IsReady(unregistered));
    EXPECT(!PhoneContact_IsReady(unmet));
    // The next steps pick the one that isn't ready, then there is nobody left in the region.
    PhoneReadiness_OnRegionSteps();
    EXPECT(PhoneContact_IsReady(johtoOne));
    EXPECT(PhoneContact_IsReady(johtoTwo));
    PhoneReadiness_OnRegionSteps();
    EXPECT(!PhoneContact_IsReady(kanto));

    // Each other region readies its own.
    StandWith(kanto);
    PhoneReadiness_OnRegionSteps();
    EXPECT(PhoneContact_IsReady(kanto));
    EXPECT(!PhoneContact_IsReady(hoenn));
    StandWith(hoenn);
    PhoneReadiness_OnRegionSteps();
    EXPECT(PhoneContact_IsReady(hoenn));
    EXPECT(!PhoneContact_IsReady(sevii));
    StandWith(sevii);
    PhoneReadiness_OnRegionSteps();
    EXPECT(PhoneContact_IsReady(sevii));
    EXPECT(WayfarerSeviiRematchIsReady(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON));
    EXPECT(!PhoneContact_IsReady(coast));
    // The coast is Kanto's.
    StandWith(coast);
    PhoneReadiness_OnRegionSteps();
    EXPECT(PhoneContact_IsReady(coast));
    EXPECT(WayfarerCoastRematchFamilyIsReady(coast - DAILY_CONTACT_COAST_FIRST));
    EXPECT(!PhoneContact_IsReady(unregistered));
    EXPECT(!PhoneContact_IsReady(unmet));
    for (i = 0; i < 5; i++)
        PhoneReadiness_OnRegionSteps();
    EXPECT(!PhoneContact_IsReady(unregistered));

    RestoreSnapshot(snapshot);
}

TEST("Phone: the pick is the day's roll, not the random number generator")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 contacts[3];
    u8 picked[2], i, j;
    bool32 differs = FALSE;
    u32 day;

    contacts[0] = ContactOf(TRAINER_ANTHONY_HNS);  // Johto contacts without a gift to find instead
    contacts[1] = ContactOf(TRAINER_RALPH_HNS);
    contacts[2] = ContactOf(TRAINER_LIZ_HNS);
    for (day = 6000; day < 6040; day++)
    {
        for (j = 0; j < 2; j++)
        {
            ClearPhoneState();
            StartDay(day);
            for (i = 0; i < 3; i++)
                MeetAndRegister(contacts[i]);
            StandWith(contacts[0]);
            PhoneReadiness_OnRegionSteps();
            picked[j] = DAILY_NO_CONTACT;
            for (i = 0; i < 3; i++)
            {
                if (PhoneContact_IsReady(contacts[i]))
                    picked[j] = contacts[i];
            }
            EXPECT(picked[j] != DAILY_NO_CONTACT);
            Random();   // the generator moving on changes nothing
        }
        // The same save, day and steps always pick the same contact.
        EXPECT_EQ(picked[0], picked[1]);
        differs |= picked[0] != contacts[0];
    }
    // Different days do not all pick the same contact.
    EXPECT(differs);

    RestoreSnapshot(snapshot);
}

TEST("Phone: a gift contact picked by the step event may hold a gift instead of readiness, 1 in 5")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 contact = ContactOf(TRAINER_WADE_HNS);

    FlagClear(FLAG_WADE_HAS_ITEM_HNS);
    // Four rolls in five do nothing; the fifth sets the flag and the item.
    EXPECT(!MatchCall_HnsRollGift(TRAINER_WADE_HNS, 1, 0));
    EXPECT(!MatchCall_HnsRollGift(TRAINER_WADE_HNS, 4, 0));
    EXPECT(!FlagGet(FLAG_WADE_HAS_ITEM_HNS));
    EXPECT(MatchCall_HnsRollGift(TRAINER_WADE_HNS, 5, 2));
    EXPECT(FlagGet(FLAG_WADE_HAS_ITEM_HNS));
    EXPECT(VarGet(VAR_WADE_ITEM_HNS) != 0);
    // A contact that has no gift never finds one.
    EXPECT(!MatchCall_HnsRollGift(TRAINER_RALPH_HNS, 0, 0));
    EXPECT(!MatchCall_HnsRollGift(TRAINER_ROSE_1, 0, 0));
    (void)contact;

    RestoreSnapshot(snapshot);
}

// ---------- call text selection ----------
// Whether the call text for `trainerId` is the text `expected` expanded with today's place, or (for a General call)
// one of the contact's General texts.
static bool32 CallIs(u16 trainerId, const u8 *expected, u8 mapGroup, u8 mapNum)
{
    u8 *actual = Alloc(400), *wanted = Alloc(400);
    bool32 same;

    SelectMatchCallMessage(trainerId, actual);
    // The place is named here, from the map the contact stands in, not read back from what the call just set.
    WayfarerGetPlaceNameOfMap(gStringVar2, mapGroup, mapNum);
    StringExpandPlaceholders(wanted, expected);
    same = StringCompare(actual, wanted) == 0;
    Free(actual);
    Free(wanted);
    return same;
}

static bool32 CallIsOneOf(u16 trainerId, const u8 *const *texts, u32 count)
{
    u8 *actual = Alloc(400), *wanted = Alloc(400);
    bool32 found = FALSE;
    u32 i;

    SelectMatchCallMessage(trainerId, actual);
    for (i = 0; i < count; i++)
    {
        StringExpandPlaceholders(wanted, texts[i]);
        found |= StringCompare(actual, wanted) == 0;
    }
    Free(actual);
    Free(wanted);
    return found;
}

static void SetBit(u8 *bits, u8 contact)
{
    bits[contact / 8] |= 1 << (contact % 8);
}

TEST("Phone: a Battle call needs the day-start ready set and a place, and names the place")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 contact = ContactOf(TRAINER_WADE_HNS);
    u8 mapGroup, mapNum, localId;
    static const u8 *const general[] = { MatchCall_HNS_Wade_General1, MatchCall_HNS_Wade_General2, MatchCall_HNS_Wade_General3 };
    u8 *text = Alloc(400), *wanted = Alloc(400), *place = Alloc(40);

    ClearPhoneState();
    SetTrainerRating(0);
    StartDay(8000);
    MeetAndRegister(contact);

    // Ready now, but not as the day began: a General call.
    gSaveBlock1Ptr->trainerRematches[contact] = 1;
    EXPECT(CallIsOneOf(TRAINER_WADE_HNS, general, 3));
    EXPECT(!SelectMatchCallMessage(TRAINER_WADE_HNS, text));
    // Ready as the day began, and standing somewhere: the Battle text, with today's place in {STR_VAR_2}.
    SetBit(gPokemonStoragePtr->dailySlots.readyAtDayStart, contact);
    ASSUME(DailyTrainers_WhereIsTrainerToday(TRAINER_WADE_HNS, &mapGroup, &mapNum, &localId));
    EXPECT(SelectMatchCallMessage(TRAINER_WADE_HNS, text));
    EXPECT(CallIs(TRAINER_WADE_HNS, MatchCall_HNS_Wade_Battle, mapGroup, mapNum));
    WayfarerGetPlaceNameOfMap(place, mapGroup, mapNum);
    EXPECT(StringLength(place) > 0);
    EXPECT_EQ(StringCompare(gStringVar2, place), 0);
    StringExpandPlaceholders(wanted, MatchCall_HNS_Wade_Battle);
    EXPECT_EQ(StringCompare(text, wanted), 0);
    // A win at its slot clears the slot for the day: no longer standing anywhere unbeaten, so a General call.
    DailyTrainers_SetCleared(DailyTrainers_FindSlot(mapGroup, mapNum, localId));
    EXPECT(!DailyTrainers_WhereIsTrainerToday(TRAINER_WADE_HNS, &mapGroup, &mapNum, &localId));
    EXPECT(!SelectMatchCallMessage(TRAINER_WADE_HNS, text));
    EXPECT(CallIsOneOf(TRAINER_WADE_HNS, general, 3));
    Free(text);
    Free(wanted);
    Free(place);

    RestoreSnapshot(snapshot);
}

TEST("Phone: a FoundItem call needs the day-start gift set, a place and the gift still held")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 contact = ContactOf(TRAINER_WADE_HNS);
    u8 mapGroup, mapNum, localId;
    static const u8 *const general[] = { MatchCall_HNS_Wade_General1, MatchCall_HNS_Wade_General2, MatchCall_HNS_Wade_General3 };
    u8 *text = Alloc(400), *wanted = Alloc(400);

    ClearPhoneState();
    StartDay(8100);
    MeetAndRegister(contact);

    // A gift found mid-day: announced from tomorrow.
    FlagSet(FLAG_WADE_HAS_ITEM_HNS);
    EXPECT(CallIsOneOf(TRAINER_WADE_HNS, general, 3));
    // Held as the day began: the FoundItem text, naming where he stands.
    SetBit(gPokemonStoragePtr->dailySlots.giftAtDayStart, contact);
    ASSUME(DailyTrainers_WhereIsTrainerToday(TRAINER_WADE_HNS, &mapGroup, &mapNum, &localId));
    EXPECT(CallIs(TRAINER_WADE_HNS, MatchCall_HNS_Wade_FoundItem, mapGroup, mapNum));
    EXPECT(!SelectMatchCallMessage(TRAINER_WADE_HNS, text));    // a gift is not a rematch request
    // Handed over: the gift is gone, so the call goes back to chatting.
    FlagClear(FLAG_WADE_HAS_ITEM_HNS);
    EXPECT(CallIsOneOf(TRAINER_WADE_HNS, general, 3));
    Free(text);
    Free(wanted);

    RestoreSnapshot(snapshot);
}

TEST("Phone: a contact whose slot is fixed calls from its home map, and goes General once its rematch is done")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 contact = ContactOf(TRAINER_LAO_1);
    static const u8 *const general[] = { MatchCall_HOENN_Lao_General1, MatchCall_HOENN_Lao_General2, MatchCall_HOENN_Lao_General3 };
    u8 mapGroup, mapNum, localId;

    EXPECT(DailyContacts_IsFixed(contact));
    ClearPhoneState();
    StartDay(8200);
    MeetAndRegister(contact);
    // Ready as the day began and still ready: the Battle text, naming its home map.
    gSaveBlock1Ptr->trainerRematches[contact] = 1;
    SetBit(gPokemonStoragePtr->dailySlots.readyAtDayStart, contact);
    EXPECT(DailyTrainers_WhereIsTrainerToday(TRAINER_LAO_1, &mapGroup, &mapNum, &localId));
    EXPECT_EQ(mapGroup, MAP_GROUP(MAP_ROUTE113));
    EXPECT_EQ(mapNum, MAP_NUM(MAP_ROUTE113));
    // A fixed contact's call names its home map, which is Route 113.
    {
        u8 *name = Alloc(40);
        static const u8 sRoute113[] = _("ROUTE 113");

        WayfarerGetPlaceNameOfMap(name, mapGroup, mapNum);
        EXPECT_EQ(StringCompare(name, sRoute113), 0);
        Free(name);
    }
    EXPECT(CallIs(TRAINER_LAO_1, MatchCall_HOENN_Lao_Battle, mapGroup, mapNum));
    // The rematch through its home script cleared its readiness: only General calls for the rest of the day.
    gSaveBlock1Ptr->trainerRematches[contact] = 0;
    EXPECT(CallIsOneOf(TRAINER_LAO_1, general, 3));
    EXPECT(!CallIs(TRAINER_LAO_1, MatchCall_HOENN_Lao_Battle, mapGroup, mapNum));

    RestoreSnapshot(snapshot);
}

TEST("Phone: Hoenn, Sevii and coast contacts call with their own Battle texts")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 rose = ContactOf(TRAINER_ROSE_1);
    u8 finn = ContactOf(TRAINER_WAYFARER_SEVII_SWIMMER_MALE_FINN);
    u8 alice = ContactOf(TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_ALICE);
    u8 mapGroup, mapNum, localId;

    ClearPhoneState();
    StartDay(8300);
    MeetAndRegister(rose);
    MeetAndRegister(finn);
    MeetAndRegister(alice);
    SetBit(gPokemonStoragePtr->dailySlots.readyAtDayStart, rose);
    SetBit(gPokemonStoragePtr->dailySlots.readyAtDayStart, finn);
    SetBit(gPokemonStoragePtr->dailySlots.readyAtDayStart, alice);
    WayfarerSeviiRematchSetReady(TRAINER_WAYFARER_SEVII_SWIMMER_MALE_FINN);
    WayfarerCoastRematchSetReady(TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_ALICE);
    UpdateRematchIfDefeated(rose);
    EXPECT(DailyTrainers_WhereIsTrainerToday(TRAINER_ROSE_1, &mapGroup, &mapNum, &localId));
    EXPECT(CallIs(TRAINER_ROSE_1, MatchCall_HOENN_Rose_Battle, mapGroup, mapNum));
    EXPECT(DailyTrainers_WhereIsTrainerToday(TRAINER_WAYFARER_SEVII_SWIMMER_MALE_FINN, &mapGroup, &mapNum, &localId));
    EXPECT(CallIs(TRAINER_WAYFARER_SEVII_SWIMMER_MALE_FINN, MatchCall_SEVII_Finn_Battle, mapGroup, mapNum));
    EXPECT(DailyTrainers_WhereIsTrainerToday(TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_ALICE, &mapGroup, &mapNum, &localId));
    EXPECT(CallIs(TRAINER_WAYFARER_COAST_SWIMMER_FEMALE_ALICE, MatchCall_COAST_Alice_Battle, mapGroup, mapNum));

    RestoreSnapshot(snapshot);
}

TEST("Phone: the place names fit {STR_VAR_2}'s 16 characters")
{
    u16 slot;
    u8 name[48];

    for (slot = 0; slot < WORLD_TRAINER_SLOT_COUNT; slot++)
    {
        const struct WorldTrainerSlot *row = DailyTrainers_GetSlot(slot);

        WayfarerGetPlaceNameOfMap(name, row->mapGroup, row->mapNum);
        EXPECT_EQ(StringLength(name) <= 16 ? 0 : slot + 1, 0);
        EXPECT(StringLength(name) <= 16);
    }
}

// ---------- the number offer ----------
TEST("Phone: a win against an unregistered contact is followed by the number offer, and asks again after a no")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 contact = ContactOf(TRAINER_ANTHONY_HNS);
    static u8 sOwnScript[] = { 0 };

    ClearPhoneState();
    // An earlier test's battle can leave a rotating occupant noted (EWRAM is not reset between tests); noting a battle
    // with no object (local id 0) clears it, so this battle is a home trainer's.
    DailyTrainers_NoteBattle(0, 0, TRAINER_ANTHONY_HNS);
    FlagSet(FLAG_HAS_MATCH_CALL);
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_ANTHONY_HNS;
    EXPECT(PhoneOffer_IsDue(TRAINER_ANTHONY_HNS));
    // Any team of the family counts.
    EXPECT(PhoneOffer_IsDue(gRematchTable[contact].trainerIds[2]));
    // A trainer without an after-battle script is followed by the shared one; with its own it runs its own, which asks.
    TRAINER_BATTLE_PARAM.battleScriptRetAddrA = NULL;
    gBattleOutcome = B_OUTCOME_WON;
    EXPECT_EQ(BattleSetup_GetTrainerPostBattleScript(), PhoneOffer_EventScript_AfterBattle);
    // A lost battle offers nothing, and neither does a Frontier one, whose trainer indexes can equal a contact's id.
    gBattleOutcome = B_OUTCOME_LOST;
    EXPECT(BattleSetup_GetTrainerPostBattleScript() != PhoneOffer_EventScript_AfterBattle);
    gBattleOutcome = B_OUTCOME_WON;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_PYRAMID;
    EXPECT(!PhoneOffer_IsDue(TRAINER_ANTHONY_HNS));
    gBattleTypeFlags = 0;
    EXPECT(PhoneOffer_IsDue(TRAINER_ANTHONY_HNS));
    TRAINER_BATTLE_PARAM.battleScriptRetAddrA = sOwnScript;
    EXPECT_EQ(BattleSetup_GetTrainerPostBattleScript(), sOwnScript);
    // The shared script names the contact by its first trainer.
    PhoneOffer_BufferTarget();
    EXPECT_EQ(gSpecialVar_0x8000, TRAINER_ANTHONY_HNS);
    EXPECT_EQ(PhoneOffer_CanOffer(), TRUE);
    // No leaves it unregistered, and the next win asks again.
    EXPECT(!PhoneContact_IsRegistered(contact));
    EXPECT(PhoneOffer_IsDue(TRAINER_ANTHONY_HNS));
    // Yes registers, and then nothing is offered.
    PhoneOffer_Accept();
    EXPECT(PhoneContact_IsRegistered(contact));
    EXPECT(!PhoneOffer_IsDue(TRAINER_ANTHONY_HNS));
    EXPECT_EQ(PhoneOffer_CanOffer(), FALSE);
    TRAINER_BATTLE_PARAM.battleScriptRetAddrA = NULL;
    EXPECT(BattleSetup_GetTrainerPostBattleScript() != PhoneOffer_EventScript_AfterBattle);
    // No phone, no offer; and a trainer with no number never offers one.
    FlagClear(PhoneContact_RegisteredFlag(contact));
    FlagClear(FLAG_HAS_MATCH_CALL);
    EXPECT(!PhoneOffer_IsDue(TRAINER_ANTHONY_HNS));
    FlagSet(FLAG_HAS_MATCH_CALL);
    EXPECT(!PhoneOffer_IsDue(TRAINER_FALKNER_1_HNS));
    TRAINER_BATTLE_PARAM.battleScriptRetAddrA = NULL;

    RestoreSnapshot(snapshot);
}

TEST("Phone: a contact that never wanders asks again when talked to, once, then its own script goes on")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 lao = ContactOf(TRAINER_LAO_1), joey = ContactOf(TRAINER_JOEY_HNS), tully = ContactOf(TRAINER_TULLY_HNS), wade = ContactOf(TRAINER_WADE_HNS);
    static const u8 sResume[] = { 0 };
    struct ScriptContext ctx;

    ClearPhoneState();
    FlagSet(FLAG_HAS_MATCH_CALL);
    // Fixed homes: Joey, Lao, Shelby, Sawyer and Tully. A contact in a daily slot gets a later win instead.
    EXPECT(DailyContacts_IsFixed(lao));
    EXPECT(DailyContacts_IsFixed(joey));
    EXPECT(DailyContacts_IsFixed(tully));
    EXPECT(!DailyContacts_IsFixed(wade));
    EXPECT(PhoneOffer_IsDueOnTalk(TRAINER_LAO_1));
    EXPECT(PhoneOffer_IsDueOnTalk(TRAINER_TULLY_HNS));
    EXPECT(!PhoneOffer_IsDueOnTalk(TRAINER_WADE_HNS));
    EXPECT(!PhoneOffer_IsDueOnTalk(TRAINER_FALKNER_1_HNS));
    // Declined, it asks again on the next talk; the offer then hands over to the script that was running.
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_LAO_1;
    PhoneOffer_StartInteraction();
    EXPECT_EQ(PhoneOffer_BeginTalkOffer(sResume), PhoneOffer_EventScript_TalkAgain);
    gSpecialVar_0x8000 = TRAINER_LAO_1;
    EXPECT_EQ(PhoneOffer_CanOffer(), TRUE);
    ctx.scriptPtr = NULL;
    PhoneOffer_Resume(&ctx);
    EXPECT_EQ(ctx.scriptPtr, sResume);
    // The script's own offer, later in the same interaction, does not ask a second time.
    EXPECT_EQ(PhoneOffer_CanOffer(), FALSE);
    PhoneOffer_StartInteraction();
    EXPECT_EQ(PhoneOffer_CanOffer(), TRUE);
    PhoneOffer_Accept();
    EXPECT(!PhoneOffer_IsDueOnTalk(TRAINER_LAO_1));
    // Without the Pokégear it never asks.
    FlagClear(FLAG_HAS_MATCH_CALL);
    EXPECT(!PhoneOffer_IsDueOnTalk(TRAINER_TULLY_HNS));

    RestoreSnapshot(snapshot);
}

// Hoenn scripts read the engine's Pokégear flag directly: tools/wayfarer_source_constants (ENGINE_SHARED_FLAGS).
TEST("Phone: a gift holder is not rolled for again")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 wade = ContactOf(TRAINER_WADE_HNS);

    // A contact that already holds a gift is not picked by the step event again.
    ClearPhoneState();
    StartDay(9000);
    MeetAndRegister(wade);
    StandWith(wade);
    FlagSet(FLAG_WADE_HAS_ITEM_HNS);
    VarSet(VAR_WADE_ITEM_HNS, ITEM_NUGGET);
    PhoneReadiness_OnRegionSteps();
    EXPECT(!PhoneContact_IsReady(wade));
    EXPECT_EQ(VarGet(VAR_WADE_ITEM_HNS), ITEM_NUGGET);
    FlagClear(FLAG_WADE_HAS_ITEM_HNS);

    RestoreSnapshot(snapshot);
}

extern const u8 MatchCall_HNS_Tully_General1[];
extern const u8 MatchCall_HNS_Tully_General2[];
extern const u8 MatchCall_HNS_Tully_General3[];
extern const u8 MatchCall_HNS_Tully_FoundItem[];

TEST("Phone: a fixed-home gift contact (Tully) calls from his home with FoundItem, and goes General after the gift")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 tully = ContactOf(TRAINER_TULLY_HNS);
    u8 mapGroup, mapNum, localId;
    static const u8 *const general[] = { MatchCall_HNS_Tully_General1, MatchCall_HNS_Tully_General2, MatchCall_HNS_Tully_General3 };
    static const u8 sRoute42[] = _("ROUTE 42");
    u8 *name = Alloc(40);

    EXPECT(DailyContacts_IsFixed(tully));
    ClearPhoneState();
    StartDay(8400);
    MeetAndRegister(tully);
    // A gift found mid-day is announced from tomorrow.
    FlagSet(FLAG_TULLY_HAS_ITEM_HNS);
    EXPECT(CallIsOneOf(TRAINER_TULLY_HNS, general, 3));
    // Held as the day began: FoundItem, naming his home map, Route 42.
    SetBit(gPokemonStoragePtr->dailySlots.giftAtDayStart, tully);
    EXPECT(DailyTrainers_WhereIsTrainerToday(TRAINER_TULLY_HNS, &mapGroup, &mapNum, &localId));
    EXPECT_EQ(mapGroup, MAP_GROUP(MAP_ROUTE42_HNS));
    EXPECT_EQ(mapNum, MAP_NUM(MAP_ROUTE42_HNS));
    WayfarerGetPlaceNameOfMap(name, mapGroup, mapNum);
    EXPECT_EQ(StringCompare(name, sRoute42), 0);
    EXPECT(CallIs(TRAINER_TULLY_HNS, MatchCall_HNS_Tully_FoundItem, mapGroup, mapNum));
    // His home script handed it over today: only General calls for the rest of the day.
    FlagClear(FLAG_TULLY_HAS_ITEM_HNS);
    EXPECT(CallIsOneOf(TRAINER_TULLY_HNS, general, 3));
    EXPECT(!CallIs(TRAINER_TULLY_HNS, MatchCall_HNS_Tully_FoundItem, mapGroup, mapNum));
    Free(name);

    RestoreSnapshot(snapshot);
}

// The Match Call list as the Pokénav builds it, run to its end. The resources struct is private to pokenav.c, so this is
// its layout.
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
}

TEST("Phone: every registered contact is in the Pokénav list once, and the list never overflows")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 contact;
    u32 contacts = 0, entries, found;
    struct PokenavMatchCallEntry *list;

    ClearPhoneState();
    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
    {
        if (!PhoneContact_IsContact(contact))
            continue;
        FlagSet(PhoneContact_RegisteredFlag(contact));
        contacts++;
    }
    EXPECT_EQ(contacts, 39 + 64 + 32 + 9);
    // The list has room for every header and every contact.
    EXPECT(MATCH_CALL_ENTRIES_MAX >= MC_HEADER_COUNT + 39 + 64 + 32 + 9);
    ASSUME(BuildMatchCallList());
    entries = GetNumberRegistered();
    EXPECT(entries <= MATCH_CALL_ENTRIES_MAX);
    list = GetMatchCallList();
    for (contact = 0; contact <= DAILY_CONTACT_LAST; contact++)
    {
        u32 i;

        if (!PhoneContact_IsContact(contact))
            continue;
        for (found = 0, i = 0; i < entries; i++)
        {
            if (!list[i].isSpecialTrainer && list[i].headerId == contact)
                found++;
        }
        EXPECT_EQ(found, 1);
    }
    // A contact's place, and whether it is ready, are read from its own home and registry.
    PhoneContact_SetReady(ContactOf(TRAINER_WAYFARER_SEVII_SWIMMER_MALE_FINN));
    {
        u32 i;
        u8 finn = ContactOf(TRAINER_WAYFARER_SEVII_SWIMMER_MALE_FINN);

        for (i = 0; i < entries; i++)
        {
            if (!list[i].isSpecialTrainer && list[i].headerId == finn)
                EXPECT(ShouldDrawRematchPokeballIcon(i));
        }
    }
    FreeMatchCallList();
    Free(gPokenavResources);
    gPokenavResources = NULL;

    RestoreSnapshot(snapshot);
}

#endif // IS_WAYFARER

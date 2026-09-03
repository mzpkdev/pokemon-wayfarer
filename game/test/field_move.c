#include "global.h"
#include "challenge_menu.h"
#include "event_data.h"
#include "field_move.h"
#include "field_player_avatar.h"
#include "item.h"
#include "pokemon.h"
#include "string_util.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/field_move.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/moves.h"

#define ONE_TYPE_CHALLENGE_DISABLED 31

static void ResetFieldMoveTestState(void)
{
    memset(gPlayerParty, 0, sizeof(gPlayerParty));
    gPlayerPartyCount = 0;
    ClearBag();
    ClearPlayerAvatarInfo();
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_Nuzlocke = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Nuzlocke_EasyMode = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_Mirror = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_PartyLimit = 0;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_OneTypeChallenge = ONE_TYPE_CHALLENGE_DISABLED;
    FlagClear(FLAG_BADGE07_GET);
#if IS_WAYFARER
    FlagClear(WAYFARER_HOENN_DIVE_AUTHORIZATION_FLAG);
#endif
}

static u16 FindSpeciesWithHmCompatibility(enum Move move, bool32 compatible)
{
    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        if (species != SPECIES_EGG
         && IsSpeciesEnabled(species)
         && !!CanLearnTeachableMove(species, move) == compatible)
            return species;
    }

    return SPECIES_NONE;
}

static void SetPlainMoves(struct Pokemon *mon)
{
    SetMonMoveSlot(mon, MOVE_TACKLE, 0);
    SetMonMoveSlot(mon, MOVE_NONE, 1);
    SetMonMoveSlot(mon, MOVE_NONE, 2);
    SetMonMoveSlot(mon, MOVE_NONE, 3);
}

static void CreatePartyMon(u32 slot, u16 species)
{
    CreateMon(&gPlayerParty[slot], species, 20, 0, OTID_STRUCT_PRESET(0));
    SetPlainMoves(&gPlayerParty[slot]);
    if (gPlayerPartyCount <= slot)
        gPlayerPartyCount = slot + 1;
}

static void MakePartyMonAnEgg(u32 slot)
{
    bool8 isEgg = TRUE;

    SetMonData(&gPlayerParty[slot], MON_DATA_IS_EGG, &isEgg);
}

TEST("HM field use: resolver accepts a known user without the HM or a badge")
{
    u8 partyIndex;
    u16 species = FindSpeciesWithHmCompatibility(MOVE_CUT, FALSE);

    ResetFieldMoveTestState();
    ASSUME(species != SPECIES_NONE);
    CreatePartyMon(0, species);
    SetMonMoveSlot(&gPlayerParty[0], MOVE_CUT, 0);

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_CUT, &partyIndex), FIELD_MOVE_USER_FOUND);
    EXPECT_EQ(partyIndex, 0);
    EXPECT(!CheckBagHasItem(ITEM_HM_CUT, 1));
    EXPECT(!FlagGet(FLAG_BADGE01_GET));
    EXPECT(!FlagGet(FLAG_BADGE02_GET));
}

TEST("HM field use: known users outrank earlier compatible party members")
{
    u8 partyIndex;
    u16 species = FindSpeciesWithHmCompatibility(MOVE_SURF, TRUE);

    ResetFieldMoveTestState();
    ASSUME(species != SPECIES_NONE);
    CreatePartyMon(0, species);
    CreatePartyMon(1, species);
    SetMonMoveSlot(&gPlayerParty[1], MOVE_SURF, 0);
    EXPECT(AddBagItem(ITEM_HM_SURF, 1));

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_SURF, &partyIndex), FIELD_MOVE_USER_FOUND);
    EXPECT_EQ(partyIndex, 1);
}

TEST("HM field use: resolver selects the first compatible user when the HM is owned")
{
    u8 partyIndex;
    u16 species = FindSpeciesWithHmCompatibility(MOVE_STRENGTH, TRUE);

    ResetFieldMoveTestState();
    ASSUME(species != SPECIES_NONE);
    CreatePartyMon(0, species);
    CreatePartyMon(1, species);
    EXPECT(AddBagItem(ITEM_HM_STRENGTH, 1));

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_STRENGTH, &partyIndex), FIELD_MOVE_USER_FOUND);
    EXPECT_EQ(partyIndex, 0);
}

TEST("HM field use: resolver distinguishes a missing HM from no eligible Pokemon")
{
    u8 partyIndex;
    u16 compatibleSpecies = FindSpeciesWithHmCompatibility(MOVE_FLASH, TRUE);
    u16 incompatibleSpecies = FindSpeciesWithHmCompatibility(MOVE_FLASH, FALSE);

    ResetFieldMoveTestState();
    ASSUME(compatibleSpecies != SPECIES_NONE);
    ASSUME(incompatibleSpecies != SPECIES_NONE);
    CreatePartyMon(0, compatibleSpecies);

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_FLASH, &partyIndex), FIELD_MOVE_USER_MISSING_ITEM);
    EXPECT_EQ(partyIndex, PARTY_SIZE);

    ResetFieldMoveTestState();
    CreatePartyMon(0, incompatibleSpecies);
    EXPECT(AddBagItem(ITEM_HM_FLASH, 1));

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_FLASH, &partyIndex), FIELD_MOVE_USER_NO_ELIGIBLE_MON);
    EXPECT_EQ(partyIndex, PARTY_SIZE);
}

TEST("HM field use: HMs Overwrite selects the first non-Egg party member")
{
    u8 partyIndex;
    u16 species = FindSpeciesWithHmCompatibility(MOVE_FLY, FALSE);

    ResetFieldMoveTestState();
    ASSUME(species != SPECIES_NONE);
    CreatePartyMon(0, species);
    MakePartyMonAnEgg(0);
    CreatePartyMon(1, species);
    CreatePartyMon(2, species);
    EXPECT(AddBagItem(ITEM_HM_FLY, 1));
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_Nuzlocke = TRUE;
    ASSUME(HMsOverwriteOptionActive());

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_FLY, &partyIndex), FIELD_MOVE_USER_FOUND);
    EXPECT_EQ(partyIndex, 1);
}

TEST("HM field use: HMs Overwrite still requires ownership and a non-Egg user")
{
    u8 partyIndex;
    u16 species = FindSpeciesWithHmCompatibility(MOVE_FLY, FALSE);

    ResetFieldMoveTestState();
    ASSUME(species != SPECIES_NONE);
    CreatePartyMon(0, species);
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_Nuzlocke = TRUE;
    ASSUME(HMsOverwriteOptionActive());

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_FLY, &partyIndex), FIELD_MOVE_USER_MISSING_ITEM);
    EXPECT_EQ(partyIndex, PARTY_SIZE);

    MakePartyMonAnEgg(0);
    EXPECT(AddBagItem(ITEM_HM_FLY, 1));

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_FLY, &partyIndex), FIELD_MOVE_USER_NO_ELIGIBLE_MON);
    EXPECT_EQ(partyIndex, PARTY_SIZE);
}

TEST("HM field use: an Egg is rejected even if corrupt data gives it a known HM move")
{
    u8 partyIndex;
    u16 species = FindSpeciesWithHmCompatibility(MOVE_CUT, TRUE);

    ResetFieldMoveTestState();
    ASSUME(species != SPECIES_NONE);
    CreatePartyMon(0, species);
    SetMonMoveSlot(&gPlayerParty[0], MOVE_CUT, 0);
    MakePartyMonAnEgg(0);

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_CUT, &partyIndex), FIELD_MOVE_USER_MISSING_ITEM);
    EXPECT_EQ(partyIndex, PARTY_SIZE);
}

TEST("HM field use: fainted compatible and known Pokemon remain eligible")
{
    u8 partyIndex;
    u32 hp = 0;
    u16 species = FindSpeciesWithHmCompatibility(MOVE_ROCK_SMASH, TRUE);

    ResetFieldMoveTestState();
    ASSUME(species != SPECIES_NONE);
    CreatePartyMon(0, species);
    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    EXPECT(AddBagItem(ITEM_HM_ROCK_SMASH, 1));

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_ROCK_SMASH, &partyIndex), FIELD_MOVE_USER_FOUND);
    EXPECT_EQ(partyIndex, 0);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_HP), 0);

    ClearBag();
    SetMonMoveSlot(&gPlayerParty[0], MOVE_ROCK_SMASH, 0);
    EXPECT_EQ(ResolveFieldMoveUser(MOVE_ROCK_SMASH, &partyIndex), FIELD_MOVE_USER_FOUND);
    EXPECT_EQ(partyIndex, 0);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_HP), 0);
}

TEST("HM field use: resolving unlearned use does not mutate Pokemon or inventory")
{
    u8 partyIndex;
    u16 species = FindSpeciesWithHmCompatibility(MOVE_WATERFALL, TRUE);
    struct Pokemon before;

    ResetFieldMoveTestState();
    ASSUME(species != SPECIES_NONE);
    CreatePartyMon(0, species);
    EXPECT(AddBagItem(ITEM_HM_WATERFALL, 1));
    memcpy(&before, &gPlayerParty[0], sizeof(before));

    EXPECT_EQ(ResolveFieldMoveUser(MOVE_WATERFALL, &partyIndex), FIELD_MOVE_USER_FOUND);
    EXPECT_EQ(partyIndex, 0);
    EXPECT_EQ(memcmp(&before, &gPlayerParty[0], sizeof(before)), 0);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_HM_WATERFALL), 1);
}

TEST("HM field use: automatic Surf uses the shared eligibility resolver")
{
    u32 hp = 0;
    u16 compatibleSpecies = FindSpeciesWithHmCompatibility(MOVE_SURF, TRUE);
    u16 incompatibleSpecies = FindSpeciesWithHmCompatibility(MOVE_SURF, FALSE);

    ResetFieldMoveTestState();
    ASSUME(compatibleSpecies != SPECIES_NONE);
    ASSUME(incompatibleSpecies != SPECIES_NONE);
    CreatePartyMon(0, compatibleSpecies);
    EXPECT(!PartyHasMonWithSurf());

    EXPECT(AddBagItem(ITEM_HM_SURF, 1));
    EXPECT(PartyHasMonWithSurf());
    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    EXPECT(PartyHasMonWithSurf());
    MakePartyMonAnEgg(0);
    EXPECT(!PartyHasMonWithSurf());

    ResetFieldMoveTestState();
    CreatePartyMon(0, incompatibleSpecies);
    SetMonMoveSlot(&gPlayerParty[0], MOVE_SURF, 0);
    EXPECT(PartyHasMonWithSurf());
    MakePartyMonAnEgg(0);
    EXPECT(!PartyHasMonWithSurf());

    ResetFieldMoveTestState();
    CreatePartyMon(0, incompatibleSpecies);
    EXPECT(AddBagItem(ITEM_HM_SURF, 1));
    EXPECT(!PartyHasMonWithSurf());

    gSaveBlock3Ptr->challengeSettings.tx_Challenges_Nuzlocke = TRUE;
    EXPECT(PartyHasMonWithSurf());
}

TEST("HM field use: Fly and Flash require knowledge or owned-HM compatibility")
{
    static const enum Move moves[] = {MOVE_FLY, MOVE_FLASH};

    for (u32 i = 0; i < ARRAY_COUNT(moves); i++)
    {
        enum Move move = moves[i];
        enum Item item = GetTMHMItemIdFromMoveId(move);
        u16 species = FindSpeciesWithHmCompatibility(move, TRUE);
        struct Pokemon mon;

        ResetFieldMoveTestState();
        ASSUME(item != ITEM_NONE);
        ASSUME(species != SPECIES_NONE);
        CreateMon(&mon, species, 20, 0, OTID_STRUCT_PRESET(0));
        SetPlainMoves(&mon);

        EXPECT(!CanPartyMonUseFieldMove(&mon, move));
        EXPECT(AddBagItem(item, 1));
        EXPECT(CanPartyMonUseFieldMove(&mon, move));

        ClearBag();
        SetMonMoveSlot(&mon, move, 0);
        EXPECT(CanPartyMonUseFieldMove(&mon, move));
    }
}

TEST("HM field use: Fly and Flash apply HMs Overwrite but reject Eggs")
{
    static const enum Move moves[] = {MOVE_FLY, MOVE_FLASH};

    for (u32 i = 0; i < ARRAY_COUNT(moves); i++)
    {
        enum Move move = moves[i];
        enum Item item = GetTMHMItemIdFromMoveId(move);
        u16 species = FindSpeciesWithHmCompatibility(move, FALSE);
        struct Pokemon mon;
        bool8 isEgg = TRUE;

        ResetFieldMoveTestState();
        ASSUME(item != ITEM_NONE);
        ASSUME(species != SPECIES_NONE);
        CreateMon(&mon, species, 20, 0, OTID_STRUCT_PRESET(0));
        SetPlainMoves(&mon);
        EXPECT(AddBagItem(item, 1));
        EXPECT(!CanPartyMonUseFieldMove(&mon, move));

        gSaveBlock3Ptr->challengeSettings.tx_Challenges_Nuzlocke = TRUE;
        ASSUME(HMsOverwriteOptionActive());
        EXPECT(CanPartyMonUseFieldMove(&mon, move));

        SetMonMoveSlot(&mon, move, 0);
        SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
        EXPECT(!CanPartyMonUseFieldMove(&mon, move));
    }
}

TEST("HM field use: party actions reserve terrain HMs for contextual use")
{
    EXPECT(!IsFieldMovePartyMenuAction(FIELD_MOVE_CUT));
    EXPECT(!IsFieldMovePartyMenuAction(FIELD_MOVE_ROCK_SMASH));
    EXPECT(!IsFieldMovePartyMenuAction(FIELD_MOVE_STRENGTH));
    EXPECT(!IsFieldMovePartyMenuAction(FIELD_MOVE_SURF));
    EXPECT(!IsFieldMovePartyMenuAction(FIELD_MOVE_WATERFALL));
    EXPECT(IsFieldMovePartyMenuAction(FIELD_MOVE_FLY));
    EXPECT(IsFieldMovePartyMenuAction(FIELD_MOVE_FLASH));
    EXPECT(IsFieldMovePartyMenuAction(FIELD_MOVE_DIG));
#if IS_WAYFARER
    EXPECT(!IsFieldMovePartyMenuAction(FIELD_MOVE_DIVE));
    EXPECT(!IsFieldMovePartyMenuAction(FIELD_MOVE_WHIRLPOOL));
#elif IS_HNS
    EXPECT(IsFieldMovePartyMenuAction(FIELD_MOVE_DIVE));
    EXPECT(!IsFieldMovePartyMenuAction(FIELD_MOVE_WHIRLPOOL));
#else
    EXPECT(!IsFieldMovePartyMenuAction(FIELD_MOVE_DIVE));
#endif
}

TEST("HM field use: every regional HM field action is badge-free")
{
    static const enum FieldMove fieldMoves[] =
    {
        FIELD_MOVE_CUT,
        FIELD_MOVE_FLY,
        FIELD_MOVE_SURF,
        FIELD_MOVE_STRENGTH,
        FIELD_MOVE_FLASH,
        FIELD_MOVE_ROCK_SMASH,
        FIELD_MOVE_WATERFALL,
#if IS_HNS
        FIELD_MOVE_WHIRLPOOL,
#elif !IS_FRLG
        FIELD_MOVE_DIVE,
#endif
    };

    ResetFieldMoveTestState();
    for (u32 i = 0; i < ARRAY_COUNT(fieldMoves); i++)
        EXPECT(IsFieldMoveUnlocked(fieldMoves[i]));

    FlagSet(FLAG_BADGE01_GET);
    FlagSet(FLAG_BADGE02_GET);
    FlagSet(FLAG_BADGE03_GET);
    FlagSet(FLAG_BADGE04_GET);
    FlagSet(FLAG_BADGE05_GET);
    FlagSet(FLAG_BADGE06_GET);
    FlagSet(FLAG_BADGE07_GET);
    FlagSet(FLAG_BADGE08_GET);
    for (u32 i = 0; i < ARRAY_COUNT(fieldMoves); i++)
        EXPECT(IsFieldMoveUnlocked(fieldMoves[i]));
}

#if IS_WAYFARER
TEST("Wayfarer HM registry appends Dive as HM09 without replacing Whirlpool")
{
    EXPECT_EQ(NUM_HIDDEN_MACHINES, 9);
    EXPECT_EQ(ITEM_HM08, 689);
    EXPECT_EQ(ITEM_HM_WHIRLPOOL, ITEM_HM08);
    EXPECT_EQ(ITEM_HM_DIVE, ITEM_HM09);
    EXPECT_EQ(ITEM_HM09, ITEM_AZURE_FLUTE + 1);
    EXPECT_EQ(ITEMS_COUNT, ITEM_HM09 + 1);
    EXPECT_EQ(GetItemTMHMMoveId(ITEM_HM08), MOVE_WHIRLPOOL);
    EXPECT_EQ(GetItemTMHMMoveId(ITEM_HM09), MOVE_DIVE);
    EXPECT_EQ(GetTMHMItemIdFromMoveId(MOVE_WHIRLPOOL), ITEM_HM08);
    EXPECT_EQ(GetTMHMItemIdFromMoveId(MOVE_DIVE), ITEM_HM09);
    EXPECT_EQ(StringCompare(GetItemName(ITEM_HM09), COMPOUND_STRING("HM09")), 0);
    EXPECT_EQ(GetItemPocket(ITEM_HM09), POCKET_TM_HM);
    EXPECT(IsMoveHM(MOVE_WHIRLPOOL));
    EXPECT(IsMoveHM(MOVE_DIVE));
}

TEST("Wayfarer Dive authorization dispatches by valid map source")
{
    static const u16 hnsMap = MAP_NEW_BARK_TOWN_HNS;
    static const u16 hoennMap = MAP_LITTLEROOT_TOWN;

    ResetFieldMoveTestState();
    EXPECT_EQ(WayfarerGetDiveMapContext(MAP_GROUP(hnsMap), MAP_NUM(hnsMap)), WAYFARER_DIVE_MAP_HNS);
    EXPECT_EQ(WayfarerGetDiveMapContext(MAP_GROUP(hoennMap), MAP_NUM(hoennMap)), WAYFARER_DIVE_MAP_HOENN);
    EXPECT_EQ(WayfarerGetDiveMapContext(-1, -1), WAYFARER_DIVE_MAP_UNKNOWN);

    for (u8 hnsBadge = FALSE; hnsBadge <= TRUE; hnsBadge++)
    {
        for (u8 hoennAuthorization = FALSE; hoennAuthorization <= TRUE; hoennAuthorization++)
        {
            if (hnsBadge)
                FlagSet(FLAG_BADGE07_GET);
            else
                FlagClear(FLAG_BADGE07_GET);
            if (hoennAuthorization)
                FlagSet(WAYFARER_HOENN_DIVE_AUTHORIZATION_FLAG);
            else
                FlagClear(WAYFARER_HOENN_DIVE_AUTHORIZATION_FLAG);

            EXPECT_EQ(WayfarerIsDiveAuthorizedForMap(MAP_GROUP(hnsMap), MAP_NUM(hnsMap)), hnsBadge);
            EXPECT_EQ(WayfarerIsDiveAuthorizedForMap(MAP_GROUP(hoennMap), MAP_NUM(hoennMap)), hoennAuthorization);
            EXPECT(!WayfarerIsDiveAuthorizedForMap(-1, -1));

            gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(hnsMap);
            gSaveBlock1Ptr->location.mapNum = MAP_NUM(hnsMap);
            EXPECT_EQ(IsFieldMoveUnlocked(FIELD_MOVE_DIVE), hnsBadge);
            gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(hoennMap);
            gSaveBlock1Ptr->location.mapNum = MAP_NUM(hoennMap);
            EXPECT_EQ(IsFieldMoveUnlocked(FIELD_MOVE_DIVE), hoennAuthorization);
            gSaveBlock1Ptr->location.mapGroup = -1;
            gSaveBlock1Ptr->location.mapNum = -1;
            EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_DIVE));
        }
    }
}
#elif IS_HNS
TEST("HM field use: HNS maps HM08 to Whirlpool and keeps Dive badge-gated")
{
    u8 partyIndex;
    u16 whirlpoolSpecies = FindSpeciesWithHmCompatibility(MOVE_WHIRLPOOL, TRUE);

    ResetFieldMoveTestState();
    ASSUME(whirlpoolSpecies != SPECIES_NONE);
    EXPECT_EQ(GetTMHMItemIdFromMoveId(MOVE_WHIRLPOOL), ITEM_HM_WHIRLPOOL);
    EXPECT_EQ(GetTMHMItemIdFromMoveId(MOVE_DIVE), ITEM_NONE);
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_WHIRLPOOL));
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_DIVE));

    CreatePartyMon(0, whirlpoolSpecies);
    EXPECT(AddBagItem(ITEM_HM_WHIRLPOOL, 1));
    EXPECT_EQ(ResolveFieldMoveUser(MOVE_WHIRLPOOL, &partyIndex), FIELD_MOVE_USER_FOUND);
    EXPECT_EQ(partyIndex, 0);

    ResetFieldMoveTestState();
    CreatePartyMon(0, whirlpoolSpecies);
    SetMonMoveSlot(&gPlayerParty[0], MOVE_DIVE, 0);
    EXPECT_EQ(ResolvePartyMoveUser(MOVE_DIVE, &partyIndex), FIELD_MOVE_USER_FOUND);
    EXPECT_EQ(partyIndex, 0);
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_DIVE));
    FlagSet(FLAG_BADGE07_GET);
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_DIVE));
}
#else
TEST("HM field use: party-move resolver shares HM ownership and user selection rules")
{
    u8 partyIndex;
    u16 species = FindSpeciesWithHmCompatibility(MOVE_CUT, TRUE);

    ResetFieldMoveTestState();
    ASSUME(species != SPECIES_NONE);
    CreatePartyMon(0, species);
    EXPECT_EQ(ResolvePartyMoveUser(MOVE_CUT, &partyIndex), FIELD_MOVE_USER_MISSING_ITEM);

    EXPECT(AddBagItem(ITEM_HM_CUT, 1));
    EXPECT_EQ(ResolvePartyMoveUser(MOVE_CUT, &partyIndex), FIELD_MOVE_USER_FOUND);
    EXPECT_EQ(partyIndex, 0);
}
#endif

TEST("HM field use: regional moves are forgettable while HM items remain important")
{
    static const enum Move moves[] =
    {
        MOVE_CUT,
        MOVE_FLY,
        MOVE_SURF,
        MOVE_STRENGTH,
        MOVE_FLASH,
        MOVE_ROCK_SMASH,
        MOVE_WATERFALL,
#if IS_HNS
        MOVE_WHIRLPOOL,
#if IS_WAYFARER
        MOVE_DIVE,
#endif
#else
        MOVE_DIVE,
#endif
    };
    static const enum Item items[] =
    {
        ITEM_HM_CUT,
        ITEM_HM_FLY,
        ITEM_HM_SURF,
        ITEM_HM_STRENGTH,
        ITEM_HM_FLASH,
        ITEM_HM_ROCK_SMASH,
        ITEM_HM_WATERFALL,
#if IS_HNS
        ITEM_HM_WHIRLPOOL,
#if IS_WAYFARER
        ITEM_HM_DIVE,
#endif
#else
        ITEM_HM_DIVE,
#endif
    };

    ResetFieldMoveTestState();
    ASSUME(ARRAY_COUNT(moves) == ARRAY_COUNT(items));
    for (u32 i = 0; i < ARRAY_COUNT(moves); i++)
    {
        EXPECT(!CannotForgetMove(moves[i]));
        EXPECT(GetItemImportance(items[i]));
        EXPECT_EQ(GetTMHMItemIdFromMoveId(moves[i]), items[i]);
    }

#if IS_HNS && !IS_WAYFARER
    EXPECT(!CannotForgetMove(MOVE_DIVE));
#endif
}

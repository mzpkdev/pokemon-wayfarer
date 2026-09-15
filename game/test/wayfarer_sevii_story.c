#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "new_game.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_size_record.h"
#include "random.h"
#include "trainer_rating.h"
#include "wayfarer_appearance.h"
#include "wayfarer_origin.h"
#include "wayfarer_persistence.h"
#include "wayfarer_sevii_story.h"
#include "test/test.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokedex.h"
#include "constants/regions.h"
#include "constants/species.h"
#include "constants/vars.h"
#include "constants/wayfarer_appearance.h"
#include "constants/wayfarer_origin.h"

#if IS_WAYFARER

extern bool8 CapeBrinkGetMoveToTeachLeadPokemon(void);
extern bool8 HasLearnedAllMovesFromCapeBrinkTutor(void);

static bool8 IsSelphySourceReward(u16 item)
{
    switch (item)
    {
    case ITEM_LUXURY_BALL:
    case ITEM_BIG_PEARL:
    case ITEM_PEARL:
    case ITEM_STARDUST:
    case ITEM_STAR_PIECE:
    case ITEM_NUGGET:
    case ITEM_RARE_CANDY:
        return TRUE;
    default:
        return FALSE;
    }
}

TEST("Wayfarer Sevii item receipts commit only after their Bag transaction")
{
    WayfarerSeviiInitPersistentState();
    ClearBag();

    EXPECT(!WayfarerSeviiTryGiveItemThenSetFlag(ITEM_NONE, FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED));
    EXPECT(!FlagGet(FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED));

    EXPECT(WayfarerSeviiTryGiveItemThenSetFlag(ITEM_IAPAPA_BERRY, FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED));
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED));
    EXPECT(CheckBagHasItem(ITEM_IAPAPA_BERRY, 1));
    EXPECT(WayfarerSeviiTryGiveItemThenSetFlag(ITEM_IAPAPA_BERRY, FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED));
    EXPECT(RemoveBagItem(ITEM_IAPAPA_BERRY, 1));
    EXPECT(!CheckBagHasItem(ITEM_IAPAPA_BERRY, 1));

    EXPECT(!WayfarerSeviiTryRemoveItemThenSetFlag(ITEM_METEORITE, FLAG_WAYFARER_SEVII_METEORITE_DELIVERED));
    EXPECT(!FlagGet(FLAG_WAYFARER_SEVII_METEORITE_DELIVERED));
    EXPECT(AddBagItem(ITEM_METEORITE, 1));
    EXPECT(WayfarerSeviiTryExchangeItemForRewardThenSetFlags(ITEM_METEORITE, ITEM_MOON_STONE,
                                                               FLAG_WAYFARER_SEVII_METEORITE_DELIVERED,
                                                               FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED));
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_METEORITE_DELIVERED));
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED));
    EXPECT(!CheckBagHasItem(ITEM_METEORITE, 1));
    EXPECT(CheckBagHasItem(ITEM_MOON_STONE, 1));
}

TEST("Wayfarer Sevii party helpers and Selphy keep a pending request until reward delivery")
{
    u8 friendship = 201;

    WayfarerSeviiInitPersistentState();
    ClearBag();
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_MEOWTH, 20, 0, OTID_STRUCT_PLAYER_ID);
    SetMonData(&gPlayerParty[0], MON_DATA_FRIENDSHIP, &friendship);
    gPlayerPartyCount = 1;

    EXPECT(WayfarerSeviiHasPartyCapacity());
    EXPECT(WayfarerSeviiPartyHasSpecies(SPECIES_MEOWTH));
    EXPECT(WayfarerSeviiPartyMonHasSpecies(0, SPECIES_MEOWTH));
    EXPECT(WayfarerSeviiPartyHasFriendshipAtLeast(201));
    EXPECT(!WayfarerSeviiPartyHasFriendshipAtLeast(202));

    EXPECT(WayfarerSeviiStartSelphyRequest(SPECIES_MEOWTH, ITEM_NUGGET));
    EXPECT(WayfarerSeviiHasActiveSelphyRequest());
    EXPECT(!WayfarerSeviiIsSelphyRequestedSpecies(SPECIES_PIDGEY));
    EXPECT(WayfarerSeviiIsPartyMonSelphyRequested(0));
    EXPECT(WayfarerSeviiTryClaimSelphyPendingReward());
    EXPECT(CheckBagHasItem(ITEM_NUGGET, 1));
    EXPECT(!WayfarerSeviiHasActiveSelphyRequest());
    EXPECT_EQ(VarGet(VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD), ITEM_NONE);
}

TEST("Wayfarer Sevii Selphy sampler freezes the source candidate and reward draw")
{
    u16 species;
    u16 reward;

    ResetPokedex();
    GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_MEOWTH), FLAG_SET_SEEN);
    WayfarerSeviiInitPersistentState();
    SeedRng(0x4C2A);
    EXPECT(WayfarerSeviiSampleSelphyRequest());
    species = VarGet(VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES);
    reward = VarGet(VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD);
    EXPECT_EQ(species, SPECIES_MEOWTH);
    EXPECT(IsSelphySourceReward(reward));
    EXPECT(!WayfarerSeviiSampleSelphyRequest());
    EXPECT_EQ(VarGet(VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES), species);
    EXPECT_EQ(VarGet(VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD), reward);

    WayfarerSeviiInitPersistentState();
    SeedRng(0x4C2A);
    EXPECT(WayfarerSeviiSampleSelphyRequest());
    EXPECT_EQ(VarGet(VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES), species);
    EXPECT_EQ(VarGet(VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD), reward);

    ResetPokedex();
    WayfarerSeviiInitPersistentState();
    EXPECT(!WayfarerSeviiSampleSelphyRequest());
    EXPECT(!WayfarerSeviiHasActiveSelphyRequest());
}

TEST("Wayfarer Sevii Egg receipt remains a party-only capacity transaction")
{
    WayfarerSeviiInitPersistentState();
    ZeroPlayerPartyMons();
    gPlayerPartyCount = 0;

    EXPECT(WayfarerSeviiTryGiveEggThenSetFlag(SPECIES_TOGEPI, FLAG_WAYFARER_SEVII_EGG_RECEIVED));
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_EGG_RECEIVED));
    EXPECT_EQ(gPlayerPartyCount, 1);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), SPECIES_TOGEPI);
    EXPECT(GetMonData(&gPlayerParty[0], MON_DATA_IS_EGG));
}

TEST("Wayfarer Sevii Moltres uses TR 55 and only resolves after a win or catch")
{
    WayfarerSeviiInitPersistentState();
    SetTrainerRating(WAYFARER_BIRD_CAPTURE_TR - 1);
    EXPECT(!WayfarerSeviiIsMoltresEligible());

    SetTrainerRating(WAYFARER_BIRD_CAPTURE_TR);
    EXPECT(WayfarerSeviiIsMoltresEligible());
    gBattleOutcome = B_OUTCOME_RAN;
    EXPECT(!WayfarerSeviiResolveMoltresBattle());
    EXPECT(!FlagGet(FLAG_WAYFARER_SEVII_MOLTRES_RESOLVED));

    gBattleOutcome = B_OUTCOME_CAUGHT;
    EXPECT(WayfarerSeviiResolveMoltresBattle());
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_MOLTRES_RESOLVED));
    EXPECT(!WayfarerSeviiIsMoltresEligible());
}

TEST("Wayfarer Sevii rival eligibility accepts any completed League")
{
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    SetGameClearStateForRegion(REGION_HOENN, FALSE);
    EXPECT(!WayfarerSeviiHasAnyLeagueClear());
    EXPECT(!WayfarerSeviiIsMainlandGiovanniComplete());

    SetGameClearStateForRegion(REGION_HOENN, TRUE);
    EXPECT(WayfarerSeviiHasAnyLeagueClear());
    SetGameClearStateForRegion(REGION_HOENN, FALSE);
}

TEST("Wayfarer Sevii Heracross record uses saved slot four and new-game default")
{
    WayfarerSeviiInitPersistentState();
    VarSet(VAR_WAYFARER_SEVII_HERACROSS_SIZE_RECORD, 1);
    InitHeracrossSizeRecord();
    EXPECT_EQ(VarGet(VAR_WAYFARER_SEVII_HERACROSS_SIZE_RECORD), 0x8000);
    EXPECT_EQ(VarGet(WAYFARER_SEVII_VAR_ID(4)), 0x8000);

    EXPECT(WayfarerConfirmPendingOrigin(ORIGIN_NEW_BARK));
    EXPECT(WayfarerConfirmPendingAppearance(APPEARANCE_GOLD));
    VarSet(VAR_WAYFARER_SEVII_HERACROSS_SIZE_RECORD, 1);
    NewGameInitData();
    EXPECT_EQ(VarGet(VAR_WAYFARER_SEVII_HERACROSS_SIZE_RECORD), 0x8000);
}

static void SetCapeBrinkLead(u16 species)
{
    u8 friendship = 255;

    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], species, 50, 0, OTID_STRUCT_PLAYER_ID);
    SetMonData(&gPlayerParty[0], MON_DATA_FRIENDSHIP, &friendship);
    gPlayerPartyCount = 1;
}

TEST("Wayfarer Cape Brink tutor tracks each reward in Sevii flags 47 through 49")
{
    WayfarerSeviiInitPersistentState();
    FlagClear(FLAG_WAYFARER_SEVII_TUTOR_FRENZY_PLANT);
    FlagClear(FLAG_WAYFARER_SEVII_TUTOR_BLAST_BURN);
    FlagClear(FLAG_WAYFARER_SEVII_TUTOR_HYDRO_CANNON);

    SetCapeBrinkLead(SPECIES_VENUSAUR);
    EXPECT(CapeBrinkGetMoveToTeachLeadPokemon());
    EXPECT_EQ(gSpecialVar_0x8005, MOVE_FRENZY_PLANT);
    EXPECT(!HasLearnedAllMovesFromCapeBrinkTutor());
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_TUTOR_FRENZY_PLANT));
    EXPECT(!FlagGet(FLAG_WAYFARER_SEVII_TUTOR_BLAST_BURN));
    EXPECT(!FlagGet(FLAG_WAYFARER_SEVII_TUTOR_HYDRO_CANNON));
    EXPECT(!CapeBrinkGetMoveToTeachLeadPokemon());

    SetCapeBrinkLead(SPECIES_CHARIZARD);
    EXPECT(CapeBrinkGetMoveToTeachLeadPokemon());
    EXPECT_EQ(gSpecialVar_0x8005, MOVE_BLAST_BURN);
    EXPECT(!HasLearnedAllMovesFromCapeBrinkTutor());
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_TUTOR_BLAST_BURN));

    SetCapeBrinkLead(SPECIES_BLASTOISE);
    EXPECT(CapeBrinkGetMoveToTeachLeadPokemon());
    EXPECT_EQ(gSpecialVar_0x8005, MOVE_HYDRO_CANNON);
    EXPECT(HasLearnedAllMovesFromCapeBrinkTutor());
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_TUTOR_HYDRO_CANNON));
}

#endif // IS_WAYFARER

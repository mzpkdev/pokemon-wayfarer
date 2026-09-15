#include "global.h"
#include "battle.h"
#include "challenge_menu.h"
#include "daycare.h"
#include "event_data.h"
#include "item.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "wayfarer_sevii_story.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/pokedex.h"
#include "constants/regions.h"
#include "constants/vars.h"

#if IS_WAYFARER

static bool8 IsSeviiReceiptFlag(u16 flag)
{
    return IS_WAYFARER_SEVII_FLAG_ID(flag)
        && WAYFARER_SEVII_ID_INDEX(flag) < WAYFARER_SEVII_FLAG_COUNT;
}

static bool8 IsStoryItem(u16 item)
{
    return item != ITEM_NONE && item < ITEMS_COUNT;
}

static bool8 IsSelphyRequestSpecies(u16 species)
{
    return species != SPECIES_NONE
        && IsSpeciesEnabled(species)
        && GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN);
}

static bool8 HasSelphyRequestSpecies(void)
{
    u16 species;

    for (species = SPECIES_BULBASAUR; species < NUM_SPECIES; species++)
    {
        if (IsSelphyRequestSpecies(species))
            return TRUE;
    }
    return FALSE;
}

static u16 SampleSelphyRequestSpecies(void)
{
    u16 i;
    u16 species = SPECIES_NONE;

    // Preserve the source's 100 random probes and descending wraparound.
    // HasSelphyRequestSpecies guarantees that its fallback terminates.
    for (i = 0; i < 100; i++)
    {
        species = (Random() % (NUM_SPECIES - 1)) + 1;
        if (IsSelphyRequestSpecies(species))
            return species;
    }

    while (!IsSelphyRequestSpecies(species))
    {
        if (species == SPECIES_BULBASAUR)
            species = NUM_SPECIES - 1;
        else
            species--;
    }
    return species;
}

static const u16 sSelphyDeluxeRewards[] =
{
    ITEM_BIG_PEARL,
    ITEM_PEARL,
    ITEM_STARDUST,
    ITEM_STAR_PIECE,
    ITEM_NUGGET,
    ITEM_RARE_CANDY,
};

static u16 SampleSelphyReward(void)
{
    if ((Random() % 100) >= 30)
        return ITEM_LUXURY_BALL;
    return sSelphyDeluxeRewards[Random() % ARRAY_COUNT(sSelphyDeluxeRewards)];
}

bool8 WayfarerSeviiPartyHasSpecies(u16 species)
{
    u8 i;

    if (species == SPECIES_NONE)
        return FALSE;

    for (i = 0; i < gPlayerPartyCount; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == species)
            return TRUE;
    }
    return FALSE;
}

bool8 WayfarerSeviiPartyMonHasSpecies(u8 partyIndex, u16 species)
{
    return partyIndex < gPlayerPartyCount
        && species != SPECIES_NONE
        && GetMonData(&gPlayerParty[partyIndex], MON_DATA_SPECIES) == species;
}

bool8 WayfarerSeviiPartyHasFriendshipAtLeast(u8 friendship)
{
    u8 i;

    for (i = 0; i < gPlayerPartyCount; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES)
         && !GetMonData(&gPlayerParty[i], MON_DATA_SANITY_IS_EGG)
         && GetMonData(&gPlayerParty[i], MON_DATA_FRIENDSHIP) >= friendship)
            return TRUE;
    }
    return FALSE;
}

bool8 WayfarerSeviiHasPartyCapacity(void)
{
    return gPlayerPartyCount < GetMaxPartySize();
}

bool8 WayfarerSeviiTryGiveItemThenSetFlag(u16 item, u16 receiptFlag)
{
    if (!IsStoryItem(item) || !IsSeviiReceiptFlag(receiptFlag))
        return FALSE;
    if (FlagGet(receiptFlag))
        return TRUE;
    if (!AddBagItem(item, 1))
        return FALSE;

    FlagSet(receiptFlag);
    return TRUE;
}

bool8 WayfarerSeviiTryRemoveItemThenSetFlag(u16 item, u16 receiptFlag)
{
    if (!IsStoryItem(item) || !IsSeviiReceiptFlag(receiptFlag))
        return FALSE;
    if (FlagGet(receiptFlag))
        return TRUE;
    if (!CheckBagHasItem(item, 1) || !RemoveBagItem(item, 1))
        return FALSE;

    FlagSet(receiptFlag);
    return TRUE;
}

bool8 WayfarerSeviiTryExchangeItemForReward(u16 sourceItem, u16 rewardItem, u16 receiptFlag)
{
    if (!IsStoryItem(sourceItem) || !IsStoryItem(rewardItem)
     || sourceItem == rewardItem || !IsSeviiReceiptFlag(receiptFlag))
        return FALSE;
    if (FlagGet(receiptFlag))
        return TRUE;
    if (!CheckBagHasItem(sourceItem, 1))
        return FALSE;

    // Reserve the destination before consuming the source. This keeps a full
    // reward pocket retryable and avoids narrating a handoff that did not land.
    if (!AddBagItem(rewardItem, 1))
        return FALSE;
    if (!RemoveBagItem(sourceItem, 1))
    {
        RemoveBagItem(rewardItem, 1);
        return FALSE;
    }

    FlagSet(receiptFlag);
    return TRUE;
}

bool8 WayfarerSeviiTryExchangeItemForRewardThenSetFlags(u16 sourceItem, u16 rewardItem, u16 deliveryFlag, u16 rewardReceiptFlag)
{
    if (!IsStoryItem(sourceItem) || !IsStoryItem(rewardItem)
     || sourceItem == rewardItem || !IsSeviiReceiptFlag(deliveryFlag)
     || !IsSeviiReceiptFlag(rewardReceiptFlag) || deliveryFlag == rewardReceiptFlag)
        return FALSE;
    if (FlagGet(deliveryFlag) && FlagGet(rewardReceiptFlag))
        return TRUE;
    if (FlagGet(deliveryFlag) || FlagGet(rewardReceiptFlag) || !CheckBagHasItem(sourceItem, 1))
        return FALSE;
    if (!AddBagItem(rewardItem, 1))
        return FALSE;
    if (!RemoveBagItem(sourceItem, 1))
    {
        RemoveBagItem(rewardItem, 1);
        return FALSE;
    }

    FlagSet(deliveryFlag);
    FlagSet(rewardReceiptFlag);
    return TRUE;
}

bool8 WayfarerSeviiTryGiveEggThenSetFlag(u16 species, u16 receiptFlag)
{
    struct Pokemon egg;

    if (species == SPECIES_NONE || species >= NUM_SPECIES || !IsSeviiReceiptFlag(receiptFlag))
        return FALSE;
    if (FlagGet(receiptFlag))
        return TRUE;

    // Unlike ScriptGiveEgg, this story gift must remain pending rather than
    // being redirected to the PC. One-type challenge eligibility is likewise
    // checked before the receipt is committed.
    if (!WayfarerSeviiHasPartyCapacity() || !DoesSpeciesPassOneTypeChallenge(species))
        return FALSE;

    CreateEgg(&egg, species, TRUE);
    if (GiveCapturedMonToPlayer(&egg) != MON_GIVEN_TO_PARTY)
        return FALSE;

    FlagSet(receiptFlag);
    return TRUE;
}

bool8 WayfarerSeviiStartSelphyRequest(u16 species, u16 rewardItem)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES || !IsStoryItem(rewardItem)
     || WayfarerSeviiHasActiveSelphyRequest())
        return FALSE;

    VarSet(VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES, species);
    VarSet(VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD, rewardItem);
    VarSet(VAR_WAYFARER_SEVII_SELPHY_REQUEST_ACTIVE, TRUE);
    return TRUE;
}

bool8 WayfarerSeviiSampleSelphyRequest(void)
{
    u16 species;

    if (WayfarerSeviiHasActiveSelphyRequest() || !HasSelphyRequestSpecies())
        return FALSE;

    species = SampleSelphyRequestSpecies();
    return WayfarerSeviiStartSelphyRequest(species, SampleSelphyReward());
}

bool8 WayfarerSeviiHasActiveSelphyRequest(void)
{
    u16 species = VarGet(VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES);
    u16 reward = VarGet(VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD);

    return VarGet(VAR_WAYFARER_SEVII_SELPHY_REQUEST_ACTIVE) != 0
        && species != SPECIES_NONE && species < NUM_SPECIES
        && IsStoryItem(reward);
}

bool8 WayfarerSeviiIsSelphyRequestedSpecies(u16 species)
{
    return WayfarerSeviiHasActiveSelphyRequest()
        && species == VarGet(VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES);
}

bool8 WayfarerSeviiIsPartyMonSelphyRequested(u8 partyIndex)
{
    return partyIndex < gPlayerPartyCount
        && WayfarerSeviiIsSelphyRequestedSpecies(GetMonData(&gPlayerParty[partyIndex], MON_DATA_SPECIES));
}

bool8 WayfarerSeviiTryClaimSelphyPendingReward(void)
{
    u16 reward;

    if (!WayfarerSeviiHasActiveSelphyRequest())
        return FALSE;

    reward = VarGet(VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD);
    if (!AddBagItem(reward, 1))
        return FALSE;

    // Clear only after the reward exists in the Bag, so a full pocket leaves
    // the same request and reward available to claim on a later visit.
    VarSet(VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES, SPECIES_NONE);
    VarSet(VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD, ITEM_NONE);
    VarSet(VAR_WAYFARER_SEVII_SELPHY_REQUEST_ACTIVE, FALSE);
    return TRUE;
}

bool8 WayfarerSeviiHasAnyLeagueClear(void)
{
    return GetGameClearStateForRegion(REGION_KANTO)
        || GetGameClearStateForRegion(REGION_JOHTO)
        || GetGameClearStateForRegion(REGION_HOENN);
}

bool8 WayfarerSeviiIsMainlandGiovanniComplete(void)
{
    // The current Wayfarer mainland import does not publish a dedicated
    // Giovanni milestone. A Kanto League clear is not a substitute: until its
    // owner exposes that semantic milestone, the non-recognition variant is
    // the only truthful dialogue branch.
    return FALSE;
}

bool8 WayfarerSeviiIsMoltresEligible(void)
{
    return !FlagGet(FLAG_WAYFARER_SEVII_MOLTRES_RESOLVED)
        && GetTrainerRating() >= WAYFARER_BIRD_CAPTURE_TR;
}

bool8 WayfarerSeviiResolveMoltresBattle(void)
{
    if (gBattleOutcome != B_OUTCOME_WON && gBattleOutcome != B_OUTCOME_CAUGHT)
        return FALSE;

    FlagSet(FLAG_WAYFARER_SEVII_MOLTRES_RESOLVED);
    return TRUE;
}

#else

bool8 WayfarerSeviiPartyHasSpecies(u16 species) { (void)species; return FALSE; }
bool8 WayfarerSeviiPartyMonHasSpecies(u8 partyIndex, u16 species) { (void)partyIndex; (void)species; return FALSE; }
bool8 WayfarerSeviiPartyHasFriendshipAtLeast(u8 friendship) { (void)friendship; return FALSE; }
bool8 WayfarerSeviiHasPartyCapacity(void) { return FALSE; }
bool8 WayfarerSeviiTryGiveItemThenSetFlag(u16 item, u16 receiptFlag) { (void)item; (void)receiptFlag; return FALSE; }
bool8 WayfarerSeviiTryRemoveItemThenSetFlag(u16 item, u16 receiptFlag) { (void)item; (void)receiptFlag; return FALSE; }
bool8 WayfarerSeviiTryExchangeItemForReward(u16 sourceItem, u16 rewardItem, u16 receiptFlag) { (void)sourceItem; (void)rewardItem; (void)receiptFlag; return FALSE; }
bool8 WayfarerSeviiTryExchangeItemForRewardThenSetFlags(u16 sourceItem, u16 rewardItem, u16 deliveryFlag, u16 rewardReceiptFlag) { (void)sourceItem; (void)rewardItem; (void)deliveryFlag; (void)rewardReceiptFlag; return FALSE; }
bool8 WayfarerSeviiTryGiveEggThenSetFlag(u16 species, u16 receiptFlag) { (void)species; (void)receiptFlag; return FALSE; }
bool8 WayfarerSeviiStartSelphyRequest(u16 species, u16 rewardItem) { (void)species; (void)rewardItem; return FALSE; }
bool8 WayfarerSeviiSampleSelphyRequest(void) { return FALSE; }
bool8 WayfarerSeviiHasActiveSelphyRequest(void) { return FALSE; }
bool8 WayfarerSeviiIsSelphyRequestedSpecies(u16 species) { (void)species; return FALSE; }
bool8 WayfarerSeviiIsPartyMonSelphyRequested(u8 partyIndex) { (void)partyIndex; return FALSE; }
bool8 WayfarerSeviiTryClaimSelphyPendingReward(void) { return FALSE; }
bool8 WayfarerSeviiHasAnyLeagueClear(void) { return FALSE; }
bool8 WayfarerSeviiIsMainlandGiovanniComplete(void) { return FALSE; }
bool8 WayfarerSeviiIsMoltresEligible(void) { return FALSE; }
bool8 WayfarerSeviiResolveMoltresBattle(void) { return FALSE; }

#endif

void WayfarerSevii_HasPartyCapacity(void)
{
    gSpecialVar_Result = WayfarerSeviiHasPartyCapacity();
}

void WayfarerSevii_PartyHasSpecies(void)
{
    gSpecialVar_Result = WayfarerSeviiPartyHasSpecies(gSpecialVar_0x8004);
}

void WayfarerSevii_PartyHasFriendshipAtLeast(void)
{
    gSpecialVar_Result = WayfarerSeviiPartyHasFriendshipAtLeast(gSpecialVar_0x8004);
}

void WayfarerSevii_TryGiveItemThenSetFlag(void)
{
    gSpecialVar_Result = WayfarerSeviiTryGiveItemThenSetFlag(gSpecialVar_0x8004, gSpecialVar_0x8005);
}

void WayfarerSevii_TryRemoveItemThenSetFlag(void)
{
    gSpecialVar_Result = WayfarerSeviiTryRemoveItemThenSetFlag(gSpecialVar_0x8004, gSpecialVar_0x8005);
}

void WayfarerSevii_TryExchangeItemForReward(void)
{
    gSpecialVar_Result = WayfarerSeviiTryExchangeItemForReward(gSpecialVar_0x8004, gSpecialVar_0x8005, gSpecialVar_0x8006);
}

void WayfarerSevii_TryExchangeItemForRewardThenSetFlags(void)
{
    gSpecialVar_Result = WayfarerSeviiTryExchangeItemForRewardThenSetFlags(gSpecialVar_0x8004, gSpecialVar_0x8005, gSpecialVar_0x8006, gSpecialVar_0x8007);
}

void WayfarerSevii_TryGiveEggThenSetFlag(void)
{
    gSpecialVar_Result = WayfarerSeviiTryGiveEggThenSetFlag(gSpecialVar_0x8004, gSpecialVar_0x8005);
}

void WayfarerSevii_StartSelphyRequest(void)
{
    gSpecialVar_Result = WayfarerSeviiStartSelphyRequest(gSpecialVar_0x8004, gSpecialVar_0x8005);
}

void WayfarerSevii_SampleSelphyRequest(void)
{
    gSpecialVar_Result = WayfarerSeviiSampleSelphyRequest();
}

void WayfarerSevii_HasActiveSelphyRequest(void)
{
    gSpecialVar_Result = WayfarerSeviiHasActiveSelphyRequest();
}

void WayfarerSevii_IsSelectedSpeciesRequested(void)
{
    gSpecialVar_Result = WayfarerSeviiIsPartyMonSelphyRequested(gSpecialVar_0x8004);
}

void WayfarerSevii_TryClaimSelphyPendingReward(void)
{
    gSpecialVar_Result = WayfarerSeviiTryClaimSelphyPendingReward();
}

void WayfarerSevii_HasAnyLeagueClear(void)
{
    gSpecialVar_Result = WayfarerSeviiHasAnyLeagueClear();
}

void WayfarerSevii_IsMainlandGiovanniComplete(void)
{
    gSpecialVar_Result = WayfarerSeviiIsMainlandGiovanniComplete();
}

void WayfarerSevii_IsMoltresEligible(void)
{
    gSpecialVar_Result = WayfarerSeviiIsMoltresEligible();
}

void WayfarerSevii_ResolveMoltresBattle(void)
{
    gSpecialVar_Result = WayfarerSeviiResolveMoltresBattle();
}

#ifndef GUARD_WAYFARER_SEVII_STORY_H
#define GUARD_WAYFARER_SEVII_STORY_H

#include "global.h"

// The three Kanto birds share this readiness policy. It is deliberately a
// threshold rather than a cost, and it is independent of story completion.
#define WAYFARER_BIRD_CAPTURE_TR 55

bool8 WayfarerSeviiPartyHasSpecies(u16 species);
bool8 WayfarerSeviiPartyMonHasSpecies(u8 partyIndex, u16 species);
bool8 WayfarerSeviiPartyHasFriendshipAtLeast(u8 friendship);
bool8 WayfarerSeviiHasPartyCapacity(void);

// These helpers own the order of every persistent story transaction. A
// completed receipt is an idempotent success; otherwise the receipt is written
// only after its item or Egg delivery has succeeded.
bool8 WayfarerSeviiTryGiveItemThenSetFlag(u16 item, u16 receiptFlag);
bool8 WayfarerSeviiTryRemoveItemThenSetFlag(u16 item, u16 receiptFlag);
bool8 WayfarerSeviiTryExchangeItemForReward(u16 sourceItem, u16 rewardItem, u16 receiptFlag);
bool8 WayfarerSeviiTryExchangeItemForRewardThenSetFlags(u16 sourceItem, u16 rewardItem, u16 deliveryFlag, u16 rewardReceiptFlag);
bool8 WayfarerSeviiTryGiveEggThenSetFlag(u16 species, u16 receiptFlag);

bool8 WayfarerSeviiStartSelphyRequest(u16 species, u16 rewardItem);
bool8 WayfarerSeviiSampleSelphyRequest(void);
bool8 WayfarerSeviiHasActiveSelphyRequest(void);
bool8 WayfarerSeviiIsSelphyRequestedSpecies(u16 species);
bool8 WayfarerSeviiIsPartyMonSelphyRequested(u8 partyIndex);
bool8 WayfarerSeviiTryClaimSelphyPendingReward(void);

bool8 WayfarerSeviiHasAnyLeagueClear(void);
bool8 WayfarerSeviiIsMainlandGiovanniComplete(void);
bool8 WayfarerSeviiIsMoltresEligible(void);
bool8 WayfarerSeviiResolveMoltresBattle(void);

// Script specials. Inputs use VAR_0x8004 onward and write VAR_RESULT.
void WayfarerSevii_HasPartyCapacity(void);
void WayfarerSevii_PartyHasSpecies(void);
void WayfarerSevii_PartyHasFriendshipAtLeast(void);
void WayfarerSevii_TryGiveItemThenSetFlag(void);
void WayfarerSevii_TryRemoveItemThenSetFlag(void);
void WayfarerSevii_TryExchangeItemForReward(void);
void WayfarerSevii_TryExchangeItemForRewardThenSetFlags(void);
void WayfarerSevii_TryGiveEggThenSetFlag(void);
void WayfarerSevii_StartSelphyRequest(void);
void WayfarerSevii_SampleSelphyRequest(void);
void WayfarerSevii_HasActiveSelphyRequest(void);
void WayfarerSevii_IsSelectedSpeciesRequested(void);
void WayfarerSevii_TryClaimSelphyPendingReward(void);
void WayfarerSevii_HasAnyLeagueClear(void);
void WayfarerSevii_IsMainlandGiovanniComplete(void);
void WayfarerSevii_IsMoltresEligible(void);
void WayfarerSevii_ResolveMoltresBattle(void);

#endif // GUARD_WAYFARER_SEVII_STORY_H

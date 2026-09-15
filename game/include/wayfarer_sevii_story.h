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

// Script specials. Inputs use VAR_0x8004 onward; specialvar stores the u16 return.
u16 WayfarerSevii_HasPartyCapacity(void);
u16 WayfarerSevii_PartyHasSpecies(void);
u16 WayfarerSevii_PartyHasFriendshipAtLeast(void);
u16 WayfarerSevii_TryGiveItemThenSetFlag(void);
u16 WayfarerSevii_TryRemoveItemThenSetFlag(void);
u16 WayfarerSevii_TryExchangeItemForReward(void);
u16 WayfarerSevii_TryExchangeItemForRewardThenSetFlags(void);
u16 WayfarerSevii_TryGiveEggThenSetFlag(void);
u16 WayfarerSevii_StartSelphyRequest(void);
u16 WayfarerSevii_SampleSelphyRequest(void);
u16 WayfarerSevii_HasActiveSelphyRequest(void);
u16 WayfarerSevii_IsSelectedSpeciesRequested(void);
u16 WayfarerSevii_TryClaimSelphyPendingReward(void);
u16 WayfarerSevii_HasAnyLeagueClear(void);
u16 WayfarerSevii_IsMainlandGiovanniComplete(void);
u16 WayfarerSevii_IsMoltresEligible(void);
u16 WayfarerSevii_ResolveMoltresBattle(void);

#endif // GUARD_WAYFARER_SEVII_STORY_H

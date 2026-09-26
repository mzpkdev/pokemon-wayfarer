#ifndef GUARD_WAYFARER_KANTO_OPENING_H
#define GUARD_WAYFARER_KANTO_OPENING_H

#include "global.h"
#include "constants/wayfarer_kanto_opening.h"

#if IS_WAYFARER
void WayfarerKanto_InitializeOpening(void);
u16 WayfarerKanto_IsPalletOrigin(void);
u16 WayfarerKanto_GetPhase(void);
u16 WayfarerKanto_GetStarterSlot(void);
u16 WayfarerKanto_GetCounterSlot(void);
u16 WayfarerKanto_GetSelectedStarterSpecies(void);
void WayfarerKanto_BufferSelectedStarterType(void);
u16 WayfarerKanto_HasReceipt(void);
u16 WayfarerKanto_IsOpeningComplete(void);
u16 WayfarerKanto_HasFirstBattleResolved(void);
u16 WayfarerKanto_GetRemainingSlot(void);
u16 WayfarerKanto_StageLab(void);
u16 WayfarerKanto_TryStageInterception(void);
u16 WayfarerKanto_CommitLabStarterChoice(void);
u16 WayfarerKanto_CommitStarterSlot(void);
u16 WayfarerKanto_TryGiveStarter(void);
u16 WayfarerKanto_ResolveFirstBattle(void);
u16 WayfarerKanto_TryReceiveParcel(void);
u16 WayfarerKanto_TryAcceptParcel(void);
u16 WayfarerKanto_CommitBlueArrival(void);
u16 WayfarerKanto_TryGrantPokedex(void);
u16 WayfarerKanto_TryGrantPokeBalls(void);
u16 WayfarerKanto_CommitPresentationDone(void);
u16 WayfarerKanto_CompleteOpening(void);
#endif

#endif

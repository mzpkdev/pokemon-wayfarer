#ifndef GUARD_WAYFARER_ORIGIN_H
#define GUARD_WAYFARER_ORIGIN_H

#include "global.h"
#include "constants/regions.h"
#include "constants/wayfarer_origin.h"

#if IS_WAYFARER
struct WayfarerOriginProfile
{
    u16 id;
    const u8 *selectionLabel;
    enum Region entryRegion;
    s8 mapGroup, mapNum, warpId;
    s16 x, y;
    u8 scenePolicies[ORIGIN_SCENES_COUNT];
    u8 (*initialRecovery)(void);
    void (*initialize)(void);
    void (*openingCallback)(void);
    const u8 *(*authoredScene)(u8 scene);
    bool8 (*regularAqua)(void);
    bool8 (*slateportTicket)(void);
    bool8 maidenVoyage;
};

void WayfarerResetPendingOrigin(void);
bool8 WayfarerSetPendingOriginCandidate(u16 id);
u16 WayfarerGetPendingOriginCandidate(void);
bool8 WayfarerConfirmPendingOrigin(u16 id);
u16 WayfarerGetConfirmedPendingOrigin(void);
const struct WayfarerOriginProfile *WayfarerGetOriginProfile(u16 id);
u16 WayfarerGetStartingOriginId(void);
bool8 WayfarerInitializeOrigin(u16 id);
u8 WayfarerGetOriginScenePolicy(u8 scene);
u16 WayfarerDispatchOriginScene(void);
void WayfarerEnterOriginOpening(void);
u16 WayfarerUsesNativeJohtoHousehold(void);
u16 WayfarerUsesNativeHoennHousehold(void);
u16 WayfarerUsesNativeJohtoOpening(void);
u16 WayfarerUsesNativeHoennOpening(void);
u16 WayfarerCanUseRegularAqua(void);
u16 WayfarerCanReceiveSlateportTicket(void);
u16 WayfarerCanUseAquaMaidenVoyage(void);
void WayfarerGrantSharedEquipment(void);
void WayfarerGrantRunningShoes(void);
u16 WayfarerHasSharedPokedex(void);
void WayfarerGrantSharedPokedex(void);
bool8 WayfarerEnsureRecoveryDestination(void);
bool8 WayfarerReplaceRecoveryDestination(u8 healLocationId);
u8 WayfarerGetInitialRecoveryDestination(void);
#if TESTING
bool8 Test_WayfarerRegisterOriginProfile(const struct WayfarerOriginProfile *profile);
#endif
#endif
#endif

#ifndef GUARD_WAYFARER_APPEARANCE_H
#define GUARD_WAYFARER_APPEARANCE_H

#include "global.h"
#include "constants/wayfarer_appearance.h"

#if IS_WAYFARER
struct WayfarerAppearanceProfile
{
    u8 id;
    u8 gender;
    u16 frontPic;
    u16 backPic;
    u16 paletteTag;
    u16 graphicsIds[PLAYER_AVATAR_STATE_VSSEEKER + 1];
};

const struct WayfarerAppearanceProfile *WayfarerGetAppearanceProfile(u8 id);
u8 WayfarerGetPlayerAppearanceId(void);
u16 WayfarerGetAppearanceGraphicsId(u8 id, u8 state);
u16 WayfarerGetAppearanceFrontPic(u8 id);
u16 WayfarerGetAppearanceBackPic(u8 id);
u16 WayfarerGetAppearancePaletteTag(u8 id);
void WayfarerResetPendingAppearance(void);
bool8 WayfarerSetPendingAppearanceCandidate(u8 id);
u8 WayfarerGetPendingAppearanceCandidate(void);
bool8 WayfarerConfirmPendingAppearance(u8 id);
u8 WayfarerGetConfirmedPendingAppearance(void);
#endif
#endif

#ifndef GUARD_BATTLE_PYRAMID_H
#define GUARD_BATTLE_PYRAMID_H

#include "map_layout.h"
#include "constants/battle_pyramid.h"

void CallBattlePyramidFunction(void);
u16 LocalIdToPyramidTrainerId(u8 localId);
bool8 GetBattlePyramidTrainerFlag(u8 eventId);
void MarkApproachingPyramidTrainersAsBattled(void);
void GenerateBattlePyramidWildMon(void);
u8 GetPyramidRunMultiplier(void);
u8 CurrentBattlePyramidLocation(void);
bool8 InBattlePyramid_(void);
void PausePyramidChallenge(void);
void SoftResetInBattlePyramid(void);
void CopyPyramidTrainerSpeechBefore(u16 trainerId);
void CopyPyramidTrainerWinSpeech(u16 trainerId);
void CopyPyramidTrainerLoseSpeech(u16 trainerId);
u8 GetTrainerEncounterMusicIdInBattlePyramid(u16 trainerId);
enum MapLayoutLoadError GenerateBattlePyramidFloorLayout(u16 *backupMapData, bool8 setPlayerPosition);
#if TESTING && IS_WAYFARER
enum MapLayoutLoadError Test_GenerateBattlePyramidFloorLayout(u16 *backupMapData,
    u8 templateId, const u8 *layoutOptions, u8 entranceSquareId, u8 exitSquareId,
    bool8 setPlayerPosition);
#endif
void LoadBattlePyramidObjectEventTemplates(void);
void LoadBattlePyramidFloorObjectEventScripts(void);
u8 GetNumBattlePyramidObjectEvents(void);
u16 GetBattlePyramidPickupItemId(void);

#endif // GUARD_BATTLE_PYRAMID_H

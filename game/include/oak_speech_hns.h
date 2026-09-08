#ifndef GUARD_OAK_SPEECH_HNS_H
#define GUARD_OAK_SPEECH_HNS_H

void StartNewGameSceneHns(void);

#if IS_WAYFARER && defined(E2E_TESTING) && E2E_TESTING
u8 E2ETest_GetOriginIntroStage(void);
#endif

#endif //GUARD_OAK_SPEECH_HNS_H

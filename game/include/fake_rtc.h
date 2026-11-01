#ifndef GUARD_FAKE_RTC_UTIL_H
#define GUARD_FAKE_RTC_UTIL_H

#include "siirtc.h"
#include "config/overworld.h"

// Wayfarer's in-game clock (.product/specs/in-game-clock.md): 12 in-game seconds
// pass for every second of play, so an in-game day lasts 120 minutes of play.
#define WAYFARER_CLOCK_SECONDS_RATIO 12
#define WAYFARER_CLOCK_START_HOUR 8

static inline bool32 UseFakeRtc(void)
{
    return OW_USE_FAKE_RTC;
}

void FakeRtc_Reset(void);
struct SiiRtcInfo* FakeRtc_GetCurrentTime(void);
void FakeRtc_GetRawInfo(struct SiiRtcInfo *rtc);
void FakeRtc_AdvanceTimeBy(u32 days, u32 hours, u32 minutes, u32 seconds);
void FakeRtc_ManuallySetTime(u32 day, u32 hour, u32 minute, u32 second);
void FakeRtc_ForwardTimeTo(u32 hour, u32 minute, u32 second);
void FakeRtc_TickTimeForward(void);
u32 FakeRtc_GetSecondsRatio(void);
void Script_PauseFakeRtc(void);
void Script_ResumeFakeRtc(void);
void Script_ToggleFakeRtc(void);

#endif // GUARD_FAKE_RTC_UTIL_H

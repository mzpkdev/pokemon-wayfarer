#ifndef GUARD_CAPTURE_CONTEXT_H
#define GUARD_CAPTURE_CONTEXT_H

struct CaptureContext
{
    bool8 hasPlayerBattler;
    u8 playerBattler;
    u8 completedTurns;
    u8 initialCatchFactor;
    u8 catchFactor;
};

static inline u32 CaptureInitialCatchFactor(u32 speciesCatchRate)
{
    u32 factor = speciesCatchRate * 100 / 1275;
    return factor == 0 ? 1 : factor;
}

static inline u32 CaptureApplyProximity(u32 speciesCatchRate, u32 initialFactor, u32 factor)
{
    if (initialFactor == 0)
        return speciesCatchRate;
    return speciesCatchRate * factor / initialFactor;
}

static inline bool8 CaptureAdvanceProximity(u8 *catchFactor, u8 *escapeFactor, u8 *approach)
{
    static const u8 increments[] = {4, 3, 2, 1};
    bool8 closer = *approach < 3;
    *catchFactor = min(20, *catchFactor + increments[min(3, *approach)]);
    *escapeFactor = min(20, *escapeFactor + 4);
    if (closer)
        (*approach)++;
    return closer;
}

u32 ComputeCaptureOdds(u32 wildMonBattler, const struct CaptureContext *context);

#endif

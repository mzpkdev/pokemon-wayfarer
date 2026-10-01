#ifndef GUARD_TRAINER_SCALER_H
#define GUARD_TRAINER_SCALER_H

struct TrainerScalerAnchor
{
    u32 tr;
    u16 value;
};

bool32 ValidateTrainerScaler(const struct TrainerScalerAnchor *anchors, u32 count);
// Authored tables must pass validation before use. Rating is never capped;
// each table holds its final value beyond its own last anchor.
u16 EvaluateTrainerScaler(const struct TrainerScalerAnchor *anchors, u32 count, u32 rating, bool32 step);

#endif

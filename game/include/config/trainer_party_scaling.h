#ifndef GUARD_CONFIG_TRAINER_PARTY_SCALING_H
#define GUARD_CONFIG_TRAINER_PARTY_SCALING_H

// Enable only after the generated classification and structural audit pass.
#ifndef B_TRAINER_PARTY_SCALING
#define B_TRAINER_PARTY_SCALING TRUE
#endif

// Gym Leaders use an authored six-slot roster and a distinct level curve.
// Keep this off until the generated roster inventory, structural audit, and
// required playtesting have all been accepted.
#ifndef B_GYM_LEADER_SCALING
#define B_GYM_LEADER_SCALING FALSE
#endif

// League scaling consumes admission TR independently of all other scaling.
#ifndef B_LEAGUE_SCALING
#define B_LEAGUE_SCALING TRUE
#endif

#endif

#ifndef GUARD_CONFIG_LEAGUE_CIRCUIT_H
#define GUARD_CONFIG_LEAGUE_CIRCUIT_H

#include "config/notable_trainers.h"

// Production default for the Johto-start circuit. Builds may set this to FALSE
// as an independent compile-time rollback; it never represents start choice
// eligibility or future Kanto/Hoenn opening work.
#ifndef WAYFARER_LEAGUE_CIRCUIT_ENABLED
#define WAYFARER_LEAGUE_CIRCUIT_ENABLED 1
#endif

// The playable event lifecycle uses the shared notable-trainer model.
#ifndef WAYFARER_LEAGUE_EVENTS
#define WAYFARER_LEAGUE_EVENTS (IS_WAYFARER && WAYFARER_V0_TRAINERS && WAYFARER_LEAGUE_CIRCUIT_ENABLED)
#endif

#endif

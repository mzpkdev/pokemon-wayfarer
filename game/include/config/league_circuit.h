#ifndef GUARD_CONFIG_LEAGUE_CIRCUIT_H
#define GUARD_CONFIG_LEAGUE_CIRCUIT_H

// Production default for the Johto-start circuit. Builds may set this to FALSE
// as an independent compile-time rollback; it never represents start choice
// eligibility or future Kanto/Hoenn opening work.
#ifndef WAYFARER_LEAGUE_CIRCUIT_ENABLED
#define WAYFARER_LEAGUE_CIRCUIT_ENABLED 1
#endif

#endif

#ifndef GUARD_LEAGUE_CIRCUIT_STATUS_H
#define GUARD_LEAGUE_CIRCUIT_STATUS_H

#include "global.h"

// The caller supplies at least LEAGUE_CIRCUIT_STATUS_BUFFER_SIZE bytes.
#define LEAGUE_CIRCUIT_STATUS_BUFFER_SIZE 160

void FormatLeagueCircuitStatus(u8 *dest, bool8 paged);
void BufferLeagueCircuitStatus(void);

#endif

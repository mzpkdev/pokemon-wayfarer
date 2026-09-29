#ifndef GUARD_NOTABLE_MOVES_H
#define GUARD_NOTABLE_MOVES_H

#include "notable_trainers.h"

struct TrainerMon;

// Apply the shared evolution-level table's downward rule to the authored species.
u16 StepDownSpeciesToLevel(u16 species, u8 level);

// Members remain indexed by their stable roster slots. Returns FALSE on invalid input.
bool32 ResolveNotableTrainerMoves(struct TrainerMon *members, u8 count, u8 aceMask,
                                  const struct NotableMovePoolEntry *pool, u32 poolCount,
                                  bool32 skipPool);

#endif

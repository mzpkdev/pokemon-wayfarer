#ifndef GUARD_NOTABLE_MOVES_H
#define GUARD_NOTABLE_MOVES_H

#include "notable_trainers.h"

#define MAX_NOTABLE_MOVE_POOL 32

struct TrainerMon;

// Apply the shared evolution-level table's downward rule to the authored species.
u16 StepDownSpeciesToLevel(u16 species, u8 level);

// The edge into a species for the wild stage rules: its non-baby predecessor and the
// level that edge evolves at (0 if the shared table has none). SPECIES_NONE for a line's first stage.
u16 GetSpeciesStepDownPredecessor(u16 species, u8 *evolutionLevel);
bool32 IsBabySpecies(u16 species);
// The lowest known evolution level among a species' evolutions; 0 if it has none.
u8 GetSpeciesLowestEvolutionLevel(u16 species);

// Members remain indexed by their stable roster slots. Returns FALSE on invalid input.
bool32 ResolveNotableTrainerMoves(struct TrainerMon *members, u8 count, u8 aceMask,
                                  const struct NotableMovePoolEntry *pool, u32 poolCount,
                                  bool32 skipPool);

#endif

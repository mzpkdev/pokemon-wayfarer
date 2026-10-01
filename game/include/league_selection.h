#ifndef GUARD_LEAGUE_SELECTION_H
#define GUARD_LEAGUE_SELECTION_H

#include "global.h"
#include "notable_trainers.h"

#define LEAGUE_LINEUP_SIZE 5

enum LeagueId
{
    LEAGUE_ID_NONE = 0,
    LEAGUE_ID_INDIGO = 1,
    LEAGUE_ID_MASTERS = 2,
    LEAGUE_ID_HOENN = 3,
};

struct LeagueTrainer
{
    u32 characterId;
    u16 sourceTrainerId;
    u16 presentationId;
    u16 objectGraphicsId;
    bool8 enabled;
};

struct LeagueLineupMember
{
    u32 characterId;
    u32 trainerTR;
    u32 leagueScore;
    u8 teamLevel;
    u8 willingness;
};

struct LeagueLineupSelection
{
    struct LeagueLineupMember members[LEAGUE_LINEUP_SIZE]; // ascending TR, then character ID
    u8 count;
};

const struct LeagueTrainer *GetLeagueTrainer(u32 characterId);
const struct LeagueTrainer *GetLeagueTrainerByIndex(u32 index);
u32 GetLeagueTrainerCount(void);
bool32 IsLeagueTrainerRegistryValid(void);
// recentIds is either five zeroes or the five IDs from the latest resolved event.
// Reign masks use bit (characterId - 1). Failure leaves out zeroed.
bool32 SelectLeagueLineup(enum LeagueId leagueId, u32 worldProgress,
                          const u32 recentIds[LEAGUE_LINEUP_SIZE],
                          u64 reignedIndigoMask, u64 reignedHoennMask,
                          struct LeagueLineupSelection *out);

#endif // GUARD_LEAGUE_SELECTION_H

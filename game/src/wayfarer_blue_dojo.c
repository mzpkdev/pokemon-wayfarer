#include "global.h"
#include "event_data.h"
#include "league_circuit.h"
#include "wayfarer_blue_dojo.h"
#include "constants/flags.h"

#if IS_WAYFARER

// FLAG_HIDE_DOJO_BLUE is a projection of the committed first Indigo victory.
// It is recomputed on Dojo entry so League commit and rollback paths never
// need to touch it.
void SyncBlueDojoVisibility(void)
{
    if (HasCommittedFirstIndigoVictory())
        FlagClear(FLAG_HIDE_DOJO_BLUE);
    else
        FlagSet(FLAG_HIDE_DOJO_BLUE);
}

#endif

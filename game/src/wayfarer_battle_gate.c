#include "global.h"
#include "trainer_only_encounter.h"
#include "event_data.h"

bool8 TrainerOnlyCanEnterWildEncounter(void)
{
#if IS_WAYFARER
    return FlagGet(FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS)
        && CalculatePlayerPartyCount() == 0;
#else
    return FALSE;
#endif
}

#include "global.h"
#include "wayfarer_johto_story.h"
#if IS_WAYFARER
#include "event_data.h"
#include "script.h"
#include "wayfarer_origin.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/wayfarer_johto_story.h"

// Special: whether Silver's scene for chapter gSpecialVar_0x8004 can play now. Silver exists
// only for the Johto origin. Starting a battle stores its chapter, which retires every earlier
// scene; a battle chapter is also retired by its own win. Each scene keeps its local condition
// (its location and host progress) in its own script.
u16 WayfarerJohtoSilverAvailable(void)
{
    u16 started = VarGet(VAR_WAYFARER_JOHTO_SILVER_CHAPTER);

    if (!WayfarerIsJohtoOrigin())
        return FALSE;
    switch (gSpecialVar_0x8004)
    {
    case SILVER_CHAPTER_NEW_BARK:
        return started == 0;
    case SILVER_CHAPTER_CHERRYGROVE:
        return started <= SILVER_CHAPTER_CHERRYGROVE && !FlagGet(FLAG_WAYFARER_JOHTO_SILVER_WON_2);
    case SILVER_CHAPTER_SPROUT_TOWER:
        return started < SILVER_CHAPTER_AZALEA;
    case SILVER_CHAPTER_AZALEA:
        return started <= SILVER_CHAPTER_AZALEA && !FlagGet(FLAG_WAYFARER_JOHTO_SILVER_WON_4);
    case SILVER_CHAPTER_BURNED_TOWER:
        return started <= SILVER_CHAPTER_BURNED_TOWER && !FlagGet(FLAG_WAYFARER_JOHTO_SILVER_WON_5);
    case SILVER_CHAPTER_OLIVINE:
        return started < SILVER_CHAPTER_UNDERGROUND && !FlagGet(FLAG_WAYFARER_JOHTO_SILVER_PLAYED_6);
    case SILVER_CHAPTER_HIDEOUT:
        return started < SILVER_CHAPTER_UNDERGROUND && !FlagGet(FLAG_WAYFARER_JOHTO_SILVER_PLAYED_7);
    case SILVER_CHAPTER_UNDERGROUND:
        return started <= SILVER_CHAPTER_UNDERGROUND && !FlagGet(FLAG_WAYFARER_JOHTO_SILVER_WON_8);
    case SILVER_CHAPTER_ECRUTEAK:
        return FlagGet(FLAG_WAYFARER_JOHTO_RADIO_TOWER_RESOLVED) && started < SILVER_CHAPTER_VICTORY_ROAD
            && !FlagGet(FLAG_WAYFARER_JOHTO_SILVER_PLAYED_9);
    case SILVER_CHAPTER_VICTORY_ROAD:
        return started <= SILVER_CHAPTER_VICTORY_ROAD && !FlagGet(FLAG_WAYFARER_JOHTO_SILVER_WON_10);
    case SILVER_CHAPTER_MT_MOON:
        return (started == SILVER_CHAPTER_VICTORY_ROAD || started == SILVER_CHAPTER_MT_MOON)
            && !FlagGet(FLAG_WAYFARER_JOHTO_SILVER_WON_11);
    case SILVER_CHAPTER_INDIGO:
        return FlagGet(FLAG_WAYFARER_JOHTO_SILVER_WON_11);
    }
    return FALSE;
}
#endif

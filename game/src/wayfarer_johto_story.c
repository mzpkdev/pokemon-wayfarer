#include "global.h"
#include "wayfarer_johto_story.h"
#if IS_WAYFARER
#include "event_data.h"
#include "item.h"
#include "trainer_rating.h"
#include "wayfarer_sevii_story.h"
#include "constants/items.h"
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
// Special: what keeps the player from Zuki's trial, as KIMONO_MISSING_* bits (0 = nothing). The
// selected branch's Wing must be in the Bag (a tossed Wing counts as missing), the Radio Tower
// resolved, and TR at the shared legendary threshold. With no branch chosen the Wing is missing.
u16 WayfarerJohtoKimonoMissing(void)
{
    u16 branch = VarGet(VAR_LUGIA_OR_HOOH);
    u16 wing = branch == 1 ? ITEM_SILVER_WING : branch == 2 ? ITEM_RAINBOW_WING : ITEM_NONE;
    u16 missing = 0;

    if (wing == ITEM_NONE || !CheckBagHasItem(wing, 1) || !FlagGet(FLAG_WAYFARER_JOHTO_RADIO_TOWER_RESOLVED))
        missing |= KIMONO_MISSING_WING;
    if (GetTrainerRating() < WAYFARER_LEGENDARY_TR)
        missing |= KIMONO_MISSING_TR;
    return missing;
}

// Special: the trial can be offered now. It is over once the ceremony is done.
u16 WayfarerJohtoKimonoTrialAvailable(void)
{
    return WayfarerJohtoKimonoMissing() == 0
        && VarGet(VAR_WAYFARER_JOHTO_KIMONO_CEREMONY_STATE) < KIMONO_CEREMONY_DONE;
}

// Special: what keeps Pewter's old man from giving the other branch's Wing, as PEWTER_MISSING_*
// bits. The main legendary's encounter is resolved at progress 3, which a win, a catch or a
// flee sets; running or Teleporting writes nothing.
u16 WayfarerJohtoPewterWingMissing(void)
{
    u16 main = VarGet(VAR_LUGIA_OR_HOOH) == 1 ? VarGet(VAR_COMPLETED_LUGIA) : VarGet(VAR_COMPLETED_HO_OH);
    u16 missing = 0;

    if (main != 3)
        missing |= PEWTER_MISSING_ENCOUNTER;
    if (GetTrainerRating() < WAYFARER_LEGENDARY_TR)
        missing |= PEWTER_MISSING_TR;
    return missing;
}
#endif

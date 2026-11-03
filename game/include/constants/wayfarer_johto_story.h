#ifndef GUARD_CONSTANTS_WAYFARER_JOHTO_STORY_H
#define GUARD_CONSTANTS_WAYFARER_JOHTO_STORY_H

// Silver's chapters, in story order (.product/specs/johto-independent-story-beats.md).
// VAR_WAYFARER_JOHTO_SILVER_CHAPTER holds the highest battle chapter started.
#define SILVER_CHAPTER_NEW_BARK        1  // static NPC, no battle
#define SILVER_CHAPTER_CHERRYGROVE     2  // battle
#define SILVER_CHAPTER_SPROUT_TOWER    3  // no battle
#define SILVER_CHAPTER_AZALEA          4  // battle
#define SILVER_CHAPTER_BURNED_TOWER    5  // battle
#define SILVER_CHAPTER_OLIVINE         6  // no battle
#define SILVER_CHAPTER_HIDEOUT         7  // no battle
#define SILVER_CHAPTER_UNDERGROUND     8  // battle
#define SILVER_CHAPTER_ECRUTEAK        9  // no battle
#define SILVER_CHAPTER_VICTORY_ROAD    10 // battle
#define SILVER_CHAPTER_MT_MOON         11 // battle
#define SILVER_CHAPTER_INDIGO          12 // battle, daily

// VAR_WAYFARER_JOHTO_KIMONO_CEREMONY_STATE, forward only. Zuki's offer is derived from the Wing,
// the Radio Tower and TR, so nothing is stored before the trial starts.
#define KIMONO_CEREMONY_STARTED  1 // the trial began: the Kimono Girls are on stage
#define KIMONO_CEREMONY_DONE     2 // the five battles are won and the Bell is given
#define KIMONO_CEREMONY_RESOLVED 3 // the level 50 encounter is resolved

// WayfarerJohtoKimonoMissing() bits: what still stands between the player and the trial.
#define KIMONO_MISSING_WING 1 // the branch's Wing, held by the Radio Tower's Director
#define KIMONO_MISSING_TR   2 // trainer rating below WAYFARER_LEGENDARY_TR
#define KIMONO_MISSING_BOTH 3

// WayfarerJohtoPewterWingMissing() bits.
#define PEWTER_MISSING_ENCOUNTER 1 // the main legendary's encounter is unresolved
#define PEWTER_MISSING_TR        2
#define PEWTER_MISSING_BOTH      3

// WayfarerGetOriginStarterSlot() result when the origin's starter is not chosen yet.
#define ORIGIN_STARTER_SLOT_NONE 3

#endif

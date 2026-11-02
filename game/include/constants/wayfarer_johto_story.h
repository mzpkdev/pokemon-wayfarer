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

// WayfarerGetOriginStarterSlot() result when the origin's starter is not chosen yet.
#define ORIGIN_STARTER_SLOT_NONE 3

#endif

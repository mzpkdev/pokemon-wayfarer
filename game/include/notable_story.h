#ifndef GUARD_NOTABLE_STORY_H
#define GUARD_NOTABLE_STORY_H

// Notable trainers inside the story: story beats that move friendship, the introduced-bit specials, and the
// temporary story placements that keep a trainer's walker away.
// Spec: .product/specs/notable-story-continuity.md (sections 1, 4 and 5).

#include "global.h"

#if IS_WAYFARER

// The character a story beat belongs to (NOTABLE_TRAINER_*), or NOTABLE_TRAINER_NONE for an unknown beat id.
u8 NotableStory_GetBeatCharacter(u8 beatId);

// Specials. VAR_0x8004 carries the character id (or, for CompleteBeat, the beat id).
// NotableStory_IsIntroduced is a specialvar: TRUE when the trainer is introduced.
u32 NotableStory_IsIntroduced(void);
void NotableStory_MarkIntroduced(void);
// Pays the beat's friendship points once, introduces the trainer; repeat calls do nothing.
void NotableStory_CompleteBeat(void);

// specialvar: TRUE while Whitney's morning copy shows at the Goldenrod Radio Tower (a morning, quiz unsolved). The
// Tower, the Gym, the Goldenrod Beauty and her walker's story-away row all read this one predicate.
u32 NotableStory_IsWhitneyAtTower(void);

// Story-away table: TRUE while any temporary story placement of this character is showing, anywhere in the world.
// The character's walker does not spawn then (wayfarer_walkers.c).
bool8 NotableStory_IsAway(u8 characterId);
// The number of table rows, and whether a row is showing, for tests.
u32 NotableStory_GetPlacementCount(void);
u8 NotableStory_GetPlacementCharacter(u32 index);
bool8 NotableStory_IsPlacementShowing(u32 index);

#endif // IS_WAYFARER
#endif // GUARD_NOTABLE_STORY_H

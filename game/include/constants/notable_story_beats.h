#ifndef GUARD_CONSTANTS_NOTABLE_STORY_BEATS_H
#define GUARD_CONSTANTS_NOTABLE_STORY_BEATS_H

// Story beats that move notable trainer friendship (.product/specs/notable-story-continuity.md#5-story-beats).
// Ids start at 1, in the spec's table order; 0 is no beat. Plain defines, so event scripts can name a beat for
// the NotableStory_CompleteBeat special (VAR_0x8004 = beat id). src/notable_story.c maps each to its character.

#define NOTABLE_STORY_BEAT_NONE                     0
#define NOTABLE_STORY_BEAT_LANCE_LAKE_REQUEST       1
#define NOTABLE_STORY_BEAT_LANCE_MAHOGANY_HIDEOUT   2
#define NOTABLE_STORY_BEAT_STEVEN_GRANITE_LETTER    3
#define NOTABLE_STORY_BEAT_STEVEN_SPACE_CENTER      4
#define NOTABLE_STORY_BEAT_TATE_LIZA_SPACE_CENTER   5
#define NOTABLE_STORY_BEAT_WALLACE_SOOTOPOLIS       6
#define NOTABLE_STORY_BEAT_JASMINE_LIGHTHOUSE       7
#define NOTABLE_STORY_BEAT_CLAIR_DRAGONS_DEN        8
#define NOTABLE_STORY_BEAT_WATTSON_NEW_MAUVILLE     9
#define NOTABLE_STORY_BEAT_NORMAN_WALLY_CATCH       10
#define NOTABLE_STORY_BEAT_LORELEI_ICEFALL          11
#define NOTABLE_STORY_BEAT_COUNT                    11

#endif // GUARD_CONSTANTS_NOTABLE_STORY_BEATS_H

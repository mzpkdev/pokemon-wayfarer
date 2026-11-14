#include "global.h"
#include "notable_story.h"

#if IS_WAYFARER
#include "event_data.h"
#include "notable_friendship.h"
#include "constants/flags.h"
#include "constants/notable_story_beats.h"
#include "constants/notable_trainers.h"
#include "constants/vars.h"
#include "constants/wayfarer_persistence.h"

// Beat id - 1 -> character (notable-story-continuity.md#5-story-beats). Ids are in constants/notable_story_beats.h.
static const u8 sBeatCharacters[NOTABLE_STORY_BEAT_COUNT] =
{
    [NOTABLE_STORY_BEAT_LANCE_LAKE_REQUEST - 1] = NOTABLE_TRAINER_LANCE,
    [NOTABLE_STORY_BEAT_LANCE_MAHOGANY_HIDEOUT - 1] = NOTABLE_TRAINER_LANCE,
    [NOTABLE_STORY_BEAT_STEVEN_GRANITE_LETTER - 1] = NOTABLE_TRAINER_STEVEN,
    [NOTABLE_STORY_BEAT_STEVEN_SPACE_CENTER - 1] = NOTABLE_TRAINER_STEVEN,
    [NOTABLE_STORY_BEAT_TATE_LIZA_SPACE_CENTER - 1] = NOTABLE_TRAINER_TATE_LIZA,
    [NOTABLE_STORY_BEAT_WALLACE_SOOTOPOLIS - 1] = NOTABLE_TRAINER_WALLACE,
    [NOTABLE_STORY_BEAT_JASMINE_LIGHTHOUSE - 1] = NOTABLE_TRAINER_JASMINE,
    [NOTABLE_STORY_BEAT_CLAIR_DRAGONS_DEN - 1] = NOTABLE_TRAINER_CLAIR,
    [NOTABLE_STORY_BEAT_WATTSON_NEW_MAUVILLE - 1] = NOTABLE_TRAINER_WATTSON,
    [NOTABLE_STORY_BEAT_NORMAN_WALLY_CATCH - 1] = NOTABLE_TRAINER_NORMAN,
    [NOTABLE_STORY_BEAT_LORELEI_ICEFALL - 1] = NOTABLE_TRAINER_LORELEI,
};

u8 NotableStory_GetBeatCharacter(u8 beatId)
{
    return beatId >= 1 && beatId <= NOTABLE_STORY_BEAT_COUNT ? sBeatCharacters[beatId - 1] : NOTABLE_TRAINER_NONE;
}

u32 NotableStory_IsIntroduced(void)
{
    return NotableFriendship_IsIntroduced(gSpecialVar_0x8004);
}

void NotableStory_MarkIntroduced(void)
{
    NotableFriendship_MarkIntroduced(gSpecialVar_0x8004);
}

void NotableStory_CompleteBeat(void)
{
    NotableFriendship_CompleteStoryBeat(NotableStory_GetBeatCharacter(gSpecialVar_0x8004), gSpecialVar_0x8004);
}

// Hoenn story flags and vars live in their own bank in Wayfarer: HOENN_FLAG_ID / HOENN_VAR_ID name them.
// Story-away table (notable-story-continuity.md#4-one-place-at-a-time). A row is a temporary story copy of a
// character, showing while ALL of its conditions hold. Only temporary placements go here: a copy that is
// always visible would suppress the walker for the whole game.
//   flag:  0 = no flag condition; otherwise the copy shows while the flag is clear (an object's hide flag, the
//          default) or, with PLACEMENT_FLAG_SET, while it is set.
//   var:   0 = no var condition; otherwise it shows while varMin <= var <= varMax.
#define PLACEMENT_FLAG_SET (1 << 0)

struct StoryPlacement
{
    u16 flag;
    u16 var;
    u8 characterId;
    u8 varMin;
    u8 varMax;
    u8 options;
};

static const struct StoryPlacement sPlacements[] =
{
    // Wallace through the Sootopolis crisis: states 2-3 the Cave of Origin and the Sky Pillar copies (their hide flags
    // stay clear after he leaves, so the state decides, not the flags), 4-5 the Sootopolis copy
    // (FLAG_HIDE_SOOTOPOLIS_CITY_WALLACE clear). State 6 hides him and ends the crisis.
    { .characterId = NOTABLE_TRAINER_WALLACE, .var = HOENN_VAR_ID(VAR_SOOTOPOLIS_CITY_STATE), .varMin = 2, .varMax = 5 },
    // Steven at the Mossdeep Space Center 2F: his object shows from the invasion (flag cleared) until the battle
    // (state 3). The 1F copy is left out: nothing hides it at New Game, so it would keep him away until the invasion.
    { .characterId = NOTABLE_TRAINER_STEVEN, .flag = HOENN_FLAG_ID(FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_2F_STEVEN),
      .var = HOENN_VAR_ID(VAR_MOSSDEEP_SPACE_CENTER_STATE), .varMin = 1, .varMax = 2 },
    // Giovanni leaves Viridian for good: once he has departed, his walker never spawns.
    { .characterId = NOTABLE_TRAINER_GIOVANNI, .flag = FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS, .options = PLACEMENT_FLAG_SET },
};

u32 NotableStory_GetPlacementCount(void)
{
    return ARRAY_COUNT(sPlacements);
}

u8 NotableStory_GetPlacementCharacter(u32 index)
{
    return index < ARRAY_COUNT(sPlacements) ? sPlacements[index].characterId : NOTABLE_TRAINER_NONE;
}

bool8 NotableStory_IsPlacementShowing(u32 index)
{
    const struct StoryPlacement *placement;
    u16 value;

    if (index >= ARRAY_COUNT(sPlacements))
        return FALSE;
    placement = &sPlacements[index];
    if (placement->flag != 0 && FlagGet(placement->flag) != ((placement->options & PLACEMENT_FLAG_SET) != 0))
        return FALSE;
    if (placement->var != 0)
    {
        value = VarGet(placement->var);
        if (value < placement->varMin || value > placement->varMax)
            return FALSE;
    }
    return TRUE;
}

bool8 NotableStory_IsAway(u8 characterId)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sPlacements); i++)
    {
        if (sPlacements[i].characterId == characterId && NotableStory_IsPlacementShowing(i))
            return TRUE;
    }
    return FALSE;
}

#endif // IS_WAYFARER

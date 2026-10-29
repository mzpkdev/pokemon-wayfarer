#ifndef GUARD_NOTABLE_TRAINERS_H
#define GUARD_NOTABLE_TRAINERS_H

#include "data.h"

#define NOTABLE_TRAINER_COUNT 38

enum NotableTrainerArchetype
{
    NOTABLE_ARCHETYPE_STEADY,
    NOTABLE_ARCHETYPE_PRODIGY,
    NOTABLE_ARCHETYPE_SLEEPER,
    NOTABLE_ARCHETYPE_VETERAN,
    NOTABLE_ARCHETYPE_RIVAL,
    NOTABLE_ARCHETYPE_LEGEND,
    NOTABLE_ARCHETYPE_STAR,
    NOTABLE_ARCHETYPE_COMEBACK,
    NOTABLE_ARCHETYPE_BURST,
};

enum NotableTrainerRegion
{
    NOTABLE_REGION_KANTO,
    NOTABLE_REGION_JOHTO,
    NOTABLE_REGION_HOENN,
};

enum NotableTrainerPlayStyle
{
    NOTABLE_STYLE_FIELD_MARSHAL,
    NOTABLE_STYLE_GAMBLER,
    NOTABLE_STYLE_HEXER,
    NOTABLE_STYLE_BRAWLER,
    NOTABLE_STYLE_TACTICIAN,
    NOTABLE_STYLE_SWEEPER,
    NOTABLE_STYLE_TURTLE,
    NOTABLE_STYLE_BOMBER,
};

// The voice bits the talk flow speaks (notable-haunts.md#talk-flow), as message text with line breaks.
// {PLAYER} is the player's name; {STR_VAR_1} is the buddy slot's species, set by the talk code.
struct NotableVoice
{
    const u8 *meet;
    const u8 *heard;
    const u8 *again;
    const u8 *hello;
    const u8 *close;
    const u8 *bye;
};

struct NotableMovePoolEntry
{
    u16 move;
    u8 fromLevel; // 0: natural level-up schedule
};

struct NotableTrainer
{
    u32 characterId;
    u16 startTR;
    u16 peakTR;
    u8 archetype;
    u8 homeRegion;
    u8 playStyle;
    bool8 traveller;
    bool8 aloof;
    bool8 bossOmniscient;
    bool8 isDoubleBattle;
    struct TrainerMon roster[PARTY_SIZE];
    s8 levelOffsets[PARTY_SIZE];
    u8 aceMask;
    u8 buddySlot; // 1-6: the roster slot {BUDDY} names in voice lines
    const struct NotableMovePoolEntry *movePool;
    u8 movePoolCount;
};

struct NotableTrainerSnapshot
{
    const struct NotableTrainer *trainer;
    u32 worldProgress;
    u32 trainerTR;
    u64 aiFlags;
    struct TrainerMon members[PARTY_SIZE]; // indexed by stable roster slot
    u8 battleOrder[PARTY_SIZE]; // output position to roster slot
    u8 teamSize;
    u8 aceCount;
};

extern const struct NotableVoice gNotableVoices[NOTABLE_TRAINER_COUNT];

const struct NotableTrainer *GetNotableTrainerById(u32 characterId);
const struct NotableTrainer *GetNotableTrainerForEncounter(u16 trainerId);
u32 GetNotableTrainerRating(const struct NotableTrainer *trainer, u32 worldProgress);
u8 GetNotableTrainerTeamLevel(u32 trainerTR);
bool32 ResolveNotableTrainerSnapshot(const struct NotableTrainer *trainer, u32 worldProgress, bool32 skipMovePool, struct NotableTrainerSnapshot *snapshot);
bool32 IsNotableTrainerCatalogValid(void);

#endif // GUARD_NOTABLE_TRAINERS_H

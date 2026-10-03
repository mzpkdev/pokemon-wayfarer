// Host build of src/wayfarer_world.c (the engine seam) for engine_test.c:
// forced in with -include ahead of the source. It stands in for the engine
// headers wayfarer_world.c includes (their guards are taken here) with just
// the types, globals and calls it uses; engine_test.c defines them.
#ifndef GUARD_WAYFARER_ENGINE_PRELUDE_H
#define GUARD_WAYFARER_ENGINE_PRELUDE_H

#include <stdbool.h>
#include <string.h>
#include "constants/global.h"
#include "gba/types.h"

#define GUARD_GLOBAL_H
#define GUARD_LEAGUE_EVENTS_H
#define GUARD_LEAGUE_SELECTION_H
#define GUARD_ALLOC_H
#define GUARD_OVERWORLD_H
#define GUARD_PALETTE_H
#define GUARD_POKEMON_STORAGE_SYSTEM_H
#define GUARD_TRAINER_RATING_H
#define GUARD_WAYFARER_PERSISTENCE_H
#define GUARD_WAYFARER_WALKERS_H

#undef EWRAM_DATA
#define EWRAM_DATA
#define STATIC_ASSERT(expr, id) typedef char id[(expr) ? 1 : -1];
#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#include "constants/regions.h"
#include "wayfarer_world_sim.h"

#define LEAGUE_LINEUP_SIZE 5
#define LEAGUE_EVENT_LINEUP_SIZE 5
#define LEAGUE_INVITATION_INVITED 1

struct LeagueSavedTeam { u16 characterId; };
struct LeagueLineupMember { u16 characterId; };
struct LeagueLineupSelection { struct LeagueLineupMember members[LEAGUE_LINEUP_SIZE]; u8 count; };
struct LeagueEventState { u8 invitationState; u8 invitedLeague; u16 recentIds[LEAGUE_LINEUP_SIZE];
                          u64 reignedIndigoMask; u64 reignedHoennMask; };
struct SaveBlock3 { struct LeagueEventState leagueEvent; };
struct WarpData { s8 mapGroup; s8 mapNum; s8 warpId; s16 x, y; };
struct SaveBlock1 { struct WarpData location; };
struct PokemonStorage { struct WayfarerWorldState wayfarerWorld; };
struct MapLayout { s32 width, height; };
struct MapHeader { const struct MapLayout *mapLayout; };
struct PaletteFadeControl { bool8 active; };

extern struct SaveBlock1 *gSaveBlock1Ptr;
extern struct SaveBlock3 *gSaveBlock3Ptr;
extern struct PokemonStorage *gPokemonStoragePtr;
extern struct PaletteFadeControl gPaletteFade;
extern u16 gHostVcount;
#define REG_VCOUNT gHostVcount

bool32 GetAcceptedLeagueEventMember(u8 match, const struct LeagueSavedTeam **team);
bool32 SelectLeagueLineup(u8 leagueId, u32 worldProgress, const u32 recentIds[LEAGUE_LINEUP_SIZE],
                          u64 reignedIndigoMask, u64 reignedHoennMask, struct LeagueLineupSelection *out);
bool8 GetBadgeStateForRegion(u8 region, u8 badgeIndex);
u32 GetTrainerRating(void);
void *Alloc(u32 size);
void Free(void *pointer);
const struct MapHeader *Overworld_GetMapHeaderByGroupAndId(u16 mapGroup, u16 mapNum);

struct WayfarerWalkersDebug { u32 lastContextScanlines; };
extern struct WayfarerWalkersDebug gWayfarerWalkersDebug;
void WayfarerWalkers_Reset(void);
void WayfarerWalkers_OnContinue(void);
void WayfarerWalkers_FlushWorldJobs(void);
u32 WayfarerWalkers_ScanlineStamp(void);
u32 WayfarerWalkers_ScanlinesSince(u32 start);
u32 WayfarerWalkers_LastUpdateScanlines(void);
void WayfarerWalkers_NoteHeartbeat(u32 scanlines, u32 contextScanlines);

#endif

// Offline driver for the notable world simulation: runs the ROM's own core
// (src/wayfarer_world_sim.c) over the generated tables on the host, for N
// heartbeats and a scripted scenario, and prints a line-oriented trace that
// report.py aggregates. Built by report.py; not part of the ROM.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants/global.h"
#include "wayfarer_world_sim.h"

#define MAX_PATH 256
#define MAX_IDS 16

static void Usage(void)
{
    fprintf(stderr,
        "usage: sim_report [--heartbeats N] [--wp N] [--badges KANTO,JOHTO,HOENN]\n"
        "                  [--path MAP,MAP,...] [--lineup ID,...] [--provisional ID,...] [--rising ID,...]\n"
        "                  [--resolve-at K --resolve-won 0|1]\n"
        "  Maps are numeric MAP_* values; IDs are NOTABLE_TRAINER_* values; badge sets are bit masks.\n"
        "  --lineup names an accepted event's members (battle order), away until --resolve-at.\n");
    exit(2);
}

static int ParseList(const char *text, long *out, int max)
{
    int count = 0;
    char *copy = strdup(text), *token, *save = NULL;
    for (token = strtok_r(copy, ",", &save); token != NULL && count < max; token = strtok_r(NULL, ",", &save))
        out[count++] = strtol(token, NULL, 0);
    free(copy);
    return count;
}

static u32 MaskOf(const long *ids, int count)
{
    u32 mask = 0;
    int i;
    for (i = 0; i < count; i++)
    {
        u8 slot = WorldSim_SlotForCharacter((u16)ids[i]);
        if (slot < WORLD_SIM_TRAINER_COUNT)
            mask |= 1u << slot;
    }
    return mask;
}

static void OnSkip(void *user, u8 slot, u8 activity, u8 reason)
{
    long heartbeat = *(long *)user;
    printf("skip %ld %u %u %u\n", heartbeat, slot, activity, reason);
}

static void BuildContext(struct WayfarerWorldContext *ctx, u16 playerMap, u32 wp, const long *badges,
                         u32 lineupMask, u32 provisionalMask, u32 risingMask)
{
    u8 slot;
    memset(ctx, 0, sizeof(*ctx));
    ctx->playerMap = playerMap;
    ctx->worldProgress = wp;
    ctx->provisionalMask = provisionalMask;
    ctx->risingMask = risingMask;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[slot];
        ctx->derived[slot] = WORLD_DERIVED_NONE;
        if (lineupMask & (1u << slot))
            ctx->derived[slot] = WORLD_STATE_AWAY_LEAGUE;
        else if ((trainer->flags & WORLD_TRAINER_FLAG_GYM_LEADER) && trainer->badgeRegion <= WORLD_REGION_HOENN
              && !(badges[trainer->badgeRegion] & (1 << trainer->badgeIndex)))
            ctx->derived[slot] = WORLD_STATE_HOME_LOCKED;
    }
}

static void PrintTables(void)
{
    u16 i;
    for (i = 0; i < gWayfarerWorldNodeCount; i++)
        printf("node %u %u %u\n", i, gWayfarerWorldNodes[i].map, gWayfarerWorldNodes[i].flags);
    for (i = 0; i < gWayfarerWorldSpotCount; i++)
        printf("spot %u %u %u %u %u\n", i, gWayfarerWorldSpots[i].node, gWayfarerWorldSpots[i].kind,
               gWayfarerWorldSpots[i].x, gWayfarerWorldSpots[i].y);
    for (i = 0; i < WORLD_SIM_TRAINER_COUNT; i++)
    {
        const struct WayfarerWorldTrainer *t = &gWayfarerWorldTrainers[i];
        printf("trainer %u %u %u %u %u %u %u\n", i, t->characterId, t->homeMap, t->homeNode, t->flags,
               t->candidateCount, t->searchBound);
    }
}

// Occupancy of every map a record is on or heading for.
static void PrintOccupancy(long heartbeat, const struct WayfarerWorldState *state)
{
    u16 maps[WORLD_SIM_TRAINER_COUNT * 2], nodes[WORLD_SIM_TRAINER_COUNT * 2];
    int count = 0, i, j;
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        u16 candidates[2];
        candidates[0] = state->records[slot].node;
        candidates[1] = WorldSim_DestNode(state, slot);
        for (j = 0; j < 2; j++)
        {
            u16 map;
            if (candidates[j] == WORLD_NODE_NONE)
                continue;
            map = WorldSim_NodeMap(candidates[j]);
            for (i = 0; i < count && maps[i] != map; i++)
                ;
            if (i == count)
            {
                maps[count] = map;
                nodes[count++] = candidates[j];
            }
        }
    }
    for (i = 0; i < count; i++)
    {
        u8 occupancy = WorldSim_Occupancy(state, maps[i], 0xFF);
        if (occupancy != 0)
            printf("occ %ld %u %u %u\n", heartbeat, maps[i], occupancy, WorldSim_MapCapacity(nodes[i]));
    }
}

static void PrintRecords(long heartbeat, const struct WayfarerWorldState *state)
{
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldRecord *r = &state->records[slot];
        u16 destNode = WorldSim_DestNode(state, slot);
        printf("rec %ld %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n", heartbeat, slot, r->node,
               WorldSim_NodeMap(r->node), r->state, r->destKind, r->destId,
               destNode == WORLD_NODE_NONE ? 0xFFFF : WorldSim_NodeMap(destNode),
               r->activity, r->dwell, r->lifeEvent, r->lifeSteps, r->step, r->arrival, r->waited);
    }
}

int main(int argc, char **argv)
{
    long heartbeats = 200, wp = 0, badges[3] = {0, 0, 0}, path[MAX_PATH], lineup[MAX_IDS];
    long provisional[MAX_IDS], rising[MAX_IDS], resolveAt = -1, resolveWon = 0, heartbeat;
    int pathCount = 0, lineupCount = 0, provisionalCount = 0, risingCount = 0, i;
    static struct WayfarerWorldState state;
    struct WayfarerWorldContext ctx;
    struct WayfarerWorldTrace trace;
    void *workspace;
    u32 lineupMask;

    for (i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        const char *value = i + 1 < argc ? argv[i + 1] : NULL;
        if (value == NULL)
            Usage();
        if (!strcmp(arg, "--heartbeats")) heartbeats = strtol(value, NULL, 0);
        else if (!strcmp(arg, "--wp")) wp = strtol(value, NULL, 0);
        else if (!strcmp(arg, "--badges")) ParseList(value, badges, 3);
        else if (!strcmp(arg, "--path")) pathCount = ParseList(value, path, MAX_PATH);
        else if (!strcmp(arg, "--lineup")) lineupCount = ParseList(value, lineup, MAX_IDS);
        else if (!strcmp(arg, "--provisional")) provisionalCount = ParseList(value, provisional, MAX_IDS);
        else if (!strcmp(arg, "--rising")) risingCount = ParseList(value, rising, MAX_IDS);
        else if (!strcmp(arg, "--resolve-at")) resolveAt = strtol(value, NULL, 0);
        else if (!strcmp(arg, "--resolve-won")) resolveWon = strtol(value, NULL, 0);
        else Usage();
        i++;
    }

    workspace = malloc(WorldSim_WorkspaceSize());
    lineupMask = MaskOf(lineup, lineupCount);
    printf("tables nodes=%u edges=%u spots=%u hash=0x%04X\n", gWayfarerWorldNodeCount, gWayfarerWorldEdgeCount,
           gWayfarerWorldSpotCount, gWayfarerWorldContentHash);

    BuildContext(&ctx, pathCount ? (u16)path[0] : 0xFFFF, (u32)wp, badges, lineupMask,
                 MaskOf(provisional, provisionalCount), MaskOf(rising, risingCount));
    PrintTables();
    WorldSim_NewGame(&state, &ctx, workspace);
    printf("valid 0 %d\n", WorldSim_IsValid(&state, workspace));
    PrintRecords(0, &state);
    PrintOccupancy(0, &state);

    for (heartbeat = 1; heartbeat <= heartbeats; heartbeat++)
    {
        u16 playerMap = pathCount ? (u16)path[(heartbeat - 1) % pathCount] : 0xFFFF;
        if (heartbeat == resolveAt && lineupCount > 0)
        {
            u16 ids[MAX_IDS];
            for (i = 0; i < lineupCount; i++)
                ids[i] = (u16)lineup[i];
            WorldSim_OnLeagueResolved(&state, ids, (u8)lineupCount, resolveWon != 0,
                                      resolveWon ? 0 : ids[lineupCount - 1]);
            lineupMask = 0;
            printf("resolved %ld %ld\n", heartbeat, resolveWon);
        }
        BuildContext(&ctx, playerMap, (u32)wp, badges, lineupMask,
                     MaskOf(provisional, provisionalCount), MaskOf(rising, risingCount));
        memset(&trace, 0, sizeof(trace));
        trace.onSkip = OnSkip;
        trace.user = &heartbeat;
        WorldSim_Heartbeat(&state, &ctx, workspace, &trace);
        printf("beat %ld %u %u %u %u %u %u %u %u %u\n", heartbeat, playerMap, trace.searches, trace.searchNodes,
               trace.searchNodesMax, trace.hops, trace.waits, trace.reroutes, trace.advances, trace.skips);
        PrintRecords(heartbeat, &state);
        PrintOccupancy(heartbeat, &state);
        if (!WorldSim_IsValid(&state, workspace))
            printf("valid %ld 0\n", heartbeat);
    }
    free(workspace);
    return 0;
}

#include "global.h"
#include "overworld.h"
#include "test/test.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/maps.h"

#if !IS_FRLG && !IS_HNS

static const struct MapEvents *GetEvents(u16 map)
{
    return Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(map), MAP_NUM(map))->events;
}

static u32 CountCoordEventsAt(u16 map, s16 x, s16 y)
{
    const struct MapEvents *events = GetEvents(map);
    u32 count = 0;

    for (u32 i = 0; i < events->coordEventCount; i++)
    {
        if (events->coordEvents[i].x == x && events->coordEvents[i].y == y)
            count++;
    }
    return count;
}

static const struct ObjectEventTemplate *FindObjectByFlag(u16 map, u16 flag)
{
    const struct MapEvents *events = GetEvents(map);

    for (u32 i = 0; i < events->objectEventCount; i++)
    {
        if (events->objectEvents[i].flagId == flag)
            return &events->objectEvents[i];
    }
    return NULL;
}

static const struct ObjectEventTemplate *FindObjectAt(u16 map, s16 x, s16 y, u8 elevation)
{
    const struct MapEvents *events = GetEvents(map);

    for (u32 i = 0; i < events->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *object = &events->objectEvents[i];

        if (object->x == x && object->y == y && object->elevation == elevation)
            return object;
    }
    return NULL;
}

static bool8 HasObjectWithFlagAndGraphicsAt(u16 map, u16 flag, u16 graphicsId, s16 x, s16 y)
{
    const struct MapEvents *events = GetEvents(map);

    for (u32 i = 0; i < events->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *object = &events->objectEvents[i];

        if (object->flagId == flag
         && object->graphicsId == graphicsId
         && object->x == x
         && object->y == y)
            return TRUE;
    }
    return FALSE;
}

static void ExpectFerryContact(u16 map, s16 x, s16 y, u8 elevation)
{
    const struct ObjectEventTemplate *object = FindObjectAt(map, x, y, elevation);

    EXPECT(object != NULL);
    if (object == NULL)
        return;
    EXPECT_EQ(object->graphicsId, OBJ_EVENT_GFX_SAILOR);
    EXPECT_EQ(object->movementType, MOVEMENT_TYPE_FACE_DOWN);
    EXPECT_EQ(object->flagId, 0);
}

TEST("Emerald traversal: compiled road events leave the approved lanes open")
{
    EXPECT_EQ(CountCoordEventsAt(MAP_OLDALE_TOWN, 0, 10), 0);

    for (s16 y = 10; y <= 12; y++)
    {
        EXPECT_EQ(CountCoordEventsAt(MAP_PETALBURG_CITY, 8, y), 1);
        EXPECT_EQ(CountCoordEventsAt(MAP_PETALBURG_CITY, 4, y), 1);
    }
    EXPECT_EQ(CountCoordEventsAt(MAP_PETALBURG_CITY, 8, 13), 0);
    EXPECT_EQ(CountCoordEventsAt(MAP_PETALBURG_CITY, 4, 13), 1);

    EXPECT_EQ(CountCoordEventsAt(MAP_PETALBURG_WOODS, 26, 23), 1);
    EXPECT_EQ(CountCoordEventsAt(MAP_PETALBURG_WOODS, 27, 23), 0);

    EXPECT_EQ(CountCoordEventsAt(MAP_ROUTE110, 33, 56), 1);
    EXPECT_EQ(CountCoordEventsAt(MAP_ROUTE110, 34, 56), 1);
    EXPECT_EQ(CountCoordEventsAt(MAP_ROUTE110, 35, 56), 0);

    EXPECT_EQ(CountCoordEventsAt(MAP_ROUTE119, 25, 31), 1);
    EXPECT_EQ(CountCoordEventsAt(MAP_ROUTE119, 26, 31), 0);
}

TEST("Emerald traversal: compiled ferry contacts match every public stop")
{
    ExpectFerryContact(MAP_ROUTE104, 14, 51, 4);
    ExpectFerryContact(MAP_DEWFORD_TOWN, 13, 9, 3);
    ExpectFerryContact(MAP_ROUTE109, 22, 24, 3);
}

TEST("Emerald traversal: compiled objects keep the optional and story lanes distinct")
{
    const struct ObjectEventTemplate *rock1 = FindObjectByFlag(MAP_ROUTE111, FLAG_TEMP_11);
    const struct ObjectEventTemplate *rock2 = FindObjectByFlag(MAP_ROUTE111, FLAG_TEMP_12);

    EXPECT(rock1 != NULL);
    EXPECT(rock2 != NULL);
    if (rock1 != NULL && rock2 != NULL)
    {
        EXPECT(!((rock1->x == 18 && rock1->y == 101) && (rock2->x == 19 && rock2->y == 100)));
        EXPECT((rock1->x == 18 && rock1->y == 101) || (rock2->x == 19 && rock2->y == 100));
    }

    EXPECT(!HasObjectWithFlagAndGraphicsAt(
        MAP_ROUTE112, FLAG_HIDE_ROUTE_112_TEAM_MAGMA, OBJ_EVENT_GFX_MAGMA_MEMBER_M, 26, 30));
    EXPECT(!HasObjectWithFlagAndGraphicsAt(
        MAP_ROUTE112, FLAG_HIDE_ROUTE_112_TEAM_MAGMA, OBJ_EVENT_GFX_MAGMA_MEMBER_M, 27, 30));
    EXPECT(HasObjectWithFlagAndGraphicsAt(
        MAP_ROUTE112, FLAG_HIDE_ROUTE_112_TEAM_MAGMA, OBJ_EVENT_GFX_MAGMA_MEMBER_M, 24, 31));
    EXPECT(HasObjectWithFlagAndGraphicsAt(
        MAP_ROUTE112, FLAG_HIDE_ROUTE_112_TEAM_MAGMA, OBJ_EVENT_GFX_MAGMA_MEMBER_M, 25, 31));

    EXPECT(!HasObjectWithFlagAndGraphicsAt(
        MAP_MT_CHIMNEY, FLAG_HIDE_MT_CHIMNEY_TEAM_AQUA, OBJ_EVENT_GFX_AQUA_MEMBER_M, 19, 39));
    EXPECT(!HasObjectWithFlagAndGraphicsAt(
        MAP_MT_CHIMNEY, FLAG_HIDE_MT_CHIMNEY_TEAM_MAGMA, OBJ_EVENT_GFX_MAGMA_MEMBER_M, 22, 39));
    EXPECT(!HasObjectWithFlagAndGraphicsAt(
        MAP_MT_CHIMNEY, FLAG_HIDE_MT_CHIMNEY_TEAM_AQUA, OBJ_EVENT_GFX_POOCHYENA, 20, 39));
    EXPECT(!HasObjectWithFlagAndGraphicsAt(
        MAP_MT_CHIMNEY, FLAG_HIDE_MT_CHIMNEY_TEAM_MAGMA, OBJ_EVENT_GFX_POOCHYENA, 21, 39));
    EXPECT(HasObjectWithFlagAndGraphicsAt(
        MAP_MT_CHIMNEY, FLAG_HIDE_MT_CHIMNEY_TEAM_AQUA, OBJ_EVENT_GFX_AQUA_MEMBER_M, 17, 38));
    EXPECT(HasObjectWithFlagAndGraphicsAt(
        MAP_MT_CHIMNEY, FLAG_HIDE_MT_CHIMNEY_TEAM_MAGMA, OBJ_EVENT_GFX_MAGMA_MEMBER_M, 24, 38));
    EXPECT(HasObjectWithFlagAndGraphicsAt(
        MAP_MT_CHIMNEY, FLAG_HIDE_MT_CHIMNEY_TEAM_AQUA, OBJ_EVENT_GFX_POOCHYENA, 18, 38));
    EXPECT(HasObjectWithFlagAndGraphicsAt(
        MAP_MT_CHIMNEY, FLAG_HIDE_MT_CHIMNEY_TEAM_MAGMA, OBJ_EVENT_GFX_POOCHYENA, 23, 38));

    EXPECT(FindObjectByFlag(MAP_ROUTE120, FLAG_HIDE_ROUTE_120_STEVEN) != NULL);
    EXPECT(FindObjectByFlag(MAP_ROUTE120, FLAG_HIDE_ROUTE_120_KECLEON_BRIDGE) != NULL);
    EXPECT(FindObjectByFlag(MAP_ROUTE120, FLAG_HIDE_ROUTE_120_KECLEON_BRIDGE_SHADOW) != NULL);
    EXPECT(FindObjectAt(MAP_ROUTE120, 13, 15, 4) != NULL);
    EXPECT(FindObjectAt(MAP_ROUTE120, 12, 16, 4) != NULL);
    EXPECT(FindObjectAt(MAP_ROUTE120, 12, 16, 3) != NULL);
}

#endif

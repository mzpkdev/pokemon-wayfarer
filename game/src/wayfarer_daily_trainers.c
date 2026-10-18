// Daily world slots, trainer half (Wayfarer only). Every daily trainer spot is
// a slot: its home trainer stands there until beaten, and from the next day each
// beaten trainer is placed somewhere in its range (class group, place group or
// home slot), with each free slot empty at its absence chance. Spec:
// .product/specs/daily-world-slots.md (Trainers, Save state, Flags and
// visibility).
//
// Everything about who stands where is a pure function of the save's seed, the
// stamped day, the defeat flags and the day-start snapshots, so a group can be
// resolved from anywhere (WhereIsTrainerToday) with the same result as on the
// map. Only the map's resolved occupants are kept, in EWRAM, for the map's
// lifetime. A free slot shows the occupant's own sprite and runs the
// occupant's own authored script: the flag checks the battle scripts make
// (GetTrainerFlag, the trainer sight check) read the slot's clearedToday bit
// instead of the trainer's permanent defeat flag.

#include "global.h"
#include "wayfarer_daily_slots.h"

#if IS_WAYFARER
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "match_call.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "wayfarer_coast_trainers.h"
#include "wayfarer_sevii_rematches.h"
#include "constants/event_objects.h"
#include "constants/maps.h"
#include "constants/opponents.h"
#include "constants/rematches.h"
#include "data/trainer_slots/slots.h"

// The phone contact index space of phone-rematches.md: rematch table rows first, then the Sevii
// families that carry a stronger team, then the coast families.
#define CONTACT_TABLE_COUNT     118
#define CONTACT_SEVII_FIRST     118
#define CONTACT_COAST_FIRST     150

STATIC_ASSERT(REMATCH_TABLE_ENTRIES <= CONTACT_TABLE_COUNT, DailyContactTableRows);
STATIC_ASSERT(WORLD_TRAINER_SEVII_FAMILIES == WAYFARER_SEVII_REMATCH_FAMILY_COUNT, DailyContactSeviiFamilies);
STATIC_ASSERT(CONTACT_COAST_FIRST + WAYFARER_COAST_REMATCH_FAMILY_COUNT <= DAILY_CONTACT_COUNT, DailyContactCoastFamilies);
STATIC_ASSERT(WORLD_TRAINER_GROUP_SLOTS_MAX <= 255, DailyTrainerGroupSize);

// The rotating slots of the current map and what stands in them today.
struct MapOccupant
{
    u16 slot;       // the slot row
    u16 occupant;   // the occupant's slot row, or DAILY_OCCUPANT_AWAY
};

#if TESTING || defined(E2E_TESTING)
// Debug and E2E builds let a journey read who stands in the map's rotating slots.
EWRAM_DATA struct MapOccupant gDailyTrainersDebugOccupants[WORLD_TRAINER_MAP_SLOTS_MAX] = {0};
EWRAM_DATA u8 gDailyTrainersDebugOccupantCount = 0;
#define sMapOccupants gDailyTrainersDebugOccupants
#define sNumMapOccupants gDailyTrainersDebugOccupantCount
#else
static EWRAM_DATA struct MapOccupant sMapOccupants[WORLD_TRAINER_MAP_SLOTS_MAX] = {0};
static EWRAM_DATA u8 sNumMapOccupants = 0;
#endif
// The slot rows (plus one; zero is none) of the battle being set up, one per opponent, and whether each was a rotating occupant.
static EWRAM_DATA u16 sNotedSlotsPlusOne[2] = {0};
static EWRAM_DATA u8 sNotedRotating = 0;

static struct WayfarerDailySlots *Daily(void)
{
    return &gPokemonStoragePtr->dailySlots;
}

// ---------- slot lookup ----------
static u16 SlotMapKey(u16 slot)
{
    return (sWorldTrainerSlots[slot].mapGroup << 8) | sWorldTrainerSlots[slot].mapNum;
}

// First slot row whose map key is not below `key` (rows are sorted by map, then local id).
static u16 MapLowerBound(u16 key)
{
    u16 low = 0, high = WORLD_TRAINER_SLOT_COUNT;

    while (low < high)
    {
        u16 middle = (low + high) / 2;

        if (SlotMapKey(middle) < key)
            low = middle + 1;
        else
            high = middle;
    }
    return low;
}

static u16 CurrentMapKey(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

u16 DailyTrainers_FindSlot(u8 mapGroup, u8 mapNum, u8 localId)
{
    u16 key = (mapGroup << 8) | mapNum;
    u16 slot;

    for (slot = MapLowerBound(key); slot < WORLD_TRAINER_SLOT_COUNT && SlotMapKey(slot) == key; slot++)
    {
        const struct WorldTrainerSlot *row = &sWorldTrainerSlots[slot];

        if (row->localId[0] == localId || (row->localId[1] != 0 && row->localId[1] == localId))
            return slot;
    }
    return NO_TRAINER_SLOT;
}

const struct WorldTrainerSlot *DailyTrainers_GetSlot(u16 slot)
{
    return &sWorldTrainerSlots[slot];
}

static u16 ClearedIndex(u16 slot)
{
    return WORLD_ITEM_SPOT_COUNT + slot;
}

bool8 DailyTrainers_IsCleared(u16 slot)
{
    u16 index = ClearedIndex(slot);

    return (Daily()->clearedToday[index / 8] >> (index % 8)) & 1;
}

void DailyTrainers_SetCleared(u16 slot)
{
    u16 index = ClearedIndex(slot);

    Daily()->clearedToday[index / 8] |= 1 << (index % 8);
}

bool8 DailyTrainers_IsHomeBeaten(u16 slot)
{
    return (Daily()->homeBeatenToday[slot / 8] >> (slot % 8)) & 1;
}

static void SetHomeBeaten(u16 slot)
{
    Daily()->homeBeatenToday[slot / 8] |= 1 << (slot % 8);
}

// ---------- phone contacts ----------
// Where a trainer id sits in the contact index space, or DAILY_NO_CONTACT. Hoenn rows join when the
// rematch table grows (phone-rematches.md); this finds them then with no change.
u8 DailyContacts_IndexOf(u16 trainerId)
{
    s32 index = TrainerIdToRematchTableId(gRematchTable, trainerId);

    if (index >= 0)
        return index < REMATCH_SPECIAL_TRAINER_START ? index : DAILY_NO_CONTACT;
    index = WayfarerSeviiRematchFamilyIndex(trainerId);
    if (index >= 0)
        return sWorldTrainerSeviiContacts[index] == 255 ? DAILY_NO_CONTACT : sWorldTrainerSeviiContacts[index];
    index = WayfarerCoastRematchFamilyIndex(trainerId);
    if (index >= 0)
        return CONTACT_COAST_FIRST + index;
    return DAILY_NO_CONTACT;
}

bool8 DailyContacts_ReadyAtDayStart(u8 contact)
{
    return contact < DAILY_CONTACT_COUNT && ((Daily()->readyAtDayStart[contact / 8] >> (contact % 8)) & 1);
}

bool8 DailyContacts_GiftAtDayStart(u8 contact)
{
    return contact < DAILY_CONTACT_COUNT && ((Daily()->giftAtDayStart[contact / 8] >> (contact % 8)) & 1);
}

static void SetContactBit(u8 *bits, u8 contact)
{
    bits[contact / 8] |= 1 << (contact % 8);
}

// The contacts that are ready for a rematch, and those that hold a gift, right now. They are fixed for the day.
void DailyContacts_TakeSnapshots(void)
{
    struct WayfarerDailySlots *daily = Daily();
    s32 i;

    memset(daily->readyAtDayStart, 0, sizeof(daily->readyAtDayStart));
    memset(daily->giftAtDayStart, 0, sizeof(daily->giftAtDayStart));
    for (i = 0; i < REMATCH_SPECIAL_TRAINER_START && i < MAX_REMATCH_ENTRIES; i++)
    {
        if (gSaveBlock1Ptr->trainerRematches[i] != 0)
            SetContactBit(daily->readyAtDayStart, i);
        if (MatchCall_HnsTrainerHasGift(gRematchTable[i].trainerIds[0]))
            SetContactBit(daily->giftAtDayStart, i);
    }
    for (i = 0; i < WAYFARER_SEVII_REMATCH_FAMILY_COUNT; i++)
    {
        if (sWorldTrainerSeviiContacts[i] != 255 && WayfarerSeviiRematchIsReady(gWayfarerSeviiRematchFamilies[i].trainerIds[0]))
            SetContactBit(daily->readyAtDayStart, sWorldTrainerSeviiContacts[i]);
    }
    for (i = 0; i < WAYFARER_COAST_REMATCH_FAMILY_COUNT; i++)
    {
        if (WayfarerCoastRematchFamilyIsReady(i))
            SetContactBit(daily->readyAtDayStart, CONTACT_COAST_FIRST + i);
    }
}

// A wanderer who is ready or holds a gift as the day starts is always placed.
static bool8 IsGuaranteed(u16 slot)
{
    u8 contact = DailyContacts_IndexOf(sWorldTrainerSlots[slot].trainerId);

    return contact != DAILY_NO_CONTACT && (DailyContacts_ReadyAtDayStart(contact) || DailyContacts_GiftAtDayStart(contact));
}

// ---------- who stands where ----------
// The slot's absence chance in percent: 0 means someone is always there.
u8 DailyTrainers_AbsencePercent(u16 slot)
{
#if TESTING || defined(E2E_TESTING)
    if (gDailySlotsDebugFlags & DAILY_DEBUG_ABSENCE)
        return gDailySlotsDebugAbsence;
#endif
    return sWorldTrainerAbsence[WORLD_TRAINER_SLOT_ABSENCE(sWorldTrainerSlots[slot].flags)];
}

static bool8 IsEmptyToday(u16 slot)
{
#if TESTING || defined(E2E_TESTING)
    if (gDailySlotsDebugFlags & DAILY_DEBUG_NO_EMPTY)
        return FALSE;
#endif
    return DailySlots_Roll(DAILY_ROLL_TRAINER_EMPTY, slot, 0) % 100 < DailyTrainers_AbsencePercent(slot);
}

// Resolves every slot of a range at once (a group's slots, or a home range's one slot). out[i] is the range's i-th slot's occupant: DAILY_OCCUPANT_HOME
// when the slot shows its own home trainer (undefeated, or beaten today), DAILY_OCCUPANT_AWAY when it is
// rotating and empty, otherwise the occupant's slot row. Wanderers are the defeated trainers that were not
// first beaten today; the rotating slots are exactly their home slots. Wanderer j is the one whose home is
// the j-th rotating slot, so every array below is indexed by that order.
static void ResolveMembers(const u16 *members, u8 count, u32 seedKey, u16 *out)
{
    u32 seed = DailySlots_Roll(DAILY_ROLL_TRAINER_GROUP, seedKey, 0);
    u8 position[WORLD_TRAINER_GROUP_SLOTS_MAX];     // group position of each rotating slot / wanderer's home
    u32 key[WORLD_TRAINER_GROUP_SLOTS_MAX];
    u8 who[WORLD_TRAINER_GROUP_SLOTS_MAX];          // the wanderer assigned to each rotating slot
    bool8 guaranteed[WORLD_TRAINER_GROUP_SLOTS_MAX];    // per wanderer: ready or holding a gift as the day began
    bool8 away[WORLD_TRAINER_GROUP_SLOTS_MAX];      // per rotating slot: empty today
    u8 rotating = 0;
    u8 i, j, k, rank;

    for (i = 0; i < count; i++)
    {
        u16 slot = members[i];

        out[i] = DAILY_OCCUPANT_HOME;
        if (HasTrainerBeenFought(sWorldTrainerSlots[slot].trainerId) && !DailyTrainers_IsHomeBeaten(slot))
        {
            position[rotating] = i;
            key[rotating] = Hash32(seed, 0, DAILY_ROLL_TRAINER_GROUP, slot, 0);
            rotating++;
        }
    }
    // The hash permutation: the wanderer with the k-th smallest key goes to the k-th rotating slot.
    for (j = 0; j < rotating; j++)
    {
        u16 slot = members[position[j]];

        rank = 0;
        for (k = 0; k < rotating; k++)
        {
            if (key[k] < key[j] || (key[k] == key[j] && k < j))
                rank++;
        }
        who[rank] = j;
        guaranteed[j] = IsGuaranteed(slot);
        away[j] = IsEmptyToday(slot);
    }
    // Guaranteed places, in group order of the wanderers' home slots.
    for (j = 0; j < rotating; j++)
    {
        u8 at, swap;

        for (at = 0; who[at] != j; at++);
        if (!away[at] || !guaranteed[j])
            continue;
        // Swap with the first placed wanderer that is neither ready nor holding a gift.
        for (swap = 0; swap < rotating; swap++)
        {
            if (!away[swap] && !guaranteed[who[swap]])
                break;
        }
        if (swap < rotating)
        {
            away[at] = TRUE;
        }
        else
        {
            // Nobody can swap: the first empty slot in group order becomes occupied for it.
            for (swap = 0; !away[swap]; swap++);
        }
        away[swap] = FALSE;
        who[at] = who[swap];
        who[swap] = j;
    }
    for (k = 0; k < rotating; k++)
        out[position[k]] = away[k] ? DAILY_OCCUPANT_AWAY : members[position[who[k]]];
}

static void ResolveGroup(u8 group, u16 *out)
{
    ResolveMembers(&sWorldTrainerGroupSlots[sWorldTrainerGroups[group].first], sWorldTrainerGroups[group].count, group, out);
}

// A home range is its slot alone: out[0] is that slot's occupant today.
static void ResolveHome(u16 slot, u16 *out)
{
    ResolveMembers(&slot, 1, slot, out);
}

bool8 DailyTrainers_IsHomeRange(u16 slot)
{
    return (sWorldTrainerSlots[slot].flags & WORLD_TRAINER_SLOT_HOME) != 0;
}

u16 DailyTrainers_RangeSize(u16 slot)
{
    return DailyTrainers_IsHomeRange(slot) ? 1 : sWorldTrainerGroups[sWorldTrainerSlots[slot].group].count;
}

// out[i] is the occupant of the range's i-th slot (a home range has the one slot).
void DailyTrainers_ResolveRange(u16 slot, u16 *out)
{
    if (DailyTrainers_IsHomeRange(slot))
        ResolveHome(slot, out);
    else
        ResolveGroup(sWorldTrainerSlots[slot].group, out);
}

u8 DailyTrainers_GroupOf(u16 slot)
{
    return sWorldTrainerSlots[slot].group;
}

u16 DailyTrainers_GroupSize(u8 group)
{
    return sWorldTrainerGroups[group].count;
}

u16 DailyTrainers_GroupSlot(u8 group, u16 position)
{
    return sWorldTrainerGroupSlots[sWorldTrainerGroups[group].first + position];
}

void DailyTrainers_ResolveGroup(u8 group, u16 *out)
{
    ResolveGroup(group, out);
}

u8 DailyTrainers_MapOccupantCount(void)
{
    return sNumMapOccupants;
}

static u8 PositionInGroup(u8 group, u16 slot)
{
    const struct WorldTrainerGroup *info = &sWorldTrainerGroups[group];
    u8 i;

    for (i = 0; i < info->count; i++)
    {
        if (sWorldTrainerGroupSlots[info->first + i] == slot)
            return i;
    }
    return 0xFF;
}

// What stands in a slot today: DAILY_OCCUPANT_HOME, DAILY_OCCUPANT_AWAY or the occupant's slot row.
u16 DailyTrainers_ResolveOccupant(u16 slot)
{
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX];

    if (DailyTrainers_IsHomeRange(slot))
    {
        ResolveHome(slot, out);
        return out[0];
    }
    ResolveGroup(sWorldTrainerSlots[slot].group, out);
    return out[PositionInGroup(sWorldTrainerSlots[slot].group, slot)];
}

// ---------- the map-load hook ----------
static struct ObjectEventTemplate *SlotTemplate(u8 localId)
{
    if (localId == 0 || localId > gMapHeader.events->objectEventCount || localId > OBJECT_EVENT_TEMPLATES_COUNT)
        return NULL;
    return &gSaveBlock1Ptr->objectEventTemplates[localId - 1];
}

// A slot's objects take the occupant's sprite and script; the slot keeps its position, facing, movement and sight.
static void ApplyOccupant(u16 slot, u16 occupant)
{
    u8 object;

    for (object = 0; object < 2; object++)
    {
        struct ObjectEventTemplate *template = SlotTemplate(sWorldTrainerSlots[slot].localId[object]);

        if (template == NULL)
            continue;
        template->graphicsId = sWorldTrainerSlots[occupant].gfx[object];
        template->script = sWorldTrainerSlots[occupant].script[object];
    }
}

// Records a slot's occupant for the map's lifetime and gives the slot its sprite and script; an empty slot only records.
static void PlaceOccupant(u16 slot, u16 occupant)
{
    if (occupant == DAILY_OCCUPANT_HOME)
        return;
    if (sNumMapOccupants < WORLD_TRAINER_MAP_SLOTS_MAX)
    {
        sMapOccupants[sNumMapOccupants].slot = slot;
        sMapOccupants[sNumMapOccupants].occupant = occupant;
        sNumMapOccupants++;
    }
    if (occupant != DAILY_OCCUPANT_AWAY)
        ApplyOccupant(slot, occupant);
}

// Map load, and Continue after the saved scripts are reset to the authored ones: every trainer slot goes back to
// its authored sprite and script, then each range of the map (each group, each home slot) is resolved and its
// occupants applied.
void DailyTrainers_RewriteTemplates(void)
{
    u8 groups[(WORLD_TRAINER_GROUP_COUNT + 7) / 8];
    u16 key = CurrentMapKey();
    u16 first = MapLowerBound(key);
    u16 slot;
    u8 group;

    memset(groups, 0, sizeof(groups));
    sNumMapOccupants = 0;
    for (slot = first; slot < WORLD_TRAINER_SLOT_COUNT && SlotMapKey(slot) == key; slot++)
    {
        const struct WorldTrainerSlot *row = &sWorldTrainerSlots[slot];
        u8 object;

        for (object = 0; object < 2; object++)
        {
            struct ObjectEventTemplate *template = SlotTemplate(row->localId[object]);

            if (template == NULL)
                continue;
            template->graphicsId = row->gfx[object];
            template->script = row->script[object];
        }
        if (row->group != WORLD_TRAINER_GROUP_HOME)
            groups[row->group / 8] |= 1 << (row->group % 8);
    }
    for (group = 0; group < WORLD_TRAINER_GROUP_COUNT; group++)
    {
        u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX];
        bool8 resolved = FALSE;

        if (!((groups[group / 8] >> (group % 8)) & 1))
            continue;
        for (slot = first; slot < WORLD_TRAINER_SLOT_COUNT && SlotMapKey(slot) == key; slot++)
        {
            u16 occupant;

            if (sWorldTrainerSlots[slot].group != group)
                continue;
            if (!resolved)
            {
                ResolveGroup(group, out);
                resolved = TRUE;
            }
            occupant = out[PositionInGroup(group, slot)];
            PlaceOccupant(slot, occupant);
        }
    }
    for (slot = first; slot < WORLD_TRAINER_SLOT_COUNT && SlotMapKey(slot) == key; slot++)
    {
        u16 out[1];

        if (!DailyTrainers_IsHomeRange(slot))
            continue;
        ResolveHome(slot, out);
        PlaceOccupant(slot, out[0]);
    }
}

static const struct MapOccupant *OccupantOfLocalId(u8 localId)
{
    u8 i;

    for (i = 0; i < sNumMapOccupants; i++)
    {
        const struct WorldTrainerSlot *row = &sWorldTrainerSlots[sMapOccupants[i].slot];

        if (row->localId[0] == localId || (row->localId[1] != 0 && row->localId[1] == localId))
            return &sMapOccupants[i];
    }
    return NULL;
}

// Spawn check (TrySpawnObjectEvents): an empty rotating slot's template never spawns. A cleared slot still spawns:
// its occupant stays and speaks.
bool8 DailyTrainers_HideTemplate(const struct ObjectEventTemplate *template)
{
    const struct MapOccupant *entry;

    if (sNumMapOccupants == 0)
        return FALSE;
    entry = OccupantOfLocalId(template->localId);
    return entry != NULL && entry->occupant == DAILY_OCCUPANT_AWAY;
}

// Walkers never spawn on a trainer slot's tile, whoever stands there today.
bool8 DailyTrainers_IsSlotTile(s16 x, s16 y)
{
    u16 key = CurrentMapKey();
    u16 slot;

    for (slot = MapLowerBound(key); slot < WORLD_TRAINER_SLOT_COUNT && SlotMapKey(slot) == key; slot++)
    {
        u8 object;

        for (object = 0; object < 2; object++)
        {
            const struct ObjectEventTemplate *template = SlotTemplate(sWorldTrainerSlots[slot].localId[object]);

            if (template != NULL && template->x + MAP_OFFSET == x && template->y + MAP_OFFSET == y)
                return TRUE;
        }
    }
    return FALSE;
}

// ---------- battles ----------
static const struct MapOccupant *OccupantOfObject(u8 objectEventId)
{
    const struct ObjectEvent *object = &gObjectEvents[objectEventId];

    if (objectEventId >= OBJECT_EVENTS_COUNT || !object->active || sNumMapOccupants == 0
     || object->mapGroup != gSaveBlock1Ptr->location.mapGroup || object->mapNum != gSaveBlock1Ptr->location.mapNum)
        return NULL;
    return OccupantOfLocalId(object->localId);
}

// An object the hook rewrote for a wanderer. The battle scripts' flag checks read its slot, not the trainer's flag.
bool8 DailyTrainers_ObjectIsRotating(u8 objectEventId)
{
    const struct MapOccupant *entry = OccupantOfObject(objectEventId);

    return entry != NULL && entry->occupant != DAILY_OCCUPANT_AWAY;
}

// A ready occupant of a free slot is met in person: it was ready for a rematch as the day began and still is, and its
// slot has not been battled today. A rematch won at the slot clears it for the day; a trainer who becomes ready
// mid-day is offered from the next day.
bool8 DailyTrainers_ObjectOffersRematch(u8 objectEventId)
{
    const struct MapOccupant *entry = OccupantOfObject(objectEventId);
    u16 trainerId;
    u8 contact;

    if (entry == NULL || entry->occupant == DAILY_OCCUPANT_AWAY || DailyTrainers_IsCleared(entry->slot))
        return FALSE;
    trainerId = sWorldTrainerSlots[entry->occupant].trainerId;
    contact = DailyContacts_IndexOf(trainerId);
    return contact != DAILY_NO_CONTACT && DailyContacts_ReadyAtDayStart(contact) && ShouldTryRematchBattleForTrainerId(trainerId);
}

// Done for the day: the slot was battled, or the occupant holds a phone gift or a rematch, which its script hands
// over first (the talk path, not the sight battle).
bool8 DailyTrainers_ObjectIsDone(u8 objectEventId)
{
    const struct MapOccupant *entry = OccupantOfObject(objectEventId);

    if (entry == NULL || entry->occupant == DAILY_OCCUPANT_AWAY)
        return TRUE;
    return DailyTrainers_IsCleared(entry->slot) || MatchCall_HnsTrainerHasGift(sWorldTrainerSlots[entry->occupant].trainerId)
        || DailyTrainers_ObjectOffersRematch(objectEventId);
}

// The latest rematch team the occupant has fought: the team a wanderer brings, or its own trainer if it has no rematches.
u16 DailyTrainers_LatestTeam(u16 trainerId)
{
    s32 index = TrainerIdToRematchTableId(gRematchTable, trainerId);
    u16 latest;

    if (index >= 0 && index < MAX_REMATCH_ENTRIES)
    {
        latest = GetLastBeatenRematchTrainerId(gRematchTable[index].trainerIds[0]);
        return latest != 0 ? latest : trainerId;
    }
    if (WayfarerCoastRematchHasFamily(trainerId))
        return WayfarerCoastRematchGetLatest(trainerId);
    if (WayfarerSeviiRematchHasFamily(trainerId))
        return WayfarerSeviiRematchGetLatest(trainerId);
    return trainerId;
}

// A battle is being set up for the object `localId`: remember its slot so a win clears it. `opponent` is the trainer
// id the script loaded; a rotating occupant fights with its latest rematch team, so this returns the id to use.
u16 DailyTrainers_NoteBattle(u8 which, u8 localId, u16 opponent)
{
    u16 slot = DailyTrainers_FindSlot(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, localId);
    const struct MapOccupant *entry;

    which &= 1;
    if (which == 0)
    {
        sNotedSlotsPlusOne[1] = 0;
        sNotedRotating = 0;
    }
    sNotedSlotsPlusOne[which] = 0;
    if (slot == NO_TRAINER_SLOT)
        return opponent;
    entry = OccupantOfLocalId(localId);
    if (entry != NULL && entry->occupant == DAILY_OCCUPANT_AWAY)
        return opponent;
    // The slot's home trainer unless the hook put a wanderer in it; anything else is another script's battle.
    if (opponent != sWorldTrainerSlots[entry != NULL ? entry->occupant : slot].trainerId)
        return opponent;
    sNotedSlotsPlusOne[which] = slot + 1;
    if (entry == NULL)
        return opponent;
    sNotedRotating |= 1 << which;
    return DailyTrainers_LatestTeam(opponent);
}

bool8 DailyTrainers_NotedRotating(u8 which)
{
    return (sNotedRotating >> (which & 1)) & 1;
}

// The player won the battle just set up. A slot battled today stays cleared; a home trainer's first loss also
// freezes the slot's draw for the day (its wanderers start tomorrow). The notes of the next battle replace the
// "was a wanderer" mark.
void DailyTrainers_OnTrainersWon(void)
{
    u8 which;

    for (which = 0; which < 2; which++)
    {
        u16 slot = sNotedSlotsPlusOne[which];

        if (slot == 0)
            continue;
        slot--;
        DailyTrainers_SetCleared(slot);
        if (!DailyTrainers_NotedRotating(which))
            SetHomeBeaten(slot);
        sNotedSlotsPlusOne[which] = 0;
    }
    // sNotedRotating stays: the after-battle script is chosen next, and it is a wanderer's to skip.
}

// ---------- lookup for the phone ----------
static u16 FirstStage(u16 trainerId)
{
    s32 index = TrainerIdToRematchTableId(gRematchTable, trainerId);

    if (index >= 0)
        return gRematchTable[index].trainerIds[0];
    index = WayfarerSeviiRematchFamilyIndex(trainerId);
    if (index >= 0)
        return gWayfarerSeviiRematchFamilies[index].trainerIds[0];
    index = WayfarerCoastRematchFamilyIndex(trainerId);
    if (index >= 0)
        return WayfarerCoastRematchFamilyFirst(index);
    return trainerId;
}

static u16 SlotOfTrainer(u16 trainerId)
{
    u16 slot;

    for (slot = 0; slot < WORLD_TRAINER_SLOT_COUNT; slot++)
    {
        if (sWorldTrainerSlots[slot].trainerId == trainerId)
            return slot;
    }
    return NO_TRAINER_SLOT;
}

// The map and local id where a trainer stands today, or FALSE: the trainer is away, or its slot was already battled
// today. A contact whose slot is fixed stands at its home (it has no daily battle, so clearedToday does not apply).
bool8 DailyTrainers_WhereIsTrainerToday(u16 trainerId, u8 *mapGroup, u8 *mapNum, u8 *localId)
{
    u16 first = FirstStage(trainerId);
    u16 home = SlotOfTrainer(first);
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX];
    u16 place, i;
    u8 position;

    if (home == NO_TRAINER_SLOT)
    {
        for (i = 0; i < WORLD_TRAINER_FIXED_HOME_COUNT; i++)
        {
            if (sWorldTrainerFixedHomes[i].trainerId == first)
            {
                *mapGroup = sWorldTrainerFixedHomes[i].mapGroup;
                *mapNum = sWorldTrainerFixedHomes[i].mapNum;
                *localId = sWorldTrainerFixedHomes[i].localId;
                return TRUE;
            }
        }
        return FALSE;
    }
    DailyTrainers_ResolveRange(home, out);
    place = NO_TRAINER_SLOT;
    if (!HasTrainerBeenFought(first) || DailyTrainers_IsHomeBeaten(home))
    {
        place = home;       // at home: waiting to be beaten, or beaten today
    }
    else
    {
        for (position = 0; position < DailyTrainers_RangeSize(home); position++)
        {
            if (out[position] == home)
            {
                place = DailyTrainers_IsHomeRange(home) ? home : sWorldTrainerGroupSlots[sWorldTrainerGroups[sWorldTrainerSlots[home].group].first + position];
                break;
            }
        }
    }
    if (place == NO_TRAINER_SLOT || DailyTrainers_IsCleared(place))
        return FALSE;
    *mapGroup = sWorldTrainerSlots[place].mapGroup;
    *mapNum = sWorldTrainerSlots[place].mapNum;
    *localId = sWorldTrainerSlots[place].localId[0];
    return TRUE;
}

#endif // IS_WAYFARER

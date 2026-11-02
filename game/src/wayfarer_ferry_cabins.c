// The S.S. ferries' cabins (Wayfarer only): hosts, their requests, and sleeping. Spec:
// .product/specs/ss-ferries.md, Cabins. The voyage state and boarding live in wayfarer_ferry.c; the host
// table (content) is in data/wayfarer_ferry_hosts.h.

#include "global.h"
#include "wayfarer_ferry.h"

#if IS_WAYFARER

#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fake_rtc.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "field_screen_effect.h"
#include "item.h"
#include "move.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "wayfarer_celadon_hideout.h"
#include "wayfarer_origin.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/pokemon.h"
#include "constants/siirtc.h"
#include "constants/species.h"
#include "constants/wayfarer_ferry.h"

#include "data/wayfarer_ferry_hosts.h"

#define FRIENDSHIP_REQUIRED 220

#if defined(E2E_TESTING)
// E2E journeys pin the drawn hosts: each one's host table index plus one, then each one's parameter index.
// Zero: not pinned.
EWRAM_DATA u8 gFerryDebugHosts[4] = {0};
#endif

static struct WayfarerVoyage *Voyage(void)
{
    return &gPokemonStoragePtr->voyage;
}

// ---------- Drawing ----------

// Boarding: two distinct hosts of the ship's pool, each with one parameter of its list.
void Ferry_DrawHosts(u8 ship)
{
    struct WayfarerVoyage *voyage = Voyage();
    u8 first = 0, count = 0, i, a, b;

    for (i = 0; i < ARRAY_COUNT(sHosts); i++)
    {
        if (sHosts[i].ship != ship)
            continue;
        if (count == 0)
            first = i;
        count++;
    }
    a = Random() % count;
    b = Random() % (count - 1);
    if (b >= a)
        b++;
    voyage->hosts[0] = first + a + 1;
    voyage->hosts[1] = first + b + 1;
    for (i = 0; i < 2; i++)
        voyage->hostParams[i] = Random() % sHosts[voyage->hosts[i] - 1].paramCount;
#if defined(E2E_TESTING)
    if (gFerryDebugHosts[0] != 0)
    {
        memcpy(voyage->hosts, gFerryDebugHosts, 2);
        memcpy(voyage->hostParams, &gFerryDebugHosts[2], 2);
    }
#endif
}

// ---------- Placement ----------

static struct ObjectEventTemplate *FindTemplate(u8 localId)
{
    u32 i;

    for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
        if (gSaveBlock1Ptr->objectEventTemplates[i].localId == localId)
            return &gSaveBlock1Ptr->objectEventTemplates[i];
    return NULL;
}

// Map load, after the temp flags are cleared: each host spot of the map takes its host's graphics, or is
// hidden when the host wasn't drawn for this voyage (or the player isn't aboard). Runs from Ferry_OnMapLoad.
void Ferry_PlaceHosts(void)
{
    const struct WayfarerVoyage *voyage = Voyage();
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sHosts); i++)
    {
        const struct FerryHost *host = &sHosts[i];
        struct ObjectEventTemplate *template;
        bool8 drawn = voyage->ship == host->ship && (voyage->hosts[0] == i + 1 || voyage->hosts[1] == i + 1);

        if (host->mapGroup != gSaveBlock1Ptr->location.mapGroup || host->mapNum != gSaveBlock1Ptr->location.mapNum)
            continue;
        template = FindTemplate(host->localId);
        if (template == NULL)
            continue;
        template->graphicsId = host->gfx;
        template->flagId = drawn ? 0 : FERRY_HIDE_FLAG;
        if (!drawn)
            FlagSet(FERRY_HIDE_FLAG);
    }
}

// ---------- Talking ----------

// Which of the voyage's two hosts the player talked to, 0xFF if none.
static u8 TalkedSlot(void)
{
    const struct WayfarerVoyage *voyage = Voyage();
    u8 slot;

    for (slot = 0; slot < 2; slot++)
    {
        const struct FerryHost *host;

        if (voyage->ship == FERRY_SHIP_NONE || voyage->hosts[slot] == 0)
            continue;
        host = &sHosts[voyage->hosts[slot] - 1];
        if (host->mapGroup == gSaveBlock1Ptr->location.mapGroup && host->mapNum == gSaveBlock1Ptr->location.mapNum
         && host->localId == gSpecialVar_LastTalked)
            return slot;
    }
    return 0xFF;
}

static const struct FerryHost *HostOf(u8 slot)
{
    return &sHosts[Voyage()->hosts[slot] - 1];
}

static u16 ParamOf(u8 slot)
{
    return HostOf(slot)->params[Voyage()->hostParams[slot]];
}

// Writes the parameter's name to STR_VAR_1 for the request text.
static void BufferParam(u8 kind, u16 param)
{
    switch (kind)
    {
    case FERRY_REQ_ITEM:
    case FERRY_REQ_KEY_ITEM:
        CopyItemName(param, gStringVar1);
        break;
    case FERRY_REQ_TYPE:
        StringCopy(gStringVar1, gTypesInfo[param].name);
        break;
    case FERRY_REQ_SPECIES:
        StringCopy(gStringVar1, GetSpeciesName(param));
        break;
    case FERRY_REQ_COLOUR:
        StringCopy(gStringVar1, sColourNames[param]);
        break;
    case FERRY_REQ_MOVE:
        StringCopy(gStringVar1, GetMoveName(param));
        break;
    }
}

// Talking to a host: small talk once their request is settled, the bed or the sleep once the cabin is the
// player's, otherwise their request. The text to show is left in gStringVar4.
u16 Ferry_HostTalk(void)
{
    const struct WayfarerVoyage *voyage = Voyage();
    u8 slot = TalkedSlot();
    const struct FerryHost *host;

    if (slot == 0xFF)
        return FERRY_HOST_NONE;
    host = HostOf(slot);
    if ((voyage->cabin >> slot) & 1)
        return host->hasBed ? FERRY_HOST_BED : FERRY_HOST_SLEEP;
    if ((voyage->hostsDone >> slot) & 1)
    {
        StringCopy(gStringVar4, host->chat);
        return FERRY_HOST_CHAT;
    }
    BufferParam(host->kind, ParamOf(slot));
    StringExpandPlaceholders(gStringVar3, sRequestText[host->kind]);
    StringCopy(gStringVar4, host->line);
    StringAppend(gStringVar4, gStringVar3);
    return FERRY_HOST_ASK;
}

u16 Ferry_GetHostKind(void)
{
    u8 slot = TalkedSlot();

    return slot == 0xFF ? 0xFF : HostOf(slot)->kind;
}

// ---------- Requests ----------

// A party Pokemon (never an Egg) for which `check` holds.
static bool8 PartyHas(bool8 (*check)(struct Pokemon *mon, u16 species, u16 param), u16 param)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);

        if (species != SPECIES_NONE && species != SPECIES_EGG && check(&gPlayerParty[i], species, param))
            return TRUE;
    }
    return FALSE;
}

static bool8 HasType(struct Pokemon *mon, u16 species, u16 type)
{
    return gSpeciesInfo[species].types[0] == type || gSpeciesInfo[species].types[1] == type;
}

static bool8 HasColour(struct Pokemon *mon, u16 species, u16 colour)
{
    return gSpeciesInfo[species].bodyColor == colour;
}

static bool8 IsSpecies(struct Pokemon *mon, u16 species, u16 wanted)
{
    return species == wanted;
}

static bool8 KnowsMove(struct Pokemon *mon, u16 species, u16 move)
{
    u32 i;

    for (i = 0; i < MAX_MON_MOVES; i++)
        if (GetMonData(mon, MON_DATA_MOVE1 + i) == move)
            return TRUE;
    return FALSE;
}

static bool8 IsFullyEvolved(struct Pokemon *mon, u16 species, u16 unused)
{
    return GetSpeciesEvolutions(species) == NULL && GetSpeciesPreEvolution(species) != SPECIES_NONE;
}

static bool8 IsFromYourRegion(struct Pokemon *mon, u16 species, u16 unused)
{
    switch (WayfarerGetStartingOriginId())
    {
    // A species outside a regional Pokedex has no number in it (zero).
    case ORIGIN_NEW_BARK:
        return SpeciesToJohtoPokedexNum(species) != 0;
    case ORIGIN_LITTLEROOT:
        return SpeciesToHoennPokedexNum(species) != 0;
    default:
        return SpeciesToKantoPokedexNum(species) != 0;
    }
}

static bool8 IsFriendly(struct Pokemon *mon, u16 species, u16 unused)
{
    return GetMonData(mon, MON_DATA_FRIENDSHIP) >= FRIENDSHIP_REQUIRED;
}

// Whether a kind's request is met by what the player has now, for the kinds with a plain check. An item
// is taken from the Bag on success. Battles and the quiz are run by the script.
bool8 Ferry_RequestMet(u8 kind, u16 param)
{
    switch (kind)
    {
    case FERRY_REQ_ITEM:
        return CheckBagHasItem(param, 1) && RemoveBagItem(param, 1);
    case FERRY_REQ_KEY_ITEM:
        return CheckBagHasItem(param, 1);
    case FERRY_REQ_TYPE:
        return PartyHas(HasType, param);
    case FERRY_REQ_SPECIES:
        return PartyHas(IsSpecies, param);
    case FERRY_REQ_COLOUR:
        return PartyHas(HasColour, param);
    case FERRY_REQ_MOVE:
        return PartyHas(KnowsMove, param);
    case FERRY_REQ_EVOLVED:
        return PartyHas(IsFullyEvolved, 0);
    case FERRY_REQ_REGION:
        return PartyHas(IsFromYourRegion, 0);
    case FERRY_REQ_FRIENDSHIP:
        return PartyHas(IsFriendly, 0);
    case FERRY_REQ_COIN:
        return Random() < 0x8000;
    }
    return FALSE;
}

u16 Ferry_CheckRequest(void)
{
    u8 slot = TalkedSlot();

    return Ferry_RequestMet(HostOf(slot)->kind, ParamOf(slot));
}

// The one-try kinds are settled whether or not they were met.
static bool8 IsOneTry(u8 kind)
{
    return kind == FERRY_REQ_BATTLE || kind == FERRY_REQ_DOUBLE_BATTLE || kind == FERRY_REQ_QUIZ || kind == FERRY_REQ_COIN;
}

// The request is met: the host's cabin is the player's, and their request is settled.
void Ferry_HostMet(void)
{
    u8 slot = TalkedSlot();

    Voyage()->cabin |= 1 << slot;
    Voyage()->hostsDone |= 1 << slot;
}

void Ferry_HostMissed(void)
{
    u8 slot = TalkedSlot();

    if (IsOneTry(HostOf(slot)->kind))
        Voyage()->hostsDone |= 1 << slot;
}

u16 Ferry_IsHostOneTry(void)
{
    return IsOneTry(HostOf(TalkedSlot())->kind);
}

// A host battle never whites out: losing restores every fainted party Pokemon to 1 HP.
static void ReviveFainted(void)
{
    u32 i;
    u16 hp = 1;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);

        if (species != SPECIES_NONE && species != SPECIES_EGG && GetMonData(&gPlayerParty[i], MON_DATA_HP) == 0)
            SetMonData(&gPlayerParty[i], MON_DATA_HP, &hp);
    }
}

// ---------- Battles ----------

static const u8 sHostDefeatText[] = _("Well fought!");

// Whether the borrowed trainer was already beaten when the battle was set up: a win marks the trainer beaten,
// which would undo that trainer's own place in the world, so the flag goes back to what it was.
static EWRAM_DATA bool8 sTrainerWasBeaten = FALSE;

// Sets up the host's battle against the borrowed trainer's party (the parameter) and lets a loss end it without
// a whiteout. FALSE when the player can't fight (no usable Pokemon, or one for a double battle).
u16 Ferry_SetUpHostBattle(void)
{
    u8 slot = TalkedSlot();
    u16 trainer = ParamOf(slot);

    if (HostOf(slot)->kind == FERRY_REQ_DOUBLE_BATTLE ? GetMonsStateToDoubles() != PLAYER_HAS_TWO_USABLE_MONS
                                                      : !WayfarerCanStartOrdinaryBattleForScript())
        return FALSE;
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.opponentA = trainer;
    TRAINER_BATTLE_PARAM.defeatTextA = (u8 *)sHostDefeatText;
    sTrainerWasBeaten = HasTrainerBeenFought(trainer);
    FlagSet(FLAG_NO_WHITEOUT);
    return TRUE;
}

// After the battle: TRUE if the player won. A loss restores the fainted Pokemon to 1 HP.
u16 Ferry_EndHostBattle(void)
{
    u16 trainer = ParamOf(TalkedSlot());

    FlagClear(FLAG_NO_WHITEOUT);
    if (!sTrainerWasBeaten)
        ClearTrainerFlag(trainer);
    if (gBattleOutcome == B_OUTCOME_WON)
        return TRUE;
    ReviveFainted();
    return FALSE;
}

// ---------- The quiz ----------

// The question goes to gStringVar4 and the three choices to STR_VAR_1 to 3.
void Ferry_BufferQuiz(void)
{
    const struct FerryQuiz *quiz = &sQuiz[ParamOf(TalkedSlot())];

    StringCopy(gStringVar4, quiz->question);
    StringCopy(gStringVar1, quiz->choices[0]);
    StringCopy(gStringVar2, quiz->choices[1]);
    StringCopy(gStringVar3, quiz->choices[2]);
}

// The player's choice (0 to 2) is in VAR_0x8004.
u16 Ferry_QuizCorrect(void)
{
    return gSpecialVar_0x8004 == sQuiz[ParamOf(TalkedSlot())].answer;
}

// ---------- Sleeping ----------

void Ferry_BufferSleepPort(void)
{
    const struct WayfarerVoyage *voyage = Voyage();

    Ferry_BufferPortName(Ferry_GetLoopPort(voyage->ship, voyage->stop));
}

// The clock moves to the voyage's next event and no further, so sleeping never skips time in port. Only at sea.
bool8 Ferry_SleepUntilEvent(void)
{
    const struct WayfarerVoyage *voyage = Voyage();
    u32 now = Ferry_GetNow();
    u32 minutes;

    if (!Ferry_IsAtSea())
        return FALSE;
    minutes = voyage->eventAt > now ? voyage->eventAt - now : 0;
    FakeRtc_AdvanceTimeBy(0, minutes / MINUTES_PER_HOUR, minutes % MINUTES_PER_HOUR, 0);
    return TRUE;
}

// Yes to the bed: the clock jumps to the arrival and the map reloads with the player where they stood. The
// field poll then announces the arrival.
u16 Ferry_Sleep(void)
{
    s16 x, y;

    if (!Ferry_SleepUntilEvent())
        return FALSE;
    PlayerGetDestCoords(&x, &y);
    SetWarpDestination(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE, x - MAP_OFFSET, y - MAP_OFFSET);
    DoWarp();
    ResetInitialPlayerAvatarState();
    return TRUE;
}

// The bed of this map belongs to a drawn host; it is the player's once that host's cabin is.
u16 Ferry_IsBedMine(void)
{
    const struct WayfarerVoyage *voyage = Voyage();
    u8 slot;

    for (slot = 0; slot < 2; slot++)
    {
        const struct FerryHost *host;

        if (voyage->ship == FERRY_SHIP_NONE || voyage->hosts[slot] == 0 || !((voyage->cabin >> slot) & 1))
            continue;
        host = HostOf(slot);
        if (host->hasBed && host->mapGroup == gSaveBlock1Ptr->location.mapGroup && host->mapNum == gSaveBlock1Ptr->location.mapNum)
            return TRUE;
    }
    return FALSE;
}

#if TESTING
// The host table for the mechanics tests.
u8 Test_Ferry_HostCount(void)
{
    return ARRAY_COUNT(sHosts);
}

u8 Test_Ferry_HostShip(u8 index)
{
    return sHosts[index].ship;
}

u8 Test_Ferry_HostKind(u8 index)
{
    return sHosts[index].kind;
}

u8 Test_Ferry_HostParamCount(u8 index)
{
    return sHosts[index].paramCount;
}

// The first host of a ship with a request kind, 0xFF if there is none.
u8 Test_Ferry_FindHost(u8 ship, u8 kind)
{
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sHosts); i++)
        if (sHosts[i].ship == ship && sHosts[i].kind == kind)
            return i;
    return 0xFF;
}

// Stands the player before a host's spot, as talking to it does.
void Test_Ferry_StandBeforeHost(u8 index)
{
    gSaveBlock1Ptr->location.mapGroup = sHosts[index].mapGroup;
    gSaveBlock1Ptr->location.mapNum = sHosts[index].mapNum;
    gSpecialVar_LastTalked = sHosts[index].localId;
}
#endif

#endif // IS_WAYFARER

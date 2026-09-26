#include "global.h"

#if IS_WAYFARER
#include "event_data.h"
#include "script.h"
#include "wayfarer_kanto_opening.h"
#include "wayfarer_kanto_opening_town.h"
#include "constants/vars.h"

// One Coast-bank variable holds the Pallet origin's FRLG town facts. The
// Coast bank is zeroed at new game, so every fact starts clear. FRLG keeps
// these in VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY and FLAG_GOT_POTION_ON_ROUTE_1,
// which are absent or zero in HNS. Scripts reach them through callnative, so
// these hooks need no data/specials.inc entries.

#define PALLET_TOWN_SIGN_LADY_MASK   0x0003 // 0 not shown, 1 Trainer Tips seen, 2 done
#define PALLET_TOWN_ROUTE1_POTION    0x0004
#define PALLET_TOWN_SIGN_LADY_DONE   2

static u16 GetState(void)
{
    return VarGet(VAR_WAYFARER_PALLET_TOWN_STATE);
}

static bool32 HasBits(u16 bits)
{
    return WayfarerKanto_IsPalletOrigin() && (GetState() & bits) == bits;
}

static void SetBits(u16 bits)
{
    if (WayfarerKanto_IsPalletOrigin())
        VarSet(VAR_WAYFARER_PALLET_TOWN_STATE, GetState() | bits);
}

void WayfarerKantoTown_GetSignLadyState(struct ScriptContext *ctx)
{
    gSpecialVar_Result = WayfarerKanto_IsPalletOrigin() ? (GetState() & PALLET_TOWN_SIGN_LADY_MASK) : 0;
}

// Mirrors FRLG setvar VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, VAR_0x8004.
void WayfarerKantoTown_SetSignLadyState(struct ScriptContext *ctx)
{
    u16 value = gSpecialVar_0x8004;

    if (!WayfarerKanto_IsPalletOrigin() || value > PALLET_TOWN_SIGN_LADY_DONE)
        return;
    VarSet(VAR_WAYFARER_PALLET_TOWN_STATE, (GetState() & ~PALLET_TOWN_SIGN_LADY_MASK) | value);
}

void WayfarerKantoTown_HasRoute1Potion(struct ScriptContext *ctx)
{
    gSpecialVar_Result = HasBits(PALLET_TOWN_ROUTE1_POTION);
}

void WayfarerKantoTown_MarkRoute1Potion(struct ScriptContext *ctx)
{
    SetBits(PALLET_TOWN_ROUTE1_POTION);
}
#endif

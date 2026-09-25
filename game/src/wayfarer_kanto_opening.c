#include "global.h"

#if IS_WAYFARER
#include "battle_main.h"
#include "challenge_menu.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "starter_choose.h"
#include "string_util.h"
#include "wayfarer_kanto_opening.h"
#include "wayfarer_origin.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/pokemon.h"

static struct WayfarerPalletOpeningState *GetOpening(void)
{
    return &gSaveBlock3Ptr->wayfarerPalletOpening;
}

void WayfarerKanto_InitializeOpening(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();

    memset(opening, 0, sizeof(*opening));
    opening->phase = PALLET_OPENING_HOME;
    opening->starterSlot = KANTO_STARTER_SLOT_NONE;
}

u16 WayfarerKanto_IsPalletOrigin(void)
{
    return WayfarerGetStartingOriginId() == ORIGIN_PALLET;
}

u16 WayfarerKanto_GetPhase(void)
{
    return WayfarerKanto_IsPalletOrigin() ? GetOpening()->phase : PALLET_OPENING_HOME;
}

u16 WayfarerKanto_GetStarterSlot(void)
{
    return WayfarerKanto_IsPalletOrigin() ? GetOpening()->starterSlot : KANTO_STARTER_SLOT_NONE;
}

u16 WayfarerKanto_GetCounterSlot(void)
{
    static const u8 counterSlots[] = {
        KANTO_STARTER_SLOT_CHARMANDER,
        KANTO_STARTER_SLOT_SQUIRTLE,
        KANTO_STARTER_SLOT_BULBASAUR,
    };
    u16 slot = WayfarerKanto_GetStarterSlot();

    return slot < ARRAY_COUNT(counterSlots) ? counterSlots[slot] : KANTO_STARTER_SLOT_NONE;
}

u16 WayfarerKanto_GetSelectedStarterSpecies(void)
{
    u16 slot = gSpecialVar_0x8004;

    if (!WayfarerKanto_IsPalletOrigin() || slot >= KANTO_STARTER_SLOT_NONE)
        return SPECIES_NONE;
    return GetKantoStarterPokemon(slot);
}

void WayfarerKanto_BufferSelectedStarterType(void)
{
    u16 species = WayfarerKanto_GetSelectedStarterSpecies();

    if (species == SPECIES_NONE)
        gStringVar2[0] = EOS;
    else
        StringCopy(gStringVar2, gTypesInfo[GetSpeciesType(species, 0)].name);
}

u16 WayfarerKanto_HasReceipt(void)
{
    return WayfarerKanto_IsPalletOrigin() && gSpecialVar_0x8004 != 0
        && (GetOpening()->receipts & gSpecialVar_0x8004) == gSpecialVar_0x8004;
}

u16 WayfarerKanto_IsOpeningComplete(void)
{
    return WayfarerKanto_IsPalletOrigin()
        && GetOpening()->phase == PALLET_OPENING_COMPLETE
        && (GetOpening()->receipts & PALLET_RECEIPT_COMPLETE);
}

u16 WayfarerKanto_HasFirstBattleResolved(void)
{
    return WayfarerKanto_IsPalletOrigin()
        && (GetOpening()->receipts & PALLET_RECEIPT_FIRST_BATTLE);
}

u16 WayfarerKanto_TryStageInterception(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();

    if (!WayfarerKanto_IsPalletOrigin() || opening->phase > PALLET_OPENING_OAK_INTERCEPTION_AVAILABLE)
        return FALSE;
    if (opening->phase == PALLET_OPENING_HOME)
        opening->phase = PALLET_OPENING_OAK_INTERCEPTION_AVAILABLE;
    return TRUE;
}

u16 WayfarerKanto_CommitLabStarterChoice(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();

    if (!WayfarerKanto_IsPalletOrigin() || opening->phase < PALLET_OPENING_OAK_INTERCEPTION_AVAILABLE)
        return FALSE;
    if (opening->phase == PALLET_OPENING_OAK_INTERCEPTION_AVAILABLE)
        opening->phase = PALLET_OPENING_LAB_STARTER_CHOICE;
    return TRUE;
}

u16 WayfarerKanto_CommitStarterSlot(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();
    u16 slot = gSpecialVar_0x8004;

    if (!WayfarerKanto_IsPalletOrigin() || opening->phase < PALLET_OPENING_LAB_STARTER_CHOICE
     || slot >= KANTO_STARTER_SLOT_NONE)
        return FALSE;
    if (opening->starterSlot == KANTO_STARTER_SLOT_NONE)
        opening->starterSlot = slot;
    return opening->starterSlot == slot;
}

u16 WayfarerKanto_TryGiveStarter(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();
    struct Pokemon mon;
    u16 species;

    if (!WayfarerKanto_IsPalletOrigin() || opening->phase < PALLET_OPENING_LAB_STARTER_CHOICE
     || opening->starterSlot >= KANTO_STARTER_SLOT_NONE)
        return FALSE;
    if (opening->receipts & PALLET_RECEIPT_STARTER)
        return TRUE;
    if (gPlayerPartyCount >= GetMaxPartySize())
        return FALSE;
    species = GetKantoStarterPokemon(opening->starterSlot);
    CreateRandomMon(&mon, species, 5);
    if (GiveScriptedMonToPlayer(&mon, PARTY_SIZE) != MON_GIVEN_TO_PARTY)
        return FALSE;
    opening->receipts |= PALLET_RECEIPT_STARTER;
    if (opening->phase < PALLET_OPENING_STARTER_RECEIVED)
        opening->phase = PALLET_OPENING_STARTER_RECEIVED;
    FlagSet(FLAG_SYS_POKEMON_GET);
    return TRUE;
}

u16 WayfarerKanto_ResolveFirstBattle(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();

    if (!WayfarerKanto_IsPalletOrigin() || !(opening->receipts & PALLET_RECEIPT_STARTER))
        return FALSE;
    opening->receipts |= PALLET_RECEIPT_FIRST_BATTLE;
    if (opening->phase < PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED)
        opening->phase = PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED;
    return TRUE;
}

u16 WayfarerKanto_TryReceiveParcel(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();

    if (!WayfarerKanto_IsPalletOrigin() || !(opening->receipts & PALLET_RECEIPT_FIRST_BATTLE))
        return FALSE;
    if (opening->receipts & PALLET_RECEIPT_PARCEL_RECEIVED)
        return TRUE;
    if (!CheckBagHasItem(ITEM_OAKS_PARCEL, 1) && !AddBagItem(ITEM_OAKS_PARCEL, 1))
        return FALSE;
    opening->receipts |= PALLET_RECEIPT_PARCEL_RECEIVED;
    if (opening->phase < PALLET_OPENING_PARCEL_RECEIVED)
        opening->phase = PALLET_OPENING_PARCEL_RECEIVED;
    return TRUE;
}

u16 WayfarerKanto_TryAcceptParcel(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();

    if (!WayfarerKanto_IsPalletOrigin() || !(opening->receipts & PALLET_RECEIPT_PARCEL_RECEIVED))
        return FALSE;
    if (opening->receipts & PALLET_RECEIPT_PARCEL_ACCEPTED)
        return TRUE;
    if (!CheckBagHasItem(ITEM_OAKS_PARCEL, 1) || !RemoveBagItem(ITEM_OAKS_PARCEL, 1))
        return FALSE;
    opening->receipts |= PALLET_RECEIPT_PARCEL_ACCEPTED;
    if (opening->phase < PALLET_OPENING_PARCEL_ACCEPTED)
        opening->phase = PALLET_OPENING_PARCEL_ACCEPTED;
    return TRUE;
}

u16 WayfarerKanto_CommitBlueArrival(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();

    if (!WayfarerKanto_IsPalletOrigin() || !(opening->receipts & PALLET_RECEIPT_PARCEL_ACCEPTED))
        return FALSE;
    opening->receipts |= PALLET_RECEIPT_BLUE_ARRIVED;
    return TRUE;
}

u16 WayfarerKanto_TryGrantPokedex(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();

    if (!WayfarerKanto_IsPalletOrigin() || !(opening->receipts & PALLET_RECEIPT_BLUE_ARRIVED))
        return FALSE;
    if (!(opening->receipts & PALLET_RECEIPT_POKEDEX))
    {
        WayfarerGrantSharedPokedex();
        opening->receipts |= PALLET_RECEIPT_POKEDEX;
        if (opening->phase < PALLET_OPENING_POKEDEX_HANDOFF_COMPLETE)
            opening->phase = PALLET_OPENING_POKEDEX_HANDOFF_COMPLETE;
    }
    return TRUE;
}

u16 WayfarerKanto_TryGrantPokeBalls(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();

    if (!WayfarerKanto_IsPalletOrigin() || !(opening->receipts & PALLET_RECEIPT_POKEDEX))
        return FALSE;
    if (opening->receipts & PALLET_RECEIPT_POKE_BALLS)
        return TRUE;
    if (!AddBagItem(ITEM_POKE_BALL, 5))
        return FALSE;
    opening->receipts |= PALLET_RECEIPT_POKE_BALLS;
    if (opening->phase < PALLET_OPENING_POKE_BALLS_RECEIVED)
        opening->phase = PALLET_OPENING_POKE_BALLS_RECEIVED;
    return TRUE;
}

u16 WayfarerKanto_CommitPresentationDone(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();

    if (!WayfarerKanto_IsPalletOrigin() || !(opening->receipts & PALLET_RECEIPT_POKE_BALLS))
        return FALSE;
    opening->receipts |= PALLET_RECEIPT_PRESENTATION_DONE;
    return TRUE;
}

u16 WayfarerKanto_CompleteOpening(void)
{
    struct WayfarerPalletOpeningState *opening = GetOpening();
    const u16 required = PALLET_RECEIPT_STARTER | PALLET_RECEIPT_FIRST_BATTLE
                       | PALLET_RECEIPT_PARCEL_RECEIVED | PALLET_RECEIPT_PARCEL_ACCEPTED
                       | PALLET_RECEIPT_BLUE_ARRIVED
                       | PALLET_RECEIPT_POKEDEX | PALLET_RECEIPT_POKE_BALLS
                       | PALLET_RECEIPT_PRESENTATION_DONE;

    if (!WayfarerKanto_IsPalletOrigin() || opening->phase < PALLET_OPENING_POKE_BALLS_RECEIVED
     || (opening->receipts & required) != required)
        return FALSE;
    opening->receipts |= PALLET_RECEIPT_COMPLETE;
    opening->phase = PALLET_OPENING_COMPLETE;
    return TRUE;
}
#endif

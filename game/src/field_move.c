#include "global.h"
#include "challenge_menu.h"
#include "event_data.h"
#include "field_move.h"
#include "fldeff.h"
#include "fldeff_misc.h"
#include "item.h"
#include "party_menu.h"
#include "pokemon.h"
#include "constants/field_move.h"
#include "constants/moves.h"
#include "constants/party_menu.h"

static bool32 IsFieldMoveUnlocked_Cut(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_Flash(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_RockSmash(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_Strength(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_Surf(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_Fly(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_Dive(void)
{
    if (IS_HNS || IS_FRLG)
        return FlagGet(FLAG_BADGE07_GET);

    return TRUE;
}

static bool32 IsFieldMoveUnlocked_Waterfall(void)
{
    return TRUE;
}

#if IS_HNS
static bool32 IsFieldMoveUnlocked_Whirlpool(void)
{
    return TRUE;
}
#endif

static enum FieldMoveUserResult ResolveMoveUser(enum Move move, bool32 allowCompatibilityWithoutItem, u8 *partyIndex)
{
    enum Item item = GetTMHMItemIdFromMoveId(move);
    bool32 hasItem = item != ITEM_NONE && CheckBagHasItem(item, 1);
    u32 partyCount = GetMaxPartySize();

    *partyIndex = PARTY_SIZE;

    for (u32 i = 0; i < partyCount; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            break;
        if (!GetMonData(mon, MON_DATA_IS_EGG) && MonKnowsMove(mon, move))
        {
            *partyIndex = i;
            return FIELD_MOVE_USER_FOUND;
        }
    }

    if (item != ITEM_NONE && !hasItem)
        return FIELD_MOVE_USER_MISSING_ITEM;
    if (item == ITEM_NONE && !allowCompatibilityWithoutItem)
        return FIELD_MOVE_USER_NO_ELIGIBLE_MON;

    for (u32 i = 0; i < partyCount; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u16 species = GetMonData(mon, MON_DATA_SPECIES);

        if (species == SPECIES_NONE)
            break;
        if (!GetMonData(mon, MON_DATA_IS_EGG) && CanLearnTeachableMove(species, move))
        {
            *partyIndex = i;
            return FIELD_MOVE_USER_FOUND;
        }
    }

    if (hasItem && HMsOverwriteOptionActive())
    {
        for (u32 i = 0; i < partyCount; i++)
        {
            struct Pokemon *mon = &gPlayerParty[i];

            if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
                break;
            if (!GetMonData(mon, MON_DATA_IS_EGG))
            {
                *partyIndex = i;
                return FIELD_MOVE_USER_FOUND;
            }
        }
    }

    return FIELD_MOVE_USER_NO_ELIGIBLE_MON;
}

enum FieldMoveUserResult ResolveFieldMoveUser(enum Move move, u8 *partyIndex)
{
    return ResolveMoveUser(move, FALSE, partyIndex);
}

enum FieldMoveUserResult ResolvePartyMoveUser(enum Move move, u8 *partyIndex)
{
    return ResolveMoveUser(move, TRUE, partyIndex);
}

bool32 CanPartyMonUseFieldMove(struct Pokemon *mon, enum Move move)
{
    enum Item item;
    u16 species;

    species = GetMonData(mon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
        return FALSE;
    if (MonKnowsMove(mon, move))
        return TRUE;

    item = GetTMHMItemIdFromMoveId(move);
    if (item == ITEM_NONE || !CheckBagHasItem(item, 1))
        return FALSE;

    return CanLearnTeachableMove(species, move) || HMsOverwriteOptionActive();
}

bool32 IsFieldMovePartyMenuAction(enum FieldMove fieldMove)
{
    return fieldMove == FIELD_MOVE_FLY
        || fieldMove == FIELD_MOVE_FLASH
        || !IsMoveHM(FieldMove_GetMoveId(fieldMove));
}

#if OW_ROCK_CLIMB_FIELD_MOVE == TRUE
static bool32 IsFieldMoveUnlocked_RockClimb(void)
{
    return TRUE;
}
#endif

static bool32 IsFieldMoveUnlocked_Teleport(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_Dig(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_SecretPower(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_MilkDrink(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_SoftBoiled(void)
{
    return TRUE;
}

static bool32 IsFieldMoveUnlocked_SweetScent(void)
{
    return TRUE;
}

#if OW_DEFOG_FIELD_MOVE == TRUE
static bool32 IsFieldMoveUnlocked_Defog(void)
{
    return TRUE;
}
#endif

const struct FieldMoveInfo gFieldMoveInfo[FIELD_MOVES_COUNT] =
{
    [FIELD_MOVE_CUT] =
    {
        .fieldMoveFunc = SetUpFieldMove_Cut,
        .isUnlockedFunc = IsFieldMoveUnlocked_Cut,
        .moveID = MOVE_CUT,
        .partyMsgID = PARTY_MSG_NOTHING_TO_CUT,
    },

    [FIELD_MOVE_FLASH] =
    {
        .fieldMoveFunc = SetUpFieldMove_Flash,
        .isUnlockedFunc = IsFieldMoveUnlocked_Flash,
        .moveID = MOVE_FLASH,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },

    [FIELD_MOVE_ROCK_SMASH] =
    {
        .fieldMoveFunc = SetUpFieldMove_RockSmash,
        .isUnlockedFunc = IsFieldMoveUnlocked_RockSmash,
        .moveID = MOVE_ROCK_SMASH,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },

    [FIELD_MOVE_STRENGTH] =
    {
        .fieldMoveFunc = SetUpFieldMove_Strength,
        .isUnlockedFunc = IsFieldMoveUnlocked_Strength,
        .moveID = MOVE_STRENGTH,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },

    [FIELD_MOVE_SURF] =
    {
        .fieldMoveFunc = SetUpFieldMove_Surf,
        .isUnlockedFunc = IsFieldMoveUnlocked_Surf,
        .moveID = MOVE_SURF,
        .partyMsgID = PARTY_MSG_CANT_SURF_HERE,
    },

    [FIELD_MOVE_FLY] =
    {
        .fieldMoveFunc = SetUpFieldMove_Fly,
        .isUnlockedFunc = IsFieldMoveUnlocked_Fly,
        .moveID = MOVE_FLY,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },

    [FIELD_MOVE_DIVE] =
    {
        .fieldMoveFunc = SetUpFieldMove_Dive,
        .isUnlockedFunc = IsFieldMoveUnlocked_Dive,
        .moveID = MOVE_DIVE,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },

    [FIELD_MOVE_WATERFALL] =
    {
        .fieldMoveFunc = SetUpFieldMove_Waterfall,
        .isUnlockedFunc = IsFieldMoveUnlocked_Waterfall,
        .moveID = MOVE_WATERFALL,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },

#if IS_HNS
    [FIELD_MOVE_WHIRLPOOL] =
    {
        .fieldMoveFunc = NULL,
        .isUnlockedFunc = IsFieldMoveUnlocked_Whirlpool,
        .moveID = MOVE_WHIRLPOOL,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },
#endif

    [FIELD_MOVE_TELEPORT] =
    {
        .fieldMoveFunc = SetUpFieldMove_Teleport,
        .isUnlockedFunc = IsFieldMoveUnlocked_Teleport,
        .moveID = MOVE_TELEPORT,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },

    [FIELD_MOVE_DIG] =
    {
        .fieldMoveFunc = SetUpFieldMove_Dig,
        .isUnlockedFunc = IsFieldMoveUnlocked_Dig,
        .moveID = MOVE_DIG,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },

    [FIELD_MOVE_SECRET_POWER] =
    {
        .fieldMoveFunc = SetUpFieldMove_SecretPower,
        .isUnlockedFunc = IsFieldMoveUnlocked_SecretPower,
        .moveID = MOVE_SECRET_POWER,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },

    [FIELD_MOVE_MILK_DRINK] =
    {
        .fieldMoveFunc = SetUpFieldMove_SoftBoiled,
        .isUnlockedFunc = IsFieldMoveUnlocked_MilkDrink,
        .moveID = MOVE_MILK_DRINK,
        .partyMsgID = PARTY_MSG_NOT_ENOUGH_HP,
    },

    [FIELD_MOVE_SOFT_BOILED] =
    {
        .fieldMoveFunc = SetUpFieldMove_SoftBoiled,
        .isUnlockedFunc = IsFieldMoveUnlocked_SoftBoiled,
        .moveID = MOVE_SOFT_BOILED,
        .partyMsgID = PARTY_MSG_NOT_ENOUGH_HP,
    },

    [FIELD_MOVE_SWEET_SCENT] =
    {
        .fieldMoveFunc = SetUpFieldMove_SweetScent,
        .isUnlockedFunc = IsFieldMoveUnlocked_SweetScent,
        .moveID = MOVE_SWEET_SCENT,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },
#if OW_ROCK_CLIMB_FIELD_MOVE == TRUE
    [FIELD_MOVE_ROCK_CLIMB] =
    {
        .fieldMoveFunc = SetUpFieldMove_RockClimb,
        .isUnlockedFunc = IsFieldMoveUnlocked_RockClimb,
        .moveID = MOVE_ROCK_CLIMB,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },
#endif
#if OW_DEFOG_FIELD_MOVE == TRUE
    [FIELD_MOVE_DEFOG] =
    {
        .fieldMoveFunc = SetUpFieldMove_Defog,
        .isUnlockedFunc = IsFieldMoveUnlocked_Defog,
        .moveID = MOVE_DEFOG,
        .partyMsgID = PARTY_MSG_CANT_USE_HERE,
    },
#endif
};

#include "global.h"
#include "event_data.h"
#include "field_message_box.h"
#include "pokedex.h"
#include "string_util.h"
#include "strings.h"

bool16 ScriptGetPokedexInfo(void)
{
    if (gSpecialVar_0x8004 == 0) // is national dex not present?
    {
        gSpecialVar_0x8005 = GetRegionalPokedexCount(FLAG_GET_SEEN);
        gSpecialVar_0x8006 = GetRegionalPokedexCount(FLAG_GET_CAUGHT);
    }
    else
    {
        gSpecialVar_0x8005 = Dex_GetNationalVisibleProgress(FLAG_GET_SEEN);
        gSpecialVar_0x8006 = Dex_GetNationalVisibleProgress(FLAG_GET_CAUGHT);
    }

    return IsNationalPokedexEnabled();
}

#define BIRCH_DEX_STRINGS 21

static const u8 *const sBirchDexRatingTexts[BIRCH_DEX_STRINGS] =
{
    gBirchDexRatingText_LessThan10,
    gBirchDexRatingText_LessThan20,
    gBirchDexRatingText_LessThan30,
    gBirchDexRatingText_LessThan40,
    gBirchDexRatingText_LessThan50,
    gBirchDexRatingText_LessThan60,
    gBirchDexRatingText_LessThan70,
    gBirchDexRatingText_LessThan80,
    gBirchDexRatingText_LessThan90,
    gBirchDexRatingText_LessThan100,
    gBirchDexRatingText_LessThan110,
    gBirchDexRatingText_LessThan120,
    gBirchDexRatingText_LessThan130,
    gBirchDexRatingText_LessThan140,
    gBirchDexRatingText_LessThan150,
    gBirchDexRatingText_LessThan160,
    gBirchDexRatingText_LessThan170,
    gBirchDexRatingText_LessThan180,
    gBirchDexRatingText_LessThan190,
    gBirchDexRatingText_LessThan200,
    gBirchDexRatingText_DexCompleted,
};

struct DexRatingBand
{
    u16 below;
    const u8 *text;
};

// Picks the band for count, with each band's bound scaled by scale / base. count is below the
// list's required count here: the caller has already handled a complete list.
static const u8 *GetRatingBandText(const struct DexRatingBand *bands, u32 bandCount, u32 count, u32 scale, u32 base, const u8 *topText)
{
    u32 i;

    for (i = 0; i < bandCount; i++)
    {
        if (count * base < bands[i].below * scale)
            return bands[i].text;
    }
    return topText;
}

// Returns how many home-list species completion needs, and takes the caught species it does not
// need out of *count. Returns 0 for a missing or broken list.
static u32 GetRequiredHomeDexCount(u32 *count)
{
    u32 i, id, species;
    u32 entryCount = Dex_GetActiveRegionalEntryCount();
    u32 maxDex = entryCount;

    for (i = 1; i <= entryCount; i++)
    {
        id = Dex_RegionalEntryToNational(i);
        if (!Dex_IsValidNationalId(id))
            return 0;
        species = NationalPokedexNumToSpecies(id);
        if (gSpeciesInfo[species].dexNotRequired || (gSpeciesInfo[species].isMythical && !gSpeciesInfo[species].dexForceRequired))
        {
            if (GetSetPokedexFlag(id, FLAG_GET_CAUGHT) && *count != 0)
                (*count)--;
            maxDex--;
        }
    }
    return maxDex;
}

#if IS_HNS

// The home bands stretch only for a list needing more than HOME_DEX_RATING_BASE species, so the
// last bands stay reachable, and never shrink, so "You've reached 200!" is never said early. The
// National bands, which name no counts, are stretched to the species the National list needs.
#define HOME_DEX_RATING_BASE 280
#define NATIONAL_DEX_RATING_BASE 493

static const struct DexRatingBand sHomeDexRatingBands[] =
{
    {10, gJohtoDexRatingText_LessThan10},
    {20, gJohtoDexRatingText_LessThan20},
    {35, gJohtoDexRatingText_LessThan35},
    {50, gJohtoDexRatingText_LessThan50},
    {65, gJohtoDexRatingText_LessThan65},
    {80, gJohtoDexRatingText_LessThan80},
    {95, gJohtoDexRatingText_LessThan95},
    {110, gJohtoDexRatingText_LessThan110},
    {125, gJohtoDexRatingText_LessThan125},
    {140, gJohtoDexRatingText_LessThan140},
    {155, gJohtoDexRatingText_LessThan155},
    {170, gJohtoDexRatingText_LessThan170},
    {185, gJohtoDexRatingText_LessThan185},
    {200, gJohtoDexRatingText_LessThan200},
    {215, gJohtoDexRatingText_LessThan215},
    {230, gJohtoDexRatingText_LessThan230},
    {245, gJohtoDexRatingText_LessThan245},
    {260, gJohtoDexRatingText_LessThan260},
    {275, gJohtoDexRatingText_LessThan275},
};

static const struct DexRatingBand sNationalDexRatingBands[] =
{
    {100, gNationalDexRatingText_LessThan100},
    {150, gNationalDexRatingText_LessThan150},
    {200, gNationalDexRatingText_LessThan200},
    {250, gNationalDexRatingText_LessThan250},
    {300, gNationalDexRatingText_LessThan300},
    {350, gNationalDexRatingText_LessThan350},
    {400, gNationalDexRatingText_LessThan400},
    {435, gNationalDexRatingText_LessThan435},
    {465, gNationalDexRatingText_LessThan465},
    {475, gNationalDexRatingText_LessThan475},
};

const u8 *GetPokedexRatingText(u32 count)
{
    u32 maxDex = GetRequiredHomeDexCount(&count);

    if (maxDex == 0)
    {
        gSpecialVar_Result = FALSE;
        return gJohtoDexRatingText_LessThan10;
    }

    if (count >= maxDex)
    {
        gSpecialVar_Result = TRUE;
        StringCopy(gStringVar3, Dex_GetActiveRegionName());
        return gJohtoDexRatingText_Complete;
    }

    gSpecialVar_Result = FALSE;
    return GetRatingBandText(sHomeDexRatingBands, ARRAY_COUNT(sHomeDexRatingBands), count,
                             max(maxDex, HOME_DEX_RATING_BASE), HOME_DEX_RATING_BASE, gJohtoDexRatingText_LessThanMaxDex);
}

const u8 *GetNationalPokedexRatingText(u32 count)
{
    enum NationalDexOrder id;
    u32 k;
    u16 maxDex = Dex_GetNationalVisibleEntryCount();

    if (maxDex == 0)
    {
        gSpecialVar_Result = FALSE;
        return gNationalDexRatingText_LessThan100;
    }

    for (id = Dex_GetFirstVisibleNationalEntry(); id != NATIONAL_DEX_NONE; id = Dex_GetNextVisibleNationalEntry(id))
    {
        k = NationalPokedexNumToSpecies(id);
        if (gSpeciesInfo[k].dexNotRequired || (gSpeciesInfo[k].isMythical && !gSpeciesInfo[k].dexForceRequired))
        {
            if (GetSetPokedexFlag(id, FLAG_GET_CAUGHT) && count != 0)
                count--;
            maxDex--;
        }
    }

    if (count >= maxDex)
    {
        gSpecialVar_Result = TRUE;
        return gNationalDexRatingText_Complete;
    }

    gSpecialVar_Result = FALSE;
    return GetRatingBandText(sNationalDexRatingBands, ARRAY_COUNT(sNationalDexRatingBands), count,
                             maxDex, NATIONAL_DEX_RATING_BASE, gNationalDexRatingText_LessThanMaxDex);
}

#else

// This shows your Hoenn Pokédex rating and not your National Dex.
const u8 *GetPokedexRatingText(u32 count)
{
    u32 i, j, k;
    u16 maxDex = Dex_GetActiveRegionalEntryCount() - 1;
    // doesNotCountForRegionalPokedex
    for (i = 0; i < Dex_GetActiveRegionalEntryCount(); i++)
    {
        j = Dex_RegionalEntryToNational(i + 1);
        k = NationalPokedexNumToSpecies(j);
        if (gSpeciesInfo[k].dexNotRequired || (gSpeciesInfo[k].isMythical && !gSpeciesInfo[k].dexForceRequired))
        {
            if (GetSetPokedexFlag(j, FLAG_GET_CAUGHT))
                count--;
            maxDex--;
        }
    }

    u32 idx = (count * (BIRCH_DEX_STRINGS - 1)) / maxDex;

    gSpecialVar_Result = idx == (BIRCH_DEX_STRINGS - 1);

    return sBirchDexRatingTexts[idx];
}

#endif // IS_HNS

void ShowPokedexRatingMessage(void)
{
    ShowFieldMessage(GetPokedexRatingText(gSpecialVar_0x8004));
}

// FRLG
extern const u8 PokedexRating_Text_LessThan10[];
extern const u8 PokedexRating_Text_LessThan20[];
extern const u8 PokedexRating_Text_LessThan30[];
extern const u8 PokedexRating_Text_LessThan40[];
extern const u8 PokedexRating_Text_LessThan50[];
extern const u8 PokedexRating_Text_LessThan60[];
extern const u8 PokedexRating_Text_LessThan70[];
extern const u8 PokedexRating_Text_LessThan80[];
extern const u8 PokedexRating_Text_LessThan90[];
extern const u8 PokedexRating_Text_LessThan100[];
extern const u8 PokedexRating_Text_LessThan110[];
extern const u8 PokedexRating_Text_LessThan120[];
extern const u8 PokedexRating_Text_LessThan130[];
extern const u8 PokedexRating_Text_LessThan140[];
extern const u8 PokedexRating_Text_LessThan150[];
extern const u8 PokedexRating_Text_Complete[];

u16 GetFrlgPokedexCount(void)
{
    if (gSpecialVar_0x8004 == 0)
    {
        gSpecialVar_0x8005 = GetKantoPokedexCount(FLAG_GET_SEEN);
        gSpecialVar_0x8006 = GetKantoPokedexCount(FLAG_GET_CAUGHT);
    }
    else
    {
        gSpecialVar_0x8005 = Dex_GetNationalVisibleProgress(FLAG_GET_SEEN);
        gSpecialVar_0x8006 = Dex_GetNationalVisibleProgress(FLAG_GET_CAUGHT);
    }
    return IsNationalPokedexEnabled();
}

// Oak's bands name counts and match his aides' gifts, so they stay fixed; only "complete" follows
// the home list.

static const struct DexRatingBand sOakDexRatingBands[] =
{
    {10, PokedexRating_Text_LessThan10},
    {20, PokedexRating_Text_LessThan20},
    {30, PokedexRating_Text_LessThan30},
    {40, PokedexRating_Text_LessThan40},
    {50, PokedexRating_Text_LessThan50},
    {60, PokedexRating_Text_LessThan60},
    {70, PokedexRating_Text_LessThan70},
    {80, PokedexRating_Text_LessThan80},
    {90, PokedexRating_Text_LessThan90},
    {100, PokedexRating_Text_LessThan100},
    {110, PokedexRating_Text_LessThan110},
    {120, PokedexRating_Text_LessThan120},
    {130, PokedexRating_Text_LessThan130},
    {140, PokedexRating_Text_LessThan140},
};

static const u8 *GetProfOaksRatingMessageByCount(u32 count)
{
    u32 maxDex = GetRequiredHomeDexCount(&count);

    gSpecialVar_Result = FALSE;
    if (maxDex == 0)
        return PokedexRating_Text_LessThan10;
    if (count >= maxDex)
    {
        gSpecialVar_Result = TRUE;
        return PokedexRating_Text_Complete;
    }
    return GetRatingBandText(sOakDexRatingBands, ARRAY_COUNT(sOakDexRatingBands), count, 1, 1,
                             PokedexRating_Text_LessThan150);
}

void GetProfOaksRatingMessage(void)
{
    ShowFieldMessage(GetProfOaksRatingMessageByCount(gSpecialVar_0x8004));
}

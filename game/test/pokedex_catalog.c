#include "global.h"
#include "event_data.h"
#include "malloc.h"
#include "pokedex.h"
#include "pokemon.h"
#include "string_util.h"
#include "strings.h"
#include "trade.h"
#include "test/test.h"
#include "constants/pokedex.h"

#if IS_WAYFARER

#define KANTO_JOHTO_LIST_COUNT 279
#define HOENN_LIST_COUNT       302

extern const u16 gPokedexOrder_Alphabetical[NATIONAL_DEX_COUNT];
extern const u16 gPokedexOrder_Weight[NATIONAL_DEX_COUNT];
extern const u16 gPokedexOrder_Height[NATIONAL_DEX_COUNT];

static void ExpectEveryNationalEntryExactlyOnce(const u16 *order)
{
    u16 id;
    u16 i;

    for (id = 1; id <= NATIONAL_DEX_COUNT; id++)
    {
        u16 occurrences = 0;

        for (i = 0; i < NATIONAL_DEX_COUNT; i++)
        {
            if (order[i] == id)
                occurrences++;
        }
        EXPECT_EQ(occurrences, 1);
    }
}

static u8 GetNationalGeneration(enum NationalDexOrder id)
{
    if (id <= NATIONAL_DEX_CELEBI)
        return 1 + (id > NATIONAL_DEX_MEW);
    if (id <= NATIONAL_DEX_DEOXYS)
        return 3;
    if (id <= NATIONAL_DEX_ARCEUS)
        return 4;
    if (id <= NATIONAL_DEX_GENESECT)
        return 5;
    return 6; // Any later generation; none is a home generation.
}

static bool8 IsHomeGeneration(enum DexRegionId region, u8 generation)
{
    if (region == DEX_REGION_HOENN)
        return generation == 3 || generation == 5;
    return generation == 1 || generation == 2;
}

static bool8 IsSinjohOnlyEvolution(enum NationalDexOrder id)
{
    // Hisuian evolutions of standard forms: their evolution items and move
    // exist only in Sinjoh, so a home list leaves them out (National only).
    return id == NATIONAL_DEX_KLEAVOR || id == NATIONAL_DEX_WYRDEER || id == NATIONAL_DEX_URSALUNA;
}

static u16 FindFamilyRoot(u16 *parent, u16 id)
{
    while (parent[id] != id)
        id = parent[id] = parent[parent[id]];
    return id;
}

// Re-derives a home list from the species data: a base species is a member
// when its family, traced through the standard forms' evolutions, has a member
// of one of the list's home generations.
static void BuildExpectedMembership(enum DexRegionId region, bool8 *isMember)
{
    u16 *parent = AllocZeroed((NATIONAL_DEX_PECHARUNT + 1) * sizeof(u16));
    bool8 *familyIsHome = AllocZeroed(NATIONAL_DEX_PECHARUNT + 1);
    u16 id;

    for (id = 0; id <= NATIONAL_DEX_PECHARUNT; id++)
        parent[id] = id;
    for (id = 1; id <= NATIONAL_DEX_PECHARUNT; id++)
    {
        u16 species = NationalPokedexNumToSpecies(id);
        const struct Evolution *evolutions;
        u32 i;

        if (!IsSpeciesEnabled(species) || IsSinjohOnlyEvolution(id))
            continue;
        evolutions = GetSpeciesEvolutions(species);
        for (i = 0; evolutions != NULL && evolutions[i].method != EVOLUTIONS_END; i++)
        {
            u16 target = evolutions[i].targetSpecies;
            enum NationalDexOrder targetId = gSpeciesInfo[target].natDexNum;

            if (!IsSpeciesEnabled(target) || targetId == NATIONAL_DEX_NONE || targetId > NATIONAL_DEX_PECHARUNT || IsSinjohOnlyEvolution(targetId))
                continue;
            parent[FindFamilyRoot(parent, id)] = FindFamilyRoot(parent, targetId);
        }
    }
    for (id = 1; id <= NATIONAL_DEX_PECHARUNT; id++)
        if (IsSpeciesEnabled(NationalPokedexNumToSpecies(id)) && IsHomeGeneration(region, GetNationalGeneration(id)))
            familyIsHome[FindFamilyRoot(parent, id)] = TRUE;
    for (id = 1; id <= NATIONAL_DEX_COUNT; id++)
        isMember[id] = id <= NATIONAL_DEX_PECHARUNT && IsSpeciesEnabled(NationalPokedexNumToSpecies(id)) && familyIsHome[FindFamilyRoot(parent, id)];
    Free(familyIsHome);
    Free(parent);
}

static void ExpectHomeListFollowsRule(enum DexRegionId region, u16 expectedCount)
{
    bool8 *expected = AllocZeroed(NATIONAL_DEX_COUNT + 1);
    bool8 *listed = AllocZeroed(NATIONAL_DEX_COUNT + 1);
    enum NationalDexOrder previous = NATIONAL_DEX_NONE;
    bool8 inOtherGenerations = FALSE;
    u16 localEntry;
    u16 id;

    BuildExpectedMembership(region, expected);
    EXPECT(Dex_SetActiveRegion(region));
    EXPECT_EQ(Dex_GetActiveRegionalEntryCount(), expectedCount);
    for (localEntry = 1; localEntry <= expectedCount; localEntry++)
    {
        enum NationalDexOrder entry = Dex_RegionEntryToNational(region, localEntry);
        bool8 isHome;

        EXPECT(Dex_IsValidNationalId(entry));
        EXPECT_LE(entry, NATIONAL_DEX_PECHARUNT);
        EXPECT(IsSpeciesEnabled(NationalPokedexNumToSpecies(entry)));
        EXPECT(!listed[entry]);
        listed[entry] = TRUE;
        EXPECT_EQ(Dex_RegionalEntryToNational(localEntry), entry);
        EXPECT_EQ(Dex_NationalToRegionalEntry(entry), localEntry);

        // Home generations first in National order, then the rest.
        isHome = IsHomeGeneration(region, GetNationalGeneration(entry));
        if (!isHome && !inOtherGenerations)
        {
            inOtherGenerations = TRUE;
            previous = NATIONAL_DEX_NONE;
        }
        EXPECT(isHome != inOtherGenerations);
        EXPECT_GT(entry, previous);
        previous = entry;
    }
    EXPECT_EQ(Dex_RegionEntryToNational(region, expectedCount + 1), NATIONAL_DEX_NONE);
    EXPECT_EQ(Dex_RegionalEntryToNational(expectedCount + 1), NATIONAL_DEX_NONE);
    for (id = 1; id <= NATIONAL_DEX_COUNT; id++)
        EXPECT_EQ(listed[id], expected[id]);
    Free(listed);
    Free(expected);
}

static u16 CountEnabledNationalEntries(void)
{
    u16 id;
    u16 count = 0;

    for (id = 1; id <= NATIONAL_DEX_COUNT; id++)
        if (IsSpeciesEnabled(NationalPokedexNumToSpecies(id)))
            count++;
    return count;
}

static bool8 IsRequired(enum NationalDexOrder id)
{
    u16 species = NationalPokedexNumToSpecies(id);
    return !(gSpeciesInfo[species].dexNotRequired || (gSpeciesInfo[species].isMythical && !gSpeciesInfo[species].dexForceRequired));
}

static void CatchActiveHomeList(void)
{
    u16 localEntry;

    for (localEntry = 1; localEntry <= Dex_GetActiveRegionalEntryCount(); localEntry++)
    {
        GetSetPokedexFlag(Dex_RegionalEntryToNational(localEntry), FLAG_SET_SEEN);
        GetSetPokedexFlag(Dex_RegionalEntryToNational(localEntry), FLAG_SET_CAUGHT);
    }
}

static void ClearCaught(enum NationalDexOrder id)
{
    gSaveBlock1Ptr->dexCaught[(id - 1) / 8] &= ~(1 << ((id - 1) % 8));
}

TEST("Pokédex sort orders cover each canonical National entry exactly once")
{
    ExpectEveryNationalEntryExactlyOnce(gPokedexOrder_Alphabetical);
    ExpectEveryNationalEntryExactlyOnce(gPokedexOrder_Weight);
    ExpectEveryNationalEntryExactlyOnce(gPokedexOrder_Height);
}

TEST("Pokédex home lists hold exactly the families of their home generations")
{
    ResetPokedex();
    ExpectHomeListFollowsRule(DEX_REGION_JOHTO, KANTO_JOHTO_LIST_COUNT);
    ExpectHomeListFollowsRule(DEX_REGION_HOENN, HOENN_LIST_COUNT);
    ResetPokedex();
}

TEST("Pokédex home lists open with their first home species and keep later evolutions")
{
    EXPECT_EQ(Dex_RegionEntryToNational(DEX_REGION_JOHTO, 1), NATIONAL_DEX_BULBASAUR);
    EXPECT_EQ(Dex_RegionEntryToNational(DEX_REGION_HOENN, 1), NATIONAL_DEX_TREECKO);
    EXPECT_EQ(Dex_RegionEntryToNational(DEX_REGION_HOENN, NATIONAL_DEX_DEOXYS - NATIONAL_DEX_CELEBI + 1), NATIONAL_DEX_VICTINI);
    EXPECT_EQ(HoennToNationalOrder(HOENN_DEX_DEOXYS + 1), NATIONAL_DEX_VICTINI);
    EXPECT_NE(SpeciesToHoennPokedexNum(SPECIES_KINGAMBIT), 0);
    EXPECT_NE(SpeciesToHoennPokedexNum(SPECIES_BUDEW), 0);
    EXPECT_NE(SpeciesToHoennPokedexNum(SPECIES_MARILL), 0);
    EXPECT_NE(SpeciesToKantoPokedexNum(SPECIES_AZURILL), 0);
    EXPECT_NE(SpeciesToKantoPokedexNum(SPECIES_MAGNEZONE), 0);
    EXPECT_NE(SpeciesToKantoPokedexNum(SPECIES_MUNCHLAX), 0);
    EXPECT_NE(SpeciesToKantoPokedexNum(SPECIES_SYLVEON), 0);
    EXPECT_EQ(SpeciesToHoennPokedexNum(SPECIES_OBSTAGOON), 0);
    EXPECT_EQ(SpeciesToKantoPokedexNum(SPECIES_PERRSERKER), 0);
    EXPECT_EQ(SpeciesToKantoPokedexNum(SPECIES_KLEAVOR), 0);
    EXPECT_EQ(SpeciesToKantoPokedexNum(SPECIES_WYRDEER), 0);
    EXPECT_EQ(SpeciesToKantoPokedexNum(SPECIES_URSALUNA), 0);
    EXPECT_EQ(SpeciesToKantoPokedexNum(SPECIES_MEOWTH_GALAR), 0);
    EXPECT_EQ(SpeciesToHoennPokedexNum(SPECIES_FLETCHLING), 0);
    EXPECT_EQ(SpeciesToKantoPokedexNum(SPECIES_FLETCHLING), 0);
}

TEST("Pokédex Kanto and Johto region values resolve to the Kanto-Johto home list")
{
    u16 localEntry;

    for (localEntry = 0; localEntry <= KANTO_JOHTO_LIST_COUNT + 1; localEntry++)
    {
        EXPECT_EQ(Dex_RegionEntryToNational(DEX_REGION_KANTO, localEntry), Dex_RegionEntryToNational(DEX_REGION_JOHTO, localEntry));
        EXPECT_EQ(KantoToNationalOrder(localEntry), JohtoToNationalOrder(localEntry));
    }
    EXPECT_EQ(SpeciesToKantoPokedexNum(SPECIES_TREECKO), 0);
    EXPECT_EQ(SpeciesToJohtoPokedexNum(SPECIES_CHIKORITA), SpeciesToKantoPokedexNum(SPECIES_CHIKORITA));

    ResetPokedex();
    EXPECT(Dex_SetActiveRegion(DEX_REGION_KANTO));
    EXPECT_EQ(StringCompare(Dex_GetActiveRegionName(), gText_KantoJohto), 0);
    EXPECT_EQ(StringCompare(Dex_GetActiveRegionTitle(), gText_DexKantoJohtoTitle), 0);
    EXPECT_EQ(StringCompare(Dex_GetActiveRegionDescription(), gText_DexKantoJohtoDescription), 0);
    EXPECT(Dex_SetActiveRegion(DEX_REGION_JOHTO));
    EXPECT_EQ(StringCompare(Dex_GetActiveRegionName(), gText_KantoJohto), 0);
    EXPECT_EQ(StringCompare(Dex_GetActiveRegionTitle(), gText_DexKantoJohtoTitle), 0);
    EXPECT_EQ(StringCompare(Dex_GetActiveRegionDescription(), gText_DexKantoJohtoDescription), 0);
    EXPECT(Dex_SetActiveRegion(DEX_REGION_HOENN));
    EXPECT_EQ(Dex_GetActiveRegionalEntryCount(), HOENN_LIST_COUNT);
    EXPECT_EQ(StringCompare(Dex_GetActiveRegionName(), gText_Hoenn), 0);
    EXPECT_EQ(StringCompare(Dex_GetActiveRegionTitle(), gText_DexHoennTitle), 0);
    ResetPokedex();
}

TEST("Pokédex regional catalog facade rejects invalid active state and local entries")
{
    ResetPokedex();

    EXPECT_EQ(Dex_GetActiveRegion(), DEX_REGION_JOHTO);
    EXPECT_EQ(Dex_GetActiveRegionalEntryCount(), KANTO_JOHTO_LIST_COUNT);
    EXPECT_EQ(Dex_RegionalEntryToNational(0), NATIONAL_DEX_NONE);
    EXPECT_EQ(Dex_RegionalEntryToNational(KANTO_JOHTO_LIST_COUNT + 1), NATIONAL_DEX_NONE);
    EXPECT(!Dex_SetActiveRegion(DEX_REGION_NONE));
    EXPECT(!Dex_SetActiveRegion(DEX_REGION_COUNT));

    gSaveBlock2Ptr->pokedex.activeRegion = DEX_REGION_NONE;
    EXPECT_EQ(Dex_GetActiveRegionalEntryCount(), 0);
    EXPECT_EQ(Dex_RegionalEntryToNational(1), NATIONAL_DEX_NONE);
    EXPECT_EQ(Dex_GetFirstVisibleNationalEntry(), NATIONAL_DEX_NONE);
    EXPECT_EQ(Dex_GetNationalVisibleEntryCount(), 0);

    ResetPokedex();
}

TEST("Pokédex flag access ignores invalid canonical National IDs")
{
    u8 seenBefore[NUM_DEX_FLAG_BYTES];
    u8 caughtBefore[NUM_DEX_FLAG_BYTES];

    ResetPokedex();
    memcpy(seenBefore, gSaveBlock1Ptr->dexSeen, sizeof(seenBefore));
    memcpy(caughtBefore, gSaveBlock1Ptr->dexCaught, sizeof(caughtBefore));

    EXPECT_EQ(GetSetPokedexFlag(NATIONAL_DEX_NONE, FLAG_GET_SEEN), 0);
    EXPECT_EQ(GetSetPokedexFlag(NATIONAL_DEX_COUNT + 1, FLAG_GET_CAUGHT), 0);
    EXPECT_EQ(GetSetPokedexFlag(NATIONAL_DEX_NONE, FLAG_SET_SEEN), 0);
    EXPECT_EQ(GetSetPokedexFlag(NATIONAL_DEX_COUNT + 1, FLAG_SET_CAUGHT), 0);
    EXPECT_EQ(memcmp(seenBefore, gSaveBlock1Ptr->dexSeen, sizeof(seenBefore)), 0);
    EXPECT_EQ(memcmp(caughtBefore, gSaveBlock1Ptr->dexCaught, sizeof(caughtBefore)), 0);
}

TEST("Pokédex regional mode shows only the origin's home list")
{
    ResetPokedex();
    EXPECT(Dex_SetActiveRegion(DEX_REGION_HOENN));
    EXPECT(Dex_IsRegionalEntryVisible(NATIONAL_DEX_PATRAT));
    EXPECT(!Dex_IsRegionalEntryVisible(NATIONAL_DEX_CHIKORITA));
    EXPECT(!Dex_IsRegionalEntryVisible(NATIONAL_DEX_FLETCHLING));
    EXPECT(Dex_SetActiveRegion(DEX_REGION_JOHTO));
    EXPECT(!Dex_IsRegionalEntryVisible(NATIONAL_DEX_PATRAT));
    EXPECT(Dex_IsRegionalEntryVisible(NATIONAL_DEX_CHIKORITA));
    EXPECT(!Dex_IsRegionalEntryVisible(NATIONAL_DEX_FLETCHLING));
    EXPECT_EQ(SpeciesToPokedexNum(SPECIES_PATRAT), 0xFFFF);
    EXPECT_EQ(SpeciesToPokedexNum(SPECIES_BULBASAUR), 1);
    ResetPokedex();
}

TEST("Pokédex National mode shows every enabled species once upgraded")
{
    enum NationalDexOrder id;
    enum NationalDexOrder previous = NATIONAL_DEX_NONE;
    u16 count = 0;

    ResetPokedex();
    GetSetPokedexFlag(NATIONAL_DEX_FLETCHLING, FLAG_SET_SEEN);
    GetSetPokedexFlag(NATIONAL_DEX_FLETCHLING, FLAG_SET_CAUGHT);
    EXPECT_EQ(Dex_GetRegionalVisibleProgress(FLAG_GET_CAUGHT), 0);
    EXPECT(Dex_UpgradeToNational());

    for (id = 1; id <= NATIONAL_DEX_COUNT; id++)
        EXPECT_EQ(Dex_IsNationalEntryVisible(id), IsSpeciesEnabled(NationalPokedexNumToSpecies(id)));
    for (id = Dex_GetFirstVisibleNationalEntry(); id != NATIONAL_DEX_NONE; id = Dex_GetNextVisibleNationalEntry(id))
    {
        EXPECT_GT(id, previous);
        previous = id;
        count++;
    }
    EXPECT_EQ(count, CountEnabledNationalEntries());
    EXPECT_EQ(Dex_GetNationalVisibleEntryCount(), CountEnabledNationalEntries());
    EXPECT_GT(CountEnabledNationalEntries(), NATIONAL_DEX_GENESECT);
    EXPECT(Dex_IsNationalEntryVisible(NATIONAL_DEX_MEOWTH_GALAR));
    EXPECT(Dex_IsNationalEntryVisible(NATIONAL_DEX_KLEAVOR));
    EXPECT_EQ(Dex_GetNationalVisibleProgress(FLAG_GET_CAUGHT), 1);
    EXPECT_EQ(SpeciesToPokedexNum(SPECIES_FLETCHLING), NATIONAL_DEX_FLETCHLING);
    EXPECT_EQ(SpeciesToPokedexNum(SPECIES_PATRAT), NATIONAL_DEX_PATRAT);
    ResetPokedex();
}

TEST("Pokédex region grants are no-ops and old mask bits change no count")
{
    enum DexRegionId region;
    u16 regionalSeen, nationalSeen, nationalCount;

    ResetPokedex();
    EXPECT(Dex_SetActiveRegion(DEX_REGION_HOENN));
    CatchActiveHomeList();
    GetSetPokedexFlag(NATIONAL_DEX_CHIKORITA, FLAG_SET_SEEN);
    regionalSeen = Dex_GetRegionalVisibleProgress(FLAG_GET_SEEN);
    EXPECT(Dex_UpgradeToNational());
    nationalSeen = Dex_GetNationalVisibleProgress(FLAG_GET_SEEN);
    nationalCount = Dex_GetNationalVisibleEntryCount();
    EXPECT_EQ(nationalSeen, HOENN_LIST_COUNT + 1);

    for (region = DEX_REGION_KANTO; region < DEX_REGION_COUNT; region++)
    {
        EXPECT(Dex_GrantNationalExtension(region));
        EXPECT(Dex_HasNationalExtension(region));
    }
    EXPECT(!Dex_GrantNationalExtension(DEX_REGION_NONE));
    EXPECT(!Dex_GrantNationalExtension(DEX_REGION_COUNT));
    EXPECT_EQ(Dex_GetNationalExtensionMask(), 0);

    // An old save may hold mask bits from the retired grants.
    gSaveBlock2Ptr->pokedex.nationalExtensionMask = 0xE;
    EXPECT_EQ(Dex_GetRegionalVisibleProgress(FLAG_GET_SEEN), regionalSeen);
    EXPECT_EQ(Dex_GetNationalVisibleProgress(FLAG_GET_SEEN), nationalSeen);
    EXPECT_EQ(Dex_GetNationalVisibleEntryCount(), nationalCount);
    EXPECT_EQ(Dex_GetNationalExtensionMask(), 0xE);
    ResetPokedex();

    gSaveBlock2Ptr->pokedex.nationalExtensionMask = 0xE;
    EXPECT(!Dex_HasNationalExtension(DEX_REGION_HOENN));
    EXPECT(!Dex_IsRegionalEntryVisible(NATIONAL_DEX_TREECKO));
    ResetPokedex();
}

TEST("Pokédex region counts read the home lists")
{
    ResetPokedex();
    EXPECT(Dex_SetActiveRegion(DEX_REGION_HOENN));
    CatchActiveHomeList();
    EXPECT_EQ(GetHoennPokedexCount(FLAG_GET_CAUGHT), HOENN_LIST_COUNT);
    EXPECT_EQ(GetHoennPokedexCount(FLAG_GET_SEEN), HOENN_LIST_COUNT);
    EXPECT_EQ(GetRegionalPokedexCount(FLAG_GET_CAUGHT), HOENN_LIST_COUNT);
    // Marill, Azumarill and Wobbuffet's families span both lands.
    EXPECT_EQ(GetKantoPokedexCount(FLAG_GET_CAUGHT), 5);
    EXPECT_EQ(GetJohtoPokedexCount(FLAG_GET_CAUGHT), 5);
    ResetPokedex();

    CatchActiveHomeList();
    EXPECT_EQ(GetKantoPokedexCount(FLAG_GET_CAUGHT), KANTO_JOHTO_LIST_COUNT);
    EXPECT_EQ(GetJohtoPokedexCount(FLAG_GET_SEEN), KANTO_JOHTO_LIST_COUNT);
    EXPECT_EQ(GetHoennPokedexCount(FLAG_GET_CAUGHT), 5);
    ResetPokedex();
}

TEST("Pokédex home-list completion needs every required home species")
{
    u16 localEntry;
    enum NationalDexOrder firstRequired = NATIONAL_DEX_NONE;

    ResetPokedex();
    EXPECT(Dex_SetActiveRegion(DEX_REGION_HOENN));
    EXPECT(!HasAllRegionalMons());
    CatchActiveHomeList();
    EXPECT(HasAllRegionalMons());
    EXPECT(HasAllHoennMons());
    EXPECT(Dex_HasCompletedRegionalVisibleEntries());
    EXPECT(!HasAllKantoMons());
    EXPECT(!HasAllJohotoMons());
    for (localEntry = 1; localEntry <= HOENN_LIST_COUNT && firstRequired == NATIONAL_DEX_NONE; localEntry++)
    {
        if (IsRequired(Dex_RegionalEntryToNational(localEntry)))
            firstRequired = Dex_RegionalEntryToNational(localEntry);
    }
    EXPECT_NE(firstRequired, NATIONAL_DEX_NONE);
    ClearCaught(firstRequired);
    EXPECT(!HasAllRegionalMons());
    EXPECT(!HasAllHoennMons());
    ResetPokedex();

    CatchActiveHomeList();
    EXPECT(HasAllRegionalMons());
    EXPECT(HasAllKantoMons());
    EXPECT(HasAllJohotoMons());
    EXPECT(!HasAllHoennMons());
    ResetPokedex();
}

TEST("Pokédex National completion needs every required enabled species")
{
    enum NationalDexOrder id;
    enum NationalDexOrder lastRequired = NATIONAL_DEX_NONE;

    ResetPokedex();
    EXPECT(Dex_UpgradeToNational());
    EXPECT(!HasAllMons());
    for (id = Dex_GetFirstVisibleNationalEntry(); id != NATIONAL_DEX_NONE; id = Dex_GetNextVisibleNationalEntry(id))
    {
        GetSetPokedexFlag(id, FLAG_SET_SEEN);
        GetSetPokedexFlag(id, FLAG_SET_CAUGHT);
        if (IsRequired(id))
            lastRequired = id;
    }
    EXPECT(HasAllMons());
    EXPECT(Dex_HasCompletedNationalVisibleEntries());
    EXPECT_EQ(Dex_GetNationalVisibleProgress(FLAG_GET_CAUGHT), CountEnabledNationalEntries());
    EXPECT_NE(lastRequired, NATIONAL_DEX_NONE);
    ClearCaught(lastRequired);
    EXPECT(!HasAllMons());
    ResetPokedex();
}

TEST("Pokédex critical-capture progress reads the home list before and after the upgrade")
{
    u16 caughtBefore, visibleBefore, caughtAfter, visibleAfter;

    ResetPokedex();
    EXPECT(Dex_SetActiveRegion(DEX_REGION_HOENN));
    GetSetPokedexFlag(NATIONAL_DEX_TREECKO, FLAG_SET_CAUGHT);
    GetSetPokedexFlag(NATIONAL_DEX_CHIKORITA, FLAG_SET_CAUGHT);
    GetSetPokedexFlag(NATIONAL_DEX_FLETCHLING, FLAG_SET_CAUGHT);
    EXPECT(Dex_GetCriticalCaptureProgress(&caughtBefore, &visibleBefore));
    EXPECT_EQ(caughtBefore, 1);
    EXPECT_EQ(visibleBefore, HOENN_LIST_COUNT);
    EXPECT(Dex_UpgradeToNational());
    EXPECT(Dex_GetCriticalCaptureProgress(&caughtAfter, &visibleAfter));
    EXPECT_EQ(caughtAfter, caughtBefore);
    EXPECT_EQ(visibleAfter, visibleBefore);

    gSaveBlock2Ptr->pokedex.activeRegion = DEX_REGION_NONE;
    EXPECT(!Dex_GetCriticalCaptureProgress(&caughtAfter, &visibleAfter));
    EXPECT_EQ(caughtAfter, 0);
    EXPECT_EQ(visibleAfter, 0);
    ResetPokedex();
}

TEST("Pokédex keeps a pre-home-list save's region, progress and layout")
{
    ResetPokedex();
    // A Kanto origin's save: Kanto region value, old grants, Johto caught.
    gSaveBlock2Ptr->pokedex.activeRegion = DEX_REGION_KANTO;
    gSaveBlock2Ptr->pokedex.nationalExtensionMask = 1u << DEX_REGION_JOHTO;
    GetSetPokedexFlag(NATIONAL_DEX_BULBASAUR, FLAG_SET_CAUGHT);
    GetSetPokedexFlag(NATIONAL_DEX_CHIKORITA, FLAG_SET_CAUGHT);
    GetSetPokedexFlag(NATIONAL_DEX_TREECKO, FLAG_SET_CAUGHT);
    EXPECT_EQ(Dex_GetActiveRegion(), DEX_REGION_KANTO);
    EXPECT_EQ(Dex_GetActiveRegionalEntryCount(), KANTO_JOHTO_LIST_COUNT);
    EXPECT_EQ(Dex_GetRegionalVisibleProgress(FLAG_GET_CAUGHT), 2);
    EXPECT(!Dex_HasNationalUpgrade());

    EXPECT_EQ(offsetof(struct Pokedex, activeRegion), 0x10);
    EXPECT_EQ(offsetof(struct Pokedex, nationalExtensionMask), 0x14);
    ResetPokedex();
}

TEST("Trade receipt records hidden species globally without exposing regional progress")
{
    enum NationalDexOrder hiddenId = NATIONAL_DEX_NONE;
    enum NationalDexOrder id;

    ResetPokedex();
    for (id = 1; id <= NATIONAL_DEX_COUNT; id++)
    {
        if (!Dex_IsRegionalEntryVisible(id) && IsSpeciesEnabled(NationalPokedexNumToSpecies(id)))
        {
            hiddenId = id;
            break;
        }
    }

    EXPECT_NE(hiddenId, NATIONAL_DEX_NONE);
    Test_UpdatePokedexForReceivedSpecies(NationalPokedexNumToSpecies(hiddenId), 0);
    EXPECT_EQ(GetSetPokedexFlag(hiddenId, FLAG_GET_SEEN), 1);
    EXPECT_EQ(GetSetPokedexFlag(hiddenId, FLAG_GET_CAUGHT), 1);
    EXPECT_EQ(Dex_GetRegionalVisibleProgress(FLAG_GET_SEEN), 0);
    EXPECT_EQ(Dex_GetRegionalVisibleProgress(FLAG_GET_CAUGHT), 0);

    EXPECT(Dex_UpgradeToNational());
    EXPECT_EQ(Dex_GetNationalVisibleProgress(FLAG_GET_SEEN), 1);
    EXPECT_EQ(Dex_GetNationalVisibleProgress(FLAG_GET_CAUGHT), 1);
    ResetPokedex();
}

TEST("Pokédex script specials validate regions without side effects")
{
    enum DexRegionId region;

    ResetPokedex();
    gSpecialVar_0x8004 = DEX_REGION_NONE;
    DexScript_GrantNationalExtension();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(Dex_GetNationalExtensionMask(), 0);

    gSpecialVar_0x8004 = DEX_REGION_COUNT;
    DexScript_SetActiveRegion();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(Dex_GetActiveRegion(), DEX_REGION_JOHTO);
    DexScript_HasNationalExtension();
    EXPECT_EQ(gSpecialVar_Result, FALSE);

    for (region = DEX_REGION_KANTO; region < DEX_REGION_COUNT; region++)
    {
        gSpecialVar_0x8004 = region;
        DexScript_SetActiveRegion();
        EXPECT_EQ(gSpecialVar_Result, TRUE);
        DexScript_GetActiveRegion();
        EXPECT_EQ(gSpecialVar_Result, region);
        DexScript_GrantNationalExtension();
        EXPECT_EQ(gSpecialVar_Result, TRUE);
        DexScript_HasNationalExtension();
        EXPECT_EQ(gSpecialVar_Result, FALSE);
    }
    EXPECT_EQ(Dex_GetNationalExtensionMask(), 0);
    DexScript_UpgradeToNational();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    DexScript_HasNationalUpgrade();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    gSpecialVar_0x8004 = DEX_REGION_HOENN;
    DexScript_HasNationalExtension();
    EXPECT_EQ(gSpecialVar_Result, TRUE);

    ResetPokedex();
}

TEST("Pokédex critical-capture threshold uses strict catalog-progress bands")
{
    u8 threshold;

    EXPECT(!Dex_CalculateCriticalCaptureThreshold(0, 0, 120, FALSE, &threshold));
    EXPECT(!Dex_CalculateCriticalCaptureThreshold(30, 650, 120, FALSE, &threshold));
    EXPECT(Dex_CalculateCriticalCaptureThreshold(31, 650, 120, FALSE, &threshold));
    EXPECT_EQ(threshold, 10);
    EXPECT(Dex_CalculateCriticalCaptureThreshold(151, 650, 120, FALSE, &threshold));
    EXPECT_EQ(threshold, 20);
    EXPECT(Dex_CalculateCriticalCaptureThreshold(301, 650, 120, FALSE, &threshold));
    EXPECT_EQ(threshold, 30);
    EXPECT(Dex_CalculateCriticalCaptureThreshold(451, 650, 120, FALSE, &threshold));
    EXPECT_EQ(threshold, 40);
    EXPECT(Dex_CalculateCriticalCaptureThreshold(601, 650, 120, FALSE, &threshold));
    EXPECT_EQ(threshold, 42);
}

#endif // IS_WAYFARER

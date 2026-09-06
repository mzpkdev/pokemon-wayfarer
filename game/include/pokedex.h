#ifndef GUARD_POKEDEX_H
#define GUARD_POKEDEX_H

extern u8 gUnusedPokedexU8;
extern void (*gPokedexVBlankCB)(void);

void ResetPokedex(void);
enum DexRegionId Dex_GetActiveRegion(void);
bool8 Dex_SetActiveRegion(enum DexRegionId region);
u16 Dex_GetActiveRegionalEntryCount(void);
enum NationalDexOrder Dex_RegionalEntryToNational(u16 localEntry);
u16 Dex_NationalToRegionalEntry(enum NationalDexOrder id);
bool8 Dex_IsRegionalEntryVisible(enum NationalDexOrder id);
u16 Dex_GetRegionalVisibleProgress(u8 flagCase);
bool8 Dex_HasCompletedRegionalVisibleEntries(void);
bool8 Dex_IsValidNationalId(enum NationalDexOrder id);
bool8 Dex_IsNationalEntryVisible(enum NationalDexOrder id);
u16 Dex_GetNationalVisibleEntryCount(void);
enum NationalDexOrder Dex_GetFirstVisibleNationalEntry(void);
enum NationalDexOrder Dex_GetNextVisibleNationalEntry(enum NationalDexOrder after);
u16 Dex_GetNationalVisibleProgress(u8 flagCase);
bool8 Dex_HasCompletedNationalVisibleEntries(void);
bool8 Dex_GetCriticalCaptureProgress(u16 *caughtCount, u16 *visibleCount);
bool8 Dex_GrantNationalExtension(enum DexRegionId region);
bool8 Dex_HasNationalExtension(enum DexRegionId region);
u32 Dex_GetNationalExtensionMask(void);
bool8 Dex_UpgradeToNational(void);
bool8 Dex_HasNationalUpgrade(void);
const u8 *Dex_GetActiveRegionName(void);
const u8 *Dex_GetActiveRegionDescription(void);
bool8 Dex_CalculateCriticalCaptureThreshold(u16 caughtCount, u16 visibleCount, u32 baseOdds, bool8 hasCatchingCharm, u8 *rollThreshold);
u32 GetRegionalPokedexCount(u8 caseID);
u16 GetHoennPokedexCount(u8 caseID);
u16 GetKantoPokedexCount(u8 caseID);
u16 GetJohtoPokedexCount(u8 caseID);
u8 DisplayCaughtMonDexPage(u16 species, bool32 isShiny, u32 personality);
u32 Pokedex_CreateCaughtMonSprite(u32 species, s32 x, s32 y);
s8 GetSetPokedexFlag(enum NationalDexOrder nationalDexNo, u8 caseID);
void DrawFootprint(u8 windowId, u16 species);
u16 CreateMonSpriteFromNationalDexNumber(enum NationalDexOrder nationalNum, s16 x, s16 y, u16 paletteSlot);
bool16 HasAllRegionalMons(void);
bool16 HasAllHoennMons(void);
bool16 HasAllKantoMons(void);
bool16 HasAllJohotoMons(void);
void ResetPokedexScrollPositions(void);
bool16 HasAllMons(void);
void CB2_OpenPokedex(void);
void PrintMonMeasurements(u16 species, u32 owned);
u8* ConvertMonHeightToString(u32 height);
u8* ConvertMonWeightToString(u32 weight);

#endif // GUARD_POKEDEX_H

#ifndef GUARD_STARTER_CHOOSE_H
#define GUARD_STARTER_CHOOSE_H

extern const u16 gBirchBagGrass_Pal[];
extern const u32 gBirchBagTilemap[];
extern const u32 gBirchGrassTilemap[];
extern const u32 gBirchBagGrass_Gfx[];
extern const u32 gPokeballSelection_Gfx[];

u16 GetStarterPokemon(u16 chosenStarterId);
#if IS_WAYFARER
u16 GetJohtoStarterPokemon(u16 chosenStarterId);
u16 WayfarerGetJohtoStarterSpecies(void);
#endif
void CB2_ChooseStarter(void);
#if TESTING
void Test_ResetStarterChooseCache(void);
#endif
#if E2E_TESTING
u8 E2ETest_GetStarterChooseStage(void);
#endif

#endif // GUARD_STARTER_CHOOSE_H

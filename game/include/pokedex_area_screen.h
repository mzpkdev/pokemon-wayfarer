#ifndef GUARD_POKEDEX_AREA_SCREEN_H
#define GUARD_POKEDEX_AREA_SCREEN_H

#include "rtc.h"

struct WildEncounterProfileView;

extern u8 gAreaTimeOfDay;

enum PokedexAreaScreenState
{
    DEX_SHOW_AREA_SCREEN,
    DEX_UPDATE_AREA_SCREEN
};

void DisplayPokedexAreaScreen(u16 species, u8 *screenSwitchState, enum TimeOfDay timeOfDay, enum PokedexAreaScreenState areaState);
void ShowPokedexAreaScreen(u16 species, u8 *screenSwitchState);

#if TESTING
bool8 PokedexArea_ProfileViewHasSpeciesForTesting(const struct WildEncounterProfileView *view, u16 species);
#endif

#endif // GUARD_POKEDEX_AREA_SCREEN_H

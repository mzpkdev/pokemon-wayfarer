#ifndef GUARD_WAYFARER_KANTO_OPENING_TOWN_H
#define GUARD_WAYFARER_KANTO_OPENING_TOWN_H

#include "global.h"

#if IS_WAYFARER
struct ScriptContext;

// Ambient FRLG town state for the Pallet origin (sign lady, Route 1 sample). The opening phase itself lives in wayfarer_kanto_opening.c.
// Map scripts reach these through callnative: getters write VAR_RESULT and
// setters read VAR_0x8004 where noted.
void WayfarerKantoTown_GetSignLadyState(struct ScriptContext *ctx);
void WayfarerKantoTown_SetSignLadyState(struct ScriptContext *ctx);
void WayfarerKantoTown_HasRoute1Potion(struct ScriptContext *ctx);
void WayfarerKantoTown_MarkRoute1Potion(struct ScriptContext *ctx);
#endif

#endif

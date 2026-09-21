#ifndef GUARD_WAYFARER_SINNOH_TEST_H
#define GUARD_WAYFARER_SINNOH_TEST_H

#include "global.h"

#if IS_WAYFARER && (TESTING || defined(E2E_TESTING))
bool8 EnterSinnohForTest(s16 mapGroup, s16 mapNum, s16 x, s16 y);
bool8 ReturnFromSinnohTest(void);
bool8 IsSinnohTestTraversalActive(void);
#endif

#endif // GUARD_WAYFARER_SINNOH_TEST_H

// Host build of src/wayfarer_ambience.c for ambience_test.c: forced in with
// -include ahead of the source. It stands in for global.h (its guard is
// taken here) with just the types and macros the module uses. Nothing here
// declares Random: a module that called it would not link.
#ifndef GUARD_WAYFARER_AMBIENCE_PRELUDE_H
#define GUARD_WAYFARER_AMBIENCE_PRELUDE_H

#include "constants/global.h"
#include "gba/types.h"
#include "gba/defines.h"

#define GUARD_GLOBAL_H

#endif

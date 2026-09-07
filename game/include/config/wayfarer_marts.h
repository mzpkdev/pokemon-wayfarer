#ifndef GUARD_CONFIG_WAYFARER_MARTS_H
#define GUARD_CONFIG_WAYFARER_MARTS_H

// Production default for the shared Trainer Rating mart catalog.  This is a
// compile-time rollback only; scripts additionally require IS_WAYFARER.
#ifndef WAYFARER_TR_MARTS_ENABLED
#define WAYFARER_TR_MARTS_ENABLED 1
#endif

#endif // GUARD_CONFIG_WAYFARER_MARTS_H

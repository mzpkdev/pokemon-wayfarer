#include "global.h"
#include "pokemon_storage_system.h"
const unsigned int save_offsets[] = {
 sizeof(struct SaveBlock1), sizeof(struct SaveBlock2), sizeof(struct SaveBlock3),
 __builtin_offsetof(struct SaveBlock1, bag), sizeof(struct Bag),
 __builtin_offsetof(struct SaveBlock1, unused_9C6),
 __builtin_offsetof(struct SaveBlock1, flags), sizeof(((struct SaveBlock1*)0)->flags),
 __builtin_offsetof(struct SaveBlock1, vars), sizeof(((struct SaveBlock1*)0)->vars),
 __builtin_offsetof(struct SaveBlock1, playerParty), sizeof(((struct SaveBlock1*)0)->playerParty),
 __builtin_offsetof(struct SaveBlock1, playerPartyCount),
 __builtin_offsetof(struct SaveBlock2, frontier), sizeof(((struct SaveBlock2*)0)->frontier),
 sizeof(struct PokemonStorage), __builtin_offsetof(struct SaveBlock2,encryptionKey),
 __builtin_offsetof(struct SaveBlock1,bag.medicine), BAG_MEDICINE_COUNT, ITEM_POTION
};

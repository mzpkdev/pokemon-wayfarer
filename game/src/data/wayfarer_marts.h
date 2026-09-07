// This file is included by src/wayfarer_marts.c after its private data types.
// Lists deliberately have no ITEM_NONE terminator: source counts are part of
// the resolver's validation contract.

static const struct WayfarerMartCommonItem sWayfarerMartCommonItems[] =
{
    // Balls
    {  0, MART_COMMON_BALLS,       ITEM_POKE_BALL },
    {  4, MART_COMMON_BALLS,       ITEM_GREAT_BALL },
    { 30, MART_COMMON_BALLS,       ITEM_ULTRA_BALL },
    // HP medicine
    {  0, MART_COMMON_HP_MEDICINE, ITEM_POTION },
    {  4, MART_COMMON_HP_MEDICINE, ITEM_SUPER_POTION },
    { 30, MART_COMMON_HP_MEDICINE, ITEM_HYPER_POTION },
    { 55, MART_COMMON_HP_MEDICINE, ITEM_MAX_POTION },
    { 55, MART_COMMON_HP_MEDICINE, ITEM_FULL_RESTORE },
    // Status medicine and revival
    {  0, MART_COMMON_STATUS_CURE, ITEM_ANTIDOTE },
    {  0, MART_COMMON_STATUS_CURE, ITEM_PARALYZE_HEAL },
    {  0, MART_COMMON_STATUS_CURE, ITEM_AWAKENING },
    {  0, MART_COMMON_STATUS_CURE, ITEM_BURN_HEAL },
    {  0, MART_COMMON_STATUS_CURE, ITEM_ICE_HEAL },
    { 40, MART_COMMON_STATUS_CURE, ITEM_FULL_HEAL },
    { 30, MART_COMMON_REVIVE,      ITEM_REVIVE },
    // Travel
    {  0, MART_COMMON_REPELS,      ITEM_REPEL },
    { 16, MART_COMMON_REPELS,      ITEM_SUPER_REPEL },
    { 40, MART_COMMON_REPELS,      ITEM_MAX_REPEL },
    {  0, MART_COMMON_ESCAPE_ROPE, ITEM_ESCAPE_ROPE },
};

static const struct WayfarerMartPpItem sWayfarerMartPpItems[] =
{
    {  0, ITEM_ETHER },
    { 16, ITEM_ELIXIR },
    { 30, ITEM_MAX_ETHER },
    { 55, ITEM_MAX_ELIXIR },
};

static const u16 sSignatureCherrygrove[] = { ITEM_HEAL_BALL, ITEM_POKE_DOLL };
static const u16 sSignatureViolet[] = { ITEM_NEST_BALL, ITEM_X_ACCURACY };
static const u16 sSignatureAzalea[] = { ITEM_NET_BALL, ITEM_NEST_BALL, ITEM_WOOD_MAIL };
static const u16 sSignatureGoldenrod2F[] = { ITEM_FIRE_STONE, ITEM_WATER_STONE, ITEM_THUNDER_STONE, ITEM_LUXURY_BALL };
static const u16 sSignatureEcruteak[] = { ITEM_DUSK_BALL, ITEM_POKE_DOLL, ITEM_RETRO_MAIL };
static const u16 sSignatureOlivine[] = { ITEM_NET_BALL, ITEM_HEAL_BALL, ITEM_HARBOR_MAIL };
static const u16 sSignatureBlackthorn[] = { ITEM_DUSK_BALL, ITEM_TIMER_BALL, ITEM_GUARD_SPEC };
static const u16 sSignatureMahogany[] = { ITEM_NET_BALL, ITEM_DUSK_BALL };
static const u16 sSignatureViridian[] = { ITEM_NEST_BALL, ITEM_POKE_DOLL, ITEM_ORANGE_MAIL };
static const u16 sSignaturePewter[] = { ITEM_DUSK_BALL, ITEM_X_DEFENSE };
static const u16 sSignatureCerulean[] = { ITEM_NET_BALL, ITEM_HEAL_BALL, ITEM_X_SPEED };
static const u16 sSignatureVermilion[] = { ITEM_NET_BALL, ITEM_DIVE_BALL, ITEM_HARBOR_MAIL };
static const u16 sSignatureLavender[] = { ITEM_DUSK_BALL, ITEM_HEAL_BALL, ITEM_SHADOW_MAIL };
static const u16 sSignatureCeladon2F[] = { ITEM_LUXURY_BALL, ITEM_POKE_DOLL, ITEM_RETRO_MAIL };
static const u16 sSignatureSaffron[] = { ITEM_X_SP_ATK, ITEM_X_SP_DEF, ITEM_GUARD_SPEC };
static const u16 sSignatureFuchsia[] = { ITEM_NET_BALL, ITEM_NEST_BALL, ITEM_FLUFFY_TAIL };
static const u16 sSignatureOldale[] = { ITEM_HEAL_BALL, ITEM_NEST_BALL };
static const u16 sSignaturePetalburg[] = { ITEM_NEST_BALL, ITEM_X_DEFENSE, ITEM_ORANGE_MAIL };
static const u16 sSignatureRustboro[] = { ITEM_TIMER_BALL, ITEM_REPEAT_BALL };
static const u16 sSignatureSlateport[] = { ITEM_NET_BALL, ITEM_DIVE_BALL, ITEM_HARBOR_MAIL, ITEM_LUXURY_BALL };
static const u16 sSignatureMauville[] = { ITEM_X_SPEED, ITEM_X_SP_ATK, ITEM_X_ACCURACY, ITEM_MECH_MAIL };
static const u16 sSignatureVerdanturf[] = { ITEM_NEST_BALL, ITEM_FLUFFY_TAIL };
static const u16 sSignatureFallarbor[] = { ITEM_DUSK_BALL, ITEM_DIRE_HIT, ITEM_X_DEFENSE };
static const u16 sSignatureLavaridge[] = { ITEM_HEAL_BALL, ITEM_GUARD_SPEC, ITEM_X_SP_DEF };
static const u16 sSignatureFortree[] = { ITEM_NEST_BALL, ITEM_NET_BALL, ITEM_WOOD_MAIL, ITEM_X_SPEED };
static const u16 sSignatureMossdeep[] = { ITEM_NET_BALL, ITEM_DIVE_BALL };
static const u16 sSignatureSootopolis[] = { ITEM_DIVE_BALL, ITEM_DUSK_BALL, ITEM_SHADOW_MAIL };
static const u16 sSignatureLilycove2FLeft[] = { ITEM_LUXURY_BALL, ITEM_FLUFFY_TAIL };
static const u16 sSignatureLilycove2FRight[] = { ITEM_WAVE_MAIL, ITEM_MECH_MAIL };

static const u16 sRetainedGoldenrod2F[] = { ITEM_FIRE_STONE, ITEM_WATER_STONE, ITEM_THUNDER_STONE };
static const u16 sRetainedPetalburg[] = { ITEM_X_SPEED, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_ORANGE_MAIL };
static const u16 sRetainedRustboro[] = { ITEM_X_SPEED, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_TIMER_BALL, ITEM_REPEAT_BALL };
static const u16 sRetainedSlateport[] = { ITEM_HARBOR_MAIL };
static const u16 sRetainedMauville[] = { ITEM_X_SPEED, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_GUARD_SPEC, ITEM_DIRE_HIT, ITEM_X_ACCURACY };
static const u16 sRetainedVerdanturf[] = { ITEM_NEST_BALL, ITEM_X_SP_ATK, ITEM_FLUFFY_TAIL };
static const u16 sRetainedFallarbor[] = { ITEM_X_SP_ATK, ITEM_X_SPEED, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_DIRE_HIT, ITEM_GUARD_SPEC };
static const u16 sRetainedLavaridge[] = { ITEM_X_SPEED };
static const u16 sRetainedFortree[] = { ITEM_WOOD_MAIL };
static const u16 sRetainedMossdeep[] = { ITEM_NET_BALL, ITEM_DIVE_BALL, ITEM_X_ATTACK, ITEM_X_DEFENSE };
static const u16 sRetainedSootopolis[] = { ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_SHADOW_MAIL };
static const u16 sRetainedLilycove2FLeft[] = { ITEM_FLUFFY_TAIL };
static const u16 sRetainedLilycove2FRight[] = { ITEM_WAVE_MAIL, ITEM_MECH_MAIL };
static const u16 sRetainedVitamins[] = { ITEM_PROTEIN, ITEM_CALCIUM, ITEM_IRON, ITEM_ZINC, ITEM_CARBOS, ITEM_HP_UP };
static const u16 sRetainedTrainerHill[] = { ITEM_X_SPEED, ITEM_X_SP_ATK, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_DIRE_HIT, ITEM_GUARD_SPEC, ITEM_X_ACCURACY };

#define MART_PROFILE_WITH_RETAINED(signature, retained, commonMask, ppRecovery, category) \
    { \
        .signatureItems = signature, \
        .retainedItems = retained, \
        .signatureItemCount = ARRAY_COUNT(signature), \
        .retainedItemCount = ARRAY_COUNT(retained), \
        .commonCategoryMask = commonMask, \
        .supportsPpRecovery = ppRecovery, \
        .catalogCategoryMask = category, \
    }

#define MART_PROFILE_NO_RETAINED(signature, commonMask, ppRecovery, category) \
    { \
        .signatureItems = signature, \
        .retainedItems = NULL, \
        .signatureItemCount = ARRAY_COUNT(signature), \
        .retainedItemCount = 0, \
        .commonCategoryMask = commonMask, \
        .supportsPpRecovery = ppRecovery, \
        .catalogCategoryMask = category, \
    }

#define MART_PROFILE_FACILITY(retained, category) \
    { \
        .signatureItems = NULL, \
        .retainedItems = retained, \
        .signatureItemCount = 0, \
        .retainedItemCount = ARRAY_COUNT(retained), \
        .commonCategoryMask = MART_COMMON_ALL, \
        .supportsPpRecovery = TRUE, \
        .catalogCategoryMask = category, \
    }

#define MART_PROFILE_EMPTY_FACILITY(category) \
    { \
        .signatureItems = NULL, \
        .retainedItems = NULL, \
        .signatureItemCount = 0, \
        .retainedItemCount = 0, \
        .commonCategoryMask = MART_COMMON_ALL, \
        .supportsPpRecovery = TRUE, \
        .catalogCategoryMask = category, \
    }

static const struct WayfarerMartProfile sWayfarerMartProfiles[MART_PROFILE_COUNT] =
{
    [MART_PROFILE_CHERRYGROVE] = MART_PROFILE_NO_RETAINED(sSignatureCherrygrove, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_VIOLET] = MART_PROFILE_NO_RETAINED(sSignatureViolet, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_AZALEA] = MART_PROFILE_NO_RETAINED(sSignatureAzalea, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_GOLDENROD_2F] = MART_PROFILE_WITH_RETAINED(sSignatureGoldenrod2F, sRetainedGoldenrod2F, MART_COMMON_ALL, TRUE, MART_CATEGORY_DEPT),
    [MART_PROFILE_ECRUTEAK] = MART_PROFILE_NO_RETAINED(sSignatureEcruteak, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_OLIVINE] = MART_PROFILE_NO_RETAINED(sSignatureOlivine, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_BLACKTHORN] = MART_PROFILE_NO_RETAINED(sSignatureBlackthorn, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_MAHOGANY] = MART_PROFILE_NO_RETAINED(sSignatureMahogany, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_VIRIDIAN] = MART_PROFILE_NO_RETAINED(sSignatureViridian, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_PEWTER] = MART_PROFILE_NO_RETAINED(sSignaturePewter, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_CERULEAN] = MART_PROFILE_NO_RETAINED(sSignatureCerulean, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_VERMILION] = MART_PROFILE_NO_RETAINED(sSignatureVermilion, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_LAVENDER] = MART_PROFILE_NO_RETAINED(sSignatureLavender, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_CELADON_2F] = MART_PROFILE_NO_RETAINED(sSignatureCeladon2F, MART_COMMON_ALL, TRUE, MART_CATEGORY_DEPT),
    [MART_PROFILE_SAFFRON] = MART_PROFILE_NO_RETAINED(sSignatureSaffron, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_FUCHSIA] = MART_PROFILE_NO_RETAINED(sSignatureFuchsia, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_OLDALE] = MART_PROFILE_NO_RETAINED(sSignatureOldale, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_PETALBURG] = MART_PROFILE_WITH_RETAINED(sSignaturePetalburg, sRetainedPetalburg, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_RUSTBORO] = MART_PROFILE_WITH_RETAINED(sSignatureRustboro, sRetainedRustboro, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_SLATEPORT] = MART_PROFILE_WITH_RETAINED(sSignatureSlateport, sRetainedSlateport, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_MAUVILLE] = MART_PROFILE_WITH_RETAINED(sSignatureMauville, sRetainedMauville, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_VERDANTURF] = MART_PROFILE_WITH_RETAINED(sSignatureVerdanturf, sRetainedVerdanturf, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_FALLARBOR] = MART_PROFILE_WITH_RETAINED(sSignatureFallarbor, sRetainedFallarbor, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_LAVARIDGE] = MART_PROFILE_WITH_RETAINED(sSignatureLavaridge, sRetainedLavaridge, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_FORTREE] = MART_PROFILE_WITH_RETAINED(sSignatureFortree, sRetainedFortree, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_MOSSDEEP] = MART_PROFILE_WITH_RETAINED(sSignatureMossdeep, sRetainedMossdeep, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_SOOTOPOLIS] = MART_PROFILE_WITH_RETAINED(sSignatureSootopolis, sRetainedSootopolis, MART_COMMON_ALL, TRUE, MART_CATEGORY_TOWN),
    [MART_PROFILE_LILYCOVE_2F_LEFT] = MART_PROFILE_WITH_RETAINED(sSignatureLilycove2FLeft, sRetainedLilycove2FLeft, MART_COMMON_BALLS | MART_COMMON_STATUS_CURE | MART_COMMON_ESCAPE_ROPE, FALSE, MART_CATEGORY_DEPT),
    [MART_PROFILE_LILYCOVE_2F_RIGHT] = MART_PROFILE_WITH_RETAINED(sSignatureLilycove2FRight, sRetainedLilycove2FRight, MART_COMMON_HP_MEDICINE | MART_COMMON_REVIVE | MART_COMMON_REPELS, TRUE, MART_CATEGORY_DEPT),
    [MART_PROFILE_INDIGO_PLATEAU] = MART_PROFILE_FACILITY(sRetainedVitamins, MART_CATEGORY_LEAGUE),
    [MART_PROFILE_EVER_GRANDE_LEAGUE] = MART_PROFILE_EMPTY_FACILITY(MART_CATEGORY_LEAGUE),
    [MART_PROFILE_BATTLE_FRONTIER_HNS] = MART_PROFILE_FACILITY(sRetainedVitamins, MART_CATEGORY_FACILITY),
    [MART_PROFILE_BATTLE_FRONTIER] = MART_PROFILE_FACILITY(sRetainedVitamins, MART_CATEGORY_FACILITY),
    [MART_PROFILE_TRAINER_HILL_HNS] = MART_PROFILE_EMPTY_FACILITY(MART_CATEGORY_FACILITY),
    [MART_PROFILE_TRAINER_HILL] = MART_PROFILE_FACILITY(sRetainedTrainerHill, MART_CATEGORY_FACILITY),
};

// The host manifest generator reads this non-emitting table alongside the
// profile definitions above.  It keeps map and script ownership auditable
// without putting debug strings in the ROM.  The final string is the authored
// literal-list source (or an explicit no-retained-stock classification).
#define WAYFARER_MART_PROFILE_BINDINGS(X) \
    X(MART_PROFILE_CHERRYGROVE, "CherrygroveCity_Mart_hns", MAP_CHERRYGROVE_CITY_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_VIOLET, "VioletCity_Mart_hns", MAP_VIOLET_CITY_MART_HNS, "VioletCity_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_AZALEA, "AzaleaTown_Mart_hns", MAP_AZALEA_TOWN_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_GOLDENROD_2F, "GoldenrodCity_DepartmentStore_2F_hns", MAP_GOLDENROD_CITY_DEPARTMENT_STORE_2F_HNS, "GoldenrodDeptStore2_EventScript_Clerk", MART_CATEGORY_DEPT, "GoldenrodCity_DepartmentStore_2F_hns/scripts.inc:GoldenrodDeptStore2_EventScript_Clerk") \
    X(MART_PROFILE_ECRUTEAK, "EcruteakCity_Mart_hns", MAP_ECRUTEAK_CITY_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_OLIVINE, "OlivineCity_Mart_hns", MAP_OLIVINE_CITY_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_BLACKTHORN, "BlackthornCity_Mart_hns", MAP_BLACKTHORN_CITY_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_MAHOGANY, "Mahoganytown_hns", MAP_MAHOGANYTOWN_HNS, "MahoganyTown_EventScript_Merchant", MART_CATEGORY_TOWN, "new exterior Supplies service; no ordinary interior list retained") \
    X(MART_PROFILE_VIRIDIAN, "ViridianCity_Mart_hns", MAP_VIRIDIAN_CITY_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_PEWTER, "PewterCity_Mart_hns", MAP_PEWTER_CITY_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_CERULEAN, "CeruleanCity_Mart_hns", MAP_CERULEAN_CITY_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_VERMILION, "VermilionCity_Mart_hns", MAP_VERMILION_CITY_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_LAVENDER, "LavenderTown_Mart_hns", MAP_LAVENDER_TOWN_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_CELADON_2F, "CeladonCity_DepartmentStore_2F_hns", MAP_CELADON_CITY_DEPARTMENT_STORE_2F_HNS, "VioletCity_Mart_EventScript_Clerk", MART_CATEGORY_DEPT, "legacy shared NULL catalog") \
    X(MART_PROFILE_SAFFRON, "SaffronCity_Mart_hns", MAP_SAFFRON_CITY_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_FUCHSIA, "FuchsiaCity_Mart_hns", MAP_FUCHSIA_CITY_MART_HNS, "Cherrygrove_Pokemart_EventScript_Clerk", MART_CATEGORY_TOWN, "legacy shared NULL catalog") \
    X(MART_PROFILE_OLDALE, "OldaleTown_Mart", MAP_OLDALE_TOWN_MART, "OldaleTown_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "OldaleTown_Mart/scripts.inc:OldaleTown_Mart_EventScript_Clerk") \
    X(MART_PROFILE_PETALBURG, "PetalburgCity_Mart", MAP_PETALBURG_CITY_MART, "PetalburgCity_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "PetalburgCity_Mart/scripts.inc:PetalburgCity_Mart_EventScript_ClerkItems") \
    X(MART_PROFILE_RUSTBORO, "RustboroCity_Mart", MAP_RUSTBORO_CITY_MART, "RustboroCity_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "RustboroCity_Mart/scripts.inc:RustboroCity_Mart_EventScript_ClerkItems") \
    X(MART_PROFILE_SLATEPORT, "SlateportCity_Mart", MAP_SLATEPORT_CITY_MART, "SlateportCity_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "SlateportCity_Mart/scripts.inc:SlateportCity_Mart_EventScript_Clerk") \
    X(MART_PROFILE_MAUVILLE, "MauvilleCity_Mart", MAP_MAUVILLE_CITY_MART, "MauvilleCity_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "MauvilleCity_Mart/scripts.inc:MauvilleCity_Mart_EventScript_Clerk") \
    X(MART_PROFILE_VERDANTURF, "VerdanturfTown_Mart", MAP_VERDANTURF_TOWN_MART, "VerdanturfTown_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "VerdanturfTown_Mart/scripts.inc:VerdanturfTown_Mart_EventScript_Clerk") \
    X(MART_PROFILE_FALLARBOR, "FallarborTown_Mart", MAP_FALLARBOR_TOWN_MART, "FallarborTown_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "FallarborTown_Mart/scripts.inc:FallarborTown_Mart_EventScript_Clerk") \
    X(MART_PROFILE_LAVARIDGE, "LavaridgeTown_Mart", MAP_LAVARIDGE_TOWN_MART, "LavaridgeTown_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "LavaridgeTown_Mart/scripts.inc:LavaridgeTown_Mart_EventScript_Clerk") \
    X(MART_PROFILE_FORTREE, "FortreeCity_Mart", MAP_FORTREE_CITY_MART, "FortreeCity_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "FortreeCity_Mart/scripts.inc:FortreeCity_Mart_EventScript_Clerk") \
    X(MART_PROFILE_MOSSDEEP, "MossdeepCity_Mart", MAP_MOSSDEEP_CITY_MART, "MossdeepCity_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "MossdeepCity_Mart/scripts.inc:MossdeepCity_Mart_EventScript_Clerk") \
    X(MART_PROFILE_SOOTOPOLIS, "SootopolisCity_Mart", MAP_SOOTOPOLIS_CITY_MART, "SootopolisCity_Mart_EventScript_Clerk", MART_CATEGORY_TOWN, "SootopolisCity_Mart/scripts.inc:SootopolisCity_Mart_EventScript_Clerk") \
    X(MART_PROFILE_LILYCOVE_2F_LEFT, "LilycoveCity_DepartmentStore_2F", MAP_LILYCOVE_CITY_DEPARTMENT_STORE_2F, "LilycoveCity_DepartmentStore_2F_EventScript_ClerkLeft", MART_CATEGORY_DEPT, "LilycoveCity_DepartmentStore_2F/scripts.inc:LilycoveCity_DepartmentStore_2F_EventScript_ClerkLeft") \
    X(MART_PROFILE_LILYCOVE_2F_RIGHT, "LilycoveCity_DepartmentStore_2F", MAP_LILYCOVE_CITY_DEPARTMENT_STORE_2F, "LilycoveCity_DepartmentStore_2F_EventScript_ClerkRight", MART_CATEGORY_DEPT, "LilycoveCity_DepartmentStore_2F/scripts.inc:LilycoveCity_DepartmentStore_2F_EventScript_ClerkRight") \
    X(MART_PROFILE_INDIGO_PLATEAU, "IndigoPlateau_PokemonCenter_hns", MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS, "BattleFrontier_Mart_EventScript_Clerk", MART_CATEGORY_LEAGUE, "BattleFrontier_Mart source reused by Indigo") \
    X(MART_PROFILE_EVER_GRANDE_LEAGUE, "EverGrandeCity_PokemonLeague_1F", MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F, "EverGrandeCity_PokemonLeague_1F_EventScript_Clerk", MART_CATEGORY_LEAGUE, "EverGrandeCity_PokemonLeague_1F/scripts.inc:EverGrandeCity_PokemonLeague_1F_EventScript_Clerk") \
    X(MART_PROFILE_BATTLE_FRONTIER_HNS, "BattleFrontier_Mart_hns", MAP_BATTLE_FRONTIER_MART_HNS, "BattleFrontier_Mart_EventScript_Clerk_hns", MART_CATEGORY_FACILITY, "BattleFrontier_Mart_hns/scripts.inc:BattleFrontier_Mart_EventScript_Clerk_hns") \
    X(MART_PROFILE_BATTLE_FRONTIER, "BattleFrontier_Mart", MAP_BATTLE_FRONTIER_MART, "BattleFrontier_Mart_EventScript_Clerk", MART_CATEGORY_FACILITY, "BattleFrontier_Mart/scripts.inc:BattleFrontier_Mart_EventScript_Clerk") \
    X(MART_PROFILE_TRAINER_HILL_HNS, "TrainerHill_Entrance_hns", MAP_TRAINER_HILL_ENTRANCE_HNS, "TrainerHill_Entrance_hns_EventScript_Clerk2", MART_CATEGORY_FACILITY, "TrainerHill_Entrance_hns/scripts.inc:TrainerHill_Entrance_hns_EventScript_Clerk2") \
    X(MART_PROFILE_TRAINER_HILL, "TrainerHill_Entrance", MAP_TRAINER_HILL_ENTRANCE, "TrainerHill_Entrance_EventScript_Clerk", MART_CATEGORY_FACILITY, "TrainerHill_Entrance/scripts.inc:TrainerHill_Entrance_EventScript_Clerk")

#undef MART_PROFILE_WITH_RETAINED
#undef MART_PROFILE_NO_RETAINED
#undef MART_PROFILE_FACILITY
#undef MART_PROFILE_EMPTY_FACILITY

static const struct WayfarerMartSharedClerkBinding sWayfarerMartSharedClerkBindings[] =
{
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_CHERRYGROVE_CITY_MART_HNS), MAP_NUM(MAP_CHERRYGROVE_CITY_MART_HNS), MART_PROFILE_CHERRYGROVE },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_AZALEA_TOWN_MART_HNS), MAP_NUM(MAP_AZALEA_TOWN_MART_HNS), MART_PROFILE_AZALEA },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_ECRUTEAK_CITY_MART_HNS), MAP_NUM(MAP_ECRUTEAK_CITY_MART_HNS), MART_PROFILE_ECRUTEAK },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_OLIVINE_CITY_MART_HNS), MAP_NUM(MAP_OLIVINE_CITY_MART_HNS), MART_PROFILE_OLIVINE },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_BLACKTHORN_CITY_MART_HNS), MAP_NUM(MAP_BLACKTHORN_CITY_MART_HNS), MART_PROFILE_BLACKTHORN },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_VIRIDIAN_CITY_MART_HNS), MAP_NUM(MAP_VIRIDIAN_CITY_MART_HNS), MART_PROFILE_VIRIDIAN },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_PEWTER_CITY_MART_HNS), MAP_NUM(MAP_PEWTER_CITY_MART_HNS), MART_PROFILE_PEWTER },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_CERULEAN_CITY_MART_HNS), MAP_NUM(MAP_CERULEAN_CITY_MART_HNS), MART_PROFILE_CERULEAN },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_VERMILION_CITY_MART_HNS), MAP_NUM(MAP_VERMILION_CITY_MART_HNS), MART_PROFILE_VERMILION },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_LAVENDER_TOWN_MART_HNS), MAP_NUM(MAP_LAVENDER_TOWN_MART_HNS), MART_PROFILE_LAVENDER },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_SAFFRON_CITY_MART_HNS), MAP_NUM(MAP_SAFFRON_CITY_MART_HNS), MART_PROFILE_SAFFRON },
    { MART_CLERK_FAMILY_CHERRYGROVE, MAP_GROUP(MAP_FUCHSIA_CITY_MART_HNS), MAP_NUM(MAP_FUCHSIA_CITY_MART_HNS), MART_PROFILE_FUCHSIA },
    { MART_CLERK_FAMILY_VIOLET, MAP_GROUP(MAP_VIOLET_CITY_MART_HNS), MAP_NUM(MAP_VIOLET_CITY_MART_HNS), MART_PROFILE_VIOLET },
    { MART_CLERK_FAMILY_VIOLET, MAP_GROUP(MAP_CELADON_CITY_DEPARTMENT_STORE_2F_HNS), MAP_NUM(MAP_CELADON_CITY_DEPARTMENT_STORE_2F_HNS), MART_PROFILE_CELADON_2F },
    { MART_CLERK_FAMILY_FRONTIER, MAP_GROUP(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS), MAP_NUM(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS), MART_PROFILE_INDIGO_PLATEAU },
    { MART_CLERK_FAMILY_FRONTIER, MAP_GROUP(MAP_BATTLE_FRONTIER_MART_HNS), MAP_NUM(MAP_BATTLE_FRONTIER_MART_HNS), MART_PROFILE_BATTLE_FRONTIER_HNS },
    { MART_CLERK_FAMILY_FRONTIER, MAP_GROUP(MAP_BATTLE_FRONTIER_MART), MAP_NUM(MAP_BATTLE_FRONTIER_MART), MART_PROFILE_BATTLE_FRONTIER },
};

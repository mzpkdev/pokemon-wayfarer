#include "global.h"
#include "move_relearner.h"
#include "pokemon.h"
#include "test/test.h"
#include "wild_encounter.h"
#include "constants/maps.h"

struct ExpectedLevelMove
{
    u8 level;
    u16 move;
};

struct NativeHmAnchor
{
    u16 species;
    u8 floor;
    u8 moveCount;
    const u16 *moves;
    const struct ExpectedLevelMove *modern;
    u8 modernCount;
    const struct ExpectedLevelMove *legacy;
    u8 legacyCount;
};

struct NativeHmSuccessor
{
    u16 anchor;
    u16 species;
    u8 moveCount;
    const u16 *moves;
};

#define LM(level, move) { level, move }
#define ANCHOR(species, floor, moves, modern, legacy) \
    { species, floor, ARRAY_COUNT(moves), moves, modern, ARRAY_COUNT(modern), legacy, ARRAY_COUNT(legacy) }
#define SUCCESSOR(anchor, species, moves) { anchor, species, ARRAY_COUNT(moves), moves }

#if IS_FRLG

static const u16 sParasMoves[] = { MOVE_CUT };
static const struct ExpectedLevelMove sParasModern[] = { LM(5, MOVE_CUT), LM(17, MOVE_CUT), LM(38, MOVE_CUT) };
static const struct ExpectedLevelMove sParasLegacy[] = { LM(5, MOVE_CUT), LM(25, MOVE_CUT), LM(49, MOVE_CUT) };
static const u16 sRattataMoves[] = { MOVE_CUT };
static const struct ExpectedLevelMove sRattataModern[] = { LM(2, MOVE_CUT), LM(13, MOVE_CUT), LM(25, MOVE_CUT) };
static const struct ExpectedLevelMove sRattataLegacy[] = { LM(2, MOVE_CUT), LM(27, MOVE_CUT) };
static const u16 sVoltorbMoves[] = { MOVE_FLASH };
static const struct ExpectedLevelMove sVoltorbModern[] = { LM(14, MOVE_FLASH), LM(26, MOVE_FLASH), LM(37, MOVE_FLASH) };
static const struct ExpectedLevelMove sVoltorbLegacy[] = { LM(14, MOVE_FLASH), LM(32, MOVE_FLASH), LM(49, MOVE_FLASH) };
static const u16 sPikachuMoves[] = { MOVE_FLASH };
static const struct ExpectedLevelMove sPikachuModern[] = { LM(3, MOVE_FLASH), LM(13, MOVE_FLASH), LM(26, MOVE_FLASH), LM(39, MOVE_FLASH), LM(50, MOVE_FLASH) };
static const struct ExpectedLevelMove sPikachuLegacy[] = { LM(3, MOVE_FLASH), LM(15, MOVE_FLASH), LM(41, MOVE_FLASH) };
static const u16 sHorseaMoves[] = { MOVE_SURF, MOVE_WATERFALL };
static const struct ExpectedLevelMove sHorseaModern[] = { LM(5, MOVE_SURF), LM(5, MOVE_WATERFALL), LM(17, MOVE_SURF), LM(17, MOVE_WATERFALL), LM(31, MOVE_SURF), LM(31, MOVE_WATERFALL), LM(46, MOVE_SURF), LM(46, MOVE_WATERFALL) };
static const struct ExpectedLevelMove sHorseaLegacy[] = { LM(5, MOVE_SURF), LM(5, MOVE_WATERFALL), LM(22, MOVE_SURF), LM(22, MOVE_WATERFALL), LM(43, MOVE_SURF), LM(43, MOVE_WATERFALL) };
static const u16 sKrabbyMoves[] = { MOVE_SURF };
static const struct ExpectedLevelMove sKrabbyModern[] = { LM(5, MOVE_SURF), LM(19, MOVE_SURF), LM(31, MOVE_SURF), LM(45, MOVE_SURF) };
static const struct ExpectedLevelMove sKrabbyLegacy[] = { LM(5, MOVE_SURF), LM(27, MOVE_SURF) };
static const u16 sMachopMoves[] = { MOVE_STRENGTH };
static const struct ExpectedLevelMove sMachopModern[] = { LM(16, MOVE_STRENGTH), LM(27, MOVE_STRENGTH), LM(39, MOVE_STRENGTH) };
static const struct ExpectedLevelMove sMachopLegacy[] = { LM(16, MOVE_STRENGTH), LM(31, MOVE_STRENGTH), LM(49, MOVE_STRENGTH) };
static const u16 sGeodudeMoves[] = { MOVE_STRENGTH, MOVE_ROCK_SMASH };
static const struct ExpectedLevelMove sGeodudeModern[] = { LM(7, MOVE_STRENGTH), LM(7, MOVE_ROCK_SMASH), LM(16, MOVE_STRENGTH), LM(16, MOVE_ROCK_SMASH), LM(24, MOVE_STRENGTH), LM(24, MOVE_ROCK_SMASH), LM(34, MOVE_STRENGTH), LM(34, MOVE_ROCK_SMASH), LM(42, MOVE_STRENGTH), LM(42, MOVE_ROCK_SMASH) };
static const struct ExpectedLevelMove sGeodudeLegacy[] = { LM(7, MOVE_STRENGTH), LM(7, MOVE_ROCK_SMASH), LM(21, MOVE_STRENGTH), LM(21, MOVE_ROCK_SMASH), LM(36, MOVE_STRENGTH), LM(36, MOVE_ROCK_SMASH) };
static const u16 sMankeyMoves[] = { MOVE_ROCK_SMASH };
static const struct ExpectedLevelMove sMankeyModern[] = { LM(2, MOVE_ROCK_SMASH), LM(15, MOVE_ROCK_SMASH), LM(29, MOVE_ROCK_SMASH), LM(43, MOVE_ROCK_SMASH) };
static const struct ExpectedLevelMove sMankeyLegacy[] = { LM(2, MOVE_ROCK_SMASH), LM(27, MOVE_ROCK_SMASH), LM(51, MOVE_ROCK_SMASH) };
static const u16 sGoldeenMoves[] = { MOVE_WATERFALL };
static const struct ExpectedLevelMove sGoldeenModern[] = { LM(5, MOVE_WATERFALL), LM(21, MOVE_WATERFALL), LM(32, MOVE_WATERFALL), LM(40, MOVE_WATERFALL) };
static const struct ExpectedLevelMove sGoldeenLegacy[] = { LM(5, MOVE_WATERFALL), LM(29, MOVE_WATERFALL), LM(38, MOVE_WATERFALL) };

static const struct NativeHmAnchor sAnchors[] =
{
    ANCHOR(SPECIES_PARAS, 5, sParasMoves, sParasModern, sParasLegacy),
    ANCHOR(SPECIES_RATTATA, 2, sRattataMoves, sRattataModern, sRattataLegacy),
    ANCHOR(SPECIES_VOLTORB, 14, sVoltorbMoves, sVoltorbModern, sVoltorbLegacy),
    ANCHOR(SPECIES_PIKACHU, 3, sPikachuMoves, sPikachuModern, sPikachuLegacy),
    ANCHOR(SPECIES_HORSEA, 5, sHorseaMoves, sHorseaModern, sHorseaLegacy),
    ANCHOR(SPECIES_KRABBY, 5, sKrabbyMoves, sKrabbyModern, sKrabbyLegacy),
    ANCHOR(SPECIES_MACHOP, 16, sMachopMoves, sMachopModern, sMachopLegacy),
    ANCHOR(SPECIES_GEODUDE, 7, sGeodudeMoves, sGeodudeModern, sGeodudeLegacy),
    ANCHOR(SPECIES_MANKEY, 2, sMankeyMoves, sMankeyModern, sMankeyLegacy),
    ANCHOR(SPECIES_GOLDEEN, 5, sGoldeenMoves, sGoldeenModern, sGoldeenLegacy),
};

static const struct NativeHmSuccessor sSuccessors[] =
{
    SUCCESSOR(SPECIES_PARAS, SPECIES_PARASECT, sParasMoves),
    SUCCESSOR(SPECIES_RATTATA, SPECIES_RATICATE, sRattataMoves),
    SUCCESSOR(SPECIES_VOLTORB, SPECIES_ELECTRODE, sVoltorbMoves),
    SUCCESSOR(SPECIES_PIKACHU, SPECIES_RAICHU, sPikachuMoves),
    SUCCESSOR(SPECIES_PIKACHU, SPECIES_RAICHU_ALOLA, sPikachuMoves),
    SUCCESSOR(SPECIES_HORSEA, SPECIES_SEADRA, sHorseaMoves),
    SUCCESSOR(SPECIES_HORSEA, SPECIES_KINGDRA, sHorseaMoves),
    SUCCESSOR(SPECIES_KRABBY, SPECIES_KINGLER, sKrabbyMoves),
    SUCCESSOR(SPECIES_MACHOP, SPECIES_MACHOKE, sMachopMoves),
    SUCCESSOR(SPECIES_MACHOP, SPECIES_MACHAMP, sMachopMoves),
    SUCCESSOR(SPECIES_GEODUDE, SPECIES_GRAVELER, sGeodudeMoves),
    SUCCESSOR(SPECIES_GEODUDE, SPECIES_GOLEM, sGeodudeMoves),
    SUCCESSOR(SPECIES_MANKEY, SPECIES_PRIMEAPE, sMankeyMoves),
    SUCCESSOR(SPECIES_MANKEY, SPECIES_ANNIHILAPE, sMankeyMoves),
    SUCCESSOR(SPECIES_GOLDEEN, SPECIES_SEAKING, sGoldeenMoves),
};

#elif IS_HNS

static const u16 sGligarMoves[] = { MOVE_CUT };
static const struct ExpectedLevelMove sGligarModern[] = { LM(19, MOVE_CUT), LM(35, MOVE_CUT), LM(55, MOVE_CUT) };
static const struct ExpectedLevelMove sGligarLegacy[] = { LM(19, MOVE_CUT), LM(44, MOVE_CUT) };
static const u16 sAipomMoves[] = { MOVE_CUT, MOVE_ROCK_SMASH };
static const struct ExpectedLevelMove sAipomModern[] = { LM(10, MOVE_CUT), LM(10, MOVE_ROCK_SMASH), LM(18, MOVE_CUT), LM(18, MOVE_ROCK_SMASH), LM(29, MOVE_CUT), LM(29, MOVE_ROCK_SMASH), LM(39, MOVE_CUT), LM(39, MOVE_ROCK_SMASH) };
static const struct ExpectedLevelMove sAipomLegacy[] = { LM(10, MOVE_CUT), LM(10, MOVE_ROCK_SMASH), LM(25, MOVE_CUT), LM(25, MOVE_ROCK_SMASH), LM(38, MOVE_CUT), LM(38, MOVE_ROCK_SMASH) };
static const u16 sChinchouMoves[] = { MOVE_FLASH, MOVE_SURF, MOVE_WHIRLPOOL };
static const struct ExpectedLevelMove sChinchouModern[] = { LM(20, MOVE_FLASH), LM(20, MOVE_SURF), LM(20, MOVE_WHIRLPOOL), LM(28, MOVE_FLASH), LM(28, MOVE_SURF), LM(28, MOVE_WHIRLPOOL), LM(34, MOVE_FLASH), LM(34, MOVE_SURF), LM(34, MOVE_WHIRLPOOL), LM(42, MOVE_FLASH), LM(42, MOVE_SURF), LM(42, MOVE_WHIRLPOOL), LM(47, MOVE_FLASH), LM(47, MOVE_SURF), LM(47, MOVE_WHIRLPOOL) };
static const struct ExpectedLevelMove sChinchouLegacy[] = { LM(20, MOVE_FLASH), LM(20, MOVE_SURF), LM(20, MOVE_WHIRLPOOL), LM(29, MOVE_FLASH), LM(29, MOVE_SURF), LM(29, MOVE_WHIRLPOOL), LM(41, MOVE_FLASH), LM(41, MOVE_SURF), LM(41, MOVE_WHIRLPOOL) };
static const u16 sMareepMoves[] = { MOVE_FLASH };
static const struct ExpectedLevelMove sMareepModern[] = { LM(5, MOVE_FLASH), LM(18, MOVE_FLASH), LM(32, MOVE_FLASH), LM(46, MOVE_FLASH) };
static const struct ExpectedLevelMove sMareepLegacy[] = { LM(5, MOVE_FLASH), LM(30, MOVE_FLASH) };
static const u16 sWooperMoves[] = { MOVE_SURF, MOVE_WATERFALL };
static const struct ExpectedLevelMove sWooperModern[] = { LM(4, MOVE_SURF), LM(4, MOVE_WATERFALL), LM(15, MOVE_SURF), LM(15, MOVE_WATERFALL), LM(29, MOVE_SURF), LM(29, MOVE_WATERFALL), LM(43, MOVE_SURF), LM(43, MOVE_WATERFALL) };
static const struct ExpectedLevelMove sWooperLegacy[] = { LM(4, MOVE_SURF), LM(4, MOVE_WATERFALL), LM(21, MOVE_SURF), LM(21, MOVE_WATERFALL), LM(41, MOVE_SURF), LM(41, MOVE_WATERFALL) };
static const u16 sSnubbullMoves[] = { MOVE_STRENGTH };
static const struct ExpectedLevelMove sSnubbullModern[] = { LM(13, MOVE_STRENGTH), LM(37, MOVE_STRENGTH) };
static const struct ExpectedLevelMove sSnubbullLegacy[] = { LM(13, MOVE_STRENGTH), LM(43, MOVE_STRENGTH) };
static const u16 sMiltankMoves[] = { MOVE_STRENGTH, MOVE_ROCK_SMASH };
static const struct ExpectedLevelMove sMiltankModern[] = { LM(21, MOVE_STRENGTH), LM(21, MOVE_ROCK_SMASH), LM(35, MOVE_STRENGTH), LM(35, MOVE_ROCK_SMASH), LM(50, MOVE_STRENGTH), LM(50, MOVE_ROCK_SMASH) };
static const struct ExpectedLevelMove sMiltankLegacy[] = { LM(21, MOVE_STRENGTH), LM(21, MOVE_ROCK_SMASH), LM(43, MOVE_STRENGTH), LM(43, MOVE_ROCK_SMASH) };
static const u16 sMarillMoves[] = { MOVE_WATERFALL };
static const struct ExpectedLevelMove sMarillModern[] = { LM(8, MOVE_WATERFALL), LM(16, MOVE_WATERFALL), LM(31, MOVE_WATERFALL) };
static const struct ExpectedLevelMove sMarillLegacy[] = { LM(8, MOVE_WATERFALL), LM(28, MOVE_WATERFALL) };
static const u16 sMantineMoves[] = { MOVE_WHIRLPOOL };
static const struct ExpectedLevelMove sMantineModern[] = { LM(15, MOVE_WHIRLPOOL), LM(27, MOVE_WHIRLPOOL), LM(46, MOVE_WHIRLPOOL) };
static const struct ExpectedLevelMove sMantineLegacy[] = { LM(15, MOVE_WHIRLPOOL), LM(43, MOVE_WHIRLPOOL) };

static const struct NativeHmAnchor sAnchors[] =
{
    ANCHOR(SPECIES_GLIGAR, 21, sGligarMoves, sGligarModern, sGligarLegacy),
    ANCHOR(SPECIES_AIPOM, 10, sAipomMoves, sAipomModern, sAipomLegacy),
    ANCHOR(SPECIES_CHINCHOU, 20, sChinchouMoves, sChinchouModern, sChinchouLegacy),
    ANCHOR(SPECIES_MAREEP, 5, sMareepMoves, sMareepModern, sMareepLegacy),
    ANCHOR(SPECIES_WOOPER, 4, sWooperMoves, sWooperModern, sWooperLegacy),
    ANCHOR(SPECIES_SNUBBULL, 13, sSnubbullMoves, sSnubbullModern, sSnubbullLegacy),
    ANCHOR(SPECIES_MILTANK, 21, sMiltankMoves, sMiltankModern, sMiltankLegacy),
    ANCHOR(SPECIES_MARILL, 8, sMarillMoves, sMarillModern, sMarillLegacy),
    ANCHOR(SPECIES_MANTINE, 15, sMantineMoves, sMantineModern, sMantineLegacy),
};

static const struct NativeHmSuccessor sSuccessors[] =
{
    SUCCESSOR(SPECIES_GLIGAR, SPECIES_GLISCOR, sGligarMoves),
    SUCCESSOR(SPECIES_AIPOM, SPECIES_AMBIPOM, sAipomMoves),
    SUCCESSOR(SPECIES_CHINCHOU, SPECIES_LANTURN, sChinchouMoves),
    SUCCESSOR(SPECIES_MAREEP, SPECIES_FLAAFFY, sMareepMoves),
    SUCCESSOR(SPECIES_MAREEP, SPECIES_AMPHAROS, sMareepMoves),
    SUCCESSOR(SPECIES_WOOPER, SPECIES_QUAGSIRE, sWooperMoves),
    SUCCESSOR(SPECIES_SNUBBULL, SPECIES_GRANBULL, sSnubbullMoves),
    SUCCESSOR(SPECIES_MARILL, SPECIES_AZUMARILL, sMarillMoves),
};

#else

static const u16 sCorphishMoves[] = { MOVE_CUT, MOVE_ROCK_SMASH };
static const struct ExpectedLevelMove sCorphishModern[] = { LM(10, MOVE_CUT), LM(10, MOVE_ROCK_SMASH), LM(20, MOVE_CUT), LM(20, MOVE_ROCK_SMASH), LM(31, MOVE_CUT), LM(31, MOVE_ROCK_SMASH), LM(39, MOVE_CUT), LM(39, MOVE_ROCK_SMASH) };
static const struct ExpectedLevelMove sCorphishLegacy[] = { LM(10, MOVE_CUT), LM(10, MOVE_ROCK_SMASH), LM(23, MOVE_CUT), LM(23, MOVE_ROCK_SMASH), LM(35, MOVE_CUT), LM(35, MOVE_ROCK_SMASH) };
static const u16 sSableyeMoves[] = { MOVE_CUT, MOVE_FLASH };
static const struct ExpectedLevelMove sSableyeModern[] = { LM(9, MOVE_CUT), LM(9, MOVE_FLASH), LM(16, MOVE_CUT), LM(16, MOVE_FLASH), LM(24, MOVE_CUT), LM(24, MOVE_FLASH), LM(31, MOVE_CUT), LM(31, MOVE_FLASH), LM(39, MOVE_CUT), LM(39, MOVE_FLASH), LM(46, MOVE_CUT), LM(46, MOVE_FLASH) };
static const struct ExpectedLevelMove sSableyeLegacy[] = { LM(9, MOVE_CUT), LM(9, MOVE_FLASH), LM(21, MOVE_CUT), LM(21, MOVE_FLASH), LM(33, MOVE_CUT), LM(33, MOVE_FLASH), LM(45, MOVE_CUT), LM(45, MOVE_FLASH) };
static const u16 sElectrikeMoves[] = { MOVE_FLASH };
static const struct ExpectedLevelMove sElectrikeModern[] = { LM(12, MOVE_FLASH), LM(24, MOVE_FLASH), LM(44, MOVE_FLASH) };
static const struct ExpectedLevelMove sElectrikeLegacy[] = { LM(12, MOVE_FLASH), LM(28, MOVE_FLASH) };
static const u16 sLotadMoves[] = { MOVE_SURF };
static const struct ExpectedLevelMove sLotadModern[] = { LM(3, MOVE_SURF), LM(15, MOVE_SURF), LM(27, MOVE_SURF) };
static const struct ExpectedLevelMove sLotadLegacy[] = { LM(3, MOVE_SURF), LM(31, MOVE_SURF) };
static const u16 sWailmerMoves[] = { MOVE_SURF, MOVE_DIVE };
static const struct ExpectedLevelMove sWailmerModern[] = { LM(10, MOVE_SURF), LM(10, MOVE_DIVE), LM(19, MOVE_SURF), LM(19, MOVE_DIVE), LM(29, MOVE_SURF), LM(29, MOVE_DIVE), LM(41, MOVE_DIVE), LM(45, MOVE_SURF), LM(45, MOVE_DIVE) };
static const struct ExpectedLevelMove sWailmerLegacy[] = { LM(10, MOVE_SURF), LM(10, MOVE_DIVE), LM(23, MOVE_SURF), LM(23, MOVE_DIVE), LM(37, MOVE_SURF), LM(37, MOVE_DIVE), LM(50, MOVE_SURF), LM(50, MOVE_DIVE) };
static const u16 sMakuhitaMoves[] = { MOVE_STRENGTH };
static const struct ExpectedLevelMove sMakuhitaModern[] = { LM(6, MOVE_STRENGTH), LM(16, MOVE_STRENGTH), LM(28, MOVE_STRENGTH), LM(40, MOVE_STRENGTH) };
static const struct ExpectedLevelMove sMakuhitaLegacy[] = { LM(6, MOVE_STRENGTH), LM(22, MOVE_STRENGTH), LM(40, MOVE_STRENGTH) };
static const u16 sTorkoalMoves[] = { MOVE_STRENGTH };
static const struct ExpectedLevelMove sTorkoalModern[] = { LM(14, MOVE_STRENGTH), LM(25, MOVE_STRENGTH), LM(38, MOVE_STRENGTH), LM(47, MOVE_STRENGTH) };
static const struct ExpectedLevelMove sTorkoalLegacy[] = { LM(14, MOVE_STRENGTH), LM(30, MOVE_STRENGTH), LM(46, MOVE_STRENGTH) };
static const u16 sAronMoves[] = { MOVE_ROCK_SMASH };
static const struct ExpectedLevelMove sAronModern[] = { LM(7, MOVE_ROCK_SMASH), LM(19, MOVE_ROCK_SMASH), LM(31, MOVE_ROCK_SMASH), LM(43, MOVE_ROCK_SMASH) };
static const struct ExpectedLevelMove sAronLegacy[] = { LM(7, MOVE_ROCK_SMASH), LM(21, MOVE_ROCK_SMASH), LM(39, MOVE_ROCK_SMASH) };
static const u16 sBarboachMoves[] = { MOVE_WATERFALL };
static const struct ExpectedLevelMove sBarboachModern[] = { LM(10, MOVE_WATERFALL), LM(20, MOVE_WATERFALL), LM(32, MOVE_WATERFALL) };
static const struct ExpectedLevelMove sBarboachLegacy[] = { LM(10, MOVE_WATERFALL), LM(26, MOVE_WATERFALL) };
static const u16 sCarvanhaMoves[] = { MOVE_WATERFALL, MOVE_DIVE };
static const struct ExpectedLevelMove sCarvanhaModern[] = { LM(10, MOVE_WATERFALL), LM(10, MOVE_DIVE), LM(18, MOVE_WATERFALL), LM(18, MOVE_DIVE), LM(29, MOVE_WATERFALL), LM(29, MOVE_DIVE), LM(39, MOVE_WATERFALL), LM(39, MOVE_DIVE) };
static const struct ExpectedLevelMove sCarvanhaLegacy[] = { LM(10, MOVE_WATERFALL), LM(10, MOVE_DIVE), LM(22, MOVE_WATERFALL), LM(22, MOVE_DIVE), LM(37, MOVE_WATERFALL), LM(37, MOVE_DIVE) };

static const struct NativeHmAnchor sAnchors[] =
{
    ANCHOR(SPECIES_CORPHISH, 10, sCorphishMoves, sCorphishModern, sCorphishLegacy),
    ANCHOR(SPECIES_SABLEYE, 9, sSableyeMoves, sSableyeModern, sSableyeLegacy),
    ANCHOR(SPECIES_ELECTRIKE, 12, sElectrikeMoves, sElectrikeModern, sElectrikeLegacy),
    ANCHOR(SPECIES_LOTAD, 3, sLotadMoves, sLotadModern, sLotadLegacy),
    ANCHOR(SPECIES_WAILMER, 10, sWailmerMoves, sWailmerModern, sWailmerLegacy),
    ANCHOR(SPECIES_MAKUHITA, 6, sMakuhitaMoves, sMakuhitaModern, sMakuhitaLegacy),
    ANCHOR(SPECIES_TORKOAL, 14, sTorkoalMoves, sTorkoalModern, sTorkoalLegacy),
    ANCHOR(SPECIES_ARON, 7, sAronMoves, sAronModern, sAronLegacy),
    ANCHOR(SPECIES_BARBOACH, 10, sBarboachMoves, sBarboachModern, sBarboachLegacy),
    ANCHOR(SPECIES_CARVANHA, 10, sCarvanhaMoves, sCarvanhaModern, sCarvanhaLegacy),
};

static const struct NativeHmSuccessor sSuccessors[] =
{
    SUCCESSOR(SPECIES_CORPHISH, SPECIES_CRAWDAUNT, sCorphishMoves),
    SUCCESSOR(SPECIES_ELECTRIKE, SPECIES_MANECTRIC, sElectrikeMoves),
    SUCCESSOR(SPECIES_LOTAD, SPECIES_LOMBRE, sLotadMoves),
    SUCCESSOR(SPECIES_LOTAD, SPECIES_LUDICOLO, sLotadMoves),
    SUCCESSOR(SPECIES_WAILMER, SPECIES_WAILORD, sWailmerMoves),
    SUCCESSOR(SPECIES_MAKUHITA, SPECIES_HARIYAMA, sMakuhitaMoves),
    SUCCESSOR(SPECIES_ARON, SPECIES_LAIRON, sAronMoves),
    SUCCESSOR(SPECIES_ARON, SPECIES_AGGRON, sAronMoves),
    SUCCESSOR(SPECIES_BARBOACH, SPECIES_WHISCASH, sBarboachMoves),
    SUCCESSOR(SPECIES_CARVANHA, SPECIES_SHARPEDO, sCarvanhaMoves),
};

#endif

struct NativeHmEncounterPlace
{
    u16 species;
    u8 minLevel;
    u8 maxLevel;
    u8 mapCount;
    const u16 *maps;
};

#define PLACE(species, minLevel, maxLevel, maps) \
    { species, minLevel, maxLevel, ARRAY_COUNT(maps), maps }

#if IS_FRLG

static const u16 sMtMoonMaps[] = { MAP_MT_MOON_1F, MAP_MT_MOON_B1F, MAP_MT_MOON_B2F };
static const u16 sSafariParasMaps[] = { MAP_SAFARI_ZONE_EAST, MAP_SAFARI_ZONE_NORTH_FRLG };
static const u16 sRoute1Maps[] = { MAP_ROUTE1 };
static const u16 sRoute2Or22Maps[] = { MAP_ROUTE2, MAP_ROUTE22 };
static const u16 sRoute10Maps[] = { MAP_ROUTE10 };
static const u16 sPowerPlantMaps[] = { MAP_POWER_PLANT };
static const u16 sViridianForestMaps[] = { MAP_VIRIDIAN_FOREST };
static const u16 sPalletTownMaps[] = { MAP_PALLET_TOWN };
static const u16 sCinnabarMaps[] = { MAP_CINNABAR_ISLAND };
static const u16 sRockTunnelMaps[] = { MAP_ROCK_TUNNEL_1F, MAP_ROCK_TUNNEL_B1F };
static const u16 sMtEmberMaps[] =
{
    MAP_MT_EMBER_EXTERIOR, MAP_MT_EMBER_SUMMIT_PATH_1F,
    MAP_MT_EMBER_SUMMIT_PATH_2F, MAP_MT_EMBER_SUMMIT_PATH_3F,
    MAP_MT_EMBER_RUBY_PATH_1F,
};
static const u16 sMtMoonGeodudeMaps[] = { MAP_MT_MOON_1F, MAP_MT_MOON_B2F };
static const u16 sRoute22Maps[] = { MAP_ROUTE22 };
static const u16 sRoute3Or4Maps[] = { MAP_ROUTE3, MAP_ROUTE4 };
static const u16 sRoute6Maps[] = { MAP_ROUTE6 };
static const u16 sRoute22Or25Maps[] = { MAP_ROUTE22, MAP_ROUTE25 };

static const struct NativeHmEncounterPlace sEncounterPlaces[] =
{
    PLACE(SPECIES_PARAS, 5, 12, sMtMoonMaps),
    PLACE(SPECIES_PARAS, 22, 23, sSafariParasMaps),
    PLACE(SPECIES_RATTATA, 2, 5, sRoute1Maps),
    PLACE(SPECIES_RATTATA, 2, 5, sRoute2Or22Maps),
    PLACE(SPECIES_VOLTORB, 14, 17, sRoute10Maps),
    PLACE(SPECIES_VOLTORB, 22, 25, sPowerPlantMaps),
    PLACE(SPECIES_PIKACHU, 3, 5, sViridianForestMaps),
    PLACE(SPECIES_PIKACHU, 22, 26, sPowerPlantMaps),
    PLACE(SPECIES_HORSEA, 5, 25, sPalletTownMaps),
    PLACE(SPECIES_HORSEA, 5, 25, sCinnabarMaps),
    PLACE(SPECIES_KRABBY, 5, 25, sPalletTownMaps),
    PLACE(SPECIES_KRABBY, 5, 25, sCinnabarMaps),
    PLACE(SPECIES_MACHOP, 16, 17, sRockTunnelMaps),
    PLACE(SPECIES_MACHOP, 31, 39, sMtEmberMaps),
    PLACE(SPECIES_GEODUDE, 7, 10, sMtMoonGeodudeMaps),
    PLACE(SPECIES_GEODUDE, 15, 17, sRockTunnelMaps),
    PLACE(SPECIES_MANKEY, 2, 5, sRoute22Maps),
    PLACE(SPECIES_MANKEY, 7, 12, sRoute3Or4Maps),
    PLACE(SPECIES_GOLDEEN, 5, 15, sRoute6Maps),
    PLACE(SPECIES_GOLDEEN, 5, 15, sRoute22Or25Maps),
};

#elif IS_HNS

static const u16 sRoute42Maps[] = { MAP_ROUTE42_HNS };
static const u16 sRoute45Maps[] = { MAP_ROUTE45_HNS };
static const u16 sAzaleaMaps[] = { MAP_AZALEA_TOWN_HNS };
static const u16 sRoute33Maps[] = { MAP_ROUTE33_HNS };
static const u16 sOlivinePortMaps[] = { MAP_OLIVINE_CITY_PORT_OUTSIDE_HNS };
static const u16 sCianwoodMaps[] = { MAP_CIANWOOD_CITY_HNS };
static const u16 sVermilionMaps[] = { MAP_VERMILION_CITY_HNS, MAP_VERMILION_CITY_PORT_OUTSIDE_HNS };
static const u16 sCinnabarMaps[] = { MAP_CINNABAR_ISLAND_HNS };
static const u16 sRoute31Maps[] = { MAP_ROUTE31_HNS };
static const u16 sRoute32Maps[] = { MAP_ROUTE32_HNS };
static const u16 sRuinsOfAlphMaps[] = { MAP_RUINS_OF_ALPH_OUTSIDE_HNS };
static const u16 sRoute34Maps[] = { MAP_ROUTE34_HNS };
static const u16 sRoute35Maps[] = { MAP_ROUTE35_HNS };
static const u16 sRoute38Maps[] = { MAP_ROUTE38_HNS };
static const u16 sRoute39Maps[] = { MAP_ROUTE39_HNS };
static const u16 sUnionCaveMaps[] = { MAP_UNION_CAVE_1F_HNS, MAP_UNION_CAVE_B1F_HNS, MAP_UNION_CAVE_B2F_HNS };
static const u16 sWhirlIslandMaps[] = { MAP_WHIRL_ISLANDS_1F_HNS, MAP_WHIRL_ISLANDS_B1F_HNS };
static const u16 sRoute41Maps[] = { MAP_ROUTE41_HNS };

static const struct NativeHmEncounterPlace sEncounterPlaces[] =
{
    PLACE(SPECIES_GLIGAR, 21, 21, sRoute42Maps),
    PLACE(SPECIES_GLIGAR, 31, 31, sRoute45Maps),
    PLACE(SPECIES_AIPOM, 10, 10, sAzaleaMaps),
    PLACE(SPECIES_AIPOM, 10, 10, sRoute33Maps),
    PLACE(SPECIES_CHINCHOU, 20, 40, sOlivinePortMaps),
    PLACE(SPECIES_CHINCHOU, 20, 20, sCianwoodMaps),
    PLACE(SPECIES_CHINCHOU, 40, 44, sVermilionMaps),
    PLACE(SPECIES_CHINCHOU, 40, 44, sCinnabarMaps),
    PLACE(SPECIES_MAREEP, 5, 7, sRoute31Maps),
    PLACE(SPECIES_MAREEP, 5, 7, sRoute32Maps),
    PLACE(SPECIES_WOOPER, 4, 19, sRoute32Maps),
    PLACE(SPECIES_WOOPER, 4, 19, sRuinsOfAlphMaps),
    PLACE(SPECIES_SNUBBULL, 13, 15, sRoute34Maps),
    PLACE(SPECIES_SNUBBULL, 13, 15, sRoute35Maps),
    PLACE(SPECIES_MILTANK, 21, 21, sRoute38Maps),
    PLACE(SPECIES_MILTANK, 21, 21, sRoute39Maps),
    PLACE(SPECIES_MARILL, 8, 9, sUnionCaveMaps),
    PLACE(SPECIES_MARILL, 20, 20, sRoute42Maps),
    PLACE(SPECIES_MANTINE, 15, 24, sWhirlIslandMaps),
    PLACE(SPECIES_MANTINE, 22, 26, sRoute41Maps),
};

#else

static const u16 sPetalburgMaps[] = { MAP_PETALBURG_CITY };
static const u16 sRoute102Or117Maps[] = { MAP_ROUTE102, MAP_ROUTE117 };
static const u16 sGraniteSableyeMaps[] = { MAP_GRANITE_CAVE_B1F, MAP_GRANITE_CAVE_B2F };
static const u16 sCaveOfOriginMaps[] =
{
    MAP_CAVE_OF_ORIGIN_1F, MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP1,
    MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP2, MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP3,
};
static const u16 sRoute110Maps[] = { MAP_ROUTE110 };
static const u16 sRoute118Maps[] = { MAP_ROUTE118 };
static const u16 sRoute102Maps[] = { MAP_ROUTE102 };
static const u16 sRoute114Maps[] = { MAP_ROUTE114 };
static const u16 sLilycoveMaps[] = { MAP_LILYCOVE_CITY };
static const u16 sMossdeepOrPacifidlogMaps[] = { MAP_MOSSDEEP_CITY, MAP_PACIFIDLOG_TOWN };
static const u16 sGraniteMakuhitaMaps[] = { MAP_GRANITE_CAVE_1F, MAP_GRANITE_CAVE_B1F, MAP_GRANITE_CAVE_STEVENS_ROOM };
static const u16 sVictoryRoad1FMaps[] = { MAP_VICTORY_ROAD_1F };
static const u16 sFieryPathMaps[] = { MAP_FIERY_PATH };
static const u16 sMagmaHideoutMaps[] =
{
    MAP_MAGMA_HIDEOUT_1F, MAP_MAGMA_HIDEOUT_2F_1R, MAP_MAGMA_HIDEOUT_2F_2R,
    MAP_MAGMA_HIDEOUT_3F_1R, MAP_MAGMA_HIDEOUT_3F_2R, MAP_MAGMA_HIDEOUT_4F,
    MAP_MAGMA_HIDEOUT_3F_3R, MAP_MAGMA_HIDEOUT_2F_3R,
};
static const u16 sGraniteAronMaps[] = { MAP_GRANITE_CAVE_B1F, MAP_GRANITE_CAVE_B2F, MAP_GRANITE_CAVE_STEVENS_ROOM };
static const u16 sRoute111Or114Maps[] = { MAP_ROUTE111, MAP_ROUTE114 };
static const u16 sRoute120OrMeteorFallsMaps[] =
{
    MAP_ROUTE120, MAP_METEOR_FALLS_1F_1R, MAP_METEOR_FALLS_1F_2R,
    MAP_METEOR_FALLS_B1F_1R, MAP_METEOR_FALLS_B1F_2R,
};
static const u16 sRoute119Maps[] = { MAP_ROUTE119 };

static const struct NativeHmEncounterPlace sEncounterPlaces[] =
{
    PLACE(SPECIES_CORPHISH, 10, 45, sPetalburgMaps),
    PLACE(SPECIES_CORPHISH, 10, 45, sRoute102Or117Maps),
    PLACE(SPECIES_SABLEYE, 9, 12, sGraniteSableyeMaps),
    PLACE(SPECIES_SABLEYE, 30, 34, sCaveOfOriginMaps),
    PLACE(SPECIES_ELECTRIKE, 12, 13, sRoute110Maps),
    PLACE(SPECIES_ELECTRIKE, 24, 26, sRoute118Maps),
    PLACE(SPECIES_LOTAD, 3, 4, sRoute102Maps),
    PLACE(SPECIES_LOTAD, 15, 16, sRoute114Maps),
    PLACE(SPECIES_WAILMER, 10, 45, sLilycoveMaps),
    PLACE(SPECIES_WAILMER, 10, 45, sMossdeepOrPacifidlogMaps),
    PLACE(SPECIES_MAKUHITA, 6, 10, sGraniteMakuhitaMaps),
    PLACE(SPECIES_MAKUHITA, 36, 36, sVictoryRoad1FMaps),
    PLACE(SPECIES_TORKOAL, 14, 16, sFieryPathMaps),
    PLACE(SPECIES_TORKOAL, 28, 30, sMagmaHideoutMaps),
    PLACE(SPECIES_ARON, 7, 12, sGraniteAronMaps),
    PLACE(SPECIES_ARON, 36, 36, sVictoryRoad1FMaps),
    PLACE(SPECIES_BARBOACH, 10, 45, sRoute111Or114Maps),
    PLACE(SPECIES_BARBOACH, 10, 45, sRoute120OrMeteorFallsMaps),
    PLACE(SPECIES_CARVANHA, 10, 45, sRoute118Maps),
    PLACE(SPECIES_CARVANHA, 10, 45, sRoute119Maps),
};

#endif

static void SelectLearnsetMode(bool8 modern)
{
    gSaveBlock3Ptr->challengeSettings.tx_Mode_Modern_Moves = modern;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
}

static bool8 MoveIsInList(u16 move, const u16 *moves, u8 count)
{
    u8 i;

    for (i = 0; i < count; i++)
    {
        if (moves[i] == move)
            return TRUE;
    }
    return FALSE;
}

static bool8 MonKnowsMove(struct Pokemon *mon, u16 move)
{
    u8 i;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (GetMonData(mon, MON_DATA_MOVE1 + i) == move)
            return TRUE;
    }
    return FALSE;
}

static void CreateNativeHmMon(struct Pokemon *mon, u16 species, u8 level)
{
    CreateMon(mon, species, level, 0, OTID_STRUCT_PRESET(0));
    GiveMonInitialMoveset(mon);
}

static void ExpectNativeMoves(struct Pokemon *mon, const u16 *moves, u8 count)
{
    u8 i;

    for (i = 0; i < count; i++)
        EXPECT(MonKnowsMove(mon, moves[i]));
}

static const struct NativeHmAnchor *FindAnchor(u16 species)
{
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sAnchors); i++)
    {
        if (sAnchors[i].species == species)
            return &sAnchors[i];
    }
    return NULL;
}

static bool8 EncounterPlaceContainsMap(const struct NativeHmEncounterPlace *place, u16 map)
{
    u8 i;

    for (i = 0; i < place->mapCount; i++)
    {
        if (place->maps[i] == map)
            return TRUE;
    }
    return FALSE;
}

TEST("Native HM anchors use the exact gated schedules and same-level ordering in both modes")
{
    u8 mode;
    u8 anchorId;

    for (mode = 0; mode < 2; mode++)
    {
        SelectLearnsetMode(mode);
        for (anchorId = 0; anchorId < ARRAY_COUNT(sAnchors); anchorId++)
        {
            const struct NativeHmAnchor *anchor = &sAnchors[anchorId];
            const struct ExpectedLevelMove *expected = mode ? anchor->modern : anchor->legacy;
            const u8 expectedCount = mode ? anchor->modernCount : anchor->legacyCount;
            const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(anchor->species);
            u8 found = 0;
            u8 i;

            for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
            {
                if (MoveIsInList(learnset[i].move, anchor->moves, anchor->moveCount))
                {
                    if (found >= expectedCount)
                    {
                        EXPECT_LT(found, expectedCount);
                        continue;
                    }
                    EXPECT_EQ(learnset[i].level, expected[found].level);
                    EXPECT_EQ(learnset[i].move, expected[found].move);
                    found++;
                }
                else if (i > 0 && learnset[i - 1].level == learnset[i].level
                      && MoveIsInList(learnset[i - 1].move, anchor->moves, anchor->moveCount))
                {
                    EXPECT(FALSE);
                }
            }
            EXPECT_EQ(found, expectedCount);
        }
    }
}

TEST("Native HM anchors retain every assigned move from their encounter floor through level 100")
{
    struct Pokemon mon;
    u8 mode;
    u8 anchorId;

    for (mode = 0; mode < 2; mode++)
    {
        SelectLearnsetMode(mode);
        for (anchorId = 0; anchorId < ARRAY_COUNT(sAnchors); anchorId++)
        {
            const struct NativeHmAnchor *anchor = &sAnchors[anchorId];
            u8 level;

            for (level = anchor->floor; level <= MAX_LEVEL; level++)
            {
                CreateNativeHmMon(&mon, anchor->species, level);
                ExpectNativeMoves(&mon, anchor->moves, anchor->moveCount);
            }
        }
    }
}

TEST("Named native HM encounter profiles retain anchors through production Trainer Rating scaling")
{
    struct Pokemon mon;
    u8 mode = 0;
    u8 placeId = 0;
    u8 parameterMode;
    u8 parameterPlaceId;

    for (parameterMode = 0; parameterMode < 2; parameterMode++)
    {
        for (parameterPlaceId = 0; parameterPlaceId < ARRAY_COUNT(sEncounterPlaces); parameterPlaceId++)
            PARAMETRIZE_LABEL("mode %d, place %d", parameterMode, parameterPlaceId) { mode = parameterMode; placeId = parameterPlaceId; }
    }

    SelectLearnsetMode(mode);
    {
        const struct NativeHmEncounterPlace *place = &sEncounterPlaces[placeId];
        const struct NativeHmAnchor *anchor = FindAnchor(place->species);
        u16 matchingSlots = 0;
        u16 headerId;

        ASSUME(anchor != NULL);
        for (headerId = 0; gWildMonHeaders[headerId].mapGroup != MAP_GROUP(MAP_UNDEFINED); headerId++)
        {
            u16 map = (gWildMonHeaders[headerId].mapGroup << 8) | gWildMonHeaders[headerId].mapNum;
            u8 timeOfDay;

            if (!EncounterPlaceContainsMap(place, map))
                continue;
            for (timeOfDay = 0; timeOfDay < TIMES_OF_DAY_COUNT; timeOfDay++)
            {
                enum WildPokemonArea area;

                for (area = WILD_AREA_LAND; area <= WILD_AREA_FISHING; area++)
                {
                    u8 rodCount = area == WILD_AREA_FISHING ? 3 : 1;
                    u8 rodId;

                    for (rodId = 0; rodId < rodCount; rodId++)
                    {
                        struct WildEncounterProfileContext context =
                        {
                            .headerId = headerId,
                            .timeOfDay = timeOfDay,
                            .area = area,
                            .fishingRod = area == WILD_AREA_FISHING ? rodId : WILD_ENCOUNTER_FISHING_ROD_NONE,
                        };
                        struct WildEncounterProfileView view;
                        u8 slot;

                        if (!GetWildEncounterProfileView(&context, &view))
                            continue;
                        for (slot = view.entryStart; slot < view.entryStart + view.entryCount; slot++)
                        {
                            const struct WildPokemon *entry;
                            u8 authoredLevel;

                            ASSUME(GetWildEncounterProfileEntry(&view, slot, &entry));
                            if (entry->species != place->species)
                                continue;
                            if (entry->minLevel < place->minLevel || entry->maxLevel > place->maxLevel)
                                continue;
#if IS_HNS
                            if (place->species == SPECIES_AIPOM)
                                EXPECT_EQ(area, WILD_AREA_ROCKS);
#endif
                            EXPECT_GE(entry->minLevel, place->minLevel);
                            EXPECT_LE(entry->maxLevel, place->maxLevel);
                            matchingSlots++;
                            for (authoredLevel = entry->minLevel; authoredLevel <= entry->maxLevel; authoredLevel++)
                            {
                                u16 rating;

                                for (rating = 10; rating <= 80; rating++)
                                {
                                    struct WildEncounterSpeciesOutcome outcome;

                                    ASSUME(GetWildEncounterSpeciesOutcome(&view, slot, authoredLevel, rating, FALSE, &outcome));
                                    EXPECT_EQ(outcome.species, place->species);
                                    if (outcome.level < anchor->floor)
                                        continue;
                                    CreateNativeHmMon(&mon, outcome.species, outcome.level);
                                    ExpectNativeMoves(&mon, anchor->moves, anchor->moveCount);
                                }
                            }
                        }
                    }
                }
            }
        }
        EXPECT_GT(matchingSlots, 0);
    }
}

TEST("Native HM successors have exact level-one roles and exact Move Reminder utility results")
{
    struct Pokemon mon;
    u16 reminderMoves[MAX_RELEARNER_MOVES];
    u8 mode;
    u8 successorId;

    for (mode = 0; mode < 2; mode++)
    {
        SelectLearnsetMode(mode);
        for (successorId = 0; successorId < ARRAY_COUNT(sSuccessors); successorId++)
        {
            const struct NativeHmSuccessor *successor = &sSuccessors[successorId];
            const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(successor->species);
            u8 levelOneFound = 0;
            u8 reminderFound = 0;
            u32 reminderCount;
            u32 i;
            enum Move none = MOVE_NONE;

            for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
            {
                if (learnset[i].level == 1
                 && MoveIsInList(learnset[i].move, successor->moves, successor->moveCount))
                {
                    if (levelOneFound >= successor->moveCount)
                    {
                        EXPECT_LT(levelOneFound, successor->moveCount);
                        continue;
                    }
                    EXPECT_EQ(learnset[i].move, successor->moves[levelOneFound]);
                    levelOneFound++;
                }
            }
            EXPECT_EQ(levelOneFound, successor->moveCount);

            CreateMon(&mon, successor->species, MAX_LEVEL, 0, OTID_STRUCT_PRESET(0));
            for (i = 0; i < MAX_MON_MOVES; i++)
                SetMonData(&mon, MON_DATA_MOVE1 + i, &none);
            reminderCount = GetBoxMonRelearnableLevelUpMoves(&mon.box, reminderMoves);
            for (i = 0; i < reminderCount; i++)
            {
                if (MoveIsInList(reminderMoves[i], successor->moves, successor->moveCount))
                {
                    if (reminderFound >= successor->moveCount)
                    {
                        EXPECT_LT(reminderFound, successor->moveCount);
                        continue;
                    }
                    EXPECT_EQ(reminderMoves[i], successor->moves[reminderFound]);
                    reminderFound++;
                }
            }
            EXPECT_EQ(reminderFound, successor->moveCount);

        }
    }
}

TEST("Native HM moves survive the species mutation used by evolution")
{
    struct Pokemon mon;
    u8 mode;
    u8 successorId;

    for (mode = 0; mode < 2; mode++)
    {
        SelectLearnsetMode(mode);
        for (successorId = 0; successorId < ARRAY_COUNT(sSuccessors); successorId++)
        {
            const struct NativeHmSuccessor *successor = &sSuccessors[successorId];
            u32 targetSpecies = successor->species;
            u8 i;

            CreateMon(&mon, successor->anchor, MAX_LEVEL, 0, OTID_STRUCT_PRESET(0));
            for (i = 0; i < successor->moveCount; i++)
            {
                enum Move move = successor->moves[i];

                SetMonData(&mon, MON_DATA_MOVE1 + i, &move);
            }
            SetMonData(&mon, MON_DATA_SPECIES, &targetSpecies);
            CalculateMonStats(&mon);
            EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIES), successor->species);
            ExpectNativeMoves(&mon, successor->moves, successor->moveCount);
        }
    }
}

#if IS_FRLG || IS_HNS
static void ExpectNoLevelUpMove(u16 species, u16 move)
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
    u8 i;

    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
        EXPECT_NE(learnset[i].move, move);
}
#endif

static void ExpectNoLevelUpMoveAtLevel(u16 species, u16 move, u8 level)
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
    u8 i;

    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].move == move && learnset[i].level == level)
            Test_ExitWithResult(TEST_RESULT_FAIL, __LINE__, ":L%s:%d: unexpected regional schedule row (%d, %d, %d)",
                                gTestRunnerState.test->filename, __LINE__, species, move, level);
    }
}

#if !IS_FRLG
static u8 CountLevelUpMoveAtLevel(u16 species, u16 move, u8 level)
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
    u8 count = 0;
    u8 i;

    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].move == move && learnset[i].level == level)
            count++;
    }
    return count;
}
#endif

#define EXPECT_NO_SCHEDULE(species, move, ...)                         \
    do                                                                 \
    {                                                                  \
        const u8 levels[] = { __VA_ARGS__ };                           \
        u8 levelId;                                                    \
                                                                       \
        for (levelId = 0; levelId < ARRAY_COUNT(levels); levelId++)    \
            ExpectNoLevelUpMoveAtLevel(species, move, levels[levelId]); \
    } while (0)

#if !IS_FRLG
static void ExpectNoKantoAdditions(bool8 modern)
{
    if (modern)
    {
        EXPECT_NO_SCHEDULE(SPECIES_PARAS, MOVE_CUT, 5, 17, 38);
        EXPECT_NO_SCHEDULE(SPECIES_RATTATA, MOVE_CUT, 2, 13, 25);
        EXPECT_NO_SCHEDULE(SPECIES_VOLTORB, MOVE_FLASH, 14, 26, 37);
        EXPECT_NO_SCHEDULE(SPECIES_PIKACHU, MOVE_FLASH, 3, 13, 26, 39, 50);
        EXPECT_NO_SCHEDULE(SPECIES_HORSEA, MOVE_SURF, 5, 17, 31, 46);
        EXPECT_NO_SCHEDULE(SPECIES_HORSEA, MOVE_WATERFALL, 5, 17, 31, 46);
        EXPECT_NO_SCHEDULE(SPECIES_KRABBY, MOVE_SURF, 5, 19, 31, 45);
        EXPECT_NO_SCHEDULE(SPECIES_MACHOP, MOVE_STRENGTH, 16, 27, 39);
        EXPECT_NO_SCHEDULE(SPECIES_GEODUDE, MOVE_STRENGTH, 7, 16, 24, 34, 42);
        EXPECT_NO_SCHEDULE(SPECIES_GEODUDE, MOVE_ROCK_SMASH, 7, 16, 24, 34, 42);
        EXPECT_NO_SCHEDULE(SPECIES_MANKEY, MOVE_ROCK_SMASH, 2, 15, 29, 43);
        EXPECT_NO_SCHEDULE(SPECIES_GOLDEEN, MOVE_WATERFALL, 5, 21, 40);
        EXPECT_EQ(CountLevelUpMoveAtLevel(SPECIES_MACHAMP, MOVE_STRENGTH, 1), 1);
    }
    else
    {
        EXPECT_NO_SCHEDULE(SPECIES_PARAS, MOVE_CUT, 5, 25, 49);
        EXPECT_NO_SCHEDULE(SPECIES_RATTATA, MOVE_CUT, 2, 27);
        EXPECT_NO_SCHEDULE(SPECIES_VOLTORB, MOVE_FLASH, 14, 32, 49);
        EXPECT_NO_SCHEDULE(SPECIES_PIKACHU, MOVE_FLASH, 3, 15, 41);
        EXPECT_NO_SCHEDULE(SPECIES_HORSEA, MOVE_SURF, 5, 22, 43);
        EXPECT_NO_SCHEDULE(SPECIES_HORSEA, MOVE_WATERFALL, 5, 22, 43);
        EXPECT_NO_SCHEDULE(SPECIES_KRABBY, MOVE_SURF, 5, 27);
        EXPECT_NO_SCHEDULE(SPECIES_MACHOP, MOVE_STRENGTH, 16, 31, 49);
        EXPECT_NO_SCHEDULE(SPECIES_GEODUDE, MOVE_STRENGTH, 7, 21, 36);
        EXPECT_NO_SCHEDULE(SPECIES_GEODUDE, MOVE_ROCK_SMASH, 7, 21, 36);
        EXPECT_NO_SCHEDULE(SPECIES_MANKEY, MOVE_ROCK_SMASH, 2, 27, 51);
        EXPECT_NO_SCHEDULE(SPECIES_GOLDEEN, MOVE_WATERFALL, 5, 29);
        EXPECT_NO_SCHEDULE(SPECIES_MACHAMP, MOVE_STRENGTH, 1);
    }
    EXPECT_NO_SCHEDULE(SPECIES_PARASECT, MOVE_CUT, 1);
    EXPECT_NO_SCHEDULE(SPECIES_RATICATE, MOVE_CUT, 1);
    EXPECT_NO_SCHEDULE(SPECIES_ELECTRODE, MOVE_FLASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_RAICHU, MOVE_FLASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_RAICHU_ALOLA, MOVE_FLASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_SEADRA, MOVE_SURF, 1);
    EXPECT_NO_SCHEDULE(SPECIES_SEADRA, MOVE_WATERFALL, 1);
    EXPECT_NO_SCHEDULE(SPECIES_KINGDRA, MOVE_SURF, 1);
    EXPECT_NO_SCHEDULE(SPECIES_KINGDRA, MOVE_WATERFALL, 1);
    EXPECT_NO_SCHEDULE(SPECIES_KINGLER, MOVE_SURF, 1);
    EXPECT_NO_SCHEDULE(SPECIES_MACHOKE, MOVE_STRENGTH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_GRAVELER, MOVE_STRENGTH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_GRAVELER, MOVE_ROCK_SMASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_GOLEM, MOVE_STRENGTH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_GOLEM, MOVE_ROCK_SMASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_PRIMEAPE, MOVE_ROCK_SMASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_ANNIHILAPE, MOVE_ROCK_SMASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_SEAKING, MOVE_WATERFALL, 1);
}
#endif

#if !IS_HNS
static void ExpectNoJohtoAdditions(bool8 modern)
{
    if (modern)
    {
        EXPECT_NO_SCHEDULE(SPECIES_GLIGAR, MOVE_CUT, 19, 35, 55);
        EXPECT_NO_SCHEDULE(SPECIES_AIPOM, MOVE_CUT, 10, 18, 29, 39);
        EXPECT_NO_SCHEDULE(SPECIES_AIPOM, MOVE_ROCK_SMASH, 10, 18, 29, 39);
        EXPECT_NO_SCHEDULE(SPECIES_CHINCHOU, MOVE_FLASH, 20, 28, 34, 42, 47);
        EXPECT_NO_SCHEDULE(SPECIES_CHINCHOU, MOVE_SURF, 20, 28, 34, 42, 47);
        EXPECT_NO_SCHEDULE(SPECIES_CHINCHOU, MOVE_WHIRLPOOL, 20, 28, 34, 42, 47);
        EXPECT_NO_SCHEDULE(SPECIES_MAREEP, MOVE_FLASH, 5, 18, 32, 46);
        EXPECT_NO_SCHEDULE(SPECIES_WOOPER, MOVE_SURF, 4, 15, 29, 43);
        EXPECT_NO_SCHEDULE(SPECIES_WOOPER, MOVE_WATERFALL, 4, 15, 29, 43);
        EXPECT_NO_SCHEDULE(SPECIES_SNUBBULL, MOVE_STRENGTH, 13, 37);
        EXPECT_NO_SCHEDULE(SPECIES_MILTANK, MOVE_STRENGTH, 21, 35, 50);
        EXPECT_NO_SCHEDULE(SPECIES_MILTANK, MOVE_ROCK_SMASH, 21, 35, 50);
        EXPECT_NO_SCHEDULE(SPECIES_MARILL, MOVE_WATERFALL, 8, 16, 31);
        EXPECT_NO_SCHEDULE(SPECIES_MANTINE, MOVE_WHIRLPOOL, 15, 27, 46);
    }
    else
    {
        EXPECT_NO_SCHEDULE(SPECIES_GLIGAR, MOVE_CUT, 19, 44);
        EXPECT_NO_SCHEDULE(SPECIES_AIPOM, MOVE_CUT, 10, 25, 38);
        EXPECT_NO_SCHEDULE(SPECIES_AIPOM, MOVE_ROCK_SMASH, 10, 25, 38);
        EXPECT_NO_SCHEDULE(SPECIES_CHINCHOU, MOVE_FLASH, 20, 29, 41);
        EXPECT_NO_SCHEDULE(SPECIES_CHINCHOU, MOVE_SURF, 20, 29, 41);
        EXPECT_NO_SCHEDULE(SPECIES_CHINCHOU, MOVE_WHIRLPOOL, 20, 29, 41);
        EXPECT_NO_SCHEDULE(SPECIES_MAREEP, MOVE_FLASH, 5, 30);
        EXPECT_NO_SCHEDULE(SPECIES_WOOPER, MOVE_SURF, 4, 21, 41);
        EXPECT_NO_SCHEDULE(SPECIES_WOOPER, MOVE_WATERFALL, 4, 21, 41);
        EXPECT_NO_SCHEDULE(SPECIES_SNUBBULL, MOVE_STRENGTH, 13, 43);
        EXPECT_NO_SCHEDULE(SPECIES_MILTANK, MOVE_STRENGTH, 21, 43);
        EXPECT_NO_SCHEDULE(SPECIES_MILTANK, MOVE_ROCK_SMASH, 21, 43);
        EXPECT_NO_SCHEDULE(SPECIES_MARILL, MOVE_WATERFALL, 8, 28);
        EXPECT_NO_SCHEDULE(SPECIES_MANTINE, MOVE_WHIRLPOOL, 15, 43);
    }
    EXPECT_NO_SCHEDULE(SPECIES_GLISCOR, MOVE_CUT, 1);
    EXPECT_NO_SCHEDULE(SPECIES_AMBIPOM, MOVE_CUT, 1);
    EXPECT_NO_SCHEDULE(SPECIES_AMBIPOM, MOVE_ROCK_SMASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_LANTURN, MOVE_FLASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_LANTURN, MOVE_SURF, 1);
    EXPECT_NO_SCHEDULE(SPECIES_LANTURN, MOVE_WHIRLPOOL, 1);
    EXPECT_NO_SCHEDULE(SPECIES_FLAAFFY, MOVE_FLASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_AMPHAROS, MOVE_FLASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_QUAGSIRE, MOVE_SURF, 1);
    EXPECT_NO_SCHEDULE(SPECIES_QUAGSIRE, MOVE_WATERFALL, 1);
    EXPECT_NO_SCHEDULE(SPECIES_GRANBULL, MOVE_STRENGTH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_AZUMARILL, MOVE_WATERFALL, 1);
}
#endif

#if IS_FRLG || IS_HNS
static void ExpectNoHoennAdditions(bool8 modern)
{
    if (modern)
    {
        EXPECT_NO_SCHEDULE(SPECIES_CORPHISH, MOVE_CUT, 10, 20, 31, 39);
        EXPECT_NO_SCHEDULE(SPECIES_CORPHISH, MOVE_ROCK_SMASH, 10, 20, 31, 39);
        EXPECT_NO_SCHEDULE(SPECIES_SABLEYE, MOVE_CUT, 9, 16, 24, 31, 39, 46);
        EXPECT_NO_SCHEDULE(SPECIES_SABLEYE, MOVE_FLASH, 9, 16, 24, 31, 39, 46);
        EXPECT_NO_SCHEDULE(SPECIES_ELECTRIKE, MOVE_FLASH, 12, 24, 44);
        EXPECT_NO_SCHEDULE(SPECIES_LOTAD, MOVE_SURF, 3, 15, 27);
        EXPECT_NO_SCHEDULE(SPECIES_WAILMER, MOVE_SURF, 10, 19, 29, 45);
        EXPECT_NO_SCHEDULE(SPECIES_WAILMER, MOVE_DIVE, 10, 19, 29, 45);
        EXPECT_NO_SCHEDULE(SPECIES_MAKUHITA, MOVE_STRENGTH, 6, 16, 28, 40);
        EXPECT_NO_SCHEDULE(SPECIES_TORKOAL, MOVE_STRENGTH, 14, 25, 38, 47);
        EXPECT_NO_SCHEDULE(SPECIES_ARON, MOVE_ROCK_SMASH, 7, 19, 31, 43);
        EXPECT_NO_SCHEDULE(SPECIES_BARBOACH, MOVE_WATERFALL, 10, 20, 32);
        EXPECT_NO_SCHEDULE(SPECIES_CARVANHA, MOVE_WATERFALL, 10, 18, 29, 39);
        EXPECT_NO_SCHEDULE(SPECIES_CARVANHA, MOVE_DIVE, 10, 18, 29, 39);
    }
    else
    {
        EXPECT_NO_SCHEDULE(SPECIES_CORPHISH, MOVE_CUT, 10, 23, 35);
        EXPECT_NO_SCHEDULE(SPECIES_CORPHISH, MOVE_ROCK_SMASH, 10, 23, 35);
        EXPECT_NO_SCHEDULE(SPECIES_SABLEYE, MOVE_CUT, 9, 21, 33, 45);
        EXPECT_NO_SCHEDULE(SPECIES_SABLEYE, MOVE_FLASH, 9, 21, 33, 45);
        EXPECT_NO_SCHEDULE(SPECIES_ELECTRIKE, MOVE_FLASH, 12, 28);
        EXPECT_NO_SCHEDULE(SPECIES_LOTAD, MOVE_SURF, 3, 31);
        EXPECT_NO_SCHEDULE(SPECIES_WAILMER, MOVE_SURF, 10, 23, 37, 50);
        EXPECT_NO_SCHEDULE(SPECIES_WAILMER, MOVE_DIVE, 10, 23, 37, 50);
        EXPECT_NO_SCHEDULE(SPECIES_MAKUHITA, MOVE_STRENGTH, 6, 22, 40);
        EXPECT_NO_SCHEDULE(SPECIES_TORKOAL, MOVE_STRENGTH, 14, 30, 46);
        EXPECT_NO_SCHEDULE(SPECIES_ARON, MOVE_ROCK_SMASH, 7, 21, 39);
        EXPECT_NO_SCHEDULE(SPECIES_BARBOACH, MOVE_WATERFALL, 10, 26);
        EXPECT_NO_SCHEDULE(SPECIES_CARVANHA, MOVE_WATERFALL, 10, 22, 37);
        EXPECT_NO_SCHEDULE(SPECIES_CARVANHA, MOVE_DIVE, 10, 22, 37);
    }
    EXPECT_NO_SCHEDULE(SPECIES_CRAWDAUNT, MOVE_CUT, 1);
    EXPECT_NO_SCHEDULE(SPECIES_CRAWDAUNT, MOVE_ROCK_SMASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_MANECTRIC, MOVE_FLASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_LOMBRE, MOVE_SURF, 1);
    EXPECT_NO_SCHEDULE(SPECIES_LUDICOLO, MOVE_SURF, 1);
    EXPECT_NO_SCHEDULE(SPECIES_WAILORD, MOVE_SURF, 1);
    EXPECT_NO_SCHEDULE(SPECIES_WAILORD, MOVE_DIVE, 1);
    EXPECT_NO_SCHEDULE(SPECIES_HARIYAMA, MOVE_STRENGTH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_LAIRON, MOVE_ROCK_SMASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_AGGRON, MOVE_ROCK_SMASH, 1);
    EXPECT_NO_SCHEDULE(SPECIES_WHISCASH, MOVE_WATERFALL, 1);
    EXPECT_NO_SCHEDULE(SPECIES_SHARPEDO, MOVE_WATERFALL, 1);
    EXPECT_NO_SCHEDULE(SPECIES_SHARPEDO, MOVE_DIVE, 1);
}
#endif

TEST("Native HM exclusions and regional build gates remain exact")
{
    u8 mode;

    for (mode = 0; mode < 2; mode++)
    {
        SelectLearnsetMode(mode);
#if IS_FRLG
        ExpectNoLevelUpMove(SPECIES_PICHU, MOVE_FLASH);
        ExpectNoLevelUpMove(SPECIES_RATTATA_ALOLA, MOVE_CUT);
        ExpectNoLevelUpMove(SPECIES_VOLTORB_HISUI, MOVE_FLASH);
        ExpectNoLevelUpMove(SPECIES_GEODUDE_ALOLA, MOVE_STRENGTH);
        ExpectNoLevelUpMove(SPECIES_GEODUDE_ALOLA, MOVE_ROCK_SMASH);
        ExpectNoJohtoAdditions(mode);
        ExpectNoHoennAdditions(mode);
#elif IS_HNS
        ExpectNoLevelUpMove(SPECIES_AZURILL, MOVE_WATERFALL);
        ExpectNoLevelUpMove(SPECIES_MANTYKE, MOVE_WHIRLPOOL);
        ExpectNoLevelUpMove(SPECIES_WOOPER_PALDEA, MOVE_SURF);
        ExpectNoLevelUpMove(SPECIES_WOOPER_PALDEA, MOVE_WATERFALL);
        ExpectNoKantoAdditions(mode);
        ExpectNoHoennAdditions(mode);
#else
        ExpectNoKantoAdditions(mode);
        ExpectNoJohtoAdditions(mode);
#endif
    }
}

TEST("Native HM anchors retain generated HM compatibility and successor exceptions")
{
    u8 anchorId;

    SelectLearnsetMode(TRUE);
    for (anchorId = 0; anchorId < ARRAY_COUNT(sAnchors); anchorId++)
    {
        const struct NativeHmAnchor *anchor = &sAnchors[anchorId];
        u8 moveId;

        for (moveId = 0; moveId < anchor->moveCount; moveId++)
            EXPECT(CanLearnTeachableMove(anchor->species, anchor->moves[moveId]));
    }
#if IS_FRLG
    EXPECT(!CanLearnTeachableMove(SPECIES_RAICHU_ALOLA, MOVE_FLASH));
    EXPECT(!CanLearnTeachableMove(SPECIES_ANNIHILAPE, MOVE_ROCK_SMASH));
#endif
}

TEST("All species stay below both learnset limits in modern and legacy modes")
{
    u8 mode;

    for (mode = 0; mode < 2; mode++)
    {
        u16 species;

        SelectLearnsetMode(mode);
        for (species = 1; species < SPECIES_EGG; species++)
        {
            const struct LevelUpMove *learnset;
            u8 count = 0;

            if (!IsSpeciesEnabled(species))
                continue;
            learnset = GetSpeciesLevelUpLearnset(species);
            while (learnset[count].move != LEVEL_UP_MOVE_END)
                count++;
            EXPECT_LT(count, MAX_LEVEL_UP_MOVES);
            EXPECT_LT(count, MAX_RELEARNER_MOVES);
        }
    }
}

// The S.S. ferries' cabin hosts (Wayfarer only). Spec: .product/specs/ss-ferries.md, Cabins.
//
// One row per host: the ship whose pool it is in, the cabin room and the host spot object authored in
// that room (a LOCALID_FERRY_HOST_* object running Ferry_EventScript_Host), the request kind with its
// list of parameters, whether the cabin has a bed event, and the host's two lines. Hosts of one ship are
// contiguous. Boarding draws two distinct rows of the ship's pool and one parameter index for each.

#define HOST_MAP(map) MAP_GROUP(map), MAP_NUM(map)

struct FerryHost
{
    u8 ship;
    u8 mapGroup;
    u8 mapNum;
    u8 localId;
    u16 gfx;
    u8 kind;
    bool8 hasBed;           // the cabin's bed event (the Tidal's) belongs to this host
    u8 paramCount;
    const u16 *params;
    const u8 *line;         // introduces the host, before the request
    const u8 *chat;         // small talk once the request is settled
};

// ---- Parameters ----

static const u16 sParamsNone[] = {0};
// A battle borrows an existing trainer's party (the "party template").
static const u16 sParamsGentleman[] = {TRAINER_EDWARD};
static const u16 sParamsCouple[] = {TRAINER_MEL_AND_PAUL};
static const u16 sParamsAllTypes[] =
{
    TYPE_NORMAL, TYPE_FIGHTING, TYPE_FLYING, TYPE_POISON, TYPE_GROUND, TYPE_ROCK, TYPE_BUG, TYPE_GHOST, TYPE_STEEL,
    TYPE_FIRE, TYPE_WATER, TYPE_GRASS, TYPE_ELECTRIC, TYPE_PSYCHIC, TYPE_ICE, TYPE_DRAGON, TYPE_DARK, TYPE_FAIRY,
};
static const u16 sParamsFamilyTypes[] = {TYPE_WATER, TYPE_GRASS, TYPE_FIRE, TYPE_ELECTRIC, TYPE_NORMAL};
static const u16 sParamsColours[] = {BODY_COLOR_RED, BODY_COLOR_BLUE, BODY_COLOR_YELLOW, BODY_COLOR_GREEN, BODY_COLOR_BROWN,
                                     BODY_COLOR_PURPLE, BODY_COLOR_PINK, BODY_COLOR_WHITE};
static const u16 sParamsFishermanA[] = {SPECIES_MAGIKARP, SPECIES_WAILMER, SPECIES_TENTACOOL, SPECIES_GOLDEEN};
static const u16 sParamsFishermanB[] = {SPECIES_LUVDISC, SPECIES_HORSEA, SPECIES_STARYU, SPECIES_REMORAID};
static const u16 sParamsFieldMoves[] = {MOVE_STRENGTH, MOVE_FLASH, MOVE_CUT, MOVE_ROCK_SMASH};
static const u16 sParamsCutRockSmash[] = {MOVE_CUT, MOVE_ROCK_SMASH};
static const u16 sParamsBerries[] = {ITEM_ORAN_BERRY, ITEM_PECHA_BERRY, ITEM_CHERI_BERRY};
static const u16 sParamsMedicine[] = {ITEM_ANTIDOTE, ITEM_FULL_HEAL};
static const u16 sParamsNuggetMilk[] = {ITEM_NUGGET, ITEM_MOOMOO_MILK};
static const u16 sParamsMilk[] = {ITEM_MOOMOO_MILK};
static const u16 sParamsFlute[] = {ITEM_POKE_FLUTE};
static const u16 sParamsQuiz[] = {0, 1, 2, 3, 4, 5, 6};

#define PARAMS(list) ARRAY_COUNT(list), list

// ---- The hosts ----

static const struct FerryHost sHosts[] =
{
    // The Aqua: sailors, merchants, a gambler.
    {
        FERRY_SHIP_AQUA, HOST_MAP(MAP_SSAQUA_ROOM_NE_HNS), LOCALID_FERRY_HOST_AQUA_NE, OBJ_EVENT_GFX_SAILOR_HNS,
        FERRY_REQ_MOVE, FALSE, PARAMS(sParamsFieldMoves),
        COMPOUND_STRING("Ahoy! I'm a deckhand off duty.\n"
          "I've cut and smashed my share."),
        COMPOUND_STRING("Fair winds, matey!"),
    },
    {
        FERRY_SHIP_AQUA, HOST_MAP(MAP_SSAQUA_ROOM_NNE_HNS), LOCALID_FERRY_HOST_AQUA_NNE, OBJ_EVENT_GFX_OLD_MAN_2_HNS,
        FERRY_REQ_ITEM, FALSE, PARAMS(sParamsBerries),
        COMPOUND_STRING("I'm a berry merchant on my way\n"
          "to market. Business is business."),
        COMPOUND_STRING("A fine trip for trade, this."),
    },
    {
        FERRY_SHIP_AQUA, HOST_MAP(MAP_SSAQUA_ROOM_NW_HNS), LOCALID_FERRY_HOST_AQUA_NW, OBJ_EVENT_GFX_BURGLAR_HNS,
        FERRY_REQ_COIN, FALSE, PARAMS(sParamsNone),
        COMPOUND_STRING("Heh heh. I'm a gambler, kid.\n"
          "I live on luck."),
        COMPOUND_STRING("Luck comes and goes, like the tide."),
    },
    {
        FERRY_SHIP_AQUA, HOST_MAP(MAP_SSAQUA_ROOM_SE_HNS), LOCALID_FERRY_HOST_AQUA_SE, OBJ_EVENT_GFX_SAILOR_HNS,
        FERRY_REQ_MOVE, FALSE, PARAMS(sParamsCutRockSmash),
        COMPOUND_STRING("I'm a sailor who knows his knots\n"
          "and his POKéMON's moves."),
        COMPOUND_STRING("Smooth seas ahead, they say."),
    },
    {
        FERRY_SHIP_AQUA, HOST_MAP(MAP_SSAQUA_ROOM_SSE_HNS), LOCALID_FERRY_HOST_AQUA_SSE, OBJ_EVENT_GFX_FAT_MAN_HNS,
        FERRY_REQ_ITEM, FALSE, PARAMS(sParamsMedicine),
        COMPOUND_STRING("I sell remedies for the road.\n"
          "I'm out of stock myself, ironically."),
        COMPOUND_STRING("Mind your health at sea."),
    },
    {
        FERRY_SHIP_AQUA, HOST_MAP(MAP_SSAQUA_ROOM_SSW_HNS), LOCALID_FERRY_HOST_AQUA_SSW, OBJ_EVENT_GFX_OLD_MAN_HNS,
        FERRY_REQ_QUIZ, FALSE, PARAMS(sParamsQuiz),
        COMPOUND_STRING("I've ridden this ship since before\n"
          "you were born. I know her timetable."),
        COMPOUND_STRING("Ah, the sea air keeps a man sharp."),
    },
    {
        FERRY_SHIP_AQUA, HOST_MAP(MAP_SSAQUA_ROOM_SW_HNS), LOCALID_FERRY_HOST_AQUA_SW, OBJ_EVENT_GFX_WOMAN_2_HNS,
        FERRY_REQ_KEY_ITEM, FALSE, PARAMS(sParamsFlute),
        COMPOUND_STRING("I'm a traveling musician.\n"
          "I'm collecting tunes from every port."),
        COMPOUND_STRING("Music carries across the water."),
    },

    // The Anne: tourists, well-off guests, an Alolan tourist, a gentleman.
    {
        FERRY_SHIP_ANNE, HOST_MAP(MAP_SSANNE_1F_ROOM1), LOCALID_FERRY_HOST_ANNE_1F_ROOM1, OBJ_EVENT_GFX_GENTLEMAN_FRLG,
        FERRY_REQ_BATTLE, FALSE, PARAMS(sParamsGentleman),
        COMPOUND_STRING("Good day. I'm a gentleman of leisure,\n"
          "and I do enjoy a proper contest."),
        COMPOUND_STRING("A civilized voyage, wouldn't you say?"),
    },
    {
        FERRY_SHIP_ANNE, HOST_MAP(MAP_SSANNE_1F_ROOM3), LOCALID_FERRY_HOST_ANNE_1F_ROOM3, OBJ_EVENT_GFX_FAT_MAN_FRLG,
        FERRY_REQ_ITEM, FALSE, PARAMS(sParamsNuggetMilk),
        COMPOUND_STRING("I'm a collector of fine things.\n"
          "My cabin is rather bare today."),
        COMPOUND_STRING("The finest things are the rarest."),
    },
    {
        FERRY_SHIP_ANNE, HOST_MAP(MAP_SSANNE_1F_ROOM4), LOCALID_FERRY_HOST_ANNE_1F_ROOM4, OBJ_EVENT_GFX_BEAUTY_FRLG,
        FERRY_REQ_REGION, FALSE, PARAMS(sParamsNone),
        COMPOUND_STRING("Alola! I'm a tourist from the\n"
          "Alolan islands, seeing the world."),
        COMPOUND_STRING("Everyone here has such lovely\nPOKéMON!"),
    },
    {
        FERRY_SHIP_ANNE, HOST_MAP(MAP_SSANNE_1F_ROOM6), LOCALID_FERRY_HOST_ANNE_1F_ROOM6, OBJ_EVENT_GFX_WOMAN_2_FRLG,
        FERRY_REQ_TYPE, FALSE, PARAMS(sParamsAllTypes),
        COMPOUND_STRING("I'm touring the ports with my\n"
          "sketchbook. I draw POKéMON."),
        COMPOUND_STRING("So many shapes and colors out here!"),
    },
    {
        FERRY_SHIP_ANNE, HOST_MAP(MAP_SSANNE_2F_ROOM1), LOCALID_FERRY_HOST_ANNE_2F_ROOM1, OBJ_EVENT_GFX_CAMPER_FRLG,
        FERRY_REQ_COLOUR, FALSE, PARAMS(sParamsColours),
        COMPOUND_STRING("I'm a tourist with a camera.\n"
          "I'm after a colorful photo."),
        COMPOUND_STRING("Say cheese! Oh, that was the sea."),
    },
    {
        FERRY_SHIP_ANNE, HOST_MAP(MAP_SSANNE_2F_ROOM3), LOCALID_FERRY_HOST_ANNE_2F_ROOM3, OBJ_EVENT_GFX_WOMAN_3_FRLG,
        FERRY_REQ_FRIENDSHIP, FALSE, PARAMS(sParamsNone),
        COMPOUND_STRING("I'm a lady of means. I judge a\n"
          "trainer by how their POKéMON feel."),
        COMPOUND_STRING("Do give your partners my regards."),
    },
    {
        FERRY_SHIP_ANNE, HOST_MAP(MAP_SSANNE_2F_ROOM5), LOCALID_FERRY_HOST_ANNE_2F_ROOM5, OBJ_EVENT_GFX_LITTLE_BOY_FRLG,
        FERRY_REQ_EVOLVED, FALSE, PARAMS(sParamsNone),
        COMPOUND_STRING("My family owns half the docks.\n"
          "I only care about the very best."),
        COMPOUND_STRING("Father says I'm spoiled. Hmph."),
    },
    {
        FERRY_SHIP_ANNE, HOST_MAP(MAP_SSANNE_2F_ROOM6), LOCALID_FERRY_HOST_ANNE_2F_ROOM6, OBJ_EVENT_GFX_OLD_WOMAN_FRLG,
        FERRY_REQ_QUIZ, FALSE, PARAMS(sParamsQuiz),
        COMPOUND_STRING("I sail the Anne every season,\n"
          "dear. I know her schedule by heart."),
        COMPOUND_STRING("The timetable never changes."),
    },

    // The Tidal: families, fishermen, a breeder, honeymooners. All eight cabins are rooms of SSTidalRooms.
    {
        FERRY_SHIP_TIDAL, HOST_MAP(MAP_SS_TIDAL_ROOMS), LOCALID_FERRY_HOST_TIDAL_A, OBJ_EVENT_GFX_POKEFAN_M,
        FERRY_REQ_COLOUR, FALSE, PARAMS(sParamsColours),
        COMPOUND_STRING("We're a family on holiday.\n"
          "The kids love colorful POKéMON."),
        COMPOUND_STRING("The kids are asleep. Shh!"),
    },
    {
        FERRY_SHIP_TIDAL, HOST_MAP(MAP_SS_TIDAL_ROOMS), LOCALID_FERRY_HOST_TIDAL_B, OBJ_EVENT_GFX_FISHERMAN,
        FERRY_REQ_SPECIES, FALSE, PARAMS(sParamsFishermanA),
        COMPOUND_STRING("I'm a fisherman. I've fished every\n"
          "sea from here to the horizon."),
        COMPOUND_STRING("The fish are biting, I hear."),
    },
    {
        FERRY_SHIP_TIDAL, HOST_MAP(MAP_SS_TIDAL_ROOMS), LOCALID_FERRY_HOST_TIDAL_C, OBJ_EVENT_GFX_POKEFAN_F,
        FERRY_REQ_TYPE, FALSE, PARAMS(sParamsFamilyTypes),
        COMPOUND_STRING("My family's traveling to LILYCOVE.\n"
          "My little one has a favorite kind."),
        COMPOUND_STRING("Look at the waves, sweetie!"),
    },
    {
        FERRY_SHIP_TIDAL, HOST_MAP(MAP_SS_TIDAL_ROOMS), LOCALID_FERRY_HOST_TIDAL_D, OBJ_EVENT_GFX_WOMAN_5,
        FERRY_REQ_DOUBLE_BATTLE, FALSE, PARAMS(sParamsCouple),
        COMPOUND_STRING("We're honeymooners! We do everything\n"
          "together, even battle."),
        COMPOUND_STRING("Isn't the sea romantic?"),
    },
    {
        FERRY_SHIP_TIDAL, HOST_MAP(MAP_SS_TIDAL_ROOMS), LOCALID_FERRY_HOST_TIDAL_E, OBJ_EVENT_GFX_FISHERMAN,
        FERRY_REQ_SPECIES, FALSE, PARAMS(sParamsFishermanB),
        COMPOUND_STRING("I'm a fisherman, and I keep a life\n"
          "list of the sea's little wonders."),
        COMPOUND_STRING("Something's biting out there."),
    },
    {
        FERRY_SHIP_TIDAL, HOST_MAP(MAP_SS_TIDAL_ROOMS), LOCALID_FERRY_HOST_TIDAL_F, OBJ_EVENT_GFX_WOMAN_2,
        FERRY_REQ_FRIENDSHIP, TRUE, PARAMS(sParamsNone),
        COMPOUND_STRING("I'm a POKéMON breeder. I can tell\n"
          "at a glance how fond they are of you."),
        COMPOUND_STRING("Raise them with love."),
    },
    {
        FERRY_SHIP_TIDAL, HOST_MAP(MAP_SS_TIDAL_ROOMS), LOCALID_FERRY_HOST_TIDAL_G, OBJ_EVENT_GFX_MAN_3,
        FERRY_REQ_EVOLVED, FALSE, PARAMS(sParamsNone),
        COMPOUND_STRING("I'm a trainer on a pilgrimage.\n"
          "I admire POKéMON that grew up."),
        COMPOUND_STRING("Growth takes time."),
    },
    {
        FERRY_SHIP_TIDAL, HOST_MAP(MAP_SS_TIDAL_ROOMS), LOCALID_FERRY_HOST_TIDAL_H, OBJ_EVENT_GFX_OLD_WOMAN,
        FERRY_REQ_ITEM, FALSE, PARAMS(sParamsMilk),
        COMPOUND_STRING("I'm a grandmother heading home.\n"
          "My grandchildren love a treat."),
        COMPOUND_STRING("The little ones will be so happy."),
    },
};

// ---- Request texts ----
// {STR_VAR_1} is the host's parameter (a type, species, colour, move or item name).

static const u8 *const sRequestText[] =
{
    [FERRY_REQ_BATTLE] = COMPOUND_STRING("\pWin a battle against me, and my\n"
                           "cabin is yours for the trip!"),
    [FERRY_REQ_DOUBLE_BATTLE] = COMPOUND_STRING("\pBeat us in a double battle, and\n"
                                  "our cabin is yours for the trip!"),
    [FERRY_REQ_ITEM] = COMPOUND_STRING("\pBring me one {STR_VAR_1}, and my\n"
                         "cabin is yours for the trip!"),
    [FERRY_REQ_TYPE] = COMPOUND_STRING("\pShow me a {STR_VAR_1}-type POKéMON,\n"
                         "and my cabin is yours for the trip!"),
    [FERRY_REQ_SPECIES] = COMPOUND_STRING("\pShow me a {STR_VAR_1}, and my\n"
                            "cabin is yours for the trip!"),
    [FERRY_REQ_COLOUR] = COMPOUND_STRING("\pShow me a {STR_VAR_1} POKéMON,\n"
                           "and my cabin is yours for the trip!"),
    [FERRY_REQ_MOVE] = COMPOUND_STRING("\pShow me a POKéMON that knows\n"
                         "{STR_VAR_1}, and my cabin is yours!"),
    [FERRY_REQ_EVOLVED] = COMPOUND_STRING("\pShow me a POKéMON that has fully\n"
                            "evolved, and my cabin is yours!"),
    [FERRY_REQ_REGION] = COMPOUND_STRING("\pShow me a POKéMON from your home\n"
                           "region, and my cabin is yours!"),
    [FERRY_REQ_FRIENDSHIP] = COMPOUND_STRING("\pShow me a POKéMON that adores you,\n"
                               "and my cabin is yours!"),
    [FERRY_REQ_KEY_ITEM] = COMPOUND_STRING("\pShow me your {STR_VAR_1}, and my\n"
                             "cabin is yours for the trip!"),
    [FERRY_REQ_QUIZ] = COMPOUND_STRING("\pAnswer my question about the ferries,\n"
                         "and my cabin is yours!"),
    [FERRY_REQ_COIN] = COMPOUND_STRING("\pCall it in the air! Win the coin\n"
                         "toss and my cabin is yours!"),
};

// ---- The quiz ----

struct FerryQuiz
{
    const u8 *question;
    const u8 *choices[3];
    u8 answer;
};

static const struct FerryQuiz sQuiz[] =
{
    {
        COMPOUND_STRING("Which ship sails from OLIVINE CITY\nby day?"),
        {COMPOUND_STRING("S.S. TIDAL"), COMPOUND_STRING("S.S. AQUA"), COMPOUND_STRING("S.S. ANNE")}, 1,
    },
    {
        COMPOUND_STRING("How long does a ship stay in port?"),
        {COMPOUND_STRING("Half an hour"), COMPOUND_STRING("Two hours"), COMPOUND_STRING("Six hours")}, 0,
    },
    {
        COMPOUND_STRING("How long is a leg at sea?"),
        {COMPOUND_STRING("Half an hour"), COMPOUND_STRING("Six hours"), COMPOUND_STRING("A whole day")}, 1,
    },
    {
        COMPOUND_STRING("After VERMILION CITY, where does\nthe S.S. ANNE sail next?"),
        {COMPOUND_STRING("ONE ISLAND"), COMPOUND_STRING("LILYCOVE CITY"), COMPOUND_STRING("OLIVINE CITY")}, 0,
    },
    {
        COMPOUND_STRING("Which ship sails to the Alolan\nislands?"),
        {COMPOUND_STRING("S.S. AQUA"), COMPOUND_STRING("S.S. TIDAL"), COMPOUND_STRING("S.S. ANNE")}, 2,
    },
    {
        COMPOUND_STRING("Which port does the S.S. TIDAL\ncall at?"),
        {COMPOUND_STRING("MELEMELE ISLAND"), COMPOUND_STRING("VERMILION CITY"), COMPOUND_STRING("SEVEN ISLAND")}, 2,
    },
    {
        COMPOUND_STRING("Is the S.S. TICKET used up when\nyou board?"),
        {COMPOUND_STRING("Yes, always"), COMPOUND_STRING("No, never"), COMPOUND_STRING("Only on the AQUA")}, 1,
    },
};

static const u8 *const sColourNames[] =
{
    [BODY_COLOR_RED] = COMPOUND_STRING("red"),
    [BODY_COLOR_BLUE] = COMPOUND_STRING("blue"),
    [BODY_COLOR_YELLOW] = COMPOUND_STRING("yellow"),
    [BODY_COLOR_GREEN] = COMPOUND_STRING("green"),
    [BODY_COLOR_BLACK] = COMPOUND_STRING("black"),
    [BODY_COLOR_BROWN] = COMPOUND_STRING("brown"),
    [BODY_COLOR_PURPLE] = COMPOUND_STRING("purple"),
    [BODY_COLOR_GRAY] = COMPOUND_STRING("gray"),
    [BODY_COLOR_WHITE] = COMPOUND_STRING("white"),
    [BODY_COLOR_PINK] = COMPOUND_STRING("pink"),
};

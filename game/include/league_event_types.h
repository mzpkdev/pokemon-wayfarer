#ifndef GUARD_LEAGUE_EVENT_TYPES_H
#define GUARD_LEAGUE_EVENT_TYPES_H

#define LEAGUE_EVENT_LINEUP_SIZE 5
#define LEAGUE_EVENT_CONTENT_VERSION_COUNT 7
#define LEAGUE_EVENT_NOTABLE_COUNT 38
#define LEAGUE_EVENT_PLAYER_CHAMPION 0xFFFF
#define LEAGUE_ACCEPT_OPTION_LEVEL_CAP (1 << 0)
#define LEAGUE_ACCEPT_OPTION_EQUALIZER_SHIFT 1
#define LEAGUE_ACCEPT_OPTION_EQUALIZER_MASK (3 << LEAGUE_ACCEPT_OPTION_EQUALIZER_SHIFT)
#define LEAGUE_ACCEPT_OPTION_RANDOMIZED_PARTY (1 << 3)
#define LEAGUE_ACCEPT_OPTION_RANDOM_BASE_STATS (1 << 4)
#define LEAGUE_ACCEPT_OPTION_RANDOM_MON_TYPES (1 << 5)
#define LEAGUE_ACCEPT_OPTION_LEGENDARY_ABILITIES (1 << 6)
#define LEAGUE_SAVED_MON_GENDER_MASK 0x03
#define LEAGUE_SAVED_MON_SHINY (1 << 2)
#define LEAGUE_SAVED_MON_GIGANTAMAX (1 << 3)
#define LEAGUE_SAVED_MON_CAN_DYNAMAX (1 << 4)

// Saved battle values, independent of TrainerMon's ROM pointers.
struct LeagueSavedMon
{
    u32 personality;
    u32 otId;
    u32 iv;
    u16 species;
    u16 heldItem;
    u16 moves[4];
    u16 ability;
    u8 ev[6];
    u8 level;
    u8 rosterSlot; // stable source identity, independent of members[] index
    u8 ball;
    u8 friendship;
    u8 nature;
    u8 flags; // gender, shiny, Gigantamax, Dynamax eligibility
    u8 teraType;
    u8 dynamaxLevel;
    u8 abilityNum;
    u8 initialLevel; // before the optional level cap adjusts battle level
    u8 types[2]; // final battle types at acceptance
};

struct LeagueSavedTeam
{
    u16 characterId;
    u16 sourceTrainerId;
    u32 trainerTR;
    u64 aiFlags;
    u8 teamSize;
    u8 battleOrder[6];
    u8 aceCount;
    struct LeagueSavedMon members[6]; // indexed by roster slot
};

struct LeagueSavedTeams
{
    u32 magic;
    u32 eventId;
    u8 acceptanceOptions; // saved level-cap and stat-equalizer flags
    u8 reserved[7];
    struct LeagueSavedTeam lineup[LEAGUE_EVENT_LINEUP_SIZE];
};

struct LeagueEventState
{
    u32 magic;
    u32 contentVersions[LEAGUE_EVENT_CONTENT_VERSION_COUNT];
    u32 eventId; // monotonic event generation, including resolved events
    u32 acceptedWorldProgress;
    u32 lineupChecksum; // covers accepted teams and their event generation
    u32 callCounter;
    u32 lastCallNumber[3];
    u32 lastCountedDay;
    u64 reignedIndigoMask;
    u64 reignedHoennMask;
    u16 galleryWins[LEAGUE_EVENT_NOTABLE_COUNT + 1]; // index 0 is the player
    u16 champions[3]; // none=0, player=LEAGUE_EVENT_PLAYER_CHAMPION
    u16 recentIds[LEAGUE_EVENT_LINEUP_SIZE];
    u8 invitationState;
    u8 daysRemaining;
    u8 invitedLeague;
    u8 acceptedLeague;
    u16 stateChecksum;
};

#endif // GUARD_LEAGUE_EVENT_TYPES_H

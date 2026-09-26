#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "config/league_circuit.h"
#include "data.h"
#include "debug.h"
#include "event_data.h"
#include "league_circuit.h"
#include "league_run_helpers.h"
#include "malloc.h"
#include "randomizer.h"
#include "test/test.h"
#include "trainer_party_scaling.h"
#include "trainer_rating.h"
#include "wayfarer_blue_dojo.h"
#include "wayfarer_local_adventures.h"
#include "wayfarer_persistence.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/heal_locations.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/species.h"
#include "constants/wayfarer_origin.h"

#if IS_WAYFARER

TEST("Viridian hidden Macho Brace does not share the Sprout Tower item flag")
{
    EXPECT_NE(FLAG_VIRIDIAN_GYM_HIDDEN_MACHO_BRACE_HNS, FLAG_ITEM_SPROUT_TOWER_ESCAPE_ROPE);
    EXPECT_EQ(FLAG_VIRIDIAN_GYM_HIDDEN_MACHO_BRACE_HNS - FLAG_HIDDEN_ITEMS_START, 0x874);
    FlagClear(FLAG_VIRIDIAN_GYM_HIDDEN_MACHO_BRACE_HNS);
    FlagClear(FLAG_ITEM_SPROUT_TOWER_ESCAPE_ROPE);
    FlagSet(FLAG_VIRIDIAN_GYM_HIDDEN_MACHO_BRACE_HNS);
    EXPECT(FlagGet(FLAG_VIRIDIAN_GYM_HIDDEN_MACHO_BRACE_HNS));
    EXPECT(!FlagGet(FLAG_ITEM_SPROUT_TOWER_ESCAPE_ROPE));
    FlagClear(FLAG_VIRIDIAN_GYM_HIDDEN_MACHO_BRACE_HNS);
    FlagSet(FLAG_ITEM_SPROUT_TOWER_ESCAPE_ROPE);
    EXPECT(!FlagGet(FLAG_VIRIDIAN_GYM_HIDDEN_MACHO_BRACE_HNS));
    EXPECT(FlagGet(FLAG_ITEM_SPROUT_TOWER_ESCAPE_ROPE));
    FlagClear(FLAG_ITEM_SPROUT_TOWER_ESCAPE_ROPE);
}

extern const u8 ViridianCity_Gym_EventScript_Jason[];
extern const u8 ViridianCity_Gym_EventScript_Jason_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Cole[];
extern const u8 ViridianCity_Gym_EventScript_Cole_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Atsushi[];
extern const u8 ViridianCity_Gym_EventScript_Atsushi_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Kiyo[];
extern const u8 ViridianCity_Gym_EventScript_Kiyo_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Takashi[];
extern const u8 ViridianCity_Gym_EventScript_Takashi_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Samuel[];
extern const u8 ViridianCity_Gym_EventScript_Samuel_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Yuji[];
extern const u8 ViridianCity_Gym_EventScript_Yuji_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Warren[];
extern const u8 ViridianCity_Gym_EventScript_Warren_TrainerBattle[];

TEST("Viridian sight battles skip each Trainer's interaction-only gate")
{
    static const struct { const u8 *start; const u8 *battle; } scripts[] = {
        {ViridianCity_Gym_EventScript_Jason, ViridianCity_Gym_EventScript_Jason_TrainerBattle},
        {ViridianCity_Gym_EventScript_Cole, ViridianCity_Gym_EventScript_Cole_TrainerBattle},
        {ViridianCity_Gym_EventScript_Atsushi, ViridianCity_Gym_EventScript_Atsushi_TrainerBattle},
        {ViridianCity_Gym_EventScript_Kiyo, ViridianCity_Gym_EventScript_Kiyo_TrainerBattle},
        {ViridianCity_Gym_EventScript_Takashi, ViridianCity_Gym_EventScript_Takashi_TrainerBattle},
        {ViridianCity_Gym_EventScript_Samuel, ViridianCity_Gym_EventScript_Samuel_TrainerBattle},
        {ViridianCity_Gym_EventScript_Yuji, ViridianCity_Gym_EventScript_Yuji_TrainerBattle},
        {ViridianCity_Gym_EventScript_Warren, ViridianCity_Gym_EventScript_Warren_TrainerBattle},
    };
    u32 i;

    for (i = 0; i < ARRAY_COUNT(scripts); i++)
        EXPECT_EQ(WayfarerResolveLocalAdventureTrainerBattleScript(scripts[i].start), scripts[i].battle);
    EXPECT_EQ(WayfarerResolveLocalAdventureTrainerBattleScript(ViridianCity_Gym_EventScript_Jason_TrainerBattle), NULL);
}

static void PrepareViridianParty(u32 rating)
{
    SetTrainerRating(rating);
    gIsDebugBattle = FALSE;
    ResetTrainerScalingSnapshot();
    gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
}

TEST("Viridian Giovanni constructs five FRLG source slots with Gym levels and authored moves")
{
    static const u16 species[] = {
        SPECIES_RHYHORN, SPECIES_DUGTRIO, SPECIES_NIDOQUEEN,
        SPECIES_NIDOKING, SPECIES_RHYHORN,
    };
    static const u8 ratings[] = {0, 40, 80};
    static const u8 levels[][5] = {
        {14, 13, 13, 14, 15},
        {41, 40, 40, 41, 42},
        {99, 98, 98, 99, 100},
    };
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    const struct Trainer *giovanni = GetTrainerStructFromId(TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS);
    u32 row, slot;

    EXPECT_EQ(giovanni->partySize, 5);
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS), TRAINER_SCALING_GYM_LEADER);
    for (row = 0; row < ARRAY_COUNT(ratings); row++)
    {
        PrepareViridianParty(ratings[row]);
        EXPECT_EQ(CreateNPCTrainerPartyForOpponent(party, TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS,
                                                   TRUE, BATTLE_TYPE_TRAINER), 5);
        for (slot = 0; slot < 5; slot++)
        {
            EXPECT_EQ(giovanni->party[slot].species, species[slot]);
            EXPECT_EQ(GetMonData(&party[slot], MON_DATA_SPECIES), species[slot]);
            EXPECT_EQ(GetMonData(&party[slot], MON_DATA_LEVEL), levels[row][slot]);
            EXPECT_EQ(GetMonData(&party[slot], MON_DATA_MOVE4), MOVE_EARTHQUAKE);
            EXPECT_EQ(GetMonData(&party[slot], MON_DATA_HP_IV), 0);
        }
    }
    Free(party);
}

TEST("Viridian Giovanni scaling is confined to an ordinary trainer battle")
{
    static const u8 sourceLevels[] = {45, 42, 44, 45, 50};
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    u32 slot;

    PrepareViridianParty(0);
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(party, TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS,
                                               TRUE, BATTLE_TYPE_TRAINER | BATTLE_TYPE_RECORDED), 5);
    for (slot = 0; slot < 5; slot++)
        EXPECT_EQ(GetMonData(&party[slot], MON_DATA_LEVEL), sourceLevels[slot]);
    Free(party);
}

TEST("Viridian Gym victories use distinct saved flags from local and Tower trainers")
{
    u16 id;

    ClearTrainerFlag(TRAINER_WAYFARER_TOWER_CHANNELER_PATRICIA);
    ClearTrainerFlag(TRAINER_NUGGET_BRIDGE_ROCKET_HNS);
    ClearTrainerFlag(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS);
    for (id = TRAINER_VIRIDIAN_GYM_FIRST; id <= TRAINER_VIRIDIAN_GYM_LAST; id++)
        ClearTrainerFlag(id);

    SetTrainerFlag(TRAINER_VIRIDIAN_GYM_TAKASHI_HNS);
    SetTrainerFlag(TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS);
    EXPECT(HasTrainerBeenFought(TRAINER_VIRIDIAN_GYM_TAKASHI_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_VIRIDIAN_GYM_YUJI_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_WAYFARER_TOWER_CHANNELER_PATRICIA));
    EXPECT(!HasTrainerBeenFought(TRAINER_NUGGET_BRIDGE_ROCKET_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS));

    SetTrainerFlag(TRAINER_WAYFARER_TOWER_CHANNELER_PATRICIA);
    SetTrainerFlag(TRAINER_NUGGET_BRIDGE_ROCKET_HNS);
    SetTrainerFlag(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS);
    ClearTrainerFlag(TRAINER_VIRIDIAN_GYM_TAKASHI_HNS);
    ClearTrainerFlag(TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS);
    EXPECT(!HasTrainerBeenFought(TRAINER_VIRIDIAN_GYM_TAKASHI_HNS));
    EXPECT(!HasTrainerBeenFought(TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_WAYFARER_TOWER_CHANNELER_PATRICIA));
    EXPECT(HasTrainerBeenFought(TRAINER_NUGGET_BRIDGE_ROCKET_HNS));
    EXPECT(HasTrainerBeenFought(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS));

    ClearTrainerFlag(TRAINER_WAYFARER_TOWER_CHANNELER_PATRICIA);
    ClearTrainerFlag(TRAINER_NUGGET_BRIDGE_ROCKET_HNS);
    ClearTrainerFlag(TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS);
}

// Bit 2 of leagueFlags is CIRCUIT_CLEAR_INDIGO, the stored first committed
// Indigo victory. Writing it directly fails this case if the seam ever reads
// another bit.
#define FIRST_INDIGO_VICTORY_BIT (1 << 2)

TEST("Dojo Blue visibility follows the committed first Indigo victory")
{
    u8 saved = gSaveBlock3Ptr->wayfarerHoenn.leagueFlags;

    gSaveBlock3Ptr->wayfarerHoenn.leagueFlags = saved & ~FIRST_INDIGO_VICTORY_BIT;
    FlagClear(FLAG_HIDE_DOJO_BLUE);
    EXPECT(!HasCommittedFirstIndigoVictory());
    SyncBlueDojoVisibility();
    EXPECT(FlagGet(FLAG_HIDE_DOJO_BLUE));

    gSaveBlock3Ptr->wayfarerHoenn.leagueFlags |= FIRST_INDIGO_VICTORY_BIT;
    EXPECT(HasCommittedFirstIndigoVictory());
    SyncBlueDojoVisibility();
    EXPECT(!FlagGet(FLAG_HIDE_DOJO_BLUE));

    // A rolled-back or never-committed victory hides Blue again on Dojo entry.
    gSaveBlock3Ptr->wayfarerHoenn.leagueFlags &= ~FIRST_INDIGO_VICTORY_BIT;
    SyncBlueDojoVisibility();
    EXPECT(FlagGet(FLAG_HIDE_DOJO_BLUE));

    gSaveBlock3Ptr->wayfarerHoenn.leagueFlags = saved;
}


#if WAYFARER_LEAGUE_CIRCUIT_ENABLED
TEST("Dojo Blue stays hidden after a rolled-back first Indigo victory")
{
    u8 badge;

    WayfarerInitPersistentState();
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NEW_BARK;
    gSaveBlock3Ptr->wayfarerHoenn.fallbackHealLocation = HEAL_LOCATION_NEW_BARK_TOWN_HNS;
    for (badge = 0; badge < 8; badge++)
        SetBadgeStateForRegion(REGION_JOHTO, badge, TRUE);
    SetTrainerRating(0);

    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
    SyncBlueDojoVisibility();
    EXPECT(!FlagGet(FLAG_HIDE_DOJO_BLUE));

    // A failed Hall of Fame save rolls the commit back without a Dojo hook.
    RollbackIndigoHallOfFameCommit(0, 0);
    EXPECT(!HasCommittedFirstIndigoVictory());
    SyncBlueDojoVisibility();
    EXPECT(FlagGet(FLAG_HIDE_DOJO_BLUE));

    EndLeagueRun();
    ConsumeRecordedCircuitClearStage();
    for (badge = 0; badge < 8; badge++)
        SetBadgeStateForRegion(REGION_JOHTO, badge, FALSE);
    WayfarerInitPersistentState();
}
#endif
#endif

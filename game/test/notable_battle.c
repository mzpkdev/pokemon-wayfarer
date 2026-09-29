#include "global.h"
#include "battle.h"
#include "battle_ai_main.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "battle_util2.h"
#include "data.h"
#include "debug.h"
#include "event_data.h"
#include "league_circuit.h"
#include "notable_trainers.h"
#include "pokemon.h"
#include "randomizer.h"
#include "test/test.h"
#include "trainer_party_scaling.h"
#include "trainer_rating.h"
#include "config/notable_trainers.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/species.h"
#include "constants/wayfarer_celadon_hideout_trainers.h"
#include "constants/wayfarer_indigo_trainers.h"
#include "constants/wayfarer_kanto_trainers.h"
#include "constants/wayfarer_viridian_trainers.h"

#if IS_WAYFARER && WAYFARER_V0_TRAINERS

static void BeginNotableConstruction(u32 rating, u32 battleTypeFlags)
{
    gBattleTypeFlags = battleTypeFlags;
    gIsDebugBattle = FALSE;
    SetTrainerRating(rating);
    ResetTrainerScalingSnapshot();
    gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingIVs = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingEVs = FALSE;
    AllocateBattleResources();
}

TEST("Notable battle builds Blue's first Pallet rival party and overrides tutorial AI")
{
    struct NotableTrainerSnapshot probe;
    u64 flags;
    u16 blue = TRAINER_WAYFARER_KANTO_BLUE_BULBASAUR;
    BeginNotableConstruction(0, BATTLE_TYPE_TRAINER | BATTLE_TYPE_FIRST_BATTLE);
    TRAINER_BATTLE_PARAM.opponentA = blue;
    TRAINER_BATTLE_PARAM.opponentB = 0;
    EXPECT_EQ(GetTrainerScalingSnapshot(), 0);
    EXPECT(GetNotableTrainerForEncounter(blue) != NULL);
    EXPECT(ResolveNotableTrainerSnapshot(GetNotableTrainerForEncounter(blue),
                                         GetTrainerScalingSnapshot(), FALSE, &probe));
    EXPECT_NE(probe.aiFlags, 0);
    EXPECT(GetTrainerStructFromId(blue)->party != NULL);
    EXPECT_GT(GetTrainerStructFromId(blue)->partySize, 0);

    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, blue, TRUE, gBattleTypeFlags), 1);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 5);
    EXPECT_EQ(GetMonData(&gEnemyParty[1], MON_DATA_SPECIES), SPECIES_NONE);
    EXPECT(GetNotableBattleAiFlags(blue, &flags));
    EXPECT(flags & AI_FLAG_BASIC_TRAINER);
    EXPECT(flags & AI_FLAG_ACE_POKEMON);
    BattleAI_SetupFlags();
    EXPECT_EQ(gAiThinkingStruct->aiFlags[B_BATTLER_1], flags);
    EXPECT_EQ(gAiThinkingStruct->aiFlags[B_BATTLER_3], flags);
    EXPECT_EQ(gAiThinkingStruct->aiFlags[B_BATTLER_0], flags);
    EXPECT_EQ(gAiThinkingStruct->aiFlags[B_BATTLER_2], flags);
    FreeBattleResources();
}

TEST("Notable battle preserves Brock's slot metadata and freezes his ordered team")
{
    u16 species[PARTY_SIZE];
    u16 items[PARTY_SIZE];
    u8 levels[PARTY_SIZE];
    u32 i;
    BeginNotableConstruction(160, BATTLE_TYPE_TRAINER);

    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_BROCK_HNS, TRUE, gBattleTypeFlags), PARTY_SIZE);
    // Fillers are sent first. Aerodactyl and Steelix are the last two aces.
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), SPECIES_OMASTAR);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_HELD_ITEM), ITEM_FOCUS_BAND);
    EXPECT_EQ(GetMonData(&gEnemyParty[1], MON_DATA_HELD_ITEM), ITEM_SCOPE_LENS);
    EXPECT_EQ(GetMonData(&gEnemyParty[3], MON_DATA_HELD_ITEM), ITEM_QUICK_CLAW);
    EXPECT_EQ(GetMonData(&gEnemyParty[4], MON_DATA_SPECIES), SPECIES_AERODACTYL);
    EXPECT_EQ(GetMonData(&gEnemyParty[5], MON_DATA_SPECIES), SPECIES_STEELIX);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        species[i] = GetMonData(&gEnemyParty[i], MON_DATA_SPECIES);
        items[i] = GetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM);
        levels[i] = GetMonData(&gEnemyParty[i], MON_DATA_LEVEL);
        EXPECT_NE(GetMonData(&gEnemyParty[i], MON_DATA_MOVE1), MOVE_NONE);
    }

    SetTrainerRating(0);
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_BROCK_HNS, TRUE, gBattleTypeFlags), PARTY_SIZE);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        EXPECT_EQ(GetMonData(&gEnemyParty[i], MON_DATA_SPECIES), species[i]);
        EXPECT_EQ(GetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM), items[i]);
        EXPECT_EQ(GetMonData(&gEnemyParty[i], MON_DATA_LEVEL), levels[i]);
    }
    FreeBattleResources();
}

TEST("Notable battle uses Giovanni's character roster in story and badge encounters")
{
    u16 species[PARTY_SIZE];
    u32 i, count;
    BeginNotableConstruction(80, BATTLE_TYPE_TRAINER);
    count = CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS, TRUE, gBattleTypeFlags);
    EXPECT_GT(count, 1);
    for (i = 0; i < count; i++)
        species[i] = GetMonData(&gEnemyParty[i], MON_DATA_SPECIES);
    ResetTrainerScalingSnapshot();
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS, TRUE, gBattleTypeFlags), count);
    for (i = 0; i < count; i++)
        EXPECT_EQ(GetMonData(&gEnemyParty[i], MON_DATA_SPECIES), species[i]);
    FreeBattleResources();
}

TEST("Notable battle freezes challenge IV and EV overrides with the team")
{
    u32 iv, ev;
    BeginNotableConstruction(160, BATTLE_TYPE_TRAINER);
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingIVs = 1;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingEVs = 2;
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_BROCK_HNS, TRUE, gBattleTypeFlags), PARTY_SIZE);
    iv = GetMonData(&gEnemyParty[0], MON_DATA_HP_IV);
    ev = GetMonData(&gEnemyParty[0], MON_DATA_HP_EV);
    EXPECT_EQ(ev, 128);
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingIVs = 0;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingEVs = 3;
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_BROCK_HNS, TRUE, gBattleTypeFlags), PARTY_SIZE);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_HP_IV), iv);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_HP_EV), ev);
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingIVs = 0;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingEVs = 0;
    FreeBattleResources();
}

TEST("Notable battle keeps Tate and Liza a double with alternating ace order")
{
    u64 flags;
    BeginNotableConstruction(0, BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE);
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_TATE_AND_LIZA_1;
    TRAINER_BATTLE_PARAM.opponentB = 0;
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_TATE_AND_LIZA_1, TRUE, gBattleTypeFlags), 2);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), SPECIES_LUNATONE);
    EXPECT_EQ(GetMonData(&gEnemyParty[1], MON_DATA_SPECIES), SPECIES_SOLROCK);
    EXPECT(GetNotableBattleAiFlags(TRAINER_TATE_AND_LIZA_1, &flags));
    EXPECT(flags & AI_FLAG_DOUBLE_BATTLE);
    BattleAI_SetupFlags();
    EXPECT_EQ(gAiThinkingStruct->aiFlags[B_BATTLER_1], flags);
    EXPECT_EQ(gAiThinkingStruct->aiFlags[B_BATTLER_3], flags);
    FreeBattleResources();

    BeginNotableConstruction(160, BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE);
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_TATE_AND_LIZA_1, TRUE, gBattleTypeFlags), PARTY_SIZE);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), SPECIES_GRUMPIG);
    EXPECT_EQ(GetMonData(&gEnemyParty[1], MON_DATA_SPECIES), SPECIES_XATU);
    EXPECT_EQ(GetMonData(&gEnemyParty[2], MON_DATA_SPECIES), SPECIES_CLAYDOL);
    EXPECT_EQ(GetMonData(&gEnemyParty[3], MON_DATA_SPECIES), SPECIES_GARDEVOIR);
    EXPECT_EQ(GetMonData(&gEnemyParty[4], MON_DATA_SPECIES), SPECIES_LUNATONE);
    EXPECT_EQ(GetMonData(&gEnemyParty[5], MON_DATA_SPECIES), SPECIES_SOLROCK);
    FreeBattleResources();
}

TEST("Notable battle species randomizer uses the source party and disables ace protection")
{
    u16 trainerId = TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS;
    const struct Trainer *source = GetTrainerStructFromId(trainerId);
    u64 flags;
    u32 i;
    BeginNotableConstruction(160, BATTLE_TYPE_TRAINER);
    gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = TRUE;
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, trainerId, TRUE, gBattleTypeFlags), source->partySize);
    for (i = 0; i < source->partySize; i++)
    {
        EXPECT_EQ(GetMonData(&gEnemyParty[i], MON_DATA_SPECIES),
                  RandomizeTrainerMon(source->trainerClass, i, source->partySize, source->party[i].species));
        EXPECT_EQ(GetMonData(&gEnemyParty[i], MON_DATA_LEVEL), source->party[i].lvl);
    }
    EXPECT(GetNotableBattleAiFlags(trainerId, &flags));
    EXPECT_EQ(flags & (AI_FLAG_ACE_POKEMON | AI_FLAG_DOUBLE_ACE_POKEMON), 0);
    gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = FALSE;
    FreeBattleResources();
}

TEST("Notable league opponent uses the frozen run entry rating")
{
    struct NotableTrainerSnapshot entry;
    u16 leagueId = TRAINER_WAYFARER_INDIGO_LORELEI;
    u16 map = MAP_POKEMON_LEAGUE_LORELEIS_ROOM;
    BeginNotableConstruction(160, BATTLE_TYPE_TRAINER);
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = TRUE;
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.stage = CIRCUIT_STAGE_INDIGO;
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry = 40;
    gSaveBlock3Ptr->wayfarerHoenn.indigoRoomDefeats = 0;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(map);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(map);
    VarSet(VAR_LEAGUE_STATE, 1);
    EXPECT(ResolveNotableTrainerSnapshot(GetNotableTrainerForEncounter(leagueId), 40, FALSE, &entry));

    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, leagueId, TRUE, gBattleTypeFlags), entry.teamSize);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), entry.members[entry.battleOrder[0]].lvl);
    EXPECT_NE(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 100);
    SetTrainerRating(0);
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, leagueId, TRUE, gBattleTypeFlags), entry.teamSize);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), entry.members[entry.battleOrder[0]].lvl);
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = FALSE;
    FreeBattleResources();
}

TEST("Notable league preparation fails before changing an existing party")
{
    u16 leagueId = TRAINER_WAYFARER_INDIGO_LORELEI;
    const struct Trainer *source = GetTrainerStructFromId(leagueId);
    u16 species[PARTY_SIZE];
    u8 levels[PARTY_SIZE];
    u64 flags;
    u32 i;
    BeginNotableConstruction(80, BATTLE_TYPE_TRAINER);
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = FALSE;
    EXPECT_EQ(CreateNPCTrainerPartyFromTrainer(gEnemyParty, source, TRUE, gBattleTypeFlags), source->partySize);
    for (i = 0; i < source->partySize; i++)
    {
        species[i] = GetMonData(&gEnemyParty[i], MON_DATA_SPECIES);
        levels[i] = GetMonData(&gEnemyParty[i], MON_DATA_LEVEL);
    }
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, leagueId, TRUE, gBattleTypeFlags), 0);
    EXPECT(!GetNotableBattleAiFlags(leagueId, &flags));
    for (i = 0; i < source->partySize; i++)
    {
        EXPECT_EQ(GetMonData(&gEnemyParty[i], MON_DATA_SPECIES), species[i]);
        EXPECT_EQ(GetMonData(&gEnemyParty[i], MON_DATA_LEVEL), levels[i]);
    }
    FreeBattleResources();
}

TEST("Notable battle excluded contexts keep source parties and clear prepared AI")
{
    u64 flags;
    const struct Trainer *source = GetTrainerStructFromId(TRAINER_BROCK_HNS);
    BeginNotableConstruction(160, BATTLE_TYPE_TRAINER);
    EXPECT_GT(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_BROCK_HNS, TRUE, gBattleTypeFlags), 0);
    EXPECT(GetNotableBattleAiFlags(TRAINER_BROCK_HNS, &flags));
    gBattleTypeFlags |= BATTLE_TYPE_RECORDED;
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_BROCK_HNS, TRUE, gBattleTypeFlags), source->partySize);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), source->party[0].species);
    EXPECT(!GetNotableBattleAiFlags(TRAINER_BROCK_HNS, &flags));
    FreeBattleResources();
}

#endif

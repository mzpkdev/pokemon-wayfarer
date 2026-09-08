#include "global.h"
#include "challenge_menu.h"
#include "event_data.h"
#include "pokemon.h"
#include "random.h"
#include "randomizer.h"
#include "starter_choose.h"
#include "wayfarer_origin.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/species.h"

#if IS_WAYFARER

extern bool8 Test_WayfarerGiveNativeHoennStarter(u16 choice);

static void ResetStarterState(void)
{
    WayfarerInitPersistentState();
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_LITTLEROOT;
    memset(&gSaveBlock3Ptr->challengeSettings, 0, sizeof(gSaveBlock3Ptr->challengeSettings));
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_OneTypeChallenge = 31;
    memset(gPlayerParty, 0, sizeof(gPlayerParty));
    gPlayerPartyCount = 0;
    memset(gSaveBlock2Ptr->playerTrainerId, 0, sizeof(gSaveBlock2Ptr->playerTrainerId));
    gSaveBlock2Ptr->playerTrainerId[0] = 19;
    VarSet(VAR_HOENN_STARTER_CHOICE, HOENN_STARTER_CHOICE_NONE);
    VarSet(VAR_STARTER_MON, 2);
    FlagClear(FLAG_HOENN_STARTER_RECEIVED);
    FlagClear(FLAG_SYS_POKEMON_GET);
}

static void CheckNativeDelivery(u16 slot)
{
    u16 species = GetStarterPokemon(slot);
    u32 personality;

    memset(gPlayerParty, 0, sizeof(gPlayerParty));
    gPlayerPartyCount = 0;
    VarSet(VAR_HOENN_STARTER_CHOICE, HOENN_STARTER_CHOICE_NONE);
    FlagClear(FLAG_HOENN_STARTER_RECEIVED);
    EXPECT(Test_WayfarerGiveNativeHoennStarter(slot));
    EXPECT_EQ(gPlayerPartyCount, 1);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), species);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_LEVEL), 5);
    EXPECT(GetMonData(&gPlayerParty[0], MON_DATA_HP) > 0);
    EXPECT_EQ(VarGet(VAR_HOENN_STARTER_CHOICE), slot);
    EXPECT_EQ(VarGet(VAR_STARTER_MON), 2);
    EXPECT(FlagGet(FLAG_HOENN_STARTER_RECEIVED));
    personality = GetMonData(&gPlayerParty[0], MON_DATA_PERSONALITY);
    EXPECT(Test_WayfarerGiveNativeHoennStarter((slot + 1) % 3));
    EXPECT_EQ(gPlayerPartyCount, 1);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_PERSONALITY), personality);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), species);
    EXPECT_EQ(VarGet(VAR_HOENN_STARTER_CHOICE), slot);
}

TEST("Wayfarer starter previews retain both three-slot native rosters and exact-once Hoenn delivery")
{
    static const u16 johto[] = {SPECIES_CHIKORITA, SPECIES_CYNDAQUIL, SPECIES_TOTODILE};
    static const u16 hoenn[] = {SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP};
    u16 slot;

    ResetStarterState();
    for (slot = 0; slot < 3; slot++)
    {
        EXPECT_EQ(GetJohtoStarterPokemon(slot), johto[slot]);
        EXPECT_EQ(GetStarterPokemon(slot), hoenn[slot]);
        CheckNativeDelivery(slot);
    }
}

TEST("Wayfarer native starter rejects an invalid uncommitted slot without receipt or party mutation")
{
    ResetStarterState();
    EXPECT(!Test_WayfarerGiveNativeHoennStarter(HOENN_STARTER_CHOICE_NONE));
    EXPECT_EQ(gPlayerPartyCount, 0);
    EXPECT_EQ(VarGet(VAR_HOENN_STARTER_CHOICE), HOENN_STARTER_CHOICE_NONE);
    EXPECT_EQ(VarGet(VAR_STARTER_MON), 2);
    EXPECT(!FlagGet(FLAG_HOENN_STARTER_RECEIVED));
}

TEST("Wayfarer monotype starters refresh after type trainer and scope changes and match actual native grants")
{
    static const u8 types[] = {TYPE_FIRE, TYPE_WATER, TYPE_DRAGON};
    u16 warm[3];
    u16 slot;
    u8 type, trainer, scope;

    ResetStarterState();
    for (type = 0; type < ARRAY_COUNT(types); type++)
    {
        gSaveBlock3Ptr->challengeSettings.tx_Challenges_OneTypeChallenge = types[type];
        for (trainer = 0; trainer < 2; trainer++)
        {
            gSaveBlock2Ptr->playerTrainerId[0] = 19 + trainer;
            for (scope = 0; scope < 2; scope++)
            {
                gSaveBlock3Ptr->challengeSettings.tx_Random_GenScope = scope;
                for (slot = 0; slot < 3; slot++)
                    warm[slot] = GetStarterPokemon(slot);
                Test_ResetStarterChooseCache();
                for (slot = 0; slot < 3; slot++)
                {
                    EXPECT_EQ(GetStarterPokemon(slot), warm[slot]);
                    EXPECT_EQ(GetJohtoStarterPokemon(slot), warm[slot]);
                    EXPECT(DoesSpeciesPassOneTypeChallenge(warm[slot]));
#if RANDOMIZER_AVAILABLE
                    EXPECT(IsSpeciesInGenScope(warm[slot]));
#endif
                    CheckNativeDelivery(slot);
                }
            }
        }
    }
}

#if RANDOMIZER_AVAILABLE
TEST("Wayfarer randomized starter previews survive RNG seed and option changes and match native delivery")
{
    static const u16 johto[] = {SPECIES_CHIKORITA, SPECIES_CYNDAQUIL, SPECIES_TOTODILE};
    static const u16 hoenn[] = {SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP};
    u16 slot, species;
    u8 mode, trainer;

    ResetStarterState();
    gSaveBlock3Ptr->challengeSettings.tx_Random_Starter = TRUE;
    for (trainer = 0; trainer < 2; trainer++)
    {
        gSaveBlock2Ptr->playerTrainerId[0] = 19 + trainer;
        for (mode = 0; mode < 3; mode++)
        {
            gSaveBlock3Ptr->challengeSettings.tx_Random_IncludeLegendaries = mode == 1;
            gSaveBlock3Ptr->challengeSettings.tx_Random_Similar = mode == 2;
            for (slot = 0; slot < 3; slot++)
            {
                species = RandomizeMon(RANDOMIZER_REASON_STARTER_AND_GIFT_MON,
                    GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE),
                    GetRandomizerSeed() ^ hoenn[slot], hoenn[slot]);
                EXPECT_EQ(GetStarterPokemon(slot), species);
                SeedRng(0xABC000 + trainer * 10 + mode);
                EXPECT_EQ(GetStarterPokemon(slot), species);
                EXPECT_EQ(GetJohtoStarterPokemon(slot),
                    RandomizeMon(RANDOMIZER_REASON_STARTER_AND_GIFT_MON,
                        GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE),
                        GetRandomizerSeed() ^ johto[slot], johto[slot]));
                CheckNativeDelivery(slot);
            }
        }
    }
}
#endif
#endif

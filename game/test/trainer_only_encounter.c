#include "global.h"
#include "battle.h"
#include "test/test.h"
#include "item_use.h"
#include "trainer_only_encounter.h"

TEST("Trainer-only rocks use bounded rational damage and passive anger")
{
    EXPECT_EQ(TrainerOnlyRockDamage(15, 30, 80, 80), 5);
    EXPECT_EQ(TrainerOnlyPassiveAnger(15, 30), 10);
    EXPECT_EQ(TrainerOnlyRockDamage(30, 30, 80, 80), 10);
    EXPECT_EQ(TrainerOnlyPassiveAnger(30, 30), 5);
    EXPECT_EQ(TrainerOnlyRockDamage(100, 1, 81, 81), 17);
    EXPECT_EQ(TrainerOnlyRockDamage(1, 100, 81, 81), 5);
    EXPECT_EQ(TrainerOnlyRockDamage(100, 1, 65535, 65535), 13107);
    EXPECT_EQ(TrainerOnlyRockDamage(30, 30, 80, 3), 3);
    EXPECT_EQ(TrainerOnlyRockDamage(1, 100, 1, 1), 1);
    EXPECT_EQ(TrainerOnlyPassiveAnger(100, 1), 3);
    EXPECT_EQ(TrainerOnlyPassiveAnger(1, 100), 15);
}
TEST("Trainer-only approach retains repeated increments and independent caps")
{
    struct TrainerOnlyState state = {.catchFactor = 1, .escapeFactor = 3};
    static const u8 expected[] = {5, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 20};
    for (u32 i = 0; i < ARRAY_COUNT(expected); i++)
    {
        TrainerOnlyApplyApproach(&state);
        EXPECT_EQ(state.catchFactor, expected[i]);
        EXPECT_EQ(state.escapeFactor, min(20, 7 + 4 * i));
        EXPECT_EQ(state.approach, min(3, i + 1));
    }
}
TEST("Trainer-only warning precedes largest gain and food fear bounds")
{
    struct TrainerOnlyState state = {.escapeFactor = 3};
    EXPECT_EQ(TrainerOnlyFleeChance(&state), 15);
    state.foodTurns = 3;
    EXPECT_EQ(TrainerOnlyFleeChance(&state), 5);
    state.rocks = 14;
    EXPECT_EQ(TrainerOnlyFleeChance(&state), 75);
    for (u32 passive = 3; passive <= 15; passive++)
        EXPECT_EQ(TrainerOnlyWarningThreshold(passive) + 25 + passive, 100);
}
TEST("Trainer-only approved escape curve scales strength and failed attempts without guarantee")
{
    EXPECT_EQ(TrainerOnlyEscapeChance(15, 30, 0), 25);
    EXPECT_EQ(TrainerOnlyEscapeChance(30, 30, 0), 50);
    EXPECT_EQ(TrainerOnlyEscapeChance(30, 30, 1), 65);
    EXPECT_EQ(TrainerOnlyEscapeChance(30, 30, 2), 80);
    EXPECT_EQ(TrainerOnlyEscapeChance(30, 30, 255), 95);
    EXPECT_EQ(TrainerOnlyEscapeChance(1, 100, 0), 5);
    EXPECT_EQ(TrainerOnlyEscapeChance(100, 1, 0), 95);
}
TEST("Trainer-only surviving turn gives retaliation priority and counts food exactly three checks")
{
    struct TrainerOnlyState state = {.cap = 30, .level = 30, .escapeFactor = 3, .foodTurns = 3};
    for (u32 i = 0; i < 3; i++)
    {
        EXPECT_EQ(TrainerOnlyFleeChance(&state), 5);
        EXPECT_EQ(TrainerOnlyResolveSurvivingTurn(&state, 99), TRAINER_ONLY_ONGOING);
        EXPECT_EQ(state.completedTurns, i + 1);
    }
    EXPECT_EQ(state.foodTurns, 0);
    EXPECT_EQ(TrainerOnlyFleeChance(&state), 15);
    state.anger = 95;
    EXPECT_EQ(TrainerOnlyResolveSurvivingTurn(&state, 0), TRAINER_ONLY_RETALIATION);
    EXPECT_EQ(state.completedTurns, 3);
    EXPECT_EQ(state.anger, 100);
    state.anger = 0;
    state.foodTurns = 3;
    EXPECT_EQ(TrainerOnlyResolveSurvivingTurn(&state, 0), TRAINER_ONLY_FLED);
    EXPECT_EQ(state.foodTurns, 3);
    EXPECT_EQ(state.completedTurns, 3);
    state.completedTurns = 255;
    EXPECT_EQ(TrainerOnlyResolveSurvivingTurn(&state, 99), TRAINER_ONLY_ONGOING);
    EXPECT_EQ(state.completedTurns, 255);
}

TEST("Trainer-only feeding preserves the reviewed berry allowlist for every item")
{
    // Preserve the original explicit list as an oracle for the compact range check.
    static const enum Item reviewed[] = {
        ITEM_CHERI_BERRY,
        ITEM_CHESTO_BERRY,
        ITEM_PECHA_BERRY,
        ITEM_RAWST_BERRY,
        ITEM_ASPEAR_BERRY,
        ITEM_LEPPA_BERRY,
        ITEM_ORAN_BERRY,
        ITEM_PERSIM_BERRY,
        ITEM_LUM_BERRY,
        ITEM_SITRUS_BERRY,
        ITEM_FIGY_BERRY,
        ITEM_WIKI_BERRY,
        ITEM_MAGO_BERRY,
        ITEM_AGUAV_BERRY,
        ITEM_IAPAPA_BERRY,
        ITEM_RAZZ_BERRY,
        ITEM_BLUK_BERRY,
        ITEM_NANAB_BERRY,
        ITEM_WEPEAR_BERRY,
        ITEM_PINAP_BERRY,
        ITEM_POMEG_BERRY,
        ITEM_KELPSY_BERRY,
        ITEM_QUALOT_BERRY,
        ITEM_HONDEW_BERRY,
        ITEM_GREPA_BERRY,
        ITEM_TAMATO_BERRY,
        ITEM_CORNN_BERRY,
        ITEM_MAGOST_BERRY,
        ITEM_RABUTA_BERRY,
        ITEM_NOMEL_BERRY,
        ITEM_SPELON_BERRY,
        ITEM_PAMTRE_BERRY,
        ITEM_WATMEL_BERRY,
        ITEM_DURIN_BERRY,
        ITEM_BELUE_BERRY,
        ITEM_CHILAN_BERRY,
        ITEM_OCCA_BERRY,
        ITEM_PASSHO_BERRY,
        ITEM_WACAN_BERRY,
        ITEM_RINDO_BERRY,
        ITEM_YACHE_BERRY,
        ITEM_CHOPLE_BERRY,
        ITEM_KEBIA_BERRY,
        ITEM_SHUCA_BERRY,
        ITEM_COBA_BERRY,
        ITEM_PAYAPA_BERRY,
        ITEM_TANGA_BERRY,
        ITEM_CHARTI_BERRY,
        ITEM_KASIB_BERRY,
        ITEM_HABAN_BERRY,
        ITEM_COLBUR_BERRY,
        ITEM_BABIRI_BERRY,
        ITEM_ROSELI_BERRY,
        ITEM_LIECHI_BERRY,
        ITEM_GANLON_BERRY,
        ITEM_SALAC_BERRY,
        ITEM_PETAYA_BERRY,
        ITEM_APICOT_BERRY,
        ITEM_LANSAT_BERRY,
        ITEM_STARF_BERRY,
        ITEM_ENIGMA_BERRY,
        ITEM_MICLE_BERRY,
        ITEM_CUSTAP_BERRY,
        ITEM_JABOCA_BERRY,
        ITEM_ROWAP_BERRY,
        ITEM_KEE_BERRY,
        ITEM_MARANGA_BERRY,
    };
    for (u32 item = ITEM_NONE; item < ITEMS_COUNT; item++)
    {
        bool32 expected = FALSE;
        for (u32 i = 0; i < ARRAY_COUNT(reviewed); i++)
            if (item == reviewed[i])
                expected = TRUE;
        EXPECT_EQ(TrainerOnlyIsFeedableBerry(item), expected);
    }
    EXPECT(!TrainerOnlyIsFeedableBerry(ITEM_ENIGMA_BERRY_E_READER));
    EXPECT(!TrainerOnlyIsFeedableBerry(0xFFFF));
}
TEST("Trainer-only feeding calms before passive anger and refreshes without erasing fear or proximity")
{
    struct TrainerOnlyState state = {.cap = 30, .level = 30, .anger = 28, .rocks = 2, .catchFactor = 14, .escapeFactor = 15, .approach = 3, .foodTurns = 1};
    TrainerOnlyApplyFood(&state);
    EXPECT_EQ(state.anger, 8);
    EXPECT_EQ(state.foodTurns, 3);
    EXPECT_EQ(state.rocks, 2);
    EXPECT_EQ(state.catchFactor, 14);
    EXPECT_EQ(state.escapeFactor, 15);
    EXPECT_EQ(state.approach, 3);
    EXPECT_EQ(TrainerOnlyResolveSurvivingTurn(&state, 99), TRAINER_ONLY_ONGOING);
    EXPECT_EQ(state.anger, 13);
    EXPECT_EQ(state.foodTurns, 2);
    TrainerOnlyApplyFood(&state);
    EXPECT_EQ(state.anger, 0);
    EXPECT_EQ(state.foodTurns, 3);
}

TEST("Trainer-only Less Escapes vetoes an otherwise successful single roll")
{
    struct TrainerOnlyState state = {.cap = 30, .level = 30, .escapeFactor = 3};
    u32 chance = TrainerOnlyEscapeChance(state.cap, state.level, state.escapeAttempts);
    EXPECT(TrainerOnlyRunSucceeds(chance, FALSE, 512));
    EXPECT(!TrainerOnlyRunSucceeds(chance, TRUE, 512));
    EXPECT(TrainerOnlyRunSucceeds(chance, TRUE, 0));
    // A veto is a failed committed Run, so its surviving-turn resolver still
    // applies passive anger and advances time. The controller owns attempts.
    EXPECT_EQ(TrainerOnlyResolveSurvivingTurn(&state, 99), TRAINER_ONLY_ONGOING);
    EXPECT_EQ(state.anger, 5);
    EXPECT_EQ(state.completedTurns, 1);
    EXPECT(!TrainerOnlyRunSucceeds(chance, FALSE, 99));
    EXPECT(!TrainerOnlyRunSucceeds(chance, TRUE, 99));
}

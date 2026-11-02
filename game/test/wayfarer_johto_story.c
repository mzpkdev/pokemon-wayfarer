#include "global.h"
#include "event_data.h"
#include "new_game.h"
#include "script.h"
#include "item.h"
#include "trainer_rating.h"
#include "wayfarer_sevii_story.h"
#include "starter_choose.h"
#include "wayfarer_appearance.h"
#include "wayfarer_johto_story.h"
#include "wayfarer_origin.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/wayfarer_johto_story.h"

#if IS_WAYFARER

static void StartAs(u16 origin)
{
    EXPECT(WayfarerConfirmPendingOrigin(origin));
    EXPECT(WayfarerConfirmPendingAppearance(APPEARANCE_GOLD));
    NewGameInitData();
    EXPECT_EQ(WayfarerGetStartingOriginId(), origin);
}

static bool8 Playable(u16 chapter)
{
    gSpecialVar_0x8004 = chapter;
    return WayfarerJohtoSilverAvailable();
}

static const u16 sWinFlags[] =
{
    [SILVER_CHAPTER_CHERRYGROVE] = FLAG_WAYFARER_JOHTO_SILVER_WON_2,
    [SILVER_CHAPTER_AZALEA] = FLAG_WAYFARER_JOHTO_SILVER_WON_4,
    [SILVER_CHAPTER_BURNED_TOWER] = FLAG_WAYFARER_JOHTO_SILVER_WON_5,
    [SILVER_CHAPTER_UNDERGROUND] = FLAG_WAYFARER_JOHTO_SILVER_WON_8,
    [SILVER_CHAPTER_VICTORY_ROAD] = FLAG_WAYFARER_JOHTO_SILVER_WON_10,
    [SILVER_CHAPTER_MT_MOON] = FLAG_WAYFARER_JOHTO_SILVER_WON_11,
};

// Chapters whose availability depends only on the stored chapter, in story order.
static const u8 sStoredChapterScenes[] =
{
    SILVER_CHAPTER_NEW_BARK, SILVER_CHAPTER_CHERRYGROVE, SILVER_CHAPTER_SPROUT_TOWER, SILVER_CHAPTER_AZALEA,
    SILVER_CHAPTER_BURNED_TOWER, SILVER_CHAPTER_OLIVINE, SILVER_CHAPTER_HIDEOUT, SILVER_CHAPTER_UNDERGROUND,
    SILVER_CHAPTER_VICTORY_ROAD,
};

struct ChapterRow
{
    u8 started;
    const char *playable; // one character per entry of sStoredChapterScenes
};

TEST("Silver chapters retire earlier scenes once a later battle has started")
{
    static const struct ChapterRow rows[] =
    {
        {0,  "111111111"},
        {2,  "011111111"},  // a lost Cherrygrove battle is offered again; only New Bark retires
        {4,  "000111111"},
        {5,  "000011111"},
        {8,  "000000011"},  // Olivine and the hideout retire with a chapter 8 battle
        {10, "000000001"},
        {11, "000000000"},
    };
    u32 i, scene;

    StartAs(ORIGIN_NEW_BARK);
    for (i = 0; i < ARRAY_COUNT(rows); i++)
    {
        VarSet(VAR_WAYFARER_JOHTO_SILVER_CHAPTER, rows[i].started);
        for (scene = 0; scene < ARRAY_COUNT(sStoredChapterScenes); scene++)
            EXPECT_EQ(Playable(sStoredChapterScenes[scene]), rows[i].playable[scene] == '1');
    }
}

TEST("Silver battle chapters are retired by their own win, and played scenes do not repeat")
{
    u32 chapter;

    StartAs(ORIGIN_NEW_BARK);
    VarSet(VAR_WAYFARER_JOHTO_SILVER_CHAPTER, SILVER_CHAPTER_VICTORY_ROAD);
    for (chapter = SILVER_CHAPTER_CHERRYGROVE; chapter <= SILVER_CHAPTER_MT_MOON; chapter++)
    {
        if (sWinFlags[chapter] == 0)
            continue;
        VarSet(VAR_WAYFARER_JOHTO_SILVER_CHAPTER, chapter);
        EXPECT(Playable(chapter));
        FlagSet(sWinFlags[chapter]);
        EXPECT(!Playable(chapter));
        FlagClear(sWinFlags[chapter]);
    }
    VarSet(VAR_WAYFARER_JOHTO_SILVER_CHAPTER, 0);
    EXPECT(Playable(SILVER_CHAPTER_OLIVINE));
    FlagSet(FLAG_WAYFARER_JOHTO_SILVER_PLAYED_6);
    EXPECT(!Playable(SILVER_CHAPTER_OLIVINE));
    EXPECT(Playable(SILVER_CHAPTER_HIDEOUT));
    FlagSet(FLAG_WAYFARER_JOHTO_SILVER_PLAYED_7);
    EXPECT(!Playable(SILVER_CHAPTER_HIDEOUT));
}

TEST("Silver's Ecruteak scene needs the resolved Radio Tower, Mt. Moon follows Victory Road, Indigo follows Mt. Moon")
{
    StartAs(ORIGIN_NEW_BARK);
    EXPECT(!Playable(SILVER_CHAPTER_ECRUTEAK));
    FlagSet(FLAG_WAYFARER_JOHTO_RADIO_TOWER_RESOLVED);
    EXPECT(Playable(SILVER_CHAPTER_ECRUTEAK));
    FlagSet(FLAG_WAYFARER_JOHTO_SILVER_PLAYED_9);
    EXPECT(!Playable(SILVER_CHAPTER_ECRUTEAK));
    FlagClear(FLAG_WAYFARER_JOHTO_SILVER_PLAYED_9);
    VarSet(VAR_WAYFARER_JOHTO_SILVER_CHAPTER, SILVER_CHAPTER_VICTORY_ROAD);
    EXPECT(!Playable(SILVER_CHAPTER_ECRUTEAK));

    VarSet(VAR_WAYFARER_JOHTO_SILVER_CHAPTER, 0);
    EXPECT(!Playable(SILVER_CHAPTER_MT_MOON));
    VarSet(VAR_WAYFARER_JOHTO_SILVER_CHAPTER, SILVER_CHAPTER_VICTORY_ROAD);
    EXPECT(Playable(SILVER_CHAPTER_MT_MOON));
    EXPECT(!Playable(SILVER_CHAPTER_INDIGO));
    FlagSet(FLAG_WAYFARER_JOHTO_SILVER_WON_11);
    EXPECT(!Playable(SILVER_CHAPTER_MT_MOON));
    EXPECT(Playable(SILVER_CHAPTER_INDIGO));
}

TEST("Silver exists only for the Johto origin")
{
    u16 origin;
    u32 chapter;

    for (origin = ORIGIN_LITTLEROOT; origin <= ORIGIN_PALLET; origin++)
    {
        StartAs(origin);
        FlagSet(FLAG_WAYFARER_JOHTO_RADIO_TOWER_RESOLVED);
        FlagSet(FLAG_WAYFARER_JOHTO_SILVER_WON_11);
        VarSet(VAR_WAYFARER_JOHTO_SILVER_CHAPTER, SILVER_CHAPTER_VICTORY_ROAD);
        for (chapter = SILVER_CHAPTER_NEW_BARK; chapter <= SILVER_CHAPTER_INDIGO; chapter++)
            EXPECT(!Playable(chapter));
    }
}

TEST("Each origin's own starter slot is read, and none is reported before it is chosen")
{
    StartAs(ORIGIN_NEW_BARK);
    EXPECT_EQ(WayfarerGetOriginStarterSlot(), ORIGIN_STARTER_SLOT_NONE);
    VarSet(VAR_STARTER_MON, 2);
    EXPECT_EQ(WayfarerGetOriginStarterSlot(), ORIGIN_STARTER_SLOT_NONE);
    FlagSet(FLAG_JOHTO_STARTER_CHOICE_COMMITTED);
    EXPECT_EQ(WayfarerGetOriginStarterSlot(), 2);

    StartAs(ORIGIN_LITTLEROOT);
    VarSet(VAR_STARTER_MON, 1); // a stale Johto value never leaks into another origin
    EXPECT_EQ(WayfarerGetOriginStarterSlot(), ORIGIN_STARTER_SLOT_NONE);
    VarSet(VAR_HOENN_STARTER_CHOICE, HOENN_STARTER_CHOICE_TORCHIC);
    EXPECT_EQ(WayfarerGetOriginStarterSlot(), 1);

    StartAs(ORIGIN_PALLET);
    EXPECT_EQ(WayfarerGetOriginStarterSlot(), ORIGIN_STARTER_SLOT_NONE);
    gSaveBlock3Ptr->wayfarerPalletOpening.starterSlot = 2;
    EXPECT_EQ(WayfarerGetOriginStarterSlot(), 2);
}

TEST("The credits starter is the origin's own, with a neutral stand-in before a choice")
{
    StartAs(ORIGIN_PALLET);
    EXPECT_EQ(WayfarerGetOriginStarterSpecies(), SPECIES_PIKACHU);
    StartAs(ORIGIN_LITTLEROOT);
    VarSet(VAR_HOENN_STARTER_CHOICE, HOENN_STARTER_CHOICE_MUDKIP);
    EXPECT_EQ(WayfarerGetOriginStarterSpecies(), GetStarterPokemon(2));
    StartAs(ORIGIN_NEW_BARK);
    FlagSet(FLAG_JOHTO_STARTER_CHOICE_COMMITTED);
    VarSet(VAR_STARTER_MON, 0);
    EXPECT_EQ(WayfarerGetOriginStarterSpecies(), GetJohtoStarterPokemon(0));
}

TEST("The Kimono trial needs the branch's Wing in the Bag, the resolved Radio Tower and the shared TR")
{
    u32 branch;
    static const u16 wings[] = {ITEM_NONE, ITEM_SILVER_WING, ITEM_RAINBOW_WING};

    StartAs(ORIGIN_NEW_BARK);
    SetTrainerRating(WAYFARER_LEGENDARY_TR);
    EXPECT_EQ(WAYFARER_BIRD_CAPTURE_TR, WAYFARER_LEGENDARY_TR);
    for (branch = 0; branch <= 2; branch++)
    {
        VarSet(VAR_LUGIA_OR_HOOH, branch);
        FlagClear(FLAG_WAYFARER_JOHTO_RADIO_TOWER_RESOLVED);
        SetTrainerRating(0);
        EXPECT_EQ(WayfarerJohtoKimonoMissing(), KIMONO_MISSING_BOTH);
        SetTrainerRating(WAYFARER_LEGENDARY_TR);
        EXPECT_EQ(WayfarerJohtoKimonoMissing(), KIMONO_MISSING_WING);
        FlagSet(FLAG_WAYFARER_JOHTO_RADIO_TOWER_RESOLVED);
        EXPECT_EQ(WayfarerJohtoKimonoMissing(), KIMONO_MISSING_WING);
        if (branch == 0)
            continue;
        EXPECT(AddBagItem(wings[branch], 1));
        EXPECT_EQ(WayfarerJohtoKimonoMissing(), 0);
        EXPECT(WayfarerJohtoKimonoTrialAvailable());
        SetTrainerRating(WAYFARER_LEGENDARY_TR - 1);
        EXPECT_EQ(WayfarerJohtoKimonoMissing(), KIMONO_MISSING_TR);
        SetTrainerRating(WAYFARER_LEGENDARY_TR);
        VarSet(VAR_WAYFARER_JOHTO_KIMONO_CEREMONY_STATE, KIMONO_CEREMONY_STARTED);
        EXPECT(WayfarerJohtoKimonoTrialAvailable());
        VarSet(VAR_WAYFARER_JOHTO_KIMONO_CEREMONY_STATE, KIMONO_CEREMONY_DONE);
        EXPECT(!WayfarerJohtoKimonoTrialAvailable());
        VarSet(VAR_WAYFARER_JOHTO_KIMONO_CEREMONY_STATE, 0);
        EXPECT(RemoveBagItem(wings[branch], 1));
        EXPECT_EQ(WayfarerJohtoKimonoMissing(), KIMONO_MISSING_WING);
    }
}

TEST("Pewter's other Wing needs the main encounter resolved and the shared TR")
{
    StartAs(ORIGIN_NEW_BARK);
    VarSet(VAR_LUGIA_OR_HOOH, 1);
    SetTrainerRating(WAYFARER_LEGENDARY_TR);
    VarSet(VAR_COMPLETED_LUGIA, 2);
    EXPECT_EQ(WayfarerJohtoPewterWingMissing(), PEWTER_MISSING_ENCOUNTER);
    VarSet(VAR_COMPLETED_LUGIA, 4);
    EXPECT_EQ(WayfarerJohtoPewterWingMissing(), PEWTER_MISSING_ENCOUNTER);
    VarSet(VAR_COMPLETED_LUGIA, 3);
    EXPECT_EQ(WayfarerJohtoPewterWingMissing(), 0);
    SetTrainerRating(WAYFARER_LEGENDARY_TR - 1);
    EXPECT_EQ(WayfarerJohtoPewterWingMissing(), PEWTER_MISSING_TR);
    VarSet(VAR_LUGIA_OR_HOOH, 2);
    EXPECT_EQ(WayfarerJohtoPewterWingMissing(), PEWTER_MISSING_BOTH);
    VarSet(VAR_COMPLETED_HO_OH, 3);
    EXPECT_EQ(WayfarerJohtoPewterWingMissing(), PEWTER_MISSING_TR);
}

#endif

#include "global.h"
#include "event_data.h"
#include "trainer_rating.h"

u8 ClampTrainerRating(u16 rating)
{
    if (rating > TRAINER_RATING_MAX)
        return TRAINER_RATING_MAX;

    return rating;
}

static u8 CalculateBadgeTrainerRating(void)
{
    u8 rating = 0;

    if (FlagGet(FLAG_BADGE01_GET))
        rating += 4;
    if (FlagGet(FLAG_BADGE02_GET))
        rating += 4;
    if (FlagGet(FLAG_BADGE03_GET))
        rating += 4;
    if (FlagGet(FLAG_BADGE04_GET))
        rating += 4;
    if (FlagGet(FLAG_BADGE05_GET))
        rating += 6;
    if (FlagGet(FLAG_BADGE06_GET))
        rating += 6;
    if (FlagGet(FLAG_BADGE07_GET))
        rating += 6;
    if (FlagGet(FLAG_BADGE08_GET))
        rating += 6;

#if IS_HNS
    // HnS has a second region. Read every Kanto badge flag directly rather
    // than the eight-entry gBadgeFlags helper or VAR_NUM_BADGES.
    if (FlagGet(FLAG_BADGE09_GET))
        rating++;
    if (FlagGet(FLAG_BADGE10_GET))
        rating++;
    if (FlagGet(FLAG_BADGE11_GET))
        rating++;
    if (FlagGet(FLAG_BADGE12_GET))
        rating++;
    if (FlagGet(FLAG_BADGE13_GET))
        rating++;
    if (FlagGet(FLAG_BADGE14_GET))
        rating++;
    if (FlagGet(FLAG_BADGE15_GET))
        rating++;
    if (FlagGet(FLAG_BADGE16_GET))
        rating++;
#endif

    return rating;
}

u8 CalculateTrainerRatingFromCurrentFacts(void)
{
    u8 rating = CalculateBadgeTrainerRating();

#if IS_FRLG
    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        rating += 15;

    if (FlagGet(FLAG_RECOVERED_SAPPHIRE))
        rating += 10;
#else
    if (FlagGet(FLAG_IS_CHAMPION))
        rating += 15;
#endif

#if IS_HNS
    if (FlagGet(FLAG_IS_KANTO_CHAMPION))
        rating += 2;
#elif !IS_FRLG
    // Birch sets this during Emerald's post-Champion National Dex upgrade.
    if (FlagGet(FLAG_SYS_NATIONAL_DEX))
        rating += 10;
#endif

    return ClampTrainerRating(rating);
}

u8 GetTrainerRating(void)
{
    u8 storedRating = ClampTrainerRating(VarGet(VAR_TRAINER_RATING));
    u8 derivedRating = CalculateTrainerRatingFromCurrentFacts();
    u8 rating = max(storedRating, derivedRating);

    if (VarGet(VAR_TRAINER_RATING) != rating)
        VarSet(VAR_TRAINER_RATING, rating);

    return rating;
}

void InitializeTrainerRatingForSaveMigration(void)
{
    VarSet(VAR_TRAINER_RATING, CalculateTrainerRatingFromCurrentFacts());
}

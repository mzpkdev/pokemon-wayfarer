#include "global.h"
#include "event_data.h"
#include "config/league_circuit.h"
#include "config/notable_trainers.h"
#include "league_circuit.h"
#include "pokemon.h"
#include "trainer_rating.h"
#include "trainer_scaler.h"

#if WAYFARER_V0_TRAINERS
static const struct TrainerScalerAnchor sTrainerRatingSoftLevelCapsV0[] =
{
    { 0, 15 }, { 40, 28 }, { 80, 50 }, { 120, 75 }, { 160, 100 },
};
#else
static const struct
{
    u8 rating;
    u8 cap;
} sTrainerRatingSoftLevelCaps[] =
{
    {  0,  15 },
    {  4,  16 },
    {  8,  18 },
    { 16,  23 },
    { 30,  30 },
    { 40,  42 },
    { 55,  60 },
    { 65,  80 },
    { 80, 100 },
};
#endif

u8 ClampTrainerRating(u16 rating)
{
#if !IS_WAYFARER
    if (rating < 10)
        return 10;
#else
    if (rating < TRAINER_RATING_MIN)
        return TRAINER_RATING_MIN;
#endif
    if (rating > TRAINER_RATING_MAX)
        return TRAINER_RATING_MAX;

    return rating;
}

#if !IS_WAYFARER
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

static u8 CalculateLegacyTrainerRating(void)
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
    if (FlagGet(FLAG_SYS_NATIONAL_DEX))
        rating += 10;
#endif
    return ClampTrainerRating(rating);
}
#endif

u32 GetTrainerRating(void)
{
#if WAYFARER_V0_TRAINERS
    u32 rating = gSaveBlock3Ptr->wayfarerHoenn.trainerRatingHighWater;

    rating = max(rating, CalculateLeagueCircuitTrainerRating());
    gSaveBlock3Ptr->wayfarerHoenn.trainerRatingHighWater = rating;
    return rating;
#else
    u8 rating = ClampTrainerRating(VarGet(VAR_TRAINER_RATING));

#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
    rating = max(rating, CalculateLeagueCircuitTrainerRating());
#elif !IS_WAYFARER
    rating = max(rating, CalculateLegacyTrainerRating());
#endif
    if (VarGet(VAR_TRAINER_RATING) != rating)
        VarSet(VAR_TRAINER_RATING, rating);

    return rating;
#endif
}

void SetTrainerRating(u32 rating)
{
#if WAYFARER_V0_TRAINERS
    gSaveBlock3Ptr->wayfarerHoenn.trainerRatingHighWater = rating;
#else
    VarSet(VAR_TRAINER_RATING, ClampTrainerRating(rating));
#endif
}

void InitializeTrainerRatingForNewGame(void)
{
    SetTrainerRating(TRAINER_RATING_MIN);
}

void InitializeTrainerRatingForSaveMigration(void)
{
#if IS_WAYFARER
    InitializeTrainerRatingForNewGame();
#else
    SetTrainerRating(CalculateLegacyTrainerRating());
#endif
}

u8 GetTrainerRatingSoftLevelCap(void)
{
#if WAYFARER_V0_TRAINERS
    return EvaluateTrainerScaler(sTrainerRatingSoftLevelCapsV0, ARRAY_COUNT(sTrainerRatingSoftLevelCapsV0), GetTrainerRating(), FALSE);
#else
    u8 rating = GetTrainerRating();
    u8 i;

    for (i = 1; i < ARRAY_COUNT(sTrainerRatingSoftLevelCaps); i++)
    {
        u8 lowerRating = sTrainerRatingSoftLevelCaps[i - 1].rating;
        u8 upperRating = sTrainerRatingSoftLevelCaps[i].rating;
        u8 lowerCap = sTrainerRatingSoftLevelCaps[i - 1].cap;
        u8 upperCap = sTrainerRatingSoftLevelCaps[i].cap;

        if (rating <= upperRating)
        {
            u16 numerator = (rating - lowerRating) * (upperCap - lowerCap);
            u16 denominator = upperRating - lowerRating;
            return lowerCap + (2 * numerator + denominator) / (2 * denominator);
        }
    }

    return sTrainerRatingSoftLevelCaps[ARRAY_COUNT(sTrainerRatingSoftLevelCaps) - 1].cap;
#endif
}

u32 ApplyTrainerRatingExperienceReduction(u16 species, u32 currentExp, u32 awardedExp)
{
#if IS_WAYFARER
    u32 capExp = gExperienceTables[gSpeciesInfo[species].growthRate][GetTrainerRatingSoftLevelCap()];
    u32 fullExp = min(awardedExp, (currentExp < capExp) ? capExp - currentExp : 0);
    u32 reducedExp = awardedExp - fullExp;

    if (reducedExp != 0)
        reducedExp = max(1, reducedExp / 2);

    return fullExp + reducedExp;
#else
    (void)species;
    (void)currentExp;
    return awardedExp;
#endif
}

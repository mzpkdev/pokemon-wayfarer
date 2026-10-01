#ifndef GUARD_TRAINER_RATING_H
#define GUARD_TRAINER_RATING_H

#define TRAINER_RATING_MIN 0
#define TRAINER_RATING_MAX 80

u8 ClampTrainerRating(u16 rating);
u32 GetTrainerRating(void);
void SetTrainerRating(u32 rating);
void InitializeTrainerRatingForNewGame(void);
void InitializeTrainerRatingForSaveMigration(void);
u8 GetTrainerRatingSoftLevelCap(void);
u32 ApplyTrainerRatingExperienceReduction(u16 species, u32 currentExp, u32 awardedExp);

#endif // GUARD_TRAINER_RATING_H

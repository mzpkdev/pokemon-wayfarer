#ifndef GUARD_TRAINER_RATING_H
#define GUARD_TRAINER_RATING_H

#define TRAINER_RATING_MIN 10
#define TRAINER_RATING_MAX 80

u8 ClampTrainerRating(u16 rating);
u8 CalculateTrainerRatingFromCurrentFacts(void);
u8 GetTrainerRating(void);
void InitializeTrainerRatingForNewGame(void);
void InitializeTrainerRatingForSaveMigration(void);

#endif // GUARD_TRAINER_RATING_H

#ifndef GUARD_FISHING_H
#define GUARD_FISHING_H

void StartFishing(u8 rod);
void UpdateChainFishingStreak();
u32 CalculateChainFishingShinyRolls(void);
bool32 ShouldUseFishingEnvironmentInBattle();
u32 CalculateFishingBiteOddsWithBonuses(u32 rod, bool32 isStickyHold, u32 followerBoost, u32 proximityBoost, u32 timeOfDayBoost);

#endif // GUARD_FISHING_H

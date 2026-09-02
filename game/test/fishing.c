#include "global.h"
#include "fishing.h"
#include "test/test.h"
#include "constants/items.h"

TEST("Standard Rod: base bite probabilities remain 25 50 and 75 percent")
{
    EXPECT_EQ(CalculateFishingBiteOddsWithBonuses(OLD_ROD, FALSE, 0, 0, 0), 25);
    EXPECT_EQ(CalculateFishingBiteOddsWithBonuses(GOOD_ROD, FALSE, 0, 0, 0), 50);
    EXPECT_EQ(CalculateFishingBiteOddsWithBonuses(SUPER_ROD, FALSE, 0, 0, 0), 75);
}

TEST("Standard Rod: fishing bite modifiers retain the 100 percent cap")
{
    EXPECT_EQ(CalculateFishingBiteOddsWithBonuses(OLD_ROD, FALSE, 50, 50, 50), 100);
    EXPECT_EQ(CalculateFishingBiteOddsWithBonuses(SUPER_ROD, FALSE, 25, 0, 0), 100);
}

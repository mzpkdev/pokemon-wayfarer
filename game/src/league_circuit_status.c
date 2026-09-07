#include "global.h"
#include "league_circuit.h"
#include "league_circuit_status.h"
#include "string_util.h"
#include "wayfarer_persistence.h"
#include "constants/characters.h"

#if IS_WAYFARER
static const u8 sBadges[] = _("Badges: ");
static const u8 sBadgeMaximum[] = _("/24\n");
static const u8 sKanto[] = _("Kanto: ");
static const u8 sJohto[] = _("Johto: ");
static const u8 sHoenn[] = _("Hoenn: ");
static const u8 sKantoRequirement[] = _("  8 badges + no prior clear");
static const u8 sJohtoRequirement[] = _("  16 badges + Kanto clear");
static const u8 sHoennRequirement[] = _("  24 badges + Kanto + Johto clears");
static const u8 sLocked[] = _("Locked");
static const u8 sAvailable[] = _("Available");
static const u8 sCleared[] = _("Cleared");
static const u8 sComplete[] = _("Circuit complete");

static const u8 *GetLeagueStateText(enum Region region)
{
    if (GetChampionStateForRegion(region))
        return sCleared;
    if (IsEligibleForLeague(region))
        return sAvailable;
    return sLocked;
}

static u8 *AppendLeagueStatus(u8 *dest, enum Region region, const u8 *name, const u8 *requirement)
{
    dest = StringCopy(dest, name);
    dest = StringCopy(dest, GetLeagueStateText(region));
    *dest++ = CHAR_NEWLINE;
    return StringCopy(dest, requirement);
}

void FormatLeagueCircuitStatus(u8 *dest)
{
    u8 *ptr = StringCopy(dest, sBadges);

    ptr = ConvertIntToDecimalStringN(ptr, GetGlobalBadgeCount(), STR_CONV_MODE_LEFT_ALIGN, 2);
    ptr = StringCopy(ptr, sBadgeMaximum);
    ptr = AppendLeagueStatus(ptr, REGION_KANTO, sKanto, sKantoRequirement);
    *ptr++ = CHAR_NEWLINE;
    ptr = AppendLeagueStatus(ptr, REGION_JOHTO, sJohto, sJohtoRequirement);
    *ptr++ = CHAR_NEWLINE;
    ptr = AppendLeagueStatus(ptr, REGION_HOENN, sHoenn, sHoennRequirement);
    if (GetChampionStateForRegion(REGION_HOENN))
    {
        *ptr++ = CHAR_NEWLINE;
        StringCopy(ptr, sComplete);
    }
}
#endif

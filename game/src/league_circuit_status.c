#include "global.h"
#include "league_circuit.h"
#include "league_circuit_status.h"
#include "string_util.h"
#include "wayfarer_persistence.h"
#include "constants/characters.h"

#if IS_WAYFARER
static const u8 sBadges[] = _("Badges: ");
static const u8 sBadgeMaximum[] = _("/24\n");
static const u8 sIndigo[] = _("Indigo League: ");
static const u8 sMasters[] = _("Sevii Masters: ");
static const u8 sHoenn[] = _("Hoenn League: ");
static const u8 sIndigoRequirement[] = _("  8 badges");
static const u8 sMastersRequirement[] = _("  16 badges + Indigo clear");
static const u8 sHoennRequirement[] = _("  24 badges + Masters clear");
static const u8 sLocked[] = _("Locked");
static const u8 sAvailable[] = _("Available");
static const u8 sCleared[] = _("Cleared");
static const u8 sComplete[] = _("Circuit complete");

static const u8 *GetLeagueStateText(enum CircuitStage stage)
{
    if (HasClearedCircuitStage(stage))
        return sCleared;
    if (IsEligibleForCircuitStage(stage))
        return sAvailable;
    return sLocked;
}

static u8 *AppendLeagueStatus(u8 *dest, enum CircuitStage stage, const u8 *name, const u8 *requirement)
{
    dest = StringCopy(dest, name);
    dest = StringCopy(dest, GetLeagueStateText(stage));
    *dest++ = CHAR_NEWLINE;
    return StringCopy(dest, requirement);
}

void FormatLeagueCircuitStatus(u8 *dest)
{
    u8 *ptr = StringCopy(dest, sBadges);

    ptr = ConvertIntToDecimalStringN(ptr, GetGlobalBadgeCount(), STR_CONV_MODE_LEFT_ALIGN, 2);
    ptr = StringCopy(ptr, sBadgeMaximum);
    ptr = AppendLeagueStatus(ptr, CIRCUIT_STAGE_INDIGO, sIndigo, sIndigoRequirement);
    *ptr++ = CHAR_NEWLINE;
    ptr = AppendLeagueStatus(ptr, CIRCUIT_STAGE_MASTERS, sMasters, sMastersRequirement);
    *ptr++ = CHAR_NEWLINE;
    ptr = AppendLeagueStatus(ptr, CIRCUIT_STAGE_HOENN, sHoenn, sHoennRequirement);
    if (HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO)
     && HasClearedCircuitStage(CIRCUIT_STAGE_MASTERS)
     && HasClearedCircuitStage(CIRCUIT_STAGE_HOENN))
    {
        *ptr++ = CHAR_NEWLINE;
        StringCopy(ptr, sComplete);
    }
}
#endif

#include "global.h"
#include "league_circuit.h"
#include "league_circuit_status.h"
#include "string_util.h"
#include "constants/characters.h"

#if IS_WAYFARER
static const u8 sBadges[] = _("Badges: ");
static const u8 sCap[] = _("/24\nCertification cap: ");
static const u8 sNext[] = _("Next: ");
static const u8 sKanto[] = _("Kanto League");
static const u8 sJohto[] = _("Johto League");
static const u8 sHoenn[] = _("Hoenn League");
static const u8 sQualified[] = _("\nQualified!");
static const u8 sNeeded[] = _("\nBadge target: ");
static const u8 sComplete[] = _("Circuit complete");

void FormatLeagueCircuitStatus(u8 *dest, bool8 paged)
{
    enum Region region = GetRequiredLeagueRegion();
    u8 *ptr = StringCopy(dest, sBadges);

    ptr = ConvertIntToDecimalStringN(ptr, GetGlobalBadgeCount(), STR_CONV_MODE_LEFT_ALIGN, 2);
    ptr = StringCopy(ptr, sCap);
    ptr = ConvertIntToDecimalStringN(ptr, GetBadgeCertificationCap(), STR_CONV_MODE_LEFT_ALIGN, 2);
    *ptr++ = paged ? CHAR_PROMPT_CLEAR : CHAR_NEWLINE;
    if (region == REGION_NONE)
    {
        StringCopy(ptr, sComplete);
        return;
    }
    ptr = StringCopy(ptr, sNext);
    if (region == REGION_KANTO)
        ptr = StringCopy(ptr, sKanto);
    else if (region == REGION_JOHTO)
        ptr = StringCopy(ptr, sJohto);
    else
        ptr = StringCopy(ptr, sHoenn);
    if (IsEligibleForLeague(region))
        StringCopy(ptr, sQualified);
    else
    {
        ptr = StringCopy(ptr, sNeeded);
        ConvertIntToDecimalStringN(ptr, GetBadgeCertificationCap(), STR_CONV_MODE_LEFT_ALIGN, 2);
    }
}

void BufferLeagueCircuitStatus(void)
{
    FormatLeagueCircuitStatus(gStringVar4, TRUE);
}
#else
void BufferLeagueCircuitStatus(void)
{
    gStringVar4[0] = EOS;
}
#endif

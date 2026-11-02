#ifndef GUARD_WAYFARER_FERRY_H
#define GUARD_WAYFARER_FERRY_H

// The S.S. ferries (Wayfarer only): the schedule, the saved voyage state, boarding, sailing and
// going ashore. Spec: .product/specs/ss-ferries.md. Everything is C-side so Hoenn-bank and
// HnS-bank maps read one state through specials.

#include "gba/types.h"
#include "constants/wayfarer_ferry.h"

// The voyage state, saved in PokemonStorage after the daily slots. Packed: 12 bytes (spec: at most 16).
struct WayfarerVoyage
{
    u32 eventAt;        // in-game minute count of the next cast-off (in port) or arrival (at sea)
    u16 slotDay;        // the daily-slot day frozen at boarding; DailySlots_GetDay returns it while aboard
    u8 hosts[2];        // the two cabin hosts drawn at boarding: host table index plus one
    u8 hostParams[2];   // each host's drawn request parameter: an index into the host's list
    u8 ship:2;          // FERRY_SHIP_*, none when not aboard
    u8 stop:2;          // index in the ship's loop of the port it's at or sailing to
    u8 phase:1;         // FERRY_PHASE_PORT or FERRY_PHASE_SEA
    u8 annePending:1;   // the Anne's first departure from Vermilion is armed
    u8 cabin:2;         // bit n set: host n's cabin is earned
    u8 hostsDone:2;     // bit n set: host n's request is settled
};

#if IS_WAYFARER

// ---- The schedule ----
u8 Ferry_ScheduledShip(u8 port, u8 hour);   // the table alone
bool8 Ferry_ShipInService(u8 ship);
u8 Ferry_ShipAt(u8 port, u8 hour);          // the table with ships not in service returned as none
bool8 Ferry_IsMaidenVoyagePending(void);
const u8 *Ferry_GetShipName(u8 ship);
u8 Ferry_GetPortStop(u8 ship, u8 port);     // index of the port in the ship's loop, 0xFF if not on it
u8 Ferry_GetLoopPort(u8 ship, u8 stop);

// ---- Maps ----
u8 Ferry_GetPortOfMap(s16 mapGroup, s16 mapNum);   // FERRY_PORT_NONE unless a berth map
bool8 Ferry_IsShipMap(s16 mapGroup, s16 mapNum);
u8 Ferry_GetShipOfMap(s16 mapGroup, s16 mapNum);

// ---- Voyage state ----
const struct WayfarerVoyage *Ferry_GetVoyage(void);
bool8 Ferry_IsAboard(void);
bool8 Ferry_IsAtSea(void);
u32 Ferry_GetNow(void);
void Ferry_InitNewGame(void);
void Ferry_ClearVoyage(void);              // going ashore: zeroes the whole state, annePending included
void Ferry_OnBlackout(void);
bool8 Ferry_BoardAt(u8 port, u8 ship, u32 now);
u8 Ferry_Advance(u32 now);                 // applies every elapsed transition; returns what to announce
bool8 Ferry_ArmAnneDeparture(void);        // the captain's reward

// ---- Cabins (wayfarer_ferry_cabins.c) ----
void Ferry_DrawHosts(u8 ship);             // boarding: two distinct hosts of the ship's pool with their parameters
void Ferry_PlaceHosts(void);               // map load: the drawn hosts' spots take their graphics, the rest hide
void Ferry_BufferPortName(u8 port);        // STR_VAR_1
bool8 Ferry_RequestMet(u8 kind, u16 param);
bool8 Ferry_SleepUntilEvent(void);         // at sea: the clock to eventAt, no further; FALSE in port
u16 Ferry_HostTalk(void);
u16 Ferry_GetHostKind(void);
u16 Ferry_CheckRequest(void);
void Ferry_HostMet(void);
void Ferry_HostMissed(void);
u16 Ferry_IsHostOneTry(void);
u16 Ferry_SetUpHostBattle(void);
u16 Ferry_EndHostBattle(void);
void Ferry_BufferQuiz(void);
u16 Ferry_QuizCorrect(void);
void Ferry_BufferSleepPort(void);
u16 Ferry_Sleep(void);
u16 Ferry_IsBedMine(void);

// ---- Field hooks (overworld.c, field_control_avatar.c) ----
void Ferry_OnMapLoad(void);
bool8 Ferry_TryStartFieldEvent(void);
void Ferry_OnMinute(void);

// ---- Scripts: berths and boarding ----
u16 Ferry_GetBerthShip(void);              // the ship the sailor here offers now, FERRY_SHIP_NONE if none
u16 Ferry_GetBerthPort(void);
bool8 Ferry_BerthNeedsRespawn(u8 shownPlusOne, u8 ship);
u16 Ferry_BufferNextShip(void);            // STR_VAR_1 ship, STR_VAR_2 hour; FALSE if no ship ever calls
u16 Ferry_BufferBoarding(void);            // STR_VAR_1 ship, STR_VAR_2 next stop
u16 Ferry_Board(void);
void Ferry_WarpAboard(void);
void Ferry_RefreshAnneWarp(void);

// ---- Scripts: ashore ----
u16 Ferry_IsAtSeaForScript(void);
u16 Ferry_IsAboardForScript(void);
u16 Ferry_CanDisembark(void);
void Ferry_BufferArrivalPort(void);
u16 Ferry_Disembark(void);
u16 Ferry_IsAnnePendingAtVermilion(void);
void Ferry_ArmAnneDepartureScript(void);

// Debug/test accessors.
#if TESTING
void Test_Ferry_SetVoyage(const struct WayfarerVoyage *voyage);
u8 Test_Ferry_HostCount(void);
u8 Test_Ferry_HostShip(u8 index);
u8 Test_Ferry_HostKind(u8 index);
u8 Test_Ferry_HostParamCount(u8 index);
u8 Test_Ferry_FindHost(u8 ship, u8 kind);
void Test_Ferry_StandBeforeHost(u8 index);
#endif

#endif // IS_WAYFARER
#endif // GUARD_WAYFARER_FERRY_H

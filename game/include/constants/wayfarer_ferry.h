#ifndef GUARD_CONSTANTS_WAYFARER_FERRY_H
#define GUARD_CONSTANTS_WAYFARER_FERRY_H

// The S.S. ferries (.product/specs/ss-ferries.md). Plain defines so scripts and C share them.

// Ships. 0 is "none" so a zeroed voyage state is "not aboard".
#define FERRY_SHIP_NONE   0
#define FERRY_SHIP_AQUA   1
#define FERRY_SHIP_ANNE   2
#define FERRY_SHIP_TIDAL  3

// Ports.
#define FERRY_PORT_OLIVINE      0
#define FERRY_PORT_VERMILION    1
#define FERRY_PORT_SLATEPORT    2
#define FERRY_PORT_LILYCOVE     3
#define FERRY_PORT_ONE_ISLAND   4
#define FERRY_PORT_SEVEN_ISLAND 5
#define FERRY_PORT_MELEMELE     6
#define FERRY_PORT_COUNT        7
#define FERRY_PORT_NONE         0xFF

#define FERRY_LOOP_LENGTH       4

// Voyage phase.
#define FERRY_PHASE_PORT  0
#define FERRY_PHASE_SEA   1

// Minutes of in-game time: half an hour in port, six hours at sea.
#define FERRY_PORT_MINUTES  30
#define FERRY_SEA_MINUTES   360

// What a field poll announced, for the script that plays it.
#define FERRY_ANNOUNCE_NONE      0
#define FERRY_ANNOUNCE_CAST_OFF  1
#define FERRY_ANNOUNCE_ARRIVAL   2

// Lilycove's menu: the ship in port, the event-island charters and Cancel.
#define FERRY_CHARTER_SHIP             0
#define FERRY_CHARTER_SOUTHERN_ISLAND  1
#define FERRY_CHARTER_FARAWAY_ISLAND   2
#define FERRY_CHARTER_BATTLE_FRONTIER  3
#define FERRY_CHARTER_CANCEL           4

// How a berth lets you aboard.
#define FERRY_BERTH_DOCK    0
#define FERRY_BERTH_LAUNCH  1

#endif // GUARD_CONSTANTS_WAYFARER_FERRY_H

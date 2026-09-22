#include "constants/global.h"
#include "constants/layouts.h"
#include "constants/map_scripts.h"
#include "constants/map_types.h"
#include "constants/maps.h"
#include "constants/metatile_labels.h"
#include "constants/weather.h"
#include "constants/region_map_sections.h"
#include "constants/songs.h"
#include "constants/trainer_hill.h"
	.include "asm/macros.inc"
	.include "asm/macros/event.inc"
	.include "constants/constants.inc"
	.set MAP_LAYOUT_TESTING_ASM, TESTING

	.section .rodata

	.include "data/layouts/layouts.inc"
	.include "data/layouts/layouts_table.inc"
	.include "data/maps/headers.inc"
	.include "data/maps/groups.inc"
	.include "data/maps/connections.inc"

#if HAS_HNS_CONTENT && !IS_WAYFARER
	.include "data/maps/CinnabarIsland_SeamPoc/scripts.inc"
	.include "data/maps/frlg_coast_overworld_poc_scripts.inc"
	.include "data/maps/cinnabar_service_port_scripts.inc"
	.include "data/maps/cinnabar_gym_mansion_port_scripts.inc"
#endif

# Map JSON data

# Inputs
MAPS_DIR = $(DATA_ASM_SUBDIR)/maps
LAYOUTS_DIR = $(DATA_ASM_SUBDIR)/layouts

# Outputs
MAPS_OUTDIR := $(MAPS_DIR)
LAYOUTS_OUTDIR := $(LAYOUTS_DIR)
INCLUDECONSTS_OUTDIR := include/constants
# Standalone products historically compile individual maps and event constants
# with the Emerald schema. Only the composite catalog needs Wayfarer's source-
# aware schema extensions.
MAP_DETAIL_VERSION := $(if $(filter wayfarer,$(MAP_VERSION)),wayfarer,emerald)
WAYFARER_SOURCE_CONSTANTS_TOOL := $(TOOLS_DIR)/wayfarer_source_constants/generate.py
WAYFARER_HOENN_SOURCE_CONSTANTS := $(DATA_ASM_SUBDIR)/wayfarer_hoenn_source_constants.inc
WAYFARER_ENGINE_SOURCE_CONSTANTS := $(DATA_ASM_SUBDIR)/wayfarer_engine_source_constants.inc
WAYFARER_COMMON_SOURCE_CONSTANTS := $(DATA_ASM_SUBDIR)/wayfarer_common_source_constants.inc
WAYFARER_COMMON_SOURCE_DATA := $(DATA_SRC_SUBDIR)/wayfarer_common_source_constants.h
WAYFARER_MAP_SOURCES := $(DATA_SRC_SUBDIR)/wayfarer_map_sources.h
WAYFARER_SEVII_MANIFEST := $(DATA_SRC_SUBDIR)/wayfarer_sevii_maps.json
WAYFARER_SEVII_MANIFEST_ARG := $(if $(filter wayfarer,$(MAP_VERSION)),--wayfarer-sevii-manifest $(WAYFARER_SEVII_MANIFEST))
WAYFARER_SEVII_SCRIPT_TOOL := $(TOOLS_DIR)/wayfarer_sevii_scripts/generate.py
WAYFARER_SEVII_EVENT_SCRIPTS := $(DATA_ASM_SUBDIR)/wayfarer_sevii_event_scripts.inc

# The map sources themselves are shared between versions, so a version switch
# must still invalidate their generated includes.  Do not make that invalidation
# unconditional, though: `generated` runs before scaninc reads the dependency
# files, and a FORCE edge here used to remake map_groups.h on every invocation.
# The resulting timestamp churn invalidated hundreds of otherwise-current .d
# files.  Evaluate the content once and only add FORCE when the selected map
# version actually differs from the recorded value.
MAP_VERSION_STAMP := .map_version
MAP_VERSION_STAMP_NEEDS_UPDATE := $(shell test -r $(MAP_VERSION_STAMP) && test "`cat $(MAP_VERSION_STAMP)`" = "$(MAP_VERSION)" || echo yes)
ifneq ($(MAP_VERSION_STAMP_NEEDS_UPDATE),)
$(MAP_VERSION_STAMP): FORCE
endif

AUTO_GEN_TARGETS += $(INCLUDECONSTS_OUTDIR)/map_groups.h
AUTO_GEN_TARGETS += $(INCLUDECONSTS_OUTDIR)/layouts.h
AUTO_GEN_TARGETS += $(INCLUDECONSTS_OUTDIR)/map_event_ids.h
AUTO_GEN_TARGETS += $(DATA_SRC_SUBDIR)/map_group_count.h
AUTO_GEN_TARGETS += $(WAYFARER_HOENN_SOURCE_CONSTANTS)
AUTO_GEN_TARGETS += $(WAYFARER_ENGINE_SOURCE_CONSTANTS)
AUTO_GEN_TARGETS += $(WAYFARER_COMMON_SOURCE_CONSTANTS)
AUTO_GEN_TARGETS += $(WAYFARER_COMMON_SOURCE_DATA)
AUTO_GEN_TARGETS += $(WAYFARER_MAP_SOURCES)

MAP_DIRS := $(dir $(wildcard $(MAPS_DIR)/*/map.json))
MAP_CONNECTIONS := $(patsubst $(MAPS_DIR)/%/,$(MAPS_DIR)/%/connections.inc,$(MAP_DIRS))
MAP_EVENTS := $(patsubst $(MAPS_DIR)/%/,$(MAPS_DIR)/%/events.inc,$(MAP_DIRS))
MAP_HEADERS := $(patsubst $(MAPS_DIR)/%/,$(MAPS_DIR)/%/header.inc,$(MAP_DIRS))
MAP_JSONS := $(patsubst $(MAPS_DIR)/%/,$(MAPS_DIR)/%/map.json,$(MAP_DIRS))
MAP_GROUP_OUTPUTS := $(MAPS_OUTDIR)/connections.inc $(MAPS_OUTDIR)/groups.inc $(MAPS_OUTDIR)/events.inc $(MAPS_OUTDIR)/headers.inc $(INCLUDECONSTS_OUTDIR)/map_groups.h $(DATA_SRC_SUBDIR)/map_group_count.h $(WAYFARER_MAP_SOURCES)
MAP_LAYOUT_OUTPUTS := $(LAYOUTS_OUTDIR)/layouts.inc $(LAYOUTS_OUTDIR)/layouts_table.inc $(INCLUDECONSTS_OUTDIR)/layouts.h

$(DATA_ASM_BUILDDIR)/maps.o: $(DATA_ASM_SUBDIR)/maps.s $(LAYOUTS_DIR)/layouts.inc $(LAYOUTS_DIR)/layouts_table.inc $(MAPS_DIR)/headers.inc $(MAPS_DIR)/groups.inc $(MAPS_DIR)/connections.inc $(MAP_CONNECTIONS) $(MAP_HEADERS)
	$(PREPROC) $< charmap.txt | $(CPP) $(CPPFLAGS) -I include - | $(PREPROC) -ie $< charmap.txt | $(AS) $(ASFLAGS) -o $@
$(DATA_ASM_BUILDDIR)/event_scripts.o: $(WAYFARER_HOENN_SOURCE_CONSTANTS) $(WAYFARER_ENGINE_SOURCE_CONSTANTS) $(WAYFARER_COMMON_SOURCE_CONSTANTS) $(WAYFARER_SEVII_EVENT_SCRIPTS)

$(WAYFARER_SEVII_EVENT_SCRIPTS): $(WAYFARER_SEVII_SCRIPT_TOOL) $(WAYFARER_SEVII_MANIFEST) $(wildcard $(DATA_ASM_SUBDIR)/scripts/wayfarer_sevii/*.inc)
	python3 $(WAYFARER_SEVII_SCRIPT_TOOL) --root .
$(DATA_ASM_BUILDDIR)/map_events.o: $(DATA_ASM_SUBDIR)/map_events.s $(MAPS_DIR)/events.inc $(MAP_EVENTS) $(WAYFARER_HOENN_SOURCE_CONSTANTS) $(WAYFARER_ENGINE_SOURCE_CONSTANTS)
	$(PREPROC) $< charmap.txt | $(CPP) $(CPPFLAGS) -I include - | $(PREPROC) -ie $< charmap.txt | $(AS) $(ASFLAGS) -o $@

$(WAYFARER_HOENN_SOURCE_CONSTANTS) $(WAYFARER_ENGINE_SOURCE_CONSTANTS) $(WAYFARER_COMMON_SOURCE_CONSTANTS) $(WAYFARER_COMMON_SOURCE_DATA) &: $(WAYFARER_SOURCE_CONSTANTS_TOOL) $(wildcard $(INCLUDECONSTS_OUTDIR)/flags*.h) $(wildcard $(INCLUDECONSTS_OUTDIR)/vars*.h) $(INCLUDECONSTS_OUTDIR)/global.h $(INCLUDECONSTS_OUTDIR)/region_map_sections.h $(DATA_ASM_SUBDIR)/event_scripts.s $(wildcard $(DATA_ASM_SUBDIR)/scripts/*.inc) $(MAP_JSONS)
	python3 $(WAYFARER_SOURCE_CONSTANTS_TOOL) --cpp $(CPP) --cc $${HOSTCC:-cc} --include-dir include --hoenn-output $(WAYFARER_HOENN_SOURCE_CONSTANTS) --engine-output $(WAYFARER_ENGINE_SOURCE_CONSTANTS) --common-output $(WAYFARER_COMMON_SOURCE_CONSTANTS) --common-data-output $(WAYFARER_COMMON_SOURCE_DATA) --event-scripts $(DATA_ASM_SUBDIR)/event_scripts.s --scripts-dir $(DATA_ASM_SUBDIR)/scripts --maps-dir $(MAPS_DIR)

$(MAPS_OUTDIR)/%/header.inc $(MAPS_OUTDIR)/%/events.inc $(MAPS_OUTDIR)/%/connections.inc &: $(MAPS_DIR)/%/map.json $(INCLUDECONSTS_OUTDIR)/map_groups.h $(MAPJSON) $(WAYFARER_SEVII_MANIFEST)
	$(MAPJSON) map $(MAP_DETAIL_VERSION) $< $(LAYOUTS_DIR)/layouts.json $(@D) $(WAYFARER_SEVII_MANIFEST_ARG)
	@touch $(@D)/header.inc $(@D)/events.inc $(@D)/connections.inc


$(MAP_GROUP_OUTPUTS) &: $(MAPS_DIR)/map_groups.json $(MAP_JSONS) $(MAP_VERSION_STAMP) $(MAPJSON) $(WAYFARER_SEVII_MANIFEST)
	@$(MAPJSON) groups $(MAP_VERSION) $(filter-out $(MAP_VERSION_STAMP) $(MAPJSON) $(WAYFARER_SEVII_MANIFEST),$^) $(MAPS_OUTDIR) $(INCLUDECONSTS_OUTDIR) $(WAYFARER_SEVII_MANIFEST_ARG)
	@echo "$(MAPJSON) groups $(MAP_VERSION) $(MAPS_DIR)/map_groups.json <MAP_JSONS> $(MAPS_OUTDIR) $(INCLUDECONSTS_OUTDIR)"
	@touch $(MAP_GROUP_OUTPUTS)

$(MAP_LAYOUT_OUTPUTS) &: $(LAYOUTS_DIR)/layouts.json $(MAP_VERSION_STAMP) $(MAPJSON) $(WAYFARER_SEVII_MANIFEST)
	$(MAPJSON) layouts $(MAP_VERSION) $< $(LAYOUTS_OUTDIR) $(INCLUDECONSTS_OUTDIR) $(WAYFARER_SEVII_MANIFEST_ARG)
	@touch $(MAP_LAYOUT_OUTPUTS)

# Generate constants for map events, which depend on data that's distributed across the map.json files.
# There's a lot of map.json files, so we print an abbreviated output with echo.
$(INCLUDECONSTS_OUTDIR)/map_event_ids.h: $(MAP_JSONS) $(MAPJSON)
	@$(MAPJSON) event_constants $(MAP_DETAIL_VERSION) $(filter-out $(MAPJSON),$^) $(INCLUDECONSTS_OUTDIR)/map_event_ids.h
	@echo "$(MAPJSON) event_constants $(MAP_DETAIL_VERSION) <MAP_JSONS> $(INCLUDECONSTS_OUTDIR)/map_event_ids.h"
	@touch $@

$(MAP_VERSION_STAMP):
	@(echo "$(MAP_VERSION)" | cmp $@ -) || echo "$(MAP_VERSION)" > .map_version

FORCE:
.PHONY : FORCE

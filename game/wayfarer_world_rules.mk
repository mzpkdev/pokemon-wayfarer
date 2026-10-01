# Notable world simulation: walker graph, spot table and trainer tables
# (tools/wayfarer_world, .product/specs/notable-world-simulation.md). Only the
# Wayfarer map version compiles them; the generator reads that version's
# mapjson outputs, so this fragment follows map_data_rules.mk.

WAYFARER_WORLD_TOOL := $(TOOLS_DIR)/wayfarer_world/generate.py
WAYFARER_WORLD_TABLES := $(DATA_SRC_SUBDIR)/wayfarer_world/tables.h
WAYFARER_WORLD_REPORT := $(BUILD_DIR)/wayfarer-world-report.json

ifeq ($(MAP_VERSION),wayfarer)
# The generator lists its static inputs; a failed query must stop Make rather
# than leave the tables without prerequisites.
WAYFARER_WORLD_INPUTS := $(shell python3 $(WAYFARER_WORLD_TOOL) --root . --print-inputs || echo WAYFARER_WORLD_INPUT_ERROR)
ifneq (,$(filter WAYFARER_WORLD_INPUT_ERROR,$(WAYFARER_WORLD_INPUTS)))
$(error Cannot list the Wayfarer world generator's inputs)
endif
AUTO_GEN_TARGETS += $(WAYFARER_WORLD_TABLES)

$(WAYFARER_WORLD_TABLES): $(WAYFARER_WORLD_INPUTS) $(MAP_EVENTS) $(MAP_CONNECTIONS) $(MAP_HEADERS) $(MAPS_DIR)/groups.inc $(INCLUDECONSTS_OUTDIR)/map_groups.h $(INCLUDECONSTS_OUTDIR)/map_event_ids.h $(MAP_VERSION_STAMP)
	PYTHONDONTWRITEBYTECODE=1 python3 $(WAYFARER_WORLD_TOOL) --root . --output $@ --report $(WAYFARER_WORLD_REPORT)
	@touch $@
endif

.PHONY: wayfarer-world-test
wayfarer-world-test:
	PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tools/wayfarer_world -p 'test_*.py' -q


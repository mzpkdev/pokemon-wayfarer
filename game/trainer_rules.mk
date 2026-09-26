# party files are run through trainerproc, which is a tool that converts party data to an output file
# matching the current trainer .h formatting

AUTO_GEN_TARGETS += src/data/trainers.h
AUTO_GEN_TARGETS += src/data/trainers_frlg.h
AUTO_GEN_TARGETS += src/data/trainers_hns.h
AUTO_GEN_TARGETS += src/data/battle_partners.h
AUTO_GEN_TARGETS += test/battle/trainer_control.h
AUTO_GEN_TARGETS += test/battle/partner_control.h
AUTO_GEN_TARGETS += src/data/debug_trainers.h
AUTO_GEN_TARGETS += test/league_tiers.h
AUTO_GEN_TARGETS += include/constants/wayfarer_sevii_trainers.h
AUTO_GEN_TARGETS += src/data/trainers_wayfarer_sevii.h
AUTO_GEN_TARGETS += include/wayfarer_sevii_trainer_defeats.h
AUTO_GEN_TARGETS += include/constants/wayfarer_coast_trainers.h
AUTO_GEN_TARGETS += src/data/trainers_wayfarer_coast.h
AUTO_GEN_TARGETS += include/wayfarer_coast_trainer_defeats.h
AUTO_GEN_TARGETS += include/constants/wayfarer_ss_anne_trainers.h
AUTO_GEN_TARGETS += src/data/trainers_wayfarer_ss_anne.h
AUTO_GEN_TARGETS += include/wayfarer_ss_anne_trainer_defeats.h
AUTO_GEN_TARGETS += src/data/trainers_wayfarer_local.h
AUTO_GEN_TARGETS += src/data/trainers_wayfarer_kanto.h

WAYFARER_SEVII_TRAINER_GENERATOR := tools/wayfarer_sevii_trainers/generate.py
WAYFARER_SEVII_TRAINER_OUTPUTS := include/constants/wayfarer_sevii_trainers.h src/data/trainers_wayfarer_sevii.h include/wayfarer_sevii_trainer_defeats.h
WAYFARER_SEVII_TRAINER_DEPS := $(WAYFARER_SEVII_TRAINER_GENERATOR) ../docs/sevii-trainer-implementation/allocation-inventory.json src/data/trainers_frlg.party data/scripts/trainers_frlg.inc $(shell find data/maps -type f \( -name map.json -o -name scripts.inc \)) tools/trainerproc/main.c tools/trainer_scaling/generate.py

# This is deliberately a selected-roster generator, not another trainerproc
# input: source FRLG numeric identities must not enter Wayfarer's runtime table.
$(WAYFARER_SEVII_TRAINER_OUTPUTS) &: $(WAYFARER_SEVII_TRAINER_DEPS)
	PYTHONDONTWRITEBYTECODE=1 python3 $(WAYFARER_SEVII_TRAINER_GENERATOR)

WAYFARER_COAST_TRAINER_GENERATOR := tools/wayfarer_coast_trainers/generate.py
WAYFARER_COAST_TRAINER_OUTPUTS := include/constants/wayfarer_coast_trainers.h src/data/trainers_wayfarer_coast.h include/wayfarer_coast_trainer_defeats.h
WAYFARER_COAST_TRAINER_DEPS := $(WAYFARER_COAST_TRAINER_GENERATOR) src/data/trainers_frlg.party tools/trainerproc/main.c tools/trainer_scaling/generate.py

$(WAYFARER_COAST_TRAINER_OUTPUTS) &: $(WAYFARER_COAST_TRAINER_DEPS)
	PYTHONDONTWRITEBYTECODE=1 python3 $(WAYFARER_COAST_TRAINER_GENERATOR)

WAYFARER_SS_ANNE_TRAINER_GENERATOR := tools/wayfarer_ss_anne_trainers/generate.py
WAYFARER_SS_ANNE_TRAINER_OUTPUTS := include/constants/wayfarer_ss_anne_trainers.h src/data/trainers_wayfarer_ss_anne.h include/wayfarer_ss_anne_trainer_defeats.h
WAYFARER_SS_ANNE_TRAINER_DEPS := $(WAYFARER_SS_ANNE_TRAINER_GENERATOR) src/data/trainers_frlg.party tools/trainerproc/main.c tools/trainer_scaling/generate.py

$(WAYFARER_SS_ANNE_TRAINER_OUTPUTS) &: $(WAYFARER_SS_ANNE_TRAINER_DEPS)
	PYTHONDONTWRITEBYTECODE=1 python3 $(WAYFARER_SS_ANNE_TRAINER_GENERATOR)

src/data/trainers.h test/league_tiers.h: $(LEARNSET_HELPERS_BUILD_VERSION)

test/league_tiers.h: tools/wayfarer_league_tiers/generate_fixture.py tools/wayfarer_league_tiers/inventory.json src/data/trainers.party src/data/trainers_hns.party $(TRAINERPROC)
	python3 tools/wayfarer_league_tiers/generate_fixture.py | $(CPP) $(CPPFLAGS) -traditional-cpp - | $(TRAINERPROC) -o $@ -i tools/wayfarer_league_tiers/generate_fixture.py -

%.h: %.party $(TRAINERPROC)
	$(CPP) $(CPPFLAGS) -traditional-cpp - < $< | $(TRAINERPROC) -o $@ -i $< -

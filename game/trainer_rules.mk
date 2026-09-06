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

src/data/trainers.h test/league_tiers.h: $(LEARNSET_HELPERS_BUILD_VERSION)

test/league_tiers.h: tools/wayfarer_league_tiers/generate_fixture.py tools/wayfarer_league_tiers/inventory.json src/data/trainers.party src/data/trainers_hns.party $(TRAINERPROC)
	python3 tools/wayfarer_league_tiers/generate_fixture.py | $(CPP) $(CPPFLAGS) -traditional-cpp - | $(TRAINERPROC) -o $@ -i tools/wayfarer_league_tiers/generate_fixture.py -

%.h: %.party $(TRAINERPROC)
	$(CPP) $(CPPFLAGS) -traditional-cpp - < $< | $(TRAINERPROC) -o $@ -i $< -

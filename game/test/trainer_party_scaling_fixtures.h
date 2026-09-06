#if IS_WAYFARER
[DIFFICULTY_NORMAL][TRAINER_JOEY_2_HNS] = {
    .trainerName = _("SCALING"),
    .trainerClass = TRAINER_CLASS_YOUNGSTER,
    .partySize = 3,
    .party = (const struct TrainerMon[]) {
        {
            .species = SPECIES_CHARIZARD, .lvl = 60, .ability = ABILITY_BLAZE,
            .gender = TRAINER_MON_FEMALE, .nature = NATURE_ADAMANT,
            .heldItem = ITEM_LEFTOVERS, .friendship = 42, .ball = BALL_MASTER,
            .teraType = TYPE_FIRE, .dynamaxLevel = 10, .shouldUseDynamax = TRUE, .gigantamaxFactor = TRUE,
            .iv = TRAINER_PARTY_IVS(21, 22, 23, 24, 25, 26),
            .ev = TRAINER_PARTY_EVS(20, 24, 28, 32, 36, 40),
            .moves = { MOVE_HYPER_BEAM, MOVE_FLY, MOVE_EARTHQUAKE, MOVE_PROTECT },
        },
        { .species = SPECIES_CHARIZARD, .lvl = 60, .ability = ABILITY_BLAZE },
        { .species = SPECIES_SCYTHER, .lvl = 5, .ability = ABILITY_SWARM },
    },
},
[DIFFICULTY_NORMAL][TRAINER_JOEY_3_HNS] = {
    .trainerName = _("ALIAS"), .trainerClass = TRAINER_CLASS_YOUNGSTER,
    .overrideTrainer = TRAINER_JOEY_2_HNS,
},
[DIFFICULTY_NORMAL][TRAINER_JOEY_4_HNS] = {
    .trainerName = _("POOL"), .trainerClass = TRAINER_CLASS_YOUNGSTER,
    .partySize = 2, .poolSize = 4,
    .party = (const struct TrainerMon[]) {
        { .species = SPECIES_RATTATA, .lvl = 5 },
        { .species = SPECIES_CATERPIE, .lvl = 20 },
        { .species = SPECIES_CHARIZARD, .lvl = 60, .tags = MON_POOL_TAG_LEAD },
        { .species = SPECIES_SCYTHER, .lvl = 40, .tags = MON_POOL_TAG_ACE },
    },
},
[DIFFICULTY_NORMAL][TRAINER_JOEY_5_HNS] = {
    .trainerName = _("NORMAL"), .trainerClass = TRAINER_CLASS_YOUNGSTER,
    .partySize = 1,
    .party = (const struct TrainerMon[]) {{ .species = SPECIES_RATTATA, .lvl = 5 }},
},
[DIFFICULTY_HARD][TRAINER_JOEY_5_HNS] = {
    .trainerName = _("HARD"), .trainerClass = TRAINER_CLASS_YOUNGSTER,
    .partySize = 1,
    .party = (const struct TrainerMon[]) {{ .species = SPECIES_CHARIZARD, .lvl = 60 }},
},
[DIFFICULTY_NORMAL][TRAINER_ROD_HNS] = {
    .trainerName = _("GYM"), .trainerClass = TRAINER_CLASS_YOUNGSTER,
    .partySize = 1,
    .party = (const struct TrainerMon[]) {{
        .species = SPECIES_RATTATA, .lvl = 5, .teraType = TYPE_NORMAL,
        .dynamaxLevel = 10, .shouldUseDynamax = TRUE,
    }},
},
[DIFFICULTY_NORMAL][TRAINER_FALKNER_1_HNS] = {
    .trainerName = _("BOSS"), .trainerClass = TRAINER_CLASS_YOUNGSTER,
    .partySize = 1,
    .party = (const struct TrainerMon[]) {{
        .species = SPECIES_CHARIZARD, .lvl = 60,
        .moves = { MOVE_HYPER_BEAM, MOVE_FLY, MOVE_EARTHQUAKE, MOVE_PROTECT },
    }},
},
#endif

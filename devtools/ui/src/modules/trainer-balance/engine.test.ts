import { describe, expect, it } from "vitest"

import catalogData from "./catalog.json"
import {
  DEFAULT_REGULAR_TRAINER_LEVEL,
  DEFAULT_TEAM_SIZE,
  DEFAULT_WILD_LEVEL,
  LEVEL_CAP_ANCHORS,
  buildTeam,
  createExperiment,
  leagueLineup,
  levelCap,
  playerRating,
  resolveTrainer,
  rosterGaps,
  scale,
  serializeExperiment,
  teamLevelFor,
  teamSizeFor,
  validateExperiment,
  validateRoster,
  worldLevels,
} from "./engine.js"
import type { Experiment, RosterSlot, TrainerRecord } from "./types.js"

const catalog = catalogData as TrainerRecord[]
const slot = (species: string, levelOffset = -2): RosterSlot => ({
  species,
  levelOffset,
  moves: "LEVEL_UP",
  item: null,
  ability: null,
  nature: null,
})
const six = ["Steelix", "Golem", "Kabutops", "Omastar", "Aerodactyl", "Rhydon"]
const trainer = (id: string, tr: number, species = six): TrainerRecord => ({
  id,
  name: id,
  region: "Kanto",
  role: "Gym Leader",
  source: { label: "Fixture", path: "fixture", trainerId: "TEST", note: "Test data" },
  referenceParty: [],
  tr,
  trSource: "Fixture",
  roster: species.map((name, index) => slot(name, index === 0 ? 0 : -2)),
  rosterSource: "Fixture",
})
const experimentWith = (records: TrainerRecord[]): Experiment => createExperiment(records)

describe("scalers", () => {
  it("interpolates linearly between anchors with halves rounded up", () => {
    const anchors = LEVEL_CAP_ANCHORS
    expect(scale(anchors, 0)).toBe(15)
    expect(scale(anchors, 1)).toBe(15) // 15.325
    expect(scale(anchors, 2)).toBe(16) // 15.65
    expect(scale(anchors, 100)).toBe(63) // 62.5 rounds up
    expect(scale(anchors, 90)).toBe(56) // 56.25
    expect(scale(anchors, 95)).toBe(59) // 59.375
  })

  it.each([
    ["level cap", LEVEL_CAP_ANCHORS, [15, 22, 28, 39, 50, 63, 75, 88, 100]],
    ["wild level curve", DEFAULT_WILD_LEVEL, [6, 15, 24, 32, 40, 49, 58, 68, 78]],
    [
      "regular trainer level curve",
      DEFAULT_REGULAR_TRAINER_LEVEL,
      [9, 18, 27, 36, 44, 53, 62, 72, 82],
    ],
  ])("reads the %s at anchors 0/40/80/120/160 and midpoints", (_name, anchors, levels) => {
    expect([0, 20, 40, 60, 80, 100, 120, 140, 160].map((tr) => scale(anchors, tr))).toEqual(levels)
    expect(scale(anchors, 400)).toBe(levels.at(-1))
  })

  it("stays flat past the last anchor and never clamps TR", () => {
    const experiment = experimentWith([trainer("fixture", 0)])
    expect(teamLevelFor(experiment, 120)).toBe(75)
    expect(teamLevelFor(experiment, 160)).toBe(100)
    expect(teamLevelFor(experiment, 240)).toBe(100)
    expect(teamSizeFor(experiment, 120)).toBe(6)
    expect(teamLevelFor(experiment, 1_000_000)).toBe(100)
    // A scaler with a later ceiling TR keeps rising past TR 160: TR is not capped.
    expect(
      scale(
        [
          [0, 10],
          [200, 110],
        ],
        150,
      ),
    ).toBe(85)
  })

  it("uses the level cap anchors for team level", () => {
    const experiment = experimentWith([trainer("fixture", 0)])
    for (let tr = 0; tr <= 200; tr += 1)
      expect(teamLevelFor(experiment, tr)).toBe(scale(LEVEL_CAP_ANCHORS, tr))
  })

  it("steps team size: 0–15 -> 2, 16–43 -> 3, 44–70 -> 4, 71–95 -> 5, 96+ -> 6", () => {
    const sizes = (trs: number[]) => trs.map((tr) => scale(DEFAULT_TEAM_SIZE, tr))
    expect(sizes([0, 8, 15])).toEqual([2, 2, 2])
    expect(sizes([16, 30, 43])).toEqual([3, 3, 3])
    expect(sizes([44, 70])).toEqual([4, 4])
    expect(sizes([71, 95])).toEqual([5, 5])
    expect(sizes([96, 160, 500])).toEqual([6, 6, 6])
  })
})

describe("teams", () => {
  it("takes the first N roster slots and fights them in reverse order", () => {
    const { team, battleOrder } = buildTeam(trainer("fixture", 0).roster, 30, 4)
    expect(team.map((member) => member.species)).toEqual([
      "Steelix",
      "Golem",
      "Kabutops",
      "Omastar",
    ])
    expect(battleOrder.map((member) => member.species)).toEqual([
      "Omastar",
      "Kabutops",
      "Golem",
      "Steelix",
    ])
    expect(battleOrder.map((member) => member.slot)).toEqual([4, 3, 2, 1])
  })

  it("adds each offset to the team level and clamps to 1–100", () => {
    const roster = [slot("Steelix", 0), slot("Golem", -6), slot("Onix", -3)]
    expect(buildTeam(roster, 42, 3).team.map((member) => member.level)).toEqual([42, 36, 39])
    expect(buildTeam(roster, 3, 3).team.map((member) => member.level)).toEqual([3, 1, 1])
    expect(buildTeam(roster, 100, 3).team.map((member) => member.level)).toEqual([100, 94, 97])
  })

  it("resolves a trainer from their own TR: TR 0 is two members, TR 160 is six at Lv 100", () => {
    const low = trainer("low", 0)
    const high = trainer("high", 160)
    const experiment = experimentWith([low, high])
    const early = resolveTrainer(low, experiment)
    expect([early.teamLevel, early.size]).toEqual([15, 2])
    expect(early.battleOrder.map((member) => [member.species, member.level])).toEqual([
      ["Golem", 13],
      ["Steelix", 15],
    ])
    const late = resolveTrainer(high, experiment)
    expect([late.teamLevel, late.size]).toEqual([100, 6])
    expect(late.battleOrder.at(-1)).toMatchObject({ species: "Steelix", level: 100 })
    expect(late.warnings).toEqual([])
  })

  it("flags a short roster and uses what exists", () => {
    const short = trainer("short", 120, six.slice(0, 5))
    const resolved = resolveTrainer(short, experimentWith([short]))
    expect(resolved.size).toBe(6)
    expect(resolved.team).toHaveLength(5)
    expect(resolved.warnings).toEqual([
      "The roster lists 5 of 6 Pokémon: add 1 more.",
      "The team fills 5 of 6 slots.",
    ])
  })

  it("never reads the player's TR: teams stay the same at every player point", () => {
    const blue = trainer("blue", 6)
    const experiment = experimentWith([blue])
    const before = resolveTrainer(blue, experiment)
    expect(playerRating({ badges: 24 })).toBe(160)
    expect(levelCap({ badges: 24 })).toBe(100)
    expect(resolveTrainer(blue, experiment)).toEqual(before)
    expect(before.teamLevel).toBe(17)
  })
})

describe("player readout", () => {
  it("gives +10 per badge for 1–8 and +5 per badge for 9–24", () => {
    const trs = [0, 1, 4, 8, 9, 16, 24].map((badges) => playerRating({ badges }))
    expect(trs).toEqual([0, 10, 40, 80, 85, 120, 160])
    expect(playerRating({ badges: 30 })).toBe(160) // badges clamp to 24
  })

  it("gives league wins no TR: the point has no league input", () => {
    expect(playerRating({ badges: 8, leagueClears: 3 } as never)).toBe(80)
    expect(Object.keys(createExperiment(catalog))).not.toContain("leagueClears")
  })

  it("reads the level cap and world scaling from the player TR", () => {
    const experiment = createExperiment(catalog)
    const at = (badges: number) => worldLevels(experiment, { badges })
    expect([0, 4, 8, 16, 24].map(at)).toEqual([
      { tr: 0, cap: 15, wild: 6, regularTrainer: 9 },
      { tr: 40, cap: 28, wild: 24, regularTrainer: 27 },
      { tr: 80, cap: 50, wild: 40, regularTrainer: 44 },
      { tr: 120, cap: 75, wild: 58, regularTrainer: 62 },
      { tr: 160, cap: 100, wild: 78, regularTrainer: 82 },
    ])
    // Early game presses against the cap; late game regular trainers fall well below it.
    expect([at(4).wild - at(4).cap, at(4).regularTrainer - at(4).cap]).toEqual([-4, -1])
    expect([at(24).wild - at(24).cap, at(24).regularTrainer - at(24).cap]).toEqual([-22, -18])
  })
})

describe("roster validation", () => {
  const valid = trainer("fixture", 0).roster
  it("accepts six roster slots with roster slot 1 at offset 0", () => {
    expect(validateRoster(valid, "roster")).toEqual(valid)
  })
  it("accepts a short roster, which the explorer reports as a gap", () => {
    expect(validateRoster(valid.slice(0, 5), "roster")).toHaveLength(5)
    const short = trainer("short", 0, six.slice(0, 4))
    expect(rosterGaps([short], experimentWith([short]))).toEqual([
      { id: "short", name: "short", length: 4 },
    ])
  })
  it.each([
    ["seven roster slots", [...valid, slot("Onix")], "1–6 Pokémon"],
    ["no roster slots", [], "1–6 Pokémon"],
    [
      "roster slot 1 off 0",
      [slot("Steelix", -1), ...valid.slice(1)],
      "roster slot 1 must have level offset 0",
    ],
    ["an offset below -6", [valid[0], slot("Golem", -7)], "levelOffset"],
    ["an offset above 0", [valid[0], slot("Golem", 1)], "levelOffset"],
    ["five moves", [{ ...slot("Steelix", 0), moves: ["A", "B", "C", "D", "E"] }], "moves"],
    ["no moves", [{ ...slot("Steelix", 0), moves: [] }], "moves"],
    ["a blank species", [slot(" ", 0)], "species"],
    ["an unknown key", [{ ...slot("Steelix", 0), ace: true }], "unknown fields"],
  ])("rejects %s", (_name, roster, message) => {
    expect(() => validateRoster(roster, "roster")).toThrow(message)
  })
})

describe("league lineup", () => {
  it("picks the top five by TR in ascending battle order, strongest last", () => {
    const lineup = leagueLineup(catalog, createExperiment(catalog))
    expect(lineup.map((row) => [row.trainer.name, row.tr, row.teamLevel])).toEqual([
      ["Bruno", 90, 56],
      ["Agatha", 92, 58],
      ["Wallace", 92, 58],
      ["Steven", 92, 58],
      ["Lance", 95, 59],
    ])
    expect(lineup.at(-1)?.battleOrder.at(-1)?.species).toBe("Dragonite")
  })

  it("keeps the first league within reach at 8 badges: TR 85–95, team level near cap Lv 50", () => {
    const lineup = leagueLineup(catalog, createExperiment(catalog))
    expect(levelCap({ badges: 8 })).toBe(50)
    for (const row of lineup) {
      expect(row.tr).toBeGreaterThanOrEqual(85)
      expect(row.tr).toBeLessThanOrEqual(95)
      expect(row.teamLevel).toBeGreaterThanOrEqual(53)
      expect(row.teamLevel).toBeLessThanOrEqual(59)
    }
  })

  it("breaks ties by array order, with no tie-break logic", () => {
    const records = ["a", "b", "c", "d", "e", "f", "g"].map((id, index) =>
      trainer(id, index === 6 ? 90 : 40),
    )
    const lineup = leagueLineup(records, experimentWith(records))
    expect(lineup.map((row) => row.trainer.id)).toEqual(["a", "b", "c", "d", "g"])
  })
})

describe("catalog", () => {
  it("ships 37 trainers with placeholder TRs and valid rosters of at most six", () => {
    expect(catalog).toHaveLength(37)
    const experiment = createExperiment(catalog)
    for (const record of catalog) {
      expect(Number.isInteger(record.tr) && record.tr >= 0).toBe(true)
      expect(record.trSource).toContain("PLACEHOLDER")
      expect(record.roster[0]?.levelOffset).toBe(0)
      expect(record.roster.length).toBeLessThanOrEqual(6)
    }
    const tr = (name: string) => catalog.find((record) => record.name === name)?.tr
    expect([tr("Falkner"), tr("Giovanni"), tr("Blue"), tr("Will"), tr("Lance")]).toEqual([
      1, 5, 6, 70, 95,
    ])
    expect(Object.keys(experiment.trainers)).toHaveLength(37)
    expect(catalog.some((record) => /^(red|tate)/.test(record.id))).toBe(false)
  })

  it("lists the rosters short of six as content gaps", () => {
    expect(rosterGaps(catalog, createExperiment(catalog)).map((gap) => gap.name)).toEqual([
      "Lorelei",
      "Bruno",
      "Agatha",
      "Koga",
      "Lance",
      "Will",
      "Karen",
      "Sidney",
      "Phoebe",
      "Glacia",
      "Drake",
    ])
  })
})

describe("experiment import", () => {
  const records = [trainer("fixture", 3)]
  const base = () => JSON.parse(serializeExperiment(experimentWith(records)))

  it("round-trips deterministic JSON and returns an independent validated value", () => {
    const experiment = experimentWith(records)
    const text = serializeExperiment(experiment)
    const restored = validateExperiment(JSON.parse(text), records)
    expect(serializeExperiment(restored)).toBe(text)
    restored.trainers.fixture!.tr = 9
    expect(experiment.trainers.fixture!.tr).toBe(3)
  })

  it.each([1, 2, 3, 4])("rejects version %i without migrating it", (version) => {
    expect(() => validateExperiment({ ...base(), version }, records)).toThrow(
      `Version ${version} experiments use a retired trainer model`,
    )
  })

  it("rejects version 5, which used the retired 0–80 player TR scale", () => {
    const v5 = base()
    v5.version = 5
    delete v5.wildLevel
    delete v5.routeTrainerLevel
    expect(() => validateExperiment(v5, records)).toThrow(
      "Version 5 experiments use the retired 0–80 player TR scale",
    )
  })

  it("requires the wild and regular trainer scalers", () => {
    const input = base()
    delete input.wildLevel
    expect(() => validateExperiment(input, records)).toThrow("missing or unknown fields")
    expect(() => validateExperiment({ ...base(), routeTrainerLevel: [[5, 9]] }, records)).toThrow(
      "start at TR 0",
    )
  })

  it("accepts any non-negative whole TR and rejects others", () => {
    const input = base()
    input.trainers.fixture.tr = 1_000_000
    expect(validateExperiment(input, records).trainers.fixture?.tr).toBe(1_000_000)
    for (const bad of [-1, 2.5, "7"]) {
      input.trainers.fixture.tr = bad
      expect(() => validateExperiment(input, records)).toThrow("non-negative whole number")
    }
  })

  it.each([
    ["not starting at TR 0", [[1, 15]], "start at TR 0"],
    [
      "repeating a TR",
      [
        [0, 15],
        [0, 16],
      ],
      "TRs must increase",
    ],
    [
      "decreasing in value",
      [
        [0, 15],
        [10, 14],
      ],
      "must not decrease",
    ],
    [
      "exceeding Lv 100",
      [
        [0, 15],
        [10, 101],
      ],
      "value",
    ],
  ])("rejects a team level scaler %s", (_name, anchors, message) => {
    expect(() => validateExperiment({ ...base(), teamLevel: anchors }, records)).toThrow(message)
  })

  it("rejects a team size above six", () => {
    expect(() => validateExperiment({ ...base(), teamSize: [[0, 7]] }, records)).toThrow("value")
  })
})

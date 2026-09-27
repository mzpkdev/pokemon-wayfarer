import { describe, expect, it } from "vitest"

import catalogData from "./catalog.json"
import {
  DEFAULT_TEAM_SIZE,
  PLAYER_CAP_ANCHORS,
  buildTeam,
  createExperiment,
  leagueField,
  playerCap,
  playerRating,
  resolveTrainer,
  rosterGaps,
  scale,
  serializeExperiment,
  teamLevelFor,
  teamSizeFor,
  validateExperiment,
  validateRoster,
} from "./engine.js"
import type { Experiment, RosterEntry, TrainerRecord } from "./types.js"

const catalog = catalogData as TrainerRecord[]
const entry = (species: string, levelOffset = -2): RosterEntry => ({
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
  roster: species.map((name, index) => entry(name, index === 0 ? 0 : -2)),
  rosterSource: "Fixture",
})
const experimentWith = (records: TrainerRecord[]): Experiment => createExperiment(records)

describe("scalers", () => {
  it("interpolates linearly between anchors with halves rounded up", () => {
    const anchors = PLAYER_CAP_ANCHORS
    expect(scale(anchors, 0)).toBe(15)
    expect(scale(anchors, 1)).toBe(15) // 15.25
    expect(scale(anchors, 2)).toBe(16) // 15.5 rounds up
    expect(scale(anchors, 12)).toBe(21) // 20.5 rounds up
    expect(scale(anchors, 40)).toBe(42)
    expect(scale(anchors, 47)).toBe(50) // 42 + 7 * 18 / 15 = 50.4
    expect(scale(anchors, 60)).toBe(70)
  })

  it("stays flat past the last anchor and never clamps TR", () => {
    const experiment = experimentWith([trainer("fixture", 0)])
    expect(teamLevelFor(experiment, 80)).toBe(100)
    expect(teamLevelFor(experiment, 120)).toBe(100)
    expect(teamSizeFor(experiment, 120)).toBe(6)
    expect(teamLevelFor(experiment, 1_000_000)).toBe(100)
    // A scaler that saturates later keeps rising past TR 80: TR is not capped at 80.
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

  it("uses the player soft-cap anchors for team level", () => {
    const experiment = experimentWith([trainer("fixture", 0)])
    for (let tr = 0; tr <= 100; tr += 1)
      expect(teamLevelFor(experiment, tr)).toBe(scale(PLAYER_CAP_ANCHORS, tr))
  })

  it("steps team size: 0–10 -> 2, 11–29 -> 3, 30–41 -> 4, 42–54 -> 5, 55+ -> 6", () => {
    const sizes = (trs: number[]) => trs.map((tr) => scale(DEFAULT_TEAM_SIZE, tr))
    expect(sizes([0, 5, 10])).toEqual([2, 2, 2])
    expect(sizes([11, 20, 29])).toEqual([3, 3, 3])
    expect(sizes([30, 41])).toEqual([4, 4])
    expect(sizes([42, 54])).toEqual([5, 5])
    expect(sizes([55, 80, 500])).toEqual([6, 6, 6])
  })
})

describe("teams", () => {
  it("takes the first N entries and fights them in reverse order", () => {
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
    const roster = [entry("Steelix", 0), entry("Golem", -6), entry("Onix", -3)]
    expect(buildTeam(roster, 42, 3).team.map((member) => member.level)).toEqual([42, 36, 39])
    expect(buildTeam(roster, 3, 3).team.map((member) => member.level)).toEqual([3, 1, 1])
    expect(buildTeam(roster, 100, 3).team.map((member) => member.level)).toEqual([100, 94, 97])
  })

  it("resolves a trainer from their own TR: TR 0 is two members, TR 120 is six at Lv 100", () => {
    const low = trainer("low", 0)
    const high = trainer("high", 120)
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

  it("flags a short roster and fields what exists", () => {
    const short = trainer("short", 60, six.slice(0, 5))
    const resolved = resolveTrainer(short, experimentWith([short]))
    expect(resolved.size).toBe(6)
    expect(resolved.team).toHaveLength(5)
    expect(resolved.warnings).toEqual([
      "The roster lists 5 of 6 entries: add 1 more.",
      "The team fills 5 of 6 slots.",
    ])
  })

  it("never reads the player's TR: teams stay the same at every player point", () => {
    const blue = trainer("blue", 6)
    const experiment = experimentWith([blue])
    const before = resolveTrainer(blue, experiment)
    expect(playerRating({ badges: 24, leagueClears: 3 })).toBe(80)
    expect(playerCap({ badges: 24, leagueClears: 3 })).toBe(100)
    expect(resolveTrainer(blue, experiment)).toEqual(before)
    expect(before.teamLevel).toBe(17)
  })
})

describe("player readout", () => {
  it("keeps the player TR formula and soft-cap curve", () => {
    expect(playerRating({ badges: 0, leagueClears: 0 })).toBe(0)
    expect(playerRating({ badges: 8, leagueClears: 0 })).toBe(40)
    expect(playerCap({ badges: 8, leagueClears: 0 })).toBe(42)
    expect(playerCap({ badges: 24, leagueClears: 0 })).toBe(62)
    expect(playerCap({ badges: 24, leagueClears: 2 })).toBe(89)
    expect(playerCap({ badges: 24, leagueClears: 3 })).toBe(100)
  })
})

describe("roster validation", () => {
  const valid = trainer("fixture", 0).roster
  it("accepts six entries with entry 1 at offset 0", () => {
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
    ["seven entries", [...valid, entry("Onix")], "1–6 entries"],
    ["no entries", [], "1–6 entries"],
    [
      "entry 1 off 0",
      [entry("Steelix", -1), ...valid.slice(1)],
      "entry 1 must have level offset 0",
    ],
    ["an offset below -6", [valid[0], entry("Golem", -7)], "levelOffset"],
    ["an offset above 0", [valid[0], entry("Golem", 1)], "levelOffset"],
    ["five moves", [{ ...entry("Steelix", 0), moves: ["A", "B", "C", "D", "E"] }], "moves"],
    ["no moves", [{ ...entry("Steelix", 0), moves: [] }], "moves"],
    ["a blank species", [entry(" ", 0)], "species"],
    ["an unknown field", [{ ...entry("Steelix", 0), ace: true }], "unknown fields"],
  ])("rejects %s", (_name, roster, message) => {
    expect(() => validateRoster(roster, "roster")).toThrow(message)
  })
})

describe("league field", () => {
  it("fields the top five by TR in ascending battle order, strongest last", () => {
    const field = leagueField(catalog, createExperiment(catalog))
    expect(field.map((row) => [row.trainer.name, row.tr])).toEqual([
      ["Bruno", 52],
      ["Agatha", 53],
      ["Wallace", 53],
      ["Steven", 53],
      ["Lance", 55],
    ])
    expect(field.at(-1)?.battleOrder.at(-1)?.species).toBe("Dragonite")
  })

  it("breaks ties by array order, with no tie-break logic", () => {
    const records = ["a", "b", "c", "d", "e", "f", "g"].map((id, index) =>
      trainer(id, index === 6 ? 90 : 40),
    )
    const field = leagueField(records, experimentWith(records))
    expect(field.map((row) => row.trainer.id)).toEqual(["a", "b", "c", "d", "g"])
  })
})

describe("catalog", () => {
  it("ships 37 trainers with provisional TRs and valid rosters of at most six", () => {
    expect(catalog).toHaveLength(37)
    const experiment = createExperiment(catalog)
    for (const record of catalog) {
      expect(Number.isInteger(record.tr) && record.tr >= 0).toBe(true)
      expect(record.trSource).toContain("PROVISIONAL")
      expect(record.roster[0]?.levelOffset).toBe(0)
      expect(record.roster.length).toBeLessThanOrEqual(6)
    }
    const tr = (name: string) => catalog.find((record) => record.name === name)?.tr
    expect([tr("Falkner"), tr("Giovanni"), tr("Blue"), tr("Will"), tr("Lance")]).toEqual([
      1, 5, 6, 40, 55,
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

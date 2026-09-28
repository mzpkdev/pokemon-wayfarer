import { describe, expect, it } from "vitest"

import catalogData from "./catalog.json"
import {
  ALOOF_MARGIN,
  ARCHETYPES,
  ARCHETYPE_KIND,
  DEFAULT_ARCHETYPE_GROWTH,
  DEFAULT_REGULAR_TRAINER_LEVEL,
  DEFAULT_TEAM_LEVEL,
  DEFAULT_TEAM_SIZE,
  DEFAULT_WILD_LEVEL,
  LEVEL_CAP_ANCHORS,
  NEAR_BAND,
  NO_EVOLUTION,
  MAX_BADGES,
  MILESTONE_SCAN_LIMIT,
  NO_LEARNSETS,
  WORLD_PROGRESS_CHECKPOINTS,
  badgeMatch,
  badgeMatchText,
  badgeTR,
  battleOrderOf,
  buildTeam,
  createExperiment,
  defaultMoveset,
  dormantReasonText,
  evolutionIndex,
  evolutionStatus,
  growthTR,
  gymLadder,
  isGymLeader,
  leagueCandidates,
  leagueScore,
  leagueSequence,
  leagueWorlds,
  learnsetIndex,
  learnsetOf,
  levelCap,
  milestoneEnd,
  milestoneText,
  milestones,
  moveSource,
  poolLearning,
  rankLeague,
  playerRating,
  resolveMovePool,
  resolveTrainer,
  rosterGaps,
  scale,
  serializeExperiment,
  stageAt,
  teamLevelFor,
  teamSizeFor,
  trainerRating,
  validateExperiment,
  validateMovePool,
  validateRoster,
  willingness,
  worldLevels,
  worldProgress,
  type Learnsets,
} from "./engine.js"
import type {
  Archetype,
  Catalog,
  Experiment,
  RosterSlot,
  TrainerRecord,
  HomeRegion,
  TravelStyle,
  WorldPoint,
} from "./types.js"

const data = catalogData as Catalog
const catalog = data.trainers
const evolution = evolutionIndex(data.evolution)
const learnsets = learnsetIndex(data.learnsets)
const slot = (species: string, levelOffset = -2, isAce = levelOffset === 0): RosterSlot => ({
  species,
  levelOffset,
  isAce,
  item: null,
  ability: null,
  nature: null,
})
const six = ["Steelix", "Golem", "Kabutops", "Omastar", "Aerodactyl", "Rhydon"]
/** A fixture trainer; by default steady with start = peak, so its TR is `tr` at every world progress. */
const trainer = (
  id: string,
  tr: number,
  species = six,
  growth: {
    archetype?: Archetype
    peakTR?: number
    homeRegion?: HomeRegion
    travel?: TravelStyle
    aloof?: boolean
  } = {},
): TrainerRecord => ({
  id,
  name: id,
  region: "Kanto",
  homeRegion: growth.homeRegion ?? "Kanto",
  travel: growth.travel ?? "homebody",
  aloof: growth.aloof ?? false,
  role: "Gym Leader",
  doubleBattle: false,
  leagueEligible: true,
  source: { label: "Fixture", path: "fixture", trainerId: "TEST", note: "Test data" },
  referenceParty: [],
  startTR: tr,
  archetype: growth.archetype ?? "steady",
  peakTR: growth.peakTR ?? tr,
  trSource: "Fixture",
  roster: species.map((name, index) => slot(name, index === 0 ? 0 : -2)),
  rosterSource: "Fixture",
  movePool: [],
  movePoolSource: "Fixture",
})
const experimentWith = (records: TrainerRecord[]): Experiment => createExperiment(records)
const defaults = createExperiment(catalog)
const at = (world: number) =>
  catalog.map((record) => resolveTrainer(record, defaults, world, evolution))

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

  it("gives team level its own low end and the level cap anchors from TR 40", () => {
    const experiment = experimentWith([trainer("fixture", 0)])
    expect(DEFAULT_TEAM_LEVEL).toEqual([
      [0, 5],
      [20, 14],
      [40, 28],
      [80, 50],
      [120, 75],
      [160, 100],
    ])
    expect([0, 10, 20, 26, 30].map((tr) => teamLevelFor(experiment, tr))).toEqual([
      5, 10, 14, 18, 21,
    ])
    for (let tr = 40; tr <= 200; tr += 1)
      expect(teamLevelFor(experiment, tr)).toBe(scale(LEVEL_CAP_ANCHORS, tr))
    // The level cap itself is unchanged.
    expect(scale(LEVEL_CAP_ANCHORS, 0)).toBe(15)
  })

  it("holds each anchor's value until the next anchor in a step scaler", () => {
    const anchors = DEFAULT_ARCHETYPE_GROWTH.burst
    expect(
      [0, 1, 39, 40, 41, 79, 80, 119, 120, 159, 160, 400].map((tr) => scale(anchors, tr, "step")),
    ).toEqual([0, 0, 0, 25, 25, 25, 50, 50, 75, 75, 100, 100])
    // The same anchors interpolated rise between the anchors instead.
    expect(scale(anchors, 20)).toBe(13)
    expect(scale(anchors, 20, "step")).toBe(0)
  })

  it("steps team size: 0–10 -> 1, 11–28 -> 2, 29–43 -> 3, 44–56 -> 4, 57–70 -> 5, 71+ -> 6", () => {
    const sizes = (trs: number[]) => trs.map((tr) => scale(DEFAULT_TEAM_SIZE, tr))
    expect(sizes([0, 5, 10])).toEqual([1, 1, 1])
    expect(sizes([11, 20, 28])).toEqual([2, 2, 2])
    expect(sizes([29, 35, 43])).toEqual([3, 3, 3])
    expect(sizes([44, 50, 56])).toEqual([4, 4, 4])
    expect(sizes([57, 63, 70])).toEqual([5, 5, 5])
    expect(sizes([71, 96, 160, 500])).toEqual([6, 6, 6, 6])
  })
})

describe("teams", () => {
  it("takes the first N roster slots and fights them in reverse order", () => {
    const { team, battleOrder } = buildTeam(trainer("fixture", 0).roster, 30, 4, NO_EVOLUTION)
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

  it("fights filler slots first and aces last, each in reverse list order", () => {
    const brock = defaults.trainers.brock!.roster
    const order = (size: number) =>
      buildTeam(brock, 100, size, evolution).battleOrder.map((member) => member.species)
    expect(brock.map((rosterSlot) => [rosterSlot.species, rosterSlot.isAce])).toEqual([
      ["Steelix", true],
      ["Golem", false],
      ["Crobat", false],
      ["Kabutops", false],
      ["Omastar", false],
      ["Aerodactyl", true],
    ])
    expect(order(6)).toEqual(["Omastar", "Kabutops", "Crobat", "Golem", "Aerodactyl", "Steelix"])
    expect(order(5)).toEqual(["Omastar", "Kabutops", "Crobat", "Golem", "Steelix"])
    expect(order(4)).toEqual(["Kabutops", "Crobat", "Golem", "Steelix"])
    expect(order(3)).toEqual(["Crobat", "Golem", "Steelix"])
    expect(order(2)).toEqual(["Golem", "Steelix"])
    expect(order(1)).toEqual(["Steelix"])
    // Aces only: plain reverse; the order never depends on species or level.
    const aces = [slot("A", 0), slot("B", -1, true), slot("C", -6, true)]
    expect(battleOrderOf(aces).map((member) => member.species)).toEqual(["C", "B", "A"])
  })

  it("unlocks roster slots by team size in list order: Aerodactyl only at size 6", () => {
    const brock = defaults.trainers.brock!.roster
    const team = (size: number) =>
      buildTeam(brock, 100, size, evolution).team.map((member) => member.species)
    for (let size = 1; size <= 5; size += 1) expect(team(size)).not.toContain("Aerodactyl")
    expect(team(6)).toContain("Aerodactyl")
    expect(team(3)).toEqual(["Steelix", "Golem", "Crobat"])
    // Steady Brock (25 → 100) reaches TR 71, the first TR at team size 6, at world progress 98:
    // 25 + 75 × 98/160 = 70.94. At 97 he is TR 70 (25 + 45.47), still team size 5.
    const record = catalog.find((entry) => entry.id === "brock")!
    const at = (world: number) => resolveTrainer(record, defaults, world, evolution)
    const before = at(97)
    const joined = at(98)
    expect([before.tr, before.size, before.team.map((member) => member.species)]).toEqual([
      70,
      5,
      ["Steelix", "Golem", "Crobat", "Kabutops", "Omastar"],
    ])
    // Team level at TR 71 is 28 + 31 × 22/40 = 45.05, so the Aerodactyl ace (offset 0) joins at Lv 45.
    expect([joined.tr, joined.size, joined.teamLevel]).toEqual([71, 6, 45])
    expect(joined.team[5]).toMatchObject({ slot: 6, species: "Aerodactyl", level: 45, isAce: true })
    expect(at(400).size).toBe(6)
  })

  it("adds each offset to the team level and clamps to 1–100", () => {
    const roster = [slot("Steelix", 0), slot("Golem", -6), slot("Onix", -3)]
    expect(buildTeam(roster, 42, 3, NO_EVOLUTION).team.map((member) => member.level)).toEqual([
      42, 36, 39,
    ])
    expect(buildTeam(roster, 3, 3, NO_EVOLUTION).team.map((member) => member.level)).toEqual([
      3, 1, 1,
    ])
    expect(buildTeam(roster, 100, 3, NO_EVOLUTION).team.map((member) => member.level)).toEqual([
      100, 94, 97,
    ])
  })

  it("resolves a trainer from their own TR: TR 0 is one member, TR 160 is six at Lv 100", () => {
    const low = trainer("low", 0)
    const mid = trainer("mid", 20)
    const high = trainer("high", 160)
    const experiment = experimentWith([low, mid, high])
    const early = resolveTrainer(low, experiment, 0, NO_EVOLUTION)
    expect([early.teamLevel, early.size]).toEqual([5, 1])
    expect(early.battleOrder.map((member) => [member.species, member.level])).toEqual([
      ["Steelix", 5],
    ])
    expect(
      resolveTrainer(mid, experiment, 0, NO_EVOLUTION).battleOrder.map((member) => [
        member.species,
        member.level,
      ]),
    ).toEqual([
      ["Golem", 12],
      ["Steelix", 14],
    ])
    const late = resolveTrainer(high, experiment, 0, NO_EVOLUTION)
    expect([late.teamLevel, late.size]).toEqual([100, 6])
    expect(late.battleOrder.at(-1)).toMatchObject({ species: "Steelix", level: 100 })
    expect(late.warnings).toEqual([])
  })

  it("flags a short roster and uses what exists", () => {
    const short = trainer("short", 120, six.slice(0, 5))
    const resolved = resolveTrainer(short, experimentWith([short]), 0, NO_EVOLUTION)
    expect(resolved.size).toBe(6)
    expect(resolved.team).toHaveLength(5)
    expect(resolved.warnings).toEqual([
      "The roster lists 5 of 6 Pokémon: add 1 more.",
      "The team fills 5 of 6 slots.",
    ])
  })

  it("reads nothing about the player but world progress: no party, no level cap", () => {
    // The point is only ever asked for its badges; any other read throws.
    const point = new Proxy(
      { badges: 8, party: [{ species: "Mewtwo", level: 100 }] },
      {
        get: (target, key) => {
          if (key !== "badges") throw new Error(`read ${String(key)}`)
          return target.badges
        },
      },
    ) as WorldPoint
    const world = worldProgress(point)
    expect(world).toBe(80)
    const brock = catalog.find((record) => record.id === "brock")!
    const resolved = resolveTrainer(brock, defaults, world, evolution)
    expect(resolved).toEqual(
      resolveTrainer(brock, defaults, playerRating({ badges: 8 }), evolution),
    )
    // Steady Brock (25 → 100) at world progress 80: 25 + 75 × 80/160 = 62.5, rounded up.
    expect(resolved.tr).toBe(63)
    // Resolution takes a world progress number and the shared evolution data, so a party or
    // level cap cannot be passed in.
    expect(resolveTrainer.length).toBe(4)
  })
})

describe("evolution", () => {
  const line = (...chain: (string | number)[]) =>
    evolutionIndex({ chains: { [String(chain.at(-1))]: chain }, notFinal: [] })
  const stages = (species: string, levels: number[]) =>
    levels.map((level) => stageAt(evolution, species, level).species)

  it("steps down through a non-level edge from the shared table: Steelix is Onix below 35", () => {
    expect(stages("Steelix", [1, 14, 34, 35, 36, 100])).toEqual([
      "Onix",
      "Onix",
      "Onix",
      "Steelix",
      "Steelix",
      "Steelix",
    ])
    expect(stageAt(evolution, "Steelix", 34)).toEqual({ species: "Onix", authoredAt: 35 })
    expect(stageAt(evolution, "Steelix", 35)).toEqual({ species: "Steelix", authoredAt: null })
  })

  it("steps down through several edges: Golem to Graveler (level, 38) to Geodude (level, 25)", () => {
    expect(stages("Golem", [12, 24, 25, 37, 38, 80])).toEqual([
      "Geodude",
      "Geodude",
      "Graveler",
      "Graveler",
      "Golem",
      "Golem",
    ])
    expect(stageAt(evolution, "Golem", 30)).toEqual({ species: "Graveler", authoredAt: 38 })
  })

  it("never evolves forward: an authored earlier stage stays at any level", () => {
    expect(stages("Onix", [5, 35, 100])).toEqual(["Onix", "Onix", "Onix"])
    expect(stages("Graveler", [10, 40, 100])).toEqual(["Geodude", "Graveler", "Graveler"])
    expect(stages("Eevee", [5, 100])).toEqual(["Eevee", "Eevee"])
    // A species with no catalog data never changes.
    expect(stageAt(evolution, "Missingno", 1)).toEqual({ species: "Missingno", authoredAt: null })
  })

  it("records the authored stage and the level it is reached at for a stepped-down member", () => {
    const member = (level: number) => buildTeam([slot("Golem", 0)], level, 1, evolution).team[0]!
    expect(member(37)).toMatchObject({
      species: "Graveler",
      authoredSpecies: "Golem",
      authoredAt: 38,
    })
    expect(member(38)).toMatchObject({
      species: "Golem",
      authoredSpecies: "Golem",
      authoredAt: null,
    })
  })

  it("resolves Brock: Onix and Geodude at Lv 14, Graveler from Lv 25, Steelix from 35, Golem from 38", () => {
    const roster = defaults.trainers.brock!.roster
    const team = (level: number) =>
      buildTeam(roster, level, 2, evolution).team.map((member) => [member.species, member.level])
    expect(team(14)).toEqual([
      ["Onix", 14],
      ["Geodude", 12],
    ])
    expect(team(26)).toEqual([
      ["Onix", 26],
      ["Geodude", 24],
    ])
    expect(team(27)).toEqual([
      ["Onix", 27],
      ["Graveler", 25],
    ])
    expect(team(35)).toEqual([
      ["Steelix", 35],
      ["Graveler", 33],
    ])
    expect(team(39)).toEqual([
      ["Steelix", 39],
      ["Graveler", 37],
    ])
    expect(team(40)).toEqual([
      ["Steelix", 40],
      ["Golem", 38],
    ])
    // The rule reads each member’s level, whatever the world progress.
    const brock = catalog.find((record) => record.id === "brock")!
    for (const world of WORLD_PROGRESS_CHECKPOINTS) {
      const resolved = resolveTrainer(brock, defaults, world, evolution)
      expect(resolved.team[0]?.species).toBe(resolved.teamLevel >= 35 ? "Steelix" : "Onix")
    }
  })

  it("validates the chains: one level per edge, increasing along the line, no cycles", () => {
    expect(() =>
      evolutionIndex({
        chains: {
          Golem: ["Geodude", 25, "Graveler", 38, "Golem"],
          Graveler: ["Geodude", 26, "Graveler"],
        },
        notFinal: [],
      }),
    ).toThrow("conflicting evolution levels")
    expect(() => line("Geodude", 40, "Graveler", 25, "Golem")).toThrow("must increase")
    expect(() => line("Geodude", 25, "Graveler", 25, "Golem")).toThrow("must increase")
    expect(() => line("Onix", 20, "Steelix", 35, "Onix")).toThrow("evolution cycle")
    expect(() => line("Onix", "Steelix")).toThrow("alternate species and levels")
    expect(() => line("Onix", 101, "Steelix")).toThrow("from 1 to 100")
    expect(line("Onix", 35, "Steelix").lines.get("Onix")).toEqual([{ species: "Onix", level: 1 }])
  })

  it("marks earlier stages and listed species as not final", () => {
    expect(evolutionStatus(evolution, "Steelix")).toEqual({ known: true, final: true })
    expect(evolutionStatus(evolution, "Onix")).toEqual({ known: true, final: false })
    expect(evolutionStatus(evolution, "Eevee")).toEqual({ known: true, final: false })
    expect(evolutionStatus(evolution, "Missingno")).toEqual({ known: false, final: true })
  })
})

/** Fixture learnsets: level-up [level, move] pairs in order, plus TM/tutor and egg moves. */
const learnsetsOf = (
  species: Record<string, { levelUp: [number, string][]; teachable?: string[]; egg?: string[] }>,
): Learnsets => ({
  moves: new Set(
    Object.values(species).flatMap(({ levelUp, teachable = [], egg = [] }) => [
      ...levelUp.map(([, move]) => move),
      ...teachable,
      ...egg,
    ]),
  ),
  species: new Map(
    Object.entries(species).map(([name, { levelUp, teachable = [], egg = [] }]) => [
      name,
      learnsetOf(levelUp, teachable, egg),
    ]),
  ),
})
const member = (slot: number, species: string, level: number, isAce = slot === 1) => ({
  slot,
  species,
  level,
  isAce,
})
const movesOf = (result: ReturnType<typeof resolveMovePool>, slot: number) =>
  (result.moves.get(slot) ?? []).map(({ move, source }) => `${move} ${source}`)

describe("move pools", () => {
  // Each species knows four level-up moves by Lv 20: A1–A4, B1–B4, C1–C4.
  const basic = (
    prefix: string,
    extra: { levelUp?: [number, string][]; teachable?: string[]; egg?: string[] } = {},
  ) => ({
    levelUp: [
      [1, `${prefix}1`],
      [5, `${prefix}2`],
      [10, `${prefix}3`],
      [15, `${prefix}4`],
      ...(extra.levelUp ?? []),
    ] as [number, string][],
    teachable: extra.teachable ?? [],
    egg: extra.egg ?? [],
  })

  it("gives the constructor's default moveset: the last four level-up moves by the level, oldest first", () => {
    const [learnset] = learnsetsOf({
      Mon: {
        levelUp: [
          [0, "Evolve"],
          [1, "A"],
          [1, "B"],
          [5, "C"],
          [9, "A"],
          [12, "D"],
          [15, "E"],
          [20, "F"],
        ],
      },
    }).species.values()
    expect(defaultMoveset(learnset, 1)).toEqual(["A", "B"])
    // A known move is not relearned, the evolution move (level 0) is skipped, and the oldest drops.
    expect(defaultMoveset(learnset, 12)).toEqual(["A", "B", "C", "D"])
    expect(defaultMoveset(learnset, 19)).toEqual(["B", "C", "D", "E"])
    expect(defaultMoveset(undefined, 50)).toEqual([])
  })

  it("visits aces first, then fillers, each in list order, and gives each entry to one member", () => {
    const tm = { teachable: ["Quake"] }
    const data = learnsetsOf({ A: basic("A", tm), B: basic("B", tm), C: basic("C", tm) })
    const quake = { move: "Quake", fromLevel: 1 }
    // Roster order: ace, filler, ace. The second ace (slot 3) picks before the filler (slot 2).
    const team = [member(1, "A", 20), member(2, "B", 20, false), member(3, "C", 20, true)]
    const result = resolveMovePool(team, [quake, quake], data)
    expect(result.pool.map((entry) => entry.slot)).toEqual([1, 3])
    expect(movesOf(result, 2)).toEqual(["B1 level-up", "B2 level-up", "B3 level-up", "B4 level-up"])
    // A third copy reaches the filler.
    const three = resolveMovePool(team, [quake, quake, quake], data)
    expect(three.pool.map((entry) => entry.slot)).toEqual([1, 3, 2])
  })

  it("takes an entry only from its from level", () => {
    const data = learnsetsOf({ A: basic("A", { teachable: ["Edge"] }) })
    const pool = [{ move: "Edge", fromLevel: 40 }]
    const low = resolveMovePool([member(1, "A", 39)], pool, data)
    expect(low.pool).toEqual([
      {
        index: 0,
        move: "Edge",
        fromLevel: 40,
        slot: null,
        species: null,
        reason: "level",
        waitLevel: 40,
        waitForm: null,
      },
    ])
    expect(dormantReasonText(low.pool[0]!)).toBe("below from level Lv 40")
    expect(resolveMovePool([member(1, "A", 40)], pool, data).pool[0]?.slot).toBe(1)
  })

  it("with a from level, checks learnability: the level-up learnset at any level, or the TM/tutor list", () => {
    const data = learnsetsOf({
      A: basic("A", { levelUp: [[60, "Late"]], teachable: ["Tm"] }),
      B: basic("B"),
    })
    const result = resolveMovePool(
      [member(1, "A", 10), member(2, "B", 10, false)],
      [
        { move: "Late", fromLevel: 1 },
        { move: "Tm", fromLevel: 1 },
        { move: "Tm", fromLevel: 1 },
        { move: "Nope" },
      ],
      data,
    )
    // A learns Late at Lv 60 by level-up, yet takes it at Lv 10 (filling its empty fourth slot);
    // Tm, from its TM list, replaces its oldest level-up move.
    expect(movesOf(result, 1)).toEqual(["Tm pool", "A2 level-up", "A3 level-up", "Late pool"])
    // B learns neither, so the second Tm stays with A, who already knows it; nobody learns Nope.
    expect(result.pool.map((entry) => [entry.move, entry.slot, entry.reason])).toEqual([
      ["Late", 1, null],
      ["Tm", 1, null],
      ["Tm", null, "taken"],
      ["Nope", null, "unlearnable"],
    ])
    expect(dormantReasonText(result.pool[3]!)).toBe("no one can learn it")
  })

  // Late is learned at Lv 30, then pushed out of the default moveset by L1–L4 at Lv 31–34.
  const late = basic("A", {
    levelUp: [
      [30, "Late"],
      [31, "L1"],
      [32, "L2"],
      [33, "L3"],
      [34, "L4"],
    ],
  })

  it("without a from level, waits for the member's level-up learn level", () => {
    const data = learnsetsOf({ A: late })
    const pool = [{ move: "Late" }]
    const early = resolveMovePool([member(1, "A", 29)], pool, data)
    expect(early.pool[0]).toMatchObject({
      fromLevel: null,
      slot: null,
      reason: "level",
      waitLevel: 30,
    })
    expect(dormantReasonText(early.pool[0]!)).toBe("below its learn level Lv 30")
    // From Lv 30 it is in reach and already in the default moveset, so the member claims it.
    const known = resolveMovePool([member(1, "A", 30)], pool, data)
    expect(known.pool[0]).toMatchObject({ slot: 1, reason: null })
    expect(movesOf(known, 1)).toEqual(["A2 level-up", "A3 level-up", "A4 level-up", "Late pool"])
    // At Lv 34 it has left the default moveset, so the pool brings it back.
    expect(movesOf(resolveMovePool([member(1, "A", 34)], pool, data), 1)).toEqual([
      "Late pool",
      "L2 level-up",
      "L3 level-up",
      "L4 level-up",
    ])
  })

  it("without a from level, skips a member that learns the move only by TM/tutor", () => {
    const data = learnsetsOf({ Tm: basic("T", { teachable: ["Late", "Only"] }), A: late })
    // The ace can learn Late only by TM, so the filler that learns it by level-up takes it.
    const team = [member(1, "Tm", 50), member(2, "A", 40, false)]
    const result = resolveMovePool(team, [{ move: "Late" }, { move: "Only" }], data)
    expect(result.pool.map((entry) => [entry.move, entry.slot, entry.reason])).toEqual([
      ["Late", 2, null],
      ["Only", null, "tm-only"],
    ])
    expect(dormantReasonText(result.pool[1]!)).toBe("TM/tutor only — needs a from level")
    // With a from level, the TM learner takes it.
    const from = resolveMovePool(
      team,
      [
        { move: "Late", fromLevel: 1 },
        { move: "Only", fromLevel: 45 },
      ],
      data,
    )
    expect(from.pool.map((entry) => entry.slot)).toEqual([1, 1])
  })

  it("without a from level, uses the lowest learn level and gives evolution moves at once", () => {
    const data = learnsetsOf({
      Twice: {
        levelUp: [
          [1, "T1"],
          [20, "Dup"],
          [21, "C1"],
          [22, "C2"],
          [23, "C3"],
          [24, "C4"],
          [45, "Dup"],
        ],
      },
      Evolved: {
        levelUp: [
          [0, "Evo"],
          [1, "E1"],
          [50, "Evo"],
        ],
      },
    })
    // Dup is listed at Lv 20 and 45: the lowest counts.
    const twice = resolveMovePool([member(1, "Twice", 30)], [{ move: "Dup" }], data)
    expect(twice.pool[0]?.slot).toBe(1)
    expect(
      resolveMovePool([member(1, "Twice", 19)], [{ move: "Dup" }], data).pool[0],
    ).toMatchObject({ reason: "level", waitLevel: 20 })
    // An evolution move (level 0) is not in the default moveset but is available at any level.
    const evolved = resolveMovePool([member(1, "Evolved", 5)], [{ move: "Evo" }], data)
    expect(movesOf(evolved, 1)).toEqual(["E1 level-up", "Evo pool"])
  })

  it("claims a move the member already knows from level-up: it counts and is protected", () => {
    const data = learnsetsOf({
      A: basic("A", { teachable: ["B1", "P1", "P2", "P3", "P4"] }),
      B: basic("B"),
    })
    const result = resolveMovePool(
      [member(1, "A", 20), member(2, "B", 20, false)],
      [{ move: "B1", fromLevel: 1 }, { move: "A1" }, { move: "A1" }],
      data,
    )
    // A takes B1 and claims A1, which it knows from level-up; the second A1 has no other learner.
    expect(result.pool.map((entry) => [entry.slot, entry.reason])).toEqual([
      [1, null],
      [1, null],
      [null, "taken"],
    ])
    expect(movesOf(result, 1)).toEqual(["A1 pool", "B1 pool", "A3 level-up", "A4 level-up"])
    expect(dormantReasonText(result.pool[2]!)).toBe(
      "taken: every learner already has it or four pool moves",
    )
    // A claimed move stays when later picks replace the oldest level-up moves, and the claim
    // counts toward the four: the fifth eligible entry stays dormant.
    const picks = ["P1", "P2", "P3", "P4"].map((move) => ({ move, fromLevel: 1 }))
    const full = resolveMovePool([member(1, "A", 20)], [{ move: "A1" }, ...picks], data)
    expect(movesOf(full, 1)).toEqual(["A1 pool", "P1 pool", "P2 pool", "P3 pool"])
    expect(full.pool.at(-1)).toMatchObject({ move: "P4", slot: null, reason: "taken" })
    // A claim needs an eligible entry: below its from level, A1 is not claimed.
    const held = resolveMovePool([member(1, "A", 20)], [{ move: "A1", fromLevel: 30 }], data)
    expect(held.pool[0]).toMatchObject({ slot: null, reason: "level" })
    expect(movesOf(held, 1)).toEqual(["A1 level-up", "A2 level-up", "A3 level-up", "A4 level-up"])
    // Two copies of one move never both go to one member.
    const b1 = { move: "B1", fromLevel: 1 }
    const twice = resolveMovePool([member(1, "A", 20)], [b1, b1], data)
    expect(twice.pool.map((entry) => entry.slot)).toEqual([1, null])
  })

  it("learns by level-up through earlier forms, at the lowest learn level on the line", () => {
    const line = evolutionIndex({ chains: { Big: ["Small", 20, "Big"] }, notFinal: [] })
    const data = learnsetsOf({
      Small: basic("S", {
        levelUp: [
          [30, "Old"],
          [25, "Both"],
        ],
      }),
      Big: basic("G", { levelUp: [[40, "Both"]] }),
    })
    const big = (level: number, pool: { move: string; fromLevel?: number }[]) =>
      resolveMovePool([member(1, "Big", level)], pool, data, line)
    // Big never learns Old itself: its earlier form Small does, at Lv 30.
    const early = big(29, [{ move: "Old" }])
    expect(early.pool[0]).toMatchObject({ reason: "level", waitLevel: 30, waitForm: "Small" })
    expect(dormantReasonText(early.pool[0]!)).toBe(
      "below its learn level: earlier form (Small) at Lv 30",
    )
    expect(big(30, [{ move: "Old" }]).pool[0]).toMatchObject({ slot: 1, species: "Big" })
    // Both is Lv 25 on Small and Lv 40 on Big: the lowest counts.
    expect(big(25, [{ move: "Both" }]).pool[0]?.slot).toBe(1)
    // Without evolution data only the member's own species counts.
    expect(resolveMovePool([member(1, "Big", 35)], [{ move: "Old" }], data).pool[0]).toMatchObject({
      reason: "unlearnable",
    })
    expect(moveSource("Big", "Old", data, line)).toEqual({
      kind: "level-up",
      level: 30,
      form: "Small",
    })
    expect(moveSource("Big", "Both", data, line)).toEqual({
      kind: "level-up",
      level: 25,
      form: "Small",
    })
  })

  it("takes a line's egg moves only with a from level, for every stage on the line", () => {
    const line = evolutionIndex({ chains: { Big: ["Small", 20, "Big"] }, notFinal: [] })
    const data = learnsetsOf({
      Small: basic("S", { egg: ["Egg", "Both"] }),
      Big: basic("G", { teachable: ["Both"] }),
    })
    const team = (level: number, pool: { move: string; fromLevel?: number }[]) =>
      resolveMovePool([member(1, "Big", level)], pool, data, line)
    // The egg moves are recorded on the line's first stage and count for Big too.
    expect(moveSource("Big", "Egg", data, line)).toEqual({ kind: "egg" })
    expect(moveSource("Small", "Egg", data, line)).toEqual({ kind: "egg" })
    // TM/tutor comes before egg: Big learns Both by TM, Small only as an egg move.
    expect(moveSource("Big", "Both", data, line)).toEqual({ kind: "tm" })
    expect(moveSource("Small", "Both", data, line)).toEqual({ kind: "egg" })
    const dormant = team(50, [{ move: "Egg" }])
    expect(dormant.pool[0]).toMatchObject({ slot: null, reason: "egg" })
    expect(dormantReasonText(dormant.pool[0]!)).toBe("egg move: needs a from level")
    expect(team(29, [{ move: "Egg", fromLevel: 30 }]).pool[0]).toMatchObject({ reason: "level" })
    expect(team(30, [{ move: "Egg", fromLevel: 30 }]).pool[0]).toMatchObject({ slot: 1 })
    // A move some member learns by TM/tutor and another only as an egg move reads as TM/tutor only.
    const mixed = resolveMovePool(
      [member(1, "Big", 50), member(2, "Small", 10, false)],
      [{ move: "Both" }],
      data,
      line,
    )
    expect(mixed.pool[0]).toMatchObject({ reason: "tm-only" })
  })

  it("fills empty move slots first, then replaces the oldest level-up moves", () => {
    const data = learnsetsOf({ A: basic("A", { teachable: ["P1", "P2", "P3", "P4", "P5"] }) })
    const pool = ["P1", "P2", "P3", "P4", "P5"].map((move) => ({ move, fromLevel: 1 }))
    // Lv 5 knows A1 and A2: P1 and P2 fill the empty slots, P3 replaces A1 (the oldest), P4 A2.
    expect(movesOf(resolveMovePool([member(1, "A", 5)], pool.slice(0, 3), data), 1)).toEqual([
      "P3 pool",
      "A2 level-up",
      "P1 pool",
      "P2 pool",
    ])
    // A full level-up moveset keeps its newest natural moves.
    expect(movesOf(resolveMovePool([member(1, "A", 20)], pool.slice(0, 2), data), 1)).toEqual([
      "P1 pool",
      "P2 pool",
      "A3 level-up",
      "A4 level-up",
    ])
    // At most four pool moves: the fifth entry stays dormant.
    const full = resolveMovePool([member(1, "A", 20)], pool, data)
    expect(movesOf(full, 1)).toEqual(["P1 pool", "P2 pool", "P3 pool", "P4 pool"])
    expect(full.pool.at(-1)).toMatchObject({ move: "P5", slot: null, reason: "taken" })
  })

  it("checks the current species after stepping down, and wakes an entry when a member evolves or joins", () => {
    const line = evolutionIndex({ chains: { Big: ["Small", 25, "Big"] }, notFinal: [] })
    const data = learnsetsOf({
      Small: basic("S"),
      Big: basic("G", { teachable: ["Quake"] }),
      Other: basic("O", { teachable: ["Quake"] }),
    })
    const pool = [{ move: "Quake", fromLevel: 1 }]
    const roster = [slot("Big", 0), slot("Other", -2)]
    // Below Lv 25 the member is Small: its own level-up moves, and it cannot learn Quake.
    const small = buildTeam(roster, 24, 1, line, pool, data)
    expect(small.team[0]?.species).toBe("Small")
    expect(small.team[0]?.moves.map((move) => move.move)).toEqual(["S1", "S2", "S3", "S4"])
    expect(small.pool[0]).toMatchObject({ slot: null, reason: "unlearnable" })
    // It evolves: Big takes Quake.
    expect(buildTeam(roster, 25, 1, line, pool, data).pool[0]).toMatchObject({
      slot: 1,
      species: "Big",
    })
    // Or a member that can learn it joins.
    expect(buildTeam(roster, 24, 2, line, pool, data).pool[0]).toMatchObject({
      slot: 2,
      species: "Other",
    })
  })

  it("is deterministic and leaves a team without learnset data with no moves", () => {
    const data = learnsetsOf({ A: basic("A", { teachable: ["X"] }) })
    const team = [member(1, "A", 20), member(2, "Missing", 20, false)]
    const pool = [{ move: "X" }, { move: "X" }]
    expect(resolveMovePool(team, pool, data)).toEqual(resolveMovePool(team, pool, data))
    expect(movesOf(resolveMovePool(team, pool, data), 2)).toEqual([])
    expect(
      resolveMovePool(team, pool, NO_LEARNSETS).pool.every((entry) => entry.slot === null),
    ).toBe(true)
  })

  it("resolves Brock's pool draft from the catalog learnsets", () => {
    const brock = catalog.find((record) => record.id === "brock")!
    const moves = (world: number) =>
      resolveTrainer(brock, defaults, world, evolution, learnsets).team.map((entry) => [
        entry.species,
        entry.moves.map(({ move, source }) => `${move}${source === "pool" ? "*" : ""}`),
      ])
    // An entry without a from level goes only to a member whose current species or an earlier
    // form learns it by level-up, once it reaches the learn level. At world progress 0 Onix (the
    // ace) takes Bind (a Lv 1 level-up move) and Curse and claims Stealth Rock, which it already
    // knows; Geodude is below its Earthquake learn level.
    expect(moves(0)).toEqual([
      ["Onix", ["Bind*", "Curse*", "Rage", "Stealth Rock*"]],
      ["Geodude", ["Rollout", "Magnitude", "Strength", "Rock Throw"]],
    ])
    const early = resolveTrainer(brock, defaults, 0, evolution, learnsets)
    expect(early.pool[0]).toMatchObject({ move: "Bind", slot: 1, species: "Onix" })
    expect(early.pool[1]).toMatchObject({ move: "Stealth Rock", slot: 1, species: "Onix" })
    const reasons = new Map(early.dormant.map((entry) => [entry.move, dormantReasonText(entry)]))
    // Sandstorm has a from level: Onix learns it by level-up only at Lv 52, Golem only by TM.
    expect(reasons.get("Sandstorm")).toBe("below from level Lv 20")
    expect(reasons.get("Earthquake")).toBe("below its learn level Lv 34")
    // Heavy Slam is an Onix egg move (Golem learns it by level-up, but has not joined yet).
    expect(reasons.get("Heavy Slam")).toBe("egg move: needs a from level")
    // Only Crobat learns Cross Poison, and it has not joined yet.
    expect(reasons.get("Cross Poison")).toBe("no one can learn it")
    const timeline = milestones(brock, defaults, evolution, learnsets).map(milestoneText)
    expect(timeline).toContain("6: Sandstorm wakes (Onix)")
    // Graveler learns Earthquake from its earlier form Geodude (Lv 34).
    expect(timeline).toContain("61: Earthquake wakes (Graveler)")
    expect(timeline).toContain(
      "76: Graveler → Golem, Golbat → Crobat, Heavy Slam wakes (Golem), Cross Poison wakes (Crobat)",
    )
    // The pool editor's hint reads every stage on the roster lines.
    const learning = (move: string) => poolLearning(move, brock.roster, evolution, learnsets)
    expect([learning("Earthquake"), learning("Toxic"), learning("Iron Defense")]).toEqual([
      { kind: "level-up", species: "Golem", form: "Geodude", level: 34 },
      { kind: "tm-only" },
      { kind: "unlearnable" },
    ])
    // Without learnsets nothing is assigned, so nothing wakes.
    expect(milestones(brock, defaults, evolution).map(milestoneText).join()).not.toContain("wakes")
  })

  it("validates a move pool: move names, from levels 1–100 and at most 64 entries", () => {
    const moves = new Set(["Earthquake", "Stone Edge"])
    expect(
      validateMovePool(
        [{ move: "Earthquake" }, { move: "Stone Edge", fromLevel: 40 }],
        "pool",
        moves,
      ),
    ).toEqual([{ move: "Earthquake" }, { move: "Stone Edge", fromLevel: 40 }])
    expect(() => validateMovePool([{ move: "Earthquak" }], "pool", moves)).toThrow(
      'pool[1]: unknown move "Earthquak"',
    )
    expect(() => validateMovePool([{ move: "Earthquake", fromLevel: 0 }], "pool", moves)).toThrow(
      "fromLevel must be an integer from 1 to 100",
    )
    expect(() => validateMovePool([{ move: "Earthquake", fromLevel: 101 }], "pool", moves)).toThrow(
      "from 1 to 100",
    )
    expect(() => validateMovePool([{ move: "Earthquake", level: 3 }], "pool", moves)).toThrow(
      "unknown fields",
    )
    expect(() => validateMovePool(Array(65).fill({ move: "Earthquake" }), "pool", moves)).toThrow(
      "at most 64 entries",
    )
    // The catalog pools use only real move names.
    for (const record of catalog)
      expect(validateMovePool(record.movePool, record.id, learnsets.moves)).toEqual(record.movePool)
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
    const at = (badges: number) => worldLevels(experiment, badgeTR(badges))
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

describe("player TR and badge presets", () => {
  it("maps each badge preset to its player TR and back", () => {
    for (let badges = 0; badges <= MAX_BADGES; badges += 1) {
      const tr = badgeTR(badges)
      expect(tr).toBe(playerRating({ badges }))
      expect(badgeMatch(tr)).toEqual({ kind: "exact", badges })
    }
    expect([0, 8, 16, 24].map(badgeTR)).toEqual([0, 80, 120, 160])
    expect(badgeMatchText(badgeMatch(10))).toBe("1 badge")
    expect(badgeMatchText(badgeMatch(80))).toBe("8 badges")
  })

  it("places off-badge TR between badge counts or beyond 24 badges", () => {
    // TR 95 is exactly 11 badges (80 + 3 × 5); 97 falls between 11 and 12.
    expect(badgeMatch(95)).toEqual({ kind: "exact", badges: 11 })
    expect(badgeMatch(97)).toEqual({ kind: "between", lower: 11, upper: 12 })
    expect(badgeMatch(5)).toEqual({ kind: "between", lower: 0, upper: 1 })
    expect(badgeMatch(83)).toEqual({ kind: "between", lower: 8, upper: 9 })
    expect(badgeMatch(170)).toEqual({ kind: "beyond", badges: 24 })
    expect(badgeMatch(300)).toEqual({ kind: "beyond", badges: 24 })
    expect(badgeMatchText(badgeMatch(97))).toBe("between 11 and 12 badges")
    expect(badgeMatchText(badgeMatch(300))).toBe("beyond 24 badges")
    expect(() => badgeMatch(-1)).toThrow("non-negative whole number")
    expect(() => badgeMatch(12.5)).toThrow("non-negative whole number")
  })

  it("resolves world scaling and notable trainers at off-badge player TR", () => {
    const experiment = createExperiment(catalog)
    // TR 95 (11 badges) and 97 (between 11 and 12): linear between the TR 80 and TR 120 anchors.
    expect(worldLevels(experiment, 95)).toEqual({ tr: 95, cap: 59, wild: 47, regularTrainer: 51 })
    expect(worldLevels(experiment, 97)).toEqual({ tr: 97, cap: 61, wild: 48, regularTrainer: 52 })
    // Past 24 badges the level cap and world scaling stay flat at their ceiling TR (160).
    expect(worldLevels(experiment, 170)).toEqual({ ...worldLevels(experiment, 160), tr: 170 })
    expect(worldLevels(experiment, 300)).toEqual({
      tr: 300,
      cap: 100,
      wild: 78,
      regularTrainer: 82,
    })
    expect([levelCap(95), levelCap(170), levelCap(300)]).toEqual([59, 100, 100])
    const brock = catalog.find((record) => record.id === "brock")!
    const blue = catalog.find((record) => record.id === "blue")!
    const tr = (record: TrainerRecord, world: number) =>
      resolveTrainer(record, experiment, world, evolution).tr
    // Steady Brock (25 → 100): 25 + 75 × 95/160 = 69.53. Flat past 160.
    expect([tr(brock, 95), tr(brock, 170), tr(brock, 300)]).toEqual([70, 100, 100])
    // Rival Blue (0 → 170): 53% + 23 × 15/40 % at 95 = 61.625% of 170 = 104.8.
    expect([tr(blue, 95), tr(blue, 170), tr(blue, 300)]).toEqual([105, 170, 170])
    expect(
      gymLadder(catalog, experiment, 95).find((row) => row.trainer.id === "brock"),
    ).toMatchObject({
      tr: 70,
      gap: -25,
      mark: "below",
    })
    expect(leagueCandidates(catalog, experiment, 300)).toEqual(
      leagueCandidates(catalog, experiment, 160),
    )
  })
})

describe("milestones", () => {
  const record = (id: string) => catalog.find((entry) => entry.id === id)!
  const timeline = (id: string) => milestones(record(id), defaults, evolution)

  it("lists Brock's team changes from world progress 0 to his peak TR", () => {
    const brock = timeline("brock")
    expect(brock[0]).toEqual({
      worldProgress: 0,
      tr: 25,
      teamLevel: 18,
      cap: 15,
      events: [{ kind: "start", team: ["Onix", "Geodude"], aboveCap: true }],
    })
    // TR 29 is the first TR at team size 3: steady Brock reaches it at world progress 8.
    expect(brock.find((m) => m.events.some((e) => e.kind === "join"))).toMatchObject({
      worldProgress: 8,
      tr: 29,
      events: [{ kind: "join", slot: 3, species: "Zubat", isAce: false }],
    })
    // Onix becomes Steelix when the team level reaches 35 (TR 52, world progress 57).
    const steelix = brock.find((m) =>
      m.events.some((e) => e.kind === "evolve" && e.to === "Steelix"),
    )!
    expect([steelix.worldProgress, steelix.tr, steelix.teamLevel]).toEqual([57, 52, 35])
    expect(resolveTrainer(record("brock"), defaults, 56, evolution).team[0]?.species).toBe("Onix")
    // TR 71 (team size 6) at world progress 98 brings in the slot-6 Aerodactyl ace at team level 45.
    expect(
      brock.find((m) => m.events.some((e) => e.kind === "join" && e.species === "Aerodactyl")),
    ).toMatchObject({
      worldProgress: 98,
      tr: 71,
      teamLevel: 45,
      events: [{ kind: "join", slot: 6, species: "Aerodactyl", isAce: true }],
    })
    // 25 + 75 × 159/160 = 99.53 rounds up to peak TR 100.
    expect(brock.at(-1)).toMatchObject({
      worldProgress: 159,
      tr: 100,
      events: [{ kind: "peak", tr: 100, reached: true }],
    })
    expect(brock.map(milestoneText)).toEqual([
      "0: Onix, Geodude (team level above the level cap)",
      "8: 3rd slot (Zubat) joins",
      "19: Zubat → Golbat",
      "27: Geodude → Graveler",
      "40: 4th slot (Kabuto) joins",
      "49: team level Lv 32 drops below the level cap Lv 33",
      "57: Onix → Steelix",
      "68: 5th slot (Omanyte) joins",
      "76: Graveler → Golem, Golbat → Crobat",
      "85: Kabuto → Kabutops, Omanyte → Omastar",
      "98: 6th slot (Aerodactyl) ace joins",
      "159: peak TR 100",
    ])
  })

  it("marks Blue's level cap crossing and peak TR", () => {
    expect(timeline("blue").map(milestoneText)).toEqual([
      "0: Eevee",
      "9: 2nd slot (Pidgey) joins",
      "22: Pidgey → Pidgeotto",
      "23: 3rd slot (Kadabra) ace joins",
      "28: team level Lv 25 passes the level cap Lv 24",
      "35: Eevee → Umbreon",
      "36: 4th slot (Nidorino) joins",
      "48: 5th slot (Scyther) joins",
      "49: Pidgeotto → Pidgeot, Nidorino → Nidoking",
      "55: Kadabra → Alakazam, Scyther → Scizor",
      "61: 6th slot (Arcanine) ace joins",
      "160: peak TR 170",
    ])
  })

  it("agrees with the resolved team at every world progress it scans", () => {
    for (const id of ["brock", "blue", "erika", "lance", "giovanni", "misty", "blaine", "agatha"]) {
      const list = timeline(id)
      const points = new Set(list.map((m) => m.worldProgress))
      const end = milestoneEnd(defaults, defaults.trainers[id]!.archetype)
      for (let world = 1; world <= end; world += 1) {
        const before = resolveTrainer(record(id), defaults, world - 1, evolution)
        const now = resolveTrainer(record(id), defaults, world, evolution)
        const changed =
          now.team.length !== before.team.length ||
          now.team.some((member, index) => member.species !== before.team[index]?.species)
        if (changed) expect([id, world, points.has(world)]).toEqual([id, world, true])
      }
    }
  })

  it("ignores ties with the level cap, so rounding cannot flicker a crossing", () => {
    // Erika's team level runs within one level of the cap from world progress 108 to 128 and
    // ties it from 113 to 122, but it crosses once: she starts above the cap and drops below.
    const crossings = timeline("erika").filter((m) => m.events.some((e) => e.kind === "cap"))
    expect(crossings.map(milestoneText)).toEqual([
      "121: team level Lv 75 drops below the level cap Lv 76",
    ])
  })

  it("notes a trainer already above the level cap at world progress 0", () => {
    expect(milestoneText(timeline("lorelei")[0]!)).toBe(
      "0: Lapras, Seel, Shellder (team level above the level cap)",
    )
  })

  it("reports a growth scaler that stops short of 100% and scans only to the ceilings", () => {
    const fixture = trainer("fixture", 10, six, { archetype: "veteran", peakTR: 60 })
    const experiment = experimentWith([fixture])
    experiment.archetypes.veteran = [
      [0, 0],
      [40, 50],
    ]
    const list = milestones(fixture, experiment, NO_EVOLUTION)
    // Growth stops at 50% (TR 35) at world progress 40; the level cap ceiling is TR 160.
    expect(milestoneEnd(experiment, "veteran")).toBe(160)
    expect(list.at(-1)?.events.at(-1)).toEqual({ kind: "peak", tr: 35, reached: false })
    expect(list.at(-1)?.worldProgress).toBe(160)
    experiment.archetypes.veteran = [
      [0, 0],
      [1_000_000, 100],
    ]
    expect(milestoneEnd(experiment, "veteran")).toBe(MILESTONE_SCAN_LIMIT)
  })
})

describe("roster validation", () => {
  const valid = trainer("fixture", 0).roster
  it("accepts six roster slots with roster slot 1 at offset 0", () => {
    expect(validateRoster(valid, "roster")).toEqual(valid)
  })
  it("accepts one to three aces, roster slot 1 always among them", () => {
    const withAces = (...aces: number[]) =>
      valid.map((rosterSlot, index) => ({
        ...rosterSlot,
        isAce: index === 0 || aces.includes(index),
      }))
    expect(validateRoster(withAces(), "roster").filter((s) => s.isAce)).toHaveLength(1)
    expect(validateRoster(withAces(2, 5), "roster").filter((s) => s.isAce)).toHaveLength(3)
    expect(() => validateRoster(withAces(2, 4, 5), "roster")).toThrow(
      "a roster has at most 3 aces (roster slot 1 plus two more)",
    )
    expect(() =>
      validateRoster([{ ...valid[0]!, isAce: false }, ...valid.slice(1)], "roster"),
    ).toThrow("roster slot 1 (the signature Pokémon) must be an ace")
    for (const record of catalog) {
      const aces = record.roster.filter((rosterSlot) => rosterSlot.isAce).length
      expect([record.id, record.roster[0]?.isAce, aces >= 1 && aces <= 3]).toEqual([
        record.id,
        true,
        true,
      ])
    }
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
    [
      "per-slot moves",
      [{ ...slot("Steelix", 0), moves: "LEVEL_UP" }],
      "roster slots carry no moves",
    ],
    ["a blank species", [slot(" ", 0)], "species"],
    ["an unknown key", [{ ...slot("Steelix", 0), ace: true }], "unknown fields"],
    [
      "a missing ace flag",
      [
        {
          species: "Steelix",
          levelOffset: 0,
          item: null,
          ability: null,
          nature: null,
        },
      ],
      "missing or unknown fields",
    ],
    [
      "a non-boolean ace flag",
      [{ ...slot("Steelix", 0), isAce: 1 }],
      "isAce must be true or false",
    ],
  ])("rejects %s", (_name, roster, message) => {
    expect(() => validateRoster(roster, "roster")).toThrow(message)
  })
})

describe("archetype growth", () => {
  const growth = DEFAULT_ARCHETYPE_GROWTH
  const fixtureExperiment = createExperiment([trainer("fixture", 0)])
  const trAt = (archetype: Archetype, start: number, peak: number) => (world: number) =>
    trainerRating(fixtureExperiment, { startTR: start, archetype, peakTR: peak }, world)

  it.each([
    ["steady", [0, 25, 50, 75, 100]],
    ["prodigy", [0, 50, 80, 95, 100]],
    ["sleeper", [0, 10, 25, 55, 100]],
    ["veteran", [0, 60, 100, 100, 100]],
  ] as const)(
    "reads %s growth at the anchors (start 0, peak 100 gives the growth %)",
    (name, pct) => {
      expect(WORLD_PROGRESS_CHECKPOINTS.map(trAt(name, 0, 100))).toEqual(pct)
      expect(growth[name].map(([world]) => world)).toEqual([...WORLD_PROGRESS_CHECKPOINTS])
    },
  )

  it.each([
    ["star", [0, 10, 50, 90, 100]],
    ["comeback", [0, 45, 50, 55, 100]],
    ["burst", [0, 25, 50, 75, 100]],
  ] as const)("reads %s growth at the §11 anchors", (name, pct) => {
    expect(WORLD_PROGRESS_CHECKPOINTS.map(trAt(name, 0, 100))).toEqual(pct)
    expect(growth[name].map(([world]) => world)).toEqual([...WORLD_PROGRESS_CHECKPOINTS])
  })

  it("makes Burst the only step scaler and lists all nine archetypes", () => {
    expect(ARCHETYPES).toEqual([
      "steady",
      "prodigy",
      "sleeper",
      "veteran",
      "rival",
      "legend",
      "star",
      "comeback",
      "burst",
    ])
    expect(ARCHETYPES.filter((name) => ARCHETYPE_KIND[name] === "step")).toEqual(["burst"])
  })

  it("interpolates Star and Comeback midpoints", () => {
    // A Star at 20 is 5%, at 60 is 30%, at 100 is 70%, at 140 is 95%.
    expect([20, 60, 100, 140].map(trAt("star", 0, 100))).toEqual([5, 30, 70, 95])
    // A Comeback at 20 is 22.5% (rounds up), at 60 is 47.5%, at 100 is 52.5%, at 140 is 77.5%.
    expect([20, 60, 100, 140].map(trAt("comeback", 0, 100))).toEqual([23, 48, 53, 78])
    // A Star 26 -> 110 at 100: 26 + 84 × 70% = 84.8.
    expect(trAt("star", 26, 110)(100)).toBe(85)
  })

  it("holds a Burst between anchors and jumps at 4, 8, 16 and 24 badges", () => {
    const read = trAt("burst", 24, 166)
    // 24 + 142 × 25% = 59.5 rounds up; 50% = 95; 75% = 130.5 rounds up.
    expect([0, 39, 40, 79, 80, 119, 120, 159, 160, 400].map(read)).toEqual([
      24, 24, 60, 60, 95, 95, 131, 131, 166, 166,
    ])
    // At the anchors a Burst equals a Steady; between them it holds below it.
    for (const world of WORLD_PROGRESS_CHECKPOINTS)
      expect(read(world)).toBe(trAt("steady", 24, 166)(world))
    expect(read(60)).toBeLessThan(trAt("steady", 24, 166)(60))
  })

  it("keeps a Legend at start TR at every world progress", () => {
    expect(growth.legend).toEqual([
      [0, 0],
      [160, 0],
    ])
    const read = trAt("legend", 95, 95)
    for (let world = 0; world <= 200; world += 1) expect(read(world)).toBe(95)
    expect(read(1_000_000)).toBe(95)
  })

  it("reads the Rival as an ordinary growth scaler: 0/20/40/80/120/160 -> 0/15/29/53/76/100%", () => {
    expect([0, 20, 40, 80, 120, 160].map(trAt("rival", 0, 100))).toEqual([0, 15, 29, 53, 76, 100])
    // The same start + (peak - start) x growth% rule as every archetype, so a start TR counts.
    expect(trAt("rival", 20, 120)(80)).toBe(73)
  })

  it("interpolates midpoints exactly and rounds halves up once", () => {
    // Steady 0 -> 100 at world progress 20 is 12.5%: 12.5 rounds up to 13.
    expect(trAt("steady", 0, 100)(20)).toBe(13)
    // Start 10, peak 50, a Steady at 20: 10 + 40 * 12.5% = 15 (no rounding of the % first).
    expect(trAt("steady", 10, 50)(20)).toBe(15)
    // A Sleeper 20 -> 180 at 100: 20 + 40% of 160 = 84.
    expect(trAt("sleeper", 20, 180)(100)).toBe(84)
    // A Prodigy 0 -> 90 at 60: 65% of 90 = 58.5 rounds up.
    expect(trAt("prodigy", 0, 90)(60)).toBe(59)
    // A Veteran 45 -> 94 at 20: 30% of 49 = 14.7.
    expect(trAt("veteran", 45, 94)(20)).toBe(60)
    expect(growthTR(growth.steady, 2, 95, 1)).toBe(3) // 2 + 93 * 0.625% = 2.58
  })

  it("stays flat past world progress 160 and keeps start at 0", () => {
    for (const name of ARCHETYPES.filter((name) => name !== "legend")) {
      const read = trAt(name, 12, 172)
      expect(read(0)).toBe(12)
      expect([read(160), read(200), read(1_000_000)]).toEqual([172, 172, 172])
    }
  })

  it("never lowers a trainer TR as world progress rises", () => {
    for (const record of catalog) {
      let previous = -1
      for (let world = 0; world <= 200; world += 1) {
        const tr = resolveTrainer(record, defaults, world, evolution).tr
        expect(tr).toBeGreaterThanOrEqual(previous)
        expect(tr).toBeLessThanOrEqual(record.peakTR)
        previous = tr
      }
    }
  })

  it("rejects a negative or fractional world progress", () => {
    expect(() => trAt("steady", 0, 100)(-1)).toThrow("World progress")
    expect(() => trAt("steady", 0, 100)(2.5)).toThrow("World progress")
  })
})

describe("placeholder balance targets", () => {
  const cap = (world: number) => scale(LEVEL_CAP_ANCHORS, world)

  it.each([...WORLD_PROGRESS_CHECKPOINTS])(
    "spreads the 24 Gym Leader entries below, near and above the player at world progress %i",
    (world) => {
      const ladder = gymLadder(catalog, defaults, world)
      expect(ladder).toHaveLength(24)
      expect(ladder.map((row) => row.trainer.id)).toContain("tate-liza")
      const count = (mark: string) => ladder.filter((row) => row.mark === mark).length
      expect(count("above")).toBeGreaterThanOrEqual(3)
      if (world === 0) {
        // Every start TR is in the Gym band (18–40), above NEAR_BAND, so every Gym Leader is
        // above the player at world progress 0. The lowest three are the approachable openers:
        // two Pokémon at or under the level cap.
        expect(count("above")).toBe(24)
        for (const row of ladder.slice(0, 3)) {
          const opener = resolveTrainer(row.trainer, defaults, 0, evolution)
          expect(opener.size).toBe(2)
          expect(opener.teamLevel).toBeLessThanOrEqual(cap(0))
        }
      } else if (world === 40) {
        // The lowest Sleeper still sits near the player at world progress 40, so none is
        // below yet; the low end is at least three Gym Leaders at or under the player TR.
        expect(count("near")).toBeGreaterThanOrEqual(3)
        expect(ladder.filter((row) => row.gap <= 0).length).toBeGreaterThanOrEqual(3)
      } else {
        expect(count("near")).toBeGreaterThanOrEqual(3)
        expect(count("below")).toBeGreaterThanOrEqual(3)
      }
      for (const row of ladder) expect(Math.abs(row.gap) <= NEAR_BAND).toBe(row.mark === "near")
    },
  )

  it("makes the hardest leaders at 24 badges Sleepers or high-peak Steadies", () => {
    const top = gymLadder(catalog, defaults, 160).slice(-5)
    for (const row of top) {
      const settings = defaults.trainers[row.trainer.id]!
      expect(
        settings.archetype === "sleeper" ||
          (settings.archetype === "steady" && settings.peakTR >= 170),
      ).toBe(true)
    }
  })

  it("starts every Gym Leader in the 18–40 band by archetype, and none is a Legend", () => {
    const bands: Record<string, [number, number]> = {
      sleeper: [18, 26],
      star: [18, 26],
      prodigy: [22, 30],
      steady: [24, 34],
      burst: [24, 34],
      veteran: [30, 40],
      comeback: [30, 40],
    }
    const gyms = catalog.filter(isGymLeader)
    expect(gyms).toHaveLength(24)
    for (const record of gyms) {
      expect([record.id, record.archetype in bands]).toEqual([record.id, true])
      const [low, high] = bands[record.archetype]!
      expect([record.id, record.startTR >= low && record.startTR <= high]).toEqual([
        record.id,
        true,
      ])
    }
  })

  it("opens every Gym Leader at team level 12 or more, within 16 levels of each other", () => {
    const levels = catalog
      .filter(isGymLeader)
      .map((record) => resolveTrainer(record, defaults, 0, evolution).teamLevel)
    for (const level of levels) expect(level).toBeGreaterThanOrEqual(12)
    expect(Math.max(...levels) - Math.min(...levels)).toBeLessThanOrEqual(16)
    // Brock (steady, start TR 25) opens at Lv 18 with Onix and Geodude.
    const brock = at(0).find((row) => row.trainer.id === "brock")!
    expect([brock.tr, brock.teamLevel, brock.size]).toEqual([25, 18, 2])
    expect(brock.team.map((member) => [member.species, member.level])).toEqual([
      ["Onix", 18],
      ["Geodude", 16],
    ])
  })

  it("starts Blue at one Eevee at Lv 5 and grows him with the Rival scaler", () => {
    const blue = (world: number) => at(world).find((row) => row.trainer.id === "blue")!
    const pallet = blue(0)
    expect([pallet.tr, pallet.size, pallet.teamLevel]).toEqual([0, 1, 5])
    expect(pallet.battleOrder.map((member) => [member.species, member.level])).toEqual([
      ["Eevee", 5],
    ])
    // Cerulean, about world progress 20: TR about 25 (25.5 rounds up), two Pokémon near Lv 18.
    const cerulean = blue(20)
    expect([cerulean.tr, cerulean.size, cerulean.teamLevel]).toEqual([26, 2, 18])
    expect(blue(40).tr).toBe(49)
  })

  it("gives every notable trainer a full team of six at their peak TR", () => {
    for (const record of catalog) {
      const settings = defaults.trainers[record.id]!
      expect([record.id, teamSizeFor(defaults, settings.peakTR)]).toEqual([record.id, 6])
      const peak = resolveTrainer(
        record,
        defaults,
        milestoneEnd(defaults, settings.archetype),
        evolution,
      )
      expect([record.id, peak.tr, peak.size, peak.team.length]).toEqual([
        record.id,
        settings.peakTR,
        6,
        6,
      ])
    }
  })

  it("keeps Blue about 10 ahead of the player from world progress 40 until his peak", () => {
    const record = catalog.find((entry) => entry.id === "blue")!
    for (let world = 40; world <= 160; world += 1) {
      const gap = resolveTrainer(record, defaults, world, evolution).tr - world
      expect(gap).toBeGreaterThanOrEqual(9)
      expect(gap).toBeLessThanOrEqual(10)
    }
    expect(resolveTrainer(record, defaults, 400, evolution).tr).toBe(170)
  })
})

describe("league lineups", () => {
  const home = { homeRegion: "Kanto", travel: "homebody" } as const
  it("scores willingness as 100 - travel cost - fatigue, at least 5", () => {
    // At home: no travel cost.
    expect(willingness("Indigo", home, false)).toEqual({
      home: true,
      travelCost: 0,
      fatigue: 0,
      score: 100,
    })
    expect(willingness("Indigo", { homeRegion: "Johto", travel: "homebody" }, false).score).toBe(
      100,
    )
    expect(willingness("Hoenn", { homeRegion: "Hoenn", travel: "homebody" }, false).score).toBe(100)
    // Away: a homebody pays 80, a traveller 10.
    expect(willingness("Hoenn", home, false)).toEqual({
      home: false,
      travelCost: 80,
      fatigue: 0,
      score: 20,
    })
    expect(willingness("Indigo", { homeRegion: "Hoenn", travel: "traveller" }, false)).toEqual({
      home: false,
      travelCost: 10,
      fatigue: 0,
      score: 90,
    })
    // Fatigue: 50 for a trainer in the previous league's lineup.
    expect(willingness("Indigo", home, true)).toEqual({
      home: true,
      travelCost: 0,
      fatigue: 50,
      score: 50,
    })
    expect(willingness("Hoenn", { homeRegion: "Johto", travel: "traveller" }, true).score).toBe(40)
    // The floor: an away, fatigued homebody would be -30 but keeps 5.
    expect(willingness("Hoenn", home, true)).toEqual({
      home: false,
      travelCost: 80,
      fatigue: 50,
      score: 5,
    })
  })

  it("treats the Sevii Masters as a neutral location: everyone is at home", () => {
    for (const homeRegion of ["Kanto", "Johto", "Hoenn"] as const)
      for (const travel of ["homebody", "traveller"] as const) {
        expect(willingness("Sevii Masters", { homeRegion, travel }, false)).toEqual({
          home: true,
          travelCost: 0,
          fatigue: 0,
          score: 100,
        })
        expect(willingness("Sevii Masters", { homeRegion, travel }, true).score).toBe(50)
      }
  })

  it("floors the league score: floor(TR × willingness / 100)", () => {
    expect(leagueScore(100, 100)).toBe(100)
    expect(leagueScore(87, 90)).toBe(78) // 78.3
    expect(leagueScore(99, 5)).toBe(4) // 4.95
    expect(leagueScore(55, 50)).toBe(27) // 27.5
    expect(leagueScore(150, 20)).toBe(30)
    expect(leagueScore(0, 100)).toBe(0)
  })

  it("ranks every eligible trainer by league score and takes the top five, ties in catalog order", () => {
    const records = [
      { ...trainer("duo", 150), leagueEligible: false, doubleBattle: true },
      trainer("a", 80),
      trainer("b", 90, six, { homeRegion: "Hoenn", travel: "traveller" }),
      trainer("c", 200, six, { homeRegion: "Hoenn" }),
      trainer("d", 60),
      trainer("e", 60, six, { homeRegion: "Johto" }),
      trainer("f", 60),
      trainer("g", 61),
    ]
    const ranking = rankLeague(
      "Indigo",
      0,
      leagueCandidates(records, experimentWith(records), 0),
      new Set(),
    )
    // The duo fights doubles, so it is not a candidate.
    expect(ranking.entrants.map((entrant) => [entrant.trainer.id, entrant.score])).toEqual([
      ["b", 81],
      ["a", 80],
      ["g", 61],
      ["d", 60],
      ["e", 60],
      ["f", 60],
      // The strongest trainer, a homebody far from home, scores floor(200 × 20 / 100).
      ["c", 40],
    ])
    expect(ranking.entrants.map((entrant) => entrant.rank)).toEqual([1, 2, 3, 4, 5, 6, 7])
    expect(ranking.entrants.map((entrant) => entrant.inLineup)).toEqual([
      true,
      true,
      true,
      true,
      true,
      false,
      false,
    ])
    // Battle order: ascending TR, strongest last; equal TRs in catalog order.
    expect(ranking.lineup.map((entrant) => entrant.trainer.id)).toEqual(["d", "e", "g", "a", "b"])
    // Fewer than five eligible trainers: all of them.
    expect(
      rankLeague(
        "Hoenn",
        0,
        leagueCandidates(records, experimentWith(records), 0).slice(0, 3),
        new Set(),
      ).lineup,
    ).toHaveLength(3)
  })

  it("lets fatigue push a trainer out of the lineup", () => {
    const records = [
      trainer("a", 100),
      trainer("b", 60),
      trainer("c", 58),
      trainer("d", 56),
      trainer("e", 54),
      trainer("f", 52),
    ]
    const candidates = leagueCandidates(records, experimentWith(records), 0)
    const ids = (previous: string[]) =>
      rankLeague("Indigo", 0, candidates, new Set(previous)).lineup.map(
        (entrant) => entrant.trainer.id,
      )
    expect(ids([])).toEqual(["e", "d", "c", "b", "a"])
    // Fatigued, a scores 50 and drops below f (52).
    expect(ids(["a"])).toEqual(["f", "e", "d", "c", "b"])
    const entrant = rankLeague("Indigo", 0, candidates, new Set(["a"])).entrants.find(
      (row) => row.trainer.id === "a",
    )!
    expect(entrant).toMatchObject({ score: 50, rank: 6, inLineup: false })
    expect(entrant.willingness).toEqual({ home: true, travelCost: 0, fatigue: 50, score: 50 })
  })

  it("scores everyone at their TR at the neutral Sevii Masters, so the top five by TR who join make it", () => {
    for (const world of WORLD_PROGRESS_CHECKPOINTS) {
      const candidates = leagueCandidates(catalog, defaults, world)
      const ranking = rankLeague("Sevii Masters", world, candidates, new Set())
      for (const entrant of ranking.entrants) {
        expect(entrant.willingness.home).toBe(true)
        expect(entrant.score).toBe(entrant.tr)
      }
      const joining = new Set(
        ranking.entrants.filter((entrant) => entrant.joins).map((entrant) => entrant.trainer.id),
      )
      const byTR = candidates
        .filter((candidate) => joining.has(candidate.trainer.id))
        .toSorted((a, b) => b.tr - a.tr || a.order - b.order)
        .slice(0, 5)
        .map((candidate) => candidate.trainer.id)
      expect(ranking.lineup.map((entrant) => entrant.trainer.id).toSorted()).toEqual(
        byTR.toSorted(),
      )
    }
  })

  it("ranks every league-eligible catalog trainer and fields five distinct ones, strongest last", () => {
    const eligible = catalog.filter((record) => record.leagueEligible).length
    for (const at of [null, 0, 40, 97, 200])
      for (const ranking of leagueSequence(catalog, defaults, leagueWorlds(at))) {
        expect(ranking.entrants).toHaveLength(eligible)
        expect(ranking.entrants.map((entrant) => entrant.trainer.id)).not.toContain("tate-liza")
        // Those who join come first, highest score first and ranked 1..n; the aloof who skip follow.
        const joins = ranking.entrants.map((entrant) => entrant.joins)
        expect(joins).toEqual(joins.toSorted((a, b) => Number(b) - Number(a)))
        const joining = ranking.entrants.filter((entrant) => entrant.joins)
        const scores = joining.map((entrant) => entrant.score)
        expect(scores).toEqual(scores.toSorted((a, b) => b - a))
        expect(joining.map((entrant) => entrant.rank)).toEqual(joining.map((_, index) => index + 1))
        for (const entrant of ranking.entrants.filter((row) => !row.joins))
          expect(entrant).toMatchObject({ aloof: true, rank: null, inLineup: false })
        const ids = ranking.lineup.map((entrant) => entrant.trainer.id)
        expect(new Set(ids).size).toBe(5)
        expect(
          ranking.entrants
            .filter((entrant) => entrant.inLineup)
            .map((entrant) => entrant.trainer.id)
            .toSorted(),
        ).toEqual(ids.toSorted())
        const trs = ranking.lineup.map((entrant) => entrant.tr)
        expect(trs).toEqual(trs.toSorted((a, b) => a - b))
        for (const entrant of ranking.entrants)
          expect(entrant.score).toBe(Math.floor((entrant.tr * entrant.willingness.score) / 100))
      }
  })

  it("reads home region and travel style from the experiment", () => {
    const edited = createExperiment(catalog)
    edited.trainers.norman!.travel = "traveller"
    const candidates = leagueCandidates(catalog, edited, 80)
    const norman = candidates.find((entry) => entry.trainer.id === "norman")!
    expect(norman.travel).toBe("traveller")
    const ranked = rankLeague("Indigo", 80, candidates, new Set()).entrants.find(
      (entry) => entry.trainer.id === "norman",
    )!
    expect(ranked.willingness.score).toBe(90)
    expect(ranked.score).toBe(Math.floor((norman.tr * 90) / 100))
  })

  it("enters Indigo, then the Sevii Masters, then Hoenn, fatiguing the previous lineup", () => {
    const worlds = leagueWorlds()
    expect(worlds).toEqual({ Indigo: 80, "Sevii Masters": 120, Hoenn: 160 })
    expect(leagueWorlds(97)).toEqual({ Indigo: 97, "Sevii Masters": 97, Hoenn: 97 })
    for (const sequenceWorlds of [worlds, leagueWorlds(97)]) {
      const [indigo, masters, hoenn] = leagueSequence(catalog, defaults, sequenceWorlds)
      expect([indigo!.league, masters!.league, hoenn!.league]).toEqual([
        "Indigo",
        "Sevii Masters",
        "Hoenn",
      ])
      for (const entrant of indigo!.entrants) expect(entrant.willingness.fatigue).toBe(0)
      for (const [ranking, before] of [
        [masters!, indigo!],
        [hoenn!, masters!],
      ] as const)
        for (const entrant of ranking.entrants)
          expect(entrant.willingness.fatigue).toBe(
            before.lineup.some((previous) => previous.trainer.id === entrant.trainer.id) ? 50 : 0,
          )
    }
    // At one world progress, fatigue keeps the Sevii Masters from repeating Indigo's lineup.
    const [indigo, masters] = leagueSequence(catalog, defaults, leagueWorlds(97))
    expect(masters!.lineup.map((entrant) => entrant.trainer.id)).not.toEqual(
      indigo!.lineup.map((entrant) => entrant.trainer.id),
    )
  })

  describe("aloof trainers", () => {
    // Team level = TR + 1, so each fixture's team level reads straight off its TR.
    const levelled = (records: TrainerRecord[]): Experiment => ({
      ...experimentWith(records),
      teamLevel: [
        [0, 1],
        [99, 100],
      ],
    })
    const rank = (records: TrainerRecord[], previous: string[] = [], league = "Indigo" as const) =>
      rankLeague(league, 0, leagueCandidates(records, levelled(records), 0), new Set(previous))
    const field = ["a", "b", "c", "d", "e"].map((id) => trainer(id, 50))
    const ids = (entrants: { trainer: TrainerRecord }[]) =>
      entrants.map((entrant) => entrant.trainer.id)

    it("joins at exactly the field level + the margin and skips one level above it", () => {
      expect(ALOOF_MARGIN).toBe(10)
      const ranking = rank([
        ...field,
        trainer("at", 60, six, { aloof: true }),
        trainer("over", 61, six, { aloof: true }),
      ])
      // The field: the top five non-aloof by league score, strongest team level Lv 51.
      expect(ranking.fieldLevel).toBe(51)
      const at = ranking.entrants.find((entrant) => entrant.trainer.id === "at")!
      const over = ranking.entrants.find((entrant) => entrant.trainer.id === "over")!
      expect(at).toMatchObject({ teamLevel: 61, joins: true, rank: 1, inLineup: true })
      expect(over).toMatchObject({ teamLevel: 62, joins: false, rank: null, inLineup: false })
      // The skipping trainer is listed after everyone who joins, and never fights.
      expect(ids(ranking.entrants).at(-1)).toBe("over")
      expect(ids(ranking.lineup)).toEqual(["a", "b", "c", "d", "at"])
    })

    it("compares the aloof with the non-aloof field only, never with each other", () => {
      const ranking = rank([
        ...field,
        trainer("x", 60, six, { aloof: true }),
        trainer("w", 70, six, { aloof: true }),
      ])
      // x (Lv 61) outscores the field but is aloof, so it does not raise the field level; w (Lv 71)
      // would pass against x (61 + 10) but is judged against the field (51 + 10) and skips.
      expect(ranking.fieldLevel).toBe(51)
      expect(ranking.entrants.find((entrant) => entrant.trainer.id === "x")!.joins).toBe(true)
      expect(ranking.entrants.find((entrant) => entrant.trainer.id === "w")!.joins).toBe(false)
      // A non-aloof trainer is never judged, however far above the field.
      const strong = rank([...field, trainer("strong", 99)])
      expect(strong.entrants.find((entrant) => entrant.trainer.id === "strong")).toMatchObject({
        joins: true,
        rank: 1,
      })
    })

    it("still applies fatigue: to the field that sets the level and to the aloof trainer's score", () => {
      const records = [
        trainer("a", 80),
        ...["b", "c", "d", "e", "f"].map((id) => trainer(id, 50)),
        trainer("x", 88, six, { aloof: true }),
      ]
      // Fresh, a (Lv 81) is in the field, so x (Lv 89) joins.
      const fresh = rank(records)
      expect(fresh.fieldLevel).toBe(81)
      expect(fresh.entrants.find((entrant) => entrant.trainer.id === "x")!.joins).toBe(true)
      // a fatigued scores 40 and leaves the field's top five: the field level drops and x skips.
      const tired = rank(records, ["a"])
      expect(tired.fieldLevel).toBe(51)
      expect(tired.entrants.find((entrant) => entrant.trainer.id === "x")!.joins).toBe(false)
      // x fatigued still joins (team level, not league score, decides) but scores 44 and drops out.
      const tiredAloof = rank(records, ["x"])
      expect(tiredAloof.entrants.find((entrant) => entrant.trainer.id === "x")).toMatchObject({
        joins: true,
        score: 44,
        inLineup: false,
      })
      expect(ids(tiredAloof.lineup)).toEqual(["b", "c", "d", "e", "a"])
    })

    it("keeps Lance out of the early leagues and fields him late in the standard sequence", () => {
      const [indigo, masters, hoenn] = leagueSequence(catalog, defaults, leagueWorlds())
      const lance = (ranking: typeof indigo) =>
        ranking!.entrants.find((entrant) => entrant.trainer.id === "lance")!
      // A Legend at TR 200 (Lv 100) is far above the Indigo and Masters fields.
      expect(lance(indigo)).toMatchObject({
        tr: 200,
        teamLevel: 100,
        joins: false,
        inLineup: false,
      })
      expect(lance(masters)).toMatchObject({ joins: false, inLineup: false })
      expect(lance(hoenn)).toMatchObject({ joins: true, inLineup: true })
      expect(hoenn!.lineup.at(-1)!.trainer.id).toBe("lance")
    })

    it("never fields an aloof trainer more than the margin above the field", () => {
      for (const at of [null, 0, 40, 80, 97, 120, 160, 200])
        for (const ranking of leagueSequence(catalog, defaults, leagueWorlds(at))) {
          expect(ranking.fieldLevel).not.toBeNull()
          for (const entrant of ranking.entrants.filter((row) => row.aloof))
            expect(entrant.joins).toBe(entrant.teamLevel <= ranking.fieldLevel! + ALOOF_MARGIN)
          for (const entrant of ranking.lineup.filter((row) => row.aloof))
            expect(entrant.teamLevel - ranking.fieldLevel!).toBeLessThanOrEqual(ALOOF_MARGIN)
        }
    })
  })

  it("is deterministic: no seed, the same inputs give the same lineups in any input order", () => {
    const worlds = leagueWorlds()
    expect(leagueSequence(catalog, defaults, worlds)).toEqual(
      leagueSequence(catalog, defaults, worlds),
    )
    const candidates = leagueCandidates(catalog, defaults, 120)
    const previous = new Set(["blue", "lance", "steven"])
    expect(rankLeague("Hoenn", 120, candidates.toReversed(), previous)).toEqual(
      rankLeague("Hoenn", 120, candidates, previous),
    )
  })
})

describe("catalog", () => {
  it("ships 38 entries (37 characters and the Tate & Liza duo) with placeholder TRs and valid rosters of at most six", () => {
    expect(catalog).toHaveLength(38)
    const experiment = createExperiment(catalog)
    for (const record of catalog) {
      expect(Number.isInteger(record.startTR) && record.startTR >= 0).toBe(true)
      expect(record.peakTR).toBeGreaterThanOrEqual(record.startTR)
      expect(Object.hasOwn(record, "lead")).toBe(false)
      expect(record.leagueEligible).toBe(!record.doubleBattle)
      expect(record.trSource).toContain("PLACEHOLDER")
      expect(
        record.trSource.includes("placeholder start TR in the 18–40 Gym band by archetype"),
      ).toBe(isGymLeader(record))
      expect(record.roster[0]?.levelOffset).toBe(0)
      expect(record.roster.length).toBeLessThanOrEqual(6)
    }
    const growth = (name: string) => {
      const record = catalog.find((entry) => entry.name === name)!
      return [record.startTR, record.archetype, record.peakTR]
    }
    expect(growth("Blue")).toEqual([0, "rival", 170])
    expect(catalog.filter((record) => record.archetype === "rival")).toHaveLength(1)
    expect(growth("Brock")).toEqual([25, "steady", 100])
    expect(growth("Lance")).toEqual([200, "legend", 200])
    for (const record of catalog)
      if (record.archetype === "legend") expect(record.peakTR).toBe(record.startTR)
    expect(Object.keys(experiment.trainers)).toHaveLength(38)
    expect(catalog.some((record) => record.id.startsWith("red"))).toBe(false)
  })

  it("applies the §11 archetype reassignment and leaves everyone else unchanged", () => {
    const byArchetype = (archetype: Archetype) =>
      catalog.filter((record) => record.archetype === archetype).map((record) => record.name)
    expect(byArchetype("star")).toEqual([
      "Misty",
      "Bugsy",
      "Whitney",
      "Flannery",
      "Tate & Liza",
      "Wallace",
    ])
    expect(byArchetype("comeback")).toEqual(["Blaine", "Bruno", "Pryce"])
    expect(byArchetype("burst")).toEqual(["Giovanni", "Chuck", "Brawly", "Steven"])
    expect(byArchetype("legend")).toEqual(["Agatha", "Lance"])
    expect(byArchetype("veteran")).toEqual(["Lt. Surge", "Lorelei", "Wattson", "Glacia", "Drake"])
    // §16: the Champions follow lore, so no Champion is a Sleeper.
    expect(byArchetype("sleeper")).toEqual(["Sabrina", "Morty", "Clair", "Winona", "Juan"])
    expect(byArchetype("prodigy")).toEqual(["Janine", "Falkner", "Will", "Sidney"])
    expect(byArchetype("steady")).toEqual([
      "Brock",
      "Erika",
      "Koga",
      "Jasmine",
      "Karen",
      "Roxanne",
      "Norman",
      "Phoebe",
    ])
    expect(byArchetype("rival")).toEqual(["Blue"])
    const agatha = catalog.find((record) => record.name === "Agatha")!
    expect([agatha.startTR, agatha.peakTR]).toEqual([95, 95])
  })

  it("gives the Champions their §16 lore archetypes: Lance a Legend at 200, Steven a Burst, Wallace a Star", () => {
    const growth = (id: string) => {
      const record = catalog.find((entry) => entry.id === id)!
      return [record.startTR, record.archetype, record.peakTR]
    }
    expect(growth("lance")).toEqual([200, "legend", 200])
    expect(growth("steven")).toEqual([50, "burst", 195])
    expect(growth("wallace")).toEqual([48, "star", 190])
    for (const world of [0, 80, 160, 300])
      expect(trainerRating(defaults, defaults.trainers.lance!, world)).toBe(200)
  })

  it("assigns the §16 aloof trait, independent of archetype and travel style", () => {
    expect(catalog.filter((record) => record.aloof).map((record) => record.name)).toEqual([
      "Sabrina",
      "Agatha",
      "Lance",
      "Clair",
      "Karen",
      "Glacia",
      "Wallace",
      "Steven",
    ])
    for (const record of catalog) {
      expect(typeof record.aloof).toBe("boolean")
      expect(defaults.trainers[record.id]!.aloof).toBe(record.aloof)
      if (record.aloof) expect(record.leagueEligible).toBe(true)
    }
  })

  it("adds Tate & Liza as one league-ineligible Gym Leader duo fought as a double battle", () => {
    const duo = catalog.find((record) => record.id === "tate-liza")!
    expect(duo).toMatchObject({
      name: "Tate & Liza",
      role: "Gym Leader duo",
      region: "Hoenn",
      doubleBattle: true,
      leagueEligible: false,
    })
    expect(duo.roster).toHaveLength(6)
    expect(duo.roster.slice(0, 2).map((slot) => [slot.species, slot.levelOffset])).toEqual([
      ["Solrock", 0],
      ["Lunatone", 0],
    ])
    expect(duo.roster.map((slot) => [slot.species, slot.isAce])).toEqual([
      ["Solrock", true],
      ["Lunatone", true],
      ["Claydol", false],
      ["Xatu", false],
      ["Grumpig", false],
      ["Gardevoir", true],
    ])
    expect(duo.rosterSource).toContain("user-directed roster draft v1")
    expect(catalog.filter((record) => record.doubleBattle).map((record) => record.id)).toEqual([
      "tate-liza",
    ])
  })

  it("assigns the §14 home regions and travel styles", () => {
    const named = (test: (record: TrainerRecord) => boolean) =>
      catalog.filter(test).map((record) => record.name)
    expect(named((record) => record.travel === "traveller")).toEqual([
      "Brock",
      "Misty",
      "Giovanni",
      "Blue",
      "Bruno",
      "Koga",
      "Lance",
      "Bugsy",
      "Will",
      "Karen",
      "Brawly",
      "Glacia",
      "Drake",
      "Wallace",
      "Steven",
    ])
    expect(named((record) => record.homeRegion === "Johto")).toEqual([
      "Koga",
      "Falkner",
      "Bugsy",
      "Whitney",
      "Morty",
      "Chuck",
      "Jasmine",
      "Pryce",
      "Clair",
      "Will",
      "Karen",
    ])
    expect(named((record) => record.homeRegion === "Kanto")).toHaveLength(13)
    expect(named((record) => record.homeRegion === "Hoenn")).toHaveLength(14)
    for (const record of catalog) {
      expect(defaults.trainers[record.id]!.homeRegion).toBe(record.homeRegion)
      expect(defaults.trainers[record.id]!.travel).toBe(record.travel)
    }
  })

  it("records the section 9 table levels and the game's own levels for level evolutions", () => {
    const edges = (species: string[]) =>
      species.map((name) => data.evolution.chains[name]?.slice(-3))
    // Shared table rows: evolutions without a level in the game data.
    expect(edges(["Steelix", "Starmie", "Arcanine"])).toEqual([
      ["Onix", 35, "Steelix"],
      ["Staryu", 30, "Starmie"],
      ["Growlithe", 35, "Arcanine"],
    ])
    // Trade evolutions this game also gives a level: the game's level wins.
    expect(edges(["Alakazam", "Gengar", "Golem", "Machamp"])).toEqual([
      ["Kadabra", 42, "Alakazam"],
      ["Haunter", 42, "Gengar"],
      ["Graveler", 38, "Golem"],
      ["Machoke", 38, "Machamp"],
    ])
  })

  it("authors final stages except the draft's Gen 4 Primeape, Ursaring and Girafarig, each with a chain", () => {
    const notFinal = catalog.flatMap((record) =>
      record.roster.flatMap((rosterSlot, index) => {
        expect(evolution.lines.has(rosterSlot.species)).toBe(true)
        return evolutionStatus(evolution, rosterSlot.species).final
          ? []
          : [`${record.id}[${index + 1}] ${rosterSlot.species}`]
      }),
    )
    expect(notFinal).toEqual([
      "bruno[6] Primeape",
      "whitney[4] Ursaring",
      "whitney[5] Girafarig",
      "chuck[2] Primeape",
    ])
    const brock = catalog.find((record) => record.id === "brock")!
    expect(
      brock.roster.map((rosterSlot) => [
        rosterSlot.species,
        rosterSlot.levelOffset,
        rosterSlot.isAce,
      ]),
    ).toEqual([
      ["Steelix", 0, true],
      ["Golem", -2, false],
      ["Crobat", -2, false],
      ["Kabutops", -2, false],
      ["Omastar", -2, false],
      ["Aerodactyl", 0, true],
    ])
    // Species in Brock's source party keep their item; roster slots carry no moves.
    expect(brock.roster.map((rosterSlot) => rosterSlot.item)).toEqual([
      null,
      "Quick Claw",
      null,
      "Scope Lens",
      "Focus Band",
      "Hard Stone",
    ])
    expect(catalog.every((record) => record.roster.every((entry) => !("moves" in entry)))).toBe(
      true,
    )
    // Blue's signature Umbreon steps down to Eevee early.
    expect(catalog.find((record) => record.id === "blue")!.roster[0]).toMatchObject({
      species: "Umbreon",
      isAce: true,
    })
  })

  it("records level-up and TM/tutor learnsets for every stage on every roster line", () => {
    for (const line of evolution.lines.values())
      for (const stage of line)
        expect([stage.species, learnsets.species.has(stage.species)]).toEqual([stage.species, true])
    const onix = learnsets.species.get("Onix")!
    expect(onix.levelUp.slice(0, 4)).toEqual([
      [1, "Mud Sport"],
      [1, "Tackle"],
      [1, "Harden"],
      [1, "Bind"],
    ])
    expect(onix.levelUp).toContainEqual([7, "Rock Throw"])
    // Earthquake is a TM move for the whole Geodude line and Steelix.
    for (const species of ["Geodude", "Graveler", "Golem", "Steelix"])
      expect(learnsets.species.get(species)!.learnable.has("Earthquake")).toBe(true)
    expect(learnsets.species.get("Zubat")!.learnable.has("Earthquake")).toBe(false)
    expect(learnsets.moves.has("Stone Edge")).toBe(true)
    // An old alias is not a move of its own: Faint Attack is Feint Attack.
    expect(learnsets.moves.has("Faint Attack")).toBe(false)
    expect(learnsets.moves.has("Feint Attack")).toBe(true)
  })

  it("records each line's egg moves on its first stage, from the species its Egg hatches as", () => {
    const egg = (species: string) => [...learnsets.species.get(species)!.egg]
    for (const [species, line] of evolution.lines)
      if (line[0]?.species !== species) expect([species, egg(species)]).toEqual([species, []])
    // Dratini, Feebas and Meowth lay their own Eggs.
    expect(egg("Dratini")).toContain("Extreme Speed")
    expect(egg("Feebas")).toContain("Mirror Coat")
    expect(egg("Meowth")).toContain("Foul Play")
    // A Pikachu Egg hatches as Pichu, so the Pikachu line has Pichu's egg moves.
    expect(egg("Pikachu")).toEqual(expect.arrayContaining(["Fake Out", "Wish", "Encore"]))
    // Egg moves count for later stages: Dragonite learns Extreme Speed as an egg move.
    expect(moveSource("Dragonite", "Extreme Speed", learnsets, evolution)).toEqual({ kind: "egg" })
  })

  it("uses the user-directed pool draft: 8–12 ordered entries, from levels only where needed", () => {
    const brock = catalog.find((record) => record.id === "brock")!
    expect(brock.movePoolSource).toBe(
      "user-directed pool draft v1: hazards and sand walls (Sturdy walls, Stealth Rock, chip)",
    )
    // Curse stands in for Iron Defense, which no roster line learns.
    expect(brock.movePool).toEqual([
      { move: "Bind" },
      { move: "Stealth Rock" },
      { move: "Sandstorm", fromLevel: 20 },
      { move: "Curse" },
      { move: "Stone Edge" },
      { move: "Earthquake" },
      { move: "Rock Slide" },
      { move: "Heavy Slam" },
      { move: "Rock Blast" },
      { move: "Cross Poison" },
      { move: "Explosion" },
    ])
    for (const record of catalog) {
      expect(record.movePoolSource).toMatch(/^user-directed pool draft v1: /)
      expect(record.movePool.length).toBeGreaterThanOrEqual(8)
      expect(record.movePool.length).toBeLessThanOrEqual(12)
      // Every entry is learnable on the roster lines; one only TM/tutor learners get has a from level.
      for (const entry of record.movePool) {
        const learning = poolLearning(entry.move, record.roster, evolution, learnsets)
        const where = `${record.id} ${entry.move}`
        expect(learning.kind, where).not.toBe("unlearnable")
        if (learning.kind !== "level-up") expect(entry.fromLevel, where).toBeDefined()
      }
    }
  })

  it("restores the draft moves that earlier forms and egg moves make legal", () => {
    const pool = (id: string) => catalog.find((record) => record.id === id)!.movePool
    const entry = (id: string, move: string) => pool(id).find((item) => item.move === move)
    // Level-up on an earlier form needs no from level: Meowth Lv 30, Misdreavus Lv 32, Shroomish Lv 40.
    expect(entry("giovanni", "Pay Day")).toEqual({ move: "Pay Day" })
    expect(entry("morty", "Pain Split")).toEqual({ move: "Pain Split" })
    expect(entry("brawly", "Spore")).toEqual({ move: "Spore" })
    // Egg moves take a from level by tier.
    expect(entry("lance", "Extreme Speed")).toEqual({ move: "Extreme Speed", fromLevel: 30 })
    expect(entry("wallace", "Mirror Coat")).toEqual({ move: "Mirror Coat", fromLevel: 30 })
    const persian = resolveTrainer(
      catalog.find((record) => record.id === "giovanni")!,
      defaults,
      80,
      evolution,
      learnsets,
    ).team.find((member) => member.species === "Persian")!
    expect(persian.moves).toContainEqual({ move: "Pay Day", source: "pool" })
  })

  it("routes Lance's gimmick moves by pool order: Hyper Beam on Dragonite, Fire Blast on Charizard", () => {
    const lance = catalog.find((record) => record.id === "lance")!
    const holder = (world: number, move: string) =>
      resolveTrainer(lance, defaults, world, evolution, learnsets).team.find((member) =>
        member.moves.some((known) => known.move === move && known.source === "pool"),
      )?.authoredSpecies
    for (const world of [80, 120, 160]) expect(holder(world, "Fire Blast")).toBe("Charizard")
    for (const world of [120, 160]) {
      const ace = resolveTrainer(lance, defaults, world, evolution, learnsets).team[0]!
      expect(ace.species).toBe("Dragonite")
      expect(ace.moves.map((move) => move.move)).toEqual(
        expect.arrayContaining(["Hyper Beam", "Extreme Speed"]),
      )
    }
  })

  it("has no roster content gaps: every catalog roster lists six Pokémon", () => {
    expect(rosterGaps(catalog, createExperiment(catalog))).toEqual([])
    expect(catalog.every((record) => record.roster.length === 6)).toBe(true)
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
    restored.trainers.fixture!.startTR = 9
    restored.archetypes.steady[1]![1] = 40
    expect(experiment.trainers.fixture!.startTR).toBe(3)
    expect(experiment.archetypes.steady[1]).toEqual([40, 25])
  })

  it("rejects version 14, which had no aloof trait", () => {
    const v14 = base()
    v14.version = 14
    delete v14.trainers.fixture.aloof
    expect(() => validateExperiment(v14, records)).toThrow(
      "Version 14 experiments have no aloof trait and the old Sleeper Champions",
    )
  })

  it("requires an aloof trait of true or false per trainer", () => {
    const missing = base()
    delete missing.trainers.fixture.aloof
    expect(() => validateExperiment(missing, records)).toThrow("missing or unknown fields")
    const wrong = base()
    wrong.trainers.fixture.aloof = "yes"
    expect(() => validateExperiment(wrong, records)).toThrow("fixture.aloof must be true or false")
    const aloof = base()
    aloof.trainers.fixture.aloof = true
    expect(validateExperiment(aloof, records).trainers.fixture!.aloof).toBe(true)
  })

  it("rejects version 13, which saved a league seed for the seeded lineup draw", () => {
    const v13 = base()
    v13.version = 13
    expect(() => validateExperiment(v13, records)).toThrow(
      "Version 13 experiments save a league seed for the retired seeded lineup draw",
    )
  })

  it("rejects version 12, which had no home regions or travel styles", () => {
    const v12 = base()
    v12.version = 12
    delete v12.trainers.fixture.homeRegion
    delete v12.trainers.fixture.travel
    expect(() => validateExperiment(v12, records)).toThrow(
      "Version 12 experiments have no home regions or travel styles",
    )
  })

  it("requires a valid home region and travel style per trainer", () => {
    const missing = base()
    delete missing.trainers.fixture.travel
    expect(() => validateExperiment(missing, records)).toThrow("missing or unknown fields")
    const region = base()
    region.trainers.fixture.homeRegion = "Sinnoh"
    expect(() => validateExperiment(region, records)).toThrow(
      "fixture.homeRegion must be one of Kanto, Johto, Hoenn",
    )
    const travel = base()
    travel.trainers.fixture.travel = "roamer"
    expect(() => validateExperiment(travel, records)).toThrow(
      "fixture.travel must be one of homebody, traveller",
    )
  })

  it("rejects version 11, which authored moves per roster slot", () => {
    const v11 = base()
    v11.version = 11
    delete v11.trainers.fixture.movePool
    for (const rosterSlot of v11.trainers.fixture.roster) rosterSlot.moves = "LEVEL_UP"
    expect(() => validateExperiment(v11, records)).toThrow(
      "Version 11 experiments author moves per roster slot and have no move pools",
    )
  })

  it("requires a move pool per trainer and checks its moves against the game data", () => {
    const missing = base()
    delete missing.trainers.fixture.movePool
    expect(() => validateExperiment(missing, records)).toThrow("missing or unknown fields")
    const pooled = base()
    pooled.trainers.fixture.movePool = [{ move: "Earthquake", fromLevel: 40 }]
    expect(validateExperiment(pooled, records, learnsets.moves).trainers.fixture!.movePool).toEqual(
      [{ move: "Earthquake", fromLevel: 40 }],
    )
    pooled.trainers.fixture.movePool = [{ move: "Earthshake" }]
    expect(() => validateExperiment(pooled, records, learnsets.moves)).toThrow(
      'fixture.movePool[1]: unknown move "Earthshake"',
    )
  })

  it("rejects version 10, which used the old archetype names", () => {
    const v10 = base()
    v10.version = 10
    expect(() => validateExperiment(v10, records)).toThrow(
      "Version 10 experiments use the old archetype names",
    )
  })

  it("rejects version 9, which had only five archetypes", () => {
    const v9 = base()
    v9.version = 9
    for (const name of ["legend", "star", "comeback", "burst"]) delete v9.archetypes[name]
    expect(() => validateExperiment(v9, records)).toThrow(
      "Version 9 experiments have only five archetypes",
    )
  })

  it("rejects version 8, which had no ace slots", () => {
    const v8 = base()
    v8.version = 8
    for (const rosterSlot of v8.trainers.fixture.roster) delete rosterSlot.isAce
    expect(() => validateExperiment(v8, records)).toThrow(
      "Version 8 experiments have no ace slots (isAce)",
    )
  })

  it("rejects version 7, which gave the Rival a fixed lead", () => {
    const v7 = base()
    v7.version = 7
    delete v7.archetypes.rival
    v7.trainers.fixture.lead = null
    expect(() => validateExperiment(v7, records)).toThrow(
      "Version 7 experiments give the Rival a fixed lead",
    )
  })

  it("rejects version 6, which gave each notable trainer one fixed TR", () => {
    const v6 = base()
    v6.version = 6
    delete v6.archetypes
    v6.trainers.fixture = { tr: 3, roster: v6.trainers.fixture.roster }
    expect(() => validateExperiment(v6, records)).toThrow(
      "Version 6 experiments give each notable trainer one fixed TR",
    )
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

  it("accepts any non-negative whole start and peak TR and rejects others", () => {
    const input = base()
    input.trainers.fixture.peakTR = 1_000_000
    expect(validateExperiment(input, records).trainers.fixture?.peakTR).toBe(1_000_000)
    for (const bad of [-1, 2.5, "7"]) {
      input.trainers.fixture.startTR = bad
      expect(() => validateExperiment(input, records)).toThrow("non-negative whole number")
    }
  })

  it.each([
    ["a peak below the start", { startTR: 10, peakTR: 9 }, "peak TR must be at least start TR"],
    ["the retired lead", { archetype: "rival", lead: 10 }, "missing or unknown fields"],
    ["an unknown archetype", { archetype: "arc" }, "archetype must be one of"],
    ["the retired fixed TR", { tr: 3 }, "missing or unknown fields"],
    [
      "a Legend whose peak differs from the start",
      { archetype: "legend", startTR: 3, peakTR: 4 },
      "a Legend's peak TR must equal start TR",
    ],
    ["a Gym Leader Legend", { archetype: "legend", startTR: 3, peakTR: 3 }, "cannot be a Legend"],
  ])("rejects %s", (_name, change, message) => {
    const input = base()
    Object.assign(input.trainers.fixture, change)
    expect(() => validateExperiment(input, records)).toThrow(message)
  })

  it("accepts a Legend outside the Gyms with peak TR equal to start TR", () => {
    const elite = [{ ...trainer("fixture", 3), role: "Elite Four" as const }]
    const input = JSON.parse(serializeExperiment(experimentWith(elite)))
    Object.assign(input.trainers.fixture, { archetype: "legend", startTR: 95, peakTR: 95 })
    expect(validateExperiment(input, elite).trainers.fixture).toMatchObject({
      archetype: "legend",
      startTR: 95,
      peakTR: 95,
    })
  })

  it("validates every archetype scaler as non-decreasing 0–100%, the step kind included", () => {
    for (const name of ARCHETYPES) {
      const input = base()
      input.archetypes[name] = [
        [0, 0],
        [40, 50],
        [80, 40],
      ]
      expect(() => validateExperiment(input, records)).toThrow(
        `${name} growth values must not decrease`,
      )
      input.archetypes[name] = [
        [0, 0],
        [40, 101],
      ]
      expect(() => validateExperiment(input, records)).toThrow(`${name} growth[1] value`)
    }
  })

  it("validates the archetype growth scalers", () => {
    const input = base()
    delete input.archetypes.veteran
    expect(() => validateExperiment(input, records)).toThrow("missing or unknown fields")
    const noRival = base()
    delete noRival.archetypes.rival
    expect(() => validateExperiment(noRival, records)).toThrow("missing or unknown fields")
    const grows = (steady: unknown) =>
      validateExperiment({ ...base(), archetypes: { ...base().archetypes, steady } }, records)
    expect(() => grows([[0, 5]])).toThrow("0% at world progress 0")
    expect(() =>
      grows([
        [0, 0],
        [40, 101],
      ]),
    ).toThrow("value")
    expect(() =>
      grows([
        [0, 0],
        [40, 50],
        [80, 40],
      ]),
    ).toThrow("must not decrease")
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

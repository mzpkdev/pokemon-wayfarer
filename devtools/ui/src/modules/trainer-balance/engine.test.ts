import { describe, expect, it } from "vitest"

import catalogData from "./catalog.json"
import {
  ARC_IDS,
  DEFAULT_ARCS,
  DEFAULT_SIZE_TABLE,
  aceAllowance,
  arcDelta,
  classifyRole,
  composeTeam,
  createExperiment,
  editionsAvailable,
  fillerJitter,
  formAt,
  gymCapSummary,
  levelBase,
  playerCap,
  playerRating,
  progressIndex,
  resolveTrainer,
  rosterGaps,
  rosterReach,
  serializeExperiment,
  sizeFor,
  validateExperiment,
  venueFeasibility,
  worldCap,
} from "./engine.js"
import type { AceForm, Experiment, RosterFiller, TrainerRecord } from "./types.js"

const form = (species: string, level: number, moves: string[] = []): AceForm => ({
  species,
  level,
  moves,
  item: null,
  ability: null,
  nature: null,
})
const filler = (
  id: string,
  baseScore: number,
  line: [string, number][],
  requiresFlag: string | null = null,
): RosterFiller => ({
  id,
  line: line.map(([species, level]) => ({ species, level })),
  baseScore,
  levelOffset: -2,
  moves: "LEVEL_UP",
  signatureMove: null,
  requiresFlag,
})
// One ace, five unlocked fillers and one flagged filler: reaches 6 without flags.
const gym: TrainerRecord = {
  id: "test-gym",
  name: "Test Gym",
  region: "Kanto",
  role: "Gym Leader",
  homeLeagues: ["Indigo"],
  source: { label: "Fixture", path: "fixture", trainerId: "TEST", note: "Test data" },
  referenceParty: [{ species: "Onix", level: 14, moves: ["Tackle"], item: null }],
  roster: {
    prototype: true,
    aces: [{ id: "steelix", line: [form("Onix", 1), form("Steelix", 35, ["Iron Tail"])] }],
    fillers: [
      filler("geodude", 60, [
        ["Geodude", 1],
        ["Graveler", 25],
        ["Golem", 40],
      ]),
      filler("kabuto", 50, [
        ["Kabuto", 1],
        ["Kabutops", 40],
      ]),
      filler("aerodactyl", 40, [["Aerodactyl", 1]]),
      filler("sandshrew", 30, [
        ["Sandshrew", 1],
        ["Sandslash", 22],
      ]),
      filler("rhyhorn", 20, [
        ["Rhyhorn", 1],
        ["Rhydon", 42],
      ]),
      filler("lapras", 90, [["Lapras", 1]], "FLAG_TICKET"),
    ],
  },
  rosterSource: "Fixture",
  bias: -2,
  allowedArcs: ["steady", "early"],
}
// Two aces and two fillers: reaches only 4 at max size.
const elite: TrainerRecord = {
  ...gym,
  id: "test-elite",
  name: "Test Elite",
  role: "Elite Four",
  homeLeagues: ["Hoenn"],
  roster: {
    prototype: true,
    aces: [
      { id: "metagross", line: [form("Metagross", 1)] },
      { id: "aggron", line: [form("Aggron", 1)] },
    ],
    fillers: [filler("skarmory", 50, [["Skarmory", 1]]), filler("claydol", 50, [["Claydol", 1]])],
  },
  bias: 0,
  allowedArcs: ["steady", "plateau"],
}
// Three aces and three fillers.
const champion: TrainerRecord = {
  ...gym,
  id: "test-champion",
  name: "Test Champion",
  role: "Champion",
  homeLeagues: ["Hoenn"],
  roster: {
    prototype: false,
    aces: [
      { id: "dragonite", line: [form("Dragonite", 1)] },
      { id: "gyarados", line: [form("Gyarados", 1)] },
      { id: "aerodactyl", line: [form("Aerodactyl", 1)] },
    ],
    fillers: [
      filler("dragonair", 30, [["Dragonair", 1]]),
      filler("kingdra", 70, [["Kingdra", 1]]),
      filler("altaria", 50, [["Altaria", 1]]),
    ],
  },
  bias: 2,
  allowedArcs: ["steady", "late"],
}
const catalog = [gym, elite, champion]
const fullCatalog = catalogData as TrainerRecord[]
const imported = (): Experiment =>
  JSON.parse(serializeExperiment(createExperiment(catalog))) as Experiment
/** A fixture experiment with no jitter, so scores equal base + modifiers. */
const fixed = (): Experiment => {
  const experiment = createExperiment(catalog)
  experiment.jitter = 0
  return experiment
}
const noFlags = { seed: 1, flags: [] as string[] }
const points = [
  ...Array.from({ length: 25 * 4 }, (_, index) => ({
    badges: index % 25,
    leagueClears: Math.floor(index / 25),
  })),
  ...[1, 2, 3].map((completedEditions) => ({ badges: 24, leagueClears: 3, completedEditions })),
]

describe("world cap", () => {
  it("retains the player badge formula and soft-cap curve", () => {
    expect([0, 4, 8, 16, 24].map((badges) => playerRating({ badges, leagueClears: 0 }))).toEqual([
      0, 16, 40, 48, 56,
    ])
    expect(playerRating({ badges: 24, leagueClears: 3 })).toBe(80)
    expect(playerRating({ badges: 999, leagueClears: 999 })).toBe(80)
  })

  it("equals the player soft cap at every canonical world point", () => {
    for (const point of points) expect(worldCap(point)).toBe(playerCap(point))
    const reference = [
      [0, 0, 15],
      [4, 0, 23],
      [8, 0, 42],
      [8, 1, 52],
      [16, 1, 62],
      [16, 2, 78],
      [24, 2, 89],
      [24, 3, 100],
    ]
    for (const [badges, leagueClears, cap] of reference)
      expect(worldCap({ badges: badges ?? 0, leagueClears: leagueClears ?? 0 })).toBe(cap)
  })

  it("rejects nonfinite world inputs instead of resolving NaN", () => {
    expect(() => playerRating({ badges: NaN, leagueClears: 0 })).toThrow(/finite/)
    expect(() => worldCap({ badges: 0, leagueClears: Infinity })).toThrow(/finite/)
    expect(() => progressIndex({ badges: 24, leagueClears: 3, completedEditions: NaN })).toThrow(
      /finite/,
    )
  })
})

describe("progress index", () => {
  it("adds 8 per completed edition, clamped at three editions", () => {
    const post = (completedEditions: number) =>
      progressIndex({ badges: 24, leagueClears: 3, completedEditions })
    expect([0, 1, 2, 3, 4, 99].map(post)).toEqual([24, 32, 40, 48, 48, 48])
    expect(post(-2)).toBe(24)
    expect(progressIndex({ badges: 13, leagueClears: 1 })).toBe(13)
  })

  it("only counts completed editions at 24 badges and 3 first clears", () => {
    expect(progressIndex({ badges: 24, leagueClears: 2, completedEditions: 2 })).toBe(24)
    expect(progressIndex({ badges: 20, leagueClears: 3, completedEditions: 1 })).toBe(20)
    expect(editionsAvailable({ badges: 24, leagueClears: 3 })).toBe(true)
    expect(editionsAvailable({ badges: 24, leagueClears: 2 })).toBe(false)
    expect(editionsAvailable({ badges: 23, leagueClears: 3 })).toBe(false)
  })

  it("keeps the world cap independent of completed editions", () => {
    for (const completedEditions of [0, 1, 3])
      expect(worldCap({ badges: 24, leagueClears: 3, completedEditions })).toBe(100)
  })
})

describe("growth arcs and standing", () => {
  it("interpolates arc tuples linearly with half-up rounding", () => {
    expect([0, 4, 8, 12, 16, 20, 24].map((b) => arcDelta(DEFAULT_ARCS.early, b))).toEqual([
      0, 2, 3, 3, 2, 2, 1,
    ])
    // late: -0.5 -> 0, -1.5 -> -1, -1 at 12 (-2 -> 0), +1.5 -> 2 at 20.
    expect([2, 6, 12, 20].map((b) => arcDelta(DEFAULT_ARCS.late, b))).toEqual([0, -1, -1, 2])
    // plateau between +1 and -1: 0.5 -> 1, -0.5 -> 0 (not -1), -0.75 -> -1.
    expect([10, 14, 15].map((b) => arcDelta(DEFAULT_ARCS.plateau, b))).toEqual([1, 0, -1])
    expect(Object.is(arcDelta(DEFAULT_ARCS.plateau, 14), -0)).toBe(false)
    expect(arcDelta(DEFAULT_ARCS.rival, -3)).toBe(0)
    expect(arcDelta(DEFAULT_ARCS.rival, 99)).toBe(3)
  })

  it("interpolates the post-game checkpoints at p = 32/40/48 and stops at 48", () => {
    const post = [24, 28, 32, 36, 40, 44, 48]
    // early 1 -> 0 -> 0 -> 0: 0.5 -> 1.
    expect(post.map((p) => arcDelta(DEFAULT_ARCS.early, p))).toEqual([1, 1, 0, 0, 0, 0, 0])
    // late 3 -> 3 -> 2 -> 2: 2.5 -> 3.
    expect(post.map((p) => arcDelta(DEFAULT_ARCS.late, p))).toEqual([3, 3, 3, 3, 2, 2, 2])
    // plateau -3 -> -3 -> -2 -> -1: -2.5 -> -2, -1.5 -> -1.
    expect(post.map((p) => arcDelta(DEFAULT_ARCS.plateau, p))).toEqual([-3, -3, -3, -2, -2, -1, -1])
    for (const arc of ARC_IDS) {
      expect(DEFAULT_ARCS[arc]).toHaveLength(7)
      expect(arcDelta(DEFAULT_ARCS[arc], 0)).toBe(0)
      expect(arcDelta(DEFAULT_ARCS[arc], 60)).toBe(arcDelta(DEFAULT_ARCS[arc], 48))
    }
  })

  it("gives every trainer an identical B=0 encounter under every arc", () => {
    const experiment = createExperiment(fullCatalog)
    for (const trainer of fullCatalog)
      for (let leagueClears = 0; leagueClears <= 3; leagueClears += 1) {
        const point = { badges: 0, leagueClears }
        const [first, ...rest] = ARC_IDS.map((arc) => {
          const resolved = resolveTrainer(trainer, experiment, point, arc)
          return [resolved.standing, resolved.strengthLevel, resolved.size, resolved.party]
        })
        for (const other of rest) expect(other).toEqual(first)
        expect(first?.[0]).toBe(trainer.bias)
      }
  })

  it("computes strength = clamp(world cap + bias + arc delta, 1, 100)", () => {
    const experiment = fixed()
    const brock = resolveTrainer(gym, experiment, { badges: 0, leagueClears: 0 })
    expect([brock.standing, brock.strengthLevel, brock.gap]).toEqual([-2, 13, -2])
    expect(brock.party.map((entry) => [entry.species, entry.level])).toEqual([
      ["Geodude", 11],
      ["Onix", 13],
    ])
    const early = resolveTrainer(gym, experiment, { badges: 8, leagueClears: 1 }, "early")
    expect([early.standing, early.worldCap, early.strengthLevel, early.gap]).toEqual([1, 52, 53, 1])

    experiment.trainers[gym.id]!.bias = 6
    // levelBase 96 + bias 6 + early 1 = 103.
    const high = resolveTrainer(gym, experiment, { badges: 24, leagueClears: 3 }, "early")
    expect([high.levelBase, high.strengthLevel, high.gap]).toEqual([96, 100, 0])
    expect(high.warnings.join(" ")).toMatch(/Strength level 103 clamped to 100/)

    experiment.arcs.late = [0, -12, -12, -12, -12, -12, -12]
    experiment.trainers[gym.id]!.bias = -6
    const low = resolveTrainer(gym, experiment, { badges: 1, leagueClears: 0 }, "late")
    expect(low.strengthLevel).toBe(9)
    expect(low.party.map((entry) => entry.level)).toEqual([7, 9])
    expect(resolveTrainer(gym, experiment, { badges: -5, leagueClears: -1 }).worldCap).toBe(15)
  })

  it("classifies roles on disjoint windows covering every standing", () => {
    const windows = { contenderMax: -2, headlinerMin: 2 }
    expect([-6, -2, -1, 0, 1, 2, 6].map((s) => classifyRole(s, windows))).toEqual([
      "contender",
      "contender",
      "elite",
      "elite",
      "elite",
      "headliner",
      "headliner",
    ])
    expect(classifyRole(0, { contenderMax: -1, headlinerMin: 1 })).toBe("elite")
    expect(classifyRole(1, { contenderMax: -1, headlinerMin: 1 })).toBe("headliner")
  })

  it("keeps Blue above the world cap wherever it is at most 96", () => {
    const experiment = createExperiment(fullCatalog)
    const blue = fullCatalog.find((trainer) => trainer.id === "blue")
    if (!blue) throw new Error("Blue is missing from the catalog")
    expect(experiment.trainers.blue?.allowedArcs).toEqual(["early", "rival"])
    for (const point of points) {
      const summary = gymCapSummary(fullCatalog, experiment, point)
      if (worldCap(point) <= 96) {
        expect(summary.blue).toMatchObject({ ahead: true, atCeiling: false })
        for (const arc of experiment.trainers.blue?.allowedArcs ?? [])
          expect(resolveTrainer(blue, experiment, point, arc).gap).toBeGreaterThan(0)
      } else expect(summary.blue?.atCeiling).toBe(true)
    }
  })

  it("checks Blue against the level base at the ceiling", () => {
    const experiment = createExperiment(fullCatalog)
    const at = (completedEditions: number) =>
      gymCapSummary(fullCatalog, experiment, { badges: 24, leagueClears: 3, completedEditions })
        .blue
    // (24, 3): base 96; early +1 +1 -> 98, rival +1 +3 -> 100. Both below the cap of 100.
    expect(at(0)).toEqual({
      gaps: [
        { arc: "early", gap: -2, baseGap: 2 },
        { arc: "rival", gap: 0, baseGap: 4 },
      ],
      ahead: true,
      atCeiling: true,
      levelBase: 96,
    })
    // Editions 2-4+: early holds at 0 from p = 32, so Blue stays one level above the base.
    for (const editions of [1, 2, 3]) {
      expect(at(editions)).toMatchObject({ ahead: true, atCeiling: true })
      expect(at(editions)?.gaps.map(({ baseGap }) => baseGap)).toEqual([1, 4])
    }
  })
})

describe("level headroom", () => {
  it("changes nothing wherever the world cap is at most 100 - headroom", () => {
    const experiment = createExperiment(fullCatalog)
    expect(experiment.headroom).toBe(4)
    for (const point of points.filter((entry) => worldCap(entry) <= 96)) {
      expect(levelBase(point, experiment.headroom)).toBe(worldCap(point))
      for (const trainer of fullCatalog) {
        const resolved = resolveTrainer(trainer, experiment, point)
        expect(resolved.levelBase).toBe(resolved.worldCap)
        expect(resolved.strengthLevel).toBe(
          Math.max(1, Math.min(100, resolved.worldCap + resolved.standing)),
        )
      }
    }
  })

  it("separates contenders, elites and headliners below level 100 at the ceiling", () => {
    const experiment = createExperiment(fullCatalog)
    for (const completedEditions of [0, 1, 2, 3]) {
      const point = { badges: 24, leagueClears: 3, completedEditions }
      expect(levelBase(point, experiment.headroom)).toBe(96)
      for (const trainer of fullCatalog)
        for (const arc of experiment.trainers[trainer.id]?.allowedArcs ?? []) {
          const resolved = resolveTrainer(trainer, experiment, point, arc)
          expect(resolved.strengthLevel).toBe(Math.min(100, 96 + resolved.standing))
          if (resolved.role === "contender") expect(resolved.strengthLevel).toBeLessThanOrEqual(94)
          if (resolved.role === "elite") expect([95, 96, 97]).toContain(resolved.strengthLevel)
          if (resolved.role === "headliner")
            expect(resolved.strengthLevel).toBeGreaterThanOrEqual(98)
        }
    }
    // Fixture Gym (bias -2, steady) at (24, 3): 96 - 2 by default, 100 - 2 without headroom.
    const fixture = createExperiment(catalog)
    const ceiling = { badges: 24, leagueClears: 3 }
    expect(resolveTrainer(gym, fixture, ceiling).strengthLevel).toBe(94)
    fixture.headroom = 0
    expect(resolveTrainer(gym, fixture, ceiling).strengthLevel).toBe(98)
  })
})

describe("team size and ace allowance", () => {
  it("sizes teams from the strength level: <20 -> 2, 20 -> 3, 30 -> 4, 45 -> 5, 60 -> 6", () => {
    const levels = [1, 19, 20, 29, 30, 44, 45, 59, 60, 100]
    expect(levels.map((level) => sizeFor(DEFAULT_SIZE_TABLE, level))).toEqual([
      2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    ])
    const custom = [
      { minLevel: 1, size: 1 },
      { minLevel: 10, size: 6 },
    ]
    expect([9, 10].map((level) => sizeFor(custom, level))).toEqual([1, 6])
  })

  it("allows at most 1 ace at 1–3 members, 2 at 4–5 and 3 at 6", () => {
    expect([1, 2, 3, 4, 5, 6].map(aceAllowance)).toEqual([1, 1, 1, 2, 2, 3])
  })

  it("gives unused ace slots to fillers: one ace at strength 65 is 1 ace + 5 fillers", () => {
    const team = composeTeam(gym.id, gym.roster, fixed(), 65, noFlags)
    expect([team.size, team.allowance, team.acesUsed]).toEqual([6, 3, 1])
    expect(team.party).toHaveLength(6)
    expect(team.party.filter((member) => member.kind === "filler")).toHaveLength(5)
    expect(rosterReach(gym.roster, [])).toBe(6)
  })

  it("fills aces first in priority order, up to the allowance", () => {
    const experiment = fixed()
    const ids = (level: number) =>
      composeTeam(champion.id, champion.roster, experiment, level, noFlags)
        .party.filter((member) => member.kind === "ace")
        .map((member) => member.id)
    // Battle order puts the top ace last.
    expect(ids(10)).toEqual(["dragonite"])
    expect(ids(30)).toEqual(["gyarados", "dragonite"])
    expect(ids(60)).toEqual(["aerodactyl", "gyarados", "dragonite"])
    const resolved = composeTeam(champion.id, champion.roster, experiment, 30, noFlags)
    expect([resolved.size, resolved.acesUsed, resolved.party.length]).toEqual([4, 2, 4])
  })

  it("orders battle as fillers by ascending score, then aces in reverse priority", () => {
    const team = composeTeam(champion.id, champion.roster, fixed(), 60, noFlags)
    expect(team.party.map((member) => member.id)).toEqual([
      "dragonair",
      "altaria",
      "kingdra",
      "aerodactyl",
      "gyarados",
      "dragonite",
    ])
    expect(team.party.map((member) => member.score ?? null)).toEqual([30, 50, 70, null, null, null])
  })

  it("breaks score ties by filler ID", () => {
    const experiment = createExperiment(catalog)
    experiment.jitter = 0
    const team = composeTeam(elite.id, elite.roster, experiment, 20, noFlags)
    // size 3: 1 ace + 2 fillers of equal score; claydol ranks before skarmory.
    const scores = team.fillerScores.map((entry) => [entry.filler.id, entry.rank])
    expect(scores).toEqual([
      ["skarmory", 2],
      ["claydol", 1],
    ])
    expect(team.party.map((member) => member.id)).toEqual(["skarmory", "claydol", "metagross"])
  })

  it("never removes a filler as size grows with fixed scores", () => {
    for (const seed of [1, 2, 99, 123456])
      for (const trainer of fullCatalog) {
        const experiment = createExperiment(fullCatalog)
        const roster = experiment.trainers[trainer.id]!.roster
        let previous = new Set<string>()
        for (let level = 1; level <= 100; level += 1) {
          const team = composeTeam(trainer.id, roster, experiment, level, { seed, flags: [] })
          const fillers = new Set(
            team.party.filter((member) => member.kind === "filler").map((member) => member.id),
          )
          for (const id of previous) expect(fillers.has(id)).toBe(true)
          expect(team.party.length).toBeGreaterThanOrEqual(previous.size)
          previous = fillers
        }
      }
  })

  it("lets an active modifier displace a filler", () => {
    const experiment = fixed()
    experiment.modifiers = [{ flag: "FLAG_TOLD", trainer: gym.id, filler: "rhyhorn", delta: 50 }]
    const fillers = (flags: string[]) =>
      composeTeam(gym.id, experiment.trainers[gym.id]!.roster, experiment, 30, { seed: 1, flags })
        .party.filter((member) => member.kind === "filler")
        .map((member) => member.id)
    // size 4 = 1 ace + 3 fillers.
    expect(fillers([])).toEqual(["aerodactyl", "kabuto", "geodude"])
    expect(fillers(["FLAG_TOLD"])).toEqual(["kabuto", "geodude", "rhyhorn"])
    // Negative deltas push a filler out too.
    experiment.modifiers = [{ flag: "FLAG_UPSET", trainer: gym.id, filler: "geodude", delta: -50 }]
    expect(fillers(["FLAG_UPSET"])).toEqual(["sandshrew", "aerodactyl", "kabuto"])
  })

  it("keeps a requiresFlag filler out until its flag is set", () => {
    const experiment = fixed()
    const team = (flags: string[]) =>
      composeTeam(gym.id, experiment.trainers[gym.id]!.roster, experiment, 20, { seed: 1, flags })
    const locked = team([]).fillerScores.find((entry) => entry.filler.id === "lapras")
    expect(locked).toMatchObject({ eligible: false, rank: null, inTeam: false })
    expect(team([]).party.map((member) => member.id)).toEqual(["kabuto", "geodude", "steelix"])
    expect(team(["FLAG_TICKET"]).party.map((member) => member.id)).toEqual([
      "geodude",
      "lapras",
      "steelix",
    ])
    const small = {
      ...elite.roster,
      fillers: [filler("lapras", 90, [["Lapras", 1]], "FLAG_TICKET")],
    }
    expect(rosterReach(small, [])).toBe(2)
    expect(rosterReach(small, ["FLAG_TICKET"])).toBe(3)
  })

  it("evolves members along their authored line at their own level", () => {
    const experiment = fixed()
    const species = (level: number) =>
      Object.fromEntries(
        composeTeam(gym.id, gym.roster, experiment, level, noFlags).party.map((member) => [
          member.id,
          `${member.species} ${member.level}`,
        ]),
      )
    expect(species(26)).toEqual({ geodude: "Geodude 24", kabuto: "Kabuto 24", steelix: "Onix 26" })
    expect(species(27)).toMatchObject({ geodude: "Graveler 25" })
    expect(species(34)).toMatchObject({ steelix: "Onix 34" })
    expect(species(35)).toMatchObject({ steelix: "Steelix 35" })
    expect(species(42)).toMatchObject({ geodude: "Golem 40", kabuto: "Kabutops 40" })
    expect(formAt(gym.roster.fillers[0]!.line, 24).species).toBe("Geodude")
    expect(formAt(gym.roster.fillers[0]!.line, 100).species).toBe("Golem")
  })

  it("jitters deterministically per seed within 0..JITTER", () => {
    for (const seed of [0, 1, 42, 4294967295]) {
      const value = fillerJitter(seed, "brock", "onix", 30)
      expect(value).toBe(fillerJitter(seed, "brock", "onix", 30))
      expect(value).toBeGreaterThanOrEqual(0)
      expect(value).toBeLessThanOrEqual(30)
    }
    expect(fillerJitter(7, "brock", "onix", 0)).toBe(0)
    const experiment = createExperiment(fullCatalog)
    const teams = (seed: number) =>
      fullCatalog.map((trainer) =>
        resolveTrainer(trainer, experiment, { badges: 12, leagueClears: 1 }, undefined, {
          seed,
          flags: [],
        }).party.map((member) => member.id),
      )
    expect(teams(1)).toEqual(teams(1))
    expect(teams(2)).not.toEqual(teams(1))
    const jitters = new Set(
      Array.from({ length: 200 }, (_, seed) => fillerJitter(seed, "brock", "onix", 30)),
    )
    expect(jitters.size).toBeGreaterThan(20)
  })

  it("reports a short roster and a strength drop as warnings", () => {
    const experiment = createExperiment(catalog)
    const short = resolveTrainer(elite, experiment, { badges: 24, leagueClears: 3 })
    expect([short.size, short.party.length]).toEqual([6, 4])
    expect(short.warnings.join(" ")).toMatch(/fills 4 of 6 slots/)
    experiment.arcs.plateau = [0, 12, -12, -12, -12, -12, -12]
    const dropped = resolveTrainer(elite, experiment, { badges: 9, leagueClears: 0 }, "plateau")
    expect(dropped.warnings.join(" ")).toMatch(/strength level dropped/)
  })

  it("never shrinks a team in edition 1 with default arcs", () => {
    const experiment = createExperiment(fullCatalog)
    for (const trainer of fullCatalog)
      for (const arc of experiment.trainers[trainer.id]?.allowedArcs ?? [])
        for (let leagueClears = 0; leagueClears <= 3; leagueClears += 1) {
          let previous = resolveTrainer(trainer, experiment, { badges: 0, leagueClears }, arc)
          for (let badges = 1; badges <= 24; badges += 1) {
            const current = resolveTrainer(trainer, experiment, { badges, leagueClears }, arc)
            expect(current.size).toBeGreaterThanOrEqual(previous.size)
            expect(current.party.length).toBeGreaterThanOrEqual(previous.party.length)
            expect(
              current.party.every(
                (entry) => Number.isInteger(entry.level) && entry.level >= 1 && entry.level <= 100,
              ),
            ).toBe(true)
            previous = current
          }
        }
  })

  it("moves post-game levels only with the arc once the level base is flat", () => {
    const experiment = createExperiment(fullCatalog)
    const drops = new Set<string>()
    for (const trainer of fullCatalog)
      for (const arc of experiment.trainers[trainer.id]?.allowedArcs ?? []) {
        let previous = resolveTrainer(trainer, experiment, { badges: 24, leagueClears: 3 }, arc)
        for (const completedEditions of [1, 2, 3]) {
          const point = { badges: 24, leagueClears: 3, completedEditions }
          const current = resolveTrainer(trainer, experiment, point, arc)
          expect(current.party).toHaveLength(previous.party.length)
          const levelDelta = current.strengthLevel - previous.strengthLevel
          expect(levelDelta).toBe(
            Math.min(100, 96 + current.standing) - Math.min(100, 96 + previous.standing),
          )
          if (levelDelta < 0) {
            drops.add(arc)
            expect(current.warnings.join(" ")).toMatch(/dropped since the previous progress step/)
          }
          previous = current
        }
      }
    // Declining post-game tuples (early 1 -> 0, late 3 -> 2) lower levels once the
    // base stops rising; plateau recovers and steady/rival hold.
    expect(drops).toEqual(new Set(["early", "late"]))
  })
})

describe("league feasibility", () => {
  it("counts guaranteed, possible, and current candidates per venue and role", () => {
    const experiment = createExperiment(catalog)
    // B=8: gym -2 steady/early -> -2/+1; elite 0 steady/plateau -> 0/+1;
    // champion +2 steady/late -> +2/0.
    const [indigo, hoenn, masters] = venueFeasibility(catalog, experiment, 8)
    const counts = (venue: typeof indigo) =>
      venue?.roles.map((r) => [r.role, r.guaranteed, r.possible, r.current, r.fallbackRisk])
    expect(indigo?.candidates).toBe(1)
    expect(counts(indigo)).toEqual([
      ["contender", 0, 1, 1, true],
      ["elite", 0, 1, 0, true],
      ["headliner", 0, 0, 0, true],
    ])
    // Every roster is a candidate; the elite's short roster is reported as a gap.
    expect(hoenn?.candidates).toBe(2)
    expect(counts(hoenn)).toEqual([
      ["contender", 0, 0, 0, true],
      ["elite", 1, 2, 1, true],
      ["headliner", 0, 1, 1, true],
    ])
    expect(hoenn?.gaps).toEqual([{ id: elite.id, name: "Test Elite", reach: 4 }])
    expect(indigo?.gaps).toEqual([])
    // Sevii Masters is open: every candidate, regardless of home leagues.
    expect(masters?.pool).toBe("open")
    expect(masters?.candidates).toBe(3)
  })

  it("reports rosters that cannot reach six under the current flags", () => {
    const experiment = createExperiment(catalog)
    experiment.trainers[gym.id]!.roster.fillers = experiment.trainers[
      gym.id
    ]!.roster.fillers.filter((f) => f.id !== "rhyhorn")
    expect(rosterGaps(catalog, experiment).map((gap) => [gap.id, gap.reach])).toEqual([
      [gym.id, 5],
      [elite.id, 4],
    ])
    expect(rosterGaps(catalog, experiment, ["FLAG_TICKET"]).map((gap) => gap.id)).toEqual([
      elite.id,
    ])
  })

  it("reads standing at the progress index, including post-game editions", () => {
    const experiment = createExperiment(catalog)
    // p = 40: gym -2 steady/early -> -2/-2; elite 0 steady/plateau -> 0/-2; champion +2 steady/late -> +2/+4.
    const [, , masters] = venueFeasibility(catalog, experiment, 40)
    expect(masters?.roles.map((r) => [r.role, r.guaranteed, r.possible])).toEqual([
      ["contender", 1, 2],
      ["elite", 0, 1],
      ["headliner", 1, 1],
    ])
  })

  it("summarises Gym leader strength-minus-cap and the Blue check", () => {
    const experiment = createExperiment(fullCatalog)
    const summary = gymCapSummary(fullCatalog, experiment, { badges: 0, leagueClears: 0 })
    expect(summary.count).toBe(23)
    expect(summary.min).toBe(-2)
    expect(summary.max).toBe(-1)
    expect(summary.blue?.ahead).toBe(true)
    const ceiling = gymCapSummary(fullCatalog, experiment, { badges: 24, leagueClears: 3 })
    expect(ceiling.blue).toMatchObject({ ahead: true, atCeiling: true })
    // Strength - cap still compares with the player cap: base 96 puts leaders at -6 / -5.
    expect([ceiling.min, ceiling.max]).toEqual([-6, -5])
  })
})

describe("catalog defaults", () => {
  it("gives one Gym Leader per region a -1 bias and a headliner path", () => {
    const experiment = createExperiment(fullCatalog)
    const leaders = fullCatalog.filter((trainer) => trainer.role === "Gym Leader")
    const strong = leaders.filter((trainer) => trainer.bias === -1)
    expect(strong.map((trainer) => [trainer.id, trainer.allowedArcs])).toEqual([
      ["sabrina", ["steady", "early"]],
      ["giovanni", ["steady", "late"]],
      ["morty", ["steady", "late"]],
      ["clair", ["steady", "early", "rival"]],
      ["norman", ["steady", "late"]],
      ["winona", ["steady", "early"]],
      ["juan", ["steady", "plateau"]],
    ])
    expect(leaders.filter((trainer) => trainer.bias !== -1).every((t) => t.bias === -2)).toBe(true)
    const progress = Array.from({ length: 49 }, (_, p) => p)
    const headliners = leaders.filter((trainer) =>
      (experiment.trainers[trainer.id]?.allowedArcs ?? []).some((arc) =>
        progress.some(
          (p) =>
            classifyRole(
              trainer.bias + arcDelta(experiment.arcs[arc], p),
              experiment.roleWindows,
            ) === "headliner",
        ),
      ),
    )
    expect(headliners.map((trainer) => trainer.id)).toEqual([
      "sabrina",
      "giovanni",
      "morty",
      "clair",
      "norman",
      "winona",
    ])
    expect(new Set(headliners.map((trainer) => trainer.region))).toEqual(
      new Set(["Kanto", "Johto", "Hoenn"]),
    )
  })
})

describe("catalog rosters", () => {
  it("ships a valid prototype roster of 1–3 aces for every trainer", () => {
    const experiment = createExperiment(fullCatalog)
    for (const trainer of fullCatalog) {
      expect(trainer.roster.prototype).toBe(true)
      expect(trainer.roster.aces.length).toBeGreaterThanOrEqual(1)
      expect(trainer.roster.aces.length).toBeLessThanOrEqual(3)
      expect(experiment.trainers[trainer.id]?.roster).toEqual(trainer.roster)
    }
  })

  it("lists the prototype rosters that cannot reach six as content gaps", () => {
    expect(rosterGaps(fullCatalog, createExperiment(fullCatalog)).map((gap) => gap.id)).toEqual([
      "lorelei",
      "bruno",
      "agatha",
      "koga",
      "lance",
      "will",
      "karen",
      "sidney",
      "phoebe",
      "glacia",
      "drake",
    ])
  })
})

describe("experiment import", () => {
  it("round-trips deterministic JSON and returns an independent validated value", () => {
    const experiment = createExperiment(catalog)
    experiment.modifiers.push({ flag: "FLAG_TOLD", trainer: gym.id, filler: "kabuto", delta: -5 })
    const json = serializeExperiment(experiment)
    const restored = validateExperiment(JSON.parse(json), catalog)
    expect(serializeExperiment(restored)).toBe(json)
    restored.arcs.early[1] = 9
    restored.trainers[gym.id]!.roster.fillers[0]!.baseScore = 1
    expect(experiment.arcs.early[1]).toBe(3)
    expect(experiment.trainers[gym.id]!.roster.fillers[0]!.baseScore).toBe(60)
  })

  it("rejects version 1 experiments with a clear message", () => {
    expect(() =>
      validateExperiment({ version: 1, levelAnchors: [], trainers: {} }, catalog),
    ).toThrow(/Version 1 experiments use the retired trainer-rating model/)
  })

  it("rejects version 2 experiments without migrating them", () => {
    const value = imported() as unknown as Record<string, unknown>
    value.version = 2
    delete value.headroom
    expect(() => validateExperiment(value, catalog)).toThrow(
      /Version 2 experiments predate post-game arcs/,
    )
  })

  it("rejects version 3 experiments with team stages without migrating them", () => {
    const value = imported() as unknown as Record<string, unknown>
    value.version = 3
    expect(() => validateExperiment(value, catalog)).toThrow(
      /Version 3 experiments use badge-keyed team stages/,
    )
  })

  it("stores allowed arcs in canonical order", () => {
    const value = imported()
    value.trainers[gym.id]!.allowedArcs = ["early", "steady"]
    expect(validateExperiment(value, catalog).trainers[gym.id]?.allowedArcs).toEqual([
      "steady",
      "early",
    ])
  })

  const roster = (value: Experiment) => value.trainers[gym.id]!.roster
  it.each([
    ["version", (value: Experiment) => void ((value as { version: number }).version = 5)],
    [
      "missing headroom",
      (value: Experiment) => void delete (value as Partial<Experiment>).headroom,
    ],
    ["headroom bounds", (value: Experiment) => void (value.headroom = 21)],
    ["four-point arc", (value: Experiment) => void (value.arcs.late as number[]).splice(4)],
    ["unknown IDs", (value: Experiment) => void (value.trainers.unknown = value.trainers[gym.id]!)],
    ["missing IDs", (value: Experiment) => void delete value.trainers[gym.id]],
    ["nonzero B=0 arc", (value: Experiment) => void (value.arcs.early[0] = 1)],
    ["arc bounds", (value: Experiment) => void (value.arcs.late[3] = 13)],
    ["empty elite window", (value: Experiment) => void (value.roleWindows.headlinerMin = -1)],
    ["bias bounds", (value: Experiment) => void (value.trainers[gym.id]!.bias = 7)],
    ["no allowed arcs", (value: Experiment) => void (value.trainers[gym.id]!.allowedArcs = [])],
    ["disallowed arc", (value: Experiment) => void (value.trainers[gym.id]!.arc = "rival")],
    [
      "leftover stages",
      (value: Experiment) =>
        void ((value.trainers[gym.id] as unknown as Record<string, unknown>).stages = []),
    ],
    ["no aces", (value: Experiment) => void (roster(value).aces = [])],
    [
      "four aces",
      (value: Experiment) =>
        void (roster(value).aces = ["a", "b", "c", "d"].map((id) => ({
          id,
          line: [form("Onix", 1)],
        }))),
    ],
    [
      "duplicate IDs",
      (value: Experiment) => void (roster(value).fillers[1]!.id = roster(value).fillers[0]!.id),
    ],
    [
      "ace and filler share an ID",
      (value: Experiment) => void (roster(value).fillers[0]!.id = "steelix"),
    ],
    ["bad ID", (value: Experiment) => void (roster(value).fillers[0]!.id = "Geo Dude")],
    ["offset below -6", (value: Experiment) => void (roster(value).fillers[0]!.levelOffset = -7)],
    ["positive offset", (value: Experiment) => void (roster(value).fillers[0]!.levelOffset = 1)],
    [
      "base score above 100",
      (value: Experiment) => void (roster(value).fillers[0]!.baseScore = 101),
    ],
    ["negative base score", (value: Experiment) => void (roster(value).fillers[0]!.baseScore = -1)],
    [
      "line not starting at 1",
      (value: Experiment) => void (roster(value).fillers[0]!.line[0]!.level = 5),
    ],
    [
      "decreasing evolve level",
      (value: Experiment) => void (roster(value).fillers[0]!.line[2]!.level = 20),
    ],
    [
      "repeated species",
      (value: Experiment) => void (roster(value).aces[0]!.line[1]!.species = "Onix"),
    ],
    [
      "five ace moves",
      (value: Experiment) =>
        void (roster(value).aces[0]!.line[1]!.moves = ["A", "B", "C", "D", "E"]),
    ],
    [
      "moves policy",
      (value: Experiment) =>
        void ((roster(value).fillers[0] as unknown as Record<string, unknown>).moves = "AUTHORED"),
    ],
    ["empty flag", (value: Experiment) => void (roster(value).fillers[0]!.requiresFlag = " ")],
    ["decreasing size table", (value: Experiment) => void (value.sizeTable[4]!.size = 4)],
    ["size table start", (value: Experiment) => void (value.sizeTable[0]!.minLevel = 2)],
    [
      "repeated size level",
      (value: Experiment) => void (value.sizeTable[2]!.minLevel = value.sizeTable[1]!.minLevel),
    ],
    ["size above 6", (value: Experiment) => void (value.sizeTable[4]!.size = 7)],
    ["jitter bounds", (value: Experiment) => void (value.jitter = 101)],
    [
      "modifier on an unknown filler",
      (value: Experiment) =>
        void value.modifiers.push({ flag: "F", trainer: gym.id, filler: "mew", delta: 10 }),
    ],
    [
      "modifier on an ace",
      (value: Experiment) =>
        void value.modifiers.push({ flag: "F", trainer: gym.id, filler: "steelix", delta: 10 }),
    ],
    [
      "modifier delta bounds",
      (value: Experiment) =>
        void value.modifiers.push({ flag: "F", trainer: gym.id, filler: "kabuto", delta: 101 }),
    ],
  ])("rejects %s", (_name, mutate) => {
    const value = imported()
    mutate(value)
    expect(() => validateExperiment(value, catalog)).toThrow(/Invalid experiment:/)
  })
})

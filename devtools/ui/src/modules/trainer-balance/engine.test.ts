import { describe, expect, it } from "vitest"

import catalogData from "./catalog.json"
import {
  ARC_IDS,
  DEFAULT_ARCS,
  arcDelta,
  classifyRole,
  createExperiment,
  editionsAvailable,
  gymCapSummary,
  levelBase,
  playerCap,
  playerRating,
  progressIndex,
  resolveTrainer,
  serializeExperiment,
  validateExperiment,
  venueFeasibility,
  worldCap,
} from "./engine.js"
import type { Experiment, TrainerRecord } from "./types.js"

const member = (species: string, level: number) => ({
  species,
  level,
  moves: ["Tackle"],
  item: null,
})
const gym: TrainerRecord = {
  id: "test-gym",
  name: "Test Gym",
  region: "Kanto",
  role: "Gym Leader",
  gymEligible: true,
  homeLeagues: ["Indigo"],
  source: { label: "Fixture", path: "fixture", trainerId: "TEST", note: "Test data" },
  referenceParty: [member("Geodude", 12), member("Onix", 14)],
  competitiveParty: [
    member("Golem", 50),
    member("Onix", 52),
    member("Kabutops", 54),
    member("Aerodactyl", 56),
    member("Rhydon", 58),
    member("Tyranitar", 60),
  ],
  competitiveSource: "Fixture",
  earlyParty: ["Geodude", "Onix"],
  bias: -2,
  allowedArcs: ["steady", "early"],
}
const elite: TrainerRecord = {
  ...gym,
  id: "test-elite",
  name: "Test Elite",
  role: "Elite Four",
  gymEligible: false,
  homeLeagues: ["Hoenn"],
  referenceParty: gym.competitiveParty.slice(0, 5),
  competitiveParty: gym.competitiveParty.slice(0, 5),
  earlyParty: [],
  bias: 0,
  allowedArcs: ["steady", "plateau"],
}
const champion: TrainerRecord = {
  ...gym,
  id: "test-champion",
  name: "Test Champion",
  role: "Champion",
  gymEligible: false,
  homeLeagues: ["Hoenn"],
  referenceParty: gym.competitiveParty,
  earlyParty: [],
  bias: 2,
  allowedArcs: ["steady", "late"],
}
const catalog = [gym, elite, champion]
const fullCatalog = catalogData as TrainerRecord[]
const imported = (): Experiment =>
  JSON.parse(serializeExperiment(createExperiment(catalog))) as Experiment
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
          return [resolved.standing, resolved.aceLevel, resolved.stage, resolved.party]
        })
        for (const other of rest) expect(other).toEqual(first)
        expect(first?.[0]).toBe(trainer.bias)
      }
  })

  it("computes ace = clamp(world cap + bias + arc delta, 1, 100)", () => {
    const experiment = createExperiment(catalog)
    const brock = resolveTrainer(gym, experiment, { badges: 0, leagueClears: 0 })
    expect([brock.standing, brock.aceLevel, brock.gap]).toEqual([-2, 13, -2])
    expect(brock.party.map((entry) => entry.level)).toEqual([11, 13])
    const early = resolveTrainer(gym, experiment, { badges: 8, leagueClears: 1 }, "early")
    expect([early.standing, early.worldCap, early.aceLevel, early.gap]).toEqual([1, 52, 53, 1])

    experiment.trainers[gym.id]!.bias = 6
    // levelBase 96 + bias 6 + early 1 = 103.
    const high = resolveTrainer(gym, experiment, { badges: 24, leagueClears: 3 }, "early")
    expect([high.levelBase, high.aceLevel, high.gap]).toEqual([96, 100, 0])
    expect(high.warnings.join(" ")).toMatch(/Ace level 103 clamped to 100/)

    experiment.arcs.late = [0, -12, -12, -12, -12, -12, -12]
    experiment.trainers[gym.id]!.bias = -6
    experiment.trainers[gym.id]!.stages[0]!.party[0]!.levelOffset = -30
    const low = resolveTrainer(gym, experiment, { badges: 1, leagueClears: 0 }, "late")
    expect(low.aceLevel).toBe(9)
    expect(low.party[0]?.level).toBe(1)
    expect(low.warnings.join(" ")).toMatch(/clipped/)
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
        expect(resolved.aceLevel).toBe(
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
          expect(resolved.aceLevel).toBe(Math.min(100, 96 + resolved.standing))
          if (resolved.role === "contender") expect(resolved.aceLevel).toBeLessThanOrEqual(94)
          if (resolved.role === "elite") expect([95, 96, 97]).toContain(resolved.aceLevel)
          if (resolved.role === "headliner") expect(resolved.aceLevel).toBeGreaterThanOrEqual(98)
        }
    }
    // Fixture Gym (bias -2, steady) at (24, 3): 96 - 2 by default, 100 - 2 without headroom.
    const fixture = createExperiment(catalog)
    const ceiling = { badges: 24, leagueClears: 3 }
    expect(resolveTrainer(gym, fixture, ceiling).aceLevel).toBe(94)
    fixture.headroom = 0
    expect(resolveTrainer(gym, fixture, ceiling).aceLevel).toBe(98)
  })
})

describe("team stages", () => {
  it("grows Gym parties 2/3/4/5/6 at 0/3/6/10/16 badges", () => {
    const stages = createExperiment(catalog).trainers[gym.id]?.stages ?? []
    expect(stages.map((stage) => [stage.minBadges, stage.party.length])).toEqual([
      [0, 2],
      [3, 3],
      [6, 4],
      [10, 5],
      [16, 6],
    ])
    expect(stages[0]?.party.map((entry) => entry.species)).toEqual(["Geodude", "Onix"])
    for (const trainer of fullCatalog.filter((entry) => entry.role === "Gym Leader")) {
      const sizes = createExperiment(fullCatalog).trainers[trainer.id]?.stages.map(
        (stage) => stage.party.length,
      )
      expect(sizes?.slice(0, 4)).toEqual([2, 3, 4, 5])
    }
  })

  it("never drops party size, or the weakest level below the ceiling, in edition 1", () => {
    const experiment = createExperiment(fullCatalog)
    expect(fullCatalog).toHaveLength(37)
    for (const trainer of fullCatalog)
      for (const arc of experiment.trainers[trainer.id]?.allowedArcs ?? [])
        for (let leagueClears = 0; leagueClears <= 3; leagueClears += 1) {
          let previous = resolveTrainer(trainer, experiment, { badges: 0, leagueClears }, arc)
          for (let badges = 1; badges <= 24; badges += 1) {
            const current = resolveTrainer(trainer, experiment, { badges, leagueClears }, arc)
            expect(current.party.length).toBeGreaterThanOrEqual(previous.party.length)
            if (previous.levelBase === 96 && current.levelBase === 96) {
              // At the ceiling (C = 3, B >= 21) the base is flat; only the arc moves levels.
              expect(current.aceLevel - previous.aceLevel).toBe(
                Math.min(100, 96 + current.standing) - Math.min(100, 96 + previous.standing),
              )
            } else {
              expect(Math.min(...current.party.map((entry) => entry.level))).toBeGreaterThanOrEqual(
                Math.min(...previous.party.map((entry) => entry.level)),
              )
              expect(current.warnings.join(" ")).not.toMatch(/dropped/)
            }
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
          const levelDelta = current.aceLevel - previous.aceLevel
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

  it("selects each stage on its exact badge threshold without altering source parties", () => {
    const before = JSON.stringify(catalog)
    const experiment = createExperiment(catalog)
    for (const stage of experiment.trainers[gym.id]?.stages ?? [])
      expect(
        resolveTrainer(gym, experiment, { badges: stage.minBadges, leagueClears: 0 }).stage,
      ).toBe(stage.label)
    expect(JSON.stringify(catalog)).toBe(before)
    expect(experiment.trainers[elite.id]?.stages).toHaveLength(1)
  })

  it("reports a weakest-member drop caused by a custom arc", () => {
    const experiment = createExperiment(catalog)
    experiment.arcs.plateau = [0, 12, -12, -12, -12, -12, -12]
    const resolved = resolveTrainer(elite, experiment, { badges: 9, leagueClears: 0 }, "plateau")
    expect(resolved.warnings.join(" ")).toMatch(/dropped/)
  })
})

describe("league feasibility", () => {
  it("counts guaranteed, possible, and current candidates per venue and role", () => {
    const experiment = createExperiment(catalog)
    // B=8: gym -2 steady/early -> -2/+1; elite 0 steady/plateau -> 0/+1 (five members);
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
    expect(hoenn?.excluded).toEqual(["Test Elite"])
    expect(counts(hoenn)).toEqual([
      ["contender", 0, 0, 0, true],
      ["elite", 0, 1, 0, true],
      ["headliner", 0, 1, 1, true],
    ])
    // Sevii Masters is open: every six-member candidate, regardless of home leagues.
    expect(masters?.pool).toBe("open")
    expect(masters?.candidates).toBe(2)
    const withIncomplete = venueFeasibility(catalog, experiment, 8, { includeIncomplete: true })
    expect(withIncomplete[2]?.candidates).toBe(3)
    expect(withIncomplete[2]?.roles.find((r) => r.role === "elite")?.guaranteed).toBe(1)
  })

  it("does not flag a role once enough candidates are guaranteed", () => {
    const experiment = createExperiment(catalog)
    const [, , masters] = venueFeasibility(catalog, experiment, 0, { includeIncomplete: true })
    expect(masters?.roles.map((r) => [r.role, r.guaranteed, r.fallbackRisk])).toEqual([
      ["contender", 1, true],
      ["elite", 1, true],
      ["headliner", 1, false],
    ])
  })

  it("reads standing at the progress index, including post-game editions", () => {
    const experiment = createExperiment(catalog)
    // p = 40: gym -2 steady/early -> -2/-3; elite 0 steady/plateau -> 0/-2; champion +2 steady/late -> +2/+4.
    const [, , masters] = venueFeasibility(catalog, experiment, 40, { includeIncomplete: true })
    expect(masters?.roles.map((r) => [r.role, r.guaranteed, r.possible])).toEqual([
      ["contender", 1, 2],
      ["elite", 0, 1],
      ["headliner", 1, 1],
    ])
  })

  it("summarises Gym leader ace-minus-cap and the Blue check", () => {
    const experiment = createExperiment(fullCatalog)
    const summary = gymCapSummary(fullCatalog, experiment, { badges: 0, leagueClears: 0 })
    expect(summary.count).toBe(23)
    expect(summary.min).toBe(-2)
    expect(summary.max).toBe(-1)
    expect(summary.blue?.ahead).toBe(true)
    const ceiling = gymCapSummary(fullCatalog, experiment, { badges: 24, leagueClears: 3 })
    expect(ceiling.blue).toMatchObject({ ahead: true, atCeiling: true })
    // Ace - cap still compares with the player cap: base 96 puts leaders at -6 / -5.
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

describe("experiment import", () => {
  it("round-trips deterministic JSON and returns an independent validated value", () => {
    const experiment = createExperiment(catalog)
    const json = serializeExperiment(experiment)
    const restored = validateExperiment(JSON.parse(json), catalog)
    expect(serializeExperiment(restored)).toBe(json)
    expect(serializeExperiment(createExperiment(catalog))).toBe(json)
    restored.arcs.early[1] = 9
    expect(experiment.arcs.early[1]).toBe(3)
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

  it("stores allowed arcs in canonical order", () => {
    const value = imported()
    value.trainers[gym.id]!.allowedArcs = ["early", "steady"]
    expect(validateExperiment(value, catalog).trainers[gym.id]?.allowedArcs).toEqual([
      "steady",
      "early",
    ])
  })

  it.each([
    ["version", (value: Experiment) => void ((value as { version: number }).version = 4)],
    [
      "missing headroom",
      (value: Experiment) => void delete (value as Partial<Experiment>).headroom,
    ],
    ["headroom bounds", (value: Experiment) => void (value.headroom = 21)],
    ["negative headroom", (value: Experiment) => void (value.headroom = -1)],
    ["four-point arc", (value: Experiment) => void (value.arcs.late as number[]).splice(4)],
    ["unknown IDs", (value: Experiment) => void (value.trainers.unknown = value.trainers[gym.id]!)],
    ["missing IDs", (value: Experiment) => void delete value.trainers[gym.id]],
    ["nonzero B=0 arc", (value: Experiment) => void (value.arcs.early[0] = 1)],
    ["arc bounds", (value: Experiment) => void (value.arcs.late[3] = 13)],
    ["fractional arc", (value: Experiment) => void (value.arcs.late[2] = 0.5)],
    [
      "missing arc",
      (value: Experiment) => void delete (value.arcs as Partial<Experiment["arcs"]>).rival,
    ],
    ["short arc", (value: Experiment) => void (value.arcs.rival as number[]).pop()],
    ["empty elite window", (value: Experiment) => void (value.roleWindows.headlinerMin = -1)],
    ["bias bounds", (value: Experiment) => void (value.trainers[gym.id]!.bias = 7)],
    ["no allowed arcs", (value: Experiment) => void (value.trainers[gym.id]!.allowedArcs = [])],
    [
      "unknown arc",
      (value: Experiment) =>
        void ((value.trainers[gym.id]!.allowedArcs as string[]) = ["steady", "sideways"]),
    ],
    [
      "repeated arc",
      (value: Experiment) => void (value.trainers[gym.id]!.allowedArcs = ["steady", "steady"]),
    ],
    ["disallowed arc", (value: Experiment) => void (value.trainers[gym.id]!.arc = "rival")],
    [
      "missing initial stage",
      (value: Experiment) => void (value.trainers[gym.id]!.stages[0]!.minBadges = 1),
    ],
    [
      "duplicate thresholds",
      (value: Experiment) => void (value.trainers[gym.id]!.stages[1]!.minBadges = 0),
    ],
    [
      "unreachable threshold",
      (value: Experiment) => void (value.trainers[gym.id]!.stages[4]!.minBadges = 25),
    ],
    ["empty label", (value: Experiment) => void (value.trainers[gym.id]!.stages[0]!.label = " ")],
    ["empty party", (value: Experiment) => void (value.trainers[gym.id]!.stages[0]!.party = [])],
    [
      "decreasing party",
      (value: Experiment) =>
        void (value.trainers[gym.id]!.stages[1]!.party = [{ species: "Onix", levelOffset: 0 }]),
    ],
    [
      "empty species",
      (value: Experiment) => void (value.trainers[gym.id]!.stages[0]!.party[0]!.species = ""),
    ],
    [
      "no ace",
      (value: Experiment) =>
        value.trainers[gym.id]!.stages[0]!.party.forEach((entry) => {
          entry.levelOffset = -1
        }),
    ],
    [
      "offset bounds",
      (value: Experiment) => void (value.trainers[gym.id]!.stages[0]!.party[0]!.levelOffset = -31),
    ],
  ])("rejects %s", (_name, mutate) => {
    const value = imported()
    mutate(value)
    expect(() => validateExperiment(value, catalog)).toThrow(/Invalid experiment:/)
  })
})

import { describe, expect, it } from "vitest"

import catalogData from "./catalog.json"
import {
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
  WORLD_PROGRESS_CHECKPOINTS,
  badgeMatch,
  badgeMatchText,
  badgeTR,
  battleOrderOf,
  buildTeam,
  createExperiment,
  evolutionIndex,
  evolutionStatus,
  growthTR,
  gymLadder,
  isGymLeader,
  leagueLineup,
  levelCap,
  milestoneEnd,
  milestoneText,
  milestones,
  playerRating,
  resolveTrainer,
  rosterGaps,
  scale,
  serializeExperiment,
  stageAt,
  teamLevelFor,
  teamSizeFor,
  trainerRating,
  validateExperiment,
  validateRoster,
  worldLevels,
  worldProgress,
} from "./engine.js"
import type {
  Archetype,
  Catalog,
  Experiment,
  RosterSlot,
  TrainerRecord,
  WorldPoint,
} from "./types.js"

const data = catalogData as Catalog
const catalog = data.trainers
const evolution = evolutionIndex(data.evolution)
const slot = (species: string, levelOffset = -2, isAce = levelOffset === 0): RosterSlot => ({
  species,
  levelOffset,
  isAce,
  moves: "LEVEL_UP",
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
  growth: { archetype?: Archetype; peakTR?: number } = {},
): TrainerRecord => ({
  id,
  name: id,
  region: "Kanto",
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
})
const experimentWith = (records: TrainerRecord[]): Experiment => createExperiment(records)
const defaults = createExperiment(catalog)
const at = (world: number) =>
  catalog.map((record) => resolveTrainer(record, defaults, world, evolution))
const lineupAt = (world: number) =>
  leagueLineup(catalog, defaults, world, evolution).map((row) => [
    row.trainer.name,
    row.tr,
    row.teamLevel,
  ])

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
    const anchors = DEFAULT_ARCHETYPE_GROWTH.bursts
    expect(
      [0, 1, 39, 40, 41, 79, 80, 119, 120, 159, 160, 400].map((tr) => scale(anchors, tr, "step")),
    ).toEqual([0, 0, 0, 25, 25, 25, 50, 50, 75, 75, 100, 100])
    // The same anchors interpolated rise between the anchors instead.
    expect(scale(anchors, 20)).toBe(13)
    expect(scale(anchors, 20, "step")).toBe(0)
  })

  it("steps team size: 0–10 -> 1, 11–28 -> 2, 29–43 -> 3, 44–70 -> 4, 71–95 -> 5, 96+ -> 6", () => {
    const sizes = (trs: number[]) => trs.map((tr) => scale(DEFAULT_TEAM_SIZE, tr))
    expect(sizes([0, 5, 10])).toEqual([1, 1, 1])
    expect(sizes([11, 20, 28])).toEqual([2, 2, 2])
    expect(sizes([29, 35, 43])).toEqual([3, 3, 3])
    expect(sizes([44, 70])).toEqual([4, 4])
    expect(sizes([71, 95])).toEqual([5, 5])
    expect(sizes([96, 160, 500])).toEqual([6, 6, 6])
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
    // Steady Brock (25 → 100) reaches TR 96, the first TR at team size 6, at world progress 151:
    // 25 + 75 × 151/160 = 95.78. At 150 he is TR 95 (25 + 70.31), still team size 5.
    const record = catalog.find((entry) => entry.id === "brock")!
    const at = (world: number) => resolveTrainer(record, defaults, world, evolution)
    const before = at(150)
    const joined = at(151)
    expect([before.tr, before.size, before.team.map((member) => member.species)]).toEqual([
      95,
      5,
      ["Steelix", "Golem", "Crobat", "Kabutops", "Omastar"],
    ])
    // Team level at TR 96 is 50 + 16 × 25/40 = 60, so the Aerodactyl ace (offset 0) joins at Lv 60.
    expect([joined.tr, joined.size, joined.teamLevel]).toEqual([96, 6, 60])
    expect(joined.team[5]).toMatchObject({ slot: 6, species: "Aerodactyl", level: 60, isAce: true })
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

  it("keeps authored moves only at the authored stage and falls back to LEVEL_UP below it", () => {
    const golem: RosterSlot = { ...slot("Golem", 0), moves: ["Rock Slide", "Earthquake"] }
    const member = (level: number) => buildTeam([golem], level, 1, evolution).team[0]!
    expect(member(37)).toMatchObject({
      species: "Graveler",
      authoredSpecies: "Golem",
      authoredAt: 38,
      moves: "LEVEL_UP",
    })
    expect(member(38)).toMatchObject({
      species: "Golem",
      authoredSpecies: "Golem",
      authoredAt: null,
      moves: ["Rock Slide", "Earthquake"],
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
    expect(leagueLineup(catalog, experiment, 300, evolution)).toEqual(
      leagueLineup(catalog, experiment, 160, evolution).map((row) => ({
        ...row,
        worldProgress: 300,
      })),
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
    // TR 96 (team size 6) at world progress 151 brings in the slot-6 Aerodactyl ace at team level 60.
    expect(
      brock.find((m) => m.events.some((e) => e.kind === "join" && e.species === "Aerodactyl")),
    ).toMatchObject({
      worldProgress: 151,
      tr: 96,
      teamLevel: 60,
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
      "76: Graveler → Golem, Golbat → Crobat",
      "85: Kabuto → Kabutops",
      "98: 5th slot (Omastar) joins",
      "151: 6th slot (Aerodactyl) ace joins",
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
      "49: Pidgeotto → Pidgeot, Nidorino → Nidoking",
      "55: Kadabra → Alakazam",
      "61: 5th slot (Scizor) joins",
      "86: 6th slot (Arcanine) ace joins",
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
    const fixture = trainer("fixture", 10, six, { archetype: "plateau", peakTR: 60 })
    const experiment = experimentWith([fixture])
    experiment.archetypes.plateau = [
      [0, 0],
      [40, 50],
    ]
    const list = milestones(fixture, experiment, NO_EVOLUTION)
    // Growth stops at 50% (TR 35) at world progress 40; the level cap ceiling is TR 160.
    expect(milestoneEnd(experiment, "plateau")).toBe(160)
    expect(list.at(-1)?.events.at(-1)).toEqual({ kind: "peak", tr: 35, reached: false })
    expect(list.at(-1)?.worldProgress).toBe(160)
    experiment.archetypes.plateau = [
      [0, 0],
      [1_000_000, 100],
    ]
    expect(milestoneEnd(experiment, "plateau")).toBe(MILESTONE_SCAN_LIMIT)
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
    ["five moves", [{ ...slot("Steelix", 0), moves: ["A", "B", "C", "D", "E"] }], "moves"],
    ["no moves", [{ ...slot("Steelix", 0), moves: [] }], "moves"],
    ["a blank species", [slot(" ", 0)], "species"],
    ["an unknown key", [{ ...slot("Steelix", 0), ace: true }], "unknown fields"],
    [
      "a missing ace flag",
      [
        {
          species: "Steelix",
          levelOffset: 0,
          moves: "LEVEL_UP",
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
    ["early bloomer", [0, 50, 80, 95, 100]],
    ["late bloomer", [0, 10, 25, 55, 100]],
    ["plateau", [0, 60, 100, 100, 100]],
  ] as const)(
    "reads %s growth at the anchors (start 0, peak 100 gives the growth %)",
    (name, pct) => {
      expect(WORLD_PROGRESS_CHECKPOINTS.map(trAt(name, 0, 100))).toEqual(pct)
      expect(growth[name].map(([world]) => world)).toEqual([...WORLD_PROGRESS_CHECKPOINTS])
    },
  )

  it.each([
    ["rising star", [0, 10, 50, 90, 100]],
    ["second wind", [0, 45, 50, 55, 100]],
    ["bursts", [0, 25, 50, 75, 100]],
  ] as const)("reads %s growth at the §11 anchors", (name, pct) => {
    expect(WORLD_PROGRESS_CHECKPOINTS.map(trAt(name, 0, 100))).toEqual(pct)
    expect(growth[name].map(([world]) => world)).toEqual([...WORLD_PROGRESS_CHECKPOINTS])
  })

  it("makes bursts the only step scaler and lists all nine archetypes", () => {
    expect(ARCHETYPES).toEqual([
      "steady",
      "early bloomer",
      "late bloomer",
      "plateau",
      "rival",
      "fixed",
      "rising star",
      "second wind",
      "bursts",
    ])
    expect(ARCHETYPES.filter((name) => ARCHETYPE_KIND[name] === "step")).toEqual(["bursts"])
  })

  it("interpolates rising star and second wind midpoints", () => {
    // Rising star at 20 is 5%, at 60 is 30%, at 100 is 70%, at 140 is 95%.
    expect([20, 60, 100, 140].map(trAt("rising star", 0, 100))).toEqual([5, 30, 70, 95])
    // Second wind at 20 is 22.5% (rounds up), at 60 is 47.5%, at 100 is 52.5%, at 140 is 77.5%.
    expect([20, 60, 100, 140].map(trAt("second wind", 0, 100))).toEqual([23, 48, 53, 78])
    // Rising star 26 -> 110 at 100: 26 + 84 × 70% = 84.8.
    expect(trAt("rising star", 26, 110)(100)).toBe(85)
  })

  it("holds bursts between anchors and jumps at 4, 8, 16 and 24 badges", () => {
    const read = trAt("bursts", 24, 166)
    // 24 + 142 × 25% = 59.5 rounds up; 50% = 95; 75% = 130.5 rounds up.
    expect([0, 39, 40, 79, 80, 119, 120, 159, 160, 400].map(read)).toEqual([
      24, 24, 60, 60, 95, 95, 131, 131, 166, 166,
    ])
    // At the anchors bursts equals steady; between them it holds below it.
    for (const world of WORLD_PROGRESS_CHECKPOINTS)
      expect(read(world)).toBe(trAt("steady", 24, 166)(world))
    expect(read(60)).toBeLessThan(trAt("steady", 24, 166)(60))
  })

  it("keeps a fixed trainer at start TR at every world progress", () => {
    expect(growth.fixed).toEqual([
      [0, 0],
      [160, 0],
    ])
    const read = trAt("fixed", 95, 95)
    for (let world = 0; world <= 200; world += 1) expect(read(world)).toBe(95)
    expect(read(1_000_000)).toBe(95)
  })

  it("reads the rival as an ordinary growth scaler: 0/20/40/80/120/160 -> 0/15/29/53/76/100%", () => {
    expect([0, 20, 40, 80, 120, 160].map(trAt("rival", 0, 100))).toEqual([0, 15, 29, 53, 76, 100])
    // The same start + (peak - start) x growth% rule as every archetype, so a start TR counts.
    expect(trAt("rival", 20, 120)(80)).toBe(73)
  })

  it("interpolates midpoints exactly and rounds halves up once", () => {
    // Steady 0 -> 100 at world progress 20 is 12.5%: 12.5 rounds up to 13.
    expect(trAt("steady", 0, 100)(20)).toBe(13)
    // Start 10, peak 50, steady at 20: 10 + 40 * 12.5% = 15 (no rounding of the % first).
    expect(trAt("steady", 10, 50)(20)).toBe(15)
    // Late bloomer 20 -> 180 at 100: 20 + 40% of 160 = 84.
    expect(trAt("late bloomer", 20, 180)(100)).toBe(84)
    // Early bloomer 0 -> 90 at 60: 65% of 90 = 58.5 rounds up.
    expect(trAt("early bloomer", 0, 90)(60)).toBe(59)
    // Plateau 45 -> 94 at 20: 30% of 49 = 14.7.
    expect(trAt("plateau", 45, 94)(20)).toBe(60)
    expect(growthTR(growth.steady, 2, 95, 1)).toBe(3) // 2 + 93 * 0.625% = 2.58
  })

  it("stays flat past world progress 160 and keeps start at 0", () => {
    for (const name of ARCHETYPES.filter((name) => name !== "fixed")) {
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

  it("puts the first league at TR 85–95 at world progress 80 (8 badges, level cap 50)", () => {
    expect(worldProgress({ badges: 8 })).toBe(80)
    for (const row of leagueLineup(catalog, defaults, 80, evolution)) {
      expect(row.tr).toBeGreaterThanOrEqual(85)
      expect(row.tr).toBeLessThanOrEqual(95)
      expect(row.teamLevel).toBeGreaterThanOrEqual(53)
      expect(row.teamLevel).toBeLessThanOrEqual(59)
    }
  })

  it("puts the lineup a little above the level cap at world progress 120", () => {
    expect(cap(120)).toBe(75)
    for (const row of leagueLineup(catalog, defaults, 120, evolution)) {
      expect(row.teamLevel - cap(120)).toBeGreaterThanOrEqual(2)
      expect(row.teamLevel - cap(120)).toBeLessThanOrEqual(8)
    }
  })

  it("puts the lineup at Lv 100 at world progress 160, where the team level scaler tops out", () => {
    // The level cap is Lv 100 at world progress 160 and team level stops at Lv 100 too, so the
    // lineup can only match the cap there. Every member sits past the team level ceiling TR.
    expect(cap(160)).toBe(100)
    for (const row of leagueLineup(catalog, defaults, 160, evolution)) {
      expect(row.tr).toBeGreaterThan(160)
      expect(row.teamLevel).toBe(cap(160))
    }
  })

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
        // The lowest late bloomer still sits near the player at world progress 40, so none is
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

  it("makes the hardest leaders at 24 badges late bloomers or high-peak steadies", () => {
    const top = gymLadder(catalog, defaults, 160).slice(-5)
    for (const row of top) {
      const settings = defaults.trainers[row.trainer.id]!
      expect(
        settings.archetype === "late bloomer" ||
          (settings.archetype === "steady" && settings.peakTR >= 170),
      ).toBe(true)
    }
  })

  it("starts every Gym Leader in the 18–40 band by archetype, and none is fixed", () => {
    const bands: Record<string, [number, number]> = {
      "late bloomer": [18, 26],
      "rising star": [18, 26],
      "early bloomer": [22, 30],
      steady: [24, 34],
      bursts: [24, 34],
      plateau: [30, 40],
      "second wind": [30, 40],
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

  it("starts Blue at one Eevee at Lv 5 and grows him with the rival scaler", () => {
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

describe("league lineup", () => {
  it("picks the top five by TR in ascending battle order, strongest last", () => {
    expect(lineupAt(80)).toEqual([
      ["Lt. Surge", 95, 59],
      ["Giovanni", 95, 59],
      ["Agatha", 95, 59],
      ["Jasmine", 95, 59],
      ["Norman", 95, 59],
    ])
    expect(leagueLineup(catalog, defaults, 80, evolution).at(-1)?.trainer.name).toBe("Norman")
  })

  it("computes the lineup at the world progress it is entered at", () => {
    expect(lineupAt(120)).toEqual([
      ["Norman", 130, 81],
      ["Steven", 130, 81],
      ["Giovanni", 131, 82],
      ["Jasmine", 131, 82],
      ["Lance", 132, 83],
    ])
    expect(lineupAt(160)).toEqual([
      ["Clair", 185, 100],
      ["Juan", 185, 100],
      ["Wallace", 190, 100],
      ["Steven", 195, 100],
      ["Lance", 200, 100],
    ])
  })

  it("leaves league-ineligible entries out of the pool, however strong", () => {
    const duo = { ...trainer("duo", 150), leagueEligible: false, doubleBattle: true }
    const records = [duo, ...["a", "b", "c", "d", "e"].map((id) => trainer(id, 40))]
    const lineup = leagueLineup(records, experimentWith(records), 0, NO_EVOLUTION)
    expect(lineup.map((row) => row.trainer.id)).toEqual(["a", "b", "c", "d", "e"])
    for (const world of WORLD_PROGRESS_CHECKPOINTS)
      expect(
        leagueLineup(catalog, defaults, world, evolution).map((row) => row.trainer.id),
      ).not.toContain("tate-liza")
  })

  it("breaks ties by array order, with no tie-break logic", () => {
    const records = ["a", "b", "c", "d", "e", "f", "g"].map((id, index) =>
      trainer(id, index === 6 ? 90 : 40),
    )
    const lineup = leagueLineup(records, experimentWith(records), 0, NO_EVOLUTION)
    expect(lineup.map((row) => row.trainer.id)).toEqual(["a", "b", "c", "d", "g"])
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
    expect(growth("Lance")).toEqual([48, "late bloomer", 200])
    for (const record of catalog)
      if (record.archetype === "fixed") expect(record.peakTR).toBe(record.startTR)
    expect(Object.keys(experiment.trainers)).toHaveLength(38)
    expect(catalog.some((record) => record.id.startsWith("red"))).toBe(false)
  })

  it("applies the §11 archetype reassignment and leaves everyone else unchanged", () => {
    const byArchetype = (archetype: Archetype) =>
      catalog.filter((record) => record.archetype === archetype).map((record) => record.name)
    expect(byArchetype("rising star")).toEqual([
      "Misty",
      "Bugsy",
      "Whitney",
      "Flannery",
      "Tate & Liza",
    ])
    expect(byArchetype("second wind")).toEqual(["Blaine", "Bruno", "Pryce"])
    expect(byArchetype("bursts")).toEqual(["Giovanni", "Chuck", "Brawly"])
    expect(byArchetype("fixed")).toEqual(["Agatha"])
    expect(byArchetype("plateau")).toEqual(["Lt. Surge", "Lorelei", "Wattson", "Glacia", "Drake"])
    expect(byArchetype("late bloomer")).toEqual([
      "Sabrina",
      "Lance",
      "Morty",
      "Clair",
      "Winona",
      "Juan",
      "Wallace",
      "Steven",
    ])
    expect(byArchetype("early bloomer")).toEqual(["Janine", "Falkner", "Will", "Sidney"])
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
    // Species in Brock's source party keep their battle content; the others use LEVEL_UP.
    expect(brock.roster.map((rosterSlot) => rosterSlot.moves !== "LEVEL_UP")).toEqual([
      false,
      true,
      false,
      true,
      true,
      true,
    ])
    expect(brock.roster[0]?.item).toBeNull()
    // Blue's signature Umbreon steps down to Eevee early.
    expect(catalog.find((record) => record.id === "blue")!.roster[0]).toMatchObject({
      species: "Umbreon",
      isAce: true,
      moves: "LEVEL_UP",
    })
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

  it("rejects version 9, which had only five archetypes", () => {
    const v9 = base()
    v9.version = 9
    for (const name of ["fixed", "rising star", "second wind", "bursts"]) delete v9.archetypes[name]
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

  it("rejects version 7, which gave the rival a fixed lead", () => {
    const v7 = base()
    v7.version = 7
    delete v7.archetypes.rival
    v7.trainers.fixture.lead = null
    expect(() => validateExperiment(v7, records)).toThrow(
      "Version 7 experiments give the rival a fixed lead",
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
      "a fixed trainer whose peak differs from the start",
      { archetype: "fixed", startTR: 3, peakTR: 4 },
      "a fixed trainer's peak TR must equal start TR",
    ],
    ["a fixed Gym Leader", { archetype: "fixed", startTR: 3, peakTR: 3 }, "cannot be fixed"],
  ])("rejects %s", (_name, change, message) => {
    const input = base()
    Object.assign(input.trainers.fixture, change)
    expect(() => validateExperiment(input, records)).toThrow(message)
  })

  it("accepts a fixed trainer outside the Gyms with peak TR equal to start TR", () => {
    const elite = [{ ...trainer("fixture", 3), role: "Elite Four" as const }]
    const input = JSON.parse(serializeExperiment(experimentWith(elite)))
    Object.assign(input.trainers.fixture, { archetype: "fixed", startTR: 95, peakTR: 95 })
    expect(validateExperiment(input, elite).trainers.fixture).toMatchObject({
      archetype: "fixed",
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
    delete input.archetypes.plateau
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

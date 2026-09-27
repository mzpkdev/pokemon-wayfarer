import { describe, expect, it } from "vitest"

import catalogData from "./catalog.json"
import {
  DEFAULT_ARCHETYPE_GROWTH,
  DEFAULT_REGULAR_TRAINER_LEVEL,
  DEFAULT_TEAM_SIZE,
  DEFAULT_WILD_LEVEL,
  LEVEL_CAP_ANCHORS,
  NEAR_BAND,
  WORLD_PROGRESS_CHECKPOINTS,
  buildTeam,
  createExperiment,
  growthTR,
  gymLadder,
  leagueLineup,
  playerRating,
  resolveTrainer,
  rosterGaps,
  scale,
  serializeExperiment,
  teamLevelFor,
  teamSizeFor,
  trainerRating,
  validateExperiment,
  validateRoster,
  worldLevels,
  worldProgress,
} from "./engine.js"
import type { Archetype, Experiment, RosterSlot, TrainerRecord, WorldPoint } from "./types.js"

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
/** A fixture trainer; by default steady with start = peak, so its TR is `tr` at every world progress. */
const trainer = (
  id: string,
  tr: number,
  species = six,
  growth: { archetype?: Archetype; peakTR?: number; lead?: number | null } = {},
): TrainerRecord => ({
  id,
  name: id,
  region: "Kanto",
  role: "Gym Leader",
  source: { label: "Fixture", path: "fixture", trainerId: "TEST", note: "Test data" },
  referenceParty: [],
  startTR: tr,
  archetype: growth.archetype ?? "steady",
  peakTR: growth.peakTR ?? tr,
  lead: growth.lead ?? null,
  trSource: "Fixture",
  roster: species.map((name, index) => slot(name, index === 0 ? 0 : -2)),
  rosterSource: "Fixture",
})
const experimentWith = (records: TrainerRecord[]): Experiment => createExperiment(records)
const defaults = createExperiment(catalog)
const at = (world: number) => catalog.map((record) => resolveTrainer(record, defaults, world))
const lineupAt = (world: number) =>
  leagueLineup(catalog, defaults, world).map((row) => [row.trainer.name, row.tr, row.teamLevel])

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
    const early = resolveTrainer(low, experiment, 0)
    expect([early.teamLevel, early.size]).toEqual([15, 2])
    expect(early.battleOrder.map((member) => [member.species, member.level])).toEqual([
      ["Golem", 13],
      ["Steelix", 15],
    ])
    const late = resolveTrainer(high, experiment, 0)
    expect([late.teamLevel, late.size]).toEqual([100, 6])
    expect(late.battleOrder.at(-1)).toMatchObject({ species: "Steelix", level: 100 })
    expect(late.warnings).toEqual([])
  })

  it("flags a short roster and uses what exists", () => {
    const short = trainer("short", 120, six.slice(0, 5))
    const resolved = resolveTrainer(short, experimentWith([short]), 0)
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
    const resolved = resolveTrainer(brock, defaults, world)
    expect(resolved).toEqual(resolveTrainer(brock, defaults, playerRating({ badges: 8 })))
    expect(resolved.tr).toBe(49)
    // Resolution takes a number, so a party or level cap cannot be passed in.
    expect(resolveTrainer.length).toBe(3)
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

describe("archetype growth", () => {
  const growth = DEFAULT_ARCHETYPE_GROWTH
  const fixtureExperiment = createExperiment([trainer("fixture", 0)])
  const trAt =
    (archetype: Archetype, start: number, peak: number, lead: number | null = null) =>
    (world: number) =>
      trainerRating(fixtureExperiment, { startTR: start, archetype, peakTR: peak, lead }, world)

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
    for (const name of ["steady", "early bloomer", "late bloomer", "plateau"] as const) {
      const read = trAt(name, 12, 172)
      expect(read(0)).toBe(12)
      expect([read(160), read(200), read(1_000_000)]).toEqual([172, 172, 172])
    }
  })

  it("keeps the rival a lead ahead until the peak, then holds the peak", () => {
    const blue = trAt("rival", 10, 180, 10)
    expect([0, 1, 40, 80, 120, 160, 169, 170, 171, 400].map(blue)).toEqual([
      10, 11, 50, 90, 130, 170, 179, 180, 180, 180,
    ])
    const record = catalog.find((entry) => entry.id === "blue")!
    for (let world = 0; world <= 170; world += 1)
      expect(resolveTrainer(record, defaults, world).tr - world).toBe(10)
    expect(() => trAt("rival", 10, 180, null)(0)).toThrow("A rival needs a lead")
  })

  it("never lowers a trainer TR as world progress rises", () => {
    for (const record of catalog) {
      let previous = -1
      for (let world = 0; world <= 200; world += 1) {
        const tr = resolveTrainer(record, defaults, world).tr
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
    for (const row of leagueLineup(catalog, defaults, 80)) {
      expect(row.tr).toBeGreaterThanOrEqual(85)
      expect(row.tr).toBeLessThanOrEqual(95)
      expect(row.teamLevel).toBeGreaterThanOrEqual(53)
      expect(row.teamLevel).toBeLessThanOrEqual(59)
    }
  })

  it("puts the lineup a little above the level cap at world progress 120", () => {
    expect(cap(120)).toBe(75)
    for (const row of leagueLineup(catalog, defaults, 120)) {
      expect(row.teamLevel - cap(120)).toBeGreaterThanOrEqual(2)
      expect(row.teamLevel - cap(120)).toBeLessThanOrEqual(8)
    }
  })

  it("puts the lineup at Lv 100 at world progress 160, where the team level scaler tops out", () => {
    // The level cap is Lv 100 at world progress 160 and team level stops at Lv 100 too, so the
    // lineup can only match the cap there. Every member sits past the team level ceiling TR.
    expect(cap(160)).toBe(100)
    for (const row of leagueLineup(catalog, defaults, 160)) {
      expect(row.tr).toBeGreaterThan(160)
      expect(row.teamLevel).toBe(cap(160))
    }
  })

  it.each([...WORLD_PROGRESS_CHECKPOINTS])(
    "spreads the 23 Gym Leaders below, near and above the player at world progress %i",
    (world) => {
      const ladder = gymLadder(catalog, defaults, world)
      expect(ladder).toHaveLength(23)
      const count = (mark: string) => ladder.filter((row) => row.mark === mark).length
      expect(count("near")).toBeGreaterThanOrEqual(3)
      expect(count("above")).toBeGreaterThanOrEqual(3)
      // No TR is below 0, so at world progress 0 the openers are the accessible leaders.
      if (world === 0)
        expect(ladder.filter((row) => row.tr <= 5).map((row) => row.trainer.name)).toEqual(
          expect.arrayContaining(["Brock", "Falkner", "Roxanne"]),
        )
      else expect(count("below")).toBeGreaterThanOrEqual(3)
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

  it("keeps Blue 10 ahead of the player at every checkpoint", () => {
    const blue = at(0).find((row) => row.trainer.id === "blue")!
    expect(blue.lead).toBe(10)
    for (const world of WORLD_PROGRESS_CHECKPOINTS)
      expect(resolveTrainer(blue.trainer, defaults, world).tr).toBe(world + 10)
  })
})

describe("league lineup", () => {
  it("picks the top five by TR in ascending battle order, strongest last", () => {
    expect(lineupAt(80)).toEqual([
      ["Giovanni", 93, 58],
      ["Bruno", 94, 59],
      ["Will", 94, 59],
      ["Norman", 94, 59],
      ["Agatha", 95, 59],
    ])
    expect(leagueLineup(catalog, defaults, 80).at(-1)?.trainer.name).toBe("Agatha")
  })

  it("computes the lineup at the world progress it is entered at", () => {
    expect(lineupAt(120)).toEqual([
      ["Blue", 130, 81],
      ["Norman", 131, 82],
      ["Giovanni", 132, 83],
      ["Lance", 132, 83],
      ["Jasmine", 132, 83],
    ])
    expect(lineupAt(160)).toEqual([
      ["Clair", 185, 100],
      ["Juan", 185, 100],
      ["Wallace", 190, 100],
      ["Steven", 195, 100],
      ["Lance", 200, 100],
    ])
  })

  it("breaks ties by array order, with no tie-break logic", () => {
    const records = ["a", "b", "c", "d", "e", "f", "g"].map((id, index) =>
      trainer(id, index === 6 ? 90 : 40),
    )
    const lineup = leagueLineup(records, experimentWith(records), 0)
    expect(lineup.map((row) => row.trainer.id)).toEqual(["a", "b", "c", "d", "g"])
  })
})

describe("catalog", () => {
  it("ships 37 trainers with placeholder TRs and valid rosters of at most six", () => {
    expect(catalog).toHaveLength(37)
    const experiment = createExperiment(catalog)
    for (const record of catalog) {
      expect(Number.isInteger(record.startTR) && record.startTR >= 0).toBe(true)
      expect(record.peakTR).toBeGreaterThanOrEqual(record.startTR)
      expect(record.lead === null).toBe(record.archetype !== "rival")
      expect(record.trSource).toContain("PLACEHOLDER")
      expect(record.roster[0]?.levelOffset).toBe(0)
      expect(record.roster.length).toBeLessThanOrEqual(6)
    }
    const growth = (name: string) => {
      const record = catalog.find((entry) => entry.name === name)!
      return [record.startTR, record.archetype, record.peakTR, record.lead]
    }
    expect(growth("Blue")).toEqual([10, "rival", 180, 10])
    expect(catalog.filter((record) => record.archetype === "rival")).toHaveLength(1)
    expect(growth("Brock")).toEqual([2, "steady", 95, null])
    expect(growth("Lance")).toEqual([48, "late bloomer", 200, null])
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
    restored.trainers.fixture!.startTR = 9
    restored.archetypes.steady[1]![1] = 40
    expect(experiment.trainers.fixture!.startTR).toBe(3)
    expect(experiment.archetypes.steady[1]).toEqual([40, 25])
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
    ["a rival without a lead", { archetype: "rival", lead: null }, "a rival needs a lead"],
    ["a lead on a non-rival", { archetype: "plateau", lead: 5 }, "only a rival has a lead"],
    ["an unknown archetype", { archetype: "arc" }, "archetype must be one of"],
    ["a negative lead", { archetype: "rival", lead: -1 }, "non-negative whole number"],
    ["the retired fixed TR", { tr: 3 }, "missing or unknown fields"],
  ])("rejects %s", (_name, change, message) => {
    const input = base()
    Object.assign(input.trainers.fixture, change)
    expect(() => validateExperiment(input, records)).toThrow(message)
  })

  it("validates the archetype growth scalers", () => {
    const input = base()
    delete input.archetypes.plateau
    expect(() => validateExperiment(input, records)).toThrow("missing or unknown fields")
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

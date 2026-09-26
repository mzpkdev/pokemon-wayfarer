import { describe, expect, it } from "vitest"

import catalogData from "./catalog.json"
import {
  createExperiment,
  playerCap,
  playerRating,
  resolveLevel,
  resolveTrainer,
  serializeExperiment,
  validateExperiment,
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
  suggestedStartTR: 0,
  suggestedMatureTR: 55,
}
const elite: TrainerRecord = {
  ...gym,
  id: "test-elite",
  role: "Elite Four",
  gymEligible: false,
  referenceParty: gym.competitiveParty.slice(0, 5),
  suggestedStartTR: 55,
  suggestedMatureTR: 63,
}
const catalog = [gym, elite]
const imported = (): Experiment =>
  JSON.parse(serializeExperiment(createExperiment(catalog))) as Experiment

describe("trainer balance progression", () => {
  it("uses authored checkpoints for an arbitrary Gym-eligible Champion", () => {
    const trainer: TrainerRecord = {
      ...gym,
      id: "renamed-dual-role",
      name: "Independent Fixture",
      role: "Champion",
      badgeTRCheckpoints: [6, 54, 57, 59],
    }
    const experiment = createExperiment([trainer])
    const checkpoints = [0, 8, 16, 24].map((badges) =>
      resolveTrainer(trainer, experiment, { badges, leagueClears: 0 }),
    )
    expect(checkpoints.map((entry) => entry.tr)).toEqual([6, 54, 57, 59])
    expect(checkpoints.map((entry) => entry.aceLevel)).toEqual([17, 59, 64, 68])
    expect(checkpoints[0]?.party).toHaveLength(2)
    expect(checkpoints[1]?.party).toHaveLength(6)
  })

  it("retains derived Gym and Elite Four curves when checkpoints are absent", () => {
    const experiment = createExperiment(catalog)
    expect(experiment.trainers[gym.id]?.ratings).toEqual([0, 35, 45, 55])
    expect(experiment.trainers[elite.id]?.ratings).toEqual([55, 58, 60, 63])
  })

  it.each([
    [6, 54, 53, 59],
    [-1, 54, 57, 59],
    [6, 54, 57, 81],
    [6, 54.5, 57, 59],
  ])("rejects invalid authored checkpoints %j instead of clamping", (...ratings) => {
    const trainer: TrainerRecord = {
      ...gym,
      badgeTRCheckpoints: ratings as [number, number, number, number],
    }
    expect(() => createExperiment([trainer])).toThrow(/Invalid experiment:/)
  })

  it("gives Blue an approachable start and Champion strength by eight badges", () => {
    const fullCatalog = catalogData as TrainerRecord[]
    const blue = fullCatalog.find((trainer) => trainer.id === "blue")
    if (!blue) throw new Error("Blue is missing from the catalog")
    const experiment = createExperiment(fullCatalog)
    const checkpoints = [0, 8, 16, 24].map((badges) =>
      resolveTrainer(blue, experiment, { badges, leagueClears: 0 }),
    )
    expect(checkpoints.map((entry) => entry.tr)).toEqual([6, 54, 57, 59])
    expect(checkpoints.map((entry) => entry.aceLevel)).toEqual([17, 59, 64, 68])
  })

  it("resolves all 37 catalog trainers across every badge and league-clear combination", () => {
    const fullCatalog = catalogData as TrainerRecord[]
    expect(fullCatalog).toHaveLength(37)
    const experiment = createExperiment(fullCatalog)
    for (const trainer of fullCatalog) {
      for (let leagueClears = 0; leagueClears <= 3; leagueClears += 1) {
        let previous = resolveTrainer(trainer, experiment, { badges: 0, leagueClears })
        for (let badges = 0; badges <= 24; badges += 1) {
          const current = resolveTrainer(trainer, experiment, { badges, leagueClears })
          expect(current).toEqual(resolveTrainer(trainer, experiment, { badges, leagueClears }))
          expect(current.tr).toBeGreaterThanOrEqual(previous.tr)
          expect(current.party.length).toBeGreaterThanOrEqual(previous.party.length)
          expect(current.party.length).toBeGreaterThanOrEqual(2)
          expect(current.party.length).toBeLessThanOrEqual(6)
          expect(Math.min(...current.party.map((entry) => entry.level))).toBeGreaterThanOrEqual(
            Math.min(...previous.party.map((entry) => entry.level)),
          )
          expect(
            current.party.every(
              (entry) => Number.isInteger(entry.level) && entry.level >= 1 && entry.level <= 100,
            ),
          ).toBe(true)
          previous = current
        }
      }
    }
  })

  it("retains the player badge formula and original level curve", () => {
    expect([0, 4, 8, 16, 24].map((badges) => playerRating({ badges, leagueClears: 0 }))).toEqual([
      0, 16, 40, 48, 56,
    ])
    expect(playerRating({ badges: 24, leagueClears: 3 })).toBe(80)
    expect(playerRating({ badges: 8, leagueClears: 1 })).toBe(48)
    expect(playerCap({ badges: 0, leagueClears: 0 })).toBe(15)
    expect(playerCap({ badges: 24, leagueClears: 3 })).toBe(100)
  })

  it("interpolates half up and distinguishes the approachable NPC baseline", () => {
    const experiment = createExperiment(catalog)
    expect(
      resolveLevel(1, [
        [0, 12],
        [2, 13],
        [80, 100],
      ]),
    ).toBe(13)
    expect(
      resolveTrainer(gym, experiment, { badges: 0, leagueClears: 0 }).party.map(
        (entry) => entry.level,
      ),
    ).toEqual([10, 12])
    expect(resolveLevel(-12, experiment.levelAnchors)).toBe(12)
    expect(resolveLevel(90, experiment.levelAnchors)).toBe(100)
  })

  it("keeps rating, stable party slots, team sizes, and transition extrema monotone", () => {
    const experiment = createExperiment(catalog)
    for (const trainer of catalog) {
      for (let leagueClears = 0; leagueClears <= 3; leagueClears += 1) {
        let previous = resolveTrainer(trainer, experiment, { badges: 0, leagueClears })
        for (let badges = 1; badges <= 24; badges += 1) {
          const current = resolveTrainer(trainer, experiment, { badges, leagueClears })
          expect(current.tr).toBeGreaterThanOrEqual(previous.tr)
          expect(current.party.length).toBeGreaterThanOrEqual(previous.party.length)
          expect(Math.min(...current.party.map((entry) => entry.level))).toBeGreaterThanOrEqual(
            Math.min(...previous.party.map((entry) => entry.level)),
          )
          expect(Math.max(...current.party.map((entry) => entry.level))).toBeGreaterThanOrEqual(
            Math.max(...previous.party.map((entry) => entry.level)),
          )
          if (current.stage === previous.stage)
            current.party.forEach((entry, slot) =>
              expect(entry.level).toBeGreaterThanOrEqual(previous.party[slot].level),
            )
          previous = current
        }
      }
    }
  })

  it("selects each team on its exact TR threshold, without altering source parties", () => {
    const before = JSON.stringify(catalog)
    const experiment = createExperiment(catalog)
    for (const stage of experiment.trainers[gym.id].stages) {
      experiment.trainers[gym.id].ratings = [stage.minTR, stage.minTR, stage.minTR, stage.minTR]
      expect(resolveTrainer(gym, experiment, { badges: 0, leagueClears: 0 }).stage).toBe(
        stage.label,
      )
    }
    expect(JSON.stringify(catalog)).toBe(before)
    expect(createExperiment(catalog).trainers[elite.id].stages[0].party).toHaveLength(5)
  })

  it("preserves authored duplicates and does not duplicate shared species while expanding", () => {
    const duplicate = { ...gym, referenceParty: [member("Onix", 12), member("Onix", 14)] }
    const experiment = createExperiment([duplicate])
    const intermediate = experiment.trainers[gym.id].stages[1].party
    expect(intermediate.filter((entry) => entry.species === "Onix")).toHaveLength(2)
    expect(intermediate).toHaveLength(4)
  })

  it("clamps world points and reports attempted overflow, cap exceedance, and level clipping", () => {
    const experiment = createExperiment(catalog)
    expect(resolveTrainer(gym, experiment, { badges: -5, leagueClears: -1 }).tr).toBe(0)
    expect(playerRating({ badges: 999, leagueClears: 999 })).toBe(80)
    experiment.trainers[gym.id].ratings = [80, 80, 80, 80]
    const saturated = resolveTrainer(gym, experiment, { badges: 0, leagueClears: 3 })
    expect(saturated.tr).toBe(80)
    expect(saturated.unclampedTR).toBe(98)
    expect(saturated.warnings.join(" ")).toMatch(/saturation.*98/)
    expect(saturated.warnings.join(" ")).toMatch(/soft level cap/)
    expect(saturated.warnings.join(" ")).toMatch(/Maximum level/)
    const low = createExperiment(catalog)
    low.levelAnchors[0][1] = 1
    expect(resolveTrainer(gym, low, { badges: 0, leagueClears: 0 }).warnings.join(" ")).toMatch(
      /clipped/,
    )
  })

  it("rejects nonfinite world and rating inputs instead of resolving NaN", () => {
    expect(() => playerRating({ badges: NaN, leagueClears: 0 })).toThrow(/finite/)
    expect(() => playerCap({ badges: 0, leagueClears: Infinity })).toThrow(/finite/)
    expect(() => resolveLevel(NaN, createExperiment(catalog).levelAnchors)).toThrow(/finite/)
    expect(() =>
      resolveLevel(5, [
        [0, 12],
        [0, NaN],
        [80, 100],
      ]),
    ).toThrow(/finite/)
  })
})

describe("experiment import", () => {
  it("round-trips deterministic JSON and returns an independent validated value", () => {
    const experiment = createExperiment(catalog)
    const json = serializeExperiment(experiment)
    const restored = validateExperiment(JSON.parse(json), catalog)
    expect(serializeExperiment(restored)).toBe(json)
    expect(serializeExperiment(createExperiment(catalog))).toBe(json)
    restored.levelAnchors[0][1] = 1
    expect(experiment.levelAnchors[0][1]).toBe(12)
  })

  it.each([
    [
      "version",
      (value: Experiment) => {
        ;(value as { version: number }).version = 2
      },
    ],
    [
      "unknown IDs",
      (value: Experiment) => {
        value.trainers.unknown = value.trainers[gym.id]
      },
    ],
    [
      "missing IDs",
      (value: Experiment) => {
        delete value.trainers[gym.id]
      },
    ],
    [
      "nonfinite rating",
      (value: Experiment) => {
        value.trainers[gym.id].ratings[0] = NaN
      },
    ],
    [
      "descending rating",
      (value: Experiment) => {
        value.trainers[gym.id].ratings[0] = 40
      },
    ],
    [
      "sparse rating",
      (value: Experiment) => {
        delete (value.trainers[gym.id].ratings as number[])[1]
      },
    ],
    [
      "growth bounds",
      (value: Experiment) => {
        value.trainers[gym.id].leagueGrowth = 21
      },
    ],
    [
      "anchor coverage",
      (value: Experiment) => {
        value.levelAnchors[0][0] = 1
      },
    ],
    [
      "anchor duplicates",
      (value: Experiment) => {
        value.levelAnchors[1][0] = 0
      },
    ],
    [
      "descending levels",
      (value: Experiment) => {
        value.levelAnchors[1][1] = 1
      },
    ],
    [
      "missing initial stage",
      (value: Experiment) => {
        value.trainers[gym.id].stages[0].minTR = 1
      },
    ],
    [
      "duplicate thresholds",
      (value: Experiment) => {
        value.trainers[gym.id].stages[1].minTR = 0
      },
    ],
    [
      "unreachable threshold",
      (value: Experiment) => {
        value.trainers[gym.id].stages[2].minTR = 81
      },
    ],
    [
      "empty label",
      (value: Experiment) => {
        value.trainers[gym.id].stages[0].label = " "
      },
    ],
    [
      "empty party",
      (value: Experiment) => {
        value.trainers[gym.id].stages[0].party = []
      },
    ],
    [
      "decreasing party",
      (value: Experiment) => {
        value.trainers[gym.id].stages[1].party = [{ species: "Onix", levelOffset: 0 }]
      },
    ],
    [
      "empty species",
      (value: Experiment) => {
        value.trainers[gym.id].stages[0].party[0].species = ""
      },
    ],
    [
      "no ace",
      (value: Experiment) => {
        value.trainers[gym.id].stages[0].party.forEach((entry) => {
          entry.levelOffset = -1
        })
      },
    ],
    [
      "offset bounds",
      (value: Experiment) => {
        value.trainers[gym.id].stages[0].party[0].levelOffset = -31
      },
    ],
  ])("rejects %s", (_name, mutate) => {
    const value = imported()
    mutate(value)
    expect(() => validateExperiment(value, catalog)).toThrow(/Invalid experiment:/)
  })
})

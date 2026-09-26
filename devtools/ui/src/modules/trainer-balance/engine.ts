import type {
  Experiment,
  LevelAnchor,
  PartyMember,
  ReferenceMember,
  ResolvedTrainer,
  TeamStage,
  TrainerRecord,
  WorldPoint,
} from "./types.js"

const NPC_ANCHORS: LevelAnchor[] = [
  [0, 12],
  [4, 16],
  [8, 18],
  [16, 23],
  [30, 30],
  [40, 42],
  [55, 60],
  [65, 80],
  [80, 100],
]
const PLAYER_ANCHORS: LevelAnchor[] = NPC_ANCHORS.map(([tr, level]) => [tr, tr === 0 ? 15 : level])
const clamp = (value: number, min: number, max: number) => Math.max(min, Math.min(max, value))
const finite = (value: number, label: string) => {
  if (!Number.isFinite(value)) throw new Error(`${label} must be finite`)
  return value
}
const normalizePoint = (point: WorldPoint): WorldPoint => ({
  badges: clamp(Math.round(finite(point.badges, "Badges")), 0, 24),
  leagueClears: clamp(Math.round(finite(point.leagueClears, "League clears")), 0, 3),
})

/** Positive, half-up interpolation, matching the ROM's integer cap rounding. */
export const resolveLevel = (tr: number, anchors: LevelAnchor[]): number => {
  const rating = clamp(finite(tr, "Trainer rating"), 0, 80)
  if (anchors.length < 2) throw new Error("Level anchors need at least two points")
  if (
    anchors[0]?.[0] !== 0 ||
    anchors.at(-1)?.[0] !== 80 ||
    anchors.some(([rating, level], index) => {
      const previous = anchors[index - 1]
      return (
        !Number.isFinite(rating) ||
        !Number.isFinite(level) ||
        level < 1 ||
        level > 100 ||
        (index > 0 && (!previous || rating <= previous[0] || level < previous[1]))
      )
    })
  ) {
    throw new Error(
      "Level anchors must cover 0–80 with increasing ratings and finite nondecreasing levels from 1–100",
    )
  }
  for (let index = 1; index < anchors.length; index += 1) {
    const lower = anchors[index - 1]
    const upper = anchors[index]
    if (!lower || !upper) throw new Error("Level anchors must not contain missing points")
    const [lowerTR, lowerLevel] = lower
    const [upperTR, upperLevel] = upper
    if (rating <= upperTR) {
      return Math.round(
        lowerLevel + ((rating - lowerTR) * (upperLevel - lowerLevel)) / (upperTR - lowerTR),
      )
    }
  }
  const last = anchors.at(-1)
  if (!last) throw new Error("Level anchors need a final point")
  return last[1]
}

export const playerRating = (point: WorldPoint): number => {
  const { badges, leagueClears } = normalizePoint(point)
  const badgeRating =
    badges <= 4 ? 4 * badges : badges <= 8 ? 16 + 6 * (badges - 4) : 40 + badges - 8
  return clamp(badgeRating + 8 * leagueClears, 0, 80)
}

export const playerCap = (point: WorldPoint): number =>
  resolveLevel(playerRating(point), PLAYER_ANCHORS)

const sourceParty = (party: ReferenceMember[]): PartyMember[] => {
  const ace = Math.max(...party.map((member) => member.level))
  return party.map((member) => ({
    species: member.species,
    levelOffset: clamp(member.level - ace, -30, 0),
  }))
}

// Preserve authored duplicate counts without doubling shared species when expanding.
const expandParty = (
  reference: ReferenceMember[],
  competitive: ReferenceMember[],
  size: number,
): ReferenceMember[] => {
  const result = reference.slice()
  const seen = new Map<string, number>()
  for (const member of competitive) {
    const count = (seen.get(member.species) ?? 0) + 1
    seen.set(member.species, count)
    if (
      result.length < size &&
      result.filter((entry) => entry.species === member.species).length < count
    )
      result.push(member)
  }
  return result
}

const preserveTransitionFloor = (stages: TeamStage[]): TeamStage[] => {
  for (let index = 1; index < stages.length; index += 1) {
    const previous = stages[index - 1]
    const current = stages[index]
    if (!previous || !current) throw new Error("Team stages must not contain missing entries")
    const previousAce = resolveLevel(current.minTR - 1, NPC_ANCHORS)
    const currentAce = resolveLevel(current.minTR, NPC_ANCHORS)
    const previousFloor =
      previousAce + Math.min(...previous.party.map((member) => member.levelOffset))
    current.party = current.party.map((member) => ({
      ...member,
      levelOffset: Math.max(member.levelOffset, previousFloor - currentAce),
    }))
  }
  return stages
}

export const createExperiment = (catalog: TrainerRecord[]): Experiment => {
  const trainers: Experiment["trainers"] = Object.create(null)
  for (const trainer of catalog) {
    const start = clamp(Math.round(trainer.suggestedStartTR), 0, 80)
    const mature = clamp(Math.max(start, Math.round(trainer.suggestedMatureTR)), 0, 80)
    const competitive = sourceParty(trainer.competitiveParty)
    let stages: TeamStage[]
    let ratings: [number, number, number, number]
    if (trainer.gymEligible) {
      const eight = clamp(35 + Math.round((mature - 55) / 5), start, mature)
      const sixteen = clamp(45 + Math.round((mature - 55) / 4), eight, mature)
      ratings = [start, eight, sixteen, mature]
      const early = trainer.earlyParty.map((species, index, party) => ({
        species,
        levelOffset: index === party.length - 1 ? 0 : -2,
      }))
      const intermediate = sourceParty(
        expandParty(
          trainer.referenceParty,
          trainer.competitiveParty,
          Math.min(competitive.length, Math.max(early.length, 4)),
        ),
      )
      stages = [
        { minTR: 0, label: "Approachable team", party: early },
        { minTR: 28, label: "Developing team", party: intermediate },
        { minTR: 48, label: "Competitive team", party: competitive },
      ]
    } else {
      ratings = [
        start,
        Math.round(start + (mature - start) / 3),
        Math.round(start + (2 * (mature - start)) / 3),
        mature,
      ]
      stages = [
        { minTR: 0, label: "Reference team", party: sourceParty(trainer.referenceParty) },
        {
          minTR: Math.min(80, Math.max(48, start + 4)),
          label: "Competitive team",
          party: competitive,
        },
      ]
    }
    if (trainer.badgeTRCheckpoints !== undefined) ratings = trainer.badgeTRCheckpoints
    trainers[trainer.id] = { ratings, leagueGrowth: 6, stages: preserveTransitionFloor(stages) }
  }
  return validateExperiment({ version: 1, levelAnchors: NPC_ANCHORS, trainers }, catalog)
}

export const resolveTrainer = (
  trainer: TrainerRecord,
  experiment: Experiment,
  point: WorldPoint,
): ResolvedTrainer => {
  const { badges, leagueClears } = normalizePoint(point)
  const settings = experiment.trainers[trainer.id]
  if (!settings) throw new Error(`Missing trainer settings: ${trainer.id}`)
  const segment = Math.min(2, Math.floor(badges / 8))
  const lower = settings.ratings[segment]
  const upper = settings.ratings[segment + 1]
  if (lower === undefined || upper === undefined)
    throw new Error(`Missing trainer rating checkpoints: ${trainer.id}`)
  const base = Math.round(lower + ((badges - segment * 8) * (upper - lower)) / 8)
  const unclampedTR = base + leagueClears * settings.leagueGrowth
  const tr = clamp(unclampedTR, 0, 80)
  const stage = settings.stages.filter((entry) => entry.minTR <= tr).at(-1)
  if (!stage) throw new Error(`Missing initial team stage: ${trainer.id}`)
  const aceLevel = resolveLevel(tr, experiment.levelAnchors)
  const cap = playerCap({ badges, leagueClears })
  const warnings: string[] = []
  if (unclampedTR > 80)
    warnings.push(`Trainer rating saturation: attempted TR ${unclampedTR}, capped at 80.`)
  const party = stage.party.map((member) => {
    const rawLevel = aceLevel + member.levelOffset
    if (rawLevel < 1 || rawLevel > 100)
      warnings.push(`${member.species} level clipped to the 1–100 range.`)
    return { ...member, level: clamp(rawLevel, 1, 100) }
  })
  if (party.some((member) => member.level > cap))
    warnings.push(`Team exceeds the player's soft level cap (${cap}); this is informational.`)
  if (aceLevel === 100)
    warnings.push("Maximum level reached; further trainer rating growth cannot raise levels.")
  return { trainer, tr, unclampedTR, aceLevel, playerCap: cap, stage: stage.label, party, warnings }
}

const fail = (message: string): never => {
  throw new Error(`Invalid experiment: ${message}`)
}
const object = (value: unknown, path: string): Record<string, unknown> => {
  if (typeof value !== "object" || value === null || Array.isArray(value))
    fail(`${path} must be an object`)
  return value as Record<string, unknown>
}
const exactKeys = (value: Record<string, unknown>, keys: string[], path: string) => {
  if (
    Object.keys(value).some((key) => !keys.includes(key)) ||
    keys.some((key) => !Object.hasOwn(value, key))
  )
    fail(`${path} has missing or unknown fields`)
}
const integer = (value: unknown, min: number, max: number, path: string): number => {
  if (typeof value !== "number" || !Number.isInteger(value) || value < min || value > max)
    fail(`${path} must be an integer from ${min} to ${max}`)
  return value as number
}
const label = (value: unknown, max: number, path: string): string => {
  if (typeof value !== "string" || !value.trim() || value.length > max)
    fail(`${path} must be a nonempty string of at most ${max} characters`)
  return value as string
}

export const validateExperiment = (value: unknown, catalog: TrainerRecord[]): Experiment => {
  const input = object(value, "root")
  exactKeys(input, ["version", "levelAnchors", "trainers"], "root")
  if (input.version !== 1) fail("version must be 1")
  if (
    !Array.isArray(input.levelAnchors) ||
    input.levelAnchors.length < 2 ||
    input.levelAnchors.length > 81
  )
    fail("levelAnchors must have 2–81 points")
  const levelAnchors: LevelAnchor[] = Array.from(
    input.levelAnchors as unknown[],
    (entry, index) => {
      if (!Array.isArray(entry) || entry.length !== 2) fail(`levelAnchors[${index}] must be a pair`)
      const pair = entry as unknown[]
      return [
        integer(pair[0], 0, 80, `levelAnchors[${index}].rating`),
        integer(pair[1], 1, 100, `levelAnchors[${index}].level`),
      ]
    },
  )
  if (levelAnchors[0]?.[0] !== 0 || levelAnchors.at(-1)?.[0] !== 80)
    fail("levelAnchors must cover TR 0 through 80")
  levelAnchors.forEach((anchor, index) => {
    const previous = levelAnchors[index - 1]
    if (index && (!previous || anchor[0] <= previous[0] || anchor[1] < previous[1]))
      fail("levelAnchors ratings must increase and levels must not decrease")
  })
  const inputTrainers = object(input.trainers, "trainers")
  const ids = catalog.map((trainer) => trainer.id)
  if (new Set(ids).size !== ids.length) fail("catalog contains duplicate trainer IDs")
  exactKeys(inputTrainers, ids, "trainers")
  const trainers: Experiment["trainers"] = Object.create(null)
  for (const id of ids) {
    const settings = object(inputTrainers[id], `trainers.${id}`)
    exactKeys(settings, ["ratings", "leagueGrowth", "stages"], `trainers.${id}`)
    if (!Array.isArray(settings.ratings) || settings.ratings.length !== 4)
      fail(`${id}.ratings must have four checkpoints`)
    const ratings = Array.from(settings.ratings as unknown[], (rating, index) =>
      integer(rating, 0, 80, `${id}.ratings[${index}]`),
    ) as [number, number, number, number]
    if (
      ratings.some((rating, index) => {
        const previous = ratings[index - 1]
        return index > 0 && (previous === undefined || rating < previous)
      })
    )
      fail(`${id}.ratings must not decrease`)
    const leagueGrowth = integer(settings.leagueGrowth, 0, 20, `${id}.leagueGrowth`)
    if (!Array.isArray(settings.stages) || !settings.stages.length || settings.stages.length > 81)
      fail(`${id}.stages must have 1–81 stages`)
    const stages: TeamStage[] = Array.from(settings.stages as unknown[], (entry, index) => {
      const stage = object(entry, `${id}.stages[${index}]`)
      exactKeys(stage, ["minTR", "label", "party"], `${id}.stages[${index}]`)
      if (!Array.isArray(stage.party) || !stage.party.length || stage.party.length > 6)
        fail(`${id}.stage party must have 1–6 members`)
      const party: PartyMember[] = Array.from(stage.party as unknown[], (entry, slot) => {
        const member = object(entry, `${id}.party[${slot}]`)
        exactKeys(member, ["species", "levelOffset"], `${id}.party[${slot}]`)
        return {
          species: label(member.species, 100, `${id}.species`),
          levelOffset: integer(member.levelOffset, -30, 0, `${id}.levelOffset`),
        }
      })
      if (!party.some((member) => member.levelOffset === 0))
        fail(`${id}.stage party needs an ace with levelOffset 0`)
      return {
        minTR: integer(stage.minTR, 0, 80, `${id}.minTR`),
        label: label(stage.label, 100, `${id}.label`),
        party,
      }
    })
    if (stages[0]?.minTR !== 0) fail(`${id}.first stage must start at TR 0`)
    stages.forEach((stage, index) => {
      const previous = stages[index - 1]
      if (index && (!previous || stage.minTR <= previous.minTR))
        fail(`${id}.stages thresholds must increase uniquely`)
      if (previous && stage.party.length < previous.party.length)
        fail(`${id}.stage party sizes must not decrease`)
    })
    trainers[id] = { ratings, leagueGrowth, stages }
  }
  return { version: 1, levelAnchors, trainers }
}

export const serializeExperiment = (experiment: Experiment): string =>
  JSON.stringify(experiment, null, 2)

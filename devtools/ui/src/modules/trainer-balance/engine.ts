import type {
  Anchor,
  Experiment,
  ResolvedTrainer,
  RosterEntry,
  TeamMember,
  TrainerRecord,
  TrainerSettings,
  WorldPoint,
} from "./types.js"

export const ROSTER_SIZE = 6
export const LEVEL_OFFSET = { min: -6, max: 0, default: -2 } as const
export const LEAGUE_FIELD = 5
export const VENUES = ["Indigo", "Sevii Masters", "Hoenn"] as const
export const MAX_ANCHORS = 20

/** The player soft-cap curve. Well-known trainer team level uses the same anchors. */
export const PLAYER_CAP_ANCHORS: readonly Anchor[] = [
  [0, 15],
  [4, 16],
  [8, 18],
  [16, 23],
  [30, 30],
  [40, 42],
  [55, 60],
  [65, 80],
  [80, 100],
]
export const DEFAULT_TEAM_LEVEL: readonly Anchor[] = PLAYER_CAP_ANCHORS
/** Step table as paired anchors: 0–10 -> 2, 11–29 -> 3, 30–41 -> 4, 42–54 -> 5, 55+ -> 6. */
export const DEFAULT_TEAM_SIZE: readonly Anchor[] = [
  [0, 2],
  [10, 2],
  [11, 3],
  [29, 3],
  [30, 4],
  [41, 4],
  [42, 5],
  [54, 5],
  [55, 6],
]

const clamp = (value: number, min: number, max: number) => Math.max(min, Math.min(max, value))
const finite = (value: number, label: string) => {
  if (!Number.isFinite(value)) throw new Error(`${label} must be finite`)
  return value
}

/**
 * Reads a scaler: linear interpolation between anchors with halves rounded up,
 * flat before the first and past the last anchor. TR is never clamped.
 */
export const scale = (anchors: readonly Anchor[], tr: number): number => {
  const rating = finite(tr, "TR")
  const first = anchors[0]
  const last = anchors.at(-1)
  if (!first || !last) throw new Error("A scaler needs at least one anchor")
  if (rating <= first[0]) return first[1]
  if (rating >= last[0]) return last[1]
  const index = anchors.findIndex(([at]) => at >= rating)
  const lower = anchors[index - 1]
  const upper = anchors[index]
  if (!lower || !upper) throw new Error("Scaler anchors must not contain missing points")
  const [lowerTR, lowerValue] = lower
  const [upperTR, upperValue] = upper
  const span = upperTR - lowerTR
  // Integer arithmetic keeps x.5 exact: floor((2n + d) / 2d) is n / d rounded half up.
  const numerator = lowerValue * span + (rating - lowerTR) * (upperValue - lowerValue)
  return Math.floor((2 * numerator + span) / (2 * span))
}

export const teamLevelFor = (experiment: Pick<Experiment, "teamLevel">, tr: number): number =>
  scale(experiment.teamLevel, tr)
export const teamSizeFor = (experiment: Pick<Experiment, "teamSize">, tr: number): number =>
  scale(experiment.teamSize, tr)

/** Clamps badges to 0–24 and first league clears to 0–3. */
export const normalizePoint = (point: WorldPoint): WorldPoint => ({
  badges: clamp(Math.round(finite(point.badges, "Badges")), 0, 24),
  leagueClears: clamp(Math.round(finite(point.leagueClears, "League clears")), 0, 3),
})

/** The player's TR (unchanged formula: badges plus first venue clears). */
export const playerRating = (point: WorldPoint): number => {
  const { badges, leagueClears } = normalizePoint(point)
  const badgeRating =
    badges <= 4 ? 4 * badges : badges <= 8 ? 16 + 6 * (badges - 4) : 40 + badges - 8
  return clamp(badgeRating + 8 * leagueClears, 0, 80)
}

/** The player soft cap. A readout for comparison: it never feeds trainer results. */
export const playerCap = (point: WorldPoint): number =>
  scale(PLAYER_CAP_ANCHORS, playerRating(point))

/**
 * Team = the first N roster entries (N = team size at TR); level =
 * clamp(team level + offset, 1, 100); battle order = the team reversed.
 */
export const buildTeam = (roster: readonly RosterEntry[], teamLevel: number, size: number) => {
  const team = roster.slice(0, size).map(
    (entry, index): TeamMember => ({
      ...entry,
      slot: index + 1,
      level: clamp(teamLevel + entry.levelOffset, 1, 100),
    }),
  )
  return { team, battleOrder: team.toReversed() }
}

/** A trainer's team from their own TR alone. The player's TR is not an input. */
export const resolveTrainer = (trainer: TrainerRecord, experiment: Experiment): ResolvedTrainer => {
  const settings = experiment.trainers[trainer.id]
  if (!settings) throw new Error(`Missing trainer settings: ${trainer.id}`)
  const teamLevel = teamLevelFor(experiment, settings.tr)
  const size = teamSizeFor(experiment, settings.tr)
  const { team, battleOrder } = buildTeam(settings.roster, teamLevel, size)
  const warnings: string[] = []
  if (settings.roster.length < ROSTER_SIZE)
    warnings.push(
      `The roster lists ${settings.roster.length} of ${ROSTER_SIZE} entries: add ${ROSTER_SIZE - settings.roster.length} more.`,
    )
  if (team.length < size) warnings.push(`The team fills ${team.length} of ${size} slots.`)
  for (const member of team)
    if (teamLevel + member.levelOffset < 1) warnings.push(`${member.species} level clipped to 1.`)
  return {
    trainer,
    tr: settings.tr,
    teamLevel,
    size,
    team,
    battleOrder,
    rosterLength: settings.roster.length,
    warnings,
  }
}

/**
 * The league field from one global pool: the top five by TR (ties keep
 * catalog order), returned in battle order, ascending TR with the strongest last.
 */
export const leagueField = (catalog: TrainerRecord[], experiment: Experiment): ResolvedTrainer[] =>
  catalog
    .map((trainer) => resolveTrainer(trainer, experiment))
    .toSorted((a, b) => b.tr - a.tr)
    .slice(0, LEAGUE_FIELD)
    .toSorted((a, b) => a.tr - b.tr)

export const defaultTrainerSettings = (trainer: TrainerRecord): TrainerSettings => ({
  tr: trainer.tr,
  roster: structuredClone(trainer.roster),
})

export const createExperiment = (catalog: TrainerRecord[]): Experiment => {
  const trainers: Experiment["trainers"] = Object.create(null)
  for (const trainer of catalog) trainers[trainer.id] = defaultTrainerSettings(trainer)
  return validateExperiment(
    {
      version: 5,
      teamLevel: structuredClone(DEFAULT_TEAM_LEVEL),
      teamSize: structuredClone(DEFAULT_TEAM_SIZE),
      trainers,
    },
    catalog,
  )
}

const fail = (message: string): never => {
  throw new Error(`Invalid experiment: ${message}`)
}
const object = (value: unknown, path: string): Record<string, unknown> => {
  if (typeof value !== "object" || value === null || Array.isArray(value))
    fail(`${path} must be an object`)
  return value as Record<string, unknown>
}
const exactKeys = (value: Record<string, unknown>, keys: readonly string[], path: string) => {
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
const tr = (value: unknown, path: string): number => {
  if (typeof value !== "number" || !Number.isSafeInteger(value) || value < 0)
    fail(`${path} must be a non-negative whole number`)
  return value as number
}
const label = (value: unknown, max: number, path: string): string => {
  if (typeof value !== "string" || !value.trim() || value.length > max)
    fail(`${path} must be a nonempty string of at most ${max} characters`)
  return value as string
}
const nullable = (value: unknown, max: number, path: string): string | null =>
  value === null ? null : label(value, max, path)

/** Anchors start at TR 0, rise strictly in TR, and never decrease in value. */
const anchors = (value: unknown, min: number, max: number, path: string): Anchor[] => {
  if (!Array.isArray(value) || value.length < 1 || value.length > MAX_ANCHORS)
    fail(`${path} must list 1–${MAX_ANCHORS} anchors`)
  const points = (value as unknown[]).map((entry, index): Anchor => {
    const where = `${path}[${index}]`
    if (!Array.isArray(entry) || entry.length !== 2) fail(`${where} must be [TR, value]`)
    const [at, result] = entry as unknown[]
    return [tr(at, `${where} TR`), integer(result, min, max, `${where} value`)]
  })
  if (points[0]?.[0] !== 0) fail(`${path} must start at TR 0`)
  points.forEach((point, index) => {
    const previous = points[index - 1]
    if (previous && point[0] <= previous[0]) fail(`${path} TRs must increase`)
    if (previous && point[1] < previous[1]) fail(`${path} values must not decrease`)
  })
  return points
}

const rosterEntry = (value: unknown, path: string): RosterEntry => {
  const entry = object(value, path)
  exactKeys(entry, ["species", "levelOffset", "moves", "item", "ability", "nature"], path)
  const moves =
    entry.moves === "LEVEL_UP"
      ? "LEVEL_UP"
      : Array.isArray(entry.moves) && entry.moves.length >= 1 && entry.moves.length <= 4
        ? entry.moves.map((move: unknown, slot) => label(move, 40, `${path}.moves[${slot}]`))
        : fail(`${path}.moves must be "LEVEL_UP" or 1–4 moves`)
  return {
    species: label(entry.species, 100, `${path}.species`),
    levelOffset: integer(
      entry.levelOffset,
      LEVEL_OFFSET.min,
      LEVEL_OFFSET.max,
      `${path}.levelOffset`,
    ),
    moves,
    item: nullable(entry.item, 60, `${path}.item`),
    ability: nullable(entry.ability, 60, `${path}.ability`),
    nature: nullable(entry.nature, 30, `${path}.nature`),
  }
}

/**
 * v0 rosters list up to six entries, and entry 1 is at offset 0 so the
 * signature Pokémon plays at the team level at every size. Fewer than six is
 * a content gap the explorer flags (see rosterGaps), not an import error.
 */
export const validateRoster = (value: unknown, path: string): RosterEntry[] => {
  if (!Array.isArray(value) || value.length < 1 || value.length > ROSTER_SIZE)
    fail(`${path} must list 1–${ROSTER_SIZE} entries (v0 requires ${ROSTER_SIZE})`)
  const entries = (value as unknown[]).map((entry, index) =>
    rosterEntry(entry, `${path}[${index + 1}]`),
  )
  if (entries[0]?.levelOffset !== 0) fail(`${path}: entry 1 must have level offset 0`)
  return entries
}

/** Rosters short of the required six. */
export const rosterGaps = (
  catalog: TrainerRecord[],
  experiment: Experiment,
): { id: string; name: string; length: number }[] =>
  catalog.flatMap((trainer) => {
    const length = experiment.trainers[trainer.id]?.roster.length ?? 0
    return length < ROSTER_SIZE ? [{ id: trainer.id, name: trainer.name, length }] : []
  })

export const OLD_VERSION_REJECTION = (version: number): string =>
  `Version ${version} experiments use a retired trainer model (standing, arcs or aces and fillers) and cannot be imported. Start from the version 5 defaults.`

export const validateExperiment = (value: unknown, catalog: TrainerRecord[]): Experiment => {
  const input = object(value, "root")
  if (typeof input.version === "number" && input.version >= 1 && input.version <= 4)
    fail(OLD_VERSION_REJECTION(input.version))
  if (input.version !== 5) fail("version must be 5")
  exactKeys(input, ["version", "teamLevel", "teamSize", "trainers"], "root")
  const inputTrainers = object(input.trainers, "trainers")
  const ids = catalog.map((trainer) => trainer.id)
  if (new Set(ids).size !== ids.length) fail("catalog contains duplicate trainer IDs")
  exactKeys(inputTrainers, ids, "trainers")
  const trainers: Experiment["trainers"] = Object.create(null)
  for (const id of ids) {
    const settings = object(inputTrainers[id], `trainers.${id}`)
    exactKeys(settings, ["tr", "roster"], `trainers.${id}`)
    trainers[id] = {
      tr: tr(settings.tr, `${id}.tr`),
      roster: validateRoster(settings.roster, `${id}.roster`),
    }
  }
  return {
    version: 5,
    teamLevel: anchors(input.teamLevel, 1, 100, "teamLevel"),
    teamSize: anchors(input.teamSize, 1, ROSTER_SIZE, "teamSize"),
    trainers,
  }
}

export const serializeExperiment = (experiment: Experiment): string =>
  JSON.stringify(experiment, null, 2)

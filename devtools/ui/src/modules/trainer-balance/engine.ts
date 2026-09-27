import type {
  AceForm,
  ArcId,
  ArcTuple,
  Experiment,
  FillerScore,
  LineStage,
  Modifier,
  ResolvedTrainer,
  Role,
  RoleFeasibility,
  RoleWindows,
  Roster,
  RosterAce,
  RosterContext,
  RosterFiller,
  RosterGap,
  SizeStep,
  TeamMember,
  TrainerRecord,
  TrainerSettings,
  VenueFeasibility,
  VenueId,
  WorldPoint,
} from "./types.js"

type LevelAnchor = [number, number]

/** Canonical arc order: the index is the arc id, so allowedArcs sort by it. */
export const ARC_IDS: readonly ArcId[] = ["steady", "early", "late", "plateau", "rival"]
/** Arc checkpoints on the progress index p = B + 8 * min(completedEditions, 3). */
export const ARC_CHECKPOINTS = [0, 8, 16, 24, 32, 40, 48] as const
export const MAX_PROGRESS = 48
export const MAX_EDITIONS = 3
export const DEFAULT_ARCS: Record<ArcId, ArcTuple> = {
  steady: [0, 0, 0, 0, 0, 0, 0],
  early: [0, 3, 2, 1, 0, 0, 0],
  late: [0, -2, 0, 3, 3, 2, 2],
  plateau: [0, 1, -1, -3, -3, -2, -1],
  rival: [0, 2, 3, 3, 3, 3, 3],
}
/** levelBase = min(worldCap, 100 - headroom). */
export const DEFAULT_HEADROOM = 4
export const MAX_HEADROOM = 20
export const DEFAULT_ROLE_WINDOWS: RoleWindows = { contenderMax: -2, headlinerMin: 2 }
export const ROLES: readonly Role[] = ["contender", "elite", "headliner"]
/** Slots per venue: two contenders, two elites, one headliner. */
export const ROLE_NEED: Record<Role, number> = { contender: 2, elite: 2, headliner: 1 }
export const VENUES: readonly { venue: VenueId; pool: "home" | "open" }[] = [
  { venue: "Indigo", pool: "home" },
  { venue: "Hoenn", pool: "home" },
  { venue: "Sevii Masters", pool: "open" },
]
/** sizeFor(strengthLevel) defaults: Lv <20 -> 2, >=20 -> 3, >=30 -> 4, >=45 -> 5, >=60 -> 6. */
export const DEFAULT_SIZE_TABLE: readonly SizeStep[] = [
  { minLevel: 1, size: 2 },
  { minLevel: 20, size: 3 },
  { minLevel: 30, size: 4 },
  { minLevel: 45, size: 5 },
  { minLevel: 60, size: 6 },
]
/** Filler jitter is uniform over 0..JITTER. */
export const DEFAULT_JITTER = 30
export const MAX_JITTER = 100
export const MAX_ACES = 3
export const MAX_FILLERS = 20
export const MAX_SEED = 0xffffffff
export const DEFAULT_CONTEXT: RosterContext = { seed: 1, flags: [] }
export const FILLER_OFFSET = { min: -6, max: 0, default: -2 } as const

// The player's soft-cap curve. The world cap reuses it unchanged.
const PLAYER_ANCHORS: LevelAnchor[] = [
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
const clamp = (value: number, min: number, max: number) => Math.max(min, Math.min(max, value))
/** Half-up integer rounding (toward +infinity on ties), without negative zero. */
const roundHalfUp = (value: number) => Math.floor(value + 0.5) + 0
const finite = (value: number, label: string) => {
  if (!Number.isFinite(value)) throw new Error(`${label} must be finite`)
  return value
}
type NormalPoint = Required<WorldPoint>
/**
 * Clamps badges to 0–24, clears to 0–3 and completed editions to 0–3 (3 means
 * "3 or more"). A completed edition implies 24 badges and 3 first clears, so
 * editions only count at that point and are 0 everywhere else.
 */
export const normalizePoint = (point: WorldPoint): NormalPoint => {
  const badges = clamp(Math.round(finite(point.badges, "Badges")), 0, 24)
  const leagueClears = clamp(Math.round(finite(point.leagueClears, "League clears")), 0, 3)
  const editions = clamp(
    Math.round(finite(point.completedEditions ?? 0, "Completed editions")),
    0,
    MAX_EDITIONS,
  )
  return {
    badges,
    leagueClears,
    completedEditions: badges === 24 && leagueClears === 3 ? editions : 0,
  }
}

/** Whether completed editions can be non-zero at this point (B = 24 and C = 3). */
export const editionsAvailable = (point: WorldPoint): boolean => {
  const { badges, leagueClears } = normalizePoint(point)
  return badges === 24 && leagueClears === 3
}

/** The arc input: p = B + 8 * min(completedEditions, 3), from 0 to 48. */
export const progressIndex = (point: WorldPoint): number => {
  const { badges, completedEditions } = normalizePoint(point)
  return badges + 8 * completedEditions
}

/** Positive, half-up interpolation, matching the ROM's integer cap rounding. */
export const resolveLevel = (tr: number, anchors: LevelAnchor[]): number => {
  const rating = clamp(finite(tr, "Trainer rating"), 0, 80)
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

/** The player soft cap computed from canonical milestones (B, C) alone. */
export const worldCap = (point: WorldPoint): number => playerCap(point)

/** min(worldCap, 100 - headroom): standing is added to this base. */
export const levelBase = (point: WorldPoint, headroom: number): number =>
  Math.min(worldCap(point), 100 - headroom)

/** Linear interpolation of an arc tuple across p = 0/8/…/48, rounded half up. */
export const arcDelta = (tuple: ArcTuple, progress: number): number => {
  const p = clamp(Math.round(finite(progress, "Progress")), 0, MAX_PROGRESS)
  const segment = Math.min(ARC_CHECKPOINTS.length - 2, Math.floor(p / 8))
  const lower = tuple[segment]
  const upper = tuple[segment + 1]
  if (lower === undefined || upper === undefined)
    throw new Error(`Arc tuples need ${ARC_CHECKPOINTS.length} values`)
  return roundHalfUp(lower + ((p - segment * 8) * (upper - lower)) / 8)
}

export const standingAt = (
  settings: Pick<TrainerSettings, "bias">,
  experiment: Pick<Experiment, "arcs">,
  arc: ArcId,
  progress: number,
): number => settings.bias + arcDelta(experiment.arcs[arc], progress)

export const classifyRole = (standing: number, windows: RoleWindows): Role =>
  standing <= windows.contenderMax
    ? "contender"
    : standing >= windows.headlinerMin
      ? "headliner"
      : "elite"

export const sortArcs = (arcs: ArcId[]): ArcId[] =>
  [...new Set(arcs)].toSorted((a, b) => ARC_IDS.indexOf(a) - ARC_IDS.indexOf(b))

const defaultArc = (allowed: ArcId[]): ArcId =>
  allowed.includes("steady") ? "steady" : (allowed[0] ?? "steady")

export const defaultTrainerSettings = (trainer: TrainerRecord): TrainerSettings => {
  const allowedArcs = sortArcs(trainer.allowedArcs)
  return {
    bias: trainer.bias,
    allowedArcs,
    arc: defaultArc(allowedArcs),
    roster: structuredClone(trainer.roster),
  }
}

export const createExperiment = (catalog: TrainerRecord[]): Experiment => {
  const trainers: Experiment["trainers"] = Object.create(null)
  for (const trainer of catalog) trainers[trainer.id] = defaultTrainerSettings(trainer)
  return validateExperiment(
    {
      version: 4,
      arcs: structuredClone(DEFAULT_ARCS),
      roleWindows: { ...DEFAULT_ROLE_WINDOWS },
      headroom: DEFAULT_HEADROOM,
      sizeTable: structuredClone(DEFAULT_SIZE_TABLE),
      jitter: DEFAULT_JITTER,
      modifiers: [],
      trainers,
    },
    catalog,
  )
}

/** sizeFor(strength level): the last row whose minLevel is at most the strength level. */
export const sizeFor = (table: readonly SizeStep[], strengthLevel: number): number =>
  table.findLast((step) => step.minLevel <= strengthLevel)?.size ?? table[0]?.size ?? 1

/**
 * The maximum number of aces at a size: 1–3 members -> 1, 4–5 -> 2, 6 -> 3.
 * Unused ace slots go to fillers, so a single-ace roster still reaches 6.
 */
export const aceAllowance = (size: number): number => (size >= 6 ? 3 : size >= 4 ? 2 : 1)

/**
 * Explorer jitter: FNV-1a over seed, trainer and filler, uniform over
 * 0..max. The ROM draws Uniform(JITTER + 1) from the seed framework key
 * (TRAINER_ROSTER / FILLER_JITTER, entityLo = characterId, entityHi = fillerId),
 * so explorer values are stable per seed but do not match ROM values.
 */
export const fillerJitter = (
  seed: number,
  trainerId: string,
  fillerId: string,
  max: number,
): number => {
  let hash = 0x811c9dc5
  for (const char of `${seed}|${trainerId}|${fillerId}`) {
    hash ^= char.codePointAt(0) ?? 0
    hash = Math.imul(hash, 0x01000193) >>> 0
  }
  return hash % (max + 1)
}

/** The species a line has reached at `level`. */
export const formAt = <T extends LineStage>(line: readonly T[], level: number): T => {
  const form = line.findLast((stage) => stage.level <= level) ?? line[0]
  if (!form) throw new Error("An evolution line needs at least one species")
  return form
}

export const modifierDelta = (
  modifiers: readonly Modifier[],
  flags: readonly string[],
  trainerId: string,
  fillerId: string,
): number =>
  modifiers
    .filter((m) => m.trainer === trainerId && m.filler === fillerId && flags.includes(m.flag))
    .reduce((sum, m) => sum + m.delta, 0)

const byId = (a: string, b: string) => (a < b ? -1 : a > b ? 1 : 0)

/**
 * Scores every filler (baseScore + jitter + active modifiers) and ranks the
 * eligible ones by score, then fillerId. Scores never depend on size, so the
 * top-K is contained in the top-(K + 1).
 */
export const scoreFillers = (
  trainerId: string,
  roster: Roster,
  experiment: Pick<Experiment, "jitter" | "modifiers">,
  context: RosterContext,
): FillerScore[] => {
  const scored = roster.fillers.map((filler): FillerScore => {
    const jitter = fillerJitter(context.seed, trainerId, filler.id, experiment.jitter)
    const modifier = modifierDelta(experiment.modifiers, context.flags, trainerId, filler.id)
    return {
      filler,
      eligible: filler.requiresFlag === null || context.flags.includes(filler.requiresFlag),
      jitter,
      modifier,
      score: filler.baseScore + jitter + modifier,
      rank: null,
      inTeam: false,
    }
  })
  scored
    .filter((entry) => entry.eligible)
    .toSorted((a, b) => b.score - a.score || byId(a.filler.id, b.filler.id))
    .forEach((entry, index) => {
      entry.rank = index + 1
    })
  return scored
}

/** Members at size 6 (allowance 3): aces used plus unlocked fillers, at most 6. */
export const rosterReach = (roster: Roster, flags: readonly string[]): number => {
  const aces = Math.min(aceAllowance(6), roster.aces.length)
  const fillers = roster.fillers.filter(
    (filler) => filler.requiresFlag === null || flags.includes(filler.requiresFlag),
  ).length
  return aces + Math.min(fillers, 6 - aces)
}

const aceMoves = (form: AceForm) => (form.moves.length ? form.moves.join(" · ") : "Unauthored")
const fillerMoves = (filler: RosterFiller) =>
  filler.signatureMove ? `LEVEL_UP + ${filler.signatureMove}` : "LEVEL_UP"

/**
 * Composes the team at a strength level: aces first in priority order, up to
 * min(allowance, aces), then the top fillers by score for every other slot. Battle order is fillers by
 * ascending score, then aces in reverse priority (the top ace last).
 */
export const composeTeam = (
  trainerId: string,
  roster: Roster,
  experiment: Pick<Experiment, "jitter" | "modifiers" | "sizeTable">,
  strengthLevel: number,
  context: RosterContext,
) => {
  const size = sizeFor(experiment.sizeTable, strengthLevel)
  const allowance = aceAllowance(size)
  const aces: RosterAce[] = roster.aces.slice(0, Math.min(allowance, size))
  const fillerScores = scoreFillers(trainerId, roster, experiment, context)
  const picked = fillerScores
    .filter((entry) => entry.rank !== null && entry.rank <= size - aces.length)
    .toSorted((a, b) => (b.rank ?? 0) - (a.rank ?? 0))
  for (const entry of picked) entry.inTeam = true
  const party: TeamMember[] = [
    ...picked.map(({ filler, score }): TeamMember => {
      const level = clamp(strengthLevel + filler.levelOffset, 1, 100)
      return {
        kind: "filler",
        id: filler.id,
        species: formAt(filler.line, level).species,
        level,
        levelOffset: filler.levelOffset,
        score,
        moves: fillerMoves(filler),
      }
    }),
    ...aces
      .map((ace, index): TeamMember => {
        const form = formAt(ace.line, strengthLevel)
        return {
          kind: "ace",
          id: ace.id,
          species: form.species,
          level: strengthLevel,
          levelOffset: 0,
          priority: index + 1,
          moves: aceMoves(form),
        }
      })
      .toReversed(),
  ]
  return { size, allowance, acesUsed: aces.length, fillerScores, party }
}

const strengthAt = (
  settings: TrainerSettings,
  experiment: Experiment,
  arc: ArcId,
  point: WorldPoint,
) =>
  clamp(
    levelBase(point, experiment.headroom) +
      standingAt(settings, experiment, arc, progressIndex(point)),
    1,
    100,
  )

/** The previous step on the progress index: one badge back, or one edition back. */
const previousPoint = (point: NormalPoint): NormalPoint | null =>
  point.completedEditions > 0
    ? { ...point, completedEditions: point.completedEditions - 1 }
    : point.badges > 0
      ? { ...point, badges: point.badges - 1 }
      : null

export const resolveTrainer = (
  trainer: TrainerRecord,
  experiment: Experiment,
  point: WorldPoint,
  arcOverride?: ArcId,
  context: RosterContext = DEFAULT_CONTEXT,
): ResolvedTrainer => {
  const world = normalizePoint(point)
  const settings = experiment.trainers[trainer.id]
  if (!settings) throw new Error(`Missing trainer settings: ${trainer.id}`)
  const arc = arcOverride ?? settings.arc
  const progress = progressIndex(world)
  const standing = standingAt(settings, experiment, arc, progress)
  const cap = worldCap(world)
  const base = levelBase(world, experiment.headroom)
  const rawStrength = base + standing
  const strengthLevel = clamp(rawStrength, 1, 100)
  const team = composeTeam(trainer.id, settings.roster, experiment, strengthLevel, context)
  const warnings: string[] = []
  if (rawStrength > 100) warnings.push(`Strength level ${rawStrength} clamped to 100.`)
  if (rawStrength < 1) warnings.push(`Strength level ${rawStrength} clamped to 1.`)
  for (const member of team.party)
    if (member.kind === "filler" && strengthLevel + member.levelOffset < 1)
      warnings.push(`${member.species} level clipped to 1.`)
  if (team.party.length < team.size)
    warnings.push(
      `The roster fills ${team.party.length} of ${team.size} slots: add fillers or unlock flagged ones.`,
    )
  const before = previousPoint(world)
  if (before && strengthAt(settings, experiment, arc, before) > strengthLevel)
    warnings.push("The strength level dropped since the previous progress step.")
  return {
    trainer,
    arc,
    progress,
    standing,
    strengthLevel,
    worldCap: cap,
    levelBase: base,
    gap: strengthLevel - cap,
    role: classifyRole(standing, experiment.roleWindows),
    ...team,
    warnings,
  }
}

/** Rosters that cannot reach six members at max size under the given flags. */
export const rosterGaps = (
  catalog: TrainerRecord[],
  experiment: Experiment,
  flags: readonly string[] = [],
): RosterGap[] =>
  catalog.flatMap((trainer) => {
    const roster = experiment.trainers[trainer.id]?.roster
    if (!roster) throw new Error(`Missing trainer settings: ${trainer.id}`)
    const reach = rosterReach(roster, flags)
    return reach < 6 ? [{ id: trainer.id, name: trainer.name, reach }] : []
  })

/**
 * Per venue and role at progress index p: candidates in-window under every
 * allowed arc (guaranteed), under some allowed arc (possible), and under the
 * currently selected arc. Every roster is a candidate (sizes follow the ace
 * level); rosters that cannot reach six at max size are listed as gaps.
 * Standing depends on p only, not on league clears or the level headroom.
 */
export const venueFeasibility = (
  catalog: TrainerRecord[],
  experiment: Experiment,
  progress: number,
  flags: readonly string[] = [],
): VenueFeasibility[] => {
  const gaps = rosterGaps(catalog, experiment, flags)
  return VENUES.map(({ venue, pool }) => {
    const candidates = catalog.filter(
      (trainer) => pool === "open" || trainer.homeLeagues.includes(venue),
    )
    const roles = candidates.map((trainer) => {
      const settings = experiment.trainers[trainer.id]
      if (!settings) throw new Error(`Missing trainer settings: ${trainer.id}`)
      const role = (arc: ArcId) =>
        classifyRole(standingAt(settings, experiment, arc, progress), experiment.roleWindows)
      return { all: settings.allowedArcs.map(role), current: role(settings.arc) }
    })
    return {
      venue,
      pool,
      candidates: candidates.length,
      gaps: gaps.filter((gap) => candidates.some((trainer) => trainer.id === gap.id)),
      roles: ROLES.map((role): RoleFeasibility => {
        const guaranteed = roles.filter((entry) => entry.all.every((r) => r === role)).length
        return {
          role,
          need: ROLE_NEED[role],
          guaranteed,
          possible: roles.filter((entry) => entry.all.includes(role)).length,
          current: roles.filter((entry) => entry.current === role).length,
          fallbackRisk: guaranteed < ROLE_NEED[role],
        }
      }),
    }
  })
}

export type GymCapSummary = {
  count: number
  mean: number
  min: number
  max: number
  /** Extremes across every allowed arc, not only the selected ones. */
  arcMin: number
  arcMax: number
  /**
   * Blue is "ahead" when every allowed arc puts his ace above levelBase. Below
   * the ceiling (worldCap <= 100 - headroom) levelBase is the cap, so this is
   * the strength − cap > 0 check; at the ceiling it is strength > levelBase.
   */
  blue: {
    gaps: { arc: ArcId; gap: number; baseGap: number }[]
    ahead: boolean
    atCeiling: boolean
    levelBase: number
  } | null
}

export const gymCapSummary = (
  catalog: TrainerRecord[],
  experiment: Experiment,
  point: WorldPoint,
): GymCapSummary => {
  const leaders = catalog.filter((trainer) => trainer.role === "Gym Leader")
  const gaps = leaders.map((trainer) => resolveTrainer(trainer, experiment, point).gap)
  const arcGaps = leaders.flatMap((trainer) =>
    (experiment.trainers[trainer.id]?.allowedArcs ?? []).map(
      (arc) => resolveTrainer(trainer, experiment, point, arc).gap,
    ),
  )
  const blueTrainer = catalog.find((trainer) => trainer.id === "blue")
  const base = levelBase(normalizePoint(point), experiment.headroom)
  const blueGaps = blueTrainer
    ? (experiment.trainers.blue?.allowedArcs ?? []).map((arc) => {
        const resolved = resolveTrainer(blueTrainer, experiment, point, arc)
        return { arc, gap: resolved.gap, baseGap: resolved.strengthLevel - resolved.levelBase }
      })
    : []
  return {
    count: leaders.length,
    mean: gaps.length ? gaps.reduce((sum, gap) => sum + gap, 0) / gaps.length : 0,
    min: Math.min(...gaps),
    max: Math.max(...gaps),
    arcMin: Math.min(...arcGaps),
    arcMax: Math.max(...arcGaps),
    blue: blueTrainer
      ? {
          gaps: blueGaps,
          ahead: blueGaps.every(({ baseGap }) => baseGap > 0),
          atCeiling: worldCap(point) > base,
          levelBase: base,
        }
      : null,
  }
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
const label = (value: unknown, max: number, path: string): string => {
  if (typeof value !== "string" || !value.trim() || value.length > max)
    fail(`${path} must be a nonempty string of at most ${max} characters`)
  return value as string
}
const arcId = (value: unknown, path: string): ArcId => {
  if (typeof value !== "string" || !(ARC_IDS as readonly string[]).includes(value))
    fail(`${path} must be one of ${ARC_IDS.join(", ")}`)
  return value as ArcId
}

const nullable = (value: unknown, max: number, path: string): string | null =>
  value === null ? null : label(value, max, path)
const flagName = (value: unknown, path: string): string => {
  const flag = label(value, 60, path)
  if (flag.trim() !== flag) fail(`${path} must not start or end with spaces`)
  return flag
}
const ID_PATTERN = /^[a-z0-9-]{1,40}$/
const memberId = (value: unknown, path: string): string => {
  if (typeof value !== "string" || !ID_PATTERN.test(value))
    fail(`${path} must be a lowercase slug (a-z, 0-9, -) of at most 40 characters`)
  return value as string
}
const list = (value: unknown, min: number, max: number, path: string): unknown[] => {
  if (!Array.isArray(value) || value.length < min || value.length > max)
    fail(`${path} must list ${min}–${max} entries`)
  return value as unknown[]
}
/** Evolution lines start at level 1, evolve at increasing levels and never repeat a species. */
const line = <T extends LineStage>(
  value: unknown,
  path: string,
  read: (stage: Record<string, unknown>, where: string, base: LineStage) => T,
  keys: readonly string[],
): T[] => {
  const stages = list(value, 1, 6, `${path}.line`).map((entry, index) => {
    const where = `${path}.line[${index}]`
    const stage = object(entry, where)
    exactKeys(stage, keys, where)
    return read(stage, where, {
      species: label(stage.species, 100, `${where}.species`),
      level: integer(stage.level, 1, 100, `${where}.level`),
    })
  })
  if (stages[0]?.level !== 1) fail(`${path}: the first species of a line starts at level 1`)
  stages.forEach((stage, index) => {
    const previous = stages[index - 1]
    if (previous && stage.level <= previous.level)
      fail(`${path}: evolve levels must increase along the line`)
  })
  if (new Set(stages.map((stage) => stage.species)).size !== stages.length)
    fail(`${path}: a line must not repeat a species`)
  return stages
}
const ACE_FORM_KEYS = ["species", "level", "moves", "item", "ability", "nature"] as const
const FILLER_KEYS = [
  "id",
  "line",
  "baseScore",
  "levelOffset",
  "moves",
  "signatureMove",
  "requiresFlag",
] as const

const roster = (value: unknown, path: string): Roster => {
  const input = object(value, path)
  exactKeys(input, ["prototype", "aces", "fillers"], path)
  if (typeof input.prototype !== "boolean") fail(`${path}.prototype must be true or false`)
  const aces = list(input.aces, 1, MAX_ACES, `${path}.aces`).map((entry, index): RosterAce => {
    const where = `${path}.aces[${index}]`
    const ace = object(entry, where)
    exactKeys(ace, ["id", "line"], where)
    return {
      id: memberId(ace.id, `${where}.id`),
      line: line(
        ace.line,
        where,
        (stage, at, base): AceForm => ({
          ...base,
          moves: list(stage.moves, 0, 4, `${at}.moves`).map((move, slot) =>
            label(move, 40, `${at}.moves[${slot}]`),
          ),
          item: nullable(stage.item, 60, `${at}.item`),
          ability: nullable(stage.ability, 60, `${at}.ability`),
          nature: nullable(stage.nature, 30, `${at}.nature`),
        }),
        ACE_FORM_KEYS,
      ),
    }
  })
  const fillers = list(input.fillers, 0, MAX_FILLERS, `${path}.fillers`).map(
    (entry, index): RosterFiller => {
      const where = `${path}.fillers[${index}]`
      const filler = object(entry, where)
      exactKeys(filler, FILLER_KEYS, where)
      if (filler.moves !== "LEVEL_UP") fail(`${where}.moves must be "LEVEL_UP"`)
      return {
        id: memberId(filler.id, `${where}.id`),
        line: line(filler.line, where, (_stage, _at, base) => base, ["species", "level"]),
        baseScore: integer(filler.baseScore, 0, 100, `${where}.baseScore`),
        levelOffset: integer(
          filler.levelOffset,
          FILLER_OFFSET.min,
          FILLER_OFFSET.max,
          `${where}.levelOffset`,
        ),
        moves: "LEVEL_UP",
        signatureMove: nullable(filler.signatureMove, 40, `${where}.signatureMove`),
        requiresFlag:
          filler.requiresFlag === null
            ? null
            : flagName(filler.requiresFlag, `${where}.requiresFlag`),
      }
    },
  )
  const ids = [...aces, ...fillers].map((entry) => entry.id)
  if (new Set(ids).size !== ids.length) fail(`${path}: ace and filler IDs must be unique`)
  return { prototype: input.prototype as boolean, aces, fillers }
}

const sizeTable = (value: unknown): SizeStep[] => {
  const steps = list(value, 1, 10, "sizeTable").map((entry, index): SizeStep => {
    const step = object(entry, `sizeTable[${index}]`)
    exactKeys(step, ["minLevel", "size"], `sizeTable[${index}]`)
    return {
      minLevel: integer(step.minLevel, 1, 100, `sizeTable[${index}].minLevel`),
      size: integer(step.size, 1, 6, `sizeTable[${index}].size`),
    }
  })
  if (steps[0]?.minLevel !== 1) fail("sizeTable must start at level 1")
  steps.forEach((step, index) => {
    const previous = steps[index - 1]
    if (previous && step.minLevel <= previous.minLevel)
      fail("sizeTable levels must increase uniquely")
    if (previous && step.size < previous.size)
      fail("sizeTable sizes must not decrease, so growth never removes a member")
  })
  return steps
}

export const V1_REJECTION =
  "Version 1 experiments use the retired trainer-rating model and cannot be imported. Start from the version 4 defaults."
export const V2_REJECTION =
  "Version 2 experiments predate post-game arcs (p = 0–48) and level headroom and cannot be imported. Start from the version 4 defaults."
export const V3_REJECTION =
  "Version 3 experiments use badge-keyed team stages, which rosters of aces and fillers replace, and cannot be imported. Start from the version 4 defaults."

export const validateExperiment = (value: unknown, catalog: TrainerRecord[]): Experiment => {
  const input = object(value, "root")
  if (input.version === 1) fail(V1_REJECTION)
  if (input.version === 2) fail(V2_REJECTION)
  if (input.version === 3) fail(V3_REJECTION)
  if (input.version !== 4) fail("version must be 4")
  exactKeys(
    input,
    ["version", "arcs", "roleWindows", "headroom", "sizeTable", "jitter", "modifiers", "trainers"],
    "root",
  )

  const inputArcs = object(input.arcs, "arcs")
  exactKeys(inputArcs, ARC_IDS, "arcs")
  const arcs = Object.create(null) as Record<ArcId, ArcTuple>
  for (const arc of ARC_IDS) {
    const tuple = inputArcs[arc]
    if (!Array.isArray(tuple) || tuple.length !== ARC_CHECKPOINTS.length)
      fail(
        `arcs.${arc} must have ${ARC_CHECKPOINTS.length} values (p = ${ARC_CHECKPOINTS.join("/")})`,
      )
    arcs[arc] = Array.from(tuple as unknown[], (delta, index) =>
      integer(delta, -12, 12, `arcs.${arc}[p = ${ARC_CHECKPOINTS[index]}]`),
    ) as ArcTuple
    if (arcs[arc][0] !== 0) fail(`arcs.${arc} must be 0 at p = 0`)
  }
  const headroom = integer(input.headroom, 0, MAX_HEADROOM, "headroom")

  const windows = object(input.roleWindows, "roleWindows")
  exactKeys(windows, ["contenderMax", "headlinerMin"], "roleWindows")
  const roleWindows: RoleWindows = {
    contenderMax: integer(windows.contenderMax, -12, 12, "roleWindows.contenderMax"),
    headlinerMin: integer(windows.headlinerMin, -12, 12, "roleWindows.headlinerMin"),
  }
  if (roleWindows.headlinerMin - roleWindows.contenderMax < 2)
    fail("roleWindows must leave at least one elite standing between contender and headliner")

  const inputTrainers = object(input.trainers, "trainers")
  const ids = catalog.map((trainer) => trainer.id)
  if (new Set(ids).size !== ids.length) fail("catalog contains duplicate trainer IDs")
  exactKeys(inputTrainers, ids, "trainers")
  const trainers: Experiment["trainers"] = Object.create(null)
  for (const id of ids) {
    const settings = object(inputTrainers[id], `trainers.${id}`)
    exactKeys(settings, ["bias", "allowedArcs", "arc", "roster"], `trainers.${id}`)
    const bias = integer(settings.bias, -6, 6, `${id}.bias`)
    if (
      !Array.isArray(settings.allowedArcs) ||
      !settings.allowedArcs.length ||
      settings.allowedArcs.length > ARC_IDS.length
    )
      fail(`${id}.allowedArcs must list 1–${ARC_IDS.length} arcs`)
    const listed = Array.from(settings.allowedArcs as unknown[], (arc, index) =>
      arcId(arc, `${id}.allowedArcs[${index}]`),
    )
    const allowedArcs = sortArcs(listed)
    if (allowedArcs.length !== listed.length) fail(`${id}.allowedArcs must not repeat arcs`)
    const arc = arcId(settings.arc, `${id}.arc`)
    if (!allowedArcs.includes(arc)) fail(`${id}.arc must be one of its allowed arcs`)
    trainers[id] = { bias, allowedArcs, arc, roster: roster(settings.roster, `${id}.roster`) }
  }

  const modifiers = list(input.modifiers, 0, 200, "modifiers").map((entry, index): Modifier => {
    const where = `modifiers[${index}]`
    const modifier = object(entry, where)
    exactKeys(modifier, ["flag", "trainer", "filler", "delta"], where)
    const trainer = label(modifier.trainer, 40, `${where}.trainer`)
    const filler = label(modifier.filler, 40, `${where}.filler`)
    if (!trainers[trainer]?.roster.fillers.some((f) => f.id === filler))
      fail(`${where} must name a filler in that trainer's roster`)
    return {
      flag: flagName(modifier.flag, `${where}.flag`),
      trainer,
      filler,
      delta: integer(modifier.delta, -100, 100, `${where}.delta`),
    }
  })

  return {
    version: 4,
    arcs,
    roleWindows,
    headroom,
    sizeTable: sizeTable(input.sizeTable),
    jitter: integer(input.jitter, 0, MAX_JITTER, "jitter"),
    modifiers,
    trainers,
  }
}

/** Every gameplay flag the experiment mentions: modifier flags and filler requirements. */
export const knownFlags = (experiment: Experiment): string[] =>
  [
    ...new Set([
      ...experiment.modifiers.map((modifier) => modifier.flag),
      ...Object.values(experiment.trainers).flatMap((settings) =>
        settings.roster.fillers.flatMap((filler) =>
          filler.requiresFlag ? [filler.requiresFlag] : [],
        ),
      ),
    ]),
  ].toSorted(byId)

export const serializeExperiment = (experiment: Experiment): string =>
  JSON.stringify(experiment, null, 2)

import type {
  ArcId,
  ArcTuple,
  Experiment,
  PartyMember,
  ReferenceMember,
  ResolvedTrainer,
  Role,
  RoleFeasibility,
  RoleWindows,
  TeamStage,
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
/** Gym stage schedule: minimum global badges -> party size. */
export const GYM_STAGE_SCHEDULE: readonly [number, number][] = [
  [0, 2],
  [3, 3],
  [6, 4],
  [10, 5],
  [16, 6],
]
const GYM_STAGE_LABELS = [
  "Opening team",
  "Developing team",
  "Rising team",
  "Established team",
  "Competitive team",
]

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

type SourcedMember = ReferenceMember & { sourceAce: number }
const sourced = (party: ReferenceMember[]): SourcedMember[] => {
  const ace = Math.max(...party.map((member) => member.level))
  return party.map((member) => ({ ...member, sourceAce: ace }))
}
const toParty = (members: SourcedMember[]): PartyMember[] =>
  members.map((member) => ({
    species: member.species,
    levelOffset: clamp(member.level - member.sourceAce, -30, 0),
  }))

/**
 * A mid-journey Gym party of `size`: the strongest reference members first,
 * then competitive members whose species count is not already represented.
 * Offsets stay relative to each member's own source ace.
 */
const mixedParty = (trainer: TrainerRecord, size: number): PartyMember[] => {
  const reference = sourced(trainer.referenceParty)
  const keep = new Set(
    reference
      .map((member, index) => ({ member, index }))
      .toSorted((a, b) => b.member.level - a.member.level || a.index - b.index)
      .slice(0, size)
      .map(({ index }) => index),
  )
  const result = reference.filter((_, index) => keep.has(index))
  const seen = new Map<string, number>()
  for (const member of sourced(trainer.competitiveParty)) {
    const count = (seen.get(member.species) ?? 0) + 1
    seen.set(member.species, count)
    if (
      result.length < size &&
      result.filter((entry) => entry.species === member.species).length < count
    )
      result.push(member)
  }
  return toParty(result)
}

/**
 * Party size never drops by schedule; raising every offset to at least the
 * previous stage's weakest offset keeps the weakest member's level from
 * dropping at a stage change whenever the ace level does not drop.
 */
const preserveStageFloor = (stages: TeamStage[]): TeamStage[] => {
  for (let index = 1; index < stages.length; index += 1) {
    const previous = stages[index - 1]
    const current = stages[index]
    if (!previous || !current) throw new Error("Team stages must not contain missing entries")
    const floor = Math.min(...previous.party.map((member) => member.levelOffset))
    current.party = current.party.map((member) => ({
      ...member,
      levelOffset: Math.max(member.levelOffset, floor),
    }))
  }
  return stages
}

export const defaultStages = (trainer: TrainerRecord): TeamStage[] => {
  const competitive = toParty(sourced(trainer.competitiveParty))
  const stages: TeamStage[] = []
  if (trainer.gymEligible) {
    GYM_STAGE_SCHEDULE.forEach(([minBadges, target], index) => {
      const size = Math.min(target, competitive.length)
      if (stages.length && size <= (stages.at(-1)?.party.length ?? 0)) return
      const party =
        index === 0 && trainer.earlyParty.length
          ? trainer.earlyParty.map((species, slot, list) => ({
              species,
              levelOffset: slot === list.length - 1 ? 0 : -2,
            }))
          : size === competitive.length
            ? competitive
            : mixedParty(trainer, size)
      const label =
        size === competitive.length ? "Competitive team" : (GYM_STAGE_LABELS[index] ?? "Team")
      stages.push({ minBadges, label: `${label} (${party.length})`, party })
    })
  } else {
    const reference = toParty(sourced(trainer.referenceParty))
    stages.push({ minBadges: 0, label: `Reference team (${reference.length})`, party: reference })
    if (JSON.stringify(reference) !== JSON.stringify(competitive))
      stages.push({
        minBadges: 8,
        label: `Competitive team (${competitive.length})`,
        party: competitive,
      })
  }
  return preserveStageFloor(stages)
}

export const defaultTrainerSettings = (trainer: TrainerRecord): TrainerSettings => {
  const allowedArcs = sortArcs(trainer.allowedArcs)
  return {
    bias: trainer.bias,
    allowedArcs,
    arc: defaultArc(allowedArcs),
    stages: defaultStages(trainer),
  }
}

export const createExperiment = (catalog: TrainerRecord[]): Experiment => {
  const trainers: Experiment["trainers"] = Object.create(null)
  for (const trainer of catalog) trainers[trainer.id] = defaultTrainerSettings(trainer)
  return validateExperiment(
    {
      version: 3,
      arcs: structuredClone(DEFAULT_ARCS),
      roleWindows: { ...DEFAULT_ROLE_WINDOWS },
      headroom: DEFAULT_HEADROOM,
      trainers,
    },
    catalog,
  )
}

const stageAt = (settings: TrainerSettings, badges: number) => {
  const index = settings.stages.findLastIndex((entry) => entry.minBadges <= badges)
  const stage = settings.stages[index]
  if (!stage) throw new Error("Missing initial team stage")
  return { stage, index }
}

const aceAt = (settings: TrainerSettings, experiment: Experiment, arc: ArcId, point: WorldPoint) =>
  levelBase(point, experiment.headroom) +
  standingAt(settings, experiment, arc, progressIndex(point))

const minLevelAt = (
  settings: TrainerSettings,
  experiment: Experiment,
  arc: ArcId,
  point: WorldPoint,
) => {
  const ace = clamp(aceAt(settings, experiment, arc, point), 1, 100)
  const { stage } = stageAt(settings, point.badges)
  return Math.min(...stage.party.map((member) => clamp(ace + member.levelOffset, 1, 100)))
}

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
): ResolvedTrainer => {
  const world = normalizePoint(point)
  const settings = experiment.trainers[trainer.id]
  if (!settings) throw new Error(`Missing trainer settings: ${trainer.id}`)
  const arc = arcOverride ?? settings.arc
  const progress = progressIndex(world)
  const standing = standingAt(settings, experiment, arc, progress)
  const cap = worldCap(world)
  const base = levelBase(world, experiment.headroom)
  const rawAce = base + standing
  const aceLevel = clamp(rawAce, 1, 100)
  const { stage, index } = stageAt(settings, world.badges)
  const warnings: string[] = []
  if (rawAce > 100) warnings.push(`Ace level ${rawAce} clamped to 100.`)
  if (rawAce < 1) warnings.push(`Ace level ${rawAce} clamped to 1.`)
  const party = stage.party.map((member) => {
    const rawLevel = aceLevel + member.levelOffset
    if (rawLevel < 1 || rawLevel > 100)
      warnings.push(`${member.species} level clipped to the 1–100 range.`)
    return { ...member, level: clamp(rawLevel, 1, 100) }
  })
  const before = previousPoint(world)
  if (
    before &&
    minLevelAt(settings, experiment, arc, before) > Math.min(...party.map((m) => m.level))
  )
    warnings.push("The weakest member's level dropped since the previous progress step.")
  return {
    trainer,
    arc,
    progress,
    standing,
    aceLevel,
    worldCap: cap,
    levelBase: base,
    gap: aceLevel - cap,
    role: classifyRole(standing, experiment.roleWindows),
    stage: stage.label,
    stageIndex: index,
    party,
    warnings,
  }
}

export const hasCompetitiveProfile = (trainer: TrainerRecord): boolean =>
  trainer.competitiveParty.length === 6

/**
 * Per venue and role at progress index p: candidates in-window under every
 * allowed arc (guaranteed), under some allowed arc (possible), and under the
 * currently selected arc. Standing depends on p only, not on league clears
 * or the level headroom.
 */
export const venueFeasibility = (
  catalog: TrainerRecord[],
  experiment: Experiment,
  progress: number,
  options: { includeIncomplete?: boolean } = {},
): VenueFeasibility[] =>
  VENUES.map(({ venue, pool }) => {
    const members = catalog.filter(
      (trainer) => pool === "open" || trainer.homeLeagues.includes(venue),
    )
    const candidates = members.filter(
      (trainer) => options.includeIncomplete || hasCompetitiveProfile(trainer),
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
      excluded: members.filter((trainer) => !candidates.includes(trainer)).map((t) => t.name),
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
   * the ace − cap > 0 check; at the ceiling it is ace > levelBase.
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
        return { arc, gap: resolved.gap, baseGap: resolved.aceLevel - resolved.levelBase }
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

export const V1_REJECTION =
  "Version 1 experiments use the retired trainer-rating model and cannot be imported. Start from the version 3 defaults."
export const V2_REJECTION =
  "Version 2 experiments predate post-game arcs (p = 0–48) and level headroom and cannot be imported. Start from the version 3 defaults."

export const validateExperiment = (value: unknown, catalog: TrainerRecord[]): Experiment => {
  const input = object(value, "root")
  if (input.version === 1) fail(V1_REJECTION)
  if (input.version === 2) fail(V2_REJECTION)
  if (input.version !== 3) fail("version must be 3")
  exactKeys(input, ["version", "arcs", "roleWindows", "headroom", "trainers"], "root")

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
    exactKeys(settings, ["bias", "allowedArcs", "arc", "stages"], `trainers.${id}`)
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
    if (!Array.isArray(settings.stages) || !settings.stages.length || settings.stages.length > 25)
      fail(`${id}.stages must have 1–25 stages`)
    const stages: TeamStage[] = Array.from(settings.stages as unknown[], (entry, index) => {
      const stage = object(entry, `${id}.stages[${index}]`)
      exactKeys(stage, ["minBadges", "label", "party"], `${id}.stages[${index}]`)
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
        minBadges: integer(stage.minBadges, 0, 24, `${id}.minBadges`),
        label: label(stage.label, 100, `${id}.label`),
        party,
      }
    })
    if (stages[0]?.minBadges !== 0) fail(`${id}.first stage must start at 0 badges`)
    stages.forEach((stage, index) => {
      const previous = stages[index - 1]
      if (index && (!previous || stage.minBadges <= previous.minBadges))
        fail(`${id}.stages badge thresholds must increase uniquely`)
      if (previous && stage.party.length < previous.party.length)
        fail(`${id}.stage party sizes must not decrease`)
    })
    trainers[id] = { bias, allowedArcs, arc, stages }
  }
  return { version: 3, arcs, roleWindows, headroom, trainers }
}

export const serializeExperiment = (experiment: Experiment): string =>
  JSON.stringify(experiment, null, 2)

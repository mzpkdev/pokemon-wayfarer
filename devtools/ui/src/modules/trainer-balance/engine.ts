import type {
  Anchor,
  Archetype,
  DormantReason,
  EvolutionData,
  Experiment,
  LadderRow,
  LearnsetData,
  Milestone,
  MilestoneEvent,
  PoolEntry,
  PoolStatus,
  ResolvedMove,
  ResolvedTrainer,
  RosterSlot,
  ScalerKind,
  TeamMember,
  TrainerRecord,
  TrainerSettings,
  WorldPoint,
} from "./types.js"

export const ROSTER_SIZE = 6
/** Aces per roster: roster slot 1 plus up to two more. */
export const MAX_ACES = 3
export const LEVEL_OFFSET = { min: -6, max: 0, default: -2 } as const
export const LINEUP_SIZE = 5
export const LEAGUES = ["Indigo", "Sevii Masters", "Hoenn"] as const
export const MAX_ANCHORS = 20
/** Moves a Pokémon knows, and pool moves a member takes. */
export const MAX_MOVES = 4
/** Entries a move pool may list. */
export const MAX_POOL_ENTRIES = 64

/** The level cap curve (24 badges = TR 160). */
export const LEVEL_CAP_ANCHORS: readonly Anchor[] = [
  [0, 15],
  [40, 28],
  [80, 50],
  [120, 75],
  [160, 100],
]
/**
 * Notable trainer team level: its own low end below TR 40 (Lv 5 at TR 0), then
 * the level cap anchors from TR 40 up. The level cap itself is unchanged.
 */
export const DEFAULT_TEAM_LEVEL: readonly Anchor[] = [
  [0, 5],
  [20, 14],
  [40, 28],
  [80, 50],
  [120, 75],
  [160, 100],
]
/** Step table as paired anchors: 0–10 -> 1, 11–28 -> 2, 29–43 -> 3, 44–70 -> 4, 71–95 -> 5, 96+ -> 6. */
export const DEFAULT_TEAM_SIZE: readonly Anchor[] = [
  [0, 1],
  [10, 1],
  [11, 2],
  [28, 2],
  [29, 3],
  [43, 3],
  [44, 4],
  [70, 4],
  [71, 5],
  [95, 5],
  [96, 6],
]
/** The wild level curve by player TR (replaces "cap - 10"). */
export const DEFAULT_WILD_LEVEL: readonly Anchor[] = [
  [0, 6],
  [40, 24],
  [80, 40],
  [120, 58],
  [160, 78],
]
/** The regular trainer level curve by player TR, before authored level bonuses and Gym-member adjustments. */
export const DEFAULT_REGULAR_TRAINER_LEVEL: readonly Anchor[] = [
  [0, 9],
  [40, 27],
  [80, 44],
  [120, 62],
  [160, 82],
]

/** Archetypes, each a growth scaler over world progress, in display order. */
export const ARCHETYPES: readonly Archetype[] = [
  "steady",
  "prodigy",
  "sleeper",
  "veteran",
  "rival",
  "legend",
  "star",
  "comeback",
  "burst",
]
/** An archetype's display name: the proper noun used in prose (Steady, Prodigy, ...). */
export const archetypeName = (archetype: Archetype): string =>
  `${archetype[0]?.toUpperCase()}${archetype.slice(1)}`
/** Burst is a step scaler; every other archetype is interpolated. The kind is not editable. */
export const ARCHETYPE_KIND: Readonly<Record<Archetype, ScalerKind>> = {
  steady: "interpolated",
  prodigy: "interpolated",
  sleeper: "interpolated",
  veteran: "interpolated",
  rival: "interpolated",
  legend: "interpolated",
  star: "interpolated",
  comeback: "interpolated",
  burst: "step",
}
/**
 * Growth % by world progress: 0 / 40 / 80 / 120 / 160 (the Rival also at 20; the Legend only at
 * 0 and 160, 0% at both).
 */
export const DEFAULT_ARCHETYPE_GROWTH: Readonly<Record<Archetype, readonly Anchor[]>> = {
  steady: [
    [0, 0],
    [40, 25],
    [80, 50],
    [120, 75],
    [160, 100],
  ],
  prodigy: [
    [0, 0],
    [40, 50],
    [80, 80],
    [120, 95],
    [160, 100],
  ],
  sleeper: [
    [0, 0],
    [40, 10],
    [80, 25],
    [120, 55],
    [160, 100],
  ],
  veteran: [
    [0, 0],
    [40, 60],
    [80, 100],
    [120, 100],
    [160, 100],
  ],
  rival: [
    [0, 0],
    [20, 15],
    [40, 29],
    [80, 53],
    [120, 76],
    [160, 100],
  ],
  legend: [
    [0, 0],
    [160, 0],
  ],
  star: [
    [0, 0],
    [40, 10],
    [80, 50],
    [120, 90],
    [160, 100],
  ],
  comeback: [
    [0, 0],
    [40, 45],
    [80, 50],
    [120, 55],
    [160, 100],
  ],
  burst: [
    [0, 0],
    [40, 25],
    [80, 50],
    [120, 75],
    [160, 100],
  ],
}
/** World progress points the explorer reports each trainer TR at: 0, 4, 8, 16 and 24 badges. */
export const WORLD_PROGRESS_CHECKPOINTS = [0, 40, 80, 120, 160] as const
/** A Gym Leader within this many TR of the player TR is near; further away is below or above. */
export const NEAR_BAND = 10

const clamp = (value: number, min: number, max: number) => Math.max(min, Math.min(max, value))
const finite = (value: number, label: string) => {
  if (!Number.isFinite(value)) throw new Error(`${label} must be finite`)
  return value
}

/**
 * The anchors around a TR strictly between the first and last anchor: the last
 * anchor at or below it and the first above it.
 */
const bracket = (anchors: readonly Anchor[], at: number): [Anchor, Anchor] => {
  const index = anchors.findIndex(([x]) => x > at)
  const lower = anchors[index - 1]
  const upper = anchors[index]
  if (!lower || !upper) throw new Error("Scaler anchors must not contain missing points")
  return [lower, upper]
}

/**
 * Reads a scaler: an interpolated scaler is linear between anchors with halves
 * rounded up; a step scaler holds each anchor's value until the next anchor.
 * Both are flat before the first and past the last anchor. TR is never clamped.
 */
export const scale = (
  anchors: readonly Anchor[],
  tr: number,
  kind: ScalerKind = "interpolated",
): number => {
  const rating = finite(tr, "TR")
  const first = anchors[0]
  const last = anchors.at(-1)
  if (!first || !last) throw new Error("A scaler needs at least one anchor")
  if (rating <= first[0]) return first[1]
  if (rating >= last[0]) return last[1]
  if (kind === "step") return bracket(anchors, rating)[0][1]
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

/** Clamps badges to 0–24. */
export const normalizePoint = (point: WorldPoint): WorldPoint => ({
  badges: clamp(Math.round(finite(point.badges, "Badges")), 0, 24),
})

/**
 * The player's TR on the rescaled formula: badges 1–8 give +10 each, badges
 * 9–24 +5 each, league wins nothing. 24 badges = TR 160; TR has no upper limit.
 */
export const playerRating = (point: WorldPoint): number => {
  const { badges } = normalizePoint(point)
  return 10 * Math.min(badges, 8) + 5 * Math.max(0, badges - 8)
}

/**
 * World progress: the player TR as the world sees it. Notable trainers grow
 * with it; it is never computed from any notable trainer TR.
 */
export const worldProgress = (point: WorldPoint): number => playerRating(point)

const progress = (value: number): number => {
  if (!Number.isSafeInteger(value) || value < 0)
    throw new Error("World progress must be a non-negative whole number")
  return value
}

/**
 * Growth along an archetype scaler: start + roundHalfUp((peak - start) *
 * growth% / 100), with growth% read exactly (no intermediate rounding) and flat
 * past the last anchor. A step scaler holds each anchor's growth % until the
 * next anchor.
 */
export const growthTR = (
  growth: readonly Anchor[],
  startTR: number,
  peakTR: number,
  world: number,
  kind: ScalerKind = "interpolated",
): number => {
  const at = progress(world)
  const first = growth[0]
  const last = growth.at(-1)
  if (!first || !last) throw new Error("A scaler needs at least one anchor")
  // growth% = numerator / denominator.
  let numerator = BigInt(last[1])
  let denominator = 1n
  if (at <= first[0]) numerator = BigInt(first[1])
  else if (at < last[0] && kind === "step") numerator = BigInt(bracket(growth, at)[0][1])
  else if (at < last[0]) {
    const index = growth.findIndex(([x]) => x >= at)
    const lower = growth[index - 1]
    const upper = growth[index]
    if (!lower || !upper) throw new Error("Scaler anchors must not contain missing points")
    denominator = BigInt(upper[0] - lower[0])
    numerator = BigInt(lower[1]) * denominator + BigInt(at - lower[0]) * BigInt(upper[1] - lower[1])
  }
  const divisor = 100n * denominator
  const gain = (2n * BigInt(peakTR - startTR) * numerator + divisor) / (2n * divisor)
  return startTR + Number(gain)
}

/**
 * A notable trainer's TR at a world progress: their archetype's growth scaler
 * (of that archetype's kind) between start and peak TR. The same rule holds
 * for every archetype.
 */
export const trainerRating = (
  experiment: Pick<Experiment, "archetypes">,
  settings: Pick<TrainerSettings, "startTR" | "archetype" | "peakTR">,
  world: number,
): number =>
  growthTR(
    experiment.archetypes[settings.archetype],
    settings.startTR,
    settings.peakTR,
    world,
    ARCHETYPE_KIND[settings.archetype],
  )

/** The player's level cap at a player TR. A readout for comparison: it never feeds notable trainer results. */
export const levelCap = (playerTR: number): number => scale(LEVEL_CAP_ANCHORS, playerTR)

export const MAX_BADGES = 24
/** The player TR a badge count gives: the badge presets of the player TR control. */
export const badgeTR = (badges: number): number => playerRating({ badges })

/** How a player TR sits on the badge formula: exactly a badge count, between two, or past 24. */
export type BadgeMatch =
  | { kind: "exact"; badges: number }
  | { kind: "between"; lower: number; upper: number }
  | { kind: "beyond"; badges: typeof MAX_BADGES }

/** Matches a player TR to the badge formula. TR has no upper limit; the formula stops at 24 badges. */
export const badgeMatch = (playerTR: number): BadgeMatch => {
  const tr = progress(playerTR)
  if (tr > badgeTR(MAX_BADGES)) return { kind: "beyond", badges: MAX_BADGES }
  let lower = 0
  for (let badges = 0; badges <= MAX_BADGES; badges += 1) {
    const at = badgeTR(badges)
    if (at === tr) return { kind: "exact", badges }
    if (at < tr) lower = badges
  }
  return { kind: "between", lower, upper: lower + 1 }
}

/** "8 badges", "between 9 and 10 badges" or "beyond 24 badges". */
export const badgeMatchText = (match: BadgeMatch): string =>
  match.kind === "exact"
    ? `${match.badges} ${match.badges === 1 ? "badge" : "badges"}`
    : match.kind === "between"
      ? `between ${match.lower} and ${match.upper} badges`
      : `beyond ${match.badges} badges`

/** World scaling from player TR: the level cap, the wild level curve and the regular trainer level curve. */
export const worldLevels = (
  experiment: Pick<Experiment, "wildLevel" | "routeTrainerLevel">,
  playerTR: number,
) => {
  const tr = progress(playerTR)
  return {
    tr,
    cap: scale(LEVEL_CAP_ANCHORS, tr),
    wild: scale(experiment.wildLevel, tr),
    regularTrainer: scale(experiment.routeTrainerLevel, tr),
  }
}

/** One stage of a line: the level it is reached at (1 for the base stage). */
export type Stage = { species: string; level: number }
/** Every stage on the catalog's chains with its line up to it, and the stages that evolve further. */
export type Evolution = {
  lines: ReadonlyMap<string, readonly Stage[]>
  notFinal: ReadonlySet<string>
}
/** No evolution data: every species is its own line and never steps down. */
export const NO_EVOLUTION: Evolution = { lines: new Map(), notFinal: new Set() }

/**
 * Indexes the catalog's predecessor chains by every stage on them. A chain
 * alternates species and levels; each edge has one level, levels increase
 * along the line, and no species repeats (no cycles).
 */
export const evolutionIndex = (data: EvolutionData): Evolution => {
  const lines = new Map<string, Stage[]>()
  const notFinal = new Set(data.notFinal)
  for (const [species, chain] of Object.entries(data.chains)) {
    const where = `Evolution chain for ${species}`
    if (chain.length % 2 !== 1 || chain.at(-1) !== species)
      throw new Error(`${where} must alternate species and levels and end at ${species}`)
    const line: Stage[] = []
    for (let index = 0; index < chain.length; index += 2) {
      const name = chain[index]
      const level = index === 0 ? 1 : chain[index - 1]
      if (typeof name !== "string" || !name) throw new Error(`${where} has a missing species`)
      if (typeof level !== "number" || !Number.isInteger(level) || level < 1 || level > 100)
        throw new Error(`${where}: ${name} needs an evolution level from 1 to 100`)
      if (line.some((stage) => stage.species === name))
        throw new Error(`${where} repeats ${name} (an evolution cycle)`)
      const previous = line.at(-1)
      if (previous && level <= previous.level)
        throw new Error(`${where}: evolution levels must increase along the line`)
      line.push({ species: name, level })
      const known = lines.get(name)
      const prefix = line.slice()
      if (
        known &&
        (known.length !== prefix.length ||
          known.some(
            (stage, at) =>
              stage.species !== prefix[at]?.species || stage.level !== prefix[at]?.level,
          ))
      )
        throw new Error(`${name} has conflicting evolution levels or predecessors`)
      lines.set(name, prefix)
      if (previous) notFinal.add(previous.species)
    }
  }
  return { lines, notFinal }
}

/** Whether a species has catalog evolution data and whether it is a final stage. */
export const evolutionStatus = (evolution: Evolution, species: string) => ({
  known: evolution.lines.has(species),
  final: !evolution.notFinal.has(species),
})

/**
 * The downward rule: the authored stage steps down its predecessor chain until
 * the level is at least that stage's evolution level. It never evolves forward.
 */
export const stageAt = (
  evolution: Evolution,
  species: string,
  level: number,
): { species: string; authoredAt: number | null } => {
  const line = evolution.lines.get(species)
  const authored = line?.at(-1)
  if (!line || !authored) return { species, authoredAt: null }
  let index = line.length - 1
  while (index > 0 && level < (line[index]?.level ?? 1)) index -= 1
  const stage = line[index] ?? authored
  return {
    species: stage.species,
    authoredAt: stage.species === species ? null : authored.level,
  }
}

/**
 * Battle order: the filler slots in reverse roster order, then the aces in
 * reverse roster order, so roster slot 1 (the signature Pokémon) is always last.
 */
export const battleOrderOf = <T extends Pick<RosterSlot, "isAce">>(team: readonly T[]): T[] => [
  ...team.filter((member) => !member.isAce).toReversed(),
  ...team.filter((member) => member.isAce).toReversed(),
]

/** One species' learnsets: level-up moves in the game's order, and every move it can learn. */
export type Learnset = {
  levelUp: readonly (readonly [level: number, move: string])[]
  /**
   * Each level-up move's lowest learn level (0 is an evolution move, available whenever the
   * species is present): an entry without a from level waits for it.
   */
  levelUpAt: ReadonlyMap<string, number>
  /** Its level-up learnset at any level, plus its TM/tutor list. */
  learnable: ReadonlySet<string>
}
/** Every valid move name and each catalog species' learnsets. */
export type Learnsets = { moves: ReadonlySet<string>; species: ReadonlyMap<string, Learnset> }
/** No learnset data: no member learns anything, and move names are not checked. */
export const NO_LEARNSETS: Learnsets = { moves: new Set(), species: new Map() }

/** One species' learnset from its level-up [level, move] pairs in the game's order and TM/tutor moves. */
export const learnsetOf = (
  levelUp: readonly (readonly [number, string])[],
  teachable: readonly string[],
): Learnset => {
  const levelUpAt = new Map<string, number>()
  for (const [level, move] of levelUp)
    levelUpAt.set(move, Math.min(level, levelUpAt.get(move) ?? level))
  return { levelUp, levelUpAt, learnable: new Set([...levelUpAt.keys(), ...teachable]) }
}

/** Indexes the catalog's compact learnsets (move indices into `moves`). */
export const learnsetIndex = (data: LearnsetData): Learnsets => {
  const name = (index: number, species: string): string => {
    const move = data.moves[index]
    if (move === undefined) throw new Error(`Learnset for ${species} lists an unknown move`)
    return move
  }
  const species = new Map<string, Learnset>()
  for (const [id, { levelUp, teachable }] of Object.entries(data.species)) {
    if (levelUp.length % 2 !== 0)
      throw new Error(`Level-up learnset for ${id} must pair levels and moves`)
    const pairs: [number, string][] = []
    for (let index = 0; index < levelUp.length; index += 2)
      pairs.push([levelUp[index] ?? 0, name(levelUp[index + 1] ?? -1, id)])
    species.set(
      id,
      learnsetOf(
        pairs,
        teachable.map((move) => name(move, id)),
      ),
    )
  }
  return { moves: new Set(data.moves), species }
}

/**
 * The constructor's default moveset: the last four level-up moves learned by `level`, oldest
 * first. It walks the learnset in order until a move above the level, skips evolution moves
 * (level 0) and moves already known, and drops the oldest when full.
 */
export const defaultMoveset = (learnset: Learnset | undefined, level: number): string[] => {
  const moves: string[] = []
  for (const [learned, move] of learnset?.levelUp ?? []) {
    if (learned > level) break
    if (learned === 0 || moves.includes(move)) continue
    if (moves.length === MAX_MOVES) moves.shift()
    moves.push(move)
  }
  return moves
}

/** What the move pool resolver reads about a member: its current species and level. */
type PoolMember = Pick<TeamMember, "slot" | "species" | "level" | "isAce">

/**
 * The level from which a member may take a pool entry, or null when it can't. An entry with a
 * from level needs the move in the member's level-up learnset at any level or its TM/tutor list,
 * and waits for the from level. An entry without one needs the move in the level-up learnset and
 * waits for its lowest learn level there (an evolution move, level 0, is available at once).
 */
const entryLevel = (entry: PoolEntry, learnset: Learnset | undefined): number | null =>
  entry.fromLevel !== undefined
    ? learnset?.learnable.has(entry.move)
      ? entry.fromLevel
      : null
    : (learnset?.levelUpAt.get(entry.move) ?? null)

/**
 * Resolves the move pool for a team whose species and levels are final (stepping down
 * included). Each member starts from its default level-up moveset. Members are visited aces
 * first, then fillers, each in list order; each walks the pool top to bottom and takes every
 * entry that is unassigned, is eligible for its current species at its level (see entryLevel)
 * and not already in its moveset, up to four. Pool moves fill empty move slots, then replace
 * the oldest level-up moves. An entry nobody takes is dormant. No randomness: a pure function
 * of the team and the pool.
 */
export const resolveMovePool = (
  team: readonly PoolMember[],
  pool: readonly PoolEntry[],
  learnsets: Learnsets,
): { moves: Map<number, ResolvedMove[]>; pool: PoolStatus[] } => {
  const taken: (PoolMember | null)[] = pool.map(() => null)
  const moves = new Map<number, ResolvedMove[]>()
  const visit = [
    ...team.filter((member) => member.isAce),
    ...team.filter((member) => !member.isAce),
  ]
  for (const member of visit) {
    const learnset = learnsets.species.get(member.species)
    const base = defaultMoveset(learnset, member.level)
    const known = new Set(base)
    const picks: string[] = []
    pool.forEach((entry, index) => {
      if (picks.length === MAX_MOVES || taken[index] || known.has(entry.move)) return
      const from = entryLevel(entry, learnset)
      if (from === null || from > member.level) return
      picks.push(entry.move)
      known.add(entry.move)
      taken[index] = member
    })
    const resolved: ResolvedMove[] = base.map((move) => ({ move, source: "level-up" }))
    let oldest = 0
    for (const move of picks) {
      if (resolved.length < MAX_MOVES) resolved.push({ move, source: "pool" })
      else resolved[oldest++] = { move, source: "pool" }
    }
    moves.set(member.slot, resolved)
  }
  const status = pool.map((entry, index): PoolStatus => {
    const fromLevel = entry.fromLevel ?? null
    const member = taken[index]
    const base = { index, move: entry.move, fromLevel }
    if (member)
      return {
        ...base,
        slot: member.slot,
        species: member.species,
        reason: null,
        waitLevel: null,
      }
    const levels = team.flatMap((other) => {
      const level = entryLevel(entry, learnsets.species.get(other.species))
      return level === null ? [] : [{ other, level }]
    })
    const learners = team.filter((other) =>
      learnsets.species.get(other.species)?.learnable.has(entry.move),
    )
    const waiting = levels.every(({ other, level }) => other.level < level)
    const reason: DormantReason = !learners.length
      ? "unlearnable"
      : !levels.length
        ? "tm-only"
        : waiting
          ? "level"
          : "taken"
    const waitLevel = reason === "level" ? Math.min(...levels.map(({ level }) => level)) : null
    return { ...base, slot: null, species: null, reason, waitLevel }
  })
  return { moves, pool: status }
}

/** A dormant entry's reason as text, e.g. "below from level Lv 40". */
export const dormantReasonText = (
  status: Pick<PoolStatus, "reason" | "fromLevel" | "waitLevel">,
): string =>
  status.reason === "unlearnable"
    ? "no one can learn it"
    : status.reason === "tm-only"
      ? "TM/tutor only — needs a from level"
      : status.reason === "level"
        ? status.fromLevel === null
          ? `below its learn level Lv ${status.waitLevel}`
          : `below from level Lv ${status.fromLevel}`
        : "taken: every learner already knows it or has four pool moves"

/** How a roster's lines can learn a move: by level-up on some stage, only by TM/tutor, or not at all. */
export type PoolLearning = "level-up" | "tm-only" | "unlearnable"

/**
 * How any stage on the roster's lines learns `move`. An entry without a from level needs a
 * level-up learner; a TM/tutor-only move needs a from level.
 */
export const poolLearning = (
  move: string,
  roster: readonly Pick<RosterSlot, "species">[],
  evolution: Evolution,
  learnsets: Learnsets,
): PoolLearning => {
  const stages = roster.flatMap(({ species }) =>
    (evolution.lines.get(species) ?? [{ species }]).map((stage) =>
      learnsets.species.get(stage.species),
    ),
  )
  if (stages.some((learnset) => learnset?.levelUpAt.has(move))) return "level-up"
  if (stages.some((learnset) => learnset?.learnable.has(move))) return "tm-only"
  return "unlearnable"
}

/**
 * Team = the first N roster slots (N = team size at TR), so join order is list
 * order; level = clamp(team level + offset, 1, 100); battle order puts the aces
 * last (battleOrderOf). Each member steps down to the stage its level supports,
 * then the move pool resolves against every member's current species and level.
 */
export const buildTeam = (
  roster: readonly RosterSlot[],
  teamLevel: number,
  size: number,
  evolution: Evolution,
  pool: readonly PoolEntry[] = [],
  learnsets: Learnsets = NO_LEARNSETS,
) => {
  const members = roster.slice(0, size).map((rosterSlot, index) => {
    const level = clamp(teamLevel + rosterSlot.levelOffset, 1, 100)
    const stage = stageAt(evolution, rosterSlot.species, level)
    return {
      ...rosterSlot,
      species: stage.species,
      slot: index + 1,
      level,
      authoredSpecies: rosterSlot.species,
      authoredAt: stage.authoredAt,
    }
  })
  const resolved = resolveMovePool(members, pool, learnsets)
  const team = members.map(
    (member): TeamMember => ({ ...member, moves: resolved.moves.get(member.slot) ?? [] }),
  )
  return { team, battleOrder: battleOrderOf(team), pool: resolved.pool }
}

/**
 * A trainer's team at a world progress: their own TR from their growth, then
 * the team from that TR alone, each member at the stage its level supports.
 * Nothing else about the player is read.
 */
export const resolveTrainer = (
  trainer: TrainerRecord,
  experiment: Experiment,
  world: number,
  evolution: Evolution,
  learnsets: Learnsets = NO_LEARNSETS,
): ResolvedTrainer => {
  const settings = experiment.trainers[trainer.id]
  if (!settings) throw new Error(`Missing trainer settings: ${trainer.id}`)
  const tr = trainerRating(experiment, settings, world)
  const teamLevel = teamLevelFor(experiment, tr)
  const size = teamSizeFor(experiment, tr)
  const { team, battleOrder, pool } = buildTeam(
    settings.roster,
    teamLevel,
    size,
    evolution,
    settings.movePool,
    learnsets,
  )
  const warnings: string[] = []
  if (settings.roster.length < ROSTER_SIZE)
    warnings.push(
      `The roster lists ${settings.roster.length} of ${ROSTER_SIZE} Pokémon: add ${ROSTER_SIZE - settings.roster.length} more.`,
    )
  if (team.length < size) warnings.push(`The team fills ${team.length} of ${size} slots.`)
  for (const member of team)
    if (teamLevel + member.levelOffset < 1)
      warnings.push(`${member.authoredSpecies} level clipped to 1.`)
  if (learnsets.species.size)
    for (const member of team)
      if (!learnsets.species.has(member.species))
        warnings.push(`No learnset data for ${member.species} in the catalog, so it has no moves.`)
  return {
    trainer,
    worldProgress: world,
    tr,
    startTR: settings.startTR,
    archetype: settings.archetype,
    peakTR: settings.peakTR,
    teamLevel,
    size,
    team,
    battleOrder,
    pool,
    dormant: pool.filter((entry) => entry.slot === null),
    rosterLength: settings.roster.length,
    warnings,
  }
}

/** The milestone scan stops here even if an archetype's growth ceiling is authored further out. */
export const MILESTONE_SCAN_LIMIT = 2000

/**
 * The world progress at and past which nothing about a trainer's team or the
 * level cap changes: the later of the archetype growth ceiling and the level
 * cap ceiling TR (at most MILESTONE_SCAN_LIMIT). Trainer TR never rises past
 * the growth ceiling.
 */
export const milestoneEnd = (experiment: Pick<Experiment, "archetypes">, archetype: Archetype) =>
  Math.min(
    MILESTONE_SCAN_LIMIT,
    Math.max(experiment.archetypes[archetype].at(-1)?.[0] ?? 0, LEVEL_CAP_ANCHORS.at(-1)?.[0] ?? 0),
  )

/**
 * Every world progress (player TR) where a notable trainer's team changes,
 * scanning whole world progress from 0 to milestoneEnd: the starting team, a
 * roster slot joining, a member's stage changing, the team level crossing the
 * level cap (strictly above it, or strictly below it again; equal keeps the
 * side), and trainer TR reaching peak TR (or stopping short of it).
 */
export const milestones = (
  trainer: TrainerRecord,
  experiment: Experiment,
  evolution: Evolution,
  learnsets: Learnsets = NO_LEARNSETS,
): Milestone[] => {
  const settings = experiment.trainers[trainer.id]
  if (!settings) throw new Error(`Missing trainer settings: ${trainer.id}`)
  const end = milestoneEnd(experiment, settings.archetype)
  const result: Milestone[] = []
  let previous: ResolvedTrainer | undefined
  let previousAbove = false
  let peakSeen = false
  // Pool entries assigned at some world progress so far; an entry wakes the first time.
  const assigned = new Set<number>()
  for (let world = 0; world <= end; world += 1) {
    const current = resolveTrainer(trainer, experiment, world, evolution, learnsets)
    const cap = levelCap(world)
    // Equal to the level cap keeps the previous side, so rounding cannot flicker the crossing.
    const above: boolean =
      current.teamLevel === cap && previous ? previousAbove : current.teamLevel > cap
    const events: MilestoneEvent[] = []
    if (!previous)
      events.push({ kind: "start", team: current.team.map((m) => m.species), aboveCap: above })
    else {
      for (const member of current.team) {
        const before = previous.team[member.slot - 1]
        if (!before)
          events.push({
            kind: "join",
            slot: member.slot,
            species: member.species,
            isAce: member.isAce,
          })
        else if (before.species !== member.species)
          events.push({
            kind: "evolve",
            slot: member.slot,
            from: before.species,
            to: member.species,
          })
      }
      for (const entry of current.pool)
        if (entry.slot !== null && entry.species !== null && !assigned.has(entry.index))
          events.push({ kind: "wake", move: entry.move, slot: entry.slot, species: entry.species })
      if (above !== previousAbove) events.push({ kind: "cap", aboveCap: above })
    }
    for (const entry of current.pool) if (entry.slot !== null) assigned.add(entry.index)
    if (!peakSeen && current.tr >= settings.peakTR) {
      peakSeen = true
      events.push({ kind: "peak", tr: current.tr, reached: true })
    } else if (!peakSeen && world === end)
      events.push({ kind: "peak", tr: current.tr, reached: false })
    if (events.length)
      result.push({
        worldProgress: world,
        tr: current.tr,
        teamLevel: current.teamLevel,
        cap,
        events,
      })
    previous = current
    previousAbove = above
  }
  return result
}

const ordinal = (value: number): string =>
  `${value}${value % 10 === 1 && value !== 11 ? "st" : value % 10 === 2 && value !== 12 ? "nd" : value % 10 === 3 && value !== 13 ? "rd" : "th"}`

/**
 * One event as timeline text, e.g. "3rd slot (Aerodactyl) ace joins", "4th slot (Kabutops) joins",
 * "Onix → Steelix" or "Earthquake wakes (Golem)".
 */
export const milestoneEventText = (event: MilestoneEvent, milestone: Milestone): string => {
  switch (event.kind) {
    case "start":
      return `${event.team.join(", ")}${event.aboveCap ? " (team level above the level cap)" : ""}`
    case "join":
      return `${ordinal(event.slot)} slot (${event.species}) ${event.isAce ? "ace joins" : "joins"}`
    case "evolve":
      return `${event.from} → ${event.to}`
    case "wake":
      return `${event.move} wakes (${event.species})`
    case "cap":
      return event.aboveCap
        ? `team level Lv ${milestone.teamLevel} passes the level cap Lv ${milestone.cap}`
        : `team level Lv ${milestone.teamLevel} drops below the level cap Lv ${milestone.cap}`
    case "peak":
      return event.reached ? `peak TR ${event.tr}` : `TR stops at ${event.tr}, short of peak TR`
  }
}

/** A milestone as timeline text: "104: Onix → Steelix". */
export const milestoneText = (milestone: Milestone): string =>
  `${milestone.worldProgress}: ${milestone.events.map((event) => milestoneEventText(event, milestone)).join(", ")}`

/**
 * The league lineup from one global pool of league-eligible trainers at a
 * world progress: the top five by TR (ties keep catalog order), returned in
 * battle order, ascending TR with the strongest last.
 */
export const leagueLineup = (
  catalog: TrainerRecord[],
  experiment: Experiment,
  world: number,
  evolution: Evolution,
  learnsets: Learnsets = NO_LEARNSETS,
): ResolvedTrainer[] =>
  catalog
    .filter((trainer) => trainer.leagueEligible)
    .map((trainer) => resolveTrainer(trainer, experiment, world, evolution, learnsets))
    .toSorted((a, b) => b.tr - a.tr)
    .slice(0, LINEUP_SIZE)
    .toSorted((a, b) => a.tr - b.tr)

/** Gym Leaders, including a Gym Leader duo. */
export const isGymLeader = (trainer: TrainerRecord): boolean =>
  trainer.role === "Gym Leader" || trainer.role === "Gym Leader duo"

/**
 * The Gym Leaders (duo included) at a world progress, ascending TR (ties keep
 * catalog order), each marked below, near (within NEAR_BAND) or above the player TR.
 */
export const gymLadder = (
  catalog: TrainerRecord[],
  experiment: Experiment,
  world: number,
): LadderRow[] =>
  catalog
    .filter(isGymLeader)
    .map((trainer): LadderRow => {
      const settings = experiment.trainers[trainer.id]
      if (!settings) throw new Error(`Missing trainer settings: ${trainer.id}`)
      const tr = trainerRating(experiment, settings, world)
      const gap = tr - world
      return {
        trainer,
        tr,
        gap,
        mark: gap < -NEAR_BAND ? "below" : gap > NEAR_BAND ? "above" : "near",
      }
    })
    .toSorted((a, b) => a.tr - b.tr)

export const defaultTrainerSettings = (trainer: TrainerRecord): TrainerSettings => ({
  startTR: trainer.startTR,
  archetype: trainer.archetype,
  peakTR: trainer.peakTR,
  roster: structuredClone(trainer.roster),
  movePool: structuredClone(trainer.movePool),
})

export const createExperiment = (
  catalog: TrainerRecord[],
  moves: ReadonlySet<string> = NO_LEARNSETS.moves,
): Experiment => {
  const trainers: Experiment["trainers"] = Object.create(null)
  for (const trainer of catalog) trainers[trainer.id] = defaultTrainerSettings(trainer)
  return validateExperiment(
    {
      version: EXPERIMENT_VERSION,
      teamLevel: structuredClone(DEFAULT_TEAM_LEVEL),
      teamSize: structuredClone(DEFAULT_TEAM_SIZE),
      wildLevel: structuredClone(DEFAULT_WILD_LEVEL),
      routeTrainerLevel: structuredClone(DEFAULT_REGULAR_TRAINER_LEVEL),
      archetypes: structuredClone(DEFAULT_ARCHETYPE_GROWTH) as Experiment["archetypes"],
      trainers,
    },
    catalog,
    moves,
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
  const points = (value as unknown[]).map((point, index): Anchor => {
    const where = `${path}[${index}]`
    if (!Array.isArray(point) || point.length !== 2) fail(`${where} must be [TR, value]`)
    const [at, result] = point as unknown[]
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

const rosterSlot = (value: unknown, path: string): RosterSlot => {
  const input = object(value, path)
  if (Object.hasOwn(input, "moves"))
    fail(`${path}: roster slots carry no moves (members draw them from the move pool)`)
  exactKeys(input, ["species", "levelOffset", "isAce", "item", "ability", "nature"], path)
  if (typeof input.isAce !== "boolean") fail(`${path}.isAce must be true or false`)
  return {
    species: label(input.species, 100, `${path}.species`),
    levelOffset: integer(
      input.levelOffset,
      LEVEL_OFFSET.min,
      LEVEL_OFFSET.max,
      `${path}.levelOffset`,
    ),
    isAce: input.isAce as boolean,
    item: nullable(input.item, 60, `${path}.item`),
    ability: nullable(input.ability, 60, `${path}.ability`),
    nature: nullable(input.nature, 30, `${path}.nature`),
  }
}

/**
 * v0 rosters list up to six roster slots, and roster slot 1 is an ace at offset 0
 * so the signature Pokémon plays at the team level at every size and is fought
 * last. A roster has 1–3 aces. Fewer than six slots is a content gap the explorer
 * flags (see rosterGaps), not an import error.
 */
export const validateRoster = (value: unknown, path: string): RosterSlot[] => {
  if (!Array.isArray(value) || value.length < 1 || value.length > ROSTER_SIZE)
    fail(`${path} must list 1–${ROSTER_SIZE} Pokémon (v0 requires ${ROSTER_SIZE})`)
  const slots = (value as unknown[]).map((slot, index) => rosterSlot(slot, `${path}[${index + 1}]`))
  if (slots[0]?.levelOffset !== 0) fail(`${path}: roster slot 1 must have level offset 0`)
  if (!slots[0]?.isAce) fail(`${path}: roster slot 1 (the signature Pokémon) must be an ace`)
  if (slots.filter((slot) => slot.isAce).length > MAX_ACES)
    fail(`${path}: a roster has at most ${MAX_ACES} aces (roster slot 1 plus two more)`)
  return slots
}

/**
 * One ordered move pool: at most MAX_POOL_ENTRIES entries, each a move name (a valid move when
 * `moves` lists any) and an optional from level in 1–100.
 */
export const validateMovePool = (
  value: unknown,
  path: string,
  moves: ReadonlySet<string> = NO_LEARNSETS.moves,
): PoolEntry[] => {
  if (!Array.isArray(value) || value.length > MAX_POOL_ENTRIES)
    fail(`${path} must list at most ${MAX_POOL_ENTRIES} entries`)
  return (value as unknown[]).map((item, index): PoolEntry => {
    const where = `${path}[${index + 1}]`
    const input = object(item, where)
    exactKeys(input, Object.hasOwn(input, "fromLevel") ? ["move", "fromLevel"] : ["move"], where)
    const move = label(input.move, 40, `${where}.move`)
    if (moves.size && !moves.has(move)) fail(`${where}: unknown move "${move}"`)
    return Object.hasOwn(input, "fromLevel")
      ? { move, fromLevel: integer(input.fromLevel, 1, 100, `${where}.fromLevel`) }
      : { move }
  })
}

/** Start and peak TR are whole numbers with peak >= start, and peak = start for a Legend. */
const growthSettings = (
  settings: Record<string, unknown>,
  id: string,
): Omit<TrainerSettings, "roster" | "movePool"> => {
  const startTR = tr(settings.startTR, `${id}.startTR`)
  const peakTR = tr(settings.peakTR, `${id}.peakTR`)
  const archetype = settings.archetype as Archetype
  if (!ARCHETYPES.includes(archetype))
    fail(`${id}.archetype must be one of ${ARCHETYPES.join(", ")}`)
  if (peakTR < startTR) fail(`${id}: peak TR must be at least start TR`)
  if (archetype === "legend" && peakTR !== startTR)
    fail(`${id}: a Legend's peak TR must equal start TR`)
  return { startTR, archetype, peakTR }
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

export const EXPERIMENT_VERSION = 12
const START_OVER = `Start from the version ${EXPERIMENT_VERSION} defaults`
export const OLD_VERSION_REJECTION = (version: number): string =>
  version === 11
    ? `Version 11 experiments author moves per roster slot and have no move pools, so they cannot be imported. ${START_OVER}.`
    : version === 10
      ? `Version 10 experiments use the old archetype names (early bloomer, late bloomer, plateau, fixed, rising star, second wind, bursts), so they cannot be imported. ${START_OVER}.`
      : version === 9
        ? `Version 9 experiments have only five archetypes (no Legend, Star, Comeback or Burst step scaler) and the old archetype assignments, so they cannot be imported. ${START_OVER}.`
        : version === 8
          ? `Version 8 experiments have no ace slots (isAce) and fight the team simply reversed, so they cannot be imported. ${START_OVER}.`
          : version === 7
            ? `Version 7 experiments give the Rival a fixed lead, copy the level cap into team level and lack Tate & Liza, so they cannot be imported. ${START_OVER}.`
            : version === 6
              ? `Version 6 experiments give each notable trainer one fixed TR and cannot be imported. ${START_OVER} (start TR, archetype and peak TR).`
              : version === 5
                ? `Version 5 experiments use the retired 0–80 player TR scale and cannot be imported. ${START_OVER}.`
                : `Version ${version} experiments use a retired trainer model (standing, arcs or aces and fillers) and cannot be imported. ${START_OVER}.`

/** Validates an experiment against the catalog; with `moves`, every move pool name must be one of them. */
export const validateExperiment = (
  value: unknown,
  catalog: TrainerRecord[],
  moves: ReadonlySet<string> = NO_LEARNSETS.moves,
): Experiment => {
  const input = object(value, "root")
  if (typeof input.version === "number" && input.version >= 1 && input.version < EXPERIMENT_VERSION)
    fail(OLD_VERSION_REJECTION(input.version))
  if (input.version !== EXPERIMENT_VERSION) fail(`version must be ${EXPERIMENT_VERSION}`)
  exactKeys(
    input,
    [
      "version",
      "teamLevel",
      "teamSize",
      "wildLevel",
      "routeTrainerLevel",
      "archetypes",
      "trainers",
    ],
    "root",
  )
  const inputTrainers = object(input.trainers, "trainers")
  const ids = catalog.map((trainer) => trainer.id)
  if (new Set(ids).size !== ids.length) fail("catalog contains duplicate trainer IDs")
  exactKeys(inputTrainers, ids, "trainers")
  const trainers: Experiment["trainers"] = Object.create(null)
  for (const trainer of catalog) {
    const id = trainer.id
    const settings = object(inputTrainers[id], `trainers.${id}`)
    exactKeys(settings, ["startTR", "archetype", "peakTR", "roster", "movePool"], `trainers.${id}`)
    const growth = growthSettings(settings, id)
    if (growth.archetype === "legend" && isGymLeader(trainer))
      fail(`${id}: a Gym Leader cannot be a Legend`)
    trainers[id] = {
      ...growth,
      roster: validateRoster(settings.roster, `${id}.roster`),
      movePool: validateMovePool(settings.movePool, `${id}.movePool`, moves),
    }
  }
  const inputArchetypes = object(input.archetypes, "archetypes")
  exactKeys(inputArchetypes, ARCHETYPES, "archetypes")
  const archetypes = Object.fromEntries(
    // Monotonic non-decreasing 0–100%, whatever the kind.
    ARCHETYPES.map((name) => {
      const points = anchors(inputArchetypes[name], 0, 100, `${name} growth`)
      if (points[0]?.[1] !== 0) fail(`${name} growth must be 0% at world progress 0`)
      return [name, points]
    }),
  ) as Experiment["archetypes"]
  return {
    version: EXPERIMENT_VERSION,
    teamLevel: anchors(input.teamLevel, 1, 100, "teamLevel"),
    teamSize: anchors(input.teamSize, 1, ROSTER_SIZE, "teamSize"),
    wildLevel: anchors(input.wildLevel, 1, 100, "wildLevel"),
    routeTrainerLevel: anchors(input.routeTrainerLevel, 1, 100, "routeTrainerLevel"),
    archetypes,
    trainers,
  }
}

export const serializeExperiment = (experiment: Experiment): string =>
  JSON.stringify(experiment, null, 2)

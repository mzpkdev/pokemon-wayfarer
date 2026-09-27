export type ReferenceMember = {
  species: string
  level: number
  moves: string[]
  item: string | null
}

/** Growth arcs; engine.ts ARC_IDS holds their canonical (arc id) order. */
export type ArcId = "steady" | "early" | "late" | "plateau" | "rival"
/** arcDelta at progress index p = 0/8/16/24/32/40/48. */
export type ArcTuple = [number, number, number, number, number, number, number]

/** One species on an authored evolution line, reached at `level` (1 for the first). */
export type LineStage = { species: string; level: number }
/** An ace form: authored moves, held item, ability and nature for this species. */
export type AceForm = LineStage & {
  moves: string[]
  item: string | null
  ability: string | null
  nature: string | null
}
export type RosterAce = { id: string; line: AceForm[] }
export type RosterFiller = {
  id: string
  line: LineStage[]
  /** 0–100. */
  baseScore: number
  /** -6..0 from the strength level (where the top ace sits). */
  levelOffset: number
  /** The latest four level-up moves at its level, plus an optional signature move. */
  moves: "LEVEL_UP"
  signatureMove: string | null
  /** Excluded until this gameplay flag is set. */
  requiresFlag: string | null
}
/** A trainer's true potential: 1–3 aces in priority order and a filler pool. */
export type Roster = { prototype: boolean; aces: RosterAce[]; fillers: RosterFiller[] }

export type TrainerRecord = {
  id: string
  name: string
  region: "Kanto" | "Johto" | "Hoenn"
  role: "Gym Leader" | "Elite Four" | "Champion"
  /** Indigo/Hoenn home pools only; Sevii Masters is an open invitational. */
  homeLeagues: string[]
  source: { label: string; path: string; trainerId: string; note: string }
  referenceParty: ReferenceMember[]
  roster: Roster
  rosterSource: string
  bias: number
  allowedArcs: ArcId[]
}

export type TrainerSettings = {
  bias: number
  allowedArcs: ArcId[]
  arc: ArcId
  roster: Roster
}
/** Disjoint windows covering every integer standing. */
export type RoleWindows = { contenderMax: number; headlinerMin: number }
export type Role = "contender" | "elite" | "headliner"
/** sizeFor(strengthLevel): the size of the last row whose minLevel <= strengthLevel. */
export type SizeStep = { minLevel: number; size: number }
/** While `flag` is set, `delta` is added to the trainer's filler score. */
export type Modifier = { flag: string; trainer: string; filler: string; delta: number }
export type Experiment = {
  version: 4
  arcs: Record<ArcId, ArcTuple>
  roleWindows: RoleWindows
  /** levelBase = min(worldCap, 100 - headroom). */
  headroom: number
  sizeTable: SizeStep[]
  /** Filler jitter is uniform over 0..jitter. */
  jitter: number
  modifiers: Modifier[]
  trainers: Record<string, TrainerSettings>
}
/** Explorer save state that rosters read: the seed and the set gameplay flags. */
export type RosterContext = { seed: number; flags: readonly string[] }
/**
 * Canonical world facts. Completed circuit editions only count at 24 badges
 * and 3 first clears (a completed edition implies both); omitted means 0.
 */
export type WorldPoint = { badges: number; leagueClears: number; completedEditions?: number }
export type FillerScore = {
  filler: RosterFiller
  eligible: boolean
  jitter: number
  modifier: number
  score: number
  /** 1-based rank among eligible fillers (score desc, then fillerId). */
  rank: number | null
  inTeam: boolean
}
export type TeamMember = {
  kind: "ace" | "filler"
  id: string
  species: string
  level: number
  levelOffset: number
  /** Ace priority (1 = top ace). */
  priority?: number
  score?: number
  moves: string
}
export type ResolvedTrainer = {
  trainer: TrainerRecord
  arc: ArcId
  /** Progress index p = B + 8 * min(completedEditions, 3). */
  progress: number
  standing: number
  /** clamp(levelBase + standing, 1, 100): drives team size; the top ace sits here. */
  strengthLevel: number
  worldCap: number
  /** min(worldCap, 100 - headroom): the base that standing is added to. */
  levelBase: number
  /** strengthLevel - worldCap, after the 1–100 clamp. */
  gap: number
  role: Role
  /** sizeFor(strength level). */
  size: number
  /** Maximum aces at this size; unused ace slots go to fillers. */
  allowance: number
  acesUsed: number
  fillerScores: FillerScore[]
  /** Battle order: fillers by ascending score, then aces in reverse priority. */
  party: TeamMember[]
  warnings: string[]
}
export type VenueId = "Indigo" | "Hoenn" | "Sevii Masters"
export type RoleFeasibility = {
  role: Role
  need: number
  guaranteed: number
  possible: number
  current: number
  fallbackRisk: boolean
}
export type RosterGap = { id: string; name: string; reach: number }
export type VenueFeasibility = {
  venue: VenueId
  pool: "home" | "open"
  candidates: number
  /** Candidates whose roster cannot reach six members at max size. */
  gaps: RosterGap[]
  roles: RoleFeasibility[]
}

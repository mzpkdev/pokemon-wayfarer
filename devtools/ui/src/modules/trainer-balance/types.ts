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

export type TrainerRecord = {
  id: string
  name: string
  region: "Kanto" | "Johto" | "Hoenn"
  role: "Gym Leader" | "Elite Four" | "Champion"
  gymEligible: boolean
  /** Indigo/Hoenn home pools only; Sevii Masters is an open invitational. */
  homeLeagues: string[]
  source: { label: string; path: string; trainerId: string; note: string }
  referenceParty: ReferenceMember[]
  competitiveParty: ReferenceMember[]
  competitiveSource: string
  earlyParty: string[]
  bias: number
  allowedArcs: ArcId[]
}

export type PartyMember = { species: string; levelOffset: number }
export type TeamStage = { minBadges: number; label: string; party: PartyMember[] }
export type TrainerSettings = {
  bias: number
  allowedArcs: ArcId[]
  arc: ArcId
  stages: TeamStage[]
}
/** Disjoint windows covering every integer standing. */
export type RoleWindows = { contenderMax: number; headlinerMin: number }
export type Role = "contender" | "elite" | "headliner"
export type Experiment = {
  version: 3
  arcs: Record<ArcId, ArcTuple>
  roleWindows: RoleWindows
  /** levelBase = min(worldCap, 100 - headroom). */
  headroom: number
  trainers: Record<string, TrainerSettings>
}
/**
 * Canonical world facts. Completed circuit editions only count at 24 badges
 * and 3 first clears (a completed edition implies both); omitted means 0.
 */
export type WorldPoint = { badges: number; leagueClears: number; completedEditions?: number }
export type ResolvedTrainer = {
  trainer: TrainerRecord
  arc: ArcId
  /** Progress index p = B + 8 * min(completedEditions, 3). */
  progress: number
  standing: number
  aceLevel: number
  worldCap: number
  /** min(worldCap, 100 - headroom): the base that standing is added to. */
  levelBase: number
  /** aceLevel - worldCap, after the 1–100 clamp. */
  gap: number
  role: Role
  stage: string
  stageIndex: number
  party: { species: string; level: number; levelOffset: number }[]
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
export type VenueFeasibility = {
  venue: VenueId
  pool: "home" | "open"
  candidates: number
  excluded: string[]
  roles: RoleFeasibility[]
}

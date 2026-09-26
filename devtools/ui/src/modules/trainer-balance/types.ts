export type ReferenceMember = {
  species: string
  level: number
  moves: string[]
  item: string | null
}

export type TrainerRecord = {
  id: string
  name: string
  region: "Kanto" | "Johto" | "Hoenn"
  role: "Gym Leader" | "Elite Four" | "Champion"
  gymEligible: boolean
  homeLeagues: string[]
  source: { label: string; path: string; trainerId: string; note: string }
  referenceParty: ReferenceMember[]
  competitiveParty: ReferenceMember[]
  competitiveSource: string
  earlyParty: string[]
  suggestedStartTR: number
  suggestedMatureTR: number
  badgeTRCheckpoints?: [number, number, number, number]
}

export type PartyMember = { species: string; levelOffset: number }
export type TeamStage = { minTR: number; label: string; party: PartyMember[] }
export type TrainerSettings = {
  ratings: [number, number, number, number]
  leagueGrowth: number
  stages: TeamStage[]
}
export type LevelAnchor = [number, number]
export type Experiment = {
  version: 1
  levelAnchors: LevelAnchor[]
  trainers: Record<string, TrainerSettings>
}
export type WorldPoint = { badges: number; leagueClears: number }
export type ResolvedTrainer = {
  trainer: TrainerRecord
  tr: number
  unclampedTR: number
  aceLevel: number
  playerCap: number
  stage: string
  party: { species: string; level: number; levelOffset: number }[]
  warnings: string[]
}

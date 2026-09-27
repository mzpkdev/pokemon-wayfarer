export type ReferenceMember = {
  species: string
  level: number
  moves: string[]
  item: string | null
}

/** One roster slot. Entry 1 (the signature Pokémon) is at offset 0 and is fought last. */
export type RosterEntry = {
  species: string
  /** -6..0 from the team level. */
  levelOffset: number
  /** Authored moves (1–4), or the latest level-up moves at its level. */
  moves: string[] | "LEVEL_UP"
  item: string | null
  ability: string | null
  nature: string | null
}

export type TrainerRecord = {
  id: string
  name: string
  region: "Kanto" | "Johto" | "Hoenn"
  role: "Gym Leader" | "Elite Four" | "Champion"
  source: { label: string; path: string; trainerId: string; note: string }
  referenceParty: ReferenceMember[]
  /** Authored, fixed trainer rating: a non-negative integer with no upper limit. */
  tr: number
  trSource: string
  /** Exactly six ordered entries in v0; shorter catalog rosters are content gaps. */
  roster: RosterEntry[]
  rosterSource: string
}

export type TrainerSettings = { tr: number; roster: RosterEntry[] }
/** A scaler anchor: [TR, value]. */
export type Anchor = [number, number]
export type Experiment = {
  version: 6
  /** Team level by TR: linear between anchors, halves up, flat past the last. */
  teamLevel: Anchor[]
  /** Team size by TR, same rules; paired anchors make it a step table. */
  teamSize: Anchor[]
  /** Wild encounter level target by player TR. */
  wildLevel: Anchor[]
  /** Ordinary (route) trainer baseline level by player TR. */
  routeTrainerLevel: Anchor[]
  trainers: Record<string, TrainerSettings>
}
/** Player progress, used for the player TR and world-curve readout only. */
export type WorldPoint = { badges: number }
export type TeamMember = RosterEntry & {
  /** 1-based roster position. */
  slot: number
  level: number
}
export type ResolvedTrainer = {
  trainer: TrainerRecord
  tr: number
  teamLevel: number
  size: number
  /** The first `size` roster entries, in roster order. */
  team: TeamMember[]
  /** The team reversed: entry 1 comes last. */
  battleOrder: TeamMember[]
  /** Roster entries authored (v0 requires 6). */
  rosterLength: number
  warnings: string[]
}

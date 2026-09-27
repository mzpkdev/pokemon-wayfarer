export type ReferenceMember = {
  species: string
  level: number
  moves: string[]
  item: string | null
}

/** One roster slot. Roster slot 1 (the signature Pokémon) is at offset 0 and is fought last. */
export type RosterSlot = {
  species: string
  /** -6..0 from the team level. */
  levelOffset: number
  /** Authored moves (1–4), or the latest level-up moves at its level. */
  moves: string[] | "LEVEL_UP"
  item: string | null
  ability: string | null
  nature: string | null
}

/** Archetypes whose growth is a scaler over world progress. */
export type GrowthArchetype = "steady" | "early bloomer" | "late bloomer" | "plateau"
/** A notable trainer's growth shape; the rival follows world progress plus a lead. */
export type Archetype = GrowthArchetype | "rival"

export type TrainerRecord = {
  id: string
  name: string
  region: "Kanto" | "Johto" | "Hoenn"
  role: "Gym Leader" | "Elite Four" | "Champion"
  source: { label: string; path: string; trainerId: string; note: string }
  referenceParty: ReferenceMember[]
  /** Trainer TR at world progress 0: a non-negative integer with no upper limit. */
  startTR: number
  archetype: Archetype
  /** The highest TR this trainer can ever reach (not a scaler's ceiling TR). */
  peakTR: number
  /** Rival only: TR kept ahead of world progress. */
  lead: number | null
  trSource: string
  /** Exactly six ordered roster slots in v0; shorter catalog rosters are content gaps. */
  roster: RosterSlot[]
  rosterSource: string
}

export type TrainerSettings = {
  startTR: number
  archetype: Archetype
  peakTR: number
  lead: number | null
  roster: RosterSlot[]
}
/** A scaler anchor: [TR, value]. */
export type Anchor = [number, number]
export type Experiment = {
  version: 7
  /** Team level by TR: linear between anchors, halves up, flat past the last. */
  teamLevel: Anchor[]
  /** Team size by TR, same rules; paired anchors make it a step table. */
  teamSize: Anchor[]
  /** The wild level curve by player TR. */
  wildLevel: Anchor[]
  /** The regular trainer level curve by player TR (saved as `routeTrainerLevel`). */
  routeTrainerLevel: Anchor[]
  /** Growth % by world progress for each archetype except the rival. */
  archetypes: Record<GrowthArchetype, Anchor[]>
  trainers: Record<string, TrainerSettings>
}
/** Player progress. Its player TR is the world progress notable trainers grow with. */
export type WorldPoint = { badges: number }
export type TeamMember = RosterSlot & {
  /** 1-based roster position. */
  slot: number
  level: number
}
export type ResolvedTrainer = {
  trainer: TrainerRecord
  /** The world progress this trainer TR was read at. */
  worldProgress: number
  tr: number
  startTR: number
  archetype: Archetype
  peakTR: number
  lead: number | null
  teamLevel: number
  size: number
  /** The first `size` roster slots, in roster order. */
  team: TeamMember[]
  /** The team reversed: roster slot 1 comes last. */
  battleOrder: TeamMember[]
  /** Roster slots authored (v0 requires 6). */
  rosterLength: number
  warnings: string[]
}
/** Where a Gym Leader sits against the player TR: more than 10 below, within 10, or more than 10 above. */
export type LadderMark = "below" | "near" | "above"
export type LadderRow = { trainer: TrainerRecord; tr: number; gap: number; mark: LadderMark }

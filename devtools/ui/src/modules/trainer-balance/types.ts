export type ReferenceMember = {
  species: string
  level: number
  moves: string[]
  item: string | null
}

/**
 * One roster slot. Roster slot 1 (the signature Pokémon) is an ace at offset 0 and is fought
 * last. Every non-ace slot is a filler slot.
 */
export type RosterSlot = {
  /** The authored stage, normally a final stage; a member below its evolution level steps down. */
  species: string
  /** -6..0 from the team level. */
  levelOffset: number
  /** An ace slot: fought after the filler slots. Roster slot 1 is always an ace; 1–3 per roster. */
  isAce: boolean
  /** Authored moves (1–4), or the latest level-up moves at its level. */
  moves: string[] | "LEVEL_UP"
  item: string | null
  ability: string | null
  nature: string | null
}

/** A notable trainer's growth shape: a scaler over world progress giving growth %. */
export type Archetype =
  | "steady"
  | "early bloomer"
  | "late bloomer"
  | "plateau"
  | "rival"
  | "fixed"
  | "rising star"
  | "second wind"
  | "bursts"
/**
 * How a scaler reads between anchors: interpolated is linear with halves rounded up; step holds
 * each anchor's value until the next anchor.
 */
export type ScalerKind = "interpolated" | "step"

export type TrainerRecord = {
  id: string
  name: string
  region: "Kanto" | "Johto" | "Hoenn"
  /** A duo is two leaders sharing one entry, fought as a double battle. */
  role: "Gym Leader" | "Gym Leader duo" | "Elite Four" | "Champion"
  /** Fought as a double battle (the duo's Pokémon come from the shared roster in order). */
  doubleBattle: boolean
  /** In the global league pool. Leagues are singles only, so a duo is not. */
  leagueEligible: boolean
  source: { label: string; path: string; trainerId: string; note: string }
  referenceParty: ReferenceMember[]
  /** Trainer TR at world progress 0: a non-negative integer with no upper limit. */
  startTR: number
  archetype: Archetype
  /** The highest TR this trainer can ever reach (not a scaler's ceiling TR); start TR when fixed. */
  peakTR: number
  trSource: string
  /** Exactly six ordered roster slots in v0; shorter catalog rosters are content gaps. */
  roster: RosterSlot[]
  rosterSource: string
}

/** Evolution data the catalog records for the roster species. */
export type EvolutionData = {
  /**
   * Each roster species' predecessor chain, ascending: [base, level, stage 2, level, …, species].
   * Each level is the evolution level of the stage after it (level evolutions from species_info,
   * others from the shared evolution-level table).
   */
  chains: Record<string, (string | number)[]>
  /** Roster species that are not final stages (earlier chain stages are not final either). */
  notFinal: string[]
}
export type Catalog = { evolution: EvolutionData; trainers: TrainerRecord[] }

export type TrainerSettings = {
  startTR: number
  archetype: Archetype
  peakTR: number
  roster: RosterSlot[]
}
/** A scaler anchor: [TR, value]. */
export type Anchor = [number, number]
export type Experiment = {
  version: 10
  /** Team level by TR: linear between anchors, halves up, flat past the last. */
  teamLevel: Anchor[]
  /** Team size by TR, same rules; paired anchors make it a step table. */
  teamSize: Anchor[]
  /** The wild level curve by player TR. */
  wildLevel: Anchor[]
  /** The regular trainer level curve by player TR (saved as `routeTrainerLevel`). */
  routeTrainerLevel: Anchor[]
  /** Growth % by world progress for each archetype. */
  archetypes: Record<Archetype, Anchor[]>
  trainers: Record<string, TrainerSettings>
}
/** Player progress. Its player TR is the world progress notable trainers grow with. */
export type WorldPoint = { badges: number }
export type TeamMember = RosterSlot & {
  /** 1-based roster position. */
  slot: number
  level: number
  /** The roster slot's authored stage; `species` is the stage at this level. */
  authoredSpecies: string
  /** The level the authored stage is reached at, when `species` is an earlier stage; else null. */
  authoredAt: number | null
}
export type ResolvedTrainer = {
  trainer: TrainerRecord
  /** The world progress this trainer TR was read at. */
  worldProgress: number
  tr: number
  startTR: number
  archetype: Archetype
  peakTR: number
  teamLevel: number
  size: number
  /** The first `size` roster slots, in roster order. */
  team: TeamMember[]
  /** Filler slots in reverse roster order, then aces in reverse roster order: roster slot 1 comes last. */
  battleOrder: TeamMember[]
  /** Roster slots authored (v0 requires 6). */
  rosterLength: number
  warnings: string[]
}
/** Where a Gym Leader sits against the player TR: more than 10 below, within 10, or more than 10 above. */
export type LadderMark = "below" | "near" | "above"
export type LadderRow = { trainer: TrainerRecord; tr: number; gap: number; mark: LadderMark }
/** One change to a notable trainer's team as world progress rises. */
export type MilestoneEvent =
  /** The team at world progress 0, in roster order, and whether its team level is above the level cap. */
  | { kind: "start"; team: string[]; aboveCap: boolean }
  /** A roster slot joins the team (team size steps up), at the stage its level supports. */
  | { kind: "join"; slot: number; species: string; isAce: boolean }
  /** A team member's stage changes along its evolution line. */
  | { kind: "evolve"; slot: number; from: string; to: string }
  /** The team level moves strictly above the level cap, or strictly below it again. */
  | { kind: "cap"; aboveCap: boolean }
  /** Trainer TR reaches peak TR, or (when growth stops short of 100%) stops below it. */
  | { kind: "peak"; tr: number; reached: boolean }
/** The world progress (player TR) where a notable trainer's team changes, with what changed. */
export type Milestone = {
  worldProgress: number
  tr: number
  teamLevel: number
  cap: number
  events: MilestoneEvent[]
}

export type ReferenceMember = {
  species: string
  level: number
  moves: string[]
  item: string | null
}

/**
 * One roster slot. Roster slot 1 (the signature Pokémon) is an ace at offset 0 and is fought
 * last. Every non-ace slot is a filler slot. Roster slots carry no moves: members draw them from
 * the trainer's move pool.
 */
export type RosterSlot = {
  /** The authored stage, normally a final stage; a member below its evolution level steps down. */
  species: string
  /** -6..0 from the team level. */
  levelOffset: number
  /** An ace slot: fought after the filler slots. Roster slot 1 is always an ace; 1–3 per roster. */
  isAce: boolean
  item: string | null
  ability: string | null
  nature: string | null
}

/** A notable trainer's growth shape: a scaler over world progress giving growth %. */
export type Archetype =
  | "steady"
  | "prodigy"
  | "sleeper"
  | "veteran"
  | "rival"
  | "legend"
  | "star"
  | "comeback"
  | "burst"
/**
 * How a scaler reads between anchors: interpolated is linear with halves rounded up; step holds
 * each anchor's value until the next anchor.
 */
export type ScalerKind = "interpolated" | "step"

/**
 * One move pool entry: a move the trainer likes, and an optional from level. Without one, a
 * member takes it only if its current species or an earlier form of its line learns the move by
 * level-up, once it reaches the lowest such learn level; with one, any learner (that level-up,
 * its species' TM/tutor list, or its line's egg moves) takes it from that level. An entry goes
 * to at most one member; list a move twice to let two members carry it.
 */
export type PoolEntry = { move: string; fromLevel?: number }

/** A notable trainer's home region: a league location whose location regions include it counts the trainer at home. */
export type HomeRegion = "Kanto" | "Johto" | "Hoenn"
/**
 * A notable trainer's play style: their battle identity, exactly one per trainer. Each adds its AI
 * flags on top of the Basic bundle (Trainer AI).
 */
export type PlayStyle =
  | "gambler"
  | "bomber"
  | "sweeper"
  | "field_marshal"
  | "hexer"
  | "turtle"
  | "brawler"
  | "tactician"
/** An engine AI flag, by its display name (game/include/constants/battle_ai.h). */
export type AiFlag =
  | "Check Bad Move"
  | "Try To Faint"
  | "Check Viability"
  | "Force Setup First Turn"
  | "Risky"
  | "Try To 2HKO"
  | "Double Battle"
  | "HP Aware"
  | "Powerful Status"
  | "Will Suicide"
  | "Prefer Status Moves"
  | "Stall"
  | "Smart Switching"
  | "Ace Pokemon"
  | "Omniscient"
  | "Smart Mon Choices"
  | "Conservative"
  | "Double Ace Pokemon"
  | "Weigh Ability Prediction"
  | "Prefer Highest Damage Move"
  | "Predict Switch"
  | "Predict Incoming Mon"
  | "Predict Move"
  | "Assume STAB"
  | "Assume Status Moves"
/** The leagues, in the standard entry sequence. */
export type League = "Indigo" | "Sevii Masters" | "Hoenn"

export type TrainerRecord = {
  id: string
  name: string
  region: "Kanto" | "Johto" | "Hoenn"
  homeRegion: HomeRegion
  /**
   * Traits are opt-in yes/no flags, default no. Traveller: an away location costs a small travel
   * cost instead of the default (also reserved for a future overworld spawning rule).
   */
  traveller: boolean
  /** Aloof: won't join a league whose base lineup is well below their level (only the lineup rule reads it). */
  aloof: boolean
  /** The authored play style (Trainer AI). */
  playStyle: PlayStyle
  /** The boss flag: Omniscient AI, authored (Lance only in v0). */
  bossOmniscient: boolean
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
  /** The highest TR this trainer can ever reach (not a scaler's ceiling TR); start TR for a Legend. */
  peakTR: number
  trSource: string
  /** Exactly six ordered roster slots in v0; shorter catalog rosters are content gaps. */
  roster: RosterSlot[]
  rosterSource: string
  /** The trainer's one ordered move pool: top entries reach the aces first. */
  movePool: PoolEntry[]
  movePoolSource: string
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
/**
 * Learnsets for every stage on every roster line, as the Wayfarer ROM builds them. Moves are
 * indices into `moves`.
 */
export type LearnsetData = {
  /** Every valid move name, sorted. */
  moves: string[]
  species: Record<
    string,
    {
      /** Flattened [level, move, level, move, …] in the game's order; level 0 is an evolution move. */
      levelUp: number[]
      /** The TM/tutor (teachable) list. */
      teachable: number[]
      /**
       * On a line's first stage only: the egg moves of the species the line's Egg hatches as
       * (e.g. Pichu for Pikachu).
       */
      egg?: number[]
    }
  >
}
export type Catalog = {
  evolution: EvolutionData
  learnsets: LearnsetData
  trainers: TrainerRecord[]
}

export type TrainerSettings = {
  startTR: number
  archetype: Archetype
  peakTR: number
  roster: RosterSlot[]
  movePool: PoolEntry[]
  homeRegion: HomeRegion
  traveller: boolean
  aloof: boolean
  playStyle: PlayStyle
}
/** A scaler anchor: [TR, value]. */
export type Anchor = [number, number]
export type Experiment = {
  version: 17
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
/**
 * One resolved move: from the move pool (a level-up move a pool entry claims counts as a pool
 * move) or the default level-up moveset.
 */
export type ResolvedMove = { move: string; source: "pool" | "level-up" }
/**
 * Why a pool entry is dormant: no current member can learn it; it has no from level and current
 * members learn it only by TM/tutor, or only as an egg move; every eligible member is below its
 * from level (or, without one, its learn level); or every eligible member already has it or four
 * pool moves.
 */
export type DormantReason = "unlearnable" | "tm-only" | "egg" | "level" | "taken"
/** A pool entry at one team: the member that took it, or why it is dormant. */
export type PoolStatus = {
  /** 0-based position in the pool. */
  index: number
  move: string
  /** The entry's from level, or null when it waits for each member's learn level. */
  fromLevel: number | null
  /** The roster slot of the member that took it, or null when dormant. */
  slot: number | null
  species: string | null
  reason: DormantReason | null
  /** For a "level" entry, the lowest level an eligible member would need; else null. */
  waitLevel: number | null
  /**
   * For a "level" entry without a from level, the earlier form whose level-up learnset sets
   * waitLevel, when it is not the member's current species; else null.
   */
  waitForm: string | null
}
export type TeamMember = RosterSlot & {
  /** 1-based roster position. */
  slot: number
  level: number
  /** The roster slot's authored stage; `species` is the stage at this level. */
  authoredSpecies: string
  /** The level the authored stage is reached at, when `species` is an earlier stage; else null. */
  authoredAt: number | null
  /** Up to four moves in move-slot order, each from the pool or the level-up moveset. */
  moves: ResolvedMove[]
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
  /** Every move pool entry at this team, in pool order. */
  pool: PoolStatus[]
  /** The pool entries no member took. */
  dormant: PoolStatus[]
  /** Roster slots authored (v0 requires 6). */
  rosterLength: number
  /** The AI flags this battle writes, from play style, TR, the team's aces and the boss flag. */
  ai: ResolvedAi
  warnings: string[]
}
/** One AI skill tier: a step of the AI skill scaler over trainer TR. */
export type AiSkillTier = { tier: number; name: string; fromTR: number; flags: readonly AiFlag[] }
/**
 * A trainer's resolved AI flags for one battle: Basic, then the play style, the AI skill tier at
 * their TR, ace protection from the resolved team's aces, the boss flag, and the engine's
 * double-battle flag; in engine bit order.
 */
export type ResolvedAi = {
  playStyle: PlayStyle
  skill: AiSkillTier
  /** Aces in the resolved team (at least 1: roster slot 1 is an ace). */
  aces: number
  flags: AiFlag[]
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
  /** A dormant move pool entry is assigned for the first time. */
  | { kind: "wake"; move: string; slot: number; species: string }
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
/**
 * A trainer's willingness score at one league: 100 - travel cost - fatigue, at least 5. Travel
 * cost is 0 at home (or at the neutral location, the Sevii Masters); away it is 10 for a traveller
 * and 80 otherwise. Fatigue is 50 for a trainer in the previous league's lineup.
 */
export type Willingness = { home: boolean; travelCost: number; fatigue: number; score: number }
/** A league-eligible trainer at the world progress a league is entered at. */
export type LeagueCandidate = {
  trainer: TrainerRecord
  tr: number
  homeRegion: HomeRegion
  traveller: boolean
  aloof: boolean
  /** The team level at this TR. */
  teamLevel: number
  /** Catalog position: stands in for characterId when breaking ties. */
  order: number
}
/** A candidate scored for one league: willingness, league score floor(TR × willingness / 100), and rank. */
export type LeagueEntrant = LeagueCandidate & {
  willingness: Willingness
  score: number
  /**
   * False only for an aloof trainer whose team level is above the base lineup level + ALOOF_MARGIN
   * (or who has no base lineup to compare with): they skip this league.
   */
  joins: boolean
  /** 1 for the highest league score among those who join; ties by catalog order. Null when skipping. */
  rank: number | null
  inLineup: boolean
}
/**
 * One league entry: every eligible trainer ranked by league score, the base lineup level the aloof
 * are compared with, and the top five in battle order.
 */
export type LeagueRanking = {
  league: League
  /** World progress when the player enters. */
  world: number
  /**
   * The base lineup level: the strongest team level in the base lineup (the top five non-aloof
   * trainers by league score); null when there is no base lineup (no non-aloof trainer is eligible).
   */
  baseLineupLevel: number | null
  /** Every eligible trainer: those who join, highest league score first, then the aloof who skip. */
  entrants: LeagueEntrant[]
  /** The top five by league score, ascending TR, strongest last. */
  lineup: LeagueEntrant[]
}

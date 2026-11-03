// A league event's five, picked the way the game picks them
// (`league_selection.c`, `SelectLeagueLineup`): the notable trainers who can
// take part are scored by their Trainer Rating and how willing they are to
// travel there, the top five are seated, and they fight weakest first. The
// page replays that rule on the builder's roster, so the numbers are the
// game's; nothing in it is random, and the lineups change from event to event
// only because the last five are tired.
import type { LeagueId } from "./landmarks"

export type RosterTrainer = {
  id: number
  slug: string
  name: string
  role: string
  home: "Kanto" | "Johto" | "Hoenn"
  traveller: boolean
  aloof: boolean
  startTR: number
  peakTR: number
  archetype: string
  /** Their battle portrait, `leaders/<pic>.png` beside the data. */
  pic: string
}

/** A curve of anchors (rating, value), read by straight lines between them. */
type Anchors = [number, number][]

export type Roster = {
  trainers: RosterTrainer[]
  /** Per archetype, how far (in percent) a trainer has come from their start toward their peak. */
  growth: Record<string, { anchors: Anchors; step: boolean }>
  /** The team level for a Trainer Rating. */
  teamLevel: Anchors
}

/** One seat of a lineup, with what put the trainer there. */
export type Seat = {
  trainer: RosterTrainer
  /** Their Trainer Rating at the player's. */
  rating: number
  /** 100 at home, less away (90 for a traveller, 20 otherwise), less 50 if they fought last. */
  willingness: number
  score: number
}

/** Five battle one another's visitor, so five. */
export const LINEUP_SIZE = 5

/** `EvaluateTrainerScaler`: straight lines between anchors, rounded half up, or steps. */
const evaluate = (anchors: Anchors, rating: number, step: boolean) => {
  for (const [i, [tr, value]] of anchors.entries()) {
    if (i === 0 || rating >= tr) continue
    const [lowTr, lowValue] = anchors[i - 1]!
    if (step) return lowValue
    const width = tr - lowTr
    return lowValue + Math.floor((2 * (rating - lowTr) * (value - lowValue) + width) / (2 * width))
  }
  return anchors[anchors.length - 1]![1]
}

/** A trainer's Trainer Rating when the player's is `progress` (`GetNotableTrainerRating`). */
export const ratingAt = (roster: Roster, trainer: RosterTrainer, progress: number) => {
  const curve = roster.growth[trainer.archetype]
  const fraction = curve ? evaluate(curve.anchors, progress, curve.step) : 0
  return trainer.startTR + Math.floor(((trainer.peakTR - trainer.startTR) * fraction + 50) / 100)
}

/**
 * The five for one event at Indigo or Hoenn, in battle order (weakest first,
 * strongest in the Champion's Room). `recent` are the trainers of the most
 * recent event, who are less willing to come again.
 */
export const selectLineup = (
  roster: Roster,
  league: Exclude<LeagueId, "masters">,
  progress: number,
  recent: ReadonlySet<number>,
): Seat[] => {
  const candidates = roster.trainers
    .map((trainer): Seat & { level: number } => {
      const rating = ratingAt(roster, trainer, progress)
      const atHome = (trainer.home === "Hoenn") === (league === "hoenn")
      const travel = atHome ? 0 : trainer.traveller ? 10 : 80
      const fatigue = recent.has(trainer.id) ? 50 : 0
      const base = 100 - travel
      const willingness = base > fatigue + 5 ? base - fatigue : 5
      return {
        trainer,
        rating,
        willingness,
        score: Math.floor((rating * willingness) / 100),
        level: evaluate(roster.teamLevel, rating, false),
      }
    })
    .toSorted((a, b) => b.score - a.score || a.trainer.id - b.trainer.id)
  // Aloof trainers only join a league whose base five (the best of the rest)
  // are not far below their level.
  const base = candidates.filter(({ trainer }) => !trainer.aloof).slice(0, LINEUP_SIZE)
  const baseLevel = Math.max(0, ...base.map(({ level }) => level))
  return candidates
    .filter(({ trainer, level }) => !trainer.aloof || (base.length > 0 && level <= baseLevel + 10))
    .slice(0, LINEUP_SIZE)
    .toSorted((a, b) => a.rating - b.rating || a.trainer.id - b.trainer.id)
}

/** The events one after another, each with the one before it as its most recent. */
export const eventSequence = (
  roster: Roster,
  league: Exclude<LeagueId, "masters">,
  progress: number,
  events: number,
): Seat[][] => {
  const lineups: Seat[][] = []
  let recent: ReadonlySet<number> = new Set()
  for (let i = 0; i < events; i++) {
    const lineup = selectLineup(roster, league, progress, recent)
    lineups.push(lineup)
    recent = new Set(lineup.map(({ trainer }) => trainer.id))
  }
  return lineups
}

// One picture of the world: everything `drawShot` needs to draw a frame. The
// timeline makes one per frame from its scenes (`shotFor`); the studio makes
// one from its panel. Keeping them in one shape is what lets a shot composed
// by hand go into the timeline as it is.
import type { Camera } from "~/world-map/camera"
import type { LeagueId } from "~/world-map/landmarks"
import type { StartId } from "~/world-map/starts"

/** What is drawn over the map, each on or off. */
export type Layers = {
  sea: boolean
  weather: boolean
  clouds: boolean
  /** The ships, the train and the time of day's tint. */
  ships: boolean
  /** The night lights. */
  lights: boolean
  /** Gym Leaders and the Elite Four on their routines. */
  walkers: boolean
  /** Wild Pokémon popping out of the grass and water close in. */
  sightings: boolean
}

export const ALL_LAYERS: Layers = {
  sea: true,
  weather: true,
  clouds: true,
  ships: true,
  lights: true,
  walkers: true,
  sightings: false,
}

export type Pins = {
  /** How much of the pins show, 0 to 1. */
  alpha: number
  /** Milliseconds since the pins came up: they pop in one after another. */
  elapsed: number
  /** How much of the league circuit is drawn, 0 (pins only) to 1. */
  circuit: number
  /**
   * The badges earned so far, lit with a gold glow and a sparkle when fresh
   * (`earnedAt`, in `performance.now()` ms), the rest faint. Leave it out and
   * every pin is bright.
   */
  earned?: Set<number>
  earnedAt?: Map<number, number>
  /** The leagues calling: their leg of the circuit's trail lights. */
  calling?: Partial<Record<LeagueId, boolean>>
  league?: LeagueId | null
}

export type Shot = {
  camera: Camera
  /** The game minute: the time of day the ships, walkers, tint and lights follow. */
  minute: number
  /** Region and town names on the map. */
  labels: boolean
  layers: Layers
  /** How the walkers are drawn: all of them, or the named few picked out. */
  walkers?: { only?: string[]; marker?: boolean; minHeight?: number; standing?: boolean }
  pins?: Pins
  /** The daily roads on `day`. */
  daily?: { day: number }
  /** The wild Pokémon waves: when their cycle began (`performance.now()` ms), or a wave held still. */
  wild?: { start: number; still: number | null }
  /** The player riding in on their bike, `progress` of the way (0 to 1) along a start's route. */
  rider?: { start: StartId; progress: number }
  /** Black over everything, 0 to 1. */
  fade: number
}

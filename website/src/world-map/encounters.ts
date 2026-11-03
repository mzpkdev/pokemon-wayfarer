// The wild Pokémon of each map, as `build/encounters.mjs` writes them:
// `encounters.json` (species names, and per map its place name and encounter
// tables) and `icons.png` (one 32×32 icon per species, by atlas slot).
import { DATA } from "./data"

/** How a Pokémon is met: on foot, surfing, with Rock Smash, or fishing with a rod. */
export type Method = "land" | "surf" | "rock" | "old" | "good" | "super"

/** A species (its atlas slot) and the percent of encounters it makes up. */
export type Slot = [species: number, percent: number]

/** One method's table: one for any time (`a`), or one for the day (morning, day) and one for the night (evening, night). */
export type Table = { a: Slot[] } | { d: Slot[]; n: Slot[] }

export type Place = {
  /** The place's name in the game, e.g. "ROUTE 101". */
  n: string
  /** Its wild Pokémon, by method; missing where it has none. */
  e?: Partial<Record<Method, Table>>
  /** A sample of its land encounter cells (tall grass and the like), as flat x, y grid pairs. */
  l?: number[]
  /** A sample of its water encounter cells, as flat x, y grid pairs. */
  w?: number[]
}

export type Encounters = {
  /** Icons per row of `icons.png`. */
  columns: number
  /** Species names by atlas slot. */
  species: string[]
  /** Each species' generation by atlas slot: 1 to 9, or 0 for a regional form. */
  gens: number[]
  /** Each atlas slot's roster entry: its own slot, the plain species for a form like LYCANROC_MIDNIGHT, or -1 for none. */
  entry: number[]
  /** The page's roster by home region, in the tour's order: every species of Gen I to VIII and the regional forms. */
  homes: { id: string; slots: number[] }[]
  /** The three Safari Zones' Kalos species, and the placed map nearest each one's gate. */
  safari: { zone: string; near: string; slots: number[] }[]
  /** By the map's folder name, as in `Meta.maps`. */
  maps: Record<string, Place>
}

/** The icon's side in `icons.png`, in pixels. */
export const ICON = 32

const loading = new Map<string, Promise<Encounters>>()

/** Loads the encounters once per base path, the first time a place is picked. */
export const loadEncounters = (base = DATA): Promise<Encounters> => {
  let encounters = loading.get(base)
  if (!encounters) {
    encounters = fetch(`${base}/encounters.json`).then((response) => {
      if (!response.ok) throw new Error(`encounters.json: ${response.status}`)
      return response.json() as Promise<Encounters>
    })
    // A failed load can be tried again.
    encounters.catch(() => loading.delete(base))
    loading.set(base, encounters)
  }
  return encounters
}

// Where the tour can begin: a town in each of Hoenn, Johto and Kanto, with the
// player who rides in on their bike to their front door. Rides are set in each
// map's own tiles, so they follow the maps if the layout moves.
import type { Bounds, Meta } from "./data"

export type StartId = "hoenn" | "johto" | "kanto"

/** A point in a map's own tiles: the center of the tile at (`x` - 0.5, `y` - 0.5). */
type Waypoint = { map: string; x: number; y: number }

export type Start = {
  id: StartId
  /** The town's name as the game's map name popup shows it. */
  label: string
  region: string
  /** The player's name, for the ride's alt text and notes. */
  player: string
  /** The town's map (a name in `meta.maps`), and the point in it the close-up is centered on. */
  town: string
  focus: { x: number; y: number }
  /** The overworld bike sheet: nine 32x32 frames, standing then pedaling. */
  rider: string
  /**
   * The ride: in from the road, then to the tile in front of the player's
   * door. Every leg runs along one row or column of the world grid.
   */
  route: Waypoint[]
  /** The map the ride starts on, if it isn't the town's: its name and the popup's. */
  road?: { map: string; label: string }
  /** The opening shot, a box in one map's tiles (it may run past the map). */
  opening: { map: string } & Bounds
  /** Region names (`meta.regions`) the "region" stop fits. */
  regions: string[]
}

export const STARTS: Start[] = [
  {
    id: "hoenn",
    label: "LITTLEROOT TOWN",
    region: "HOENN",
    player: "BRENDAN",
    town: "LittlerootTown",
    focus: { x: 10, y: 10 },
    rider: "/game/sprites/brendan-mach-bike.png",
    // In from OLDALE TOWN's edge, down ROUTE 101's path (hopping its ledges),
    // into LITTLEROOT TOWN, and left to the door of the house on the left.
    route: [
      { map: "Route101", x: 10.5, y: -3.5 },
      { map: "LittlerootTown", x: 10.5, y: 9.5 },
      { map: "LittlerootTown", x: 5.5, y: 9.5 },
    ],
    road: { map: "Route101", label: "ROUTE 101" },
    // All of ROUTE 101, grass and all.
    opening: { map: "Route101", left: -2, top: -1, right: 22, bottom: 21 },
    regions: ["HOENN"],
  },
  {
    id: "johto",
    label: "NEW BARK TOWN",
    region: "JOHTO",
    player: "GOLD",
    town: "NewBarkTown_hns",
    focus: { x: 14, y: 16 },
    rider: "/game/sprites/gold-mach-bike.png",
    // Out of ROUTE 29's tall grass, up to the row that runs east through its
    // gap into town, and along the town's path to the front door of the house
    // at the right. ROUTE 29 ends at its column 70, NEW BARK TOWN's column 0
    // and five rows lower.
    route: [
      { map: "Route29_hns", x: 60.5, y: 19.5 },
      { map: "Route29_hns", x: 60.5, y: 17.5 },
      { map: "NewBarkTown_hns", x: 20.5, y: 12.5 },
    ],
    road: { map: "Route29_hns", label: "ROUTE 29" },
    opening: { map: "Route29_hns", left: 48, top: 8, right: 72, bottom: 28 },
    regions: ["JOHTO"],
  },
  {
    id: "kanto",
    label: "PALLET TOWN",
    region: "KANTO",
    player: "RED",
    town: "PalletTown_hns",
    focus: { x: 12, y: 10 },
    rider: "/game/sprites/red-mach-bike.png",
    // Down the road at the north edge between the two houses, and left along
    // the row in front of RED's house to its door.
    route: [
      { map: "PalletTown_hns", x: 12.5, y: 0.5 },
      { map: "PalletTown_hns", x: 12.5, y: 8.5 },
      { map: "PalletTown_hns", x: 6.5, y: 8.5 },
    ],
    opening: { map: "PalletTown_hns", left: 0, top: -4, right: 24, bottom: 18 },
    regions: ["KANTO"],
  },
]

/** The start the tour opens on. */
export const DEFAULT_START: StartId = "hoenn"

export const startOf = (id: StartId) => STARTS.find((start) => start.id === id)!

const mapIn = (meta: Meta, name: string) => meta.maps.find((map) => map.name === name)!

/** The ride's points on the world grid. */
export const routeIn = (meta: Meta, start: Start) =>
  start.route.map(({ map, x, y }) => {
    const placed = mapIn(meta, map)
    return { x: placed.x + x, y: placed.y + y }
  })

/** The opening shot's box on the world grid. */
export const openingIn = (meta: Meta, start: Start): Bounds => {
  const { map, left, top, right, bottom } = start.opening
  const placed = mapIn(meta, map)
  return {
    left: placed.x + left,
    top: placed.y + top,
    right: placed.x + right,
    bottom: placed.y + bottom,
  }
}

/** The close-up's center on the world grid. */
export const focusIn = (meta: Meta, start: Start) => {
  const placed = mapIn(meta, start.town)
  return { x: placed.x + start.focus.x, y: placed.y + start.focus.y }
}

/** The name the map name popup shows for a rider whose feet are at `feet` on the world grid. */
export const placeAt = (meta: Meta, start: Start, feet: { x: number; y: number }) => {
  const town = mapIn(meta, start.town)
  const inTown =
    feet.x >= town.x &&
    feet.x < town.x + town.width &&
    feet.y >= town.y &&
    feet.y < town.y + town.height + 1
  return start.road && !inTown ? start.road.label : start.label
}

/** How long a ride takes, in milliseconds: a steady pace, but never a blink. */
export const rideMs = (route: { x: number; y: number }[]) => {
  let length = 0
  for (let i = 1; i < route.length; i++)
    length += Math.hypot(route[i]!.x - route[i - 1]!.x, route[i]!.y - route[i - 1]!.y)
  return Math.max(2000, length * 85)
}

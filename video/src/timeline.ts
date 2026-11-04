// The storyboard's pictures: where the camera is in each scene, what time of
// day the world keeps, what is drawn over the map and what is laid over it.
// The words are in `script.ts`; each direction here is keyed by its scene.
import { DAY_START } from "~/world-map/living"
import type { StartId } from "~/world-map/starts"

import type { Layers } from "./shot"

/** Where the camera looks: resolved against the map's data once it has loaded. */
export type Target =
  | { kind: "world"; room?: number }
  /** A point on the world grid at a zoom: what the studio's COPY gives. */
  | { kind: "point"; x: number; y: number; zoom: number }
  /** The named regions (`meta.regions`), all fitted in the view. */
  | { kind: "regions"; names: string[]; room?: number }
  /** A point in a map's own tiles (its center when none), at a zoom in CSS pixels per tile. */
  | { kind: "map"; name: string; x?: number; y?: number; zoom: number }
  /** The 24 Gyms and the three leagues. */
  | { kind: "circuit"; room?: number }
  /** A Gym Leader or Elite Four member (their slug in `notables.json`), wherever the scene's clock has them. */
  | { kind: "notable"; slug: string; zoom: number }

/** The camera at `at` of the way through the scene (0 to 1); the camera flies between keyframes. */
export type Keyframe = { at: number; to: Target }

/** A span of a scene, as fractions of its length. */
export type Span = { from: number; to: number }

export type Direction = {
  shots: Keyframe[]
  /** The game clock, in game minutes: held, or run from one time to another across the scene. */
  clock: { at: number } | { from: number; to: number }
  /** Layers to turn off or on, over the default of everything but the sightings. */
  layers?: Partial<Layers>
  /** The wild Pokémon waves, popping out of the whole world from the scene's start. */
  wild?: boolean
  /**
   * The player rides in on their bike along a start's route over each span,
   * the camera coming in with them; between rides they wait at the door, and
   * the game's warp (a fade to black and back) cuts to the next start.
   */
  rides?: (Span & { start: StartId })[]
  /**
   * The badge pins, and how much of the league circuit is drawn (0 pins only,
   * 1 the whole circuit). With `earn`, the named badges are earned one after
   * another across that span, each lighting with a sparkle, and a league calls
   * once it has its eight, its leg of the trail lighting.
   */
  badges?: { circuit: number | [number, number]; earn?: Span & { badges: number[] } }
  /** The daily trainers and items on every road, with the day stepping from 1 to `days` across the scene. */
  daily?: { days: number }
  /** Gym Leaders and the Elite Four walking their routines: everyone, or the named few picked out. */
  notables?: { only?: string[]; marker?: boolean; minHeight?: number }
  /** A card over the map: the logo and lines of text. */
  title?: Span & { lines: string[]; logo?: boolean }
  /** A gameplay clip in a GBA frame beside the map (`clips/<clip>/`). */
  insert?: Span & { clip: string }
}

const h = (hours: number, minutes = 0) => hours * 60 + minutes

export const DIRECTIONS: Record<string, Direction> = {
  // The whole world, slowly closer, under the title.
  hook: {
    shots: [
      { at: 0, to: { kind: "world", room: 0.7 } },
      { at: 1, to: { kind: "world", room: 1.05 } },
    ],
    clock: { at: h(10) },
    wild: true,
    title: { from: 0, to: 0.45, lines: [], logo: true },
  },
  // RED, GOLD and BRENDAN each ride to their front door, a warp between.
  start: {
    shots: [{ at: 0, to: { kind: "map", name: "PalletTown_hns", x: 12, y: 10, zoom: 24 } }],
    clock: { at: h(9) },
    rides: [
      { start: "kanto", from: 0, to: 0.24 },
      { start: "johto", from: 0.33, to: 0.57 },
      { start: "hoenn", from: 0.66, to: 0.9 },
    ],
    insert: { from: 0.1, to: 0.95, clip: "oak-choice" },
  },
  // Out of Littleroot, where the last ride ended, to the whole world, each land's Pokémon popping out.
  lands: {
    shots: [
      { at: 0, to: { kind: "map", name: "LittlerootTown", x: 10, y: 10, zoom: 12 } },
      { at: 0.45, to: { kind: "regions", names: ["HOENN"], room: 0.9 } },
      { at: 1, to: { kind: "world", room: 1 } },
    ],
    clock: { at: h(10) },
    wild: true,
  },
  // The Aqua at Vermilion's pier, then out along the lanes to the Sevii Islands and Alola.
  ships: {
    shots: [
      { at: 0, to: { kind: "map", name: "VermilionCity_PortOutside_hns", zoom: 10 } },
      { at: 0.3, to: { kind: "map", name: "VermilionCity_PortOutside_hns", zoom: 10 } },
      { at: 0.65, to: { kind: "regions", names: ["KANTO", "SEVII ISLANDS"], room: 0.9 } },
      { at: 1, to: { kind: "regions", names: ["HOENN", "SEVII ISLANDS", "ALOLA"], room: 0.95 } },
    ],
    clock: { at: h(14) },
    insert: { from: 0.6, to: 0.95, clip: "gangway" },
  },
  // Dawn to the next dawn over Johto and Kanto, closing on Goldenrod as its lights come on.
  day: {
    shots: [
      { at: 0, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 1 } },
      { at: 0.45, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 1 } },
      { at: 0.8, to: { kind: "map", name: "GoldenrodCity_hns", zoom: 9 } },
      { at: 1, to: { kind: "map", name: "GoldenrodCity_hns", zoom: 9 } },
    ],
    clock: { from: h(5), to: h(29) },
  },
  // Early afternoon: Falkner training at Sprout Tower, Erika on her way to the department store.
  leaders: {
    shots: [
      { at: 0, to: { kind: "notable", slug: "falkner", zoom: 16 } },
      { at: 0.4, to: { kind: "notable", slug: "falkner", zoom: 16 } },
      { at: 0.7, to: { kind: "notable", slug: "erika", zoom: 16 } },
      { at: 1, to: { kind: "notable", slug: "erika", zoom: 16 } },
    ],
    clock: { at: h(13, 30) },
    notables: { marker: true, minHeight: 64 },
  },
  // Route 15 across four days: the trainers and items pop into new spots, or stay away.
  daily: {
    shots: [
      { at: 0, to: { kind: "map", name: "Route15_hns", x: 36, y: 10, zoom: 20 } },
      { at: 1, to: { kind: "map", name: "Route15_hns", x: 48, y: 10, zoom: 20 } },
    ],
    clock: { at: h(10) },
    daily: { days: 4 },
  },
  // Eight badges from three regions land one by one over Johto and Kanto; at the eighth the
  // Indigo League calls and the circuit comes up, its trail to the Masters lighting.
  gyms: {
    shots: [
      { at: 0, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.92 } },
      { at: 0.55, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.92 } },
      { at: 0.85, to: { kind: "circuit", room: 0.9 } },
      { at: 1, to: { kind: "circuit", room: 0.9 } },
    ],
    clock: { at: h(11) },
    badges: {
      circuit: [0, 1],
      earn: { from: 0.08, to: 0.6, badges: [0, 8, 2, 9, 1, 11, 3, 10] },
    },
    insert: { from: 0.62, to: 0.95, clip: "league-call" },
  },
  open: {
    shots: [
      { at: 0, to: { kind: "regions", names: ["KANTO"], room: 1 } },
      { at: 1, to: { kind: "world", room: 1 } },
    ],
    clock: { at: h(17) },
    title: { from: 0.25, to: 1, lines: ["Free. Made in the open.", "20.01.2027"], logo: true },
  },
}

/** The clock scenes without one keep: mid-morning, the map at its brightest. */
export const DEFAULT_MINUTES = DAY_START + 120

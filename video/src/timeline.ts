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
  /** A ship (`aqua`, `anne` or `tidal`), wherever the scene's clock has her: at her pier or out on her lane. */
  | { kind: "ship"; id: string; zoom: number }

/**
 * The camera at `at` of the way through the scene (0 to 1); the camera flies
 * between keyframes. `ease` shapes the flight into this keyframe: "both" (the
 * default) starts and ends gently, "in" only starts gently and arrives at
 * speed, "out" leaves at speed and settles, "none" keeps one pace, so a long
 * move can pass through a keyframe without stopping.
 */
export type Keyframe = { at: number; to: Target; ease?: "both" | "in" | "out" | "none" }

/** A span of a scene, as fractions of its length. */
export type Span = { from: number; to: number }

export type Direction = {
  shots: Keyframe[]
  /** A fade from black at the scene's start, and to black at its end, in seconds. */
  fade?: { in?: number; out?: number }
  /**
   * The game clock, in game minutes: held, run from one time to another across
   * the scene, or through `keys` of [fraction of the scene, minute], running
   * straight from each to the next and held before the first and after the last.
   */
  clock: { at: number } | { from: number; to: number } | { keys: [number, number][] }
  /** Layers to turn off or on, over the default of everything but the sightings. */
  layers?: Partial<Layers>
  /** The wild Pokémon waves, popping out of the whole world from the scene's start, or from `from` of the way through it. */
  wild?: boolean | { from: number }
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
  /**
   * Gym Leaders and the Elite Four walking their routines: everyone, or the named
   * few picked out. `minHeight` is their least height close in (zoom 16); further
   * out it shrinks back to the map's own, so a wide shot isn't crowded with them.
   */
  notables?: { only?: string[]; marker?: boolean; minHeight?: number }
  /** A card over the map: the logo and lines of text. */
  title?: Span & { lines: string[]; logo?: boolean }
  /** A gameplay clip in a GBA frame beside the map (`clips/<clip>/`). */
  insert?: Span & { clip: string }
}

const h = (hours: number, minutes = 0) => hours * 60 + minutes

/** Half a warp between rides, and the fade a scene cuts with, in seconds. */
const CUT = 0.35
/** Every land, fitted, which fills the frame better than the whole grid and its sea. */
const LANDS: Target = {
  kind: "regions",
  names: ["JOHTO", "KANTO", "HOENN", "SINJOH", "SINNOH", "SEVII ISLANDS", "ALOLA"],
  room: 0.95,
}

/** One Gym Leader where their routine has them at `minute`, picked out and held close. */
const vignette = (slug: string, minute: number): Direction => ({
  shots: [
    { at: 0, to: { kind: "notable", slug, zoom: 14 } },
    { at: 1, to: { kind: "notable", slug, zoom: 16 } },
  ],
  clock: { from: minute, to: minute + 10 },
  notables: { only: [slug], marker: true, minHeight: 64 },
  fade: { in: CUT, out: CUT },
})

export const DIRECTIONS: Record<string, Direction> = {
  // Cold open: RED rides in to his front door, the camera coming in with him.
  pallet: {
    shots: [{ at: 0, to: { kind: "map", name: "PalletTown_hns", x: 12, y: 10, zoom: 24 } }],
    clock: { at: h(9) },
    rides: [{ start: "kanto", from: 0, to: 0.7 }],
    fade: { in: 0.6, out: CUT },
  },
  // GOLD rides in to New Bark Town.
  newbark: {
    shots: [{ at: 0, to: { kind: "map", name: "NewBarkTown_hns", x: 14, y: 16, zoom: 24 } }],
    clock: { at: h(9) },
    rides: [{ start: "johto", from: 0, to: 0.8 }],
    fade: { in: CUT, out: CUT },
  },
  // BRENDAN rides in to Littleroot Town.
  littleroot: {
    shots: [{ at: 0, to: { kind: "map", name: "LittlerootTown", x: 10, y: 10, zoom: 24 } }],
    clock: { at: h(9) },
    rides: [{ start: "hoenn", from: 0, to: 0.8 }],
    fade: { in: CUT, out: CUT },
  },
  // The Anne at Vermilion's pier in the evening; she casts off at half past eight and
  // the camera rides out of the bay with her, drawing back only a little.
  anne: {
    shots: [
      { at: 0, to: { kind: "ship", id: "anne", zoom: 24 } },
      { at: 0.3, to: { kind: "ship", id: "anne", zoom: 24 } },
      { at: 1, to: { kind: "ship", id: "anne", zoom: 16 }, ease: "in" },
    ],
    clock: { from: h(20, 18), to: h(21, 5) },
    fade: { in: CUT },
  },
  // Still with the Anne while the box answers where she was going, and that she
  // won't wait, as she steams off without you; then, on "And
  // Kanto was never the whole map", one pull-out in the evening light: Kanto and
  // Johto with their Pokémon popping out once they fill the frame, then every land.
  lands: {
    // One move: the camera keeps riding with her, slows through Kanto and Johto without
    // stopping while their Pokémon pop, then goes on at speed and settles on every land.
    shots: [
      { at: 0, to: { kind: "ship", id: "anne", zoom: 16 } },
      { at: 0.33, to: { kind: "ship", id: "anne", zoom: 12 }, ease: "none" },
      { at: 0.56, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.8 }, ease: "in" },
      { at: 0.85, to: LANDS, ease: "out" },
      { at: 1, to: LANDS },
    ],
    clock: { from: h(21, 5), to: h(23, 30) },
    wild: { from: 0.5 },
  },
  // Straight on from every land at night: every Gym Leader and Elite Four member's marker
  // lights up, then one dive onto Brock while the night and the next morning go by; he is
  // walking south out of Pewter, all the way to the Day-Care on Route 34.
  leaders: {
    shots: [
      { at: 0, to: LANDS },
      { at: 0.14, to: LANDS },
      { at: 0.55, to: { kind: "notable", slug: "brock", zoom: 16 } },
      { at: 1, to: { kind: "notable", slug: "brock", zoom: 16 } },
    ],
    // The night goes by before the camera is close; then he walks at about a walk's pace.
    clock: {
      keys: [
        [0.12, h(23, 30)],
        [0.46, h(34)],
        [1, h(35)],
      ],
    },
    notables: { marker: true, minHeight: 64 },
    fade: { out: CUT },
  },
  // Three more lives, a warp between each: Lt. Surge at the Game Corner in the afternoon,
  // Wallace fishing Route 119 (where Feebas is) at noon, Giovanni at the Burned Tower at night.
  surge: vignette("lt-surge", h(16)),
  wallace: vignette("wallace", h(12, 30)),
  giovanni: vignette("giovanni", h(0, 30)),
  // Evening into night over Johto and Kanto, closing on Goldenrod as its lights come on.
  night: {
    shots: [
      { at: 0, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 1 } },
      { at: 0.4, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 1 } },
      { at: 0.8, to: { kind: "map", name: "GoldenrodCity_hns", zoom: 9 } },
      { at: 1, to: { kind: "map", name: "GoldenrodCity_hns", zoom: 9 } },
    ],
    clock: { from: h(18), to: h(26) },
    fade: { in: CUT },
  },
  // Route 15 across four days: the trainers and items pop into new spots, or stay away.
  daily: {
    shots: [
      { at: 0, to: { kind: "map", name: "Route15_hns", x: 36, y: 10, zoom: 20 } },
      { at: 1, to: { kind: "map", name: "Route15_hns", x: 48, y: 10, zoom: 20 } },
    ],
    clock: { at: h(10) },
    daily: { days: 4 },
    fade: { in: CUT },
  },
  // Eight badges from three regions land one by one over Johto and Kanto; at the eighth the
  // Indigo League calls, and the circuit comes up as the box asks who's in the Elite Four.
  gyms: {
    shots: [
      { at: 0, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.92 } },
      { at: 0.4, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.92 } },
      { at: 0.62, to: { kind: "circuit", room: 0.9 } },
      { at: 1, to: { kind: "circuit", room: 0.9 } },
    ],
    clock: { at: h(11) },
    badges: {
      circuit: [0, 1],
      earn: { from: 0.06, to: 0.36, badges: [0, 8, 2, 9, 1, 11, 3, 10] },
    },
  },
  open: {
    shots: [
      { at: 0, to: { kind: "regions", names: ["KANTO"], room: 1 } },
      { at: 1, to: LANDS },
    ],
    clock: { at: h(17) },
    title: { from: 0.25, to: 1, lines: ["Free. Made in the open.", "20.01.2027"], logo: true },
  },
}

/** The clock scenes without one keep: mid-morning, the map at its brightest. */
export const DEFAULT_MINUTES = DAY_START + 120

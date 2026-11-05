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
  /** `dx` and `dy` move the camera off them, in tiles, to keep a seam of the map out of the frame. */
  | { kind: "notable"; slug: string; zoom: number; dx?: number; dy?: number }
  /** A ship (`aqua`, `anne` or `tidal`), wherever the scene's clock has her: at her pier or out on her lane. */
  | { kind: "ship"; id: string; zoom: number }

/**
 * A moment of a scene: a fraction of the way through it (0 to 1), or a phrase
 * of its narration, when the voice starts it (or the box, without a voice).
 * A phrase holds the picture to the words however long the voice takes.
 */
export type Moment = number | string

/**
 * The camera at the moment `at` of the scene; the camera flies
 * between keyframes. `ease` shapes the flight into this keyframe: "both" (the
 * default) starts and ends gently, "in" only starts gently and arrives at
 * speed, "out" leaves at speed and settles, "none" keeps one pace, so a long
 * move can pass through a keyframe without stopping.
 */
export type Keyframe<M = Moment> = { at: M; to: Target; ease?: "both" | "in" | "out" | "none" }

/** A span of a scene, between two moments. */
export type Span<M = Moment> = { from: M; to: M }

/** A scene's pictures; `Direction<number>` once its moments are fractions of the scene. */
export type Direction<M = Moment> = {
  shots: Keyframe<M>[]
  /** A fade from black at the scene's start, and to black at its end, in seconds. */
  fade?: { in?: number; out?: number }
  /**
   * The game clock, in game minutes: held, run from one time to another across
   * the scene, or through `keys` of [fraction of the scene, minute], running
   * straight from each to the next and held before the first and after the last.
   */
  clock: { at: number } | { from: number; to: number } | { keys: [number, number][] }
  /**
   * The ships and the train held at this game minute while the clock is elsewhere:
   * a scene that jumps in time at its cut keeps them where the last scene left them.
   */
  shipsAt?: number
  /** Layers to turn off or on, over the default of everything but the sightings. */
  layers?: Partial<Layers>
  /**
   * The wild Pokémon waves, popping out of the whole world from the scene's start, or from
   * the moment `from`. They pop one home after another (`HOMES` in the website's `wild.ts`);
   * `wave` times the loop so the named home's wave (its `id`) begins at its moment.
   */
  wild?: boolean | { from: M; wave?: { home: string; at: M } }
  /**
   * The player rides in on their bike along a start's route over each span,
   * the camera coming in with them; between rides they wait at the door, and
   * the game's warp (a fade to black and back) cuts to the next start. `look`
   * moves where the camera ends up off the door, in tiles.
   */
  rides?: (Span<M> & { start: StartId; look?: { dx: number; dy: number } })[]
  /**
   * The badge pins, and how much of the league circuit is drawn (0 pins only,
   * 1 the whole circuit). With `earn`, the named badges are earned one after
   * another across that span, each lighting with a sparkle, and a league calls
   * once it has its eight, its leg of the trail lighting.
   */
  badges?: { circuit: number | [number, number]; earn?: Span<M> & { badges: number[] } }
  /** The daily trainers and items on every road, with the day stepping from 1 to `days` across the scene. */
  daily?: { days: number } | { day: number }
  /** The game's yellow arrow over a daily trainer standing at this world cell, to pick them out. */
  mark?: { x: number; y: number }
  /**
   * Gym Leaders and the Elite Four walking their routines: everyone, or the named
   * few picked out. `minHeight` is their least height close in (zoom 16); further
   * out it shrinks back to the map's own, so a wide shot isn't crowded with them.
   */
  notables?: { only?: string[]; marker?: boolean; minHeight?: number }
  /** A card over the map: the logo and lines of text. */
  title?: Span<M> & { lines: string[]; logo?: boolean }
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

/** Brock at the Day-Care, framed with the sea to his west. */
const BROCK: Target = { kind: "notable", slug: "brock", zoom: 32, dx: -15 }

/** The Sevii Islands, Galar's land, all of the chain in view. */
const SEVII: Target = { kind: "regions", names: ["SEVII ISLANDS"], room: 0.9 }

/** One Gym Leader where their routine has them at `minute`, picked out and held close. */
const vignette = (
  slug: string,
  minute: number,
  { zoom = [14, 16], dx = 0 }: { zoom?: [number, number]; dx?: number } = {},
): Direction => ({
  shots: [
    { at: 0, to: { kind: "notable", slug, zoom: zoom[0], dx } },
    { at: 1, to: { kind: "notable", slug, zoom: zoom[1], dx } },
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
    // Ending a little south of the door, with Pallet in the frame's upper part above the box,
    // keeps the seam where Route 1's forest meets Pallet's out of the top of the frame.
    rides: [{ start: "kanto", from: 0, to: 0.7, look: { dx: 0, dy: 9 } }],
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
  // Still with the Anne while the box answers where she was going; as it says she keeps her
  // own hours the camera lets her steam off without you, one pull-out in the evening light:
  // through Kanto and Johto as the box says Kanto was never the whole map, their Pokémon
  // popping out, and settling on every land on "Seven lands", held there for the rest.
  lands: {
    // One move: the camera keeps riding with her, slows through Kanto and Johto without
    // stopping while their Pokémon pop, then goes on at speed and settles on every land.
    shots: [
      { at: 0, to: { kind: "ship", id: "anne", zoom: 16 } },
      { at: "Keeps her", to: { kind: "ship", id: "anne", zoom: 12 }, ease: "none" },
      {
        at: "the whole map",
        to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.8 },
        ease: "in",
      },
      // Settled by "Walk every", so it reads as there on "Seven lands".
      { at: "Walk every", to: LANDS, ease: "out" },
      // Over to the Sevii Islands while the box passes on what folk there say, their
      // Galar Pokémon popping, and back out to every land for Kalos, far off.
      { at: "Folk on", to: LANDS },
      { at: "Ask the Wooloo", to: SEVII },
      { at: "Never been", to: SEVII },
      { at: "Too far", to: LANDS },
      { at: 1, to: LANDS },
    ],
    clock: { from: h(21, 5), to: h(23, 30) },
    // Popping from the pull-out on, timed so Galar's wave pops on the Sevii Islands as the box
    // passes on what folk there say, still popping on the punchline.
    wild: { from: "And Kanto", wave: { home: "sevii", at: "Galar now" } },
  },
  // Straight on from every land at night: every Gym Leader and Elite Four member's marker
  // lights up, then one dive onto Brock, landing as the box names him, on the morning
  // two days on that he's caring for the Pokémon at the Day-Care on Route 34, in Johto.
  leaders: {
    shots: [
      { at: 0, to: LANDS },
      { at: "Gym Leaders do", to: LANDS },
      // Landing close, the camera west of him over the sea, so Goldenrod's cut-off edge
      // to his east stays out of frame.
      { at: "Brock's helping", to: BROCK },
      { at: 1, to: BROCK },
    ],
    // Two days on from the Anne, cut on as the markers light up, so nobody races to get there;
    // the ships stay where the last scene left them.
    clock: { from: h(82), to: h(82, 15) },
    shipsAt: h(23, 30),
    notables: { marker: true, minHeight: 64 },
    fade: { out: CUT },
  },
  // Two more lives, a warp between each: Lt. Surge at the Game Corner in the afternoon,
  // and Wallace fishing Route 119 at noon, where Feebas hides on a few tiles of the river.
  surge: vignette("lt-surge", h(16)),
  // Closer, and a few tiles west of him, so a pale strip of river fill stays out of frame.
  wallace: vignette("wallace", h(12, 30), { zoom: [22, 25], dx: -4 }),
  // Route 29 off the road, deep into the night, the wild Pokémon of the night popping in the grass.
  night: {
    shots: [
      { at: 0, to: { kind: "map", name: "Route29_hns", x: 30, y: 14, zoom: 18 } },
      { at: 1, to: { kind: "map", name: "Route29_hns", x: 38, y: 14, zoom: 20 } },
    ],
    // The game's darkest tint runs from 1:00 to 4:00.
    clock: { from: h(25), to: h(27, 30) },
    // The one scene about the time of day: the tint and the lights are on.
    layers: { tint: true, lights: true, sightings: true },
    fade: { in: CUT, out: CUT },
  },
  // Bug Catcher Don, the first trainer most GSC players fought, on Route 30 on day 10 of the
  // daily draw; two days on he stands in Ilex Forest (the draw puts him there on day 12).
  don: {
    shots: [
      { at: 0, to: { kind: "point", x: 720.5, y: 1102, zoom: 20 } },
      { at: 1, to: { kind: "point", x: 720.5, y: 1102, zoom: 22 } },
    ],
    clock: { at: h(10) },
    daily: { day: 10 },
    mark: { x: 720, y: 1103 },
    fade: { in: CUT, out: CUT },
  },
  donagain: {
    shots: [
      { at: 0, to: { kind: "point", x: 577.5, y: 1253, zoom: 20 } },
      { at: 1, to: { kind: "point", x: 577.5, y: 1253, zoom: 22 } },
    ],
    clock: { at: h(10) },
    daily: { day: 12 },
    mark: { x: 577, y: 1254 },
    fade: { in: CUT },
  },
  // Eight badges from three regions land one by one over Johto and Kanto while word of you
  // gets around; the eighth lands as the phone rings and the Indigo League calls, and the
  // circuit comes up as the box asks who's in the Elite Four.
  gyms: {
    shots: [
      { at: 0, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.92 } },
      { at: "Then the phone", to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.92 } },
      { at: "And who's in", to: { kind: "circuit", room: 0.9 } },
      { at: 1, to: { kind: "circuit", room: 0.9 } },
    ],
    clock: { at: h(11) },
    badges: {
      circuit: [0, 1],
      // Clair's Rising Badge first, as the box names her: the one GSC players remember as the wall.
      earn: { from: "Fancy", to: "Then the phone", badges: [7, 8, 2, 9, 1, 11, 3, 10] },
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

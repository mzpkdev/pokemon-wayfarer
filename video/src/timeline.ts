// The storyboard's pictures: where the camera is in each scene, what time of
// day the world keeps, what is drawn over the map and what is laid over it.
// The words are in `script.ts`; each direction here is keyed by its scene.
import type { Bounds } from "~/world-map/data"
import { DAY_START } from "~/world-map/living"
import type { StartId } from "~/world-map/starts"

import type { Actor, Toss } from "./actors"
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
 * `{ on, plus }` is `plus` seconds after the moment `on` (before it, when negative).
 */
export type Moment = number | string | { on: Moment; plus: number }

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
   * moves where the camera ends up off the door, in tiles. With `track` the
   * camera stays close on the rider instead, as the game's does, at a zoom (or
   * from one to another across the ride) and never looking outside `within`,
   * a box in the town's own tiles: what lies past it has seams to keep out of frame.
   */
  rides?: (Span<M> & {
    start: StartId
    look?: { dx: number; dy: number }
    track?: { zoom: number | [number, number]; within: Bounds }
    /** Tiles of the route already ridden when the ride begins: it picks up that far along. */
    skip?: number
  })[]
  /**
   * The badge pins, and how much of the league circuit is drawn (0 pins only,
   * 1 the whole circuit). With `earn`, the named badges are earned one after
   * another across that span, each lighting with a sparkle, and a league calls
   * once it has its eight, its leg of the trail lighting.
   */
  badges?: { circuit: number | [number, number]; earn?: Span<M> & { badges: number[] } }
  /** The daily trainers and items on every road, with the day stepping from 1 to `days` across the scene. */
  daily?: { days: number } | { day: number }
  /**
   * Gym Leaders and the Elite Four walking their routines: everyone, or the named
   * few picked out. `minHeight` is their least height close in (zoom 16); further
   * out it shrinks back to the map's own, so a wide shot isn't crowded with them.
   */
  notables?: { only?: string[]; except?: string[]; marker?: boolean; minHeight?: number }
  /** A card over the map: the logo and lines of text. */
  title?: Span<M> & { lines: string[]; logo?: boolean }
  /**
   * The scene's actors: the player, people and Pokémon walking the map and
   * the bubbles over their heads (`actors.ts`), and things thrown between them.
   */
  cast?: Actor<M>[]
  tosses?: Toss<M>[]
  /** The phone ringing over the picture for a span; the caller's name comes up at `named`. */
  phone?: Span<M> & { caller: string; named: M }
  /**
   * The league's five picked like a slot machine, as the website's tour shows it: from a
   * moment on the map dims and five seats shuffle through every trainer's face, each
   * stopping on its own (by their slug in the roster) at its moment, in battle order. Then
   * the `star` steps forward at `grow`, the rest falling away, and a partner's picture
   * slides in beside them.
   */
  lineup?: {
    from: M
    seats: { slug: string; at: M }[]
    star: string
    grow: M
    partner: { src: string; at: M }
  }
  /** White flashes at these moments, as the game flashes into a battle. */
  flashes?: M[]
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

/** Brock at the Day-Care, close: four pixels a pixel, 15 tiles either side of him. */
const BROCK: Target = { kind: "notable", slug: "brock", zoom: 64 }

/** The Sevii Islands, Galar's land, all of the chain in view. */
const SEVII: Target = { kind: "regions", names: ["SEVII ISLANDS"], room: 0.9 }

/** Melemele's beach, where the Anne lands in Alola: the sand, its palms and the water she comes in by. */
const MELEMELE = "MelemeleIsle_hns"
const MELEMELE_BEACH: Target = { kind: "map", name: MELEMELE, x: 37.5, y: 9.5, zoom: 64 }

/** The tug on the line, a beat after the box says to fish; and setting off on what was caught. */
const BITE: Moment = { on: "Something", plus: -0.5 }
const RIDE: Moment = { on: BITE, plus: 2.1 }
/** Off the beach and away east, clear of the palm on the next islet. */
const RIDE_MARKS = [
  { at: RIDE, x: 40, y: 10 },
  { at: { on: RIDE, plus: 0.5 }, x: 40, y: 10 },
  { at: { on: RIDE, plus: 1 }, x: 42, y: 9 },
  { at: 1, x: 54, y: 9 },
]

/** When you notice the Anne leaving without you: as the box says she keeps her own hours. */
const NOTICED = { on: "hours", plus: 0.2 }

/** Five Isle Meadow, close, where the Wooloo graze. */
const MEADOW: Target = { kind: "map", name: "FiveIsland_Meadow_Frlg", x: 12.5, y: 27, zoom: 64 }
/** The flock: where each stands, the step it takes, and the way it faces first. */
const WOOLOO: [number, number, number, number, "south" | "west" | "east" | "north"][] = [
  [11, 23, 1, 0, "east"],
  [13, 24, 0, 1, "south"],
  [12, 26, 0, 0, "south"],
  [14, 26, -1, 0, "west"],
  [11, 28, 1, 0, "east"],
  [13, 29, 0, -1, "north"],
]

/** You, walking up to Clair across the grass by the Dragon's Den. */
const CLAIR_CAST: Direction["cast"] = [
  {
    sprite: "gold",
    map: "BlackthornCity_hns",
    marks: [
      { at: 0, x: 23, y: 16 },
      { at: "Fancy", x: 23, y: 16 },
      { at: "Go on", x: 29, y: 16 },
    ],
  },
  // Clair, standing by the water until you reach her; then she turns to you.
  {
    sprite: "clair",
    map: "BlackthornCity_hns",
    marks: [{ at: 0, x: 30, y: 16 }],
    face: [
      { at: 0, to: "south" },
      { at: { on: "Go on", plus: 0.15 }, to: "west" },
    ],
    emotes: [{ at: { on: "Go on", plus: 0.15 }, kind: "exclamation", seconds: 1.2 }],
  },
]

export const DIRECTIONS: Record<string, Direction> = {
  // Cold open: RED rides in to his front door, the camera coming in with him.
  pallet: {
    shots: [{ at: 0, to: { kind: "map", name: "PalletTown_hns", x: 12, y: 10, zoom: 24 } }],
    clock: { at: h(9) },
    // Close on him all the way, four pixels a pixel. Past the town's own tiles the map has
    // seams: Route 1's forest to the north, and to the east the forest that fills the gap to
    // the coast. The forest to the west is clean.
    rides: [
      {
        start: "kanto",
        from: 0,
        to: 0.55,
        track: { zoom: 64, within: { left: -9, top: 0.5, right: 23, bottom: 19.5 } },
      },
    ],
    // Blue is one of the cast here, standing outside the lab: the map's own stands on its
    // door. He looks round as you reach yours.
    notables: { except: ["blue"] },
    cast: [
      {
        sprite: "blue",
        map: "PalletTown_hns",
        marks: [{ at: 0, x: 16, y: 14 }],
        face: [
          { at: 0, to: "south" },
          { at: 0.57, to: "west" },
        ],
      },
    ],
    fade: { in: 0.4, out: CUT },
  },
  // GOLD rides in to New Bark Town.
  newbark: {
    shots: [{ at: 0, to: { kind: "map", name: "NewBarkTown_hns", x: 14, y: 16, zoom: 24 } }],
    clock: { at: h(9) },
    // Close on him from Route 29's grass to his door, clear of the cliff north of town
    // and of the open sea past the pond to the east.
    rides: [
      {
        start: "johto",
        from: 0,
        to: 0.8,
        track: { zoom: 64, within: { left: -40, top: 1, right: 33.5, bottom: 38 } },
      },
    ],
    fade: { in: CUT, out: CUT },
  },
  // BRENDAN rides in to Littleroot Town, close on him like the other two. The ride picks up
  // at the foot of Route 101, below its last ledge, and the camera never looks north of there:
  // a bike going down over ledges is not a thing the game lets you do.
  littleroot: {
    shots: [{ at: 0, to: { kind: "map", name: "LittlerootTown", x: 10, y: 10, zoom: 24 } }],
    clock: { at: h(9) },
    rides: [
      {
        start: "hoenn",
        from: 0,
        to: 0.8,
        skip: 18,
        track: { zoom: 64, within: { left: -14, top: -5.9, right: 34, bottom: 20 } },
      },
    ],
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
  // The answer, shown: morning at Melemele, and the Anne steams in to the beach she lands at.
  // You step off onto the sand, and the palm you walk past turns to look at you.
  alola: {
    shots: [
      { at: 0, to: MELEMELE_BEACH },
      { at: 1, to: MELEMELE_BEACH },
    ],
    // She is due at nine the morning after she left Vermilion, and is in as the box goes on.
    clock: {
      keys: [
        [0, h(32, 51)],
        [0.36, h(33)],
        [1, h(33, 6)],
      ],
    },
    cast: [
      {
        sprite: "exeggutor-alola",
        map: MELEMELE,
        marks: [{ at: 0, x: 34, y: 10 }],
        face: [
          { at: 0, to: "south" },
          { at: { on: "sail with her", plus: 0.5 }, to: "east" },
        ],
      },
      {
        sprite: "gold",
        map: MELEMELE,
        from: "And now",
        marks: [
          { at: "And now", x: 39, y: 10 },
          { at: { on: "And now", plus: 0.3 }, x: 39, y: 11 },
          { at: { on: "sail with her", plus: 0.4 }, x: 36, y: 11 },
        ],
        face: [{ at: { on: "sail with her", plus: 0.4 }, to: "west" }],
        hops: [{ on: "sail with her", plus: 0.7 }],
        emotes: [{ at: { on: "sail with her", plus: 0.7 }, kind: "exclamation", seconds: 1.2 }],
      },
    ],
    fade: { in: CUT },
  },
  // Half an hour on she casts off again while you're still looking at the palm tree:
  // you turn, run for the water's edge, and she's gone.
  wait: {
    shots: [
      { at: 0, to: MELEMELE_BEACH },
      { at: 1, to: MELEMELE_BEACH },
    ],
    // She casts off at half past nine, a second into the scene, and is out of sight in three.
    clock: { from: h(33, 28), to: h(33, 39) },
    cast: [
      { sprite: "exeggutor-alola", map: MELEMELE, marks: [{ at: 0, x: 34, y: 10 }], face: "east" },
      {
        sprite: "gold",
        map: MELEMELE,
        to: { on: NOTICED, plus: 0.4 },
        marks: [{ at: 0, x: 36, y: 11 }],
        face: [
          { at: 0, to: "west" },
          { at: NOTICED, to: "east" },
        ],
        emotes: [{ at: NOTICED, kind: "exclamation", seconds: 0.4 }],
      },
      {
        sprite: "gold-running",
        map: MELEMELE,
        from: { on: NOTICED, plus: 0.4 },
        marks: [
          { at: { on: NOTICED, plus: 0.4 }, x: 36, y: 11 },
          { at: { on: NOTICED, plus: 0.8 }, x: 39, y: 11 },
          { at: { on: NOTICED, plus: 0.95 }, x: 39, y: 10 },
        ],
        face: [{ at: { on: NOTICED, plus: 0.95 }, to: "north" }],
      },
    ],
  },
  // Left behind on the beach, the camera lifts off you: Alola, then every land there is,
  // their Pokémon popping out, settled as the box counts them.
  lands: {
    shots: [
      { at: 0, to: MELEMELE_BEACH },
      { at: 0.04, to: MELEMELE_BEACH },
      { at: "Seven lands", to: LANDS, ease: "both" },
      { at: 1, to: LANDS },
    ],
    clock: { from: h(33, 39), to: h(34, 30) },
    cast: [
      { sprite: "exeggutor-alola", map: MELEMELE, marks: [{ at: 0, x: 34, y: 10 }], face: "east" },
      { sprite: "gold", map: MELEMELE, marks: [{ at: 0, x: 39, y: 10 }], face: "north" },
    ],
    wild: { from: "the whole map" },
  },
  // Over to the Sevii Islands while the box passes on what folk there say, their Galar
  // Pokémon popping, then straight down into Five Isle Meadow for the punchline: the flock.
  galar: {
    shots: [
      { at: 0, to: LANDS },
      { at: "swear", to: SEVII },
      { at: "Galar now", to: SEVII },
      { at: "Ask the", to: MEADOW, ease: "in" },
      { at: 1, to: MEADOW },
    ],
    clock: { at: h(34, 30) },
    wild: { from: 0, wave: { home: "sevii", at: "they're Galar" } },
    cast: WOOLOO.map(([x, y, dx, dy, face], i) => ({
      sprite: "wooloo" as const,
      map: "FiveIsland_Meadow_Frlg",
      marks: [
        { at: 0, x, y },
        { at: { on: "Ask the", plus: 0.3 + i * 0.15 }, x, y },
        { at: { on: "Ask the", plus: 1.1 + i * 0.15 }, x: x + dx, y: y + dy },
      ],
      face: [
        { at: 0, to: face },
        { at: { on: "Wooloo", plus: 0.5 }, to: "south" as const },
      ],
      hops: i === 2 ? [{ on: "Wooloo", plus: 0.6 }] : undefined,
    })),
    fade: { out: CUT },
  },
  // Every land again, and Kalos nowhere on it; its Pokémon pop at the three Safari Zones' gates.
  kalos: {
    shots: [
      { at: 0, to: LANDS },
      { at: 1, to: LANDS },
    ],
    clock: { at: h(34, 30) },
    wild: { from: 0, wave: { home: "safari", at: "The Safari" } },
    fade: { in: CUT, out: CUT },
  },
  // Back on Melemele's beach, where the Anne left you: you have been standing there all this
  // while. You turn to the water, cast, and what bites carries you off east, the palm tree
  // seeing you go. It is a Luvdisc: of what a rod brings up off Melemele by day, the one
  // that knows Surf the day it's caught and that the game draws you riding (it has surfing
  // sprites for some Pokémon, and a plain blob for the rest).
  surf: {
    shots: [
      { at: 0, to: MELEMELE_BEACH },
      { at: RIDE, to: MELEMELE_BEACH },
      { at: 1, to: { kind: "map", name: MELEMELE, x: 43, y: 9.5, zoom: 64 } },
    ],
    clock: { at: h(36) },
    cast: [
      {
        sprite: "exeggutor-alola",
        map: MELEMELE,
        marks: [{ at: 0, x: 34, y: 10 }],
        face: "east",
        hops: [{ on: RIDE, plus: 1 }],
      },
      {
        sprite: "gold",
        map: MELEMELE,
        to: "Fish",
        marks: [{ at: 0, x: 39, y: 10 }],
        face: [
          { at: 0, to: "north" },
          { at: { on: "Still", plus: 0.7 }, to: "south" },
          { at: "Nobody", to: "east" },
        ],
        emotes: [{ at: { on: "Nobody", plus: 0.2 }, kind: "question", seconds: 1.4 }],
      },
      // The rod out over the water, the float down, a tug, and the reel in.
      {
        sprite: "gold-fishing",
        map: MELEMELE,
        from: "Fish",
        to: RIDE,
        marks: [{ at: 0, x: 39, y: 10 }],
        face: "east",
        poses: [
          { at: "Fish", frame: 0 },
          { at: { on: "Fish", plus: 0.12 }, frame: 1 },
          { at: { on: "Fish", plus: 0.24 }, frame: 2 },
          { at: { on: "Fish", plus: 0.36 }, frame: 3 },
          { at: BITE, frame: 2 },
          { at: { on: BITE, plus: 0.12 }, frame: 3 },
          { at: { on: BITE, plus: 0.24 }, frame: 2 },
          { at: { on: BITE, plus: 0.36 }, frame: 3 },
          { at: { on: BITE, plus: 0.6 }, frame: 2 },
          { at: { on: BITE, plus: 0.72 }, frame: 1 },
          { at: { on: BITE, plus: 0.84 }, frame: 0 },
        ],
        emotes: [{ at: BITE, kind: "exclamation", seconds: 0.6 }],
      },
      {
        sprite: "luvdisc",
        map: MELEMELE,
        from: { on: BITE, plus: 0.7 },
        to: { on: BITE, plus: 1.75 },
        marks: [{ at: 0, x: 41, y: 10 }],
        face: "west",
        hops: [{ on: BITE, plus: 0.7 }],
      },
      { sprite: "luvdisc-surf", map: MELEMELE, from: RIDE, marks: RIDE_MARKS, face: "east" },
      {
        sprite: "gold-surfing",
        map: MELEMELE,
        from: RIDE,
        marks: RIDE_MARKS,
        face: "east",
        lift: 0.35,
      },
    ],
    tosses: [
      {
        sprite: "ball",
        map: MELEMELE,
        from: { x: 39, y: 10 },
        to: { x: 41, y: 10 },
        span: { from: { on: BITE, plus: 1.25 }, to: { on: BITE, plus: 1.75 } },
        height: 1,
      },
    ],
    fade: { in: CUT, out: CUT },
  },
  // Straight on from every land at night: every Gym Leader and Elite Four member's marker
  // lights up, then one dive onto Brock, landing as the box names him, on the morning
  // two days on that he's caring for the Pokémon at the Day-Care on Route 34, in Johto.
  leaders: {
    shots: [
      { at: 0, to: LANDS },
      { at: "Gym Leaders do", to: LANDS },
      // Landing close on him, so Goldenrod's cut-off edges, 16 tiles and more to his east,
      // stay out of frame.
      { at: "Brock's helping", to: BROCK },
      { at: 1, to: BROCK },
    ],
    // Two days on from the Anne, cut on as the markers light up, so nobody races to get there;
    // the ships stay where the last scene left them.
    clock: { from: h(82), to: h(82, 15) },
    shipsAt: h(34, 30),
    // Brock is one of the cast here, so he can turn to you: the map's own Brock stays out.
    notables: { marker: true, minHeight: 64, except: ["brock"] },
    // You come down the road with a Graveler at your heels; he turns, the Graveler goes
    // over in its ball, and one comes back. What a trade does to a Graveler, players know.
    cast: [
      {
        sprite: "brock",
        marks: [{ at: 0, x: 530, y: 1186 }],
        face: [
          { at: 0, to: "south" },
          { at: "Trade with", to: "west" },
        ],
        emotes: [{ at: { on: "Trade with", plus: 2.3 }, kind: "heart", seconds: 1.5 }],
      },
      {
        sprite: "gold",
        from: "Brock's helping",
        marks: [
          { at: "Brock's helping", x: 523, y: 1177 },
          { at: { on: "Brock's helping", plus: 1.9 }, x: 523, y: 1186 },
          { at: { on: "Brock's helping", plus: 3 }, x: 528, y: 1186 },
        ],
      },
      {
        sprite: "graveler",
        from: { on: "Brock's helping", plus: 0.2 },
        to: { on: "Trade with", plus: 0.5 },
        marks: [
          { at: { on: "Brock's helping", plus: 0.2 }, x: 523, y: 1177 },
          { at: { on: "Brock's helping", plus: 2.1 }, x: 523, y: 1186 },
          { at: { on: "Brock's helping", plus: 3 }, x: 527, y: 1186 },
        ],
      },
    ],
    tosses: [
      {
        sprite: "ball",
        from: { x: 527, y: 1186 },
        to: { x: 530, y: 1186 },
        span: { from: { on: "Trade with", plus: 0.5 }, to: { on: "Trade with", plus: 1.1 } },
      },
      {
        sprite: "ball",
        from: { x: 530, y: 1186 },
        to: { x: 528, y: 1186 },
        span: { from: { on: "Trade with", plus: 1.5 }, to: { on: "Trade with", plus: 2.1 } },
      },
    ],
    fade: { in: CUT, out: CUT },
  },
  // Two more lives, a warp between each. Lt. Surge in Celadon in the afternoon, coming down
  // the street, through the gap in the fence and in at the Game Corner's door, on "Again."
  surge: {
    shots: [
      { at: 0, to: { kind: "point", x: 1281.5, y: 1003.5, zoom: 60 } },
      { at: 1, to: { kind: "point", x: 1281.5, y: 1003.5, zoom: 64 } },
    ],
    clock: { from: h(16), to: h(16, 10) },
    // The map's own Surge stands on the door; this one walks to it.
    layers: { walkers: false },
    cast: [
      {
        sprite: "surge",
        to: { on: "Again", plus: 0.35 },
        marks: [
          { at: { on: "Again", plus: -5 }, x: 1293, y: 1006 },
          { at: { on: "Again", plus: -1 }, x: 1281, y: 1006 },
          { at: "Again", x: 1281, y: 1003 },
          { at: { on: "Again", plus: 0.35 }, x: 1281, y: 1002 },
        ],
      },
    ],
    fade: { in: CUT, out: CUT },
  },
  // Wallace on Route 119 at noon, on the open bank east of the river with his rod out over
  // the water, where Feebas hides on a few tiles of it. He doesn't move: that's the joke.
  wallace: {
    shots: [
      { at: 0, to: { kind: "point", x: 378.5, y: 1627, zoom: 60 } },
      { at: 1, to: { kind: "point", x: 378.5, y: 1627, zoom: 64 } },
    ],
    clock: { from: h(12, 30), to: h(12, 40) },
    // The map's own Wallace stands in the long grass across the river.
    layers: { walkers: false },
    cast: [
      { sprite: "wallace", marks: [{ at: 0, x: 378, y: 1626 }], face: "west" },
      // The rod in his hands, reaching half a tile out over the water.
      { sprite: "rod", marks: [{ at: 0, x: 377.5, y: 1626 }] },
    ],
    fade: { in: CUT, out: CUT },
  },
  // Route 29 off the road, deep into the night, the wild Pokémon of the night popping in the grass.
  night: {
    shots: [
      { at: 0, to: { kind: "map", name: "Route29_hns", x: 39, y: 14.5, zoom: 60 } },
      { at: 1, to: { kind: "map", name: "Route29_hns", x: 42, y: 14.5, zoom: 64 } },
    ],
    // The game's darkest tint runs from 1:00 to 4:00.
    clock: { from: h(25), to: h(27, 30) },
    // The one scene about the time of day: the tint and the lights are on.
    layers: { tint: true, lights: true, sightings: true },
    // You come out of the gate and down its path, then leave it for the tall grass as the
    // box asks, along the open row south of the trees; a Noctowl, Route 29's own by night,
    // drops in front of you, and the screen flashes into the battle.
    cast: [
      {
        sprite: "gold",
        map: "Route29_hns",
        marks: [
          { at: 0, x: 35, y: 11 },
          { at: 0.16, x: 35, y: 16 },
          { at: "Pack more", x: 43, y: 16 },
        ],
        emotes: [{ at: { on: "Pack more", plus: 0.35 }, kind: "exclamation", seconds: 1.5 }],
        hops: [{ on: "Pack more", plus: 0.35 }],
      },
      {
        sprite: "noctowl",
        map: "Route29_hns",
        from: { on: "Pack more", plus: 0.2 },
        marks: [{ at: 0, x: 45, y: 16 }],
        face: "west",
        hops: [{ on: "Pack more", plus: 0.2 }],
      },
    ],
    flashes: [
      { on: "you think", plus: 0.9 },
      { on: "you think", plus: 1.15 },
    ],
    fade: { in: CUT, out: CUT },
  },
  // Bug Catcher Don, the first trainer most GSC players fought, on Route 30 on day 10 of the
  // daily draw; two days on he stands in Ilex Forest (the draw puts him there on day 12).
  don: {
    shots: [
      // Close on him, so the forest fill's seams either side of Route 30 stay out of frame.
      { at: 0, to: { kind: "point", x: 720.5, y: 1102, zoom: 60 } },
      { at: 1, to: { kind: "point", x: 720.5, y: 1102, zoom: 64 } },
    ],
    clock: { at: h(10) },
    daily: { day: 10 },
    // You, walking on up the road past him, the way the road lets you: it has a ledge
    // below this stretch and another above it, which only drop southward, so you start
    // above the one and step round the other by the grass beside it, up to the stairs.
    cast: [
      {
        sprite: "gold",
        map: "Route30_hns",
        marks: [
          { at: 0, x: 22, y: 10 },
          { at: 0.39, x: 22, y: 4 },
          { at: 0.52, x: 24, y: 4 },
          { at: 0.65, x: 24, y: 2 },
          { at: 0.72, x: 23, y: 2 },
          { at: 0.85, x: 23, y: 0 },
        ],
      },
    ],
    fade: { in: CUT, out: CUT },
  },
  donagain: {
    shots: [
      // Close on the two of you and a little south, so you stand above the box and the seam
      // where the fills of two forests meet, north of the clearing, stays out of frame.
      { at: 0, to: { kind: "point", x: 583.5, y: 1257, zoom: 60 } },
      { at: 1, to: { kind: "point", x: 583.5, y: 1257, zoom: 64 } },
    ],
    clock: { at: h(10) },
    // Don is one of the cast here (the day's draw has him in Ilex Forest, tucked beside a
    // tree at the clearing's end): he stands in the open on the row you walk in along, sees
    // you, and comes over, the way a trainer does.
    cast: [
      {
        sprite: "bug-catcher",
        map: "IlexForest_hns",
        marks: [
          { at: 0, x: 42, y: 8 },
          { at: { on: "He hasn't", plus: 0.7 }, x: 42, y: 8 },
          { at: { on: "He hasn't", plus: 1.7 }, x: 46, y: 8 },
        ],
        face: [{ at: 0, to: "east" }],
        emotes: [{ at: "He hasn't", kind: "exclamation", seconds: 0.7 }],
      },
      // You come into the clearing from the east, and stop short when he looks up.
      {
        sprite: "gold",
        map: "IlexForest_hns",
        marks: [
          { at: 0, x: 56, y: 8 },
          { at: { on: "He hasn't", plus: -0.1 }, x: 47, y: 8 },
        ],
        hops: [{ on: "He hasn't", plus: 0.1 }],
      },
    ],
    fade: { in: CUT },
  },
  // Clair at the Dragon's Den before noon, and you walk straight up to her, no badges on you:
  // she turns, and the screen flashes the way it does when a battle starts.
  clair: {
    shots: [
      { at: 0, to: { kind: "map", name: "BlackthornCity_hns", x: 27, y: 15, zoom: 60 } },
      { at: 1, to: { kind: "map", name: "BlackthornCity_hns", x: 27, y: 15, zoom: 64 } },
    ],
    clock: { at: h(11) },
    // She is one of the cast, so she can stand still and turn: the map's own Clair, who
    // steps on the spot and wears a marker, stays out.
    layers: { walkers: false },
    cast: CLAIR_CAST,
    flashes: [
      { on: "fair fight", plus: 1.0 },
      { on: "fair fight", plus: 1.25 },
    ],
    fade: { in: CUT, out: CUT },
  },
  // Eight badges from three regions land one by one over Johto and Kanto while word of you
  // gets around; the eighth lands as the phone rings, and it's the Indigo League calling.
  fame: {
    shots: [
      { at: 0, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.92 } },
      { at: 1, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.92 } },
    ],
    clock: { at: h(11) },
    badges: {
      circuit: 0,
      // Clair's Rising Badge first: the one GSC players remember as the wall.
      earn: { from: 0.02, to: "Then the phone", badges: [7, 8, 2, 9, 1, 11, 3, 10] },
    },
    phone: { from: "Then the phone", to: 1, caller: "INDIGO LEAGUE", named: "It's the" },
    fade: { in: CUT },
  },
  // The whole circuit, then who you'll face, picked like a slot machine as on the website:
  // four seats stop on Elite Four regulars and the last on Brock. He steps forward, the
  // Golem slides in beside him, and the box wonders where he got it.
  golem: {
    shots: [
      { at: 0, to: { kind: "regions", names: ["JOHTO", "KANTO"], room: 0.92 } },
      { at: "Elite Four", to: { kind: "circuit", room: 0.9 } },
      { at: 1, to: { kind: "circuit", room: 0.9 } },
    ],
    clock: { at: h(11) },
    badges: {
      circuit: [0, 1],
      earn: { from: -1, to: -0.5, badges: [7, 8, 2, 9, 1, 11, 3, 10] },
    },
    // The five in battle order, weakest first. The reels stop on the four you'd expect one
    // after another while the box asks, and the first seat, last, on the one you wouldn't.
    lineup: {
      from: "And who's in",
      seats: [
        { slug: "brock", at: "Brock" },
        { slug: "koga", at: { on: "And who's in", plus: 1.3 } },
        { slug: "lorelei", at: { on: "And who's in", plus: 1.7 } },
        { slug: "will", at: { on: "And who's in", plus: 2.1 } },
        { slug: "karen", at: { on: "And who's in", plus: 2.5 } },
      ],
      star: "brock",
      grow: "With a",
      partner: { src: "/actors/golem-front.png", at: "With a" },
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

// The Gym Leaders and Elite Four walking the map on the page's clock. The data
// is notables.json (see build/notables.mjs): who they are, their homes, activity
// cycles and favourite places, and the routes between those places.
//
// This is a page-side simulation of the game's routines. The game itself runs
// them on map changes, not on a clock, so the timing here (how long each
// dwell lasts, how fast they walk, which favourite on which round, where they
// start) is the page's, and the walking routes are ours. `notablesAt` is
// deterministic: the same game minute always puts everyone in the same place.
import { DATA } from "./data"
import type { Overlay } from "./render"

type Point = [number, number]

/** An activity of a cycle with the places it may be done at (indices into `stops`). */
type Step = { a: string; s: number[] }

export type NotablesData = {
  /** [x, y, place name, "at" | "in"] per place; x and y are grid cells, in metatiles. */
  stops: [number, number, string, "at" | "in"][]
  trainers: {
    slug: string
    name: string
    /** "Gym Leader", "Elite Four" or "Champion". */
    role: string
    region?: string
    /** The stop they live at: their Gym's door, or their town. */
    home: number
    /** The placed map their home is on, when it has a card. */
    homeMap?: string
    cycle: Step[]
  }[]
  /** Routes by "a>b" stop indices (a < b): flat x, y cells; `sea` where it crosses water. */
  legs: Record<string, { pts: number[]; w?: (0 | 1)[] }>
}

/** A route from one stop to another, with the distance along it at each point. */
type Leg = {
  points: Point[]
  along: number[]
  length: number
  /** Per segment, whether it crosses water (the walker leaves a wake there). */
  water: boolean[]
  minutes: number
}

type Plan = {
  slug: string
  name: string
  role: string
  region?: string
  home: number
  homeMap?: string
  cycle: Step[]
  /** Game minutes of a round: every step's dwell and the longest trip between each pair of steps. */
  round: number
  /** Game minutes offset so the trainers don't all start a round together. */
  phase: number
  hash: number
  /** The row of the sprite sheet. */
  row: number
}

export type Notables = {
  stops: NotablesData["stops"]
  plans: Plan[]
  legs: Map<string, Leg>
  image: HTMLImageElement
  /** Which rows of the sheet have no walking frames (their trainers hop instead). */
  still: boolean[]
}

export type Facing = "down" | "up" | "left" | "right"

/** Where a notable is at a game minute. */
export type Notable = {
  slug: string
  /** "Brock". */
  name: string
  role: string
  /** A grid point at their feet, in metatiles. */
  x: number
  y: number
  facing: Facing
  moving: boolean
  /** Crossing water, by ship or ferry. */
  sea: boolean
  row: number
  /** The activity now, or the one they are heading for. */
  activity: string
  /** The place they are at or heading for. */
  place: string
  /** "MISTY · fishing at Nugget Bridge", "BROCK · on the way to Oak's Lab". */
  label: string
}

const SPRITES = "/game/sprites/notables.png"
/** The sprite sheet, for showing a notable outside the map: 16x32 frames, nine to a row. */
export const NOTABLES_SRC = SPRITES

/**
 * How many beats each activity dwells, as the game's `sDwell`
 * (src/wayfarer_world_sim.c); a beat is the page's `BEAT` game minutes.
 */
const DWELL: Record<string, number> = {
  visit: 1,
  care: 2,
  shop: 2,
  relax: 2,
  sightsee: 2,
  gamble: 3,
  train: 3,
  fish: 3,
  study: 3,
  home: 4,
  "lie low": 4,
}
const BEAT = 45

/**
 * Game minutes a notable takes per metatile, on land and across water alike,
 * so nobody changes pace mid-trip: about the player's walking pace (3.75
 * metatiles a second) on the page's clock.
 */
const WALK_MINUTES = 3.2

/** What each activity reads as, "fishing at Nugget Bridge". */
const VERB: Record<string, string> = {
  visit: "visiting",
  care: "caring for Pokémon",
  shop: "shopping",
  relax: "relaxing",
  sightsee: "sightseeing",
  gamble: "gambling",
  train: "training",
  fish: "fishing",
  study: "studying",
  home: "resting",
  "lie low": "lying low",
}

const loadImage = (src: string) =>
  new Promise<HTMLImageElement>((resolve, reject) => {
    const image = new Image()
    image.addEventListener("load", () => resolve(image))
    image.addEventListener("error", reject)
    image.src = src
  })

/** A 32-bit integer hash of a few integers, for choosing among favourites. */
const mix = (...values: number[]) => {
  let h = 0x9e3779b9
  for (const value of values) {
    h = Math.imul(h ^ (value | 0), 0x85ebca6b)
    h ^= h >>> 13
    h = Math.imul(h, 0xc2b2ae35)
    h ^= h >>> 16
  }
  return h >>> 0
}
const hashOf = (text: string) => {
  let h = 0
  for (let i = 0; i < text.length; i++) h = Math.imul(h ^ text.charCodeAt(i), 0x01000193)
  return mix(h)
}

const toLeg = (flat: number[], water: (0 | 1)[] = []): Leg => {
  // Cells' centers.
  const points: Point[] = []
  for (let i = 0; i < flat.length; i += 2) points.push([flat[i]! + 0.5, flat[i + 1]! + 0.5])
  const along = [0]
  for (let i = 1; i < points.length; i++)
    along.push(
      along[i - 1]! +
        Math.hypot(points[i]![0] - points[i - 1]![0], points[i]![1] - points[i - 1]![1]),
    )
  const length = along.at(-1)!
  return {
    points,
    along,
    length,
    water: points.slice(1).map((_, i) => water[i] === 1),
    minutes: Math.ceil(length * WALK_MINUTES),
  }
}

const reversed = (leg: Leg): Leg => ({
  ...leg,
  points: leg.points.toReversed(),
  along: leg.along.map((d) => leg.length - d).toReversed(),
  water: leg.water.toReversed(),
})

const loading = new Map<string, Promise<Notables>>()

/** Loads notables.json and the sprite sheet once per base path. */
export const loadNotables = (base = DATA, sprites = SPRITES): Promise<Notables> => {
  const key = `${base}|${sprites}`
  let loaded = loading.get(key)
  if (!loaded) {
    loaded = (async () => {
      const [data, image] = await Promise.all([
        fetch(`${base}/notables.json`).then((response) => response.json() as Promise<NotablesData>),
        loadImage(sprites),
      ])
      const legs = new Map<string, Leg>()
      for (const [pair, { pts, w }] of Object.entries(data.legs)) {
        const [a, b] = pair.split(">")
        const leg = toLeg(pts, w)
        legs.set(pair, leg)
        legs.set(`${b}>${a}`, reversed(leg))
      }
      const trip = (from: number, to: number) =>
        from === to ? 0 : legs.get(`${from}>${to}`)!.minutes
      const plans = data.trainers.map((trainer, row): Plan => {
        const steps = trainer.cycle
        const dwell = steps.reduce((sum, step) => sum + (DWELL[step.a] ?? 2) * BEAT, 0)
        // Room for the longest trip into each step, so any picks fit one round.
        const trips = steps.reduce((sum, step, i) => {
          const next = steps[(i + 1) % steps.length]!
          return sum + Math.max(...step.s.flatMap((from) => next.s.map((to) => trip(from, to))))
        }, 0)
        const round = dwell + trips
        const hash = hashOf(trainer.slug)
        return { ...trainer, round, phase: hash % round, hash, row }
      })
      return { stops: data.stops, plans, legs, image, still: stillRows(image, plans.length) }
    })()
    loaded.catch(() => loading.delete(key))
    loading.set(key, loaded)
  }
  return loaded
}

/** The rows of the sprite sheet whose walking frames are empty. */
const stillRows = (image: HTMLImageElement, rows: number) => {
  const canvas = document.createElement("canvas")
  canvas.width = image.naturalWidth
  canvas.height = image.naturalHeight
  const context = canvas.getContext("2d", { willReadFrequently: true })
  if (!context) return Array.from({ length: rows }, () => false)
  context.drawImage(image, 0, 0)
  return Array.from({ length: rows }, (_, row) => {
    const frame = context.getImageData(3 * 16, row * 32, 16, 32).data
    for (let i = 3; i < frame.length; i += 4) if (frame[i]) return false
    return true
  })
}

const pointAlong = (leg: Leg, distance: number) => {
  const { points, along } = leg
  let i = 1
  while (i < points.length - 1 && along[i]! < distance) i++
  const [ax, ay] = points[i - 1]!
  const [bx, by] = points[i]!
  const span = along[i]! - along[i - 1]!
  const t = span > 0 ? Math.min(1, Math.max(0, (distance - along[i - 1]!) / span)) : 0
  return {
    x: ax + (bx - ax) * t,
    y: ay + (by - ay) * t,
    dx: bx - ax,
    dy: by - ay,
    water: leg.water[i - 1] ?? false,
  }
}

const facingOf = (dx: number, dy: number): Facing =>
  Math.abs(dx) > Math.abs(dy) ? (dx < 0 ? "left" : "right") : dy < 0 ? "up" : "down"

/** The place a trainer does a step of a round at: the favourite that round's hash picks. */
const pick = (plan: Plan, round: number, step: number) => {
  const options = plan.cycle[step]!.s
  return options[mix(plan.hash, round, step) % options.length]!
}

const text = (name: string, activity: string, stop: NotablesData["stops"][number]) =>
  `${name.toUpperCase()} · ${VERB[activity] ?? activity} ${stop[3]} ${stop[2]}`

const whereAt = (world: Notables, plan: Plan, minutes: number): Notable => {
  const { stops, legs } = world
  const { cycle } = plan
  const time = minutes + plan.phase
  const round = Math.floor(time / plan.round)
  const local = time - round * plan.round
  // The round: the trip from where the last one ended to its first place,
  // then each place's dwell and the trip on to the next. What the trips leave
  // of the round is shared out among the dwells in proportion to their beats.
  const places = cycle.map((_, i) => pick(plan, round, i))
  const before = pick(plan, round - 1, cycle.length - 1)
  const trips = [before, ...places].map((from, i) => {
    const to = places[i]
    return to === undefined || from === to ? undefined : legs.get(`${from}>${to}`)
  })
  const travel = trips.reduce((sum, leg) => sum + (leg?.minutes ?? 0), 0)
  const beats = cycle.reduce((sum, step) => sum + (DWELL[step.a] ?? 2), 0)
  const per = (plan.round - travel) / beats
  let at = 0
  for (let i = 0; i < cycle.length; i++) {
    const trip = trips[i]
    // The trip into step i (the last step's trip on to the next round's first place is that round's).
    const into = trip?.minutes ?? 0
    const step = cycle[i]!
    const stop = stops[places[i]!]!
    if (local < at + into && trip) {
      const p = pointAlong(trip, ((local - at) / into) * trip.length)
      return {
        slug: plan.slug,
        name: plan.name,
        role: plan.role,
        x: p.x,
        y: p.y,
        facing: facingOf(p.dx, p.dy),
        moving: true,
        sea: p.water,
        row: plan.row,
        activity: step.a,
        place: stop[2],
        label: `${plan.name.toUpperCase()} · ${p.water ? "crossing the sea" : "on the way"} to ${stop[2]}`,
      }
    }
    at += into
    const dwell = (DWELL[step.a] ?? 2) * per
    if (local < at + dwell || i === cycle.length - 1) {
      return {
        slug: plan.slug,
        name: plan.name,
        role: plan.role,
        x: stop[0] + 0.5,
        y: stop[1] + 0.5,
        facing: "down",
        moving: false,
        sea: false,
        row: plan.row,
        activity: step.a,
        place: stop[2],
        label: text(plan.name, step.a, stop),
      }
    }
    at += dwell
  }
  throw new Error("unreachable")
}

/** Where one notable is at a game minute, by their slug ("misty"). */
export const notableAt = (world: Notables, slug: string, minutes: number): Notable | undefined => {
  const plan = world.plans.find((p) => p.slug === slug)
  return plan && whereAt(world, plan, minutes)
}

/** Where every notable is at a game minute. */
export const notablesAt = (world: Notables, minutes: number): Notable[] =>
  world.plans.map((plan) => whereAt(world, plan, minutes))

/** Sprite size: its height, in CSS pixels, is never smaller than this, so they read from far out. */
const MIN_HEIGHT_CSS = 26
/** Smaller on a narrow view, where the whole world is a small map. */
const minHeight = (viewWidth: number) => (viewWidth < 600 ? 18 : MIN_HEIGHT_CSS)
/** Where the feet stand in their tile, in metatiles below its center (the game draws them near the tile's bottom). */
const FOOT = 0.4
/** The sprite's height in metatiles: the game's 2, or as many as keep it `MIN_HEIGHT_CSS` tall on screen. */
const heightTiles = (zoom: number, viewWidth: number) => Math.max(2, minHeight(viewWidth) / zoom)

/** Walk cycle frames: [stand, walk 1, walk 2] for each facing, in the sheet's nine. */
const FRAMES: Record<"down" | "up" | "left", [number, number, number]> = {
  down: [0, 3, 4],
  up: [1, 5, 6],
  left: [2, 7, 8],
}
/** A walking frame lasts about this long, in ms (the game's 8 ticks). */
const STEP_MS = 140

/**
 * The notables over the map: each as the overworld sprite, facing the way it
 * walks and stepping while it does, drawn at least 26 CSS pixels (18 on a narrow view) tall so they
 * read when the map is far out, and at true scale close in. `minutes` is a game
 * minute, or a function returning one.
 */
export const notablesOverlay =
  (
    world: Notables,
    minutes: number | (() => number),
    {
      only,
      minHeight: least,
      marker,
    }: {
      /** Draws only the notables this keeps, e.g. the one a card is about. */
      only?: (slug: string) => boolean
      /** The least height on screen, in CSS pixels, in place of the default. */
      minHeight?: number
      /** A bouncing arrow over their heads, to pick them out. */
      marker?: boolean
    } = {},
  ): Overlay =>
  (context, { scale, ratio, toScreen }) => {
    const now = typeof minutes === "function" ? minutes() : minutes
    const { width, height } = context.canvas
    const zoom = scale / ratio
    const h =
      (least ? Math.max(2, least / zoom) : heightTiles(zoom, context.canvas.width / ratio)) * scale
    const w = h / 2
    const far = h > 2 * scale * 1.01
    const tick = Math.floor(performance.now() / STEP_MS)
    context.imageSmoothingEnabled = false
    for (const notable of notablesAt(world, now)) {
      if (only && !only(notable.slug)) continue
      const p = toScreen(notable.x, notable.y + FOOT)
      if (p.x < -w || p.y < -h || p.x > width + w || p.y > height + 2 * h) continue
      const face = notable.facing === "right" ? "left" : notable.facing
      const [stand, one, two] = FRAMES[face]
      const phase = (tick + notable.row) % 4
      const still = world.still[notable.row]
      const frame = !notable.moving || still ? stand : phase === 1 ? one : phase === 3 ? two : stand
      // A sheet with no walking frames hops a little instead.
      const hop = notable.moving && still ? (phase % 2) * 0.04 * h : 0
      // A soft shadow, or a wake in the water.
      context.fillStyle = notable.sea ? "rgb(255 255 255 / 0.55)" : "rgb(0 0 0 / 0.3)"
      context.beginPath()
      context.ellipse(p.x, p.y - h * 0.04, w * 0.5, h * 0.09, 0, 0, Math.PI * 2)
      context.fill()
      context.save()
      if (far) {
        context.shadowColor = "rgb(0 0 0 / 0.9)"
        context.shadowBlur = 2 * ratio
      }
      context.translate(p.x, p.y - h - hop)
      if (notable.facing === "right") {
        context.translate(w, 0)
        context.scale(-1, 1)
      }
      context.drawImage(world.image, frame * 16, notable.row * 32, 16, 32, 0, 0, w, h)
      context.restore()
      if (marker) {
        // The game's down arrow, in its yellow, bobbing two steps.
        const size = Math.round(6 * ratio)
        const bob = Math.floor(performance.now() / 300) % 2 ? size * 0.5 : 0
        const top = p.y - h - hop - size * 2.2 + bob
        context.fillStyle = "rgb(255 214 0)"
        context.strokeStyle = "rgb(0 0 0 / 0.8)"
        context.lineWidth = Math.max(1, ratio)
        context.beginPath()
        context.moveTo(p.x - size, top)
        context.lineTo(p.x + size, top)
        context.lineTo(p.x, top + size * 1.2)
        context.closePath()
        context.fill()
        context.stroke()
      }
    }
  }

/**
 * The notable under a grid point at a camera zoom (CSS pixels per metatile):
 * the nearest whose drawn sprite, with a finger's slack, holds it.
 */
export const pickNotable = (
  world: Notables,
  minutes: number,
  x: number,
  y: number,
  zoom: number,
  viewWidth: number,
): Notable | undefined => {
  const h = heightTiles(zoom, viewWidth)
  const slack = 10 / zoom
  return notablesAt(world, minutes)
    .map((notable) => ({
      notable,
      dx: Math.abs(notable.x - x),
      dy: Math.abs(notable.y + FOOT - h / 2 - y),
    }))
    .filter(({ dx, dy }) => dx <= h / 4 + slack && dy <= h / 2 + slack)
    .toSorted((a, b) => Math.hypot(a.dx, a.dy) - Math.hypot(b.dx, b.dy))[0]?.notable
}

/** The notables whose home is on a placed map, and those on it now (by the map's rectangle on the grid). */
export const notablesOn = (
  world: Notables,
  minutes: number,
  map: { name: string; x: number; y: number; width: number; height: number } | undefined,
) => {
  const there = notablesAt(world, minutes)
  return {
    living: world.plans.filter((plan) => plan.homeMap === map?.name),
    present: map
      ? there.filter(
          (n) =>
            n.x >= map.x && n.x < map.x + map.width && n.y >= map.y && n.y < map.y + map.height,
        )
      : [],
  }
}

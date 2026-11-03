// The living world over the map: the S.S. ferries sailing their loops, the
// lanes they sail, the Magnet Train on its line, and the game's time-of-day tint. The data is living.json
// (see build/living.mjs); `livingOverlay` draws it for a game time.
import { DATA } from "./data"
import type { Overlay, Projection } from "./render"

type Point = [number, number]

export type Living = {
  ports: Record<string, { name: string; x: number; y: number }>
  ferry: {
    /** Game minutes a ship waits in port, and sails between two ports. */
    portMinutes: number
    seaMinutes: number
    ships: {
      id: string
      /** "S.S. AQUA". */
      name: string
      /** Port ids in loop order; the ship sails on from the last back to the first. */
      ports: string[]
      /** The lane from each port to the next, as keys of `lanes`. */
      lanes: string[]
      /** The game minute of a day the cycle starts at its first port. */
      start: number
    }[]
  }
  /** Sea lanes by "from>to" port ids: polylines of grid points in metatiles. */
  lanes: Record<string, Point[]>
  /** The Magnet Train, back and forth along its line. */
  train?: {
    /** "MAGNET TRAIN". */
    name: string
    /** The stations at the line's two ends, first to last. */
    stations: [string, string]
    /** The middle of its deck, station to station, in metatiles. */
    line: Point[]
    /** Game minutes it waits at a station, and rides between the two. */
    stationMinutes: number
    rideMinutes: number
    /** The game minute it leaves the first station. */
    start: number
  }
  /** Each time of day, by the hours it begins and ends. */
  periods: { name: string; from: number; to: number }[]
  /** [hour, multiplier per channel out of 256] the game blends the overworld by. */
  tint: [number, [number, number, number]][]
}

/** A lane with the distance along it at each point, so a ship can be put at a fraction. */
export type Lane = { points: Point[]; along: number[]; length: number }

export type LivingWorld = Living & {
  shapes: Map<string, Lane>
  sprites: Record<string, HTMLImageElement>
  /** The train's line, measured. */
  rails?: Lane
}

/** Where the train is at a game time. */
export type Train = {
  /** How far along its line the middle of the train is, in metatiles. */
  along: number
  /** How far along its ride it is, from the station it left (0) to the one ahead (1). */
  progress: number
  /** The station it left or waits at, and the one ahead (the same while at a station). */
  from: string
  to: string
  at: "station" | "line"
  /** "MAGNET TRAIN · Goldenrod → Saffron", or "MAGNET TRAIN · in Goldenrod". */
  label: string
}

/** Where a ship is at a game time. */
export type Ship = {
  id: string
  /** "S.S. AQUA". */
  name: string
  /** A grid point, in metatiles. */
  x: number
  y: number
  /** True when it sails east, so the sprite (drawn bow to the west) is mirrored. */
  flip: boolean
  at: "port" | "sea"
  /** The port it is in or left, and the one it is bound for (the same while in port). */
  from: string
  to: string
  /** The port it sails for next: `to` at sea, the one after `from` in port. */
  next: string
  /** How far along its leg it is, from the port it left (0) to the next (1); 0 in port. */
  progress: number
  /** "S.S. AQUA · Olivine → Vermilion", or "S.S. AQUA · in Olivine" in port. */
  label: string
  /** At sea, the lane it sails and how far along it is, in metatiles, for its wake. */
  lane?: Lane
  along?: number
}

/** Game minutes per second of play: the game's own clock, and a quick one for the page (a day in 2 minutes). */
export const LIVING_RATE = { game: 12 / 60, page: 1440 / 120 }

const MINUTES_PER_DAY = 1440

/** The game starts at 8:00, the start of Day. */
export const DAY_START = 8 * 60

const loadImage = (src: string) =>
  new Promise<HTMLImageElement>((resolve, reject) => {
    const image = new Image()
    image.addEventListener("load", () => resolve(image))
    image.addEventListener("error", reject)
    image.src = src
  })

const shapeOf = (points: Point[]): Lane => {
  const along = [0]
  for (let i = 1; i < points.length; i++)
    along.push(
      along[i - 1]! +
        Math.hypot(points[i]![0] - points[i - 1]![0], points[i]![1] - points[i - 1]![1]),
    )
  return { points, along, length: along.at(-1)! }
}

const loading = new Map<string, Promise<LivingWorld>>()

/** Loads living.json, the three ship sprites and the train's once per base path. */
export const loadLiving = (base = DATA, sprites = "/game/sprites"): Promise<LivingWorld> => {
  const key = `${base}|${sprites}`
  let world = loading.get(key)
  if (!world) {
    world = (async () => {
      const [living, aqua, anne, tidal, train] = await Promise.all([
        fetch(`${base}/living.json`).then((response) => response.json() as Promise<Living>),
        loadImage(`${sprites}/ss-aqua.png`),
        loadImage(`${sprites}/ss-anne.png`),
        loadImage(`${sprites}/ss-tidal.png`),
        loadImage(`${sprites}/magnet-train.png`),
      ])
      return {
        ...living,
        shapes: new Map(Object.entries(living.lanes).map(([k, points]) => [k, shapeOf(points)])),
        sprites: { aqua, anne, tidal, train },
        rails: living.train && shapeOf(living.train.line),
      }
    })()
    world.catch(() => loading.delete(key))
    loading.set(key, world)
  }
  return world
}

/**
 * A game clock from real time: a function returning the game minute now, from
 * `start` (default 8:00 on day 1), running `rate` game minutes per second
 * (`LIVING_RATE.game` for the game's own pace, `.page` for a day in 2 minutes).
 */
export const gameClock = (rate: number = LIVING_RATE.page, start = DAY_START) => {
  const began = performance.now()
  return () => start + ((performance.now() - began) / 1000) * rate
}

/** "08:05", the game minute as the hour and minute of its day. */
export const clockText = (minutes: number) => {
  const m = Math.floor(minutes) % MINUTES_PER_DAY
  return `${String(Math.floor(m / 60)).padStart(2, "0")}:${String(m % 60).padStart(2, "0")}`
}

/** Which day of play a game minute falls in; the first is 1. */
export const dayOf = (minutes: number) => Math.floor(minutes / MINUTES_PER_DAY) + 1

/** The multiplier the game tints the overworld by at an hour of the day, per channel from 0 to 255. */
export const tintAt = (living: Living, minutes: number): [number, number, number] => {
  const hour = (((minutes / 60) % 24) + 24) % 24
  const stops = living.tint
  let i = stops.findIndex(([h]) => h > hour)
  if (i < 0) i = stops.length - 1
  if (i === 0) i = 1
  const [h0, c0] = stops[i - 1]!
  const [h1, c1] = stops[i]!
  const t = (hour - h0) / (h1 - h0)
  return [0, 1, 2].map((k) => Math.min(255, c0[k]! + (c1[k]! - c0[k]!) * t)) as [
    number,
    number,
    number,
  ]
}

/** The time of day at a game minute, with its clock text and the tint over the map. */
export const periodAt = (living: Living, minutes: number) => {
  const hour = Math.floor((((minutes / 60) % 24) + 24) % 24)
  const period = living.periods.find(({ from, to }) => hour >= from && hour < to)
  return {
    /** "Morning", "Day", "Evening" or "Night". */
    name: period?.name ?? "Day",
    /** "08:05". */
    clock: clockText(minutes),
    day: dayOf(minutes),
    tint: tintAt(living, minutes),
  }
}

const pointAlong = (lane: Lane, distance: number) => {
  const { points, along } = lane
  let i = 1
  while (i < points.length - 1 && along[i]! < distance) i++
  const [ax, ay] = points[i - 1]!
  const [bx, by] = points[i]!
  const span = along[i]! - along[i - 1]!
  const t = span > 0 ? Math.min(1, Math.max(0, (distance - along[i - 1]!) / span)) : 0
  return { x: ax + (bx - ax) * t, y: ay + (by - ay) * t, dx: bx - ax, dy: by - ay }
}

/**
 * The train's length in metatiles: its sprite's 128 pixels drawn twice the
 * game's size, so it fills the viaduct's deck (four metatiles wide) the way
 * it fills its station's track. Two cars of eight.
 */
const TRAIN_TILES = 16

/** Where the train is at a game minute: waiting at a station, or riding between. */
export const trainAt = (living: LivingWorld, minutes: number): Train | undefined => {
  const { train, rails } = living
  if (!train || !rails) return undefined
  const { stationMinutes: wait, rideMinutes: ride, stations, name } = train
  const cycle = (wait + ride) * 2
  const t = (((minutes - train.start + wait) % cycle) + cycle) % cycle
  const first = TRAIN_TILES / 2,
    last = rails.length - TRAIN_TILES / 2
  const [a, b] = stations
  if (t < wait)
    return { along: first, progress: 0, from: a, to: a, at: "station", label: `${name} · in ${a}` }
  if (t < wait + ride) {
    const p = (t - wait) / ride
    const eased = p * p * (3 - 2 * p)
    return {
      along: first + (last - first) * eased,
      progress: eased,
      from: a,
      to: b,
      at: "line",
      label: `${name} · ${a} → ${b}`,
    }
  }
  if (t < wait * 2 + ride)
    return { along: last, progress: 0, from: b, to: b, at: "station", label: `${name} · in ${b}` }
  const p = (t - wait * 2 - ride) / ride
  const eased = p * p * (3 - 2 * p)
  return {
    along: last - (last - first) * eased,
    progress: eased,
    from: b,
    to: a,
    at: "line",
    label: `${name} · ${b} → ${a}`,
  }
}

/** The size, in CSS pixels, a car of the train is never drawn smaller than. */
const MIN_CAR_CSS = 20

/**
 * Draws the train: its two cars, each where it is along the line and turned
 * the way the line runs there, so it takes the corners. The western car (the
 * sprite's left half, nose first) is the one nearer the line's start.
 */
const drawTrain = (
  context: CanvasRenderingContext2D,
  living: LivingWorld,
  train: Train,
  { scale, ratio, toScreen }: Projection,
) => {
  const image = living.sprites.train
  const rails = living.rails
  if (!image || !rails) return null
  const carW = Math.max((TRAIN_TILES / 2) * scale, MIN_CAR_CSS * ratio)
  const carH = (carW * image.naturalHeight) / (image.naturalWidth / 2)
  // Cars as long as they're drawn, so they meet however far out.
  const half = carW / scale / 2
  const sourceW = image.naturalWidth / 2
  for (const [car, offset] of [
    [0, -half],
    [1, half],
  ] as const) {
    const at = pointAlong(rails, Math.min(rails.length, Math.max(0, train.along + offset)))
    const p = toScreen(at.x, at.y)
    const { width, height } = context.canvas
    if (p.x < -carW || p.y < -carW || p.x > width + carW || p.y > height + carW) continue
    context.save()
    context.translate(p.x, p.y)
    context.rotate(Math.atan2(at.dy, at.dx))
    context.drawImage(
      image,
      car * sourceW,
      0,
      sourceW,
      image.naturalHeight,
      -carW / 2,
      -carH / 2,
      carW,
      carH,
    )
    context.restore()
  }
  // Its name goes over the middle of the train, clear of the cars however
  // they turn (under it, at Saffron, the KANTO plate would cover it).
  const middle = pointAlong(rails, Math.min(rails.length, Math.max(0, train.along)))
  const p = toScreen(middle.x, middle.y)
  return { x: p.x, y: p.y - carW / 2 - 20 * ratio }
}

/** The names under the ships and the train, so a visitor who played the games knows them. */
const drawName = (
  context: CanvasRenderingContext2D,
  name: string,
  x: number,
  y: number,
  ratio: number,
) => {
  context.fillStyle = "rgb(0 0 0 / 0.7)"
  context.fillText(name, x + ratio * 1.5, y + ratio * 1.5)
  context.fillStyle = "rgb(248 232 128)"
  context.fillText(name, x, y)
}

/** Where every ship is at a game minute. */
export const shipsAt = (living: LivingWorld, minutes: number): Ship[] => {
  const { portMinutes, seaMinutes, ships } = living.ferry
  const leg = portMinutes + seaMinutes
  return ships.map((ship) => {
    const cycle = leg * ship.ports.length
    const t = (((minutes - ship.start) % cycle) + cycle) % cycle
    const stop = Math.floor(t / leg)
    const within = t - stop * leg
    const here = ship.ports[stop]!
    const next = ship.ports[(stop + 1) % ship.ports.length]!
    const portName = (id: string) => living.ports[id]!.name
    if (within < portMinutes) {
      const arrived = living.shapes.get(
        ship.lanes[(stop + ship.ports.length - 1) % ship.ports.length]!,
      )!
      const end = pointAlong(arrived, arrived.length)
      const dock = living.ports[here]!
      return {
        id: ship.id,
        name: ship.name,
        x: dock.x,
        y: dock.y,
        flip: end.dx > 0,
        at: "port",
        from: here,
        to: here,
        next,
        progress: 0,
        label: `${ship.name} · in ${portName(here)}`,
      }
    }
    const lane = living.shapes.get(ship.lanes[stop]!)!
    const p = (within - portMinutes) / seaMinutes
    // Easing out of one port and into the next, half strength.
    const eased = (p + p * p * (3 - 2 * p)) / 2
    const at = pointAlong(lane, lane.length * eased)
    return {
      id: ship.id,
      name: ship.name,
      x: at.x,
      y: at.y,
      flip: at.dx > 0,
      at: "sea",
      from: here,
      to: next,
      next,
      progress: eased,
      label: `${ship.name} · ${portName(here)} → ${portName(next)}`,
      lane,
      along: lane.length * eased,
    }
  })
}

/** The size, in CSS pixels, a ship is never drawn smaller than (its sprite's width); further out it stays this big. */
const MIN_SHIP_CSS = 34

/** A ship sprite's width in metatiles: the sprite's pixels over the game's 16 a metatile. */
const spriteTiles = (image: HTMLImageElement) => image.naturalWidth / 16

/**
 * How far from a ship's point a pointer still hits it, in metatiles, at a
 * camera zoom (CSS pixels per metatile): the ship's drawn size.
 */
export const shipReach = (zoom: number, spriteWidth = 6) =>
  Math.max(spriteWidth, MIN_SHIP_CSS / zoom) / 2

/** The ship nearest a grid point within `reach` metatiles (see `shipReach`), if any. */
export const pickShip = (
  living: LivingWorld,
  minutes: number,
  x: number,
  y: number,
  reach: number,
): Ship | undefined =>
  shipsAt(living, minutes)
    .map((ship) => ({ ship, d: Math.hypot(ship.x - x, ship.y - y) }))
    .filter(({ d }) => d <= reach)
    .toSorted((a, b) => a.d - b.d)[0]?.ship

/** How long a ship's wake is, in ship widths, and how long its foam takes to fade, in ms. */
const WAKE_WIDTHS = 1.8
const FOAM_MS = 1400

/**
 * The wake behind a ship at sea: a pale, tapering trail along the lane it
 * came by, and flecks of foam that drift back and fade.
 */
const drawWake = (
  context: CanvasRenderingContext2D,
  ship: Ship,
  width: number,
  height: number,
  { toScreen, ratio, scale }: Projection,
  now: number,
) => {
  const { lane, along } = ship
  if (!lane || along === undefined) return
  const length = (width / scale) * WAKE_WIDTHS
  const segments = 6
  context.lineCap = "round"
  context.lineJoin = "round"
  for (let i = 0; i < segments; i++) {
    const from = pointAlong(lane, Math.max(0, along - (length * i) / segments))
    const to = pointAlong(lane, Math.max(0, along - (length * (i + 1)) / segments))
    const a = toScreen(from.x, from.y)
    const b = toScreen(to.x, to.y)
    const fade = 1 - i / segments
    context.strokeStyle = `rgb(255 255 255 / ${0.42 * fade})`
    context.lineWidth = Math.max(1.5 * ratio, height * 0.34 * fade)
    context.beginPath()
    context.moveTo(a.x, a.y + height * 0.2)
    context.lineTo(b.x, b.y + height * 0.2)
    context.stroke()
  }
  // Foam: a few flecks born at the stern, carried back along the trail.
  const fleck = Math.max(1, Math.round(ratio))
  context.fillStyle = "rgb(255 255 255)"
  for (let k = 0; k < 5; k++) {
    const phase = (now / FOAM_MS + k / 5) % 1
    const back = pointAlong(lane, Math.max(0, along - length * phase * 0.9))
    const p = toScreen(back.x, back.y)
    const side = (k % 2 ? 1 : -1) * height * 0.12 * phase
    context.globalAlpha = (1 - phase) * 0.7
    context.fillRect(
      Math.round(p.x - fleck / 2),
      Math.round(p.y + height * 0.2 + side),
      fleck * 2,
      fleck,
    )
  }
  context.globalAlpha = 1
}

const drawLane = (
  context: CanvasRenderingContext2D,
  { points }: Lane,
  toScreen: Projection["toScreen"],
) => {
  context.beginPath()
  points.forEach(([x, y], i) => {
    const p = toScreen(x, y)
    if (i === 0) context.moveTo(p.x, p.y)
    else context.lineTo(p.x, p.y)
  })
  context.stroke()
}

/**
 * The living world over the map: the lanes faintly, the ships on them as the
 * game's own sprites, then the time of day's tint over all of it. `minutes` is
 * a game minute, or a function returning one (so one overlay can run on a clock).
 */
export const livingOverlay =
  (
    living: LivingWorld,
    minutes: number | (() => number),
    { still = false }: { still?: boolean } = {},
  ): Overlay =>
  (context, projection) => {
    const { scale, ratio, toScreen } = projection
    const now = typeof minutes === "function" ? minutes() : minutes
    const { width, height } = context.canvas
    const clock = still ? 0 : performance.now()

    context.lineJoin = "round"
    context.lineCap = "round"
    context.strokeStyle = "rgb(255 255 255 / 0.3)"
    context.lineWidth = Math.min(3, Math.max(1.5, scale * 0.12)) * ratio
    context.setLineDash([6 * ratio, 8 * ratio])
    for (const lane of living.shapes.values()) drawLane(context, lane, toScreen)
    context.setLineDash([])

    context.imageSmoothingEnabled = false
    const names: { name: string; x: number; y: number }[] = []
    for (const ship of shipsAt(living, now)) {
      const image = living.sprites[ship.id]!
      const p = toScreen(ship.x, ship.y)
      const w = Math.max(
        spriteTiles(image) * scale,
        MIN_SHIP_CSS * ratio * (image.naturalWidth / 96),
      )
      const h = (w * image.naturalHeight) / image.naturalWidth
      if (p.x < -w * 3 || p.y < -h * 3 || p.x > width + w * 3 || p.y > height + h * 3) continue
      if (ship.at === "sea") drawWake(context, ship, w, h, projection, clock)
      // A soft shadow on the water under the hull, and a gentle lift and fall
      // on the swell, a game pixel at a time.
      const bob = still
        ? 0
        : Math.round(Math.sin(clock / 900 + ship.id.length) * 0.75) *
          Math.max(1, Math.round(scale / 16))
      context.fillStyle = "rgb(0 24 64 / 0.28)"
      context.beginPath()
      context.ellipse(p.x, p.y + h * 0.28, w * 0.5, h * 0.2, 0, 0, Math.PI * 2)
      context.fill()
      context.save()
      context.translate(p.x, p.y + bob)
      if (ship.flip) context.scale(-1, 1)
      context.drawImage(image, -w / 2, -h / 2, w, h)
      context.restore()
      names.push({ name: ship.name, x: p.x, y: p.y + h / 2 + 4 * ratio })
    }

    const train = trainAt(living, now)
    const trainName = train && drawTrain(context, living, train, projection)
    if (train && trainName) names.push({ name: living.train!.name, ...trainName })

    // The game multiplies each palette color by the period's tint; multiply
    // over the canvas does the same to what's drawn.
    const [r, g, b] = tintAt(living, now)
    if (r < 255 || g < 255 || b < 255) {
      context.globalCompositeOperation = "multiply"
      context.fillStyle = `rgb(${Math.round(r)} ${Math.round(g)} ${Math.round(b)})`
      context.fillRect(0, 0, width, height)
      context.globalCompositeOperation = "source-over"
    }

    // The names last, untinted, so they read at night too.
    context.font = `${Math.round(16 * ratio)}px "Wayfarer Text", monospace`
    context.textAlign = "center"
    context.textBaseline = "top"
    for (const { name, x, y } of names) drawName(context, name, x, y, ratio)
    context.textBaseline = "alphabetic"
  }

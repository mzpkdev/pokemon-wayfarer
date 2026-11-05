// Draws a shot of the world on the canvas, and makes the timeline's shots.
// Everything drawn is the website's own code (`~/world-map`); this file only
// decides what it draws when.
import { type Camera, fit as fitView, fly as flyEased } from "~/world-map/camera"
import { cloudsOverlay } from "~/world-map/clouds"
import { DAILY_SEED, dailyOn, loadDaily, placedMaps } from "~/world-map/daily"
import { type DailyScene, dailyOverlay } from "~/world-map/dailyView"
import { type Bounds, loadWorld, type World } from "~/world-map/data"
import { loadEncounters } from "~/world-map/encounters"
import {
  BADGES_SRC,
  badgeOverlays,
  callingFor,
  type Landmarks,
  landmarkBounds,
  loadLandmarks,
  type StopClock,
} from "~/world-map/landmarks"
import { lightsOverlay } from "~/world-map/lights"
import { type LivingWorld, livingOverlay, loadLiving, shipsAt, tintAt } from "~/world-map/living"
import { loadNotables, notableAt, type Notables, notablesOverlay } from "~/world-map/notables"
import { type Overlay, type Renderer, rendererFor } from "~/world-map/render"
import { riderAt } from "~/world-map/rider"
import { seaOverlay } from "~/world-map/sea"
import { sightingsOverlay } from "~/world-map/sightings"
import {
  focusIn,
  openingIn,
  placeAt,
  rideMs,
  routeIn,
  type Start,
  type StartId,
  startOf,
  STARTS,
} from "~/world-map/starts"
import { loadWater } from "~/world-map/water"
import { weatherOverlay } from "~/world-map/weather"
import { stepped } from "~/world-map/fade"
import {
  HOMES,
  loadWild,
  timeline as waveTimeline,
  type WildClock,
  wildOverlay,
} from "~/world-map/wild"

import { type CastImages, castSprites, EMOTES, emotesOverlay, poseCast, SHEETS } from "./actors"
import { boxCover } from "./box"
import { fractionOf, type Slot } from "./schedule"
import { ALL_LAYERS, type Shot } from "./shot"
import {
  DEFAULT_MINUTES,
  type Direction,
  DIRECTIONS,
  type Moment,
  type Span,
  type Target,
} from "./timeline"

/** The website's `ease`, which `fly` applies: gentle at both ends. */
const easeBoth = (t: number) => (t < 0.5 ? 4 * t * t * t : 1 - (-2 * t + 2) ** 3 / 2)
/** Its inverse, so a flight can be given another shape and still pass through the website's `fly`. */
const uneaseBoth = (y: number) => {
  let lo = 0
  let hi = 1
  for (let i = 0; i < 40; i++) {
    const mid = (lo + hi) / 2
    if (easeBoth(mid) < y) lo = mid
    else hi = mid
  }
  return (lo + hi) / 2
}
const SHAPES = {
  both: (t: number) => t,
  in: (t: number) => uneaseBoth(t * t),
  out: (t: number) => uneaseBoth(1 - (1 - t) ** 2),
  none: (t: number) => uneaseBoth(t),
}

/** The camera `t` of the way from `from` to `to`, the flight shaped by `ease` (see `Keyframe`). */
export const fly = (from: Camera, to: Camera, t: number, ease: keyof typeof SHAPES = "both") =>
  flyEased(from, to, SHAPES[ease](Math.min(1, Math.max(0, t))))

/** The closest zoom the rider is drawn at; further out he would be a speck. */
export const RIDER_MIN_ZOOM = 6
/** The zoom of the close-up at the end of a ride, in CSS pixels per tile. */
export const HOME_ZOOM = 24
/** Each half of the warp between rides, in seconds: the game's 8 steps to black, and back. */
const WARP_SECONDS = 0.35

export type Stage = {
  world: World
  landmarks: Landmarks
  living: LivingWorld
  notables: Notables
  renderer: Renderer
  sea: Overlay
  weather: Overlay
  clouds: Overlay
  ships: Overlay
  lights: Overlay
  sightings: Overlay
  pins: Overlay[]
  stopClock: StopClock
  wild: Overlay
  wildClock: WildClock
  daily: Overlay
  /** The daily roads' day on show, and when it came up, so its people pop in. */
  today: { day: number; scene: DailyScene; since: number } | null
  dailyFor: (day: number) => DailyScene
  /** Each start's rider on their bike. */
  riders: Record<StartId, HTMLImageElement>
  /** The actors' sheets and the bubbles over their heads. */
  cast: CastImages
  /** The game minute the overlays read; `drawShot` sets it from the shot. */
  clock: { minute: number; ships: number }
}

/** The real clock the sea, the clouds and the walkers' steps run on. */
export const now = () => performance.now()

const loadImage = (src: string) =>
  new Promise<HTMLImageElement>((resolve, reject) => {
    const image = new Image()
    image.addEventListener("load", () => resolve(image), { once: true })
    image.addEventListener("error", () => reject(new Error(`Could not load ${src}`)), {
      once: true,
    })
    image.src = src
  })

/** Loads the world and everything drawn over it. */
export const loadStage = async (): Promise<Stage> => {
  const world = await loadWorld()
  const [water, landmarks, living, notables, badges, daily, trainers, encounters, icons, wild] =
    await Promise.all([
      loadWater(world.meta),
      loadLandmarks(),
      loadLiving(),
      loadNotables(),
      loadImage(BADGES_SRC),
      loadDaily(),
      loadImage("/world-map/trainers.png"),
      loadEncounters(),
      loadImage("/world-map/icons.png"),
      loadWild(),
    ])
  const riders = Object.fromEntries(
    await Promise.all(
      STARTS.map(async (start) => [start.id, await loadImage(start.rider)] as const),
    ),
  ) as Record<StartId, HTMLImageElement>
  const images = async <K extends string>(sources: Record<K, string>) =>
    Object.fromEntries(
      await Promise.all(
        Object.entries<string>(sources).map(async ([id, src]) => [id, await loadImage(src)]),
      ),
    ) as Record<K, HTMLImageElement>
  const cast: CastImages = {
    sheets: await images(
      Object.fromEntries(Object.entries(SHEETS).map(([id, sheet]) => [id, sheet.src])) as Record<
        keyof typeof SHEETS,
        string
      >,
    ),
    emotes: await images(EMOTES),
  }
  const clock = { minute: DEFAULT_MINUTES, ships: DEFAULT_MINUTES }
  const minutes = () => clock.minute
  // Made by a call, not written out: the bundler takes a literal's `alpha: 0` for a constant,
  // since it is only ever set through `stage`, and drops the pins' drawing as dead code.
  const stopClock: StopClock = structuredClone({ alpha: 0, elapsed: 0, animate: true, circuit: 0 })
  const wildClock: WildClock = { start: now(), still: null }
  const roads = placedMaps(daily)
  const stage: Stage = {
    world,
    landmarks,
    living,
    notables,
    renderer: rendererFor(world),
    sea: seaOverlay(water, true, now),
    weather: weatherOverlay(world.meta, true, now),
    clouds: cloudsOverlay(true, now),
    // Kept off the bridge decks, as on the site; the tint is drawn on its own (`shot.layers.tint`).
    ships: livingOverlay(living, () => clock.ships, { tint: false, decks: world.meta.overpasses }),
    lights: lightsOverlay(world.meta, living, minutes, true, now),
    // Livelier than the site's (a fifth of the time), so a few seconds of grass show a few.
    sightings: sightingsOverlay(encounters, icons, minutes, true, now, { quiet: 0.5 }),
    pins: badgeOverlays(landmarks, badges, stopClock),
    stopClock,
    wild: wildOverlay(wild, wildClock, true, now),
    wildClock,
    daily: () => {},
    today: null,
    dailyFor: (day) => dailyOn(daily, roads, day, DAILY_SEED),
    riders,
    cast,
    clock,
  }
  stage.daily = dailyOverlay({ image: trainers, ...daily.sheet }, () => stage.today, true, now)
  return stage
}

const days = new Map<number, DailyScene>()
const dayScene = (stage: Stage, day: number) => {
  let scene = days.get(day)
  if (!scene) days.set(day, (scene = stage.dailyFor(day)))
  return scene
}

const union = (boxes: Bounds[]): Bounds => ({
  left: Math.min(...boxes.map((b) => b.left)),
  top: Math.min(...boxes.map((b) => b.top)),
  right: Math.max(...boxes.map((b) => b.right)),
  bottom: Math.max(...boxes.map((b) => b.bottom)),
})

const warned = new Set<string>()
const warn = (message: string) => {
  if (warned.has(message)) return
  warned.add(message)
  console.warn(message)
}

export const wholeWorld = (stage: Stage): Bounds => ({
  left: 0,
  top: 0,
  right: stage.world.meta.width,
  bottom: stage.world.meta.height,
})

/**
 * The camera for a target in a view `width` by `height` CSS pixels. A target
 * fitted to bounds fits the view above its bottom `below` pixels, where the
 * text box sits, so nothing the shot is framing hides under the box.
 */
export const resolve = (
  target: Target,
  stage: Stage,
  minute: number,
  width: number,
  height: number,
  below = 0,
): Camera => {
  const { meta } = stage.world
  const whole = wholeWorld(stage)
  const fit = (bounds: Bounds, room?: number): Camera => {
    const camera = fitView(bounds, width, height - below, room)
    return { ...camera, y: camera.y + below / 2 / camera.zoom }
  }
  switch (target.kind) {
    case "world":
      return fit(whole, target.room)
    case "point":
      return { x: target.x, y: target.y, zoom: target.zoom }
    case "regions": {
      const boxes = target.names.map((name) => {
        const region = meta.regions.find((r) => r.name === name)
        if (!region) warn(`No region named ${name}; the camera shows the world instead.`)
        return (
          region?.bounds ??
          (region
            ? {
                left: region.x - 100,
                top: region.y - 60,
                right: region.x + 100,
                bottom: region.y + 60,
              }
            : whole)
        )
      })
      return fit(union(boxes), target.room)
    }
    case "map": {
      const map = meta.maps.find((m) => m.name === target.name)
      if (!map) {
        warn(`No map named ${target.name}; the camera shows the world instead.`)
        return fit(whole)
      }
      return {
        x: map.x + (target.x ?? map.width / 2),
        y: map.y + (target.y ?? map.height / 2),
        zoom: target.zoom,
      }
    }
    case "circuit": {
      const bounds = landmarkBounds(stage.landmarks, target.room ?? 0.9)
      return fit(bounds ?? whole, target.room)
    }
    case "ship": {
      const ship = shipsAt(stage.living, minute).find((s) => s.id === target.id)
      if (!ship) {
        warn(`No ship named ${target.id}; the camera shows the world instead.`)
        return fit(whole)
      }
      return { x: ship.x, y: ship.y, zoom: target.zoom }
    }
    case "notable": {
      const who = notableAt(stage.notables, target.slug, minute)
      if (!who) {
        warn(
          `${target.slug} isn't on the map at ${Math.floor(minute / 60)}:00; the camera shows the world instead.`,
        )
        return fit(whole)
      }
      return { x: who.x + (target.dx ?? 0), y: who.y + (target.dy ?? 0), zoom: target.zoom }
    }
  }
}

/** The camera `fraction` of the way through a scene, flying between its keyframes. */
export const cameraAt = (
  direction: Direction<number>,
  fraction: number,
  stage: Stage,
  minute: number,
  width: number,
  height: number,
  below = 0,
): Camera => {
  const shots = direction.shots.toSorted((a, b) => a.at - b.at)
  const first = shots[0]
  if (!first) return resolve({ kind: "world" }, stage, minute, width, height, below)
  const at = (shot: typeof first) => resolve(shot.to, stage, minute, width, height, below)
  if (fraction <= first.at) return at(first)
  for (let i = 0; i < shots.length - 1; i++) {
    const a = shots[i]!
    const b = shots[i + 1]!
    if (fraction <= b.at) return fly(at(a), at(b), (fraction - a.at) / (b.at - a.at), b.ease)
  }
  return at(shots[shots.length - 1]!)
}

/** The ride's two shots: the start's opening, and the close-up at the door. */
export const rideShots = (stage: Stage, start: Start, width: number, height: number) => {
  const { meta } = stage.world
  return {
    opening: fitView(openingIn(meta, start), width, height),
    home: { ...focusIn(meta, start), zoom: HOME_ZOOM },
    route: routeIn(meta, start),
    ms: rideMs(routeIn(meta, start)),
  }
}

/** The name the map name popup shows while the rider is on screen. */
export const riderPlace = (stage: Stage, rider: Shot["rider"]) => {
  if (!rider) return null
  const start = startOf(rider.start)
  const sprite = riderAt(
    routeIn(stage.world.meta, start),
    rider.progress,
    stage.riders[rider.start],
  )
  return placeAt(stage.world.meta, start, { x: sprite.x + 1, y: sprite.y + 2 })
}

const minuteAt = ({ clock }: Direction<number>, fraction: number) => {
  if ("at" in clock) return clock.at
  const keys: [number, number][] =
    "keys" in clock
      ? clock.keys
      : [
          [0, clock.from],
          [1, clock.to],
        ]
  const next = keys.findIndex(([at]) => fraction < at)
  if (next === 0) return keys[0]![1]
  if (next === -1) return keys[keys.length - 1]![1]
  const [a, from] = keys[next - 1]!
  const [b, to] = keys[next]!
  return from + ((to - from) * (fraction - a)) / (b - a)
}

/** The zoom a notable's `minHeight` is given for; further out it shrinks with the zoom. */
const WALKERS_CLOSE_ZOOM = 16
/** The least height notables are drawn at far out, as the map's own overlay has it. */
const WALKERS_FAR_HEIGHT = 26

const mix = (value: number | [number, number], fraction: number) =>
  typeof value === "number" ? value : value[0] + (value[1] - value[0]) * fraction

/**
 * The direction for a slot, its moments as fractions of the scene: a phrase
 * falls where the slot's words have it. A scene with none holds the whole world by day.
 */
export const directionFor = (slot: Slot): Direction<number> => {
  const direction: Direction = DIRECTIONS[slot.id] ?? {
    shots: [{ at: 0, to: { kind: "world" } }],
    clock: { at: DEFAULT_MINUTES },
  }
  const at = (moment: Moment) => fractionOf(slot, moment)
  const span = <S extends Span>(s: S) => ({ ...s, from: at(s.from), to: at(s.to) })
  const { shots, wild, rides, badges, title, cast, tosses, phone, flashes, portraits } = direction
  return {
    ...direction,
    cast: cast?.map((actor) => ({
      ...actor,
      marks: actor.marks.map((mark) => ({ ...mark, at: at(mark.at) })),
      from: actor.from === undefined ? undefined : at(actor.from),
      to: actor.to === undefined ? undefined : at(actor.to),
      face:
        typeof actor.face === "object"
          ? actor.face.map((turn) => ({ ...turn, at: at(turn.at) }))
          : actor.face,
      emotes: actor.emotes?.map((emote) => ({ ...emote, at: at(emote.at) })),
      hops: actor.hops?.map(at),
    })),
    tosses: tosses?.map((toss) => ({ ...toss, span: span(toss.span) })),
    phone: phone && { ...span(phone), named: at(phone.named) },
    portraits: portraits && {
      from: at(portraits.from),
      pictures: portraits.pictures.map((picture) => ({ ...picture, at: at(picture.at) })),
    },
    flashes: flashes?.map(at),
    shots: shots.map((shot) => ({ ...shot, at: at(shot.at) })),
    wild:
      typeof wild === "object"
        ? { from: at(wild.from), wave: wild.wave && { ...wild.wave, at: at(wild.wave.at) } }
        : wild,
    rides: rides?.map(span),
    badges: badges && { ...badges, earn: badges.earn && span(badges.earn) },
    title: title && span(title),
  }
}

/** Least height a daily trainer is drawn at, in CSS pixels, as the daily overlay has it. */
const DAILY_MIN_HEIGHT = 28

/**
 * The game's yellow down arrow, bobbing over the daily trainer standing at a
 * world cell: the sprite stands on the cell's bottom edge at least two tiles tall.
 */
const markOverlay =
  ({ x, y }: { x: number; y: number }): Overlay =>
  (context, { scale, ratio, toScreen }) => {
    const feet = toScreen(x + 0.5, y + 1)
    const tall = Math.max(scale * 2, DAILY_MIN_HEIGHT * ratio)
    const size = Math.round(6 * ratio)
    const bob = Math.floor(now() / 300) % 2 ? size * 0.5 : 0
    const top = feet.y - tall - size * 2.2 + bob
    context.fillStyle = "rgb(255 214 0)"
    context.strokeStyle = "rgb(0 0 0 / 0.8)"
    context.lineWidth = Math.max(1, ratio)
    context.beginPath()
    context.moveTo(feet.x - size, top)
    context.lineTo(feet.x + size, top)
    context.lineTo(feet.x, top + size * 1.2)
    context.closePath()
    context.fill()
    context.stroke()
  }

/** The game's time of day tint, multiplied over what is drawn so far, as the game does to its palettes. */
const tintOverlay =
  (living: Stage["living"], minutes: () => number): Overlay =>
  (context) => {
    const [r, g, b] = tintAt(living, minutes())
    if (r >= 255 && g >= 255 && b >= 255) return
    context.globalCompositeOperation = "multiply"
    context.fillStyle = `rgb(${Math.round(r)} ${Math.round(g)} ${Math.round(b)})`
    context.fillRect(0, 0, context.canvas.width, context.canvas.height)
    context.globalCompositeOperation = "source-over"
  }

/** The timeline's shot `fraction` of the way through `slot`, for a view `width` by `height`. */
export const shotFor = (
  stage: Stage,
  slot: Slot,
  fraction: number,
  width: number,
  height: number,
): Shot => {
  const direction = directionFor(slot)
  const minute = minuteAt(direction, fraction)
  const local = fraction * slot.length
  // Wide shots frame what they show above the text box, in a scene that has one.
  const below = slot.boxed === false ? 0 : boxCover(width)
  let camera = cameraAt(direction, fraction, stage, minute, width, height, below)
  let rider: Shot["rider"]
  // The scene's own fades from and to black, then the warps between rides.
  let fade = 0
  if (direction.fade?.in) fade = Math.max(fade, stepped(1 - local / direction.fade.in))
  if (direction.fade?.out)
    fade = Math.max(fade, stepped(1 - (slot.length - local) / direction.fade.out))
  const rides = direction.rides?.toSorted((a, b) => a.from - b.from) ?? []
  if (rides.length) {
    // The ride under way, or the last one done: its rider waits at the door until the next.
    const index = Math.max(
      0,
      rides.findLastIndex((ride) => ride.from <= fraction),
    )
    const ride = rides[index]!
    const progress = Math.min(1, Math.max(0, (fraction - ride.from) / (ride.to - ride.from)))
    rider = { start: ride.start, progress }
    const shots = rideShots(stage, startOf(ride.start), width, height)
    const home = ride.look
      ? { ...shots.home, x: shots.home.x + ride.look.dx, y: shots.home.y + ride.look.dy }
      : shots.home
    camera = fly(shots.opening, home, progress)
    // The warp to each ride after the first: black at its start, fading out before and in after.
    for (const next of rides.slice(1)) {
      const off = Math.abs(fraction - next.from) * slot.length
      if (off < WARP_SECONDS) fade = Math.max(fade, stepped(1 - off / WARP_SECONDS))
    }
  }
  return {
    camera,
    minute,
    shipsMinute: direction.shipsAt,
    labels: camera.zoom < 12,
    layers: { ...ALL_LAYERS, ...direction.layers },
    walkers: direction.notables && {
      ...direction.notables,
      minHeight:
        direction.notables.minHeight &&
        Math.max(
          WALKERS_FAR_HEIGHT,
          direction.notables.minHeight * Math.min(1, camera.zoom / WALKERS_CLOSE_ZOOM),
        ),
    },
    pins: direction.badges && {
      alpha: 1,
      elapsed: local * 1000,
      circuit: mix(direction.badges.circuit, fraction),
      ...earnedBy(direction.badges.earn, local, slot.length),
    },
    daily: direction.daily && {
      day:
        "day" in direction.daily
          ? direction.daily.day
          : 1 + Math.min(direction.daily.days - 1, Math.floor(fraction * direction.daily.days)),
    },
    mark: direction.mark,
    // The waves' cycle began at a set point of the scene, so they are the same at a time however it was reached.
    wild: wildAt(direction.wild, local, slot.length),
    rider,
    cast: castAt(stage, direction, local, slot.length),
    fade,
    flash: flashAt(direction.flashes, local, slot.length),
  }
}

/** The scene's actors `local` seconds into it; their moments are fractions of the scene, their emotes' lengths seconds. */
const castAt = (stage: Stage, direction: Direction<number>, local: number, length: number) => {
  if (!direction.cast && !direction.tosses) return undefined
  const s = (fraction: number) => fraction * length
  const actors = (direction.cast ?? []).map((actor) => ({
    ...actor,
    marks: actor.marks.map((mark) => ({ ...mark, at: s(mark.at) })),
    from: actor.from === undefined ? undefined : s(actor.from),
    to: actor.to === undefined ? undefined : s(actor.to),
    face:
      typeof actor.face === "object"
        ? actor.face.map((turn) => ({ ...turn, at: s(turn.at) }))
        : actor.face,
    emotes: actor.emotes?.map((emote) => ({ ...emote, at: s(emote.at) })),
    hops: actor.hops?.map(s),
  }))
  const tosses = (direction.tosses ?? []).map((toss) => ({
    ...toss,
    span: { from: s(toss.span.from), to: s(toss.span.to) },
  }))
  return poseCast(actors, tosses, local, (name) => {
    if (!name) return { x: 0, y: 0 }
    const map = stage.world.meta.maps.find((m) => m.name === name)
    if (!map) warn(`No map named ${name}; its actors stand at the world's corner.`)
    return map ?? { x: 0, y: 0 }
  })
}

/** Seconds a flash into battle lasts: white at once, then gone. */
const FLASH_SECONDS = 0.14

const flashAt = (flashes: number[] | undefined, local: number, length: number) => {
  const since = flashes?.map((at) => local - at * length).filter((t) => t >= 0 && t < FLASH_SECONDS)
  return since?.length ? stepped(1 - Math.min(...since) / FLASH_SECONDS) : 0
}

/**
 * The badges earned `local` seconds into a scene, one after another across
 * the span, each with the real-clock moment it landed (so its sparkle plays
 * the same however the time was reached), and the leagues that calls up.
 */
const earnedBy = (
  earn: NonNullable<Direction<number>["badges"]>["earn"],
  local: number,
  length: number,
): Pick<Shot, "pins">["pins"] extends infer P ? Partial<NonNullable<P>> : never => {
  if (!earn) return {}
  const from = earn.from * length
  const each = ((earn.to - earn.from) * length) / earn.badges.length
  const earned = new Set<number>()
  const earnedAt = new Map<number, number>()
  earn.badges.forEach((badge, i) => {
    const at = from + each * (i + 1)
    if (local < at) return
    earned.add(badge)
    earnedAt.set(badge, now() - (local - at) * 1000)
  })
  return { earned, earnedAt, calling: callingFor(earned) }
}

/**
 * The wild waves' clock `local` seconds into a scene, once they have begun: the loop
 * starts at `from`, or wherever has the scene's `wave` begin at its moment.
 */
const wildAt = (wild: Direction<number>["wild"], local: number, length: number): Shot["wild"] => {
  if (!wild) return undefined
  const from = typeof wild === "object" ? wild.from * length : 0
  if (local < from) return undefined
  const wave = typeof wild === "object" && wild.wave
  const index = wave ? HOMES.findIndex((home) => home.id === wave.home) : -1
  if (wave && index < 0) warn(`No wild home named ${wave.home}; the waves start at \`from\`.`)
  const loop = wave && index >= 0 ? wave.at * length - waveTimeline().starts[index]! / 1000 : from
  return { start: now() - (local - loop) * 1000, still: null }
}

const EMPTY = new Set<number>()

/**
 * Draws the shot onto the canvas. True when some map chunks are still to come
 * and the frame should be drawn again.
 */
export const drawShot = (
  stage: Stage,
  context: CanvasRenderingContext2D,
  shot: Shot,
  extra: Overlay[] = [],
): boolean => {
  const { canvas } = context
  const width = canvas.clientWidth
  const height = canvas.clientHeight
  stage.clock.minute = shot.minute
  stage.clock.ships = shot.shipsMinute ?? shot.minute
  const minutes = () => stage.clock.minute

  // In the website's order: the sea under all, the names last.
  const overlays: Overlay[] = []
  if (shot.layers.sea) overlays.push(stage.sea)
  if (shot.layers.weather) overlays.push(stage.weather)
  if (shot.layers.clouds) overlays.push(stage.clouds)
  if (shot.pins) {
    const { stopClock } = stage
    stopClock.alpha = shot.pins.alpha
    stopClock.elapsed = shot.pins.elapsed
    stopClock.circuit = shot.pins.circuit
    stopClock.earned = shot.pins.earned
    stopClock.earnedAt = shot.pins.earnedAt
    stopClock.calling = shot.pins.calling ?? {}
    stopClock.league = shot.pins.league ?? null
    overlays.push(...stage.pins)
  }
  if (shot.layers.sightings) overlays.push(stage.sightings)
  if (shot.wild) {
    stage.wildClock.start = shot.wild.start
    stage.wildClock.still = shot.wild.still
    overlays.push(stage.wild)
  }
  if (shot.daily) {
    const { day } = shot.daily
    if (stage.today?.day !== day) stage.today = { day, scene: dayScene(stage, day), since: now() }
    overlays.push(stage.daily)
  }
  if (shot.layers.walkers) {
    const { only, except, ...rest } = shot.walkers ?? {}
    const picked = only && new Set(only)
    const hidden = new Set(except)
    overlays.push(
      notablesOverlay(stage.notables, minutes, {
        ...rest,
        only:
          picked || hidden.size
            ? (slug) => (picked?.has(slug) ?? true) && !hidden.has(slug)
            : undefined,
      }),
    )
  }
  if (shot.cast?.length) overlays.push(emotesOverlay(shot.cast, stage.cast))
  if (shot.mark) overlays.push(markOverlay(shot.mark))
  if (shot.layers.ships) overlays.push(stage.ships)
  // Over everything the world draws, people and ships too, as the game tints its palettes; the lights shine through.
  if (shot.layers.tint) overlays.push(tintOverlay(stage.living, minutes))
  if (shot.layers.lights) overlays.push(stage.lights)
  overlays.push(...extra)

  const sprites =
    shot.rider && shot.camera.zoom >= RIDER_MIN_ZOOM
      ? [
          riderAt(
            routeIn(stage.world.meta, startOf(shot.rider.start)),
            shot.rider.progress,
            stage.riders[shot.rider.start],
          ),
        ]
      : []
  if (shot.cast?.length) sprites.push(...castSprites(shot.cast, stage.cast))

  const pending = stage.renderer.draw(context, {
    camera: shot.camera,
    focus: { x: width / 2, y: height / 2 },
    labelWidth: width,
    labels: shot.labels,
    sprites,
    overlays,
  })
  if (shot.fade > 0) {
    context.setTransform(1, 0, 0, 1, 0, 0)
    context.fillStyle = `rgb(0 0 0 / ${shot.fade})`
    context.fillRect(0, 0, canvas.width, canvas.height)
  }
  if (shot.flash) {
    context.setTransform(1, 0, 0, 1, 0, 0)
    context.fillStyle = `rgb(255 255 255 / ${shot.flash})`
    context.fillRect(0, 0, canvas.width, canvas.height)
  }
  return pending
}

/** Draws the timeline's frame `fraction` of the way through `slot`; see `drawShot`. */
export const drawFrame = (
  stage: Stage,
  context: CanvasRenderingContext2D,
  slot: Slot,
  fraction: number,
): { pending: boolean; shot: Shot } => {
  const shot = shotFor(
    stage,
    slot,
    fraction,
    context.canvas.clientWidth,
    context.canvas.clientHeight,
  )
  return { pending: drawShot(stage, context, shot), shot }
}

export { EMPTY as NO_BADGES }

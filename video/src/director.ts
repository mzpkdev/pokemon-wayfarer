// Draws a shot of the world on the canvas, and makes the timeline's shots.
// Everything drawn is the website's own code (`~/world-map`); this file only
// decides what it draws when.
import { type Camera, fit, fly as flyEased } from "~/world-map/camera"
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
import { type LivingWorld, livingOverlay, loadLiving, shipsAt } from "~/world-map/living"
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
import { loadWild, type WildClock, wildOverlay } from "~/world-map/wild"

import type { Slot } from "./schedule"
import { ALL_LAYERS, type Shot } from "./shot"
import { DEFAULT_MINUTES, type Direction, DIRECTIONS, type Target } from "./timeline"

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
  /** The game minute the overlays read; `drawShot` sets it from the shot. */
  clock: { minute: number }
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
  const clock = { minute: DEFAULT_MINUTES }
  const minutes = () => clock.minute
  const stopClock: StopClock = { alpha: 0, elapsed: 0, animate: true, circuit: 0 }
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
    ships: livingOverlay(living, minutes),
    lights: lightsOverlay(world.meta, living, minutes, true, now),
    sightings: sightingsOverlay(encounters, icons, minutes, true, now),
    pins: badgeOverlays(landmarks, badges, stopClock),
    stopClock,
    wild: wildOverlay(wild, wildClock, true, now),
    wildClock,
    daily: () => {},
    today: null,
    dailyFor: (day) => dailyOn(daily, roads, day, DAILY_SEED),
    riders,
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

/** The camera for a target in a view `width` by `height` CSS pixels. */
export const resolve = (
  target: Target,
  stage: Stage,
  minute: number,
  width: number,
  height: number,
): Camera => {
  const { meta } = stage.world
  const whole = wholeWorld(stage)
  switch (target.kind) {
    case "world":
      return fit(whole, width, height, target.room)
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
      return fit(union(boxes), width, height, target.room)
    }
    case "map": {
      const map = meta.maps.find((m) => m.name === target.name)
      if (!map) {
        warn(`No map named ${target.name}; the camera shows the world instead.`)
        return fit(whole, width, height)
      }
      return {
        x: map.x + (target.x ?? map.width / 2),
        y: map.y + (target.y ?? map.height / 2),
        zoom: target.zoom,
      }
    }
    case "circuit": {
      const bounds = landmarkBounds(stage.landmarks, target.room ?? 0.9)
      return fit(bounds ?? whole, width, height, target.room)
    }
    case "ship": {
      const ship = shipsAt(stage.living, minute).find((s) => s.id === target.id)
      if (!ship) {
        warn(`No ship named ${target.id}; the camera shows the world instead.`)
        return fit(whole, width, height)
      }
      return { x: ship.x, y: ship.y, zoom: target.zoom }
    }
    case "notable": {
      const who = notableAt(stage.notables, target.slug, minute)
      if (!who) {
        warn(
          `${target.slug} isn't on the map at ${Math.floor(minute / 60)}:00; the camera shows the world instead.`,
        )
        return fit(whole, width, height)
      }
      return { x: who.x, y: who.y, zoom: target.zoom }
    }
  }
}

/** The camera `fraction` of the way through a scene, flying between its keyframes. */
export const cameraAt = (
  direction: Direction,
  fraction: number,
  stage: Stage,
  minute: number,
  width: number,
  height: number,
): Camera => {
  const shots = direction.shots.toSorted((a, b) => a.at - b.at)
  const first = shots[0]
  if (!first) return fit(wholeWorld(stage), width, height)
  const at = (shot: typeof first) => resolve(shot.to, stage, minute, width, height)
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
    opening: fit(openingIn(meta, start), width, height),
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

const minuteAt = ({ clock }: Direction, fraction: number) => {
  if ("at" in clock) return clock.at
  const { from, to } = clock.over ?? { from: 0, to: 1 }
  const t = Math.min(1, Math.max(0, (fraction - from) / (to - from)))
  return clock.from + (clock.to - clock.from) * t
}

/** The zoom a notable's `minHeight` is given for; further out it shrinks with the zoom. */
const WALKERS_CLOSE_ZOOM = 16
/** The least height notables are drawn at far out, as the map's own overlay has it. */
const WALKERS_FAR_HEIGHT = 26

const mix = (value: number | [number, number], fraction: number) =>
  typeof value === "number" ? value : value[0] + (value[1] - value[0]) * fraction

/** The direction for a slot; a scene with none holds the whole world by day. */
export const directionFor = (slot: Slot): Direction =>
  DIRECTIONS[slot.id] ?? {
    shots: [{ at: 0, to: { kind: "world" } }],
    clock: { at: DEFAULT_MINUTES },
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
  let camera = cameraAt(direction, fraction, stage, minute, width, height)
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
    camera = fly(shots.opening, shots.home, progress)
    // The warp to each ride after the first: black at its start, fading out before and in after.
    for (const next of rides.slice(1)) {
      const off = Math.abs(fraction - next.from) * slot.length
      if (off < WARP_SECONDS) fade = Math.max(fade, stepped(1 - off / WARP_SECONDS))
    }
  }
  return {
    camera,
    minute,
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
      day: 1 + Math.min(direction.daily.days - 1, Math.floor(fraction * direction.daily.days)),
    },
    // The waves' cycle began at a set point of the scene, so they are the same at a time however it was reached.
    wild: wildAt(direction.wild, local, slot.length),
    rider,
    fade,
  }
}

/**
 * The badges earned `local` seconds into a scene, one after another across
 * the span, each with the real-clock moment it landed (so its sparkle plays
 * the same however the time was reached), and the leagues that calls up.
 */
const earnedBy = (
  earn: NonNullable<Direction["badges"]>["earn"],
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

/** The wild waves' clock `local` seconds into a scene, once they have begun. */
const wildAt = (wild: Direction["wild"], local: number, length: number): Shot["wild"] => {
  if (!wild) return undefined
  const from = typeof wild === "object" ? wild.from * length : 0
  return local >= from ? { start: now() - (local - from) * 1000, still: null } : undefined
}

const EMPTY = new Set<number>()

/**
 * Draws the shot onto the canvas. True when some map chunks are still to come
 * and the frame should be drawn again.
 */
export const drawShot = (stage: Stage, context: CanvasRenderingContext2D, shot: Shot): boolean => {
  const { canvas } = context
  const width = canvas.clientWidth
  const height = canvas.clientHeight
  stage.clock.minute = shot.minute
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
    const { only, ...rest } = shot.walkers ?? {}
    const picked = only && new Set(only)
    overlays.push(
      notablesOverlay(stage.notables, minutes, {
        ...rest,
        only: picked ? (slug) => picked.has(slug) : undefined,
      }),
    )
  }
  if (shot.layers.ships) overlays.push(stage.ships)
  if (shot.layers.lights) overlays.push(stage.lights)

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

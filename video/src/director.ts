// Draws one frame of the video: the website's world map for the scene's
// camera, at the scene's time of day, with the overlays the scene asks for.
// Everything here is the website's own code (`~/world-map`); this file only
// decides what it draws when.
import { type Camera, fit, fly } from "~/world-map/camera"
import { cloudsOverlay } from "~/world-map/clouds"
import { type Bounds, loadWorld, type World } from "~/world-map/data"
import {
  BADGES_SRC,
  badgeOverlays,
  type Landmarks,
  landmarkBounds,
  loadLandmarks,
  type StopClock,
} from "~/world-map/landmarks"
import { lightsOverlay } from "~/world-map/lights"
import { type LivingWorld, livingOverlay, loadLiving } from "~/world-map/living"
import { loadNotables, notableAt, type Notables, notablesOverlay } from "~/world-map/notables"
import { type Overlay, type Renderer, rendererFor } from "~/world-map/render"
import { seaOverlay } from "~/world-map/sea"
import { loadWater } from "~/world-map/water"
import { weatherOverlay } from "~/world-map/weather"

import type { Slot } from "./schedule"
import { DEFAULT_MINUTES, type Direction, DIRECTIONS, type Target } from "./timeline"

export type Stage = {
  world: World
  landmarks: Landmarks
  living: LivingWorld
  notables: Notables
  renderer: Renderer
  /** The sea, the weather and the clouds, which run on the real clock. */
  ambient: Overlay[]
  ships: Overlay
  lights: Overlay
  pins: Overlay[]
  stopClock: StopClock
  /** The game minute the overlays read; `drawFrame` sets it for the scene. */
  clock: { minute: number }
}

/** The real clock the sea, the clouds and the walkers' steps run on. */
const now = () => performance.now()

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
  const [water, landmarks, living, notables, badges] = await Promise.all([
    loadWater(world.meta),
    loadLandmarks(),
    loadLiving(),
    loadNotables(),
    loadImage(BADGES_SRC),
  ])
  const clock = { minute: DEFAULT_MINUTES }
  const minutes = () => clock.minute
  const stopClock: StopClock = {
    alpha: 0,
    elapsed: 0,
    animate: true,
    circuit: 0,
    earned: new Set(),
    earnedAt: new Map(),
    calling: {},
    league: null,
    selected: null,
  }
  return {
    world,
    landmarks,
    living,
    notables,
    renderer: rendererFor(world),
    ambient: [
      seaOverlay(water, true, now),
      weatherOverlay(world.meta, true, now),
      cloudsOverlay(true, now),
    ],
    ships: livingOverlay(living, minutes),
    lights: lightsOverlay(world.meta, living, minutes, true, now),
    pins: badgeOverlays(landmarks, badges, stopClock),
    stopClock,
    clock,
  }
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

/** The camera for a target in a view `width` by `height` CSS pixels. */
export const resolve = (
  target: Target,
  { world: { meta }, landmarks, notables }: Stage,
  minute: number,
  width: number,
  height: number,
): Camera => {
  const whole = { left: 0, top: 0, right: meta.width, bottom: meta.height }
  switch (target.kind) {
    case "world":
      return fit(whole, width, height, target.room)
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
      const bounds = landmarkBounds(landmarks, target.room ?? 0.9)
      return fit(bounds ?? whole, width, height, target.room)
    }
    case "notable": {
      const who = notableAt(notables, target.slug, minute)
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
  const { meta } = stage.world
  const shots = direction.shots.toSorted((a, b) => a.at - b.at)
  const first = shots[0]
  if (!first) return fit({ left: 0, top: 0, right: meta.width, bottom: meta.height }, width, height)
  const at = (shot: typeof first) => resolve(shot.to, stage, minute, width, height)
  if (fraction <= first.at) return at(first)
  for (let i = 0; i < shots.length - 1; i++) {
    const a = shots[i]!
    const b = shots[i + 1]!
    if (fraction <= b.at) return fly(at(a), at(b), (fraction - a.at) / (b.at - a.at))
  }
  return at(shots[shots.length - 1]!)
}

const minuteAt = (direction: Direction, fraction: number) =>
  "at" in direction.clock
    ? direction.clock.at
    : direction.clock.from + (direction.clock.to - direction.clock.from) * fraction

const mix = (value: number | [number, number], fraction: number) =>
  typeof value === "number" ? value : value[0] + (value[1] - value[0]) * fraction

/** The direction for a slot; a scene with none holds the whole world by day. */
export const directionFor = (slot: Slot): Direction =>
  DIRECTIONS[slot.id] ?? {
    shots: [{ at: 0, to: { kind: "world" } }],
    clock: { at: DEFAULT_MINUTES },
  }

/**
 * Draws the scene `fraction` of the way through `slot` onto the canvas. True
 * when some map chunks are still to come and the frame should be drawn again.
 */
export const drawFrame = (
  stage: Stage,
  context: CanvasRenderingContext2D,
  slot: Slot,
  fraction: number,
): boolean => {
  const { canvas } = context
  const width = canvas.clientWidth
  const height = canvas.clientHeight
  const direction = directionFor(slot)
  stage.clock.minute = minuteAt(direction, fraction)
  const camera = cameraAt(direction, fraction, stage, stage.clock.minute, width, height)

  const overlays: Overlay[] = [...stage.ambient]
  if (direction.badges) {
    stage.stopClock.alpha = 1
    stage.stopClock.elapsed = fraction * slot.length * 1000
    stage.stopClock.circuit = mix(direction.badges.circuit, fraction)
    overlays.push(...stage.pins)
  }
  if (direction.notables) {
    const { only, marker, minHeight } = direction.notables
    const picked = only && new Set(only)
    overlays.push(
      notablesOverlay(stage.notables, () => stage.clock.minute, {
        only: picked ? (slug) => picked.has(slug) : undefined,
        marker,
        minHeight,
      }),
    )
  } else overlays.push(notablesOverlay(stage.notables, () => stage.clock.minute))
  overlays.push(stage.ships, stage.lights)

  return stage.renderer.draw(context, {
    camera,
    focus: { x: width / 2, y: height / 2 },
    labelWidth: width,
    // Names help far out; close in, the caption says where we are.
    labels: camera.zoom < 12,
    overlays,
  })
}

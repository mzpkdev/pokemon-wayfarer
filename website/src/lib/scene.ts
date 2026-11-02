// The title scene's choreography, in GBA screen pixels and 60 Hz frames, taken
// from game/src/intro.c (Task_Scene1_PanUp) and game/src/title_screen.c.

import art from "~/generated/game-art.json"

export const SCREEN_WIDTH = 240
export const SCREEN_HEIGHT = 160
/** Frames the pan up from the Poké Ball to the mountains takes. */
export const PAN_FRAMES = 343

export type Layer = {
  src: string
  /** Vertical scroll (BGnVOFS) with the camera down at the Poké Ball. */
  ballOffset: number
  /** Vertical scroll once the camera has panned up to the mountains. */
  titleOffset: number
}

/** Back to front: BG3 (sky and mountains) never moves, BG0 (leaves) moves fastest. */
export const LAYERS: Layer[] = [
  { src: "/game/intro/bg3.png", ballOffset: 0, titleOffset: 0 },
  { src: "/game/intro/bg2.png", ballOffset: 80, titleOffset: -70 },
  { src: "/game/intro/bg1.png", ballOffset: 24, titleOffset: -147 },
  { src: "/game/intro/bg0.png", ballOffset: 40, titleOffset: -217 },
]

/** Scroll of a layer at `pan`, from 0 (down at the Poké Ball) to 1 (up at the title). */
export const layerOffset = (layer: Layer, pan: number): number =>
  Math.round(layer.ballOffset + (layer.titleOffset - layer.ballOffset) * pan)

/** The Poké Ball sits in the hollow and rides BG2 out of view as the camera pans up. */
export const POKEBALL = { src: "/game/intro/pokeball.png", x: 120, y: 111, size: 64 }

export const pokeballCenterY = (pan: number): number => {
  const bg2 = LAYERS[1]!
  return POKEBALL.y + (bg2.ballOffset - layerOffset(bg2, pan))
}

type Placed = { src: string; x: number; y: number; width: number; height: number }

const place = (name: keyof typeof art, originX: number, originY: number): Placed => {
  const entry = art[name]
  return {
    src: `/game/${name}`,
    x: originX + entry.x,
    y: originY + entry.y,
    width: entry.width,
    height: entry.height,
  }
}

/** Four 64x64 logo sprites centered at x 64-256, y 32. */
export const LOGO = place("title/logo.png", 32, 0)
/** Two 64x32 wordmark sprites centered at x 88 and 152, y 70. */
export const WORDMARK = place("title/wordmark.png", 56, 54)
/** Five 32x8 sprites centered at x 64-192, y 108. */
export const PRESS_START = place("title/press-start.png", 48, 104)

export const LOGO_RISE = { offset: 6, frames: 12, delay: 0 }
export const WORDMARK_RISE = { offset: 4, frames: 10, delay: 6 }

/** Pixels below the resting place a rising title sprite is drawn, `frame` frames into the reveal. */
export const riseOffset = (
  rise: { offset: number; frames: number; delay: number },
  frame: number,
): number | null => {
  const elapsed = frame - rise.delay
  if (elapsed < 0) return null
  const remaining = Math.max(0, rise.frames - elapsed)
  return Math.trunc((rise.offset * remaining) / rise.frames)
}

/** PRESS START shows while bit 4 of its frame counter is set: 16 frames on, 16 off. */
export const pressStartVisible = (frame: number): boolean => ((frame + 1) & 16) !== 0

/** The logo shine: a 12px window, skewed 12px between halves, sweeping 4px a frame. */
export const SHINE = { width: 12, skew: 12, speed: 4, every: 360 }

export type PasserFrame = { frame: number; duration: number }

export type Passer = {
  name: string
  src: string
  /** Size of one animation frame in the vertical strip. */
  size: [number, number]
  y: number
  speed: number
  offscreen: number
  run: PasserFrame[]
  bicycle?: boolean
  bob?: boolean
}

const loop = (frames: number[], duration: number): PasserFrame[] =>
  frames.map((frame) => ({ frame, duration }))

/** Sprites face left; they are mirrored when they pass rightward. */
export const PASSERS: Passer[] = [
  {
    name: "volbeat",
    src: "/game/sprites/volbeat.png",
    size: [32, 32],
    y: 96,
    speed: 2,
    offscreen: 16,
    run: loop([0, 1], 2),
    bob: true,
  },
  {
    name: "torchic",
    src: "/game/sprites/torchic.png",
    size: [32, 32],
    y: 88,
    speed: 2,
    offscreen: 16,
    run: loop([0, 1, 2, 1], 3),
  },
  {
    name: "cyclist",
    src: "/game/sprites/brendan.png",
    size: [64, 64],
    y: 80,
    speed: 6,
    offscreen: 32,
    run: loop([0, 1, 2, 3], 4),
    bicycle: true,
  },
  {
    name: "manectric",
    src: "/game/sprites/manectric.png",
    size: [64, 64],
    y: 88,
    speed: 12,
    offscreen: 32,
    run: loop([0, 1, 2, 3], 2),
  },
]

export const BICYCLE = {
  src: "/game/sprites/bicycle.png",
  size: [64, 32] as const,
  run: loop([0, 1], 4),
}

/** Torchic trips at mid-screen, lies there, then gets up and runs on. */
export const TORCHIC_TRIP: PasserFrame[] = [
  { frame: 3, duration: 4 },
  { frame: 4, duration: 6 },
  { frame: 5, duration: 24 },
  { frame: 5, duration: 4 },
  { frame: 4, duration: 6 },
  { frame: 3, duration: 4 },
  { frame: 2, duration: 3 },
]

/** Picks the animation frame `tick` frames into a looping sequence. */
export const loopFrame = (frames: PasserFrame[], tick: number): number => {
  const total = frames.reduce((sum, frame) => sum + frame.duration, 0)
  let at = tick % total
  for (const frame of frames) {
    if (at < frame.duration) return frame.frame
    at -= frame.duration
  }
  return frames[0]!.frame
}

/**
 * How large to draw a GBA pixel: as large as the viewport height allows while
 * keeping at least ~200 GBA pixels across, so the logo fits on phones.
 */
export const pixelScale = (width: number, height: number): number => {
  let scale = Math.max(2, Math.round(height / 170))
  while (scale > 2 && width / scale < 200) scale--
  if (width / scale < 160) scale = 1
  return scale
}

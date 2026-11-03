// Draws the world on a canvas for a camera: the far-out view when zoomed out,
// the game's own metatiles close in, cross-fading between, with region and
// town names while they help.
import type { Camera } from "./camera"
import { EMPTY, type World } from "./data"

/** The open sea past the edges, the color the far-out view deepens to. */
export const OPEN_SEA = "rgb(52 92 178)"
/** Metatiles a chunk is wide and tall. */
const CHUNK = 32
/** Device pixels per metatile where the tiles start fading in over the far-out view, and finish. */
const FADE_FROM = 5
const FADE_TO = 10
/** How long a frame may spend drawing new chunks; the rest wait for the next frames. */
const CHUNK_BUDGET_MS = 8
/** Lands labeled smaller and in white, beside the four main regions in gold. */
const MINOR_REGIONS = new Set(["SINJOH", "SEVII ISLANDS", "ALOLA"])

/** A sprite on the map: a cut of `image`, placed and sized in metatiles. */
export type Sprite = {
  image: CanvasImageSource
  source: { x: number; y: number; width: number; height: number }
  x: number
  y: number
  width: number
  height: number
  /** Mirrored left to right. */
  flip?: boolean
}

/**
 * Where the map is on the canvas, for drawing over it: device pixels per
 * metatile, and the grid point at the canvas's top-left corner.
 */
export type Projection = {
  scale: number
  left: number
  top: number
  /** Device pixels per CSS pixel. */
  ratio: number
  /** A grid point (in metatiles) to device pixels on the canvas. */
  toScreen: (x: number, y: number) => { x: number; y: number }
}

/** Something drawn over the map in device pixels, e.g. ships, pins or a tint. */
export type Overlay = (context: CanvasRenderingContext2D, projection: Projection) => void

export type View = {
  camera: Camera
  /** Where on the canvas the camera's point sits, in CSS pixels. */
  focus: { x: number; y: number }
  /** How wide the map's part of the canvas is, in CSS pixels, to size the region names to. */
  labelWidth: number
  /** Whether to name regions and towns at all. */
  labels: boolean
  /** Drawn over the map, under the names. */
  sprites?: Sprite[]
  /** Drawn in order over the map and sprites, under the names. */
  overlays?: Overlay[]
}

/** "NewBarkTown_hns" to "NEW BARK TOWN". */
const townName = (name: string) =>
  name
    .replace(/_.*$/, "")
    .replace(/([a-z])([A-Z])/g, "$1 $2")
    .toUpperCase()

const createRenderer = ({ meta, atlas, grid, overview }: World) => {
  const { width, height, columns } = meta

  // Chunks of metatiles, drawn on demand at 4 or 16 pixels a metatile and
  // kept until too many pile up.
  const chunks = new Map<string, HTMLCanvasElement>()
  const drawChunk = (key: string, cx: number, cy: number, size: number) => {
    const c = document.createElement("canvas")
    c.width = CHUNK * size
    c.height = CHUNK * size
    const cc = c.getContext("2d")!
    cc.imageSmoothingEnabled = size < 16
    cc.imageSmoothingQuality = "high"
    for (let y = 0; y < CHUNK; y++)
      for (let x = 0; x < CHUNK; x++) {
        const gx = cx * CHUNK + x
        const gy = cy * CHUNK + y
        if (gx >= width || gy >= height) continue
        const k = grid[gy * width + gx]!
        if (k === EMPTY) continue
        cc.drawImage(
          atlas,
          (k % columns) * 16,
          Math.floor(k / columns) * 16,
          16,
          16,
          x * size,
          y * size,
          size,
          size,
        )
      }
    chunks.set(key, c)
    const limit = size === 16 ? 96 : 400
    let over = [...chunks.keys()].filter((k) => k.startsWith(`${size}:`)).length - limit
    for (const k of chunks.keys()) {
      if (over <= 0) break
      if (k.startsWith(`${size}:`)) {
        chunks.delete(k)
        over--
      }
    }
    return c
  }

  /** Draws the view; true when some chunks are still to come and another frame should follow. */
  const draw = (context: CanvasRenderingContext2D, view: View): boolean => {
    const { canvas } = context
    const ratio = window.devicePixelRatio
    const w = Math.round(canvas.clientWidth * ratio)
    const h = Math.round(canvas.clientHeight * ratio)
    if (canvas.width !== w || canvas.height !== h) {
      canvas.width = w
      canvas.height = h
    }
    const { camera } = view
    const scale = camera.zoom * ratio
    const left = camera.x - (view.focus.x * ratio) / scale
    const top = camera.y - (view.focus.y * ratio) / scale
    context.setTransform(1, 0, 0, 1, 0, 0)
    context.fillStyle = OPEN_SEA
    context.fillRect(0, 0, w, h)

    // The tiles close in, over the far-out view while they fade in or while
    // some of their chunks are still being drawn.
    const tiles = Math.min(1, Math.max(0, (scale - FADE_FROM) / (FADE_TO - FADE_FROM)))
    const visible: {
      chunk: HTMLCanvasElement | undefined
      x: number
      y: number
      w: number
      h: number
    }[] = []
    let pending = false
    if (tiles > 0) {
      const size = scale < 10 ? 4 : 16
      const started = performance.now()
      const firstX = Math.max(0, Math.floor(left / CHUNK))
      const firstY = Math.max(0, Math.floor(top / CHUNK))
      const lastX = Math.min(Math.ceil(width / CHUNK) - 1, Math.floor((left + w / scale) / CHUNK))
      const lastY = Math.min(Math.ceil(height / CHUNK) - 1, Math.floor((top + h / scale) / CHUNK))
      for (let cy = firstY; cy <= lastY; cy++)
        for (let cx = firstX; cx <= lastX; cx++) {
          const key = `${size}:${cx}:${cy}`
          let chunk = chunks.get(key)
          if (chunk) {
            chunks.delete(key)
            chunks.set(key, chunk)
          } else if (performance.now() - started < CHUNK_BUDGET_MS) {
            chunk = drawChunk(key, cx, cy, size)
          } else pending = true
          const x = Math.round((cx * CHUNK - left) * scale)
          const y = Math.round((cy * CHUNK - top) * scale)
          visible.push({
            chunk,
            x,
            y,
            w: Math.round(((cx + 1) * CHUNK - left) * scale) - x,
            h: Math.round(((cy + 1) * CHUNK - top) * scale) - y,
          })
        }
    }
    if (tiles < 1 || pending) {
      context.imageSmoothingEnabled = scale < 2
      context.imageSmoothingQuality = "high"
      context.drawImage(overview, -left * scale, -top * scale, width * scale, height * scale)
    }
    if (tiles > 0) {
      context.globalAlpha = tiles
      context.imageSmoothingEnabled = scale < (scale < 10 ? 4 : 16)
      for (const { chunk, x, y, w: cw, h: ch } of visible)
        if (chunk) context.drawImage(chunk, x, y, cw, ch)
      context.globalAlpha = 1
    }
    context.imageSmoothingEnabled = false
    for (const sprite of view.sprites ?? []) {
      const x = Math.round((sprite.x - left) * scale)
      const y = Math.round((sprite.y - top) * scale)
      const spriteWidth = Math.round(sprite.width * scale)
      const spriteHeight = Math.round(sprite.height * scale)
      const { source } = sprite
      context.save()
      context.translate(sprite.flip ? x + spriteWidth : x, y)
      if (sprite.flip) context.scale(-1, 1)
      context.drawImage(
        sprite.image,
        source.x,
        source.y,
        source.width,
        source.height,
        0,
        0,
        spriteWidth,
        spriteHeight,
      )
      context.restore()
    }

    if (view.overlays?.length) {
      const projection: Projection = {
        scale,
        left,
        top,
        ratio,
        toScreen: (x, y) => ({ x: (x - left) * scale, y: (y - top) * scale }),
      }
      for (const overlay of view.overlays) {
        context.save()
        context.setTransform(1, 0, 0, 1, 0, 0)
        overlay(context, projection)
        context.restore()
      }
    }

    if (!view.labels) return pending

    // Region names far out, fading as the town names take over.
    const regionAlpha = Math.min(1, Math.max(0, (1.1 - camera.zoom) / 0.4))
    if (regionAlpha > 0) {
      context.globalAlpha = regionAlpha
      // Each name on a plate like the game's map name signs: dark, rimmed in
      // gold for the four main regions and white for the lesser lands, so it
      // reads over land, sea, ships and the night alike. Sizes are whole
      // steps of the game font; Sinjoh's sits beside its narrow neck.
      const wide = view.labelWidth >= 700
      context.textBaseline = "middle"
      for (const region of meta.regions) {
        const minor = MINOR_REGIONS.has(region.name)
        const size = Math.round((minor ? (wide ? 20 : 16) : wide ? 28 : 20) * ratio)
        context.font = `${size}px "Wayfarer Text", monospace`
        const aside = region.name === "SINJOH"
        const text = context.measureText(region.name).width
        const padX = Math.round(size * 0.45)
        const padY = Math.round(size * 0.3)
        const boxW = Math.round(text + padX * 2)
        const boxH = Math.round(size + padY * 2)
        const cx = (region.x - left) * scale + (aside ? 70 * scale + boxW / 2 : 0)
        const cy = (region.y - top) * scale
        const bx = Math.round(cx - boxW / 2)
        const by = Math.round(cy - boxH / 2)
        const rim = Math.max(2, Math.round(2 * ratio))
        context.fillStyle = "rgb(0 0 0 / 0.35)"
        context.fillRect(bx + rim, by + rim, boxW, boxH)
        context.fillStyle = minor ? "rgb(248 248 248)" : "rgb(255 214 0)"
        context.fillRect(bx, by, boxW, boxH)
        context.fillStyle = "rgb(8 41 0 / 0.92)"
        context.fillRect(bx + rim, by + rim, boxW - rim * 2, boxH - rim * 2)
        context.textAlign = "center"
        context.fillStyle = "rgb(0 0 0 / 0.8)"
        context.fillText(region.name, cx + ratio, cy + ratio * 1.5)
        context.fillStyle = minor ? "rgb(248 248 248)" : "rgb(255 214 0)"
        context.fillText(region.name, cx, cy + ratio * 0.5)
      }
      context.globalAlpha = 1
      context.textBaseline = "alphabetic"
    }

    // Town names while they help: not when everything is a speck, nor when
    // the town fills the screen.
    if (camera.zoom > 0.9 && camera.zoom < 14) {
      context.font = `${Math.round(16 * ratio)}px "Wayfarer Text", monospace`
      context.textAlign = "center"
      for (const town of meta.towns) {
        const x = (town.x - left) * scale
        const y = (town.y - top) * scale
        if (x < -200 || y < -50 || x > w + 200 || y > h + 50) continue
        context.fillStyle = "rgb(0 0 0 / 0.7)"
        context.fillText(townName(town.name), x + ratio * 1.5, y + ratio * 1.5)
        context.fillStyle = "rgb(248 248 248)"
        context.fillText(townName(town.name), x, y)
      }
    }
    return pending
  }

  return { draw }
}

export type Renderer = ReturnType<typeof createRenderer>

const renderers = new WeakMap<World, Renderer>()

/** One renderer per world, so the tour and the explorer share their drawn chunks. */
export const rendererFor = (world: World): Renderer => {
  let renderer = renderers.get(world)
  if (!renderer) renderers.set(world, (renderer = createRenderer(world)))
  return renderer
}

// Clouds drifting over the world, seen from above as from a satellite:
// faint, irregular masses in hard pixels, each casting its shadow on the
// world to the south-east. The sky is one noise field that repeats only every
// few tens of thousands of metatiles, far wider than any screen, baked in
// chunks as they come into view (a few each frame, shared by every map on
// the page) and composed into one image per frame, so there are no seams.
// The clouds thin out as the camera comes in, so the game's own tiles are
// never under them, but their shadows keep passing over the world at every
// zoom.
import { ramp, smooth, tiledNoise } from "./noise"
import type { Overlay } from "./render"

/** Metatiles each of a cloud's pixels (a texel) covers. */
const TEXEL = 4
/** Texels to each sample of the noise, which is smooth enough to blend between. */
const SAMPLE = 2
/** Samples across the noise's base lattice: about the size of a cloud. */
const PUFF = 30
/** Texels across a chunk of the sky, and chunks before the sky repeats. */
const CHUNK = 128
const CHUNKS = 60
/** Texels of noise kept around a chunk, for the slope its light comes from. */
const MARGIN = SAMPLE
/** Milliseconds of baking a frame may spend, a chunk's fade in, and chunks kept. */
const BUDGET = 4
const FADE = 400
const KEPT = 200
/** CSS pixels per metatile from which the clouds thin, and past which they are gone. */
const THIN_FROM = 1.6
const GONE_AT = 4.5
/** CSS pixels per metatile by which the shadows have gone from blocks to soft. */
const SOFT_AT = 10
/** Metatiles the clouds drift per second, east and a little south. */
const DRIFT = { x: 2.4, y: 0.9 }
/** Where on the noise's 0 to 1 a cloud's wisps begin, and where it is solid. */
const WISP = 0.72
const SOLID = 0.9
/** How opaque the thick of a cloud is, and how much its shadow darkens. */
const BODY = 0.55
const SHADE = 0.3
/** How much lighter the shadows are close in, where the clouds are gone. */
const SHADOW_FADE = 0.35
/** Texels south-east of its cloud that a shadow falls. */
const SHADOW = 8
const SEED = 0x5c10
/** A cloud's tones, and its shadow's. */
const LIT = [255, 255, 255]
const BASE = [232, 238, 250]
const SHADED = [192, 204, 232]
const DARK = [6, 14, 40]

/** How cloudy the sky is, 0 to 1, at a sample of the noise. */
let field: ((x: number, y: number) => number) | undefined
const cover = () => {
  if (field) return field
  const lattice = (CHUNK * CHUNKS) / SAMPLE / PUFF
  // The clouds' own noise, pushed around by a slower one so their edges curl
  // instead of following the lattice, and a slower one still that gathers
  // them into banks with clear sky between.
  const body = tiledNoise(lattice, 5, SEED)
  const warpX = tiledNoise(lattice / 2, 2, SEED + 1)
  const warpY = tiledNoise(lattice / 2, 2, SEED + 2)
  const banks = tiledNoise(lattice / 4, 2, SEED + 3)
  field = (x, y) => {
    const u = x / PUFF
    const v = y / PUFF
    const wu = u + (warpX(u / 2, v / 2) - 0.5) * 1.3
    const wv = v + (warpY(u / 2, v / 2) - 0.5) * 1.3
    const n = body(wu, wv) + (banks(u / 4, v / 4) - 0.5) * 0.55
    return smooth((n - WISP) / (SOLID - WISP))
  }
  return field
}

type Chunk = {
  /** The cover at each texel, 0 to 255, with a margin around the chunk. */
  density: Uint8Array
  /** When it was baked, to fade it in. */
  born: number
  cloud?: HTMLCanvasElement
  soft?: HTMLCanvasElement
}

const chunks = new Map<number, Chunk>()
const SIDE = CHUNK + 2 * MARGIN

const bakeChunk = (ci: number, cj: number, born: number): Chunk => {
  const at = cover()
  // The noise at every other texel, blended up to each texel between.
  const x0 = ci * CHUNK - MARGIN
  const y0 = cj * CHUNK - MARGIN
  const sx = Math.floor((x0 + 0.5) / SAMPLE - 0.5)
  const sy = Math.floor((y0 + 0.5) / SAMPLE - 0.5)
  const n = Math.floor((x0 + SIDE - 0.5) / SAMPLE - 0.5) - sx + 2
  const coarse = new Float32Array(n * n)
  for (let y = 0; y < n; y++) for (let x = 0; x < n; x++) coarse[y * n + x] = at(sx + x, sy + y)
  const sample = (x: number, y: number) => coarse[y * n + x]!
  const density = new Uint8Array(SIDE * SIDE)
  for (let y = 0; y < SIDE; y++)
    for (let x = 0; x < SIDE; x++) {
      const fx = (x0 + x + 0.5) / SAMPLE - 0.5 - sx
      const fy = (y0 + y + 0.5) / SAMPLE - 0.5 - sy
      const ix = Math.floor(fx)
      const iy = Math.floor(fy)
      const tx = fx - ix
      const ty = fy - iy
      const top = sample(ix, iy) + (sample(ix + 1, iy) - sample(ix, iy)) * tx
      const bottom = sample(ix, iy + 1) + (sample(ix + 1, iy + 1) - sample(ix, iy + 1)) * tx
      density[y * SIDE + x] = Math.round(255 * (top + (bottom - top) * ty))
    }
  return { density, born }
}

const paint = (fill: (data: Uint8ClampedArray) => void) => {
  const canvas = document.createElement("canvas")
  canvas.width = canvas.height = CHUNK
  const context = canvas.getContext("2d")!
  const image = context.createImageData(CHUNK, CHUNK)
  fill(image.data)
  context.putImageData(image, 0, 0)
  return canvas
}

const densityAt = (chunk: Chunk, x: number, y: number) =>
  chunk.density[(y + MARGIN) * SIDE + x + MARGIN]! / 255

// Three whole steps of cover, a wisp, the body and the thick of it, so the
// clouds keep the map's hard pixel edges, lit from the north-west in a few
// flat tones: white where a cloud rises toward the light, a cool grey where
// it falls away from it. Its shadow is the same steps in the dark.
const cloudOf = (chunk: Chunk) =>
  (chunk.cloud ??= paint((data) => {
    for (let y = 0; y < CHUNK; y++)
      for (let x = 0; x < CHUNK; x++) {
        const d = densityAt(chunk, x, y)
        const step = d < 0.12 ? 0 : d < 0.45 ? 1 : d < 0.78 ? 2 : 3
        if (step === 0) continue
        const i = (y * CHUNK + x) * 4
        const slope =
          densityAt(chunk, x - SAMPLE, y - SAMPLE) - densityAt(chunk, x + SAMPLE, y + SAMPLE)
        data.set(slope > 0.06 ? LIT : slope < -0.06 ? SHADED : BASE, i)
        data[i + 3] = (255 * BODY * step) / 3
      }
  }))

// Close in, where a texel is many of the game's pixels, the shadow passes as
// a soft dimming instead of in blocks.
const softOf = (chunk: Chunk) =>
  (chunk.soft ??= paint((data) => {
    for (let y = 0; y < CHUNK; y++)
      for (let x = 0; x < CHUNK; x++) {
        const i = (y * CHUNK + x) * 4
        data.set(DARK, i)
        data[i + 3] = 255 * SHADE * densityAt(chunk, x, y)
      }
  }))

/** Where the chunks on screen are composed, a texel to a pixel. */
let sky: HTMLCanvasElement | undefined

/** The clouds and their shadows, on a clock (`now`, in ms); still without `animate`. */
export const cloudsOverlay = (animate: boolean, now: () => number): Overlay => {
  const frozen = now()
  return (context, projection) => {
    const { scale, ratio, left, top } = projection
    const zoom = scale / ratio
    const body = 1 - ramp(zoom, THIN_FROM, GONE_AT)
    const shade = 1 - SHADOW_FADE * ramp(zoom, THIN_FROM, GONE_AT)
    const blend = ramp(zoom, GONE_AT, SOFT_AT)
    const t = (animate ? now() : frozen) / 1000
    const { width, height } = context.canvas
    const span = CHUNK * CHUNKS * TEXEL
    const driftX = (t * DRIFT.x) % span
    const driftY = (t * DRIFT.y) % span
    // The sky's texels on screen, from a shadow's length north-west of it
    // (whose shadows fall on it), with one more on every side so a blended
    // texel at the screen's edge blends with its neighbour.
    const x0 = Math.floor((left - driftX) / TEXEL) - SHADOW - 1
    const y0 = Math.floor((top - driftY) / TEXEL) - SHADOW - 1
    const x1 = Math.ceil((left + width / scale - driftX) / TEXEL) + 1
    const y1 = Math.ceil((top + height / scale - driftY) / TEXEL) + 1
    const w = x1 - x0
    const h = y1 - y0

    // The chunks those texels are in: kept, or baked now while the frame's
    // budget lasts (at least one a frame; all of them when the clock is
    // still, since there may be no next frame).
    const started = performance.now()
    let baked = false
    const shown: { chunk: Chunk; x: number; y: number; alpha: number }[] = []
    for (let cj = Math.floor(y0 / CHUNK); cj <= Math.floor((y1 - 1) / CHUNK); cj++)
      for (let ci = Math.floor(x0 / CHUNK); ci <= Math.floor((x1 - 1) / CHUNK); ci++) {
        const wi = ((ci % CHUNKS) + CHUNKS) % CHUNKS
        const wj = ((cj % CHUNKS) + CHUNKS) % CHUNKS
        const key = wj * CHUNKS + wi
        let chunk = chunks.get(key)
        if (chunk) chunks.delete(key)
        else if (!animate || !baked || performance.now() - started < BUDGET) {
          chunk = bakeChunk(wi, wj, animate ? performance.now() : -Infinity)
          baked = true
        } else continue
        chunks.set(key, chunk)
        const alpha = Math.min(1, (started - chunk.born) / FADE)
        if (alpha > 0) shown.push({ chunk, x: ci * CHUNK - x0, y: cj * CHUNK - y0, alpha })
      }
    while (chunks.size > KEPT) chunks.delete(chunks.keys().next().value!)
    if (shown.length === 0) return

    sky ??= document.createElement("canvas")
    if (sky.width < w || sky.height < h) {
      sky.width = Math.max(sky.width, w)
      sky.height = Math.max(sky.height, h)
    }
    const composer = sky.getContext("2d")!
    composer.imageSmoothingEnabled = false
    const compose = (layer: (chunk: Chunk) => HTMLCanvasElement, tint?: number[]) => {
      composer.clearRect(0, 0, w, h)
      for (const { chunk, x, y, alpha } of shown) {
        composer.globalAlpha = alpha
        composer.drawImage(layer(chunk), x, y)
      }
      composer.globalAlpha = 1
      if (tint) {
        composer.globalCompositeOperation = "source-in"
        composer.fillStyle = `rgb(${tint.join(",")})`
        composer.fillRect(0, 0, w, h)
        composer.globalCompositeOperation = "source-over"
      }
    }
    // The composed texels, onto the map: whole blocks once a texel is a
    // couple of pixels, blended below that so the far-out clouds don't
    // shimmer as they drift, and always for the soft shadow.
    const place = (shift: number, alpha: number, smoothed: boolean) => {
      context.imageSmoothingEnabled = smoothed
      context.globalAlpha = alpha
      const x = ((x0 + shift) * TEXEL + driftX - left) * scale
      const y = ((y0 + shift) * TEXEL + driftY - top) * scale
      context.drawImage(sky!, 0, 0, w, h, x, y, w * TEXEL * scale, h * TEXEL * scale)
    }
    const blocky = TEXEL * scale >= 2
    if (blend < 1) {
      compose(cloudOf, DARK)
      place(SHADOW, (shade * (1 - blend) * SHADE) / BODY, !blocky)
    }
    if (blend > 0) {
      compose(softOf)
      place(SHADOW, shade * blend, true)
    }
    if (body > 0) {
      compose(cloudOf)
      place(0, body, !blocky)
    }
    context.globalAlpha = 1
  }
}

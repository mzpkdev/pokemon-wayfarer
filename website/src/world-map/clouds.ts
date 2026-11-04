// Clouds drifting over the world when it's seen from far, as from a
// satellite: soft, irregular masses of white, thick in the middle and wispy
// at the edges, each casting its shadow on the world to the south-east. One
// noise field, baked once, tiles the sky only every few thousand metatiles,
// wider than the world, so it never shows a repeat. The clouds thin out as
// the camera comes in, so the game's own tiles are never under them, but
// their shadows keep passing over the world at every zoom.
import { ramp, smooth, tiledNoise } from "./noise"
import type { Overlay } from "./render"

/** Texels in the tiling texture, and metatiles each texel covers. */
const TEXELS = 960
const TEXEL = 4
/** Texels to each sample of the noise, which is smooth enough to blend between. */
const SAMPLE = 2
/** Samples across the noise's base lattice: about the size of a cloud. */
const PUFF = 30
/** CSS pixels per metatile from which the clouds thin, and past which they are gone. */
const THIN_FROM = 1.6
const GONE_AT = 4.5
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
/** CSS pixels per metatile by which the shadows have gone from blocks to soft. */
const SOFT_AT = 10
/** Metatiles south-east of its cloud that a shadow falls: whole texels. */
const SHADOW_OFFSET = 32
const SEED = 0x5c10
/** A cloud's tones, and its shadow's. */
const LIT = [255, 255, 255]
const BASE = [232, 238, 250]
const SHADED = [192, 204, 232]
const DARK = [6, 14, 40]

const bake = () => {
  const samples = TEXELS / SAMPLE
  const lattice = samples / PUFF
  // The clouds' own noise, pushed around by a slower one so their edges curl
  // instead of following the lattice, and a slower one still that gathers
  // them into banks with clear sky between.
  const body = tiledNoise(lattice, 5, SEED)
  const warpX = tiledNoise(lattice / 2, 2, SEED + 1)
  const warpY = tiledNoise(lattice / 2, 2, SEED + 2)
  const banks = tiledNoise(lattice / 4, 2, SEED + 3)
  const coarse = new Float32Array(samples * samples)
  for (let y = 0; y < samples; y++)
    for (let x = 0; x < samples; x++) {
      const u = x / PUFF
      const v = y / PUFF
      const wu = u + (warpX(u / 2, v / 2) - 0.5) * 1.3
      const wv = v + (warpY(u / 2, v / 2) - 0.5) * 1.3
      const n = body(wu, wv) + (banks(u / 4, v / 4) - 0.5) * 0.55
      coarse[y * samples + x] = smooth((n - WISP) / (SOLID - WISP))
    }
  const sample = (x: number, y: number) =>
    coarse[((y + samples) % samples) * samples + ((x + samples) % samples)]!
  // Blended up to whole texels, so the steps below cut edges a texel fine.
  const density = new Float32Array(TEXELS * TEXELS)
  for (let y = 0; y < TEXELS; y++)
    for (let x = 0; x < TEXELS; x++) {
      const fx = (x + 0.5) / SAMPLE - 0.5
      const fy = (y + 0.5) / SAMPLE - 0.5
      const x0 = Math.floor(fx)
      const y0 = Math.floor(fy)
      const tx = fx - x0
      const ty = fy - y0
      const top = sample(x0, y0) + (sample(x0 + 1, y0) - sample(x0, y0)) * tx
      const bottom = sample(x0, y0 + 1) + (sample(x0 + 1, y0 + 1) - sample(x0, y0 + 1)) * tx
      density[y * TEXELS + x] = top + (bottom - top) * ty
    }
  const at = (x: number, y: number) =>
    density[((y + TEXELS) % TEXELS) * TEXELS + ((x + TEXELS) % TEXELS)]!
  const cloud = document.createElement("canvas")
  const shadow = document.createElement("canvas")
  const soft = document.createElement("canvas")
  cloud.width = shadow.width = soft.width = TEXELS
  cloud.height = shadow.height = soft.height = TEXELS
  const c = cloud.getContext("2d")!
  const s = shadow.getContext("2d")!
  const f = soft.getContext("2d")!
  const cd = c.createImageData(TEXELS, TEXELS)
  const sd = s.createImageData(TEXELS, TEXELS)
  const fd = f.createImageData(TEXELS, TEXELS)
  for (let y = 0; y < TEXELS; y++)
    for (let x = 0; x < TEXELS; x++) {
      // Three whole steps of cover, a wisp, the body and the thick of it, so
      // the clouds keep the map's hard pixel edges.
      const d = at(x, y)
      const i = (y * TEXELS + x) * 4
      // Close in, where a texel is many of the game's pixels, the shadow
      // passes as a soft dimming instead of in blocks.
      fd.data.set(DARK, i)
      fd.data[i + 3] = 255 * SHADE * d
      const step = d < 0.12 ? 0 : d < 0.45 ? 1 : d < 0.78 ? 2 : 3
      if (step === 0) continue
      // Lit from the north-west, in a few flat tones: white where the cloud
      // rises toward the light, a cool grey where it falls away from it.
      const slope = at(x - SAMPLE, y - SAMPLE) - at(x + SAMPLE, y + SAMPLE)
      cd.data.set(slope > 0.06 ? LIT : slope < -0.06 ? SHADED : BASE, i)
      cd.data[i + 3] = (255 * BODY * step) / 3
      sd.data.set(DARK, i)
      sd.data[i + 3] = (255 * SHADE * step) / 3
    }
  c.putImageData(cd, 0, 0)
  s.putImageData(sd, 0, 0)
  f.putImageData(fd, 0, 0)
  return { cloud, shadow, soft }
}

/** The clouds and their shadows, on a clock (`now`, in ms); still without `animate`. */
export const cloudsOverlay = (animate: boolean, now: () => number): Overlay => {
  let baked: ReturnType<typeof bake> | undefined
  const frozen = now()
  return (context, projection) => {
    const { scale, ratio, left, top } = projection
    const zoom = scale / ratio
    // The clouds thin out as the camera comes in; their shadows stay, a
    // little lighter, so they still pass over the towns close in.
    const body = 1 - ramp(zoom, THIN_FROM, GONE_AT)
    const shade = 1 - SHADOW_FADE * ramp(zoom, THIN_FROM, GONE_AT)
    const blend = ramp(zoom, GONE_AT, SOFT_AT)
    baked ??= bake()
    const t = (animate ? now() : frozen) / 1000
    const { width, height } = context.canvas
    // The texture tiles the sky every TEXELS * TEXEL metatiles, drifting.
    const span = TEXELS * TEXEL
    const offsetX = ((t * DRIFT.x) % span) + span
    const offsetY = ((t * DRIFT.y) % span) + span
    // Where a tile's texel edge falls on the canvas, `at` texels into tile
    // `i`. Every texel runs from its own rounded edge to the next one's, so
    // nothing gaps or doubles up when a texel's size isn't whole.
    const edgeX = (i: number, at: number, dx: number) =>
      Math.round((i * span + at * TEXEL - offsetX - left) * scale + dx)
    const edgeY = (j: number, at: number, dy: number) =>
      Math.round((j * span + at * TEXEL - offsetY - top) * scale + dy)
    const firstX = Math.floor((left + offsetX) / span) - 1
    const firstY = Math.floor((top + offsetY) / span) - 1
    const lastX = Math.floor((left + width / scale + offsetX) / span)
    const lastY = Math.floor((top + height / scale + offsetY) / span)
    const texel = TEXEL * scale
    // Whole blocks once a texel is a couple of pixels; blended below that,
    // so the far-out clouds don't shimmer as they drift.
    const blocky = texel >= 2
    const cast = SHADOW_OFFSET * scale
    for (const [image, shift, alpha, smoothed] of [
      [baked.shadow, cast, shade * (1 - blend), !blocky],
      [baked.soft, cast, shade * blend, true],
      [baked.cloud, 0, body, !blocky],
    ] as const) {
      if (alpha <= 0) continue
      context.imageSmoothingEnabled = smoothed
      context.globalAlpha = alpha
      for (let j = firstY; j <= lastY; j++)
        for (let i = firstX; i <= lastX; i++) {
          // Only the texels on the canvas, so close in it doesn't scale the
          // whole texture up to draw a corner of it.
          const x = edgeX(i, 0, shift)
          const y = edgeY(j, 0, shift)
          const sx = Math.max(0, Math.floor(-x / texel))
          const sy = Math.max(0, Math.floor(-y / texel))
          const ex = Math.min(TEXELS, Math.ceil((width - x) / texel) + 1)
          const ey = Math.min(TEXELS, Math.ceil((height - y) / texel) + 1)
          if (sx >= ex || sy >= ey) continue
          const dx = edgeX(i, sx, shift)
          const dy = edgeY(j, sy, shift)
          const dw = edgeX(i, ex, shift) - dx
          const dh = edgeY(j, ey, shift) - dy
          context.drawImage(image, sx, sy, ex - sx, ey - sy, dx, dy, dw, dh)
        }
    }
    context.globalAlpha = 1
  }
}

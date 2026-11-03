// Cloud shadows drifting over the world when it's seen from far: a tiling
// noise texture, thresholded to a few whole steps so it stays as blocky as
// the far-out view, scrolled slowly across the map. They thin out as the
// camera comes in, so the game's own tiles are never under them.
import { ramp, tiledNoise } from "./noise"
import type { Overlay } from "./render"

/** Texels in the tiling texture, and metatiles each texel covers. */
const TEXELS = 128
const TEXEL = 4
/** CSS pixels per metatile from which the clouds thin, and past which they are gone. */
const THIN_FROM = 1.6
const GONE_AT = 4.5
/** Metatiles the clouds drift per second, east and a little south. */
const DRIFT = { x: 2.4, y: 0.9 }
/** How much a cloud darkens what's under its shadow, at most. */
const SHADE = 0.2
/** The cloud itself, drawn faintly above its shadow, offset to the north-west. */
const BODY = 0.12
const BODY_OFFSET = 3
/** Texels across a cloud's bones: the noise's base lattice spacing. */
const PUFF = 32
/** Where a cloud's wisps begin, where its body does and where it is thick, on the noise's 0 to 1. */
const WISP = 0.63
const EDGE = 0.67
const THICK = 0.72

const bake = () => {
  const noise = tiledNoise(TEXELS / PUFF, 3, 0x5c10)
  const shadow = document.createElement("canvas")
  const body = document.createElement("canvas")
  shadow.width = body.width = TEXELS
  shadow.height = body.height = TEXELS
  const s = shadow.getContext("2d")!
  const b = body.getContext("2d")!
  const sd = s.createImageData(TEXELS, TEXELS)
  const bd = b.createImageData(TEXELS, TEXELS)
  for (let y = 0; y < TEXELS; y++)
    for (let x = 0; x < TEXELS; x++) {
      const n = noise(x / PUFF, y / PUFF)
      // Three steps of cover: a faint wisp, the cloud's body and the thick of it.
      const cover = n < WISP ? 0 : n < EDGE ? 1 : n < THICK ? 2 : 3
      const i = (y * TEXELS + x) * 4
      sd.data[i] = 8
      sd.data[i + 1] = 16
      sd.data[i + 2] = 48
      sd.data[i + 3] = Math.round((255 * SHADE * cover) / 3)
      bd.data[i] = 255
      bd.data[i + 1] = 255
      bd.data[i + 2] = 255
      bd.data[i + 3] = Math.round((255 * BODY * cover) / 3)
    }
  s.putImageData(sd, 0, 0)
  b.putImageData(bd, 0, 0)
  return { shadow, body }
}

/** The cloud shadows, on a clock (`now`, in ms); still without `animate`. */
export const cloudsOverlay = (animate: boolean, now: () => number): Overlay => {
  let baked: ReturnType<typeof bake> | undefined
  const frozen = now()
  return (context, projection) => {
    const { scale, ratio, left, top } = projection
    const zoom = scale / ratio
    const alpha = 1 - ramp(zoom, THIN_FROM, GONE_AT)
    if (alpha <= 0) return
    baked ??= bake()
    const t = (animate ? now() : frozen) / 1000
    const { width, height } = context.canvas
    // The texture tiles the world every TEXELS * TEXEL metatiles, drifting.
    const span = TEXELS * TEXEL
    const offsetX = ((t * DRIFT.x) % span) + span
    const offsetY = ((t * DRIFT.y) % span) + span
    const size = span * scale
    const firstX = Math.floor((left + offsetX) / span) - 1
    const firstY = Math.floor((top + offsetY) / span) - 1
    // One tile more at the far side, for the body layer drawn shifted up and left.
    const lastX = Math.floor((left + width / scale + offsetX) / span) + 1
    const lastY = Math.floor((top + height / scale + offsetY) / span) + 1
    context.imageSmoothingEnabled = false
    context.globalAlpha = alpha
    const lift = BODY_OFFSET * TEXEL * scale
    for (const [image, dx, dy] of [
      [baked.shadow, 0, 0],
      [baked.body, -lift, -lift],
    ] as const)
      for (let j = firstY; j <= lastY; j++)
        for (let i = firstX; i <= lastX; i++) {
          const x = Math.round((i * span - offsetX - left) * scale + dx)
          const y = Math.round((j * span - offsetY - top) * scale + dy)
          if (x > width || y > height || x + size < 0 || y + size < 0) continue
          context.drawImage(image, x, y, Math.round(size), Math.round(size))
        }
    context.globalAlpha = 1
  }
}

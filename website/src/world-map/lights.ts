// The world after dark: as the time of day's tint deepens, the towns glow
// warm, each ship shows a light, and the Olivine lighthouse sweeps its beam
// over the sea. Drawn after the tint, so the lights stay bright.
import type { Meta } from "./data"
import { type LivingWorld, shipsAt, tintAt } from "./living"
import { ramp } from "./noise"
import type { Overlay } from "./render"

/** CSS pixels per metatile past which the town glows thin out and are gone (the tiles have no lit windows to match). */
const GLOW_THIN_FROM = 9
const GLOW_GONE_AT = 16
/** A town's glow, in CSS pixels: never smaller than this, and never bigger. */
const GLOW_MIN = 14
const GLOW_MAX = 120
/** The lighthouse: the map it stands on, where its lamp is in that map's tiles, and how long a sweep takes. */
const LIGHTHOUSE = { map: "OlivineCity_hns", x: 39.5, y: 40.6 }
const SWEEP_MS = 7000
const BEAM_HALF = 0.085

/** How dark the tint makes the map, 0 by day to about a half at night. */
export const darknessAt = (living: LivingWorld, minutes: number) => {
  const [r, g, b] = tintAt(living, minutes)
  return Math.max(0, 1 - (r + g + b) / (3 * 255))
}

/** The night lights, on the page's game clock and a real clock (`now`, in ms) for the beam. */
export const lightsOverlay = (
  meta: Meta,
  living: LivingWorld,
  minutes: () => number,
  animate: boolean,
  now: () => number,
): Overlay => {
  const stand = meta.maps.find((map) => map.name === LIGHTHOUSE.map)
  const lamp = stand && { x: stand.x + LIGHTHOUSE.x, y: stand.y + LIGHTHOUSE.y }
  const frozen = now()
  return (context, projection) => {
    const game = minutes()
    const dark = darknessAt(living, game)
    if (dark <= 0.02) return
    const { scale, ratio, toScreen } = projection
    const zoom = scale / ratio
    const { width, height } = context.canvas
    // The night's share of the way from evening: lights come up as it darkens.
    const night = Math.min(1, dark / 0.45)
    context.globalCompositeOperation = "screen"

    const glow = 1 - ramp(zoom, GLOW_THIN_FROM, GLOW_GONE_AT)
    if (glow > 0) {
      const radius = Math.min(GLOW_MAX, Math.max(GLOW_MIN, zoom * 11)) * ratio
      for (const town of meta.towns) {
        const p = toScreen(town.x, town.y)
        if (p.x < -radius || p.y < -radius || p.x > width + radius || p.y > height + radius)
          continue
        const halo = context.createRadialGradient(p.x, p.y, 0, p.x, p.y, radius)
        halo.addColorStop(0, `rgb(255 196 96 / ${0.5 * night * glow})`)
        halo.addColorStop(0.35, `rgb(255 160 64 / ${0.22 * night * glow})`)
        halo.addColorStop(1, "rgb(255 140 40 / 0)")
        context.fillStyle = halo
        context.fillRect(p.x - radius, p.y - radius, radius * 2, radius * 2)
        // A core of lit windows, a few pixels across.
        const core = Math.max(2, Math.round(Math.min(6, zoom * 2) * ratio))
        context.fillStyle = `rgb(255 232 160 / ${0.7 * night * glow})`
        context.fillRect(Math.round(p.x - core / 2), Math.round(p.y - core / 2), core, core)
      }
    }

    // A lamp on each ship, and the lighthouse's beam over the sea.
    const lamps = Math.max(2, Math.round(3 * ratio))
    for (const ship of shipsAt(living, game)) {
      const p = toScreen(ship.x, ship.y)
      if (p.x < -40 || p.y < -40 || p.x > width + 40 || p.y > height + 40) continue
      const halo = context.createRadialGradient(p.x, p.y, 0, p.x, p.y, lamps * 5)
      halo.addColorStop(0, `rgb(255 220 140 / ${0.6 * night})`)
      halo.addColorStop(1, "rgb(255 200 100 / 0)")
      context.fillStyle = halo
      context.fillRect(p.x - lamps * 5, p.y - lamps * 5, lamps * 10, lamps * 10)
    }
    if (lamp) {
      const p = toScreen(lamp.x, lamp.y)
      const reach = Math.min(520, Math.max(70, zoom * 80)) * ratio
      if (p.x > -reach && p.y > -reach && p.x < width + reach && p.y < height + reach) {
        const t = animate ? now() : frozen
        const angle = ((t % SWEEP_MS) / SWEEP_MS) * Math.PI * 2
        const beam = context.createRadialGradient(p.x, p.y, 0, p.x, p.y, reach)
        beam.addColorStop(0, `rgb(255 244 200 / ${0.55 * night})`)
        beam.addColorStop(0.5, `rgb(255 236 170 / ${0.22 * night})`)
        beam.addColorStop(1, "rgb(255 230 150 / 0)")
        context.fillStyle = beam
        context.beginPath()
        context.moveTo(p.x, p.y)
        context.arc(p.x, p.y, reach, angle - BEAM_HALF, angle + BEAM_HALF)
        context.closePath()
        context.fill()
        const lampSize = Math.max(2, Math.round(2 * ratio))
        context.fillStyle = `rgb(255 250 220 / ${0.9 * night})`
        context.fillRect(
          Math.round(p.x - lampSize / 2),
          Math.round(p.y - lampSize / 2),
          lampSize,
          lampSize,
        )
      }
    }
    context.globalCompositeOperation = "source-over"
  }
}

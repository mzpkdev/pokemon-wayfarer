// Wild Pokémon in the explorer: close in, a few of a route's own Pokémon pop
// out of its tall grass and water now and then, the way the overworld shows
// a rustle, then duck back in. Who appears follows the route's encounter
// table for the time of day (morning and day use the day table, evening and
// night the night one), weighted by how often each comes up. Nothing is
// stored: each of a map's encounter cells has a chance of a sighting for
// each stretch of time, a hash of the map, the cell and the stretch, so every
// frame agrees.
import { ICON, type Encounters, type Slot, type Table } from "./encounters"
import type { Overlay } from "./render"

/** CSS pixels per metatile where sightings start to fade in, and are fully in. */
export const SIGHTING_ZOOM = 7
const FULL_ZOOM = 10
/** How long a cell's stretch is, and how much of it the sighting is out for, in ms. */
const STRETCH_MS = 6500
const OUT_MS = 3800
/** How long a sighting takes to pop out, and to duck back in. */
const POP_MS = 260
const DUCK_MS = 320
/** The idle hop: a pixel up and back, like a party icon's two frames, this often. */
const HOP_MS = 260
/** Of a cell's stretches, how many have nobody out: "not all, but some". */
const QUIET = 0.8

const hash = (n: number) => {
  let h = Math.imul(n ^ 0x9e3779b9, 0x85ebca6b)
  h ^= h >>> 13
  h = Math.imul(h, 0xc2b2ae35)
  return (h ^ (h >>> 16)) >>> 0
}
/** A hash of several numbers, as a fraction in [0, 1). */
const roll = (...parts: number[]) =>
  parts.reduce((h, part) => hash(h ^ Math.imul(part + 1, 0x27d4eb2d)), 0x2545f491) / 0x100000000

/** Evening and night use a table's night slots, morning and day its day slots (the in-game clock spec). */
const isNight = (minutes: number) => {
  const hour = (((minutes / 60) % 24) + 24) % 24
  return hour >= 20 || hour < 4
}

const slotsOf = (table: Table | undefined, night: boolean): Slot[] =>
  !table ? [] : "a" in table ? table.a : night ? table.n : table.d

/** One of `slots` by its percent, for a roll in [0, 1). */
const pick = (slots: Slot[], r: number) => {
  const total = slots.reduce((sum, [, percent]) => sum + Math.max(1, percent), 0)
  let left = r * total
  for (const [species, percent] of slots) if ((left -= Math.max(1, percent)) < 0) return species
  return slots.at(-1)![0]
}

/** Overshoots a little on the way up: 0 to 1 over `t` in [0, 1]. */
const pop = (t: number) => {
  const c = 2.2
  return 1 + (c + 1) * (t - 1) ** 3 + c * (t - 1) ** 2
}

type Spot = {
  map: number
  /** The map's cells of this kind, flat x, y pairs. */
  cells: number[]
  water: boolean
  /** Who comes up there: the land or rock table on land, surfing or fishing on water. */
  tables: (Table | undefined)[]
  box: { left: number; top: number; right: number; bottom: number }
}

/** The explorer's sightings overlay, reading the page's clock. */
export const sightingsOverlay = (
  encounters: Encounters,
  icons: HTMLImageElement,
  minutes: () => number,
  animate: boolean,
  now: () => number,
): Overlay => {
  const spots: Spot[] = []
  Object.values(encounters.maps).forEach((place, map) => {
    if (!place.e) return
    for (const [cells, water, tables] of [
      [place.l, false, [place.e.land, place.e.rock]],
      [place.w, true, [place.e.surf, place.e.old]],
    ] as const) {
      if (!cells?.length) continue
      const xs = cells.filter((_, i) => i % 2 === 0)
      const ys = cells.filter((_, i) => i % 2 === 1)
      spots.push({
        map,
        cells,
        water,
        tables: [...tables],
        box: {
          left: Math.min(...xs),
          top: Math.min(...ys),
          right: Math.max(...xs) + 1,
          bottom: Math.max(...ys) + 1,
        },
      })
    }
  })

  return (context, projection) => {
    if (!icons.complete || !icons.naturalWidth) return
    const zoom = projection.scale / projection.ratio
    const fade = Math.min(1, Math.max(0, (zoom - SIGHTING_ZOOM) / (FULL_ZOOM - SIGHTING_ZOOM)))
    if (fade <= 0) return
    const { width, height } = context.canvas
    const { scale } = projection
    const view = {
      left: projection.left - 2,
      top: projection.top - 2,
      right: projection.left + width / scale + 2,
      bottom: projection.top + height / scale + 2,
    }
    const night = isNight(minutes())
    // With reduced motion the sightings change on the clock's slower beat and just appear.
    const t = animate ? now() : Math.floor(now() / STRETCH_MS) * STRETCH_MS
    // An icon is 32 game pixels: two metatiles.
    const size = Math.round(2 * scale)
    const k = size / ICON
    context.imageSmoothingEnabled = false
    const drawn: {
      x: number
      y: number
      size: number
      water: boolean
      slot: number
      age: number
      left: number
    }[] = []
    for (const spot of spots) {
      const { box } = spot
      if (box.right < view.left || box.left > view.right) continue
      if (box.bottom < view.top || box.top > view.bottom) continue
      const slots = spot.tables.flatMap((table) => slotsOf(table, night))
      if (!slots.length) continue
      for (let cell = 0; cell < spot.cells.length / 2; cell++) {
        // Cells are out of step, so a route never empties or fills at once.
        const offset = roll(spot.map, cell, spot.water ? 1 : 0) * STRETCH_MS
        const stretch = Math.floor((t + offset) / STRETCH_MS)
        const age = t + offset - stretch * STRETCH_MS
        if (age > OUT_MS && animate) continue
        const seed = [spot.map, cell, stretch, spot.water ? 1 : 0] as const
        if (roll(...seed, 7) < QUIET) continue
        const x = spot.cells[cell * 2]! + 0.5
        const y = spot.cells[cell * 2 + 1]! + 0.5
        if (x < view.left || x > view.right || y < view.top || y > view.bottom) continue
        drawn.push({
          x,
          y,
          size,
          water: spot.water,
          slot: pick(slots, roll(...seed, 2)),
          age: animate ? age : POP_MS,
          left: animate ? OUT_MS - age : OUT_MS,
        })
      }
    }
    // Front to back: the lower on the map, the nearer.
    drawn.sort((a, b) => a.y - b.y)
    for (const sighting of drawn) {
      const at = projection.toScreen(sighting.x, sighting.y)
      const out = Math.min(1, sighting.age / POP_MS)
      const back = Math.min(1, sighting.left / DUCK_MS)
      const grow = Math.max(0.05, sighting.age < POP_MS ? pop(out) : back)
      const w = Math.max(1, Math.round(size * grow))
      const hop =
        animate && Math.floor((now() + sighting.slot * 97) / HOP_MS) % 2
          ? Math.max(1, Math.round(k))
          : 0
      // Feet on the cell's lower half; on the water, half under it.
      const sink = sighting.water ? Math.round(w * 0.35) : Math.round(scale * 0.15)
      const x = Math.round(at.x - w / 2)
      const y = Math.round(at.y + scale * 0.4 - w + sink) - hop
      context.globalAlpha = fade * Math.min(1, back * 2)
      if (sighting.water) {
        // A ripple ring where it breaks the surface.
        context.strokeStyle = "rgb(255 255 255 / 0.7)"
        context.lineWidth = Math.max(1, Math.round(k))
        context.beginPath()
        context.ellipse(
          Math.round(at.x),
          Math.round(at.y + scale * 0.4),
          w * 0.42,
          w * 0.14,
          0,
          0,
          Math.PI * 2,
        )
        context.stroke()
        context.save()
        context.beginPath()
        context.rect(x - 1, y - 1, w + 2, Math.round(at.y + scale * 0.4) - y + 1)
        context.clip()
      } else {
        // A flat shadow, the way the overworld draws one.
        context.fillStyle = "rgb(0 0 0 / 0.3)"
        context.beginPath()
        context.ellipse(
          Math.round(at.x),
          Math.round(at.y + scale * 0.4),
          w * 0.3,
          w * 0.1,
          0,
          0,
          Math.PI * 2,
        )
        context.fill()
      }
      const { columns } = encounters
      context.drawImage(
        icons,
        (sighting.slot % columns) * ICON,
        Math.floor(sighting.slot / columns) * ICON,
        ICON,
        ICON,
        x,
        y,
        w,
        w,
      )
      if (sighting.water) context.restore()
    }
    context.globalAlpha = 1
  }
}

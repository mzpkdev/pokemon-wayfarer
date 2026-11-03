// The tour's daily route, drawn: today's trainers where they stand, item
// balls where they lie, a glint where a hidden item is, and a faint mark
// where a spot is empty today. Who and what is where comes from `daily.ts`;
// this only draws it, popping everything in again when the day changes.
import type { Overlay } from "./render"

/** What the route shows on one day, as `dailyOn` resolves it. */
export type DailyScene = {
  trainers: { objects: { x: number; y: number; sprite: number }[] }[]
  items: { x: number; y: number; hidden: boolean }[]
  empty: { x: number; y: number }[]
}

/** The sprite sheet: square cells, `columns` to a row, the item ball's cell among them. */
export type DailySheet = {
  image: HTMLImageElement
  cell: number
  columns: number
  itemBall: number
}

/** The least a sprite is drawn tall, in CSS pixels, so a whole route's people still read. */
const MIN_HEIGHT = 28
/** How long a sprite takes to pop in on a new day, and how far apart they start. */
const POP_MS = 280
const STAGGER_MS = 60
/** A hidden item's glint: on this long, every this often. */
const GLINT_ON_MS = 500
const GLINT_EVERY_MS = 1400

/** Overshoots a little on the way up: 0 to 1 over `t` in [0, 1]. */
const pop = (t: number) => {
  const c = 2.2
  return 1 + (c + 1) * (t - 1) ** 3 + c * (t - 1) ** 2
}

/**
 * The overlay for the daily stop. `scene` gives today's scene and when the
 * day last changed; with `animate` off everything just stands.
 */
export const dailyOverlay =
  (
    sheet: DailySheet,
    scene: () => { scene: DailyScene; since: number } | null,
    animate: boolean,
    now: () => number,
  ): Overlay =>
  (context, projection) => {
    const today = scene()
    if (!today || !sheet.image.complete || !sheet.image.naturalWidth) return
    const { scale, ratio } = projection
    // Whole multiples of the sheet's pixels, at least MIN_HEIGHT tall and as big as the map's own.
    const k = Math.max(1, Math.round(Math.max(scale * 2, MIN_HEIGHT * ratio) / sheet.cell))
    const size = sheet.cell * k
    const age = animate ? now() - today.since : Infinity
    context.imageSmoothingEnabled = false
    let order = 0
    const draw = (cell: number, x: number, y: number) => {
      const own = age - order++ * STAGGER_MS
      if (own < 0) return
      const t = Math.min(1, own / POP_MS)
      const grow = Math.max(0.05, pop(t))
      const w = Math.round(size * grow)
      // Feet on the bottom of the tile.
      const at = projection.toScreen(x + 0.5, y + 1)
      context.globalAlpha = 0.3
      context.fillStyle = "#000"
      context.beginPath()
      context.ellipse(Math.round(at.x), Math.round(at.y - k), w * 0.22, w * 0.07, 0, 0, Math.PI * 2)
      context.fill()
      context.globalAlpha = 1
      context.drawImage(
        sheet.image,
        (cell % sheet.columns) * sheet.cell,
        Math.floor(cell / sheet.columns) * sheet.cell,
        sheet.cell,
        sheet.cell,
        Math.round(at.x - w / 2),
        Math.round(at.y - w),
        w,
        w,
      )
    }
    // Empty spots: a faint ring where somebody could stand.
    context.strokeStyle = "rgb(255 255 255 / 0.45)"
    context.lineWidth = Math.max(1, Math.round(ratio))
    for (const spot of today.scene.empty) {
      const at = projection.toScreen(spot.x + 0.5, spot.y + 1)
      context.setLineDash([3 * ratio, 3 * ratio])
      context.beginPath()
      context.ellipse(
        Math.round(at.x),
        Math.round(at.y - k),
        size * 0.22,
        size * 0.08,
        0,
        0,
        Math.PI * 2,
      )
      context.stroke()
    }
    context.setLineDash([])
    // Front to back: the lower on the map, the nearer.
    const people = today.scene.trainers
      .flatMap((trainer) => trainer.objects)
      .toSorted((a, b) => a.y - b.y)
    for (const item of today.scene.items.filter((spot) => !spot.hidden))
      draw(sheet.itemBall, item.x, item.y)
    for (const person of people) draw(person.sprite, person.x, person.y)
    // Hidden items glint now and then, each on its own beat.
    const t = animate ? now() : 0
    today.scene.items
      .filter((spot) => spot.hidden)
      .forEach((spot, i) => {
        if (animate && (t + i * 377) % GLINT_EVERY_MS > GLINT_ON_MS) return
        const at = projection.toScreen(spot.x + 0.5, spot.y + 0.5)
        const r = Math.max(3 * ratio, scale * 0.35)
        context.fillStyle = "rgb(255 255 255 / 0.9)"
        context.fillRect(
          Math.round(at.x - r),
          Math.round(at.y - ratio),
          Math.round(r * 2),
          Math.round(2 * ratio),
        )
        context.fillRect(
          Math.round(at.x - ratio),
          Math.round(at.y - r),
          Math.round(2 * ratio),
          Math.round(r * 2),
        )
      })
  }

// Where the map is looked at from: the world point at the view's focus, and
// how big a metatile is there, in CSS pixels.
import type { Bounds } from "./data"

export type Camera = { x: number; y: number; zoom: number }

/** The camera that fits `bounds` into a view `width` by `height` CSS pixels, with `room` to spare. */
export const fit = (bounds: Bounds, width: number, height: number, room = 0.95): Camera => ({
  x: (bounds.left + bounds.right) / 2,
  y: (bounds.top + bounds.bottom) / 2,
  zoom:
    Math.min(width / (bounds.right - bounds.left), height / (bounds.bottom - bounds.top)) * room,
})

const ease = (t: number) => (t < 0.5 ? 4 * t * t * t : 1 - (-2 * t + 2) ** 3 / 2)

/**
 * The camera `t` of the way (0 to 1, eased) from `from` to `to`. The zoom
 * changes geometrically so it feels even, and the camera pans in step with
 * how much the view widens, so the pull-back reads as one move.
 */
export const fly = (from: Camera, to: Camera, t: number): Camera => {
  const eased = ease(Math.min(1, Math.max(0, t)))
  const zoom = from.zoom * (to.zoom / from.zoom) ** eased
  const pan =
    from.zoom === to.zoom ? eased : (1 / zoom - 1 / from.zoom) / (1 / to.zoom - 1 / from.zoom)
  return {
    x: from.x + (to.x - from.x) * pan,
    y: from.y + (to.y - from.y) * pan,
    zoom,
  }
}

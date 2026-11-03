// The player on their bike, riding in from the road to their front door.
// Where each start's ride goes is in starts.ts; this draws the rider along it.
// He pedals while he moves, like holding a direction in the game, and stops
// and faces the door at the end.
import type { Sprite } from "./render"

type Facing = "south" | "north" | "west" | "east"

/** Standing and pedaling frames each way; east is west mirrored. */
const FRAMES: Record<Facing, { stand: number; pedal: [number, number] }> = {
  south: { stand: 0, pedal: [3, 4] },
  north: { stand: 1, pedal: [5, 6] },
  west: { stand: 2, pedal: [7, 8] },
  east: { stand: 2, pedal: [7, 8] },
}

/** Metatiles ridden per pedal stroke. */
const STROKE = 0.5

/** The player `t` of the way along `route` (0 to 1), as a sprite to draw. */
export const riderAt = (
  route: { x: number; y: number }[],
  t: number,
  sheet: CanvasImageSource,
): Sprite => {
  const lengths = route
    .slice(1)
    .map((point, i) => Math.hypot(point.x - route[i]!.x, point.y - route[i]!.y))
  const total = lengths.reduce((sum, length) => sum + length, 0)
  let left = Math.min(1, Math.max(0, t)) * total
  let leg = 0
  while (leg < lengths.length - 1 && left > lengths[leg]!) left -= lengths[leg++]!
  const from = route[leg]!
  const to = route[leg + 1]!
  const along = lengths[leg] ? left / lengths[leg]! : 0
  const dx = to.x - from.x
  const dy = to.y - from.y
  const facing: Facing =
    Math.abs(dx) > Math.abs(dy) ? (dx < 0 ? "west" : "east") : dy < 0 ? "north" : "south"
  // At the door he stops and faces it; before he sets off, he faces down the road.
  const arrived = t >= 1
  const shown = arrived ? "north" : facing
  const ridden = Math.min(1, Math.max(0, t)) * total
  const frames = FRAMES[shown]
  const frame =
    arrived || t <= 0
      ? frames.stand
      : Math.floor(ridden / STROKE) % 2 === 0
        ? frames.pedal[Math.floor(ridden / (STROKE * 2)) % 2]!
        : frames.stand
  return {
    image: sheet,
    source: { x: frame * 32, y: 0, width: 32, height: 32 },
    // His feet on the tile, the way the game stands a 32x32 object on its
    // square: two metatiles wide and tall, bottom on the tile's bottom.
    x: from.x + dx * along - 1,
    y: from.y + dy * along + 0.5 - 2,
    width: 2,
    height: 2,
    flip: shown === "east",
  }
}

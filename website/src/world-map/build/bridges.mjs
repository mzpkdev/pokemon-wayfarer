// Bridges: a stretch of a road map that is a deck over water, which ships
// pass under. The land builder digs a harbour's channel through one where
// that is the way to the sea, the ferries' lanes cross it, and the explorer
// draws its deck again over the ships. Rows and the deck's columns are the
// map's own, end exclusive.
export const BRIDGES = [
  // The cycling road is a long bridge over the sea west of VERMILION CITY;
  // under this stretch of it, the harbour's water reaches the open sea.
  { map: "Route17_hns", rows: [74, 98], deck: [11, 26] },
]

/** How far past the rows ships pass under the deck is drawn again, for a ship's own height. */
const DECK_MARGIN = 8

/**
 * The bridges on the world grid, given the placed maps (`name`, `x`, `y`,
 * `width`, `height` in grid cells): each one's `pass`, the rows ships pass
 * under across the map's full width, and `deck`, the cells drawn over them.
 */
export const placeBridges = (maps) =>
  BRIDGES.flatMap(({ map, rows, deck }) => {
    const p = maps.find((m) => m.name === map)
    if (!p) return []
    const top = p.y + rows[0],
      bottom = p.y + rows[1]
    return [
      {
        pass: { left: p.x, top, right: p.x + p.width, bottom },
        deck: {
          left: p.x + deck[0],
          top: Math.max(p.y, top - DECK_MARGIN),
          right: p.x + deck[1],
          bottom: Math.min(p.y + p.height, bottom + DECK_MARGIN),
        },
      },
    ]
  })

/**
 * The marked cells of a W x H mask as rectangles that hold nothing else: each
 * row's runs, joined to the run just above with the same ends.
 */
export const rectsOf = (mask, W, H) => {
  const rects = []
  let above = new Map()
  for (let y = 0; y < H; y++) {
    const row = new Map()
    for (let x = 0; x < W; x++) {
      if (!mask[y * W + x]) continue
      const left = x
      while (x < W && mask[y * W + x]) x++
      const key = `${left}:${x}`
      const rect = above.get(key) ?? { left, top: y, right: x, bottom: y }
      rect.bottom = y + 1
      if (!above.has(key)) rects.push(rect)
      row.set(key, rect)
    }
    above = row
  }
  return rects
}

/** Marks each cell inside any of `rects` in a W x H grid. */
export const maskOf = (rects, W, H) => {
  const mask = new Uint8Array(W * H)
  for (const { left, top, right, bottom } of rects)
    for (let y = Math.max(0, top); y < Math.min(H, bottom); y++)
      for (let x = Math.max(0, left); x < Math.min(W, right); x++) mask[y * W + x] = 1
  return mask
}

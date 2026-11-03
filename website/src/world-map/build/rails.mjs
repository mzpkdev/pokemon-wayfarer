// The Magnet Train's line, drawn over the world: only Route 32 holds its
// track, an elevated viaduct five metatiles tall that runs off both its edges.
// It is carried on to the two stations it serves, west into GOLDENROD CITY and
// east, then north, then east again into SAFFRON CITY, over whatever lies
// between. The way east is the one covering fewest map cells (towns most of
// all) for few turns. Runs north or south use the track turned on its side,
// new metatiles drawn from Route 32's.
const TRACK = { map: "Route32_hns", top: 49, rows: 5, pillar: 15, unit: 12 }
const ENDS = { west: "GoldenrodCity_hns", east: "SaffronCity_hns" }

export const layRails = (game, world, land, atlas) => {
  const { all, minX, minY, W, H } = world
  const grid = land.grid.slice()
  const placed = (name) => all.find((p) => p.m.name === name)
  const route = placed(TRACK.map),
    west = placed(ENDS.west),
    east = placed(ENDS.east)
  if (!route || !west || !east) return grid
  const gx = (p) => p.x - minX,
    gy = (p) => p.y - minY
  const top = gy(route) + TRACK.top
  const tileAt = (x, y) => grid[y * W + x]
  // One unit of track, from Route 32: `unit` columns from a pillar, `rows` tall.
  const unit = []
  for (let r = 0; r < TRACK.rows; r++)
    for (let c = 0; c < TRACK.unit; c++) unit.push(tileAt(gx(route) + TRACK.pillar + c, top + r))
  // The top row is part background; the track's pixels are those it shares
  // with the same column of track a unit along, over other ground.
  const mask = []
  for (let c = 0; c < TRACK.unit; c++) {
    const a = atlas.tiles[tileAt(gx(route) + TRACK.pillar + c, top)]
    const b = atlas.tiles[tileAt(gx(route) + TRACK.pillar + TRACK.unit + c, top)]
    const m = new Uint8Array(256)
    for (let p = 0; p < 256; p++)
      m[p] = a[p * 4] === b[p * 4] && a[p * 4 + 1] === b[p * 4 + 1] && a[p * 4 + 2] === b[p * 4 + 2]
    mask.push(m)
  }
  const made = new Map()
  const tile = (key, draw) => {
    if (!made.has(key)) {
      made.set(key, atlas.tiles.length)
      atlas.tiles.push(draw())
    }
    return made.get(key)
  }
  // A track cell: row `r` (0 is the top edge), phase `c`, over `under`, upright
  // or turned on its side (rows run left to right then, the top edge leftmost).
  const trackCell = (r, c, under, turned) => {
    const k = unit[r * TRACK.unit + c]
    if (!turned && r > 0) return k
    return tile(`${k}:${c}:${r === 0 ? under : ""}:${turned ? 1 : 0}`, () => {
      const src = atlas.tiles[k],
        bg = atlas.tiles[under]
      const out = new Uint8Array(1024)
      for (let y = 0; y < 16; y++)
        for (let x = 0; x < 16; x++) {
          // Turned: the pixel at (x, y) comes from the upright one at (y, x).
          const p = turned ? x * 16 + y : y * 16 + x
          const track = r > 0 || mask[c][p]
          out.set(
            (track ? src : bg).subarray(
              (track ? p : y * 16 + x) * 4,
              (track ? p : y * 16 + x) * 4 + 4,
            ),
            (y * 16 + x) * 4,
          )
        }
      return out
    })
  }
  const phase = (x) => (((x - gx(route) - TRACK.pillar) % TRACK.unit) + TRACK.unit) % TRACK.unit
  const across = (x0, x1, y) => {
    for (let x = x0; x < x1; x++)
      for (let r = 0; r < TRACK.rows; r++) {
        const i = (y + r) * W + x
        grid[i] = trackCell(r, phase(x), land.grid[i], false)
      }
  }
  // West, from Goldenrod's east edge to Route 32's west edge.
  across(gx(west) + west.l.width, gx(route), top)
  // East: the cheapest way from Route 32's east edge to Saffron's west edge,
  // level with its station: a line of east and north-or-south runs, each cell
  // costing the map cells the viaduct would cover there (a town's many times
  // a route's), and each turn as much as a long run over a route.
  const TOWN = 8,
    TURN = 600
  const weight = new Float32Array(all.length).map((_, m) =>
    ["MAP_TYPE_TOWN", "MAP_TYPE_CITY"].includes(all[m].m.map_type) ? TOWN : 1,
  )
  const cost = (x, y) => {
    const m = land.owner[y * W + x]
    return m >= 0 ? weight[m] : 0
  }
  const door = game.maps[east.m.id].warp_events?.find((w) => /TRAIN_STATION/.test(w.dest_map))
  const level = gy(east) + (door ? door.y - TRACK.rows + 1 : 6)
  const x0 = gx(route) + route.l.width,
    x1 = gx(east)
  const yLo = Math.max(0, Math.min(top, level) - 120),
    yHi = Math.min(H - TRACK.rows, Math.max(top, level) + 120)
  const span = yHi - yLo + 1
  const id = (x, y, d) => ((x - x0) * span + (y - yLo)) * 2 + d
  const states = (x1 - x0 + 1) * span * 2
  const dist = new Float64Array(states).fill(Infinity)
  const back = new Int32Array(states).fill(-1)
  // A run's cost at a cell: the five cells across it.
  const runCost = (x, y, d) => {
    let c = 0
    for (let r = 0; r < TRACK.rows; r++) c += d === 0 ? cost(x, y + r) : cost(x + r, y)
    return c
  }
  // Dijkstra over a binary heap.
  const heap = []
  const push = (s, v) => {
    heap.push([v, s])
    let i = heap.length - 1
    while (i > 0) {
      const j = (i - 1) >> 1
      if (heap[j][0] <= heap[i][0]) break
      ;[heap[i], heap[j]] = [heap[j], heap[i]]
      i = j
    }
  }
  const pop = () => {
    const top0 = heap[0],
      last = heap.pop()
    if (heap.length > 0) {
      heap[0] = last
      let i = 0
      for (;;) {
        const l = i * 2 + 1,
          r = l + 1
        let m = i
        if (l < heap.length && heap[l][0] < heap[m][0]) m = l
        if (r < heap.length && heap[r][0] < heap[m][0]) m = r
        if (m === i) break
        ;[heap[i], heap[m]] = [heap[m], heap[i]]
        i = m
      }
    }
    return top0
  }
  const start = id(x0, top, 0)
  dist[start] = 0
  push(start, 0)
  const goal = id(x1, level, 0)
  while (heap.length > 0) {
    const [v, s] = pop()
    if (v > dist[s]) continue
    if (s === goal) break
    const d = s % 2,
      cell = (s - d) / 2
    const x = x0 + Math.floor(cell / span),
      y = yLo + (cell % span)
    const next = []
    if (d === 0 && x < x1) next.push([x + 1, y, 0, 0])
    if (d === 1) {
      if (y > yLo) next.push([x, y - 1, 1, 0])
      if (y < yHi) next.push([x, y + 1, 1, 0])
    }
    // Turn in place: the corner is shared.
    next.push([x, y, 1 - d, TURN])
    for (const [nx, ny, nd, extra] of next) {
      const t = id(nx, ny, nd)
      const value = v + extra + runCost(nx, ny, nd) + 0.01
      if (value < dist[t]) {
        dist[t] = value
        back[t] = s
        push(t, value)
      }
    }
  }
  if (!Number.isFinite(dist[goal])) return grid
  const path = []
  for (let s = goal; s >= 0; s = back[s]) path.push(s)
  path.reverse()
  // Upright runs first, so the level runs pass over the corners.
  const cells = path.map((s) => {
    const d = s % 2,
      cell = (s - d) / 2
    return { x: x0 + Math.floor(cell / span), y: yLo + (cell % span), d }
  })
  for (const { x, y, d } of cells)
    if (d === 1)
      for (let r = 0; r < TRACK.rows; r++) {
        const i = y * W + x + r
        grid[i] = trackCell(r, phase(y), land.grid[i], true)
      }
  for (const { x, y, d } of cells)
    if (d === 0)
      for (let r = 0; r < TRACK.rows; r++) {
        const i = (y + r) * W + x
        grid[i] = trackCell(r, phase(x), land.grid[i], false)
      }
  // On into Saffron, up to the station's side.
  if (door) across(x1, gx(east) + door.x - 3, level)
  const turns = cells.filter((c, k) => k > 0 && c.d !== cells[k - 1].d).length
  console.log(`magnet train: ${turns} turns, cost ${Math.round(dist[goal])}`)
  return grid
}

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
  // Each way from Route 32 is the cheapest line of level runs and runs north
  // or south: each cell costs the map cells the viaduct covers there (a
  // town's many times a route's), and each turn as much as a long run.
  const TOWN = 8,
    TURN = 600
  const weight = new Float32Array(all.length).map((_, m) =>
    ["MAP_TYPE_TOWN", "MAP_TYPE_CITY"].includes(all[m].m.map_type) ? TOWN : 1,
  )
  const cost = (x, y) => {
    const m = land.owner[y * W + x]
    return m >= 0 ? weight[m] : 0
  }
  const runCost = (x, y, d) => {
    let c = 0
    for (let r = 0; r < TRACK.rows; r++) c += d === 0 ? cost(x, y + r) : cost(x + r, y)
    return c
  }
  const lay = (xStart, yStart, xGoal, yGoal) => {
    const step = Math.sign(xGoal - xStart) || 1
    const length = Math.abs(xGoal - xStart) + 1
    const yLo = Math.max(0, Math.min(yStart, yGoal) - 120),
      yHi = Math.min(H - TRACK.rows, Math.max(yStart, yGoal) + 120)
    const span = yHi - yLo + 1
    const id = (x, y, d) => (Math.abs(x - xStart) * span + (y - yLo)) * 2 + d
    const at = (s) => {
      const d = s % 2,
        cell = (s - d) / 2
      return { x: xStart + step * Math.floor(cell / span), y: yLo + (cell % span), d }
    }
    const dist = new Float64Array(length * span * 2).fill(Infinity)
    const back = new Int32Array(length * span * 2).fill(-1)
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
      const first = heap[0],
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
      return first
    }
    const start = id(xStart, yStart, 0),
      goal = id(xGoal, yGoal, 0)
    dist[start] = runCost(xStart, yStart, 0)
    push(start, dist[start])
    while (heap.length > 0) {
      const [v, s] = pop()
      if (v > dist[s]) continue
      if (s === goal) break
      const { x, y, d } = at(s)
      const next = []
      if (d === 0 && x !== xGoal) next.push([x + step, y, 0, 0])
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
    if (!Number.isFinite(dist[goal])) return null
    const cells = []
    for (let s = goal; s >= 0; s = back[s]) cells.push(at(s))
    // Upright runs first, so the level runs pass over the corners.
    for (const { x, y, d } of cells)
      if (d === 1)
        for (let r = 0; r < TRACK.rows; r++) {
          const i = y * W + x + r
          grid[i] = trackCell(r, phase(y), land.grid[i], true)
        }
    for (const { x, y, d } of cells) if (d === 0) across(x, x + 1, y)
    return {
      turns: cells.filter((c, k) => k > 0 && c.d !== cells[k - 1].d).length,
      cost: dist[goal],
    }
  }
  const stationDoor = (p) =>
    game.maps[p.m.id].warp_events?.find((w) => /TRAIN_STATION/.test(w.dest_map))
  // West: to Goldenrod's east edge, where the city's own track (three rows,
  // just above the station door) runs out; the viaduct's deck meets it there.
  const westDoor = stationDoor(west)
  const westLevel = gy(west) + (westDoor ? westDoor.y - 4 : 6)
  const toWest = lay(gx(route) - 1, top, gx(west) + west.l.width, westLevel)
  // East: to Saffron's west edge, level with its station, then on into the
  // city up to the station's side.
  const eastDoor = stationDoor(east)
  const eastLevel = gy(east) + (eastDoor ? eastDoor.y - TRACK.rows + 1 : 6)
  const toEast = lay(gx(route) + route.l.width, top, gx(east), eastLevel)
  if (toEast && eastDoor) across(gx(east), gx(east) + eastDoor.x - 3, eastLevel)
  for (const [name, way] of [
    ["west", toWest],
    ["east", toEast],
  ])
    console.log(
      way
        ? `magnet train ${name}: ${way.turns} turns, cost ${Math.round(way.cost)}`
        : `magnet train ${name}: no way found`,
    )
  return grid
}

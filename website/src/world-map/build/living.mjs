// The living world's data: the three S.S. ferries with their loops, schedule
// and sea lanes, and the times of day with the game's tint. Writes living.json.
//
// Ports, loops, port and sea minutes, the day and night hours and the tint
// colors are read from the game's own source, so they follow it. The lanes are
// the one thing the game doesn't have (its ferries are a menu and a timer, not
// a ship on a map), so they are routed here over the world grid's water, kept
// off the coasts and smoothed.
import fs from "node:fs"
import path from "node:path"
import { fileURLToPath } from "node:url"
import { distanceFrom } from "./land.mjs"

const here = path.dirname(fileURLToPath(import.meta.url))
const gameSource = path.resolve(here, "../../../../game")

/** Where each port sits: the map to anchor on and where in it (0 to 1), the dock side. */
const PORTS = {
  FERRY_PORT_OLIVINE: { name: "Olivine", map: "OlivineCity_PortOutside_hns", at: [0.55, 1] },
  FERRY_PORT_VERMILION: { name: "Vermilion", map: "VermilionCity_PortOutside_hns", at: [0.6, 1] },
  FERRY_PORT_SLATEPORT: { name: "Slateport", map: "SlateportCity", at: [0.5, 1] },
  FERRY_PORT_LILYCOVE: { name: "Lilycove", map: "LilycoveCity", at: [0.5, 1] },
  FERRY_PORT_ONE_ISLAND: { name: "One Island", map: "OneIsland_Frlg", at: [0.5, 0.5] },
  FERRY_PORT_SEVEN_ISLAND: { name: "Seven Island", map: "SevenIsland_Frlg", at: [0.5, 0.5] },
  FERRY_PORT_MELEMELE: { name: "Melemele", map: "MelemeleIsle_hns", at: [0.5, 0.5] },
}
const SHIPS = { FERRY_SHIP_AQUA: "aqua", FERRY_SHIP_ANNE: "anne", FERRY_SHIP_TIDAL: "tidal" }

/** Lanes stay at least this many metatiles off land; their ends sit this far off the dock. */
const CLEAR = 3
/** Beyond this far off land a cell costs nothing extra, short of it the cost climbs. */
const COAST_COST = 10

const read = (file) => fs.readFileSync(path.join(gameSource, file), "utf8")
const defineOf = (source, name) =>
  Number(source.match(new RegExp(`#define\\s+${name}\\s+(\\d+)`))[1])

/** The ferry constants, loops and ship names, from the game's source. */
const readFerries = () => {
  const constants = read("include/constants/wayfarer_ferry.h")
  const source = read("src/wayfarer_ferry.c")
  const loops = {}
  const block = source.match(/sLoops\[\]\[FERRY_LOOP_LENGTH\] =\s*\{([\s\S]*?)\n\};/)[1]
  for (const [, ship, ports] of block.matchAll(/\[(FERRY_SHIP_\w+)\]\s*=\s*\{([^}]*)\}/g))
    loops[SHIPS[ship]] = ports
      .split(",")
      .map((p) => p.trim())
      .filter(Boolean)
  const names = Object.fromEntries(
    [...source.matchAll(/sShipName(\w+)\[\] = _\("([^"]+)"\)/g)].map(([, key, text]) => [
      key.toLowerCase(),
      text,
    ]),
  )
  const hours = read("include/constants/rtc.h").match(/#if IS_WAYFARER([\s\S]*?)#elif/)[1]
  const hour = (name) => defineOf(hours, name)
  return {
    portMinutes: defineOf(constants, "FERRY_PORT_MINUTES"),
    seaMinutes: defineOf(constants, "FERRY_SEA_MINUTES"),
    loops,
    names,
    day: [hour("DAY_HOUR_BEGIN"), hour("DAY_HOUR_END")],
    periods: [
      ["Night", hour("NIGHT_HOUR_BEGIN"), hour("NIGHT_HOUR_END")],
      ["Morning", hour("MORNING_HOUR_BEGIN"), hour("MORNING_HOUR_END")],
      ["Day", hour("DAY_HOUR_BEGIN"), hour("DAY_HOUR_END")],
      ["Evening", hour("EVENING_HOUR_BEGIN"), hour("EVENING_HOUR_END")],
    ],
    source,
  }
}

/** The tint colors per period: the byte per channel the game multiplies the palette by (/256). */
const q = (n) => Math.floor(n * 256)
const bytes = (value) => [value & 255, (value >> 8) & 255, (value >> 16) & 255]
const readTints = () => {
  const source = read("src/overworld.c")
  const night = source.match(
    /#define TINT_NIGHT Q_8_8\(([\d.]+)\) \| Q_8_8\(([\d.]+)\) << 8 \| Q_8_8\(([\d.]+)\) << 16/,
  )
  const colors = Object.fromEntries(
    [...source.matchAll(/\[TIME_(\w+)\]\s*=\s*\{\.coeff = \d+,\s*\.blendColor = (\w+),/g)].map(
      ([, time, color]) => [time.toLowerCase(), color],
    ),
  )
  const resolve = (color) =>
    color === "TINT_NIGHT"
      ? [q(night[1]), q(night[2]), q(night[3])]
      : color === "0"
        ? [256, 256, 256]
        : bytes(Number(color))
  return Object.fromEntries(Object.entries(colors).map(([time, color]) => [time, resolve(color)]))
}

/** The ship's hour at which the schedule table first has it at its first port, in game minutes. */
const firstBerth = (source, ship, port) => {
  const ports = source.match(/sPorts\[FERRY_PORT_COUNT\] =\s*\{([\s\S]*?)\n\};/)[1]
  const entry = ports.match(new RegExp(`\\[${port}\\]\\s*=\\s*\\{([\\s\\S]*?)\\n    \\},`))[1]
  const [, day, night] = entry.match(/(FERRY_SHIP_\w+), (FERRY_SHIP_\w+), (?:TRUE|FALSE), s/)
  return SHIPS[day] === ship ? "day" : SHIPS[night] === ship ? "night" : null
}

/** A binary min-heap of [cost, cell] pairs. */
const createHeap = () => {
  const costs = []
  const cells = []
  return {
    get size() {
      return cells.length
    },
    push(cost, cell) {
      let i = cells.length
      costs.push(cost)
      cells.push(cell)
      while (i > 0) {
        const parent = (i - 1) >> 1
        if (costs[parent] <= costs[i]) break
        ;[costs[parent], costs[i]] = [costs[i], costs[parent]]
        ;[cells[parent], cells[i]] = [cells[i], cells[parent]]
        i = parent
      }
    },
    pop() {
      const top = cells[0]
      const lastCost = costs.pop()
      const lastCell = cells.pop()
      if (cells.length) {
        costs[0] = lastCost
        cells[0] = lastCell
        let i = 0
        for (;;) {
          const l = 2 * i + 1
          const r = l + 1
          let m = i
          if (l < cells.length && costs[l] < costs[m]) m = l
          if (r < cells.length && costs[r] < costs[m]) m = r
          if (m === i) break
          ;[costs[m], costs[i]] = [costs[i], costs[m]]
          ;[cells[m], cells[i]] = [cells[i], cells[m]]
          i = m
        }
      }
      return top
    },
  }
}

export const writeLiving = ({ grid, isWater, width: W, height: H, maps }, outDir) => {
  const ferries = readFerries()
  const N = W * H
  const land = new Uint8Array(N)
  for (let i = 0; i < N; i++) land[i] = isWater(grid[i]) ? 0 : 1
  const toLand = distanceFrom(land, W, H)
  // Only water the open sea reaches counts: not a pond or a lake inside a map.
  const open = new Uint8Array(N)
  {
    const stack = []
    const seed = (i) => {
      if (!land[i] && !open[i]) {
        open[i] = 1
        stack.push(i)
      }
    }
    for (let x = 0; x < W; x++) {
      seed(x)
      seed((H - 1) * W + x)
    }
    for (let y = 0; y < H; y++) {
      seed(y * W)
      seed(y * W + W - 1)
    }
    while (stack.length) {
      const i = stack.pop()
      const x = i % W
      if (x > 0) seed(i - 1)
      if (x < W - 1) seed(i + 1)
      if (i >= W) seed(i - W)
      if (i < N - W) seed(i + W)
    }
  }
  // How far off land a lane may go; narrow straits make it give a little.
  let clearMin = CLEAR
  const sailable = (i, min = clearMin) => open[i] && toLand[i] >= min

  // The port's point: the open water at least CLEAR off land nearest where the map's dock side is.
  const portPoint = (port) => {
    const map = maps.find((m) => m.name === port.map)
    const cx = Math.round(map.x + map.width * port.at[0])
    const cy = Math.round(map.y + map.height * port.at[1])
    let best = -1
    let bestDistance = Infinity
    for (let r = 0; r < 120 && best < 0; r++)
      for (let y = Math.max(0, cy - r); y <= Math.min(H - 1, cy + r); y++)
        for (let x = Math.max(0, cx - r); x <= Math.min(W - 1, cx + r); x++) {
          const i = y * W + x
          if (!sailable(i, CLEAR)) continue
          const d = (x - cx) ** 2 + (y - cy) ** 2
          if (d < bestDistance) {
            bestDistance = d
            best = i
          }
        }
    if (best < 0) throw new Error(`no open water near ${port.name}`)
    return [best % W, Math.floor(best / W)]
  }

  const NEIGHBORS = [
    [1, 0, 1],
    [-1, 0, 1],
    [0, 1, 1],
    [0, -1, 1],
    [1, 1, Math.SQRT2],
    [1, -1, Math.SQRT2],
    [-1, 1, Math.SQRT2],
    [-1, -1, Math.SQRT2],
  ]
  const cost = new Float64Array(N)
  const from = new Int32Array(N)
  const search = (a, b) => {
    cost.fill(Infinity)
    from.fill(-1)
    const start = a[1] * W + a[0]
    const goal = b[1] * W + b[0]
    const heap = createHeap()
    cost[start] = 0
    heap.push(0, start)
    while (heap.size) {
      const i = heap.pop()
      if (i === goal) break
      const x = i % W
      const y = (i - x) / W
      for (const [dx, dy, step] of NEIGHBORS) {
        const nx = x + dx
        const ny = y + dy
        if (nx < 0 || ny < 0 || nx >= W || ny >= H) continue
        const j = ny * W + nx
        if (!sailable(j)) continue
        const near = Math.max(0, (COAST_COST - toLand[j]) / COAST_COST)
        const next = cost[i] + step * (1 + 8 * near * near)
        if (next >= cost[j]) continue
        cost[j] = next
        from[j] = i
        heap.push(next + Math.hypot(nx - b[0], ny - b[1]), j)
      }
    }
    if (from[goal] < 0) return null
    const cells = []
    for (let i = goal; i >= 0; i = from[i]) cells.push([i % W, Math.floor(i / W)])
    return cells.toReversed()
  }

  // Whether a straight run between two points keeps CLEAR off land.
  const clear = (a, b) => {
    const steps = Math.ceil(Math.hypot(b[0] - a[0], b[1] - a[1]) * 2)
    for (let s = 0; s <= steps; s++) {
      const x = Math.round(a[0] + ((b[0] - a[0]) * s) / steps)
      const y = Math.round(a[1] + ((b[1] - a[1]) * s) / steps)
      if (!sailable(y * W + x)) return false
    }
    return true
  }
  /** Pulls the path taut: each point jumps to the furthest one in clear view. */
  const pull = (cells) => {
    const kept = [cells[0]]
    let at = 0
    while (at < cells.length - 1) {
      let next = at + 1
      for (let j = cells.length - 1; j > at + 1; j--)
        if (clear(cells[at], cells[j])) {
          next = j
          break
        }
      kept.push(cells[next])
      at = next
    }
    return kept
  }
  /**
   * Rounds each corner of a taut path with a quadratic curve that starts and
   * ends up to `reach` along its two sides, taking a smaller bend where the
   * full one would come within CLEAR - 1 of land.
   */
  const round = (points, reach = 24) => {
    const line = [points[0]]
    for (let i = 1; i < points.length - 1; i++) {
      const [px, py] = points[i - 1]
      const [cx, cy] = points[i]
      const [nx, ny] = points[i + 1]
      const before = Math.hypot(cx - px, cy - py)
      const after = Math.hypot(nx - cx, ny - cy)
      for (let t = Math.min(reach, before / 2, after / 2); ; t /= 2) {
        const a = [cx + ((px - cx) / before) * t, cy + ((py - cy) / before) * t]
        const b = [cx + ((nx - cx) / after) * t, cy + ((ny - cy) / after) * t]
        const bend = []
        for (let k = 0; k <= 12; k++) {
          const u = k / 12
          bend.push([
            (1 - u) ** 2 * a[0] + 2 * u * (1 - u) * cx + u ** 2 * b[0],
            (1 - u) ** 2 * a[1] + 2 * u * (1 - u) * cy + u ** 2 * b[1],
          ])
        }
        if (
          t < 1 ||
          bend.every(([x, y]) => sailable(Math.round(y) * W + Math.round(x), clearMin - 1))
        ) {
          line.push(...(t < 1 ? [[cx, cy]] : bend))
          break
        }
      }
    }
    line.push(points.at(-1))
    return line
  }

  const portNames = Object.keys(PORTS)
  const points = Object.fromEntries(portNames.map((id) => [id, portPoint(PORTS[id])]))
  const lanes = {}
  const laneOf = (a, b) => {
    const key = `${a}>${b}`
    if (lanes[key]) return key
    let cells = null
    for (const min of [CLEAR, 2, 1.5, 1]) {
      clearMin = min
      cells = search(points[a], points[b])
      if (cells) break
    }
    if (!cells) throw new Error(`no sea lane ${key}`)
    const pulled = pull(cells)
    console.log(
      key,
      "raw min",
      Math.min(...cells.map(([x, y]) => toLand[y * W + x])),
      "pulled",
      pulled.map(([x, y]) => `${x},${y}:${toLand[y * W + x]}`).join(" "),
    )
    const line = round(pulled)
    // The smoothing must not have cut a corner into the coast.
    const worst = Math.min(...line.map(([x, y]) => toLand[Math.round(y) * W + Math.round(x)]))
    console.log(
      `  lane ${key}: ${line.length} points, nearest land ${worst.toFixed(1)}, clearance used ${clearMin}`,
    )
    lanes[key] = line.map(([x, y]) => [Math.round(x * 10) / 10, Math.round(y * 10) / 10])
    return key
  }

  const ships = Object.entries(ferries.loops).map(([id, loop]) => {
    // The cycle starts the first moment the schedule table has this ship at
    // its first port: 8:00 if it's that port's day ship, else 20:00.
    const shift = firstBerth(ferries.source, id, loop[0])
    return {
      id,
      name: ferries.names[id],
      ports: loop,
      lanes: loop.map((port, i) => laneOf(port, loop[(i + 1) % loop.length])),
      start: (shift === "night" ? ferries.day[1] : ferries.day[0]) * 60,
    }
  })

  const tints = readTints()
  const living = {
    ports: Object.fromEntries(
      portNames.map((id) => [id, { name: PORTS[id].name, x: points[id][0], y: points[id][1] }]),
    ),
    ferry: { portMinutes: ferries.portMinutes, seaMinutes: ferries.seaMinutes, ships },
    lanes,
    // Hours the periods begin and end, and the tint multiplier (/256) at hours
    // the game blends between: evening 0:00, night 1:00 to 4:00, morning 6:00,
    // day 8:00 to 20:00, evening again by 24:00.
    periods: ferries.periods.map(([name, begin, end]) => ({ name, from: begin, to: end })),
    tint: [
      [0, tints.evening],
      [1, tints.night],
      [4, tints.night],
      [6, tints.morning],
      [8, tints.day],
      [20, tints.day],
      [24, tints.evening],
    ],
  }
  fs.writeFileSync(`${outDir}/living.json`, JSON.stringify(living))
  const size = fs.statSync(`${outDir}/living.json`).size
  console.log(
    "living.json",
    ships.length,
    "ships,",
    Object.keys(lanes).length,
    "lanes,",
    size,
    "size",
  )
}

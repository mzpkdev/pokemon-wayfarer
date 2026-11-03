// The notables' data: the Gym Leaders and Elite Four the game's world
// simulation runs, with their homes, activity cycles and favourite places on
// the world grid, and the walking routes between those places. Writes
// notables.json.
//
// Read from the game (../game/tools/wayfarer_world): who the 25 simulated
// trainers are, their home map and Gym, activity cycle, favourites and the
// named spots' places. Indoor places are put on the land at the door of the
// outdoor map that leads in (the gym-door logic of landmarks.mjs).
//
// Not the game's: the game moves them one map hop per map change, with no
// clock and no walking path. The routes here are found over the world grid's
// land, and the page times them (see src/world-map/notables.ts).
import fs from "node:fs"
import path from "node:path"
import { fileURLToPath } from "node:url"
import { createDoors, townName } from "./landmarks.mjs"
import { createHeap } from "./living.mjs"

const here = path.dirname(fileURLToPath(import.meta.url))
const tools = path.resolve(here, "../../../../game/tools")
const readJson = (file) => JSON.parse(fs.readFileSync(path.join(tools, file), "utf8"))

/** What a step of a cycle costs to cross one metatile of: land on a placed map, filler land, open water. */
const COST = { map: 1, filler: 1.6, dark: 4, water: 12 }
/** A leg with this many water cells (or more) goes by sea. */
const SEA_CELLS = 8
/** Route points closer than this to a straight line between their neighbours are dropped. */
const TOLERANCE = 1.5

/** Places the world map doesn't hold, put on the placed map they hang off. The Battle Frontier has none. */
const HUNG_OFF = { TrainerHill_Courtyard_hns: "Route40_hns" }

/** "PewterCity_hns" to "Pewter City". */
const placeName = (map) =>
  townName(map)
    .toLowerCase()
    .replace(/\b[a-z]/g, (c) => c.toUpperCase())

/** Douglas-Peucker over [x, y] points. */
const simplify = (points) => {
  if (points.length < 3) return points
  const keep = new Uint8Array(points.length)
  keep[0] = keep[points.length - 1] = 1
  const stack = [[0, points.length - 1]]
  while (stack.length) {
    const [a, b] = stack.pop()
    const [ax, ay] = points[a]
    const [bx, by] = points[b]
    const len = Math.hypot(bx - ax, by - ay) || 1
    let far = -1
    let farD = TOLERANCE
    for (let i = a + 1; i < b; i++) {
      const d = Math.abs((bx - ax) * (ay - points[i][1]) - (ax - points[i][0]) * (by - ay)) / len
      if (d > farD) {
        farD = d
        far = i
      }
    }
    if (far < 0) continue
    keep[far] = 1
    stack.push([a, far], [far, b])
  }
  return points.filter((_, i) => keep[i])
}

export const writeNotables = (game, world, land, landmarks, outDir) => {
  const { all, minX, minY, W, H } = world
  const { grid, owner, isWater } = land
  const { every, doorTo } = createDoors(game, world)
  const routines = readJson("wayfarer_world/routines.json").trainers
  // The simulated walkers, then the Gym Leaders whose sprite only stands,
  // who wait at their Gym (source/stationary-leaders.json; same order as the
  // sprite sheet).
  const stationary = JSON.parse(
    fs.readFileSync(new URL("./source/stationary-leaders.json", import.meta.url), "utf8"),
  ).leaders.map((s) => ({ ...routines.find((r) => r.slug === s.slug), ...s, stationary: true }))
  const trainers = [...routines.filter((t) => t.simulated), ...stationary]
  const spots = readJson("wayfarer_world/named_spots.json").spots
  const catalog = Object.fromEntries(
    readJson("notable_trainers/catalog.json").trainers.map((t) => [t.slug, t]),
  )

  const idOf = Object.fromEntries(Object.values(every).map((m) => [m.name, m.id]))
  const placedByName = new Map(all.map((p) => [p.m.name, p]))

  // Land and its cost per cell; open sea and water are walkable too, at a
  // price, so a ferry crossing can still be drawn.
  const N = W * H
  const cost = new Float32Array(N)
  const landCell = new Uint8Array(N)
  for (let i = 0; i < N; i++) {
    const k = grid[i]
    const water = k === 0xffff || isWater(k)
    landCell[i] = water ? 0 : 1
    cost[i] = water ? COST.water : owner[i] >= 0 ? COST.map : COST.filler
  }

  /** The nearest land cell to a point, as a cell index. */
  const snap = (x, y, reach = 14) => {
    const cx = Math.min(W - 1, Math.max(0, Math.floor(x)))
    const cy = Math.min(H - 1, Math.max(0, Math.floor(y)))
    let best = -1
    let bestD = Infinity
    for (let dy = -reach; dy <= reach; dy++)
      for (let dx = -reach; dx <= reach; dx++) {
        const gx = cx + dx
        const gy = cy + dy
        if (gx < 0 || gy < 0 || gx >= W || gy >= H) continue
        const i = gy * W + gx
        if (!landCell[i]) continue
        const d = dx * dx + dy * dy
        if (d < bestD) {
          bestD = d
          best = i
        }
      }
    return best
  }

  /** A land cell with water beside it, nearest the middle of a placed map: somewhere to fish from. */
  const shore = (p) => {
    const cx = p.x - minX + p.l.width / 2
    const cy = p.y - minY + p.l.height / 2
    let best = -1
    let bestD = Infinity
    for (let y = p.y - minY; y < p.y - minY + p.l.height; y++)
      for (let x = p.x - minX; x < p.x - minX + p.l.width; x++) {
        const i = y * W + x
        if (!landCell[i] || owner[i] < 0) continue
        if (landCell[i - 1] && landCell[i + 1] && landCell[i - W] && landCell[i + W]) continue
        const d = (x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2
        if (d < bestD) {
          bestD = d
          best = i
        }
      }
    return best
  }

  // Where a map is on the grid: a placed outdoor map's own tile or middle, or
  // the door of the placed map the way in leads from.
  const unplaced = new Set()
  const pointIn = (mapName, kind, x, y) => {
    if (HUNG_OFF[mapName]) return pointIn(HUNG_OFF[mapName], kind)
    const id = idOf[mapName]
    const p = placedByName.get(mapName)
    if (p) {
      if (x !== undefined && y !== undefined)
        return snap(p.x - minX + x + 0.5, p.y - minY + y + 0.5)
      if (kind === "water_edge") {
        const cell = shore(p)
        if (cell >= 0) return cell
      }
      return snap(p.x - minX + p.l.width / 2, p.y - minY + p.l.height / 2)
    }
    const door = id && doorTo(id)
    if (door) return snap(door.x, door.y, 6)
    unplaced.add(mapName)
    return -1
  }

  const stops = []
  const stopIndex = new Map()
  const stopAt = (cell, label, prep) => {
    const key = `${cell}|${label}`
    if (!stopIndex.has(key)) {
      stopIndex.set(key, stops.length)
      stops.push({ cell, label, prep })
    }
    return stopIndex.get(key)
  }

  // Indigo Plateau is no placed map: its Town Map cell is the league pin.
  const indigo = landmarks.circuit.find((c) => c.id === "indigo")

  /** The outdoor place a trainer's town is: where their Gym's door is, or their home map. */
  const homeOf = (t) => {
    const town = placedByName.get(t.home)
    if (t.gym) {
      const door = doorTo(idOf[t.gym])
      if (door)
        return {
          cell: snap(door.x, door.y, 6),
          map: door.p.m.name,
          label: `${placeName(door.p.m.name)} Gym`,
          town_cell: snap(
            door.p.x - minX + door.p.l.width / 2,
            door.p.y - minY + door.p.l.height / 2,
          ),
          town_label: placeName(door.p.m.name),
        }
    }
    if (town) {
      return {
        cell: snap(town.x - minX + town.l.width / 2, town.y - minY + town.l.height / 2),
        map: t.home,
        label: placeName(t.home),
        town: true,
      }
    }
    if (t.home === "IndigoPlateau_hns") {
      const near = placedByName.get("Route22_hns")
      return {
        cell: snap(indigo.x, indigo.y, 80),
        map: near?.m.name ?? t.home,
        label: "Indigo Plateau",
        town: true,
        noCard: true,
      }
    }
    // A home map that is not placed (an island's): where its door leads.
    const door = doorTo(idOf[t.home])
    if (door)
      return {
        cell: snap(door.x, door.y, 6),
        map: door.p.m.name,
        label: placeName(door.p.m.name),
        town: true,
      }
    throw new Error(`no place for ${t.slug}'s home ${t.home}`)
  }

  const out = []
  /** Pairs of stops a route is needed between, for every step to the next. */
  const wanted = new Set()
  for (const t of trainers) {
    const home = homeOf(t)
    const homeStop = stopAt(home.cell, home.label, home.town ? "in" : "at")
    const gymStop = t.gym ? homeStop : undefined
    const townStop = home.town ? homeStop : stopAt(home.town_cell, home.town_label, "in")
    const info = catalog[t.slug]
    const cycle = (t.stationary ? ["wait"] : t.cycle).map((activity) => {
      let options = []
      for (const fav of t.favourites ?? []) {
        if (fav.serves !== activity || fav.disabled) continue
        const spot =
          fav.kind === "named"
            ? spots.find((s) => s.label === fav.label && s.map === fav.map)
            : undefined
        const cell = pointIn(fav.map, fav.kind, fav.x ?? spot?.x, fav.y ?? spot?.y)
        if (cell < 0) continue
        options.push(stopAt(cell, fav.label ?? fav.note ?? placeName(fav.map), "at"))
      }
      if (!options.length) {
        // No favourite for the step: it is chosen near the home base, the
        // Gym for a Gym Leader's training and rest at home, the town otherwise.
        options = [
          gymStop !== undefined &&
          (activity === "train" || activity === "home" || activity === "wait")
            ? gymStop
            : townStop,
        ]
      }
      return { a: activity, s: [...new Set(options)] }
    })
    cycle.forEach((step, i) => {
      for (const next of cycle[(i + 1) % cycle.length].s)
        for (const from of step.s)
          if (from !== next) wanted.add(from < next ? `${from}>${next}` : `${next}>${from}`)
    })
    out.push({
      slug: t.slug,
      name: t.name,
      role: info?.role ?? (t.gym ? "Gym Leader" : "Elite Four"),
      region: info?.homeRegion ?? t.badge?.region,
      home: homeStop,
      homeMap: home.noCard ? undefined : home.map,
      cycle,
      ...(t.offset ? { offset: t.offset } : {}),
    })
  }

  // The route between two stops: the cheapest way over the grid's land, kept
  // to the placed maps where it can, with open water only when there is no
  // land to cross (a ferry or a ship).
  const g = new Float32Array(N)
  const seen = new Int32Array(N)
  const parent = new Int32Array(N)
  const closed = new Int32Array(N)
  let stamp = 0
  const route = (from, to) => {
    stamp++
    const tx = to % W
    const ty = (to - tx) / W
    const heuristic = (i) => {
      const x = i % W
      return Math.hypot(x - tx, (i - x) / W - ty)
    }
    const heap = createHeap()
    g[from] = 0
    seen[from] = stamp
    parent[from] = -1
    heap.push(heuristic(from), from)
    while (heap.size) {
      const i = heap.pop()
      if (closed[i] === stamp) continue
      closed[i] = stamp
      if (i === to) break
      const x = i % W
      const y = (i - x) / W
      for (let dy = -1; dy <= 1; dy++)
        for (let dx = -1; dx <= 1; dx++) {
          if (!dx && !dy) continue
          const nx = x + dx
          const ny = y + dy
          if (nx < 0 || ny < 0 || nx >= W || ny >= H) continue
          const j = ny * W + nx
          const step = dx && dy ? Math.SQRT2 : 1
          const c = g[i] + step * ((cost[i] + cost[j]) / 2)
          if (seen[j] !== stamp || c < g[j]) {
            seen[j] = stamp
            g[j] = c
            parent[j] = i
            heap.push(c + heuristic(j), j)
          }
        }
    }
    const cells = []
    for (let i = to; i !== -1; i = parent[i]) cells.push(i)
    cells.reverse()
    return cells
  }

  const legs = {}
  let seaLegs = 0
  for (const key of [...wanted].toSorted()) {
    const [a, b] = key.split(">").map(Number)
    const cells = route(stops[a].cell, stops[b].cell)
    const water = cells.filter((i) => !landCell[i]).length
    if (water < SEA_CELLS) {
      legs[key] = { pts: simplify(cells.map((i) => [i % W, Math.floor(i / W)])).flat() }
      continue
    }
    // A leg that crosses water is simplified a run of land or water at a
    // time, so each segment is all one or the other; `w` marks the water
    // segments, where the walker leaves a wake.
    seaLegs++
    const points = []
    const w = []
    let start = 0
    for (let i = 1; i <= cells.length; i++) {
      if (i < cells.length && landCell[cells[i]] === landCell[cells[start]]) continue
      // The run's last cell is shared with the next run's first point.
      const run = simplify(
        cells.slice(start, Math.min(i + 1, cells.length)).map((c) => [c % W, Math.floor(c / W)]),
      )
      const skip = points.length ? 1 : 0
      for (let k = skip; k < run.length; k++) {
        points.push(run[k])
        if (points.length > 1) w.push(landCell[cells[start]] ? 0 : 1)
      }
      start = i
    }
    legs[key] = { pts: points.flat(), w }
  }

  const data = {
    stops: stops.map(({ cell, label, prep }) => [cell % W, Math.floor(cell / W), label, prep]),
    trainers: out,
    legs,
  }
  fs.writeFileSync(`${outDir}/notables.json`, JSON.stringify(data))
  return {
    trainers: out.length,
    stops: stops.length,
    legs: Object.keys(legs).length,
    seaLegs,
    unplaced: [...unplaced],
  }
}

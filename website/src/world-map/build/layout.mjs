// Where every map goes in the world: pieces of connected maps are laid out
// along a spanning tree of their connections and settled round loops, loose
// pieces are placed by their Town Map cells, overlaps are separated, and the
// regions are arranged on one sea.
import { connections, readTownMapCells, targetOf } from "./game.mjs"

// Connections only promise that each pair of neighbors lines up; around a
// loop of maps the offsets need not add up, so one link per loop must give.
// Place a piece's maps along a spanning tree of links, and choose which links
// to leave out so the fewest tiles end up drawn over each other: a leftover
// mismatch then shows as a gap, filled like any other, instead of one map cut
// through another.
const edgeKey = (a, b) => [a, b].toSorted().join(" ~ ")
const walk = (game, root, forbidden) => {
  const { maps, layouts } = game
  const placed = new Map()
  const queue = [[root, 0, 0]]
  while (queue.length > 0) {
    const [id, x, y] = queue.shift()
    if (placed.has(id)) continue
    const m = maps[id],
      l = layouts[m.layout]
    placed.set(id, { x, y, m, l })
    for (const c of connections(m)) {
      const target = targetOf(m, c)
      const t = maps[target]
      if (!t || placed.has(target) || forbidden.has(edgeKey(id, target))) continue
      const tl = layouts[t.layout],
        o = c.offset
      const at = {
        up: [x + o, y - tl.height],
        down: [x + o, y + l.height],
        left: [x - tl.width, y + o],
        right: [x + l.width, y + o],
      }[c.direction]
      if (at) queue.push([target, ...at])
    }
  }
  return placed
}
const overlap = (list) => {
  let area = 0
  for (let i = 0; i < list.length; i++)
    for (let j = i + 1; j < list.length; j++) {
      const a = list[i],
        b = list[j]
      const w = Math.min(a.x + a.l.width, b.x + b.l.width) - Math.max(a.x, b.x)
      const h = Math.min(a.y + a.l.height, b.y + b.l.height) - Math.max(a.y, b.y)
      if (w > 0 && h > 0) area += w * h
    }
  return area
}
const layOut = (game, root) => {
  const { maps } = game
  const forbidden = new Set()
  let placed = walk(game, root, forbidden)
  const total = placed.size
  const links = new Set()
  for (const id of placed.keys())
    for (const c of connections(maps[id]))
      if (placed.has(targetOf(maps[id], c))) links.add(edgeKey(id, targetOf(maps[id], c)))
  let best = overlap([...placed.values()])
  for (let round = 0; round < 12 && best > 0; round++) {
    let pick = null
    for (const link of links) {
      if (forbidden.has(link)) continue
      forbidden.add(link)
      const trial = walk(game, root, forbidden)
      forbidden.delete(link)
      if (trial.size < total) continue
      const area = overlap([...trial.values()])
      if (area < best) {
        best = area
        pick = link
      }
    }
    if (!pick) break
    forbidden.add(pick)
  }
  placed = walk(game, root, forbidden)
  if (forbidden.size > 0) console.log(`  ${maps[root].name}: left out ${[...forbidden].join(", ")}`)
  return settle([...placed.values()], forbidden)
}

// The links left out of the tree still say where their maps belong. Settle
// every link at once, one axis at a time. Along an edge a link is exact: the
// maps it joins share that coordinate, so a road crossing the edge lines up.
// Across the edge it may give: a loop's leftover error becomes gaps between
// maps (which `land.mjs` bridges by stretching both edges toward each other),
// spread over the loop's links, and overlaps only where gaps can't take it.
// Links given up on (see `settle`), by `edgeKey`.
const dropped = new Set()
const linkList = (list, withDropped = false) => {
  const at = new Map(list.map((p) => [p.m.id, p]))
  const links = []
  for (const a of list)
    for (const c of connections(a.m)) {
      const b = at.get(targetOf(a.m, c))
      if (!b || a.m.id > b.m.id) continue
      if (!withDropped && dropped.has(edgeKey(a.m.id, b.m.id))) continue
      const o = c.offset
      const want = {
        up: [o, -b.l.height],
        down: [o, a.l.height],
        left: [-b.l.width, o],
        right: [a.l.width, o],
      }[c.direction]
      if (!want) continue
      const vertical = c.direction === "up" || c.direction === "down"
      links.push({ a, b, want, vertical })
    }
  return links
}
const placementErrors = (list) =>
  linkList(list)
    .map(({ a, b, want }) => ({ a, b, dx: b.x - a.x - want[0], dy: b.y - a.y - want[1] }))
    .filter(({ dx, dy }) => dx || dy)

const settleAxis = (list, links, axis, apart = []) => {
  const key = axis === 0 ? "x" : "y"
  // Links exact on this axis: vertical ones fix x, horizontal ones fix y.
  const exact = links.filter((link) => link.vertical === (axis === 0))
  const loose = links.filter((link) => link.vertical !== (axis === 0))
  // Groups held together by exact links, each map's place within its group.
  const group = new Map(),
    within = new Map()
  const neighbors = new Map(list.map((p) => [p, []]))
  for (const { a, b, want } of exact) {
    neighbors.get(a).push([b, want[axis]])
    neighbors.get(b).push([a, -want[axis]])
  }
  let groups = 0
  for (const start of list) {
    if (group.has(start)) continue
    group.set(start, groups)
    within.set(start, 0)
    const queue = [start]
    while (queue.length > 0) {
      const p = queue.shift()
      for (const [q, d] of neighbors.get(p))
        if (!group.has(q)) {
          group.set(q, groups)
          within.set(q, within.get(p) + d)
          queue.push(q)
        }
    }
    groups++
  }
  // A loop of exact links that doesn't add up can't be exact all round; its
  // maps keep the tree's places relative to each other and it is reported.
  const broken = exact.filter(({ a, b, want }) => within.get(b) - within.get(a) !== want[axis])
  if (broken.length)
    console.log(
      `  ${key}: ${broken.length} exact links off (${broken.map(({ a, b }) => `${a.m.name} ~ ${b.m.name}`).join(", ")})`,
    )
  // Each group's offset by weighted least squares over the loose links, a gap
  // weighing far less than an overlap, reweighted until the overlaps settle.
  const offset = new Float64Array(groups)
  const root = group.get(list[0])
  for (const p of list) offset[group.get(p)] = p[key] - within.get(p)
  const between = loose
    .map(({ a, b, want }) => ({
      ga: group.get(a),
      gb: group.get(b),
      d: want[axis] - within.get(b) + within.get(a),
      // Which way is apart: b lies past a on this axis by `want`.
      sign: Math.sign(want[axis]) || 1,
      weight: 1,
    }))
    .filter(({ ga, gb }) => ga !== gb)
  // Maps that aren't linked but would overlap: kept at least `d` apart, a
  // pull that only acts while they overlap.
  for (const { p, q, d } of apart) {
    const ga = group.get(p),
      gb = group.get(q)
    if (ga === gb) continue
    between.push({
      ga,
      gb,
      d: d - within.get(q) + within.get(p),
      sign: Math.sign(d) || 1,
      weight: 0,
      apart: true,
    })
  }
  for (let round = 0; round < 30; round++) {
    for (let sweep = 0; sweep < 1500; sweep++)
      for (let g = 0; g < groups; g++) {
        if (g === root) continue
        let sum = 0,
          weight = 0
        for (const e of between) {
          if (e.gb === g) {
            sum += e.weight * (offset[e.ga] + e.d)
            weight += e.weight
          } else if (e.ga === g) {
            sum += e.weight * (offset[e.gb] - e.d)
            weight += e.weight
          }
        }
        if (weight > 0) offset[g] = sum / weight
      }
    for (const e of between) {
      const gap = (offset[e.gb] - offset[e.ga] - e.d) * e.sign
      if (e.apart) e.weight = gap < -0.5 ? Math.max(30, e.weight * 3) : 0
      else e.weight = gap < -0.5 ? Math.min(e.weight * 3, 1e4) : 1
    }
  }
  for (const p of list) p[key] = Math.round(offset[group.get(p)]) + within.get(p)
}

// Maps covering each other more than a few edge rows, in cells.
const deepOverlap = (list) => {
  let area = 0
  for (let i = 0; i < list.length; i++)
    for (let j = i + 1; j < list.length; j++) {
      const p = list[i],
        q = list[j]
      if (p.m.name === "SootopolisCity" || q.m.name === "SootopolisCity") continue
      const w = Math.min(p.x + p.l.width, q.x + q.l.width) - Math.max(p.x, q.x)
      const h = Math.min(p.y + p.l.height, q.y + q.l.height) - Math.max(p.y, q.y)
      if (Math.min(w, h) >= 4) area += w * h
    }
  return area
}

// Settles a piece with every link; where that lays maps deep over each other
// (a connection the ported maps get wrong), gives up the tree's left-out
// links one at a time, whichever helps most, and settles again.
const settle = (list, leftOut) => {
  if (list.length < 2) return list
  const start = list.map((p) => [p.x, p.y])
  const trial = () => {
    list.forEach((p, k) => {
      p.x = start[k][0]
      p.y = start[k][1]
    })
    settleWith(list)
    return deepOverlap(list)
  }
  let best = trial()
  const candidates = new Set(leftOut)
  while (best > 0 && candidates.size > 0) {
    let pick = null
    for (const link of candidates) {
      dropped.add(link)
      const area = trial()
      dropped.delete(link)
      if (area < best && (!pick || area < pick.area)) pick = { link, area }
    }
    if (!pick) break
    dropped.add(pick.link)
    candidates.delete(pick.link)
    best = pick.area
    console.log(`  gave up ${pick.link}`)
  }
  trial()
  report(list)
  return list
}

const settleWith = (list) => {
  const links = linkList(list)
  const root = list[0]
  const origin = { x: root.x, y: root.y }
  settleAxis(list, links, 0)
  settleAxis(list, links, 1)
  // Then push apart maps that overlap without a link between them, along
  // whichever axis they overlap less, and settle that axis again.
  const linked = new Set(links.map(({ a, b }) => `${a.m.id}|${b.m.id}`))
  const apart = [[], []]
  for (let round = 0; round < 6; round++) {
    let added = 0
    for (let i = 0; i < list.length; i++)
      for (let j = i + 1; j < list.length; j++) {
        const p = list[i],
          q = list[j]
        if (linked.has(`${p.m.id}|${q.m.id}`) || linked.has(`${q.m.id}|${p.m.id}`)) continue
        const w = Math.min(p.x + p.l.width, q.x + q.l.width) - Math.max(p.x, q.x)
        const h = Math.min(p.y + p.l.height, q.y + q.l.height) - Math.max(p.y, q.y)
        if (w <= 0 || h <= 0) continue
        const axis = w <= h ? 0 : 1
        const key = axis === 0 ? "x" : "y",
          size = axis === 0 ? "width" : "height"
        const after = q[key] + q.l[size] / 2 >= p[key] + p.l[size] / 2
        apart[axis].push({ p, q, d: after ? p.l[size] + 1 : -(q.l[size] + 1) })
        added++
      }
    if (added === 0) break
    settleAxis(list, links, 0, apart[0])
    settleAxis(list, links, 1, apart[1])
  }
  // Keep the root where it was.
  const dx = origin.x - root.x,
    dy = origin.y - root.y
  for (const p of list) {
    p.x += dx
    p.y += dy
  }
}

const report = (list) => {
  const links = linkList(list)
  const root = list[0]
  const errors = placementErrors(list)
  const along = errors.filter(({ a, b, dx: ex, dy: ey }) => {
    const link = links.find((l) => l.a === a && l.b === b)
    return link.vertical ? ex !== 0 : ey !== 0
  })
  console.log(
    `  ${root.m.name}: ${errors.length} links apart (${along.length} along an edge), overlap ${overlap(list)}, deep ${deepOverlap(list)}`,
  )
  return list
}

// Town Map cells, to place the pieces no connection reaches. Emerald's event
// islands have none. Faraway Island, on no Town Map at all, lies out at sea
// south of Hoenn. Navel Rock and Birth Island sit near FRLG's cells for them
// on the Sevii Town Maps (10,8 and 18,13), moved just clear of the Five and
// Six Island pieces, which the world draws bigger than their cells: on the
// cells themselves the pieces pushed apart would shuffle the islands round.
const STAND_IN_CELLS = {
  MAPSEC_FARAWAY_ISLAND: { x: 15, y: 16 },
  MAPSEC_NAVEL_ROCK: { x: 8, y: 9 },
  MAPSEC_BIRTH_ISLAND: { x: 18, y: 20 },
}
export const cellLookup = (cells) => (m) => {
  if (STAND_IN_CELLS[m.region_map_section]) return STAND_IN_CELLS[m.region_map_section]
  const section = cells.get(m.region_map_section)
  return section && { x: section.x + section.width / 2, y: section.y + section.height / 2 }
}

/**
 * The regions, each a set of pieces in one Town Map's cell space. The first
 * root's piece is laid out by its connections; the cell scale (metatiles per
 * cell) and the region's origin then place every other piece by one of its
 * map's cells. `at` puts the region in the world, in metatiles.
 */
const REGIONS = [
  { name: "Johto and Kanto", roots: ["MAP_NEW_BARK_TOWN_HNS"], at: [0, 0] },
  {
    name: "Hoenn",
    roots: [
      "MAP_LITTLEROOT_TOWN",
      "MAP_SOOTOPOLIS_CITY",
      // The islands Lilycove's charters sail to.
      "MAP_SOUTHERN_ISLAND_EXTERIOR",
      "MAP_FARAWAY_ISLAND_ENTRANCE",
      "MAP_BATTLE_FRONTIER_OUTSIDE_WEST",
    ],
    at: [-280, 740],
  },
  { name: "Sinjoh", roots: ["MAP_ROUTE49_HNS"], at: [0, 0], beside: "Sinjoh" },
  {
    name: "Sinnoh",
    roots: [
      "MAP_TWINLEAF_TOWN",
      "MAP_HEARTHOME_CITY",
      "MAP_FIGHT_AREA",
      "MAP_SNOWPOINT_CITY",
      "MAP_ROUTE224",
    ],
    at: [0, 0],
    beside: "Sinnoh",
  },
  {
    name: "Sevii Islands",
    roots: [
      "MAP_ONE_ISLAND",
      "MAP_TWO_ISLAND",
      "MAP_THREE_ISLAND",
      "MAP_THREE_ISLAND_BERRY_FOREST",
      "MAP_MT_EMBER_EXTERIOR",
      "MAP_FOUR_ISLAND",
      "MAP_FIVE_ISLAND",
      "MAP_SIX_ISLAND",
      "MAP_SIX_ISLAND_PATTERN_BUSH",
      "MAP_SEVEN_ISLAND",
      // Vermilion's Seagallop sails to these two too.
      "MAP_NAVEL_ROCK_EXTERIOR",
      "MAP_BIRTH_ISLAND_EXTERIOR_HNS",
    ],
    at: [780, 700],
  },
  { name: "Alola", roots: ["MAP_MELEMELE_ISLE_HNS"], at: [1400, 880] },
]
/**
 * Enclaves: outdoor places the game reaches only through a door, so no connection leads to
 * them. The Safari Zones are the ones worth showing: each
 * is a piece of connected maps, set down by hand `at` so many metatiles from the top left
 * of the placed map it is `beside`, in the nearest room there is to its gate, and it joins
 * that map's region. The land between grows round it like any other.
 *
 * - Hoenn's lies north of Route 121, as on Hoenn's own Town Map, a little east of its gate
 *   to keep clear of Route 120.
 * - Johto's lies where HeartGold and SoulSilver have it: west of Cianwood City by Route 47's
 *   cliffs (through the Cliff Edge Gate), north up Route 48 to the Safari Zone Gate, with the
 *   Safari Zone behind it.
 * - Kanto's has no room north of Fuchsia City, between the city and the channel the ships
 *   take to Vermilion, so it lies on the shore south of Route 15, east of Route 19.
 */
const ENCLAVES = [
  { root: "MAP_SAFARI_ZONE_SOUTH", beside: "Route121", at: [5, -93] },
  { root: "MAP_ROUTE47_HNS", beside: "CianwoodCity_hns", at: [-124, -59] },
  { root: "MAP_SAFARI_ZONE_ENTERANCE_HNS", beside: "CianwoodCity_hns", at: [-136, -146] },
  { root: "MAP_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS", beside: "FuchsiaCity_hns", at: [43, 51] },
]

// Each Sevii Town Map is its own cell space; set them out like the islands lie.
const SEVII_MAPS = [
  {
    islands: [
      "ONE",
      "TWO",
      "THREE",
      "KINDLE",
      "TREASURE",
      "CAPE",
      "BOND",
      "BERRY",
      "MT_EMBER",
      "THREE_ISLE",
    ],
    at: [0, 0],
  },
  {
    islands: ["FOUR", "FIVE", "RESORT", "WATER_LABYRINTH", "FIVE_ISLE", "MEMORIAL", "NAVEL_ROCK"],
    at: [6, 14],
  },
  {
    islands: [
      "SIX",
      "SEVEN",
      "TRAINER_TOWER",
      "CANYON",
      "SEVAULT",
      "TANOBY",
      "WATER_PATH",
      "GREEN_PATH",
      "RUIN_VALLEY",
      "OUTCAST",
      "PATTERN",
      "BIRTH_ISLAND",
    ],
    at: [-6, 30],
  },
]
const seviiCellLookup = (cellOf) => (m) => {
  const cell = cellOf(m)
  if (!cell) return null
  const name = m.region_map_section.replace("MAPSEC_", "")
  const group = SEVII_MAPS.find(({ islands }) => islands.some((island) => name.startsWith(island)))
  return group ? { x: cell.x + group.at[0], y: cell.y + group.at[1] } : cell
}

const center = (p) => ({ x: p.x + p.l.width / 2, y: p.y + p.l.height / 2 })
// Metatiles per Town Map cell, measured on Hoenn, whose maps all connect.
export const measureCell = (hoennPiece, cellOf) => {
  const towns = hoennPiece.filter((p) => cellOf(p.m))
  let num = 0,
    den = 0
  for (const a of towns)
    for (const b of towns) {
      const ca = cellOf(a.m),
        cb = cellOf(b.m),
        pa = center(a),
        pb = center(b)
      num += (pa.x - pb.x) * (ca.x - cb.x) + (pa.y - pb.y) * (ca.y - cb.y)
      den += (ca.x - cb.x) ** 2 + (ca.y - cb.y) ** 2
    }
  return num / den
}

// A Town Map cell is coarse, so pieces placed by their cells can land on one
// another. Nudge each piece but the first in shrinking steps, whichever way
// most reduces what it covers of the others, until nothing overlaps (with a
// few tiles of room). Sootopolis belongs inside Route 126's crater, so the
// two may overlap.
const ROOM = 3
const covered = (a, b) => {
  if (a.m.name === "SootopolisCity" || b.m.name === "SootopolisCity") return 0
  const w = Math.min(a.x + a.l.width, b.x + b.l.width) - Math.max(a.x, b.x) + ROOM
  const h = Math.min(a.y + a.l.height, b.y + b.l.height) - Math.max(a.y, b.y) + ROOM
  return w > 0 && h > 0 ? w * h : 0
}
const pieceOverlap = (piece, others) => {
  let area = 0
  for (const p of piece) for (const q of others) area += covered(p, q)
  return area
}
const separate = (pieces) => {
  for (let round = 0; round < 400; round++) {
    let moved = false
    for (let k = 1; k < pieces.length; k++) {
      const others = pieces.filter((_, j) => j !== k).flat()
      let area = pieceOverlap(pieces[k], others)
      if (area === 0) continue
      for (const step of [64, 32, 16, 8, 4, 2, 1]) {
        let best = null
        for (const [dx, dy] of [
          [step, 0],
          [-step, 0],
          [0, step],
          [0, -step],
        ]) {
          for (const p of pieces[k]) {
            p.x += dx
            p.y += dy
          }
          const trial = pieceOverlap(pieces[k], others)
          for (const p of pieces[k]) {
            p.x -= dx
            p.y -= dy
          }
          if (trial < area && (!best || trial < best.area)) best = { dx, dy, area: trial }
        }
        if (best) {
          for (const p of pieces[k]) {
            p.x += best.dx
            p.y += best.dy
          }
          area = best.area
          moved = true
          break
        }
      }
    }
    if (!moved) break
  }
}

/**
 * Places every map of the world. Returns the placed maps (`all`, each with its
 * map `m`, layout `l`, position `x`, `y` and `region`), and the world's size
 * `W` x `H` with the top left `minX`, `minY`, in metatiles, sea margin included
 * (a map of an enclave has `enclave` set),
 * and the `links` between placed maps (given-up ones too).
 */
export const placeWorld = (game) => {
  const { maps } = game
  const cellOf = cellLookup(readTownMapCells(game))
  const seviiCell = seviiCellLookup(cellOf)
  const hoennPiece = layOut(game, "MAP_LITTLEROOT_TOWN")
  const CELL = measureCell(hoennPiece, cellOf)
  console.log("metatiles per Town Map cell:", CELL.toFixed(1))

  const all = []
  const regionBounds = {}
  for (const region of REGIONS) {
    const cellFor = region.name === "Sevii Islands" ? seviiCell : cellOf
    // The Sevii Town Maps are drawn much closer in than Hoenn's.
    const scale = region.name === "Sevii Islands" ? CELL * 0.4 : CELL
    let origin = null
    const pieces = []
    for (const root of region.roots) {
      if (!maps[root]) {
        console.log(`  missing root ${root}`)
        continue
      }
      if (pieces.some((piece) => piece.some((p) => p.m.id === root))) continue
      const piece = root === "MAP_LITTLEROOT_TOWN" ? hoennPiece : layOut(game, root)
      // Line the piece up on the region's cells, by the average of its maps' cells.
      const anchored = piece.filter((p) => cellFor(p.m))
      if (origin === null) {
        origin = anchored.length
          ? {
              x:
                anchored.reduce(
                  (sum, p) => sum + (p.x + p.l.width / 2 - cellFor(p.m).x * scale),
                  0,
                ) / anchored.length,
              y:
                anchored.reduce(
                  (sum, p) => sum + (p.y + p.l.height / 2 - cellFor(p.m).y * scale),
                  0,
                ) / anchored.length,
            }
          : { x: 0, y: 0 }
      } else if (anchored.length) {
        const dx =
          anchored.reduce(
            (sum, p) => sum + (cellFor(p.m).x * scale + origin.x - (p.x + p.l.width / 2)),
            0,
          ) / anchored.length
        const dy =
          anchored.reduce(
            (sum, p) => sum + (cellFor(p.m).y * scale + origin.y - (p.y + p.l.height / 2)),
            0,
          ) / anchored.length
        for (const p of piece) {
          p.x += Math.round(dx)
          p.y += Math.round(dy)
        }
      }
      pieces.push(piece)
    }
    separate(pieces)
    const placed = pieces.flat()
    const left = Math.min(...placed.map((p) => p.x)),
      top = Math.min(...placed.map((p) => p.y))
    for (const p of placed) {
      p.x += region.at[0] - left
      p.y += region.at[1] - top
      p.region = region.name
    }
    regionBounds[region.name] = {
      width: Math.max(...placed.map((p) => p.x + p.l.width)) - region.at[0],
      height: Math.max(...placed.map((p) => p.y + p.l.height)) - region.at[1],
    }
    console.log(
      `${region.name}: ${placed.length} maps in ${pieces.length} pieces, ${regionBounds[region.name].width}x${regionBounds[region.name].height}`,
    )
    all.push(...placed)
  }
  // Sinjoh lies past MT. SILVER, north of Route 28, and Sinnoh beyond it, with
  // MT. CORONET's south side facing Sinjoh, close enough that the fill between
  // them reads as one stretch of land.
  const shift = (region, dx, dy) => {
    for (const p of all)
      if (p.region === region) {
        p.x += dx
        p.y += dy
      }
  }
  const bounds = (region) => {
    const list = all.filter((p) => p.region === region)
    return {
      left: Math.min(...list.map((p) => p.x)),
      top: Math.min(...list.map((p) => p.y)),
      right: Math.max(...list.map((p) => p.x + p.l.width)),
      bottom: Math.max(...list.map((p) => p.y + p.l.height)),
    }
  }
  const route28 = all.find((p) => p.m.name === "Route28_hns")
  {
    const sinjoh = bounds("Sinjoh")
    shift(
      "Sinjoh",
      Math.round(route28.x + route28.l.width / 2 - (sinjoh.left + sinjoh.right) / 2),
      0,
    )
    // Seat it on whichever Johto and Kanto maps lie below it.
    const placed = bounds("Sinjoh")
    const below = all.filter(
      (p) => p.region === "Johto and Kanto" && p.x < placed.right && p.x + p.l.width > placed.left,
    )
    shift("Sinjoh", 0, Math.min(...below.map((p) => p.y)) - 6 - placed.bottom)
    const sinjohNow = bounds("Sinjoh")
    const sinnohBelow = all.filter(
      (p) => p.region === "Sinnoh" && p.x < sinjohNow.right && p.x + p.l.width > sinjohNow.left,
    )
    const coronet = all.find((p) => p.m.name === "Route207")
    const dx = Math.round(
      (sinjohNow.left + sinjohNow.right) / 2 - (coronet.x + coronet.l.width / 2),
    )
    const sinnohBottom = Math.max(
      ...all
        .filter(
          (p) =>
            p.region === "Sinnoh" &&
            p.x + dx < sinjohNow.right &&
            p.x + p.l.width + dx > sinjohNow.left,
        )
        .map((p) => p.y + p.l.height),
    )
    void sinnohBelow
    // Away from Sinjoh, keep a strait between Sinnoh and Johto and Kanto wide
    // enough that their coasts don't meet, so the land runs only through Sinjoh.
    const STRAIT = 44
    let lift = sinjohNow.top - 6 - sinnohBottom
    for (const s of all.filter((p) => p.region === "Sinnoh")) {
      const left = s.x + dx,
        right = left + s.l.width
      if (left < sinjohNow.right + STRAIT / 2 && right > sinjohNow.left - STRAIT / 2) continue
      for (const j of all.filter((p) => p.region === "Johto and Kanto"))
        if (left < j.x + j.l.width + STRAIT && right > j.x - STRAIT)
          lift = Math.min(lift, j.y - STRAIT - (s.y + s.l.height))
    }
    shift("Sinnoh", dx, lift)
    console.log("  Sinnoh to Sinjoh gap:", sinjohNow.top - (sinnohBottom + lift))
  }
  for (const p of all.filter((q) => q.region === "Sinjoh"))
    console.log("  ", p.m.name, p.x, p.y, p.l.width, p.l.height)
  // The enclaves, each beside a map already placed. Their place is given, not solved, so
  // nothing else moves for them.
  for (const enclave of ENCLAVES) {
    const beside = all.find((p) => p.m.name === enclave.beside)
    if (!maps[enclave.root] || !beside) {
      console.log(`  missing enclave ${enclave.root} or ${enclave.beside}`)
      continue
    }
    const piece = layOut(game, enclave.root)
    const left = Math.min(...piece.map((p) => p.x)),
      top = Math.min(...piece.map((p) => p.y))
    for (const p of piece) {
      p.x += beside.x + enclave.at[0] - left
      p.y += beside.y + enclave.at[1] - top
      p.region = beside.region
      p.enclave = true
    }
    const taken = piece.flatMap((p) => all.filter((q) => covered(p, q)).map((q) => q.m.name))
    if (taken.length)
      throw new Error(`the enclave ${enclave.root} lies over ${[...new Set(taken)].join(", ")}`)
    all.push(...piece)
  }
  // Open sea all round.
  const MARGIN = 72
  const minX = Math.min(...all.map((p) => p.x)) - MARGIN,
    minY = Math.min(...all.map((p) => p.y)) - MARGIN
  const W = Math.max(...all.map((p) => p.x + p.l.width)) - minX + MARGIN,
    H = Math.max(...all.map((p) => p.y + p.l.height)) - minY + MARGIN
  console.log("world", all.length, "maps,", W, "x", H, "metatiles")
  return { all, minX, minY, W, H, links: linkList(all, true) }
}

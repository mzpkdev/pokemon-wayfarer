// Where every map goes in the world: pieces of connected maps are laid out
// along a spanning tree of their connections and relaxed round loops, loose
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
  return relax([...placed.values()])
}

// The links left out of the tree still say where their maps belong, and
// their whole mismatch sits on that one link. Spread it round the loop: move
// every map a little toward where all its links want it, then push apart any
// maps that touch, until it settles. A loop's error then shows as a few short
// gaps the fill turns into land, not one long one.
const linkErrors = (list) => {
  const at = new Map(list.map((p) => [p.m.id, p]))
  const links = []
  for (const a of list)
    for (const c of connections(a.m)) {
      const b = at.get(targetOf(a.m, c))
      if (!b || a.m.id > b.m.id) continue
      const o = c.offset
      const [dx, dy] = {
        up: [o, -b.l.height],
        down: [o, a.l.height],
        left: [-b.l.width, o],
        right: [a.l.width, o],
      }[c.direction] ?? [null, null]
      if (dx === null) continue
      links.push({ a, b, dx, dy })
    }
  return links
}
const relax = (list) => {
  const links = linkErrors(list)
  if (links.length < list.length) return list
  // The worst link is what shows: one long gap or skewed road.
  const error = () =>
    Math.max(
      ...links.map(({ a, b, dx, dy }) => Math.abs(b.x - a.x - dx) + Math.abs(b.y - a.y - dy)),
    )
  const before = error()
  if (before === 0) return list
  const start = list.map((p) => [p.x, p.y])
  const root = list[0]
  for (let round = 0; round < 400; round++) {
    const pull = new Map(list.map((p) => [p, { x: 0, y: 0, n: 0 }]))
    for (const { a, b, dx, dy } of links) {
      const pa = pull.get(a),
        pb = pull.get(b)
      pb.x += a.x + dx - b.x
      pb.y += a.y + dy - b.y
      pb.n++
      pa.x += b.x - dx - a.x
      pa.y += b.y - dy - a.y
      pa.n++
    }
    for (const p of list) {
      if (p === root) continue
      const { x, y, n } = pull.get(p)
      if (n === 0) continue
      p.x += Math.round((x / n) * 0.5)
      p.y += Math.round((y / n) * 0.5)
    }
    // Push touching maps apart along their shallower overlap.
    for (let i = 0; i < list.length; i++)
      for (let j = i + 1; j < list.length; j++) {
        const a = list[i],
          b = list[j]
        const w = Math.min(a.x + a.l.width, b.x + b.l.width) - Math.max(a.x, b.x)
        const h = Math.min(a.y + a.l.height, b.y + b.l.height) - Math.max(a.y, b.y)
        if (w <= 0 || h <= 0) continue
        const [mover, still] = b === root ? [a, b] : [b, a]
        if (w < h) mover.x += mover.x + mover.l.width / 2 >= still.x + still.l.width / 2 ? w : -w
        else mover.y += mover.y + mover.l.height / 2 >= still.y + still.l.height / 2 ? h : -h
      }
  }
  const after = error()
  const clash = overlap(list)
  if (clash > 0 || after >= before) {
    list.forEach((p, k) => {
      p.x = start[k][0]
      p.y = start[k][1]
    })
    console.log(
      `  ${root.m.name}: kept the tree (worst link ${after} vs ${before}, overlap ${clash})`,
    )
  } else {
    const worst = links.toSorted(
      (p, q) =>
        Math.abs(q.b.x - q.a.x - q.dx) +
        Math.abs(q.b.y - q.a.y - q.dy) -
        (Math.abs(p.b.x - p.a.x - p.dx) + Math.abs(p.b.y - p.a.y - p.dy)),
    )[0]
    console.log(
      `  ${root.m.name}: worst link ${before} -> ${after} metatiles (${worst.a.m.name} ~ ${worst.b.m.name})`,
    )
  }
  return list
}

// Town Map cells, to place the pieces no connection reaches.
export const cellLookup = (cells) => (m) => {
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
    roots: ["MAP_LITTLEROOT_TOWN", "MAP_SOOTOPOLIS_CITY"],
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
    ],
    at: [780, 700],
  },
  { name: "Alola", roots: ["MAP_MELEMELE_ISLE_HNS"], at: [1400, 880] },
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
  { islands: ["FOUR", "FIVE", "RESORT", "WATER_LABYRINTH", "FIVE_ISLE", "MEMORIAL"], at: [6, 14] },
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
 * `W` x `H` with the top left `minX`, `minY`, in metatiles, sea margin included.
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
  // Open sea all round.
  const MARGIN = 72
  const minX = Math.min(...all.map((p) => p.x)) - MARGIN,
    minY = Math.min(...all.map((p) => p.y)) - MARGIN
  const W = Math.max(...all.map((p) => p.x + p.l.width)) - minX + MARGIN,
    H = Math.max(...all.map((p) => p.y + p.l.height)) - minY + MARGIN
  console.log("world", all.length, "maps,", W, "x", H, "metatiles")
  return { all, minX, minY, W, H }
}

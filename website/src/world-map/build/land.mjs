// The land between the maps: fills the world grid with the placed maps'
// metatiles, then grows land out from them with noise, keeps regions apart
// with straits of sea, and fills the rest from the maps' border blocks.
import { maskOf, placeBridges } from "./bridges.mjs"

// Fractal value noise, fixed seed, in [-1, 1].
const hash = (x, y) => {
  let h = (x * 374761393 + y * 668265263) ^ 0x5bd1e995
  h = Math.imul(h ^ (h >>> 13), 1274126177)
  return ((h ^ (h >>> 16)) >>> 0) / 4294967296
}
const valueNoise = (x, y) => {
  const ix = Math.floor(x),
    iy = Math.floor(y)
  const fx = x - ix,
    fy = y - iy
  const sx = fx * fx * (3 - 2 * fx),
    sy = fy * fy * (3 - 2 * fy)
  const top = hash(ix, iy) + (hash(ix + 1, iy) - hash(ix, iy)) * sx
  const bottom = hash(ix, iy + 1) + (hash(ix + 1, iy + 1) - hash(ix, iy + 1)) * sx
  return top + (bottom - top) * sy
}
export const fractal = (x, y, base = 1 / 48) => {
  let sum = 0,
    weight = 0,
    amplitude = 1,
    frequency = base
  for (let octave = 0; octave < 4; octave++) {
    sum += amplitude * valueNoise(x * frequency + octave * 17.3, y * frequency - octave * 9.1)
    weight += amplitude
    amplitude /= 2
    frequency *= 2
  }
  return (sum / weight) * 2 - 1
}
// Domain-warped: the noise sampled at a point pushed about by more noise,
// which bends coastlines into capes and inlets instead of ripples.
const coastNoise = (x, y) => {
  const wx = fractal(x + 101, y - 57, 1 / 90) * 28
  const wy = fractal(x - 33, y + 211, 1 / 90) * 28
  return fractal(x + wx, y + wy, 1 / 70)
}

// Distance (in metatiles, chamfer 3-4) from each cell to the nearest cell in `seed`.
export const distanceFrom = (seed, W, H) => {
  const N = W * H
  const d = new Float32Array(N).fill(1e9)
  for (let i = 0; i < N; i++) if (seed[i]) d[i] = 0
  for (let y = 0; y < H; y++)
    for (let x = 0; x < W; x++) {
      const i = y * W + x
      let v = d[i]
      if (x > 0) v = Math.min(v, d[i - 1] + 1)
      if (y > 0) {
        v = Math.min(v, d[i - W] + 1)
        if (x > 0) v = Math.min(v, d[i - W - 1] + Math.SQRT2)
        if (x < W - 1) v = Math.min(v, d[i - W + 1] + Math.SQRT2)
      }
      d[i] = v
    }
  for (let y = H - 1; y >= 0; y--)
    for (let x = W - 1; x >= 0; x--) {
      const i = y * W + x
      let v = d[i]
      if (x < W - 1) v = Math.min(v, d[i + 1] + 1)
      if (y < H - 1) {
        v = Math.min(v, d[i + W] + 1)
        if (x < W - 1) v = Math.min(v, d[i + W + 1] + Math.SQRT2)
        if (x > 0) v = Math.min(v, d[i + W - 1] + Math.SQRT2)
      }
      d[i] = v
    }
  return d
}

// For each cell, the nearest cell that `owner` marks (>= 0), by a breadth-first
// sweep out from all of them at once (-1 where there are none).
const nearestOf = (owner, W, H) => {
  const N = W * H
  const from = new Int32Array(N).fill(-1)
  const queue = new Int32Array(N)
  let tail = 0
  for (let i = 0; i < N; i++)
    if (owner[i] >= 0) {
      from[i] = i
      queue[tail++] = i
    }
  for (let head = 0; head < tail; head++) {
    const i = queue[head]
    const x = i % W
    for (const j of [x > 0 ? i - 1 : -1, x < W - 1 ? i + 1 : -1, i - W, i + W])
      if (j >= 0 && j < N && from[j] < 0) {
        from[j] = from[i]
        queue[tail++] = j
      }
  }
  return from
}

// Tile classes by average color, over the atlas.
export const createClassifier = (atlas) => {
  const tileColor = (() => {
    const cache = new Map()
    return (k) => {
      if (cache.has(k)) return cache.get(k)
      const tile = atlas.tiles[k]
      let r = 0,
        g = 0,
        b = 0
      for (let i = 0; i < 256; i++) {
        r += tile[i * 4]
        g += tile[i * 4 + 1]
        b += tile[i * 4 + 2]
      }
      const color = [r / 256, g / 256, b / 256]
      cache.set(k, color)
      return color
    }
  })()
  // Water is a clear, saturated blue; grey-blue rock, snow and ice are land.
  const isWater = (k) => {
    const [r, g, b] = tileColor(k)
    return b > 150 && b - r > 55 && b - g > 28
  }
  const isDark = (k) => {
    const [r, g, b] = tileColor(k)
    return r + g + b < 120
  }
  return { tileColor, isWater, isDark }
}

// The grid of atlas indices, every placed map's metatiles first. `owner` says
// which placed map each cell belongs to (-1: none).
const EMPTY = 0xffff
const paintMaps = (game, world, atlas) => {
  const { all, minX, minY, W, H } = world
  const grid = new Uint16Array(W * H).fill(EMPTY)
  const owner = new Int32Array(W * H).fill(-1)
  all.forEach((p, mapIndex) => {
    const blocks = game.read(p.l.blockdata_filepath)
    const wide = game.wideOf(p.l)
    for (let y = 0; y < p.l.height; y++)
      for (let x = 0; x < p.l.width; x++) {
        const i = (p.y - minY + y) * W + (p.x - minX + x)
        grid[i] = atlas.index(
          p.l.primary_tileset,
          p.l.secondary_tileset,
          blocks.readUInt16LE((y * p.l.width + x) * 2) & 0x3ff,
          wide,
        )
        owner[i] = mapIndex
      }
  })
  return { grid, owner }
}

// Where two linked maps ended up apart (a loop of connections that doesn't add
// up), bridge the gap: each map's edge row is stretched halfway across, so a
// road or a river runs on into the next map. Where the maps are also off
// along the edge, the stretch slides across the gap by the same amount, so
// the road jogs a tile at a time instead of stopping. Bridged cells count as
// the map's own.
const bridgeLinks = (world, grid, owner) => {
  const { all, links, minX, minY, W } = world
  const index = new Map(all.map((p, m) => [p, m]))
  let bridged = 0
  for (const { a, b, want, vertical } of links) {
    const ahead = (vertical ? want[1] : want[0]) > 0
    const [first, second] = ahead ? [a, b] : [b, a]
    const [along, across, size, span, k] = vertical
      ? ["x", "y", "height", "width", 0]
      : ["y", "x", "width", "height", 1]
    const from = first[across] + first.l[size],
      to = second[across]
    if (to <= from) continue
    // How far `second` sits along the edge past where the link puts it.
    const off = b[along] - a[along] - want[k]
    const shift = ahead ? off : -off
    const cell = (u, v) => (vertical ? (v - minY) * W + (u - minX) : (u - minY) * W + (v - minX))
    const lo = Math.min(first[along], second[along]),
      hi = Math.max(first[along] + first.l[span], second[along] + second.l[span])
    for (let c = from; c < to; c++) {
      const f = (c - from + 0.5) / (to - from)
      const near = f < 0.5 ? first : second
      const edge = near === first ? from - 1 : to
      for (let t = lo; t < hi; t++) {
        // The point on the near map's edge this cell continues.
        const u = Math.round(near === first ? t - shift * f : t + shift * (1 - f))
        if (u < near[along] || u >= near[along] + near.l[span]) continue
        const i = cell(t, c)
        if (owner[i] >= 0) continue
        grid[i] = grid[cell(u, edge)]
        owner[i] = index.get(near)
        bridged++
      }
    }
  }
  return bridged
}

// Each placed map's border blocks, and what kind of ground they are.
const readBorders = (game, world, atlas, grid, { isWater, isDark, tileColor }) => {
  const { all } = world
  const { layouts, byName } = game
  // Sinjoh's maps are bordered by a cave's black void; give them the snow past
  // Route 217's edge instead, so Sinjoh joins the land on either side.
  const route217 = layouts[byName.Route217.layout]
  // Ilex Forest is walled in by a rock border in the game, which would grow a
  // field of bare rock round a forest; it takes Azalea Town's trees instead.
  const azalea = layouts[byName.AzaleaTown_hns.layout]
  const borders = all.map((p) => {
    const layout = p.region === "Sinjoh" ? route217 : p.m.name === "IlexForest_hns" ? azalea : p.l
    const bin = game.read(layout.border_filepath)
    // A few layouts declare a bigger border than their file holds; fall back to 2x2.
    const declared = (layout.border_width ?? 2) * (layout.border_height ?? 2)
    const fits = bin.length / 2 >= declared
    const bw = fits ? (layout.border_width ?? 2) : 2,
      bh = fits ? (layout.border_height ?? 2) : 2
    const ids = Array.from({ length: bw * bh }, (_, k) =>
      atlas.index(
        layout.primary_tileset,
        layout.secondary_tileset,
        bin.readUInt16LE(k * 2) & 0x3ff,
        game.wideOf(layout),
      ),
    )
    const water = ids.filter(isWater).length
    const kind = ids.some(isDark) ? "none" : water * 2 > ids.length ? "water" : "land"
    return { bw, bh, ids, kind }
  })
  // The ground each map lends the land around it: its border if that is land.
  // A map bordered by water (an island town) lends the ground its own edge
  // shows most, so the land grown round it is its own, not a far map's.
  const { W } = world
  borders.forEach((b, m) => {
    if (b.kind === "land") {
      b.land = b
      return
    }
    const p = all[m]
    const counts = new Map()
    const x0 = p.x - world.minX,
      y0 = p.y - world.minY
    for (let y = 0; y < p.l.height; y++)
      for (let x = 0; x < p.l.width; x++) {
        if (x > 1 && y > 1 && x < p.l.width - 2 && y < p.l.height - 2) continue
        const k = grid[(y0 + y) * W + x0 + x]
        // Ground, not grey docks, roofs or fences.
        const color = tileColor(k)
        if (isWater(k) || isDark(k) || Math.max(...color) - Math.min(...color) < 40) continue
        counts.set(k, (counts.get(k) ?? 0) + 1)
      }
    const [top] = [...counts].toSorted((a, c) => c[1] - a[1])
    b.land = top && b.kind === "water" ? { bw: 1, bh: 1, ids: [top[0]] } : null
  })
  return borders
}
// A border's block for cell `i`, counted from the corner of the map that lends
// it, as the game does past a map's edge, so the fill carries on the map's own
// rows of trees in step instead of half a tree out wherever a map's corner is odd.
const mod = (n, m) => ((n % m) + m) % m
const pattern = (b, i, W, origin = [0, 0]) => {
  const x = i % W,
    y = (i - x) / W
  return b.ids[mod(y - origin[1], b.bh) * b.bw + mod(x - origin[0], b.bw)]
}

// The land between the maps. Map tiles say where the world is land and where
// it is water; land grows out from the maps' land to a coastline that wanders
// with fractal noise, sea stays sea, and stretches of water that no open sea
// reaches (the insides of route loops) become land too. Returns `land` (1 for
// land) and `toLand`, the distance from each cell to a map's land. (Each filled
// cell then takes the border blocks of the nearest map with that kind of
// border, the way the game fills past a map's edge, tiled on the world grid.)
const growLand = (world, grid, owner, { isWater, tileColor }) => {
  const { all, W, H } = world
  const N = W * H
  // Land grows only from maps that are mostly land; a sea route's rocks and
  // islets don't make a coast, and the water past a sea route stays sea.
  const waterShare = new Float32Array(all.length)
  const cellCount = new Float32Array(all.length)
  for (let i = 0; i < N; i++)
    if (owner[i] >= 0) {
      cellCount[owner[i]]++
      if (isWater(grid[i])) waterShare[owner[i]]++
    }
  for (let m = 0; m < all.length; m++) waterShare[m] /= cellCount[m]
  if (process.env.DEBUG_MAPS)
    for (const name of process.env.DEBUG_MAPS.split(",")) {
      const m = all.findIndex((p) => p.m.name === name)
      const colors = new Map()
      for (let i = 0; i < N; i++)
        if (owner[i] === m && isWater(grid[i])) colors.set(grid[i], (colors.get(grid[i]) ?? 0) + 1)
      const top = [...colors].toSorted((a, b) => b[1] - a[1]).slice(0, 4)
      console.log(
        name,
        waterShare[m].toFixed(2),
        top.map(([k, n]) => `${tileColor(k).map(Math.round)}x${n}`).join(" "),
      )
    }
  const mapLand = new Uint8Array(N)
  const mapSea = new Uint8Array(N)
  for (let i = 0; i < N; i++)
    if (owner[i] >= 0) {
      if (waterShare[owner[i]] > 0.55) mapSea[i] = 1
      else if (!isWater(grid[i])) mapLand[i] = 1
    }
  const toLand = distanceFrom(mapLand, W, H)
  const toSeaRoute = distanceFrom(mapSea, W, H)
  // How far land reaches past the maps: about 18 metatiles, give or take 24.
  const REACH = 18,
    WANDER = 24
  let land = new Uint8Array(N)
  for (let i = 0; i < N; i++) {
    if (owner[i] >= 0) land[i] = owner[i] >= 0 && !isWater(grid[i]) ? 1 : 0
    else {
      const x = i % W,
        y = (i - x) / W
      land[i] = toLand[i] < REACH + WANDER * coastNoise(x, y) && toLand[i] < toSeaRoute[i] ? 1 : 0
    }
  }
  // Water at a map's edge (a river, a lake's shore, a bay) runs on past it a
  // little way, a wandering 2 to 16 metatiles or so, so it doesn't stop dead in a
  // straight line against the fill. Each filled cell looks at the nearest map
  // cell; `spill` remembers which cells became water this way, and from where.
  // It keeps clear of other maps' land, so it doesn't run up against the
  // straight edge of the next map instead.
  const nearestCell = nearestOf(owner, W, H)
  const landOwner = new Int32Array(N).fill(-1)
  for (let i = 0; i < N; i++) if (mapLand[i]) landOwner[i] = owner[i]
  const nearestLand = nearestOf(landOwner, W, H)
  const spill = new Int32Array(N).fill(-1)
  for (let i = 0; i < N; i++) {
    if (owner[i] >= 0) continue
    const from = nearestCell[i]
    if (from < 0 || !isWater(grid[from])) continue
    const shore = nearestLand[i]
    if (shore >= 0 && owner[shore] !== owner[from] && toLand[i] < 5) continue
    const x = i % W,
      y = (i - x) / W
    const fx = from % W,
      fy = (from - fx) / W
    const reach = 8 + 4 * fractal(x + 3100, y - 1700, 1 / 24) + 5 * fractal(x - 800, y + 60, 1 / 7)
    if (Math.hypot(x - fx, y - fy) < reach) {
      land[i] = 0
      spill[i] = from
    }
  }
  // Smooth the filled coast: each filled cell takes the majority of its 5x5
  // neighborhood, twice, which rounds off stray corners and specks.
  for (let pass = 0; pass < 2; pass++) {
    const next = land.slice()
    for (let y = 2; y < H - 2; y++)
      for (let x = 2; x < W - 2; x++) {
        const i = y * W + x
        if (owner[i] >= 0) continue
        let count = 0
        for (let dy = -2; dy <= 2; dy++)
          for (let dx = -2; dx <= 2; dx++) count += land[i + dy * W + dx]
        next[i] = count > 12 ? 1 : 0
      }
    land = next
  }
  // Water that no open sea reaches is enclosed by land: the inside of a route
  // loop, not a lake any map draws. Fill it in.
  {
    const open = new Uint8Array(N)
    const queue = []
    for (let x = 0; x < W; x++) queue.push(x, (H - 1) * W + x)
    for (let y = 0; y < H; y++) queue.push(y * W, y * W + W - 1)
    for (const i of queue) open[i] = land[i] ? 0 : 1
    let head = 0
    const stack = queue.filter((i) => open[i])
    while (head < stack.length) {
      const i = stack[head++]
      const x = i % W,
        y = (i - x) / W
      for (const j of [
        x > 0 ? i - 1 : -1,
        x < W - 1 ? i + 1 : -1,
        y > 0 ? i - W : -1,
        y < H - 1 ? i + W : -1,
      ])
        if (j >= 0 && !open[j] && !land[j]) {
          open[j] = 1
          stack.push(j)
        }
    }
    for (let i = 0; i < N; i++)
      if (!land[i] && !open[i] && owner[i] < 0 && spill[i] < 0) land[i] = 1
  }
  // A map's land never meets the sea at its own edge: a few cells of fill
  // always lie in front, so the coast is grown, not cut along the rectangle.
  for (let i = 0; i < N; i++) {
    if (owner[i] >= 0 || spill[i] >= 0) continue
    const x = i % W,
      y = (i - x) / W
    if (toLand[i] < 3.5 + 1.5 * fractal(x - 700, y + 900, 1 / 10)) land[i] = 1
  }
  for (let i = 0; i < N; i++) if (land[i]) spill[i] = -1
  return { land, toLand, spill }
}

// Sinjoh is the land that joins Johto to Sinnoh: fill every column under it
// from its maps up to Sinnoh's, so a wide strait elsewhere doesn't cut it off.
// The ends of the run narrow with noise, so the land shoulders off.
const bridgeSinjoh = (world, owner, land) => {
  const { all, W } = world
  const sinjoh = all.map((p, m) => (p.region === "Sinjoh" ? m : -1)).filter((m) => m >= 0)
  if (sinjoh.length === 0) return
  const regionOf = (i) => (owner[i] >= 0 ? all[owner[i]].region : null)
  const left = Math.min(...sinjoh.map((m) => all[m].x - world.minX))
  const right = Math.max(...sinjoh.map((m) => all[m].x - world.minX + all[m].l.width))
  const top = Math.min(...sinjoh.map((m) => all[m].y - world.minY))
  // Where Sinnoh's maps end above each column, looking a little to either
  // side too, so the bridge is as wide as Sinjoh even under a narrow route.
  const REACH_UP = 120,
    SIDE = 28
  const sinnohBottom = new Map()
  for (let x = left - SIDE; x < right + SIDE; x++)
    for (let y = top - 1; y > Math.max(0, top - REACH_UP); y--)
      if (regionOf(y * W + x) === "Sinnoh") {
        sinnohBottom.set(x, y)
        break
      }
  for (let x = left; x < right; x++) {
    let y = -1
    for (let dx = -SIDE; dx <= SIDE; dx++) y = Math.max(y, sinnohBottom.get(x + dx) ?? -1)
    if (y < 0) continue
    const edge = Math.min(x - left, right - 1 - x)
    const shoulder = 10 + 6 * fractal(x * 2, y, 1 / 12)
    for (let fy = y + 1; fy < top; fy++) {
      const i = fy * W + x
      if (owner[i] >= 0) continue
      // Near the ends, keep only the middle of the run.
      const fromEnd = Math.min(fy - y, top - fy)
      if (edge < shoulder && fromEnd < shoulder - edge) continue
      land[i] = 1
    }
  }
}

const cutStraits = (world, owner, land) => {
  const { all, W, H } = world
  const N = W * H
  // Regions stay apart: each cell belongs to its nearest region, and where two
  // regions' cells meet, a strait of sea runs between them. Sinjoh is the
  // exception; it is the land that joins Johto to Sinnoh.
  {
    const regionOf = all.map((p) => p.region)
    const nearestRegion = new Int16Array(N).fill(-1)
    const names = [...new Set(regionOf)]
    const queue = []
    for (let i = 0; i < N; i++)
      if (owner[i] >= 0) {
        nearestRegion[i] = names.indexOf(regionOf[owner[i]])
        queue.push(i)
      }
    let head = 0
    while (head < queue.length) {
      const i = queue[head++]
      const x = i % W,
        y = (i - x) / W
      for (const j of [
        x > 0 ? i - 1 : -1,
        x < W - 1 ? i + 1 : -1,
        y > 0 ? i - W : -1,
        y < H - 1 ? i + W : -1,
      ])
        if (j >= 0 && nearestRegion[j] < 0) {
          nearestRegion[j] = nearestRegion[i]
          queue.push(j)
        }
    }
    const sinjoh = names.indexOf("Sinjoh")
    const HALF = 5
    for (let y = HALF; y < H - HALF; y++)
      for (let x = HALF; x < W - HALF; x++) {
        const i = y * W + x
        if (owner[i] >= 0 || !land[i]) continue
        const mine = nearestRegion[i]
        if (mine === sinjoh) continue
        for (const [dx, dy] of [
          [HALF, 0],
          [-HALF, 0],
          [0, HALF],
          [0, -HALF],
        ]) {
          const other = nearestRegion[i + dy * W + dx]
          if (other !== mine && other !== sinjoh) {
            land[i] = 0
            break
          }
        }
      }
  }
}

/** The ferry ports' maps (`PORTS` in living.mjs): their water must reach the open sea. */
const HARBOURS = [
  "OlivineCity_PortOutside_hns",
  "VermilionCity_PortOutside_hns",
  "SlateportCity",
  "LilycoveCity",
  "OneIsland_Frlg",
  "SevenIsland_Frlg",
  "MelemeleIsle_hns",
]
/** A Sevii Island town whose warp leads into a harbour building has a dock to keep open too. */
const HARBOUR_DOOR = /_HARBOR$/
/** A channel's half width: narrow by the dock, wider out toward the sea, in metatiles. */
const CHANNEL = 3,
  CHANNEL_GROWTH = 0.25,
  CHANNEL_MAX = 8
/** How far off any coast the ocean counts as open, in metatiles. */
const OPEN_SEA = 6

// Keeps every harbour on the open sea. Land grown around an island can close
// the bay its dock sits in; then the cheapest way from the dock's water to the
// open ocean, through the land grown between maps (never a map's own cells,
// except under a bridge, which the way may pass beneath), is cut back to
// sea. Water on the way costs little, so the channel takes the narrowest
// land, and it keeps its own width clear of the maps, so it never squeezes
// through a slit between two; it widens toward the sea and its banks wander,
// like a natural inlet.
const openHarbours = (world, owner, land, grid, isWater, under) => {
  const { all, W, H } = world
  const N = W * H
  // How far each cell is from a map, for the room a channel needs.
  const mapped = new Uint8Array(N)
  for (let i = 0; i < N; i++) mapped[i] = owner[i] >= 0 ? 1 : 0
  const toMap = distanceFrom(mapped, W, H)
  // The open ocean: sea outside the maps, joined to the world's corner.
  const ocean = new Uint8Array(N)
  const queue = [0]
  ocean[0] = 1
  for (let head = 0; head < queue.length; head++) {
    const i = queue[head]
    const x = i % W
    for (const j of [x > 0 ? i - 1 : -1, x < W - 1 ? i + 1 : -1, i - W, i + W])
      if (j >= 0 && j < N && !ocean[j] && owner[j] < 0 && !land[j]) {
        ocean[j] = 1
        queue.push(j)
      }
  }
  // A channel ends where the ocean is wide open, a few cells off any coast,
  // so a dock behind a gap a cell or two wide gets a real way out too.
  const shore = new Uint8Array(N)
  for (let i = 0; i < N; i++) shore[i] = ocean[i] ? 0 : 1
  const offShore = distanceFrom(shore, W, H)
  const harbours = all
    .map((p, m) => {
      const door =
        p.region === "Sevii Islands" &&
        (p.m.warp_events ?? []).find((w) => HARBOUR_DOOR.test(w.wayfarer_dest_map ?? w.dest_map))
      if (!door && !HARBOURS.includes(p.m.name)) return null
      return {
        m,
        door: door ? (p.y - world.minY + door.y) * W + (p.x - world.minX + door.x) : undefined,
      }
    })
    .filter(Boolean)
  const neighbours = (i) => {
    const x = i % W,
      y = (i - x) / W
    const out = []
    for (let dy = -1; dy <= 1; dy++)
      for (let dx = -1; dx <= 1; dx++)
        if ((dx || dy) && x + dx >= 0 && x + dx < W && y + dy >= 0 && y + dy < H)
          out.push([i + dy * W + dx, dx && dy ? Math.SQRT2 : 1])
    return out
  }
  let cut = 0
  // The cells dug, which take the nearest map's water plainly: a channel is
  // too narrow for the fill's wandering boundaries, which would blot it.
  const dug = new Uint8Array(N)
  for (const { m, door } of harbours) {
    // The dock's water: the map's water joined to the cells by its harbour
    // door (all the map's water for a port with no door on the map).
    const start = []
    if (door === undefined) {
      for (let i = 0; i < N; i++) if (owner[i] === m && isWater(grid[i])) start.push(i)
    } else {
      const seen = new Set([door])
      const near = [door]
      for (let head = 0; head < near.length && start.length === 0; head++)
        for (const [j] of neighbours(near[head]))
          if (owner[j] === m && !seen.has(j)) {
            seen.add(j)
            ;(isWater(grid[j]) ? start : near).push(j)
          }
      for (let head = 0; head < start.length; head++)
        for (const [j] of neighbours(start[head]))
          if (owner[j] === m && !seen.has(j) && isWater(grid[j])) {
            seen.add(j)
            start.push(j)
          }
    }
    // The cheapest way out to the ocean, outside the maps.
    const cost = new Float64Array(N).fill(Infinity)
    const parent = new Int32Array(N).fill(-1)
    const heap = new MinHeap()
    for (const i of start) {
      cost[i] = 0
      heap.push(i, 0)
    }
    let reached = -1
    while (heap.size) {
      const [i, c] = heap.pop()
      if (c > cost[i]) continue
      if (offShore[i] >= OPEN_SEA) {
        reached = i
        break
      }
      for (const [j, step] of neighbours(i)) {
        // A map's cells are never dug, but its water (a sea route's) and the
        // water under a bridge are sailed through.
        const afloat = !land[j] || under[j]
        if (owner[j] >= 0 && !afloat && !start.includes(j)) continue
        // Land to dig needs the channel's width free of the maps.
        if (!afloat && toMap[j] <= CHANNEL) continue
        const next = c + step * (afloat ? 0.15 : 1)
        if (next < cost[j]) {
          cost[j] = next
          parent[j] = i
          heap.push(j, next)
        }
      }
    }
    if (reached < 0) continue
    const path = []
    for (let i = reached; i >= 0; i = parent[i]) if (owner[i] < 0) path.push(i)
    path.reverse()
    // Nothing to cut where the way out is already open water, wide enough.
    if (!path.some((i) => land[i] || offShore[i] < CHANNEL)) continue
    // Cut the land along the way, wider the further from the dock.
    const channel = new Map()
    path.forEach((i, t) => {
      const x = i % W,
        y = (i - x) / W
      const r =
        Math.min(CHANNEL_MAX, CHANNEL + CHANNEL_GROWTH * t) +
        2.5 * fractal(x + 9100, y - 4700, 1 / 14)
      const R = Math.ceil(r)
      for (let dy = -R; dy <= R; dy++)
        for (let dx = -R; dx <= R; dx++) {
          if (x + dx < 0 || x + dx >= W || y + dy < 0 || y + dy >= H) continue
          const j = (y + dy) * W + (x + dx)
          const d = Math.hypot(dx, dy) - r
          if (d <= 2 && owner[j] < 0) channel.set(j, Math.min(channel.get(j) ?? Infinity, d))
        }
    })
    for (const [j, d] of channel)
      if (d <= 0 && land[j]) {
        land[j] = 0
        dug[j] = 1
        cut++
      }
    // Round its banks off: each cell near the channel takes the majority of
    // its 5x5 neighbourhood, so the cut doesn't leave corners and specks.
    for (let pass = 0; pass < 2; pass++) {
      const next = new Map()
      for (const [j, d] of channel) {
        if (d <= -1) continue
        let count = 0
        for (let dy = -2; dy <= 2; dy++)
          for (let dx = -2; dx <= 2; dx++) count += land[j + dy * W + dx] ?? 0
        next.set(j, count > 12 ? 1 : 0)
      }
      for (const [j, v] of next) land[j] = v
    }
  }
  return { cut, dug }
}

// A binary min-heap of [item, priority].
class MinHeap {
  items = []
  get size() {
    return this.items.length
  }
  push(item, priority) {
    const a = this.items
    a.push([item, priority])
    for (let k = a.length - 1; k > 0;) {
      const up = (k - 1) >> 1
      if (a[up][1] <= a[k][1]) break
      ;[a[up], a[k]] = [a[k], a[up]]
      k = up
    }
  }
  pop() {
    const a = this.items
    const top = a[0]
    const last = a.pop()
    if (a.length) {
      a[0] = last
      for (let k = 0; ;) {
        const l = 2 * k + 1,
          r = l + 1
        let low = k
        if (l < a.length && a[l][1] < a[low][1]) low = l
        if (r < a.length && a[r][1] < a[low][1]) low = r
        if (low === k) break
        ;[a[low], a[k]] = [a[k], a[low]]
        k = low
      }
    }
    return top
  }
}

// Fills every cell outside the maps: land takes the nearest map's border
// blocks, sea the nearest water border or the open ocean's. Returns
// `fillBorder`, the map each land cell's border came from (-1: not filled).
const fillBorders = (
  world,
  { grid, owner, land, spill, dug, borders, isWater, tileColor, tint },
) => {
  const { all, W, H } = world
  const N = W * H
  // Which map each cell takes its border from: the nearest map of the right kind.
  const nearest = (lends) => {
    const from = new Int32Array(N).fill(-1)
    const queue = []
    for (let i = 0; i < N; i++)
      if (owner[i] >= 0 && lends(borders[owner[i]])) {
        from[i] = owner[i]
        queue.push(i)
      }
    let head = 0
    while (head < queue.length) {
      const i = queue[head++]
      const x = i % W,
        y = (i - x) / W
      for (const j of [
        x > 0 ? i - 1 : -1,
        x < W - 1 ? i + 1 : -1,
        y > 0 ? i - W : -1,
        y < H - 1 ? i + W : -1,
      ])
        if (j >= 0 && from[j] < 0) {
          from[j] = from[i]
          queue.push(j)
        }
    }
    return from
  }
  // Land takes the nearest land-bordered map's border; only land with none
  // near (an island town's) takes the ground of the nearest island map.
  const mainland = nearest((b) => b.kind === "land")
  const island = nearest((b) => b.land)
  const landKind = new Uint8Array(N)
  for (let i = 0; i < N; i++)
    landKind[i] = owner[i] >= 0 && borders[owner[i]].kind === "land" ? 1 : 0
  const toMainland = distanceFrom(landKind, W, H)
  const landFrom = new Int32Array(N)
  for (let i = 0; i < N; i++) landFrom[i] = toMainland[i] < 70 ? mainland[i] : island[i]
  const ocean = borders[all.findIndex((p) => p.m.name === "Route105")]
  // The water each map lends the sea beside it: its border if that is water,
  // else, when its water runs off its edge (a sea route's, a river's; not a
  // pond's, which would paint the sea its own pale blue), the water tile it
  // draws most, else none. The sea near the coasts takes the nearest such
  // map's, so a sea route's water runs on in its own tiles.
  const WATER_AT_EDGE = 12
  const waterOf = borders.map((b, m) => {
    if (b.kind === "water") return b
    const { l } = all[m]
    const x0 = all[m].x - world.minX,
      y0 = all[m].y - world.minY
    let atEdge = 0
    for (let y = 0; y < l.height; y++)
      for (let x = 0; x < l.width; x++) {
        if (x > 1 && y > 1 && x < l.width - 2 && y < l.height - 2) continue
        const i = (y0 + y) * W + x0 + x
        if (owner[i] === m && isWater(grid[i])) atEdge++
      }
    if (atEdge < WATER_AT_EDGE) return null
    const counts = new Map()
    for (let i = 0; i < N; i++)
      if (owner[i] === m && isWater(grid[i])) counts.set(grid[i], (counts.get(grid[i]) ?? 0) + 1)
    const [top] = [...counts].toSorted((p, q) => q[1] - p[1])
    return top ? { bw: 1, bh: 1, ids: [top[0]] } : null
  })
  const waterOwner = new Int32Array(N).fill(-1)
  for (let i = 0; i < N; i++)
    if (owner[i] >= 0 && waterOf[owner[i]] && isWater(grid[i])) waterOwner[i] = owner[i]
  const waterNearest = nearestOf(waterOwner, W, H)
  const waterFrom = new Int32Array(N).fill(-1)
  for (let i = 0; i < N; i++) if (waterNearest[i] >= 0) waterFrom[i] = owner[waterNearest[i]]
  // How deep the sea is: shallow by the coasts and the maps (a sea route's own
  // water never darkens, so the sea beside it doesn't either), then deeper in
  // four steps that wander with the noise. Each step tints the nearest map's
  // water a quarter further toward the one deep sea, so every game's water
  // meets the others out at sea, in the same color and the open ocean's tile.
  const shore = new Uint8Array(N)
  for (let i = 0; i < N; i++) shore[i] = land[i] || owner[i] >= 0 ? 1 : 0
  const toShore = distanceFrom(shore, W, H)
  const depthOf = (i) => {
    const x = i % W,
      y = (i - x) / W
    const d = toShore[i] + 5 * fractal(x - 4100, y + 2300, 1 / 36)
    return d < 10 ? 0 : d < 20 ? 1 : d < 31 ? 2 : d < 44 ? 3 : 4
  }
  const waterColor = waterOf.map((b) => {
    if (!b) return null
    const colors = b.ids.map(tileColor)
    return [0, 1, 2].map((n) => colors.reduce((sum, c) => sum + c[n], 0) / colors.length)
  })
  // Look the nearest map up from a point pushed about by noise, so where one
  // map's fill meets another's the boundary wanders instead of running straight.
  const warped = (from, i) => {
    const x = i % W,
      y = (i - x) / W
    const wx = Math.round(x + fractal(x + 400, y, 1 / 40) * 22)
    const wy = Math.round(y + fractal(x, y + 400, 1 / 40) * 22)
    const j = Math.min(H - 1, Math.max(0, wy)) * W + Math.min(W - 1, Math.max(0, wx))
    return from[j] >= 0 ? from[j] : from[i]
  }
  // Each map's top-left corner on the grid, which its border's blocks count from.
  const corner = (m) => [all[m].x - world.minX, all[m].y - world.minY]
  const fillBorder = new Int32Array(N).fill(-1)
  for (let i = 0; i < N; i++) {
    if (owner[i] >= 0) continue
    if (land[i]) {
      fillBorder[i] = warped(landFrom, i)
      grid[i] = pattern(borders[fillBorder[i]].land, i, W, corner(fillBorder[i]))
    } else if (spill[i] >= 0 && waterOf[owner[spill[i]]])
      grid[i] = pattern(waterOf[owner[spill[i]]], i, W, corner(owner[spill[i]]))
    else {
      const depth = depthOf(i)
      // Right by a map, its own water, so the sea doesn't outline its rectangle.
      const near = waterNearest[i]
      const close =
        near >= 0 && Math.hypot((i % W) - (near % W), Math.floor(i / W) - Math.floor(near / W)) < 14
      const from = close || dug[i] ? waterFrom[i] : warped(waterFrom, i)
      if (depth === 4 || from < 0) grid[i] = tint(pattern(ocean, i, W), DEEP_SEA)
      else if (depth === 0) grid[i] = pattern(waterOf[from], i, W, corner(from))
      else
        grid[i] = tint(
          pattern(waterOf[from], i, W, corner(from)),
          stepColor(waterColor[from], depth),
        )
    }
  }
  return {
    fillBorder,
    ocean: { bw: ocean.bw, bh: ocean.bh, ids: ocean.ids.map((k) => tint(k, DEEP_SEA)) },
  }
}

/** The open ocean's color, in the GBA's 5-bit steps; the sea past the grid. */
export const DEEP_SEA = [48, 88, 176]
/** A water color `depth` quarters of the way to the deep sea, in 5-bit steps. */
const stepColor = (color, depth) =>
  color.map((c, n) => Math.round((c + (DEEP_SEA[n] - c) * (depth / 4)) / 8) * 8)

// A water tile tinted to average `color`, as a new metatile in the atlas:
// every pixel scaled per channel, so its ripples stay, in 5-bit colors.
const createTinter = (atlas, tileColor) => {
  const made = new Map()
  return (k, color) => {
    const key = `${k}:${color.join(",")}`
    if (!made.has(key)) {
      const average = tileColor(k)
      const factor = average.map((c, n) => color[n] / Math.max(1, c))
      const source = atlas.tiles[k]
      const tile = new Uint8Array(source.length)
      for (let p = 0; p < 256; p++) {
        for (let n = 0; n < 3; n++)
          tile[p * 4 + n] = Math.min(248, Math.round((source[p * 4 + n] * factor[n]) / 8) * 8)
        tile[p * 4 + 3] = 255
      }
      made.set(key, atlas.tiles.length)
      atlas.tiles.push(tile)
    }
    return made.get(key)
  }
}

// Builds the whole filled world grid. Returns everything the far view needs.
export const buildLand = (game, world, atlas) => {
  const { grid, owner } = paintMaps(game, world, atlas)
  const { all, minX, minY, W, H } = world
  const bridges = placeBridges(
    all.map((p) => ({
      name: p.m.name,
      x: p.x - minX,
      y: p.y - minY,
      width: p.l.width,
      height: p.l.height,
    })),
  )
  const under = maskOf(
    bridges.map((b) => b.pass),
    W,
    H,
  )
  console.log("bridged", bridgeLinks(world, grid, owner), "cells between linked maps")
  const classify = createClassifier(atlas)
  const borders = readBorders(game, world, atlas, grid, classify)
  const { land, toLand, spill } = growLand(world, grid, owner, classify)
  bridgeSinjoh(world, owner, land)
  cutStraits(world, owner, land)
  const harbours = openHarbours(world, owner, land, grid, classify.isWater, under)
  console.log("harbours opened", harbours.cut, "cells")
  const tint = createTinter(atlas, classify.tileColor)
  const { fillBorder, ocean } = fillBorders(world, {
    grid,
    owner,
    land,
    spill,
    dug: harbours.dug,
    borders,
    isWater: classify.isWater,
    tileColor: classify.tileColor,
    tint,
  })
  return { grid, owner, borders, fillBorder, toLand, ocean, bridges, ...classify }
}

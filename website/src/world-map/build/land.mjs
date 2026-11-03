// The land between the maps: fills the world grid with the placed maps'
// metatiles, then grows land out from them with noise, keeps regions apart
// with straits of sea, and fills the rest from the maps' border blocks.

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
    for (let y = 0; y < p.l.height; y++)
      for (let x = 0; x < p.l.width; x++) {
        const i = (p.y - minY + y) * W + (p.x - minX + x)
        grid[i] = atlas.index(
          p.l.primary_tileset,
          p.l.secondary_tileset,
          blocks.readUInt16LE((y * p.l.width + x) * 2) & 0x3ff,
        )
        owner[i] = mapIndex
      }
  })
  return { grid, owner }
}

// Each placed map's border blocks, and what kind of ground they are.
const readBorders = (game, world, atlas, { isWater, isDark }) => {
  const { all } = world
  const { layouts, byName } = game
  // Sinjoh's maps are bordered by a cave's black void; give them the snow past
  // Route 217's edge instead, so Sinjoh joins the land on either side.
  const route217 = layouts[byName.Route217.layout]
  const borders = all.map((p) => {
    const layout = p.region === "Sinjoh" ? route217 : p.l
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
      ),
    )
    const water = ids.filter(isWater).length
    const kind = ids.some(isDark) ? "none" : water * 2 > ids.length ? "water" : "land"
    return { bw, bh, ids, kind }
  })
  return borders
}
const pattern = (b, i, W) => {
  const x = i % W,
    y = (i - x) / W
  return b.ids[(y % b.bh) * b.bw + (x % b.bw)]
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
    for (let i = 0; i < N; i++) if (!land[i] && !open[i] && owner[i] < 0) land[i] = 1
  }
  return { land, toLand }
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

// Fills every cell outside the maps: land takes the nearest map's border
// blocks, sea the nearest water border or the open ocean's. Returns
// `fillBorder`, the map each land cell's border came from (-1: not filled).
const fillBorders = (world, { grid, owner, land, toLand, borders }) => {
  const { all, W, H } = world
  const N = W * H
  // Which map each cell takes its border from: the nearest map of the right kind.
  const nearest = (kind) => {
    const from = new Int32Array(N).fill(-1)
    const queue = []
    for (let i = 0; i < N; i++)
      if (owner[i] >= 0 && borders[owner[i]].kind === kind) {
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
  const landFrom = nearest("land")
  const waterFrom = nearest("water")
  const ocean = borders[all.findIndex((p) => p.m.name === "Route105")]
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
  const fillBorder = new Int32Array(N).fill(-1)
  for (let i = 0; i < N; i++) {
    if (owner[i] >= 0) continue
    if (land[i]) {
      fillBorder[i] = warped(landFrom, i)
      grid[i] = pattern(borders[fillBorder[i]], i, W)
    } else
      grid[i] =
        waterFrom[i] >= 0 && toLand[i] < 40
          ? pattern(borders[waterFrom[i]], i, W)
          : pattern(ocean, i, W)
  }
  return fillBorder
}

// Builds the whole filled world grid. Returns everything the far view needs.
export const buildLand = (game, world, atlas) => {
  const { grid, owner } = paintMaps(game, world, atlas)
  const classify = createClassifier(atlas)
  const borders = readBorders(game, world, atlas, classify)
  const { land, toLand } = growLand(world, grid, owner, classify)
  cutStraits(world, owner, land)
  const fillBorder = fillBorders(world, { grid, owner, land, toLand, borders })
  return { grid, owner, borders, fillBorder, toLand, ...classify }
}

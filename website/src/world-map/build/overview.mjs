// Paints the far-out view of the world.
import { distanceFrom, fractal } from "./land.mjs"

// The far-out view, drawn like a map, from the same grid but freer with it:
// - one sea, light in narrow shallows that vary along the coast, deepening
//   offshore, whatever game the water tiles come from;
// - land specks under a few tiles (a sea route's rocks) drop into the sea, so
//   sea routes don't trace their rectangles in dots;
// - the coastline is blurred and re-cut with noise, so the straight edges and
//   square corners of map rectangles become coast;
// - filled land, and a map's own border blocks, take one world palette per
//   kind of ground (forest, mountain, snow, sand) with relief, so every game's
//   tiles share one style; the rest of a map keeps its tiles' colors.
const SEA_SHALLOW = [104, 164, 224],
  SEA_DEEP = [52, 92, 178]
const BIOMES = {
  forest: [78, 152, 74],
  mountain: [180, 148, 110],
  snow: [230, 236, 244],
  sand: [222, 206, 150],
}
const biomeOf = ([r, g, b]) => {
  const bright = (r + g + b) / 3,
    spread = Math.max(r, g, b) - Math.min(r, g, b)
  if (bright > 180 && spread < 45) return "snow"
  if (g > r + 12 && g >= b) return "forest"
  if (r > 205 && g > 185 && b < 170 && r - b > 50) return "sand"
  return "mountain"
}

// Returns the overview's RGBA pixels, one per grid cell.
export const paintOverview = (world, land) => {
  const { all, W, H } = world
  const N = W * H
  const { grid, owner, borders, fillBorder, toLand, tileColor, isWater } = land
  const borderSets = borders.map((b) => new Set(b.ids))
  const borderColor = (() => {
    const cache = new Map()
    return (m) => {
      if (!cache.has(m)) {
        const colors = borders[m].ids.map(tileColor)
        cache.set(
          m,
          [0, 1, 2].map((c) => colors.reduce((sum, color) => sum + color[c], 0) / colors.length),
        )
      }
      return cache.get(m)
    }
  })()
  const overview = new Uint8Array(N * 4)
  // Land, without the specks.
  let landMask = new Uint8Array(N)
  for (let i = 0; i < N; i++) landMask[i] = isWater(grid[i]) ? 0 : 1
  const MIN_ISLAND = 24
  {
    const seen = new Uint8Array(N)
    const stack = []
    for (let start = 0; start < N; start++) {
      if (!landMask[start] || seen[start]) continue
      const component = [start]
      seen[start] = 1
      stack.push(start)
      while (stack.length > 0) {
        const i = stack.pop()
        const x = i % W,
          y = (i - x) / W
        for (const j of [
          x > 0 ? i - 1 : -1,
          x < W - 1 ? i + 1 : -1,
          y > 0 ? i - W : -1,
          y < H - 1 ? i + W : -1,
        ])
          if (j >= 0 && landMask[j] && !seen[j]) {
            seen[j] = 1
            stack.push(j)
            if (component.length < MIN_ISLAND) component.push(j)
          }
      }
      if (component.length < MIN_ISLAND) for (const i of component) landMask[i] = 0
    }
  }
  // Blur the coast over a 7x7 box (a summed-area table) and cut it again
  // through noise.
  {
    const sums = new Float64Array((W + 1) * (H + 1))
    for (let y = 0; y < H; y++) {
      let row = 0
      for (let x = 0; x < W; x++) {
        row += landMask[y * W + x]
        sums[(y + 1) * (W + 1) + x + 1] = sums[y * (W + 1) + x + 1] + row
      }
    }
    const R = 3
    const cut = new Uint8Array(N)
    for (let y = 0; y < H; y++)
      for (let x = 0; x < W; x++) {
        const x0 = Math.max(0, x - R),
          y0 = Math.max(0, y - R),
          x1 = Math.min(W, x + R + 1),
          y1 = Math.min(H, y + R + 1)
        const sum =
          sums[y1 * (W + 1) + x1] -
          sums[y0 * (W + 1) + x1] -
          sums[y1 * (W + 1) + x0] +
          sums[y0 * (W + 1) + x0]
        const share = sum / ((x1 - x0) * (y1 - y0))
        cut[y * W + x] = share + 0.32 * fractal(x * 3 + 5000, y * 3, 1 / 24) > 0.5 ? 1 : 0
      }
    landMask = cut
  }

  // Each cell's land color before the coast moved: the world palette for
  // filled ground and border blocks, tile colors for the rest; cells the new
  // coast made land borrow the nearest old land cell's color.
  const base = new Float32Array(N * 3).fill(-1)
  const queue = []
  for (let i = 0; i < N; i++) {
    if (isWater(grid[i])) continue
    let color
    const fill =
      fillBorder[i] >= 0
        ? fillBorder[i]
        : owner[i] >= 0 && borderSets[owner[i]].has(grid[i])
          ? owner[i]
          : -1
    if (fill >= 0) {
      const x = i % W,
        y = (i - x) / W
      // Snow only in the cold north; elsewhere pale ground is bare rock.
      const found = biomeOf(borderColor(fill))
      const cold = all[fill].region === "Sinnoh" || all[fill].region === "Sinjoh"
      const biome = found === "snow" && !cold ? "mountain" : found
      color = BIOMES[biome].slice()
      // Relief: denser forest inland, rock where the noise rises, all mottled.
      const inland = Math.min(1, Math.max(0, (toLand[i] - 4) / 24))
      const ridge = Math.max(0, fractal(x + 900, y - 300, 1 / 60) - 0.1) * 1.5
      if (biome === "forest") {
        const deep = inland * 0.35
        color = color.map((c, k) => c + ([40, 104, 50][k] - c) * deep)
        const rock = Math.min(0.5, inland * ridge)
        color = color.map((c, k) => c + (BIOMES.mountain[k] - c) * rock)
      } else if (biome === "mountain") {
        const peak = Math.min(0.45, inland * ridge)
        color = color.map((c, k) => c + ([214, 200, 182][k] - c) * peak)
      }
      const shade = 1 + 0.08 * fractal(x, y, 1 / 18)
      color = color.map((c) => c * shade)
    } else color = tileColor(grid[i])
    base.set(color, i * 3)
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
      if (j >= 0 && base[j * 3] < 0) {
        base[j * 3] = base[i * 3]
        base[j * 3 + 1] = base[i * 3 + 1]
        base[j * 3 + 2] = base[i * 3 + 2]
        queue.push(j)
      }
  }

  const toShore = distanceFrom(landMask, W, H)
  const sea = new Uint8Array(N)
  for (let i = 0; i < N; i++) sea[i] = 1 - landMask[i]
  const toSea = distanceFrom(sea, W, H)
  for (let i = 0; i < N; i++) {
    const x = i % W,
      y = (i - x) / W
    let r, g, b
    if (landMask[i]) {
      r = base[i * 3]
      g = base[i * 3 + 1]
      b = base[i * 3 + 2]
      if (toSea[i] <= 1.5) {
        r *= 0.62
        g *= 0.62
        b *= 0.62
      }
    } else {
      // Shallows 3 to 9 tiles wide along the coast, then the deep.
      const width = 6 + 3 * fractal(x - 2000, y + 700, 1 / 50)
      const shallow = Math.max(0, 1 - (toShore[i] - 1) / width)
      const t = Math.min(1, Math.max(0, (toShore[i] - width * 0.5) / 26)) ** 0.8
      const ripple = 1 + 0.03 * fractal(x, y, 1 / 18)
      r = (SEA_SHALLOW[0] + (SEA_DEEP[0] - SEA_SHALLOW[0]) * t) * ripple
      g = (SEA_SHALLOW[1] + (SEA_DEEP[1] - SEA_SHALLOW[1]) * t) * ripple
      b = (SEA_SHALLOW[2] + (SEA_DEEP[2] - SEA_SHALLOW[2]) * t) * ripple
      r += (205 - r) * shallow * 0.45
      g += (236 - g) * shallow * 0.45
      b += (250 - b) * shallow * 0.45
    }
    overview.set([r, g, b, 255], i * 4)
  }
  return overview
}

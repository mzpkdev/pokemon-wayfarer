// The far-out view of the world: the grid's own metatiles averaged, one pixel
// a metatile, and coarser levels for further out. Each pixel is the average
// of the tiles under it, so the far-out view and the tiles agree on every
// color and every coast, in the GBA's 5-bit colors.

/**
 * The levels, coarsest last: level k has a pixel per 2^k by 2^k metatiles.
 * A coarser pixel is land or sea by the majority of the four under it (land
 * on a tie, so small islands hold on), and its color is the average of those
 * of its kind, so coasts stay crisp instead of blurring into the sea.
 */
export const overviewLevels = ({ grid, W, H, tiles, isWater }, count = 5) => {
  const colorOf = new Map()
  const tileColor = (k) => {
    let color = colorOf.get(k)
    if (!color) {
      const tile = tiles[k]
      color = [0, 0, 0]
      for (let p = 0; p < 256; p++) for (let n = 0; n < 3; n++) color[n] += tile[p * 4 + n] / 256
      colorOf.set(k, color)
    }
    return color
  }
  let width = W,
    height = H
  let color = new Float32Array(W * H * 3)
  let land = new Uint8Array(W * H)
  for (let i = 0; i < W * H; i++) {
    color.set(tileColor(grid[i]), i * 3)
    land[i] = isWater(grid[i]) ? 0 : 1
  }
  const levels = []
  const emit = () => {
    const rgba = new Uint8Array(width * height * 4)
    for (let i = 0; i < width * height; i++) {
      for (let n = 0; n < 3; n++)
        rgba[i * 4 + n] = Math.min(248, Math.round(color[i * 3 + n] / 8) * 8)
      rgba[i * 4 + 3] = 255
    }
    levels.push({ width, height, rgba })
  }
  emit()
  for (let level = 1; level < count; level++) {
    const w = Math.ceil(width / 2),
      h = Math.ceil(height / 2)
    const nextColor = new Float32Array(w * h * 3)
    const nextLand = new Uint8Array(w * h)
    for (let y = 0; y < h; y++)
      for (let x = 0; x < w; x++) {
        const under = []
        for (const [dx, dy] of [
          [0, 0],
          [1, 0],
          [0, 1],
          [1, 1],
        ]) {
          const sx = Math.min(width - 1, x * 2 + dx),
            sy = Math.min(height - 1, y * 2 + dy)
          under.push(sy * width + sx)
        }
        const lands = under.filter((i) => land[i]).length
        const isLand = lands * 2 >= under.length ? 1 : 0
        const kind = under.filter((i) => land[i] === isLand)
        const o = (y * w + x) * 3
        for (const i of kind)
          for (let n = 0; n < 3; n++) nextColor[o + n] += color[i * 3 + n] / kind.length
        nextLand[y * w + x] = isLand
      }
    width = w
    height = h
    color = nextColor
    land = nextLand
    emit()
  }
  return levels
}

/**
 * The levels packed in one image: the finest on the left, the rest stacked
 * down its right. Returns the image and where each level sits in it.
 */
export const packLevels = (levels) => {
  const [first, ...rest] = levels
  const width = first.width + (rest[0]?.width ?? 0)
  const height = first.height
  const rgba = new Uint8Array(width * height * 4)
  const places = []
  let top = 0
  levels.forEach((level, k) => {
    const x = k === 0 ? 0 : first.width,
      y = k === 0 ? 0 : top
    if (k > 0) top += level.height
    for (let row = 0; row < level.height; row++)
      rgba.set(
        level.rgba.subarray(row * level.width * 4, (row + 1) * level.width * 4),
        ((y + row) * width + x) * 4,
      )
    places.push({ x, y, width: level.width, height: level.height })
  })
  return { width, height, rgba, places }
}

// The world's data, as `build/` writes it: a sheet of every metatile used
// (`atlas.png`), a grid of indices into it (`grid.bin.gz`), the far-out view
// one pixel a metatile and coarser (`overview.png`), and `meta.json` with the
// grid's size and where its towns, regions and maps are.

/** A box on the grid, in metatiles. */
export type Bounds = { left: number; top: number; right: number; bottom: number }

export type Meta = {
  width: number
  height: number
  /** Metatiles per row of the atlas. */
  columns: number
  count: number
  /** Where each level of the far-out view sits in `overview.png`; level k has a pixel per 2^k metatiles. */
  overview: { x: number; y: number; width: number; height: number }[]
  /** The open sea past the grid: a pattern of atlas indices, tiled on the grid's cells. */
  ocean: { bw: number; bh: number; ids: number[] }
  /** The open sea's color, as far out as the tiles' average. */
  sea: [number, number, number]
  towns: { name: string; x: number; y: number }[]
  regions: { name: string; x: number; y: number; bounds?: Bounds }[]
  /** Bridge decks the ships pass under (grid cells): no ship is drawn over them. */
  overpasses?: Bounds[]
  maps: {
    name: string
    region?: string
    x: number
    y: number
    width: number
    height: number
    /** The weather the game gives the map, where it has one worth drawing (see `weather.ts`). */
    weather?: Weather
  }[]
}

/** The kinds of weather build/index.mjs keeps: the game's `WEATHER_*` worth drawing on a map. */
export type Weather = "rain" | "storm" | "snow" | "sand" | "shade" | "leaves"

export type World = {
  meta: Meta
  atlas: HTMLImageElement
  /** Atlas index per metatile, row by row; EMPTY where nothing is drawn. */
  grid: Uint16Array
  overview: HTMLImageElement
}

/** A grid cell with no metatile: open sea, drawn by the far-out view's color. */
export const EMPTY = 0xffff

/** Where the site serves the data from. */
export const DATA = "/world-map"

const loadImage = (src: string) =>
  new Promise<HTMLImageElement>((resolve, reject) => {
    const image = new Image()
    image.addEventListener("load", () => resolve(image))
    image.addEventListener("error", reject)
    image.src = src
  })

/**
 * A gzipped file's bytes, unzipped here: a server may or may not have
 * unzipped it on the way already, so look for gzip's magic bytes rather than
 * trust the name.
 */
export const loadBytes = async (src: string) => {
  const bytes = new Uint8Array(await (await fetch(src)).arrayBuffer())
  if (bytes[0] !== 0x1f || bytes[1] !== 0x8b) return bytes
  const stream = new Blob([bytes]).stream().pipeThrough(new DecompressionStream("gzip"))
  return new Uint8Array(await new Response(stream).arrayBuffer())
}

/** The grid: little-endian atlas indices. */
const loadGrid = async (src: string) => {
  const bytes = await loadBytes(src)
  return new Uint16Array(bytes.buffer, bytes.byteOffset, bytes.byteLength / 2)
}

const loading = new Map<string, Promise<World>>()

/** Loads the world once per base path; the tour and the explorer share it. */
export const loadWorld = (base = DATA): Promise<World> => {
  let world = loading.get(base)
  if (!world) {
    world = (async () => {
      const [meta, atlas, grid, overview] = await Promise.all([
        fetch(`${base}/meta.json`).then((response) => response.json() as Promise<Meta>),
        loadImage(`${base}/atlas.png`),
        loadGrid(`${base}/grid.bin.gz`),
        loadImage(`${base}/overview.png`),
      ])
      return { meta, atlas, grid, overview }
    })()
    // A failed load can be tried again.
    world.catch(() => loading.delete(base))
    loading.set(base, world)
  }
  return world
}

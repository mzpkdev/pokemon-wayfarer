// Which cells of the world are water, a bit each, as build/index.mjs writes
// water.bin.gz. The living sea keeps its glints and ripples to them.
import { DATA, loadBytes, type Meta } from "./data"

export type Water = { width: number; height: number; bits: Uint8Array }

/** Whether a grid cell is water; past the grid's edges, the open sea always is. */
export const isWaterAt = (water: Water, x: number, y: number) => {
  if (x < 0 || y < 0 || x >= water.width || y >= water.height) return true
  const i = y * water.width + x
  return (water.bits[i >> 3]! & (0x80 >> (i & 7))) !== 0
}

const loading = new Map<string, Promise<Water>>()

/** Loads the water mask once per base path. */
export const loadWater = (meta: Meta, base = DATA): Promise<Water> => {
  let water = loading.get(base)
  if (!water) {
    water = loadBytes(`${base}/water.bin.gz`).then((bits) => ({
      width: meta.width,
      height: meta.height,
      bits,
    }))
    water.catch(() => loading.delete(base))
    loading.set(base, water)
  }
  return water
}

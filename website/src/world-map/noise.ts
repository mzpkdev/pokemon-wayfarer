// Hashes and noise the map's effects share: every frame agrees on where a
// glint, a raindrop or a cloud is, because each is a function of time and a
// few whole numbers, never stored.

export const hash = (n: number) => {
  let h = Math.imul(n ^ 0x9e3779b9, 0x85ebca6b)
  h ^= h >>> 13
  h = Math.imul(h, 0xc2b2ae35)
  return (h ^ (h >>> 16)) >>> 0
}

const mix = (h: number, part: number) => hash(h ^ Math.imul(part + 1, 0x27d4eb2d))

/** A hash of up to four whole numbers, as a fraction in [0, 1). Fixed arity: it runs per cell. */
export const roll = (a: number, b: number, c = 0, d = 0) =>
  mix(mix(mix(mix(0x2545f491, a), b), c), d) / 0x100000000

/** 0 to 1 over `t` in [0, 1], easing in and out. */
export const smooth = (t: number) => {
  const c = Math.min(1, Math.max(0, t))
  return c * c * (3 - 2 * c)
}

/** 0 below `from`, 1 past `to`, smooth between. */
export const ramp = (value: number, from: number, to: number) =>
  smooth((value - from) / (to - from))

/**
 * Value noise that tiles every `size` cells: a lattice of hashed values,
 * smoothly blended, summed over `octaves` doublings. In [0, 1].
 */
export const tiledNoise = (size: number, octaves: number, seed: number) => {
  return (x: number, y: number) => {
    let sum = 0
    let weight = 0
    let amplitude = 1
    let frequency = 1
    for (let octave = 0; octave < octaves; octave++) {
      const fx = x * frequency
      const fy = y * frequency
      const x0 = Math.floor(fx)
      const y0 = Math.floor(fy)
      const tx = smooth(fx - x0)
      const ty = smooth(fy - y0)
      const cell = size * frequency
      const at = (gx: number, gy: number) =>
        roll(((gx % cell) + cell) % cell, ((gy % cell) + cell) % cell, octave, seed)
      const top = at(x0, y0) + (at(x0 + 1, y0) - at(x0, y0)) * tx
      const bottom = at(x0, y0 + 1) + (at(x0 + 1, y0 + 1) - at(x0, y0 + 1)) * tx
      sum += (top + (bottom - top) * ty) * amplitude
      weight += amplitude
      amplitude /= 2
      frequency *= 2
    }
    return sum / weight
  }
}

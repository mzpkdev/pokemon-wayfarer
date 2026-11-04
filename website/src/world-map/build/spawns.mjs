// Where wild Pokémon can turn up on each placed map, for the explorer's
// sprites: the cells whose metatile behavior has wild encounters
// (`sTileBitAttributes` in src/metatile_behavior.c), on land (tall grass,
// long grass, sand) or on the water. A map keeps a spread-out sample of each.

/** The most cells kept per map and kind. */
const SAMPLE = 32

const TILE_FLAG_HAS_ENCOUNTERS = 1
export const TILE_FLAG_SURFABLE = 2

/** Behavior value -> its tile flags, from the behavior enum and the flags table. */
export const readBehaviorFlags = (game) => {
  const header = game.read("include/constants/metatile_behaviors.h").toString()
  const values = new Map()
  let next = 0
  for (const [, name, value] of header.matchAll(/^\s*(MB_\w+)\s*(?:=\s*(\w+))?\s*,/gm)) {
    const at = value === undefined ? next : Number(value)
    values.set(name, at)
    next = at + 1
  }
  const source = game.read("src/metatile_behavior.c").toString()
  const table = source.slice(source.indexOf("sTileBitAttributes"), source.indexOf("};"))
  const flags = new Map()
  for (const [, name, bits] of table.matchAll(/\[(MB_\w+)\]\s*=\s*([^,\n]+)/g)) {
    if (!values.has(name)) continue
    flags.set(
      values.get(name),
      (bits.includes("TILE_FLAG_HAS_ENCOUNTERS") ? TILE_FLAG_HAS_ENCOUNTERS : 0) |
        (bits.includes("TILE_FLAG_SURFABLE") ? TILE_FLAG_SURFABLE : 0),
    )
  }
  if (!flags.size) throw new Error("no metatile behavior flags read")
  return flags
}

/** Every `step`-th of `cells` (flat x, y pairs), at most `SAMPLE` of them. */
const sample = (cells) => {
  const count = cells.length / 2
  if (count <= SAMPLE) return cells
  const out = []
  for (let i = 0; i < SAMPLE; i++) {
    const k = Math.floor(((i + 0.5) * count) / SAMPLE)
    out.push(cells[k * 2], cells[k * 2 + 1])
  }
  return out
}

/** A metatile id's behavior, for a layout's pair of tilesets. */
export const readBehavior = (game, primaryName, secondaryName) => {
  const primary = game.tileset(primaryName)
  const secondary = game.tileset(secondaryName)
  return (id) => {
    const source = id < primary.count ? primary : secondary
    const local = id < primary.count ? id : id - primary.count
    const { attributes } = source
    if (!attributes) return 0
    // FRLG's are u32 with a 9-bit behavior, Emerald's u16 with 8 bits.
    return attributes.length / source.count >= 4
      ? attributes.readUInt32LE(local * 4) & 0x1ff
      : attributes.readUInt16LE(local * 2) & 0xff
  }
}

/**
 * Per placed map (by folder name), its land and water encounter cells in grid
 * coordinates, as flat x, y pairs: `{ l, w }`, each missing when empty.
 */
export const readSpawns = (game, { all, minX, minY }) => {
  const flagsOf = readBehaviorFlags(game)
  const spawns = {}
  for (const p of all) {
    const behavior = readBehavior(game, p.l.primary_tileset, p.l.secondary_tileset)
    const blocks = game.read(p.l.blockdata_filepath)
    const land = []
    const water = []
    for (let y = 0; y < p.l.height; y++)
      for (let x = 0; x < p.l.width; x++) {
        const flags = flagsOf.get(behavior(blocks.readUInt16LE((y * p.l.width + x) * 2) & 0x3ff))
        if (!flags || !(flags & TILE_FLAG_HAS_ENCOUNTERS)) continue
        ;(flags & TILE_FLAG_SURFABLE ? water : land).push(p.x - minX + x, p.y - minY + y)
      }
    const entry = {}
    if (land.length) entry.l = sample(land)
    if (water.length) entry.w = sample(water)
    if (land.length || water.length) spawns[p.m.name] = entry
  }
  return spawns
}

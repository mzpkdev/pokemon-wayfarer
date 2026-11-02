#!/usr/bin/env node
// Renders the website's art from the game's own graphics in ../game/graphics:
// the opening's parallax layers and Poké Ball, the title screen's logo,
// wordmark and passing sprites, the default text window frame, the dawn
// scene, key item icons, and a web font traced from the game's normal Latin font.
//
// The output is committed under public/game/, so building the site does not
// need the game tree. Rerun with `pnpm run assets` after the source art changes.
import * as fs from "node:fs"
import * as path from "node:path"
import { fileURLToPath } from "node:url"

import { Font, Glyph, Path } from "opentype.js"

import { readJascPalette, readTiles, renderIndexed, renderTilemap } from "./lib/gba.mjs"
import { readPng, writePng } from "./lib/png.mjs"

const here = path.dirname(fileURLToPath(import.meta.url))
const game = path.resolve(here, "../../game")
const graphics = path.join(game, "graphics")
const out = path.resolve(here, "../public/game")

const source = (...parts) => path.join(graphics, ...parts)
const manifest = {}
const save = (name, image) => {
  manifest[name] = { width: image.width, height: image.height, x: image.x ?? 0, y: image.y ?? 0 }
  const file = path.join(out, name)
  fs.mkdirSync(path.dirname(file), { recursive: true })
  writePng(file, image.width, image.height, image.rgba)
  console.log(`  ${path.relative(process.cwd(), file)} ${image.width}x${image.height}`)
}

/**
 * The opaque box of a front picture's first frame, with its lowest row taken
 * over both frames so neither one sinks below the floor.
 */
const opaqueBounds = (image, frameHeight = 64) => {
  let left = Infinity
  let right = -Infinity
  let top = Infinity
  let bottom = -Infinity
  for (let y = 0; y < image.height; y++) {
    for (let x = 0; x < image.width; x++) {
      if (image.rgba[(y * image.width + x) * 4 + 3] === 0) continue
      const frameY = y % frameHeight
      bottom = Math.max(bottom, frameY)
      if (y >= frameHeight) continue
      left = Math.min(left, x)
      right = Math.max(right, x)
      top = Math.min(top, frameY)
    }
  }
  return { left, right, top, bottom }
}

/** Copies a width x height block of an RGBA image. */
const crop = (image, x, y, width, height) => {
  const rgba = new Uint8Array(width * height * 4)
  for (let row = 0; row < height; row++) {
    const start = ((y + row) * image.width + x) * 4
    rgba.set(image.rgba.subarray(start, start + width * 4), row * width * 4)
  }
  return { width, height, rgba }
}

/** Lays images side by side, top-aligned. */
const sideBySide = (images) => {
  const width = images.reduce((sum, image) => sum + image.width, 0)
  const height = Math.max(...images.map((image) => image.height))
  const rgba = new Uint8Array(width * height * 4)
  let x = 0
  for (const image of images) {
    for (let y = 0; y < image.height; y++)
      rgba.set(
        image.rgba.subarray(y * image.width * 4, (y + 1) * image.width * 4),
        (y * width + x) * 4,
      )
    x += image.width
  }
  return { width, height, rgba }
}

/** Cuts an image into 8x8 tiles in VRAM order, the way 1D sprite tiles are numbered. */
const spriteTiles = (image) => {
  const tiles = []
  for (let y = 0; y < image.height; y += 8)
    for (let x = 0; x < image.width; x += 8) tiles.push(crop(image, x, y, 8, 8))
  return tiles
}

/** Trims fully transparent edges, keeping where the trimmed image sat. */
const trim = (image) => {
  let [left, top, right, bottom] = [image.width, image.height, -1, -1]
  for (let y = 0; y < image.height; y++)
    for (let x = 0; x < image.width; x++)
      if (image.rgba[(y * image.width + x) * 4 + 3]) {
        left = Math.min(left, x)
        right = Math.max(right, x)
        top = Math.min(top, y)
        bottom = Math.max(bottom, y)
      }
  return { ...crop(image, left, top, right - left + 1, bottom - top + 1), x: left, y: top }
}

const tilemap = (tiles, mapFile, bpp, palette) =>
  renderTilemap({ ...tiles, palette: palette ?? tiles.palette, map: fs.readFileSync(mapFile), bpp })

fs.rmSync(out, { recursive: true, force: true })
console.log("Rendering game art")

/** Drops trailing rows filled with the bottom row's color: map filler no camera position shows. */
const trimFlatBottom = (image) => {
  const pixel = (x, y) =>
    image.rgba.subarray((y * image.width + x) * 4, (y * image.width + x) * 4 + 4).join()
  const filler = pixel(0, image.height - 1)
  const rowIsFlat = (y) => {
    for (let x = 0; x < image.width; x++) if (pixel(x, y) !== filler) return false
    return true
  }
  let height = image.height
  while (height > 1 && rowIsFlat(height - 1)) height--
  return crop(image, 0, 0, image.width, height)
}

// Opening scene 1: four 4bpp parallax layers sharing one tile sheet. Only the
// 240 columns a GBA screen shows are drawn; the 16 past them are empty.
const introTiles = readTiles(source("intro/scene_1/bg.png"))
for (const layer of [0, 1, 2, 3]) {
  const image = crop(
    tilemap(introTiles, source(`intro/scene_1/bg${layer}_map.bin`), 4),
    0,
    0,
    240,
    256,
  )
  save(`intro/bg${layer}.png`, layer === 3 ? trimFlatBottom(image) : image)
}
/** An image followed by its mirror image, so it tiles without a seam. */
const mirrorPair = (image) => {
  const pair = {
    width: image.width * 2,
    height: image.height,
    rgba: new Uint8Array(image.width * 2 * image.height * 4),
  }
  for (let y = 0; y < image.height; y++)
    for (let x = 0; x < image.width; x++) {
      const pixel = image.rgba.subarray((y * image.width + x) * 4, (y * image.width + x) * 4 + 4)
      pair.rgba.set(pixel, (y * pair.width + x) * 4)
      pair.rgba.set(pixel, (y * pair.width + pair.width - 1 - x) * 4)
    }
  return pair
}

/** Draws `top` over `bottom` at (x, y), skipping transparent pixels. */
const overlay = (bottom, top, x, y) => {
  for (let row = 0; row < top.height; row++)
    for (let column = 0; column < top.width; column++) {
      const from = (row * top.width + column) * 4
      if (!top.rgba[from + 3]) continue
      bottom.rgba.set(
        top.rgba.subarray(from, from + 4),
        ((y + row) * bottom.width + x + column) * 4,
      )
    }
  return bottom
}

const layer = (n) =>
  crop(tilemap(introTiles, source(`intro/scene_1/bg${n}_map.bin`), 4), 0, 0, 240, 256)

// The page's top edge, which rises over the opening like a nearer layer: BG1's
// leafy plants with BG0's grass silhouette in front, whose solid part is the
// ground color the page continues in.
const edge = { width: 240, height: 112, rgba: new Uint8Array(240 * 112 * 4) }
overlay(edge, crop(layer(1), 0, 0, 240, 100), 0, 0)
overlay(edge, crop(layer(0), 0, 176, 240, 39), 0, 73)
save("page/edge.png", mirrorPair(edge))

// The bright meadow from the title screen, for the foot of the page.
save("page/meadow.png", mirrorPair(crop(layer(2), 0, 16, 240, 64)))

save("intro/pokeball.png", renderIndexed(source("intro/scene_1/wayfarer_pokeball.png")))

// The dawn artwork the Wayfarer title screen loads behind the mountain pan.
// Only its GBA screen's 240x160 is drawn.
save(
  "title/dawn.png",
  crop(
    tilemap(
      readTiles(source("title_screen/wayfarer/background/scene.png")),
      source("title_screen/wayfarer/background/scene.bin"),
      8,
      readJascPalette(source("title_screen/wayfarer/background/shared.pal")),
    ),
    0,
    0,
    240,
    160,
  ),
)

// The Pokémon logo is four 64x64 sprites stacked in its sheet, shown side by side.
const logo = renderIndexed(source("title_screen/wayfarer/overlays/pokemon_logo_obj.png"))
save("title/logo.png", trim(sideBySide([0, 1, 2, 3].map((i) => crop(logo, 0, i * 64, 64, 64)))))

// The wordmark's two 64x32 halves are stacked too.
const wordmark = renderIndexed(source("title_screen/wayfarer/wayfarer_version.png"))
save(
  "title/wordmark.png",
  trim(sideBySide([crop(wordmark, 0, 0, 64, 32), crop(wordmark, 0, 32, 64, 32)])),
)

// PRESS START is five 32x8 sprites starting at tile 1.
const pressStart = spriteTiles(renderIndexed(source("title_screen/press_start.png")))
save("title/press-start.png", trim(sideBySide(pressStart.slice(1, 21))))

// Passing characters, kept as vertical frame strips.
const playerPalette = readJascPalette(source("intro/scene_2/player.pal"))
for (const name of ["volbeat", "torchic", "manectric"])
  save(`sprites/${name}.png`, renderIndexed(source(`intro/scene_2/${name}.png`)))
save("sprites/brendan.png", renderIndexed(source("intro/scene_2/brendan.png"), playerPalette))
save("sprites/bicycle.png", renderIndexed(source("intro/scene_2/bicycle.png"), playerPalette))

save("text/down-arrow.png", renderIndexed(source("fonts/down_arrow.png")))

// Text window frame 1, the game's default.
save("frames/1.png", renderIndexed(source("text_window/1.png")))

// PROF. OAK from the FireRed and LeafGreen intro, an 8bpp picture whose
// palette the game loads at BG palette 6, so pixel n takes color n - 96.
const oakPalette = Array.from({ length: 256 }, () => [0, 0, 0, 0])
readJascPalette(source("oak_speech/oak/pal.pal")).forEach(
  (color, i) => (oakPalette[96 + i] = color),
)
const oak = renderIndexed(source("oak_speech/oak/pic.png"), oakPalette)
save("people/oak.png", oak)
manifest["people/oak.png"].art = opaqueBounds(oak, oak.height)

// The grey oval PROF. OAK stands on in Wayfarer's new game intro. At runtime
// the intro loads its gradient palette one slot up over BG palette 0, so slot
// n shows gradient color n - 1, at the GBA's 5-bit color depth.
const stagePalette = [
  ...readJascPalette(source("oak_speech_hns/bg0_hns.pal")),
  ...readJascPalette(source("oak_speech_hns/bg1_hns.pal")),
]
const gradient = readJascPalette(source("oak_speech_hns/bg2_hns.pal"))
for (let slot = 1; slot <= 8; slot++)
  stagePalette[slot] = [...gradient[slot - 1].slice(0, 3).map((c) => c & 0xf8), 255]
const stage = tilemap(
  readTiles(source("oak_speech_hns/shadow_hns.png")),
  source("oak_speech_hns/map_hns.bin"),
  4,
  stagePalette,
)
// The intro draws it on black; drop the black so it sits on the page instead.
const oval = crop(stage, 56, 72, 128, 32)
for (let i = 0; i < oval.rgba.length; i += 4)
  if (oval.rgba[i] === 0 && oval.rgba[i + 1] === 0 && oval.rgba[i + 2] === 0) oval.rgba[i + 3] = 0
save("people/stage.png", oval)

// The Poké Ball that releases it (closed, opening, open) and the particles
// that burst from it, colored like the circle impact the game lends them.
save("people/poke-ball.png", renderIndexed(source("balls/poke.png")))
save(
  "people/ball-particles.png",
  renderIndexed(
    source("battle_anims/sprites/particles.png"),
    readPng(source("battle_anims/sprites/circle_impact.png")).palette,
  ),
)

// The POKéMON OAK sends out in turn: WOOPER from his own intro, then the
// opening's cast. Two front frames stacked, in the GBA style the game builds
// with (P_GBA_STYLE_SPECIES_GFX).
// Each records where its art sits in the 64x64 frame, so the site can scale
// it to size and stand its feet on the floor.
for (const species of ["wooper", "torchic", "manectric", "volbeat", "flygon"]) {
  const name = `people/mons/${species}.png`
  const image = renderIndexed(
    source(`pokemon/${species}/anim_front_gba.png`),
    readJascPalette(source(`pokemon/${species}/normal_gba.pal`)),
  )
  save(name, image)
  manifest[name].art = opaqueBounds(image)
}

// The region maps the POKéNAV and Town Map draw, for the world section. Each
// is an 8bpp affine background, one byte per tile on a 64x64 tile grid. Its
// sea is a two-color stripe; drop it so every region can sit on one shared
// ocean, and keep the stripe's colors to paint that ocean with.
const regionMap = (name, { width = 512, from = source("pokenav/region_map") } = {}) => {
  const sheet = readPng(path.join(from, `${name}.png`))
  const map = fs.readFileSync(path.join(from, `${name}.bin`))
  const palette = sheet.palette
  const tilesPerRow = sheet.width / 8
  const counts = new Map()
  for (const tile of map) counts.set(tile, (counts.get(tile) ?? 0) + 1)
  const seaTile = [...counts].toSorted((a, b) => b[1] - a[1])[0][0]
  const seaIndex = (row) =>
    sheet.samples[
      (Math.floor(seaTile / tilesPerRow) * 8 + row) * sheet.width + (seaTile % tilesPerRow) * 8
    ]
  const sea = [seaIndex(0), seaIndex(1)]
  const seaColors = sea.map((index) => palette[index].slice(0, 3))
  const isSea = (index) =>
    seaColors.some((color) => color.every((channel, i) => channel === palette[index][i]))

  const image = { width, height: 512, rgba: new Uint8Array(width * 512 * 4) }
  map.forEach((tile, i) => {
    const originX = (i % 64) * 8
    const originY = Math.floor(i / 64) * 8
    if (originX >= width) return
    const tileX = (tile % tilesPerRow) * 8
    const tileY = Math.floor(tile / tilesPerRow) * 8
    for (let y = 0; y < 8; y++)
      for (let x = 0; x < 8; x++) {
        const index = sheet.samples[(tileY + y) * sheet.width + tileX + x]
        if (index === 0 || isSea(index)) continue
        image.rgba.set(
          [...palette[index].slice(0, 3), 255],
          ((originY + y) * width + originX + x) * 4,
        )
      }
  })
  return { image: trim(image), seaColors }
}

// The Sevii maps end in the Town Map's switch-map button at column 22; leave it off.
for (const [name, file, width] of [
  ["hoenn", "map"],
  ["johto-kanto", "map_jk"],
  ["sevii-123", "map_sevii_123", 168],
  ["sevii-45", "map_sevii_45", 168],
  ["sevii-67", "map_sevii_67", 168],
]) {
  const { image, seaColors } = regionMap(file, { width })
  save(`world/${name}.png`, image)
  manifest[`world/${name}.png`].sea = seaColors
}

// Sinnoh has maps in the game but no Town Map art; its region map comes from
// the project the maps were ported from (see scripts/source/sinnoh).
const sinnoh = regionMap("map", { from: path.join(here, "source/sinnoh") }).image
save("world/sinnoh.png", sinnoh)

// Alola and Sinjoh have no region map anywhere, so they are painted in the
// region maps' own colors, sampled from the Johto and Kanto map: a dark
// outline at the coast, then lighter greens inland.
const LAND = [
  [0, 115, 0],
  [0, 115, 0],
  [57, 172, 8],
  [57, 172, 8],
  [98, 213, 0],
  [98, 213, 0],
  [98, 213, 0],
  [172, 238, 49],
]
const SNOW = [
  [0, 115, 0],
  [96, 120, 152],
  [168, 184, 208],
  [168, 184, 208],
  [216, 224, 240],
  [216, 224, 240],
  [248, 248, 248],
]
const ROUTE = [230, 164, 0]

/**
 * Paints a land mask (1 = land) by each pixel's distance to the nearest sea.
 * `beyond` names the edges where the land carries on past the image.
 */
const paintLand = (mask, width, height, { snow = () => false, beyond = [] } = {}) => {
  const image = { width, height, rgba: new Uint8Array(width * height * 4) }
  const land = (x, y) => {
    if (x < 0) return beyond.includes("left")
    if (y >= height) return beyond.includes("bottom")
    return y >= 0 && x < width && mask[y * width + x] === 1
  }
  for (let y = 0; y < height; y++)
    for (let x = 0; x < width; x++) {
      if (!land(x, y)) continue
      let distance = 1
      search: for (; distance < 8; distance++)
        for (let dy = -distance; dy <= distance; dy++)
          for (let dx = -distance; dx <= distance; dx++) if (!land(x + dx, y + dy)) break search
      const palette = snow(x, y) ? SNOW : LAND
      image.rgba.set([...palette[Math.min(distance, palette.length) - 1], 255], (y * width + x) * 4)
    }
  return image
}

/** The town marker, lifted from the Hoenn map (OLDALE TOWN). */
const hoennArt = regionMap("map").image
const marker = (cellX, cellY) => {
  const tile = crop(hoennArt, (cellX + 1) * 8, (cellY + 2) * 8, 8, 8)
  // Keep only the marker itself: drop the land and route colors around it.
  const ground = new Set([...LAND, ROUTE, [238, 189, 57], [41, 131, 230], [74, 156, 230]].map(String))
  for (let i = 0; i < tile.rgba.length; i += 4)
    if (ground.has(String([...tile.rgba.subarray(i, i + 3)]))) tile.rgba[i + 3] = 0
  return tile
}
const townMarker = marker(4, 9)
const stamp = (image, sprite, x, y) => {
  for (let row = 0; row < sprite.height; row++)
    for (let column = 0; column < sprite.width; column++) {
      const from = (row * sprite.width + column) * 4
      if (!sprite.rgba[from + 3]) continue
      const to = ((y + row) * image.width + x + column) * 4
      if (to >= 0 && to < image.rgba.length) image.rgba.set(sprite.rgba.subarray(from, from + 4), to)
    }
}

/**
 * A region's outdoor maps laid out by their connections, as a land mask at
 * one pixel per `scale` metatiles: water is a metatile at elevation 1 that
 * can be crossed; anything else, walkable or not, is land.
 */
const mapsById = new Map()
for (const dir of fs.readdirSync(path.join(game, "data/maps"))) {
  const file = path.join(game, "data/maps", dir, "map.json")
  if (fs.existsSync(file)) {
    const map = JSON.parse(fs.readFileSync(file, "utf8"))
    mapsById.set(map.id, map)
  }
}
const layoutsById = new Map(
  JSON.parse(fs.readFileSync(path.join(game, "data/layouts/layouts.json"), "utf8"))
    .layouts.filter((layout) => layout.id)
    .map((layout) => [layout.id, layout]),
)
const overworldMask = (root, scale) => {
  const placed = new Map()
  const queue = [[root, 0, 0]]
  while (queue.length > 0) {
    const [id, x, y] = queue.shift()
    if (placed.has(id)) continue
    const map = mapsById.get(id)
    const layout = layoutsById.get(map.layout)
    placed.set(id, { x, y, map, layout })
    for (const { map: next, offset, direction } of map.connections ?? []) {
      const nextLayout = layoutsById.get(mapsById.get(next)?.layout)
      if (!nextLayout || placed.has(next)) continue
      const at = {
        up: [x + offset, y - nextLayout.height],
        down: [x + offset, y + layout.height],
        left: [x - nextLayout.width, y + offset],
        right: [x + layout.width, y + offset],
      }[direction]
      if (at) queue.push([next, ...at])
    }
  }
  const all = [...placed.values()]
  const left = Math.min(...all.map(({ x }) => x))
  const top = Math.min(...all.map(({ y }) => y))
  const width = Math.ceil((Math.max(...all.map(({ x, layout }) => x + layout.width)) - left) / scale)
  const height = Math.ceil((Math.max(...all.map(({ y, layout }) => y + layout.height)) - top) / scale)
  const landCount = new Float32Array(width * height)
  for (const { x, y, layout } of all) {
    const blocks = fs.readFileSync(path.join(game, layout.blockdata_filepath))
    for (let row = 0; row < layout.height; row++)
      for (let column = 0; column < layout.width; column++) {
        const block = blocks.readUInt16LE((row * layout.width + column) * 2)
        const water = block >> 12 === 1 && ((block >> 10) & 3) === 0
        if (water) continue
        const cell =
          Math.floor((y - top + row) / scale) * width + Math.floor((x - left + column) / scale)
        landCount[cell] += 1 / (scale * scale)
      }
  }
  const towns = all
    .filter(({ map }) => map.map_type === "MAP_TYPE_TOWN" || map.map_type === "MAP_TYPE_CITY")
    .map(({ x, y, layout }) => ({
      x: Math.round((x - left + layout.width / 2) / scale),
      y: Math.round((y - top + layout.height / 2) / scale),
    }))
  return { mask: landCount.map((share) => (share >= 0.5 ? 1 : 0)), width, height, towns, placed: all }
}

// Alola's four islands from Melemele, Akala, Ula'ula and Poni, at about the
// region maps' own scale, each marked like a town.
const ALOLA_SCALE = 2
const alola = overworldMask("MAP_MELEMELE_ISLE_HNS", ALOLA_SCALE)
// The island maps meet edge to edge, framed in trees, so their union is one
// block. Round each island inside its own map instead, leaving sea between.
const alolaLeft = Math.min(...alola.placed.map(({ x }) => x))
const alolaTop = Math.min(...alola.placed.map(({ y }) => y))
const islands = alola.placed
  .filter(({ map }) => map.id !== "MAP_ALOLA_WATER_HNS")
  .map(({ x, y, layout, map }) => ({
    id: map.id,
    x: (x - alolaLeft + layout.width / 2) / ALOLA_SCALE,
    y: (y - alolaTop + layout.height / 2) / ALOLA_SCALE,
    radiusX: (layout.width / ALOLA_SCALE / 2) * 0.8,
    radiusY: (layout.height / ALOLA_SCALE / 2) * 0.8,
  }))
const islandMask = alola.mask.map((land, i) => {
  const x = (i % alola.width) + 0.5
  const y = Math.floor(i / alola.width) + 0.5
  return islands.some(({ x: cx, y: cy, radiusX, radiusY }) => {
    const angle = Math.atan2(y - cy, x - cx)
    const wobble = 1 + Math.sin(angle * 3 + cx) * 0.12 + Math.sin(angle * 5 + cy) * 0.06
    return ((x - cx) / radiusX) ** 2 + ((y - cy) / radiusY) ** 2 <= wobble ** 2
  })
    ? land
    : 0
})
const alolaArt = paintLand(islandMask, alola.width, alola.height)
const islandCenters = islands.map(({ id, x, y }) => ({ id, x: Math.round(x), y: Math.round(y) }))
for (const { x, y } of islandCenters) stamp(alolaArt, townMarker, x - 4, y - 4)
save("world/alola.png", alolaArt)
manifest["world/alola.png"].islands = islandCenters

// Johto's map is cut off along its top edge. Close it with a northern coast,
// snowed over past MT. SILVER (cell 14, 7) where Sinjoh lies, with NEW SINJOH
// and the SINJOH RUINS on Routes 49 and 50.
const johtoKanto = regionMap("map_jk").image
const NORTH = 40
const coast = { width: johtoKanto.width, height: NORTH }
const coastMask = new Uint8Array(coast.width * coast.height)
const landAlongTop = []
for (let x = 0; x < johtoKanto.width; x++) if (johtoKanto.rgba[x * 4 + 3]) landAlongTop.push(x)
const coastEnd = Math.max(...landAlongTop) + 1
for (let x = 0; x < coastEnd; x++) {
  // Highest at the middle, falling to the shore at the east end, with a
  // gently wandering edge.
  const rise = Math.min(1, ((coastEnd - x) / coastEnd) * 3)
  const wander = Math.sin(x * 0.21) * 3 + Math.sin(x * 0.07 + 1) * 4
  const reach = Math.round((NORTH - 10) * rise + wander * rise)
  for (let y = Math.max(0, NORTH - reach); y < NORTH; y++) coastMask[y * coast.width + x] = 1
}
const SINJOH = { x: (14 + 1) * 8 + 4, width: 34 }
const northCoast = paintLand(coastMask, coast.width, coast.height, {
  beyond: ["left", "bottom"],
  snow: (x, y) => Math.abs(x - SINJOH.x) < SINJOH.width / 2 + Math.sin(y * 0.6) * 2 + Math.sin(y * 0.23) * 3,
})
// Routes 49 and 50 run north from the map's edge to NEW SINJOH and the ruins.
const sinjohTowns = [
  { x: SINJOH.x - 4, y: 22 },
  { x: SINJOH.x + 5, y: 10 },
]
for (let y = sinjohTowns[1].y; y < NORTH; y++)
  for (let x = SINJOH.x - 2; x < SINJOH.x + 2; x++)
    northCoast.rgba.set([...ROUTE, 255], (y * coast.width + x) * 4)
for (const { x, y } of sinjohTowns) stamp(northCoast, townMarker, x - 4, y - 4)
save("world/north-coast.png", northCoast)
manifest["world/north-coast.png"].sinjoh = sinjohTowns

// The player's icon on the region map, and the SEAGALLOP ferry to sail the lanes.
save("world/brendan.png", renderIndexed(source("pokenav/region_map/brendan_icon.png")))
save(
  "world/ferry.png",
  renderIndexed(
    source("seagallop/ferry_sprite.png"),
    readJascPalette(source("seagallop/ferry_and_wake.pal")),
  ),
)

// Key item icons, each colored by its palette (named when the icon shares one).
for (const [item, palette = item] of [
  ["town_map"],
  ["ss_ticket"],
  ["sun_stone"],
  ["vs_seeker"],
  ["rare_candy"],
  ["master_ball"],
  ["soothe_bell"],
  ["tri_pass"],
  ["letter", "lava_cookie_and_letter"],
]) {
  const colors = readJascPalette(source(`items/icon_palettes/${palette}.pal`))
  save(
    `items/${item.replaceAll("_", "-")}.png`,
    renderIndexed(source(`items/icons/${item}.png`), colors),
  )
}

// Font: trace the normal Latin font's ink into outlines. Index 1 is ink and
// index 2 the baked drop shadow, which the site draws with text-shadow instead.
const unit = 64
const ascent = 12
const descent = 4
const font = readPng(source("fonts/latin_normal.png"))
const widthsSource = fs.readFileSync(path.join(game, "src/fonts.c"), "utf8")
const widths = widthsSource
  .match(/gFontNormalLatinGlyphWidths\[\] = \{([^}]*)\}/)[1]
  .split(",")
  .map((value) => Number(value.trim()))
  .filter((value) => !Number.isNaN(value))

const ink = (glyph, x, y) =>
  x >= 0 &&
  x < 16 &&
  y >= 0 &&
  y < 16 &&
  font.samples[(Math.floor(glyph / 16) * 16 + y) * font.width + (glyph % 16) * 16 + x] === 1

/** Traces the union of a glyph's ink pixels into closed outlines. */
const traceGlyph = (glyph) => {
  // Directed boundary edges, clockwise around ink in pixel space (y down).
  const edges = new Map()
  const add = (x1, y1, x2, y2) => {
    const key = `${x1},${y1}`
    if (!edges.has(key)) edges.set(key, [])
    edges.get(key).push([x2, y2])
  }
  for (let y = 0; y < 16; y++)
    for (let x = 0; x < 16; x++) {
      if (!ink(glyph, x, y)) continue
      if (!ink(glyph, x, y - 1)) add(x, y, x + 1, y)
      if (!ink(glyph, x + 1, y)) add(x + 1, y, x + 1, y + 1)
      if (!ink(glyph, x, y + 1)) add(x + 1, y + 1, x, y + 1)
      if (!ink(glyph, x - 1, y)) add(x, y + 1, x, y)
    }

  const loops = []
  for (const [startKey, targets] of edges) {
    while (targets.length) {
      const [sx, sy] = startKey.split(",").map(Number)
      const loop = [[sx, sy]]
      let [x, y] = targets.pop()
      while (x !== sx || y !== sy) {
        loop.push([x, y])
        ;[x, y] = edges.get(`${x},${y}`).pop()
      }
      // Drop points in the middle of straight runs.
      loops.push(
        loop.filter((point, i) => {
          const before = loop[(i - 1 + loop.length) % loop.length]
          const after = loop[(i + 1) % loop.length]
          return !(
            (before[0] === point[0] && point[0] === after[0]) ||
            (before[1] === point[1] && point[1] === after[1])
          )
        }),
      )
    }
  }

  const outline = new Path()
  for (const loop of loops) {
    loop.forEach(([x, y], i) => {
      const command = i === 0 ? "moveTo" : "lineTo"
      outline[command](x * unit, (ascent - y) * unit)
    })
    outline.close()
  }
  return outline
}

const characters = new Map()
for (const line of fs.readFileSync(path.join(game, "charmap.txt"), "utf8").split("\n")) {
  const match = line.match(/^'(\\.|[^\\])'\s*=\s*([0-9A-F]{2})\s*(@.*)?$/)
  if (!match) continue
  const character = match[1].length === 2 ? match[1][1] : match[1]
  if (!characters.has(character)) characters.set(character, Number.parseInt(match[2], 16))
}
// ASCII stand-ins the game spells with typographic glyphs.
characters.set('"', characters.get("”"))
characters.set("`", characters.get("‘"))
characters.set(" ", 0)

const glyphs = [
  new Glyph({
    name: ".notdef",
    unicode: 0,
    advanceWidth: 6 * unit,
    path: new Path(),
  }),
]
for (const [character, glyph] of characters) {
  if (glyph === undefined) continue
  glyphs.push(
    new Glyph({
      name: `uni${character.codePointAt(0).toString(16).toUpperCase().padStart(4, "0")}`,
      unicode: character.codePointAt(0),
      advanceWidth: widths[glyph] * unit,
      path: traceGlyph(glyph),
    }),
  )
}

const typeface = new Font({
  familyName: "Wayfarer Text",
  styleName: "Regular",
  unitsPerEm: 16 * unit,
  ascender: ascent * unit,
  descender: -descent * unit,
  // A fixed date keeps reruns byte-identical.
  createdTimestamp: Date.UTC(2026, 0, 1) / 1000,
  glyphs,
})
/**
 * opentype.js stamps the head table's modified date with the current time.
 * Copy the fixed created date over it and redo the checksums, so reruns of
 * this script produce identical bytes.
 */
const pinModifiedDate = (buffer) => {
  const bytes = Buffer.from(buffer)
  const sum = (start, length) => {
    let total = 0
    for (let i = start; i < start + length; i += 4) total = (total + bytes.readUInt32BE(i)) >>> 0
    return total
  }
  const tables = bytes.readUInt16BE(4)
  for (let i = 0; i < tables; i++) {
    const record = 12 + i * 16
    if (bytes.toString("latin1", record, record + 4) !== "head") continue
    const head = bytes.readUInt32BE(record + 8)
    const length = bytes.readUInt32BE(record + 12)
    bytes.copy(bytes, head + 28, head + 20, head + 28)
    bytes.writeUInt32BE(0, head + 8)
    bytes.writeUInt32BE(sum(head, Math.ceil(length / 4) * 4), record + 4)
    bytes.writeUInt32BE((0xb1b0afba - sum(0, bytes.length)) >>> 0, head + 8)
  }
  return bytes
}

fs.mkdirSync(path.join(out, "fonts"), { recursive: true })
fs.writeFileSync(
  path.join(out, "fonts/wayfarer-text.ttf"),
  pinModifiedDate(typeface.toArrayBuffer()),
)
console.log(`  public/game/fonts/wayfarer-text.ttf ${glyphs.length} glyphs`)

const manifestFile = path.resolve(here, "../src/generated/game-art.json")
fs.mkdirSync(path.dirname(manifestFile), { recursive: true })
fs.writeFileSync(manifestFile, `${JSON.stringify(manifest, null, 2)}\n`)
console.log(`  ${path.relative(process.cwd(), manifestFile)}`)

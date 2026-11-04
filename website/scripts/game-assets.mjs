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
import { openGame } from "../src/world-map/build/game.mjs"
import { openObjectGraphics } from "./lib/object-graphics.mjs"
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

// BRENDAN on his MACH BIKE in the overworld, for the world map: nine 32x32
// frames, standing south, north and west, then two pedaling frames each way.
save(
  "sprites/brendan-mach-bike.png",
  renderIndexed(
    source("object_events/pics/people/brendan/mach_bike.png"),
    readJascPalette(source("object_events/palettes/brendan.pal")),
  ),
)

// The other two starts' players on their bikes, laid out like BRENDAN's sheet:
// GOLD with his own palette, RED with the FireRed and LeafGreen player's.
for (const [name, file, palette] of [
  ["gold", "gold/mach_bike_hns", "gold_hns"],
  ["red", "red/red_bike", "player_frlg"],
])
  save(
    `sprites/${name}-mach-bike.png`,
    renderIndexed(
      source(`object_events/pics/people/${file}.png`),
      readJascPalette(source(`object_events/palettes/${palette}.pal`)),
    ),
  )

// The 24 Gym Badges as the trainer card draws them, 16x16 each in one row:
// Johto's eight, Kanto's eight, then Hoenn's, each in the order it is earned.
// The HNS sheet holds Johto's row above Kanto's; Hoenn's has its own sheet.
const johtoKanto = renderIndexed(source("trainer_card/hns/combined_badges.png"))
const hoennBadges = renderIndexed(source("trainer_card/badges.png"))
// The game draws its badges in greys (the Trainer Card's, the only badge art
// it has); the site tints each in its familiar colour, our art on the game's
// shapes. Each grey step takes a step of the badge's colour: the highlight,
// the light, the colour itself and its shadow; the black outline stays.
// In sheet order: Johto's eight, Kanto's, then Hoenn's.
const BADGE_COLORS = [
  // Zephyr, Hive, Plain, Fog, Storm, Mineral, Glacier, Rising
  [168, 184, 216],
  [216, 72, 64],
  [240, 192, 64],
  [152, 104, 200],
  [200, 128, 72],
  [136, 160, 200],
  [128, 200, 240],
  [64, 112, 216],
  // Boulder, Cascade, Thunder, Rainbow, Marsh, Soul, Volcano, Earth
  [176, 160, 144],
  [72, 152, 240],
  [248, 176, 48],
  [112, 200, 96],
  [232, 192, 64],
  [240, 120, 176],
  [232, 80, 48],
  [96, 176, 88],
  // Stone, Knuckle, Dynamo, Heat, Balance, Feather, Mind, Rain
  [168, 152, 136],
  [120, 144, 208],
  [240, 208, 64],
  [240, 104, 48],
  [224, 88, 88],
  [104, 200, 224],
  [232, 120, 200],
  [64, 136, 232],
]
const mixColor = (a, b, t) => a.map((v, i) => Math.round(v + (b[i] - v) * t))
const tintBadges = (sheet) => {
  for (let i = 0; i < sheet.width * sheet.height; i++) {
    const at = i * 4
    if (!sheet.rgba[at + 3]) continue
    const grey = sheet.rgba[at]
    if (grey === 0) continue
    const base = BADGE_COLORS[Math.floor((i % sheet.width) / 16)]
    const t = grey / 248
    const color =
      t >= 0.95
        ? mixColor(base, [255, 255, 255], 0.7)
        : t >= 0.8
          ? mixColor(base, [255, 255, 255], 0.3)
          : t >= 0.65
            ? base
            : mixColor(base, [0, 0, 0], 0.4)
    sheet.rgba.set(color, at)
  }
  return sheet
}
save(
  "badges.png",
  tintBadges(
    sideBySide([crop(johtoKanto, 0, 0, 128, 16), crop(johtoKanto, 0, 16, 128, 16), hoennBadges]),
  ),
)

// The three ferries as they sit at the game's docks, each with its own palette,
// for the world map's living overlay.
for (const [name, file, palette] of [
  ["aqua", "ss_aqua_hns", "ss_aqua_hns"],
  ["anne", "ss_anne", "ss_anne"],
  ["tidal", "ss_tidal", "ss_tidal"],
])
  save(
    `sprites/ss-${name}.png`,
    renderIndexed(
      source(`object_events/pics/misc/${file}.png`),
      readJascPalette(source(`object_events/palettes/${palette}.pal`)),
    ),
  )

// The Magnet Train, its two ends side by side (west nose, east nose) as one
// two-car train, for the world map's living overlay.
{
  const palette = readJascPalette(source("object_events/palettes/train_hns.pal"))
  save(
    "sprites/magnet-train.png",
    trim(
      sideBySide(
        ["train_west_hns", "train_east_hns"].map((file) =>
          renderIndexed(source(`object_events/pics/misc/${file}.png`), palette),
        ),
      ),
    ),
  )
}

// The 25 simulated Gym Leaders and Elite Four as the overworld draws them, for
// the world map's walkers: one row per trainer in routines.json order, nine
// 16x32 frames each (stand south, north, west; walk south, north, west, two
// frames each). The sheet, palette and frames come from the trainer's
// OBJ_EVENT_GFX_* graphics info, the way the game looks them up.
{
  const objectGraphics = openObjectGraphics(game)
  // Then the Gym Leaders whose sprite only stands (three frames, the rest of
  // the row left empty), who stand at their Gym on the map.
  const trainers = [
    ...JSON.parse(
      fs.readFileSync(path.join(game, "tools/wayfarer_world/routines.json"), "utf8"),
    ).trainers.filter((trainer) => trainer.simulated),
    ...JSON.parse(
      fs.readFileSync(
        path.join(here, "../src/world-map/build/source/stationary-leaders.json"),
        "utf8",
      ),
    ).leaders,
  ]
  const sheet = {
    width: 144,
    height: 32 * trainers.length,
    rgba: new Uint8Array(144 * 32 * trainers.length * 4),
  }
  trainers.forEach((trainer, row) => {
    const { sheet: frames } = objectGraphics(trainer.graphics)
    if ((frames.width !== 144 && frames.width !== 48) || frames.height !== 32)
      throw new Error(
        `${trainer.graphics}: ${frames.width}x${frames.height}, not 9 or 3 frames of 16x32`,
      )
    for (let y = 0; y < 32; y++)
      sheet.rgba.set(
        frames.rgba.subarray(y * frames.width * 4, (y + 1) * frames.width * 4),
        (row * 32 + y) * 144 * 4,
      )
  })
  save("sprites/notables.png", sheet)
}

save("text/down-arrow.png", renderIndexed(source("fonts/down_arrow.png")))

// Text window frame 1, the game's default.
save("frames/1.png", renderIndexed(source("text_window/1.png")))

// The wood map name popup, for the world map's ROUTE 101 and LITTLEROOT TOWN,
// framed as DrawMapNamePopUpFrame lays out the outline tiles: 12 along the
// top, three down each side, 12 along the bottom. Wood colors its outline
// with the body's palette.
const placeTile = (target, tile, x, y) => {
  for (let row = 0; row < 8; row++)
    target.rgba.set(tile.rgba.subarray(row * 32, row * 32 + 32), ((y + row) * target.width + x) * 4)
}
const woodPalette = readPng(source("map_popup/wood.png")).palette
const popupBody = renderIndexed(source("map_popup/wood.png"), woodPalette)
const popupOutline = spriteTiles(renderIndexed(source("map_popup/wood_outline.png"), woodPalette))
const popup = { width: 96, height: 40, rgba: new Uint8Array(96 * 40 * 4) }
for (let y = 0; y < 24; y++)
  popup.rgba.set(popupBody.rgba.subarray(y * 80 * 4, (y + 1) * 80 * 4), ((y + 8) * 96 + 8) * 4)
for (let i = 0; i < 12; i++) {
  placeTile(popup, popupOutline[i], i * 8, 0)
  placeTile(popup, popupOutline[18 + i], i * 8, 32)
}
for (let side = 0; side < 3; side++) {
  placeTile(popup, popupOutline[12 + side * 2], 0, 8 + side * 8)
  placeTile(popup, popupOutline[13 + side * 2], 88, 8 + side * 8)
}
save("popups/wood.png", popup)

// Route 103's water, one metatile, for the pond under the page's SURF prompt.
const water = openGame(game).drawMetatile("gTileset_General", "gTileset_Petalburg", 0x170)
save("water.png", { width: 16, height: 16, rgba: water })

// The Bag the BAG screen holds up, its six pockets stacked as frames.
save(
  "bag.png",
  renderIndexed(source("bag/hns/bag_male.png"), readJascPalette(source("bag/hns/bag.pal"))),
)

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
  ["safari_ball"],
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

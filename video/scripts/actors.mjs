#!/usr/bin/env node
// Cuts the sprites the video's actors are drawn from out of the game's own
// art in ../game/graphics, into actors/: the player walking and running,
// people and Pokémon as the overworld draws them, the emotion bubbles and a
// Poké Ball. The output is committed, so rendering the video does not need
// the game tree; rerun with `pnpm actors` after the source art changes.
import * as fs from "node:fs"
import * as path from "node:path"
import { fileURLToPath } from "node:url"

import { readJascPalette, renderIndexed } from "../../website/scripts/lib/gba.mjs"
import { openObjectGraphics } from "../../website/scripts/lib/object-graphics.mjs"
import { writePng } from "../../website/scripts/lib/png.mjs"

const here = path.dirname(fileURLToPath(import.meta.url))
const game = path.resolve(here, "../../game")
const graphics = path.join(game, "graphics")
const out = path.resolve(here, "../actors")

const save = (name, image) => {
  fs.mkdirSync(out, { recursive: true })
  writePng(path.join(out, `${name}.png`), image.width, image.height, image.rgba)
  console.log(`  actors/${name}.png ${image.width}x${image.height}`)
}
const palette = (name) => readJascPalette(path.join(graphics, "object_events/palettes", name))

// GOLD on foot: nine 16x32 frames (stand south, north, west; then two steps each way).
save(
  "gold-walking",
  renderIndexed(
    path.join(graphics, "object_events/pics/people/gold/walking_hns.png"),
    palette("gold_hns.pal"),
  ),
)

// A rod for someone the game never drew fishing: the one in GOLD's hands, cast out west
// (his fourth frame: five rows of it, left of his hands), on a 16x32 frame of its own to
// lay over a person standing beside the water.
{
  const cast = renderIndexed(
    path.join(graphics, "object_events/pics/people/gold/fishing_hns.png"),
    palette("gold_hns.pal"),
  )
  const rod = { width: 16, height: 32, rgba: new Uint8Array(16 * 32 * 4) }
  // His hands are at column 18; row 25 runs into them, so it stops short.
  for (let y = 21; y <= 25; y++)
    for (let x = 4; x <= (y === 25 ? 16 : 17); x++) {
      const from = (y * cast.width + 3 * 32 + x) * 4
      rod.rgba.set(cast.rgba.subarray(from, from + 4), ((y - 1) * 16 + x - 4) * 4)
    }
  save("rod", rod)
}

// What a surfer rides: the Pokémon itself, as the game draws the ones it has surfing
// sprites for. The sheet's top row is six 32x32 frames (two each facing south, north and
// west); the row under it, drawn over the rider, is left out.
for (const [name, file] of [["luvdisc-surf", "0370_luvdisc"]]) {
  const sheet = renderIndexed(
    path.join(graphics, `object_events/pics/pokemon/surfable/${file}.png`),
  )
  save(name, { width: 192, height: 32, rgba: sheet.rgba.subarray(0, 192 * 32 * 4) })
}

// People by their OBJ_EVENT_GFX_* name, laid out the same way.
const objectGraphics = openObjectGraphics(game)

// The other players, each for the scenes in their own land. RED also fishes (twelve
// 32x32 frames: four each casting west, north and south) and sits a surfing Pokémon
// (the first three 16x32 frames of his surfing and running sheet: south, north, west).
save("red-fishing", objectGraphics("OBJ_EVENT_GFX_RED_FISH").sheet)
{
  const { sheet } = objectGraphics("OBJ_EVENT_GFX_RED_SURF")
  const sitting = { width: 48, height: 32, rgba: new Uint8Array(48 * 32 * 4) }
  for (let y = 0; y < 32; y++)
    sitting.rgba.set(
      sheet.rgba.subarray(y * sheet.width * 4, (y * sheet.width + 48) * 4),
      y * 48 * 4,
    )
  save("red-surfing", sitting)
}

for (const [name, id] of [
  ["red", "OBJ_EVENT_GFX_RED_NORMAL"],
  ["leaf", "OBJ_EVENT_GFX_GREEN_NORMAL"],
  ["kris", "OBJ_EVENT_GFX_KRIS_NORMAL_HNS"],
  ["brock", "OBJ_EVENT_GFX_BROCK_HNS"],
  ["blue", "OBJ_EVENT_GFX_BLUE_HNS"],
  ["surge", "OBJ_EVENT_GFX_SURGE_HNS"],
  ["wallace", "OBJ_EVENT_GFX_WALLACE"],
  ["clair", "OBJ_EVENT_GFX_CLAIR_HNS"],
  // A tenth frame after the usual nine, which the video doesn't use.
  ["bug-catcher", "OBJ_EVENT_GFX_BUG_CATCHER_HNS"],
])
  save(name, objectGraphics(id).sheet)

// Pokémon as they follow the player: six frames (two steps south, north and
// west), 32x32 each, or 64x64 for the big ones.
for (const [name, dir] of [
  ["wooloo", "wooloo"],
  ["graveler", "graveler"],
  ["golem", "golem"],
  ["exeggutor-alola", "exeggutor/alola"],
  ["noctowl", "noctowl"],
  ["luvdisc", "luvdisc"],
  // Kalos, as the Safari Zone at Fuchsia keeps it.
  ["gogoat", "gogoat"],
  ["pyroar", "pyroar"],
  ["furfrou", "furfrou"],
  ["vivillon", "vivillon"],
])
  save(name, renderIndexed(path.join(graphics, "pokemon", dir, "overworld.png")))

// The bubbles over a head, 16x16, and the Poké Ball as an item on the ground.
for (const name of ["exclamation", "question", "heart"])
  save(
    `emote-${name}`,
    renderIndexed(path.join(graphics, `field_effects/pics/emotion_${name}.png`)),
  )
save("ball", renderIndexed(path.join(graphics, "object_events/pics/misc/ball_poke.png")))

// GOLEM as a battle shows it, for the Elite Four's reveal: the first 64x64 frame of its front picture.
{
  const front = renderIndexed(path.join(graphics, "pokemon/golem/anim_front.png"))
  save("golem-front", { width: 64, height: 64, rgba: front.rgba.subarray(0, 64 * 64 * 4) })
}

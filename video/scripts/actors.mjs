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

// GOLD on foot: nine 16x32 frames each (stand south, north, west; then two
// steps each way), walking and running.
for (const gait of ["walking", "running"])
  save(
    `gold-${gait}`,
    renderIndexed(
      path.join(graphics, `object_events/pics/people/gold/${gait}_hns.png`),
      palette("gold_hns.pal"),
    ),
  )

// GOLD with a rod (twelve 32x32 frames: four each casting west, north and
// south) and on the water (two each south, north and west; the first of each
// pair sits, the second jumps).
for (const [name, file] of [
  ["gold-fishing", "fishing_hns"],
  ["gold-surfing", "surfing_hns"],
])
  save(
    name,
    renderIndexed(
      path.join(graphics, `object_events/pics/people/gold/${file}.png`),
      palette("gold_hns.pal"),
    ),
  )
// What a surfer rides: the game's blob, three 32x32 frames.
save("surf-blob", renderIndexed(path.join(graphics, "field_effects/pics/surf_blob.png")))

// People by their OBJ_EVENT_GFX_* name, laid out the same way.
const objectGraphics = openObjectGraphics(game)
for (const [name, id] of [["brock", "OBJ_EVENT_GFX_BROCK_HNS"]])
  save(name, objectGraphics(id).sheet)

// Pokémon as they follow the player: six frames (two steps south, north and
// west), 32x32 each, or 64x64 for the big ones.
for (const [name, dir] of [
  ["wooloo", "wooloo"],
  ["graveler", "graveler"],
  ["golem", "golem"],
  ["exeggutor-alola", "exeggutor/alola"],
  ["noctowl", "noctowl"],
  ["mareanie", "mareanie"],
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

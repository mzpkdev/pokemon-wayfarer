// Overworld sprites by OBJ_EVENT_GFX_* name, found the way the game looks them
// up: the graphics info pointer table gives the info, which gives the pic
// table (the first picture is the sheet), the palette tag and the frame size;
// the palette tag gives the palette file.
import * as fs from "node:fs"
import * as path from "node:path"

import { readJascPalette, renderIndexed } from "./gba.mjs"

/** Reads the object event tables of the game tree at `game`. */
export const openObjectGraphics = (game) => {
  const objectEvents = (file) =>
    fs.readFileSync(path.join(game, "src/data/object_events", file), "utf8")
  const infoSource = objectEvents("object_event_graphics_info.h")
  const picTables = objectEvents("object_event_pic_tables.h")
  const pictures = objectEvents("object_event_graphics.h")
  const pointers = objectEvents("object_event_graphics_info_pointers.h")
  const movement = fs.readFileSync(path.join(game, "src/event_object_movement.c"), "utf8")
  const incbin = (symbol) =>
    pictures.match(new RegExp(`${symbol}\\[\\]\\s*=\\s*INCBIN_U\\d+\\("([^"]+)"`))?.[1]

  /**
   * The whole sheet of a graphics id, colored with its palette, and the size
   * of one frame in it.
   */
  return (graphics) => {
    const info = pointers.match(
      new RegExp(`\\[${graphics}\\]\\s*=\\s*&(gObjectEventGraphicsInfo_\\w+)`),
    )?.[1]
    const body = infoSource.match(new RegExp(`${info} =\\s*\\{([\\s\\S]*?)\\};`))?.[1]
    if (!body) throw new Error(`no graphics info for ${graphics}`)
    const paletteTag = body.match(/paletteTag = (\w+)/)?.[1] ?? body.match(/^\s*\w+,\s*(\w+)/)?.[1]
    const table = body.match(/sPicTable_\w+/)[0]
    const picture = picTables.match(
      new RegExp(`${table}\\[\\] = \\{\\s*overworld_(?:frame|ascending_frames)\\((\\w+),`),
    )[1]
    const paletteSymbol = movement.match(
      new RegExp(`\\{(gObjectEventPal_\\w+),\\s*${paletteTag}\\}`),
    )?.[1]
    const pictureFile = incbin(picture)
    const paletteFile = paletteSymbol && incbin(paletteSymbol)
    if (!pictureFile || !paletteFile) throw new Error(`no art for ${graphics}`)
    const sheet = renderIndexed(
      path.join(game, pictureFile.replace(/\.4bpp$/, ".png")),
      readJascPalette(path.join(game, paletteFile.replace(/\.gbapal$/, ".pal"))),
    )
    // The HNS infos list their fields in order: tile tag, palette tag,
    // reflection palette tag, size, width, height.
    const fields = body.split(",").map((field) => field.trim())
    const size = (name, position) =>
      +(body.match(new RegExp(`\\.${name} = (\\d+)`))?.[1] ?? fields[position])
    return { sheet, width: size("width", 4), height: size("height", 5) }
  }
}

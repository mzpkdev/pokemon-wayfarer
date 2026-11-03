// The daily world slots for the tour's "routes change daily": where the
// regular trainers can stand and where items can lie, with the tables the
// client needs to replay the game's own daily draws (src/daily.ts).
//
// The tables are the generated ones the game compiles: the trainer slots,
// their absence levels and groups (src/data/trainer_slots/slots.h) and the
// item spots and pools (src/data/item_slots/). Specs:
// .product/specs/daily-world-slots.md and .product/specs/world-items.md.
//
// Every slot is kept, placed or not, because a group's slots all take
// wanderers and the shuffle depends on the whole group. A slot on a map the
// world does not place has no objects. Item spots are drawn one by one, so
// only the placed ones are kept, with their index in the game's table.
import fs from "node:fs"

import { writePng } from "../../../scripts/lib/png.mjs"
import { openObjectGraphics } from "../../../scripts/lib/object-graphics.mjs"

/** A cell of trainers.png: a sprite stands bottom-centred in it. */
const CELL = 32
/** Sprites per row of trainers.png. */
const COLUMNS = 16
/** The item ball's graphics id, as the maps' item balls use it. */
const BALL = "OBJ_EVENT_GFX_POKE_BALL"
/** WorldItemRegion, WorldItemTier: enum order in include/wayfarer_daily_slots.h. */
const REGIONS = ["KANTO", "SEVII", "JOHTO", "ALOLA", "SINJOH", "HOENN"]
const TIERS = ["ROAD", "WILDS", "OUTLANDS"]

/** The text between the first "{" after `marker` and its closing "}". */
const block = (text, marker) => {
  const start = text.indexOf("{", text.indexOf(marker))
  let depth = 0
  for (let i = start; i < text.length; i++) {
    if (text[i] === "{") depth++
    else if (text[i] === "}" && --depth === 0) return text.slice(start + 1, i)
  }
  throw new Error(`no block after ${marker}`)
}

/** Every trainer's name and class constant, from the party files and the .h trainer tables. */
const readTrainers = (game) => {
  const trainers = new Map()
  const dir = `${game.root}/src/data`
  for (const file of fs.readdirSync(dir).filter((f) => f.endsWith(".party")))
    for (const part of fs.readFileSync(`${dir}/${file}`, "utf8").split(/^=== /m).slice(1)) {
      const field = (name) => new RegExp(`^${name}: (.+)$`, "m").exec(part)?.[1].trim()
      trainers.set(part.slice(0, part.indexOf(" ===")), {
        name: field("Name"),
        // "Bug Catcher Hns" is TRAINER_CLASS_BUG_CATCHER_HNS.
        cls: `TRAINER_CLASS_${field("Class")?.toUpperCase().replaceAll(" ", "_")}`,
      })
    }
  for (const file of fs.readdirSync(dir).filter((f) => /^trainers_.*\.h$/.test(f)))
    for (const [, id, body] of fs
      .readFileSync(`${dir}/${file}`, "utf8")
      .matchAll(/^\[(TRAINER_\w+)\] =\s*\{([\s\S]*?)^\s*\.party/gm))
      trainers.set(id, {
        name: /\.trainerName = _\("([^"]*)"\)/.exec(body)?.[1],
        cls: /\.trainerClass = (\w+)/.exec(body)?.[1],
      })
  return trainers
}

/** The in-game name of each trainer class (gTrainerClasses in battle_main.c). */
const readClassNames = (game) =>
  new Map(
    [
      ...game
        .read("src/battle_main.c")
        .toString()
        .matchAll(/\[(TRAINER_CLASS_\w+)\]\s*=\s*\{\s*_\("([^"]*)"\)/g),
    ].map(([, id, name]) => [id, name.replaceAll("{PKMN}", "POKéMON")]),
  )

/** Item names by ITEM_ constant; a TM shows its number and move, as the Bag does. */
const readItemNames = (game) => {
  const names = (file, key, macro) =>
    new Map(
      [
        ...game
          .read(file)
          .toString()
          .matchAll(
            new RegExp(`\\[(${key}\\w+)\\] =\\s*\\{\\s*\\.name = ${macro}\\("([^"]*)"\\)`, "g"),
          ),
      ].map(([, id, name]) => [id, name]),
    )
  const items = names("src/data/items.h", "ITEM_", "ITEM_NAME")
  const moves = names("src/data/moves_info.h", "MOVE_", "COMPOUND_STRING")
  // Older names are aliases of the item's constant (ITEM_BLK_APRICORN).
  const aliases = new Map(
    [
      ...game
        .read("include/constants/items.h")
        .toString()
        .matchAll(/^\s*(ITEM_\w+)\s*=\s*(ITEM_\w+),/gm),
    ].map(([, alias, item]) => [alias, item]),
  )
  // The TMs are numbered in the order of the build's FOREACH_TM list.
  const tmList = game.read("include/constants/tms_hms.h").toString()
  const tms = [...tmList.slice(0, tmList.indexOf("FOREACH_HM")).matchAll(/F\((\w+)\)/g)].map(
    (m) => m[1],
  )
  return (item) => {
    const tm = tms.indexOf(item.replace("ITEM_TM_", ""))
    if (item.startsWith("ITEM_TM_") && tm >= 0)
      return `TM${String(tm + 1).padStart(2, "0")} ${moves.get(item.replace("ITEM_", "MOVE_").replace("MOVE_TM_", "MOVE_"))}`
    const name = items.get(item) ?? items.get(aliases.get(item))
    if (!name) throw new Error(`no name for ${item}`)
    return name
  }
}

/** The trainer slots, groups and absence levels of slots.h. */
const readTrainerSlots = (game) => {
  const text = game.read("src/data/trainer_slots/slots.h").toString()
  const absence = [...block(text, "sWorldTrainerAbsence").matchAll(/\d+/g)].map((m) => +m[0])
  const slots = []
  for (const line of text.split("\n")) {
    const m =
      /^\s*\{ (TRAINER_\w+), MAP_GROUP\(\w+\), MAP_NUM\(\w+\), \{ (\d+), (\d+) \}, (\w+), (.+?), \{ (\w+), (\w+) \}, \{ .* \} \}, \/\/ (\w+)/.exec(
        line,
      )
    if (!m) continue
    const flags = m[5]
    slots.push({
      trainer: m[1],
      map: m[8],
      local: [+m[2], +m[3]].filter(Boolean),
      group: m[4] === "WORLD_TRAINER_GROUP_HOME" ? -1 : +m[4],
      pair: flags.includes("WORLD_TRAINER_SLOT_PAIR"),
      absence: absence[+/WORLD_TRAINER_ABSENCE\((\d+)\)/.exec(flags)[1]],
      gfx: [m[6], m[7]].filter((g) => g !== "0"),
    })
  }
  const flat = [...block(text, "sWorldTrainerGroupSlots[]").matchAll(/\d+/g)].map((m) => +m[0])
  const groups = [
    ...block(text, "sWorldTrainerGroups[WORLD_TRAINER_GROUP_COUNT]").matchAll(
      /\{ (\d+), (\d+) \}/g,
    ),
  ].map(([, first, count]) => flat.slice(+first, +first + +count))
  return { slots, groups }
}

/** The item spots of spots.h, in the game's slot index order. */
const readItemSpots = (game) =>
  [
    ...game
      .read("src/data/item_slots/spots.h")
      .toString()
      .matchAll(
        /^\s*\{ MAP_GROUP\(\w+\), MAP_NUM\(\w+\), (\d+), WORLD_ITEM_ATTRS\(WORLD_ITEM_SPOT_(\w+), WORLD_ITEM_SPOT_(\w+), WORLD_ITEM_TIER_(\w+), WORLD_ITEM_REGION_(\w+)\), (\w+), \w+ \}, \/\/ (\w+) (\w+) (\d+),(\d+)/gm,
      ),
  ].map(([, id, hidden, prize, tier, region, item, map, , x, y]) => ({
    id: +id,
    hidden: hidden === "HIDDEN",
    prize: prize === "PRIZE",
    tier: TIERS.indexOf(tier),
    region: REGIONS.indexOf(region),
    item,
    map,
    x: +x,
    y: +y,
  }))

/** The tier odds and the pools of pools.h. Pool entries are [item, weight]; a negative item stands for a group. */
const readPools = (game, nameOf) => {
  const text = game.read("src/data/item_slots/pools.h").toString()
  const list = (marker) => [...block(text, marker).matchAll(/ITEM_\w+/g)].map((m) => nameOf(m[0]))
  const odds = [...block(text, "sWorldItemTierOdds").matchAll(/\{ (\d+), (\d+) \}/g)].map(
    ([, a, b]) => [+a, +b],
  )
  const names = []
  const indexOf = (name) => {
    if (!names.includes(name)) names.push(name)
    return names.indexOf(name)
  }
  const pools = REGIONS.map((region) =>
    ["Regular", "Better", "Special"].map((tier) =>
      [
        ...block(
          text,
          `sWorldItemPool_${region[0]}${region.slice(1).toLowerCase()}_${tier}[]`,
        ).matchAll(/\{ (\w+), (\d+) \}/g),
      ].map(([, item, weight]) => [
        item === "WORLD_ITEM_GROUP_STONE"
          ? -1
          : item === "WORLD_ITEM_GROUP_TM"
            ? -2
            : indexOf(nameOf(item)),
        +weight,
      ]),
    ),
  )
  return {
    empty: +/WORLD_ITEM_EMPTY_PERCENT (\d+)/.exec(text)[1],
    odds,
    pools,
    stones: list("sWorldItemStones").map(indexOf),
    tms: list("sWorldItemDynamicTms").map(indexOf),
    names,
    indexOf,
  }
}

/**
 * Writes daily.json and trainers.png in `outDir` for the placed maps
 * (`world.all`). Returns what it wrote, for the log.
 */
export const writeDaily = (game, world, outDir) => {
  const placed = new Map(world.all.map((p) => [p.m.name, p]))
  const at = (p, object) => [p.x - world.minX + object.x, p.y - world.minY + object.y]
  const trainers = readTrainers(game)
  const classes = readClassNames(game)
  const nameOf = readItemNames(game)
  const graphics = openObjectGraphics(game.root)

  // One cell per distinct graphics id, in the order first asked for.
  const sprites = []
  const spriteOf = (gfx) => {
    if (!sprites.includes(gfx)) sprites.push(gfx)
    return sprites.indexOf(gfx)
  }
  const ball = spriteOf(BALL)

  const { slots, groups } = readTrainerSlots(game)
  const outSlots = slots.map((slot) => {
    const trainer = trainers.get(slot.trainer)
    const cls = classes.get(trainer?.cls)
    if (!trainer?.name || !cls) throw new Error(`no name or class for ${slot.trainer}`)
    const out = {
      n: `${cls} ${trainer.name}`,
      s: slot.gfx.map(spriteOf),
      g: slot.group,
      a: slot.absence,
    }
    if (slot.pair) out.p = 1
    const p = placed.get(slot.map)
    if (p) {
      out.m = slot.map
      // Local id N is the map's N-th object event.
      out.o = slot.local.map((id) => at(p, p.m.object_events[id - 1]))
    }
    return out
  })

  const pools = readPools(game, nameOf)
  const spots = []
  readItemSpots(game).forEach((spot, index) => {
    const p = placed.get(spot.map)
    if (!p) return
    // The cell is in the table's comment (a Sevii spot's id is not its object's index).
    const [x, y] = at(p, spot)
    const out = { i: index, m: spot.map, x, y, t: spot.tier, r: spot.region }
    if (spot.hidden) out.h = 1
    if (spot.prize) out.z = pools.indexOf(nameOf(spot.item))
    spots.push(out)
  })

  // trainers.png: each sprite's first frame (standing, facing down), bottom-centred in its cell.
  const rows = Math.ceil(sprites.length / COLUMNS)
  const sheet = new Uint8Array(COLUMNS * CELL * rows * CELL * 4)
  sprites.forEach((gfx, k) => {
    const { sheet: image, width, height } = graphics(gfx)
    if (width > CELL || height > CELL) throw new Error(`${gfx} is ${width}x${height}`)
    const left = (k % COLUMNS) * CELL + (CELL - width) / 2
    const top = Math.floor(k / COLUMNS) * CELL + CELL - height
    for (let y = 0; y < height; y++)
      sheet.set(
        image.rgba.subarray(y * image.width * 4, (y * image.width + width) * 4),
        ((top + y) * COLUMNS * CELL + left) * 4,
      )
  })
  writePng(`${outDir}/trainers.png`, COLUMNS * CELL, rows * CELL, sheet)

  fs.writeFileSync(
    `${outDir}/daily.json`,
    JSON.stringify({
      sheet: { cell: CELL, columns: COLUMNS, itemBall: ball },
      slots: outSlots,
      groups,
      spots,
      items: pools.names,
      itemEmpty: pools.empty,
      tierOdds: pools.odds,
      pools: pools.pools,
      stones: pools.stones,
      tms: pools.tms,
    }),
  )
  return {
    slots: outSlots.length,
    placed: outSlots.filter((s) => s.m).length,
    spots: spots.length,
    sprites: sprites.length,
  }
}

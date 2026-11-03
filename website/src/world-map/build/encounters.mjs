// The wild Pokémon of each placed map, for the explorer's place cards.
//
// The wayfarer build compiles only the v2 encounter tables
// (game/src/data/wild_encounters_v2/*.json; wild_encounters.json is for the
// standalone Emerald, FireRed/LeafGreen and HNS builds, see
// tools/wild_encounters/wild_encounters_to_header.py). A v2 table is a list of
// species per slot, per method (land, surf, rock smash, fishing), for a day and
// a night table: morning and day use the day table, evening and night the
// night table, and a missing night table falls back to the day one. How often
// a slot comes up follows the slot's weight: the field rates in
// wild_encounters.json for land, surf and rock smash, and
// standard_rod_fishing.json for each rod.
//
// Each species' icon is the first frame of its icon.png, colored with the icon
// palette its species info picks (.iconPalIndex), as the game does.
import fs from "node:fs"

import { readJascPalette } from "../../../scripts/lib/gba.mjs"
import { readPng, writePng } from "../../../scripts/lib/png.mjs"

const REGION_FILES = ["kanto", "johto", "hoenn", "alola", "sevii", "safari", "sinjoh"]
/** Atlas icons per row. */
const COLUMNS = 16
const ICON = 32

/** Where each generation's national dex numbers end (Gen VII: Meltan and Melmetal; Gen VIII: Enamorus, Hisuian species included). */
const GEN_ENDS = [151, 251, 386, 493, 649, 721, 809, 905]
/** The generation of a national dex number: 1 to 8, or 9 for anything after (the client leaves it out). */
const genOf = (dex) => GEN_ENDS.findIndex((end) => dex <= end) + 1 || 9
/** Marks a regional form (Alolan, Galarian, Hisuian, Paldean) in `gens`, which holds 1 to 9 otherwise. */
const FORM = 0

/** The national dex numbers by NATIONAL_DEX_ name: enum order, so NONE is 0. */
const readDexNumbers = (game) => {
  const text = game.read("include/constants/pokedex.h").toString()
  const body = text.slice(text.indexOf("enum NationalDexOrder"), text.indexOf("};"))
  const numbers = new Map()
  let next = 0
  for (const [, name, alias] of body.matchAll(
    /^\s*NATIONAL_DEX_(\w+)\s*(?:=\s*NATIONAL_DEX_(\w+))?\s*,/gm,
  )) {
    numbers.set(name, alias ? numbers.get(alias) : next)
    next = numbers.get(name) + 1
  }
  return numbers
}

const json = (game, file) => JSON.parse(game.read(file))

/** Each species' name, icon file and icon palette from the species info, with the build's config applied. */
/** The arguments of the call whose "(" is at `open` in `source`. */
const argsAt = (source, open) => {
  const args = []
  let depth = 0
  let from = open + 1
  for (let i = open; i < source.length; i++) {
    const c = source[i]
    if (c === "(") depth++
    else if (c === ")" && --depth === 0) {
      args.push(source.slice(from, i).trim())
      return args
    } else if (c === "," && depth === 1) {
      args.push(source.slice(from, i).trim())
      from = i + 1
    }
  }
  return args
}

export const readSpecies = (game) => {
  const root = "src/data/pokemon/species_info"
  const files = fs
    .readdirSync(`${game.root}/${root}`)
    .filter((f) => /^gen_\d+_families\.h$/.test(f))
  const graphics = game.read("src/data/graphics/pokemon.h").toString()
  // The first INCBIN of an icon symbol is the full-color icon, the build's
  // (P_GBA_STYLE_SPECIES_ICONS is FALSE).
  const iconFile = (symbol) =>
    graphics
      .match(new RegExp(`${symbol}\\[\\]\\s*=\\s*INCBIN_U8\\(\\s*"([^"]+?)\\.4bpp"`))?.[1]
      ?.concat(".png")
  const dexNumbers = readDexNumbers(game)
  const species = new Map()
  for (const file of files) {
    const text = game.read(`${root}/${file}`).toString()
    // An entry is a block of its own or a macro that expands to one. Macros
    // take arguments, pasted into names with ## (gMonIcon_Vivillon ##form),
    // and call one another.
    const macros = new Map(
      [...text.matchAll(/^[ \t]*#define (\w+)\(([^)]*)\)((?:.*\\\n)*.*)$/gm)].map((m) => [
        m[1],
        { params: m[2].split(",").map((p) => p.trim()), body: m[3] },
      ]),
    )
    const expand = (name, args = [], depth = 0) => {
      const macro = macros.get(name)
      if (!macro || depth > 8) return ""
      let body = macro.params.reduce(
        (done, param, i) =>
          param ? done.replace(new RegExp(`\\b${param}\\b`, "g"), args[i] ?? "") : done,
        macro.body,
      )
      body = body.replace(/\s*##\s*/g, "")
      const calls = [...body.matchAll(/(\w+)\(/g)]
        .filter((m) => macros.has(m[1]))
        .map((m) => expand(m[1], argsAt(body, m.index + m[1].length), depth + 1))
      return body + calls.join("")
    }
    const starts = [...text.matchAll(/\[SPECIES_(\w+)\]\s*=\s*(?:\{|(\w+)\()/g)]
    starts.forEach((start, i) => {
      const block = start[2]
        ? expand(start[2], argsAt(text, start.index + start[0].length - 1))
        : // A plain block ends where the next species starts, or where a
          // macro for the next family is defined before it; macros it calls
          // (VIVILLON_MISC_INFO(IcySnow, …)) add their fields.
          ((plain) =>
            plain +
            [...plain.matchAll(/(\w+)\(/g)]
              .filter((m) => macros.has(m[1]))
              .map((m) => expand(m[1], argsAt(plain, m.index + m[1].length)))
              .join(""))(
            text
              .slice(start.index, starts[i + 1]?.index ?? text.length)
              .split(/^[ \t]*#define /m)[0],
          )
      const symbol = block.match(/\.iconSprite\s*=\s*(\w+)/)?.[1]
      const palette = block.match(/\.iconPalIndex\s*=\s*([^,]+),/)?.[1]?.trim()
      // `P_GBA_STYLE_SPECIES_ICONS ? gba : full`: the build uses the second.
      const index = palette?.includes("?") ? palette.split(":").pop().trim() : palette
      // The last .natDexNum is the one for a build without separate regional
      // forms; a regional form has its own species id and is told by it.
      const dex = [...block.matchAll(/\.natDexNum\s*=\s*NATIONAL_DEX_(\w+)/g)].at(-1)?.[1]
      species.set(start[1], {
        form: /_(ALOLA|GALAR|HISUI|PALDEA)(_|$)/.test(start[1]),
        dex: dexNumbers.get(dex),
        name: block.match(/\.speciesName\s*=\s*_\("([^"]*)"\)/)?.[1],
        icon: symbol && iconFile(symbol),
        palette: /^\d$/.test(index ?? "") ? Number(index) : undefined,
      })
    })
  }
  // SPECIES_BASCULIN and the like are aliases.
  const aliases = new Map(
    [
      ...game
        .read("include/constants/species.h")
        .toString()
        .matchAll(/#define SPECIES_(\w+)\s+SPECIES_(\w+)\s*$/gm),
    ].map((m) => [m[1], m[2]]),
  )
  const lookup = (id) => {
    while (aliases.has(id)) id = aliases.get(id)
    return species.get(id)
  }
  lookup.all = species
  return lookup
}

/** A species entry the atlas can draw and name. */
const usable = (info) =>
  info?.icon !== undefined && info.palette !== undefined && info.name && info.dex !== undefined

/** The last national dex number on the page's roster: Gen I to VIII. */
const ROSTER_LAST = 905

/**
 * The page's roster: every species of Gen I to VIII, one entry per national
 * dex number (its plainest id: VENUSAUR, not VENUSAUR_MEGA), then the Alolan,
 * Galarian and Hisuian forms of those. Each with an icon and a name.
 */
const readRoster = (speciesOf) => {
  const byDex = new Map()
  const forms = []
  for (const [id, info] of speciesOf.all) {
    if (!usable(info) || info.dex < 1 || info.dex > ROSTER_LAST) continue
    if (info.form) {
      // PIKACHU_ALOLA is a cap, not a regional form.
      if (/_(ALOLA|GALAR|HISUI)(_STANDARD)?$/.test(id) && !id.startsWith("PIKACHU")) forms.push(id)
      continue
    }
    const kept = byDex.get(info.dex)
    if (!kept || id.length < kept.length) byDex.set(info.dex, id)
  }
  return {
    species: [...byDex].toSorted((a, b) => a[0] - b[0]).map(([, id]) => id),
    forms: forms.toSorted(),
  }
}

/** The weight of each slot, per method; `rod` for each rod. */
const readWeights = (game) => {
  const fields = json(game, "src/data/wild_encounters.json").wild_encounter_groups.find(
    (group) => group.label === "gWildMonHeaders",
  ).fields
  const rates = (type) => fields.find((field) => field.type === type).encounter_rates
  const rods = json(game, "src/data/standard_rod_fishing.json").qualityWeights
  return {
    land: rates("land_mons"),
    surf: rates("water_mons"),
    rock: rates("rock_smash_mons"),
    old: rods.OLD_ROD,
    good: rods.GOOD_ROD,
    super: rods.SUPER_ROD,
  }
}

/** Region map section ids to their names ("MAPSEC_ROUTE_101" is "ROUTE 101"), Hoenn's and the HNS ones. */
const readSectionNames = (game) => {
  const file = json(game, "src/data/region_map/region_map_sections.json")
  return new Map(
    [...file.map_sections, ...file.hns_map_sections].map((section) => [section.id, section.name]),
  )
}

/**
 * Builds encounters.json and icons.png in `outDir` for the placed `maps`
 * (names of map.json folders). Returns what it wrote, for the log.
 */
export const writeEncounters = (game, maps, outDir) => {
  const speciesOf = readSpecies(game)
  const weights = readWeights(game)
  const sections = readSectionNames(game)
  const tables = {}
  for (const region of REGION_FILES)
    Object.assign(tables, json(game, `src/data/wild_encounters_v2/${region}.json`))

  const slots = new Map() // species id -> atlas slot
  const slotOf = (id) => {
    if (!slots.has(id)) slots.set(id, slots.size)
    return slots.get(id)
  }
  /** Slots of a table to [atlas slot, percent] pairs, most likely first. */
  const share = (list, rates) => {
    if (!list?.length) return undefined
    const total = rates.reduce((a, b) => a + b, 0)
    const by = new Map()
    list.forEach((id, i) => by.set(id, (by.get(id) ?? 0) + rates[i]))
    return [...by]
      .toSorted((a, b) => b[1] - a[1])
      .map(([id, weight]) => [slotOf(id), Math.round((weight / total) * 100)])
  }

  const out = {}
  let withEncounters = 0
  for (const name of maps) {
    const map = game.byName[name]
    if (!map) continue
    const entry = { n: sections.get(map.region_map_section) ?? name }
    const table = tables[map.id]
    const methods = {}
    for (const [method, source, rates] of [
      ["land", table?.land, weights.land],
      ["surf", table?.surf, weights.surf],
      ["rock", table?.rock, weights.rock],
      ["old", table?.fish, weights.old],
      ["good", table?.fish, weights.good],
      ["super", table?.fish, weights.super],
    ]) {
      if (!source) continue
      const day = share(source.day, rates)
      const night = share(source.night, rates) ?? day
      if (!day) continue
      methods[method] =
        JSON.stringify(day) === JSON.stringify(night) ? { a: day } : { d: day, n: night }
    }
    if (Object.keys(methods).length) {
      entry.e = methods
      withEncounters++
    }
    out[name] = entry
  }

  // The full roster joins the atlas after the wild species, for the tour's
  // Pokédex waves: every species of Gen I to VIII and their regional forms.
  const roster = readRoster(speciesOf)
  const rosterSlots = {
    gens: Array.from({ length: 8 }, (_, i) =>
      roster.species.filter((id) => genOf(speciesOf(id).dex) === i + 1).map(slotOf),
    ),
    forms: roster.forms.map(slotOf),
  }

  // The icon atlas, in slot order.
  const ids = [...slots.keys()]
  const rows = Math.ceil(ids.length / COLUMNS)
  const width = COLUMNS * ICON
  const rgba = new Uint8Array(width * rows * ICON * 4)
  const palettes = Array.from({ length: 6 }, (_, i) =>
    readJascPalette(`${game.root}/graphics/pokemon/icon_palettes/pal${i}.pal`),
  )
  const missing = []
  const gens = []
  const names = ids.map((id, slot) => {
    const info = speciesOf(id)
    if (
      info?.icon === undefined ||
      info.palette === undefined ||
      !info.name ||
      info.dex === undefined
    )
      missing.push(id)
    else {
      gens.push(info.form ? FORM : genOf(info.dex))
      const png = readPng(`${game.root}/${info.icon}`)
      const palette = palettes[info.palette]
      for (let y = 0; y < ICON; y++)
        for (let x = 0; x < ICON; x++) {
          const index = png.samples[y * png.width + x]
          if (index === 0) continue
          rgba.set(
            [...palette[index].slice(0, 3), 255],
            ((Math.floor(slot / COLUMNS) * ICON + y) * width + (slot % COLUMNS) * ICON + x) * 4,
          )
        }
    }
    return info?.name ?? id
  })
  if (missing.length) throw new Error(`no icon or name for ${missing.join(", ")}`)

  fs.mkdirSync(outDir, { recursive: true })
  writePng(`${outDir}/icons.png`, width, rows * ICON, rgba)
  // The roster per generation, I to VIII, then the regional forms: the page's
  // Pokédex counts, and which icons its waves draw from.
  const counts = [...rosterSlots.gens.map((list) => list.length), rosterSlots.forms.length]
  fs.writeFileSync(
    `${outDir}/encounters.json`,
    JSON.stringify({
      columns: COLUMNS,
      species: names,
      gens,
      counts,
      roster: [...rosterSlots.gens, rosterSlots.forms],
      maps: out,
    }),
  )
  return { counts, maps: Object.keys(out).length, withEncounters, species: ids.length }
}

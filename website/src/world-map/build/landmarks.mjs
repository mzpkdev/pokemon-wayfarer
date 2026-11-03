// The tour's landmarks, read from the game: where each of the 24 Gym Badges is
// won, and where the three leagues of the league circuit are held. Positions
// are door tiles on the world grid, in metatiles.
import fs from "node:fs"
import { readTownMapCells } from "./game.mjs"
import { cellLookup, measureCell } from "./layout.mjs"

/** The three regions' badges, in the order the badge art sheet holds them. */
export const BADGE_REGIONS = ["Johto", "Kanto", "Hoenn"]
/** The league circuit in the order it is climbed (include/constants/league_circuit.h). */
const LEAGUES = [
  // A town map, the door in it, and the map that door leads to.
  // INDIGO PLATEAU's maps are not part of the world map, so it sits where its
  // Town Map cell puts it, off ROUTE 22's.
  { id: "indigo", name: "INDIGO PLATEAU", map: "MAP_INDIGO_PLATEAU_HNS", near: "MAP_ROUTE22_HNS" },
  {
    id: "masters",
    name: "SEVII MASTERS",
    map: "MAP_SEVEN_ISLAND",
    to: "MAP_SEVEN_ISLAND_HOUSE_ROOM1",
  },
  {
    id: "hoenn",
    name: "HOENN LEAGUE",
    map: "MAP_EVER_GRANDE_CITY",
    to: "MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F",
  },
]

/** "VIOLET CITY" from "VioletCity_hns", "MAHOGANY TOWN" from "Mahoganytown_hns". */
export const townName = (name) =>
  name
    .replace(/_.*$/, "")
    .replace(/([a-z])(?=[A-Z])/g, "$1 ")
    .replace(/([a-z])(town)$/, "$1 $2")
    .toUpperCase()

/**
 * Every map in the game tree (`every`, by id) and `doorTo`: the door in a
 * placed map that the way into a given map starts from, found by following
 * warps out through any halls and caves between. The world simulation's
 * notables use it to put indoor places (labs, Gyms, Game Corners) on the land.
 */
export const createDoors = (game, { all, minX, minY }) => {
  const { root } = game
  const placed = new Map(all.map((p) => [p.m.id, p]))

  // Every map in the tree, to follow warps from a gym back out to the land.
  const every = {}
  for (const dir of fs.readdirSync(`${root}/data/maps`)) {
    const file = `${root}/data/maps/${dir}/map.json`
    if (!fs.existsSync(file)) continue
    const map = JSON.parse(fs.readFileSync(file, "utf8"))
    every[map.id] = { ...map, dir }
  }
  // Where each map is entered from: a warp's destination, with the
  // Wayfarer override the game honors, leads back to the map holding the warp.
  const entrances = {}
  for (const map of Object.values(every))
    for (const warp of map.warp_events ?? [])
      (entrances[warp.wayfarer_dest_map ?? warp.dest_map] ??= []).push({
        from: map.id,
        x: warp.x,
        y: warp.y,
      })

  /** The door in a placed map the way into `target` starts from, found through any caves or halls between. */
  const doorTo = (target) => {
    const seen = new Set([target])
    const queue = [target]
    while (queue.length) {
      const current = queue.shift()
      for (const entrance of entrances[current] ?? []) {
        if (seen.has(entrance.from)) continue
        seen.add(entrance.from)
        const p = placed.get(entrance.from)
        if (p) return { p, x: p.x - minX + entrance.x + 0.5, y: p.y - minY + entrance.y + 0.5 }
        queue.push(entrance.from)
      }
    }
    return null
  }
  return { every, doorTo }
}

/**
 * `all` is the placed maps and `minX`/`minY` the world's origin, as
 * `placeWorld` returns them; `regionOf` names a placed map's region as the
 * explorer's labels do (JOHTO, KANTO, HOENN, ...).
 */
export const buildLandmarks = (game, world, regionOf) => {
  const { root } = game
  const { all, minX, minY } = world
  const placed = new Map(all.map((p) => [p.m.id, p]))
  const { every, doorTo } = createDoors(game, world)

  // A gym is a map whose script gives a badge (setflag FLAG_BADGEnn_GET). Each
  // region numbers its eight badges in the order they are earned, and the
  // Kanto and Hoenn maps number them from 1 or 9 on, so the place in a
  // region's order is the flag's number less one, modulo eight.
  // Each Gym's leader, from the notable routines: by the Gym map they keep,
  // or else by their badge (a Gym of several floors gives it on another).
  // `simulated` leaders walk the map (see notables.mjs); the rest have no
  // walking sprite and stay put.
  const routines = JSON.parse(
    fs.readFileSync(`${root}/tools/wayfarer_world/routines.json`, "utf8"),
  ).trainers.filter((t) => t.badge)
  // A leader the routines leave out (Tate & Liza) comes from the notable
  // trainers' catalog, by the Gym's first leader battle: TRAINER_TATE_AND_LIZA_1
  // is "tate-liza".
  const catalog = JSON.parse(
    fs.readFileSync(`${root}/tools/notable_trainers/catalog.json`, "utf8"),
  ).trainers
  // Every trainer's class and battle picture, from the party files, so a Gym's
  // leader battle gives the portrait the game shows ("Leader Brock Hns" is
  // graphics/trainers/front_pics/leader_brock_hns.png).
  const parties = new Map()
  for (const file of fs.readdirSync(`${root}/src/data`).filter((f) => f.endsWith(".party")))
    for (const block of fs
      .readFileSync(`${root}/src/data/${file}`, "utf8")
      .split(/^=== /m)
      .slice(1)) {
      const id = block.slice(0, block.indexOf(" ==="))
      const field = (name) => new RegExp(`^${name}: (.+)$`, "m").exec(block)?.[1].trim()
      parties.set(id, { name: field("Name"), class: field("Class") ?? "", pic: field("Pic") })
    }
  const picOf = (script) => {
    for (const [, id] of script.matchAll(/trainerbattle_\w+ (TRAINER_\w+)/g)) {
      const party = parties.get(id)
      if (party?.class.startsWith("Leader") && party.pic)
        return party.pic.toLowerCase().replaceAll(" ", "_")
    }
    return undefined
  }
  const leaderOf = (map, region, order, script) => {
    const t =
      routines.find((r) => r.gym === map.name) ??
      routines.find((r) => r.badge.region === region && r.badge.index === order)
    // Where the badge's map holds no battle with them (Clair gives hers after
    // the Dragon's Den), their own leader battle by name.
    const pic =
      picOf(script) ??
      [...parties.values()]
        .find((p) => p.class.startsWith("Leader") && p.pic && p.name === t?.name.toUpperCase())
        ?.pic?.toLowerCase()
        .replaceAll(" ", "_")
    if (t) return { name: t.name, slug: t.slug, simulated: Boolean(t.simulated), pic }
    const battle = /trainerbattle_\w+ TRAINER_([A-Z_]+?)_1\b/.exec(script)?.[1]
    const slug = battle?.toLowerCase().replaceAll("_and_", "_").replaceAll("_", "-")
    const entry = catalog.find((c) => c.slug === slug)
    if (!entry) return undefined
    // A duo stands on the map as its two halves (stationary-leaders.json):
    // "tate-liza" as TATE beside LIZA.
    const [first, second] = entry.slug.split("-")
    return entry.role.endsWith("duo")
      ? { name: entry.name, slug: first, also: second, simulated: false, pic }
      : { name: entry.name, slug: entry.slug, simulated: false, pic }
  }
  const gyms = []
  for (const map of Object.values(every)) {
    const file = `${root}/data/maps/${map.dir}/scripts.inc`
    if (!fs.existsSync(file)) continue
    const flags = [...fs.readFileSync(file, "utf8").matchAll(/setflag FLAG_BADGE(\d+)_GET/g)].map(
      (m) => Number(m[1]),
    )
    if (!flags.length) continue
    const door = doorTo(map.id)
    // The Viridian gym is reached twice over (a Kanto-only copy); a gym no
    // town leads to is not part of the world.
    if (!door) continue
    const region = regionOf(door.p)
    const order = (flags[0] - 1) % 8
    gyms.push({
      region: region[0] + region.slice(1).toLowerCase(),
      order,
      town: townName(door.p.m.name),
      x: door.x,
      y: door.y,
      leader: leaderOf(
        map,
        region[0] + region.slice(1).toLowerCase(),
        order,
        fs.readFileSync(file, "utf8"),
      ),
    })
  }
  const badges = gyms
    .map((gym) => ({ ...gym, badge: BADGE_REGIONS.indexOf(gym.region) * 8 + gym.order }))
    .toSorted((a, b) => a.badge - b.badge)
  if (badges.length !== 24 || badges.some((gym, i) => gym.badge !== i))
    throw new Error(`expected 24 gyms, one per badge: ${badges.map((g) => g.badge).join(",")}`)

  const cellOf = cellLookup(readTownMapCells(game))
  const cell = measureCell(
    all.filter((p) => p.region === "Hoenn"),
    cellOf,
  )
  const circuit = LEAGUES.map(({ id, name, map, to, near }) => {
    if (near) {
      const from = placed.get(near)
      const there = cellOf(every[map])
      const here = cellOf(from.m)
      return {
        id,
        name,
        x: from.x - minX + from.l.width / 2 + (there.x - here.x) * cell,
        y: from.y - minY + from.l.height / 2 + (there.y - here.y) * cell,
      }
    }
    const p = placed.get(map)
    const warp = every[map].warp_events.find((w) => (w.wayfarer_dest_map ?? w.dest_map) === to)
    if (!p || !warp) throw new Error(`no door from ${map} to ${to}`)
    return { id, name, x: p.x - minX + warp.x + 0.5, y: p.y - minY + warp.y + 0.5 }
  })
  return {
    badges: badges.map(({ badge, region, town, x, y, leader }) => ({
      badge,
      region,
      town,
      x,
      y,
      ...(leader ? { leader } : {}),
    })),
    circuit,
  }
}

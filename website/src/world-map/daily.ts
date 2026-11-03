// The daily world slots, as `build/daily.mjs` writes them: `daily.json` (the
// game's trainer slots and groups, item spots and pools) and `trainers.png`
// (one cell per sprite). `dailyOn` replays the game's own draws for a day, so
// the tour shows the routes as they change from day to day. The rules are in
// .product/specs/daily-world-slots.md; the draws are game/src/
// wayfarer_daily_slots.c (Hash32, DailySlots_Roll, DailyItems_ResolveSpotTier)
// and wayfarer_daily_trainers.c (ResolveMembers).
import { DATA } from "./data"

/** A trainer slot: one daily trainer (or pair) and the spot it stands at. */
export type DailySlot = {
  /** The home trainer's name with its class, as the game shows it ("BUG CATCHER WADE"). */
  n: string
  /** Each object's sprite: a cell of `trainers.png`. */
  s: number[]
  /** Its group, an index into `groups`; -1 for a home range, which is the slot alone. */
  g: number
  /** The chance in percent that the slot is empty on a given day. */
  a: number
  /** Set for a pair: two objects, one double-battle trainer. */
  p?: 1
  /** The placed map (as in `Meta.maps`) and each object's world grid cell; missing where the world does not place the map. */
  m?: string
  o?: [x: number, y: number][]
}

/** An item spot on a placed map. */
export type DailySpot = {
  /** The spot's index in the game's table, which its draws are keyed by. */
  i: number
  m: string
  x: number
  y: number
  /** Reach tier (0 road, 1 wilds, 2 outlands) and region (0 Kanto, 1 Sevii, 2 Johto, 3 Alola, 4 Sinjoh, 5 Hoenn). */
  t: number
  r: number
  /** Set for a hidden item; a ball otherwise. */
  h?: 1
  /** The prize's index in `items` for a prize spot, which lies there once; missing for a dynamic spot. */
  z?: number
}

export type Daily = {
  /** `trainers.png`: the side of a square cell in pixels (a sprite stands bottom-centred in it), cells per row, and the item ball's cell. */
  sheet: { cell: number; columns: number; itemBall: number }
  slots: DailySlot[]
  /** Each group's slots in the game's fixed order (slot indices), including slots on maps the world does not place. */
  groups: number[][]
  spots: DailySpot[]
  /** Item names, which the pools and prizes index. */
  items: string[]
  /** The percent of days a spot is empty. */
  itemEmpty: number
  /** Per reach tier, the cumulative percent below which a find is Regular, then Better (Special otherwise). */
  tierOdds: [number, number][]
  /** Per region and pool tier (Regular, Better, Special), the entries as [item, weight]; item -1 stands for any stone and -2 for any TM. */
  pools: [item: number, weight: number][][][]
  stones: number[]
  tms: number[]
}

/** A trainer standing in a slot today, and where its objects are. */
export type DailyTrainer = {
  /** The slot it stands in. */
  slot: number
  x: number
  y: number
  sprite: number
  name: string
  objects: { x: number; y: number; sprite: number }[]
}

export type DailyDay = {
  trainers: DailyTrainer[]
  items: { spot: number; x: number; y: number; hidden: boolean; item: string }[]
  /** The cells of slots that are empty today. */
  empty: { x: number; y: number }[]
}

/** The seed the showcase uses: any trainer ID will do, this one is only fixed. */
export const DAILY_SEED = 0x5a17_c0de

const loading = new Map<string, Promise<Daily>>()

/** Loads the daily slots once per base path, the first time the tour needs them. */
export const loadDaily = (base = DATA): Promise<Daily> => {
  let daily = loading.get(base)
  if (!daily) {
    daily = fetch(`${base}/daily.json`).then((response) => {
      if (!response.ok) throw new Error(`daily.json: ${response.status}`)
      return response.json() as Promise<Daily>
    })
    // A failed load can be tried again.
    daily.catch(() => loading.delete(base))
    loading.set(base, daily)
  }
  return daily
}

/** Kinds of draw, in the order of DailyRollKind (the hash takes the kind). */
const ITEM_EMPTY = 0
const ITEM_TIER = 1
const ITEM_PICK = 2
const ITEM_GROUP = 3
const TRAINER_EMPTY = 4
const TRAINER_GROUP = 5

const rotateRight = (value: number, bits: number) => (value >>> bits) | (value << (32 - bits))

const mixIn = (h: number, k: number) => {
  k = Math.imul(k, 0xcc9e2d51)
  k = rotateRight(k, 17)
  k = Math.imul(k, 0x1b873593)
  h ^= k
  h = rotateRight(h, 19)
  return (Math.imul(h, 5) + 0xe6546b64) | 0
}

/** The game's Hash32: murmur3's block step and finaliser over five u32 inputs. */
export const hash32 = (seed: number, day: number, kind: number, a: number, b: number) => {
  let h = seed | 0
  for (const k of [day, kind, a, b]) h = mixIn(h, k)
  h ^= h >>> 16
  h = Math.imul(h, 0x85ebca6b)
  h ^= h >>> 13
  h = Math.imul(h, 0xc2b2ae35)
  h ^= h >>> 16
  return h >>> 0
}

/**
 * The slots of the groups that have a slot on `maps`, resolved for `day` with
 * the save's `seed` (its trainer ID). The save is one that has beaten every
 * trainer in those maps' groups before, so every trainer is a wanderer, none
 * was first beaten today, and no wanderer is ready for a rematch or holds a
 * gift (those are guaranteed a place, which needs the phone's state). Every
 * prize is taken, so a prize spot rolls like a dynamic one: the showcase is
 * the daily churn. Nothing is picked up, so no spot or slot is cleared.
 */
export const dailyOn = (daily: Daily, maps: string[], day: number, seed: number): DailyDay => {
  const shown = new Set(maps)
  // Draws key on the day's low 16 bits, the saved stamp.
  const stamp = day & 0xffff
  const roll = (kind: number, a: number, b: number) => hash32(seed, stamp, kind, a, b)

  const trainers: DailyTrainer[] = []
  const empty: DailyDay["empty"] = []
  // Resolves a range (a group, or a home slot alone): the occupant of each
  // member slot, or -1 when it is empty. As ResolveMembers does with every
  // member rotating: the wanderer with the k-th smallest key stands in the
  // k-th slot (ties by position), and a slot is empty at its absence chance.
  const resolve = (members: number[], seedKey: number) => {
    const groupSeed = roll(TRAINER_GROUP, seedKey, 0)
    const keys = members.map((slot) => hash32(groupSeed, 0, TRAINER_GROUP, slot, 0))
    const order = members
      .map((_, j) => j)
      .toSorted((j, k) => (keys[j]! < keys[k]! ? -1 : keys[j]! > keys[k]! ? 1 : j - k))
    return members.map((slot, k) =>
      roll(TRAINER_EMPTY, slot, 0) % 100 < daily.slots[slot]!.a ? -1 : members[order[k]!]!,
    )
  }
  const placeRange = (members: number[], seedKey: number) => {
    const occupants = resolve(members, seedKey)
    members.forEach((slot, k) => {
      const at = daily.slots[slot]!
      if (!at.m || !shown.has(at.m)) return
      const objects = at.o!.map(([x, y]) => ({ x, y }))
      const occupant = occupants[k]!
      if (occupant < 0) {
        empty.push(...objects)
        return
      }
      const who = daily.slots[occupant]!
      trainers.push({
        slot,
        x: objects[0]!.x,
        y: objects[0]!.y,
        sprite: who.s[0]!,
        name: who.n,
        objects: objects.map((object, i) => ({ ...object, sprite: who.s[i] ?? who.s[0]! })),
      })
    })
  }
  const done = new Set<number>()
  daily.slots.forEach((slot, index) => {
    if (!slot.m || !shown.has(slot.m)) return
    // A home range is its slot alone, seeded by the slot.
    if (slot.g < 0) return placeRange([index], index)
    if (done.has(slot.g)) return
    done.add(slot.g)
    placeRange(daily.groups[slot.g]!, slot.g)
  })

  const items: DailyDay["items"] = []
  for (const spot of daily.spots) {
    if (!shown.has(spot.m)) continue
    if (roll(ITEM_EMPTY, spot.i, 0) % 100 < daily.itemEmpty) continue
    const odds = daily.tierOdds[spot.t]!
    const pick = roll(ITEM_TIER, spot.i, 0) % 100
    const tier = pick < odds[0] ? 0 : pick < odds[1] ? 1 : 2
    const pool = daily.pools[spot.r]![tier]!
    let weight = roll(ITEM_PICK, spot.i, tier) % pool.reduce((sum, [, w]) => sum + w, 0)
    let [item] = pool.find(([, w]) => (weight < w ? true : ((weight -= w), false)))!
    // A group entry picks among its members equally.
    if (item < 0) {
      const group = item === -1 ? daily.stones : daily.tms
      item = group[roll(ITEM_GROUP, spot.i, tier) % group.length]!
    }
    items.push({ spot: spot.i, x: spot.x, y: spot.y, hidden: !!spot.h, item: daily.items[item]! })
  }
  return { trainers, items, empty }
}

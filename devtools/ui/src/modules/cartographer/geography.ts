import type { CatalogMap } from "./catalog.js"

export type CardinalDirection = "up" | "down" | "left" | "right"

export type Placement = {
  x: number
  y: number
  width: number
  height: number
}

export type Geography = {
  placements: Record<string, Placement>
  components: Array<{ id: string; maps: string[]; bounds: Placement }>
  overlaps: AtlasLayoutOverlap[]
}

export type AtlasLayoutOverlap = {
  maps: [string, string]
  area: Placement
}

const cardinalDirections = new Set<CardinalDirection>(["up", "down", "left", "right"])
const componentGap = 8

const dimensions = (map: CatalogMap): Pick<Placement, "width" | "height"> => {
  return { width: map.layout.widthMetatiles, height: map.layout.heightMetatiles }
}

const placeConnection = (
  source: Placement,
  destination: Pick<Placement, "width" | "height">,
  direction: CardinalDirection,
  offset: number,
): Placement => {
  if (direction === "right")
    return { x: source.x + source.width, y: source.y + offset, ...destination }
  if (direction === "left")
    return { x: source.x - destination.width, y: source.y + offset, ...destination }
  if (direction === "down")
    return { x: source.x + offset, y: source.y + source.height, ...destination }
  return { x: source.x + offset, y: source.y - destination.height, ...destination }
}

const placeSourceFromDestination = (
  destination: Placement,
  source: Pick<Placement, "width" | "height">,
  direction: CardinalDirection,
  offset: number,
): Placement => {
  if (direction === "right")
    return { x: destination.x - source.width, y: destination.y - offset, ...source }
  if (direction === "left")
    return { x: destination.x + destination.width, y: destination.y - offset, ...source }
  if (direction === "down")
    return { x: destination.x - offset, y: destination.y - source.height, ...source }
  return { x: destination.x - offset, y: destination.y + destination.height, ...source }
}

const boundsFor = (names: readonly string[], placements: Record<string, Placement>): Placement => {
  const firstName = names[0]
  if (!firstName) throw new Error("a map component cannot be empty")
  const first = placements[firstName]!
  let left = first.x
  let top = first.y
  let right = first.x + first.width
  let bottom = first.y + first.height
  for (const name of names.slice(1)) {
    const placement = placements[name]!
    left = Math.min(left, placement.x)
    top = Math.min(top, placement.y)
    right = Math.max(right, placement.x + placement.width)
    bottom = Math.max(bottom, placement.y + placement.height)
  }
  return { x: left, y: top, width: right - left, height: bottom - top }
}

const atlasOverlapsFor = (
  names: readonly string[],
  placements: Record<string, Placement>,
): AtlasLayoutOverlap[] => {
  const overlaps: AtlasLayoutOverlap[] = []
  for (const [index, firstName] of names.entries()) {
    const first = placements[firstName]!
    for (const secondName of names.slice(index + 1)) {
      const second = placements[secondName]!
      const left = Math.max(first.x, second.x)
      const top = Math.max(first.y, second.y)
      const right = Math.min(first.x + first.width, second.x + second.width)
      const bottom = Math.min(first.y + first.height, second.y + second.height)
      if (left >= right || top >= bottom) continue
      overlaps.push({
        maps: [firstName, secondName],
        area: { x: left, y: top, width: right - left, height: bottom - top },
      })
    }
  }
  return overlaps
}

/** The default cartographer only places surface maps that the catalog exposes by default. */
export const visibleSurfaceMaps = (maps: readonly CatalogMap[]): CatalogMap[] => {
  return maps.filter((map) => map.world.layer === "surface" && map.world.defaultVisible)
}

/** Resolve cardinal source connections into a deterministic packed map layout. */
export const solveGeography = (maps: readonly CatalogMap[]): Geography => {
  const ordered = [...maps].sort((left, right) => left.name.localeCompare(right.name, "en"))
  const byName = new Map(ordered.map((map) => [map.name, map]))
  const placements: Record<string, Placement> = {}
  const components: Array<{ id: string; maps: string[]; bounds: Placement }> = []

  const connections = new Map<
    string,
    Array<{ name: string; direction: CardinalDirection; offset: number; forward: boolean }>
  >()
  for (const map of ordered) {
    for (const connection of map.connections) {
      if (
        !connection.destinationMap ||
        !cardinalDirections.has(connection.direction as CardinalDirection)
      )
        continue
      const direction = connection.direction as CardinalDirection
      const forward = connections.get(map.name) ?? []
      forward.push({
        name: connection.destinationMap,
        direction,
        offset: connection.offsetMetatiles,
        forward: true,
      })
      connections.set(map.name, forward)
      const reverse = connections.get(connection.destinationMap) ?? []
      reverse.push({
        name: map.name,
        direction,
        offset: connection.offsetMetatiles,
        forward: false,
      })
      connections.set(connection.destinationMap, reverse)
    }
  }

  for (const root of ordered) {
    if (placements[root.name]) continue
    placements[root.name] = { x: 0, y: 0, ...dimensions(root) }
    const queue = [root.name]
    const names: string[] = []
    for (let index = 0; index < queue.length; index += 1) {
      const name = queue[index]!
      names.push(name)
      const current = placements[name]!
      for (const neighbor of connections.get(name) ?? []) {
        const neighborMap = byName.get(neighbor.name)
        if (!neighborMap) continue
        const expected = neighbor.forward
          ? placeConnection(current, dimensions(neighborMap), neighbor.direction, neighbor.offset)
          : placeSourceFromDestination(
              current,
              dimensions(neighborMap),
              neighbor.direction,
              neighbor.offset,
            )
        const actual = placements[neighbor.name]
        if (!actual) {
          placements[neighbor.name] = expected
          queue.push(neighbor.name)
        }
      }
    }
    names.sort((left, right) => left.localeCompare(right, "en"))
    components.push({ id: names[0]!, maps: names, bounds: boundsFor(names, placements) })
  }

  const packed = [...components].sort(
    (left, right) =>
      right.bounds.height - left.bounds.height ||
      right.bounds.width - left.bounds.width ||
      left.id.localeCompare(right.id, "en"),
  )
  const targetWidth = Math.max(
    1,
    Math.ceil(
      Math.sqrt(
        packed.reduce(
          (total, component) => total + component.bounds.width * component.bounds.height,
          0,
        ),
      ),
    ),
  )
  let x = 0
  let y = 0
  let shelfHeight = 0
  for (const component of packed) {
    if (x > 0 && x + component.bounds.width > targetWidth) {
      x = 0
      y += shelfHeight + componentGap
      shelfHeight = 0
    }
    const shiftX = x - component.bounds.x
    const shiftY = y - component.bounds.y
    for (const name of component.maps) {
      const placement = placements[name]!
      placements[name] = { ...placement, x: placement.x + shiftX, y: placement.y + shiftY }
    }
    component.bounds = boundsFor(component.maps, placements)
    x += component.bounds.width + componentGap
    shelfHeight = Math.max(shelfHeight, component.bounds.height)
  }
  return {
    placements,
    components: packed,
    overlaps: atlasOverlapsFor(
      ordered.map((map) => map.name),
      placements,
    ),
  }
}

export const toOpenLayersExtent = (
  placement: Placement,
  pixelsPerMetatile: number,
): [number, number, number, number] => {
  const x = placement.x * pixelsPerMetatile
  const y = placement.y * pixelsPerMetatile
  const width = placement.width * pixelsPerMetatile
  const height = placement.height * pixelsPerMetatile
  return [x, -(y + height), x + width, -y]
}

export const cartographerExtent = (
  placements: Record<string, Placement>,
  pixelsPerMetatile: number,
): [number, number, number, number] | null => {
  const values = Object.values(placements)
  if (values.length === 0) return null
  const first = toOpenLayersExtent(values[0]!, pixelsPerMetatile)
  let [minX, minY, maxX, maxY] = first
  for (const placement of values.slice(1)) {
    const [left, bottom, right, top] = toOpenLayersExtent(placement, pixelsPerMetatile)
    minX = Math.min(minX, left)
    minY = Math.min(minY, bottom)
    maxX = Math.max(maxX, right)
    maxY = Math.max(maxY, top)
  }
  return [minX, minY, maxX, maxY]
}

import type {
  CatalogMap,
  CatalogPlacement,
  TopologyConnectionRecord,
  TopologyDiagnostic,
  TopologySourceHeader,
} from "./types"

type CardinalDirection = "up" | "down" | "left" | "right"

type ConnectionRecord = TopologyConnectionRecord & {
  connectionIndex: number
}

const cardinalDirections = new Set<CardinalDirection>(["up", "down", "left", "right"])

const dimensions = (map: CatalogMap): Pick<CatalogPlacement, "width" | "height"> => {
  return { width: map.layout.widthMetatiles, height: map.layout.heightMetatiles }
}

const placeConnection = (
  source: CatalogPlacement,
  destination: Pick<CatalogPlacement, "width" | "height">,
  direction: CardinalDirection,
  offset: number,
): CatalogPlacement => {
  if (direction === "right")
    return { x: source.x + source.width, y: source.y + offset, ...destination }
  if (direction === "left")
    return { x: source.x - destination.width, y: source.y + offset, ...destination }
  if (direction === "down")
    return { x: source.x + offset, y: source.y + source.height, ...destination }
  return { x: source.x + offset, y: source.y - destination.height, ...destination }
}

const placeSourceFromDestination = (
  destination: CatalogPlacement,
  source: Pick<CatalogPlacement, "width" | "height">,
  direction: CardinalDirection,
  offset: number,
): CatalogPlacement => {
  if (direction === "right")
    return { x: destination.x - source.width, y: destination.y - offset, ...source }
  if (direction === "left")
    return { x: destination.x + destination.width, y: destination.y - offset, ...source }
  if (direction === "down")
    return { x: destination.x - offset, y: destination.y - source.height, ...source }
  return { x: destination.x - offset, y: destination.y + destination.height, ...source }
}

const headerFor = (map: string, connectionIndex: number): TopologySourceHeader => {
  return {
    map,
    path: `data/maps/${map}/map.json`,
    pointer: `/connections/${connectionIndex}`,
  }
}

const oppositeDirection = (direction: CardinalDirection): CardinalDirection => {
  if (direction === "up") return "down"
  if (direction === "down") return "up"
  if (direction === "left") return "right"
  return "left"
}

const inverseOffset = (offset: number): number => {
  return offset === 0 ? 0 : -offset
}

const recordKey = (record: ConnectionRecord): string => {
  return `${record.source.map}:${record.connectionIndex}`
}

const asConnection = ({
  connectionIndex: _connectionIndex,
  ...connection
}: ConnectionRecord): TopologyConnectionRecord => {
  return connection
}

const directMismatch = (
  connection: ConnectionRecord,
  reverseConnection: ConnectionRecord,
  mapsByName: ReadonlyMap<string, CatalogMap>,
): TopologyDiagnostic => {
  const source = { x: 0, y: 0, ...dimensions(mapsByName.get(connection.source.map)!) }
  const destination = dimensions(mapsByName.get(connection.destination.map)!)
  const forwardPlacement = placeConnection(
    source,
    destination,
    connection.direction,
    connection.offsetMetatiles,
  )
  const reversePlacement = placeSourceFromDestination(
    source,
    destination,
    reverseConnection.direction,
    reverseConnection.offsetMetatiles,
  )
  return {
    code: "direct_connection_mismatch",
    explanation: `${connection.source.map} declares ${connection.destination.map} ${connection.direction} at offset ${connection.offsetMetatiles}, but ${reverseConnection.source.map}'s reverse record declares ${reverseConnection.direction} at offset ${reverseConnection.offsetMetatiles}.`,
    connection: asConnection(connection),
    reverseConnection: asConnection(reverseConnection),
    expectedReverse: {
      direction: oppositeDirection(connection.direction),
      offsetMetatiles: inverseOffset(connection.offsetMetatiles),
    },
    forwardPlacement,
    reversePlacement,
  }
}

const missingReverseConnection = (connection: ConnectionRecord): TopologyDiagnostic => {
  return {
    code: "missing_reverse_connection",
    explanation: `${connection.source.map} declares ${connection.destination.map} ${connection.direction} at offset ${connection.offsetMetatiles}, but ${connection.destination.map} has no distinct reverse connection record back to ${connection.source.map}.`,
    connection: asConnection(connection),
    expectedReverse: {
      direction: oppositeDirection(connection.direction),
      offsetMetatiles: inverseOffset(connection.offsetMetatiles),
    },
  }
}

/**
 * Reports only contradictory reciprocal records. Graph cycles are valid map topology and
 * cannot identify faulty source data by themselves, so they are intentionally not diagnostics.
 */
export const topologyConflicts = (maps: readonly CatalogMap[]): TopologyDiagnostic[] => {
  const ordered = [...maps].sort((left, right) => left.name.localeCompare(right.name, "en"))
  const mapsByName = new Map(ordered.map((map) => [map.name, map]))
  const records = ordered.flatMap((map) =>
    map.connections.flatMap((connection, connectionIndex) => {
      if (
        !connection.destinationMap ||
        !mapsByName.has(connection.destinationMap) ||
        !cardinalDirections.has(connection.direction as CardinalDirection)
      )
        return []
      return [
        {
          source: {
            map: map.name,
            mapId: map.id,
            header: headerFor(map.name, connectionIndex),
          },
          destination: {
            map: connection.destinationMap,
            mapId: mapsByName.get(connection.destinationMap)!.id,
          },
          direction: connection.direction as CardinalDirection,
          offsetMetatiles: connection.offsetMetatiles,
          connectionIndex,
        },
      ]
    }),
  )
  const recordsBySource = new Map<string, ConnectionRecord[]>()
  for (const record of records) {
    const sourceRecords = recordsBySource.get(record.source.map) ?? []
    sourceRecords.push(record)
    recordsBySource.set(record.source.map, sourceRecords)
  }

  const diagnostics: TopologyDiagnostic[] = []
  const pairedRecords = new Set<string>()
  for (const record of records) {
    if (pairedRecords.has(recordKey(record))) continue
    const reverseCandidates = (recordsBySource.get(record.destination.map) ?? [])
      .filter((candidate) => candidate.destination.map === record.source.map)
      .sort((left, right) => recordKey(left).localeCompare(recordKey(right), "en"))
    const availableReverseCandidates = reverseCandidates.filter(
      (candidate) => !pairedRecords.has(recordKey(candidate)),
    )
    if (availableReverseCandidates.length === 0) {
      diagnostics.push(missingReverseConnection(record))
      pairedRecords.add(recordKey(record))
      continue
    }
    const matchingReverse = availableReverseCandidates.find(
      (candidate) =>
        candidate.direction === oppositeDirection(record.direction) &&
        candidate.offsetMetatiles === inverseOffset(record.offsetMetatiles),
    )
    const reverse = matchingReverse ?? availableReverseCandidates[0]!
    pairedRecords.add(recordKey(record))
    pairedRecords.add(recordKey(reverse))
    if (!matchingReverse) diagnostics.push(directMismatch(record, reverse, mapsByName))
  }
  return diagnostics
}

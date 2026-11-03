import { useEffect, useState } from "react"

import { DATA, type Meta, loadWorld } from "./data"
import { BADGES_SRC, type Landmarks, loadLandmarks } from "./landmarks"
import { type LivingWorld, loadLiving } from "./living"
import type { PlaceInfo } from "./PlaceCard"

/**
 * What else a place card can say about a town: the Gym and its badge, a
 * stop on the league circuit, and the ferries that call there.
 */
export const PlaceExtra = ({ place, base = DATA }: { place: PlaceInfo; base?: string }) => {
  const [landmarks, setLandmarks] = useState<Landmarks>()
  const [living, setLiving] = useState<LivingWorld>()
  const [meta, setMeta] = useState<Meta>()
  useEffect(() => {
    void loadWorld(base).then(
      (world) => setMeta(world.meta),
      () => {},
    )
    void loadLandmarks(base).then(setLandmarks)
    void loadLiving(base).then(setLiving, () => {})
  }, [base])

  const name = place.name.toUpperCase()
  // A Gym or league stop belongs to the map its door is on.
  const rect = meta?.maps.find((map) => map.name === place.map)
  const on = ({ x, y }: { x: number; y: number }) =>
    !!rect && x >= rect.x && x <= rect.x + rect.width && y >= rect.y && y <= rect.y + rect.height
  const gym = landmarks?.badges.find((badge) => badge.town === name || on(badge))
  const league = landmarks?.circuit.find(on)
  // Ports are named by their town's first word ("Olivine" for OLIVINE CITY).
  const ports = new Set(
    Object.entries(living?.ports ?? {})
      .filter(([, port]) => name.split(" ")[0] === port.name.toUpperCase().split(" ")[0])
      .map(([id]) => id),
  )
  const ships = living?.ferry.ships.filter((ship) => ship.ports.some((id) => ports.has(id))) ?? []
  if (!gym && !league && ships.length === 0) return null

  return (
    <ul className="mt-3 grid gap-2 text-paper">
      {gym && (
        <li className="flex items-center gap-3">
          <span
            aria-hidden
            className="inline-block size-8 shrink-0 [image-rendering:pixelated]"
            style={{
              backgroundImage: `url(${BADGES_SRC})`,
              backgroundSize: `${24 * 32}px 32px`,
              backgroundPosition: `${-gym.badge * 32}px 0`,
            }}
          />
          <span className="pixel-label text-[16px] leading-none">
            {gym.region.toUpperCase()} GYM · BADGE {(gym.badge % 8) + 1}
          </span>
        </li>
      )}
      {league && (
        <li className="pixel-label text-[16px] leading-none text-logo-yellow">
          LEAGUE CIRCUIT · {league.name.toUpperCase()}
        </li>
      )}
      {ships.length > 0 && (
        <li className="pixel-label text-[16px] leading-none">
          FERRY PORT · {ships.map((ship) => ship.name).join(", ")}
        </li>
      )}
    </ul>
  )
}

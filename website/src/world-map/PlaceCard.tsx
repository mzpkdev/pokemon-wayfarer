import { type ReactNode, type Ref, useState } from "react"

import { ICON, type Encounters, type Method, type Place, type Slot, type Table } from "./encounters"

/** What the explorer knows about the picked place, for the `extra` slot. */
export type PlaceInfo = {
  /** The map's folder name in the game, as in `Meta.maps`. */
  map: string
  /** Its name in the game, e.g. "LITTLEROOT TOWN". */
  name: string
}

const METHODS: [Method, string][] = [
  ["land", "ON FOOT"],
  ["surf", "SURFING"],
  ["rock", "ROCK SMASH, HEADBUTT"],
  ["old", "OLD ROD"],
  ["good", "GOOD ROD"],
  ["super", "SUPER ROD"],
]

const slotsOf = (table: Table, night: boolean): Slot[] =>
  "a" in table ? table.a : night ? table.n : table.d

const Row = ({ slot, encounters, base }: { slot: Slot; encounters: Encounters; base: string }) => {
  const [species, percent] = slot
  const { columns } = encounters
  return (
    <li className="flex items-center gap-2">
      <span
        aria-hidden="true"
        className="pixelated block shrink-0"
        style={{
          width: ICON,
          height: ICON,
          backgroundImage: `url(${base}/icons.png)`,
          backgroundSize: `${columns * ICON}px auto`,
          backgroundPosition: `-${(species % columns) * ICON}px -${Math.floor(species / columns) * ICON}px`,
        }}
      />
      <span className="gba-text gba-text-light min-w-0 flex-1 truncate [--px:1px]">
        {encounters.species[species]}
      </span>
      <span className="gba-text gba-text-light shrink-0 [--px:1px]">{percent}%</span>
    </li>
  )
}

/**
 * A place's card: its name, a slot for `extra` (a town's Gym, ports and so on)
 * and, with the debug menu's `rates`, its wild Pokémon by method and time of
 * day with their exact rates. Without them the map shows its Pokémon close
 * in. A bottom sheet on a phone, a panel on the left from `sm` up.
 */
export const PlaceCard = ({
  ref,
  place,
  info,
  encounters,
  base,
  failed,
  rates,
  extra,
  onClose,
}: {
  ref?: Ref<HTMLElement>
  place: Place | undefined
  info: PlaceInfo
  encounters: Encounters | undefined
  base: string
  failed: boolean
  /** Show the exact encounter tables (the debug menu's option). */
  rates: boolean
  extra?: (place: PlaceInfo) => ReactNode
  onClose: () => void
}) => {
  const [night, setNight] = useState(false)
  const tables = METHODS.flatMap(([method, label]) => {
    const table = place?.e?.[method]
    return table ? [{ method, label, table }] : []
  })
  const timed = tables.some(({ table }) => "d" in table)
  const slot = extra?.(info)
  return (
    <section
      ref={ref}
      aria-label={info.name}
      className="pixel-corners pixel-panel absolute inset-x-0 bottom-0 flex max-h-[46%] flex-col gap-3 p-5 text-paper sm:inset-x-auto sm:top-4 sm:bottom-auto sm:left-4 sm:max-h-[calc(100%-2rem)] sm:w-80"
    >
      <div className="flex items-start justify-between gap-3">
        <h2 className="pixel-label min-w-0 text-sign [overflow-wrap:anywhere]">{info.name}</h2>
        <button
          type="button"
          onClick={onClose}
          aria-label="Close place card"
          className="pixel-label -mt-1 -mr-2 shrink-0 px-2 text-paper/70 hover:text-paper focus-visible:text-paper focus-visible:underline focus-visible:outline-none"
        >
          ×
        </button>
      </div>
      {slot}
      <div className="flex min-h-0 flex-1 flex-col gap-3 overflow-y-auto overscroll-contain">
        {failed && rates && (
          <p className="gba-text gba-text-light [--px:1px]">Couldn't load this place's Pokémon.</p>
        )}
        {encounters && !rates && tables.length > 0 && (
          <p className="gba-text gba-text-light [--px:1px]">
            Wild Pokémon live here. Zoom in to spot them in the grass and water.
          </p>
        )}
        {encounters && rates && tables.length === 0 && (
          <p className="gba-text gba-text-light [--px:1px]">No wild Pokémon here.</p>
        )}
        {rates && timed && (
          <fieldset className="flex gap-2">
            <legend className="sr-only">Time of day</legend>
            {(
              [
                [false, "DAY", "Morning and day"],
                [true, "NIGHT", "Evening and night"],
              ] as const
            ).map(([isNight, label, title]) => (
              <button
                key={label}
                type="button"
                title={title}
                aria-pressed={night === isNight}
                onClick={() => setNight(isNight)}
                className="gba-text gba-text-light px-3 py-1 [--px:1px] shadow-[inset_0_0_0_2px_rgb(255_255_255/0.25)] aria-pressed:bg-sign aria-pressed:[--ink:var(--color-ink)] aria-pressed:[--shadow:transparent] aria-pressed:shadow-none focus-visible:outline-2 focus-visible:outline-sign"
              >
                {label}
              </button>
            ))}
          </fieldset>
        )}
        {encounters &&
          rates &&
          tables.map(({ method, label, table }) => (
            <div key={method}>
              <h3 className="gba-text mb-1 [--ink:var(--color-sign)] [--px:1px] [--shadow:rgb(0_16_0)]">
                {label}
              </h3>
              <ul className="flex flex-col">
                {slotsOf(table, night).map((entry) => (
                  <Row key={entry[0]} slot={entry} encounters={encounters} base={base} />
                ))}
              </ul>
            </div>
          ))}
      </div>
    </section>
  )
}

import { useEffect, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"

/** Story beats that stand alone, by their map name popups: two from Kanto, two from Johto. */
const BEATS = ["MT. MOON", "SLOWPOKE WELL", "SILPH CO.", "RADIO TOWER"]

/** One save's order after another's, each a permutation of BEATS. */
const ORDERS = [
  [0, 1, 2, 3],
  [3, 0, 1, 2],
  [1, 3, 2, 0],
  [2, 1, 0, 3],
]

/** A popup's height plus the gap below it, in GBA pixels. */
const ROW = 46

/**
 * The game's wooden map name popups, numbered, reshuffling every few seconds
 * into another save's order: the same story beats, met in any order. Each
 * popup slides to its new place in the popup's own 20-step motion. Decorative:
 * the copy beside it says the same.
 */
export const StoryShuffle = () => {
  const reducedMotion = useReducedMotion()
  const [turn, setTurn] = useState(0)

  useEffect(() => {
    if (reducedMotion) return
    const timer = window.setInterval(() => setTurn((count) => (count + 1) % ORDERS.length), 2600)
    return () => window.clearInterval(timer)
  }, [reducedMotion])

  const order = ORDERS[turn]!

  return (
    <div
      className="relative [--px:2px]"
      style={{
        width: "calc(120 * var(--px))",
        height: `calc(${ROW * BEATS.length - 6} * var(--px))`,
      }}
      aria-hidden="true"
    >
      {BEATS.map((_, place) => (
        <span
          key={place}
          className="gba-text gba-text-light absolute left-0 grid place-items-center"
          style={{
            top: `calc(${ROW * place} * var(--px))`,
            width: "calc(16 * var(--px))",
            height: "calc(40 * var(--px))",
          }}
        >
          {place + 1}
        </span>
      ))}
      {BEATS.map((beat, index) => (
        <div
          key={beat}
          className="absolute top-0 grid h-[calc(40*var(--px))] w-[calc(96*var(--px))] place-items-center bg-[url(/game/popups/wood.png)] bg-size-[100%_100%] [image-rendering:pixelated] transition-[translate] duration-[330ms] ease-[steps(20)] motion-reduce:transition-none"
          style={{
            left: "calc(24 * var(--px))",
            translate: `0 calc(${ROW * order.indexOf(index)} * var(--px))`,
          }}
        >
          <span className="gba-text whitespace-nowrap">{beat}</span>
        </div>
      ))}
    </div>
  )
}

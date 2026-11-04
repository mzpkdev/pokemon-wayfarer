import { type KeyboardEvent, useRef, useState } from "react"

import { usePrinter } from "~/components/DialogBox"
import { cn } from "~/lib/cn"
import type { Highlight } from "~/lib/content"
import { useHydrated, useReducedMotion } from "~/lib/hooks"

/**
 * The tour's highlights as the BAG screen: the Bag on the left under its
 * pocket's name, the pocket's items in a list with the ▶ cursor, and the
 * item under the cursor shown by its icon and printed in the message box
 * below, the way the game describes an item. Pointing at an item or moving
 * the cursor with the arrow keys picks it, and the Bag gives the shake it
 * gives when you rummage in it.
 *
 * Every description is in the page; only the chosen one shows, so the box is
 * as tall as the longest and never jumps.
 */
export const BagScreen = ({ items }: { items: Highlight[] }) => {
  const hydrated = useHydrated()
  const reducedMotion = useReducedMotion()
  const [chosen, setChosen] = useState(0)
  const buttons = useRef<(HTMLButtonElement | null)[]>([])
  const item = items[chosen]!
  const shown = usePrinter(item.body, hydrated && !reducedMotion, 1)

  const choose = (index: number, focus = false) => {
    const next = (index + items.length) % items.length
    setChosen(next)
    if (focus) buttons.current[next]?.focus()
  }

  // Steps from the item that has focus, wherever the pointer last chose.
  const onKeyDown = (event: KeyboardEvent, index: number) => {
    const step = { ArrowDown: 1, ArrowUp: -1 }[event.key]
    if (step === undefined) return
    event.preventDefault()
    choose(index + step, true)
  }

  return (
    <div className="grid gap-[calc(4*var(--px))] [--px:2px] md:grid-cols-[auto_minmax(0,1fr)]">
      <div
        className="hidden flex-col items-center gap-[calc(4*var(--px))] md:flex"
        aria-hidden="true"
      >
        <p className="gba-window gba-text w-full text-center">KEY ITEMS</p>
        {/* Keyed by the item so each pick remounts it and plays the shake again. */}
        <div key={chosen} className="bag-shake pixelated" />
      </div>

      <ul aria-label="Highlights" className="gba-window gba-text grid content-start">
        {items.map((entry, index) => {
          const on = index === chosen
          return (
            <li key={entry.title}>
              <button
                ref={(element) => {
                  buttons.current[index] = element
                }}
                type="button"
                aria-pressed={on}
                aria-controls="bag-description"
                tabIndex={on ? 0 : -1}
                onFocus={() => choose(index)}
                onMouseEnter={() => choose(index)}
                onClick={() => choose(index)}
                onKeyDown={(event) => onKeyDown(event, index)}
                className="flex w-full items-center text-left focus-visible:outline-none"
              >
                <span aria-hidden="true" className="inline-block w-[calc(10*var(--px))] shrink-0">
                  {on ? "▶" : ""}
                </span>
                {/* Capitals the game's way, which keeps POKéMON's small é. */}
                {entry.title.toUpperCase().replaceAll("É", "é")}
              </button>
            </li>
          )
        })}
      </ul>

      <div className="flex items-stretch gap-[calc(4*var(--px))] md:col-span-2">
        <div className="gba-window grid shrink-0 place-items-center" aria-hidden="true">
          <img
            key={item.icon}
            src={`/game/items/${item.icon}.png`}
            alt=""
            width={24}
            height={24}
            className="pixelated size-[calc(24*var(--px))]"
          />
        </div>
        <div
          id="bag-description"
          aria-live="polite"
          className="gba-window gba-text grid min-w-0 flex-1"
        >
          {items.map((entry, index) => {
            const on = index === chosen
            return (
              <p
                key={entry.title}
                aria-hidden={!on}
                className={cn("col-start-1 row-start-1", !on && "invisible")}
              >
                {on ? (
                  <>
                    <span className="sr-only">{entry.body}</span>
                    <span aria-hidden="true">
                      {entry.body.slice(0, shown)}
                      <span className="invisible">{entry.body.slice(shown)}</span>
                    </span>
                  </>
                ) : (
                  entry.body
                )}
              </p>
            )
          })}
        </div>
      </div>
    </div>
  )
}

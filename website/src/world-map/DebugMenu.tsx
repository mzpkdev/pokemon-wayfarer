import { useEffect, useState } from "react"

/** Where the debug options are kept between visits, in this browser only. */
const STORE = "wayfarer:world-map-debug"

const read = (): { rates: boolean } => {
  try {
    return { rates: false, ...JSON.parse(localStorage.getItem(STORE) ?? "{}") }
  } catch {
    return { rates: false }
  }
}

export type Debug = {
  open: boolean
  setOpen: (open: boolean) => void
  /** Exact encounter rates on the place cards. */
  rates: boolean
  setRates: (rates: boolean) => void
}

/**
 * The explorer's debug options, out of a visitor's way: the menu opens with
 * `?debug` in the address or the backquote key, and its options stay set in
 * this browser.
 */
export const useDebug = (): Debug => {
  // The explorer only mounts in the browser, on a visitor's tap.
  const [open, setOpen] = useState(() => new URLSearchParams(location.search).has("debug"))
  const [rates, setRatesState] = useState(() => read().rates)
  useEffect(() => {
    const toggle = (event: KeyboardEvent) => {
      if (event.key !== "`" || event.target instanceof HTMLInputElement) return
      event.preventDefault()
      setOpen((was) => !was)
    }
    window.addEventListener("keydown", toggle)
    return () => window.removeEventListener("keydown", toggle)
  }, [])
  const setRates = (next: boolean) => {
    setRatesState(next)
    try {
      localStorage.setItem(STORE, JSON.stringify({ ...read(), rates: next }))
    } catch {
      // Without storage it holds for this visit.
    }
  }
  return { open, setOpen, rates, setRates }
}

/** The debug menu: a small panel under the explorer's CLOSE button. */
export const DebugMenu = ({ debug }: { debug: Debug }) =>
  debug.open ? (
    <section
      aria-label="Debug"
      className="pixel-corners pixel-panel absolute top-20 right-4 flex w-60 flex-col gap-3 p-4 text-paper"
    >
      <div className="flex items-start justify-between gap-3">
        <h2 className="pixel-label text-logo-yellow">DEBUG</h2>
        <button
          type="button"
          onClick={() => debug.setOpen(false)}
          aria-label="Close debug menu"
          className="pixel-label -mt-1 -mr-2 px-2 text-paper/70 hover:text-paper focus-visible:text-paper focus-visible:underline focus-visible:outline-none"
        >
          ×
        </button>
      </div>
      <label className="flex cursor-pointer items-center gap-3 text-sm">
        <input
          type="checkbox"
          checked={debug.rates}
          onChange={(event) => debug.setRates(event.target.checked)}
          className="size-4 accent-[rgb(232_208_64)]"
        />
        Encounter rates on place cards
      </label>
    </section>
  ) : null

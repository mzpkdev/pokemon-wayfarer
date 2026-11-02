import { useSyncExternalStore } from "react"

const FRAME_COUNT = 20
const STORAGE_KEY = "wayfarer:frame"
const CHANGE_EVENT = "wayfarer:frame"

const readSaved = (): number => {
  try {
    const saved = Number(localStorage.getItem(STORAGE_KEY))
    return saved >= 1 && saved <= FRAME_COUNT ? saved : 1
  } catch {
    return 1
  }
}

const subscribe = (onChange: () => void) => {
  window.addEventListener(CHANGE_EVENT, onChange)
  window.addEventListener("storage", onChange)
  return () => {
    window.removeEventListener(CHANGE_EVENT, onChange)
    window.removeEventListener("storage", onChange)
  }
}

const save = (frame: number) => {
  document.documentElement.style.setProperty("--frame", `url("/game/frames/${frame}.png")`)
  try {
    localStorage.setItem(STORAGE_KEY, String(frame))
  } catch {
    // Private windows may refuse storage; the frame still applies to this visit.
  }
  window.dispatchEvent(new Event(CHANGE_EVENT))
}

/** OPTIONS' FRAME setting: picks which of the game's 20 text window frames every window uses. */
export const FrameOptions = () => {
  // The layout's inline script has already applied the saved frame before paint.
  const frame = useSyncExternalStore(subscribe, readSaved, () => 1)

  const step = (by: number) => save(((frame - 1 + by + FRAME_COUNT) % FRAME_COUNT) + 1)

  const arrow =
    "gba-text cursor-pointer px-[calc(2*var(--px))] outline-none focus-visible:gba-text-blue"

  return (
    <div className="gba-window">
      <p className="gba-text gba-text-blue">OPTIONS</p>
      <div className="flex items-center justify-between gap-[calc(12*var(--px))]">
        <span className="gba-text" aria-hidden="true">
          FRAME
        </span>
        <fieldset className="flex items-center">
          <legend className="sr-only">Text window frame</legend>
          <button
            type="button"
            className={arrow}
            onClick={() => step(-1)}
            aria-label="Previous frame"
          >
            <span className="inline-block -scale-x-100">▶</span>
          </button>
          <output
            className="gba-text inline-block w-[calc(44*var(--px))] text-center"
            aria-live="polite"
          >
            TYPE {frame}
          </output>
          <button type="button" className={arrow} onClick={() => step(1)} aria-label="Next frame">
            ▶
          </button>
        </fieldset>
      </div>
    </div>
  )
}

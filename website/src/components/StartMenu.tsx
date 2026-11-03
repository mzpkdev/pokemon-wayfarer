import { useEffect, useId, useRef, useState } from "react"

import { REPOSITORY_URL } from "~/lib/content"
import { useReducedMotion } from "~/lib/hooks"

/**
 * The page's menu, as the game's START menu: a MENU button in the corner once
 * the title screen is behind (top right; a pill in the bottom corner on a
 * phone), opening a text window of entries with the ▶ cursor, and under it
 * the window that describes the entry the cursor is on.
 * The arrow keys move the cursor, Enter picks, Esc (or B, as on the GBA)
 * closes. Each entry scrolls to a part of the page; the world's lands on the
 * map once the warp has arrived.
 */

type Entry = {
  label: string
  /** What the entry does, in the description window. */
  hint: string
  /** A section to scroll to, by selector. */
  target?: string
  /** An address to go to instead. */
  href?: string
}

const WORLD = ".world-stage"

const ENTRIES: Entry[] = [
  { label: "WORLD", hint: "Tour the world in the game's own tiles.", target: WORLD },
  { label: "PROF. OAK", hint: "Leave your email for release day.", target: "#notify" },
  { label: "OPEN WORLD", hint: "How an open world stays fair.", target: "#open-world" },
  { label: "RIGHT NOW", hint: "Who is where on the page's clock.", target: "#right-now" },
  { label: "QUESTIONS", hint: "Playing, battling, trading, saves.", target: "#faq" },
  { label: "GITHUB", hint: "Follow every change as it lands.", href: REPOSITORY_URL },
  { label: "EXIT", hint: "Close this menu." },
]

/** Screens scrolled before the button appears: once the title screen is left behind. */
const SHOW_AFTER = 0.75

export const StartMenu = () => {
  const reducedMotion = useReducedMotion()
  const id = useId()
  const rootRef = useRef<HTMLDivElement>(null)
  const buttonRef = useRef<HTMLButtonElement>(null)
  const itemsRef = useRef<(HTMLElement | null)[]>([])
  const [shown, setShown] = useState(false)
  const [open, setOpen] = useState(false)
  const [cursor, setCursor] = useState(0)

  // The button goes with the title screen, and so does an open menu:
  // scrolling back up to the title closes it.
  useEffect(() => {
    const update = () => {
      const height = window.innerHeight
      // On a phone the button is a pill in the bottom corner, where the map
      // tour keeps its own controls, so it steps aside while the map is pinned.
      const narrow = !window.matchMedia("(min-width: 640px)").matches
      const stage = document.querySelector(WORLD)?.getBoundingClientRect()
      const onMap = stage ? stage.top <= height / 2 && stage.bottom >= height / 2 : false
      const past = window.scrollY > height * SHOW_AFTER && !(narrow && onMap)
      setShown(past)
      if (!past) setOpen(false)
    }
    update()
    window.addEventListener("scroll", update, { passive: true })
    window.addEventListener("resize", update)
    return () => {
      window.removeEventListener("scroll", update)
      window.removeEventListener("resize", update)
    }
  }, [])

  // Opening puts the cursor, and the focus, on the first entry.
  useEffect(() => {
    if (open) itemsRef.current[0]?.focus()
  }, [open])

  /** Closes the menu, handing focus back to the button unless the pointer took it elsewhere. */
  const close = (back = true) => {
    setOpen(false)
    if (back) buttonRef.current?.focus()
  }

  useEffect(() => {
    if (!open) return
    const keydown = (event: KeyboardEvent) => {
      if (event.key === "Escape" || event.key === "b" || event.key === "B") {
        event.preventDefault()
        setOpen(false)
        buttonRef.current?.focus()
      } else if (event.key === "ArrowDown" || event.key === "ArrowUp") {
        event.preventDefault()
        const step = event.key === "ArrowDown" ? 1 : -1
        const next = (cursor + step + ENTRIES.length) % ENTRIES.length
        itemsRef.current[next]?.focus()
      }
    }
    const pointerdown = (event: PointerEvent) => {
      if (!rootRef.current?.contains(event.target as Node)) setOpen(false)
    }
    document.addEventListener("keydown", keydown)
    document.addEventListener("pointerdown", pointerdown)
    return () => {
      document.removeEventListener("keydown", keydown)
      document.removeEventListener("pointerdown", pointerdown)
    }
  }, [open, cursor])

  const go = (entry: Entry) => {
    close(false)
    if (!entry.target) return
    const element = document.querySelector<HTMLElement>(entry.target)
    if (!element) return
    const behavior: ScrollBehavior = reducedMotion ? "auto" : "smooth"
    if (entry.target === WORLD)
      // The map is there once the stage's top reaches the top of the screen
      // (see lib/warp.ts); a couple of pixels past it, so a scroll that
      // settles a fraction short doesn't stop on the warp's black.
      window.scrollTo({ top: element.getBoundingClientRect().top + window.scrollY + 2, behavior })
    else {
      const aim = () => element.scrollIntoView({ behavior, block: "start" })
      aim()
      // The RIGHT NOW board loads as the scroll passes it and grows, moving
      // everything after it, so the aim is corrected while the page settles.
      const observer = new ResizeObserver(aim)
      observer.observe(document.body)
      window.setTimeout(() => observer.disconnect(), 2500)
    }
  }

  const hint = ENTRIES[cursor]!.hint

  return (
    <div
      ref={rootRef}
      className={`fixed right-3 bottom-3 z-40 flex flex-col-reverse items-end gap-2 transition-opacity duration-300 sm:top-2 sm:right-2 sm:bottom-auto sm:flex-col ${
        shown ? "opacity-100" : "pointer-events-none opacity-0"
      }`}
      aria-hidden={!shown}
    >
      <button
        ref={buttonRef}
        type="button"
        aria-haspopup="menu"
        aria-expanded={open}
        aria-controls={id}
        tabIndex={shown ? 0 : -1}
        onClick={() => (open ? close() : setOpen(true))}
        className="pixel-corners pixel-panel pixel-label px-4 py-1.5 text-[16px] leading-none text-paper hover:text-logo-yellow focus-visible:text-logo-yellow focus-visible:outline-none"
      >
        MENU
      </button>
      <div
        className={`flex flex-col items-end gap-[calc(4*var(--px))] [--px:2px] ${open ? "" : "hidden"}`}
      >
        <ul id={id} role="menu" aria-label="Page menu" className="gba-window gba-text">
          {ENTRIES.map((entry, index) => {
            const on = index === cursor
            const className =
              "flex w-full items-center whitespace-nowrap pr-[calc(8*var(--px))] text-left text-ink hover:text-ink focus-visible:outline-none"
            const mark = (
              <span aria-hidden="true" className="inline-block w-[calc(10*var(--px))]">
                {on ? "▶" : ""}
              </span>
            )
            return (
              <li key={entry.label} role="none">
                {entry.href ? (
                  <a
                    ref={(element) => {
                      itemsRef.current[index] = element
                    }}
                    role="menuitem"
                    href={entry.href}
                    tabIndex={on ? 0 : -1}
                    onFocus={() => setCursor(index)}
                    onMouseEnter={() => setCursor(index)}
                    onClick={() => close(false)}
                    className={className}
                  >
                    {mark}
                    {entry.label}
                  </a>
                ) : (
                  <button
                    ref={(element) => {
                      itemsRef.current[index] = element
                    }}
                    type="button"
                    role="menuitem"
                    tabIndex={on ? 0 : -1}
                    onFocus={() => setCursor(index)}
                    onMouseEnter={() => setCursor(index)}
                    onClick={() => go(entry)}
                    className={className}
                  >
                    {mark}
                    {entry.label}
                  </button>
                )}
              </li>
            )
          })}
        </ul>
        {/* Two lines tall whatever the hint, so the list above it never shifts under a finger. */}
        <p
          aria-live="polite"
          className="gba-window gba-text min-h-[calc(2lh+20*var(--px))] w-[min(calc(100vw-0.75rem),calc(232*var(--px)))] text-ink"
        >
          {hint}
        </p>
      </div>
    </div>
  )
}

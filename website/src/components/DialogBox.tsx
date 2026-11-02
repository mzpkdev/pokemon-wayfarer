import { useEffect, useState } from "react"

import { cn } from "~/lib/cn"
import { useHydrated, useReducedMotion } from "~/lib/hooks"

type DialogBoxProps = {
  lines: string[]
  /**
   * Prints while true. To print again from the start, remount it with a new
   * `key`. Inactive boxes show their whole text.
   */
  active?: boolean
  /** Frames between letters, as with the game's text speed (MID is 4, FAST 1). */
  speed?: number
  className?: string
}

/**
 * A message box that prints its text a letter at a time, then bounces the
 * down arrow, like dialogue in the field. The full text is always in the DOM
 * for screen readers and without JavaScript; only its visible part changes.
 */
export const DialogBox = ({ lines, active = true, speed = 2, className }: DialogBoxProps) => {
  const text = lines.join("\n")
  const hydrated = useHydrated()
  const reducedMotion = useReducedMotion()
  const printing = hydrated && active && !reducedMotion
  const [printed, setPrinted] = useState(0)

  useEffect(() => {
    if (!printing) return
    const timer = window.setInterval(
      () =>
        setPrinted((count) => {
          if (count + 1 >= text.length) window.clearInterval(timer)
          return Math.min(text.length, count + 1)
        }),
      (speed * 1000) / 60,
    )
    return () => window.clearInterval(timer)
  }, [printing, speed, text])

  const shown = printing ? printed : text.length
  const done = shown >= text.length

  return (
    <div className={cn("gba-window relative", className)}>
      <p className="gba-text whitespace-pre-line">
        <span className="sr-only">{text}</span>
        <span aria-hidden="true">
          {text.slice(0, shown)}
          <span className="invisible">{text.slice(shown)}</span>
        </span>
      </p>
      <span
        aria-hidden="true"
        className={cn("gba-down-arrow absolute", !done && "invisible")}
        style={{ right: "calc(0 * var(--px))", bottom: "calc(-2 * var(--px))" }}
      />
    </div>
  )
}

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
/**
 * How much of `text` the game's text printer has put down so far, a letter
 * every `speed` frames while `printing`, starting over whenever the text
 * changes. Without printing it is all of it at once.
 */
export const usePrinter = (text: string, printing: boolean, speed = 2) => {
  // Counted per text, so a new text starts from nothing without resetting in an effect.
  const [printed, setPrinted] = useState({ text, count: 0 })

  useEffect(() => {
    if (!printing) return
    const timer = window.setInterval(
      () =>
        setPrinted((last) => {
          const count = Math.min(text.length, (last.text === text ? last.count : 0) + 1)
          if (count >= text.length) window.clearInterval(timer)
          return { text, count }
        }),
      (speed * 1000) / 60,
    )
    return () => window.clearInterval(timer)
  }, [printing, speed, text])

  if (!printing) return text.length
  return printed.text === text ? printed.count : 0
}

export const DialogBox = ({ lines, active = true, speed = 2, className }: DialogBoxProps) => {
  const text = lines.join("\n")
  const hydrated = useHydrated()
  const reducedMotion = useReducedMotion()
  const shown = usePrinter(text, hydrated && active && !reducedMotion, speed)
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

import { useEffect, useRef, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"
import { TORCHIC_TRIP } from "~/lib/scene"

/** One tick of the trip, in ms: the title scene's 60 per second, slowed a touch to read on a page. */
const TICK_MS = 20

/**
 * TORCHIC running through the meadow at the foot of the page, as on the
 * title screen, and a little easter egg: click or tap it and it trips the
 * way it does on the title screen (the same frames, in the same order),
 * lies there a moment, gets up and runs on. With motion reduced it stands
 * in the grass instead, and still trips when asked.
 */
export const MeadowTorchic = () => {
  const reducedMotion = useReducedMotion()
  const [tripFrame, setTripFrame] = useState<number | null>(null)
  const timers = useRef<number[]>([])
  // Set the moment a trip starts, so a second press before React re-renders can't start another.
  const trippingRef = useRef(false)

  useEffect(
    () => () => {
      for (const timer of timers.current) window.clearTimeout(timer)
    },
    [],
  )

  const trip = () => {
    if (trippingRef.current) return
    trippingRef.current = true
    let at = 0
    for (const { frame, duration } of TORCHIC_TRIP) {
      timers.current.push(window.setTimeout(() => setTripFrame(frame), at))
      at += duration * TICK_MS
    }
    timers.current.push(
      window.setTimeout(() => {
        setTripFrame(null)
        timers.current = []
        trippingRef.current = false
      }, at),
    )
  }

  const tripping = tripFrame !== null

  return (
    <div
      className={`torchic-path absolute bottom-[calc(14*var(--scene-px))] left-0 ${tripping ? "paused" : ""}`}
    >
      <button
        type="button"
        aria-label={
          reducedMotion
            ? "Torchic. Press to trip it up."
            : "Torchic, running by. Press to trip it up."
        }
        onClick={trip}
        className={`torchic block focus-visible:outline-4 focus-visible:outline-offset-4 focus-visible:outline-sign-light ${
          tripping ? "tripping still" : reducedMotion ? "still" : ""
        }`}
        style={
          tripping
            ? { backgroundPositionY: `calc(${tripFrame} * -32 * var(--scene-px))` }
            : undefined
        }
      />
    </div>
  )
}

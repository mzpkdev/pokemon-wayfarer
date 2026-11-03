import { useSyncExternalStore } from "react"

const noSubscription = () => () => {}

/** False while rendering at build time and hydrating, true once running in the browser. */
export const useHydrated = (): boolean =>
  useSyncExternalStore(
    noSubscription,
    () => true,
    () => false,
  )

const reducedMotionQuery = "(prefers-reduced-motion: reduce)"

/** Whether the visitor asked for less motion; false at build time. */
export const useReducedMotion = (): boolean =>
  useSyncExternalStore(
    (onChange) => {
      const query = window.matchMedia(reducedMotionQuery)
      query.addEventListener("change", onChange)
      return () => query.removeEventListener("change", onChange)
    },
    () => window.matchMedia(reducedMotionQuery).matches,
    () => false,
  )

const subscribeToSeconds = (onTick: () => void) => {
  const timer = window.setInterval(onTick, 1000)
  return () => window.clearInterval(timer)
}

/** The current time in whole seconds, ticking each second; null at build time. */
export const useSecondsClock = (): number | null =>
  useSyncExternalStore(
    subscribeToSeconds,
    () => Math.floor(Date.now() / 1000),
    () => null,
  )

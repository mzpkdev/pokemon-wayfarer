// One game clock for the whole page, so the tour and the explorer show the
// same time of day and the same ships at sea. It starts at 8:00 the first
// time anything asks, and runs a day every two minutes (LIVING_RATE.page).
import { gameClock } from "./living"

let clock: (() => number) | undefined

/** The page's game minute now. */
export const pageMinutes = () => (clock ??= gameClock())()

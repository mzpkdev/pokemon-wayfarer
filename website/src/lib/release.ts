/** When Pokémon Wayfarer releases: 20 January 2027, midnight UTC. */
export const RELEASE_AT = new Date("2027-01-20T00:00:00Z")

/** The release date as the page prints it, e.g. "20.01.2027". */
export const RELEASE_LABEL = "20.01.2027"

export type TimeLeft = {
  days: number
  hours: number
  minutes: number
  seconds: number
  /** True once the release moment has passed. */
  released: boolean
}

/** Whole days, hours, minutes and seconds from `now` until `target`, never negative. */
export const timeLeft = (now: Date, target: Date = RELEASE_AT): TimeLeft => {
  const total = Math.max(0, Math.floor((target.getTime() - now.getTime()) / 1000))
  return {
    days: Math.floor(total / 86_400),
    hours: Math.floor(total / 3_600) % 24,
    minutes: Math.floor(total / 60) % 60,
    seconds: total % 60,
    released: total === 0,
  }
}

export const pad = (value: number): string => String(value).padStart(2, "0")

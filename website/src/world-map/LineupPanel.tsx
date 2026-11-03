import { useEffect, useMemo, useState } from "react"

import type { LeagueId } from "./landmarks"
import { eventSequence, LINEUP_SIZE, type Roster, type Seat } from "./lineup"

type Hall = { name: string; condition: string }

/** The shuffle, then each seat settling in turn, then the five held for reading, in milliseconds. */
const SHUFFLE_MS = 1500
const SETTLE_MS = 420
const HOLD_MS = 4800
const TICK_MS = 90
const LEAD_MS = 900
const settledAt = (seat: number) => SHUFFLE_MS + seat * SETTLE_MS
const CYCLE_MS = SHUFFLE_MS + (LINEUP_SIZE - 1) * SETTLE_MS + HOLD_MS

const loaded = new Set<HTMLImageElement>()
/** Starts loading every portrait, so the shuffle never shows a gap. Held so they stay cached. */
export const preloadPortraits = (base: string, roster: Roster | undefined) => {
  for (const { pic } of roster?.trainers ?? []) {
    const image = new Image()
    image.src = `${base}/leaders/${pic}.png`
    loaded.add(image)
  }
}

const portrait = (base: string, pic: string) => `${base}/leaders/${pic}.png`

const REGION_COLOR = {
  Kanto: "rgb(240 96 80)",
  Johto: "rgb(255 214 0)",
  Hoenn: "rgb(80 168 248)",
}

/**
 * A league event's five as five seats in battle order, one per hall: the
 * weakest in the first hall, the strongest in the Champion's Room. The whole
 * roster shuffles past, then the five settle seat by seat; a few seconds
 * later the next event picks again, with the last five tired. The picks are
 * the game's (`lineup.ts`); the shuffle is only for show. With reduced motion
 * it is just the first event's five.
 */
export const LineupPanel = ({
  roster,
  league,
  halls,
  progress,
  base,
  reducedMotion,
  variant,
}: {
  roster: Roster
  league: Exclude<LeagueId, "masters">
  halls: Hall[]
  /** The player's Trainer Rating, which the trainers' ratings grow with. */
  progress: number
  base: string
  reducedMotion: boolean
  /**
   * `card` sits in the stop's card, in portraits of 1x and under a screen wide
   * enough for the other; `map` lies over the map's lower part, in portraits of 2x.
   */
  variant: "card" | "map"
}) => {
  const big = variant === "map"
  // Two events are all there are: the last five are tired, so the next
  // five are others, and the five after that are the first again.
  const events = useMemo(
    () => eventSequence(roster, league, progress, 2),
    [roster, league, progress],
  )
  // Cycled through while a seat shuffles, in an order of its own.
  const pool = useMemo(
    () => roster.trainers.toSorted((a, b) => ((a.id * 37) % 41) - ((b.id * 37) % 41)),
    [roster],
  )

  const [elapsed, setElapsed] = useState(0)
  useEffect(() => {
    if (reducedMotion) return
    // A beat after arriving, so the shuffle is seen from the start.
    const start = performance.now() + LEAD_MS
    const timer = window.setInterval(
      () => setElapsed(Math.max(0, performance.now() - start)),
      TICK_MS,
    )
    return () => window.clearInterval(timer)
  }, [reducedMotion])

  const event = reducedMotion ? 0 : Math.floor(elapsed / CYCLE_MS)
  const t = reducedMotion ? CYCLE_MS : elapsed % CYCLE_MS
  const lineup: Seat[] = events[event % events.length]!
  const settled = (seat: number) => t >= settledAt(seat)
  const done = settled(LINEUP_SIZE - 1)
  const tick = Math.floor(elapsed / TICK_MS)

  return (
    <section
      aria-label="The five"
      className={
        big
          ? "pixel-corners pixel-panel pointer-events-auto absolute bottom-12 left-12 w-[min(736px,calc(100%-32.5rem))] p-4 max-xl:hidden"
          : "mt-3 xl:hidden"
      }
    >
      <p className="pixel-label flex justify-between gap-3 text-[16px] leading-none">
        <span className="text-logo-yellow">THE FIVE</span>
        <span className="text-paper/70">{done ? `EVENT ${event + 1}` : "PICKING…"}</span>
      </p>
      <p className="sr-only" aria-live="polite">
        {done
          ? `Event ${event + 1}, in battle order: ${lineup.map(({ trainer }) => trainer.name).join(", ")}.`
          : ""}
      </p>
      <ol className={`mt-3 grid grid-cols-5 ${big ? "mt-4 gap-3" : "gap-0"}`}>
        {lineup.map((seat, i) => {
          const hall = halls[i]
          const here = settled(i)
          const age = t - settledAt(i)
          const shown = here ? seat.trainer : pool[(tick * (3 + i) + i * 7) % pool.length]!
          const champion = i === LINEUP_SIZE - 1
          return (
            <li key={i} className="flex min-w-0 flex-col items-center text-center">
              {big && (
                <p className="flex h-11 w-full flex-col justify-end">
                  <span className="truncate font-sans text-sm leading-tight font-bold text-paper">
                    {hall?.name}
                  </span>
                  <span className="truncate font-sans text-xs leading-tight text-paper/60">
                    {hall?.condition}
                  </span>
                </p>
              )}
              <div
                className={`relative mt-1 grid w-full max-w-16 place-items-center bg-[rgb(24_40_88)] transition-transform duration-150 ${big ? "max-w-32" : "max-w-16"} ${
                  champion
                    ? "shadow-[0_0_0_2px_rgb(255_214_0)]"
                    : "shadow-[0_0_0_2px_rgb(255_255_255/0.25)]"
                } ${here && age < 200 && !reducedMotion ? "scale-105" : ""}`}
              >
                <img
                  src={portrait(base, shown.pic)}
                  alt=""
                  width={64}
                  height={64}
                  className={`aspect-square w-full [image-rendering:pixelated] ${here ? "" : "opacity-60 brightness-75"}`}
                  style={here ? undefined : { translate: `0 ${tick % 2 ? -3 : 3}px` }}
                />
                <span
                  aria-hidden="true"
                  className={`pixel-label absolute top-0 left-0 px-1 pt-0.5 pb-1 text-[16px] leading-none ${
                    champion ? "bg-logo-yellow text-ink" : "bg-black/60 text-paper"
                  }`}
                >
                  {i + 1}
                </span>
              </div>
              <p
                className={`mt-2 w-full truncate font-sans leading-tight font-bold uppercase ${big ? "text-sm" : "text-[11px]"} ${
                  here ? "text-paper" : "text-paper/30"
                }`}
              >
                {here ? seat.trainer.name : "…"}
              </p>
              <p
                className={`w-full truncate font-sans leading-tight text-paper/60 ${big ? "text-xs" : "text-[11px]"}`}
              >
                {here ? (
                  <>
                    <span
                      aria-hidden="true"
                      className="mr-1 inline-block size-1.5 rounded-full"
                      style={{ background: REGION_COLOR[seat.trainer.home] }}
                    />
                    {big && `${seat.trainer.home} · `}TR {seat.rating}
                  </>
                ) : (
                  " "
                )}
              </p>
            </li>
          )
        })}
      </ol>
      <p
        className={`font-sans leading-snug text-paper/60 ${big ? "mt-4 text-xs" : "mt-3 text-[11px]"}`}
      >
        At Trainer Rating {progress}.{" "}
        {event > 0
          ? "The last event's five are tired, so others come."
          : "Nobody has fought here yet."}
      </p>
      {!big && (
        <ol className="mt-3 grid gap-0.5 text-[13px] sm:gap-1 sm:text-sm">
          {halls.map((hall, i) => (
            <li key={hall.name} className="flex justify-between gap-3">
              <span className="text-paper/90">
                {i + 1} · {hall.name}
              </span>
              <span className="text-right text-paper/60">{hall.condition}</span>
            </li>
          ))}
        </ol>
      )}
    </section>
  )
}

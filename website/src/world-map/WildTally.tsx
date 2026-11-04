import { type RefObject, useEffect, useState } from "react"

import { HOMES, TICK_MS, type Wild, type WildClock, startOf, waveAt } from "./wild"

const NONE = HOMES.map(() => 0)

/**
 * The wild stop's tally: the roster per home region, one chip each, with a
 * line saying whose home is playing. Each ticks up as its wave
 * pops on the map, the playing wave's lit; a chip taps to hold its wave. With
 * reduced motion everything is in at once and a chip picks which icons the
 * map holds still.
 */
export const WildTally = ({
  wild,
  shared,
  shown,
  first,
  reducedMotion,
  onChange,
}: {
  wild: Wild | undefined
  /** The clock the map's icons read; the tally owns it. */
  shared: RefObject<WildClock>
  shown: boolean
  /** The wave to begin on: the start's own region's. */
  first: number
  reducedMotion: boolean
  onChange: () => void
}) => {
  const counts = wild?.counts ?? NONE
  const [clock, setClock] = useState<WildClock>({ start: 0, still: null })
  const [now, setNow] = useState(0)
  const apply = (next: WildClock) => {
    shared.current.start = next.start
    shared.current.still = next.still
    setClock(next)
    setNow(performance.now())
    onChange()
  }
  // Each arrival starts the cycle over, on the start's own region; with
  // reduced motion that region just stands.
  useEffect(() => {
    if (!shown) return
    const at = performance.now()
    apply(reducedMotion ? { start: at, still: first } : { start: startOf(first, at), still: null })
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [shown])
  useEffect(() => {
    if (!shown || reducedMotion) return
    const timer = window.setInterval(() => setNow(performance.now()), 80)
    return () => window.clearInterval(timer)
  }, [shown, reducedMotion])
  // A tap holds that wave on the map and stops the loop; tapping the held
  // chip again lets the loop play on from there.
  const pick = (wave: number) =>
    clock.still === wave && !reducedMotion
      ? apply({ start: startOf(wave, performance.now()), still: null })
      : apply({ start: performance.now(), still: wave })

  const playing =
    reducedMotion || clock.still !== null
      ? { wave: clock.still ?? first, into: Infinity }
      : waveAt(now - clock.start)
  const held = clock.still !== null
  const lit = playing.wave
  // The waves already played this cycle, counted from the one it began on.
  const order = (wave: number) => (wave - first + HOMES.length) % HOMES.length
  const shownCounts = counts.map((count, i) =>
    order(i) < order(playing.wave) || reducedMotion || held
      ? count
      : i === playing.wave
        ? Math.round(count * Math.min(1, playing.into / TICK_MS))
        : 0,
  )
  const cells = HOMES.map((home, i) => ({
    label: home.chip,
    value: shownCounts[i]!,
    wave: i,
    name: `${home.line} ${counts[i]} species`,
  }))
  return (
    <>
      <ul className="mt-3 grid grid-cols-4 gap-1.5 sm:mt-4 sm:gap-2">
        {cells.map(({ label, value, wave, name }) => {
          const on = lit === wave
          return (
            <li key={label} className="contents">
              <button
                type="button"
                disabled={!shown}
                aria-pressed={on}
                aria-label={name}
                onClick={() => pick(wave)}
                className={`pixel-corners flex flex-col items-center justify-between gap-1 px-1 pt-1.5 pb-2 font-[family-name:var(--font-gba)] leading-none ${
                  on
                    ? "bg-sign text-ink"
                    : "bg-white/8 text-paper/80 hover:bg-white/15 hover:text-paper"
                }`}
              >
                <span
                  className={`text-center text-[12px] leading-[14px] sm:text-[14px] sm:leading-[16px] ${on ? "" : "text-paper/60"}`}
                >
                  {label}
                </span>
                <span className="text-[16px] sm:text-[20px]">{value}</span>
              </button>
            </li>
          )
        })}
      </ul>
      <p className="mt-3 min-h-[2lh] text-sm leading-snug text-paper/85">{HOMES[lit]!.line}</p>
      <p className="mt-1 text-xs text-paper/60">
        {held && !reducedMotion
          ? "Held. Tap it again to play them all."
          : "Tap a region to hold it."}
      </p>
    </>
  )
}

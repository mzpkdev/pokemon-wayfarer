import { type RefObject, useEffect, useState } from "react"

import { TICK_MS, type Wild, type WildClock, WAVES, waveAt, startOf } from "./wild"

const NONE = Array.from({ length: 9 }, () => 0)

/**
 * The wild stop's tally: the species met in the wild per generation, one chip
 * each (I to VIII, the regional forms) and the total. Each ticks up as its
 * wave pops on the map, the playing wave's lit; a chip taps to play its wave
 * again. With reduced motion everything is in at once and a chip picks which
 * icons the map holds still.
 */
export const WildTally = ({
  wild,
  shared,
  shown,
  reducedMotion,
  onChange,
}: {
  wild: Wild | undefined
  /** The clock the map's icons read; the tally owns it. */
  shared: RefObject<WildClock>
  shown: boolean
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
  // Each arrival starts the cycle over.
  useEffect(() => {
    if (shown) apply({ start: performance.now(), still: null })
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [shown])
  useEffect(() => {
    if (!shown || reducedMotion) return
    const timer = window.setInterval(() => setNow(performance.now()), 80)
    return () => window.clearInterval(timer)
  }, [shown, reducedMotion])
  const pick = (wave: number) =>
    apply({
      start: startOf(counts, wave, performance.now()),
      still: reducedMotion ? wave : null,
    })

  const playing = reducedMotion
    ? { wave: clock.still ?? WAVES.length - 1, into: Infinity }
    : waveAt(counts, now - clock.start)
  const lit = playing.wave
  const shownCounts = counts.map((count, i) =>
    i < playing.wave || reducedMotion || playing.wave === WAVES.length - 1
      ? count
      : i === playing.wave
        ? Math.round(count * Math.min(1, playing.into / TICK_MS))
        : 0,
  )
  const total = shownCounts.reduce((a, b) => a + b, 0)
  const cells = [
    ...WAVES.slice(0, 9).map((label, i) => ({ label, value: shownCounts[i]!, wave: i })),
    { label: "ALL", value: total, wave: WAVES.length - 1 },
  ]
  return (
    <ul className="mt-3 grid grid-cols-5 gap-1.5 sm:mt-4 sm:gap-2">
      {cells.map(({ label, value, wave }) => {
        const on = lit === wave
        return (
          <li key={label} className="contents">
            <button
              type="button"
              disabled={!shown}
              aria-pressed={on}
              aria-label={
                wave === WAVES.length - 1
                  ? `All: ${counts.reduce((a, b) => a + b, 0)} species`
                  : `${label === "FORMS" ? "Regional forms" : `Generation ${label}`}: ${counts[wave]} species`
              }
              onClick={() => pick(wave)}
              className={`pixel-corners flex flex-col items-center gap-0.5 px-1 pt-1.5 pb-2 font-[family-name:var(--font-gba)] leading-none ${
                on
                  ? "bg-[rgb(232_208_64)] text-[rgb(56_56_56)]"
                  : "bg-black/30 text-paper/80 hover:bg-black/50 hover:text-paper"
              }`}
            >
              <span className={`text-[12px] sm:text-[14px] ${on ? "" : "text-paper/60"}`}>
                {label}
              </span>
              <span className="text-[16px] sm:text-[20px]">{value}</span>
            </button>
          </li>
        )
      })}
    </ul>
  )
}

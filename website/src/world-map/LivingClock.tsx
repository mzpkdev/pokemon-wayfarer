import { clsx } from "clsx"

import { type Living, periodAt } from "./living"

/**
 * A small game-styled badge: the time of day and the clock, with a swatch in
 * the period's tint. `minutes` is the game minute (see `gameClock`).
 */
export const LivingClock = ({
  living,
  minutes,
  className,
}: {
  living: Living
  minutes: number
  className?: string
}) => {
  const { name, clock, day, tint } = periodAt(living, minutes)
  return (
    <div
      role="timer"
      aria-label={`${name}, ${clock}`}
      className={clsx(
        "pixel-panel gba-text gba-text-light flex items-center gap-3 px-4 py-3",
        className,
      )}
    >
      <span
        aria-hidden
        className="inline-block size-4 shrink-0 shadow-[0_0_0_2px_rgb(0_16_0)]"
        style={{ background: `rgb(${tint.map(Math.round).join(" ")})` }}
      />
      <span className="uppercase">{name}</span>
      <span className="tabular-nums" title={`Day ${day}`}>
        {clock}
      </span>
    </div>
  )
}

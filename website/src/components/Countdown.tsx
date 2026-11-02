import { cn } from "~/lib/cn"
import { useSecondsClock } from "~/lib/hooks"
import { pad, RELEASE_AT, timeLeft } from "~/lib/release"

type CountdownProps = {
  className?: string
}

const UNITS = [
  { key: "days", label: "DAYS" },
  { key: "hours", label: "HRS" },
  { key: "minutes", label: "MIN" },
  { key: "seconds", label: "SEC" },
] as const

/** Time left until release, in a window drawn with the player's chosen frame. */
export const Countdown = ({ className }: CountdownProps) => {
  // Rendered at build time without a clock; the numbers fill in once hydrated.
  const seconds = useSecondsClock()
  const left = seconds === null ? null : timeLeft(new Date(seconds * 1000))

  if (left?.released)
    return (
      <div className={cn("gba-window", className)}>
        <p className="gba-text gba-text-2x text-center">OUT NOW!</p>
      </div>
    )

  return (
    <div
      className={cn("gba-window", className)}
      role="timer"
      aria-label={
        left
          ? `${left.days} days, ${left.hours} hours and ${left.minutes} minutes until release`
          : "Time until release"
      }
    >
      <dl className="flex items-start gap-[calc(6*var(--px))]">
        {UNITS.map(({ key, label }, i) => (
          <div key={key} className="flex flex-col items-center">
            <dt className="gba-text gba-text-blue order-2">{label}</dt>
            <dd className="gba-text gba-text-2x order-1 tabular-nums">
              {left ? (key === "days" ? String(left[key]) : pad(left[key])) : "--"}
              {i < UNITS.length - 1 && <span className="sr-only">,</span>}
            </dd>
          </div>
        ))}
      </dl>
      <time className="sr-only" dateTime={RELEASE_AT.toISOString()}>
        {RELEASE_AT.toUTCString()}
      </time>
    </div>
  )
}

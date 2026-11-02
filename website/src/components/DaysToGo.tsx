import { useSecondsClock } from "~/lib/hooks"
import { RELEASE_LABEL, timeLeft } from "~/lib/release"

/** "78 DAYS TO GO" until release; the release date at build time and once it's out. */
export const DaysToGo = ({ className }: { className?: string }) => {
  const seconds = useSecondsClock()
  if (seconds === null) return <span className={className}>OUT {RELEASE_LABEL}</span>

  const left = timeLeft(new Date(seconds * 1000))
  if (left.released) return <span className={className}>OUT NOW!</span>
  if (left.days === 0) return <span className={className}>ALMOST HERE!</span>
  return (
    <span className={className}>
      {left.days} {left.days === 1 ? "DAY" : "DAYS"} TO GO
    </span>
  )
}

import { useEffect, useRef, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"

/** The Bag's frames in public/game/bag.png: shut, and open at the top. */
const SHUT = 0
const OPEN = 3

/** How long a hop takes, in ms, and how high it arcs above the line to the Bag, in CSS pixels. */
const HOP_MS = 560
const ARC = 96
/** A hop moves in this many whole steps, the way the game moves sprites. */
const STEPS = 12

/**
 * The Bag from the BAG screen, beside the highlights, and a little easter
 * egg: click a highlight's item icon (a button marked `data-bag-item`) and
 * the item hops into the Bag, which opens for it and shakes as it lands, and
 * it's gone from its row until the page loads again. With motion reduced the
 * item just goes, without the hop or the shake.
 */
export const BagDrop = () => {
  const reducedMotion = useReducedMotion()
  const bagRef = useRef<HTMLDivElement>(null)
  const [shakes, setShakes] = useState(0)
  const [hopping, setHopping] = useState(0)

  useEffect(() => {
    const bag = bagRef.current
    const section = bag?.closest("section")
    if (!bag || !section) return
    const hops: Animation[] = []

    const hop = (button: HTMLElement) => {
      const icon = button.querySelector("img")
      if (!icon) return
      const from = icon.getBoundingClientRect()
      const to = bag.getBoundingClientRect()
      const origin = section.getBoundingClientRect()
      const copy = icon.cloneNode() as HTMLElement
      Object.assign(copy.style, {
        position: "absolute",
        left: `${from.left - origin.left}px`,
        top: `${from.top - origin.top}px`,
        width: `${from.width}px`,
        height: `${from.height}px`,
        pointerEvents: "none",
        zIndex: "10",
      })
      section.append(copy)
      const dx = to.left + to.width / 2 - (from.left + from.width / 2)
      const dy = to.top + to.height * 0.3 - (from.top + from.height / 2)
      const frames = Array.from({ length: STEPS + 1 }, (_, step) => {
        const t = step / STEPS
        const y = dy * t - ARC * 4 * t * (1 - t)
        return { transform: `translate(${dx * t}px, ${y}px) scale(${1 - 0.6 * t})` }
      })
      setHopping((count) => count + 1)
      const animation = copy.animate(frames, { duration: HOP_MS, easing: `steps(${STEPS})` })
      hops.push(animation)
      animation.finished
        .then(
          () => setShakes((count) => count + 1),
          () => {},
        )
        .finally(() => {
          copy.remove()
          setHopping((count) => count - 1)
        })
    }

    const onClick = (event: MouseEvent) => {
      const button = (event.target as Element).closest<HTMLElement>("[data-bag-item]")
      if (!button || button.hidden) return
      if (!reducedMotion) hop(button)
      // In the Bag now, so gone from its row.
      button.hidden = true
    }
    section.addEventListener("click", onClick)

    return () => {
      section.removeEventListener("click", onClick)
      hops.forEach((animation) => animation.cancel())
    }
  }, [reducedMotion])

  return (
    <div ref={bagRef} aria-hidden="true" className="bag-drop shrink-0">
      {/* Keyed by the shake count, so each shake remounts it and plays again. */}
      <div
        key={shakes}
        className={`bag-sprite pixelated ${shakes ? "bag-shake" : ""}`}
        style={{
          backgroundPositionY: `calc(${hopping > 0 ? OPEN : SHUT} * -64 * var(--bag-px))`,
        }}
      />
    </div>
  )
}

import { useEffect, useRef, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"

/** The Bag's frames in public/game/bag.png: shut, and open at the top. */
const SHUT = 0
const OPEN = 3

/** Time between one item's hop and the next's, and how long a hop takes, in ms. */
const STAGGER = 220
const HOP_MS = 560
/** How high a hop arcs above the straight line to the Bag, in CSS pixels. */
const ARC = 96
/** A hop moves in this many whole steps, the way the game moves sprites. */
const STEPS = 12

/**
 * The Bag from the BAG screen, beside the highlights. The first time the
 * Bag comes well into view, a copy of each highlight's item icon (marked
 * `data-bag-item`) hops into the open Bag in turn, and it gives the shake it
 * gives when you rummage in it as each one lands. After that it shakes when
 * the pointer moves onto a highlight (`data-bag-row`). The highlights
 * themselves never move or hide. Decorative; with motion reduced it just
 * sits there.
 */
export const BagDrop = () => {
  const reducedMotion = useReducedMotion()
  const bagRef = useRef<HTMLDivElement>(null)
  const [shakes, setShakes] = useState(0)
  const [open, setOpen] = useState(false)

  useEffect(() => {
    const bag = bagRef.current
    const section = bag?.closest("section")
    if (!bag || !section || reducedMotion) return
    const shake = () => setShakes((count) => count + 1)
    const timers: number[] = []
    const hops: Animation[] = []

    const hop = (icon: HTMLElement) => {
      const from = icon.getBoundingClientRect()
      const to = bag.getBoundingClientRect()
      const origin = section.getBoundingClientRect()
      const copy = icon.cloneNode() as HTMLElement
      copy.removeAttribute("data-bag-item")
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
        const x = dx * t
        const y = dy * t - ARC * 4 * t * (1 - t)
        return { transform: `translate(${x}px, ${y}px) scale(${1 - 0.6 * t})` }
      })
      const animation = copy.animate(frames, { duration: HOP_MS, easing: `steps(${STEPS})` })
      hops.push(animation)
      animation.finished.then(
        () => {
          copy.remove()
          shake()
        },
        () => copy.remove(),
      )
    }

    const drop = () => {
      const icons = [...section.querySelectorAll<HTMLElement>("[data-bag-item]")]
      setOpen(true)
      icons.forEach((icon, index) =>
        timers.push(window.setTimeout(() => hop(icon), index * STAGGER)),
      )
      timers.push(
        window.setTimeout(() => setOpen(false), (icons.length - 1) * STAGGER + HOP_MS + 200),
      )
    }

    // The Bag itself, whole and clear of the bottom of the screen, starts it:
    // the section can be taller than any threshold of it a phone would reach.
    const observer = new IntersectionObserver(
      ([entry]) => {
        if (!entry?.isIntersecting) return
        observer.disconnect()
        drop()
      },
      { threshold: 1, rootMargin: "0px 0px -20% 0px" },
    )
    observer.observe(bag)

    const onPointerOver = (event: PointerEvent) => {
      const row = (event.target as Element).closest("[data-bag-row]")
      const before =
        event.relatedTarget instanceof Element
          ? event.relatedTarget.closest("[data-bag-row]")
          : null
      if (row && row !== before) shake()
    }
    section.addEventListener("pointerover", onPointerOver)

    return () => {
      observer.disconnect()
      section.removeEventListener("pointerover", onPointerOver)
      timers.forEach((timer) => window.clearTimeout(timer))
      hops.forEach((animation) => animation.cancel())
    }
  }, [reducedMotion])

  return (
    <div ref={bagRef} aria-hidden="true" className="bag-drop shrink-0">
      {/* Keyed by the shake count, so each shake remounts it and plays again. */}
      <div
        key={shakes}
        className={`bag-sprite pixelated ${shakes ? "bag-shake" : ""}`}
        style={{ backgroundPositionY: `calc(${open ? OPEN : SHUT} * -64 * var(--bag-px))` }}
      />
    </div>
  )
}

// The warp between the title scene and the world map, as the game plays a
// door: the screen fades to black, the scene changes, and it fades back in.
// Fading out follows the scroll, so the visitor walks into it; fading in
// plays on its own, so nobody is left on a black screen. The fade's 8 steps
// are the world map's too, for the tour's own warps.
import { FADE_STEPS, stepped } from "~/world-map/fade"

/** How long the fade-in takes: the game's 8 frames, slowed a little to read on a page. */
const FADE_IN_MS = 400
/** Screens before the map's arrival where the fade to black begins. */
const FADE_OUT_FROM = 0.6
/**
 * The darkest the scroll takes it: a step short of black, so stopping just
 * before the map never leaves a black screen. The last step comes with the
 * cut to the map.
 */
const DARKEST = (FADE_STEPS - 1) / FADE_STEPS

/**
 * Drives `overlay`'s opacity from where `stage` is: hidden until its top
 * reaches the top of the screen, at black, and shown from then on. Fires
 * `world-tour:start` on the stage's section the first time the fade-in ends.
 */
export const warp = (stage: HTMLElement, overlay: HTMLElement) => {
  const section = stage.querySelector("section")!
  const reduced = window.matchMedia("(prefers-reduced-motion: reduce)")
  let arrivedAt: number | null = null
  let raf = 0

  const update = (now: number) => {
    raf = 0
    // Screens left until the stage reaches the top; 0 or below once it has.
    const ahead = stage.getBoundingClientRect().top / window.innerHeight
    let level: number
    if (ahead > 0) {
      arrivedAt = null
      stage.toggleAttribute("data-hidden", true)
      level = Math.min(DARKEST, stepped((FADE_OUT_FROM - ahead) / FADE_OUT_FROM))
    } else {
      stage.toggleAttribute("data-hidden", false)
      arrivedAt ??= now
      const t = reduced.matches ? 1 : (now - arrivedAt) / FADE_IN_MS
      level = t >= 1 ? 0 : stepped(1 - t)
      if (t < 1) raf = requestAnimationFrame(update)
      else if (!section.dataset.started) {
        section.dataset.started = "true"
        section.dispatchEvent(new Event("world-tour:start"))
      }
    }
    overlay.style.opacity = String(level)
  }

  const schedule = () => {
    if (!raf) raf = requestAnimationFrame(update)
  }
  window.addEventListener("scroll", schedule, { passive: true })
  window.addEventListener("resize", schedule)
  schedule()
}

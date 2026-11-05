// Bits the player and the studio share: the GBA pixel size for the stage,
// game art drawn at it, and the map name popup's plank.
import { type RefObject, useEffect, useState } from "react"

/** GBA pixels across the frame: the video is 480 of them wide, so 4 CSS pixels each at 1080p. */
const GBA_COLUMNS = 480

/** One GBA pixel in CSS pixels for a stage element, following its width. */
export const useStagePx = (stageRef: RefObject<HTMLElement | null>) => {
  const [px, setPx] = useState(4)
  useEffect(() => {
    const element = stageRef.current
    if (!element) return
    const measure = () => setPx(Math.max(1, Math.round(element.clientWidth / GBA_COLUMNS)))
    measure()
    const observer = new ResizeObserver(measure)
    observer.observe(element)
    return () => observer.disconnect()
  }, [stageRef])
  return px
}

/** Game art at `--px` CSS pixels per pixel, once its size is known. */
export const PixelImage = ({ src, alt }: { src: string; alt: string }) => {
  const [pixels, setPixels] = useState(0)
  return (
    <img
      src={src}
      alt={alt}
      onLoad={(event) => setPixels(event.currentTarget.naturalWidth)}
      style={{
        width: pixels ? `calc(${pixels} * var(--px))` : undefined,
        imageRendering: "pixelated",
      }}
    />
  )
}

/** The game's map name popup: a wood plank at the top left naming where we are. */
export const Popup = ({ label }: { label: string | null }) => (
  <div
    aria-hidden="true"
    className="pointer-events-none absolute top-0 left-0 grid h-[calc(40*var(--px))] w-[calc(96*var(--px))] place-items-center bg-[url(/game/popups/wood.png)] bg-size-[100%_100%] [image-rendering:pixelated] transition-[translate,opacity] duration-[330ms] ease-[steps(20)]"
    style={{ translate: label ? "0 0" : "0 -100%", opacity: label ? 1 : 0 }}
  >
    <span className="gba-text whitespace-nowrap">{label}</span>
  </div>
)

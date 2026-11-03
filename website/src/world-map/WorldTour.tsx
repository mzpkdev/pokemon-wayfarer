import { useEffect, useRef, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"

import { type Camera, fit, fly } from "./camera"
import { type Bounds, DATA, type Meta, loadWorld } from "./data"
import { OPEN_SEA, rendererFor } from "./render"
import { WorldMap } from "./WorldMap"

export type Caption = {
  place: string
  title: string
  body: string
  stats?: { value: string; label: string }[]
}

/**
 * Scroll progress (0 to 1) to the camera: a hold on each shot and eased moves
 * between them. The tour opens in ROUTE 101's tall grass, where the title
 * scene's grass leaves off, walks down into LITTLEROOT TOWN, then pulls back
 * to all of Hoenn and the whole world.
 */
const BEATS = [
  { from: 0, to: 0.06, shot: 0 },
  { from: 0.06, to: 0.2, shot: 0, next: 1 },
  { from: 0.2, to: 0.28, shot: 1 },
  { from: 0.28, to: 0.54, shot: 1, next: 2 },
  { from: 0.54, to: 0.62, shot: 2 },
  { from: 0.62, to: 0.88, shot: 2, next: 3 },
  { from: 0.88, to: 1, shot: 3 },
] as const

/** Which caption each shot shows: the walk into town keeps the first. */
const SHOT_CAPTION = [0, 0, 1, 2]
/** The map name popup the game shows on each close-up shot. */
const SHOT_POPUP = ["ROUTE 101", "LITTLEROOT TOWN", null, null]

/** Rows of the dither that dissolves the page's ground into the map, in scene pixels. */
const SEAM_ROWS = 20
/** A 4x4 ordered dither, thresholds 0 to 15. */
const BAYER = [0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5]

/**
 * The game's yellow OK button from the naming screen; pixel corners clip an
 * outline, so focus shows as a blue rim inside instead.
 */
const BUTTON =
  "pixel-corners pixel-label bg-[rgb(232_208_64)] px-7 pt-2 pb-3 text-[rgb(56_56_56)] shadow-[inset_0_0_0_4px_rgb(56_56_56),inset_0_-8px_0_rgb(184_160_32),inset_0_8px_0_rgb(248_240_152)] hover:bg-[rgb(248_224_96)] focus-visible:shadow-[inset_0_0_0_4px_rgb(48_80_200),inset_0_-8px_0_rgb(184_160_32),inset_0_8px_0_rgb(248_240_152)] focus-visible:outline-none active:translate-y-0.5"

/** Metatiles across and down in the close-up shots. */
const CLOSE_UP = { width: 44, height: 30 }

const boundsOf = (maps: Meta["maps"]): Bounds => ({
  left: Math.min(...maps.map(({ x }) => x)),
  top: Math.min(...maps.map(({ y }) => y)),
  right: Math.max(...maps.map(({ x, width }) => x + width)),
  bottom: Math.max(...maps.map(({ y, height }) => y + height)),
})

/** The shots for a map area `width` by `height` CSS pixels. */
const shotsFor = (meta: Meta, width: number, height: number): Camera[] => {
  const littleroot = meta.towns.find(({ name }) => name === "LittlerootTown")!
  const route101 = meta.maps.find(({ name }) => name === "Route101")!
  const hoenn =
    meta.regions.find(({ name }) => name === "HOENN")?.bounds ??
    boundsOf(meta.maps.filter(({ region }) => region === "Hoenn"))
  const close = Math.min(width / CLOSE_UP.width, height / CLOSE_UP.height)
  return [
    // Close enough that ROUTE 101, grass and all, fills the frame.
    {
      x: route101.x + route101.width / 2,
      y: route101.y + route101.height / 2,
      zoom: Math.min(width / (route101.width + 4), height / (route101.height + 2)),
    },
    { x: littleroot.x, y: littleroot.y, zoom: close },
    fit(hoenn, width, height, 0.95),
    fit(boundsOf(meta.maps), width, height, 0.92),
  ]
}

const cameraAt = (shots: Camera[], progress: number) => {
  const p = Math.min(1, Math.max(0, progress))
  const beat = BEATS.find(({ to }) => p <= to) ?? BEATS.at(-1)!
  const from = shots[beat.shot]!
  if (!("next" in beat)) return { camera: from, shot: beat.shot }
  const t = (p - beat.from) / (beat.to - beat.from)
  return { camera: fly(from, shots[beat.next]!, t), shot: t < 0.5 ? beat.shot : beat.next }
}

/**
 * Dissolves a band of `color` at the top of the section into the map, in
 * whole `pixel`-sized dots, so the page above runs into the map without a
 * hard edge. `top` is where the section's top is on the canvas.
 */
const drawSeam = (context: CanvasRenderingContext2D, top: number, pixel: number, color: string) => {
  const ratio = window.devicePixelRatio
  const dot = pixel * ratio
  const y0 = top * ratio
  if (y0 + SEAM_ROWS * dot <= 0) return
  context.setTransform(1, 0, 0, 1, 0, 0)
  context.fillStyle = color
  const columns = Math.ceil(context.canvas.width / dot)
  for (let row = 0; row < SEAM_ROWS; row++) {
    const y = Math.round(y0 + row * dot)
    const next = Math.round(y0 + (row + 1) * dot)
    if (next <= 0) continue
    // How much of the row is still ground, from all of it to none.
    const cover = 16 * (1 - (row + 0.5) / SEAM_ROWS)
    for (let column = 0; column < columns; column++)
      if (BAYER[(row % 4) * 4 + (column % 4)]! < cover) {
        const x = Math.round(column * dot)
        context.fillRect(x, y, Math.round((column + 1) * dot) - x, next - y)
      }
  }
}

/**
 * The world in the game's own tiles, with a camera that starts in ROUTE
 * 101's tall grass, walks into LITTLEROOT TOWN and pulls back to all of
 * Hoenn, then to the whole world, as its section scrolls by, and a button to
 * open the map and look around. Fills its sticky parent; the section around
 * it sets how long the tour lasts.
 *
 * `seam` names the color of the page right above the section and the CSS
 * variable holding its pixel size, for the dither that joins the two.
 */
export const WorldTour = ({
  captions,
  base = DATA,
  seam,
}: {
  captions: Caption[]
  base?: string
  seam?: { color: string; pixel: string }
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const captionRef = useRef<HTMLDivElement>(null)
  const dialogRef = useRef<HTMLDialogElement>(null)
  const cameraRef = useRef<Camera | null>(null)
  const reducedMotion = useReducedMotion()
  const [beat, setBeat] = useState(0)
  const [exploring, setExploring] = useState<Camera | null>(null)
  const [popup, setPopup] = useState<{ label: string | null; shown: boolean }>({
    label: null,
    shown: false,
  })
  const seamColor = seam?.color
  const seamPixel = seam?.pixel

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    const section = canvas?.closest("section")
    if (!canvas || !context || !section) return
    let raf = 0
    let disposed = false
    let cleanup = () => {}

    // The map name popup slides away, then back with the next name, the way
    // the game's does on walking into a new area.
    let place: string | null = null
    let popupTimer = 0
    const showPlace = (next: string | null) => {
      if (next === place) return
      place = next
      window.clearTimeout(popupTimer)
      setPopup((current) => ({ ...current, shown: false }))
      if (next) popupTimer = window.setTimeout(() => setPopup({ label: next, shown: true }), 340)
    }

    const progress = () => {
      if (reducedMotion) return 1
      const { top, height } = section.getBoundingClientRect()
      const travel = height - window.innerHeight
      return travel > 0 ? -top / travel : 1
    }

    void loadWorld(base).then((world) => {
      if (disposed) return
      const renderer = rendererFor(world)
      const draw = () => {
        raf = 0
        // The map gets the screen beside the caption on wide screens, above
        // it on narrow ones.
        const box = canvas.getBoundingClientRect()
        const area = { width: box.width, height: box.height }
        const caption = captionRef.current?.getBoundingClientRect()
        if (caption) {
          if (caption.left - box.left > box.width * 0.4) area.width = caption.left - box.left - 24
          else area.height = caption.top - box.top - 16
        }
        const shots = shotsFor(world.meta, area.width, area.height)
        const scrolled = progress()
        const { camera, shot } = cameraAt(shots, scrolled)
        cameraRef.current = camera
        setBeat(SHOT_CAPTION[shot]!)
        // The popup names the close-ups once the map has the whole screen.
        showPlace(scrolled >= 0 ? SHOT_POPUP[shot]! : null)
        const pending = renderer.draw(context, {
          camera,
          focus: { x: area.width / 2, y: area.height / 2 },
          labelWidth: area.width,
          // The popup names the close-ups; names on the map come in as it pulls back.
          labels: camera.zoom < shots[1]!.zoom * 0.7,
        })
        if (seamColor && seamPixel) {
          const pixel = parseFloat(getComputedStyle(canvas).getPropertyValue(seamPixel)) || 4
          drawSeam(context, section.getBoundingClientRect().top - box.top, pixel, seamColor)
        }
        if (pending) schedule()
      }
      const schedule = () => {
        if (!raf) raf = requestAnimationFrame(draw)
      }
      window.addEventListener("scroll", schedule, { passive: true })
      const resizer = new ResizeObserver(schedule)
      resizer.observe(canvas)
      cleanup = () => {
        window.removeEventListener("scroll", schedule)
        resizer.disconnect()
      }
      void document.fonts.ready.then(schedule)
      schedule()
    })

    return () => {
      disposed = true
      cancelAnimationFrame(raf)
      window.clearTimeout(popupTimer)
      cleanup()
    }
  }, [base, reducedMotion, seamColor, seamPixel])

  // The explorer opens as a modal over the page, starting where the tour is.
  useEffect(() => {
    if (exploring) dialogRef.current?.showModal()
  }, [exploring])

  return (
    <>
      <canvas
        ref={canvasRef}
        aria-hidden="true"
        className="absolute inset-0 size-full"
        style={{ background: OPEN_SEA }}
      />
      <div
        aria-hidden="true"
        className="pointer-events-none absolute top-0 left-0 grid h-[calc(40*var(--px))] w-[calc(96*var(--px))] place-items-center bg-[url(/game/popups/wood.png)] bg-size-[100%_100%] [image-rendering:pixelated] transition-[translate,opacity] duration-[330ms] ease-[steps(20)] motion-reduce:transition-none"
        style={{
          translate: popup.shown ? "0 0" : "0 -100%",
          opacity: popup.shown ? 1 : 0,
        }}
      >
        <span className="gba-text whitespace-nowrap">{popup.label}</span>
      </div>
      <div className="pointer-events-none relative flex h-full items-end justify-end p-4 sm:p-8 lg:items-center lg:p-12">
        <div
          ref={captionRef}
          className="pixel-corners pixel-panel pointer-events-auto relative w-full p-6 sm:p-8 lg:w-[400px]"
        >
          <div className="grid">
            {captions.map((caption, i) => {
              const shown = (reducedMotion ? captions.length - 1 : beat) === i
              return (
                <div
                  key={caption.place}
                  aria-hidden={!shown}
                  className="col-start-1 row-start-1 transition-opacity duration-300"
                  style={{ opacity: shown ? 1 : 0 }}
                >
                  <p className="pixel-label text-logo-yellow">{caption.place}</p>
                  <p className="pixel-heading mt-3">{caption.title}</p>
                  <p className="mt-4 text-lg leading-relaxed text-paper/80">{caption.body}</p>
                  {caption.stats && (
                    <dl className="mt-6 grid grid-cols-3 gap-4">
                      {caption.stats.map(({ value, label }) => (
                        <div key={label}>
                          <dt className="sr-only">{label}</dt>
                          <dd className="logo-number text-4xl">{value}</dd>
                          <dd className="mt-1 text-sm text-paper/70" aria-hidden="true">
                            {label}
                          </dd>
                        </div>
                      ))}
                    </dl>
                  )}
                </div>
              )
            })}
          </div>
          <button
            type="button"
            onClick={() => setExploring(cameraRef.current ?? { x: 0, y: 0, zoom: 0 })}
            className={`${BUTTON} mt-6`}
          >
            EXPLORE THE MAP
          </button>
        </div>
      </div>
      <dialog
        ref={dialogRef}
        aria-label="World map"
        onClose={() => setExploring(null)}
        className="m-0 h-svh max-h-none w-screen max-w-none bg-transparent p-0 backdrop:bg-black/70"
      >
        {exploring && (
          <>
            <WorldMap base={base} start={exploring.zoom > 0 ? exploring : undefined} />
            <button
              type="button"
              onClick={() => dialogRef.current?.close()}
              className={`${BUTTON} absolute top-4 right-4`}
            >
              CLOSE
            </button>
          </>
        )}
      </dialog>
    </>
  )
}

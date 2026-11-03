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
 * Scroll progress (0 to 1) to the camera: a hold on each shot (LITTLEROOT
 * TOWN, all of Hoenn, the whole world) and eased pulls between them.
 */
const BEATS = [
  { from: 0, to: 0.12, shot: 0 },
  { from: 0.12, to: 0.42, shot: 0, next: 1 },
  { from: 0.42, to: 0.52, shot: 1 },
  { from: 0.52, to: 0.86, shot: 1, next: 2 },
  { from: 0.86, to: 1, shot: 2 },
] as const

/**
 * The game's yellow OK button from the naming screen; pixel corners clip an
 * outline, so focus shows as a blue rim inside instead.
 */
const BUTTON =
  "pixel-corners pixel-label bg-[rgb(232_208_64)] px-7 pt-2 pb-3 text-[rgb(56_56_56)] shadow-[inset_0_0_0_4px_rgb(56_56_56),inset_0_-8px_0_rgb(184_160_32),inset_0_8px_0_rgb(248_240_152)] hover:bg-[rgb(248_224_96)] focus-visible:shadow-[inset_0_0_0_4px_rgb(48_80_200),inset_0_-8px_0_rgb(184_160_32),inset_0_8px_0_rgb(248_240_152)] focus-visible:outline-none active:translate-y-0.5"

/** Metatiles of LITTLEROOT TOWN and around it in the first shot, across and down. */
const CLOSE_UP = { width: 44, height: 30 }

const boundsOf = (maps: Meta["maps"]): Bounds => ({
  left: Math.min(...maps.map(({ x }) => x)),
  top: Math.min(...maps.map(({ y }) => y)),
  right: Math.max(...maps.map(({ x, width }) => x + width)),
  bottom: Math.max(...maps.map(({ y, height }) => y + height)),
})

/** The three shots for a map area `width` by `height` CSS pixels. */
const shotsFor = (meta: Meta, width: number, height: number): Camera[] => {
  const littleroot = meta.towns.find(({ name }) => name === "LittlerootTown")!
  const hoenn =
    meta.regions.find(({ name }) => name === "HOENN")?.bounds ??
    boundsOf(meta.maps.filter(({ region }) => region === "Hoenn"))
  return [
    {
      x: littleroot.x,
      y: littleroot.y,
      zoom: Math.min(width / CLOSE_UP.width, height / CLOSE_UP.height),
    },
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
 * The world in the game's own tiles, with a camera that starts on LITTLEROOT
 * TOWN and pulls back to all of Hoenn, then to the whole world, as its
 * section scrolls by, and a button to open the map and look around. Fills
 * its sticky parent; the section around it sets how long the pull lasts.
 */
export const WorldTour = ({ captions, base = DATA }: { captions: Caption[]; base?: string }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const captionRef = useRef<HTMLDivElement>(null)
  const dialogRef = useRef<HTMLDialogElement>(null)
  const cameraRef = useRef<Camera | null>(null)
  const reducedMotion = useReducedMotion()
  const [beat, setBeat] = useState(0)
  const [exploring, setExploring] = useState<Camera | null>(null)

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    const section = canvas?.closest("section")
    if (!canvas || !context || !section) return
    let raf = 0
    let disposed = false
    let cleanup = () => {}

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
        const { camera, shot } = cameraAt(shotsFor(world.meta, area.width, area.height), progress())
        cameraRef.current = camera
        setBeat(shot)
        const pending = renderer.draw(context, {
          camera,
          focus: { x: area.width / 2, y: area.height / 2 },
          labelWidth: area.width,
          labels: true,
        })
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
      cleanup()
    }
  }, [base, reducedMotion])

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

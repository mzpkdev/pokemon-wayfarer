import { useEffect, useRef, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"

import { type Camera, fit, fly } from "./camera"
import { type Bounds, DATA, type Meta, loadWorld } from "./data"
import { OPEN_SEA, rendererFor } from "./render"
import { RIDER_SRC, riderAt, routeIn } from "./rider"
import { WorldMap } from "./WorldMap"

export type Caption = {
  place: string
  title: string
  body: string
  stats?: { value: string; label: string }[]
}

/** How long BRENDAN's ride from the grass to his door takes, in milliseconds. */
const RIDE_MS = 3200
/** How long a flight between stops takes: longer the more the zoom changes. */
const flightMs = (from: Camera, to: Camera) =>
  Math.min(2400, 900 + 450 * Math.abs(Math.log2(to.zoom / from.zoom)))
/** The closest zoom he is drawn at; further out he is a speck. */
const RIDER_MIN_ZOOM = 4

/**
 * The fade in from a warp, as the game plays it on stepping out of a door:
 * the palette blends from the fade color to full color in 8 steps (deltaY 2
 * in palette.c), a little slower here so it reads on a page.
 */
const FADE_STEPS = 8
const FADE_MS = 400
/** A beat between the fade ending and BRENDAN setting off. */
const RIDE_DELAY_MS = 250

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

/**
 * The world in the game's own tiles, held still at one stop at a time with a
 * card beside it. When the section comes into view, BRENDAN rides out of the
 * grass down ROUTE 101 to his door in LITTLEROOT TOWN; the card's button then
 * flies the camera on to all of Hoenn and the whole world, and the last one
 * opens the map to look around. The page scrolls past it as usual. Fills its
 * section.
 *
 * The map starts hidden under `fade`, the color of the page above it, and
 * fades in the way the game's screen does on stepping out of a door.
 */
export const WorldTour = ({
  captions,
  base = DATA,
  fade = "rgb(0 0 0)",
}: {
  captions: Caption[]
  base?: string
  fade?: string
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const captionRef = useRef<HTMLDivElement>(null)
  const dialogRef = useRef<HTMLDialogElement>(null)
  const cameraRef = useRef<Camera | null>(null)
  const goRef = useRef<(stop: number) => void>(() => {})
  const reducedMotion = useReducedMotion()
  const [stop, setStop] = useState(0)
  const [exploring, setExploring] = useState<Camera | null>(null)
  const [popup, setPopup] = useState<{ label: string | null; shown: boolean }>({
    label: null,
    shown: false,
  })

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

    // Where the tour is: the stop shown (its shot is one past it, after ROUTE
    // 101), the flight under way if any, and when the ride began.
    let current = 0
    let flight: { from: Camera; to: number; start: number; duration: number } | null = null
    let fadeStart: number | null = null

    const rider = new Image()
    rider.src = RIDER_SRC
    void Promise.all([loadWorld(base), rider.decode().catch(() => {})]).then(([world]) => {
      if (disposed) return
      const renderer = rendererFor(world)
      const route = routeIn(world.meta)
      const littlerootTop = world.meta.maps.find(({ name }) => name === "LittlerootTown")!.y

      const area = () => {
        // The map gets the screen beside the caption on wide screens, above
        // it on narrow ones.
        const box = canvas.getBoundingClientRect()
        const size = { width: box.width, height: box.height }
        const caption = captionRef.current?.getBoundingClientRect()
        if (caption) {
          if (caption.left - box.left > box.width * 0.4) size.width = caption.left - box.left - 24
          else size.height = caption.top - box.top - 16
        }
        return size
      }

      const draw = (now: number) => {
        raf = 0
        const { width, height } = area()
        const shots = shotsFor(world.meta, width, height)
        const rideStart = fadeStart === null ? null : fadeStart + FADE_MS + RIDE_DELAY_MS
        const ride = reducedMotion
          ? 1
          : rideStart === null
            ? 0
            : Math.max(0, (now - rideStart) / RIDE_MS)
        let camera: Camera
        if (flight) {
          const t = (now - flight.start) / flight.duration
          camera = fly(flight.from, shots[flight.to + 1]!, t)
          if (t >= 1) flight = null
        } else if (current === 0) camera = fly(shots[0]!, shots[1]!, ride)
        else camera = shots[current + 1]!
        cameraRef.current = camera

        const brendan = riderAt(route, ride, rider)
        // The popup names where BRENDAN is while the camera is with him.
        showPlace(
          current > 0 || rideStart === null || now < rideStart
            ? null
            : brendan.y + 2 < littlerootTop
              ? "ROUTE 101"
              : "LITTLEROOT TOWN",
        )
        const pending = renderer.draw(context, {
          camera,
          focus: { x: width / 2, y: height / 2 },
          labelWidth: width,
          // The popup names the close-ups; names on the map come in as it pulls back.
          labels: camera.zoom < shots[1]!.zoom * 0.7,
          sprites:
            rider.complete && rider.naturalWidth && camera.zoom >= RIDER_MIN_ZOOM ? [brendan] : [],
        })
        // The fade, in the game's whole steps.
        const hidden = reducedMotion
          ? 0
          : fadeStart === null
            ? 1
            : Math.ceil(Math.max(0, 1 - (now - fadeStart) / FADE_MS) * FADE_STEPS) / FADE_STEPS
        if (hidden > 0) {
          context.setTransform(1, 0, 0, 1, 0, 0)
          context.globalAlpha = hidden
          context.fillStyle = fade
          context.fillRect(0, 0, canvas.width, canvas.height)
          context.globalAlpha = 1
        }
        const riding = rideStart !== null && ride < 1
        if (pending || flight || riding) schedule()
      }
      const schedule = () => {
        if (!raf) raf = requestAnimationFrame(draw)
      }

      goRef.current = (next) => {
        if (next === current) return
        const { width, height } = area()
        const from = cameraRef.current ?? shotsFor(world.meta, width, height)[current + 1]!
        const to = shotsFor(world.meta, width, height)[next + 1]!
        flight = reducedMotion
          ? null
          : { from, to: next, start: performance.now(), duration: flightMs(from, to) }
        current = next
        schedule()
      }

      // The fade, then the ride, start the first time nearly all of the
      // section is on screen.
      const watcher = new IntersectionObserver(
        ([entry]) => {
          if (!entry?.isIntersecting || fadeStart !== null) return
          fadeStart = performance.now()
          watcher.disconnect()
          schedule()
        },
        { threshold: 0.7 },
      )
      watcher.observe(section)
      const resizer = new ResizeObserver(schedule)
      resizer.observe(canvas)
      cleanup = () => {
        watcher.disconnect()
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
  }, [base, reducedMotion, fade])

  const go = (next: number) => {
    setStop(next)
    goRef.current(next)
  }

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
          <div className="grid" aria-live="polite">
            {captions.map((caption, i) => {
              const shown = stop === i
              return (
                <div
                  key={caption.place}
                  aria-hidden={!shown}
                  className="col-start-1 row-start-1 transition-opacity duration-300"
                  style={{ opacity: shown ? 1 : 0 }}
                >
                  <p className="pixel-label flex justify-between gap-4 text-logo-yellow">
                    <span>{caption.place}</span>
                    <span
                      className="text-paper/60"
                      aria-label={`Stop ${i + 1} of ${captions.length}`}
                    >
                      {i + 1}/{captions.length}
                    </span>
                  </p>
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
          <div className="mt-6 flex items-center justify-between gap-4">
            {stop > 0 ? (
              <button
                type="button"
                onClick={() => go(stop - 1)}
                className="pixel-label px-2 pt-2 pb-3 text-paper/70 hover:text-paper focus-visible:text-paper focus-visible:underline focus-visible:outline-none"
              >
                BACK
              </button>
            ) : (
              <span />
            )}
            {stop < captions.length - 1 ? (
              <button type="button" onClick={() => go(stop + 1)} className={BUTTON}>
                NEXT ▶
              </button>
            ) : (
              <button
                type="button"
                onClick={() => setExploring(cameraRef.current ?? { x: 0, y: 0, zoom: 0 })}
                className={BUTTON}
              >
                EXPLORE THE MAP
              </button>
            )}
          </div>
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

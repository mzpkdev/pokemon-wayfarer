import { type ReactNode, useCallback, useEffect, useRef, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"

import { type Camera, fit } from "./camera"
import { cloudsOverlay } from "./clouds"
import { DATA, type Meta, loadWorld } from "./data"
import { lightsOverlay } from "./lights"
import { type Encounters, loadEncounters } from "./encounters"
import { pageMinutes } from "./clock"
import { LivingClock } from "./LivingClock"
import { type LivingWorld, livingOverlay, loadLiving } from "./living"
import { type Notable, type Notables, loadNotables, notablesOverlay, pickNotable } from "./notables"
import { DebugMenu, useDebug } from "./DebugMenu"
import { PlaceCard, type PlaceInfo } from "./PlaceCard"
import { OPEN_SEA, type Overlay, rendererFor } from "./render"
import { seaOverlay } from "./sea"
import { sightingsOverlay } from "./sightings"
import { loadWater } from "./water"
import { weatherOverlay } from "./weather"

export type { PlaceInfo } from "./PlaceCard"

/** The closest zoom, in CSS pixels per metatile. */
const MAX_ZOOM = 48
/** A press shorter than this, in ms, and moving less than `TAP_SLOP` CSS pixels, is a tap, not a drag. */
const TAP_TIME = 500
const TAP_SLOP = 8

type PlacedMap = Meta["maps"][number]

/** The smallest map whose rectangle holds the grid point, if any. */
const mapAt = (maps: PlacedMap[], x: number, y: number) => {
  let found: PlacedMap | undefined
  for (const map of maps)
    if (x >= map.x && x < map.x + map.width && y >= map.y && y < map.y + map.height)
      if (!found || map.width * map.height < found.width * found.height) found = map
  return found
}

/** A subtle frame and tint over the picked map. */
const highlight =
  (map: PlacedMap): Overlay =>
  (context, { toScreen, ratio }) => {
    const a = toScreen(map.x, map.y)
    const b = toScreen(map.x + map.width, map.y + map.height)
    context.fillStyle = "rgb(255 214 0 / 0.1)"
    context.fillRect(a.x, a.y, b.x - a.x, b.y - a.y)
    const line = Math.max(2, Math.round(2 * ratio))
    context.strokeStyle = "rgb(255 214 0 / 0.85)"
    context.lineWidth = line
    context.strokeRect(a.x + line / 2, a.y + line / 2, b.x - a.x - line, b.y - a.y - line)
  }

/** What the canvas effect offers the component: a redraw and a nudge to bring a map into the free part of the view. */
type Api = {
  redraw: () => void
  /** Pans so the map is clear of `covered` (a box in CSS pixels over the canvas). */
  reveal: (name: string, covered: { left: number; top: number }) => void
}

/**
 * The whole world to pan and zoom: drag to pan, pinch, wheel or double-click
 * to zoom, or the buttons. Close in, wild Pokémon pop out of the routes' grass
 * and water. Tap a map (or press Enter on the one at the center) for a card
 * with its name; its exact encounter rates are in the debug menu (`?debug`, or
 * the backquote key). Fills its parent.
 *
 * `extra` renders into the card under the name, for what else is at the place.
 */
export const WorldMap = ({
  base = DATA,
  start,
  extra,
}: {
  base?: string
  start?: Camera
  extra?: (place: PlaceInfo) => ReactNode
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const cardRef = useRef<HTMLElement>(null)
  const zoomBy = useRef<(factor: number) => void>(() => {})
  const api = useRef<Api>({ redraw: () => {}, reveal: () => {} })
  const [failed, setFailed] = useState(false)
  // The picked map (its folder name), and what the explorer has loaded to say about it.
  const [picked, setPicked] = useState<string | null>(null)
  const pickedRef = useRef<string | null>(null)
  const [encounters, setEncounters] = useState<Encounters>()
  const [encountersFailed, setEncountersFailed] = useState(false)
  // The ships and the time of day, on the page's shared clock.
  const [living, setLiving] = useState<LivingWorld>()
  const [minutes, setMinutes] = useState(pageMinutes)
  // With reduced motion the ships and walkers hold still, moving on the clock's tick.
  const reducedMotion = useReducedMotion()
  const reducedRef = useRef(reducedMotion)
  useEffect(() => {
    reducedRef.current = reducedMotion
    api.current.redraw()
  }, [reducedMotion])
  useEffect(() => {
    void loadLiving(base).then(setLiving, () => {})
    const timer = window.setInterval(() => {
      setMinutes(pageMinutes())
      if (reducedRef.current) api.current.redraw()
    }, 500)
    return () => window.clearInterval(timer)
  }, [base])
  const livingRef = useRef<LivingWorld>(undefined)
  useEffect(() => {
    livingRef.current = living
    api.current.redraw()
  }, [living])
  // The Gym Leaders and Elite Four, and the one a tap last found.
  const [notables, setNotables] = useState<Notables>()
  const notablesRef = useRef<Notables>(undefined)
  const [spotted, setSpotted] = useState<Notable>()
  useEffect(() => {
    void loadNotables(base).then(setNotables, () => {})
  }, [base])
  useEffect(() => {
    notablesRef.current = notables
    api.current.redraw()
  }, [notables])
  useEffect(() => {
    if (!spotted) return
    const timer = window.setTimeout(() => setSpotted(undefined), 5000)
    return () => window.clearTimeout(timer)
  }, [spotted])
  const refocus = useRef(false)

  // Encounters load with the map: the sightings need them, and so do the place cards.
  useEffect(() => {
    if (encounters) return
    void loadEncounters(base).then(
      (loaded) => {
        setEncounters(loaded)
        setEncountersFailed(false)
      },
      () => setEncountersFailed(true),
    )
  }, [encounters, base])
  // The wild Pokémon that pop out close in, once their icons are in.
  const sightingsRef = useRef<Overlay>(undefined)
  useEffect(() => {
    if (!encounters) return
    const icons = new Image()
    icons.src = `${base}/icons.png`
    let live = true
    void icons
      .decode()
      .catch(() => {})
      .then(() => {
        if (!live) return
        sightingsRef.current = sightingsOverlay(
          encounters,
          icons,
          pageMinutes,
          !reducedMotion,
          performance.now.bind(performance),
        )
        api.current.redraw()
      })
    return () => {
      live = false
    }
  }, [encounters, base, reducedMotion])
  const debug = useDebug()
  // The sea, the weather and the clouds hold still with reduced motion: their clock stops.
  const clock = useCallback(() => (reducedRef.current ? 0 : performance.now()), [])

  // Redraw with the frame on the picked map, and bring the map out from under the card.
  useEffect(() => {
    pickedRef.current = picked
    api.current.redraw()
    if (!picked || (!encounters && !encountersFailed)) return
    const frame = requestAnimationFrame(() => {
      const card = cardRef.current?.getBoundingClientRect()
      const canvas = canvasRef.current?.getBoundingClientRect()
      if (!card || !canvas) return
      // A sheet across the bottom covers from its top; a panel at the left, up to its right edge.
      const sheet = card.width > canvas.width * 0.8
      api.current.reveal(
        picked,
        sheet
          ? { left: 0, top: card.top - canvas.top }
          : { left: card.right - canvas.left, top: 0 },
      )
      if (refocus.current) cardRef.current?.querySelector("button")?.focus()
    })
    return () => cancelAnimationFrame(frame)
  }, [picked, encounters, encountersFailed])

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    if (!canvas || !context) return
    const listening = new AbortController()
    const { signal } = listening
    let raf = 0
    const choose = (name: string | null, keyboard = false) => {
      refocus.current = keyboard
      setPicked(name)
    }

    void loadWorld(base).then(
      async (world) => {
        if (signal.aborted) return
        const { width, height } = world.meta
        const renderer = rendererFor(world)
        let sea: Overlay | undefined
        void loadWater(world.meta, base).then(
          (water) => {
            if (signal.aborted) return
            sea = seaOverlay(water, true, clock)
            schedule()
          },
          () => {},
        )
        const weather = weatherOverlay(world.meta, true, clock)
        const clouds = cloudsOverlay(true, clock)
        // The night lights, made once per loaded living world.
        let lights: { of: LivingWorld; overlay: Overlay } | undefined
        const lightsFor = (of: LivingWorld) => {
          if (lights?.of !== of)
            lights = { of, overlay: lightsOverlay(world.meta, of, pageMinutes, true, clock) }
          return lights.overlay
        }
        const all = { left: 0, top: 0, right: width, bottom: height }
        const fitZoom = () => fit(all, canvas.clientWidth, canvas.clientHeight).zoom
        const camera = start ? { ...start } : fit(all, canvas.clientWidth, canvas.clientHeight)
        const clamp = () => {
          camera.zoom = Math.min(MAX_ZOOM, Math.max(fitZoom() * 0.8, camera.zoom))
          camera.x = Math.min(width, Math.max(0, camera.x))
          camera.y = Math.min(height, Math.max(0, camera.y))
        }

        const draw = () => {
          raf = 0
          const highlighted = pickedRef.current
            ? world.meta.maps.find((map) => map.name === pickedRef.current)
            : undefined
          const pending = renderer.draw(context, {
            camera,
            focus: { x: canvas.clientWidth / 2, y: canvas.clientHeight / 2 },
            labelWidth: canvas.clientWidth,
            labels: true,
            overlays: [
              ...(sea ? [sea] : []),
              ...(sightingsRef.current ? [sightingsRef.current] : []),
              ...(notablesRef.current
                ? [
                    notablesOverlay(notablesRef.current, pageMinutes, {
                      standing: reducedRef.current,
                    }),
                  ]
                : []),
              weather,
              clouds,
              ...(livingRef.current
                ? [
                    livingOverlay(livingRef.current, pageMinutes, { still: reducedRef.current }),
                    lightsFor(livingRef.current),
                  ]
                : []),
              ...(highlighted ? [highlight(highlighted)] : []),
            ],
          })
          // The sea glints, the clouds drift, the ships sail and the wild
          // Pokémon pop while the map is open.
          if (pending || !reducedRef.current) schedule()
        }
        const schedule = () => {
          if (!raf) raf = requestAnimationFrame(draw)
        }

        const zoomAt = (factor: number, screenX: number, screenY: number) => {
          const before = camera.zoom
          camera.zoom *= factor
          clamp()
          const applied = camera.zoom / before
          // Keep the world point under the cursor in place.
          const dx = screenX - canvas.clientWidth / 2
          const dy = screenY - canvas.clientHeight / 2
          camera.x += (dx / before) * (1 - 1 / applied)
          camera.y += (dy / before) * (1 - 1 / applied)
          clamp()
          schedule()
        }
        api.current = {
          redraw: schedule,
          reveal: (name, covered) => {
            const map = world.meta.maps.find((m) => m.name === name)
            if (!map) return
            const w = canvas.clientWidth
            const h = canvas.clientHeight
            // The part of the view the card leaves free.
            const free =
              covered.left > 0
                ? { left: covered.left, top: 0, right: w, bottom: h }
                : { left: 0, top: 0, right: w, bottom: covered.top }
            const at = (x: number, y: number) => ({
              x: w / 2 + (x - camera.x) * camera.zoom,
              y: h / 2 + (y - camera.y) * camera.zoom,
            })
            const centre = at(map.x + map.width / 2, map.y + map.height / 2)
            const margin = 24
            const inside =
              centre.x > free.left + margin &&
              centre.x < free.right - margin &&
              centre.y > free.top + margin &&
              centre.y < free.bottom - margin
            if (inside) return
            // Ease the map's centre to the middle of the free part.
            const from = { x: camera.x, y: camera.y }
            const to = {
              x: map.x + map.width / 2 - ((free.left + free.right) / 2 - w / 2) / camera.zoom,
              y: map.y + map.height / 2 - ((free.top + free.bottom) / 2 - h / 2) / camera.zoom,
            }
            const began = performance.now()
            const step = (now: number) => {
              if (signal.aborted) return
              const t = Math.min(1, (now - began) / 280)
              const eased = 1 - (1 - t) ** 3
              camera.x = from.x + (to.x - from.x) * eased
              camera.y = from.y + (to.y - from.y) * eased
              clamp()
              schedule()
              if (t < 1) requestAnimationFrame(step)
            }
            requestAnimationFrame(step)
          },
        }
        zoomBy.current = (factor) => zoomAt(factor, canvas.clientWidth / 2, canvas.clientHeight / 2)

        // Drag to pan, pinch to zoom, wheel to zoom at the cursor.
        const pointers = new Map<number, { x: number; y: number }>()
        let pinch = 0
        // The press that might turn out to be a tap.
        let press: { id: number; x: number; y: number; at: number } | null = null
        const placeAt = (screenX: number, screenY: number) =>
          mapAt(
            world.meta.maps,
            camera.x + (screenX - canvas.clientWidth / 2) / camera.zoom,
            camera.y + (screenY - canvas.clientHeight / 2) / camera.zoom,
          )
        canvas.addEventListener(
          "pointerdown",
          (event) => {
            canvas.setPointerCapture(event.pointerId)
            pointers.set(event.pointerId, { x: event.offsetX, y: event.offsetY })
            press =
              pointers.size === 1
                ? { id: event.pointerId, x: event.offsetX, y: event.offsetY, at: event.timeStamp }
                : null
            if (pointers.size === 2) {
              const [a, b] = [...pointers.values()]
              pinch = Math.hypot(a!.x - b!.x, a!.y - b!.y)
            }
          },
          { signal },
        )
        canvas.addEventListener(
          "pointermove",
          (event) => {
            const last = pointers.get(event.pointerId)
            if (!last) {
              // Hovering: a pointer over what a tap answers (a walker, a map),
              // the open hand over the sea.
              if (event.pointerType === "touch") return
              const tappable =
                (notablesRef.current &&
                  pickNotable(
                    notablesRef.current,
                    pageMinutes(),
                    camera.x + (event.offsetX - canvas.clientWidth / 2) / camera.zoom,
                    camera.y + (event.offsetY - canvas.clientHeight / 2) / camera.zoom,
                    camera.zoom,
                    canvas.clientWidth,
                  )) ||
                placeAt(event.offsetX, event.offsetY)
              canvas.style.cursor = tappable ? "pointer" : ""
              return
            }
            canvas.style.cursor = ""
            const now = { x: event.offsetX, y: event.offsetY }
            pointers.set(event.pointerId, now)
            if (press && Math.hypot(now.x - press.x, now.y - press.y) > TAP_SLOP) press = null
            if (pointers.size === 1) {
              camera.x -= (now.x - last.x) / camera.zoom
              camera.y -= (now.y - last.y) / camera.zoom
              clamp()
              schedule()
            } else if (pointers.size === 2) {
              const [a, b] = [...pointers.values()]
              const distance = Math.hypot(a!.x - b!.x, a!.y - b!.y)
              if (pinch > 0) zoomAt(distance / pinch, (a!.x + b!.x) / 2, (a!.y + b!.y) / 2)
              pinch = distance
            }
          },
          { signal },
        )
        const release = (event: PointerEvent) => {
          pointers.delete(event.pointerId)
          pinch = 0
        }
        canvas.addEventListener(
          "pointerup",
          (event) => {
            const tapped =
              press?.id === event.pointerId &&
              event.timeStamp - press.at < TAP_TIME &&
              Math.hypot(event.offsetX - press.x, event.offsetY - press.y) <= TAP_SLOP
            release(event)
            if (!tapped) return
            // A tap on a Gym Leader or Elite Four names them and leaves the map alone.
            const found = notablesRef.current
              ? pickNotable(
                  notablesRef.current,
                  pageMinutes(),
                  camera.x + (event.offsetX - canvas.clientWidth / 2) / camera.zoom,
                  camera.y + (event.offsetY - canvas.clientHeight / 2) / camera.zoom,
                  camera.zoom,
                  canvas.clientWidth,
                )
              : undefined
            if (found) {
              setSpotted(found)
              return
            }
            setSpotted(undefined)
            // A tap on a map picks it, on the sea or between maps drops the pick.
            choose(placeAt(event.offsetX, event.offsetY)?.name ?? null)
          },
          { signal },
        )
        canvas.addEventListener(
          "pointercancel",
          (event) => {
            press = null
            release(event)
          },
          { signal },
        )
        canvas.addEventListener(
          "wheel",
          (event) => {
            event.preventDefault()
            zoomAt(Math.exp(-event.deltaY * 0.0015), event.offsetX, event.offsetY)
          },
          { passive: false, signal },
        )
        canvas.addEventListener("dblclick", (event) => zoomAt(2, event.offsetX, event.offsetY), {
          signal,
        })
        // Arrow keys pan, + and - zoom, for a keyboard.
        canvas.addEventListener(
          "keydown",
          (event) => {
            const step = 80 / camera.zoom
            const moves: Record<string, [number, number]> = {
              ArrowLeft: [-step, 0],
              ArrowRight: [step, 0],
              ArrowUp: [0, -step],
              ArrowDown: [0, step],
            }
            const move = moves[event.key]
            if (event.key === "Enter") {
              // Picks the map at the center of the view.
              const found = placeAt(canvas.clientWidth / 2, canvas.clientHeight / 2)
              if (!found) return
              choose(found.name, true)
            } else if (move) {
              camera.x += move[0]
              camera.y += move[1]
              clamp()
              schedule()
            } else if (event.key === "+" || event.key === "=") zoomBy.current(1.6)
            else if (event.key === "-") zoomBy.current(1 / 1.6)
            else return
            event.preventDefault()
          },
          { signal },
        )
        const resizer = new ResizeObserver(schedule)
        resizer.observe(canvas)
        signal.addEventListener("abort", () => resizer.disconnect())
        await document.fonts.ready
        if (!signal.aborted) schedule()
      },
      () => setFailed(true),
    )

    return () => {
      listening.abort()
      cancelAnimationFrame(raf)
    }
  }, [base, start, clock])

  const close = (focusMap: boolean) => {
    setPicked(null)
    if (focusMap) canvasRef.current?.focus()
  }

  return (
    <div
      role="presentation"
      className="relative size-full"
      style={{ background: OPEN_SEA }}
      onKeyDown={(event) => {
        // Esc closes the card first, and only then the dialog around the map.
        if (event.key !== "Escape" || !picked) return
        event.preventDefault()
        event.stopPropagation()
        close(true)
      }}
    >
      <canvas
        ref={canvasRef}
        tabIndex={0}
        aria-roledescription="map"
        aria-label="Map of the whole world. Drag or use the arrow keys to look around; pinch, scroll or use the plus and minus keys to zoom. Tap a place, or press Enter on the one in the middle, to see what is there."
        className="block size-full cursor-grab touch-none focus-visible:outline-4 focus-visible:-outline-offset-4 focus-visible:outline-logo-yellow active:cursor-grabbing"
      />
      {failed && (
        <p className="absolute inset-0 grid place-items-center p-6 text-center text-lg text-paper">
          The map didn't load. Check your connection and try again.
        </p>
      )}
      {picked && (encounters || encountersFailed) && (
        <PlaceCard
          key={picked}
          ref={cardRef}
          place={encounters?.maps[picked]}
          info={{ map: picked, name: encounters?.maps[picked]?.n ?? picked }}
          encounters={encounters}
          base={base}
          failed={encountersFailed}
          rates={debug.rates}
          extra={extra}
          onClose={() => close(true)}
        />
      )}
      <DebugMenu debug={debug} />
      {spotted && (
        <output className="pixel-panel gba-text gba-text-light pointer-events-none absolute top-4 left-1/2 max-w-[calc(100%-2rem)] -translate-x-1/2 px-4 py-3 text-center">
          {spotted.label}
        </output>
      )}
      {living && (
        <LivingClock
          living={living}
          minutes={minutes}
          className={`absolute bottom-4 left-4 ${picked ? "max-sm:hidden sm:left-auto sm:right-20" : ""}`}
        />
      )}
      <div
        data-open={picked ? "" : undefined}
        className="absolute right-4 bottom-4 flex flex-col gap-2 max-sm:data-open:hidden"
      >
        {(
          [
            ["+", 1.6, "Zoom in"],
            ["−", 1 / 1.6, "Zoom out"],
          ] as const
        ).map(([label, factor, name]) => (
          <button
            key={label}
            type="button"
            onClick={() => zoomBy.current(factor)}
            className="pixel-corners pixel-panel size-12 text-2xl text-paper"
            aria-label={name}
          >
            {label}
          </button>
        ))}
      </div>
    </div>
  )
}

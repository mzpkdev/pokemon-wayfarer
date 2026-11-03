import { useEffect, useRef, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"

import { type Camera, fit, fly } from "./camera"
import { type Bounds, DATA, type Meta, loadWorld } from "./data"
import { stepped } from "./fade"
import {
  BADGES_SRC,
  badgeOverlays,
  landmarkBounds,
  type Landmarks,
  loadLandmarks,
  type StopClock,
} from "./landmarks"
import { OPEN_SEA, type Overlay, rendererFor } from "./render"
import { riderAt } from "./rider"
import {
  DEFAULT_START,
  focusIn,
  openingIn,
  placeAt,
  rideMs,
  routeIn,
  type Start,
  type StartId,
  STARTS,
  startOf,
} from "./starts"
import { pageMinutes } from "./clock"
import { LivingClock } from "./LivingClock"
import { type LivingWorld, livingOverlay, loadLiving } from "./living"
import { type Notables, loadNotables, notablesOverlay } from "./notables"
import { PlaceExtra } from "./PlaceExtra"
import type { PlaceInfo } from "./PlaceCard"
import { WorldMap } from "./WorldMap"

/** The explorer's place cards also name a town's Gym, league stop and ferries. */
const placeExtra = (place: PlaceInfo) => <PlaceExtra place={place} />

/** The tour's stops, in the order its cards come. */
export type StopName = "town" | "region" | "badges" | "world"
const STOPS: StopName[] = ["town", "region", "badges", "world"]
const BADGES_STOP = STOPS.indexOf("badges")

export type Caption = {
  place: string
  title: string
  body: string
  stats?: { value: string; label: string }[]
  /** What the card says instead for each start, in any of the fields above. */
  variants?: Partial<Record<StartId, Partial<Omit<Caption, "variants">>>>
}

/**
 * Something drawn over the map while the tour is at `stop`: a hook for what
 * the page wants to show on a stop that the tour itself doesn't. `animated`
 * keeps frames coming while it is up.
 */
export type StopOverlay = { stop: StopName; overlay: Overlay; animated?: boolean }

/** How long a flight between stops takes: longer the more the zoom changes. */
const flightMs = (from: Camera, to: Camera) =>
  Math.min(2400, 900 + 450 * Math.abs(Math.log2(to.zoom / from.zoom)))
/** The closest zoom the rider is drawn at; further out he is a speck. */
const RIDER_MIN_ZOOM = 4

/** A beat between the tour starting and the rider setting off. */
const RIDE_DELAY_MS = 250
/** The closest the visitor can zoom in, in CSS pixels per metatile. */
const MAX_ZOOM = 48
/** How long a gesture hint stays up. */
const HINT_MS = 1600

/** A warp between starts: the game's 8-step fade to black, the cut, and the fade back in. */
const FADE_OUT_MS = 300
const FADE_IN_MS = 400

/** The badge stop's pins and trail come in a moment after the camera, and go out quickly. */
const BADGES_IN_DELAY_MS = 450
const BADGES_IN_MS = 400
const BADGES_OUT_MS = 250

/**
 * The game's yellow OK button from the naming screen; pixel corners clip an
 * outline, so focus shows as a blue rim inside instead.
 */
const BUTTON =
  "pixel-corners pixel-label bg-[rgb(232_208_64)] px-7 pt-2 pb-3 text-[rgb(56_56_56)] shadow-[inset_0_0_0_4px_rgb(56_56_56),inset_0_-8px_0_rgb(184_160_32),inset_0_8px_0_rgb(248_240_152)] hover:bg-[rgb(248_224_96)] focus-visible:shadow-[inset_0_0_0_4px_rgb(48_80_200),inset_0_-8px_0_rgb(184_160_32),inset_0_8px_0_rgb(248_240_152)] focus-visible:outline-none active:translate-y-0.5 disabled:opacity-50"

/** A choice of start: the same yellow when picked, a dark plate otherwise. */
const OPTION =
  "pixel-corners flex w-full items-baseline justify-between gap-3 px-4 pt-2 pb-3 text-left font-[family-name:var(--font-gba)] text-[16px] leading-[16px] sm:text-[24px] sm:leading-[24px] has-focus-visible:shadow-[inset_0_0_0_4px_rgb(48_80_200)]"

/** Metatiles across and down in the close-up shots. */
const CLOSE_UP = { width: 44, height: 30 }

const boundsOf = (maps: Meta["maps"]): Bounds => ({
  left: Math.min(...maps.map(({ x }) => x)),
  top: Math.min(...maps.map(({ y }) => y)),
  right: Math.max(...maps.map(({ x, width }) => x + width)),
  bottom: Math.max(...maps.map(({ y, height }) => y + height)),
})

const unionOf = (boxes: Bounds[]): Bounds => ({
  left: Math.min(...boxes.map(({ left }) => left)),
  top: Math.min(...boxes.map(({ top }) => top)),
  right: Math.max(...boxes.map(({ right }) => right)),
  bottom: Math.max(...boxes.map(({ bottom }) => bottom)),
})

/** Where a region (by its label) lies, from its bounds or else the maps its pieces hold. */
const regionBounds = (meta: Meta, name: string): Bounds =>
  meta.regions.find((region) => region.name === name)?.bounds ??
  boundsOf(
    meta.maps.filter(({ region }) => region === (name === "HOENN" ? "Hoenn" : "Johto and Kanto")),
  )

/**
 * The shots for a map area `width` by `height` CSS pixels, for a start: the
 * opening on the road, then the stops: the town, its region (or regions), every
 * Gym and the league circuit, and the whole world.
 */
const shotsFor = (
  meta: Meta,
  landmarks: Landmarks,
  start: Start,
  width: number,
  height: number,
): Camera[] => {
  const close = Math.min(width / CLOSE_UP.width, height / CLOSE_UP.height)
  const region = unionOf(start.regions.map((name) => regionBounds(meta, name)))
  const regions = ["JOHTO", "KANTO", "HOENN"].map((name) => regionBounds(meta, name))
  const gyms = landmarkBounds(landmarks, 36) ?? unionOf(regions)
  return [
    { ...fit(openingIn(meta, start), width, height, 1) },
    { ...focusIn(meta, start), zoom: close },
    fit(region, width, height, 0.95),
    fit(gyms, width, height, 0.95),
    fit(boundsOf(meta.maps), width, height, 0.92),
  ]
}

/**
 * The world in the game's own tiles, held still at one stop at a time with a
 * card beside it. It opens with the player riding in on their bike to their
 * front door, in the town the first card picks (LITTLEROOT TOWN, NEW BARK TOWN
 * or PALLET TOWN; another start plays a warp: the screen fades to black and
 * back in). The card's button then flies the camera on: the region, every Gym
 * Badge and the league circuit, and the whole world. The last one opens the
 * map to look around. The page scrolls past it as usual. Fills its section.
 *
 * The ride starts the first time nearly all of the section is on screen, or,
 * given `startOn`, when the section receives that event (or already carries
 * `data-started`), for a page that brings the map in its own way.
 *
 * `captions` are one card per stop, in order: the town, the region, the
 * badges, the world. `stopOverlays` draw over the map at a stop.
 */
export const WorldTour = ({
  captions,
  base = DATA,
  startOn,
  stopOverlays,
}: {
  captions: Caption[]
  base?: string
  startOn?: string
  stopOverlays?: StopOverlay[]
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const captionRef = useRef<HTMLDivElement>(null)
  const dialogRef = useRef<HTMLDialogElement>(null)
  const cameraRef = useRef<Camera | null>(null)
  const goRef = useRef<(stop: number) => void>(() => {})
  const zoomRef = useRef<(factor: number) => void>(() => {})
  const [hint, setHint] = useState<"wheel" | "touch" | null>(null)
  const chooseRef = useRef<(id: StartId) => void>(() => {})
  // The world stop shows the living world: the ferries on their loops and
  // the time of day, on the page's shared clock.
  const [living, setLiving] = useState<LivingWorld>()
  const [minutes, setMinutes] = useState(pageMinutes)
  const [notables, setNotables] = useState<Notables>()
  useEffect(() => {
    void loadLiving(base).then(setLiving, () => {})
    void loadNotables(base).then(setNotables, () => {})
  }, [base])
  const stopOverlaysRef = useRef(stopOverlays)
  useEffect(() => {
    // The Gym Leaders and Elite Four go under the time of day's tint.
    stopOverlaysRef.current = [
      ...(notables
        ? [
            {
              stop: "world" as const,
              overlay: notablesOverlay(notables, pageMinutes),
              animated: true,
            },
          ]
        : []),
      ...(living
        ? [{ stop: "world" as const, overlay: livingOverlay(living, pageMinutes), animated: true }]
        : []),
      ...(stopOverlays ?? []),
    ]
  }, [stopOverlays, living, notables])
  const reducedMotion = useReducedMotion()
  const [stop, setStop] = useState(0)
  const atWorld = STOPS[stop] === "world"
  useEffect(() => {
    if (!atWorld) return
    const timer = window.setInterval(() => setMinutes(pageMinutes()), 500)
    return () => window.clearInterval(timer)
  }, [atWorld])
  const [chosen, setChosen] = useState<StartId>(DEFAULT_START)
  const [warping, setWarping] = useState(false)
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

    // Where the tour is: the stop shown (its shot is one past it, after the
    // opening), the flight under way if any, and when the ride began.
    let current = 0
    let flight: { from: Camera; to: number; start: number; duration: number } | null = null
    // Where the visitor has moved the map to, from the second stop on; the
    // next NEXT or BACK flies on from there.
    let manual: Camera | null = null
    let hintTimer = 0
    const showHint = (kind: "wheel" | "touch") => {
      setHint(kind)
      window.clearTimeout(hintTimer)
      hintTimer = window.setTimeout(() => setHint(null), HINT_MS)
    }
    let started: number | null = null
    // The warp to another start, if one is playing: when it began and, once
    // the screen is black, when the cut happened.
    let warp: { to: StartId; start: number; cut: number | null } | null = null
    /** Whether the visitor may pan and zoom: from the second stop on, outside a warp. */
    const free = () => current >= 1 && !warp
    // The badge stop's clock: when the camera set off for it and when it left.
    let badgesSince: number | null = null
    let badgesLeft: number | null = null

    const sheets = new Map<StartId, HTMLImageElement>()
    const images = STARTS.map((start) => {
      const image = new Image()
      image.src = start.rider
      sheets.set(start.id, image)
      return image
    })
    const badgeSheet = new Image()
    badgeSheet.src = BADGES_SRC
    images.push(badgeSheet)
    void Promise.all([
      loadWorld(base),
      loadLandmarks(base),
      ...images.map((image) => image.decode().catch(() => {})),
    ]).then(([world, landmarks]) => {
      if (disposed) return
      const renderer = rendererFor(world)
      let start = startOf(DEFAULT_START)
      let route = routeIn(world.meta, start)
      let ride = rideMs(route)
      const stopClock: StopClock = { alpha: 0, elapsed: 0, animate: !reducedMotion }
      const pins = badgeOverlays(landmarks, badgeSheet, stopClock)

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

      /** The cut: the screen is black, so everything switches to the new start. */
      const cut = (to: StartId, now: number) => {
        start = startOf(to)
        route = routeIn(world.meta, start)
        ride = rideMs(route)
        current = 0
        flight = null
        manual = null
        badgesSince = null
        badgesLeft = null
        // The ride begins once the screen has faded back in.
        started = now + (reducedMotion ? 0 : FADE_IN_MS)
      }

      const draw = (now: number) => {
        raf = 0

        // The warp's fade: 0 clear to 1 black, in the game's whole steps.
        let fade = 0
        if (warp) {
          if (warp.cut === null) {
            const t = (now - warp.start) / FADE_OUT_MS
            if (t >= 1) {
              cut(warp.to, now)
              warp.cut = now
              fade = 1
            } else fade = stepped(t)
          } else {
            const t = (now - warp.cut) / FADE_IN_MS
            if (t >= 1) {
              warp = null
              setWarping(false)
            } else fade = stepped(1 - t)
          }
        }

        const { width, height } = area()
        const shots = shotsFor(world.meta, landmarks, start, width, height)
        const rideStart = started === null ? null : started + RIDE_DELAY_MS
        const progress = reducedMotion
          ? 1
          : rideStart === null
            ? 0
            : Math.max(0, (now - rideStart) / ride)
        let camera: Camera
        if (flight) {
          const t = (now - flight.start) / flight.duration
          camera = fly(flight.from, shots[flight.to + 1]!, t)
          if (t >= 1) flight = null
        } else if (manual) camera = manual
        else if (current === 0) camera = fly(shots[0]!, shots[1]!, progress)
        else camera = shots[current + 1]!
        cameraRef.current = camera

        const sheet = sheets.get(start.id)!
        const rider = riderAt(route, progress, sheet)
        // The popup names where the rider is while the camera is with them.
        showPlace(
          current > 0 || rideStart === null || now < rideStart
            ? null
            : placeAt(world.meta, start, { x: rider.x + 1, y: rider.y + 2 }),
        )

        // The badge stop's pins and trail show while the tour is there, and
        // fade as it leaves.
        let badgesUp = false
        if (current === BADGES_STOP && badgesSince !== null) {
          stopClock.elapsed = now - badgesSince
          stopClock.alpha = reducedMotion
            ? 1
            : Math.min(1, Math.max(0, (stopClock.elapsed - BADGES_IN_DELAY_MS) / BADGES_IN_MS))
          badgesUp = true
        } else if (badgesLeft !== null && !reducedMotion) {
          stopClock.alpha = Math.max(0, 1 - (now - badgesLeft) / BADGES_OUT_MS)
          badgesUp = stopClock.alpha > 0
        } else stopClock.alpha = 0
        const overlays = badgesUp ? [...pins] : []
        let animated = badgesUp && !reducedMotion
        for (const entry of stopOverlaysRef.current ?? [])
          if (entry.stop === STOPS[current]) {
            overlays.push(entry.overlay)
            animated ||= Boolean(entry.animated)
          }

        const pending = renderer.draw(context, {
          camera,
          focus: { x: width / 2, y: height / 2 },
          labelWidth: width,
          // The popup names the close-ups; names on the map come in as it pulls back.
          labels: camera.zoom < shots[1]!.zoom * 0.7,
          sprites:
            rider.image && sheet.complete && sheet.naturalWidth && camera.zoom >= RIDER_MIN_ZOOM
              ? [rider]
              : [],
          overlays,
        })
        if (fade > 0) {
          context.setTransform(1, 0, 0, 1, 0, 0)
          context.fillStyle = `rgb(0 0 0 / ${fade})`
          context.fillRect(0, 0, canvas.width, canvas.height)
        }
        const riding = rideStart !== null && progress < 1
        if (pending || flight || riding || warp || animated) schedule()
      }
      const schedule = () => {
        if (!raf) raf = requestAnimationFrame(draw)
      }

      goRef.current = (next) => {
        if (next === current || warp) return
        const { width, height } = area()
        const now = performance.now()
        const shots = shotsFor(world.meta, landmarks, start, width, height)
        const from = cameraRef.current ?? shots[current + 1]!
        const to = shots[next + 1]!
        manual = null
        flight = reducedMotion ? null : { from, to: next, start: now, duration: flightMs(from, to) }
        if (next === BADGES_STOP) {
          badgesSince = now
          badgesLeft = null
        } else if (current === BADGES_STOP) badgesLeft = now
        current = next
        schedule()
      }

      chooseRef.current = (id) => {
        if (id === start.id || warp) return
        const now = performance.now()
        warp = { to: id, start: now, cut: null }
        // With reduced motion there is no fade: the new start is just there.
        if (reducedMotion) {
          cut(id, now)
          warp = null
          setWarping(false)
        }
        schedule()
      }

      // Pan and zoom, from the second stop on, without taking the page's
      // scroll: a mouse drags and Ctrl (or ⌘) + wheel zooms, while a plain
      // wheel scrolls the page; on touch two fingers move the map and one
      // still scrolls the page. A hint says so when a gesture goes to the page.
      const focus = () => {
        const { width, height } = area()
        return { x: width / 2, y: height / 2 }
      }
      const limits = () => {
        const { width, height } = area()
        const whole = shotsFor(world.meta, landmarks, start, width, height)[STOPS.length]!
        return { min: whole.zoom * 0.8, max: MAX_ZOOM }
      }
      const move = (change: (camera: Camera) => Camera) => {
        const from = manual ?? cameraRef.current
        if (!from) return
        flight = null
        const next = change({ ...from })
        const { min, max } = limits()
        next.zoom = Math.min(max, Math.max(min, next.zoom))
        next.x = Math.min(world.meta.width, Math.max(0, next.x))
        next.y = Math.min(world.meta.height, Math.max(0, next.y))
        manual = next
        schedule()
      }
      const zoomAt = (factor: number, x: number, y: number) =>
        move((camera) => {
          const f = focus()
          // Keep the world point under (x, y) in place.
          const wx = camera.x + (x - f.x) / camera.zoom
          const wy = camera.y + (y - f.y) / camera.zoom
          const { min, max } = limits()
          const zoom = Math.min(max, Math.max(min, camera.zoom * factor))
          return { x: wx - (x - f.x) / zoom, y: wy - (y - f.y) / zoom, zoom }
        })
      zoomRef.current = (factor) => {
        const f = focus()
        zoomAt(factor, f.x, f.y)
      }
      const local = (event: { clientX: number; clientY: number }) => {
        const box = canvas.getBoundingClientRect()
        return { x: event.clientX - box.left, y: event.clientY - box.top }
      }
      const listening = new AbortController()
      const { signal } = listening
      let drag: { x: number; y: number } | null = null
      canvas.addEventListener(
        "pointerdown",
        (event) => {
          if (event.pointerType === "touch" || event.button !== 0 || !free()) return
          canvas.setPointerCapture(event.pointerId)
          drag = local(event)
        },
        { signal },
      )
      canvas.addEventListener(
        "pointermove",
        (event) => {
          if (!drag) return
          const now = local(event)
          const dx = now.x - drag.x
          const dy = now.y - drag.y
          drag = now
          move((camera) => ({
            ...camera,
            x: camera.x - dx / camera.zoom,
            y: camera.y - dy / camera.zoom,
          }))
        },
        { signal },
      )
      const release = () => {
        drag = null
      }
      canvas.addEventListener("pointerup", release, { signal })
      canvas.addEventListener("pointercancel", release, { signal })
      canvas.addEventListener(
        "wheel",
        (event) => {
          if (!free()) return
          if (!event.ctrlKey && !event.metaKey) {
            showHint("wheel")
            return
          }
          event.preventDefault()
          const at = local(event)
          zoomAt(Math.exp(-event.deltaY * 0.0025), at.x, at.y)
        },
        { passive: false, signal },
      )
      canvas.addEventListener(
        "dblclick",
        (event) => {
          if (!free()) return
          const at = local(event)
          zoomAt(2, at.x, at.y)
        },
        { signal },
      )
      let pinch: { x: number; y: number; distance: number } | null = null
      const twoFingers = (touches: TouchList) => {
        const [a, b] = [touches[0]!, touches[1]!]
        const box = canvas.getBoundingClientRect()
        return {
          x: (a.clientX + b.clientX) / 2 - box.left,
          y: (a.clientY + b.clientY) / 2 - box.top,
          distance: Math.hypot(a.clientX - b.clientX, a.clientY - b.clientY),
        }
      }
      canvas.addEventListener(
        "touchstart",
        (event) => {
          if (!free()) return
          if (event.touches.length >= 2) {
            event.preventDefault()
            pinch = twoFingers(event.touches)
          }
        },
        { passive: false, signal },
      )
      canvas.addEventListener(
        "touchmove",
        (event) => {
          if (!free()) return
          // One finger scrolls the page; say how to move the map instead.
          if (event.touches.length < 2) {
            if (!pinch) showHint("touch")
            return
          }
          if (!pinch) return
          event.preventDefault()
          const now = twoFingers(event.touches)
          const before = pinch
          pinch = now
          move((camera) => ({
            ...camera,
            x: camera.x - (now.x - before.x) / camera.zoom,
            y: camera.y - (now.y - before.y) / camera.zoom,
          }))
          if (before.distance > 0) zoomAt(now.distance / before.distance, now.x, now.y)
        },
        { passive: false, signal },
      )
      canvas.addEventListener(
        "touchend",
        (event) => {
          if (event.touches.length < 2) pinch = null
        },
        { signal },
      )

      const go = () => {
        if (started !== null) return
        started = performance.now()
        schedule()
      }
      const watcher = new IntersectionObserver(
        ([entry]) => {
          if (entry?.isIntersecting) go()
        },
        { threshold: 0.7 },
      )
      if (!startOn) watcher.observe(section)
      else if (section.dataset.started) go()
      else section.addEventListener(startOn, go, { once: true })
      const resizer = new ResizeObserver(schedule)
      resizer.observe(canvas)
      cleanup = () => {
        listening.abort()
        watcher.disconnect()
        if (startOn) section.removeEventListener(startOn, go)
        resizer.disconnect()
      }
      void document.fonts.ready.then(schedule)
      schedule()
    })

    return () => {
      disposed = true
      cancelAnimationFrame(raf)
      window.clearTimeout(popupTimer)
      window.clearTimeout(hintTimer)
      cleanup()
    }
  }, [base, reducedMotion, startOn])

  const go = (next: number) => {
    setStop(next)
    goRef.current(next)
  }

  const choose = (id: StartId) => {
    if (id === chosen || warping) return
    setChosen(id)
    setWarping(!reducedMotion)
    chooseRef.current(id)
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
        className={`absolute inset-0 size-full touch-pan-x touch-pan-y ${stop >= 1 && !warping ? "cursor-grab active:cursor-grabbing" : ""}`}
        style={{ background: OPEN_SEA }}
      />
      {stop >= 1 && !warping && (
        <div className="absolute top-4 right-4 z-10 flex flex-col gap-2">
          {(
            [
              ["+", 1.6, "Zoom in"],
              ["−", 1 / 1.6, "Zoom out"],
            ] as const
          ).map(([label, factor, name]) => (
            <button
              key={label}
              type="button"
              onClick={() => zoomRef.current(factor)}
              className="pixel-corners pixel-panel size-12 text-2xl text-paper"
              aria-label={name}
            >
              {label}
            </button>
          ))}
        </div>
      )}
      <div
        aria-live="polite"
        className={`pointer-events-none absolute inset-x-0 top-1/3 z-10 flex justify-center px-4 transition-opacity duration-200 ${hint ? "opacity-100" : "opacity-0"}`}
      >
        {hint && (
          <p className="pixel-panel gba-text gba-text-light px-4 py-3 text-center">
            {hint === "touch"
              ? "USE TWO FINGERS TO MOVE THE MAP"
              : /Mac|iPhone|iPad/.test(navigator.platform)
                ? "HOLD CMD AND SCROLL TO ZOOM"
                : "HOLD CTRL AND SCROLL TO ZOOM"}
          </p>
        )}
      </div>
      {living && (
        <LivingClock
          living={living}
          minutes={minutes}
          className={`absolute top-4 left-4 transition-opacity duration-300 ${atWorld ? "opacity-100" : "pointer-events-none opacity-0"}`}
        />
      )}
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
              const text = { ...caption, ...caption.variants?.[chosen] }
              return (
                <div
                  key={caption.place}
                  aria-hidden={!shown}
                  inert={!shown}
                  className="col-start-1 row-start-1 transition-opacity duration-300"
                  style={{ opacity: shown ? 1 : 0, pointerEvents: shown ? undefined : "none" }}
                >
                  <p className="pixel-label flex justify-between gap-4 text-logo-yellow">
                    <span>{text.place}</span>
                    <span
                      className="text-paper/60"
                      aria-label={`Stop ${i + 1} of ${captions.length}`}
                    >
                      {i + 1}/{captions.length}
                    </span>
                  </p>
                  <p className="pixel-heading mt-3">{text.title}</p>
                  <p className="mt-4 text-lg leading-relaxed text-paper/80">{text.body}</p>
                  {i === 0 && (
                    <fieldset className="mt-5 grid gap-2">
                      <legend className="sr-only">Where to begin</legend>
                      {STARTS.map((option) => {
                        const picked = option.id === chosen
                        return (
                          <label
                            key={option.id}
                            className={`${OPTION} ${
                              picked
                                ? "bg-[rgb(232_208_64)] text-[rgb(56_56_56)] shadow-[inset_0_0_0_4px_rgb(56_56_56),inset_0_-4px_0_rgb(184_160_32),inset_0_4px_0_rgb(248_240_152)]"
                                : "cursor-pointer bg-black/30 text-paper/80 shadow-[inset_0_0_0_4px_rgb(0_0_0/0.4)] hover:bg-black/50 hover:text-paper"
                            }`}
                          >
                            <input
                              type="radio"
                              name="start"
                              className="sr-only"
                              checked={picked}
                              disabled={warping}
                              onChange={() => choose(option.id)}
                            />
                            <span>
                              {picked ? "▶ " : ""}
                              {option.label}
                            </span>
                            <span className={picked ? "text-[rgb(56_56_56)]/70" : "text-paper/50"}>
                              {option.region}
                            </span>
                          </label>
                        )
                      })}
                    </fieldset>
                  )}
                  {text.stats && (
                    <dl className="mt-6 grid grid-cols-3 gap-4">
                      {text.stats.map(({ value, label }) => (
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
                disabled={warping}
                onClick={() => go(stop - 1)}
                className="pixel-label px-2 pt-2 pb-3 text-paper/70 hover:text-paper focus-visible:text-paper focus-visible:underline focus-visible:outline-none"
              >
                BACK
              </button>
            ) : (
              <span />
            )}
            {stop < captions.length - 1 ? (
              <button
                type="button"
                disabled={warping}
                onClick={() => go(stop + 1)}
                className={BUTTON}
              >
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
            <WorldMap
              base={base}
              start={exploring.zoom > 0 ? exploring : undefined}
              extra={placeExtra}
            />
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

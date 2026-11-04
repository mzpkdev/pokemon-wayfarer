// The studio: the map with every knob the video can turn, to compose shots
// by hand. The camera is free (drag and wheel), the clock is a slider, each
// layer a switch, and the set pieces are buttons: the ride in, a warp, the
// badges earned one by one, the wild waves, the days passing. COPY gives a
// camera for `timeline.ts`.
import { type PointerEvent, useEffect, useRef, useState, type WheelEvent } from "react"

import { type Camera, fit, fly } from "~/world-map/camera"
import { callingFor } from "~/world-map/landmarks"
import { clockText, shipsAt } from "~/world-map/living"
import { notableAt } from "~/world-map/notables"
import { type StartId, STARTS, startOf } from "~/world-map/starts"
import { stepped } from "~/world-map/fade"

import { drawShot, now, resolve, riderPlace, rideShots, type Stage, wholeWorld } from "./director"
import { ALL_LAYERS, type Layers, type Shot } from "./shot"
import { Popup, useStagePx } from "./ui"
import type { Target } from "./timeline"

/** How long a camera flight to a preset takes, and each half of a warp's fade. */
const FLIGHT_MS = 1400
const FADE_MS = 600
/** Game minutes a real second: hold, the game's own pace, the website's (a day in two minutes), a time-lapse. */
const SPEEDS: [label: string, rate: number][] = [
  ["Hold", 0],
  ["Game pace", 0.2],
  ["Page pace", 12],
  ["Time-lapse", 120],
]
const PRESETS: [label: string, target: Target][] = [
  ["World", { kind: "world" }],
  ["Johto", { kind: "regions", names: ["JOHTO"], room: 0.9 }],
  ["Kanto", { kind: "regions", names: ["KANTO"], room: 0.9 }],
  ["Hoenn", { kind: "regions", names: ["HOENN"], room: 0.9 }],
  ["Sinnoh", { kind: "regions", names: ["SINNOH"], room: 0.9 }],
  ["Sevii", { kind: "regions", names: ["SEVII ISLANDS"], room: 0.9 }],
  ["Alola", { kind: "regions", names: ["ALOLA"], room: 0.9 }],
  ["Circuit", { kind: "circuit", room: 0.9 }],
]

type Panel = {
  minute: number
  speed: number
  layers: Layers
  labels: boolean
  markers: boolean
  standing: boolean
  /** The walker the camera keeps with, by slug. */
  follow: string
  pins: boolean
  bright: boolean
  circuit: number
  dailyOn: boolean
  day: number
  wild: boolean
  hold: boolean
  start: StartId
  glide: boolean
  fade: number
}

const INITIAL: Panel = {
  minute: 10 * 60,
  speed: 0,
  layers: ALL_LAYERS,
  labels: true,
  markers: false,
  standing: false,
  follow: "",
  pins: false,
  bright: true,
  circuit: 0,
  dailyOn: false,
  day: 1,
  wild: false,
  hold: false,
  start: "kanto",
  glide: true,
  fade: 0,
}

/** What moves on its own between frames, outside React. */
type Live = {
  camera: Camera
  minute: number
  last: number
  pinsSince: number | null
  earned: Set<number>
  earnedAt: Map<number, number>
  wildStart: number
  wildStill: number | null
  flight: { from: Camera; to: Camera; began: number } | null
  ride: { start: StartId; began: number; warp: boolean; cut: boolean } | null
  drag: { x: number; y: number; camera: Camera } | null
}

const CLASS = {
  section: "space-y-1.5 border-t border-neutral-800 pt-2",
  heading: "text-[10px] font-semibold tracking-wider text-neutral-400 uppercase",
  button:
    "rounded bg-neutral-800 px-2 py-1 text-neutral-100 hover:bg-neutral-700 active:bg-neutral-600 disabled:opacity-40",
  row: "flex flex-wrap items-center gap-1.5",
  label: "flex items-center gap-1.5",
}

export const Studio = ({ stage, onPlay }: { stage: Stage; onPlay: () => void }) => {
  const stageRef = useRef<HTMLDivElement>(null)
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const px = useStagePx(stageRef)
  const [panel, setPanel] = useState<Panel>(INITIAL)
  const panelRef = useRef(panel)
  useEffect(() => {
    panelRef.current = panel
  }, [panel])
  const [readout, setReadout] = useState<{ camera: Camera; minute: number; popup: string | null }>()
  const [copied, setCopied] = useState("")
  const live = useRef<Live>({
    camera: { x: stage.world.meta.width / 2, y: stage.world.meta.height / 2, zoom: 0.5 },
    minute: INITIAL.minute,
    last: 0,
    pinsSince: null,
    earned: new Set(),
    earnedAt: new Map(),
    wildStart: 0,
    wildStill: null,
    flight: null,
    ride: null,
    drag: null,
  })
  const set = (change: Partial<Panel>) => setPanel((current) => ({ ...current, ...change }))

  const size = () => {
    const canvas = canvasRef.current
    return { width: canvas?.clientWidth ?? 1920, height: canvas?.clientHeight ?? 1080 }
  }
  const camera = () => live.current.camera
  const flyTo = (to: Camera) => {
    const l = live.current
    l.ride = null
    if (panelRef.current.glide) l.flight = { from: camera(), to, began: now() }
    else l.camera = to
    set({ follow: "" })
  }
  const preset = (target: Target) => {
    const { width, height } = size()
    flyTo(resolve(target, stage, live.current.minute, width, height))
  }
  const ride = (warp: boolean) => {
    const l = live.current
    l.flight = null
    const start = startOf(panelRef.current.start)
    const { width, height } = size()
    if (!warp) l.camera = rideShots(stage, start, width, height).opening
    l.ride = { start: start.id, began: now(), warp, cut: !warp }
    set({ follow: "" })
  }
  const earn = (count: number) => {
    const l = live.current
    const at = now()
    for (let badge = 0; badge < 24 && l.earned.size < count; badge++)
      if (!l.earned.has(badge)) {
        l.earned.add(badge)
        l.earnedAt.set(badge, at)
      }
    if (count === 0) {
      l.earned.clear()
      l.earnedAt.clear()
    }
    set({ pins: true, bright: false })
  }

  // The first picture: the whole world.
  useEffect(() => {
    const { width, height } = size()
    live.current.camera = fit(wholeWorld(stage), width, height)
  }, [stage])

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    if (!canvas || !context) return
    let raf = 0
    const draw = () => {
      raf = requestAnimationFrame(draw)
      const p = panelRef.current
      const l = live.current
      const t = now()
      const dt = Math.min(0.1, (t - l.last) / 1000)
      l.last = t
      if (p.speed > 0) l.minute = (l.minute + p.speed * dt) % 1440
      const { width, height } = size()

      let fade = p.fade
      let rider: Shot["rider"]
      let cam = l.camera
      if (l.ride) {
        const start = startOf(l.ride.start)
        const shots = rideShots(stage, start, width, height)
        let since = t - l.ride.began
        if (l.ride.warp) {
          // The game's warp: 8 steps to black, the cut, 8 steps back.
          if (since < FADE_MS) fade = Math.max(fade, stepped(since / FADE_MS))
          else {
            if (!l.ride.cut) {
              l.ride.cut = true
              l.camera = shots.opening
            }
            if (since < 2 * FADE_MS) fade = Math.max(fade, stepped(1 - (since - FADE_MS) / FADE_MS))
          }
          since -= 2 * FADE_MS
        }
        const progress = Math.min(1, Math.max(0, since / shots.ms))
        rider = { start: start.id, progress }
        if (l.ride.cut) cam = l.camera = fly(shots.opening, shots.home, progress)
      } else if (l.flight) {
        const k = (t - l.flight.began) / FLIGHT_MS
        cam = l.camera = fly(l.flight.from, l.flight.to, k)
        if (k >= 1) l.flight = null
      } else if (p.follow.startsWith("ship:")) {
        const ship = shipsAt(stage.living, l.minute).find((s) => s.id === p.follow.slice(5))
        if (ship) cam = l.camera = { x: ship.x, y: ship.y, zoom: l.camera.zoom }
      } else if (p.follow) {
        const who = notableAt(stage.notables, p.follow, l.minute)
        if (who) cam = l.camera = { x: who.x, y: who.y, zoom: l.camera.zoom }
      }

      if (p.pins && l.pinsSince === null) l.pinsSince = t
      if (!p.pins) l.pinsSince = null
      const shot: Shot = {
        camera: cam,
        minute: l.minute,
        labels: p.labels,
        layers: p.layers,
        walkers: { marker: p.markers, standing: p.standing, minHeight: p.markers ? 64 : undefined },
        pins: p.pins
          ? {
              alpha: 1,
              elapsed: t - (l.pinsSince ?? t),
              circuit: p.circuit,
              earned: p.bright ? undefined : l.earned,
              earnedAt: p.bright ? undefined : l.earnedAt,
              calling: p.bright ? {} : callingFor(l.earned),
            }
          : undefined,
        daily: p.dailyOn ? { day: p.day } : undefined,
        wild: p.wild ? { start: l.wildStart, still: l.wildStill } : undefined,
        rider,
        fade,
      }
      drawShot(stage, context, shot)
      setReadout({ camera: cam, minute: l.minute, popup: riderPlace(stage, rider) })
    }
    raf = requestAnimationFrame(draw)
    return () => cancelAnimationFrame(raf)
  }, [stage])

  // The free camera: drag to pan, wheel to zoom about the pointer.
  const onPointerDown = (event: PointerEvent<HTMLCanvasElement>) => {
    const l = live.current
    l.ride = null
    l.flight = null
    if (panel.follow) set({ follow: "" })
    l.drag = { x: event.clientX, y: event.clientY, camera: { ...l.camera } }
    event.currentTarget.setPointerCapture(event.pointerId)
  }
  const onPointerMove = (event: PointerEvent<HTMLCanvasElement>) => {
    const l = live.current
    if (!l.drag) return
    const { zoom } = l.drag.camera
    l.camera = {
      x: l.drag.camera.x - (event.clientX - l.drag.x) / zoom,
      y: l.drag.camera.y - (event.clientY - l.drag.y) / zoom,
      zoom,
    }
  }
  const onPointerUp = () => {
    live.current.drag = null
  }
  const onWheel = (event: WheelEvent<HTMLCanvasElement>) => {
    const l = live.current
    const box = event.currentTarget.getBoundingClientRect()
    const cx = event.clientX - box.left - box.width / 2
    const cy = event.clientY - box.top - box.height / 2
    const from = l.camera.zoom
    const zoom = Math.min(64, Math.max(0.2, from * Math.exp(-event.deltaY * 0.0015)))
    l.flight = null
    l.camera = {
      x: l.camera.x + cx * (1 / from - 1 / zoom),
      y: l.camera.y + cy * (1 / from - 1 / zoom),
      zoom,
    }
  }

  const copy = () => {
    const c = readout?.camera ?? camera()
    const text = JSON.stringify({
      kind: "point",
      x: Math.round(c.x * 10) / 10,
      y: Math.round(c.y * 10) / 10,
      zoom: Math.round(c.zoom * 100) / 100,
    })
    setCopied(text)
    void navigator.clipboard?.writeText(text).catch(() => {})
  }

  const leaders = stage.landmarks.badges.flatMap((gym) =>
    gym.leader?.simulated ? [{ slug: gym.leader.slug, name: gym.leader.name, town: gym.town }] : [],
  )
  const layer = (key: keyof Layers, label: string) => (
    <label key={key} className={CLASS.label}>
      <input
        type="checkbox"
        checked={panel.layers[key]}
        onChange={(event) => set({ layers: { ...panel.layers, [key]: event.target.checked } })}
      />
      {label}
    </label>
  )

  return (
    <>
      <div
        ref={stageRef}
        className="stage relative overflow-hidden bg-black"
        style={{ "--px": `${px}px` } as React.CSSProperties}
      >
        <canvas
          ref={canvasRef}
          className="block h-full w-full cursor-grab touch-none active:cursor-grabbing"
          onPointerDown={onPointerDown}
          onPointerMove={onPointerMove}
          onPointerUp={onPointerUp}
          onPointerCancel={onPointerUp}
          onWheel={onWheel}
        />
        <Popup label={readout?.popup ?? null} />
      </div>

      <aside className="h-dvh w-80 shrink-0 space-y-2 overflow-y-auto bg-neutral-900 p-3 font-sans text-xs text-neutral-200">
        <div className="flex items-center justify-between">
          <h1 className="text-sm font-semibold">Studio</h1>
          <button type="button" className={CLASS.button} onClick={onPlay}>
            ▶ Play the timeline
          </button>
        </div>

        <section className={CLASS.section}>
          <h2 className={CLASS.heading}>Camera</h2>
          <div className={CLASS.row}>
            {PRESETS.map(([label, target]) => (
              <button
                key={label}
                type="button"
                className={CLASS.button}
                onClick={() => preset(target)}
              >
                {label}
              </button>
            ))}
          </div>
          <div className={CLASS.row}>
            <label className={CLASS.label}>
              <input
                type="checkbox"
                checked={panel.glide}
                onChange={(e) => set({ glide: e.target.checked })}
              />
              Fly between
            </label>
            <label className={CLASS.label}>
              <input
                type="checkbox"
                checked={panel.labels}
                onChange={(e) => set({ labels: e.target.checked })}
              />
              Names
            </label>
          </div>
          <p className="font-mono text-[11px] text-neutral-400">
            {readout
              ? `x ${readout.camera.x.toFixed(1)}  y ${readout.camera.y.toFixed(1)}  zoom ${readout.camera.zoom.toFixed(2)}`
              : "…"}
          </p>
          <div className={CLASS.row}>
            <button type="button" className={CLASS.button} onClick={copy}>
              Copy camera
            </button>
            {copied && <code className="break-all text-[10px] text-neutral-400">{copied}</code>}
          </div>
          <p className="text-neutral-500">Drag to pan, wheel to zoom.</p>
        </section>

        <section className={CLASS.section}>
          <h2 className={CLASS.heading}>Clock · {readout ? clockText(readout.minute) : ""}</h2>
          <input
            type="range"
            min={0}
            max={1439}
            value={Math.floor(readout?.minute ?? panel.minute)}
            onChange={(e) => {
              live.current.minute = Number(e.target.value)
              set({ minute: live.current.minute })
            }}
            className="w-full"
          />
          <div className={CLASS.row}>
            {SPEEDS.map(([label, rate]) => (
              <button
                key={label}
                type="button"
                className={`${CLASS.button} ${panel.speed === rate ? "ring-1 ring-amber-400" : ""}`}
                onClick={() => set({ speed: rate })}
              >
                {label}
              </button>
            ))}
          </div>
        </section>

        <section className={CLASS.section}>
          <h2 className={CLASS.heading}>Layers</h2>
          <div className="grid grid-cols-2 gap-1">
            {layer("sea", "Sea")}
            {layer("weather", "Weather")}
            {layer("clouds", "Clouds")}
            {layer("ships", "Ships & tint")}
            {layer("lights", "Night lights")}
            {layer("sightings", "Wild close in")}
            {layer("walkers", "Gym Leaders")}
          </div>
          <div className={CLASS.row}>
            <label className={CLASS.label}>
              <input
                type="checkbox"
                checked={panel.markers}
                onChange={(e) => set({ markers: e.target.checked })}
              />
              Markers
            </label>
            <label className={CLASS.label}>
              <input
                type="checkbox"
                checked={panel.standing}
                onChange={(e) => set({ standing: e.target.checked })}
              />
              Standing
            </label>
            <select
              value={panel.follow}
              onChange={(e) => {
                live.current.flight = null
                live.current.ride = null
                set({ follow: e.target.value, layers: { ...panel.layers, walkers: true } })
              }}
              className="rounded bg-neutral-800 px-1 py-0.5"
            >
              <option value="">Follow nobody</option>
              {stage.living.ferry.ships.map((ship) => (
                <option key={ship.id} value={`ship:${ship.id}`}>
                  {ship.name}
                </option>
              ))}
              {leaders.map((who) => (
                <option key={who.slug} value={who.slug}>
                  {who.name} ({who.town})
                </option>
              ))}
            </select>
          </div>
        </section>

        <section className={CLASS.section}>
          <h2 className={CLASS.heading}>Badges & leagues</h2>
          <div className={CLASS.row}>
            <label className={CLASS.label}>
              <input
                type="checkbox"
                checked={panel.pins}
                onChange={(e) => set({ pins: e.target.checked })}
              />
              Pins
            </label>
            <label className={CLASS.label}>
              <input
                type="checkbox"
                checked={panel.bright}
                onChange={(e) => set({ bright: e.target.checked })}
              />
              All bright
            </label>
          </div>
          <label className="block">
            Circuit {panel.circuit.toFixed(2)}
            <input
              type="range"
              min={0}
              max={1}
              step={0.01}
              value={panel.circuit}
              onChange={(e) => set({ circuit: Number(e.target.value), pins: true })}
              className="w-full"
            />
          </label>
          <div className={CLASS.row}>
            <button
              type="button"
              className={CLASS.button}
              onClick={() => earn(live.current.earned.size + 1)}
            >
              Earn a badge
            </button>
            <button type="button" className={CLASS.button} onClick={() => earn(8)}>
              Earn 8
            </button>
            <button type="button" className={CLASS.button} onClick={() => earn(24)}>
              Earn all
            </button>
            <button type="button" className={CLASS.button} onClick={() => earn(0)}>
              Clear
            </button>
          </div>
          <p className="text-neutral-500">
            Earned badges glow and sparkle; 8 and a league calls, lighting its trail.
          </p>
        </section>

        <section className={CLASS.section}>
          <h2 className={CLASS.heading}>Daily roads</h2>
          <div className={CLASS.row}>
            <label className={CLASS.label}>
              <input
                type="checkbox"
                checked={panel.dailyOn}
                onChange={(e) => set({ dailyOn: e.target.checked })}
              />
              On
            </label>
            <button
              type="button"
              className={CLASS.button}
              onClick={() => set({ day: Math.max(1, panel.day - 1), dailyOn: true })}
            >
              ◀
            </button>
            <span>Day {panel.day}</span>
            <button
              type="button"
              className={CLASS.button}
              onClick={() => set({ day: panel.day + 1, dailyOn: true })}
            >
              ▶
            </button>
          </div>
        </section>

        <section className={CLASS.section}>
          <h2 className={CLASS.heading}>Wild waves</h2>
          <div className={CLASS.row}>
            <label className={CLASS.label}>
              <input
                type="checkbox"
                checked={panel.wild}
                onChange={(e) => {
                  live.current.wildStart = now()
                  live.current.wildStill = null
                  set({ wild: e.target.checked, hold: false })
                }}
              />
              On
            </label>
            <button
              type="button"
              className={CLASS.button}
              onClick={() => {
                live.current.wildStart = now()
                live.current.wildStill = null
                set({ wild: true, hold: false })
              }}
            >
              Restart
            </button>
            <label className={CLASS.label}>
              <input
                type="checkbox"
                checked={panel.hold}
                onChange={(e) => {
                  live.current.wildStill = e.target.checked ? now() : null
                  set({ hold: e.target.checked })
                }}
              />
              Hold the wave
            </label>
          </div>
          <p className="text-neutral-500">
            Each generation pops out of its home region in turn, best from far out.
          </p>
        </section>

        <section className={CLASS.section}>
          <h2 className={CLASS.heading}>The ride in</h2>
          <div className={CLASS.row}>
            <select
              value={panel.start}
              onChange={(e) => set({ start: e.target.value as StartId })}
              className="rounded bg-neutral-800 px-1 py-0.5"
            >
              {STARTS.map((start) => (
                <option key={start.id} value={start.id}>
                  {start.player} · {start.label}
                </option>
              ))}
            </select>
            <button type="button" className={CLASS.button} onClick={() => ride(false)}>
              Ride in
            </button>
            <button type="button" className={CLASS.button} onClick={() => ride(true)}>
              Warp & ride
            </button>
            <button
              type="button"
              className={CLASS.button}
              onClick={() => {
                live.current.ride = null
              }}
            >
              Clear
            </button>
          </div>
          <p className="text-neutral-500">
            The player bikes from the road to their door with the map name popup, as on the site.
          </p>
        </section>

        <section className={CLASS.section}>
          <h2 className={CLASS.heading}>Fade {panel.fade.toFixed(2)}</h2>
          <input
            type="range"
            min={0}
            max={1}
            step={0.01}
            value={panel.fade}
            onChange={(e) => set({ fade: Number(e.target.value) })}
            className="w-full"
          />
        </section>
      </aside>
    </>
  )
}

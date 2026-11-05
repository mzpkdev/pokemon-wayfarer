// Scouting a location for the storyboard: one still of the world at a place,
// with every tile's coordinates over it, to pick an actor's marks from.
//
//   /?scout&map=Route30_hns&x=8&y=20&zoom=48&minute=600&day=10
//
// `x` and `y` are in the map's own tiles when `map` is given (and so are the
// coordinates drawn), else in the world grid. `day` shows that day's daily
// trainers, `pins` the badge pins and the league circuit, `night` the night's
// tint and lights; `plain` leaves the grid off, to see the picture as a scene would.
import { useEffect, useRef } from "react"

import type { Overlay } from "~/world-map/render"

import { drawShot, type Stage } from "./director"
import { ALL_LAYERS, type Shot } from "./shot"
import { DEFAULT_MINUTES } from "./timeline"

declare global {
  // eslint-disable-next-line no-var
  var wayfarerScout: { pending: boolean } | undefined
}

const gridOverlay =
  (origin: { x: number; y: number }): Overlay =>
  (context, { scale, left, top, ratio, toScreen }) => {
    const { width, height } = context.canvas
    const every = scale >= 40 * ratio ? 1 : scale >= 20 * ratio ? 2 : scale >= 8 * ratio ? 5 : 20
    context.lineWidth = 1
    context.font = `${Math.round(Math.min(11 * ratio, scale * every * 0.3))}px monospace`
    context.textBaseline = "top"
    for (let gx = Math.floor(left); gx * 1 < left + width / scale; gx++) {
      const major = (gx - origin.x) % every === 0
      context.strokeStyle = major ? "rgb(0 0 0 / 0.45)" : "rgb(0 0 0 / 0.12)"
      const x = Math.round(toScreen(gx, 0).x) + 0.5
      context.beginPath()
      context.moveTo(x, 0)
      context.lineTo(x, height)
      context.stroke()
    }
    for (let gy = Math.floor(top); gy < top + height / scale; gy++) {
      const major = (gy - origin.y) % every === 0
      context.strokeStyle = major ? "rgb(0 0 0 / 0.45)" : "rgb(0 0 0 / 0.12)"
      const y = Math.round(toScreen(0, gy).y) + 0.5
      context.beginPath()
      context.moveTo(0, y)
      context.lineTo(width, y)
      context.stroke()
    }
    for (let gy = Math.floor(top); gy < top + height / scale; gy++)
      for (let gx = Math.floor(left); gx < left + width / scale; gx++) {
        if ((gx - origin.x) % every !== 0 || (gy - origin.y) % every !== 0) continue
        const at = toScreen(gx, gy)
        const label = `${gx - origin.x},${gy - origin.y}`
        context.fillStyle = "rgb(0 0 0 / 0.75)"
        context.fillText(label, at.x + 3, at.y + 3)
        context.fillStyle = "rgb(255 255 120)"
        context.fillText(label, at.x + 2, at.y + 2)
      }
  }

export const Scout = ({ stage, params }: { stage: Stage; params: URLSearchParams }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    if (!canvas || !context) return
    const number = (name: string, otherwise: number) =>
      params.has(name) ? Number(params.get(name)) : otherwise
    const map = stage.world.meta.maps.find((m) => m.name === params.get("map"))
    const origin = map ?? { x: 0, y: 0 }
    const day = params.has("day") ? Number(params.get("day")) : null
    const shot: Shot = {
      camera: {
        x: origin.x + number("x", map ? map.width / 2 : stage.world.meta.width / 2),
        y: origin.y + number("y", map ? map.height / 2 : stage.world.meta.height / 2),
        zoom: number("zoom", 32),
      },
      minute: number("minute", DEFAULT_MINUTES),
      labels: false,
      layers: { ...ALL_LAYERS, tint: params.has("night"), lights: params.has("night") },
      walkers: { marker: true, minHeight: 32 },
      daily: day === null ? undefined : { day },
      pins: params.has("pins") ? { alpha: 1, elapsed: 60_000, circuit: 1 } : undefined,
      fade: 0,
    }
    const grid = params.has("plain") ? null : gridOverlay(origin)
    let raf = 0
    const draw = () => {
      raf = requestAnimationFrame(draw)
      const pending = drawShot(stage, context, shot, grid ? [grid] : [])
      globalThis.wayfarerScout = { pending }
    }
    raf = requestAnimationFrame(draw)
    return () => cancelAnimationFrame(raf)
  }, [stage, params])
  return (
    <div className="stage relative overflow-hidden">
      <canvas ref={canvasRef} className="block h-full w-full" aria-hidden="true" />
    </div>
  )
}

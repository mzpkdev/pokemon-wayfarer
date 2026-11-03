import { useEffect, useRef, useState } from "react"

import { type Camera, fit } from "./camera"
import { DATA, loadWorld } from "./data"
import { OPEN_SEA, rendererFor } from "./render"

/** The closest zoom, in CSS pixels per metatile. */
const MAX_ZOOM = 48

/**
 * The whole world to pan and zoom: drag to pan, pinch, wheel or double-click
 * to zoom, or the buttons. Fills its parent.
 */
export const WorldMap = ({ base = DATA, start }: { base?: string; start?: Camera }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const zoomBy = useRef<(factor: number) => void>(() => {})
  const [failed, setFailed] = useState(false)

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    if (!canvas || !context) return
    const listening = new AbortController()
    const { signal } = listening
    let raf = 0

    void loadWorld(base).then(
      async (world) => {
        if (signal.aborted) return
        const { width, height } = world.meta
        const renderer = rendererFor(world)
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
          const pending = renderer.draw(context, {
            camera,
            focus: { x: canvas.clientWidth / 2, y: canvas.clientHeight / 2 },
            labelWidth: canvas.clientWidth,
            labels: true,
          })
          if (pending) schedule()
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
        zoomBy.current = (factor) => zoomAt(factor, canvas.clientWidth / 2, canvas.clientHeight / 2)

        // Drag to pan, pinch to zoom, wheel to zoom at the cursor.
        const pointers = new Map<number, { x: number; y: number }>()
        let pinch = 0
        canvas.addEventListener(
          "pointerdown",
          (event) => {
            canvas.setPointerCapture(event.pointerId)
            pointers.set(event.pointerId, { x: event.offsetX, y: event.offsetY })
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
            if (!last) return
            const now = { x: event.offsetX, y: event.offsetY }
            pointers.set(event.pointerId, now)
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
        canvas.addEventListener("pointerup", release, { signal })
        canvas.addEventListener("pointercancel", release, { signal })
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
            if (move) {
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
  }, [base, start])

  return (
    <div className="relative size-full" style={{ background: OPEN_SEA }}>
      <canvas
        ref={canvasRef}
        tabIndex={0}
        aria-roledescription="map"
        aria-label="Map of the whole world. Drag or use the arrow keys to look around; pinch, scroll or use the plus and minus keys to zoom."
        className="block size-full cursor-grab touch-none focus-visible:outline-4 focus-visible:-outline-offset-4 focus-visible:outline-logo-yellow active:cursor-grabbing"
      />
      {failed && (
        <p className="absolute inset-0 grid place-items-center p-6 text-center text-lg text-paper">
          The map didn't load. Check your connection and try again.
        </p>
      )}
      <div className="absolute right-4 bottom-4 flex flex-col gap-2">
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

import { useEffect, useRef } from "react"

/**
 * Sketch: the overworld drawn from the game's own metatiles, pannable and
 * zoomable. The data is a sheet of every metatile used (`atlas.png`), a grid
 * of indices into it (`grid.bin`), and the region's size (`meta.json`).
 */
type Meta = {
  width: number
  height: number
  columns: number
  count: number
  towns: { name: string; x: number; y: number }[]
  regions: { name: string; x: number; y: number }[]
}

const EMPTY = 0xffff
const CHUNK = 32
/** Device pixels per metatile where the tiles start fading in over the map view, and finish. */
const FADE_FROM = 5
const FADE_TO = 10
/** Zoom limits, in CSS pixels per metatile. */
const MAX_ZOOM = 48

const loadImage = (src: string) =>
  new Promise<HTMLImageElement>((resolve, reject) => {
    const image = new Image()
    image.addEventListener("load", () => resolve(image))
    image.addEventListener("error", reject)
    image.src = src
  })

/** "NewBarkTown_hns" to "NEW BARK TOWN". */
const townName = (name: string) =>
  name
    .replace(/_.*$/, "")
    .replace(/([a-z])([A-Z])/g, "$1 $2")
    .toUpperCase()

export const WorldExplorer = ({ base }: { base: string }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const zoomBy = useRef<(factor: number) => void>(() => {})

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    if (!canvas || !context) return
    let disposed = false
    let raf = 0

    void (async () => {
      const [meta, atlas, grid] = await Promise.all([
        fetch(`${base}/meta.json`).then((response) => response.json() as Promise<Meta>),
        loadImage(`${base}/atlas.png`),
        fetch(`${base}/grid.bin`)
          .then((response) => response.arrayBuffer())
          .then((buffer) => new Uint16Array(buffer)),
      ])
      if (disposed) return
      const { width, height, columns } = meta

      // The far-out view, one pixel per metatile, drawn like a map by the data script.
      const overview = await loadImage(`${base}/overview.png`)
      if (disposed) return

      // Chunks of 32x32 metatiles, drawn on demand at 4 or 16 pixels a
      // metatile and kept until too many pile up.
      const chunks = new Map<string, HTMLCanvasElement>()
      const chunk = (cx: number, cy: number, size: number) => {
        const key = `${size}:${cx}:${cy}`
        const cached = chunks.get(key)
        if (cached) {
          chunks.delete(key)
          chunks.set(key, cached)
          return cached
        }
        const c = document.createElement("canvas")
        c.width = CHUNK * size
        c.height = CHUNK * size
        const cc = c.getContext("2d")!
        cc.imageSmoothingEnabled = size < 16
        cc.imageSmoothingQuality = "high"
        for (let y = 0; y < CHUNK; y++)
          for (let x = 0; x < CHUNK; x++) {
            const gx = cx * CHUNK + x
            const gy = cy * CHUNK + y
            if (gx >= width || gy >= height) continue
            const k = grid[gy * width + gx]!
            if (k === EMPTY) continue
            cc.drawImage(
              atlas,
              (k % columns) * 16,
              Math.floor(k / columns) * 16,
              16,
              16,
              x * size,
              y * size,
              size,
              size,
            )
          }
        chunks.set(key, c)
        const limit = size === 16 ? 96 : 400
        let over = [...chunks.keys()].filter((k) => k.startsWith(`${size}:`)).length - limit
        for (const k of chunks.keys()) {
          if (over <= 0) break
          if (k.startsWith(`${size}:`)) {
            chunks.delete(k)
            over--
          }
        }
        return c
      }

      // The camera: the world point at the screen's center, and CSS pixels per metatile.
      const fitZoom = () =>
        Math.min(canvas.clientWidth / width, canvas.clientHeight / height) * 0.95
      const camera = { x: width / 2, y: height / 2, zoom: fitZoom() }
      const clamp = () => {
        camera.zoom = Math.min(MAX_ZOOM, Math.max(fitZoom() * 0.8, camera.zoom))
        camera.x = Math.min(width, Math.max(0, camera.x))
        camera.y = Math.min(height, Math.max(0, camera.y))
      }

      const draw = () => {
        raf = 0
        const ratio = window.devicePixelRatio
        const w = Math.round(canvas.clientWidth * ratio)
        const h = Math.round(canvas.clientHeight * ratio)
        if (canvas.width !== w || canvas.height !== h) {
          canvas.width = w
          canvas.height = h
        }
        const scale = camera.zoom * ratio
        const left = camera.x - w / 2 / scale
        const top = camera.y - h / 2 / scale
        context.setTransform(1, 0, 0, 1, 0, 0)
        // Open sea past the edges, the color the map view deepens to.
        context.fillStyle = "rgb(52 92 178)"
        context.fillRect(0, 0, w, h)

        // The map view far out, the tiles close in, cross-fading between.
        const tiles = Math.min(1, Math.max(0, (scale - FADE_FROM) / (FADE_TO - FADE_FROM)))
        if (tiles > 0) {
          const size = scale < 10 ? 4 : 16
          context.imageSmoothingEnabled = scale < size
          const firstX = Math.max(0, Math.floor(left / CHUNK))
          const firstY = Math.max(0, Math.floor(top / CHUNK))
          const lastX = Math.min(
            Math.ceil(width / CHUNK) - 1,
            Math.floor((left + w / scale) / CHUNK),
          )
          const lastY = Math.min(
            Math.ceil(height / CHUNK) - 1,
            Math.floor((top + h / scale) / CHUNK),
          )
          for (let cy = firstY; cy <= lastY; cy++)
            for (let cx = firstX; cx <= lastX; cx++) {
              const x = Math.round((cx * CHUNK - left) * scale)
              const y = Math.round((cy * CHUNK - top) * scale)
              const next = Math.round(((cx + 1) * CHUNK - left) * scale)
              const below = Math.round(((cy + 1) * CHUNK - top) * scale)
              context.drawImage(chunk(cx, cy, size), x, y, next - x, below - y)
            }
        }
        if (tiles < 1) {
          context.globalAlpha = 1 - tiles
          context.imageSmoothingEnabled = scale < 2
          context.imageSmoothingQuality = "high"
          context.drawImage(overview, -left * scale, -top * scale, width * scale, height * scale)
          context.globalAlpha = 1
        }

        // Region names far out, fading as the town names take over.
        const regionAlpha = Math.min(1, Math.max(0, (1.1 - camera.zoom) / 0.4))
        if (regionAlpha > 0) {
          context.globalAlpha = regionAlpha
          context.font = `${Math.round(28 * ratio)}px "Wayfarer Text", monospace`
          context.textAlign = "center"
          context.textBaseline = "middle"
          for (const region of meta.regions) {
            const x = (region.x - left) * scale
            const y = (region.y - top) * scale
            context.fillStyle = "rgb(0 0 0 / 0.75)"
            context.fillText(region.name, x + ratio * 2, y + ratio * 2)
            context.fillStyle = "rgb(255 214 0)"
            context.fillText(region.name, x, y)
          }
          context.globalAlpha = 1
          context.textBaseline = "alphabetic"
        }

        // Town names while they help: not when everything is a speck, nor
        // when the town fills the screen.
        if (camera.zoom > 0.9 && camera.zoom < 14) {
          context.font = `${Math.round(16 * ratio)}px "Wayfarer Text", monospace`
          context.textAlign = "center"
          for (const town of meta.towns) {
            const x = (town.x - left) * scale
            const y = (town.y - top) * scale
            if (x < -200 || y < -50 || x > w + 200 || y > h + 50) continue
            context.fillStyle = "rgb(0 0 0 / 0.7)"
            context.fillText(townName(town.name), x + ratio * 1.5, y + ratio * 1.5)
            context.fillStyle = "rgb(248 248 248)"
            context.fillText(townName(town.name), x, y)
          }
        }
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
      canvas.addEventListener("pointerdown", (event) => {
        canvas.setPointerCapture(event.pointerId)
        pointers.set(event.pointerId, { x: event.offsetX, y: event.offsetY })
        if (pointers.size === 2) {
          const [a, b] = [...pointers.values()]
          pinch = Math.hypot(a!.x - b!.x, a!.y - b!.y)
        }
      })
      canvas.addEventListener("pointermove", (event) => {
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
      })
      const release = (event: PointerEvent) => {
        pointers.delete(event.pointerId)
        pinch = 0
      }
      canvas.addEventListener("pointerup", release)
      canvas.addEventListener("pointercancel", release)
      canvas.addEventListener(
        "wheel",
        (event) => {
          event.preventDefault()
          zoomAt(Math.exp(-event.deltaY * 0.0015), event.offsetX, event.offsetY)
        },
        { passive: false },
      )
      canvas.addEventListener("dblclick", (event) => zoomAt(2, event.offsetX, event.offsetY))
      new ResizeObserver(schedule).observe(canvas)
      await document.fonts.ready
      schedule()
    })()

    return () => {
      disposed = true
      cancelAnimationFrame(raf)
    }
  }, [base])

  return (
    <div className="relative size-full">
      <canvas ref={canvasRef} className="block size-full touch-none" aria-label="World map" />
      <div className="absolute right-4 bottom-4 flex flex-col gap-2">
        {[
          ["+", 1.6],
          ["−", 1 / 1.6],
        ].map(([label, factor]) => (
          <button
            key={label}
            type="button"
            onClick={() => zoomBy.current(factor as number)}
            className="pixel-corners pixel-panel size-12 text-2xl text-paper"
            aria-label={label === "+" ? "Zoom in" : "Zoom out"}
          >
            {label}
          </button>
        ))}
      </div>
    </div>
  )
}

import { useEffect, useRef, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"
import { LANES, LITTLEROOT, REGIONS, SEA, VOYAGES, along, frameAt } from "~/lib/world"

const loadImage = (src: string) =>
  new Promise<HTMLImageElement>((resolve, reject) => {
    const image = new Image()
    image.addEventListener("load", () => resolve(image))
    image.addEventListener("error", reject)
    image.src = src
  })

export type Caption = {
  place: string
  title: string
  body: string
  stats?: { value: string; label: string }[]
}

/** Where the lanes start drawing in and the ships set sail, in scroll progress. */
const LANES_FROM = 0.6
const LANES_TO = 0.86
/** How far a ship sails each second, in map pixels. */
const KNOTS = 14
/** The ferry's size on the map, a quarter of its 64x40 sprite. */
const SHIP = { width: 16, height: 10 }

/**
 * The world's region maps on one ocean, with a camera that starts on
 * LITTLEROOT TOWN and pulls back to all of Hoenn, then to the whole world,
 * as its section scrolls by. Fills its sticky parent; the section around it
 * sets how long the pull lasts.
 */
export const WorldZoom = ({ captions }: { captions: Caption[] }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const captionRef = useRef<HTMLDivElement>(null)
  const reducedMotion = useReducedMotion()
  const [beat, setBeat] = useState(0)

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    const section = canvas?.closest("section")
    if (!canvas || !context || !section) return

    let raf = 0
    let visible = false
    let images: {
      regions: HTMLImageElement[]
      brendan: HTMLImageElement
      ferry: HTMLImageElement
    } | null = null

    const progress = () => {
      if (reducedMotion) return 1
      const { top, height } = section.getBoundingClientRect()
      const travel = height - window.innerHeight
      return travel > 0 ? -top / travel : 1
    }

    const draw = (time: number) => {
      raf = 0
      // Keep sailing while on screen; otherwise redraw only on scroll.
      if (visible && !reducedMotion) raf = requestAnimationFrame(draw)
      const ratio = window.devicePixelRatio
      const width = Math.round(canvas.clientWidth * ratio)
      const height = Math.round(canvas.clientHeight * ratio)
      if (canvas.width !== width || canvas.height !== height) {
        canvas.width = width
        canvas.height = height
      }
      const scrolled = progress()
      const frame = frameAt(scrolled)
      setBeat(frame.beat)

      // The map gets the screen beside the caption on wide screens, above it
      // on narrow ones.
      const area = { width, height }
      const caption = captionRef.current?.getBoundingClientRect()
      if (caption) {
        const canvasBox = canvas.getBoundingClientRect()
        if (caption.left - canvasBox.left > canvasBox.width * 0.4)
          area.width = (caption.left - canvasBox.left - 24) * ratio
        else area.height = (caption.top - canvasBox.top - 16) * ratio
      }

      // Device pixels per map pixel, fitting the frame into that space.
      const scale = Math.min(area.width / frame.width, area.height / frame.height)
      const originX = (area.width - frame.width * scale) * frame.alignX - frame.x * scale
      const originY = (area.height - frame.height * scale) * frame.alignY - frame.y * scale

      context.setTransform(1, 0, 0, 1, 0, 0)
      context.imageSmoothingEnabled = false
      // The sea's stripes, a map pixel tall, while they are big enough to see.
      context.fillStyle = SEA[0]!
      context.fillRect(0, 0, width, height)
      if (scale >= 2) {
        context.fillStyle = SEA[1]!
        const firstRow = Math.floor(-originY / scale)
        for (let row = firstRow; row * scale + originY < height; row++) {
          if (row % 2 === 0) continue
          const y = Math.round(row * scale + originY)
          context.fillRect(0, y, width, Math.round((row + 1) * scale + originY) - y)
        }
      }
      if (!images) return

      context.setTransform(scale, 0, 0, scale, originX, originY)
      REGIONS.forEach(({ x, y }, i) => context.drawImage(images!.regions[i]!, x, y))
      context.drawImage(images.brendan, LITTLEROOT.x - 8, LITTLEROOT.y - 8)

      // The ferry lanes draw in as the camera pulls out to the world, dotted
      // every few map pixels, then the three ships sail their loops.
      const drawn = Math.min(1, Math.max(0, (scrolled - LANES_FROM) / (LANES_TO - LANES_FROM)))
      if (drawn === 0) return
      context.fillStyle = "rgb(248 248 248 / 0.85)"
      for (const lane of LANES) {
        for (let d = 0; d < lane.length * drawn; d += 5) {
          const { x, y } = along(lane, d)
          context.fillRect(Math.round(x) - 1, Math.round(y) - 1, 2, 2)
        }
      }

      context.globalAlpha = drawn
      context.imageSmoothingEnabled = true
      context.imageSmoothingQuality = "high"
      VOYAGES.forEach((voyage, i) => {
        const { x, y, heading } = along(
          voyage,
          (time / 1000) * KNOTS + (voyage.length * i) / VOYAGES.length,
        )
        context.save()
        context.translate(Math.round(x), Math.round(y))
        if (heading > 0) context.scale(-1, 1)
        context.drawImage(images!.ferry, -SHIP.width / 2, -SHIP.height + 2, SHIP.width, SHIP.height)
        context.restore()
      })
      context.globalAlpha = 1
    }

    const schedule = () => {
      if (!raf) raf = requestAnimationFrame(draw)
    }

    void Promise.all([
      Promise.all(REGIONS.map(({ file }) => loadImage(`/game/world/${file}.png`))),
      loadImage("/game/world/brendan.png"),
      loadImage("/game/world/ferry.png"),
    ]).then(([regions, brendan, ferry]) => {
      images = { regions, brendan, ferry }
      schedule()
    })

    const watcher = new IntersectionObserver(([entry]) => {
      visible = entry?.isIntersecting ?? false
      schedule()
    })
    watcher.observe(canvas)

    schedule()
    window.addEventListener("scroll", schedule, { passive: true })
    const resizer = new ResizeObserver(schedule)
    resizer.observe(canvas)
    return () => {
      cancelAnimationFrame(raf)
      window.removeEventListener("scroll", schedule)
      resizer.disconnect()
      watcher.disconnect()
    }
  }, [reducedMotion])

  return (
    <>
      <canvas ref={canvasRef} aria-hidden="true" className="absolute inset-0 size-full" />
      <div className="pointer-events-none relative flex h-full items-end justify-end p-4 sm:p-8 lg:items-center lg:p-12">
        <div
          ref={captionRef}
          className="pixel-corners pixel-panel pointer-events-auto relative grid w-full p-6 sm:p-8 lg:w-[400px]"
        >
          {captions.map((caption, i) => (
            <div
              key={caption.place}
              className="col-start-1 row-start-1 transition-opacity duration-300"
              style={{ opacity: (reducedMotion ? captions.length - 1 : beat) === i ? 1 : 0 }}
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
          ))}
        </div>
      </div>
    </>
  )
}

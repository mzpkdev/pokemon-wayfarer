import { useEffect, useRef } from "react"

import {
  OAK,
  OPEN_FADE,
  OVAL,
  type ReleaseState,
  STAGE,
  releaseAt,
  restingRelease,
} from "~/lib/release-mon"

const loadImage = (src: string) =>
  new Promise<HTMLImageElement>((resolve, reject) => {
    const image = new Image()
    image.addEventListener("load", () => resolve(image))
    image.addEventListener("error", reject)
    image.src = src
  })

/**
 * PROF. OAK on the intro's grey oval, throwing a Poké Ball that releases
 * WOOPER, drawn at GBA resolution and scaled by whole pixels through CSS.
 */
export const OakStage = ({ className }: { className?: string }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    if (!canvas || !context) return
    context.imageSmoothingEnabled = false

    const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches
    let raf = 0
    let visible = true
    let frame = 0
    let carry = 0
    let lastTime: number | null = null

    const monCanvas = document.createElement("canvas")
    monCanvas.width = 64
    monCanvas.height = 64
    const monContext = monCanvas.getContext("2d")!

    const run = async () => {
      const [oval, oak, mon, ball, particles] = await Promise.all(
        [
          "/game/people/stage.png",
          "/game/people/oak.png",
          "/game/people/wooper.png",
          "/game/people/poke-ball.png",
          "/game/people/ball-particles.png",
        ].map(loadImage),
      )

      const draw = ({
        ball: ballState,
        particles: particleStates,
        mon: monState,
      }: ReleaseState) => {
        context.clearRect(0, 0, STAGE.width, STAGE.height)
        context.drawImage(oval!, OVAL.x, OVAL.y)

        if (monState) {
          // Tint toward the ball's open color, then draw scaled about its center.
          monContext.clearRect(0, 0, 64, 64)
          monContext.globalCompositeOperation = "source-over"
          monContext.drawImage(mon!, 0, monState.frame * 64, 64, 64, 0, 0, 64, 64)
          if (monState.tint > 0) {
            monContext.globalCompositeOperation = "source-atop"
            monContext.globalAlpha = monState.tint
            monContext.fillStyle = OPEN_FADE
            monContext.fillRect(0, 0, 64, 64)
            monContext.globalAlpha = 1
          }
          const size = Math.max(1, Math.round(64 * monState.scale))
          context.drawImage(
            monCanvas,
            Math.round(monState.x - size / 2),
            Math.round(monState.y - size / 2),
            size,
            size,
          )
        }

        context.drawImage(oak!, OAK.x, OAK.y)

        if (ballState) {
          context.save()
          context.translate(ballState.x, ballState.y)
          context.rotate((ballState.rotation * Math.PI) / 2)
          context.drawImage(ball!, 0, ballState.frame * 16, 16, 16, -8, -8, 16, 16)
          context.restore()
        }

        for (const particle of particleStates) {
          context.save()
          context.translate(particle.x, particle.y)
          if (particle.flip) context.scale(-1, 1)
          context.drawImage(particles!, 0, particle.frame * 8, 8, 8, -4, -4, 8, 8)
          context.restore()
        }
      }

      if (reducedMotion) {
        draw(restingRelease)
        return
      }

      const tick = (time: number) => {
        raf = requestAnimationFrame(tick)
        if (!visible) {
          lastTime = null
          return
        }
        if (lastTime !== null) carry += ((time - lastTime) * 60) / 1000
        lastTime = time
        const steps = Math.min(Math.floor(carry), 10)
        carry -= steps
        frame += steps
        draw(releaseAt(frame))
      }
      raf = requestAnimationFrame(tick)
    }
    void run()

    const observer = new IntersectionObserver(([entry]) => {
      visible = entry?.isIntersecting ?? true
    })
    observer.observe(canvas)
    return () => {
      cancelAnimationFrame(raf)
      observer.disconnect()
    }
  }, [])

  return (
    <canvas
      ref={canvasRef}
      width={STAGE.width}
      height={STAGE.height}
      aria-hidden="true"
      className={className}
      style={{
        width: `calc(${STAGE.width} * var(--s))`,
        height: `calc(${STAGE.height} * var(--s))`,
        imageRendering: "pixelated",
      }}
    />
  )
}

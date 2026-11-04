import { useEffect, useRef } from "react"

import {
  OAK,
  OPEN_FADE,
  OVAL,
  ROSTER,
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
 * PROF. OAK on the intro's grey oval (or, without it, on his own shadow),
 * throwing a Poké Ball that releases WOOPER, then each of the opening's
 * POKéMON in turn, drawn at GBA resolution and scaled by whole pixels through CSS.
 */
export const OakStage = ({ className, oval = true }: { className?: string; oval?: boolean }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    if (!canvas || !context) return

    // Draw at the screen's own resolution, `pixels` device pixels to a GBA
    // pixel, so POKéMON scaled to their height keep their detail.
    let pixels = 1
    const fit = () => {
      pixels = Math.max(1, Math.round((canvas.clientWidth / STAGE.width) * window.devicePixelRatio))
      canvas.width = STAGE.width * pixels
      canvas.height = STAGE.height * pixels
      context.imageSmoothingEnabled = false
    }
    fit()

    const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches
    let raf = 0
    let visible = true
    let frame = 0
    let carry = 0
    let lastTime: number | null = null
    // Redraws a still scene after a resize; the animation redraws on its own.
    let resized = () => {}

    const monCanvas = document.createElement("canvas")
    monCanvas.width = 64
    monCanvas.height = 64
    const monContext = monCanvas.getContext("2d")!

    const run = async () => {
      const [[ovalImage, oak, ball, particles], mons] = await Promise.all([
        Promise.all(
          [
            "/game/people/stage.png",
            "/game/people/oak.png",
            "/game/people/poke-ball.png",
            "/game/people/ball-particles.png",
          ].map(loadImage),
        ),
        Promise.all(ROSTER.map(({ species }) => loadImage(`/game/people/mons/${species}.png`))),
      ])

      const draw = ({
        ball: ballState,
        particles: particleStates,
        mon: monState,
      }: ReleaseState) => {
        context.setTransform(pixels, 0, 0, pixels, 0, 0)
        context.clearRect(0, 0, STAGE.width, STAGE.height)
        if (oval) context.drawImage(ovalImage!, OVAL.x, OVAL.y)
        else {
          // No oval: just the shade he casts on the grass, where the oval would lie.
          context.fillStyle = "rgb(0 0 0 / 0.32)"
          context.beginPath()
          context.ellipse(OVAL.x + 64, OVAL.y + 18, 56, 11, 0, 0, Math.PI * 2)
          context.fill()
        }

        if (monState) {
          // Tint toward the ball's open color, then draw scaled about its center.
          monContext.clearRect(0, 0, 64, 64)
          monContext.globalCompositeOperation = "source-over"
          monContext.drawImage(
            mons[monState.species]!,
            0,
            monState.frame * 64,
            64,
            64,
            0,
            0,
            64,
            64,
          )
          if (monState.tint > 0) {
            monContext.globalCompositeOperation = "source-atop"
            monContext.globalAlpha = monState.tint
            monContext.fillStyle = OPEN_FADE
            monContext.fillRect(0, 0, 64, 64)
            monContext.globalAlpha = 1
          }
          // Stand its feet on (x, y), snapped to whole device pixels.
          const { center, foot } = ROSTER[monState.species]!
          const snap = (value: number) => Math.round(value * pixels) / pixels
          const size = Math.max(1 / pixels, snap(64 * monState.scale))
          context.drawImage(
            monCanvas,
            snap(monState.x - center * monState.scale),
            snap(monState.y - foot * monState.scale),
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
        resized = () => draw(restingRelease)
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
    // The stage grows a step at wider screens; redraw at the new resolution.
    const resizer = new ResizeObserver(() => {
      fit()
      resized()
    })
    resizer.observe(canvas)
    return () => {
      cancelAnimationFrame(raf)
      observer.disconnect()
      resizer.disconnect()
    }
  }, [oval])

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

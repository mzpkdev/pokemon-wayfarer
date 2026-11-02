import { type CSSProperties, useEffect, useRef, useState } from "react"

import { Countdown } from "~/components/Countdown"
import { DialogBox } from "~/components/DialogBox"
import { RELEASE_LABEL } from "~/lib/release"
import {
  BICYCLE,
  LAYERS,
  LOGO,
  LOGO_RISE,
  PAN_FRAMES,
  PASSERS,
  type Passer,
  POKEBALL,
  PRESS_START,
  SCREEN_HEIGHT,
  SCREEN_WIDTH,
  SHINE,
  TORCHIC_TRIP,
  WORDMARK,
  WORDMARK_RISE,
  layerOffset,
  loopFrame,
  pixelScale,
  pokeballCenterY,
  pressStartVisible,
  riseOffset,
} from "~/lib/scene"

const SKY = "rgb(131 205 238)"
const GROUND = "rgb(8 41 0)"

/** Intro timeline in frames: fade in on the Poké Ball, wait, then pan up. */
const FADE_FRAMES = 32
const PAN_START = FADE_FRAMES + 90
const INTRO_END = PAN_START + PAN_FRAMES

/** How far, in viewport heights, scrolling takes the camera back down to the Poké Ball. */
const SCRUB_VH = 1.6
/**
 * Where scrolling leaves the camera: a little lower than the intro starts, so
 * the Poké Ball sits between the countdown and the dialog box.
 */
const BALL_PAN = -0.08
/** The Poké Ball's top and bottom on screen once scrolled down to it. */
const BALL_TOP = Math.round(pokeballCenterY(BALL_PAN)) - POKEBALL.size / 2
const BALL_BOTTOM = BALL_TOP + POKEBALL.size
/** Rough heights of the countdown and a two-line dialog box, in GBA pixels. */
const COUNTDOWN_HEIGHT = 70
const DIALOG_HEIGHT = 48

/**
 * Extra viewport heights the stage stays pinned at the Poké Ball. The page
 * after it overlaps all but the first 0.3 of this (see index.astro), so it
 * rises over the pinned scene like a nearer layer.
 */
const HOLD_VH = 1

type Viewport = { scale: number; width: number; height: number; originX: number; originY: number }

const measure = (): Viewport => {
  const scale = pixelScale(window.innerWidth, window.innerHeight)
  const width = Math.ceil(window.innerWidth / scale)
  const height = Math.ceil(window.innerHeight / scale)
  return {
    scale,
    width,
    height,
    originX: Math.floor((width - SCREEN_WIDTH) / 2),
    originY: Math.floor((height - SCREEN_HEIGHT) / 2),
  }
}

type Pass = {
  passer: Passer
  x: number
  direction: 1 | -1
  tick: number
  /** Torchic runs, trips at mid-screen, then runs on. */
  state: "run" | "trip" | "again"
  tripTick: number
}

const TRIP_FRAMES = TORCHIC_TRIP.reduce((sum, step) => sum + step.duration, 0)

const loadImage = (src: string) =>
  new Promise<HTMLImageElement>((resolve, reject) => {
    const image = new Image()
    image.addEventListener("load", () => resolve(image))
    image.addEventListener("error", reject)
    image.src = src
  })

const prefersReducedMotion = () => window.matchMedia("(prefers-reduced-motion: reduce)").matches

/** Scrolls far enough that the camera has panned back down to the Poké Ball. */
const scrollToBall = (section: HTMLElement) =>
  window.scrollTo({
    top: section.offsetTop + window.innerHeight * SCRUB_VH,
    behavior: prefersReducedMotion() ? "auto" : "smooth",
  })

export const TitleScene = () => {
  const sectionRef = useRef<HTMLElement>(null)
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const [viewport, setViewport] = useState<Viewport | null>(null)
  const [atBall, setAtBall] = useState(false)
  const [atTitle, setAtTitle] = useState(false)
  const skipRef = useRef<() => void>(() => {})

  useEffect(() => {
    const canvas = canvasRef.current
    const section = sectionRef.current
    if (!canvas || !section) return
    const context = canvas.getContext("2d")
    if (!context) return

    let view = measure()
    const resize = () => {
      view = measure()
      canvas.width = view.width
      canvas.height = view.height
      context.imageSmoothingEnabled = false
      setViewport(view)
      // Lets the page below draw its pixel art at the same scale and tiling origin.
      const root = document.documentElement.style
      root.setProperty("--scene-px", `${view.scale}px`)
      root.setProperty("--scene-x", `${view.originX * view.scale}px`)
    }
    resize()

    const reducedMotion = prefersReducedMotion()
    // Start at the title, skipping the intro, when motion is reduced or the page
    // was reloaded part-way down.
    let frame = reducedMotion || window.scrollY > 0 ? INTRO_END : 0
    let revealStart = frame === INTRO_END ? INTRO_END - LOGO_RISE.frames - 10 : INTRO_END
    let nextPass = INTRO_END + 180
    let pass: Pass | null = null
    let visible = true
    let lastTime: number | null = null
    let carry = 0
    let raf = 0
    let titleShown: boolean | null = null
    let ballShown: boolean | null = null

    skipRef.current = () => {
      if (frame < INTRO_END) {
        frame = INTRO_END
        revealStart = INTRO_END
        nextPass = INTRO_END + 180
      }
    }

    /** Viewport heights scrolled into the section. */
    const scrolledScreens = () => -section.getBoundingClientRect().top / window.innerHeight

    /** 0 at the Poké Ball, 1 at the title; scrolling down pans back down. */
    const scrollPan = () => {
      const travelled = -section.getBoundingClientRect().top
      const progress = Math.min(1, Math.max(0, travelled / (window.innerHeight * SCRUB_VH)))
      return 1 - progress * (1 - BALL_PAN)
    }

    const startPass = (): Pass => {
      const passer = PASSERS[Math.floor(Math.random() * PASSERS.length)]!
      const direction = Math.random() < 0.5 ? 1 : -1
      return {
        passer,
        direction,
        x: direction > 0 ? -passer.offscreen : SCREEN_WIDTH + passer.offscreen,
        tick: 0,
        state: "run",
        tripTick: 0,
      }
    }

    const stepPass = (current: Pass): Pass | null => {
      const { passer, direction } = current
      current.tick++
      if (current.state === "trip") {
        if (++current.tripTick >= TRIP_FRAMES) current.state = "again"
        return current
      }
      current.x += direction * passer.speed
      const pastMiddle =
        direction > 0 ? current.x >= SCREEN_WIDTH / 2 : current.x <= SCREEN_WIDTH / 2
      if (passer.name === "torchic" && current.state === "run" && pastMiddle) {
        current.x = SCREEN_WIDTH / 2
        current.state = "trip"
      }
      const gone =
        direction > 0 ? current.x > SCREEN_WIDTH + passer.offscreen : current.x < -passer.offscreen
      return gone ? null : current
    }

    const run = async () => {
      const [layers, pokeball, logo, wordmark, pressStart, bicycle, ...passers] = await Promise.all(
        [
          Promise.all(LAYERS.map((layer) => loadImage(layer.src))),
          loadImage(POKEBALL.src),
          loadImage(LOGO.src),
          loadImage(WORDMARK.src),
          loadImage(PRESS_START.src),
          loadImage(BICYCLE.src),
          ...PASSERS.map((passer) => loadImage(passer.src)),
        ],
      )
      const passerImages = new Map(PASSERS.map((passer, i) => [passer.name, passers[i]!]))

      // The logo with its shine is composed off-screen, then drawn as one sprite.
      const logoCanvas = document.createElement("canvas")
      logoCanvas.width = LOGO.width
      logoCanvas.height = LOGO.height
      const logoContext = logoCanvas.getContext("2d")!

      // Past the GBA screen's edges each layer continues as alternating mirror
      // images, so the edges always meet themselves without a seam.
      const mirrored = new Map(
        layers.map((image) => {
          const flipped = document.createElement("canvas")
          flipped.width = image.width
          flipped.height = image.height
          const flippedContext = flipped.getContext("2d")!
          flippedContext.scale(-1, 1)
          flippedContext.drawImage(image, -image.width, 0)
          return [image, flipped] as const
        }),
      )

      const drawLayer = (image: HTMLImageElement, offset: number) => {
        const { originX, originY, width } = view
        const y = originY - offset
        const first = Math.floor(-originX / SCREEN_WIDTH)
        for (let tile = first; originX + tile * SCREEN_WIDTH < width; tile++)
          context.drawImage(
            tile % 2 === 0 ? image : mirrored.get(image)!,
            originX + tile * SCREEN_WIDTH,
            y,
          )
      }

      const drawPass = (current: Pass) => {
        const { passer, direction } = current
        const [width, height] = passer.size
        const image = passerImages.get(passer.name)!
        const frameIndex =
          current.state === "trip"
            ? loopFrame(TORCHIC_TRIP, current.tripTick)
            : loopFrame(passer.run, current.tick)
        const bob = passer.bob ? Math.round(Math.sin((current.tick * 8 * Math.PI) / 128) * 6) : 0
        const x = view.originX + Math.round(current.x) - width / 2
        const y = view.originY + passer.y - height / 2 + bob
        context.save()
        if (direction > 0) {
          context.translate(x + width, 0)
          context.scale(-1, 1)
        } else context.translate(x, 0)
        if (passer.bicycle) {
          const [bikeWidth, bikeHeight] = BICYCLE.size
          context.drawImage(
            bicycle,
            0,
            loopFrame(BICYCLE.run, current.tick) * bikeHeight,
            bikeWidth,
            bikeHeight,
            0,
            y + height / 2 + 8 - bikeHeight / 2,
            bikeWidth,
            bikeHeight,
          )
        }
        context.drawImage(image, 0, frameIndex * height, width, height, 0, y, width, height)
        context.restore()
      }

      const drawLogo = (reveal: number) => {
        const rise = riseOffset(LOGO_RISE, reveal)
        if (rise === null) return
        logoContext.clearRect(0, 0, LOGO.width, LOGO.height)
        logoContext.globalCompositeOperation = "source-over"
        logoContext.drawImage(logo, 0, 0)
        const sweep = reveal - LOGO_RISE.frames - 30
        if (sweep >= 0 && !reducedMotion) {
          // Shine windows are screen columns; the top half leads the bottom by the skew.
          const screenX = (sweep % SHINE.every) * SHINE.speed - SHINE.width - SHINE.skew - LOGO.x
          logoContext.globalCompositeOperation = "source-atop"
          logoContext.fillStyle = "rgb(255 255 255 / 0.55)"
          const half = Math.floor(LOGO.height / 2)
          logoContext.fillRect(screenX + SHINE.skew, 0, SHINE.width, half)
          logoContext.fillRect(screenX, half, SHINE.width, LOGO.height - half)
        }
        context.drawImage(logoCanvas, view.originX + LOGO.x, view.originY + LOGO.y + rise)
      }

      const draw = (pan: number) => {
        const { originX, originY, width, height } = view
        // Backdrop: sky above the scene, ground below it.
        const horizon = originY + 128
        context.fillStyle = SKY
        context.fillRect(0, 0, width, horizon)
        context.fillStyle = GROUND
        context.fillRect(0, horizon, width, height - horizon)

        const [bg3, bg2, bg1, bg0] = layers
        drawLayer(bg3!, layerOffset(LAYERS[0]!, pan))

        const reveal = frame - revealStart
        if (frame >= INTRO_END) {
          if (pass) drawPass(pass)
          drawLogo(reveal)
          const wordmarkRise = riseOffset(WORDMARK_RISE, reveal)
          if (wordmarkRise !== null)
            context.drawImage(wordmark, originX + WORDMARK.x, originY + WORDMARK.y + wordmarkRise)
        }

        drawLayer(bg2!, layerOffset(LAYERS[1]!, pan))
        drawLayer(bg1!, layerOffset(LAYERS[2]!, pan))
        const ballY = pokeballCenterY(pan)
        if (ballY < SCREEN_HEIGHT + 32)
          context.drawImage(
            pokeball,
            originX + POKEBALL.x - POKEBALL.size / 2,
            originY + ballY - POKEBALL.size / 2,
          )
        drawLayer(bg0!, layerOffset(LAYERS[3]!, pan))

        const pressStartReady = reveal > LOGO_RISE.frames + 20
        if (pan === 1 && pressStartReady && (reducedMotion || pressStartVisible(reveal)))
          context.drawImage(pressStart, originX + PRESS_START.x, originY + PRESS_START.y)

        if (frame < FADE_FRAMES) {
          // Palette fades step through 16 levels.
          const level = Math.floor((frame / FADE_FRAMES) * 16)
          context.fillStyle = `rgb(0 0 0 / ${(16 - level) / 16})`
          context.fillRect(0, 0, width, height)
        }
      }

      const tick = (time: number) => {
        raf = requestAnimationFrame(tick)
        if (!visible) {
          lastTime = null
          return
        }
        // Advance at the GBA's 60 frames a second, whatever the display rate.
        if (lastTime !== null) carry += ((time - lastTime) * 60) / 1000
        lastTime = time
        const steps = Math.min(Math.floor(carry), 10)
        carry -= Math.floor(carry)

        const scrolled = scrollPan()
        for (let step = 0; step < steps; step++) {
          frame++
          if (frame >= INTRO_END && scrolled === 1 && !reducedMotion) {
            if (pass) pass = stepPass(pass)
            else if (frame >= nextPass) {
              pass = startPass()
              nextPass = frame + 480 + Math.floor(Math.random() * 360)
            }
          }
        }
        if (frame === INTRO_END - 1) revealStart = INTRO_END

        const pan =
          frame < PAN_START ? 0 : frame < INTRO_END ? (frame - PAN_START) / PAN_FRAMES : scrolled
        draw(pan)

        const title = frame >= INTRO_END && pan === 1
        if (title !== titleShown) setAtTitle((titleShown = title))
        // The countdown and dialog leave as the page below starts rising over the scene.
        const ball = frame >= INTRO_END && pan < 0.06 && scrolledScreens() < SCRUB_VH + 0.35
        if (ball !== ballShown) setAtBall((ballShown = ball))
      }
      raf = requestAnimationFrame(tick)
    }
    void run()

    const observer = new IntersectionObserver(([entry]) => {
      visible = entry?.isIntersecting ?? true
    })
    observer.observe(section)
    window.addEventListener("resize", resize)
    return () => {
      cancelAnimationFrame(raf)
      observer.disconnect()
      window.removeEventListener("resize", resize)
    }
  }, [])

  useEffect(() => {
    // Any input during the intro skips to the title, as in the game.
    const skip = () => skipRef.current()
    const keydown = (event: KeyboardEvent) => {
      skip()
      if (event.target === document.body && event.key === "Enter" && window.scrollY < 8) {
        event.preventDefault()
        if (sectionRef.current) scrollToBall(sectionRef.current)
      }
    }
    window.addEventListener("keydown", keydown)
    window.addEventListener("pointerdown", skip)
    window.addEventListener("wheel", skip, { passive: true })
    window.addEventListener("touchmove", skip, { passive: true })
    return () => {
      window.removeEventListener("keydown", keydown)
      window.removeEventListener("pointerdown", skip)
      window.removeEventListener("wheel", skip)
      window.removeEventListener("touchmove", skip)
    }
  }, [])

  const pressStart = () => {
    skipRef.current()
    if (sectionRef.current) scrollToBall(sectionRef.current)
  }

  const s = viewport?.scale ?? 3
  const at = (x: number, y: number): CSSProperties =>
    viewport
      ? { left: (viewport.originX + x) * s, top: (viewport.originY + y) * s }
      : { visibility: "hidden" }

  return (
    <section
      ref={sectionRef}
      id="top"
      aria-labelledby="title"
      style={{ height: `${(1 + SCRUB_VH + HOLD_VH) * 100}vh` }}
      className="relative"
    >
      <div
        className="sticky top-0 h-screen overflow-hidden bg-black"
        style={{ "--px": `${s}px` } as CSSProperties}
      >
        <canvas
          ref={canvasRef}
          aria-hidden="true"
          className="pixelated block"
          style={viewport ? { width: viewport.width * s, height: viewport.height * s } : undefined}
        />
        <h1 id="title" className="sr-only">
          Pokémon Wayfarer
        </h1>

        <button
          type="button"
          onClick={pressStart}
          aria-label="Press Start: go to the release countdown"
          className="absolute cursor-pointer focus-visible:outline-2 focus-visible:outline-offset-4 focus-visible:outline-white"
          style={{
            ...at(PRESS_START.x - 6, PRESS_START.y - 4),
            width: (PRESS_START.width + 12) * s,
            height: (PRESS_START.height + 8) * s,
            pointerEvents: atTitle ? "auto" : "none",
          }}
        />

        <p
          className="gba-text gba-text-light absolute w-max -translate-x-1/2 transition-opacity duration-300"
          style={{ ...at(SCREEN_WIDTH / 2, 136), opacity: atTitle ? 1 : 0 }}
        >
          Release {RELEASE_LABEL}
        </p>

        <div
          className="absolute inset-x-0 flex justify-center transition-[opacity,translate] duration-300 data-[shown=false]:-translate-y-4 data-[shown=false]:opacity-0"
          data-shown={atBall}
          style={
            viewport
              ? {
                  bottom:
                    (viewport.height -
                      Math.max(COUNTDOWN_HEIGHT + 2, viewport.originY + BALL_TOP - 2)) *
                    s,
                }
              : { visibility: "hidden" }
          }
          aria-hidden={!atBall}
        >
          <Countdown />
        </div>

        <div
          className="absolute inset-x-0 flex justify-center px-[var(--px)] transition-[opacity,translate] duration-300 data-[shown=false]:translate-y-4 data-[shown=false]:opacity-0"
          data-shown={atBall}
          style={
            viewport
              ? {
                  top:
                    Math.min(
                      viewport.height - DIALOG_HEIGHT - 2,
                      viewport.originY + BALL_BOTTOM - 6,
                    ) * s,
                }
              : { visibility: "hidden" }
          }
          aria-hidden={!atBall}
        >
          <DialogBox
            key={String(atBall)}
            active={atBall}
            className="w-full max-w-[calc(240*var(--px))]"
            lines={["A POKé BALL lies in the tall grass.", `It opens on ${RELEASE_LABEL}!`]}
          />
        </div>
      </div>
    </section>
  )
}

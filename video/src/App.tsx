import { useEffect, useRef, useState } from "react"

import { clockText } from "~/world-map/living"

import { directionFor, drawFrame, loadStage, type Stage } from "./director"
import { type Cues, loadCues, type Slot, schedule, slotAt, totalSeconds } from "./schedule"
import { DIRECTIONS, type Span } from "./timeline"

/** Letters the caption prints a second: the game's MID text speed, a letter every two frames. */
const LETTERS_PER_SECOND = 30
/** Seconds a card or clip takes to fade in or out, and the video to fade from and to black. */
const FADE_SECONDS = 0.5
/** GBA pixels across the frame: the video is 480 of them wide, so 4 CSS pixels each at 1080p. */
const GBA_COLUMNS = 480

/** How much of a span is showing at `seconds` into the scene, fading at both ends. */
const alphaIn = (span: Span, seconds: number, length: number) => {
  const from = span.from * length
  const to = span.to * length
  if (seconds < from || seconds > to) return 0
  return Math.min(1, (seconds - from) / FADE_SECONDS, (to - seconds) / FADE_SECONDS)
}

type Clip = { fps: number; frames: string[] }

/** What the page is doing, for `scripts/frames.mjs` and the keyboard. */
export type Controls = {
  ready: boolean
  duration: number
  slots: Slot[]
  time: () => number
  seek: (seconds: number) => void
  play: () => void
  pause: () => void
  playing: () => boolean
  /** Whether the last frame left map chunks still to draw. */
  pending: () => boolean
}

declare global {
  // eslint-disable-next-line no-var
  var wayfarerVideo: Controls | undefined
}

/** Game art at `--px` CSS pixels per pixel, once its size is known. */
const PixelImage = ({ src, alt }: { src: string; alt: string }) => {
  const [pixels, setPixels] = useState(0)
  return (
    <img
      src={src}
      alt={alt}
      onLoad={(event) => setPixels(event.currentTarget.naturalWidth)}
      style={{
        width: pixels ? `calc(${pixels} * var(--px))` : undefined,
        imageRendering: "pixelated",
      }}
    />
  )
}

type Shown = {
  slot: Slot
  fraction: number
  seconds: number
  minute: number
}

/** Whether `scripts/frames.mjs` is driving the page: nothing but the picture then. */
const capture = new URLSearchParams(location.search).has("capture")

export const App = () => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const stageRef = useRef<HTMLDivElement>(null)
  const [stage, setStage] = useState<Stage>()
  const [cues, setCues] = useState<Cues | null | undefined>()
  const [failed, setFailed] = useState<string>()
  const [shown, setShown] = useState<Shown>()
  const [px, setPx] = useState(4)
  const [clips, setClips] = useState<Record<string, Clip | null>>({})

  useEffect(() => {
    loadStage().then(setStage, (error: unknown) => setFailed(String(error)))
    void loadCues().then(setCues)
    // The gameplay clips' manifests, for the scenes that show one; a missing clip shows a placeholder.
    for (const name of new Set(
      Object.values(DIRECTIONS).flatMap((d) => (d.insert ? [d.insert.clip] : [])),
    ))
      fetch(`/clips/${name}/manifest.json`)
        .then((response) => (response.ok ? (response.json() as Promise<Clip>) : null))
        .then(
          (loaded) => setClips((known) => ({ ...known, [name]: loaded })),
          () => {},
        )
  }, [])

  // The stage keeps 16:9 inside the window; one GBA pixel is a 480th of it.
  useEffect(() => {
    const element = stageRef.current
    if (!element) return
    const measure = () => setPx(Math.max(1, Math.round(element.clientWidth / GBA_COLUMNS)))
    measure()
    const observer = new ResizeObserver(measure)
    observer.observe(element)
    return () => observer.disconnect()
  }, [])

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    if (!stage || cues === undefined || !canvas || !context) return
    const slots = schedule(cues)
    const duration = totalSeconds(slots)
    const params = new URLSearchParams(location.search)
    let playing = !params.has("paused")
    let at = Math.min(duration, Math.max(0, Number(params.get("t")) || 0))
    let anchor = performance.now()
    let pending = false
    const time = () => (playing ? Math.min(duration, at + (performance.now() - anchor) / 1000) : at)
    const voices = new Map<string, HTMLAudioElement>()
    const spoken = new Set<string>()
    const hush = () => {
      for (const audio of voices.values()) audio.pause()
      spoken.clear()
    }
    const seek = (seconds: number) => {
      at = Math.min(duration, Math.max(0, seconds))
      anchor = performance.now()
      hush()
    }
    const pause = () => {
      at = time()
      playing = false
      hush()
    }
    const play = () => {
      if (time() >= duration) at = 0
      anchor = performance.now()
      playing = true
    }
    const controls: Controls = {
      ready: true,
      duration,
      slots,
      time,
      seek,
      play,
      pause,
      playing: () => playing,
      pending: () => pending,
    }
    globalThis.wayfarerVideo = controls

    let raf = 0
    const draw = () => {
      raf = requestAnimationFrame(draw)
      const seconds = time()
      const found = slotAt(slots, seconds)
      if (!found) return
      const { slot, fraction } = found
      pending = drawFrame(stage, context, slot, fraction)
      setShown({ slot, fraction, seconds, minute: stage.clock.minute })
      // In the browser the voice plays along, from each scene's cue.
      if (playing && slot.voiceAt !== null && seconds >= slot.voiceAt && !spoken.has(slot.id)) {
        spoken.add(slot.id)
        let audio = voices.get(slot.id)
        if (!audio) voices.set(slot.id, (audio = new Audio(`/audio/vo/${slot.id}.mp3`)))
        audio.currentTime = Math.max(0, seconds - slot.voiceAt)
        void audio.play().catch(() => {})
      }
      if (playing && seconds >= duration) pause()
    }
    raf = requestAnimationFrame(draw)

    const keys = (event: KeyboardEvent) => {
      const found = slotAt(slots, time())
      if (event.key === " ") (playing ? pause : play)()
      else if (event.key === "ArrowRight" && found) seek(found.slot.start + found.slot.length)
      else if (event.key === "ArrowLeft" && found)
        seek(
          time() - found.slot.start > 1 ? found.slot.start : Math.max(0, found.slot.start - 0.01),
        )
      else if (event.key === "Home") seek(0)
      else return
      event.preventDefault()
    }
    window.addEventListener("keydown", keys)
    return () => {
      cancelAnimationFrame(raf)
      window.removeEventListener("keydown", keys)
      hush()
      globalThis.wayfarerVideo = undefined
    }
  }, [stage, cues])

  const direction = shown && directionFor(shown.slot)
  const local = shown ? shown.fraction * shown.slot.length : 0
  const duration = shown ? totalSeconds(schedule(cues ?? null)) : 0
  const black = shown
    ? Math.max(0, 1 - Math.min(shown.seconds, duration - shown.seconds) / (FADE_SECONDS * 1.6))
    : 1

  const caption = shown?.slot.caption
  const captionText = caption?.join("\n") ?? ""
  const printed = Math.min(captionText.length, Math.floor(local * LETTERS_PER_SECOND))

  const title = direction?.title
  const titleAlpha = title && shown ? alphaIn(title, local, shown.slot.length) : 0

  const insert = direction?.insert
  const insertAlpha = insert && shown ? alphaIn(insert, local, shown.slot.length) : 0
  const clip = insert ? clips[insert.clip] : undefined
  const clipFrame =
    clip && insert && shown
      ? clip.frames[
          Math.min(
            clip.frames.length - 1,
            Math.floor(Math.max(0, local - insert.from * shown.slot.length) * clip.fps),
          )
        ]
      : undefined

  return (
    <div className="flex h-dvh w-dvw items-center justify-center bg-black">
      <div
        ref={stageRef}
        className="stage relative overflow-hidden"
        style={{ "--px": `${px}px` } as React.CSSProperties}
      >
        <canvas ref={canvasRef} className="block h-full w-full" aria-hidden="true" />

        {insert && (
          <div
            className="gba-window absolute top-1/2 right-[calc(16*var(--px))] -translate-y-1/2 bg-black"
            style={
              {
                opacity: insertAlpha,
                // The clip at 2.5 GBA pixels per pixel: 600 by 400 at 1080p, clear of the caption.
                "--px": `${px * 0.625}px`,
                width: "calc(256 * var(--px))",
                height: "calc(176 * var(--px))",
              } as React.CSSProperties
            }
          >
            {clipFrame ? (
              <img
                src={`/clips/${insert.clip}/${clipFrame}`}
                alt=""
                className="h-full w-full"
                style={{ imageRendering: "pixelated" }}
              />
            ) : (
              <div className="flex h-full w-full flex-col items-center justify-center gap-[calc(8*var(--px))] bg-black">
                <span className="gba-text gba-text-light">CLIP</span>
                <span className="gba-text gba-text-light">{insert.clip}</span>
              </div>
            )}
          </div>
        )}

        {caption && (
          <div
            className="absolute bottom-[calc(8*var(--px))] left-1/2 -translate-x-1/2"
            style={{ width: "calc(240 * var(--px))" }}
          >
            <div className="gba-window relative bg-paper">
              <p
                className="gba-text whitespace-pre-line"
                style={{ minHeight: "calc(32 * var(--px))" }}
              >
                {captionText.slice(0, printed)}
                <span className="invisible">{captionText.slice(printed)}</span>
              </p>
              {printed >= captionText.length && (
                <span
                  aria-hidden="true"
                  className="gba-down-arrow absolute"
                  style={{ right: 0, bottom: "calc(-2 * var(--px))" }}
                />
              )}
            </div>
          </div>
        )}

        {title && titleAlpha > 0 && (
          <div
            className="absolute inset-0 flex flex-col items-center justify-center gap-[calc(12*var(--px))] bg-black/55"
            style={{ opacity: titleAlpha }}
          >
            {title.logo && (
              <div className="flex flex-col items-center gap-[calc(4*var(--px))]">
                <PixelImage src="/game/title/logo.png" alt="Pokémon" />
                <PixelImage src="/game/title/wordmark.png" alt="Wayfarer" />
              </div>
            )}
            {title.lines.map((line) => (
              <p key={line} className="gba-text gba-text-light">
                {line}
              </p>
            ))}
          </div>
        )}

        <div className="pointer-events-none absolute inset-0 bg-black" style={{ opacity: black }} />

        {failed && (
          <p className="absolute inset-x-0 top-1/2 text-center font-sans text-paper">{failed}</p>
        )}
      </div>

      {/* The dashboard, outside the frame: where we are, for the keyboard (space, arrows, Home). */}
      {shown && !capture && (
        <div
          className="pointer-events-none fixed right-3 bottom-2 font-mono text-xs text-white/60"
          data-dashboard
        >
          {shown.slot.id} · {shown.seconds.toFixed(1)}s / {duration.toFixed(1)}s ·{" "}
          {clockText(shown.minute)}
        </div>
      )}
    </div>
  )
}

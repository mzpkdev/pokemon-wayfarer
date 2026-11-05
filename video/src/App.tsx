import { useEffect, useMemo, useRef, useState } from "react"

import { clockText } from "~/world-map/living"

import {
  BOX_BOTTOM,
  BOX_FONT,
  BOX_TEXT_HEIGHT,
  boxMeasure,
  pageAt,
  pagesFor,
  printedAt,
} from "./box"
import { directionFor, drawFrame, loadStage, riderPlace, type Stage } from "./director"
import { type Cues, loadCues, type Slot, schedule, slotAt, totalSeconds } from "./schedule"
import { Scout } from "./Scout"
import { Studio } from "./Studio"
import type { Span } from "./timeline"
import { PixelImage, Popup, useStagePx } from "./ui"

/** Seconds a battle picture takes to slide in. */
const PORTRAIT_SECONDS = 0.3

/** Seconds a card takes to fade in or out, and the video to fade from and to black. */
const FADE_SECONDS = 0.5

/** How much of a span is showing at `seconds` into the scene, fading at both ends. */
const alphaIn = (span: Span<number>, seconds: number, length: number) => {
  const from = span.from * length
  const to = span.to * length
  if (seconds < from || seconds > to) return 0
  return Math.min(1, (seconds - from) / FADE_SECONDS, (to - seconds) / FADE_SECONDS)
}

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

type Shown = {
  slot: Slot
  fraction: number
  seconds: number
  minute: number
  popup: string | null
}

const params = new URLSearchParams(location.search)
/** Whether `scripts/frames.mjs` is driving the page: nothing but the picture then. */
const capture = params.has("capture")

/** The timeline playing: the frame, the text box, the title card, the fades and the keyboard. */
const Player = ({
  stage,
  cues,
  onStudio,
}: {
  stage: Stage
  cues: Cues | null
  onStudio: () => void
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const stageRef = useRef<HTMLDivElement>(null)
  const px = useStagePx(stageRef)
  const [shown, setShown] = useState<Shown>()

  // Each scene's narration, paged for the text box and printed as its words are said.
  const pages = useMemo(() => {
    const measure = boxMeasure()
    return new Map(
      schedule(cues).map((slot) => [
        slot.id,
        slot.boxed === false ? [] : pagesFor(slot.words.text, slot.times, measure),
      ]),
    )
  }, [cues])

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    if (!canvas || !context) return
    const slots = schedule(cues)
    const duration = totalSeconds(slots)
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
    globalThis.wayfarerVideo = {
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

    let raf = 0
    const draw = () => {
      raf = requestAnimationFrame(draw)
      const seconds = time()
      const found = slotAt(slots, seconds)
      if (!found) return
      const { slot, fraction } = found
      const drawn = drawFrame(stage, context, slot, fraction)
      pending = drawn.pending
      setShown({
        slot,
        fraction,
        seconds,
        minute: stage.clock.minute,
        popup: riderPlace(stage, drawn.shot.rider),
      })
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
  const duration = totalSeconds(schedule(cues))
  const black = shown
    ? Math.max(0, 1 - Math.min(shown.seconds, duration - shown.seconds) / (FADE_SECONDS * 1.6))
    : 1

  const page = shown && pageAt(pages.get(shown.slot.id) ?? [], local)
  const pageText = page?.text ?? ""
  const printed = page ? printedAt(page, local) : 0

  const title = direction?.title
  const titleAlpha = title && shown ? alphaIn(title, local, shown.slot.length) : 0

  const length = shown?.slot.length ?? 1
  const phone = direction?.phone
  const ringing = phone && local >= phone.from * length && local <= phone.to * length
  // The handset shakes left and right eight times a second, as the site's does.
  const shake = [0, -8, 0, 8][Math.floor(local / 0.125) % 4]!
  const portraits = direction?.portraits
  const facing = portraits && local >= portraits.from * length

  return (
    <>
      <div
        ref={stageRef}
        className="stage relative overflow-hidden"
        style={{ "--px": `${px}px` } as React.CSSProperties}
      >
        <canvas ref={canvasRef} className="block h-full w-full" aria-hidden="true" />

        <Popup label={shown?.popup ?? null} />

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

        {facing && (
          <div className="absolute inset-0 bg-black/50">
            {portraits.pictures.map((picture) => {
              const since = local - picture.at * length
              if (since < 0) return null
              // In from its side of the frame, fast then settling, as a battle slides its pictures in.
              const off = (1 - Math.min(1, since / PORTRAIT_SECONDS)) ** 2 * 200
              const sign = picture.side === "left" ? -1 : 1
              return (
                <div
                  key={picture.src}
                  className="absolute top-[calc(36*var(--px))]"
                  style={{
                    left: `calc(${picture.side === "left" ? 104 : 248} * var(--px))`,
                    translate: `calc(${Math.round(sign * off)} * var(--px)) 0`,
                  }}
                >
                  {/* Twice the frame's pixel: a 64-pixel picture stands half the frame tall. */}
                  <div style={{ ["--px" as string]: `${px * 2}px` }}>
                    <PixelImage src={picture.src} alt="" />
                  </div>
                </div>
              )
            })}
          </div>
        )}

        {ringing && (
          <div className="absolute top-[calc(20*var(--px))] left-1/2 flex -translate-x-1/2 items-center gap-[calc(8*var(--px))]">
            <svg
              viewBox="0 0 12 16"
              shapeRendering="crispEdges"
              aria-hidden="true"
              style={{
                width: "calc(24 * var(--px))",
                height: "calc(32 * var(--px))",
                rotate: `${shake}deg`,
              }}
            >
              <rect x="1" y="0" width="10" height="16" fill="rgb(56 56 56)" />
              <rect x="2" y="1" width="8" height="14" fill="rgb(48 80 200)" />
              <rect x="2" y="1" width="8" height="1" fill="rgb(120 152 248)" />
              <rect x="3" y="2" width="6" height="5" fill="rgb(56 56 56)" />
              <rect x="4" y="3" width="4" height="3" fill="rgb(168 224 160)" />
              {[9, 11, 13].flatMap((y) =>
                [3, 5, 7].map((x) => (
                  <rect
                    key={`${x}-${y}`}
                    x={x}
                    y={y}
                    width="2"
                    height="1"
                    fill="rgb(248 248 248)"
                  />
                )),
              )}
            </svg>
            {local >= phone.named * length && (
              <div className="gba-window bg-paper">
                <p className="gba-text whitespace-nowrap">{phone.caller}</p>
              </div>
            )}
          </div>
        )}

        {page && (
          <div
            className="absolute left-1/2 -translate-x-1/2"
            style={{ bottom: `calc(${BOX_BOTTOM} * var(--px))`, width: "calc(240 * var(--px))" }}
          >
            <div className="gba-window relative bg-paper">
              <p
                className="gba-text whitespace-pre-line"
                style={{ minHeight: `calc(${BOX_TEXT_HEIGHT} * var(--px))` }}
              >
                {pageText.slice(0, printed)}
                <span className="invisible">{pageText.slice(printed)}</span>
              </p>
              {printed >= pageText.length && (
                <span
                  aria-hidden="true"
                  className="gba-down-arrow absolute"
                  style={{ right: 0, bottom: "calc(-2 * var(--px))" }}
                />
              )}
            </div>
          </div>
        )}

        <div className="pointer-events-none absolute inset-0 bg-black" style={{ opacity: black }} />
      </div>

      {/* The dashboard, outside the frame: where we are, for the keyboard (space, arrows, Home). */}
      {shown && !capture && (
        <div className="pointer-events-none fixed right-3 bottom-2 flex items-center gap-3 font-mono text-xs text-white/60">
          <span>
            {shown.slot.id} · {shown.seconds.toFixed(1)}s / {duration.toFixed(1)}s ·{" "}
            {clockText(shown.minute)}
          </span>
          <button
            type="button"
            className="pointer-events-auto rounded bg-white/10 px-2 py-0.5 text-white/80 hover:bg-white/20"
            onClick={onStudio}
          >
            Studio
          </button>
        </div>
      )}
    </>
  )
}

export const App = () => {
  const [stage, setStage] = useState<Stage>()
  const [cues, setCues] = useState<Cues | null>()
  const [failed, setFailed] = useState<string>()
  const [studio, setStudio] = useState(params.has("studio") && !capture)

  useEffect(() => {
    // The text box's font first: the narration is wrapped by measuring it.
    Promise.all([loadStage(), document.fonts.load(BOX_FONT)]).then(
      ([loaded]) => setStage(loaded),
      (error: unknown) => setFailed(String(error)),
    )
    void loadCues().then(setCues)
  }, [])

  return (
    <div
      className={`flex h-dvh w-dvw items-center justify-center bg-black ${capture ? "capture" : ""}`}
      style={{ "--aside": studio ? "20rem" : "0px" } as React.CSSProperties}
    >
      {failed && <p className="font-sans text-paper">{failed}</p>}
      {stage && params.has("scout") && <Scout stage={stage} params={params} />}
      {stage && cues !== undefined && !studio && !params.has("scout") && (
        <Player stage={stage} cues={cues} onStudio={() => setStudio(true)} />
      )}
      {stage && studio && <Studio stage={stage} onPlay={() => setStudio(false)} />}
    </div>
  )
}

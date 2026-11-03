import { useEffect, useRef, useState } from "react"

import type { LeagueId } from "./landmarks"
import type { Seat } from "./lineup"
import { BUTTON } from "./ui"

type Caller = Exclude<LeagueId, "masters">

/**
 * The league's front desk, as the game words it (`league_phone.c`): the
 * name on the call's name box (`sIndigoCaller`), and the league and its place
 * in the invitation (`BufferLeagueNames`).
 */
const DESKS: Record<Caller, { caller: string; place: string }> = {
  indigo: { caller: "INDIGO LEAGUE", place: "INDIGO PLATEAU" },
  hoenn: { caller: "HOENN LEAGUE", place: "EVER GRANDE CITY" },
}

/** How the call goes, in the order the game plays it (`data/scripts/league_phone.inc`). */
type Step = "ring" | "invitation" | "prompt" | "confirm" | "accepted" | "declined"

/**
 * The call's lines, from `LeaguePhone_Text_*`. `\p` waits for the player and
 * clears the box; the game's line breaks are dropped, since the window here
 * is whatever width the screen has. `{PLAYER}` is the player's name.
 */
const lines = (step: Step, desk: (typeof DESKS)[Caller], player: string, lineup: Seat[]) => {
  const names = lineup.map(({ trainer }) => trainer.name.toUpperCase())
  const champion = names[names.length - 1]
  switch (step) {
    case "invitation":
      return `Hello, ${player}! This is the front desk at ${desk.place}.\\pThe ${desk.caller} is holding its next event, and you're invited!\\pFive top TRAINERS will battle. Will you take part?`
    case "prompt":
      return `Accept the ${desk.caller}'s invitation?`
    case "confirm":
      return "Decline? The event will go ahead without you."
    // "A, B,\nC, D,\pand last, E." is the lineup the game buffers.
    case "accepted":
      return `Wonderful! Your opponents, in battle order:\\p${names[0]}, ${names[1]},\n${names[2]}, ${names[3]},\\pand last, ${champion}.\\pThe event will wait for you at ${desk.place}. Good luck!`
    case "declined":
      return `Understood. The event will go ahead without you.\\p…And it's over! ${champion} won, and reigns as CHAMPION now.\\pWe'll call with the next invitation. Take care!`
    default:
      return ""
  }
}

/** Frames between letters at the game's MID speed is 4; the site's own boxes print at 2. */
const LETTER_MS = 30
/** How long the phone rings before it picks up by itself. */
const RING_MS = 2600

/** The handset, in pixels: a body, a screen and a keypad. */
const PhoneIcon = ({ ringing }: { ringing: boolean }) => (
  <svg
    viewBox="0 0 12 16"
    shapeRendering="crispEdges"
    aria-hidden="true"
    className="h-16 w-12 [image-rendering:pixelated]"
    style={ringing ? { animation: "call-shake 0.5s steps(1) infinite" } : undefined}
  >
    <rect x="1" y="0" width="10" height="16" fill="rgb(56 56 56)" />
    <rect x="2" y="1" width="8" height="14" fill="rgb(48 80 200)" />
    <rect x="2" y="1" width="8" height="1" fill="rgb(120 152 248)" />
    <rect x="3" y="2" width="6" height="5" fill="rgb(56 56 56)" />
    <rect x="4" y="3" width="4" height="3" fill="rgb(168 224 160)" />
    {[9, 11, 13].flatMap((y) =>
      [3, 5, 7].map((x) => (
        <rect key={`${x}-${y}`} x={x} y={y} width="2" height="1" fill="rgb(248 248 248)" />
      )),
    )}
  </svg>
)

/**
 * A call from a league's front desk, the way the game rings it once a player
 * is known there: the phone rings, then the call's window prints the
 * invitation, asks for an answer and plays the accept or decline lines. It is
 * a modal dialog, so Escape closes it. `onClose` says whether the invitation
 * was accepted, so the tour can go to that league.
 */
export const LeagueCall = ({
  league,
  player,
  lineup,
  reducedMotion,
  onClose,
}: {
  league: Caller
  player: string
  lineup: Seat[]
  reducedMotion: boolean
  onClose: (accepted: boolean) => void
}) => {
  const desk = DESKS[league]
  const dialogRef = useRef<HTMLDialogElement>(null)
  const [step, setStep] = useState<Step>("ring")
  const [page, setPage] = useState(0)
  const [printed, setPrinted] = useState(0)
  const answered = useRef(false)

  useEffect(() => {
    dialogRef.current?.showModal()
  }, [])

  // It picks up by itself after a few rings.
  useEffect(() => {
    if (step !== "ring") return
    const timer = window.setTimeout(() => setStep("invitation"), reducedMotion ? 1200 : RING_MS)
    return () => window.clearTimeout(timer)
  }, [step, reducedMotion])

  const pages = lines(step, desk, player, lineup).split("\\p")
  const text = pages[page] ?? ""
  const typing = step !== "ring" && !reducedMotion
  useEffect(() => {
    if (!typing) return
    const timer = window.setInterval(() => setPrinted((count) => count + 1), LETTER_MS)
    return () => window.clearInterval(timer)
  }, [typing])
  const shown = typing ? Math.min(printed, text.length) : text.length
  const printedAll = shown >= text.length
  const last = page >= pages.length - 1

  const go = (next: Step) => {
    setStep(next)
    setPage(0)
    setPrinted(0)
  }
  // A press prints the page out, then moves on to the next one.
  const advance = () => {
    if (!printedAll) setPrinted(text.length)
    else if (!last) {
      setPage(page + 1)
      setPrinted(0)
    } else if (step === "invitation") go("prompt")
  }

  const done = (accepted: boolean) => {
    answered.current = true
    dialogRef.current?.close()
    onClose(accepted)
  }

  // The answer buttons wait for the line to be read; the focus goes to YES.
  const asking = (step === "prompt" || step === "confirm") && printedAll
  const yes = useRef<HTMLButtonElement>(null)
  const advanceRef = useRef<HTMLButtonElement>(null)
  // Each new line of the call takes the focus, so Enter reads on.
  useEffect(() => {
    if (step !== "ring") advanceRef.current?.focus()
  }, [step])
  useEffect(() => {
    if (asking) yes.current?.focus()
  }, [asking])
  const finished = (step === "accepted" || step === "declined") && last && printedAll

  return (
    <dialog
      ref={dialogRef}
      aria-label={`Call from the ${desk.caller}`}
      onClose={() => {
        if (!answered.current) onClose(false)
      }}
      className="m-0 hidden size-full max-h-none max-w-none items-end justify-center bg-transparent p-3 text-ink backdrop:bg-black/55 open:flex sm:p-6"
    >
      <style>{`
        @keyframes call-shake { 0%,100% { transform: rotate(0) } 25% { transform: rotate(-8deg) } 75% { transform: rotate(8deg) } }
        @keyframes call-wave { 0% { opacity: 0.9; transform: scale(0.6) } 100% { opacity: 0; transform: scale(1.6) } }
        @media (prefers-reduced-motion: reduce) { .call-wave, .call-shake { animation: none !important } }
      `}</style>
      <div className="relative w-full max-w-[640px]">
        {/* The screen reader hears who calls, and each line as it is shown. */}
        <p className="sr-only" aria-live="assertive">
          {step === "ring" ? `Incoming call from the ${desk.caller}.` : text}
        </p>
        {step === "ring" ? (
          <div className="gba-window flex items-center gap-5">
            <div className="relative grid size-20 shrink-0 place-items-center">
              {!reducedMotion &&
                [0, 1].map((i) => (
                  <span
                    key={i}
                    aria-hidden="true"
                    className="call-wave absolute inset-1 rounded-full border-[3px] border-[rgb(48_80_200)]"
                    style={{ animation: `call-wave 1.1s ${i * 0.55}s ease-out infinite` }}
                  />
                ))}
              <PhoneIcon ringing={!reducedMotion} />
            </div>
            <div className="min-w-0 flex-1">
              <p className="gba-text">{desk.caller}</p>
              <p className="gba-text gba-text-blue">is calling…</p>
            </div>
            <button
              type="button"
              autoFocus
              onClick={() => setStep("invitation")}
              className={`${BUTTON} shrink-0 px-4`}
            >
              ANSWER
            </button>
          </div>
        ) : (
          <>
            <div className="gba-window relative">
              {/* The name box on the window's edge, as the game puts the speaker's. */}
              <p className="gba-window absolute bottom-[calc(100%+4*var(--px))] left-[calc(-8*var(--px))] z-10 px-3 py-0">
                <span className="gba-text">{desk.caller}</span>
              </p>
              <button
                ref={advanceRef}
                type="button"
                onClick={advance}
                tabIndex={asking || finished ? -1 : 0}
                aria-label="Next line"
                className="absolute inset-0 cursor-pointer focus-visible:outline-2 focus-visible:outline-[rgb(48_80_200)]"
              />
              <p className="gba-text pointer-events-none relative whitespace-pre-line">
                <span aria-hidden="true">
                  {text.slice(0, shown)}
                  <span className="invisible">{text.slice(shown)}</span>
                </span>
              </p>
              <span
                aria-hidden="true"
                className={`gba-down-arrow absolute right-2 bottom-2 ${printedAll && !asking && !finished ? "" : "invisible"}`}
              />
              <div
                className={`relative mt-3 flex min-h-12 flex-wrap items-center justify-end gap-3 ${asking || finished ? "" : "invisible"}`}
              >
                {asking && (
                  <>
                    <button
                      type="button"
                      onClick={() => (step === "prompt" ? go("accepted") : go("declined"))}
                      ref={yes}
                      className={`${BUTTON} px-6`}
                    >
                      YES
                    </button>
                    <button
                      type="button"
                      onClick={() => go(step === "prompt" ? "confirm" : "prompt")}
                      className={`${BUTTON} px-6`}
                    >
                      NO
                    </button>
                  </>
                )}
                {finished && (
                  <button
                    type="button"
                    autoFocus
                    onClick={() => done(step === "accepted")}
                    className={BUTTON}
                  >
                    {step === "accepted" ? "TO THE LEAGUE ▶" : "OK"}
                  </button>
                )}
              </div>
            </div>
          </>
        )}
        <button
          type="button"
          onClick={() => dialogRef.current?.close()}
          aria-label="Hang up"
          className="pixel-label absolute -top-9 right-0 px-2 text-[16px] text-paper/80 hover:text-paper"
        >
          ✕ HANG UP
        </button>
      </div>
    </dialog>
  )
}

// The league's five, picked the way the website's tour shows it: five seats
// in battle order, the whole roster shuffling past in each like a slot
// machine's reels, and each seat stopping on its trainer in turn. The video
// then lets one of them step forward: the rest fall away, their picture grows
// to a battle picture's size, and a partner slides in beside them.
import type { Roster } from "~/world-map/lineup"

import type { Direction } from "./timeline"

/** Seconds a reel shows each face, as on the website. */
const TICK_SECONDS = 0.09
/** Seconds a seat stands a little larger once it stops. */
const POP_SECONDS = 0.2
/** Seconds the star takes to step forward, and the partner to slide in. */
const GROW_SECONDS = 0.35
const SLIDE_SECONDS = 0.3

/** The row of seats, in GBA pixels: 64-pixel portraits, 8 apart, centered in the 480-pixel frame. */
const SEAT = 64
const GAP = 8
const ROW_TOP = 60
const rowLeft = (seats: number) => (480 - seats * SEAT - (seats - 1) * GAP) / 2
/** Where the star ends up, and the partner beside them: 128-pixel pictures, as a battle shows who you face. */
const STAR = { left: 104, top: 36, size: 128 }
const PARTNER_LEFT = 248

const portrait = (pic: string) => `/world-map/leaders/${pic}.png`
const px = (value: number) => `calc(${value} * var(--px))`

export const Lineup = ({
  lineup,
  roster,
  seconds,
  length,
}: {
  lineup: NonNullable<Direction<number>["lineup"]>
  roster: Roster
  /** Seconds into the scene, and the scene's length: the lineup's moments are fractions of it. */
  seconds: number
  length: number
}) => {
  const at = (fraction: number) => fraction * length
  const since = seconds - at(lineup.from)
  // The reels' faces, in an order of their own, as the website shuffles them.
  const pool = roster.trainers.toSorted((a, b) => ((a.id * 37) % 41) - ((b.id * 37) % 41))
  const tick = Math.floor(Math.max(0, since) / TICK_SECONDS)
  const grown = Math.min(1, Math.max(0, (seconds - at(lineup.grow)) / GROW_SECONDS))
  // Leaves gently, arrives gently.
  const eased = grown * grown * (3 - 2 * grown)
  const left = rowLeft(lineup.seats.length)
  const partnerSince = seconds - at(lineup.partner.at)
  const partnerOff = (1 - Math.min(1, partnerSince / SLIDE_SECONDS)) ** 2 * 200

  return (
    // Mounted for the whole scene, so every face is loaded before the reels turn.
    <div className="absolute inset-0 bg-black/50" style={{ opacity: since >= 0 ? 1 : 0 }}>
      {lineup.seats.map((seat, i) => {
        const trainer = roster.trainers.find((t) => t.slug === seat.slug)
        if (!trainer) return null
        const settled = seconds - at(seat.at)
        const here = settled >= 0
        const star = seat.slug === lineup.star
        const shown = here ? trainer : pool[(tick * (3 + i) + i * 7) % pool.length]!
        const pop = here && settled < POP_SECONDS && !eased ? 1.08 : 1
        const mix = (from: number, to: number) => (star ? from + (to - from) * eased : from)
        const size = mix(SEAT, STAR.size)
        const last = i === lineup.seats.length - 1
        return (
          <div key={seat.slug} style={{ opacity: star ? 1 : 1 - eased }}>
            <div
              className="absolute overflow-hidden bg-[rgb(24_40_88)]"
              style={{
                left: px(mix(left + i * (SEAT + GAP), STAR.left)),
                top: px(mix(ROW_TOP, STAR.top)),
                width: px(size),
                height: px(size),
                scale: pop,
                // The seat's frame goes as the star steps out of it.
                background: star && eased ? `rgb(24 40 88 / ${1 - eased})` : undefined,
                boxShadow: `0 0 0 ${px(1)} ${
                  last || (star && here)
                    ? `rgb(255 214 0 / ${star ? 1 - eased : 1})`
                    : "rgb(255 255 255 / 0.25)"
                }`,
                zIndex: star ? 1 : 0,
              }}
            >
              {/* Every face of the reel stacked, one showing: none is loaded mid-spin. */}
              {pool.map((face) => (
                <img
                  key={face.slug}
                  src={portrait(face.pic)}
                  alt=""
                  className="absolute inset-0 h-full w-full [image-rendering:pixelated]"
                  style={{
                    visibility: face.slug === shown.slug ? "visible" : "hidden",
                    opacity: here ? 1 : 0.6,
                    filter: here ? undefined : "brightness(0.75)",
                    translate: here ? undefined : `0 ${px(tick % 2 ? -1 : 1)}`,
                  }}
                />
              ))}
              <span
                className={`gba-text absolute top-0 left-0 px-[calc(2*var(--px))] ${last ? "bg-sign" : "gba-text-light bg-black/60"}`}
                style={{ opacity: 1 - eased }}
              >
                {i + 1}
              </span>
            </div>
            <p
              className="gba-text gba-text-light absolute text-center whitespace-nowrap"
              style={{
                left: px(left + i * (SEAT + GAP) - GAP / 2),
                top: px(ROW_TOP + SEAT + 4),
                width: px(SEAT + GAP),
                opacity: here ? 1 - eased : 0,
              }}
            >
              {trainer.name.toUpperCase()}
            </p>
          </div>
        )
      })}
      {partnerSince >= 0 && (
        <img
          src={lineup.partner.src}
          alt=""
          className="absolute [image-rendering:pixelated]"
          style={{
            left: px(PARTNER_LEFT),
            top: px(STAR.top),
            width: px(STAR.size),
            height: px(STAR.size),
            translate: `${px(Math.round(partnerOff))} 0`,
          }}
        />
      )}
    </div>
  )
}

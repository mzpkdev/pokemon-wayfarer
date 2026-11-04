import { type ReactNode, useEffect, useMemo, useRef, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"

import { type Camera, fit, fly } from "./camera"
import { cloudsOverlay } from "./clouds"
import { type Bounds, DATA, type Meta, loadWorld } from "./data"
import { lightsOverlay } from "./lights"
import { stepped } from "./fade"
import {
  BADGES_SRC,
  badgeOverlays,
  callerFor,
  callingFor,
  type Gym,
  landmarkBounds,
  type Landmarks,
  type League,
  type LeagueId,
  loadLandmarks,
  QUALIFYING_RATING,
  type StopClock,
  trainerRating,
} from "./landmarks"
import { LeagueCall } from "./LeagueCall"
import { LineupPanel, preloadPortraits } from "./LineupPanel"
import { eventSequence, type Roster } from "./lineup"
import { OPEN_SEA, type Overlay, rendererFor } from "./render"
import { riderAt } from "./rider"
import {
  DEFAULT_START,
  dailyFocusIn,
  focusIn,
  openingIn,
  placeAt,
  rideMs,
  routeIn,
  type Start,
  type StartId,
  STARTS,
  startOf,
} from "./starts"
import { pageMinutes } from "./clock"
import { LivingClock } from "./LivingClock"
import { type LivingWorld, livingOverlay, loadLiving } from "./living"
import { type Notables, loadNotables, notableAt, notablesOverlay } from "./notables"
import { PlaceExtra } from "./PlaceExtra"
import type { PlaceInfo } from "./PlaceCard"
import { seaOverlay } from "./sea"
import { loadWater } from "./water"
import { weatherOverlay } from "./weather"
import { DAILY_SEED, type Daily, type DailyDay, dailyOn, loadDaily } from "./daily"
import { type DailyScene, dailyOverlay } from "./dailyView"
import { BUTTON } from "./ui"
import { loadWild, type Wild, type WildClock, wildOverlay } from "./wild"
import { WildTally } from "./WildTally"
import { WorldMap } from "./WorldMap"

/** The real clock the sea, the weather, the clouds and the lights run on. */
const tick = () => performance.now()

/** The explorer's place cards also name a town's Gym, league stop and ferries. */
const placeExtra = (place: PlaceInfo) => <PlaceExtra place={place} />

/**
 * The tour's stops, in the order its cards come: one journey that only pulls
 * out. Home, a busy route of its region changing from day to day, the
 * start's region and its Gyms,
 * the whole world and who lives where, the league circuit, then the map to
 * explore.
 */
export type StopName = "town" | "daily" | "region" | "world" | "circuit" | "explore"
const STOPS: StopName[] = ["town", "daily", "region", "world", "circuit", "explore"]
const stopIndex = (name: StopName) => STOPS.indexOf(name)
/** The regions in the order the page names them. */
const REGION_ORDER = ["Kanto", "Johto", "Hoenn"]
/** The stops the Gym pins are up at: the start's region, and the whole circuit. */
const pinsAt = (stop: number) => STOPS[stop] === "region" || STOPS[stop] === "circuit"
/** The leagues, in the order the circuit stop lists them. */
const LEAGUE_ORDER: LeagueId[] = ["indigo", "hoenn", "masters"]
/** Each stop's mark in the journey strip: an item from the game, or the start's own. */
const MARKS: Record<StopName, string> = {
  town: "Home",
  daily: "Routes change daily",
  region: "Your region",
  world: "The world",
  circuit: "The league circuit",
  explore: "Explore the map",
}
/** What it takes for a league to call, as the league stops say it. */
const REASONS: Record<LeagueId, string> = {
  indigo: "8 badges and one from Kanto or Johto",
  hoenn: "8 badges and one from Hoenn",
  masters: "after winning Indigo and Hoenn",
}
export type Caption = {
  place: string
  title: string
  body: string
  stats?: { value: string; label: string }[]
  /** What the card says instead for each start, in any of the fields above. */
  variants?: Partial<Record<StartId, Partial<Omit<Caption, "variants">>>>
}

/**
 * Something drawn over the map while the tour is at `stop`: a hook for what
 * the page wants to show on a stop that the tour itself doesn't. `animated`
 * keeps frames coming while it is up.
 */
export type StopOverlay = { stop: StopName; overlay: Overlay; animated?: boolean }

/** How long a flight between stops takes: longer the more the zoom changes. */
const flightMs = (from: Camera, to: Camera) =>
  Math.min(2400, 900 + 450 * Math.abs(Math.log2(to.zoom / from.zoom)))
/** The closest zoom the rider is drawn at; further out he is a speck. */
const RIDER_MIN_ZOOM = 4

/** How long the daily stop shows each day before stepping on by itself. */
const DAY_MS = 3200

/** A beat between the tour starting and the rider setting off. */
const RIDE_DELAY_MS = 250
/** How long the camera takes to pan onto a Gym's leader, in ms. */
const FIND_MS = 900
/** How long a gesture hint stays up. */
const HINT_MS = 1600

/** A warp between starts: the game's 8-step fade to black, the cut, and the fade back in. */
const FADE_OUT_MS = 300
const FADE_IN_MS = 400

/** The Gym stop's pins come in a moment after the camera, and go out quickly. */
const BADGES_IN_DELAY_MS = 450
const BADGES_IN_MS = 400
const BADGES_OUT_MS = 250
/** How long the trail and plates take to come in or go out between the Gym stop and the league stops. */
const CIRCUIT_MS = 300

/** A choice of start: the same yellow when picked, a dark plate otherwise. */
const OPTION =
  "pixel-corners flex w-full items-baseline justify-between gap-3 px-4 pt-2 pb-3 text-left font-[family-name:var(--font-gba)] text-[16px] leading-[16px] sm:text-[24px] sm:leading-[24px] has-focus-visible:shadow-[inset_0_0_0_4px_rgb(48_80_200)]"

/** Metatiles across and down in the close-up shots, and in a league's: its region around it. */
const CLOSE_UP = { width: 44, height: 30 }
const LEAGUE_VIEW = { width: 180, height: 122 }
/** Metatiles across and down at the daily stop: a stretch of its route. */
const DAILY_VIEW = { width: 32, height: 22 }

const boundsOf = (maps: Meta["maps"]): Bounds => ({
  left: Math.min(...maps.map(({ x }) => x)),
  top: Math.min(...maps.map(({ y }) => y)),
  right: Math.max(...maps.map(({ x, width }) => x + width)),
  bottom: Math.max(...maps.map(({ y, height }) => y + height)),
})

const unionOf = (boxes: Bounds[]): Bounds => ({
  left: Math.min(...boxes.map(({ left }) => left)),
  top: Math.min(...boxes.map(({ top }) => top)),
  right: Math.max(...boxes.map(({ right }) => right)),
  bottom: Math.max(...boxes.map(({ bottom }) => bottom)),
})

/** Where a region (by its label) lies, from its bounds or else the maps its pieces hold. */
const regionBounds = (meta: Meta, name: string): Bounds =>
  meta.regions.find((region) => region.name === name)?.bounds ??
  boundsOf(
    meta.maps.filter(({ region }) => region === (name === "HOENN" ? "Hoenn" : "Johto and Kanto")),
  )

/** Where a league's shot is in `shotsFor`'s list: after the opening and every stop's. */
const leagueShot = (id: LeagueId) => STOPS.length + 1 + LEAGUE_ORDER.indexOf(id)

/**
 * The shots for a map area `width` by `height` CSS pixels, for a start: the
 * opening on the road, then the stops: the town, its daily route, its region
 * (or regions), the whole world, the circuit and the world again to explore;
 * then each league's, which the circuit stop flies to.
 */
const shotsFor = (
  meta: Meta,
  landmarks: Landmarks,
  start: Start,
  width: number,
  height: number,
): Camera[] => {
  const close = Math.min(width / CLOSE_UP.width, height / CLOSE_UP.height)
  const region = unionOf(start.regions.map((name) => regionBounds(meta, name)))
  const regions = ["JOHTO", "KANTO", "HOENN"].map((name) => regionBounds(meta, name))
  const gyms = landmarkBounds({ ...landmarks, circuit: [] }, 36) ?? unionOf(regions)
  // Without a league's place, the whole circuit, with room for the plates above and below.
  const all = landmarkBounds(landmarks, 36)
  const circuit = all ? { ...all, top: all.top - 24, bottom: all.bottom + 36 } : gyms
  const leagueCamera = (id: LeagueId): Camera => {
    const league = landmarks.circuit.find((entry) => entry.id === id)
    if (!league) return fit(circuit, width, height, 0.95)
    const zoom = Math.min(width / LEAGUE_VIEW.width, height / LEAGUE_VIEW.height)
    // On a wide screen the lineup lies over the map's lower part, so the league sits higher.
    return { x: league.x, y: league.y + (width > 700 ? (height * 0.14) / zoom : 0), zoom }
  }
  // The daily route's busiest stretch, close enough to follow who comes and goes.
  const daily = Math.min(width / DAILY_VIEW.width, height / DAILY_VIEW.height)
  const world = fit(boundsOf(meta.maps), width, height, 0.92)
  return [
    { ...fit(openingIn(meta, start), width, height, 1) },
    { ...focusIn(meta, start), zoom: close },
    { ...dailyFocusIn(meta, start), zoom: daily },
    fit(region, width, height, 0.95),
    world,
    fit(circuit, width, height, 0.95),
    world,
    ...LEAGUE_ORDER.map(leagueCamera),
  ]
}

/**
 * A Gym's card at the badge stop: its badge, earned, and town, its leader as
 * their battle shows them with what they are up to on the page's clock, and a
 * button that pans the map onto them (or onto the Gym, for a leader who stays
 * put). Beside the map on wide screens; over the caption on narrow ones, so
 * the little map above stays clear.
 */
const GymCard = ({
  gym,
  base,
  notables,
  minutes,
  count,
  onFind,
  onClose,
}: {
  gym: Gym
  base: string
  notables: Notables | undefined
  minutes: number
  count: number
  onFind: () => void
  onClose: () => void
}) => {
  const { leader } = gym
  const walking = leader && notables ? notableAt(notables, leader.slug, minutes) : undefined
  return (
    <section
      aria-label={`${gym.town} Gym`}
      className="pixel-corners pixel-panel absolute top-[4.75rem] left-4 z-20 w-[min(340px,calc(100%-2rem))] p-5 text-paper max-lg:top-auto max-lg:right-4 max-lg:bottom-4 max-lg:w-auto sm:max-lg:bottom-8 sm:max-lg:inset-x-8"
    >
      <div className="flex items-start justify-between gap-3">
        <p className="pixel-label flex items-center gap-2 text-[16px] leading-none text-logo-yellow">
          <span
            aria-hidden
            className="size-6 shrink-0 [image-rendering:pixelated]"
            style={{
              backgroundImage: `url(${BADGES_SRC})`,
              backgroundSize: `${24 * 24}px 24px`,
              backgroundPosition: `${-gym.badge * 24}px 0`,
            }}
          />
          {gym.name ? `${gym.name} EARNED` : "BADGE EARNED"}
        </p>
        <button
          type="button"
          onClick={onClose}
          aria-label="Close"
          className="pixel-label -mt-2 -mr-1 px-2 text-[16px] text-paper/70 hover:text-paper"
        >
          ✕
        </button>
      </div>
      <p className="pixel-label mt-1 text-[16px] leading-none text-paper/60">
        {gym.region.toUpperCase()} GYM · BADGES {count}/24
      </p>
      <div className="mt-4 flex items-center gap-4">
        {leader?.pic ? (
          // The leader as the battle shows them.
          <img
            src={`${base}/leaders/${leader.pic}.png`}
            alt=""
            width={64}
            height={64}
            className="size-24 shrink-0 [image-rendering:pixelated]"
          />
        ) : (
          <span
            aria-hidden
            className="size-12 shrink-0 [image-rendering:pixelated]"
            style={{
              backgroundImage: `url(${BADGES_SRC})`,
              backgroundSize: `${24 * 48}px 48px`,
              backgroundPosition: `${-gym.badge * 48}px 0`,
            }}
          />
        )}
        <div className="min-w-0">
          <p className="pixel-heading text-[24px]">
            {leader ? leader.name.toUpperCase() : gym.town}
          </p>
          <p className="mt-1 text-sm text-paper/70">{gym.town}</p>
          <p className="mt-2 text-sm text-paper/90">{walking?.label.split(" · ")[1]}</p>
        </div>
      </div>
      <button type="button" onClick={onFind} className={`${BUTTON} mt-5 w-full`}>
        {walking ? `FIND ${leader!.name.toUpperCase()}` : `GO TO ${gym.town}`}
      </button>
    </section>
  )
}

/** Whether a league is calling, as a plate would show it. */
const LeagueState = ({
  league,
  calling,
  reason = true,
}: {
  league: League
  calling: boolean
  reason?: boolean
}) => (
  <div className="flex flex-col gap-0.5">
    <p className="pixel-label flex items-baseline justify-between gap-3 text-[16px] leading-none">
      <span className={calling ? "text-logo-yellow" : "text-paper/70"}>{league.name}</span>
      <span className={calling ? "text-logo-yellow" : "text-paper/50"}>
        {calling ? "CALLING" : league.id === "masters" ? "LOCKED" : "NOT YET"}
      </span>
    </p>
    {reason && <p className="text-[13px] text-paper/60 sm:text-sm">{REASONS[league.id]}</p>}
  </div>
)

/**
 * A league stop's card: whether it is calling and what it takes, then the
 * five that fight there, drawn by the game's rule and shown seat by seat for
 * Indigo and Hoenn; the Sevii Masters' lineup format isn't in the game yet,
 * so it shows its halls and how it opens.
 */
const LeagueHalls = ({
  league,
  calling,
  roster,
  progress,
  base,
  reducedMotion,
}: {
  league: League
  calling: boolean
  roster: Roster | undefined
  progress: number
  base: string
  reducedMotion: boolean
}) => (
  <div className="mt-3">
    <LeagueState league={league} calling={calling} />
    {league.id !== "masters" && league.halls && roster && (
      <LineupPanel
        roster={roster}
        league={league.id}
        halls={league.halls}
        progress={progress}
        base={base}
        reducedMotion={reducedMotion}
        variant="card"
      />
    )}
    {league.id === "masters" && league.halls && (
      <>
        <p className="pixel-label mt-3 text-[16px] leading-none text-paper/70 sm:mt-4">HALLS</p>
        <ol className="mt-2 grid gap-0.5 text-[13px] sm:gap-1 sm:text-sm">
          {league.halls.map((hall) => (
            <li key={hall.name} className="flex justify-between gap-3">
              <span className="text-paper/90">{hall.name}</span>
              <span className="text-right text-paper/60">{hall.condition}</span>
            </li>
          ))}
        </ol>
        <p className="mt-3 text-[13px] text-paper/60 sm:text-sm">
          The Masters' lineup is its own format, and it isn't in the game yet, so there is no draft
          to show here.
        </p>
      </>
    )}
  </div>
)

/**
 * The circuit stop's leagues: each one's state, a tap flying the map on to
 * it; at a league, its five (or the Masters' halls) and the way back out.
 */
const CircuitLeagues = ({
  leagues,
  earned,
  focused,
  onFocus,
  roster,
  progress,
  base,
  reducedMotion,
}: {
  leagues: League[]
  earned: ReadonlySet<number>
  focused: LeagueId | null
  onFocus: (league: LeagueId | null) => void
  roster: Roster | undefined
  progress: number
  base: string
  reducedMotion: boolean
}) => {
  const calling = callingFor(earned)
  const ordered = LEAGUE_ORDER.flatMap((id) => leagues.filter((entry) => entry.id === id))
  const list = (
    <ul className="mt-4 grid gap-2">
      {ordered.map((entry) => (
        <li key={entry.id}>
          <button
            type="button"
            onClick={() => onFocus(entry.id)}
            className="pixel-corners w-full bg-black/30 px-4 pt-2 pb-3 text-left hover:bg-black/50 focus-visible:shadow-[inset_0_0_0_4px_rgb(48_80_200)] focus-visible:outline-none"
          >
            <LeagueState league={entry} calling={calling[entry.id]} />
          </button>
        </li>
      ))}
    </ul>
  )
  const at = (entry: League) => (
    <div className="mt-4">
      <button
        type="button"
        onClick={() => onFocus(null)}
        className="pixel-label text-[16px] leading-none text-paper/70 hover:text-paper focus-visible:text-paper focus-visible:underline focus-visible:outline-none"
      >
        ◀ ALL 24 GYMS
      </button>
      <LeagueHalls
        league={entry}
        calling={calling[entry.id]}
        roster={roster}
        progress={progress}
        base={base}
        // A view not shown keeps its five still: it only holds their room.
        reducedMotion={reducedMotion || entry.id !== focused}
      />
    </div>
  )
  // Every view sits in one cell, the ones not shown invisible (and, below
  // lg, where the card scrolls, left out), so the card keeps the height of
  // its tallest and doesn't jump as a league is picked.
  const views: [LeagueId | null, ReactNode][] = [
    [null, list],
    ...ordered.map((entry): [LeagueId, ReactNode] => [entry.id, at(entry)]),
  ]
  return (
    <div className="grid">
      {views.map(([id, view]) => {
        const shown = id === (focused ?? null)
        return (
          <div
            key={id ?? "all"}
            aria-hidden={!shown}
            inert={!shown}
            className={`col-start-1 row-start-1 ${shown ? "" : "invisible max-lg:hidden"}`}
          >
            {view}
          </div>
        )
      })}
    </div>
  )
}

/** Who is out on a day, as the daily card lists them. */
const trainersOn = (scene: DailyDay) =>
  scene.trainers.map((trainer) => trainer.name).join(", ") || "Nobody about today."
/** What is lying around on a day, as the daily card lists it. */
const itemsOn = (scene: DailyDay) =>
  scene.items.map((item) => (item.hidden ? `${item.item} (hidden)` : item.item)).join(", ") ||
  "Nothing lying around today."
// A daily list over an invisible copy of its longest day's, so stepping the
// days never grows or shrinks the card.
const held = (text: string, room: string) => (
  <dd className="mt-1 grid text-paper/85">
    <span className="col-start-1 row-start-1">{text}</span>
    <span aria-hidden className="invisible col-start-1 row-start-1">
      {room}
    </span>
  </dd>
)
/** How many days ahead the daily card holds room for. */
const HELD_DAYS = 60

/**
 * The daily stop's card body: the day shown, with buttons to step it, and
 * who and what is out on the route that day. Stepping by hand stops it
 * stepping by itself.
 */
const DailyCard = ({
  scene,
  longest,
  day,
  onDay,
}: {
  scene: ReturnType<typeof dailyOn> | null
  /** The longest lists the route has over its first days, to hold the card's height. */
  longest: { trainers: string; items: string }
  day: number
  onDay: (day: number) => void
}) => {
  if (!scene) return null
  const step =
    "pixel-label px-3 pt-1 pb-2 text-paper/70 hover:text-paper disabled:opacity-30 focus-visible:text-paper focus-visible:underline focus-visible:outline-none"
  return (
    <div className="mt-4">
      <div className="pixel-corners flex items-center justify-between bg-black/30 px-2">
        <button
          type="button"
          className={step}
          disabled={day <= 1}
          onClick={() => onDay(day - 1)}
          aria-label="The day before"
        >
          ◀
        </button>
        <p className="pixel-label text-logo-yellow">DAY {day}</p>
        <button
          type="button"
          className={step}
          onClick={() => onDay(day + 1)}
          aria-label="The next day"
        >
          ▶
        </button>
      </div>
      <dl className="mt-3 grid gap-2 text-[13px] leading-snug sm:text-sm">
        <div>
          <dt className="pixel-label text-[16px] leading-none text-paper/60">
            TRAINERS {scene.trainers.length}
            {scene.empty.length ? ` · ${scene.empty.length} SPOTS EMPTY` : ""}
          </dt>
          {held(trainersOn(scene), longest.trainers)}
        </div>
        <div>
          <dt className="pixel-label text-[16px] leading-none text-paper/60">
            ITEMS {scene.items.length}
          </dt>
          {held(itemsOn(scene), longest.items)}
        </div>
      </dl>
    </div>
  )
}

/** An item's icon from the game, for a journey mark. */
const item = (name: string) => (
  <img src={`/game/items/${name}.png`} alt="" width={24} height={24} className="pixelated size-6" />
)

/** The journey so far: a mark per stop, the current one lit; a tap goes there. */
const JourneyStrip = ({
  stop,
  start,
  badge,
  disabled,
  onGo,
}: {
  stop: number
  start: Start
  /** The first badge of the start's region, for its region's mark. */
  badge: number
  disabled: boolean
  onGo: (stop: number) => void
}) => {
  const marks: Record<StopName, ReactNode> = {
    town: (
      <span
        className="pixelated block size-8"
        style={{ backgroundImage: `url(${start.rider})`, backgroundSize: "288px 32px" }}
      />
    ),
    daily: (
      <img
        src="/game/intro/pokeball.png"
        alt=""
        width={32}
        height={32}
        className="pixelated size-8"
      />
    ),
    region: (
      <span
        className="pixelated block size-6"
        style={{
          backgroundImage: `url(${BADGES_SRC})`,
          backgroundSize: `${24 * 24}px 24px`,
          backgroundPosition: `${-badge * 24}px 0`,
        }}
      />
    ),
    world: item("ss-ticket"),
    circuit: item("vs-seeker"),
    explore: item("town-map"),
  }
  return (
    <ol className="mb-3 flex gap-1.5 sm:mb-4" aria-label="The tour">
      {STOPS.map((name, i) => (
        <li key={name}>
          <button
            type="button"
            disabled={disabled}
            onClick={() => onGo(i)}
            aria-label={`${MARKS[name]}, stop ${i + 1} of ${STOPS.length}`}
            aria-current={i === stop ? "step" : undefined}
            className={`pixel-corners grid size-9 place-items-center sm:size-10 ${
              i === stop
                ? "bg-[rgb(232_208_64)]"
                : `bg-black/30 hover:bg-black/50 ${i < stop ? "" : "opacity-50 hover:opacity-100"}`
            } focus-visible:shadow-[inset_0_0_0_3px_rgb(48_80_200)] focus-visible:outline-none`}
          >
            {marks[name]}
          </button>
        </li>
      ))}
    </ol>
  )
}

/**
 * The world in the game's own tiles, held still at one stop at a time with a
 * card beside it. It opens with the player riding in on their bike to their
 * front door, in the town the first card picks (PALLET TOWN, NEW BARK TOWN or
 * LITTLEROOT TOWN; another start plays a warp: the screen fades to black and
 * back in). The card's button then flies the camera on, one journey that only
 * pulls out: the first route, its wild Pokémon in the grass; the region and
 * its Gyms (tap the badges you'd win); the whole world and who lives where;
 * the league circuit (at eight badges a league phones, and a league's plate
 * flies on to it); and the whole map, which the last button opens to look
 * around. A journey strip on the card marks the stops. The page scrolls past
 * it as usual. Fills its section.
 *
 * The ride starts the first time nearly all of the section is on screen, or,
 * given `startOn`, when the section receives that event (or already carries
 * `data-started`), for a page that brings the map in its own way.
 *
 * `captions` are one card per stop, in order: the town, its first route, its
 * region, the world, the circuit and the whole map. `stopOverlays` draw over
 * the map at a stop.
 */
export const WorldTour = ({
  captions,
  base = DATA,
  startOn,
  stopOverlays,
}: {
  captions: Caption[]
  base?: string
  startOn?: string
  stopOverlays?: StopOverlay[]
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const captionRef = useRef<HTMLDivElement>(null)
  const captionsRef = useRef<HTMLDivElement>(null)
  const dialogRef = useRef<HTMLDialogElement>(null)
  const cameraRef = useRef<Camera | null>(null)
  const goRef = useRef<(stop: number) => void>(() => {})
  const focusRef = useRef<(league: LeagueId | null) => void>(() => {})
  const findRef = useRef<(gym: Gym) => void>(() => {})
  const hoverRef = useRef<(badge: number | null) => void>(() => {})
  const toggleRef = useRef<(gym: Gym) => void>(() => {})
  const [gymPins, setGymPins] = useState<Gym[]>([])
  const [leagues, setLeagues] = useState<League[]>([])
  const [roster, setRoster] = useState<Roster>()
  // The badges the visitor has earned at the Gym stop, by tapping their
  // pins. They persist through the league stops and across a change of
  // start: they are about the player, not where they begin.
  const [earned, setEarned] = useState<ReadonlySet<number>>(new Set())
  const earnedRef = useRef(earned)
  // When each was earned, for its sparkle; and which leagues have phoned.
  const earnedAtRef = useRef(new Map<number, number>())
  const calledRef = useRef(new Set<LeagueId>())
  const callTimer = useRef(0)
  const [call, setCall] = useState<Exclude<LeagueId, "masters"> | null>(null)
  const redrawRef = useRef<() => void>(() => {})
  const exploringRef = useRef(false)
  // The button that moves on, which takes the focus when the one pressed goes.
  const nextRef = useRef<HTMLButtonElement>(null)
  useEffect(() => {
    earnedRef.current = earned
    redrawRef.current()
  }, [earned])
  const [hint, setHint] = useState(false)
  // The Gym whose card is open, at the badge stop.
  const [gym, setGym] = useState<Gym | null>(null)
  const gymRef = useRef<Gym | null>(null)
  useEffect(() => {
    gymRef.current = gym
  }, [gym])
  const chooseRef = useRef<(id: StartId) => void>(() => {})
  // The world stop shows the living world: the ferries on their loops and
  // the time of day, on the page's shared clock.
  const [stop, setStop] = useState(0)
  // The stop as of the latest NEXT/BACK, for the map to catch up to if they
  // were pressed before it loaded.
  const stopRef = useRef(0)
  // Whether the map's data is in; until then a line says it's coming.
  const [ready, setReady] = useState(false)
  // The world's meta once it's in, for what the overlays need of it.
  const [meta, setMeta] = useState<Meta>()
  const [living, setLiving] = useState<LivingWorld>()
  const [minutes, setMinutes] = useState(pageMinutes)
  const [notables, setNotables] = useState<Notables>()
  // These load on the way to the stops that show them, not with the page.
  const nearWorld = stop >= stopIndex("world") - 1
  useEffect(() => {
    if (!nearWorld) return
    void loadLiving(base).then(setLiving, () => {})
    void loadNotables(base).then(setNotables, () => {})
  }, [base, nearWorld])
  const notablesRef = useRef<Notables>(undefined)
  useEffect(() => {
    notablesRef.current = notables
  }, [notables])
  const reducedMotion = useReducedMotion()
  // The world stop's waves of icons, on a clock that restarts on arriving.
  const [wild, setWild] = useState<Wild>()
  const nearWild = stop >= stopIndex("world") - 1
  useEffect(() => {
    if (nearWild) void loadWild(base).then(setWild, () => {})
  }, [base, nearWild])
  // The daily route: its trainers and items, one day after another.
  const [daily, setDaily] = useState<Daily>()
  const [trainerSheet, setTrainerSheet] = useState<HTMLImageElement>()
  const sceneRef = useRef<{ scene: DailyScene; since: number } | null>(null)
  useEffect(() => {
    let live = true
    const image = new Image()
    image.src = `${base}/trainers.png`
    void Promise.all([loadDaily(base), image.decode()]).then(
      ([loaded]) => {
        if (!live) return
        setDaily(loaded)
        setTrainerSheet(image)
      },
      () => {},
    )
    return () => {
      live = false
    }
  }, [base])
  const nearGyms = stop >= stopIndex("region") - 1
  useEffect(() => {
    if (nearGyms) preloadPortraits(base, roster)
  }, [base, roster, nearGyms])
  const wildClock = useRef<WildClock>({ start: 0, still: null })
  const stopOverlaysRef = useRef(stopOverlays)
  useEffect(() => {
    // The Gym Leaders and Elite Four go under the time of day's tint.
    stopOverlaysRef.current = [
      ...(daily && trainerSheet
        ? [
            {
              stop: "daily" as const,
              overlay: dailyOverlay(
                { image: trainerSheet, ...daily.sheet },
                () => sceneRef.current,
                !reducedMotion,
                performance.now.bind(performance),
              ),
              animated: !reducedMotion,
            },
          ]
        : []),
      ...(notables
        ? [
            {
              stop: "explore" as const,
              overlay: notablesOverlay(notables, pageMinutes, { standing: reducedMotion }),
              animated: !reducedMotion,
            },
          ]
        : []),
      ...(living
        ? (["world", "explore"] as const).map((at) => ({
            stop: at,
            overlay: livingOverlay(living, pageMinutes, { still: reducedMotion }),
            animated: !reducedMotion,
          }))
        : []),
      // After dark the towns glow and the lighthouse sweeps, over the tint.
      ...(living && meta
        ? (["world", "explore"] as const).map((at) => ({
            stop: at,
            overlay: lightsOverlay(meta, living, pageMinutes, !reducedMotion, tick),
            animated: !reducedMotion,
          }))
        : []),
      ...(wild
        ? [
            {
              stop: "world" as const,
              overlay: wildOverlay(
                wild,
                wildClock.current,
                !reducedMotion,
                performance.now.bind(performance),
              ),
              animated: !reducedMotion,
            },
          ]
        : []),
      ...(stopOverlays ?? []),
    ]
    redrawRef.current()
  }, [stopOverlays, living, notables, wild, daily, trainerSheet, reducedMotion, meta])
  const atWorld = STOPS[stop] === "world" || STOPS[stop] === "explore"
  const ticking = atWorld || gym !== null
  useEffect(() => {
    if (!ticking) return
    // With reduced motion the map redraws on the clock's tick, not every frame.
    const timer = window.setInterval(() => {
      setMinutes(pageMinutes())
      if (reducedMotion) redrawRef.current()
    }, 500)
    return () => window.clearInterval(timer)
  }, [ticking, reducedMotion])
  const [chosen, setChosen] = useState<StartId>(DEFAULT_START)
  // The day the daily stop shows. It steps on by itself while the stop is up,
  // until a visitor steps it by hand.
  const [day, setDay] = useState(1)
  const [stepping, setStepping] = useState(true)
  const atDaily = STOPS[stop] === "daily"
  useEffect(() => {
    if (!atDaily || reducedMotion || !stepping) return
    const timer = window.setInterval(() => setDay((was) => was + 1), DAY_MS)
    return () => window.clearInterval(timer)
  }, [atDaily, reducedMotion, stepping])
  const dailyMap = startOf(chosen).dailyRoute.map
  const scene = useMemo(
    () => (daily ? dailyOn(daily, [dailyMap], day, DAILY_SEED) : null),
    [daily, dailyMap, day],
  )
  const longest = useMemo(() => {
    const most = { trainers: "", items: "" }
    if (!daily) return most
    for (let d = 1; d <= HELD_DAYS; d++) {
      const on = dailyOn(daily, [dailyMap], d, DAILY_SEED)
      const trainers = trainersOn(on),
        items = itemsOn(on)
      if (trainers.length > most.trainers.length) most.trainers = trainers
      if (items.length > most.items.length) most.items = items
    }
    return most
  }, [daily, dailyMap])
  useEffect(() => {
    sceneRef.current = scene ? { scene, since: performance.now() } : null
    redrawRef.current()
  }, [scene])
  const [warping, setWarping] = useState(false)
  const [exploring, setExploring] = useState<Camera | null>(null)
  const [popup, setPopup] = useState<{ label: string | null; shown: boolean }>({
    label: null,
    shown: false,
  })

  useEffect(() => {
    const canvas = canvasRef.current
    const context = canvas?.getContext("2d")
    const section = canvas?.closest("section")
    if (!canvas || !context || !section) return
    let raf = 0
    let disposed = false
    let cleanup = () => {}
    // Frames are only for a canvas that can be seen: on screen, with its
    // stage shown, and no explorer over it.
    const stage = section.parentElement
    let onScreen = true
    const shown = () => onScreen && !stage?.hasAttribute("data-hidden") && !exploringRef.current

    // The map name popup slides away, then back with the next name, the way
    // the game's does on walking into a new area.
    let place: string | null = null
    let popupTimer = 0
    const showPlace = (next: string | null) => {
      if (next === place) return
      place = next
      window.clearTimeout(popupTimer)
      setPopup((current) => ({ ...current, shown: false }))
      if (next) popupTimer = window.setTimeout(() => setPopup({ label: next, shown: true }), 340)
    }

    // Where the tour is: the stop shown (its shot is one past it, after the
    // opening), the flight under way if any, and when the ride began.
    let current = 0
    // At the circuit stop, the league the camera has flown on to, if any.
    let leagueFocus: LeagueId | null = null
    // Where the camera rests: the stop's shot, or its league's.
    const aim = () => (leagueFocus ? leagueShot(leagueFocus) : current + 1)
    // A flight's `to` is a shot's index in `shotsFor`'s list.
    let flight: { from: Camera; to: number; start: number; duration: number } | null = null
    // Where the visitor has moved the map to, from the second stop on; the
    // next NEXT or BACK flies on from there.
    let manual: Camera | null = null
    let hintTimer = 0
    const showHint = () => {
      setHint(true)
      window.clearTimeout(hintTimer)
      hintTimer = window.setTimeout(() => setHint(false), HINT_MS)
    }
    // The camera panning onto a Gym's leader (or Gym), then keeping them in view.
    let follow: { gym: Gym; from: Camera; start: number } | null = null
    let started: number | null = null
    // The warp to another start, if one is playing: when it began and, once
    // the screen is black, when the cut happened.
    let warp: { to: StartId; start: number; cut: number | null } | null = null
    /** Whether the visitor may pan and zoom: from the second stop on, outside a warp. */
    const free = () => current >= 1 && !warp
    // The badge stop's clock: when the camera set off for it and when it left.
    let badgesSince: number | null = null
    let badgesLeft: number | null = null
    // How much of the circuit (trail and league plates) is up, 0 to 1.
    let circuitMix = 0
    let lastFrame = 0

    const sheets = new Map<StartId, HTMLImageElement>()
    const images = STARTS.map((start) => {
      const image = new Image()
      image.src = start.rider
      sheets.set(start.id, image)
      return image
    })
    const badgeSheet = new Image()
    badgeSheet.src = BADGES_SRC
    images.push(badgeSheet)
    void Promise.all([
      loadWorld(base),
      loadLandmarks(base),
      ...images.map((image) => image.decode().catch(() => {})),
    ]).then(([world, landmarks]) => {
      if (disposed) return
      setReady(true)
      setMeta(world.meta)
      const renderer = rendererFor(world)
      // Under everything at every stop: the living sea, the maps' weather and
      // the clouds' shadows (which thin out as the camera comes in).
      let sea: Overlay | undefined
      void loadWater(world.meta, base).then(
        (water) => {
          if (disposed) return
          sea = seaOverlay(water, !reducedMotion, tick)
          schedule()
        },
        () => {},
      )
      const weather = weatherOverlay(world.meta, !reducedMotion, tick)
      const clouds = cloudsOverlay(!reducedMotion, tick)
      let start = startOf(DEFAULT_START)
      let route = routeIn(world.meta, start)
      let ride = rideMs(route)
      const stopClock: StopClock = { alpha: 0, elapsed: 0, animate: !reducedMotion }
      const pins = badgeOverlays(landmarks, badgeSheet, stopClock)

      const area = () => {
        // The map gets the screen beside the caption on wide screens, above
        // it on narrow ones.
        const box = canvas.getBoundingClientRect()
        const size = { width: box.width, height: box.height }
        const caption = captionRef.current?.getBoundingClientRect()
        if (caption) {
          if (caption.left - box.left > box.width * 0.4) size.width = caption.left - box.left - 24
          else size.height = caption.top - box.top - 16
        }
        return size
      }

      /** The cut: the screen is black, so everything switches to the new start. */
      const cut = (to: StartId, now: number) => {
        start = startOf(to)
        route = routeIn(world.meta, start)
        ride = rideMs(route)
        current = 0
        leagueFocus = null
        flight = null
        manual = null
        follow = null
        badgesSince = null
        badgesLeft = null
        circuitMix = 0
        // The ride begins once the screen has faded back in.
        started = now + (reducedMotion ? 0 : FADE_IN_MS)
      }

      const draw = (now: number) => {
        raf = 0
        if (!shown()) return

        // The warp's fade: 0 clear to 1 black, in the game's whole steps.
        let fade = 0
        if (warp) {
          if (warp.cut === null) {
            const t = (now - warp.start) / FADE_OUT_MS
            if (t >= 1) {
              cut(warp.to, now)
              warp.cut = now
              fade = 1
            } else fade = stepped(t)
          } else {
            const t = (now - warp.cut) / FADE_IN_MS
            if (t >= 1) {
              warp = null
              setWarping(false)
            } else fade = stepped(1 - t)
          }
        }

        const { width, height } = area()
        const shots = shotsFor(world.meta, landmarks, start, width, height)
        const rideStart = started === null ? null : started + RIDE_DELAY_MS
        const progress = reducedMotion
          ? 1
          : rideStart === null
            ? 0
            : Math.max(0, (now - rideStart) / ride)
        let camera: Camera
        if (flight) {
          const t = (now - flight.start) / flight.duration
          camera = fly(flight.from, shots[flight.to]!, t)
          if (t >= 1) flight = null
        } else if (follow) {
          const { gym: target, from } = follow
          const leader =
            target.leader && notablesRef.current
              ? notableAt(notablesRef.current, target.leader.slug, pageMinutes())
              : undefined
          const to = leader ?? target
          const t = reducedMotion ? 1 : Math.min(1, (now - follow.start) / FIND_MS)
          const eased = t < 0.5 ? 4 * t * t * t : 1 - (-2 * t + 2) ** 3 / 2
          camera = {
            x: from.x + (to.x - from.x) * eased,
            y: from.y + (to.y - from.y) * eased,
            zoom: from.zoom,
          }
          manual = camera
        } else if (manual) camera = manual
        else if (current === 0) camera = fly(shots[0]!, shots[1]!, progress)
        else camera = shots[aim()]!
        cameraRef.current = camera

        const sheet = sheets.get(start.id)!
        const rider = riderAt(route, progress, sheet)
        // The popup names where the rider is while the camera is with them.
        showPlace(
          current > 0 || rideStart === null || now < rideStart
            ? null
            : placeAt(world.meta, start, { x: rider.x + 1, y: rider.y + 2 }),
        )

        // The badge stop's pins and trail show while the tour is there, and
        // fade as it leaves.
        let badgesUp = false
        if (pinsAt(current) && badgesSince !== null) {
          stopClock.elapsed = now - badgesSince
          stopClock.alpha = reducedMotion
            ? 1
            : Math.min(1, Math.max(0, (stopClock.elapsed - BADGES_IN_DELAY_MS) / BADGES_IN_MS))
          badgesUp = true
        } else if (badgesLeft !== null && !reducedMotion) {
          stopClock.alpha = Math.max(0, 1 - (now - badgesLeft) / BADGES_OUT_MS)
          badgesUp = stopClock.alpha > 0
        } else stopClock.alpha = 0
        const overlays: Overlay[] = [...(sea ? [sea] : []), weather, clouds]
        if (badgesUp) overlays.push(...pins)
        stopClock.selected = gymRef.current?.badge ?? null
        const target = STOPS[current] === "circuit" ? 1 : 0
        const step = Math.min(100, now - lastFrame) / CIRCUIT_MS
        lastFrame = now
        circuitMix = reducedMotion
          ? target
          : target > circuitMix
            ? Math.min(target, circuitMix + step)
            : Math.max(target, circuitMix - step)
        stopClock.circuit = circuitMix
        stopClock.earned = earnedRef.current
        stopClock.earnedAt = earnedAtRef.current
        stopClock.calling = callingFor(earnedRef.current)
        stopClock.league = leagueFocus
        // The sea, the weather and the clouds keep every stop moving.
        let animated = !reducedMotion
        if (circuitMix !== target) animated = true
        // The leader whose Gym card is open walks the badge stop's map.
        const card = gymRef.current
        if (badgesUp && card?.leader && notablesRef.current) {
          const { slug, also } = card.leader
          overlays.push(
            notablesOverlay(notablesRef.current, pageMinutes, {
              only: (s) => s === slug || s === also,
              minHeight: 44,
              marker: true,
              standing: reducedMotion,
            }),
          )
          if (!reducedMotion) animated = true
        }
        for (const entry of stopOverlaysRef.current ?? [])
          if (entry.stop === STOPS[current]) {
            overlays.push(entry.overlay)
            animated ||= Boolean(entry.animated)
          }

        const pending = renderer.draw(context, {
          camera,
          focus: { x: width / 2, y: height / 2 },
          labelWidth: width,
          // The popup names the close-ups; names on the map come in as it pulls back.
          labels: camera.zoom < shots[1]!.zoom * 0.7,
          sprites:
            rider.image && sheet.complete && sheet.naturalWidth && camera.zoom >= RIDER_MIN_ZOOM
              ? [rider]
              : [],
          overlays,
        })
        if (fade > 0) {
          context.setTransform(1, 0, 0, 1, 0, 0)
          context.fillStyle = `rgb(0 0 0 / ${fade})`
          context.fillRect(0, 0, canvas.width, canvas.height)
        }
        const riding = rideStart !== null && progress < 1
        if (pending || flight || (follow && !reducedMotion) || riding || warp || animated)
          schedule()
      }
      const schedule = () => {
        if (!raf && shown()) raf = requestAnimationFrame(draw)
      }
      redrawRef.current = schedule

      goRef.current = (next) => {
        if (next === current || warp) return
        const { width, height } = area()
        const now = performance.now()
        const shots = shotsFor(world.meta, landmarks, start, width, height)
        const from = cameraRef.current ?? shots[aim()]!
        const to = shots[next + 1]!
        manual = null
        follow = null
        leagueFocus = null
        stopClock.hover = null
        stopClock.pressed = null
        canvas.style.cursor = ""
        flight = reducedMotion
          ? null
          : { from, to: next + 1, start: now, duration: flightMs(from, to) }
        // The pins come in at the region and the circuit, and go between them.
        if (pinsAt(next) && !pinsAt(current)) {
          badgesSince = now
          badgesLeft = null
        } else if (!pinsAt(next) && pinsAt(current)) {
          badgesSince = null
          badgesLeft = now
        }
        current = next
        schedule()
      }

      // At the circuit stop, fly on to a league, or back out to the whole circuit.
      focusRef.current = (league) => {
        if (STOPS[current] !== "circuit" || league === leagueFocus || warp) return
        const { width, height } = area()
        const shots = shotsFor(world.meta, landmarks, start, width, height)
        const from = manual ?? cameraRef.current ?? shots[aim()]!
        leagueFocus = league
        manual = null
        follow = null
        const to = shots[aim()]!
        flight = reducedMotion
          ? null
          : { from, to: aim(), start: performance.now(), duration: flightMs(from, to) }
        schedule()
      }

      chooseRef.current = (id) => {
        if (id === start.id || warp) return
        const now = performance.now()
        warp = { to: id, start: now, cut: null }
        // With reduced motion there is no fade: the new start is just there.
        if (reducedMotion) {
          cut(id, now)
          warp = null
          setWarping(false)
        }
        schedule()
      }

      // NEXT pressed while the map was still loading moved only the card:
      // fly the map on to where the card is.
      if (stopRef.current !== current) goRef.current(stopRef.current)

      // Pan, from the second stop on, without taking the page's scroll: a
      // mouse drags the map, and on touch two fingers move it while one still
      // scrolls the page (a hint says so). The zoom stays the stop's.
      const move = (dx: number, dy: number) => {
        const from = manual ?? cameraRef.current
        if (!from) return
        flight = null
        follow = null
        manual = {
          zoom: from.zoom,
          x: Math.min(world.meta.width, Math.max(0, from.x - dx / from.zoom)),
          y: Math.min(world.meta.height, Math.max(0, from.y - dy / from.zoom)),
        }
        schedule()
      }
      const local = (event: { clientX: number; clientY: number }) => {
        const box = canvas.getBoundingClientRect()
        return { x: event.clientX - box.left, y: event.clientY - box.top }
      }
      const listening = new AbortController()
      const { signal } = listening
      let drag: { x: number; y: number } | null = null
      // How far the last press moved, so a drag doesn't also count as a tap.
      let travelled = 0
      canvas.addEventListener(
        "pointerdown",
        (event) => {
          travelled = 0
          if (event.pointerType === "touch" || event.button !== 0 || !free()) return
          canvas.setPointerCapture(event.pointerId)
          drag = local(event)
        },
        { signal },
      )
      canvas.addEventListener(
        "pointermove",
        (event) => {
          if (!drag) return
          const now = local(event)
          travelled += Math.hypot(now.x - drag.x, now.y - drag.y)
          move(now.x - drag.x, now.y - drag.y)
          drag = now
        },
        { signal },
      )
      const release = () => {
        drag = null
      }
      canvas.addEventListener("pointerup", release, { signal })
      canvas.addEventListener("pointercancel", release, { signal })
      let pair: { x: number; y: number } | null = null
      const middle = (touches: TouchList) => {
        const [a, b] = [touches[0]!, touches[1]!]
        const box = canvas.getBoundingClientRect()
        return {
          x: (a.clientX + b.clientX) / 2 - box.left,
          y: (a.clientY + b.clientY) / 2 - box.top,
        }
      }
      canvas.addEventListener(
        "touchstart",
        (event) => {
          if (!free() || event.touches.length < 2) return
          event.preventDefault()
          pair = middle(event.touches)
        },
        { passive: false, signal },
      )
      canvas.addEventListener(
        "touchmove",
        (event) => {
          if (!free()) return
          // One finger scrolls the page; say how to move the map instead.
          if (event.touches.length < 2) {
            if (!pair) showHint()
            return
          }
          if (!pair) return
          event.preventDefault()
          const now = middle(event.touches)
          travelled += Math.hypot(now.x - pair.x, now.y - pair.y)
          move(now.x - pair.x, now.y - pair.y)
          pair = now
        },
        { passive: false, signal },
      )
      canvas.addEventListener(
        "touchend",
        (event) => {
          if (event.touches.length < 2) pair = null
        },
        { signal },
      )

      // The pin (if any) under a point on the canvas, at the badge stop.
      const pinAt = (at: { x: number; y: number }): Gym | null => {
        if (!pinsAt(current) || stopClock.alpha <= 0 || !cameraRef.current) return null
        const camera = cameraRef.current
        const { width, height } = area()
        let best: Gym | null = null
        let bestDistance = Math.max(18, canvas.clientWidth > 700 ? 25 : 13)
        for (const pin of landmarks.badges) {
          const x = (pin.x - camera.x) * camera.zoom + width / 2
          const y = (pin.y - camera.y) * camera.zoom + height / 2
          const distance = Math.hypot(at.x - x, at.y - y)
          if (distance < bestDistance) {
            best = pin
            bestDistance = distance
          }
        }
        return best
      }
      // Pins answer the pointer: a hover lights one up and names its town,
      // a press dips it, and a tap opens its Gym's card.
      const hoverPin = (badge: number | null) => {
        if (stopClock.hover === badge) return
        stopClock.hover = badge
        canvas.style.cursor = badge === null ? "" : "pointer"
        schedule()
      }
      hoverRef.current = hoverPin
      canvas.addEventListener(
        "pointermove",
        (event) => {
          if (event.pointerType === "touch") return
          if (drag && travelled > 6) {
            stopClock.pressed = null
            hoverPin(null)
            return
          }
          hoverPin(pinAt(local(event))?.badge ?? null)
        },
        { signal },
      )
      canvas.addEventListener("pointerleave", () => hoverPin(null), { signal })
      canvas.addEventListener(
        "pointerdown",
        (event) => {
          const pin = pinAt(local(event))
          if (!pin) return
          stopClock.pressed = pin.badge
          schedule()
        },
        { signal },
      )
      const unpress = () => {
        if (stopClock.pressed == null) return
        stopClock.pressed = null
        schedule()
      }
      canvas.addEventListener("pointerup", unpress, { signal })
      canvas.addEventListener("pointercancel", unpress, { signal })
      canvas.addEventListener(
        "click",
        (event) => {
          if (travelled > 6) return
          const pin = pinAt(local(event))
          if (pin) toggleRef.current(pin)
        },
        { signal },
      )
      setGymPins(landmarks.badges)
      setLeagues(landmarks.circuit)
      setRoster(landmarks.roster)

      // The Gym card's button: pan onto the leader and keep them in view as
      // they walk, or onto the Gym when its leader doesn't walk the map.
      findRef.current = (target) => {
        const from = manual ?? cameraRef.current
        if (!from) return
        flight = null
        follow = { gym: target, from, start: performance.now() }
        schedule()
      }

      const go = () => {
        if (started !== null) return
        started = performance.now()
        schedule()
      }
      const watcher = new IntersectionObserver(
        ([entry]) => {
          if (entry?.isIntersecting) go()
        },
        { threshold: 0.7 },
      )
      if (!startOn) watcher.observe(section)
      else if (section.dataset.started) go()
      else section.addEventListener(startOn, go, { once: true })
      const seen = new IntersectionObserver(([entry]) => {
        onScreen = entry?.isIntersecting ?? true
        schedule()
      })
      seen.observe(section)
      const unhidden = new MutationObserver(schedule)
      if (stage) unhidden.observe(stage, { attributes: true, attributeFilter: ["data-hidden"] })
      const resizer = new ResizeObserver(schedule)
      resizer.observe(canvas)
      cleanup = () => {
        listening.abort()
        watcher.disconnect()
        seen.disconnect()
        unhidden.disconnect()
        if (startOn) section.removeEventListener(startOn, go)
        resizer.disconnect()
      }
      void document.fonts.ready.then(schedule)
      schedule()
    })

    return () => {
      disposed = true
      cancelAnimationFrame(raf)
      window.clearTimeout(popupTimer)
      window.clearTimeout(hintTimer)
      cleanup()
    }
  }, [base, reducedMotion, startOn])

  // At the circuit stop, the league the map has flown on to.
  const [focused, setFocused] = useState<LeagueId | null>(null)
  const go = (next: number) => {
    // Each card starts at its top on a phone, where the cards scroll.
    captionsRef.current?.scrollTo({ top: 0 })
    setGym(null)
    setFocused(null)
    setStop(next)
    stopRef.current = next
    goRef.current(next)
  }
  const focus = (league: LeagueId | null) => {
    setGym(null)
    setFocused(league)
    focusRef.current(league)
  }

  // A tap on a pin (or its hidden button) earns the badge and opens its
  // Gym's card, or gives it back and closes the card. The first time the set
  // qualifies a league that hasn't phoned yet, its call rings a moment later.
  const toggle = (pin: Gym) => {
    const next = new Set(earnedRef.current)
    if (next.delete(pin.badge)) {
      if (gymRef.current?.badge === pin.badge) setGym(null)
    } else {
      next.add(pin.badge)
      earnedAtRef.current.set(pin.badge, performance.now())
      setGym(pin)
      const caller = callerFor(next, calledRef.current)
      if (caller) {
        calledRef.current.add(caller)
        window.clearTimeout(callTimer.current)
        callTimer.current = window.setTimeout(() => setCall(caller), reducedMotion ? 0 : 900)
      }
    }
    earnedRef.current = next
    setEarned(next)
    redrawRef.current()
  }
  useEffect(() => {
    toggleRef.current = toggle
  })
  useEffect(() => () => window.clearTimeout(callTimer.current), [])

  // The call: the league's five at the player's rating, and where it leads.
  const rating = Math.max(QUALIFYING_RATING, trainerRating(earned.size))
  const callLineup = call && roster ? eventSequence(roster, call, rating, 1)[0]! : null
  const answered = (accepted: boolean) => {
    const league = call
    setCall(null)
    if (accepted && league) {
      if (STOPS[stop] !== "circuit") go(stopIndex("circuit"))
      focus(league)
      // The dialog gives the focus back to a pin that is about to leave.
      requestAnimationFrame(() => nextRef.current?.focus())
    }
  }

  const choose = (id: StartId) => {
    if (id === chosen || warping) return
    setChosen(id)
    setWarping(!reducedMotion)
    chooseRef.current(id)
  }

  // The explorer opens as a modal over the page, starting where the tour is.
  useEffect(() => {
    exploringRef.current = exploring !== null
    if (exploring) dialogRef.current?.showModal()
    else redrawRef.current()
  }, [exploring])

  return (
    <>
      <canvas
        ref={canvasRef}
        aria-hidden="true"
        className={`absolute inset-0 size-full touch-pan-x touch-pan-y ${stop >= 1 && !warping ? "cursor-grab active:cursor-grabbing" : ""}`}
        style={{ background: OPEN_SEA }}
      />
      {!ready && (
        // On a slow connection the map can still be on its way after the warp.
        <p
          aria-hidden="true"
          className="pointer-events-none absolute inset-x-0 top-1/3 text-center gba-text gba-text-light animate-pulse motion-reduce:animate-none"
        >
          LOADING THE WORLD...
        </p>
      )}
      {pinsAt(stop) && (
        // The pins for a keyboard: each Gym a button, lighting its pin while focused.
        // Fixed to the screen, so focusing one never scrolls the page. At the
        // region stop, only its own.
        <ul className="sr-only" style={{ position: "fixed", top: 0, left: 0 }}>
          {gymPins
            .filter(
              (pin) =>
                STOPS[stop] !== "region" || pin.region.toUpperCase() === startOf(chosen).region,
            )
            .toSorted((a, b) => REGION_ORDER.indexOf(a.region) - REGION_ORDER.indexOf(b.region))
            .map((pin) => (
              <li key={pin.badge}>
                <button
                  type="button"
                  aria-pressed={earned.has(pin.badge)}
                  onFocus={() => hoverRef.current(pin.badge)}
                  onBlur={() => hoverRef.current(null)}
                  onClick={() => toggleRef.current(pin)}
                >
                  {pin.name ?? "Badge"}, {pin.town} Gym{pin.leader ? `, ${pin.leader.name}` : ""}
                </button>
              </li>
            ))}
        </ul>
      )}
      {pinsAt(stop) && (
        // How many are earned, and how many more it takes for a league to phone.
        <p
          aria-live="polite"
          className="pixel-corners pixel-panel absolute top-4 left-4 z-20 flex items-center gap-3 px-4 py-3 text-paper"
        >
          <span
            aria-hidden
            className="size-6 shrink-0 [image-rendering:pixelated]"
            style={{
              backgroundImage: `url(${BADGES_SRC})`,
              backgroundSize: `${24 * 24}px 24px`,
              backgroundPosition: `${-(earned.size ? [...earned].at(-1)! : 8) * 24}px 0`,
            }}
          />
          <span className="pixel-label text-[16px] leading-none">BADGES {earned.size}/24</span>
          {earned.size < 8 && (
            <span className="text-[13px] text-paper/60 max-sm:hidden">
              {8 - earned.size} more and a league phones
            </span>
          )}
        </p>
      )}
      {gym && pinsAt(stop) && (
        <GymCard
          gym={gym}
          base={base}
          notables={notables}
          minutes={minutes}
          count={earned.size}
          onFind={() => findRef.current(gym)}
          onClose={() => setGym(null)}
        />
      )}
      {roster &&
        leagues
          .filter(
            (league) =>
              STOPS[stop] === "circuit" &&
              league.id !== "masters" &&
              league.halls &&
              league.id === focused,
          )
          .map((league) => (
            // On a wide screen the five lie over the map, beside the card.
            <LineupPanel
              key={league.id}
              roster={roster}
              league={league.id as Exclude<LeagueId, "masters">}
              halls={league.halls!}
              progress={rating}
              base={base}
              reducedMotion={reducedMotion}
              variant="map"
            />
          ))}
      <div
        aria-live="polite"
        className={`pointer-events-none absolute inset-x-0 top-1/3 z-10 flex justify-center px-4 transition-opacity duration-200 ${hint ? "opacity-100" : "opacity-0"}`}
      >
        {hint && (
          <p className="pixel-panel gba-text gba-text-light px-4 py-3 text-center">
            USE TWO FINGERS TO MOVE THE MAP
          </p>
        )}
      </div>
      {living && (
        <LivingClock
          living={living}
          minutes={minutes}
          className={`absolute top-4 left-4 transition-opacity duration-300 ${atWorld ? "opacity-100" : "pointer-events-none opacity-0"}`}
        />
      )}
      <div
        aria-hidden="true"
        className="pointer-events-none absolute top-0 left-0 grid h-[calc(40*var(--px))] w-[calc(96*var(--px))] place-items-center bg-[url(/game/popups/wood.png)] bg-size-[100%_100%] [image-rendering:pixelated] transition-[translate,opacity] duration-[330ms] ease-[steps(20)] motion-reduce:transition-none"
        style={{
          translate: popup.shown ? "0 0" : "0 -100%",
          opacity: popup.shown ? 1 : 0,
        }}
      >
        <span className="gba-text whitespace-nowrap">{popup.label}</span>
      </div>
      <div className="pointer-events-none relative flex h-full items-end justify-end p-4 sm:p-8 lg:items-center lg:p-12">
        <div
          ref={captionRef}
          className="pixel-corners pixel-panel pointer-events-auto relative w-full p-4 sm:p-8 lg:w-[400px]"
        >
          <JourneyStrip
            stop={stop}
            start={startOf(chosen)}
            badge={Math.min(
              ...gymPins
                .filter((pin) => pin.region.toUpperCase() === startOf(chosen).region)
                .map((pin) => pin.badge),
            )}
            disabled={warping}
            onGo={(next) => {
              if (next !== stop) go(next)
            }}
          />
          <p className="sr-only" aria-live="polite">
            {(() => {
              const text = { ...captions[stop], ...captions[stop]?.variants?.[chosen] }
              return `${text.place}. ${text.title}`
            })()}
          </p>
          {/*
            The card is one height at every stop, a share of the screen: below
            lg the map sits above it, so it keeps to a third of the screen; from
            lg it keeps within the screen. A stop with more to say scrolls
            inside it, so the card never jumps as content changes or loads.
          */}
          <div
            ref={captionsRef}
            className="grid max-lg:h-[34svh] max-lg:overflow-y-auto max-lg:overscroll-contain lg:h-[calc(100svh-20rem)] lg:overflow-y-auto lg:overscroll-contain"
          >
            {captions.map((caption, i) => {
              const shown = stop === i
              const text = { ...caption, ...caption.variants?.[chosen] }
              return (
                <div
                  key={caption.place}
                  aria-hidden={!shown}
                  inert={!shown}
                  className="col-start-1 row-start-1 transition-opacity duration-300"
                  style={{ opacity: shown ? 1 : 0, pointerEvents: shown ? undefined : "none" }}
                >
                  <p className="pixel-label text-logo-yellow">{text.place}</p>
                  <p className="pixel-heading mt-2 sm:mt-3">{text.title}</p>
                  <p className="mt-2 text-base leading-snug text-paper/80 sm:mt-4 sm:text-lg sm:leading-relaxed">
                    {text.body}
                  </p>
                  {i === 0 && (
                    <fieldset className="mt-5 grid gap-2">
                      <legend className="sr-only">Where to begin</legend>
                      {STARTS.map((option) => {
                        const picked = option.id === chosen
                        return (
                          <label
                            key={option.id}
                            className={`${OPTION} ${
                              picked
                                ? "bg-[rgb(232_208_64)] text-[rgb(56_56_56)] shadow-[inset_0_0_0_4px_rgb(56_56_56),inset_0_-4px_0_rgb(184_160_32),inset_0_4px_0_rgb(248_240_152)]"
                                : "cursor-pointer bg-black/30 text-paper/80 shadow-[inset_0_0_0_4px_rgb(0_0_0/0.4)] hover:bg-black/50 hover:text-paper"
                            }`}
                          >
                            <input
                              type="radio"
                              name="start"
                              className="sr-only"
                              checked={picked}
                              disabled={warping}
                              onChange={() => choose(option.id)}
                            />
                            <span>
                              {picked ? "▶ " : ""}
                              {option.label}
                            </span>
                            <span className={picked ? "text-[rgb(56_56_56)]/70" : "text-paper/50"}>
                              {option.region}
                            </span>
                          </label>
                        )
                      })}
                    </fieldset>
                  )}
                  {STOPS[i] === "daily" && (
                    <DailyCard
                      scene={scene}
                      longest={longest}
                      day={day}
                      onDay={(next) => {
                        setStepping(false)
                        setDay(Math.max(1, next))
                      }}
                    />
                  )}
                  {STOPS[i] === "world" && (
                    <WildTally
                      wild={wild}
                      shared={wildClock}
                      shown={shown}
                      first={["kanto", "johto", "hoenn"].indexOf(chosen)}
                      reducedMotion={reducedMotion}
                      onChange={() => redrawRef.current()}
                    />
                  )}
                  {STOPS[i] === "circuit" && (
                    <CircuitLeagues
                      leagues={leagues}
                      earned={earned}
                      focused={shown ? focused : null}
                      onFocus={focus}
                      roster={roster}
                      progress={rating}
                      base={base}
                      reducedMotion={reducedMotion}
                    />
                  )}
                  {text.stats && (
                    <dl className="mt-6 grid grid-cols-3 gap-4">
                      {text.stats.map(({ value, label }) => (
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
              )
            })}
          </div>
          <div className="mt-3 flex items-center justify-between gap-4 sm:mt-6">
            {stop > 0 ? (
              <button
                type="button"
                disabled={warping}
                onClick={() => {
                  go(stop - 1)
                  if (stop === 1) nextRef.current?.focus()
                }}
                className="pixel-label px-2 pt-2 pb-3 text-paper/70 hover:text-paper focus-visible:text-paper focus-visible:underline focus-visible:outline-none"
              >
                BACK
              </button>
            ) : (
              <span />
            )}
            {stop < captions.length - 1 ? (
              <button
                type="button"
                ref={nextRef}
                disabled={warping}
                onClick={() => go(stop + 1)}
                className={BUTTON}
              >
                NEXT ▶
              </button>
            ) : (
              <button
                type="button"
                ref={nextRef}
                onClick={() => setExploring(cameraRef.current ?? { x: 0, y: 0, zoom: 0 })}
                className={BUTTON}
              >
                EXPLORE THE MAP
              </button>
            )}
          </div>
        </div>
      </div>
      {call && callLineup && (
        <LeagueCall
          league={call}
          player={startOf(chosen).player}
          lineup={callLineup}
          reducedMotion={reducedMotion}
          onClose={answered}
        />
      )}
      <dialog
        ref={dialogRef}
        aria-label="World map"
        onClose={() => setExploring(null)}
        className="m-0 h-svh max-h-none w-screen max-w-none bg-transparent p-0 backdrop:bg-black/70"
      >
        {exploring && (
          <>
            <WorldMap
              base={base}
              start={exploring.zoom > 0 ? exploring : undefined}
              extra={placeExtra}
            />
            <button
              type="button"
              onClick={() => dialogRef.current?.close()}
              className={`${BUTTON} absolute top-4 right-4`}
            >
              CLOSE
            </button>
          </>
        )}
      </dialog>
    </>
  )
}

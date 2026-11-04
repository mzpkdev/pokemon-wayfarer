import { type CSSProperties, useEffect, useRef, useState } from "react"

import { useReducedMotion } from "~/lib/hooks"
import {
  LivingClock,
  type LivingWorld,
  loadLiving,
  loadNotables,
  NOTABLES_SRC,
  type Notable,
  type Notables,
  notablesAt,
  pageMinutes,
  periodAt,
  type Ship,
  shipsAt,
  type Train,
  trainAt,
  WALK_FRAMES,
} from "~/world-map"

/**
 * The world as it is this moment on the page's clock, the same clock the map
 * runs on: the time of day, the three ships on their legs, the Magnet Train
 * on its line and every Gym Leader, Elite Four and Champion at whatever they
 * are up to, as the game's own overworld sprites. Nothing here is a loop of
 * canned states: it is the map's simulation (`~/world-map`), read at the
 * page's game minute, so a visitor who scrolls back up finds everyone where
 * they left them.
 */

/** The ships' sprites, drawn bow to the west; on the track they sail east, so they are mirrored. */
const SHIPS: Record<string, { src: string; width: number; height: number }> = {
  aqua: { src: "/game/sprites/ss-aqua.png", width: 96, height: 40 },
  anne: { src: "/game/sprites/ss-anne.png", width: 128, height: 64 },
  tidal: { src: "/game/sprites/ss-tidal.png", width: 96, height: 40 },
}
const TRAIN = { src: "/game/sprites/magnet-train.png", width: 128, height: 32 }

/** How big a ship or the train is drawn on its track: the sprite's pixels, halved. */
const TRACK_SCALE = 0.5

/** A walking frame lasts about this long, in ms (the game's 8 ticks). */
const STEP_MS = 140
/** Sprites on the board at twice the game's size. */
const SPRITE_PX = 2

type Group = {
  id: string
  label: string
  /** The group's members, by their place in the roster (region and role). */
  keep: (region: string | undefined, role: string) => boolean
}

/** Who stands together on the board: each region's Gym Leaders, then the leagues' trainers. */
const GROUPS: Group[] = [
  {
    id: "kanto",
    label: "KANTO",
    keep: (region, role) => region === "Kanto" && role === "Gym Leader",
  },
  {
    id: "johto",
    label: "JOHTO",
    keep: (region, role) => region === "Johto" && role === "Gym Leader",
  },
  {
    id: "hoenn",
    label: "HOENN",
    keep: (region, role) => region === "Hoenn" && role === "Gym Leader",
  },
  { id: "leagues", label: "LEAGUES", keep: (_, role) => role !== "Gym Leader" },
]

/**
 * A leg drawn as a line between two places, with whatever rides it put at
 * `progress` along the way (0 at `from`, 1 at `to`). The line is dashed like
 * the map's sea lanes, and ends `--end` in from the edges, half the widest
 * sprite, so a ship in port stays inside the board.
 */
const Track = ({
  from,
  to,
  progress,
  children,
}: {
  from: string
  to: string
  progress: number
  children: React.ReactNode
}) => (
  <div className="relative pt-9 pb-5 [--end:2rem]">
    <div
      aria-hidden="true"
      className="mx-[var(--end)] h-0 border-t-2 border-dashed border-paper/30"
    />
    <div
      aria-hidden="true"
      className="absolute top-9 -translate-x-1/2 -translate-y-[60%] motion-safe:transition-[left] motion-safe:duration-150 motion-safe:ease-linear"
      style={{ left: `calc(var(--end) + (100% - 2 * var(--end)) * ${progress})` }}
    >
      {children}
    </div>
    <div className="mt-2 flex justify-between gap-4 text-xs text-paper/60">
      <span className="truncate">{from}</span>
      <span className="truncate text-right">{to}</span>
    </div>
  </div>
)

const ShipRow = ({ ship, living }: { ship: Ship; living: LivingWorld }) => {
  const sprite = SHIPS[ship.id]!
  const from = living.ports[ship.from]!.name
  const next = living.ports[ship.next]!.name
  return (
    <li className="grid items-center gap-x-6 sm:grid-cols-[10rem_1fr]">
      <div>
        <p className="pixel-label text-[16px] leading-none text-sign">{ship.name}</p>
        <p className="mt-1.5 text-sm leading-snug text-paper/75">
          {ship.at === "port" ? `In port at ${from}` : `${from} to ${next}`}
        </p>
      </div>
      <Track from={from} to={next} progress={ship.progress}>
        <img
          src={sprite.src}
          alt=""
          width={sprite.width}
          height={sprite.height}
          className="pixelated block max-w-none -scale-x-100"
          style={{ width: sprite.width * TRACK_SCALE, height: sprite.height * TRACK_SCALE }}
        />
      </Track>
    </li>
  )
}

const TrainRow = ({ train, name }: { train: Train; name: string }) => (
  <li className="grid items-center gap-x-6 sm:grid-cols-[10rem_1fr]">
    <div>
      <p className="pixel-label text-[16px] leading-none text-sign">{name}</p>
      <p className="mt-1.5 text-sm leading-snug text-paper/75">
        {train.at === "station" ? `At ${train.from} Station` : `${train.from} to ${train.to}`}
      </p>
    </div>
    <Track from={train.from} to={train.to} progress={train.progress}>
      <img
        src={TRAIN.src}
        alt=""
        width={TRAIN.width}
        height={TRAIN.height}
        className="pixelated block max-w-none"
        style={{ width: TRAIN.width * TRACK_SCALE, height: TRAIN.height * TRACK_SCALE }}
      />
    </Track>
  </li>
)

/** A notable as their overworld sprite, facing the way they walk and stepping while they do. */
const Walker = ({
  notable,
  world,
  tick,
  still,
}: {
  notable: Notable
  world: Notables
  tick: number
  still: boolean
}) => {
  const face = notable.facing === "right" ? "left" : notable.facing
  const [stand, one, two] = WALK_FRAMES[face]
  const phase = (tick + notable.row) % 4
  const noWalk = world.still[notable.row]
  const frame =
    still || !notable.moving || noWalk ? stand : phase === 1 ? one : phase === 3 ? two : stand
  // A sheet with no walking frames hops a little instead.
  const hop = !still && notable.moving && noWalk ? phase % 2 : 0
  const style: CSSProperties = {
    width: 16 * SPRITE_PX,
    height: 32 * SPRITE_PX,
    backgroundImage: `url(${NOTABLES_SRC})`,
    backgroundSize: `${world.image.naturalWidth * SPRITE_PX}px ${world.image.naturalHeight * SPRITE_PX}px`,
    backgroundPosition: `${-frame * 16 * SPRITE_PX}px ${-notable.row * 32 * SPRITE_PX}px`,
    scale: notable.facing === "right" ? "-1 1" : undefined,
    translate: hop ? `0 ${-hop * SPRITE_PX}px` : undefined,
  }
  return (
    <span className="relative block" style={{ width: 16 * SPRITE_PX, height: 32 * SPRITE_PX }}>
      <span
        aria-hidden="true"
        className={`absolute bottom-0.5 left-1/2 h-1.5 w-5 -translate-x-1/2 rounded-full ${notable.sea ? "bg-white/50" : "bg-black/35"}`}
      />
      <span aria-hidden="true" className="pixelated absolute inset-0 block" style={style} />
    </span>
  )
}

/** "fishing at Nugget Bridge", "on the way to Oak's Lab": the label after the name. */
const doing = (notable: Notable) => notable.label.split(" · ")[1] ?? notable.place

export const RightNow = () => {
  const reducedMotion = useReducedMotion()
  const rootRef = useRef<HTMLDivElement>(null)
  const [living, setLiving] = useState<LivingWorld | null>(null)
  const [notables, setNotables] = useState<Notables | null>(null)
  const [failed, setFailed] = useState(false)
  const [now, setNow] = useState<{ minutes: number; tick: number } | null>(null)
  const [group, setGroup] = useState(GROUPS[0]!.id)

  useEffect(() => {
    let cancelled = false
    Promise.all([loadLiving(), loadNotables()]).then(
      ([livingWorld, notablesWorld]) => {
        if (cancelled) return
        setLiving(livingWorld)
        setNotables(notablesWorld)
      },
      () => {
        if (!cancelled) setFailed(true)
      },
    )
    return () => {
      cancelled = true
    }
  }, [])

  // The board reads the clock only while it is on screen, at the walk
  // cycle's pace (or once a second with motion reduced, when nobody walks).
  useEffect(() => {
    const root = rootRef.current
    if (!root) return
    let timer = 0
    const read = () =>
      setNow({ minutes: pageMinutes(), tick: Math.floor(performance.now() / STEP_MS) })
    const start = () => {
      if (timer) return
      read()
      timer = window.setInterval(read, reducedMotion ? 1000 : STEP_MS)
    }
    const stop = () => {
      window.clearInterval(timer)
      timer = 0
    }
    const observer = new IntersectionObserver(([entry]) => {
      if (entry?.isIntersecting) start()
      else stop()
    })
    observer.observe(root)
    return () => {
      stop()
      observer.disconnect()
    }
  }, [reducedMotion])

  const ready = living && notables && now
  const period = ready ? periodAt(living, now.minutes) : null
  const ships = ready ? shipsAt(living, now.minutes) : []
  const train = ready ? trainAt(living, now.minutes) : undefined
  const chosen = GROUPS.find((entry) => entry.id === group) ?? GROUPS[0]!
  const walking = ready
    ? notablesAt(notables, now.minutes).filter((notable) => {
        const plan = notables.plans.find((entry) => entry.slug === notable.slug)
        return plan && chosen.keep(plan.region, plan.role)
      })
    : []

  return (
    <div
      ref={rootRef}
      className="pixel-corners mat relative px-5 py-6 sm:px-8 sm:py-8"
      aria-busy={!ready && !failed}
    >
      {failed ? (
        <p className="gba-text gba-text-light [--px:1px]">The world is out of reach right now.</p>
      ) : !ready ? (
        <p className="gba-text gba-text-light min-h-[20rem] [--px:1px]">Loading the world…</p>
      ) : (
        <>
          <div className="flex flex-wrap items-center gap-x-5 gap-y-3">
            <LivingClock
              living={living}
              minutes={now.minutes}
              className="pixel-corners [--px:1px]"
            />
            <p className="pixel-label text-[16px] text-paper/70">DAY {period!.day}</p>
            <p className="text-sm text-paper/60">
              Two minutes to a day here. In the game a day is two hours of play.
            </p>
          </div>

          <h3 className="pixel-label mt-8 text-[16px] text-sign">AT SEA AND ON THE RAILS</h3>
          <ul className="mt-4 grid gap-5">
            {ships.map((ship) => (
              <ShipRow key={ship.id} ship={ship} living={living} />
            ))}
            {train && living.train && <TrainRow train={train} name={living.train.name} />}
          </ul>

          <div className="mt-8 flex flex-wrap items-center justify-between gap-3">
            <h3 className="pixel-label text-[16px] text-sign">ON THEIR ROUNDS</h3>
            <ul className="flex flex-wrap gap-1.5" aria-label="Who to show">
              {GROUPS.map((entry) => {
                const on = entry.id === chosen.id
                return (
                  <li key={entry.id}>
                    <button
                      type="button"
                      aria-pressed={on}
                      onClick={() => setGroup(entry.id)}
                      className={`pixel-corners px-3 py-1.5 font-[family-name:var(--font-gba)] text-[14px] leading-[16px] ${
                        on
                          ? "bg-sign text-ink"
                          : "bg-black/30 text-paper/80 hover:bg-black/50 hover:text-paper"
                      }`}
                    >
                      {entry.label}
                    </button>
                  </li>
                )
              })}
            </ul>
          </div>
          <ul className="mt-5 grid grid-cols-2 gap-x-4 gap-y-5 sm:grid-cols-3 lg:grid-cols-4">
            {walking.map((notable) => (
              <li key={notable.slug} className="flex items-start gap-3">
                <Walker notable={notable} world={notables} tick={now.tick} still={reducedMotion} />
                <div className="min-w-0 pt-1">
                  <p className="pixel-label text-[16px] leading-none">
                    {notable.name.toUpperCase()}
                  </p>
                  <p className="mt-1 text-[13px] leading-snug text-paper/70">{doing(notable)}</p>
                </div>
              </li>
            ))}
          </ul>
        </>
      )}
    </div>
  )
}

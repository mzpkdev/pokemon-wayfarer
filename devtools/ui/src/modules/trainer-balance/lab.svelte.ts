import catalogData from "./catalog.json"
import {
  ARCHETYPES,
  ARCHETYPE_KIND,
  EXPERIMENT_VERSION,
  HOME_REGIONS,
  LEVEL_OFFSET,
  MAX_ACES,
  MAX_ANCHORS,
  MAX_POOL_ENTRIES,
  OLD_VERSION_REJECTION,
  LEVEL_CAP_ANCHORS,
  MAX_BADGES,
  WORLD_PROGRESS_CHECKPOINTS,
  badgeMatch,
  badgeTR,
  createExperiment,
  evolutionIndex,
  evolutionStatus,
  gymLadder,
  isGymLeader,
  leagueSequence,
  leagueWorlds,
  learnsetIndex,
  levelCap,
  milestoneEnd,
  milestones,
  poolLearning,
  resolveTrainer,
  rosterGaps,
  serializeExperiment,
  validateExperiment,
  worldLevels,
} from "./engine.js"
import type {
  Anchor,
  Archetype,
  Catalog,
  Experiment,
  HomeRegion,
  PoolEntry,
  RosterSlot,
  ScalerKind,
  TrainerRecord,
  TrainerSettings,
} from "./types.js"

const data = catalogData as Catalog
export const catalog: TrainerRecord[] = data.trainers
/** The catalog's predecessor chains, indexed by every stage on them. */
export const evolution = evolutionIndex(data.evolution)
/** Every roster line stage's learnsets, and the valid move names. */
export const learnsets = learnsetIndex(data.learnsets)
/** Valid move names, sorted, for the move pool editor. */
export const moveNames: readonly string[] = data.learnsets.moves

/** A species' line with evolution levels, e.g. "Geodude → Graveler Lv 25 → Golem Lv 38". */
export const lineText = (species: string): string =>
  (evolution.lines.get(species) ?? [{ species, level: 1 }])
    .map((stage, index) => (index === 0 ? stage.species : `${stage.species} Lv ${stage.level}`))
    .join(" → ")
/** The roster editor's evolution warning for an authored species, if any. */
export const stageWarning = (species: string): string | null => {
  const { known, final } = evolutionStatus(evolution, species)
  if (!known) return `No evolution data for ${species} in the catalog, so it never steps down.`
  if (!final) return `${species} is not a final stage. Roster slots normally author final stages.`
  return null
}
type TRScalerId = "teamLevel" | "teamSize" | "wildLevel" | "routeTrainerLevel"
/** TR scalers by their experiment key; archetype growth scalers by archetype name. */
export type ScalerId = TRScalerId | Archetype
export const SCALER_IDS: readonly ScalerId[] = [
  "teamLevel",
  "teamSize",
  "wildLevel",
  "routeTrainerLevel",
  ...ARCHETYPES,
]
const isGrowth = (id: ScalerId): id is Archetype => (ARCHETYPES as readonly string[]).includes(id)
const anchorsOf = (experiment: Experiment, id: ScalerId): Anchor[] =>
  isGrowth(id) ? experiment.archetypes[id] : experiment[id]
/**
 * How a scaler reads between anchors. The kind is fixed: Burst is the only step scaler; team
 * size is interpolated, and its paired anchors make its steps.
 */
export const scalerKind = (id: ScalerId): ScalerKind =>
  isGrowth(id) ? ARCHETYPE_KIND[id] : "interpolated"
const storageKey = "wayfarer-trainer-balance-v16"
/** Where the leagues are entered: each at its own badge point (8 / 16 / 24), or all at the player TR. */
export type LeagueEntryPoint = "badges" | "player"
/** The chart runs across player TR 0 to at least this. */
export const CHART_MIN_END = Math.max(200, LEVEL_CAP_ANCHORS.at(-1)?.[0] ?? 0)
/** The chart never runs past this player TR; a higher player TR is marked at the right edge. */
export const CHART_MAX_END = 400
/** The player TR slider's range; the number field takes any larger whole TR. */
export const PLAYER_TR_SLIDER_MAX = 200
const firstTrainer = catalog[0]
if (!firstTrainer) throw new Error("The trainer catalog is empty.")
const initialTrainerId = firstTrainer.id

/**
 * Reads a move pool entry's fields: a known move name and an optional from level (blank waits
 * for each member's level-up learn level).
 */
export const poolEntry = (moveText: string, fromText: string, where: string): PoolEntry => {
  const move = moveText.trim()
  if (!move) throw new Error(`${where}: enter a move name.`)
  // Names match regardless of case; the pool stores the game's spelling.
  const name = learnsets.moves.has(move)
    ? move
    : moveNames.find((known) => known.toLowerCase() === move.toLowerCase())
  if (!name) throw new Error(`${where}: “${move}” is not a move in the game data.`)
  const from = fromText.trim()
  if (!from) return { move: name }
  const fromLevel = Number(from)
  if (!Number.isInteger(fromLevel) || fromLevel < 1 || fromLevel > 100)
    throw new Error(`${where}: the from level must be a whole number from 1 to 100, or blank.`)
  return { move: name, fromLevel }
}

/**
 * The pool editor's hint for one entry: the roster's earliest level-up learner (own species or
 * an earlier form, e.g. "Level-up: Persian, earlier form (Meowth) at Lv 30"), or a move learned
 * only by TM/tutor or as an egg move, which needs a from level.
 */
export const poolLearningHint = (
  entry: PoolEntry,
  roster: readonly Pick<RosterSlot, "species">[],
): { text: string; needsFrom: boolean } => {
  const learning = poolLearning(entry.move, roster, evolution, learnsets)
  const from = entry.fromLevel
  if (learning.kind === "unlearnable") return { text: "No roster line learns it", needsFrom: false }
  if (learning.kind !== "level-up") {
    const label = learning.kind === "egg" ? "Egg move" : "TM/tutor only"
    return from === undefined
      ? {
          text:
            learning.kind === "egg"
              ? "Egg move: needs a from level"
              : `${label} — needs a from level`,
          needsFrom: true,
        }
      : { text: `${label}: from Lv ${from}`, needsFrom: false }
  }
  const at = learning.level === 0 ? "on evolving" : `at Lv ${learning.level}`
  const learner =
    learning.form === learning.species
      ? `${learning.species} ${at}`
      : `${learning.species}, earlier form (${learning.form}) ${at}`
  return {
    text:
      from === undefined
        ? `Level-up: ${learner}`
        : `Any learner from Lv ${from} (level-up: ${learner})`,
    needsFrom: false,
  }
}

export class BalanceLab {
  #_experiment = $state<Experiment>(createExperiment(catalog, learnsets.moves))
  /** The player TR: world progress for notable trainers and the input to world scaling. */
  playerTR = $state(0)
  query = $state("")
  region = $state("All regions")
  role = $state("All trainers")
  selectedId = $state(initialTrainerId)
  leagueAt = $state<LeagueEntryPoint>("badges")
  editor = $state("")
  error = $state("")
  notice = $state("")

  world = $derived(worldLevels(this.#_experiment, this.playerTR))
  /** Where the player TR sits on the badge formula. */
  badgeMatch = $derived(badgeMatch(this.playerTR))
  /** The badge count the player TR matches exactly, or null between badge counts and past 24. */
  badges = $derived(this.badgeMatch.kind === "exact" ? this.badgeMatch.badges : null)
  /** The badge slider position: the highest badge count at or below the player TR. */
  badgeFloor = $derived(
    this.badgeMatch.kind === "exact"
      ? this.badgeMatch.badges
      : this.badgeMatch.kind === "between"
        ? this.badgeMatch.lower
        : MAX_BADGES,
  )
  /** World progress is the player TR as notable trainers see it. */
  worldProgress = $derived(this.world.tr)
  cap = $derived(this.world.cap)
  anchors = $derived(
    Object.fromEntries(SCALER_IDS.map((id) => [id, anchorsOf(this.#_experiment, id)])) as Record<
      ScalerId,
      Anchor[]
    >,
  )
  allRows = $derived(
    catalog.map((trainer) =>
      resolveTrainer(trainer, this.#_experiment, this.worldProgress, evolution, learnsets),
    ),
  )
  rows = $derived(
    this.allRows.filter(({ trainer }) => {
      const query = this.query.trim().toLowerCase()
      return (
        (this.region === "All regions" || trainer.region === this.region) &&
        (this.role === "All trainers" ||
          (this.role === "Gym Leaders" ? isGymLeader(trainer) : trainer.role === this.role)) &&
        (!query || `${trainer.name} ${trainer.region}`.toLowerCase().includes(query))
      )
    }),
  )
  selected = $derived.by(() => {
    const row = this.allRows.find((row) => row.trainer.id === this.selectedId) ?? this.allRows[0]
    if (!row) throw new Error("The trainer catalog is empty.")
    return row
  })
  settings = $derived(this.#_settings(this.selected.trainer.id))
  gaps = $derived(rosterGaps(catalog, this.#_experiment))
  /** World progress for each league in the standard entry sequence. */
  leagueWorlds = $derived(leagueWorlds(this.leagueAt === "player" ? this.playerTR : null))
  /** The standard sequence ranked by league score, each lineup resolved at its league's world progress. */
  leagues = $derived(
    leagueSequence(catalog, this.#_experiment, this.leagueWorlds).map((ranking) => ({
      ...ranking,
      matches: ranking.lineup.map((entrant) =>
        resolveTrainer(entrant.trainer, this.#_experiment, ranking.world, evolution, learnsets),
      ),
    })),
  )
  ladder = $derived(gymLadder(catalog, this.#_experiment, this.worldProgress))
  /** The selected trainer's TR, team level and team (by roster slot) at each world progress checkpoint. */
  growth = $derived(
    WORLD_PROGRESS_CHECKPOINTS.map((world) => {
      const { tr, teamLevel, team } = resolveTrainer(
        this.selected.trainer,
        this.#_experiment,
        world,
        evolution,
        learnsets,
      )
      return { world, tr, teamLevel, team }
    }),
  )
  /** Every world progress where the selected trainer's team changes. */
  milestones = $derived(milestones(this.selected.trainer, this.#_experiment, evolution, learnsets))
  /**
   * The chart series: the selected trainer's team level and the level cap at each whole player TR
   * from 0 to at least CHART_MIN_END, further (up to CHART_MAX_END) when the player TR or the
   * milestones go past it. Nothing changes past the milestones, so a higher player TR is flat.
   */
  chart = $derived.by(() => {
    const end = Math.min(
      CHART_MAX_END,
      Math.max(
        CHART_MIN_END,
        Math.ceil(this.playerTR / 20) * 20,
        Math.ceil(milestoneEnd(this.#_experiment, this.settings.archetype) / 20) * 20,
      ),
    )
    return Array.from({ length: end + 1 }, (_, world) => {
      const { tr, teamLevel } = resolveTrainer(
        this.selected.trainer,
        this.#_experiment,
        world,
        evolution,
      )
      return { world, tr, teamLevel, cap: levelCap(world) }
    })
  })
  aboveCap = $derived(this.allRows.filter((row) => row.teamLevel > this.cap).length)

  constructor() {
    this.#_syncEditors()
  }

  #_settings(id: string): TrainerSettings {
    const settings = this.#_experiment.trainers[id]
    if (!settings) throw new Error(`Unknown trainer: ${id}`)
    return settings
  }

  #_name = (id: string): string => catalog.find((trainer) => trainer.id === id)?.name ?? id

  #_syncEditors = (): void => {
    this.editor = JSON.stringify(this.#_experiment.trainers[this.selectedId], null, 2)
  }

  #_persist = (): void => {
    try {
      localStorage.setItem(storageKey, this.exportText())
    } catch {
      this.notice = "Browser storage is unavailable. Export the experiment to keep your changes."
    }
  }

  #_clone = (): Experiment => JSON.parse(serializeExperiment(this.#_experiment)) as Experiment

  #_accept = (candidate: unknown, message: string): void => {
    this.#_experiment = validateExperiment(candidate, catalog, learnsets.moves)
    this.error = ""
    this.notice = message
    this.#_syncEditors()
    this.#_persist()
  }

  #_edit = (fallback: string, change: (next: Experiment) => string | undefined): void => {
    try {
      const next = this.#_clone()
      const message = change(next)
      if (message) this.#_accept(next, message)
    } catch (error) {
      this.error = error instanceof Error ? error.message : fallback
    }
  }

  #_roster = (next: Experiment): RosterSlot[] => {
    const roster = next.trainers[this.selectedId]?.roster
    if (!roster) throw new Error("Unknown selected trainer.")
    return roster
  }

  #_pool = (next: Experiment): PoolEntry[] => {
    const pool = next.trainers[this.selectedId]?.movePool
    if (!pool) throw new Error("Unknown selected trainer.")
    return pool
  }

  load = (): void => {
    try {
      const saved = localStorage.getItem(storageKey)
      if (saved) this.importText(saved, "Restored your local experiment.")
    } catch {
      this.notice = "Browser storage is unavailable. You can still import and export experiments."
    }
  }

  select = (id: string): void => {
    if (!catalog.some((trainer) => trainer.id === id)) return
    this.selectedId = id
    this.error = ""
    this.#_syncEditors()
    this.#_persist()
  }

  /** A badge preset: sets the player TR from the badge formula. */
  setBadges = (badges: number): void => {
    this.playerTR = badgeTR(
      Number.isFinite(badges) ? Math.min(MAX_BADGES, Math.max(0, Math.round(badges))) : 0,
    )
    this.#_persist()
  }

  /** Sets the player TR directly: any whole number of 0 or more (TR has no upper limit). */
  setPlayerTR = (value: number): void => {
    if (!Number.isFinite(value)) return
    this.playerTR = Math.min(Number.MAX_SAFE_INTEGER, Math.max(0, Math.round(value)))
    this.#_persist()
  }

  setLeagueAt = (at: LeagueEntryPoint): void => {
    this.leagueAt = at
    this.#_persist()
  }

  /** Sets the selected trainer's home region. */
  setHomeRegion = (value: string): void =>
    this.#_edit("Could not change the home region.", (next) => {
      const settings = next.trainers[this.selectedId]
      if (!settings) throw new Error("Unknown selected trainer.")
      if (!HOME_REGIONS.includes(value as HomeRegion)) throw new Error("Unknown home region.")
      settings.homeRegion = value as HomeRegion
      return `${this.#_name(this.selectedId)}’s home region is now ${value}.`
    })

  /** Sets whether the selected trainer is a traveller. */
  setTraveller = (traveller: boolean): void =>
    this.#_edit("Could not change the traveller trait.", (next) => {
      const settings = next.trainers[this.selectedId]
      if (!settings) throw new Error("Unknown selected trainer.")
      settings.traveller = traveller
      return `${this.#_name(this.selectedId)} is now ${traveller ? "a traveller" : "not a traveller"}.`
    })

  /** Sets whether the selected trainer is aloof. */
  setAloof = (aloof: boolean): void =>
    this.#_edit("Could not change the aloof trait.", (next) => {
      const settings = next.trainers[this.selectedId]
      if (!settings) throw new Error("Unknown selected trainer.")
      settings.aloof = aloof
      return `${this.#_name(this.selectedId)} is now ${aloof ? "aloof" : "not aloof"}.`
    })

  /** Reads start TR, archetype and peak TR from the growth form. */
  applyGrowth = (form: HTMLFormElement): void =>
    this.#_edit("Could not change the growth.", (next) => {
      const settings = next.trainers[this.selectedId]
      if (!settings) throw new Error("Unknown selected trainer.")
      const data = new FormData(form)
      const whole = (name: string, label: string): number => {
        const text = String(data.get(name) ?? "").trim()
        const value = Number(text)
        if (!text || !Number.isSafeInteger(value) || value < 0)
          throw new Error(`${label} must be a whole number of 0 or more.`)
        return value
      }
      const archetype = String(data.get("archetype")) as Archetype
      const startTR = whole("startTR", "Start TR")
      const peakTR = whole("peakTR", "Peak TR")
      if (peakTR < startTR) throw new Error("Peak TR must be at least start TR.")
      if (archetype === "legend" && peakTR !== startTR)
        throw new Error("A Legend never grows: set peak TR equal to start TR.")
      Object.assign(settings, { startTR, archetype, peakTR })
      return `Updated ${this.#_name(this.selectedId)}’s growth.`
    })

  /** Swaps a roster slot with its neighbour; roster slot 1 must stay an ace at offset 0. */
  moveSlot = (index: number, direction: -1 | 1): void =>
    this.#_edit("Could not reorder the roster.", (next) => {
      const roster = this.#_roster(next)
      const moved = roster[index]
      const other = roster[index + direction]
      if (!moved || !other) return undefined
      roster[index] = other
      roster[index + direction] = moved
      return `Moved ${moved.species} to roster slot ${index + direction + 1}.`
    })

  addSlot = (): void =>
    this.#_edit("Could not add a roster slot.", (next) => {
      const roster = this.#_roster(next)
      roster.push({
        species: "Unown",
        levelOffset: LEVEL_OFFSET.default,
        isAce: false,
        item: null,
        ability: null,
        nature: null,
      })
      return `Added roster slot ${roster.length}. Rename its species in the roster editor.`
    })

  removeSlot = (index: number): void =>
    this.#_edit("Could not remove the roster slot.", (next) => {
      const [removed] = this.#_roster(next).splice(index, 1)
      return removed ? `Removed ${removed.species}.` : undefined
    })

  /**
   * Marks a roster slot as an ace or a filler slot. Roster slot 1 is always an ace, and a
   * roster has at most MAX_ACES. Returns whether the change was accepted.
   */
  setAce = (index: number, isAce: boolean): boolean => {
    let accepted = false
    this.#_edit("Could not change the ace slots.", (next) => {
      const roster = this.#_roster(next)
      const slot = roster[index]
      if (!slot || slot.isAce === isAce) return undefined
      if (index === 0) throw new Error("Roster slot 1 is the signature Pokémon and always an ace.")
      if (isAce && roster.filter((entry) => entry.isAce).length >= MAX_ACES)
        throw new Error(
          `A roster has at most ${MAX_ACES} aces (roster slot 1 plus two more). Clear another ace first.`,
        )
      slot.isAce = isAce
      accepted = true
      return isAce
        ? `${slot.species} (roster slot ${index + 1}) is now an ace.`
        : `${slot.species} (roster slot ${index + 1}) is now a filler slot.`
    })
    return accepted
  }

  /** Reads species, offset and item for every roster slot from the roster form. */
  applyRoster = (form: HTMLFormElement): void =>
    this.#_edit("Could not apply the roster.", (next) => {
      const data = new FormData(form)
      const text = (name: string) => String(data.get(name) ?? "").trim()
      this.#_roster(next).forEach((slot, index) => {
        slot.species = text(`slot-${index}-species`)
        const offset = text(`slot-${index}-offset`)
        slot.levelOffset = offset === "" ? Number.NaN : Number(offset)
        slot.item = text(`slot-${index}-item`) || null
      })
      return `Updated ${this.selected.trainer.name}’s roster.`
    })

  /** Reads every move pool entry's move and from level from the move pool form. */
  applyPool = (form: HTMLFormElement): void =>
    this.#_edit("Could not apply the move pool.", (next) => {
      const data = new FormData(form)
      const text = (name: string) => String(data.get(name) ?? "")
      const pool = this.#_pool(next)
      pool.forEach((_, index) => {
        pool[index] = poolEntry(
          text(`pool-${index}-move`),
          text(`pool-${index}-from`),
          `Pool entry ${index + 1}`,
        )
      })
      return `Updated ${this.selected.trainer.name}’s move pool.`
    })

  /** Adds a move to the bottom of the pool. Returns whether it was added. */
  addPoolEntry = (move: string, fromLevel: string): boolean => {
    let added = false
    this.#_edit("Could not add the move.", (next) => {
      const pool = this.#_pool(next)
      if (pool.length >= MAX_POOL_ENTRIES)
        throw new Error(`A move pool lists at most ${MAX_POOL_ENTRIES} entries.`)
      const entry = poolEntry(move, fromLevel, "New pool entry")
      pool.push(entry)
      added = true
      return `Added ${entry.move} as pool entry ${pool.length}.`
    })
    return added
  }

  /** Swaps a pool entry with its neighbour: pool order is identity, top entries reach the aces first. */
  movePoolEntry = (index: number, direction: -1 | 1): void =>
    this.#_edit("Could not reorder the move pool.", (next) => {
      const pool = this.#_pool(next)
      const moved = pool[index]
      const other = pool[index + direction]
      if (!moved || !other) return undefined
      pool[index] = other
      pool[index + direction] = moved
      return `Moved ${moved.move} to pool entry ${index + direction + 1}.`
    })

  removePoolEntry = (index: number): void =>
    this.#_edit("Could not remove the pool entry.", (next) => {
      const [removed] = this.#_pool(next).splice(index, 1)
      return removed ? `Removed ${removed.move} from the move pool.` : undefined
    })

  applyTeam = (): void =>
    this.#_edit("Could not apply the settings.", (next) => {
      next.trainers[this.selectedId] = JSON.parse(this.editor)
      return `Updated ${this.selected.trainer.name}’s settings.`
    })

  applyScalers = (form: HTMLFormElement): void =>
    this.#_edit("Could not apply the scalers.", (next) => {
      const data = new FormData(form)
      const read = (scaler: ScalerId) =>
        anchorsOf(next, scaler).map(
          (_, index): Anchor => [
            index === 0 ? 0 : Number(data.get(`${scaler}-tr-${index}`)),
            Number(data.get(`${scaler}-value-${index}`)),
          ],
        )
      for (const scaler of SCALER_IDS) {
        const anchors = read(scaler)
        if (isGrowth(scaler)) next.archetypes[scaler] = anchors
        else next[scaler] = anchors
      }
      return "Updated the scalers."
    })

  addAnchor = (scaler: ScalerId): void =>
    this.#_edit("Could not add an anchor.", (next) => {
      const anchors = anchorsOf(next, scaler)
      const last = anchors.at(-1)
      if (!last) return undefined
      if (anchors.length >= MAX_ANCHORS)
        throw new Error(`A scaler has at most ${MAX_ANCHORS} anchors.`)
      anchors.push([last[0] + 10, last[1]])
      return "Added an anchor. Edit its TR and value, then apply the scalers."
    })

  removeAnchor = (scaler: ScalerId, index: number): void =>
    this.#_edit("Could not remove the anchor.", (next) => {
      if (index === 0) return undefined
      anchorsOf(next, scaler).splice(index, 1)
      return "Removed the anchor."
    })

  reset = (): void => {
    this.#_accept(
      createExperiment(catalog, learnsets.moves),
      "Restored the catalog growth, rosters, move pools, home regions, traits and scalers.",
    )
  }

  resetTrainer = (): void =>
    this.#_edit("Could not restore the trainer.", (next) => {
      const defaults = createExperiment(catalog, learnsets.moves).trainers[this.selectedId]
      if (!defaults) throw new Error("Unknown selected trainer.")
      next.trainers[this.selectedId] = defaults
      return `Restored ${this.selected.trainer.name}’s catalog growth, roster, move pool, home region and traits. Other trainers and the scalers are unchanged.`
    })

  exportText = (): string =>
    JSON.stringify(
      {
        tool: "wayfarer-trainer-balance",
        version: EXPERIMENT_VERSION,
        point: { playerTR: this.playerTR },
        league: { at: this.leagueAt },
        selectedTrainer: this.selectedId,
        experiment: JSON.parse(serializeExperiment(this.#_experiment)),
      },
      null,
      2,
    )

  importText = (text: string, message = "Imported experiment."): void => {
    try {
      const data = JSON.parse(text)
      if (
        data?.tool === "wayfarer-trainer-balance" &&
        typeof data.version === "number" &&
        data.version >= 1 &&
        data.version < EXPERIMENT_VERSION
      )
        throw new Error(OLD_VERSION_REJECTION(data.version))
      if (data?.tool !== "wayfarer-trainer-balance" || data.version !== EXPERIMENT_VERSION)
        throw new Error(`This is not a version ${EXPERIMENT_VERSION} Wayfarer balance experiment.`)
      const experiment = validateExperiment(data.experiment, catalog, learnsets.moves)
      // The point is a player TR; early version 8 files saved 0–24 badges instead, and a
      // badge point is still read.
      const point = data.point
      const keys = point && typeof point === "object" ? Object.keys(point) : []
      const playerTR =
        keys.length === 1 && keys[0] === "playerTR" && Number.isSafeInteger(point.playerTR)
          ? (point.playerTR as number)
          : keys.length === 1 &&
              keys[0] === "badges" &&
              Number.isInteger(point.badges) &&
              point.badges >= 0 &&
              point.badges <= MAX_BADGES
            ? badgeTR(point.badges)
            : -1
      if (playerTR < 0)
        throw new Error("The player point must be a player TR of 0 or more (or 0–24 badges).")
      if (!catalog.some((trainer) => trainer.id === data.selectedTrainer))
        throw new Error("Unknown selected trainer.")
      const league = data.league
      if (
        !league ||
        typeof league !== "object" ||
        Object.keys(league).join() !== "at" ||
        (league.at !== "badges" && league.at !== "player")
      )
        throw new Error("The league settings must be an entry point (badges or player).")
      this.#_experiment = experiment
      this.playerTR = playerTR
      this.leagueAt = league.at
      this.selectedId = data.selectedTrainer
      this.error = ""
      this.notice = message
      this.#_syncEditors()
      this.#_persist()
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not read the experiment."
    }
  }
}

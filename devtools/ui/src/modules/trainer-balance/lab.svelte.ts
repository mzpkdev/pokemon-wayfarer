import catalogData from "./catalog.json"
import {
  EXPERIMENT_VERSION,
  GROWTH_ARCHETYPES,
  LEVEL_OFFSET,
  MAX_ANCHORS,
  OLD_VERSION_REJECTION,
  WORLD_PROGRESS_CHECKPOINTS,
  createExperiment,
  gymLadder,
  leagueLineup,
  resolveTrainer,
  rosterGaps,
  serializeExperiment,
  teamLevelFor,
  trainerRating,
  validateExperiment,
  worldLevels,
} from "./engine.js"
import type {
  Anchor,
  Archetype,
  Experiment,
  GrowthArchetype,
  RosterSlot,
  TrainerRecord,
  TrainerSettings,
} from "./types.js"

export const catalog = catalogData as TrainerRecord[]
type TRScalerId = "teamLevel" | "teamSize" | "wildLevel" | "routeTrainerLevel"
/** TR scalers by their experiment key; archetype growth scalers by archetype name. */
export type ScalerId = TRScalerId | GrowthArchetype
export const SCALER_IDS: readonly ScalerId[] = [
  "teamLevel",
  "teamSize",
  "wildLevel",
  "routeTrainerLevel",
  ...GROWTH_ARCHETYPES,
]
const isGrowth = (id: ScalerId): id is GrowthArchetype =>
  (GROWTH_ARCHETYPES as readonly string[]).includes(id)
const anchorsOf = (experiment: Experiment, id: ScalerId): Anchor[] =>
  isGrowth(id) ? experiment.archetypes[id] : experiment[id]
/** A form field name for a scaler (archetype names contain spaces). */
export const scalerField = (id: ScalerId): string => id.replaceAll(" ", "-")
const storageKey = "wayfarer-trainer-balance-v7"
const firstTrainer = catalog[0]
if (!firstTrainer) throw new Error("The trainer catalog is empty.")
const initialTrainerId = firstTrainer.id

/** "LEVEL_UP" (or blank) keeps the level-up policy; otherwise moves split on commas or dots. */
export const parseMoves = (text: string): RosterSlot["moves"] => {
  const value = text.trim()
  if (!value || value.toUpperCase() === "LEVEL_UP") return "LEVEL_UP"
  return value
    .split(/[,·]/)
    .map((move) => move.trim())
    .filter(Boolean)
}
export const movesText = (moves: RosterSlot["moves"]): string =>
  moves === "LEVEL_UP" ? "LEVEL_UP" : moves.join(", ")

export class BalanceLab {
  #_experiment = $state<Experiment>(createExperiment(catalog))
  badges = $state(0)
  query = $state("")
  region = $state("All regions")
  role = $state("All trainers")
  selectedId = $state(initialTrainerId)
  editor = $state("")
  error = $state("")
  notice = $state("")

  world = $derived(worldLevels(this.#_experiment, { badges: this.badges }))
  playerTR = $derived(this.world.tr)
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
    catalog.map((trainer) => resolveTrainer(trainer, this.#_experiment, this.worldProgress)),
  )
  rows = $derived(
    this.allRows.filter(({ trainer }) => {
      const query = this.query.trim().toLowerCase()
      return (
        (this.region === "All regions" || trainer.region === this.region) &&
        (this.role === "All trainers" ||
          (this.role === "Gym Leaders"
            ? trainer.role === "Gym Leader"
            : trainer.role === this.role)) &&
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
  league = $derived(leagueLineup(catalog, this.#_experiment, this.worldProgress))
  ladder = $derived(gymLadder(catalog, this.#_experiment, this.worldProgress))
  /** The selected trainer's TR and team level at each world progress checkpoint. */
  growth = $derived(
    WORLD_PROGRESS_CHECKPOINTS.map((world) => {
      const tr = trainerRating(this.#_experiment, this.settings, world)
      return { world, tr, teamLevel: teamLevelFor(this.#_experiment, tr) }
    }),
  )
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
    this.#_experiment = validateExperiment(candidate, catalog)
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

  setBadges = (badges: number): void => {
    this.badges = Number.isFinite(badges) ? Math.min(24, Math.max(0, Math.round(badges))) : 0
    this.#_persist()
  }

  /** Reads start TR, archetype, peak TR and lead from the growth form. A rival needs a lead. */
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
      const leadText = String(data.get("lead") ?? "").trim()
      if (archetype === "rival" && !leadText) throw new Error("A rival needs a lead.")
      const lead = archetype === "rival" ? whole("lead", "Lead") : null
      Object.assign(settings, { startTR, archetype, peakTR, lead })
      return `Updated ${this.#_name(this.selectedId)}’s growth.`
    })

  /** Swaps a roster slot with its neighbour; roster slot 1 must stay at offset 0. */
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
        moves: "LEVEL_UP",
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

  /** Reads species, offset, moves and item for every roster slot from the roster form. */
  applyRoster = (form: HTMLFormElement): void =>
    this.#_edit("Could not apply the roster.", (next) => {
      const data = new FormData(form)
      const text = (name: string) => String(data.get(name) ?? "").trim()
      this.#_roster(next).forEach((slot, index) => {
        slot.species = text(`slot-${index}-species`)
        const offset = text(`slot-${index}-offset`)
        slot.levelOffset = offset === "" ? Number.NaN : Number(offset)
        slot.moves = parseMoves(text(`slot-${index}-moves`))
        slot.item = text(`slot-${index}-item`) || null
      })
      return `Updated ${this.selected.trainer.name}’s roster.`
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
            index === 0 ? 0 : Number(data.get(`${scalerField(scaler)}-tr-${index}`)),
            Number(data.get(`${scalerField(scaler)}-value-${index}`)),
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
    this.#_accept(createExperiment(catalog), "Restored the catalog growth, rosters and scalers.")
  }

  resetTrainer = (): void =>
    this.#_edit("Could not restore the trainer.", (next) => {
      const defaults = createExperiment(catalog).trainers[this.selectedId]
      if (!defaults) throw new Error("Unknown selected trainer.")
      next.trainers[this.selectedId] = defaults
      return `Restored ${this.selected.trainer.name}’s catalog growth and roster. Other trainers and the scalers are unchanged.`
    })

  exportText = (): string =>
    JSON.stringify(
      {
        tool: "wayfarer-trainer-balance",
        version: EXPERIMENT_VERSION,
        point: { badges: this.badges },
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
      const experiment = validateExperiment(data.experiment, catalog)
      const point = data.point
      if (
        !point ||
        !Number.isInteger(point.badges) ||
        point.badges < 0 ||
        point.badges > 24 ||
        Object.keys(point).length !== 1
      )
        throw new Error("The player point must be 0–24 badges.")
      if (!catalog.some((trainer) => trainer.id === data.selectedTrainer))
        throw new Error("Unknown selected trainer.")
      this.#_experiment = experiment
      this.badges = point.badges
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

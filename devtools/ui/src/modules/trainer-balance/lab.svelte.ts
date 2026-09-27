import catalogData from "./catalog.json"
import {
  LEVEL_OFFSET,
  MAX_ANCHORS,
  OLD_VERSION_REJECTION,
  createExperiment,
  leagueField,
  playerCap,
  playerRating,
  resolveTrainer,
  rosterGaps,
  serializeExperiment,
  validateExperiment,
} from "./engine.js"
import type { Anchor, Experiment, RosterEntry, TrainerRecord, TrainerSettings } from "./types.js"

export const catalog = catalogData as TrainerRecord[]
export type ScalerId = "teamLevel" | "teamSize"
const storageKey = "wayfarer-trainer-balance-v5"
const firstTrainer = catalog[0]
if (!firstTrainer) throw new Error("The trainer catalog is empty.")
const initialTrainerId = firstTrainer.id

/** "LEVEL_UP" (or blank) keeps the level-up policy; otherwise moves split on commas or dots. */
export const parseMoves = (text: string): RosterEntry["moves"] => {
  const value = text.trim()
  if (!value || value.toUpperCase() === "LEVEL_UP") return "LEVEL_UP"
  return value
    .split(/[,·]/)
    .map((move) => move.trim())
    .filter(Boolean)
}
export const movesText = (moves: RosterEntry["moves"]): string =>
  moves === "LEVEL_UP" ? "LEVEL_UP" : moves.join(", ")

export class BalanceLab {
  #_experiment = $state<Experiment>(createExperiment(catalog))
  badges = $state(0)
  leagueClears = $state(0)
  query = $state("")
  region = $state("All regions")
  role = $state("All trainers")
  selectedId = $state(initialTrainerId)
  editor = $state("")
  error = $state("")
  notice = $state("")

  point = $derived({ badges: this.badges, leagueClears: this.leagueClears })
  playerTR = $derived(playerRating(this.point))
  cap = $derived(playerCap(this.point))
  teamLevel = $derived(this.#_experiment.teamLevel)
  teamSize = $derived(this.#_experiment.teamSize)
  allRows = $derived(catalog.map((trainer) => resolveTrainer(trainer, this.#_experiment)))
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
    const row =
      this.allRows.find((entry) => entry.trainer.id === this.selectedId) ?? this.allRows[0]
    if (!row) throw new Error("The trainer catalog is empty.")
    return row
  })
  settings = $derived(this.#_settings(this.selected.trainer.id))
  gaps = $derived(rosterGaps(catalog, this.#_experiment))
  league = $derived(leagueField(catalog, this.#_experiment))
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

  #_roster = (next: Experiment): RosterEntry[] => {
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

  setClears = (clears: number): void => {
    this.leagueClears = Number.isFinite(clears) ? Math.min(3, Math.max(0, Math.round(clears))) : 0
    this.#_persist()
  }

  setTR = (id: string, value: string): void =>
    this.#_edit("Could not change the TR.", (next) => {
      const settings = next.trainers[id]
      if (!settings) throw new Error("Unknown trainer.")
      const tr = Number(value.trim())
      if (!value.trim() || !Number.isSafeInteger(tr) || tr < 0)
        throw new Error("TR must be a whole number of 0 or more.")
      settings.tr = tr
      return `${this.#_name(id)} is now TR ${tr}.`
    })

  /** Swaps an entry with its neighbour; entry 1 must stay at offset 0. */
  moveEntry = (index: number, direction: -1 | 1): void =>
    this.#_edit("Could not reorder the roster.", (next) => {
      const roster = this.#_roster(next)
      const entry = roster[index]
      const other = roster[index + direction]
      if (!entry || !other) return undefined
      roster[index] = other
      roster[index + direction] = entry
      return `Moved ${entry.species} to entry ${index + direction + 1}.`
    })

  addEntry = (): void =>
    this.#_edit("Could not add an entry.", (next) => {
      const roster = this.#_roster(next)
      roster.push({
        species: "Unown",
        levelOffset: LEVEL_OFFSET.default,
        moves: "LEVEL_UP",
        item: null,
        ability: null,
        nature: null,
      })
      return `Added entry ${roster.length}. Rename its species in the roster editor.`
    })

  removeEntry = (index: number): void =>
    this.#_edit("Could not remove the entry.", (next) => {
      const [removed] = this.#_roster(next).splice(index, 1)
      return removed ? `Removed ${removed.species}.` : undefined
    })

  /** Reads species, offset, moves and item for every entry from the roster form. */
  applyRoster = (form: HTMLFormElement): void =>
    this.#_edit("Could not apply the roster.", (next) => {
      const data = new FormData(form)
      const text = (name: string) => String(data.get(name) ?? "").trim()
      this.#_roster(next).forEach((entry, index) => {
        entry.species = text(`entry-${index}-species`)
        const offset = text(`entry-${index}-offset`)
        entry.levelOffset = offset === "" ? Number.NaN : Number(offset)
        entry.moves = parseMoves(text(`entry-${index}-moves`))
        entry.item = text(`entry-${index}-item`) || null
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
        next[scaler].map(
          (_, index): Anchor => [
            index === 0 ? 0 : Number(data.get(`${scaler}-tr-${index}`)),
            Number(data.get(`${scaler}-value-${index}`)),
          ],
        )
      next.teamLevel = read("teamLevel")
      next.teamSize = read("teamSize")
      return "Updated the team level and team size scalers."
    })

  addAnchor = (scaler: ScalerId): void =>
    this.#_edit("Could not add an anchor.", (next) => {
      const last = next[scaler].at(-1)
      if (!last) return undefined
      if (next[scaler].length >= MAX_ANCHORS)
        throw new Error(`A scaler has at most ${MAX_ANCHORS} anchors.`)
      next[scaler].push([last[0] + 10, last[1]])
      return "Added an anchor. Edit its TR and value, then apply the scalers."
    })

  removeAnchor = (scaler: ScalerId, index: number): void =>
    this.#_edit("Could not remove the anchor.", (next) => {
      if (index === 0) return undefined
      next[scaler].splice(index, 1)
      return "Removed the anchor."
    })

  reset = (): void => {
    this.#_accept(createExperiment(catalog), "Restored the catalog TRs, rosters and scalers.")
  }

  resetTrainer = (): void =>
    this.#_edit("Could not restore the trainer.", (next) => {
      const defaults = createExperiment(catalog).trainers[this.selectedId]
      if (!defaults) throw new Error("Unknown selected trainer.")
      next.trainers[this.selectedId] = defaults
      return `Restored ${this.selected.trainer.name}’s catalog TR and roster. Other trainers and the scalers are unchanged.`
    })

  exportText = (): string =>
    JSON.stringify(
      {
        tool: "wayfarer-trainer-balance",
        version: 5,
        point: { badges: this.badges, leagueClears: this.leagueClears },
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
        data.version <= 4
      )
        throw new Error(OLD_VERSION_REJECTION(data.version))
      if (data?.tool !== "wayfarer-trainer-balance" || data.version !== 5)
        throw new Error("This is not a version 5 Wayfarer balance experiment.")
      const experiment = validateExperiment(data.experiment, catalog)
      const point = data.point
      if (
        !point ||
        !Number.isInteger(point.badges) ||
        point.badges < 0 ||
        point.badges > 24 ||
        !Number.isInteger(point.leagueClears) ||
        point.leagueClears < 0 ||
        point.leagueClears > 3
      )
        throw new Error("Use 0–24 badges and 0–3 first league clears.")
      if (!catalog.some((trainer) => trainer.id === data.selectedTrainer))
        throw new Error("Unknown selected trainer.")
      this.#_experiment = experiment
      this.badges = point.badges
      this.leagueClears = point.leagueClears
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

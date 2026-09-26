import catalogData from "./catalog.json"
import {
  createExperiment,
  playerCap,
  playerRating,
  resolveTrainer,
  serializeExperiment,
  validateExperiment,
} from "./engine.js"
import type { Experiment, TrainerRecord, TrainerSettings, WorldPoint } from "./types.js"

export const catalog = catalogData as TrainerRecord[]
const storageKey = "wayfarer-trainer-balance-v1"
const firstTrainer = catalog[0]
if (!firstTrainer) throw new Error("The trainer catalog is empty.")
const initialTrainerId = firstTrainer.id

export class BalanceLab {
  #_experiment = $state<Experiment>(createExperiment(catalog))
  badges = $state(0)
  leagueClears = $state(0)
  query = $state("")
  region = $state("All regions")
  role = $state("All trainers")
  selectedId = $state(initialTrainerId)
  editor = $state("")
  anchorEditor = $state("")
  error = $state("")
  notice = $state("")

  point = $derived({ badges: this.badges, leagueClears: this.leagueClears })
  playerTR = $derived(playerRating(this.point))
  cap = $derived(playerCap(this.point))
  allRows = $derived(
    catalog.map((trainer) => resolveTrainer(trainer, this.#_experiment, this.point)),
  )
  rows = $derived(
    this.allRows.filter(({ trainer }) => {
      const query = this.query.trim().toLowerCase()
      return (
        (this.region === "All regions" || trainer.region === this.region) &&
        (this.role === "All trainers" ||
          (this.role === "Gym Leaders" ? trainer.gymEligible : trainer.role === this.role)) &&
        (!query ||
          `${trainer.name} ${trainer.region} ${trainer.homeLeagues.join(" ")}`
            .toLowerCase()
            .includes(query))
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
  aboveCap = $derived(
    this.allRows.filter((row) => row.trainer.gymEligible && row.aceLevel > this.cap).length,
  )
  gymCount = catalog.filter((trainer) => trainer.gymEligible).length
  curve = $derived(
    Array.from({ length: 25 }, (_, badges) => {
      const point = { badges, leagueClears: this.leagueClears }
      return {
        badges,
        ace: resolveTrainer(this.selected.trainer, this.#_experiment, point).aceLevel,
        cap: playerCap(point),
      }
    }),
  )

  constructor() {
    this.#_syncEditors()
  }

  #_settings(id: string): TrainerSettings {
    const settings = this.#_experiment.trainers[id]
    if (!settings) throw new Error(`Unknown trainer: ${id}`)
    return settings
  }

  #_syncEditors = (): void => {
    this.editor = JSON.stringify(this.#_experiment.trainers[this.selectedId], null, 2)
    this.anchorEditor = JSON.stringify(this.#_experiment.levelAnchors)
  }

  #_persist = (): void => {
    try {
      localStorage.setItem(storageKey, this.exportText())
    } catch {
      this.notice = "Browser storage is unavailable. Export the experiment to keep your changes."
    }
  }

  #_accept = (candidate: unknown, message: string): void => {
    this.#_experiment = validateExperiment(candidate, catalog)
    this.error = ""
    this.notice = message
    this.#_syncEditors()
    this.#_persist()
  }

  #_point = (value: unknown): WorldPoint => {
    if (!value || typeof value !== "object") throw new Error("Missing world progression.")
    const point = value as WorldPoint
    if (
      !Number.isInteger(point.badges) ||
      point.badges < 0 ||
      point.badges > 24 ||
      !Number.isInteger(point.leagueClears) ||
      point.leagueClears < 0 ||
      point.leagueClears > 3
    )
      throw new Error("Use 0–24 badges and 0–3 first league clears.")
    return point
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

  ratingAt = (id: string, index: number): number => this.#_settings(id).ratings[index] ?? 0

  applyRatings = (form: HTMLFormElement): void => {
    try {
      const data = new FormData(form)
      const next = JSON.parse(serializeExperiment(this.#_experiment)) as Experiment
      const ratings = [0, 8, 16, 24].map((badges) => Number(data.get(`rating-${badges}`)))
      const settings = next.trainers[this.selectedId]
      if (!settings) throw new Error("Unknown selected trainer.")
      settings.ratings = ratings as [number, number, number, number]
      settings.leagueGrowth = Number(data.get("league-growth"))
      this.#_accept(next, `Updated ${this.selected.trainer.name}’s growth curve.`)
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not apply ratings."
    }
  }

  applyTeam = (): void => {
    try {
      const next = JSON.parse(serializeExperiment(this.#_experiment)) as Experiment
      next.trainers[this.selectedId] = JSON.parse(this.editor)
      this.#_accept(next, `Updated ${this.selected.trainer.name}’s team stages.`)
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not apply team stages."
    }
  }

  applyAnchors = (): void => {
    try {
      const next = JSON.parse(serializeExperiment(this.#_experiment)) as Experiment
      next.levelAnchors = JSON.parse(this.anchorEditor)
      this.#_accept(next, "Updated the NPC level curve. The player cap is unchanged.")
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not apply the level curve."
    }
  }

  reset = (): void => {
    this.#_accept(createExperiment(catalog), "Restored the experimental defaults for all trainers.")
  }

  resetTrainer = (): void => {
    const next = JSON.parse(serializeExperiment(this.#_experiment)) as Experiment
    const defaults = createExperiment(catalog).trainers[this.selectedId]
    if (!defaults) throw new Error("Unknown selected trainer.")
    next.trainers[this.selectedId] = defaults
    this.#_accept(
      next,
      `Restored ${this.selected.trainer.name}’s current defaults. Other trainers and the shared level curve are unchanged.`,
    )
  }

  exportText = (): string =>
    JSON.stringify(
      {
        tool: "wayfarer-trainer-balance",
        version: 1,
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
      if (data.tool !== "wayfarer-trainer-balance" || data.version !== 1) {
        throw new Error("This is not a version 1 Wayfarer balance experiment.")
      }
      const experiment = validateExperiment(data.experiment, catalog)
      const point = this.#_point(data.point)
      if (!catalog.some((trainer) => trainer.id === data.selectedTrainer)) {
        throw new Error("Unknown selected trainer.")
      }
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

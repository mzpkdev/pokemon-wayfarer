// When each scene starts and how long it runs. A scene lasts at least its
// `seconds`; with a voice-over clip (`audio/vo/cues.json`, from `pnpm voice`)
// it lasts the clip plus a lead-in and a tail, so the picture waits for the words.
import { SCENES, type Scene } from "./script"

/** Seconds of picture before the voice starts in a scene, and after it ends. */
export const LEAD_SECONDS = 0.6
export const TAIL_SECONDS = 0.8

export type Cue = { seconds: number; hash: string; text: string }
export type Cues = { voice?: string; model?: string; scenes: Record<string, Cue> }

export type Slot = Scene & { start: number; length: number; voiceAt: number | null }

export const schedule = (cues: Cues | null, scenes = SCENES): Slot[] => {
  const slots: Slot[] = []
  let start = 0
  for (const scene of scenes) {
    const cue = cues?.scenes[scene.id]
    const length = Math.max(scene.seconds, cue ? cue.seconds + LEAD_SECONDS + TAIL_SECONDS : 0)
    slots.push({ ...scene, start, length, voiceAt: cue ? start + LEAD_SECONDS : null })
    start += length
  }
  return slots
}

export const totalSeconds = (slots: Slot[]) =>
  slots.length ? slots[slots.length - 1]!.start + slots[slots.length - 1]!.length : 0

/** The slot at `seconds`, and how far through it (0 to 1). */
export const slotAt = (slots: Slot[], seconds: number) => {
  const last = slots[slots.length - 1]
  if (!last) return null
  const slot = slots.find((s) => seconds < s.start + s.length) ?? last
  const fraction = Math.min(1, Math.max(0, (seconds - slot.start) / slot.length))
  return { slot, fraction }
}

export const loadCues = async (): Promise<Cues | null> => {
  try {
    const response = await fetch("/audio/vo/cues.json")
    if (!response.ok) return null
    return (await response.json()) as Cues
  } catch {
    return null
  }
}

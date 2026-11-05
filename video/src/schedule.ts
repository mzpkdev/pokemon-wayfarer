// When each scene starts and how long it runs, and when each of its words is
// said. A scene lasts at least its `seconds`; with a voice-over clip
// (`audio/vo/cues.json`, from `pnpm voice`) it lasts the clip plus a lead-in
// and a tail, so the picture waits for the words. Without one its words are
// paced as a voice would read them (`words.ts`), and it lasts as long as that.
import { SCENES, type Scene } from "./script"
import { paced, type Words, wordsOf } from "./words"

/** Seconds of picture before the voice starts in a scene, and after it ends, unless the scene sets its own. */
export const LEAD_SECONDS = 0.6
export const TAIL_SECONDS = 0.8

/**
 * A scene's voice clip: how long it runs, and when each letter of its words
 * (`wordsOf(narration).text`) is said, in seconds from the clip's start.
 */
export type Cue = { seconds: number; hash: string; text: string; times?: number[] }
export type Cues = { voice?: string; model?: string; scenes: Record<string, Cue> }

export type Slot = Scene & {
  start: number
  length: number
  voiceAt: number | null
  /** How long the voice clip runs, when there is one. */
  voiceSeconds: number | null
  /** The narration without its beats, as the box prints it. */
  words: Words
  /** When each letter of `words.text` is said, in seconds into the scene. */
  times: number[]
}

/** When each letter is said, from the first, and when the last is done: the voice's, or paced. */
const timing = (words: Words, cue: Cue | undefined) => {
  const own = paced(words)
  if (!cue) return own
  if (cue.times?.length === words.text.length) return { times: cue.times, end: cue.seconds }
  // A clip made before the voice kept its timing: the paced words, stretched over it.
  const stretch = own.end ? cue.seconds / own.end : 1
  return { times: own.times.map((t) => t * stretch), end: cue.seconds }
}

export const schedule = (cues: Cues | null, scenes = SCENES): Slot[] => {
  const slots: Slot[] = []
  let start = 0
  for (const scene of scenes) {
    const cue = cues?.scenes[scene.id]
    const words = wordsOf(scene.narration)
    const said = timing(words, cue)
    const lead = scene.lead ?? LEAD_SECONDS
    const tail = scene.tail ?? TAIL_SECONDS
    const length = Math.max(scene.seconds, words.text ? said.end + lead + tail : 0)
    slots.push({
      ...scene,
      start,
      length,
      voiceAt: cue ? start + lead : null,
      voiceSeconds: cue ? cue.seconds : null,
      words,
      times: said.times.map((t) => t + lead),
    })
    start += length
  }
  return slots
}

const missing = new Set<string>()

/**
 * How far through its scene (0 to 1) a moment of the timeline falls: a
 * fraction as it is, or a phrase of the narration, when the voice starts it.
 */
type Moment = number | string | { on: Moment; plus: number }

export const fractionOf = (slot: Slot, moment: Moment): number => {
  if (typeof moment === "object") return fractionOf(slot, moment.on) + moment.plus / slot.length
  if (typeof moment === "number") return moment
  const index = slot.words.text.indexOf(moment)
  if (index < 0) {
    if (!missing.has(`${slot.id}:${moment}`))
      console.warn(`${slot.id}: no "${moment}" in its narration; the moment falls at its start.`)
    missing.add(`${slot.id}:${moment}`)
    return 0
  }
  return slot.times[index]! / slot.length
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

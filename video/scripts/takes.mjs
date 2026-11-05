#!/usr/bin/env node
// Brings in voice takes made by hand (ElevenLabs' website, Voice Changer, a
// microphone): one file per scene, in any folder, named for what it says. Each
// is matched to its scene by its words, trimmed of the silence at its ends and
// copied to audio/vo/<scene>.mp3, and audio/vo/cues.json gets its length and
// when each letter is said, as `pnpm voice` writes them, so the box and the
// timeline follow the take.
//
//   pnpm takes <folder> [--dry-run]
//
// A take carries no timing of its own, so the letters are placed from its
// pauses: the silences ffmpeg finds are matched to the places the script can
// pause (its beats first, then sentence ends, then commas), and the words
// between two pauses are spread over the speech between them. A take whose
// words differ from the script still matches its scene; say so in the script
// first, since the box prints the script. Needs ffmpeg on the PATH.
import { spawnSync } from "node:child_process"
import { createHash } from "node:crypto"
import * as fs from "node:fs"
import * as path from "node:path"
import { parseArgs } from "node:util"
import { fileURLToPath } from "node:url"

const here = path.dirname(fileURLToPath(import.meta.url))
const root = path.resolve(here, "..")
const vo = path.join(root, "audio", "vo")
const cuesPath = path.join(vo, "cues.json")

const { values, positionals } = parseArgs({
  allowPositionals: true,
  options: { "dry-run": { type: "boolean", default: false } },
})
const folder = positionals[0]
if (!folder || !fs.statSync(folder, { throwIfNoEntry: false })?.isDirectory()) {
  console.error("Usage: pnpm takes <folder of takes> [--dry-run]")
  process.exit(1)
}

const { SCENES } = await import("../src/script.ts")
const { wordsOf } = await import("../src/words.ts")

/** Silence quieter than this, for at least this long, is a pause. */
const SILENCE_DB = -40
const SILENCE_SECONDS = 0.14

const ffmpeg = (args) => {
  const run = spawnSync("ffmpeg", ["-hide_banner", "-nostats", ...args], { encoding: "utf8" })
  if (run.status !== 0) throw new Error(`ffmpeg ${args.join(" ")}\n${run.stderr}`)
  return run.stderr
}

const durationOf = (file) => {
  const run = spawnSync(
    "ffprobe",
    ["-v", "error", "-show_entries", "format=duration", "-of", "csv=p=0", file],
    { encoding: "utf8" },
  )
  return Number(run.stdout.trim())
}

/** The speech in a clip: its stretches between pauses, in seconds. */
const speechOf = (file, seconds) => {
  const log = ffmpeg([
    "-i",
    file,
    "-af",
    `silencedetect=noise=${SILENCE_DB}dB:d=${SILENCE_SECONDS}`,
    "-f",
    "null",
    "-",
  ])
  const starts = [...log.matchAll(/silence_start: ([\d.]+)/g)].map((m) => Number(m[1]))
  const ends = [...log.matchAll(/silence_end: ([\d.]+)/g)].map((m) => Number(m[1]))
  const speech = []
  let at = 0
  starts.forEach((start, i) => {
    if (start > at + 0.02) speech.push({ start: at, end: start })
    at = ends[i] ?? seconds
  })
  if (seconds > at + 0.02) speech.push({ start: at, end: seconds })
  return speech
}

/** The words a text is made of, for matching a take to its scene. */
const tokens = (text) =>
  new Set(
    text
      .toLowerCase()
      .replace(/[^a-z0-9' ]+/g, " ")
      .split(/\s+/)
      .filter(Boolean),
  )
const likeness = (a, b) => {
  const shared = [...a].filter((w) => b.has(w)).length
  return shared / (a.size + b.size - shared)
}

/**
 * Where the text can pause: the index of the first letter after each place,
 * and how sure a pause is there (a beat is all but certain; a comma rarely).
 */
const pausesOf = ({ text, beats }) => {
  const places = new Map()
  const add = (i, weight) => {
    while (i < text.length && /\s/.test(text[i])) i++
    if (i > 0 && i < text.length) places.set(i, Math.max(places.get(i) ?? 0, weight))
  }
  for (const [i] of beats) add(i, 1.5)
  for (let i = 0; i < text.length - 1; i++) {
    if (text[i] === "\n") add(i + 1, 0.4)
    else if (/[.?!]/.test(text[i]) && /\s/.test(text[i + 1]) && !/\b[A-Z]$/.test(text.slice(0, i)))
      add(i + 1, 0.4)
    else if (/[,;:—]/.test(text[i]) && /\s/.test(text[i + 1])) add(i + 1, 0.06)
  }
  return [...places].toSorted((a, b) => a[0] - b[0])
}

/**
 * When each letter of `words.text` is said: the take's speech stretches are
 * grouped and matched, in order, to stretches of the text that end where it
 * can pause, choosing the grouping whose lengths best fit the letters each
 * holds; then each stretch of text is spread over its speech.
 */
const align = (words, speech) => {
  const { text } = words
  const places = pausesOf(words)
  const cuts = [0, ...places.map(([i]) => i), text.length]
  const weight = [0, ...places.map(([, w]) => w), 0]
  const letters = (from, to) => text.slice(from, to).replace(/\s/g, "").length || 1
  const talk = (from, to) => speech.slice(from, to).reduce((t, s) => t + s.end - s.start, 0)
  const rate = letters(0, text.length) / talk(0, speech.length)
  const S = speech.length
  const K = cuts.length - 1
  const best = Array.from({ length: S + 1 }, () => Array(K + 1).fill(Infinity))
  const back = Array.from({ length: S + 1 }, () => Array(K + 1).fill(null))
  best[0][0] = 0
  for (let j = 0; j < S; j++)
    for (let k = 0; k < K; k++) {
      if (best[j][k] === Infinity) continue
      for (let j2 = j + 1; j2 <= S; j2++)
        for (let k2 = k + 1; k2 <= K; k2++) {
          const said = talk(j, j2)
          const expected = letters(cuts[k], cuts[k2]) / rate
          let cost = Math.log((said + 0.15) / (expected + 0.15)) ** 2
          for (let m = k + 1; m < k2; m++) cost += weight[m] * 0.4
          for (let g = j + 1; g < j2; g++) cost += 0.15 + (speech[g].start - speech[g - 1].end)
          if (best[j][k] + cost < best[j2][k2]) {
            best[j2][k2] = best[j][k] + cost
            back[j2][k2] = [j, k]
          }
        }
    }
  const groups = []
  for (let at = [S, K]; at[0] > 0; at = back[at[0]][at[1]]) {
    const [j, k] = back[at[0]][at[1]]
    groups.unshift({ speech: speech.slice(j, at[0]), from: cuts[k], to: cuts[at[1]] })
  }
  const times = []
  for (const group of groups) {
    const total = group.speech.reduce((t, s) => t + s.end - s.start, 0)
    const count = group.to - group.from
    for (let i = 0; i < count; i++) {
      // The speech time this letter falls at, skipping the group's own pauses.
      let left = (i / count) * total
      let at = group.speech[0].start
      for (const s of group.speech) {
        const length = s.end - s.start
        if (left <= length) {
          at = s.start + left
          break
        }
        left -= length
        at = s.end
      }
      times.push(Math.round(at * 1000) / 1000)
    }
  }
  return { times, groups }
}

// Match every take to the scene whose words it shares most.
const takes = fs
  .readdirSync(folder)
  .filter((f) => /\.(mp3|wav|m4a|ogg|flac)$/i.test(f))
  .map((file) => ({ file, words: tokens(file.replace(/\.[a-z0-9]+$/i, "")) }))
const spoken = SCENES.filter((s) => s.narration.trim())
const pairs = []
for (const take of takes)
  for (const scene of spoken)
    pairs.push({ take, scene, score: likeness(take.words, tokens(scene.narration)) })
pairs.sort((a, b) => b.score - a.score)
const matched = new Map()
const used = new Set()
for (const { take, scene, score } of pairs) {
  if (matched.has(scene.id) || used.has(take.file) || score < 0.3) continue
  matched.set(scene.id, take.file)
  used.add(take.file)
}
for (const take of takes)
  if (!used.has(take.file)) console.warn(`  no scene for "${take.file}"; left out`)

fs.mkdirSync(vo, { recursive: true })
const cues = fs.existsSync(cuesPath)
  ? JSON.parse(fs.readFileSync(cuesPath, "utf8"))
  : { scenes: {} }
cues.voice = "takes"
cues.model = "takes"
const scratch = fs.mkdtempSync(path.join(root, "out", ".takes-"))
let brought = 0
for (const scene of spoken) {
  const file = matched.get(scene.id)
  if (!file) {
    console.warn(`  ${scene.id}: no take`)
    continue
  }
  // Trimmed at both ends: the schedule adds its own lead-in and tail.
  const trimmed = path.join(scratch, `${scene.id}.mp3`)
  const edge = `silenceremove=start_periods=1:start_threshold=${SILENCE_DB}dB:start_silence=0.03`
  ffmpeg([
    "-y",
    "-i",
    path.join(folder, file),
    "-af",
    `${edge},areverse,${edge},areverse`,
    "-ar",
    "44100",
    "-b:a",
    "128k",
    trimmed,
  ])
  const seconds = durationOf(trimmed)
  const words = wordsOf(scene.narration)
  const { times, groups } = align(words, speechOf(trimmed, seconds))
  console.log(`  ${scene.id} ← ${file} (${seconds.toFixed(2)}s)`)
  for (const g of groups)
    console.log(
      `      ${g.speech[0].start.toFixed(2)}–${g.speech.at(-1).end.toFixed(2)}  ${JSON.stringify(words.text.slice(g.from, g.to).trim())}`,
    )
  if (values["dry-run"]) continue
  fs.copyFileSync(trimmed, path.join(vo, `${scene.id}.mp3`))
  const hash = createHash("sha256").update(fs.readFileSync(trimmed)).digest("hex").slice(0, 16)
  cues.scenes[scene.id] = { hash, seconds, text: scene.narration, times }
  brought++
}
fs.rmSync(scratch, { recursive: true, force: true })
if (values["dry-run"]) process.exit(0)

cues.scenes = Object.fromEntries(
  SCENES.filter((s) => cues.scenes[s.id]).map((s) => [s.id, cues.scenes[s.id]]),
)
fs.writeFileSync(cuesPath, `${JSON.stringify(cues, null, 2)}\n`)
console.log(`${brought} takes brought in; ${path.relative(process.cwd(), cuesPath)}`)

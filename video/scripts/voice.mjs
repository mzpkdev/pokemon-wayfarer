#!/usr/bin/env node
// Speaks the script with ElevenLabs: one clip per scene into audio/vo/, and
// audio/vo/cues.json with each clip's length and when each letter is said,
// which the page and the render read to give every scene the time its words
// need and to print the text box along with the voice. A beat in the script
// (`|`) goes to the voice as a pause.
//
//   ELEVENLABS_API_KEY=... ELEVENLABS_VOICE_ID=... pnpm voice [--force] [--scene gyms]
//   ELEVENLABS_API_KEY=... pnpm voice --list        # the voices on the account
//
// A scene whose words, voice and model haven't changed since its clip was made
// is skipped, so re-running after a script edit only speaks what changed. The
// clips are meant to be committed, so a render needs no key.
import { createHash } from "node:crypto"
import * as fs from "node:fs"
import * as path from "node:path"
import { parseArgs } from "node:util"
import { fileURLToPath } from "node:url"

const here = path.dirname(fileURLToPath(import.meta.url))
const root = path.resolve(here, "..")
const vo = path.join(root, "audio", "vo")
const cuesPath = path.join(vo, "cues.json")

const { values } = parseArgs({
  options: {
    force: { type: "boolean", default: false },
    list: { type: "boolean", default: false },
    scene: { type: "string" },
    model: { type: "string", default: process.env.ELEVENLABS_MODEL ?? "eleven_multilingual_v2" },
  },
})

const key = process.env.ELEVENLABS_API_KEY
if (!key) {
  console.error("Set ELEVENLABS_API_KEY.")
  process.exit(1)
}
const headers = { "xi-api-key": key, "content-type": "application/json" }
const API = "https://api.elevenlabs.io/v1"

if (values.list) {
  const response = await fetch(`${API}/voices`, { headers })
  if (!response.ok) throw new Error(`Listing voices failed: HTTP ${response.status}`)
  const { voices } = await response.json()
  for (const voice of voices) {
    const labels = Object.values(voice.labels ?? {}).join(", ")
    console.log(`${voice.voice_id}  ${voice.name}${labels ? `  (${labels})` : ""}`)
  }
  process.exit(0)
}

const voiceId = process.env.ELEVENLABS_VOICE_ID
if (!voiceId) {
  console.error("Set ELEVENLABS_VOICE_ID (pnpm voice --list shows them).")
  process.exit(1)
}

// Node runs the TypeScript as is: the script and its words import nothing.
const { SCENES } = await import("../src/script.ts")
const { voiceText, wordsOf } = await import("../src/words.ts")

/**
 * When each letter of `text` is said, from ElevenLabs' alignment of `sent`
 * (the text with its pause tags). The tags, if the alignment echoes them, are
 * skipped; a letter it reads otherwise (a number spelled out, say) takes its
 * time from the letters around it.
 */
const lettersTimed = (text, { characters, character_start_times_seconds: starts }) => {
  const spoken = text.replaceAll("\n", " ")
  const tags = [...characters.join("").matchAll(/<break[^>]*\/>/g)]
  const said = characters
    .map((letter, i) => ({ letter, at: starts[i] }))
    .filter((_, i) => !tags.some((tag) => i >= tag.index && i < tag.index + tag[0].length))
  const times = []
  let j = 0
  for (const letter of spoken) {
    const found = said.slice(j, j + 8).findIndex((s) => s.letter === letter)
    if (found < 0) times.push(null)
    else {
      times.push(said[j + found].at)
      j += found + 1
    }
  }
  // Fill what wasn't matched between its neighbours.
  for (let i = 0; i < times.length; i++) {
    if (times[i] !== null) continue
    const before = times.findLast((t, k) => k < i && t !== null) ?? 0
    const after = times.find((t, k) => k > i && t !== null) ?? before
    times[i] = (before + after) / 2
  }
  return times.map((t) => Math.round(t * 1000) / 1000)
}

fs.mkdirSync(vo, { recursive: true })
const cues = fs.existsSync(cuesPath)
  ? JSON.parse(fs.readFileSync(cuesPath, "utf8"))
  : { scenes: {} }
// Hand-made takes (`pnpm takes`) can't be spoken again: replacing them is a choice.
if (cues.voice === "takes" && !values.force) {
  console.error("audio/vo/ holds hand-made takes (pnpm takes); --force replaces them.")
  process.exit(1)
}
cues.voice = voiceId
cues.model = values.model

const hashOf = (text) =>
  createHash("sha256").update(`${voiceId}\n${values.model}\n${text}`).digest("hex").slice(0, 16)

let spoke = 0
for (const scene of SCENES) {
  if (values.scene && scene.id !== values.scene) continue
  const clip = path.join(vo, `${scene.id}.mp3`)
  if (!scene.narration.trim()) {
    delete cues.scenes[scene.id]
    fs.rmSync(clip, { force: true })
    continue
  }
  const words = wordsOf(scene.narration)
  const sent = voiceText(words)
  const hash = hashOf(sent)
  const have = cues.scenes[scene.id]
  if (!values.force && have?.hash === hash && fs.existsSync(clip)) {
    console.log(`  ${scene.id}: unchanged (${have.seconds.toFixed(2)}s)`)
    continue
  }
  const response = await fetch(
    `${API}/text-to-speech/${voiceId}/with-timestamps?output_format=mp3_44100_128`,
    {
      method: "POST",
      headers,
      body: JSON.stringify({ text: sent, model_id: values.model }),
    },
  )
  if (!response.ok) throw new Error(`${scene.id}: HTTP ${response.status} ${await response.text()}`)
  const { audio_base64, alignment } = await response.json()
  fs.writeFileSync(clip, Buffer.from(audio_base64, "base64"))
  const ends = alignment?.character_end_times_seconds ?? []
  const seconds = ends.length ? ends[ends.length - 1] : 0
  const times = alignment ? lettersTimed(words.text, alignment) : undefined
  cues.scenes[scene.id] = { hash, seconds, text: scene.narration, times }
  console.log(`  ${scene.id}: ${seconds.toFixed(2)}s`)
  spoke++
}

// The cues keep the script's order, so diffs read like the storyboard.
cues.scenes = Object.fromEntries(
  SCENES.filter((s) => cues.scenes[s.id]).map((s) => [s.id, cues.scenes[s.id]]),
)
fs.writeFileSync(cuesPath, `${JSON.stringify(cues, null, 2)}\n`)
console.log(`${spoke} spoken; ${path.relative(process.cwd(), cuesPath)}`)

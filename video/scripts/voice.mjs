#!/usr/bin/env node
// Speaks the script with ElevenLabs: one clip per scene into audio/vo/, and
// audio/vo/cues.json with each clip's length, which the page and the render
// read to give every scene the time its words need.
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

// Node runs the TypeScript as is: the script's words have no types to compile.
const { SCENES } = await import("../src/script.ts")

fs.mkdirSync(vo, { recursive: true })
const cues = fs.existsSync(cuesPath)
  ? JSON.parse(fs.readFileSync(cuesPath, "utf8"))
  : { scenes: {} }
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
  const hash = hashOf(scene.narration)
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
      body: JSON.stringify({ text: scene.narration, model_id: values.model }),
    },
  )
  if (!response.ok) throw new Error(`${scene.id}: HTTP ${response.status} ${await response.text()}`)
  const { audio_base64, alignment } = await response.json()
  fs.writeFileSync(clip, Buffer.from(audio_base64, "base64"))
  const ends = alignment?.character_end_times_seconds ?? []
  const seconds = ends.length ? ends[ends.length - 1] : 0
  cues.scenes[scene.id] = { hash, seconds, text: scene.narration }
  console.log(`  ${scene.id}: ${seconds.toFixed(2)}s`)
  spoke++
}

// The cues keep the script's order, so diffs read like the storyboard.
cues.scenes = Object.fromEntries(
  SCENES.filter((s) => cues.scenes[s.id]).map((s) => [s.id, cues.scenes[s.id]]),
)
fs.writeFileSync(cuesPath, `${JSON.stringify(cues, null, 2)}\n`)
console.log(`${spoke} spoken; ${path.relative(process.cwd(), cuesPath)}`)

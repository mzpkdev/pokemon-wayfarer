#!/usr/bin/env node
// Muxes the rendered frames with the voice-over and music into the video:
//
//   pnpm render [--no-voice] [--music path] [--out out/wayfarer-launch.mp4]
//
// Reads out/frames/ and out/schedule.json from `pnpm frames`, the voice clips
// from audio/vo/ (placed where the schedule has each scene's cue) and the
// first track in audio/music/. Needs ffmpeg on the PATH.
import { spawnSync } from "node:child_process"
import * as fs from "node:fs"
import * as path from "node:path"
import { parseArgs } from "node:util"
import { fileURLToPath } from "node:url"

const here = path.dirname(fileURLToPath(import.meta.url))
const root = path.resolve(here, "..")
const out = path.join(root, "out")
const frames = path.join(out, "frames")

const { values } = parseArgs({
  options: {
    voice: { type: "boolean", default: true },
    "no-voice": { type: "boolean", default: false },
    music: { type: "string" },
    out: { type: "string", default: path.join(out, "wayfarer-launch.mp4") },
    "music-volume": { type: "string", default: "0.25" },
  },
})
const voice = values.voice && !values["no-voice"]

const schedulePath = path.join(out, "schedule.json")
if (!fs.existsSync(schedulePath) || !fs.existsSync(frames)) {
  console.error(
    "Render the frames first: out/frames/ or out/schedule.json is missing (pnpm frames).",
  )
  process.exit(1)
}
const { fps, duration, slots } = JSON.parse(fs.readFileSync(schedulePath, "utf8"))
const cuesPath = path.join(root, "audio", "vo", "cues.json")
const cues = fs.existsSync(cuesPath)
  ? JSON.parse(fs.readFileSync(cuesPath, "utf8"))
  : { scenes: {} }
const files = fs
  .readdirSync(frames)
  .filter((f) => f.endsWith(".png"))
  .toSorted()
if (!files.length) {
  console.error("out/frames/ is empty.")
  process.exit(1)
}
const firstFrame = Number(files[0].replace(".png", ""))
const start = firstFrame / fps
const length = files.length / fps

const musicDir = path.join(root, "audio", "music")
const music =
  values.music ??
  (fs.existsSync(musicDir)
    ? fs
        .readdirSync(musicDir)
        .filter((f) => /\.(mp3|m4a|wav|ogg|flac)$/i.test(f))
        .map((f) => path.join(musicDir, f))[0]
    : undefined)

const inputs = [
  "-framerate",
  String(fps),
  "-start_number",
  String(firstFrame),
  "-i",
  path.join(frames, "%06d.png"),
]
const filters = []
const mixed = []
let n = 1
if (voice)
  for (const slot of slots) {
    if (slot.voiceAt === null) continue
    const clip = path.join(root, "audio", "vo", `${slot.id}.mp3`)
    if (!fs.existsSync(clip)) continue
    const at = slot.voiceAt - start
    // A clip that ends before the rendered range, or starts after it, stays out.
    if (at + (cues.scenes[slot.id]?.seconds ?? 0) <= 0 || at > length) continue
    inputs.push("-i", clip)
    // A clip that starts before the rendered range is trimmed, not delayed.
    filters.push(
      at >= 0
        ? `[${n}:a]adelay=${Math.round(at * 1000)}:all=1[v${n}]`
        : `[${n}:a]atrim=start=${(-at).toFixed(3)},asetpts=PTS-STARTPTS[v${n}]`,
    )
    mixed.push(`[v${n}]`)
    n++
  }
if (music) {
  inputs.push("-stream_loop", "-1", "-i", music)
  const fade = Math.min(3, length / 2)
  filters.push(
    `[${n}:a]volume=${values["music-volume"]},afade=t=in:st=0:d=1,afade=t=out:st=${(length - fade).toFixed(3)}:d=${fade.toFixed(3)}[m]`,
  )
  mixed.push("[m]")
  n++
}

const args = ["-y", ...inputs]
if (mixed.length) {
  filters.push(`${mixed.join("")}amix=inputs=${mixed.length}:normalize=0:duration=longest[a]`)
  args.push(
    "-filter_complex",
    filters.join(";"),
    "-map",
    "0:v",
    "-map",
    "[a]",
    "-c:a",
    "aac",
    "-b:a",
    "192k",
  )
} else args.push("-an")
args.push(
  "-c:v",
  "libx264",
  "-pix_fmt",
  "yuv420p",
  "-crf",
  "18",
  "-preset",
  "slow",
  "-r",
  String(fps),
  "-movflags",
  "+faststart",
  "-t",
  length.toFixed(3),
  values.out,
)

fs.mkdirSync(path.dirname(values.out), { recursive: true })
console.log(
  `${files.length} frames (${length.toFixed(1)}s of ${duration.toFixed(1)}s)` +
    `${voice ? `, voice in ${mixed.length - (music ? 1 : 0)} scenes` : ", no voice"}` +
    `${music ? `, music ${path.basename(music)}` : ", no music"}`,
)
const result = spawnSync("ffmpeg", args, { stdio: ["ignore", "inherit", "inherit"] })
if (result.status !== 0) process.exit(result.status ?? 1)
console.log(`  ${path.relative(process.cwd(), values.out)}`)

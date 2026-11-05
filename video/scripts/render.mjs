#!/usr/bin/env node
// Muxes the rendered frames with the voice-over and music into the video:
//
//   pnpm render [--no-voice] [--music path] [--music-at seconds] [--duck 0.35] [--out out/wayfarer-launch.mp4]
//
// Reads out/frames/ and out/schedule.json from `pnpm frames`, the voice clips
// from audio/vo/ (placed where the schedule has each scene's cue) and the
// first track in audio/music/. A track shorter than the film starts late, so
// its own ending lands on the film's; a longer one starts with the film and
// fades out at its end. `--music-at` says where in the film it starts instead.
// The music dips to `--duck` of its level while the narrator speaks, easing
// down just before each line and back up after it.
// Needs ffmpeg on the PATH.
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
    "music-at": { type: "string" },
    "music-volume": { type: "string", default: "0.3" },
    duck: { type: "string", default: "0.35" },
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
/** Where the narrator speaks, in seconds of the rendered range, for the music to dip under. */
const spoken = []
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
    spoken.push([at, at + (cues.scenes[slot.id]?.seconds ?? 0)])
    n++
  }

/** Seconds the music takes to dip before a line, and to come back after it. */
const EASE = 0.5
/**
 * The music's level over the rendered range, as an ffmpeg expression of `t`:
 * full, then down to `duck` under each line. Lines closer than two eases stay
 * under one dip, so the music doesn't bob up between them.
 */
const duckedLevel = (base, duck) => {
  const dips = []
  for (const [from, to] of spoken.toSorted((a, b) => a[0] - b[0])) {
    const last = dips.at(-1)
    if (last && from - last[1] < 2 * EASE) last[1] = Math.max(last[1], to)
    else dips.push([from, to])
  }
  if (!dips.length) return String(base)
  // How far under a dip the music is at t: 0 clear of it, 1 inside it, eased between.
  const under = dips
    .map(
      ([from, to]) =>
        `clip(min((t-${(from - EASE).toFixed(3)})/${EASE},(${(to + EASE).toFixed(3)}-t)/${EASE}),0,1)`,
    )
    .reduce((a, b) => `max(${a},${b})`)
  return `${base}*(1-${(1 - duck).toFixed(3)}*${under})`
}
if (music) {
  const track = Number(
    spawnSync(
      "ffprobe",
      ["-v", "error", "-show_entries", "format=duration", "-of", "csv=p=0", music],
      { encoding: "utf8" },
    ).stdout.trim(),
  )
  // Where in the film the track starts, and so where in the rendered range.
  const musicAt =
    values["music-at"] !== undefined ? Number(values["music-at"]) : Math.max(0, duration - track)
  const at = musicAt - start
  const ends = at + track
  if (ends > 0 && at < length) {
    inputs.push("-i", music)
    const steps = []
    // Started before the rendered range: trimmed to it, not delayed.
    if (at < 0) steps.push(`atrim=start=${(-at).toFixed(3)}`, "asetpts=PTS-STARTPTS")
    else steps.push(`adelay=${Math.round(at * 1000)}:all=1`)
    // After the delay, so its clock is the rendered range's, as the lines are.
    const level = duckedLevel(Number(values["music-volume"]), Number(values.duck))
    steps.push(`volume='${level}':eval=frame`)
    // A track that outlasts the range fades out with it; one that ends inside it ends as it was made.
    if (ends > length + 0.05) {
      const fade = Math.min(3, length / 2)
      steps.push(`afade=t=out:st=${(length - fade).toFixed(3)}:d=${fade.toFixed(3)}`)
    }
    filters.push(`[${n}:a]${steps.join(",")}[m]`)
    mixed.push("[m]")
    n++
    console.log(
      `  music from ${musicAt.toFixed(2)}s of the film to ${(musicAt + track).toFixed(2)}s`,
    )
  }
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
    `${voice ? `, voice in ${mixed.filter((m) => m !== "[m]").length} scenes` : ", no voice"}` +
    `${music ? `, music ${path.basename(music)}` : ", no music"}`,
)
const result = spawnSync("ffmpeg", args, { stdio: ["ignore", "inherit", "inherit"] })
if (result.status !== 0) process.exit(result.status ?? 1)
console.log(`  ${path.relative(process.cwd(), values.out)}`)

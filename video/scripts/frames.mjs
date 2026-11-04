#!/usr/bin/env node
// Renders the video's frames: the built page in a headless browser whose clock
// this script turns by hand, one frame at a time, screenshotting each into
// out/frames/. Nothing depends on how fast the machine draws.
//
//   pnpm build && pnpm frames [--fps 30] [--from 12] [--to 20] [--scene gyms]
//
// Writes out/schedule.json too, which `render.mjs` reads to place the voice.
// Playwright's own Chromium is used when installed; otherwise set CHROME.
import * as fs from "node:fs"
import * as path from "node:path"
import { parseArgs } from "node:util"
import { fileURLToPath } from "node:url"

import { chromium } from "playwright"

import { serve } from "./serve.mjs"

const here = path.dirname(fileURLToPath(import.meta.url))
const root = path.resolve(here, "..")
const dist = path.join(root, "dist")
const out = path.join(root, "out")
const frames = path.join(out, "frames")

const { values } = parseArgs({
  options: {
    fps: { type: "string", default: "30" },
    from: { type: "string" },
    to: { type: "string" },
    scene: { type: "string" },
    width: { type: "string", default: "1920" },
    height: { type: "string", default: "1080" },
  },
})
const fps = Number(values.fps)
const width = Number(values.width)
const height = Number(values.height)

if (!fs.existsSync(path.join(dist, "index.html"))) {
  console.error("Build the page first: dist/index.html is missing (pnpm build).")
  process.exit(1)
}
const cues = path.join(root, "audio", "vo", "cues.json")
if (
  fs.existsSync(cues) &&
  fs.statSync(cues).mtimeMs > fs.statSync(path.join(dist, "index.html")).mtimeMs
)
  console.warn(
    "audio/vo/cues.json is newer than the build: run pnpm build first, or the scenes keep their old lengths.",
  )

const { server, url } = await serve(dist)
const browser = await chromium.launch(
  process.env.CHROME ? { executablePath: process.env.CHROME } : {},
)
try {
  const page = await browser.newPage({ viewport: { width, height }, deviceScaleFactor: 1 })
  page.on("pageerror", (error) => console.error("page error:", error.message))
  // The page's clocks (performance.now, requestAnimationFrame, timers) are
  // ours from the first script on: they move only when this script turns them.
  await page.clock.install({ time: new Date("2027-01-20T10:00:00Z") })
  await page.goto(`${url}?paused&capture`, { waitUntil: "networkidle" })
  await page.waitForFunction(() => globalThis.wayfarerVideo?.ready === true, null, {
    timeout: 60_000,
  })
  await page.evaluate(() => document.fonts.ready)
  // The page's clock ran in real time while it loaded; pause it just past where it got to.
  await page.clock.pauseAt((await page.evaluate(() => Date.now())) + 1000)

  const { duration, slots } = await page.evaluate(() => {
    const video = globalThis.wayfarerVideo
    return { duration: video.duration, slots: video.slots }
  })
  let from = values.from ? Number(values.from) : 0
  let to = values.to ? Number(values.to) : duration
  if (values.scene) {
    const slot = slots.find((s) => s.id === values.scene)
    if (!slot) throw new Error(`No scene named ${values.scene}`)
    from = slot.start
    to = slot.start + slot.length
  }
  const first = Math.round(from * fps)
  const last = Math.ceil(to * fps)
  const total = last - first
  fs.rmSync(frames, { recursive: true, force: true })
  fs.mkdirSync(frames, { recursive: true })
  fs.writeFileSync(
    path.join(out, "schedule.json"),
    JSON.stringify({ fps, duration, slots }, null, 2),
  )
  console.log(
    `${total} frames at ${fps} fps, ${from.toFixed(2)}s to ${to.toFixed(2)}s of ${duration.toFixed(2)}s`,
  )

  const started = Date.now()
  // The clock moves in whole milliseconds, so each frame takes it to the
  // rounded total rather than a rounded step: 33, 34, 33 ms, not 34 every time.
  let ran = 0
  for (let n = first; n < last; n++) {
    const seconds = n / fps
    await page.evaluate((s) => globalThis.wayfarerVideo.seek(s), seconds)
    // A frame's worth of time passes, so the animation frame fires and the
    // sea, clouds and walkers move on; a frame that left chunks to draw gets another.
    const target = Math.round(((n - first + 1) * 1000) / fps)
    await page.clock.runFor(target - ran)
    ran = target
    for (let settle = 0; settle < 4; settle++) {
      if (!(await page.evaluate(() => globalThis.wayfarerVideo.pending()))) break
      await page.clock.runFor(1)
    }
    await page.screenshot({
      path: path.join(frames, `${String(n).padStart(6, "0")}.png`),
      clip: { x: 0, y: 0, width, height },
    })
    if ((n - first) % Math.max(1, Math.round(fps)) === 0) {
      const done = n - first + 1
      const rate = done / Math.max(1, (Date.now() - started) / 1000)
      process.stdout.write(`\r  ${done}/${total}  ${rate.toFixed(1)} frames/s   `)
    }
  }
  console.log(`\n  ${path.relative(process.cwd(), frames)}/ ${total} frames`)
} finally {
  await browser.close()
  server.close()
}

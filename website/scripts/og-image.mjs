#!/usr/bin/env node
// Renders the social preview image, public/og.png: the built site's title
// screen at 1200x630, the size link previews ask for. The scene is rendered
// settled (as with reduced motion), so the logo is up and PRESS START shows.
//
// Build the site first (`pnpm build`); this serves dist/ to a headless
// browser. Playwright's own Chromium is used when installed; otherwise set
// CHROME to a Chrome or Chromium executable.
import * as fs from "node:fs"
import * as http from "node:http"
import * as path from "node:path"
import { fileURLToPath } from "node:url"

import { chromium } from "playwright"

const here = path.dirname(fileURLToPath(import.meta.url))
const dist = path.resolve(here, "../dist")
const out = path.resolve(here, "../public/og.png")

const TYPES = {
  ".html": "text/html",
  ".js": "text/javascript",
  ".css": "text/css",
  ".png": "image/png",
  ".svg": "image/svg+xml",
  ".ttf": "font/ttf",
  ".json": "application/json",
}

if (!fs.existsSync(path.join(dist, "index.html"))) {
  console.error("Build the site first: dist/index.html is missing.")
  process.exit(1)
}

const server = http.createServer((request, response) => {
  let pathname = decodeURIComponent(new URL(request.url, "http://localhost").pathname)
  if (pathname.endsWith("/")) pathname += "index.html"
  const file = path.join(dist, pathname)
  if (!file.startsWith(dist) || !fs.existsSync(file) || fs.statSync(file).isDirectory()) {
    response.statusCode = 404
    response.end()
    return
  }
  response.setHeader("content-type", TYPES[path.extname(file)] ?? "application/octet-stream")
  fs.createReadStream(file).pipe(response)
})
await new Promise((resolve) => server.listen(0, "127.0.0.1", resolve))
const { port } = server.address()

const browser = await chromium.launch(
  process.env.CHROME ? { executablePath: process.env.CHROME } : {},
)
try {
  const page = await browser.newPage({ viewport: { width: 1200, height: 630 } })
  await page.emulateMedia({ reducedMotion: "reduce" })
  await page.goto(`http://127.0.0.1:${port}/`, { waitUntil: "networkidle" })
  // The art loads, then one frame draws it.
  await page.waitForTimeout(1500)
  await page.screenshot({ path: out })
  console.log(`  ${path.relative(process.cwd(), out)} 1200x630`)
} finally {
  await browser.close()
  server.close()
}

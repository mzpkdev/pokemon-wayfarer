#!/usr/bin/env node
// Stills of the world for scouting a scene's locations: the built page's scout
// view (`src/Scout.tsx`), one picture per query, each tile's coordinates drawn.
//
//   pnpm build && pnpm scout "map=Route30_hns&x=8&y=20&zoom=48&day=10" ["..."]
//
// Writes out/scout/1.png, 2.png, ... in the order given.
import * as fs from "node:fs"
import * as path from "node:path"
import { fileURLToPath } from "node:url"

import { chromium } from "playwright"

import { serve } from "./serve.mjs"

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..")
const out = path.join(root, "out", "scout")
const queries = process.argv.slice(2)
if (!queries.length) {
  console.error('Give at least one query, e.g. "map=PalletTown_hns&zoom=32".')
  process.exit(1)
}
fs.mkdirSync(out, { recursive: true })
const { server, url } = await serve(path.join(root, "dist"))
const browser = await chromium.launch(
  process.env.CHROME ? { executablePath: process.env.CHROME } : {},
)
try {
  const page = await browser.newPage({ viewport: { width: 1920, height: 1080 } })
  page.on("pageerror", (error) => console.error("page error:", error.message))
  for (const [index, query] of queries.entries()) {
    await page.goto(`${url}?scout&${query}`, { waitUntil: "networkidle" })
    await page.waitForFunction(() => globalThis.wayfarerScout?.pending === false, null, {
      timeout: 60_000,
    })
    await page.waitForTimeout(300)
    const file = path.join(out, `${index + 1}.png`)
    await page.screenshot({ path: file })
    console.log(`  ${path.relative(process.cwd(), file)}  ${query}`)
  }
} finally {
  await browser.close()
  server.close()
}

import * as fs from "node:fs"
import * as path from "node:path"
import { fileURLToPath } from "node:url"

import tailwindcss from "@tailwindcss/vite"
import react from "@vitejs/plugin-react"
import { defineConfig, type Plugin } from "vite"

const here = path.dirname(fileURLToPath(import.meta.url))
const website = path.resolve(here, "../website")

/**
 * The voice-over clips and gameplay clips live beside this config, not under
 * the website's `public/`: serve them in dev and copy them into the build.
 */
const extras = (dirs: string[]): Plugin => ({
  name: "wayfarer-video-extras",
  configureServer(server) {
    server.middlewares.use((request, response, next) => {
      const url = decodeURIComponent((request.url ?? "").split("?")[0] ?? "")
      const dir = dirs.find((d) => url.startsWith(`/${d}/`))
      if (!dir) return next()
      const file = path.join(here, url)
      if (!file.startsWith(path.join(here, dir))) return next()
      // A clip or voice clip that isn't there is a 404, not the page (Vite's fallback).
      if (!fs.existsSync(file) || fs.statSync(file).isDirectory()) {
        response.statusCode = 404
        return response.end()
      }
      response.setHeader("content-type", TYPES[path.extname(file)] ?? "application/octet-stream")
      fs.createReadStream(file).pipe(response)
    })
  },
  closeBundle() {
    for (const dir of dirs) {
      const from = path.join(here, dir)
      if (fs.existsSync(from)) fs.cpSync(from, path.join(here, "dist", dir), { recursive: true })
    }
  },
})

const TYPES: Record<string, string> = {
  ".mp3": "audio/mpeg",
  ".json": "application/json",
  ".png": "image/png",
}

export default defineConfig({
  plugins: [react(), tailwindcss(), extras(["audio", "clips"])],
  // The map's data and the game's art are the website's; the page reads them at the same paths.
  publicDir: path.join(website, "public"),
  resolve: {
    alias: { "~": path.join(website, "src") },
  },
  server: {
    fs: { allow: [here, website] },
  },
  build: {
    // The frames are rendered from this build at one size; no need to split or hash for a CDN.
    assetsInlineLimit: 0,
  },
})

import * as fs from "node:fs"
import * as path from "node:path"
import * as url from "node:url"

import tailwindcss from "@tailwindcss/vite"
import { svelte } from "@sveltejs/vite-plugin-svelte"
import type { Plugin, ViteDevServer } from "vite"
import { defineConfig } from "webanvil"

import { codeStyle } from "../webanvil.shared.js"

const catalogDirectory = url.fileURLToPath(
  new url.URL("../../build/cartographer/map-catalog", import.meta.url),
)

const catalogFile = (requestUrl: string | undefined): string | null => {
  try {
    const pathname = decodeURIComponent(new URL(requestUrl ?? "/", "http://localhost").pathname)
    const file = path.resolve(catalogDirectory, `.${pathname}`)
    return file.startsWith(`${catalogDirectory}${path.sep}`) ? file : null
  } catch {
    return null
  }
}

export const generatedCatalogContentType = (file: string): string => {
  switch (path.extname(file)) {
    case ".css":
      return "text/css; charset=utf-8"
    case ".html":
      return "text/html; charset=utf-8"
    case ".js":
      return "text/javascript; charset=utf-8"
    case ".json":
      return "application/json; charset=utf-8"
    case ".png":
      return "image/png"
    default:
      return "application/octet-stream"
  }
}

type MiddlewareServer = Pick<ViteDevServer, "middlewares">

const registerGeneratedCatalog = (server: MiddlewareServer) => {
  server.middlewares.use((request, response, next) => {
    if (request.method !== "GET" && request.method !== "HEAD") return next()
    const file = catalogFile(request.url)
    if (!file) return next()

    try {
      const stats = fs.statSync(file)
      if (!stats.isFile()) return next()

      response.setHeader("Cache-Control", "no-cache")
      response.setHeader("Content-Length", stats.size)
      response.setHeader("Content-Type", generatedCatalogContentType(file))
      if (request.method === "HEAD") return response.end()
      return fs.createReadStream(file).on("error", next).pipe(response)
    } catch (error) {
      if (error instanceof Error && "code" in error && error.code === "ENOENT") return next()
      return next(error)
    }
  })
}

const previewGeneratedCatalog: Plugin = {
  name: "wayfarer-preview-generated-catalog",
  configureServer: (server) => {
    registerGeneratedCatalog(server)
  },
  configurePreviewServer: (server) => {
    registerGeneratedCatalog(server)
  },
}

export default defineConfig({
  ...codeStyle,
  build: {
    mode: "web",
    entry: "index.html",
    outDir: "dist",
  },
  plugins: [tailwindcss(), svelte()],
  vite: {
    base: "./",
    publicDir: false,
    build: {
      copyPublicDir: false,
    },
    plugins: [previewGeneratedCatalog],
  },
  test: {
    exclude: ["e2e/**"],
  },
})

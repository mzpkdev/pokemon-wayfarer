// Serves a built folder to the headless browser on a free local port.
import * as fs from "node:fs"
import * as http from "node:http"
import * as path from "node:path"

const TYPES = {
  ".html": "text/html",
  ".js": "text/javascript",
  ".css": "text/css",
  ".png": "image/png",
  ".svg": "image/svg+xml",
  ".ttf": "font/ttf",
  ".woff2": "font/woff2",
  ".json": "application/json",
  ".mp3": "audio/mpeg",
  ".gz": "application/gzip",
}

/** Serves `dist` on 127.0.0.1; resolves to the server and its root URL. */
export const serve = async (dist) => {
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
  return { server, url: `http://127.0.0.1:${server.address().port}/` }
}

import * as fs from "node:fs"
import * as path from "node:path"
import * as url from "node:url"

import { stageStaticSite } from "./site.mjs"

export const pagesSiteLimitBytes = 950 * 1024 * 1024

const escapeHtml = (value) =>
  value.replaceAll("&", "&amp;").replaceAll("<", "&lt;").replaceAll(">", "&gt;")

const validatePreviewNumber = (value) => {
  if (!/^\d+$/.test(value) || Number(value) < 1) {
    throw new Error(`Invalid pull request number: ${JSON.stringify(value)}`)
  }
  return Number(value)
}

export const previewNumbersFromTsv = (contents) => {
  const previews = new Set()
  for (const line of contents.split(/\r?\n/)) {
    if (!line) continue
    const [number] = line.split("\t", 1)
    previews.add(validatePreviewNumber(number))
  }
  return [...previews].toSorted((left, right) => left - right)
}

export const previewIndexHtml = (previews) => {
  const items = previews
    .map((number) => {
      const label = `Pull request #${number}`
      return `      <li><a href="preview/pr-${number}/">${escapeHtml(label)}</a></li>`
    })
    .join("\n")
  const content = items || "      <li>No same-repository pull requests are currently open.</li>"
  return `<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="robots" content="noindex">
    <title>Pokémon Wayfarer previews</title>
  </head>
  <body>
    <main>
      <h1>Pokémon Wayfarer previews</h1>
      <p>Open Devtools previews.</p>
      <ul>
${content}
      </ul>
    </main>
  </body>
</html>
`
}

export const writePreviewIndex = ({ output, previews }) => {
  const outputDirectory = path.resolve(output)
  fs.mkdirSync(outputDirectory, { recursive: true })
  fs.writeFileSync(path.join(outputDirectory, "index.html"), previewIndexHtml(previews))
}

const filesIn = (directory) => {
  let total = 0
  for (const entry of fs.readdirSync(directory, { withFileTypes: true })) {
    const target = path.join(directory, entry.name)
    if (entry.isDirectory()) {
      total += filesIn(target)
    } else if (entry.isFile()) {
      total += fs.statSync(target).size
    } else {
      throw new Error(`GitHub Pages artifacts cannot contain ${entry.name}.`)
    }
  }
  return total
}

/** Return the artifact size or fail before GitHub Pages' 1 GB published-site limit. */
export const assertPagesSiteSize = ({ directory, limitBytes = pagesSiteLimitBytes }) => {
  const root = path.resolve(directory)
  if (!fs.statSync(root).isDirectory()) {
    throw new Error(`The Pages site must be a directory: ${root}`)
  }
  const size = filesIn(root)
  if (size > limitBytes) {
    throw new Error(
      `Pages artifact is ${size} bytes, above the ${limitBytes}-byte deployment limit.`,
    )
  }
  return size
}

const optionValue = (arguments_, name) => {
  const index = arguments_.indexOf(name)
  if (index === -1) return null
  const value = arguments_[index + 1]
  if (!value || value.startsWith("--")) throw new Error(`${name} requires a value.`)
  if (arguments_.indexOf(name, index + 1) !== -1) throw new Error(`${name} may only be provided once.`)
  return value
}

const main = () => {
  const [command, ...arguments_] = process.argv.slice(2)
  if (!command) throw new Error("Choose index, stage, or guard.")

  if (command === "index") {
    const output = optionValue(arguments_, "--output")
    const previews = optionValue(arguments_, "--previews")
    if (!output || !previews || arguments_.length !== 4) {
      throw new Error("Usage: pages-preview.mjs index --output <directory> --previews <tsv>")
    }
    writePreviewIndex({ output, previews: previewNumbersFromTsv(fs.readFileSync(previews, "utf8")) })
    return
  }

  if (command === "stage") {
    const source = optionValue(arguments_, "--source")
    const output = optionValue(arguments_, "--output")
    if (!source || !output || arguments_.length !== 4) {
      throw new Error("Usage: pages-preview.mjs stage --source <directory> --output <directory>")
    }
    stageStaticSite({ source, output })
    return
  }

  if (command === "guard") {
    const directory = optionValue(arguments_, "--directory")
    if (!directory || arguments_.length !== 2) {
      throw new Error("Usage: pages-preview.mjs guard --directory <directory>")
    }
    const size = assertPagesSiteSize({ directory })
    console.log(`Pages artifact size: ${size} bytes.`)
    return
  }

  throw new Error(`Unknown pages preview command: ${command}`)
}

if (process.argv[1] && path.resolve(process.argv[1]) === url.fileURLToPath(import.meta.url)) {
  main()
}

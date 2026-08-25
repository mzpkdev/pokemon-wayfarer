import * as fs from "node:fs"
import * as path from "node:path"
import { defineCommand, terminal } from "cmdore"

import maps from "../arguments/maps"
import allExteriors from "../options/all-exteriors"
import catalog from "../options/catalog"
import output from "../options/output"
import repo from "../options/repo"
import { renderCatalog } from "../catalog"
import { discoverExteriorMaps, renderMap } from "../renderer"

const findRepositoryRoot = (start: string): string => {
  let current = path.resolve(start)
  while (true) {
    if (
      fs.existsSync(path.resolve(current, "data/maps")) &&
      fs.existsSync(path.resolve(current, "data/layouts/layouts.json"))
    ) {
      return current
    }
    const parent = path.dirname(current)
    if (parent === current) {
      throw new Error("cannot find a Pokémon Wayfarer source tree; pass --repo <path>")
    }
    current = parent
  }
}

export default defineCommand({
  name: "render",
  description: "Render static terrain PNGs from Pokémon Wayfarer map sources",
  arguments: [maps],
  options: [allExteriors, catalog, output, repo],
  run: ({
    maps: mapNames,
    "all-exteriors": renderAllExteriors,
    catalog: renderCatalogFile,
    output: outputPath,
    repo: repository,
  }) => {
    if (renderCatalogFile && (renderAllExteriors || mapNames.length > 0)) {
      throw new Error("--catalog cannot be combined with map names or --all-exteriors")
    }
    if (renderAllExteriors && mapNames.length > 0) {
      throw new Error("pass map names or --all-exteriors, not both")
    }
    if (!renderCatalogFile && !renderAllExteriors && mapNames.length === 0) {
      throw new Error("provide at least one map name, use --all-exteriors, or use --catalog")
    }

    const root = repository ? path.resolve(repository) : findRepositoryRoot(process.cwd())
    if (renderCatalogFile) {
      const outputDirectory = path.resolve(
        root,
        outputPath === "build/map-renders" ? "build/cartographer/map-catalog" : outputPath,
      )
      const result = renderCatalog(root, outputDirectory)
      terminal.log(`rendered catalog for ${result.mapCount} map(s) to ${result.output}`)
      return
    }
    const targets = renderAllExteriors ? discoverExteriorMaps(root) : [...new Set(mapNames)]
    const outputDirectory = path.resolve(root, outputPath)
    for (const mapName of targets) {
      const dimensions = renderMap(root, mapName, path.resolve(outputDirectory, `${mapName}.png`))
      terminal.log(`${mapName}: ${dimensions.width}x${dimensions.height} metatiles`)
    }
    terminal.log(`rendered ${targets.length} map(s) to ${outputDirectory}`)
  },
})

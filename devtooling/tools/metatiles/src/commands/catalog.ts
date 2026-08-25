import * as path from "node:path"
import { defineCommand, terminal } from "cmdore"

import { buildMetatileCatalog } from "../catalog"
import output from "../options/output"
import repo from "../options/repo"
import { findRepositoryRoot } from "../source"

export default defineCommand({
  name: "catalog",
  description: "Generate a source-driven metatile catalog and palette-correct context atlases",
  options: [output, repo],
  run: ({ output: outputPath, repo: repository }) => {
    const root = repository ? path.resolve(repository) : findRepositoryRoot(process.cwd())
    const result = buildMetatileCatalog(root, path.resolve(root, outputPath))
    terminal.log(
      `rendered ${result.metatileCount} metatile(s) across ${result.contextCount} context(s) to ${result.output}`,
    )
  },
})

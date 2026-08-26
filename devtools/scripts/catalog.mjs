import * as childProcess from "node:child_process"
import * as fs from "node:fs"
import * as path from "node:path"
import * as url from "node:url"

const devtoolsRoot = path.resolve(path.dirname(url.fileURLToPath(import.meta.url)), "..")
const repositoryRoot = path.resolve(devtoolsRoot, "..")
const gameRoot = path.join(repositoryRoot, "game")
const catalogDirectory = path.join(repositoryRoot, "build/cartographer/map-catalog")

const run = (arguments_) => {
  childProcess.execFileSync(process.execPath, arguments_, {
    cwd: devtoolsRoot,
    stdio: "inherit",
  })
}

export const generateCatalog = () => {
  fs.rmSync(catalogDirectory, { force: true, recursive: true })
  run([
    path.join(devtoolsRoot, "tools/cartographer/dist/index.js"),
    "--repo",
    gameRoot,
    "--catalog",
    "--output",
    catalogDirectory,
  ])
  run([
    path.join(devtoolsRoot, "tools/metatiles/dist/index.js"),
    "--repo",
    gameRoot,
    "--output",
    path.join(catalogDirectory, "metatiles"),
  ])
}

const main = () => {
  const arguments_ = process.argv.slice(2)
  if (arguments_.length > 0) {
    throw new Error(`Unknown catalog option(s): ${arguments_.join(" ")}`)
  }
  generateCatalog()
  console.log("Generated Cartographer and Metatiles catalogs.")
}

if (process.argv[1] && path.resolve(process.argv[1]) === url.fileURLToPath(import.meta.url)) {
  main()
}

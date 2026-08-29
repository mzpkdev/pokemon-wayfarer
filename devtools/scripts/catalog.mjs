import * as childProcess from "node:child_process"
import * as fs from "node:fs"
import * as path from "node:path"
import * as url from "node:url"

const devtoolsRoot = path.resolve(path.dirname(url.fileURLToPath(import.meta.url)), "..")
const repositoryRoot = path.resolve(devtoolsRoot, "..")
const gameRoot = path.join(repositoryRoot, "game")
const catalogDirectory = path.join(repositoryRoot, "build/cartographer/map-catalog")
const wildEncounterProjection = path.join(
  repositoryRoot,
  "build/cartographer/wild-encounter-projection.json",
)

const run = (arguments_) => {
  childProcess.execFileSync(process.execPath, arguments_, {
    cwd: devtoolsRoot,
    stdio: "inherit",
  })
}

const requireCommand = (command, label) => {
  const result = childProcess.spawnSync(command, ["--version"], {
    cwd: gameRoot,
    stdio: "ignore",
  })
  if (result.error?.code === "ENOENT") {
    throw new Error(
      `Cannot generate the devtools catalog: ${label} command ${JSON.stringify(command)} was not found. See devtools/README.md prerequisites.`,
    )
  }
  if (result.error || result.status !== 0) {
    throw new Error(
      `Cannot generate the devtools catalog: ${label} command ${JSON.stringify(command)} is not runnable. See devtools/README.md prerequisites.`,
      { cause: result.error },
    )
  }
}

export const assertCatalogPrerequisites = () => {
  requireCommand("python3", "Python 3")
  requireCommand(process.env.CPP || "cpp", "C preprocessor")
}

export const generateCatalog = () => {
  assertCatalogPrerequisites()
  fs.rmSync(catalogDirectory, { force: true, recursive: true })
  fs.mkdirSync(path.dirname(wildEncounterProjection), { recursive: true })
  childProcess.execFileSync(
    "python3",
    [
      path.join(gameRoot, "tools/wild_encounters/wild_encounters_to_header.py"),
      "--cartographer-projection",
      wildEncounterProjection,
    ],
    { cwd: gameRoot, stdio: "inherit" },
  )
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

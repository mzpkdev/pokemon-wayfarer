import * as fs from "node:fs"
import * as path from "node:path"
import * as url from "node:url"

const devtoolsRoot = path.resolve(path.dirname(url.fileURLToPath(import.meta.url)), "..")
const repositoryRoot = path.resolve(devtoolsRoot, "..")

export const catalogDirectory = path.join(repositoryRoot, "build/cartographer/map-catalog")
export const uiBuildDirectory = path.join(devtoolsRoot, "ui/dist")

const requireDirectory = (directory, label) => {
  if (!fs.statSync(directory).isDirectory()) {
    throw new Error(`${label} must be a directory: ${directory}`)
  }
}

const requireFile = (file, label) => {
  if (!fs.statSync(file).isFile()) {
    throw new Error(`${label} must be a file: ${file}`)
  }
}

/** Copy a built UI into a generated catalog without copying the catalog itself. */
export const stageStaticSite = ({ source, output }) => {
  const sourceDirectory = path.resolve(source)
  const outputDirectory = path.resolve(output)
  if (sourceDirectory === outputDirectory) {
    throw new Error("The UI build and static site output must be different directories.")
  }
  requireDirectory(sourceDirectory, "The UI build")
  requireFile(path.join(sourceDirectory, "index.html"), "The UI build index")
  requireDirectory(outputDirectory, "The generated catalog")
  requireFile(path.join(outputDirectory, "catalog.json"), "The generated map catalog")

  fs.cpSync(sourceDirectory, outputDirectory, { recursive: true, force: true })
  requireFile(path.join(outputDirectory, "index.html"), "The static site index")
  return outputDirectory
}

const main = () => {
  if (process.argv.length !== 2) {
    throw new Error("This command does not accept arguments.")
  }
  const output = stageStaticSite({ source: uiBuildDirectory, output: catalogDirectory })
  console.log(`Staged static Cartographer site at ${output}.`)
}

if (process.argv[1] && path.resolve(process.argv[1]) === url.fileURLToPath(import.meta.url)) {
  main()
}

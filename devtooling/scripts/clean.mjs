import * as fs from "node:fs"
import * as path from "node:path"
import * as url from "node:url"

const repositoryRoot = path.resolve(path.dirname(url.fileURLToPath(import.meta.url)), "../..")
const buildRoot = path.join(repositoryRoot, "build")

const assertGeneratedDirectory = (directory, rootBuild = buildRoot) => {
  const relativeDirectory = path.relative(rootBuild, directory)
  if (
    relativeDirectory.length === 0 ||
    relativeDirectory.startsWith("..") ||
    path.isAbsolute(relativeDirectory)
  ) {
    throw new Error(`Refusing to remove a directory outside ${rootBuild}: ${directory}`)
  }
}

export const cleanGeneratedArtifacts = ({ root = repositoryRoot } = {}) => {
  const rootBuild = path.join(root, "build")
  const generatedDirectories = [
    path.join(rootBuild, "cartographer"),
    path.join(rootBuild, "metatiles"),
  ]
  for (const directory of generatedDirectories) {
    assertGeneratedDirectory(directory, rootBuild)
    fs.rmSync(directory, { force: true, recursive: true })
  }
}

export const cleanWorkspace = ({ root = repositoryRoot } = {}) => {
  cleanGeneratedArtifacts({ root })
}

if (process.argv[1] && path.resolve(process.argv[1]) === url.fileURLToPath(import.meta.url)) {
  cleanWorkspace()
}

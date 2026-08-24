import * as childProcess from "node:child_process"
import * as fs from "node:fs"

import { skyEmuDirectory } from "./paths.js"

const revision = "46efbcbdb3b902373a09f4724e6d3b1a5acc4af3"
const repository = "https://github.com/skylersaleh/SkyEmu.git"

const run = (command: string, args: string[]): void => {
  const result = childProcess.spawnSync(command, args, { cwd: skyEmuDirectory, encoding: "utf8" })
  if (result.status === 0) return
  throw new Error([result.error?.message, result.stdout, result.stderr].filter(Boolean).join("\n"))
}

await fs.promises.mkdir(skyEmuDirectory, { recursive: true })
if (!fs.existsSync(`${skyEmuDirectory}/.git`)) {
  run("git", ["init"])
  run("git", ["remote", "add", "origin", repository])
}

run("git", ["fetch", "--depth=1", "origin", revision])
run("git", ["checkout", "--detach", "--force", "FETCH_HEAD"])
run("cmake", [
  "-S",
  ".",
  "-B",
  "build",
  "-DUSE_SYSTEM_CURL=ON",
  "-DUSE_SYSTEM_OPENSSL=ON",
  "-DUSE_SYSTEM_SDL2=ON",
])
run("cmake", ["--build", "build", "--parallel"])

import { existsSync } from "node:fs"
import { mkdir } from "node:fs/promises"
import { spawnSync } from "node:child_process"

import { skyEmuDirectory } from "./paths.js"

const revision = "46efbcbdb3b902373a09f4724e6d3b1a5acc4af3"
const repository = "https://github.com/skylersaleh/SkyEmu.git"
const httpModePatch = `diff --git a/src/main.c b/src/main.c
--- a/src/main.c
+++ b/src/main.c
@@ -8883,1 +8882,0 @@ sapp_desc sokol_main(int argc, char* argv[]) {
-  if(emu_state.cmd_line_arg_count >3&&strcmp("http_server",emu_state.cmd_line_args[1])==0)headless_mode();
`

const run = (command: string, args: string[], input?: string): void => {
  const result = spawnSync(command, args, { cwd: skyEmuDirectory, encoding: "utf8", input })
  if (result.status === 0) return
  throw new Error([result.error?.message, result.stdout, result.stderr].filter(Boolean).join("\n"))
}

await mkdir(skyEmuDirectory, { recursive: true })
if (!existsSync(`${skyEmuDirectory}/.git`)) {
  run("git", ["init"])
  run("git", ["remote", "add", "origin", repository])
}

run("git", ["fetch", "--depth=1", "origin", revision])
run("git", ["checkout", "--detach", "--force", "FETCH_HEAD"])
run("git", ["apply", "--check", "--unidiff-zero", "--verbose", "-"], httpModePatch)
run("git", ["apply", "--unidiff-zero", "--verbose", "-"], httpModePatch)
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

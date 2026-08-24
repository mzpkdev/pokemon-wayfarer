import { defineConfig } from "webanvil"

export default defineConfig({
  build: {
    mode: "node",
    entries: { "./setup-skyemu": "src/setup-skyemu.ts" },
    outDir: "dist",
    bundle: true,
    platform: "node",
    target: "node24",
  },
  format: {
    semi: false,
  },
  test: {
    fileParallelism: false,
    hookTimeout: 60_000,
    testTimeout: 60_000,
  },
})

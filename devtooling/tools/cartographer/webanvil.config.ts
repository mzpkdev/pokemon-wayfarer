import { defineConfig } from "webanvil"

import { codeStyle } from "../../webanvil.shared.js"

export default defineConfig({
  ...codeStyle,
  build: {
    mode: "node",
    entries: { ".": "src/index.ts", "./renderer": "src/renderer/index.ts" },
    outDir: "dist",
    bundle: true,
  },
})

import { defineConfig } from "webanvil"

export default defineConfig({
  format: {
    semi: false,
  },
  lint: {
    rules: {
      "func-style": ["error", "expression"],
      "prefer-arrow-callback": "error",
      "typescript/consistent-type-definitions": ["error", "type"],
      "unicorn/prefer-node-protocol": "error",
      "unicorn/import-style": [
        "error",
        {
          styles: {
            "node:child_process": { default: false, named: false, namespace: true },
            "node:crypto": { default: false, named: false, namespace: true },
            "node:fs": { default: false, named: false, namespace: true },
            "node:net": { default: false, named: false, namespace: true },
            "node:path": { default: false, named: false, namespace: true },
            "node:url": { default: false, named: false, namespace: true },
          },
        },
      ],
    },
  },
  test: {
    fileParallelism: true,
    hookTimeout: 60_000,
    testTimeout: 120_000,
  },
})

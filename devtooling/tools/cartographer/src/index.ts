#!/usr/bin/env node

import { execute } from "cmdore"

import renderCommand from "./commands/render"

await execute(renderCommand, {
  metadata: {
    name: "wcartographer",
    version: "0.0.0",
    description: "Render Pokémon Wayfarer exterior map terrain",
  },
})

#!/usr/bin/env node

import { execute } from "cmdore"

import catalogCommand from "./commands/catalog"

await execute(catalogCommand, {
  metadata: {
    name: "wmetatiles",
    version: "0.0.0",
    description: "Generate palette-correct Pokémon Wayfarer metatile catalogs",
  },
})

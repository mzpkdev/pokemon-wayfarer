import { defineOption } from "cmdore"

export default defineOption({
  name: "output",
  description: "Directory for catalog.json and context atlases, relative to the source root",
  hint: "path",
  arity: 1,
  defaultValue: () => "build/cartographer/map-catalog/metatiles",
})

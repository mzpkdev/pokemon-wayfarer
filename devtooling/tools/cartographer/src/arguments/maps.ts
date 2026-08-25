import { defineArgument } from "cmdore"

export default defineArgument({
  name: "maps",
  description: "Map directory names under data/maps",
  variadic: true,
})

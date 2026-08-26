import { defineOption } from "cmdore"

export default defineOption({
  name: "output",
  description: "Directory for rendered PNG files, relative to the source root",
  hint: "path",
  arity: 1,
  defaultValue: () => "build/map-renders",
})

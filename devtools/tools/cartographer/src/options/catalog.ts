import { defineOption } from "cmdore"

export default defineOption({
  name: "catalog",
  description: "Render every exterior map plus a cartographer-ready catalog.json",
  arity: 0,
})

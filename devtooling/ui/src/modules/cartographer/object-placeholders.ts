import type { CatalogObject } from "./catalog.js"

export type ObjectPlaceholderKind = "stateful" | "variable" | "expression" | "unresolved"

export type ObjectPlaceholder = {
  kind: ObjectPlaceholderKind
  label: string
}

export const objectPlaceholderFor = (object: CatalogObject): ObjectPlaceholder | null => {
  if (object.sprite) return null

  switch (object.diagnostic?.code) {
    case "unresolved_graphics_info":
      return { kind: "stateful", label: "Stateful graphic" }
    case "unknown_graphics_id":
      return { kind: "variable", label: "Runtime graphic" }
    case "unsupported_graphics_expression":
      return { kind: "expression", label: "Unresolved expression" }
    default:
      return { kind: "unresolved", label: "Unresolved graphic" }
  }
}

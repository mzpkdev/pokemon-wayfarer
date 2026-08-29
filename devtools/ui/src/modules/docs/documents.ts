import { buildDocumentCatalog } from "./catalog.js"

const productDocumentSources = import.meta.glob(
  ["../../../../../.product/**/*.md", "!../../../../../.product/**/__*__.md"],
  {
    eager: true,
    import: "default",
    query: "?raw",
  },
) as Record<string, string>

export const productDocuments = buildDocumentCatalog(productDocumentSources)

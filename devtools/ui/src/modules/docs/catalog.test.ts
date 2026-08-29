import { describe, expect, it } from "vitest"

import {
  buildDocumentCatalog,
  docsHash,
  groupDocuments,
  parseDocsHash,
  resolveMarkdownHref,
} from "./catalog.js"

describe("product document catalog", () => {
  it("finds documents, uses the first H1 as a title, and falls back to the filename", () => {
    const documents = buildDocumentCatalog({
      "../../../../../.product/specs/fallback-title.md": "No heading here.",
      "../../../../../.product/research/audit.md": "# **Traversal audit**\n\nBody.",
      "../../../../../README.md": "# Ignore me",
    })

    expect(documents).toEqual([
      {
        group: "specs",
        path: "specs/fallback-title.md",
        source: "No heading here.",
        title: "Fallback title",
      },
      {
        group: "research",
        path: "research/audit.md",
        source: "# **Traversal audit**\n\nBody.",
        title: "Traversal audit",
      },
    ])
  })

  it("groups the standard product folders in a stable order", () => {
    const groups = groupDocuments(
      buildDocumentCatalog({
        "/repo/.product/research/notes.md": "# Notes",
        "/repo/.product/specs/behavior.md": "# Behavior",
        "/repo/.product/prds/need.md": "# Need",
      }),
    )

    expect(groups.map(({ id, label }) => ({ id, label }))).toEqual([
      { id: "prds", label: "Product requirements" },
      { id: "specs", label: "Specifications" },
      { id: "research", label: "Research" },
    ])
  })
})

describe("Docs URLs", () => {
  it("round-trips a document path and heading", () => {
    const hash = docsHash("research/player journey.md", "story-gates")

    expect(hash).toBe("#docs/research/player%20journey.md?section=story-gates")
    expect(parseDocsHash(hash)).toEqual({
      path: "research/player journey.md",
      section: "story-gates",
    })
  })

  it("rewrites same-document and relative Markdown links", () => {
    const paths = new Set(["research/audit.md", "specs/traversal.md"])

    expect(resolveMarkdownHref("research/audit.md", "#Known limits", paths)).toBe(
      "#docs/research/audit.md?section=known-limits",
    )
    expect(resolveMarkdownHref("research/audit.md", "../specs/traversal.md#Rules", paths)).toBe(
      "#docs/specs/traversal.md?section=rules",
    )
    expect(resolveMarkdownHref("research/audit.md", "https://example.com", paths)).toBe(
      "https://example.com",
    )
  })
})

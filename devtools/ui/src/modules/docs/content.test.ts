/** @vitest-environment jsdom */

import { describe, expect, it } from "vitest"

import { renderDocument } from "./content.js"

describe("product document rendering", () => {
  it("removes executable and presentation HTML from Markdown", () => {
    const html = renderDocument(
      [
        "# Safe page",
        "",
        "<script>globalThis.compromised = true</script>",
        '<a href="javascript:alert(1)">unsafe link</a>',
        '<img src="missing.png" onerror="alert(1)" style="display:none">',
      ].join("\n"),
      "research/safe.md",
      new Set(["research/safe.md"]),
    )

    expect(html).toContain('id="safe-page"')
    expect(html).toContain('tabindex="-1"')
    expect(html).not.toContain("<script")
    expect(html).not.toContain("javascript:")
    expect(html).not.toContain("onerror")
    expect(html).not.toContain("style=")
  })

  it("gives repeated headings unique deep links", () => {
    const html = renderDocument(
      "# Limits\n\n## Limits",
      "research/audit.md",
      new Set(["research/audit.md"]),
    )

    expect(html).toContain('id="limits"')
    expect(html).toContain('id="limits-1"')
    expect(html).toContain("#docs/research/audit.md?section=limits-1")
  })
})

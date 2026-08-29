import DOMPurify from "dompurify"
import { Marked, Renderer } from "marked"

import { docsHash, headingSlug, resolveMarkdownHref } from "./catalog.js"

const escapeAttribute = (value: string): string => {
  return value
    .replaceAll("&", "&amp;")
    .replaceAll('"', "&quot;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
}

export const renderDocument = (
  source: string,
  currentPath: string,
  documentPaths: ReadonlySet<string>,
): string => {
  const renderer = new Renderer()
  const slugs = new Map<string, number>()

  renderer.heading = function ({ tokens, depth }) {
    const text = this.parser.parseInline(tokens)
    const baseSlug =
      headingSlug(this.parser.parseInline(tokens, this.parser.textRenderer)) || "section"
    const duplicateCount = slugs.get(baseSlug) ?? 0
    slugs.set(baseSlug, duplicateCount + 1)
    const slug = duplicateCount === 0 ? baseSlug : `${baseSlug}-${duplicateCount}`
    const href = docsHash(currentPath, slug)
    return `<h${depth} id="${escapeAttribute(slug)}" tabindex="-1"><a class="docs-heading-link" href="${escapeAttribute(href)}">${text}</a></h${depth}>\n`
  }

  renderer.link = function ({ href, title, tokens }) {
    const resolvedHref = resolveMarkdownHref(currentPath, href, documentPaths)
    const titleAttribute = title ? ` title="${escapeAttribute(title)}"` : ""
    return `<a href="${escapeAttribute(resolvedHref)}"${titleAttribute}>${this.parser.parseInline(tokens)}</a>`
  }

  const marked = new Marked({ gfm: true, renderer })
  const html = marked.parse(source) as string
  return DOMPurify.sanitize(html, {
    FORBID_ATTR: ["style"],
    FORBID_TAGS: ["embed", "iframe", "object", "script", "style"],
    USE_PROFILES: { html: true },
  })
}

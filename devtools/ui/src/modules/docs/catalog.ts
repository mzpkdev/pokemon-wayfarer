export type ProductDocument = {
  group: string
  path: string
  source: string
  title: string
}

export type DocumentGroup = {
  id: string
  label: string
  documents: ProductDocument[]
}

const preferredGroupOrder = ["prds", "specs", "research"]

const wordsFromName = (name: string): string => {
  return name
    .replace(/\.md$/i, "")
    .replace(/^__|__$/g, "")
    .replace(/[-_]+/g, " ")
    .replace(/\s+/g, " ")
    .trim()
}

const sentenceCase = (value: string): string => {
  return value ? `${value.charAt(0).toUpperCase()}${value.slice(1)}` : value
}

const plainHeading = (heading: string): string => {
  return heading
    .replace(/!\[([^\]]*)\]\([^)]*\)/g, "$1")
    .replace(/\[([^\]]+)\]\([^)]*\)/g, "$1")
    .replace(/[`*_~]/g, "")
    .replace(/<[^>]+>/g, "")
    .trim()
}

export const documentPathFromImport = (importPath: string): string | null => {
  const marker = "/.product/"
  const markerIndex = importPath.lastIndexOf(marker)
  if (markerIndex === -1 || !importPath.toLowerCase().endsWith(".md")) return null
  const path = importPath.slice(markerIndex + marker.length)
  const filename = path.split("/").at(-1) ?? path
  return /^__.+__\.md$/i.test(filename) ? null : path
}

export const documentTitle = (path: string, source: string): string => {
  const heading = source.match(/^#\s+(.+?)\s*$/m)?.[1]
  if (heading) {
    const title = plainHeading(heading)
    if (title) return title
  }

  return sentenceCase(wordsFromName(path.split("/").at(-1) ?? path))
}

export const groupLabel = (group: string): string => {
  switch (group) {
    case "prds":
      return "Product requirements"
    case "specs":
      return "Specifications"
    default:
      return sentenceCase(wordsFromName(group))
  }
}

export const buildDocumentCatalog = (sources: Record<string, string>): ProductDocument[] => {
  return Object.entries(sources)
    .flatMap(([importPath, source]) => {
      const path = documentPathFromImport(importPath)
      if (!path) return []
      return [
        {
          group: path.split("/")[0] ?? "other",
          path,
          source,
          title: documentTitle(path, source),
        },
      ]
    })
    .sort(
      (left, right) => left.title.localeCompare(right.title) || left.path.localeCompare(right.path),
    )
}

export const groupDocuments = (documents: ProductDocument[]): DocumentGroup[] => {
  const grouped = Map.groupBy(documents, (document) => document.group)
  return [...grouped.entries()]
    .map(([id, entries]) => ({ id, label: groupLabel(id), documents: entries }))
    .sort((left, right) => {
      const leftIndex = preferredGroupOrder.indexOf(left.id)
      const rightIndex = preferredGroupOrder.indexOf(right.id)
      if (leftIndex !== -1 || rightIndex !== -1) {
        if (leftIndex === -1) return 1
        if (rightIndex === -1) return -1
        return leftIndex - rightIndex
      }
      return left.label.localeCompare(right.label)
    })
}

const safeDecode = (value: string): string => {
  try {
    return decodeURIComponent(value)
  } catch {
    return value
  }
}

const encodedDocumentPath = (path: string): string => {
  return path.split("/").map(encodeURIComponent).join("/")
}

export const docsHash = (path: string, section?: string | null): string => {
  const base = `#docs/${encodedDocumentPath(path)}`
  return section ? `${base}?section=${encodeURIComponent(section)}` : base
}

export type DocsRoute = {
  path: string | null
  section: string | null
}

export const parseDocsHash = (hash: string): DocsRoute => {
  if (!hash.startsWith("#docs/")) return { path: null, section: null }
  const route = hash.slice("#docs/".length)
  const questionIndex = route.indexOf("?")
  const encodedPath = questionIndex === -1 ? route : route.slice(0, questionIndex)
  const query = questionIndex === -1 ? "" : route.slice(questionIndex + 1)
  const path = encodedPath.split("/").map(safeDecode).join("/") || null
  const section = new URLSearchParams(query).get("section")?.trim() || null
  return { path, section }
}

const normalizedRelativePath = (path: string): string | null => {
  const result: string[] = []
  for (const part of path.split("/")) {
    if (!part || part === ".") continue
    if (part === "..") {
      if (!result.pop()) return null
    } else {
      result.push(part)
    }
  }
  return result.join("/")
}

export const headingSlug = (heading: string): string => {
  return heading
    .toLowerCase()
    .trim()
    .replace(/<[^>]+>/g, "")
    .replace(/[^\p{Letter}\p{Mark}\p{Number}\s_-]/gu, "")
    .replace(/\s+/g, "-")
}

export const resolveMarkdownHref = (
  currentPath: string,
  href: string,
  documentPaths: ReadonlySet<string>,
): string => {
  if (!href) return href
  if (href.startsWith("#")) return docsHash(currentPath, headingSlug(safeDecode(href.slice(1))))
  if (/^(?:[a-z][a-z\d+.-]*:|\/\/)/i.test(href)) return href

  const hashIndex = href.indexOf("#")
  const pathPart = hashIndex === -1 ? href : href.slice(0, hashIndex)
  const fragment = hashIndex === -1 ? null : safeDecode(href.slice(hashIndex + 1))
  const queryIndex = pathPart.indexOf("?")
  const pathWithoutQuery = queryIndex === -1 ? pathPart : pathPart.slice(0, queryIndex)
  const productRelative = pathWithoutQuery.replace(/^\/?\.product\//, "")
  const currentDirectory = currentPath.split("/").slice(0, -1).join("/")
  const candidate = normalizedRelativePath(
    pathWithoutQuery.startsWith("/.product/") || pathWithoutQuery.startsWith(".product/")
      ? productRelative
      : `${currentDirectory}/${productRelative}`,
  )

  if (!candidate || !documentPaths.has(candidate)) return href
  return docsHash(candidate, fragment ? headingSlug(fragment) : null)
}

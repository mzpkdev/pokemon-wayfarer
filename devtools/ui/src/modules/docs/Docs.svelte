<script lang="ts">
  import { onMount, tick } from "svelte"

  import { docsHash, groupDocuments, parseDocsHash } from "./catalog.js"
  import { renderDocument } from "./content.js"
  import { productDocuments } from "./documents.js"

  const groups = groupDocuments(productDocuments)
  const documentPaths = new Set(productDocuments.map((document) => document.path))
  const firstDocument = productDocuments[0]

  let docsRoot: HTMLElement | undefined = $state()
  let route = $state(parseDocsHash(""))
  let selectedDocument = $derived(
    productDocuments.find((document) => document.path === route.path) ?? firstDocument,
  )
  let renderedDocument = $derived(
    selectedDocument
      ? renderDocument(selectedDocument.source, selectedDocument.path, documentPaths)
      : "",
  )

  const scrollToSection = async (section: string | null): Promise<void> => {
    if (!section) return
    await tick()
    const heading = [...(docsRoot?.querySelectorAll<HTMLElement>("[id]") ?? [])].find(
      (element) => element.id === section,
    )
    heading?.scrollIntoView({ block: "start" })
    heading?.focus({ preventScroll: true })
  }

  const updateRoute = (): void => {
    route = parseDocsHash(window.location.hash)
    void scrollToSection(route.section)
  }

  const handleContentClick = (event: MouseEvent): void => {
    const target = event.target
    if (!(target instanceof Element) || !docsRoot?.contains(target)) return
    const link = target.closest<HTMLAnchorElement>("a")
    const href = link?.getAttribute("href")
    if (!href?.startsWith("#docs/")) return

    event.preventDefault()
    if (window.location.hash === href) {
      route = parseDocsHash(href)
      void scrollToSection(route.section)
    } else {
      window.location.hash = href
    }
  }

  onMount(() => {
    updateRoute()
    window.addEventListener("hashchange", updateRoute)
    document.addEventListener("click", handleContentClick)
    return () => {
      window.removeEventListener("hashchange", updateRoute)
      document.removeEventListener("click", handleContentClick)
    }
  })
</script>

<div class="docs-shell">
  <aside class="docs-sidebar cartographer-scrollbar" aria-label="Product documents">
    <div class="docs-sidebar-intro">
      <p class="docs-eyebrow">Product docs</p>
      <p>Markdown from <code>.product/</code>, bundled from the current checkout.</p>
    </div>

    {#each groups as group (group.id)}
      <section class="docs-nav-group">
        <h2>{group.label}</h2>
        <ul>
          {#each group.documents as document (document.path)}
            <li>
              <a
                href={docsHash(document.path)}
                aria-current={selectedDocument?.path === document.path ? "page" : undefined}
              >
                {document.title}
              </a>
            </li>
          {/each}
        </ul>
      </section>
    {/each}
  </aside>

  <section class="docs-reader">
    {#if selectedDocument}
      <p class="docs-path">.product/{selectedDocument.path}</p>
      <article bind:this={docsRoot} class="docs-content" aria-label={selectedDocument.title}>
        {@html renderedDocument}
      </article>
    {:else}
      <div class="docs-empty">
        <h1>No product documents found</h1>
        <p>Add a Markdown file below <code>.product/</code> and restart the development server.</p>
      </div>
    {/if}
  </section>
</div>

<style>
  .docs-shell {
    display: grid;
    grid-template-columns: minmax(15rem, 20rem) minmax(0, 1fr);
    min-height: calc(100vh - 3.1rem);
  }

  .docs-sidebar {
    background: color-mix(in srgb, var(--color-cartographer-field) 94%, transparent);
    border-right: 1px solid var(--color-cartographer-border);
    max-height: calc(100vh - 3.1rem);
    overflow-y: auto;
    padding: 1.5rem clamp(1rem, 2.5vw, 2rem) 3rem;
    position: sticky;
    top: 0;
  }

  .docs-sidebar-intro {
    border-bottom: 1px solid var(--color-cartographer-border);
    color: var(--color-cartographer-muted);
    font-size: 0.85rem;
    line-height: 1.55;
    margin-bottom: 1.5rem;
    padding-bottom: 1.25rem;
  }

  .docs-sidebar-intro p {
    margin: 0;
  }

  .docs-sidebar-intro code,
  .docs-empty code {
    color: var(--color-cartographer-signal-soft);
    font-family: var(--font-cartographer-mono);
  }

  .docs-eyebrow {
    color: var(--color-cartographer-ink);
    font-size: 0.72rem;
    font-weight: 700;
    letter-spacing: 0.12em;
    margin-bottom: 0.35rem !important;
    text-transform: uppercase;
  }

  .docs-nav-group + .docs-nav-group {
    margin-top: 1.5rem;
  }

  .docs-nav-group h2 {
    color: var(--color-cartographer-muted-soft);
    font-size: 0.68rem;
    font-weight: 700;
    letter-spacing: 0.11em;
    margin: 0 0 0.45rem;
    text-transform: uppercase;
  }

  .docs-nav-group ul {
    display: grid;
    gap: 0.15rem;
    list-style: none;
    margin: 0;
    padding: 0;
  }

  .docs-nav-group a {
    border-left: 2px solid transparent;
    color: var(--color-cartographer-muted);
    display: block;
    font-size: 0.86rem;
    line-height: 1.35;
    padding: 0.5rem 0.65rem;
    text-decoration: none;
  }

  .docs-nav-group a:hover {
    background: var(--color-cartographer-panel-raised);
    color: var(--color-cartographer-ink);
  }

  .docs-nav-group a[aria-current="page"] {
    background: var(--color-cartographer-panel);
    border-left-color: var(--color-cartographer-signal);
    color: var(--color-cartographer-signal-soft);
  }

  .docs-nav-group a:focus-visible,
  :global(.docs-content a:focus-visible) {
    outline: 2px solid var(--color-cartographer-signal);
    outline-offset: 2px;
  }

  .docs-reader {
    min-width: 0;
    padding: clamp(1.5rem, 4vw, 4rem) clamp(1rem, 6vw, 6rem) 6rem;
  }

  .docs-path {
    color: var(--color-cartographer-muted-soft);
    font-family: var(--font-cartographer-mono);
    font-size: 0.72rem;
    margin: 0 auto 1.25rem;
    max-width: 72rem;
    overflow-wrap: anywhere;
  }

  .docs-content,
  .docs-empty {
    color: var(--color-cartographer-slate-100);
    font-size: clamp(0.96rem, 1.2vw, 1.06rem);
    line-height: 1.72;
    margin: 0 auto;
    max-width: 72rem;
  }

  :global(.docs-content h1),
  :global(.docs-content h2),
  :global(.docs-content h3),
  :global(.docs-content h4),
  :global(.docs-content h5),
  :global(.docs-content h6) {
    color: var(--color-cartographer-ink);
    line-height: 1.2;
    scroll-margin-top: 1.5rem;
  }

  :global(.docs-content h1) {
    font-size: clamp(2rem, 5vw, 3.5rem);
    letter-spacing: -0.035em;
    margin: 0 0 1.75rem;
  }

  :global(.docs-content h2) {
    border-top: 1px solid var(--color-cartographer-border);
    font-size: clamp(1.4rem, 3vw, 2rem);
    margin: 3rem 0 1rem;
    padding-top: 1.5rem;
  }

  :global(.docs-content h3) {
    font-size: 1.25rem;
    margin: 2.25rem 0 0.75rem;
  }

  :global(.docs-content h4),
  :global(.docs-content h5),
  :global(.docs-content h6) {
    font-size: 1.05rem;
    margin: 1.75rem 0 0.65rem;
  }

  :global(.docs-heading-link) {
    color: inherit !important;
    text-decoration: none !important;
  }

  :global(.docs-heading-link:hover)::after {
    color: var(--color-cartographer-muted-soft);
    content: " #";
    font-weight: 400;
  }

  :global(.docs-content p),
  :global(.docs-content ul),
  :global(.docs-content ol),
  :global(.docs-content blockquote) {
    margin: 0.85rem 0;
  }

  :global(.docs-content ul),
  :global(.docs-content ol) {
    padding-left: 1.6rem;
  }

  :global(.docs-content li + li) {
    margin-top: 0.35rem;
  }

  :global(.docs-content a) {
    color: var(--color-cartographer-signal-soft);
    text-decoration-color: color-mix(in srgb, var(--color-cartographer-signal) 60%, transparent);
    text-underline-offset: 0.18em;
  }

  :global(.docs-content a:hover) {
    color: var(--color-cartographer-ink);
  }

  :global(.docs-content code) {
    background: var(--color-cartographer-panel-raised);
    border: 1px solid var(--color-cartographer-border);
    border-radius: 0.25rem;
    color: var(--color-cartographer-signal-soft);
    font-family: var(--font-cartographer-mono);
    font-size: 0.87em;
    overflow-wrap: anywhere;
    padding: 0.1em 0.3em;
  }

  :global(.docs-content pre) {
    background: var(--color-cartographer-slate-900);
    border: 1px solid var(--color-cartographer-border);
    border-radius: 0.35rem;
    margin: 1.25rem 0;
    overflow-x: auto;
    padding: 1rem;
  }

  :global(.docs-content pre code) {
    background: transparent;
    border: 0;
    overflow-wrap: normal;
    padding: 0;
  }

  :global(.docs-content blockquote) {
    border-left: 3px solid var(--color-cartographer-signal);
    color: var(--color-cartographer-muted);
    padding-left: 1rem;
  }

  :global(.docs-content hr) {
    border: 0;
    border-top: 1px solid var(--color-cartographer-border);
    margin: 2.5rem 0;
  }

  :global(.docs-content table) {
    border-collapse: collapse;
    display: block;
    font-size: 0.82rem;
    margin: 1.5rem 0;
    max-width: 100%;
    overflow-x: auto;
    scrollbar-color: var(--color-cartographer-border-strong) var(--color-cartographer-field);
    white-space: normal;
    width: max-content;
  }

  :global(.docs-content th),
  :global(.docs-content td) {
    border: 1px solid var(--color-cartographer-border);
    min-width: 10rem;
    padding: 0.6rem 0.75rem;
    text-align: left;
    vertical-align: top;
  }

  :global(.docs-content th) {
    background: var(--color-cartographer-panel-raised);
    color: var(--color-cartographer-ink);
    font-weight: 650;
  }

  :global(.docs-content tbody tr:nth-child(even)) {
    background: color-mix(in srgb, var(--color-cartographer-panel) 50%, transparent);
  }

  :global(.docs-content img) {
    height: auto;
    max-width: 100%;
  }

  .docs-empty h1 {
    margin-top: 0;
  }

  @media (max-width: 760px) {
    .docs-shell {
      display: block;
    }

    .docs-sidebar {
      border-bottom: 1px solid var(--color-cartographer-border);
      border-right: 0;
      max-height: 18rem;
      position: static;
    }

    .docs-reader {
      padding-top: 2rem;
    }
  }
</style>

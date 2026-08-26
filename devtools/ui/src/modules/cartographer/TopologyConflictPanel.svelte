<script lang="ts">
  import type {
    CatalogDirectTopologyMismatch,
    CatalogTopologyConnection,
    CatalogTopologyDiagnostic,
  } from "./catalog.js"

  type Props = {
    diagnostics: readonly CatalogTopologyDiagnostic[]
    visible: boolean
  }

  let { diagnostics, visible }: Props = $props()

  const connectionLabel = (connection: CatalogTopologyConnection): string => {
    return `${connection.source.map} → ${connection.destination.map} (${connection.direction}, ${connection.offsetMetatiles})`
  }

  const directKey = (diagnostic: CatalogDirectTopologyMismatch): string => {
    return `${diagnostic.connection.source.map}:${diagnostic.connection.source.header.pointer}`
  }

  const diagnosticKey = (diagnostic: CatalogTopologyDiagnostic): string => {
    if (diagnostic.code === "direct_connection_mismatch") return directKey(diagnostic)
    return `missing:${diagnostic.connection.source.map}:${diagnostic.connection.source.header.pointer}`
  }
</script>

{#if visible && diagnostics.length > 0}
  <section
    class="border-t border-cartographer-diagnostic-border bg-cartographer-diagnostic-panel px-4 py-3"
    aria-label="Topology diagnostic details"
  >
    <p
      class="m-0 font-cartographer-mono text-[0.68rem] font-bold tracking-[0.12em] text-cartographer-garnet-400"
    >
      Topology diagnostics
    </p>
    <p class="mb-3 mt-1 text-xs leading-5 text-cartographer-muted">
      Diagnostics identify a direct disagreement between reciprocal records or a missing reverse
      record.
    </p>
    <ul class="m-0 grid max-h-48 list-none gap-1.5 overflow-y-auto p-0">
      {#each diagnostics as diagnostic (diagnosticKey(diagnostic))}
        <li class="border border-cartographer-border bg-cartographer-panel px-3 py-2 text-xs">
          {#if diagnostic.code === "direct_connection_mismatch"}
            <p class="m-0 font-cartographer-mono text-cartographer-signal-soft">
              Direct connection mismatch · {connectionLabel(diagnostic.connection)}
            </p>
            <p class="mb-0 mt-1 text-cartographer-muted">
              Expected reverse: {diagnostic.expectedReverse.direction} · offset {diagnostic
                .expectedReverse.offsetMetatiles}. Recorded reverse: {connectionLabel(
                diagnostic.reverseConnection,
              )}.
            </p>
            <p class="mb-0 mt-1 text-cartographer-muted">{diagnostic.explanation}</p>
            <p class="mb-0 mt-1 font-cartographer-mono text-[0.65rem] text-cartographer-muted">
              Inspect {diagnostic.connection.source.header.path}{diagnostic.connection.source.header
                .pointer} and {diagnostic.reverseConnection.source.header.path}{diagnostic
                .reverseConnection.source.header.pointer}
            </p>
          {:else}
            <p class="m-0 font-cartographer-mono text-cartographer-signal-soft">
              Missing reverse connection · {connectionLabel(diagnostic.connection)}
            </p>
            <p class="mb-0 mt-1 text-cartographer-muted">
              Expected reverse: {diagnostic.expectedReverse.direction} · offset {diagnostic
                .expectedReverse.offsetMetatiles}. No reciprocal source record was found.
            </p>
            <p class="mb-0 mt-1 text-cartographer-muted">{diagnostic.explanation}</p>
            <p class="mb-0 mt-1 font-cartographer-mono text-[0.65rem] text-cartographer-muted">
              Inspect {diagnostic.connection.source.header.path}{diagnostic.connection.source.header
                .pointer}
            </p>
          {/if}
        </li>
      {/each}
    </ul>
  </section>
{/if}

<script lang="ts">
  import { cubicOut } from "svelte/easing"
  import { fade } from "svelte/transition"
  import { onMount } from "svelte"

  import Cartographer from "./modules/cartographer/Cartographer.svelte"
  import Metatiles from "./modules/metatiles/Metatiles.svelte"

  type ModuleId = "cartographer" | "metatiles"

  const fadeIn = { delay: 75, duration: 125, easing: cubicOut }

  const moduleFromHash = (): ModuleId => {
    return window.location.hash === "#metatiles" ? "metatiles" : "cartographer"
  }

  let activeModule = $state<ModuleId>("cartographer")

  onMount(() => {
    const handleHashChange = (): void => {
      activeModule = moduleFromHash()
    }

    handleHashChange()
    window.addEventListener("hashchange", handleHashChange)
    return () => {
      window.removeEventListener("hashchange", handleHashChange)
    }
  })
</script>

<svelte:head>
  <title>{activeModule === "cartographer" ? "Cartographer" : "Metatiles"} · Wayfarer</title>
  <meta
    name="description"
    content="An interactive terrain index for the Pokémon HnS world source."
  />
</svelte:head>

<main class="min-h-screen">
  <nav
    class="flex items-stretch justify-between border-b border-cartographer-border/80 bg-cartographer-field/90 px-[clamp(1rem,3vw,2.5rem)] backdrop-blur"
    aria-label="Modules"
  >
    <div class="flex items-center gap-3 py-3">
      <span class="text-xs font-semibold tracking-[0.08em] text-cartographer-muted">Wayfarer</span>
      <span aria-hidden="true" class="h-3.5 w-px bg-cartographer-border"></span>
      <a
        class="relative inline-flex items-center text-sm font-semibold text-cartographer-signal no-underline after:absolute after:-bottom-3 after:left-0 after:h-px after:w-full after:bg-cartographer-signal"
        class:text-cartographer-muted={activeModule !== "cartographer"}
        class:after:hidden={activeModule !== "cartographer"}
        href="#cartographer"
        aria-current={activeModule === "cartographer" ? "page" : undefined}
      >
        Cartographer
      </a>
      <a
        class="relative inline-flex items-center text-sm font-semibold text-cartographer-signal no-underline after:absolute after:-bottom-3 after:left-0 after:h-px after:w-full after:bg-cartographer-signal"
        class:text-cartographer-muted={activeModule !== "metatiles"}
        class:after:hidden={activeModule !== "metatiles"}
        href="#metatiles"
        aria-current={activeModule === "metatiles" ? "page" : undefined}
      >
        Metatiles
      </a>
    </div>
    <span class="hidden items-center text-xs text-cartographer-muted sm:flex">Local source</span>
  </nav>

  {#key activeModule}
    <div in:fade={fadeIn}>
      {#if activeModule === "cartographer"}
        <Cartographer />
      {:else}
        <Metatiles />
      {/if}
    </div>
  {/key}
</main>

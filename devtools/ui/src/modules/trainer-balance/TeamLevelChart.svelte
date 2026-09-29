<script lang="ts">
  import type { Milestone } from "./types.js"

  type Point = { world: number; tr: number; teamLevel: number; cap: number }
  let {
    name,
    points,
    milestones,
    playerTR,
  }: { name: string; points: Point[]; milestones: Milestone[]; playerTR: number } = $props()

  // Plot geometry in viewBox units; the SVG scales to the panel width.
  const width = 360
  const height = 190
  const margin = { top: 16, right: 12, bottom: 34, left: 34 }
  const plotWidth = width - margin.left - margin.right
  const plotHeight = height - margin.top - margin.bottom
  const levels = [0, 25, 50, 75, 100]

  const end = $derived(points.at(-1)?.world ?? 0)
  const x = (world: number): number => margin.left + (Math.min(world, end) / (end || 1)) * plotWidth
  const y = (level: number): number => margin.top + (1 - level / 100) * plotHeight
  const path = (value: (point: Point) => number): string =>
    points
      .map((point, index) => `${index ? "L" : "M"}${x(point.world)},${y(value(point))}`)
      .join("")
  const teamPath = $derived(path((point) => point.teamLevel))
  const capPath = $derived(path((point) => point.cap))
  const ticks = $derived(Array.from({ length: Math.floor(end / 40) + 1 }, (_, index) => index * 40))
  const dots = $derived(
    milestones.filter((milestone) => milestone.worldProgress <= end && milestone.worldProgress > 0),
  )
  const offChart = $derived(playerTR > end)

  let hover = $state<number | null>(null)
  const hovered = $derived(hover === null ? null : (points[Math.min(hover, end)] ?? null))
  const pointAt = (event: PointerEvent): void => {
    const box = (event.currentTarget as HTMLElement).getBoundingClientRect()
    const at = ((event.clientX - box.left) / box.width) * width
    hover = Math.round(Math.max(0, Math.min(1, (at - margin.left) / plotWidth)) * end)
  }
  const keyStep = (event: KeyboardEvent): void => {
    const step = event.shiftKey ? 10 : 1
    const from = hover ?? Math.min(playerTR, end)
    if (event.key === "ArrowRight") hover = Math.min(end, from + step)
    else if (event.key === "ArrowLeft") hover = Math.max(0, from - step)
    else if (event.key === "Home") hover = 0
    else if (event.key === "End") hover = end
    else return
    event.preventDefault()
  }
</script>

<figure class="level-chart" data-testid="level-chart">
  <figcaption>
    <span class="chart-title">Team level vs level cap by player TR</span>
    <span class="legend" aria-hidden="true"
      ><span class="key"><span class="swatch team"></span>{name}’s team level</span><span
        class="key"><span class="swatch cap"></span>Level cap</span
      ><span class="key"><span class="swatch dot"></span>Milestone</span></span
    >
  </figcaption>
  <div
    class="plot"
    role="slider"
    tabindex="0"
    aria-label={`Read ${name}’s chart by player TR`}
    aria-valuemin={0}
    aria-valuemax={end}
    aria-valuenow={hovered?.world ?? Math.min(playerTR, end)}
    aria-valuetext={hovered
      ? `Player TR ${hovered.world}: team level Lv ${hovered.teamLevel}, level cap Lv ${hovered.cap}`
      : `Player TR ${playerTR}`}
    onpointermove={pointAt}
    onpointerleave={() => (hover = null)}
    onfocus={() => (hover = Math.min(playerTR, end))}
    onblur={() => (hover = null)}
    onkeydown={keyStep}
  >
    <svg
      viewBox={`0 0 ${width} ${height}`}
      role="img"
      aria-label={`${name}’s team level (solid line) and the level cap (dashed line) across player TR 0 to ${end}; the player TR ${playerTR} is marked.`}
    >
      {#each levels as level}<line
          class="grid"
          x1={margin.left}
          x2={width - margin.right}
          y1={y(level)}
          y2={y(level)}
        /><text class="tick" x={margin.left - 6} y={y(level) + 3} text-anchor="end">{level}</text
        >{/each}
      {#each ticks as tick}<text
          class="tick"
          x={x(tick)}
          y={height - margin.bottom + 12}
          text-anchor="middle">{tick}</text
        >{/each}
      <text class="axis-title" x={margin.left + plotWidth / 2} y={height - 4} text-anchor="middle"
        >Player TR (world progress)</text
      >
      <text
        class="axis-title"
        transform={`translate(10 ${margin.top + plotHeight / 2}) rotate(-90)`}
        text-anchor="middle">Level</text
      >
      <path class="series cap" d={capPath} data-testid="chart-cap" />
      <path class="series team" d={teamPath} data-testid="chart-team" />
      {#each dots as milestone (milestone.worldProgress)}<circle
          class="milestone"
          cx={x(milestone.worldProgress)}
          cy={y(milestone.teamLevel)}
          r="4"
        />{/each}
      <line
        class="now"
        x1={x(playerTR)}
        x2={x(playerTR)}
        y1={margin.top - 4}
        y2={height - margin.bottom}
        data-testid="chart-now"
      />
      <text
        class="now-label"
        x={x(playerTR)}
        y={margin.top - 7}
        text-anchor={x(playerTR) > width - 60
          ? "end"
          : x(playerTR) < margin.left + 40
            ? "start"
            : "middle"}>{offChart ? `Player TR ${playerTR} →` : `Player TR ${playerTR}`}</text
      >
      {#if hovered}<line
          class="crosshair"
          x1={x(hovered.world)}
          x2={x(hovered.world)}
          y1={margin.top}
          y2={height - margin.bottom}
        />{/if}
    </svg>
    {#if hovered}<div
        class="chart-tooltip"
        aria-hidden="true"
        data-testid="chart-tooltip"
        style:left={`${(x(hovered.world) / width) * 100}%`}
        class:flip={hovered.world > end / 2}
      >
        <strong>Player TR {hovered.world}</strong>
        <span
          ><span class="swatch team"></span><b>Lv {hovered.teamLevel}</b> team level (TR {hovered.tr})</span
        >
        <span><span class="swatch cap"></span><b>Lv {hovered.cap}</b> level cap</span>
      </div>{/if}
  </div>
</figure>

<style>
  .level-chart {
    /* Validated against the panel surface (#191c20): the dataviz check passes lightness, chroma,
       colour-blind separation and contrast. The level cap is also dashed, so identity is never
       colour alone. The explorer is dark-only, like the rest of the app. */
    --team: #c98500;
    --cap: #3987e5;
    --ink: var(--color-cartographer-ink);
    --muted: var(--color-cartographer-muted);
    --grid: #2b3037;
    margin: 12px 0 20px;
  }
  figcaption {
    display: flex;
    flex-wrap: wrap;
    justify-content: space-between;
    gap: 6px 12px;
    font-size: 11px;
    color: var(--muted);
    margin-bottom: 6px;
  }
  .chart-title {
    color: var(--ink);
    font-weight: 500;
    font-size: 12px;
  }
  .legend {
    display: flex;
    flex-wrap: wrap;
    gap: 4px 12px;
  }
  .key {
    display: inline-flex;
    align-items: center;
    gap: 5px;
  }
  .swatch {
    display: inline-block;
    width: 14px;
    height: 0;
    border-top: 2px solid var(--team);
    vertical-align: middle;
  }
  .swatch.cap {
    border-top: 2px dashed var(--cap);
  }
  .swatch.dot {
    width: 8px;
    height: 8px;
    border: 0;
    border-radius: 50%;
    background: var(--team);
  }
  .plot {
    position: relative;
    touch-action: pan-y;
  }
  svg {
    display: block;
    width: 100%;
    height: auto;
    overflow: visible;
  }
  .plot:focus-visible {
    outline: 2px solid var(--accent, #e9be75);
    outline-offset: 3px;
  }
  .grid {
    stroke: var(--grid);
    stroke-width: 1;
  }
  .tick {
    fill: var(--muted);
    font-size: 9px;
    font-variant-numeric: tabular-nums;
  }
  .axis-title {
    fill: var(--muted);
    font-size: 9px;
  }
  .series {
    fill: none;
    stroke-width: 2;
    stroke-linejoin: round;
    stroke-linecap: round;
  }
  .series.team {
    stroke: var(--team);
  }
  .series.cap {
    stroke: var(--cap);
    stroke-dasharray: 5 3;
  }
  .milestone {
    fill: var(--team);
    stroke: var(--color-cartographer-panel);
    stroke-width: 2;
  }
  .now {
    stroke: var(--ink);
    stroke-width: 1;
  }
  .now-label {
    fill: var(--ink);
    font-size: 9px;
    font-weight: 500;
  }
  .crosshair {
    stroke: var(--muted);
    stroke-width: 1;
  }
  .chart-tooltip {
    position: absolute;
    top: 24px;
    transform: translateX(8px);
    display: grid;
    gap: 2px;
    min-width: 150px;
    padding: 7px 9px;
    border: 1px solid var(--color-cartographer-border);
    border-radius: 5px;
    background: var(--color-cartographer-field);
    font-size: 11px;
    color: var(--muted);
    pointer-events: none;
    white-space: nowrap;
  }
  .chart-tooltip.flip {
    transform: translateX(calc(-100% - 8px));
  }
  .chart-tooltip strong {
    color: var(--ink);
    font-weight: 500;
  }
  .chart-tooltip b {
    color: var(--ink);
    font-weight: 500;
  }
  .chart-tooltip .swatch {
    width: 10px;
    margin-right: 5px;
  }
</style>

<script lang="ts">
  import { onMount } from "svelte"
  import { BalanceLab, catalog } from "./lab.svelte.js"

  const lab = new BalanceLab()
  const checkpoints = [0, 8, 16, 24]
  const ticks = [0, 4, 8, 12, 16, 20, 24]
  let importInput: HTMLInputElement
  const line = (key: "ace" | "cap"): string =>
    lab.curve.map((point) => `${24 + point.badges * 13},${140 - point[key] * 1.2}`).join(" ")
  const download = (): void => {
    const url = URL.createObjectURL(new Blob([lab.exportText()], { type: "application/json" }))
    const link = document.createElement("a")
    link.href = url
    link.download = `wayfarer-balance-${lab.badges}-badges.json`
    link.click()
    setTimeout(() => URL.revokeObjectURL(url), 1000)
  }
  const importFile = async (event: Event): Promise<void> => {
    const input = event.currentTarget as HTMLInputElement
    const file = input.files?.[0]
    if (file) {
      try {
        lab.importText(await file.text())
      } catch {
        lab.error = "Could not read that file."
      }
    }
    input.value = ""
  }
  onMount(lab.load)
</script>

<section class="balance-lab" aria-label="Trainer balance explorer">
  <header class="lab-header">
    <div>
      <div class="eyebrow">Wayfarer / design tools <span class="prototype">Experimental</span></div>
      <h1>Trainer balance</h1>
      <p>
        Move through the badge journey. See who grows, what they bring, and where the numbers drift.
      </p>
    </div>
    <div class="actions">
      <button type="button" onclick={() => importInput.click()}>Import</button>
      <button type="button" class="primary" onclick={download}>Export experiment</button>
      <input
        class="visually-hidden"
        type="file"
        accept=".json,application/json"
        aria-label="Import experiment file"
        bind:this={importInput}
        onchange={importFile}
      />
    </div>
  </header>

  <div class="world-panel">
    <div class="badge-count">
      <span class="eyebrow">Badges earned</span>
      <div><strong data-testid="badge-count">{lab.badges}</strong><span>/ 24</span></div>
    </div>
    <div class="badge-slider">
      <label for="badge-progress" class="visually-hidden">Badges earned</label>
      <input
        id="badge-progress"
        type="range"
        min="0"
        max="24"
        step="1"
        value={lab.badges}
        oninput={(event) => lab.setBadges(event.currentTarget.valueAsNumber)}
      />
      <div class="slider-ticks">
        {#each ticks as badge}<button
            type="button"
            class:active={lab.badges === badge}
            onclick={() => lab.setBadges(badge)}
            aria-label={`Set ${badge} badges`}>{badge}</button
          >{/each}
      </div>
      <span class="hint"
        >Progress is global. Every trainer develops, including leaders you haven’t challenged.</span
      >
    </div>
    <div class="world-facts">
      <label
        >First league clears<select
          aria-label="First league clears"
          value={lab.leagueClears}
          onchange={(event) => lab.setClears(Number(event.currentTarget.value))}
          >{#each [0, 1, 2, 3] as clears}<option value={clears}>{clears} / 3</option>{/each}</select
        ></label
      >
      <div>
        <span class="eyebrow">Player TR</span><strong data-testid="player-tr">{lab.playerTR}</strong
        >
      </div>
      <div>
        <span class="eyebrow">Player soft cap</span><strong data-testid="player-cap"
          >Lv. {lab.cap}</strong
        >
      </div>
    </div>
  </div>

  <div class="model-note">
    <span class="note-dot"></span><span
      >Prototype growth and team stages. Source parties are shown separately. This models species,
      party size and levels; moves, items, AI and win rates are not simulated.</span
    >
  </div>

  {#if lab.error}<div role="alert" class="message error">{lab.error}</div>{/if}
  {#if lab.notice}<div role="status" class="message">{lab.notice}</div>{/if}

  <div class="workspace">
    <section class="pool-panel" aria-label="Trainer pool">
      <div class="panel-title">
        <h2>The pool <span>{catalog.length}</span></h2>
        <span class="hint">{lab.aboveCap}/{lab.gymCount} Gym aces above player cap</span>
      </div>
      <div class="filters">
        <input
          type="search"
          aria-label="Search trainers"
          placeholder="Find a trainer…"
          bind:value={lab.query}
        />
        <select aria-label="Filter region" bind:value={lab.region}
          >{#each ["All regions", "Kanto", "Johto", "Hoenn"] as region}<option>{region}</option
            >{/each}</select
        >
        <select aria-label="Filter role" bind:value={lab.role}
          >{#each ["All trainers", "Gym Leaders", "Elite Four", "Champion"] as role}<option
              >{role}</option
            >{/each}</select
        >
      </div>
      <div class="pool-scroll cartographer-scrollbar">
        <table class="pool-table">
          <thead
            ><tr
              ><th>Trainer</th><th>Start TR</th><th>Current TR</th><th>Party</th><th>Ace / cap</th
              ></tr
            ></thead
          >
          <tbody>
            {#each lab.rows as row (row.trainer.id)}
              <tr class:selected={row.trainer.id === lab.selectedId}>
                <td
                  ><button
                    class="trainer-select"
                    type="button"
                    aria-pressed={row.trainer.id === lab.selectedId}
                    onclick={() => lab.select(row.trainer.id)}
                    ><strong>{row.trainer.name}</strong><span
                      >{row.trainer.region} · {row.trainer.role}</span
                    ></button
                  ></td
                >
                <td class="numeric muted">{lab.ratingAt(row.trainer.id, 0)}</td>
                <td class="numeric current-tr">{row.tr}</td>
                <td class="numeric">{row.party.length}<span class="muted"> / 6</span></td>
                <td
                  ><span class="level-pill" class:over-cap={row.aceLevel > lab.cap}
                    >Lv. {row.aceLevel}</span
                  ><span class="delta"
                    >{row.aceLevel - lab.cap > 0 ? "+" : ""}{row.aceLevel - lab.cap}</span
                  ></td
                >
              </tr>
            {/each}
          </tbody>
        </table>
        {#if lab.rows.length === 0}<div class="empty">
            No trainers match these filters.<button
              type="button"
              onclick={() => {
                lab.query = ""
                lab.region = "All regions"
                lab.role = "All trainers"
              }}>Clear filters</button
            >
          </div>{/if}
      </div>
      <div class="pool-footer">
        {lab.rows.length} trainers shown <span>Singles only · Tate & Liza excluded</span>
      </div>
    </section>

    <aside class="trainer-panel" aria-label="Selected trainer">
      <div class="selected-heading">
        <div>
          <span class="eyebrow">{lab.selected.trainer.region} · {lab.selected.trainer.role}</span>
          <h2>{lab.selected.trainer.name}</h2>
        </div>
        <div class="tr-badge">
          <span>Current TR</span><strong data-testid="selected-tr">{lab.selected.tr}</strong>
        </div>
      </div>
      <div class="home-leagues">Home leagues: {lab.selected.trainer.homeLeagues.join(" · ")}</div>
      <div class="section-label">
        <h3>Party at {lab.badges} badges</h3>
        <span>{lab.selected.stage}</span>
      </div>
      <ol class="party" data-testid="generated-party">
        {#each lab.selected.party as member, index}<li>
            <span class="slot">{String(index + 1).padStart(2, "0")}</span><strong
              >{member.species}</strong
            ><span class="member-level" class:over-cap={member.level > lab.cap}
              >Lv. {member.level}</span
            >
          </li>{/each}
      </ol>
      {#if lab.selected.warnings.length > 0}<ul class="warnings">
          {#each lab.selected.warnings as warning}<li>{warning}</li>{/each}
        </ul>{/if}

      <div class="section-label">
        <h3>Badge journey</h3>
        <div class="legend">
          <span class="ace-key">Ace</span><span class="cap-key">Player cap</span>
        </div>
      </div>
      <svg
        class="growth-chart"
        viewBox="0 0 360 163"
        role="img"
        aria-label={`${lab.selected.trainer.name} ace level and player cap from zero to 24 badges`}
      >
        {#each [20, 40, 60, 80, 100] as level}<line
            x1="24"
            x2="336"
            y1={140 - level * 1.2}
            y2={140 - level * 1.2}
            class="chart-grid"
          /><text x="2" y={144 - level * 1.2}>{level}</text>{/each}
        <polyline points={line("cap")} class="cap-line" /><polyline
          points={line("ace")}
          class="ace-line"
        />
        <line
          x1={24 + lab.badges * 13}
          x2={24 + lab.badges * 13}
          y1="18"
          y2="140"
          class="position-line"
        />
        <circle
          cx={24 + lab.badges * 13}
          cy={140 - lab.selected.aceLevel * 1.2}
          r="4"
          class="ace-dot"
        />
        {#each checkpoints as badge}<text x={24 + badge * 13} y="158" text-anchor="middle"
            >{badge}</text
          >{/each}
      </svg>

      <details class="reference-panel">
        <summary>Compare source party <span>{lab.selected.trainer.source.label}</span></summary>
        <p>{lab.selected.trainer.source.note}</p>
        <ul>
          {#each lab.selected.trainer.referenceParty as member}<li>
              <strong>{member.species}</strong><span>Lv. {member.level}</span
              >{#if member.moves.length}<small
                  >{member.moves.join(" · ")}{member.item ? ` · Item: ${member.item}` : ""}</small
                >{/if}
            </li>{/each}
        </ul>
        <p class="source-path">
          {lab.selected.trainer.source.path}<br />{lab.selected.trainer.source.trainerId}
        </p>
        <p class="hint">Later roster reference: {lab.selected.trainer.competitiveSource}</p>
      </details>

      <details class="tuning-panel" open>
        <summary>Tune this trainer</summary>
        {#key lab.settings}
          <form
            onsubmit={(event) => {
              event.preventDefault()
              lab.applyRatings(event.currentTarget)
            }}
          >
            <div class="rating-inputs">
              {#each checkpoints as badge, index}<label
                  >{badge} badges<input
                    aria-label={`TR at ${badge} badges`}
                    name={`rating-${badge}`}
                    type="number"
                    min="0"
                    max="80"
                    step="1"
                    required
                    value={lab.settings.ratings[index]}
                  /></label
                >{/each}
            </div>
            <div class="growth-input">
              <label
                >TR per first league clear<input
                  aria-label="TR per first league clear"
                  name="league-growth"
                  type="number"
                  min="0"
                  max="20"
                  step="1"
                  required
                  value={lab.settings.leagueGrowth}
                /></label
              ><button type="submit">Apply ratings</button>
            </div>
          </form>
        {/key}
        <p class="hint">
          TR interpolates between checkpoints. Changes are saved in this browser and included in
          exports.
        </p>
        <button type="button" onclick={lab.resetTrainer}>Restore this trainer’s defaults</button>
        <details class="advanced">
          <summary>Edit team stages</summary>
          <p class="hint">
            Stages unlock at their minimum TR. Edit species and offsets from the ace level; at least
            one member must use offset 0.
          </p>
          <label class="visually-hidden" for="team-editor">Trainer settings JSON</label><textarea
            id="team-editor"
            spellcheck="false"
            bind:value={lab.editor}></textarea><button type="button" onclick={lab.applyTeam}
            >Apply team stages</button
          >
        </details>
      </details>
    </aside>
  </div>

  <details class="global-settings">
    <summary>Shared NPC level curve & experiment settings</summary>
    <p>
      The pairs below are [TR, ace level]. They affect every trainer. The player soft-cap curve
      stays unchanged. League clears can be combined with any badge count here to explore balance;
      this tool does not enforce entry requirements.
    </p>
    <label for="anchor-editor">NPC level anchors</label><textarea
      id="anchor-editor"
      rows="2"
      spellcheck="false"
      bind:value={lab.anchorEditor}></textarea>
    <div class="actions">
      <button type="button" onclick={lab.applyAnchors}>Apply level curve</button><button
        type="button"
        onclick={lab.reset}>Reset all trainer defaults</button
      >
    </div>
    <p class="hint">
      Deterministic milestone model; no seed variation or league lineup sampling yet. This is
      separate from the ROM’s current scaler.
    </p>
  </details>
</section>

<style>
  .balance-lab {
    --accent: #e9be75;
    --surface: var(--color-cartographer-panel);
    --line: var(--color-cartographer-border);
    max-width: 1600px;
    margin: 0 auto;
    padding: 32px clamp(16px, 3vw, 42px) 48px;
  }
  .lab-header,
  .actions,
  .panel-title,
  .selected-heading,
  .section-label,
  .growth-input,
  .pool-footer {
    display: flex;
    justify-content: space-between;
    align-items: center;
    gap: 12px;
  }
  .lab-header {
    margin-bottom: 26px;
    align-items: flex-end;
  }
  .eyebrow {
    color: var(--color-cartographer-muted);
    font-size: 10px;
    letter-spacing: 0.09em;
    text-transform: uppercase;
  }
  .prototype {
    color: var(--accent);
    border: 1px solid #665239;
    border-radius: 4px;
    padding: 3px 7px;
    margin-left: 10px;
    letter-spacing: 0.03em;
  }
  h1 {
    font-size: clamp(26px, 3vw, 36px);
    font-weight: 600;
    letter-spacing: -0.035em;
    margin: 9px 0 6px;
  }
  p {
    color: var(--color-cartographer-muted);
    font-size: 13px;
    line-height: 1.6;
    margin: 8px 0;
  }
  button,
  select,
  input[type="search"],
  input[type="number"],
  textarea {
    border: 1px solid var(--line);
    background: var(--color-cartographer-field);
    color: var(--color-cartographer-ink);
    border-radius: 5px;
    font: inherit;
    font-size: 12px;
  }
  button {
    cursor: pointer;
    padding: 9px 12px;
    white-space: nowrap;
  }
  button:hover {
    background: var(--color-cartographer-panel-raised);
    border-color: var(--color-cartographer-muted);
  }
  button.primary {
    background: var(--accent);
    border-color: var(--accent);
    color: #231d14;
    font-weight: 600;
  }
  button.primary:hover {
    background: #f2ce90;
  }
  :is(button, input, select, textarea, summary):focus-visible {
    outline: 2px solid var(--accent);
    outline-offset: 3px;
  }
  select {
    padding: 9px 28px 9px 10px;
    max-width: 100%;
  }
  .world-panel {
    display: grid;
    grid-template-columns: 105px minmax(180px, 1fr) auto;
    align-items: center;
    gap: 28px;
    padding: 24px;
    border: 1px solid var(--line);
    border-radius: 9px;
    background: var(--surface);
  }
  .badge-count strong {
    font-size: 48px;
    font-weight: 500;
    line-height: 1.2;
    font-variant-numeric: tabular-nums;
    letter-spacing: -0.06em;
  }
  .badge-count div > span {
    color: var(--color-cartographer-muted);
    font-size: 13px;
    margin-left: 8px;
  }
  input[type="range"] {
    width: 100%;
    accent-color: var(--accent);
    cursor: pointer;
  }
  .slider-ticks {
    display: flex;
    justify-content: space-between;
    margin: 3px -5px 6px;
  }
  .slider-ticks button {
    padding: 3px 6px;
    border: none;
    color: var(--color-cartographer-muted);
    background: transparent;
  }
  .slider-ticks button.active {
    color: var(--accent);
  }
  .hint {
    color: var(--color-cartographer-muted);
    font-size: 11px;
    line-height: 1.5;
  }
  .world-facts {
    display: flex;
    gap: 24px;
    align-items: center;
    border-left: 1px solid var(--line);
    padding-left: 24px;
  }
  .world-facts label {
    display: flex;
    flex-direction: column;
    gap: 6px;
    font-size: 11px;
    color: var(--color-cartographer-muted);
  }
  .world-facts strong {
    display: block;
    font-size: 23px;
    font-weight: 500;
    margin-top: 5px;
    font-variant-numeric: tabular-nums;
  }
  .model-note {
    display: flex;
    gap: 9px;
    align-items: baseline;
    color: var(--color-cartographer-muted);
    font-size: 11px;
    line-height: 1.6;
    margin: 13px 0 24px;
  }
  .note-dot {
    width: 6px;
    height: 6px;
    border-radius: 50%;
    flex-shrink: 0;
    background: var(--accent);
  }
  .message {
    padding: 10px 14px;
    margin: 0 0 15px;
    border: 1px solid var(--line);
    border-radius: 5px;
    font-size: 12px;
    color: var(--color-cartographer-signal-soft);
  }
  .message.error {
    border-color: #a15e64;
    color: #ffb8bc;
    background: #311e24;
  }
  .workspace {
    display: grid;
    grid-template-columns: minmax(480px, 1.55fr) minmax(340px, 1fr);
    gap: 24px;
    align-items: start;
  }
  .pool-panel,
  .trainer-panel {
    min-width: 0;
    border: 1px solid var(--line);
    border-radius: 8px;
    background: var(--surface);
    overflow: hidden;
  }
  .panel-title {
    padding: 18px 18px 12px;
    flex-wrap: wrap;
  }
  h2 {
    font-size: 17px;
    font-weight: 600;
    margin: 0;
  }
  .panel-title h2 span {
    color: var(--color-cartographer-muted);
    font-size: 12px;
    font-weight: 400;
    margin-left: 7px;
  }
  .filters {
    display: flex;
    gap: 8px;
    padding: 0 18px 16px;
  }
  .filters input {
    min-width: 70px;
    width: 100%;
    padding: 9px 11px;
  }
  .filters select {
    min-width: 110px;
  }
  .pool-scroll {
    max-height: 670px;
    overflow: auto;
  }
  .pool-table {
    width: 100%;
    border-collapse: collapse;
    font-size: 12px;
    white-space: nowrap;
  }
  th {
    position: sticky;
    top: 0;
    z-index: 1;
    background: #20242a;
    text-align: left;
    font-size: 10px;
    font-weight: 500;
    letter-spacing: 0.04em;
    color: var(--color-cartographer-muted);
    padding: 11px 12px;
  }
  th:first-child {
    padding-left: 18px;
  }
  td {
    border-top: 1px solid #2b3037;
    padding: 10px 12px;
  }
  td:first-child {
    padding: 0 0 0 4px;
  }
  tr.selected {
    background: #333025;
  }
  tr.selected td:first-child {
    box-shadow: inset 3px 0 var(--accent);
  }
  .trainer-select {
    display: block;
    width: 100%;
    text-align: left;
    padding: 12px 14px;
    border: 0;
    border-radius: 0;
    background: transparent;
  }
  .trainer-select strong {
    font-size: 13px;
    font-weight: 500;
  }
  .trainer-select span {
    display: block;
    color: var(--color-cartographer-muted);
    font-size: 10px;
    margin-top: 3px;
  }
  .numeric {
    font-variant-numeric: tabular-nums;
  }
  .current-tr {
    color: var(--accent);
    font-weight: 600;
  }
  .muted {
    color: var(--color-cartographer-muted);
  }
  .level-pill {
    background: #26332c;
    color: #b9d8be;
    border-radius: 4px;
    padding: 4px 7px;
    font-size: 11px;
  }
  .level-pill.over-cap {
    background: #403224;
    color: #f0c887;
  }
  .delta {
    color: var(--color-cartographer-muted);
    font-size: 10px;
    margin-left: 7px;
  }
  .pool-footer {
    border-top: 1px solid var(--line);
    padding: 12px 18px;
    font-size: 10px;
    color: var(--color-cartographer-muted);
  }
  .empty {
    padding: 35px 18px;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 14px;
    font-size: 13px;
    color: var(--color-cartographer-muted);
  }
  .trainer-panel {
    padding: 20px;
  }
  .selected-heading h2 {
    font-size: 27px;
    margin-top: 5px;
    letter-spacing: -0.025em;
  }
  .tr-badge {
    text-align: right;
  }
  .tr-badge span {
    display: block;
    font-size: 10px;
    color: var(--color-cartographer-muted);
  }
  .tr-badge strong {
    color: var(--accent);
    font-size: 28px;
    font-weight: 500;
  }
  .home-leagues {
    font-size: 10px;
    color: var(--color-cartographer-muted);
    margin: 11px 0 22px;
  }
  h3 {
    font-size: 12px;
    font-weight: 500;
    margin: 0;
  }
  .section-label > span {
    color: var(--color-cartographer-muted);
    font-size: 10px;
  }
  .party {
    list-style: none;
    padding: 0;
    margin: 10px 0 20px;
    display: grid;
    gap: 5px;
  }
  .party li {
    display: flex;
    align-items: center;
    gap: 10px;
    background: var(--color-cartographer-field);
    border: 1px solid #2d333a;
    border-radius: 5px;
    padding: 10px 12px;
  }
  .slot {
    color: #8694a0;
    font-size: 10px;
    font-family: var(--font-cartographer-mono);
  }
  .party strong {
    font-weight: 500;
    font-size: 13px;
  }
  .member-level {
    margin-left: auto;
    font-size: 12px;
    color: #b9d8be;
  }
  .member-level.over-cap {
    color: var(--accent);
  }
  .warnings {
    color: var(--accent);
    font-size: 11px;
    padding-left: 16px;
    margin: -6px 0 20px;
    line-height: 1.5;
  }
  .legend {
    display: flex;
    gap: 10px;
    font-size: 10px;
  }
  .ace-key {
    color: var(--accent);
  }
  .cap-key {
    color: #99adbd;
  }
  .growth-chart {
    width: 100%;
    margin: 10px 0 16px;
  }
  .growth-chart text {
    fill: #a4abb4;
    font-size: 9px;
  }
  .chart-grid {
    stroke: #2d343d;
    stroke-width: 1;
  }
  .cap-line {
    fill: none;
    stroke: #99adbd;
    stroke-width: 1.5;
    stroke-dasharray: 4 3;
  }
  .ace-line {
    fill: none;
    stroke: var(--accent);
    stroke-width: 2;
  }
  .position-line {
    stroke: #646260;
    stroke-dasharray: 2 3;
  }
  .ace-dot {
    fill: var(--accent);
    stroke: var(--surface);
    stroke-width: 2;
  }
  details {
    border-top: 1px solid var(--line);
    padding-top: 13px;
    margin-top: 14px;
  }
  summary {
    cursor: pointer;
    font-size: 12px;
    font-weight: 500;
    line-height: 1.6;
  }
  summary span {
    float: right;
    color: var(--color-cartographer-muted);
    font-size: 10px;
    font-weight: 400;
  }
  .reference-panel ul {
    list-style: none;
    padding: 0;
    margin: 10px 0;
  }
  .reference-panel li {
    display: flex;
    flex-wrap: wrap;
    justify-content: space-between;
    gap: 3px;
    padding: 8px 0;
    border-bottom: 1px solid #2b3037;
    font-size: 12px;
  }
  .reference-panel strong {
    font-weight: 500;
  }
  .reference-panel small {
    width: 100%;
    color: var(--color-cartographer-muted);
    font-size: 10px;
    line-height: 1.5;
  }
  .source-path {
    font-family: var(--font-cartographer-mono);
    font-size: 9px;
    overflow-wrap: anywhere;
  }
  .reference-panel p {
    font-size: 11px;
    overflow-wrap: anywhere;
  }
  .rating-inputs {
    display: grid;
    grid-template-columns: repeat(4, 1fr);
    gap: 8px;
    margin: 14px 0;
  }
  .rating-inputs label,
  .growth-input label {
    color: var(--color-cartographer-muted);
    font-size: 10px;
  }
  .rating-inputs input {
    width: 100%;
    min-width: 0;
    margin-top: 5px;
    padding: 8px;
    font-size: 13px;
  }
  .growth-input label {
    display: flex;
    align-items: center;
    gap: 8px;
  }
  .growth-input input {
    width: 51px;
    padding: 7px;
  }
  .advanced {
    border: 0;
    padding: 0;
    margin: 12px 0 0;
  }
  textarea {
    display: block;
    width: 100%;
    padding: 10px;
    font-family: var(--font-cartographer-mono);
    font-size: 11px;
    line-height: 1.5;
    margin: 10px 0;
    resize: vertical;
  }
  .advanced textarea {
    min-height: 250px;
  }
  .global-settings {
    background: var(--surface);
    border: 1px solid var(--line);
    border-radius: 8px;
    padding: 16px 20px;
    margin-top: 24px;
  }
  .global-settings label {
    font-size: 12px;
  }
  .global-settings .actions {
    justify-content: flex-start;
  }
  .visually-hidden {
    position: absolute;
    width: 1px;
    height: 1px;
    padding: 0;
    margin: -1px;
    overflow: hidden;
    clip: rect(0, 0, 0, 0);
    white-space: nowrap;
    border: 0;
  }
  @media (max-width: 1150px) {
    .world-panel {
      gap: 18px;
      grid-template-columns: 90px minmax(150px, 1fr);
    }
    .world-facts {
      grid-column: 1 / -1;
      border-left: 0;
      border-top: 1px solid var(--line);
      padding: 15px 0 0;
      justify-content: space-between;
    }
    .workspace {
      grid-template-columns: minmax(390px, 1.2fr) minmax(320px, 1fr);
      gap: 16px;
    }
    .filters {
      flex-wrap: wrap;
    }
    .filters input {
      flex-basis: 100%;
    }
    .filters select {
      flex: 1;
    }
    .pool-footer {
      flex-wrap: wrap;
    }
    .growth-input {
      flex-wrap: wrap;
    }
  }
  @media (max-width: 800px) {
    .workspace {
      grid-template-columns: 1fr;
    }
    .pool-scroll {
      max-height: 330px;
    }
    .lab-header {
      align-items: flex-start;
      flex-direction: column;
      gap: 15px;
    }
    .lab-header p {
      max-width: 540px;
    }
    .trainer-panel {
      padding: 18px;
    }
    .world-panel {
      padding: 18px;
    }
    .filters {
      flex-wrap: nowrap;
    }
    .filters input {
      flex-basis: auto;
    }
  }
  @media (max-width: 480px) {
    .balance-lab {
      padding-top: 23px;
    }
    .filters {
      flex-wrap: wrap;
    }
    .filters input {
      flex-basis: 100%;
    }
    .world-panel {
      grid-template-columns: 66px 1fr;
      gap: 12px;
    }
    .badge-count strong {
      font-size: 39px;
    }
    .badge-count div > span {
      font-size: 10px;
      margin-left: 4px;
    }
    .world-facts {
      gap: 12px;
    }
    .world-facts strong {
      font-size: 20px;
    }
    .hint {
      font-size: 10px;
    }
    .pool-footer {
      font-size: 9px;
    }
    .lab-header .actions {
      width: 100%;
    }
    .lab-header .primary {
      flex: 1;
    }
  }
</style>

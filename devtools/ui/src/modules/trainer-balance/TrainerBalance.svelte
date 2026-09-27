<script lang="ts">
  import { onMount } from "svelte"
  import { ARC_CHECKPOINTS, ARC_IDS, FILLER_OFFSET, MAX_EDITIONS, MAX_JITTER } from "./engine.js"
  import { BalanceLab, catalog } from "./lab.svelte.js"
  import type { ArcId, LineStage, Role } from "./types.js"

  const lab = new BalanceLab()
  const ticks = [0, 4, 8, 12, 16, 20, 24]
  let importInput: HTMLInputElement
  const signed = (value: number): string => (value > 0 ? `+${value}` : `${value}`)
  const tone = (gap: number): string => (gap > 0 ? "above" : gap < 0 ? "below" : "even")
  const slug = (value: string): string => value.toLowerCase().replaceAll(" ", "-")
  const roleLabel: Record<Role, string> = {
    contender: "Contender",
    elite: "Elite",
    headliner: "Headliner",
  }
  const windowLabel = (role: Role): string =>
    role === "contender"
      ? `≤ ${signed(lab.roleWindows.contenderMax)}`
      : role === "headliner"
        ? `≥ ${signed(lab.roleWindows.headlinerMin)}`
        : lab.roleWindows.contenderMax + 1 === lab.roleWindows.headlinerMin - 1
          ? signed(lab.roleWindows.contenderMax + 1)
          : `${signed(lab.roleWindows.contenderMax + 1)} … ${signed(lab.roleWindows.headlinerMin - 1)}`
  const x = (progress: number): number => 24 + progress * 6.5
  const y = (level: number): number => 140 - level * 1.2
  const capLine = (): string =>
    lab.curve.map((point) => `${x(point.progress)},${y(point.cap)}`).join(" ")
  const arcLine = (arc: ArcId): string =>
    lab.curve.map((point) => `${x(point.progress)},${y(point.aces[arc] ?? 0)}`).join(" ")
  const editionLabel = (editions: number): string =>
    editions === MAX_EDITIONS ? `${editions}+` : `${editions}`
  const blueStatus = (): string =>
    !lab.gymSummary.blue?.ahead ? "No" : lab.gymSummary.blue.atCeiling ? "At ceiling" : "Yes"
  const download = (): void => {
    const url = URL.createObjectURL(new Blob([lab.exportText()], { type: "application/json" }))
    const link = document.createElement("a")
    link.href = url
    link.download = `wayfarer-balance-p${lab.progress}.json`
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
  const lineText = (line: LineStage[]): string =>
    line
      .map((stage, index) => (index ? `${stage.species} (${stage.level})` : stage.species))
      .join(" → ")
  const plural = (count: number, word: string): string =>
    `${count} ${word}${count === 1 ? "" : "s"}`
  onMount(lab.load)
</script>

<section class="balance-lab" aria-label="Trainer balance explorer">
  <header class="lab-header">
    <div>
      <div class="eyebrow">Wayfarer / design tools <span class="prototype">Experimental</span></div>
      <h1>Trainer balance</h1>
      <p>
        Move through the badge journey. See where each rival stands against the world cap, and
        whether every league can fill its field.
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
      <label title="Fully completed circuit editions. Needs 24 badges and 3 first clears."
        >Completed editions<select
          aria-label="Completed editions"
          aria-describedby="editions-hint"
          value={lab.completedEditions}
          disabled={!lab.editionsEnabled}
          onchange={(event) => lab.setEditions(Number(event.currentTarget.value))}
          >{#each Array.from({ length: MAX_EDITIONS + 1 }, (_, index) => index) as editions}<option
              value={editions}>{editionLabel(editions)}</option
            >{/each}</select
        ></label
      >
      <div>
        <span class="eyebrow">Progress p</span><strong data-testid="progress-index"
          >{lab.progress}</strong
        >
      </div>
      <div>
        <span class="eyebrow">Player TR</span><strong data-testid="player-tr">{lab.playerTR}</strong
        >
      </div>
      <div>
        <span class="eyebrow">World cap</span><strong data-testid="player-cap">Lv. {lab.cap}</strong
        >
      </div>
      <div>
        <span class="eyebrow">Level base</span><strong
          data-testid="level-base"
          class:ceiling={lab.levelBase < lab.cap}>Lv. {lab.levelBase}</strong
        >
      </div>
    </div>
    <span class="hint editions-hint" id="editions-hint"
      >{lab.editionsEnabled
        ? "Post-game: each completed edition adds 8 to p (up to 3, p = 48), then standings stop changing."
        : "Completed editions unlock at 24 badges and 3 first clears, since finishing an edition needs both."}
      Level base = min(world cap, 100 − headroom {lab.headroom}).</span
    >
  </div>

  <div class="model-note">
    <span class="note-dot"></span><span
      >Living Rivals model: strength level = level base + standing, where standing = bias + growth
      arc at p = badges + 8 × completed editions, and level base = min(world cap, 100 − headroom).
      The top ace sits at the strength level, and team size follows it. The world cap is the player
      soft cap from badges and first clears alone. This models species, team size and levels; moves,
      items, AI and win rates are not simulated.</span
    >
  </div>

  {#if lab.error}<div role="alert" class="message error">{lab.error}</div>{/if}
  {#if lab.notice}<div role="status" class="message">{lab.notice}</div>{/if}

  <section class="roster-bar" aria-label="Save seed and gameplay flags">
    <label
      >Save seed<input
        aria-label="Save seed"
        type="text"
        inputmode="numeric"
        value={lab.seed}
        onchange={(event) => lab.setSeed(event.currentTarget.value)}
      /></label
    >
    <fieldset class="flag-choices">
      <legend>Gameplay flags</legend>
      {#each lab.knownFlags as flag (flag)}<label
          ><input
            type="checkbox"
            aria-label={`Flag ${flag}`}
            checked={lab.flags.includes(flag)}
            onchange={(event) => lab.toggleFlag(flag, event.currentTarget.checked)}
          />{flag}</label
        >{:else}<span class="hint"
          >None yet. Add a modifier or a filler requirement to create a flag.</span
        >{/each}
    </fieldset>
    <span class="hint"
      >Filler jitter is 0–{lab.jitter}. The explorer hashes seed + trainer + filler; the ROM draws
      it from the TRAINER_ROSTER / FILLER_JITTER seed key, so values differ from the game.</span
    >
  </section>

  <section class="summary-strip" aria-label="Gym and cap summary">
    <div>
      <span class="eyebrow">Gym leaders · strength − cap</span>
      <strong class="gap-value {tone(Math.round(lab.gymSummary.mean))}" data-testid="gym-gap-mean"
        >{lab.gymSummary.mean > 0 ? "+" : ""}{lab.gymSummary.mean.toFixed(1)}</strong
      >
      <span class="hint">Mean of {lab.gymSummary.count} leaders on their selected arcs</span>
    </div>
    <div>
      <span class="eyebrow">Range</span>
      <strong data-testid="gym-gap-range"
        >{signed(lab.gymSummary.min)} … {signed(lab.gymSummary.max)}</strong
      >
      <span class="hint"
        >Every allowed arc: {signed(lab.gymSummary.arcMin)} … {signed(lab.gymSummary.arcMax)}</span
      >
    </div>
    {#if lab.gymSummary.blue}
      <div>
        <span class="eyebrow"
          >{lab.gymSummary.blue.atCeiling
            ? "Blue above the level base"
            : "Blue ahead of the cap"}</span
        >
        <strong
          class="gap-value {lab.gymSummary.blue.ahead ? 'above' : 'risk'}"
          data-testid="blue-check">{blueStatus()}</strong
        >
        <span class="hint"
          >{#if lab.gymSummary.blue.atCeiling}vs base Lv. {lab.gymSummary.blue.levelBase}: {lab.gymSummary.blue.gaps
              .map(({ arc, baseGap }) => `${arc} ${signed(baseGap)}`)
              .join(" · ")}{:else}{lab.gymSummary.blue.gaps
              .map(({ arc, gap }) => `${arc} ${signed(gap)}`)
              .join(" · ")}{/if}{lab.gymSummary.blue.ahead ? "" : " · needs tuning"}</span
        >
      </div>
    {/if}
  </section>

  <div class="workspace">
    <section class="pool-panel" aria-label="Trainer pool">
      <div class="panel-title">
        <h2>The pool <span>{catalog.length}</span></h2>
        <span class="hint"
          >Levels at world cap Lv. {lab.cap}{lab.levelBase < lab.cap
            ? ` · base Lv. ${lab.levelBase}`
            : ""}</span
        >
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
              ><th>Trainer</th><th>Arc</th><th>Standing</th><th>Strength</th><th>Strength − cap</th
              ><th>Role</th><th>Team</th></tr
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
                <td
                  ><select
                    class="arc-select"
                    aria-label={`${row.trainer.name} arc`}
                    value={row.arc}
                    disabled={lab.arcsFor(row.trainer.id).length < 2}
                    onchange={(event) => lab.setArc(row.trainer.id, event.currentTarget.value)}
                    >{#each lab.arcsFor(row.trainer.id) as arc}<option value={arc}>{arc}</option
                      >{/each}</select
                  ></td
                >
                <td class="numeric standing">{signed(row.standing)}</td>
                <td><span class="level-pill">Lv. {row.strengthLevel}</span></td>
                <td
                  ><span class="gap-chip {tone(row.gap)}" data-testid={`gap-${row.trainer.id}`}
                    >{signed(row.gap)}</span
                  ></td
                >
                <td><span class="role-chip {row.role}">{roleLabel[row.role]}</span></td>
                <td
                  class="numeric"
                  title={`${plural(row.acesUsed, "ace")} + ${plural(row.party.length - row.acesUsed, "filler")}`}
                  >{row.party.length}<span class="muted"> / 6</span></td
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
          <span>Standing</span><strong data-testid="selected-standing"
            >{signed(lab.selected.standing)}</strong
          >
        </div>
      </div>
      <div class="home-leagues">
        Home pool: {lab.selected.trainer.homeLeagues.join(" · ") || "none"} · Sevii Masters is open to
        all
      </div>
      <dl class="stat-grid">
        <div>
          <dt>Arc</dt>
          <dd data-testid="selected-arc">{lab.selected.arc}</dd>
        </div>
        <div>
          <dt>Strength</dt>
          <dd data-testid="selected-strength">Lv. {lab.selected.strengthLevel}</dd>
        </div>
        <div>
          <dt>Strength − cap</dt>
          <dd class="gap-value {tone(lab.selected.gap)}" data-testid="selected-gap">
            {signed(lab.selected.gap)}
          </dd>
        </div>
        <div>
          <dt>Role</dt>
          <dd data-testid="selected-role">{roleLabel[lab.selected.role]}</dd>
        </div>
      </dl>
      <dl class="stat-grid team-grid">
        <div>
          <dt>Team size</dt>
          <dd data-testid="team-size">{lab.selected.size}</dd>
        </div>
        <div>
          <dt>Max aces</dt>
          <dd data-testid="ace-allowance">{lab.selected.allowance}</dd>
        </div>
        <div>
          <dt>Aces used</dt>
          <dd data-testid="aces-used">{lab.selected.acesUsed}</dd>
        </div>
        <div>
          <dt>Fillers</dt>
          <dd data-testid="filler-count">{lab.selected.party.length - lab.selected.acesUsed}</dd>
        </div>
      </dl>
      <p class="hint allowance-note" data-testid="allowance-note">
        Size {lab.selected.size} from strength Lv. {lab.selected.strengthLevel}. Up to {plural(
          lab.selected.allowance,
          "ace",
        )} may play; {lab.selected.trainer.name} has {plural(
          lab.settings.roster.aces.length,
          "ace",
        )}{lab.selected.allowance > lab.selected.acesUsed
          ? `, so ${plural(lab.selected.allowance - lab.selected.acesUsed, "unused ace slot")} ${lab.selected.allowance - lab.selected.acesUsed === 1 ? "goes" : "go"} to fillers`
          : ""}.
      </p>
      <div class="section-label">
        <h3>
          Team at {lab.badges} badges{lab.completedEditions
            ? ` · ${editionLabel(lab.completedEditions)} editions`
            : ""}
        </h3>
        <span>Battle order: fillers by score, top ace last</span>
      </div>
      <ol class="party" data-testid="generated-party">
        {#each lab.selected.party as member, index (member.id)}<li
            class:ace={member.kind === "ace"}
          >
            <span class="slot">{String(index + 1).padStart(2, "0")}</span>
            <div class="member-name">
              <strong>{member.species}</strong><small
                >{member.kind === "ace"
                  ? `Ace ${member.priority}`
                  : `Filler · score ${member.score}`} · {member.moves}</small
              >
            </div>
            <span class="member-level" class:over-cap={member.level > lab.cap}
              >Lv. {member.level}</span
            >
          </li>{/each}
      </ol>
      {#if lab.selected.warnings.length > 0}<ul class="warnings">
          {#each lab.selected.warnings as warning}<li>{warning}</li>{/each}
        </ul>{/if}

      <div class="section-label">
        <h3>
          Roster{#if lab.settings.roster.prototype}<span class="prototype-chip">Prototype</span
            >{/if}
        </h3>
        <span
          >{plural(lab.settings.roster.aces.length, "ace")} · {plural(
            lab.settings.roster.fillers.length,
            "filler",
          )}</span
        >
      </div>
      <table class="roster-table" data-testid="roster-aces">
        <thead><tr><th>Ace</th><th>Line (evolve level)</th><th>In team</th></tr></thead>
        <tbody>
          {#each lab.settings.roster.aces as ace, index (ace.id)}
            <tr class:in-team={index < lab.selected.acesUsed}>
              <td class="numeric">{index + 1}</td>
              <td>{lineText(ace.line)}</td>
              <td>{index < lab.selected.acesUsed ? "Yes" : "No"}</td>
            </tr>
          {/each}
        </tbody>
      </table>
      <div class="roster-scroll cartographer-scrollbar">
        <table class="roster-table" data-testid="roster-fillers">
          <thead
            ><tr
              ><th>Filler line</th><th>Base</th><th>Jitter</th><th>Mod</th><th>Score</th><th
                >Rank</th
              ><th>In team</th></tr
            ></thead
          >
          <tbody>
            {#each lab.selected.fillerScores as entry (entry.filler.id)}
              <tr
                class:in-team={entry.inTeam}
                class:locked={!entry.eligible}
                data-testid={`filler-${entry.filler.id}`}
              >
                <td
                  >{lineText(entry.filler.line)}{#if entry.filler.requiresFlag}<small
                      >Needs {entry.filler.requiresFlag}</small
                    >{/if}</td
                >
                <td class="numeric">{entry.filler.baseScore}</td>
                <td class="numeric">{entry.jitter}</td>
                <td class="numeric">{entry.modifier ? signed(entry.modifier) : "0"}</td>
                <td class="numeric score">{entry.score}</td>
                <td class="numeric">{entry.rank ?? "—"}</td>
                <td>{entry.inTeam ? "Yes" : entry.eligible ? "No" : "Locked"}</td>
              </tr>
            {:else}
              <tr><td colspan="7" class="muted">No fillers. Add them in the JSON editor.</td></tr>
            {/each}
          </tbody>
        </table>
      </div>

      <div class="section-label">
        <h3>Progress journey</h3>
        <div class="legend">
          <span class="ace-key">{lab.selected.arc}</span
          >{#if lab.settings.allowedArcs.length > 1}<span class="alt-key">Other allowed arcs</span
            >{/if}<span class="cap-key">World cap</span>
        </div>
      </div>
      <svg
        class="growth-chart"
        viewBox="0 0 360 163"
        role="img"
        aria-label={`${lab.selected.trainer.name} strength level per allowed arc and the world cap from progress 0 to 48; progress 32 to 48 are completed editions at 24 badges and 3 clears`}
      >
        {#each [20, 40, 60, 80, 100] as level}<line
            x1="24"
            x2="336"
            y1={y(level)}
            y2={y(level)}
            class="chart-grid"
          /><text x="2" y={y(level) + 4}>{level}</text>{/each}
        <polyline points={capLine()} class="cap-line" />
        {#each lab.settings.allowedArcs.filter((arc) => arc !== lab.selected.arc) as arc}<polyline
            points={arcLine(arc)}
            class="alt-line"
          />{/each}
        <polyline points={arcLine(lab.selected.arc)} class="ace-line" />
        <line x1={x(24)} x2={x(24)} y1="18" y2="140" class="chart-grid" />
        <text x={x(36)} y="14" text-anchor="middle">post-game editions</text>
        <line x1={x(lab.progress)} x2={x(lab.progress)} y1="18" y2="140" class="position-line" />
        <circle cx={x(lab.progress)} cy={y(lab.selected.strengthLevel)} r="4" class="ace-dot" />
        {#each ARC_CHECKPOINTS as progress}<text x={x(progress)} y="158" text-anchor="middle"
            >{progress}</text
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
        <p class="hint">Roster provenance: {lab.selected.trainer.rosterSource}</p>
      </details>

      <details class="tuning-panel" open>
        <summary>Tune this trainer</summary>
        {#key lab.settings}
          <form
            onsubmit={(event) => {
              event.preventDefault()
              lab.applyTrainer(event.currentTarget)
            }}
          >
            <div class="growth-input">
              <label
                >Standing bias<input
                  aria-label="Standing bias"
                  name="bias"
                  type="number"
                  min="-6"
                  max="6"
                  step="1"
                  required
                  value={lab.settings.bias}
                /></label
              ><button type="submit">Apply standing</button>
            </div>
            <fieldset class="arc-choices">
              <legend>Allowed arcs</legend>
              {#each ARC_IDS as arc}<label
                  ><input
                    type="checkbox"
                    name={`allow-${arc}`}
                    aria-label={`Allow ${arc} arc`}
                    checked={lab.settings.allowedArcs.includes(arc)}
                  />{arc}</label
                >{/each}
            </fieldset>
          </form>
        {/key}
        <p class="hint">
          Standing is bias plus the arc’s delta at the current progress index p. Changes are saved
          in this browser and included in exports.
        </p>
        <button type="button" onclick={lab.resetTrainer}>Restore this trainer’s defaults</button>
        <details class="advanced">
          <summary>Edit roster</summary>
          <p class="hint">
            Aces fill first in priority order. Evolve levels are the level where each later species
            on a line is reached. Fillers score base + jitter + modifiers; offsets run {FILLER_OFFSET.min}
            to {FILLER_OFFSET.max}. A filler with a flag stays out until the flag is set.
          </p>
          <ol class="ace-order">
            {#each lab.settings.roster.aces as ace, index (ace.id)}<li>
                <span>{index + 1}. {ace.line.at(-1)?.species}</span><button
                  type="button"
                  aria-label={`Move ${ace.id} up`}
                  disabled={index === 0}
                  onclick={() => lab.moveAce(index, -1)}>↑</button
                ><button
                  type="button"
                  aria-label={`Move ${ace.id} down`}
                  disabled={index === lab.settings.roster.aces.length - 1}
                  onclick={() => lab.moveAce(index, 1)}>↓</button
                >
              </li>{/each}
          </ol>
          {#key lab.settings}
            <form
              class="roster-form"
              onsubmit={(event) => {
                event.preventDefault()
                lab.applyRoster(event.currentTarget)
              }}
            >
              {#each lab.settings.roster.aces as ace (ace.id)}
                <fieldset>
                  <legend>Ace {ace.id}</legend>
                  <div class="line-inputs">
                    {#each ace.line as stage, index}{#if index}<label
                          >{stage.species} at<input
                            aria-label={`${ace.id} evolves to ${stage.species} at`}
                            name={`ace-${ace.id}-level-${index}`}
                            type="number"
                            min="2"
                            max="100"
                            step="1"
                            required
                            value={stage.level}
                          /></label
                        >{:else}<span>{stage.species}</span>{/if}{/each}
                  </div>
                </fieldset>
              {/each}
              {#each lab.settings.roster.fillers as filler (filler.id)}
                <fieldset>
                  <legend>Filler {filler.id}</legend>
                  <div class="line-inputs">
                    <label
                      >Base<input
                        aria-label={`${filler.id} base score`}
                        name={`filler-${filler.id}-score`}
                        type="number"
                        min="0"
                        max="100"
                        step="1"
                        required
                        value={filler.baseScore}
                      /></label
                    ><label
                      >Offset<input
                        aria-label={`${filler.id} level offset`}
                        name={`filler-${filler.id}-offset`}
                        type="number"
                        min={FILLER_OFFSET.min}
                        max={FILLER_OFFSET.max}
                        step="1"
                        required
                        value={filler.levelOffset}
                      /></label
                    >{#each filler.line as stage, index}{#if index}<label
                          >{stage.species} at<input
                            aria-label={`${filler.id} evolves to ${stage.species} at`}
                            name={`filler-${filler.id}-level-${index}`}
                            type="number"
                            min="2"
                            max="100"
                            step="1"
                            required
                            value={stage.level}
                          /></label
                        >{/if}{/each}
                    <label class="wide"
                      >Signature move<input
                        aria-label={`${filler.id} signature move`}
                        name={`filler-${filler.id}-signature`}
                        type="text"
                        value={filler.signatureMove ?? ""}
                      /></label
                    ><label class="wide"
                      >Requires flag<input
                        aria-label={`${filler.id} requires flag`}
                        name={`filler-${filler.id}-flag`}
                        type="text"
                        placeholder="none"
                        value={filler.requiresFlag ?? ""}
                      /></label
                    >
                  </div>
                </fieldset>
              {/each}
              <label class="prototype-toggle"
                ><input
                  type="checkbox"
                  name="roster-prototype"
                  aria-label="Prototype roster"
                  checked={lab.settings.roster.prototype}
                />Prototype roster (clear once authored)</label
              >
              <button type="submit">Apply roster</button>
            </form>
          {/key}
        </details>
        <details class="advanced">
          <summary>Edit settings as JSON</summary>
          <p class="hint">
            Add or remove aces and fillers, rename species, or author moves, items, abilities and
            natures per ace form. IDs are lowercase slugs, unique within the roster.
          </p>
          <label class="visually-hidden" for="team-editor">Trainer settings JSON</label><textarea
            id="team-editor"
            spellcheck="false"
            bind:value={lab.editor}></textarea><button type="button" onclick={lab.applyTeam}
            >Apply settings</button
          >
        </details>
      </details>
    </aside>
  </div>

  <section class="feasibility" aria-label="League feasibility">
    <div class="panel-title">
      <h2>League feasibility <span>at p = {lab.progress}</span></h2>
      <span class="hint" data-testid="roster-gap-count"
        >{lab.gaps.length
          ? `${plural(lab.gaps.length, "roster")} can’t reach 6 at max size`
          : "Every roster reaches 6 at max size"}</span
      >
    </div>
    <p class="hint feasibility-note">
      Guaranteed: in the role window under every allowed arc. Possible: under some allowed arc.
      Current: under the selected arcs. Every trainer is a candidate and brings a team sized by
      their strength level. Validation needs every roster to reach 6 at max size (up to 3 aces plus
      unlocked fillers), so shorter rosters are listed as content gaps. Standing depends on p
      (badges + 8 × completed editions) only, so league clears and headroom do not change these
      counts.
    </p>
    <div class="venue-grid">
      {#each lab.feasibility as venue (venue.venue)}
        <div class="venue-card" data-testid={`feasibility-${slug(venue.venue)}`}>
          <div class="venue-heading">
            <h3>{venue.venue}</h3>
            <span class="hint"
              >{venue.pool === "open" ? "Open invitational" : "Home pool"} · {venue.candidates} candidates</span
            >
          </div>
          <table class="feasibility-table">
            <thead
              ><tr
                ><th>Role</th><th>Window</th><th>Need</th><th>Guaranteed</th><th>Possible</th><th
                  >Current</th
                ></tr
              ></thead
            >
            <tbody>
              {#each venue.roles as entry (entry.role)}
                <tr
                  class:risk={entry.fallbackRisk}
                  data-testid={`${slug(venue.venue)}-${entry.role}`}
                >
                  <td
                    >{roleLabel[entry.role]}{#if entry.fallbackRisk}<span class="risk-flag"
                        >Fallback risk</span
                      >{/if}</td
                  >
                  <td class="numeric muted">{windowLabel(entry.role)}</td>
                  <td class="numeric">{entry.need}</td>
                  <td class="numeric guaranteed">{entry.guaranteed}</td>
                  <td class="numeric">{entry.possible}</td>
                  <td class="numeric">{entry.current}</td>
                </tr>
              {/each}
            </tbody>
          </table>
          {#if venue.gaps.length}<p class="hint excluded" data-testid={`gaps-${slug(venue.venue)}`}>
              Rosters short of 6: {venue.gaps.map((gap) => `${gap.name} ${gap.reach}/6`).join(", ")}
            </p>{/if}
        </div>
      {/each}
    </div>
  </section>

  <details class="global-settings">
    <summary>Growth arcs, role windows & experiment settings</summary>
    <p>
      Each arc is an arcDelta tuple at progress p = 0 / 8 / 16 / 24 / 32 / 40 / 48, interpolated
      linearly and rounded half up. p is badges in edition 1 (0–24); editions 2, 3 and 4+ enter at p
      = 32, 40 and 48, after which standings stop changing. Every arc stays 0 at p = 0, so first
      encounters match in every save. The player cap is unchanged. League clears can be combined
      with any badge count here; this tool does not enforce entry requirements.
    </p>
    {#key lab.arcs}
      <form
        onsubmit={(event) => {
          event.preventDefault()
          lab.applyWorld(event.currentTarget)
        }}
      >
        <div class="arc-scroll cartographer-scrollbar">
          <table class="arc-table">
            <thead
              ><tr
                ><th>Arc</th>{#each ARC_CHECKPOINTS as progress}<th>p = {progress}</th>{/each}</tr
              ></thead
            >
            <tbody>
              {#each ARC_IDS as arc}
                <tr>
                  <th scope="row">{arc}</th>
                  {#each ARC_CHECKPOINTS as progress, index}<td
                      >{#if progress === 0}<span class="muted">0</span>{:else}<input
                          aria-label={`${arc} arc at p ${progress}`}
                          name={`arc-${arc}-${progress}`}
                          type="number"
                          min="-12"
                          max="12"
                          step="1"
                          required
                          value={lab.arcs[arc][index]}
                        />{/if}</td
                    >{/each}
                </tr>
              {/each}
            </tbody>
          </table>
        </div>
        <div class="window-inputs">
          <label
            >Contender at most<input
              aria-label="Contender maximum standing"
              name="contender-max"
              type="number"
              min="-12"
              max="12"
              step="1"
              required
              value={lab.roleWindows.contenderMax}
            /></label
          >
          <label
            >Headliner at least<input
              aria-label="Headliner minimum standing"
              name="headliner-min"
              type="number"
              min="-12"
              max="12"
              step="1"
              required
              value={lab.roleWindows.headlinerMin}
            /></label
          >
          <span class="hint">Elite covers the standings in between.</span>
        </div>
        <div class="window-inputs">
          <label
            >Level headroom<input
              aria-label="Level headroom"
              name="headroom"
              type="number"
              min="0"
              max="20"
              step="1"
              required
              value={lab.headroom}
            /></label
          >
          <span class="hint"
            >Level base = min(world cap, 100 − headroom). No effect while the cap is at most {100 -
              lab.headroom}; at the ceiling it leaves room for elites and headliners above
            contenders. Ace − cap still compares with the player cap.</span
          >
        </div>
        <div class="actions">
          <button type="submit">Apply arcs, windows & headroom</button><button
            type="button"
            onclick={lab.reset}>Reset all trainer defaults</button
          >
        </div>
      </form>
    {/key}
    <h3 class="settings-heading">Team size, jitter & modifiers</h3>
    <p>
      Team size = sizeFor(strength level). The ace allowance is a maximum tied to size (1–3 members:
      1, 4–5: 2, 6: 3), and unused ace slots go to fillers. Sizes must not decrease, so with fixed
      scores growth never removes a filler.
    </p>
    {#key lab.sizeTable}
      <form
        onsubmit={(event) => {
          event.preventDefault()
          lab.applyRosterRules(event.currentTarget)
        }}
      >
        <div class="window-inputs size-inputs">
          {#each lab.sizeTable as step, index}<label
              >{#if index === 0}From Lv. 1{:else}From Lv.<input
                  aria-label={`Size row ${index + 1} minimum strength level`}
                  name={`size-level-${index}`}
                  type="number"
                  min="2"
                  max="100"
                  step="1"
                  required
                  value={step.minLevel}
                />{/if}<input
                aria-label={`Size row ${index + 1} team size`}
                name={`size-${index}`}
                type="number"
                min="1"
                max="6"
                step="1"
                required
                value={step.size}
              /></label
            >{/each}
        </div>
        <div class="window-inputs">
          <label
            >Filler jitter (JITTER)<input
              aria-label="Filler jitter"
              name="jitter"
              type="number"
              min="0"
              max={MAX_JITTER}
              step="1"
              required
              value={lab.jitter}
            /></label
          ><button type="submit">Apply sizes & jitter</button>
        </div>
      </form>
    {/key}
    <p>
      Modifiers add a delta to one trainer’s filler score while a gameplay flag is set (for example
      telling Misty where Lapras lives). Toggle flags above the pool.
    </p>
    {#if lab.modifiers.length}<ul class="modifier-list" data-testid="modifier-list">
        {#each lab.modifiers as modifier, index}<li>
            <span
              >{modifier.flag} → {modifier.trainer}/{modifier.filler}
              <strong>{signed(modifier.delta)}</strong></span
            ><button
              type="button"
              aria-label={`Remove modifier ${index + 1}`}
              onclick={() => lab.removeModifier(index)}>Remove</button
            >
          </li>{/each}
      </ul>{/if}
    <form
      class="window-inputs"
      onsubmit={(event) => {
        event.preventDefault()
        lab.addModifier(event.currentTarget)
      }}
    >
      <label
        >Flag<input
          aria-label="Modifier flag"
          name="modifier-flag"
          type="text"
          required
          placeholder="FLAG_NAME"
        /></label
      >
      <label
        >Filler<select aria-label="Modifier filler" name="modifier-target" required
          >{#each catalog as trainer (trainer.id)}<optgroup label={trainer.name}
              >{#each lab.fillersFor(trainer.id) as filler (filler.id)}<option
                  value={`${trainer.id}/${filler.id}`}>{trainer.name} · {filler.id}</option
                >{/each}</optgroup
            >{/each}</select
        ></label
      >
      <label
        >Delta<input
          aria-label="Modifier delta"
          name="modifier-delta"
          type="number"
          min="-100"
          max="100"
          step="1"
          required
          value="40"
        /></label
      ><button type="submit">Add modifier</button>
    </form>
    <p class="hint">
      Arcs are shown for a chosen preview, not sampled from a seed. League lineup generation is not
      implemented. This is separate from the ROM’s current scaler.
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
    flex-wrap: wrap;
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
  .world-facts strong.ceiling {
    color: var(--accent);
  }
  .editions-hint {
    grid-column: 1 / -1;
    margin-top: -14px;
  }
  .arc-scroll {
    max-width: 100%;
    overflow-x: auto;
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
  .growth-input label {
    color: var(--color-cartographer-muted);
    font-size: 10px;
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
  .summary-strip {
    display: grid;
    grid-template-columns: repeat(3, minmax(0, 1fr));
    gap: 1px;
    margin-bottom: 24px;
    border: 1px solid var(--line);
    border-radius: 8px;
    overflow: hidden;
    background: var(--line);
  }
  .summary-strip > div {
    display: flex;
    flex-direction: column;
    gap: 5px;
    padding: 14px 18px;
    background: var(--surface);
  }
  .summary-strip strong {
    font-size: 22px;
    font-weight: 500;
    font-variant-numeric: tabular-nums;
  }
  .gap-value.above {
    color: #f0c887;
  }
  .gap-value.below {
    color: #b9d8be;
  }
  .gap-value.even {
    color: var(--color-cartographer-ink);
  }
  .gap-value.risk {
    color: #ffb8bc;
  }
  .gap-chip {
    display: inline-block;
    min-width: 34px;
    text-align: center;
    border-radius: 4px;
    padding: 4px 7px;
    font-size: 11px;
    font-weight: 600;
    font-variant-numeric: tabular-nums;
  }
  .gap-chip.above {
    background: #403224;
    color: #f0c887;
  }
  .gap-chip.below {
    background: #26332c;
    color: #b9d8be;
  }
  .gap-chip.even {
    background: #2b3037;
    color: var(--color-cartographer-ink);
  }
  .role-chip {
    font-size: 11px;
    color: var(--color-cartographer-muted);
  }
  .role-chip.headliner {
    color: var(--accent);
  }
  .role-chip.elite {
    color: #99adbd;
  }
  .standing {
    color: var(--accent);
    font-weight: 600;
  }
  .arc-select {
    padding: 5px 24px 5px 7px;
    font-size: 11px;
  }
  .arc-select:disabled {
    opacity: 0.75;
    cursor: default;
  }
  .stat-grid {
    display: grid;
    grid-template-columns: repeat(4, minmax(0, 1fr));
    gap: 8px;
    margin: 0 0 20px;
  }
  .stat-grid div {
    background: var(--color-cartographer-field);
    border: 1px solid #2d333a;
    border-radius: 5px;
    padding: 8px 10px;
    min-width: 0;
  }
  .stat-grid dt {
    color: var(--color-cartographer-muted);
    font-size: 10px;
  }
  .stat-grid dd {
    margin: 4px 0 0;
    font-size: 14px;
    font-variant-numeric: tabular-nums;
    overflow-wrap: anywhere;
  }
  .alt-key {
    color: #8a7a5c;
  }
  .alt-line {
    fill: none;
    stroke: #8a7a5c;
    stroke-width: 1.25;
    opacity: 0.8;
  }
  .arc-choices {
    display: flex;
    flex-wrap: wrap;
    gap: 6px 14px;
    border: 0;
    padding: 0;
    margin: 0 0 12px;
  }
  .arc-choices legend {
    color: var(--color-cartographer-muted);
    font-size: 10px;
    margin-bottom: 6px;
  }
  .arc-choices label {
    display: flex;
    align-items: center;
    gap: 5px;
    font-size: 12px;
  }
  .arc-choices input {
    accent-color: var(--accent);
  }
  .feasibility {
    margin-top: 24px;
    border: 1px solid var(--line);
    border-radius: 8px;
    background: var(--surface);
    padding-bottom: 18px;
  }
  .feasibility-note {
    margin: 0 18px 14px;
  }
  .venue-grid {
    display: grid;
    grid-template-columns: repeat(3, minmax(0, 1fr));
    gap: 14px;
    padding: 0 18px;
  }
  .venue-card {
    min-width: 0;
    border: 1px solid #2d333a;
    border-radius: 6px;
    background: var(--color-cartographer-field);
    overflow-x: auto;
  }
  .venue-heading {
    display: flex;
    justify-content: space-between;
    align-items: baseline;
    flex-wrap: wrap;
    gap: 6px;
    padding: 12px 12px 8px;
  }
  .feasibility-table {
    width: 100%;
    border-collapse: collapse;
    font-size: 12px;
  }
  .feasibility-table th {
    position: static;
    background: transparent;
    padding: 6px 8px;
  }
  .feasibility-table td {
    padding: 8px;
  }
  .feasibility-table td:first-child,
  .feasibility-table th:first-child {
    padding-left: 12px;
  }
  .guaranteed {
    font-weight: 600;
  }
  tr.risk .guaranteed {
    color: #ffb8bc;
  }
  .risk-flag {
    display: block;
    color: #ffb8bc;
    font-size: 10px;
    margin-top: 2px;
  }
  .excluded {
    margin: 8px 12px 10px;
  }
  .arc-table {
    border-collapse: collapse;
    font-size: 12px;
    margin: 10px 0;
  }
  .arc-table th {
    position: static;
    background: transparent;
    padding: 6px 8px;
    white-space: nowrap;
  }
  .global-settings .actions {
    flex-wrap: wrap;
    justify-content: flex-start;
  }
  .arc-table th[scope="row"] {
    color: var(--color-cartographer-ink);
    font-size: 12px;
    padding-left: 0;
  }
  .arc-table td {
    border-top: 0;
    padding: 4px 8px;
  }
  .arc-table td:first-child {
    padding: 4px 8px;
  }
  .arc-table input,
  .window-inputs input {
    width: 60px;
    padding: 6px 7px;
  }
  .window-inputs {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 12px 18px;
    margin: 6px 0 12px;
  }
  .window-inputs label {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 11px;
    color: var(--color-cartographer-muted);
  }
  .roster-bar {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 12px 24px;
    padding: 14px 18px;
    margin-bottom: 24px;
    border: 1px solid var(--line);
    border-radius: 8px;
    background: var(--surface);
  }
  .roster-bar > label {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 11px;
    color: var(--color-cartographer-muted);
  }
  .roster-bar input[type="text"] {
    width: 110px;
    padding: 7px 9px;
  }
  .roster-bar > .hint {
    flex: 1 1 260px;
  }
  .flag-choices {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 6px 14px;
    border: 0;
    padding: 0;
    margin: 0;
    min-width: 0;
  }
  .flag-choices legend {
    float: left;
    margin-right: 10px;
    color: var(--color-cartographer-muted);
    font-size: 11px;
  }
  .flag-choices label {
    display: flex;
    align-items: center;
    gap: 5px;
    font-size: 11px;
    font-family: var(--font-cartographer-mono);
    overflow-wrap: anywhere;
  }
  .flag-choices input,
  .prototype-toggle input {
    accent-color: var(--accent);
  }
  input[type="text"] {
    border: 1px solid var(--line);
    background: var(--color-cartographer-field);
    color: var(--color-cartographer-ink);
    border-radius: 5px;
    font: inherit;
    font-size: 12px;
    min-width: 0;
  }
  .team-grid {
    margin-bottom: 6px;
  }
  .allowance-note {
    margin: 0 0 16px;
  }
  .member-name {
    display: flex;
    flex-direction: column;
    gap: 2px;
    min-width: 0;
  }
  .member-name small {
    color: var(--color-cartographer-muted);
    font-size: 10px;
    overflow-wrap: anywhere;
  }
  .party li.ace {
    border-color: #4d4331;
  }
  .prototype-chip {
    color: var(--accent);
    border: 1px solid #665239;
    border-radius: 4px;
    padding: 1px 6px;
    margin-left: 8px;
    font-size: 10px;
  }
  .roster-scroll {
    max-width: 100%;
    overflow-x: auto;
    margin-bottom: 20px;
  }
  .roster-table {
    width: 100%;
    border-collapse: collapse;
    font-size: 11px;
    margin: 8px 0 12px;
  }
  .roster-table th {
    position: static;
    background: transparent;
    padding: 6px 8px;
  }
  .roster-table th:first-child,
  .roster-table td:first-child {
    padding-left: 4px;
  }
  .roster-table td {
    padding: 7px 8px;
  }
  .roster-table small {
    display: block;
    color: var(--accent);
    font-size: 10px;
  }
  .roster-table tr.in-team td:last-child {
    color: #b9d8be;
  }
  .roster-table tr.locked {
    color: var(--color-cartographer-muted);
  }
  .roster-table .score {
    font-weight: 600;
  }
  .ace-order {
    list-style: none;
    padding: 0;
    margin: 10px 0;
    display: grid;
    gap: 5px;
  }
  .ace-order li {
    display: flex;
    align-items: center;
    gap: 6px;
    font-size: 12px;
  }
  .ace-order span {
    flex: 1;
  }
  .ace-order button {
    padding: 4px 9px;
  }
  .ace-order button:disabled {
    opacity: 0.4;
    cursor: default;
  }
  .roster-form fieldset {
    border: 1px solid #2d333a;
    border-radius: 5px;
    padding: 8px 10px 10px;
    margin: 0 0 8px;
    min-width: 0;
  }
  .roster-form legend {
    font-size: 11px;
    color: var(--color-cartographer-muted);
    padding: 0 4px;
  }
  .line-inputs {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 8px 14px;
    font-size: 11px;
  }
  .line-inputs label {
    display: flex;
    align-items: center;
    gap: 6px;
    color: var(--color-cartographer-muted);
  }
  .line-inputs input[type="number"] {
    width: 56px;
    padding: 5px 6px;
  }
  .line-inputs label.wide {
    flex: 1 1 150px;
  }
  .line-inputs label.wide input {
    width: 100%;
    padding: 5px 7px;
  }
  .prototype-toggle {
    display: flex;
    align-items: center;
    gap: 6px;
    font-size: 11px;
    margin: 6px 0 10px;
  }
  .settings-heading {
    margin-top: 18px;
  }
  .size-inputs input {
    margin-left: 4px;
  }
  .modifier-list {
    list-style: none;
    padding: 0;
    margin: 8px 0;
    display: grid;
    gap: 5px;
    font-size: 12px;
  }
  .modifier-list li {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 10px;
    font-family: var(--font-cartographer-mono);
    overflow-wrap: anywhere;
  }
  .modifier-list button {
    padding: 4px 9px;
  }
  .window-inputs select {
    padding: 6px 26px 6px 8px;
    max-width: 260px;
  }
  .window-inputs input[type="text"] {
    width: 170px;
    padding: 6px 7px;
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
    .venue-grid {
      grid-template-columns: 1fr;
    }
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
    .summary-strip {
      grid-template-columns: 1fr;
    }
    .stat-grid {
      grid-template-columns: repeat(2, minmax(0, 1fr));
    }
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

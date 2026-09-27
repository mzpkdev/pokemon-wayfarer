<script lang="ts">
  import { onMount } from "svelte"
  import { ARCHETYPES, LEVEL_OFFSET, NEAR_BAND, ROSTER_SIZE, LEAGUES } from "./engine.js"
  import { BalanceLab, catalog, movesText, scalerField, type ScalerId } from "./lab.svelte.js"

  const lab = new BalanceLab()
  const ticks = [0, 4, 8, 12, 16, 20, 24]
  let importInput: HTMLInputElement
  const signed = (value: number): string => (value > 0 ? `+${value}` : `${value}`)
  const tone = (gap: number): string => (gap > 0 ? "above" : gap < 0 ? "below" : "even")
  const scalers: {
    id: ScalerId
    title: string
    by: string
    at: string
    unit: string
    min: number
    max: number
  }[] = [
    {
      id: "teamLevel",
      title: "Team level",
      by: "trainer TR",
      at: "TR",
      unit: "Lv.",
      min: 1,
      max: 100,
    },
    {
      id: "teamSize",
      title: "Team size",
      by: "trainer TR",
      at: "TR",
      unit: "Size",
      min: 1,
      max: ROSTER_SIZE,
    },
    {
      id: "wildLevel",
      title: "Wild level",
      by: "player TR",
      at: "TR",
      unit: "Lv.",
      min: 1,
      max: 100,
    },
    {
      id: "routeTrainerLevel",
      title: "Regular trainer level",
      by: "player TR",
      at: "TR",
      unit: "Lv.",
      min: 1,
      max: 100,
    },
    ...ARCHETYPES.map((id) => ({
      id,
      title: `${id[0]?.toUpperCase()}${id.slice(1)} growth`,
      by: "world progress",
      at: "World",
      unit: "%",
      min: 0,
      max: 100,
    })),
  ]
  const download = (): void => {
    const url = URL.createObjectURL(new Blob([lab.exportText()], { type: "application/json" }))
    const link = document.createElement("a")
    link.href = url
    link.download = `wayfarer-balance-b${lab.badges}.json`
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
      <div class="eyebrow">
        Wayfarer / design tools <span class="prototype">Experimental · v0</span>
      </div>
      <h1>Trainer balance</h1>
      <p>
        Author each notable trainer’s growth (start TR, archetype, peak TR) and six-slot roster, and
        see their TR, team, the Gym ladder and the league lineup at any world progress.
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
      <span class="eyebrow">Player badges</span>
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
        >Player TR: badges 1–8 give +10, 9–24 give +5, league wins give nothing. It sets the level
        cap and world scaling, and it is the world progress notable trainers grow with.</span
      >
    </div>
    <div class="world-facts">
      <div>
        <span class="eyebrow">Player TR</span><strong data-testid="player-tr">{lab.playerTR}</strong
        >
      </div>
      <div>
        <span class="eyebrow">World progress</span><strong data-testid="world-progress"
          >{lab.worldProgress}</strong
        ><small>= player TR</small>
      </div>
      <div>
        <span class="eyebrow">Level cap</span><strong data-testid="level-cap">Lv. {lab.cap}</strong>
      </div>
      <div>
        <span class="eyebrow">Wild level</span><strong data-testid="wild-level"
          >Lv. {lab.world.wild}</strong
        ><small data-testid="wild-gap">{signed(lab.world.wild - lab.cap)} vs cap</small>
      </div>
      <div>
        <span class="eyebrow">Regular trainers</span><strong data-testid="regular-trainer-level"
          >Lv. {lab.world.regularTrainer}</strong
        ><small data-testid="regular-trainer-gap"
          >{signed(lab.world.regularTrainer - lab.cap)} vs cap</small
        >
      </div>
    </div>
  </div>

  <div class="model-note">
    <span class="note-dot"></span><span
      >TR v0: each notable trainer grows with world progress from start TR to peak TR along their
      archetype’s growth scaler, the same rule for every archetype. Team level and team size are
      scalers of that trainer TR (anchor tables, linear between anchors, halves rounded up, flat
      past the last anchor, the ceiling TR; TR itself is uncapped). The team is the first N roster
      slots at team level + offset, and battle order is the team reversed, so roster slot 1 comes
      last. Moves, items, AI and win rates are not simulated.</span
    >
  </div>

  {#if lab.error}<div role="alert" class="message error">{lab.error}</div>{/if}
  {#if lab.notice}<div role="status" class="message">{lab.notice}</div>{/if}

  <section class="summary-strip" aria-label="Roster and cap summary">
    <div>
      <span class="eyebrow">Incomplete rosters</span>
      <strong class="gap-value {lab.gaps.length ? 'risk' : 'even'}" data-testid="roster-gap-count"
        >{lab.gaps.length}</strong
      >
      <span class="hint" data-testid="roster-gap-list"
        >{lab.gaps.length
          ? `Short of ${ROSTER_SIZE}: ${lab.gaps.map((gap) => `${gap.name} ${gap.length}/${ROSTER_SIZE}`).join(", ")}`
          : `Every roster lists ${ROSTER_SIZE} Pokémon`}</span
      >
    </div>
    <div>
      <span class="eyebrow">Above the level cap</span>
      <strong data-testid="above-cap">{lab.aboveCap}</strong>
      <span class="hint">Trainers whose team level exceeds Lv. {lab.cap}</span>
    </div>
    <div>
      <span class="eyebrow">League lineup</span>
      <strong data-testid="league-range"
        >TR {lab.league[0]?.tr ?? "—"} … {lab.league.at(-1)?.tr ?? "—"}</strong
      >
      <span class="hint">Top five by TR at world progress {lab.worldProgress}, one global pool</span
      >
    </div>
  </section>

  <div class="workspace">
    <section class="pool-panel" aria-label="Trainer pool">
      <div class="panel-title">
        <h2>Notable trainers <span>{catalog.length}</span></h2>
        <span class="hint"
          >TR at world progress {lab.worldProgress} · gap vs level cap Lv. {lab.cap}</span
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
              ><th>Trainer</th><th>TR</th><th>Start → peak</th><th>Archetype</th><th>Team level</th
              ><th>Size</th><th>Lv − cap</th><th>Roster</th></tr
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
                    ><strong
                      >{row.trainer.name}{#if row.trainer.doubleBattle}<span
                          class="prototype-chip double-chip">Double battle</span
                        >{/if}</strong
                    ><span>{row.trainer.region} · {row.trainer.role}</span></button
                  ></td
                >
                <td class="numeric current-tr" data-testid={`tr-${row.trainer.id}`}>{row.tr}</td>
                <td class="numeric muted" data-testid={`growth-${row.trainer.id}`}
                  >{row.startTR} → {row.peakTR}</td
                >
                <td class="muted" data-testid={`archetype-${row.trainer.id}`}>{row.archetype}</td>
                <td
                  ><span class="level-pill" data-testid={`level-${row.trainer.id}`}
                    >Lv. {row.teamLevel}</span
                  ></td
                >
                <td class="numeric" data-testid={`size-${row.trainer.id}`}>{row.size}</td>
                <td
                  ><span
                    class="gap-chip {tone(row.teamLevel - lab.cap)}"
                    data-testid={`gap-${row.trainer.id}`}>{signed(row.teamLevel - lab.cap)}</span
                  ></td
                >
                <td class="numeric" class:incomplete={row.rosterLength < ROSTER_SIZE}
                  >{row.rosterLength}<span class="muted"> / {ROSTER_SIZE}</span></td
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
        {lab.rows.length} trainers shown
        <span>Red excluded · Tate & Liza fight doubles and skip leagues</span>
      </div>
    </section>

    <aside class="trainer-panel" aria-label="Selected trainer">
      <div class="selected-heading">
        <div>
          <span class="eyebrow">{lab.selected.trainer.region} · {lab.selected.trainer.role}</span>
          <h2>
            {lab.selected.trainer.name}{#if lab.selected.trainer.doubleBattle}<span
                class="prototype-chip double-chip"
                data-testid="double-battle">Double battle</span
              >{/if}
          </h2>
        </div>
        <div class="tr-badge">
          <span>TR</span><strong data-testid="selected-tr">{lab.selected.tr}</strong>
        </div>
      </div>
      {#if lab.selected.trainer.doubleBattle}<p class="hint" data-testid="double-battle-note">
          A double battle: both leaders send Pokémon from the shared roster in order. Leagues are
          singles only, so this entry is not in the league pool.
        </p>{/if}
      <p class="hint provenance">{lab.selected.trainer.trSource}</p>
      {#key lab.settings}
        <form
          class="growth-form"
          data-testid="growth-editor"
          novalidate
          onsubmit={(event) => {
            event.preventDefault()
            lab.applyGrowth(event.currentTarget)
          }}
        >
          <label
            >Start TR<input
              aria-label="Start TR"
              name="startTR"
              type="number"
              min="0"
              step="1"
              required
              value={lab.settings.startTR}
            /></label
          ><label
            >Archetype<select aria-label="Archetype" name="archetype" value={lab.settings.archetype}
              >{#each ARCHETYPES as archetype}<option value={archetype}>{archetype}</option
                >{/each}</select
            ></label
          ><label
            >Peak TR<input
              aria-label="Peak TR"
              name="peakTR"
              type="number"
              min="0"
              step="1"
              required
              value={lab.settings.peakTR}
            /></label
          >
          <button type="submit">Apply growth</button>
        </form>
      {/key}
      <p class="hint">
        TR = start TR + (peak TR − start TR) × the archetype’s growth %, halves rounded up.
      </p>
      <table class="growth-table" data-testid="growth-table">
        <thead
          ><tr
            ><th>World progress</th>{#each lab.growth as point}<th
                class:current={point.world === lab.worldProgress}>{point.world}</th
              >{/each}</tr
          ></thead
        >
        <tbody>
          <tr
            ><th>TR</th>{#each lab.growth as point}<td
                class:current={point.world === lab.worldProgress}>{point.tr}</td
              >{/each}</tr
          >
          <tr
            ><th>Team level</th>{#each lab.growth as point}<td
                class:current={point.world === lab.worldProgress}>{point.teamLevel}</td
              >{/each}</tr
          >
        </tbody>
      </table>
      <dl class="stat-grid">
        <div>
          <dt>Team level</dt>
          <dd data-testid="selected-level">Lv. {lab.selected.teamLevel}</dd>
        </div>
        <div>
          <dt>Team size</dt>
          <dd data-testid="team-size">{lab.selected.size}</dd>
        </div>
        <div>
          <dt>Lv − cap</dt>
          <dd class="gap-value {tone(lab.selected.teamLevel - lab.cap)}" data-testid="selected-gap">
            {signed(lab.selected.teamLevel - lab.cap)}
          </dd>
        </div>
        <div>
          <dt>Roster</dt>
          <dd data-testid="roster-length" class:risk-text={lab.selected.rosterLength < ROSTER_SIZE}>
            {lab.selected.rosterLength} / {ROSTER_SIZE}
          </dd>
        </div>
      </dl>
      <div class="section-label">
        <h3>Team at TR {lab.selected.tr} (world progress {lab.worldProgress})</h3>
        <span>Battle order: roster slot 1 last</span>
      </div>
      <ol class="party" data-testid="battle-order">
        {#each lab.selected.battleOrder as member (member.slot)}<li
            class:signature={member.slot === 1}
          >
            <span class="slot">#{member.slot}</span>
            <div class="member-name">
              <strong>{member.species}</strong><small
                >Offset {member.levelOffset} · {movesText(member.moves)}{member.item
                  ? ` · ${member.item}`
                  : ""}</small
              >
            </div>
            <span class="member-level" class:over-cap={member.level > lab.cap}
              >Lv. {member.level}</span
            >
          </li>{/each}
      </ol>
      {#if lab.selected.warnings.length > 0}<ul class="warnings" data-testid="warnings">
          {#each lab.selected.warnings as warning}<li>{warning}</li>{/each}
        </ul>{/if}

      <div class="section-label">
        <h3>
          Roster{#if lab.selected.rosterLength < ROSTER_SIZE}<span
              class="prototype-chip incomplete-chip"
              data-testid="roster-incomplete">Incomplete</span
            >{/if}
        </h3>
        <span>First {lab.selected.size} play at this TR</span>
      </div>
      {#key lab.settings}
        <form
          class="roster-form"
          data-testid="roster-editor"
          onsubmit={(event) => {
            event.preventDefault()
            lab.applyRoster(event.currentTarget)
          }}
        >
          {#each lab.settings.roster as slot, index (index)}
            <fieldset class:in-team={index < lab.selected.size}>
              <legend>Roster slot {index + 1}{index < lab.selected.size ? " · in team" : ""}</legend
              >
              <div class="line-inputs">
                <label class="wide"
                  >Species<input
                    aria-label={`Roster slot ${index + 1} species`}
                    name={`slot-${index}-species`}
                    type="text"
                    required
                    value={slot.species}
                  /></label
                ><label
                  >Offset<input
                    aria-label={`Roster slot ${index + 1} level offset`}
                    name={`slot-${index}-offset`}
                    type="number"
                    min={LEVEL_OFFSET.min}
                    max={LEVEL_OFFSET.max}
                    step="1"
                    required
                    value={slot.levelOffset}
                  /></label
                ><label class="wide"
                  >Moves<input
                    aria-label={`Roster slot ${index + 1} moves`}
                    name={`slot-${index}-moves`}
                    type="text"
                    placeholder="LEVEL_UP"
                    value={movesText(slot.moves)}
                  /></label
                ><label class="wide"
                  >Item<input
                    aria-label={`Roster slot ${index + 1} item`}
                    name={`slot-${index}-item`}
                    type="text"
                    placeholder="none"
                    value={slot.item ?? ""}
                  /></label
                >
                <div class="slot-tools">
                  <button
                    type="button"
                    aria-label={`Move roster slot ${index + 1} up`}
                    disabled={index === 0}
                    onclick={() => lab.moveSlot(index, -1)}>↑</button
                  ><button
                    type="button"
                    aria-label={`Move roster slot ${index + 1} down`}
                    disabled={index === lab.settings.roster.length - 1}
                    onclick={() => lab.moveSlot(index, 1)}>↓</button
                  ><button
                    type="button"
                    aria-label={`Remove roster slot ${index + 1}`}
                    disabled={lab.settings.roster.length === 1}
                    onclick={() => lab.removeSlot(index)}>Remove</button
                  >
                </div>
              </div>
            </fieldset>
          {/each}
          <div class="actions roster-actions">
            <button type="submit">Apply roster</button>
            {#if lab.settings.roster.length < ROSTER_SIZE}<button
                type="button"
                onclick={lab.addSlot}>Add roster slot</button
              >{/if}
          </div>
        </form>
      {/key}
      <p class="hint">
        Roster slot 1 must stay at offset 0; offsets run {LEVEL_OFFSET.min} to {LEVEL_OFFSET.max}.
        Moves are LEVEL_UP or up to four names separated by commas. v0 needs exactly {ROSTER_SIZE}
        roster slots.
      </p>

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

      <details class="advanced-panel">
        <summary>Edit settings as JSON</summary>
        <p class="hint">
          Edit the growth and every roster slot setting, including ability and nature. Moves are
          <code>"LEVEL_UP"</code> or a list of one to four names.
        </p>
        <label class="visually-hidden" for="team-editor">Trainer settings JSON</label><textarea
          id="team-editor"
          spellcheck="false"
          bind:value={lab.editor}></textarea>
        <div class="actions roster-actions">
          <button type="button" onclick={lab.applyTeam}>Apply settings</button><button
            type="button"
            onclick={lab.resetTrainer}>Restore this trainer’s defaults</button
          >
        </div>
      </details>
    </aside>
  </div>

  <section class="league" aria-label="Gym ladder">
    <div class="panel-title">
      <h2>
        Gym ladder <span
          >{lab.ladder.length} Gym Leader entries at world progress {lab.worldProgress}</span
        >
      </h2>
      <span class="hint" data-testid="ladder-counts"
        >{lab.ladder.filter((row) => row.mark === "below").length} below · {lab.ladder.filter(
          (row) => row.mark === "near",
        ).length} near · {lab.ladder.filter((row) => row.mark === "above").length} above</span
      >
    </div>
    <p class="hint league-note">
      Sorted by TR. Near means within {NEAR_BAND} of the player TR ({lab.playerTR}); below and above
      are further away.
    </p>
    <ol class="ladder" data-testid="gym-ladder">
      {#each lab.ladder as row (row.trainer.id)}
        <li class="ladder-row {row.mark}" data-testid={`ladder-${row.trainer.id}`}>
          <span>{row.trainer.name}{row.trainer.doubleBattle ? " · double battle" : ""}</span><span
            class="numeric">TR {row.tr}</span
          ><span class="gap-chip {row.mark === 'near' ? 'even' : row.mark}"
            >{row.mark} {signed(row.gap)}</span
          >
        </li>
      {/each}
    </ol>
  </section>

  <section class="league" aria-label="League preview">
    <div class="panel-title">
      <h2>
        League lineup <span
          >top {lab.league.length} by TR at world progress {lab.worldProgress}</span
        >
      </h2>
      <span class="hint" data-testid="league-names"
        >One global pool, so {LEAGUES.join(", ")} all use this lineup in v0.</span
      >
    </div>
    <p class="hint league-note">
      Entering a league computes each league-eligible notable trainer’s TR at the current world
      progress and takes the top five (Tate & Liza fight doubles, so they are not in the pool).
      Battle order runs by ascending TR, strongest last. Ties keep catalog order. Each opponent uses
      their own TR, team and levels.
    </p>
    <ol class="league-grid" data-testid="league-lineup">
      {#each lab.league as row, index (row.trainer.id)}
        <li class="match-card" data-testid={`league-match-${index + 1}`}>
          <div class="match-heading">
            <h3><span class="muted">Match {index + 1}</span> {row.trainer.name}</h3>
            <span class="hint">TR {row.tr} · Lv. {row.teamLevel} · {row.team.length} Pokémon</span>
          </div>
          <ol class="league-team">
            {#each row.battleOrder as member (member.slot)}<li>
                <span>{member.species}</span><span class="member-level">Lv. {member.level}</span>
              </li>{/each}
          </ol>
        </li>
      {/each}
    </ol>
  </section>

  <details class="global-settings">
    <summary>Scalers & experiment settings</summary>
    <p>
      Each scaler maps TR to a value through anchors: linear between them, halves rounded up, and
      flat past the last anchor. That anchor’s TR is the ceiling TR: a higher TR is never clamped
      but stops adding level or size. Anchors start at TR 0, rise in TR and never decrease in value.
      Team level and team size read each notable trainer’s own TR. Team level has its own low end
      (Lv 5 at TR 0, Lv 14 at TR 20) and matches the level cap anchors from TR 40 (Lv 100 at TR
      160); team size uses paired anchors to make a step table (0–10 → 1, 11–28 → 2, 29–43 → 3,
      44–70 → 4, 71–95 → 5, 96+ → 6). The wild level curve and regular trainer level curve are world
      scaling: they read the player’s TR and only feed the readout above. The five archetype growth
      scalers, the rival included, read world progress and give the growth % (0% at world progress
      0) from start TR toward peak TR.
    </p>
    {#key lab.anchors}
      <form
        onsubmit={(event) => {
          event.preventDefault()
          lab.applyScalers(event.currentTarget)
        }}
      >
        <div class="scaler-grid">
          {#each scalers as scaler (scaler.id)}
            <div class="scaler" data-testid={`scaler-${scaler.id}`}>
              <h3>{scaler.title} <span class="muted">by {scaler.by}</span></h3>
              <table class="arc-table">
                <thead><tr><th>{scaler.at}</th><th>{scaler.unit}</th><th></th></tr></thead>
                <tbody>
                  {#each lab.anchors[scaler.id] as [at, value], index}
                    <tr>
                      <td
                        >{#if index === 0}<span class="muted">0</span>{:else}<input
                            aria-label={`${scaler.title} anchor ${index + 1} ${scaler.at === "TR" ? "TR" : "world progress"}`}
                            name={`${scalerField(scaler.id)}-tr-${index}`}
                            type="number"
                            min="1"
                            step="1"
                            required
                            value={at}
                          />{/if}</td
                      >
                      <td
                        ><input
                          aria-label={`${scaler.title} anchor ${index + 1} value`}
                          name={`${scalerField(scaler.id)}-value-${index}`}
                          type="number"
                          min={scaler.min}
                          max={scaler.max}
                          step="1"
                          required
                          {value}
                        /></td
                      >
                      <td
                        >{#if index > 0}<button
                            type="button"
                            class="small"
                            aria-label={`Remove ${scaler.title} anchor ${index + 1}`}
                            onclick={() => lab.removeAnchor(scaler.id, index)}>Remove</button
                          >{/if}</td
                      >
                    </tr>
                  {/each}
                </tbody>
              </table>
              <button type="button" class="small" onclick={() => lab.addAnchor(scaler.id)}
                >Add {scaler.title.toLowerCase()} anchor</button
              >
            </div>
          {/each}
        </div>
        <div class="actions">
          <button type="submit">Apply scalers</button><button type="button" onclick={lab.reset}
            >Reset all to catalog defaults</button
          >
        </div>
      </form>
    {/key}
    <p class="hint">
      Catalog growth and rosters are placeholders. Seeded archetypes and lineup rules beyond the top
      five are out of scope for v0. This is separate from the scaler the ROM uses today.
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
    align-items: flex-start;
    border-left: 1px solid var(--line);
    padding-left: 24px;
  }
  .world-facts strong {
    display: block;
    font-size: 23px;
    font-weight: 500;
    margin-top: 5px;
    font-variant-numeric: tabular-nums;
  }
  .world-facts small {
    display: block;
    margin-top: 2px;
    color: var(--color-cartographer-muted);
    font-size: 11px;
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
    min-width: 128px;
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
  .global-settings {
    background: var(--surface);
    border: 1px solid var(--line);
    border-radius: 8px;
    padding: 16px 20px;
    margin-top: 24px;
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
  .match-card {
    min-width: 0;
    border: 1px solid #2d333a;
    border-radius: 6px;
    background: var(--color-cartographer-field);
    overflow-x: auto;
  }
  .match-heading {
    display: flex;
    justify-content: space-between;
    align-items: baseline;
    flex-wrap: wrap;
    gap: 6px;
    padding: 12px 12px 8px;
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
  .arc-table td {
    border-top: 0;
    padding: 4px 8px;
  }
  .arc-table td:first-child {
    padding: 4px 8px;
  }
  .arc-table input {
    width: 60px;
    padding: 6px 7px;
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
  .party li.signature {
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
  button:disabled {
    opacity: 0.4;
    cursor: default;
  }
  button.small {
    padding: 4px 9px;
  }
  .current-tr {
    color: var(--accent);
    font-weight: 600;
  }
  .growth-form {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 8px 14px;
    font-size: 11px;
    margin: 0 0 4px;
  }
  .growth-form label {
    display: flex;
    align-items: center;
    gap: 6px;
    color: var(--color-cartographer-muted);
  }
  .growth-form input {
    width: 64px;
    padding: 5px 6px;
  }
  .growth-form select {
    padding: 5px 24px 5px 8px;
  }
  .growth-form button {
    padding: 5px 10px;
  }
  .growth-table {
    width: 100%;
    border-collapse: collapse;
    font-size: 12px;
    margin: 4px 0 20px;
    font-variant-numeric: tabular-nums;
  }
  .growth-table th {
    position: static;
    background: transparent;
    padding: 5px 6px;
    white-space: nowrap;
  }
  .growth-table td {
    border-top: 1px solid #2b3037;
    padding: 5px 6px;
  }
  .growth-table td:first-child {
    padding: 5px 6px;
  }
  .growth-table .current {
    color: var(--accent);
  }
  .ladder {
    list-style: none;
    margin: 0;
    padding: 0 18px;
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
    gap: 6px 14px;
  }
  .ladder-row {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 12px;
    border: 1px solid #2d333a;
    border-radius: 5px;
    background: var(--color-cartographer-field);
    padding: 6px 8px;
  }
  .ladder-row > span:first-child {
    flex: 1;
    min-width: 0;
  }
  .ladder-row.near {
    border-color: #4d4331;
  }
  td.incomplete,
  .risk-text {
    color: #ffb8bc;
  }
  .provenance {
    margin: 8px 0 18px;
  }
  .incomplete-chip {
    color: #ffb8bc;
    border-color: #a15e64;
  }
  .double-chip {
    color: #9fd0ff;
    border-color: #3e6485;
  }
  .trainer-select .double-chip {
    display: inline;
    margin-top: 0;
    color: #9fd0ff;
  }
  .roster-form {
    margin: 10px 0 0;
  }
  .roster-form fieldset.in-team {
    border-color: #4d4331;
  }
  .slot-tools {
    display: flex;
    gap: 6px;
    margin-left: auto;
  }
  .slot-tools button {
    padding: 4px 9px;
  }
  .roster-actions {
    justify-content: flex-start;
    flex-wrap: wrap;
    margin: 8px 0;
  }
  .advanced-panel textarea {
    min-height: 250px;
  }
  .league {
    margin-top: 24px;
    border: 1px solid var(--line);
    border-radius: 8px;
    background: var(--surface);
    padding-bottom: 18px;
  }
  .league-note {
    margin: 0 18px 14px;
  }
  .league-grid {
    list-style: none;
    margin: 0;
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(210px, 1fr));
    gap: 14px;
    padding: 0 18px;
  }
  .league-team {
    list-style: none;
    margin: 0;
    padding: 0 12px 12px;
    display: grid;
    gap: 4px;
    font-size: 12px;
  }
  .league-team li {
    display: flex;
    justify-content: space-between;
    gap: 8px;
    border-top: 1px solid #2b3037;
    padding-top: 4px;
  }
  .scaler-grid {
    display: flex;
    flex-wrap: wrap;
    gap: 12px 48px;
    margin: 10px 0 12px;
  }
  .scaler h3 {
    margin-top: 6px;
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
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
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

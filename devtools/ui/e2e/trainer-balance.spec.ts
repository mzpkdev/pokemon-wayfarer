import { expect, test } from "webanvil/e2e"

test("explores badge progression, team size and bias, and restores an exported experiment", async ({
  page,
}) => {
  const errors: string[] = []
  page.on("pageerror", (error) => errors.push(error.message))
  await page.goto("/#trainer-balance")
  await expect(page.getByRole("heading", { name: "Trainer balance", exact: true })).toBeVisible()
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 15")
  const party = page.getByTestId("generated-party")
  // Strength 13: size 2 = the top ace (Golem line, still Geodude) + one filler at offset -2.
  await expect(party.locator("li")).toHaveCount(2)
  await expect(party.locator("li").nth(0)).toContainText("Omanyte")
  await expect(party.locator("li").nth(0)).toContainText("Lv. 11")
  await expect(party.locator("li").nth(1)).toContainText("Geodude")
  await expect(party.locator("li").nth(1)).toContainText("Ace 1")
  await expect(party.locator("li").nth(1)).toContainText("Lv. 13")
  await expect(page.getByTestId("roster-aces")).toContainText(
    "Geodude → Graveler (25) → Golem (35)",
  )
  await expect(page.getByTestId("selected-standing")).toHaveText("-2")
  await expect(page.getByTestId("selected-gap")).toHaveText("-2")
  await expect(page.getByTestId("gap-brock")).toHaveText("-2")
  await expect(page.getByTestId("selected-role")).toHaveText("Contender")

  // Every arc is 0 at B=0: switching Brock's arc leaves his first encounter unchanged.
  await page.getByLabel("Brock arc", { exact: true }).selectOption("late")
  await expect(page.getByTestId("selected-arc")).toHaveText("late")
  await expect(party).toContainText("Lv. 13")

  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("player-tr")).toHaveText("40")
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 42")
  await expect(page.getByTestId("selected-standing")).toHaveText("-4")
  await expect(page.getByTestId("selected-strength")).toHaveText("Lv. 38")
  await expect(party.locator("li")).toHaveCount(4)
  await expect(page.getByTestId("team-size")).toHaveText("4")
  await expect(page.getByTestId("ace-allowance")).toHaveText("2")
  await expect(page.getByTestId("aces-used")).toHaveText("1")
  await expect(page.getByTestId("filler-count")).toHaveText("3")
  await expect(page.getByTestId("allowance-note")).toContainText(
    "Brock has 1 ace, so 1 unused ace slot goes to fillers",
  )
  await expect(party.locator("li").last()).toContainText("Golem")
  await expect(party.locator("li").last()).toContainText("Lv. 38")
  await page.getByRole("button", { name: "Set 24 badges", exact: true }).click()
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 62")
  // Strength 63: size 6, one ace plus five fillers.
  await expect(party.locator("li")).toHaveCount(6)
  await expect(page.getByTestId("filler-count")).toHaveText("5")
  await expect(page.getByTestId("allowance-note")).toContainText("2 unused ace slots go to fillers")
  await expect(party.locator("li").nth(4)).toContainText("Omastar")
  await expect(page.getByTestId("selected-gap")).toHaveText("+1")
  await expect(page.getByTestId("selected-role")).toHaveText("Elite")

  await page.getByLabel("Standing bias", { exact: true }).fill("0")
  await page.getByRole("button", { name: "Apply standing", exact: true }).click()
  await expect(page.getByTestId("selected-standing")).toHaveText("+3")
  await expect(page.getByTestId("selected-role")).toHaveText("Headliner")
  await page.getByLabel("First league clears", { exact: true }).selectOption("2")
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 89")
  await expect(page.getByTestId("selected-strength")).toHaveText("Lv. 92")
  await expect(page.getByTestId("gap-brock")).toHaveText("+3")
  await page.getByLabel("Save seed", { exact: true }).fill("7")
  await page.getByLabel("Save seed", { exact: true }).press("Enter")
  await page.getByText("Edit roster", { exact: true }).click()
  await page.getByLabel("kabutops base score", { exact: true }).fill("90")
  await page.getByRole("button", { name: "Apply roster", exact: true }).click()
  await expect(page.getByTestId("filler-kabutops")).toContainText("99")

  const downloadPromise = page.waitForEvent("download")
  await page.getByRole("button", { name: "Export experiment", exact: true }).click()
  const download = await downloadPromise
  const exportedPath = await download.path()
  if (!exportedPath) throw new Error("Export did not produce a file")
  await page.getByText("Growth arcs, role windows & experiment settings", { exact: true }).click()
  await page.getByRole("button", { name: "Reset all trainer defaults", exact: true }).click()
  await expect(page.getByTestId("selected-standing")).toHaveText("-2")
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles(exportedPath)
  await expect(page.getByTestId("selected-standing")).toHaveText("+3")
  await page.reload()
  await expect(page.getByTestId("selected-standing")).toHaveText("+3")
  await expect(page.getByTestId("selected-arc")).toHaveText("late")
  await expect(page.getByTestId("badge-count")).toHaveText("24")
  await expect(page.getByLabel("Save seed", { exact: true })).toHaveValue("7")
  await expect(page.getByTestId("filler-kabutops")).toContainText("99")
  expect(errors).toEqual([])
})

test("edits arc tuples and role windows globally", async ({ page }) => {
  await page.goto("/#trainer-balance")
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await page.getByText("Growth arcs, role windows & experiment settings", { exact: true }).click()
  await page.getByLabel("steady arc at p 8", { exact: true }).fill("2")
  await page.getByRole("button", { name: "Apply arcs, windows & headroom", exact: true }).click()
  await expect(page.getByTestId("selected-standing")).toHaveText("0")
  await expect(page.getByTestId("selected-role")).toHaveText("Elite")
  await page.getByLabel("Contender maximum standing", { exact: true }).fill("0")
  await page.getByLabel("Headliner minimum standing", { exact: true }).fill("3")
  await page.getByRole("button", { name: "Apply arcs, windows & headroom", exact: true }).click()
  await expect(page.getByTestId("selected-role")).toHaveText("Contender")
  await expect(page.getByTestId("indigo-elite")).toContainText("+1 … +2")
})

test("rejects invalid changes and version 1, 2 and 3 files without replacing the experiment", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  await page.getByLabel("Save seed", { exact: true }).fill("-3")
  await page.getByLabel("Save seed", { exact: true }).press("Enter")
  await expect(page.getByRole("alert")).toContainText("whole-number seed")
  await page.getByText("Growth arcs, role windows & experiment settings", { exact: true }).click()
  await page.getByLabel("Headliner minimum standing", { exact: true }).fill("-1")
  await page.getByRole("button", { name: "Apply arcs, windows & headroom", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText("at least one elite standing")
  await expect(page.getByTestId("selected-standing")).toHaveText("-2")
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles({
    name: "v1.json",
    mimeType: "application/json",
    buffer: Buffer.from(
      JSON.stringify({
        tool: "wayfarer-trainer-balance",
        version: 1,
        point: { badges: 8, leagueClears: 0 },
        selectedTrainer: "blue",
        experiment: { version: 1, levelAnchors: [], trainers: {} },
      }),
    ),
  })
  await expect(page.getByRole("alert")).toContainText("Version 1 experiments")
  await expect(page.getByTestId("badge-count")).toHaveText("0")
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles({
    name: "v2.json",
    mimeType: "application/json",
    buffer: Buffer.from(
      JSON.stringify({
        tool: "wayfarer-trainer-balance",
        version: 2,
        point: { badges: 8, leagueClears: 0 },
        selectedTrainer: "blue",
        experiment: { version: 2, arcs: {}, roleWindows: {}, trainers: {} },
      }),
    ),
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 2 experiments predate post-game arcs",
  )
  await expect(page.getByTestId("badge-count")).toHaveText("0")
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles({
    name: "v3.json",
    mimeType: "application/json",
    buffer: Buffer.from(
      JSON.stringify({
        tool: "wayfarer-trainer-balance",
        version: 3,
        point: { badges: 8, leagueClears: 0, completedEditions: 0 },
        selectedTrainer: "blue",
        experiment: { version: 3, arcs: {}, roleWindows: {}, headroom: 4, trainers: {} },
      }),
    ),
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 3 experiments use badge-keyed team stages",
  )
  await expect(page.getByTestId("badge-count")).toHaveText("0")
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles({
    name: "invalid.json",
    mimeType: "application/json",
    buffer: Buffer.from('{"version":2}'),
  })
  await expect(page.getByRole("alert")).toContainText("not a version 4")
  await expect(page.getByTestId("selected-standing")).toHaveText("-2")
  await page.getByLabel("Search trainers", { exact: true }).fill("does-not-exist")
  await expect(page.getByText("No trainers match these filters.")).toBeVisible()
  await page.getByRole("button", { name: "Clear filters", exact: true }).click()
  await page.getByLabel("Filter region", { exact: true }).selectOption("Hoenn")
  await page.getByLabel("Filter role", { exact: true }).selectOption("Gym Leaders")
  await expect(page.locator(".pool-table tbody tr")).toHaveCount(7)
  await page.getByRole("button", { name: "Juan Hoenn · Gym Leader", exact: true }).click()
  await expect(page.getByRole("complementary", { name: "Selected trainer" })).toContainText("Juan")
  await expect(page.getByTestId("selected-standing")).toHaveText("-1")
})

test("keeps controls and parties usable at a narrow viewport", async ({ page }) => {
  await page.setViewportSize({ width: 390, height: 844 })
  await page.goto("/#trainer-balance")
  await page.getByLabel("Badges earned", { exact: true }).focus()
  await page.keyboard.press("ArrowRight")
  await expect(page.getByTestId("badge-count")).toHaveText("1")
  await page.getByRole("button", { name: "Set 16 badges", exact: true }).click()
  await expect(page.getByTestId("badge-count")).toHaveText("16")
  await expect(page.getByTestId("generated-party")).toBeVisible()
  await expect(page.getByTestId("feasibility-sevii-masters")).toBeVisible()
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
})

test("keeps Blue a step ahead with only fast arcs and restores only his defaults", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  await page.getByLabel("Standing bias", { exact: true }).fill("-3")
  await page.getByRole("button", { name: "Apply standing", exact: true }).click()
  await page.getByLabel("Search trainers", { exact: true }).fill("Blue")
  await page.getByRole("button", { name: "Blue Kanto · Champion", exact: true }).click()
  await expect(page.getByTestId("selected-standing")).toHaveText("+1")
  await expect(page.getByTestId("selected-gap")).toHaveText("+1")
  await expect(page.getByTestId("generated-party").locator("li")).toHaveCount(2)
  await expect(page.getByLabel("Blue arc", { exact: true }).locator("option")).toHaveText([
    "early",
    "rival",
  ])
  await expect(page.getByTestId("blue-check")).toHaveText("Yes")
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("selected-arc")).toHaveText("early")
  await expect(page.getByTestId("selected-gap")).toHaveText("+4")
  await expect(page.getByTestId("selected-role")).toHaveText("Headliner")
  await page.getByLabel("Blue arc", { exact: true }).selectOption("rival")
  await expect(page.getByTestId("selected-gap")).toHaveText("+3")
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 42")
  await page.reload()
  await expect(page.getByTestId("selected-arc")).toHaveText("rival")
  await page.getByRole("button", { name: "Restore this trainer’s defaults", exact: true }).click()
  await expect(page.getByTestId("selected-arc")).toHaveText("early")
  await expect(page.getByTestId("badge-count")).toHaveText("8")
  await page.getByLabel("Search trainers", { exact: true }).fill("Brock")
  await page.getByRole("button", { name: "Brock Kanto · Gym Leader", exact: true }).click()
  await expect(page.getByLabel("Standing bias", { exact: true })).toHaveValue("-3")
})

test("reports league feasibility per venue, roster gaps and the Gym cap summary", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("gym-gap-mean")).toHaveText("-1.0")
  await expect(page.getByTestId("gym-gap-range")).toHaveText("-2 … +1")
  const indigo = page.getByTestId("feasibility-indigo")
  // Every roster is a candidate; there is no six-member-profile filter.
  await expect(indigo).toContainText("Home pool · 24 candidates")
  await expect(page.getByLabel("Count five-member profiles")).toHaveCount(0)
  const cells = (row: string) => page.getByTestId(row).locator("td")
  // Need, guaranteed, possible, current.
  await expect(cells("indigo-contender").nth(2)).toHaveText("2")
  await expect(cells("indigo-contender").nth(3)).toHaveText("3")
  await expect(cells("indigo-contender").nth(4)).toHaveText("15")
  await expect(page.getByTestId("indigo-contender")).not.toContainText("Fallback risk")
  await expect(cells("indigo-elite").nth(3)).toHaveText("4")
  await expect(page.getByTestId("hoenn-contender")).toContainText("Fallback risk")
  await expect(page.getByTestId("feasibility-sevii-masters")).toContainText(
    "Open invitational · 37 candidates",
  )
  await expect(page.getByTestId("roster-gap-count")).toHaveText(
    "11 rosters can’t reach 6 at max size",
  )
  await expect(page.getByTestId("gaps-hoenn")).toHaveText(
    "Rosters short of 6: Sidney 5/6, Phoebe 5/6, Glacia 5/6, Drake 5/6",
  )
})

test("authors rosters: modifiers, flags, evolve levels, ace order and seeded jitter", async ({
  page,
}) => {
  const errors: string[] = []
  page.on("pageerror", (error) => errors.push(error.message))
  await page.goto("/#trainer-balance")
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  const party = page.getByTestId("generated-party").locator("li")
  const row = (id: string) => page.getByTestId(`filler-${id}`)
  // Seed 1: Omanyte 77, Kleavor 72 and Aerodactyl 65 fill the three filler slots.
  await expect(party.nth(0)).toContainText("Aerodactyl")
  await expect(party.nth(2)).toContainText("Omanyte")
  await expect(row("relicanth").locator("td").last()).toHaveText("No")

  // Jitter is deterministic per seed: seed 7 changes the scores, seed 1 restores them.
  const seed = page.getByLabel("Save seed", { exact: true })
  await seed.fill("7")
  await seed.press("Enter")
  await expect(row("omastar").locator("td").nth(4)).toHaveText("61")
  await expect(party.nth(1)).toContainText("Onix")
  await seed.fill("1")
  await seed.press("Enter")
  await expect(row("omastar").locator("td").nth(4)).toHaveText("77")

  // A modifier only applies while its flag is set, and displaces the lowest filler.
  await page.getByText("Growth arcs, role windows & experiment settings", { exact: true }).click()
  await page.getByLabel("Modifier flag", { exact: true }).fill("FLAG_TOLD_BROCK")
  await page.getByLabel("Modifier filler", { exact: true }).selectOption("brock/relicanth")
  await page.getByLabel("Modifier delta", { exact: true }).fill("40")
  await page.getByRole("button", { name: "Add modifier", exact: true }).click()
  await expect(page.getByTestId("modifier-list")).toContainText("FLAG_TOLD_BROCK → brock/relicanth")
  await expect(row("relicanth").locator("td").last()).toHaveText("No")
  await page.getByLabel("Flag FLAG_TOLD_BROCK", { exact: true }).check()
  await expect(row("relicanth").locator("td").nth(3)).toHaveText("+40")
  await expect(row("relicanth").locator("td").last()).toHaveText("Yes")
  await expect(row("aerodactyl").locator("td").last()).toHaveText("No")
  await expect(party.nth(2)).toContainText("Relicanth")

  // requiresFlag keeps a filler out until the flag is set; evolve levels are editable.
  await page.getByText("Edit roster", { exact: true }).click()
  await page.getByLabel("omastar requires flag", { exact: true }).fill("FLAG_TICKET")
  await page.getByLabel("golem evolves to Golem at", { exact: true }).fill("45")
  await page.getByLabel("aerodactyl level offset", { exact: true }).fill("-6")
  await page.getByRole("button", { name: "Apply roster", exact: true }).click()
  await expect(row("omastar")).toContainText("Needs FLAG_TICKET")
  await expect(row("omastar").locator("td").last()).toHaveText("Locked")
  await expect(party.last()).toContainText("Graveler")
  await expect(party.nth(0)).toContainText("Aerodactyl")
  await expect(party.nth(0)).toContainText("Lv. 34")
  await page.getByLabel("Flag FLAG_TICKET", { exact: true }).check()
  await expect(row("omastar").locator("td").last()).toHaveText("Yes")

  // A second ace (added as JSON) fills the second ace slot at size 4; order is editable.
  const editor = page.locator("#team-editor")
  await page.getByText("Edit settings as JSON", { exact: true }).click()
  const settings = JSON.parse(await editor.inputValue())
  settings.roster.aces.push({
    id: "steelix",
    line: [
      { species: "Onix", level: 1, moves: [], item: null, ability: null, nature: null },
      {
        species: "Steelix",
        level: 35,
        moves: ["Iron Tail"],
        item: null,
        ability: null,
        nature: null,
      },
    ],
  })
  settings.roster.fillers = settings.roster.fillers.filter((f: { id: string }) => f.id !== "onix")
  await editor.fill(JSON.stringify(settings))
  await page.getByRole("button", { name: "Apply settings", exact: true }).click()
  await expect(page.getByTestId("aces-used")).toHaveText("2")
  await expect(party.last()).toContainText("Graveler")
  await expect(party.nth(2)).toContainText("Steelix")
  await page.getByRole("button", { name: "Move steelix up", exact: true }).click()
  await expect(party.last()).toContainText("Steelix")
  await expect(page.getByTestId("roster-aces").locator("tbody tr").first()).toContainText("Steelix")

  // Size table and jitter are global settings.
  await page.getByLabel("Size row 3 minimum strength level", { exact: true }).fill("45")
  await page.getByLabel("Size row 4 minimum strength level", { exact: true }).fill("50")
  await page.getByLabel("Filler jitter", { exact: true }).fill("0")
  await page.getByRole("button", { name: "Apply sizes & jitter", exact: true }).click()
  await expect(page.getByTestId("team-size")).toHaveText("3")
  await expect(page.getByTestId("aces-used")).toHaveText("1")
  await expect(row("kabutops").locator("td").nth(2)).toHaveText("0")
  expect(errors).toEqual([])
})

test("models post-game editions, the level headroom and Blue at the ceiling", async ({ page }) => {
  await page.goto("/#trainer-balance")
  const editions = page.getByLabel("Completed editions", { exact: true })
  await expect(editions).toBeDisabled()
  await page.getByRole("button", { name: "Set 24 badges", exact: true }).click()
  await page.getByLabel("First league clears", { exact: true }).selectOption("2")
  await expect(editions).toBeDisabled()
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 89")
  await expect(page.getByTestId("level-base")).toHaveText("Lv. 89")
  await expect(page.getByTestId("blue-check")).toHaveText("Yes")

  // (24, 3): cap 100, base min(100, 100 - 4) = 96. Brock (steady, -2) -> 94.
  await page.getByLabel("First league clears", { exact: true }).selectOption("3")
  await expect(editions).toBeEnabled()
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 100")
  await expect(page.getByTestId("level-base")).toHaveText("Lv. 96")
  await expect(page.getByTestId("selected-strength")).toHaveText("Lv. 94")
  await expect(page.getByTestId("selected-gap")).toHaveText("-6")
  await expect(page.getByTestId("blue-check")).toHaveText("At ceiling")

  await expect(editions.locator("option")).toHaveText(["0", "1", "2", "3+"])
  await editions.selectOption("3")
  await expect(page.getByTestId("progress-index")).toHaveText("48")
  await expect(page.getByRole("heading", { name: "League feasibility at p = 48" })).toBeVisible()
  await expect(page.getByTestId("blue-check")).toHaveText("At ceiling")
  await page.getByLabel("Brock arc", { exact: true }).selectOption("late")
  // late +2 at p = 48: standing 0 -> 96.
  await expect(page.getByTestId("selected-standing")).toHaveText("0")
  await expect(page.getByTestId("selected-strength")).toHaveText("Lv. 96")
  await page.reload()
  await expect(page.getByTestId("progress-index")).toHaveText("48")

  await page.getByText("Growth arcs, role windows & experiment settings", { exact: true }).click()
  await page.getByLabel("Level headroom", { exact: true }).fill("0")
  await page.getByRole("button", { name: "Apply arcs, windows & headroom", exact: true }).click()
  await expect(page.getByTestId("level-base")).toHaveText("Lv. 100")
  await expect(page.getByTestId("selected-strength")).toHaveText("Lv. 100")
  await expect(page.getByTestId("blue-check")).toHaveText("No")

  // Leaving (24, 3) resets completed editions: p follows badges again.
  await page.getByRole("button", { name: "Set 20 badges", exact: true }).click()
  await expect(editions).toBeDisabled()
  await expect(editions).toHaveValue("0")
  await expect(page.getByTestId("progress-index")).toHaveText("20")
})

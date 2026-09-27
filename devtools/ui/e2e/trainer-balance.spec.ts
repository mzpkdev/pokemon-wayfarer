import { expect, test } from "webanvil/e2e"

test("builds teams from each trainer's own TR and round-trips an exported experiment", async ({
  page,
}) => {
  const errors: string[] = []
  page.on("pageerror", (error) => errors.push(error.message))
  await page.goto("/#trainer-balance")
  await expect(page.getByRole("heading", { name: "Trainer balance", exact: true })).toBeVisible()
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 15")
  await expect(page.getByLabel("First league clears")).toHaveCount(0)
  const order = page.getByTestId("battle-order").locator("li")
  // Brock TR 2: team level 16 (15.65 rounds to 16), size 2; entry 1 comes last.
  await expect(page.getByTestId("selected-tr")).toHaveText("2")
  await expect(page.getByTestId("selected-level")).toHaveText("Lv. 16")
  await expect(page.getByTestId("team-size")).toHaveText("2")
  await expect(order).toHaveCount(2)
  await expect(order.nth(0)).toContainText("Aerodactyl")
  await expect(order.nth(0)).toContainText("Lv. 14")
  await expect(order.nth(1)).toContainText("Golem")
  await expect(order.nth(1)).toContainText("Lv. 16")

  // The player's progress is a readout only: Brock's team does not move.
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("player-tr")).toHaveText("80")
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 50")
  await expect(page.getByTestId("wild-level")).toHaveText("Lv. 40")
  await expect(page.getByTestId("wild-gap")).toHaveText("-10 vs cap")
  await expect(page.getByTestId("route-trainer-level")).toHaveText("Lv. 44")
  await expect(page.getByTestId("route-trainer-gap")).toHaveText("-6 vs cap")
  await expect(page.getByTestId("selected-level")).toHaveText("Lv. 16")
  await expect(page.getByTestId("gap-brock")).toHaveText("-34")

  // TR is uncapped; the scalers stay flat past their last anchor (TR 160).
  const tr = page.getByLabel("Brock TR", { exact: true })
  await tr.fill("200")
  await tr.press("Enter")
  await expect(page.getByTestId("selected-tr")).toHaveText("200")
  await expect(page.getByTestId("level-brock")).toHaveText("Lv. 100")
  await expect(page.getByTestId("size-brock")).toHaveText("6")
  await expect(order).toHaveCount(6)
  await expect(order.last()).toContainText("Golem")
  await expect(order.last()).toContainText("Lv. 100")
  await expect(order.first()).toContainText("Kleavor")
  await expect(order.first()).toContainText("Lv. 98")
  await tr.fill("2")
  await tr.press("Enter")

  // Reorder and edit entries; the team is always the first N.
  await page.getByRole("button", { name: "Move entry 3 up", exact: true }).click()
  await expect(order.nth(0)).toContainText("Kabutops")
  await page.getByLabel("Entry 2 species", { exact: true }).fill("Onix")
  await page.getByLabel("Entry 2 level offset", { exact: true }).fill("-6")
  await page.getByLabel("Entry 2 moves", { exact: true }).fill("Rock Throw, Bind")
  await page.getByLabel("Entry 2 item", { exact: true }).fill("Hard Stone")
  await page.getByRole("button", { name: "Apply roster", exact: true }).click()
  await expect(order.nth(0)).toContainText("Onix")
  await expect(order.nth(0)).toContainText("Lv. 10")
  await expect(order.nth(0)).toContainText("Rock Throw, Bind · Hard Stone")

  const downloadPromise = page.waitForEvent("download")
  await page.getByRole("button", { name: "Export experiment", exact: true }).click()
  const exportedPath = await (await downloadPromise).path()
  if (!exportedPath) throw new Error("Export did not produce a file")
  await page.getByText("Scalers & experiment settings", { exact: true }).click()
  await page.getByRole("button", { name: "Reset all to catalog defaults", exact: true }).click()
  await expect(order.nth(0)).toContainText("Aerodactyl")
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles(exportedPath)
  await expect(order.nth(0)).toContainText("Onix")
  await page.reload()
  await expect(order.nth(0)).toContainText("Onix")
  await expect(page.getByTestId("badge-count")).toHaveText("8")
  expect(errors).toEqual([])
})

test("fields the top five by TR from one pool, strongest last", async ({ page }) => {
  await page.goto("/#trainer-balance")
  const matches = page.getByTestId("league-field").locator(":scope > li")
  await expect(matches).toHaveCount(5)
  const names = ["Bruno", "Agatha", "Wallace", "Steven", "Lance"]
  for (const [index, name] of names.entries())
    await expect(matches.nth(index).locator("h3")).toContainText(name)
  await expect(page.getByTestId("league-venues")).toContainText(
    "Indigo, Sevii Masters, Hoenn all field this five",
  )
  // Beatable at 8 badges (cap Lv 50): the field runs TR 90–95, team level 56–59.
  await expect(page.getByTestId("league-range")).toHaveText("TR 90 … 95")
  await expect(page.getByTestId("league-match-1")).toContainText("TR 90 · Lv. 56 · 5 Pokémon")
  await expect(page.getByTestId("league-match-5")).toContainText("TR 95 · Lv. 59 · 5 Pokémon")
  await expect(page.getByTestId("league-match-5").locator("li").last()).toContainText("Dragonite")
  // Raising Brock above everyone moves him into the last match and drops Bruno.
  const tr = page.getByLabel("Brock TR", { exact: true })
  await tr.fill("100")
  await tr.press("Enter")
  await expect(page.getByTestId("league-match-5")).toContainText("Brock")
  await expect(page.getByTestId("league-match-1")).toContainText("Agatha")
  await expect(page.getByTestId("league-field")).not.toContainText("Bruno")
})

test("flags incomplete rosters and rejects invalid edits and version 4 and 5 files", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  await expect(page.getByTestId("roster-gap-count")).toHaveText("11")
  await expect(page.getByTestId("roster-incomplete")).toHaveCount(0)

  await page.getByLabel("Entry 1 level offset", { exact: true }).fill("-1")
  await page.getByRole("button", { name: "Apply roster", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText("entry 1 must have level offset 0")
  await page.getByRole("button", { name: "Move entry 1 down", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText("entry 1 must have level offset 0")
  const tr = page.getByLabel("Brock TR", { exact: true })
  await tr.fill("-3")
  await tr.press("Enter")
  await expect(page.getByRole("alert")).toContainText("TR must be a whole number")
  await expect(page.getByTestId("selected-tr")).toHaveText("2")

  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles({
    name: "v4.json",
    mimeType: "application/json",
    buffer: Buffer.from(
      JSON.stringify({
        tool: "wayfarer-trainer-balance",
        version: 4,
        point: { badges: 8, leagueClears: 0, completedEditions: 0 },
        seed: 1,
        flags: [],
        selectedTrainer: "blue",
        experiment: { version: 4, arcs: {}, trainers: {} },
      }),
    ),
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 4 experiments use a retired trainer model",
  )
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles({
    name: "v5.json",
    mimeType: "application/json",
    buffer: Buffer.from(
      JSON.stringify({
        tool: "wayfarer-trainer-balance",
        version: 5,
        point: { badges: 8, leagueClears: 1 },
        selectedTrainer: "blue",
        experiment: { version: 5, teamLevel: [[0, 15]], teamSize: [[0, 2]], trainers: {} },
      }),
    ),
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 5 experiments use the retired 0–80 player TR scale",
  )
  await expect(page.getByTestId("badge-count")).toHaveText("0")
  await expect(page.getByTestId("selected-tr")).toHaveText("2")

  await page.getByRole("button", { name: "Lorelei Kanto · Elite Four", exact: true }).click()
  await expect(page.getByTestId("roster-incomplete")).toBeVisible()
  await expect(page.getByTestId("roster-length")).toHaveText("5 / 6")
  await expect(page.getByTestId("warnings")).toContainText("lists 5 of 6 entries")
  await page.getByRole("button", { name: "Add entry", exact: true }).click()
  await page.getByLabel("Entry 6 species", { exact: true }).fill("Articuno")
  await page.getByRole("button", { name: "Apply roster", exact: true }).click()
  await expect(page.getByTestId("roster-length")).toHaveText("6 / 6")
  await expect(page.getByTestId("roster-incomplete")).toHaveCount(0)
  await expect(page.getByTestId("roster-gap-count")).toHaveText("10")
  await expect(page.getByRole("button", { name: "Add entry", exact: true })).toHaveCount(0)
})

test("edits the trainer and world scalers globally", async ({ page }) => {
  await page.goto("/#trainer-balance")
  await page.getByText("Scalers & experiment settings", { exact: true }).click()
  await expect(page.getByTestId("size-lance")).toHaveText("5")
  // Without the (95, 5) anchor, size ramps from (71, 5) to (96, 6): TR 95 rounds to 6.
  await page.getByRole("button", { name: "Remove Team size anchor 8", exact: true }).click()
  await expect(page.getByTestId("size-lance")).toHaveText("6")
  await page.getByLabel("Team level anchor 1 value", { exact: true }).fill("30")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText("values must not decrease")
  await page.getByLabel("Team level anchor 1 value", { exact: true }).fill("16")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByTestId("level-falkner")).toHaveText("Lv. 16")

  // The wild and route-trainer curves read the player TR and move only the world readout.
  await page.getByRole("button", { name: "Set 4 badges", exact: true }).click()
  await expect(page.getByTestId("player-tr")).toHaveText("40")
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 28")
  await expect(page.getByTestId("wild-gap")).toHaveText("-4 vs cap")
  await expect(page.getByTestId("route-trainer-gap")).toHaveText("-1 vs cap")
  await page.getByLabel("Wild level anchor 2 value", { exact: true }).fill("28")
  await page.getByLabel("Route trainer level anchor 2 value", { exact: true }).fill("30")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByTestId("wild-level")).toHaveText("Lv. 28")
  await expect(page.getByTestId("route-trainer-gap")).toHaveText("+2 vs cap")
  await expect(page.getByTestId("level-falkner")).toHaveText("Lv. 16")

  await page.getByRole("button", { name: "Set 24 badges", exact: true }).click()
  await expect(page.getByTestId("player-tr")).toHaveText("160")
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 100")
  await page.getByRole("button", { name: "Reset all to catalog defaults", exact: true }).click()
  await expect(page.getByTestId("size-lance")).toHaveText("5")
  await expect(page.getByTestId("wild-level")).toHaveText("Lv. 78")
  await expect(page.getByTestId("wild-gap")).toHaveText("-22 vs cap")
  await expect(page.getByTestId("route-trainer-level")).toHaveText("Lv. 82")
  await expect(page.getByTestId("route-trainer-gap")).toHaveText("-18 vs cap")
})

test("keeps controls and teams usable at a narrow viewport", async ({ page }) => {
  await page.setViewportSize({ width: 390, height: 844 })
  await page.goto("/#trainer-balance")
  await page.getByLabel("Badges earned", { exact: true }).focus()
  await page.keyboard.press("ArrowRight")
  await expect(page.getByTestId("badge-count")).toHaveText("1")
  await expect(page.getByTestId("battle-order")).toBeVisible()
  await expect(page.getByTestId("league-field")).toBeVisible()
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
})

import { expect, test, type Page } from "webanvil/e2e"

/** Sets the selected trainer's growth through the growth form. */
const setGrowth = async (
  page: Page,
  growth: { start?: string; archetype?: string; peak?: string },
) => {
  if (growth.start !== undefined)
    await page.getByLabel("Start TR", { exact: true }).fill(growth.start)
  if (growth.archetype !== undefined)
    await page.getByLabel("Archetype", { exact: true }).selectOption(growth.archetype)
  if (growth.peak !== undefined) await page.getByLabel("Peak TR", { exact: true }).fill(growth.peak)
  await page.getByRole("button", { name: "Apply growth", exact: true }).click()
}

test("grows each trainer with world progress and round-trips an exported experiment", async ({
  page,
}) => {
  const errors: string[] = []
  page.on("pageerror", (error) => errors.push(error.message))
  await page.goto("/#trainer-balance")
  await expect(page.getByRole("heading", { name: "Trainer balance", exact: true })).toBeVisible()
  await expect(page.getByTestId("level-cap")).toHaveText("Lv. 15")
  await expect(page.getByTestId("world-progress")).toHaveText("0")
  const order = page.getByTestId("battle-order").locator("li")
  // Brock (start TR 20, steady, peak TR 95) at world progress 0: TR 20, team level 14, size 2.
  await expect(page.getByTestId("selected-tr")).toHaveText("20")
  await expect(page.getByTestId("growth-brock")).toHaveText("20 → 95")
  await expect(page.getByTestId("archetype-brock")).toHaveText("steady")
  await expect(page.getByTestId("selected-level")).toHaveText("Lv. 14")
  await expect(page.getByTestId("team-size")).toHaveText("2")
  await expect(order).toHaveCount(2)
  await expect(order.nth(0)).toContainText("Aerodactyl")
  await expect(order.nth(0)).toContainText("Lv. 12")
  await expect(order.nth(1)).toContainText("Golem")
  await expect(order.nth(1)).toContainText("Lv. 14")

  // Reorder and edit roster slots; the team is always the first N.
  await page.getByRole("button", { name: "Move roster slot 3 up", exact: true }).click()
  await expect(order.nth(0)).toContainText("Kabutops")
  await page.getByLabel("Roster slot 2 species", { exact: true }).fill("Onix")
  await page.getByLabel("Roster slot 2 level offset", { exact: true }).fill("-6")
  await page.getByLabel("Roster slot 2 moves", { exact: true }).fill("Rock Throw, Bind")
  await page.getByLabel("Roster slot 2 item", { exact: true }).fill("Hard Stone")
  await page.getByRole("button", { name: "Apply roster", exact: true }).click()
  await expect(order.nth(0)).toContainText("Onix")
  await expect(order.nth(0)).toContainText("Lv. 8")
  await expect(order.nth(0)).toContainText("Rock Throw, Bind · Hard Stone")

  // World progress is the player TR; Brock grows with it along his archetype.
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("player-tr")).toHaveText("80")
  await expect(page.getByTestId("world-progress")).toHaveText("80")
  await expect(page.getByTestId("level-cap")).toHaveText("Lv. 50")
  await expect(page.getByTestId("wild-level")).toHaveText("Lv. 40")
  await expect(page.getByTestId("wild-gap")).toHaveText("-10 vs cap")
  await expect(page.getByTestId("regular-trainer-level")).toHaveText("Lv. 44")
  await expect(page.getByTestId("regular-trainer-gap")).toHaveText("-6 vs cap")
  await expect(page.getByTestId("selected-tr")).toHaveText("58")
  await expect(page.getByTestId("tr-brock")).toHaveText("58")
  await expect(page.getByTestId("selected-level")).toHaveText("Lv. 38")
  await expect(page.getByTestId("team-size")).toHaveText("4")
  await expect(page.getByTestId("gap-brock")).toHaveText("-12")
  await expect(page.getByTestId("growth-table").locator("tbody tr").first()).toHaveText(
    /TR\s*20\s*39\s*58\s*76\s*95/,
  )
  await expect(order).toHaveCount(4)
  await expect(order.nth(2)).toContainText("Onix")
  await expect(order.nth(2)).toContainText("Lv. 32")

  // TR is uncapped; the scalers stay flat past their last anchor (TR 160).
  await setGrowth(page, { archetype: "plateau", peak: "200" })
  await expect(page.getByTestId("selected-tr")).toHaveText("200")
  await expect(page.getByTestId("archetype-brock")).toHaveText("plateau")
  await expect(page.getByTestId("level-brock")).toHaveText("Lv. 100")
  await expect(page.getByTestId("size-brock")).toHaveText("6")
  await expect(order).toHaveCount(6)
  await expect(order.last()).toContainText("Golem")
  await expect(order.last()).toContainText("Lv. 100")
  await expect(order.first()).toContainText("Kleavor")
  await expect(order.first()).toContainText("Lv. 98")
  await setGrowth(page, { archetype: "steady", peak: "95" })
  await expect(page.getByTestId("selected-tr")).toHaveText("58")

  const downloadPromise = page.waitForEvent("download")
  await page.getByRole("button", { name: "Export experiment", exact: true }).click()
  const exportedPath = await (await downloadPromise).path()
  if (!exportedPath) throw new Error("Export did not produce a file")
  await page.getByText("Scalers & experiment settings", { exact: true }).click()
  await page.getByRole("button", { name: "Reset all to catalog defaults", exact: true }).click()
  await expect(order.nth(2)).toContainText("Aerodactyl")
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles(exportedPath)
  await expect(order.nth(2)).toContainText("Onix")
  await page.reload()
  await expect(order.nth(2)).toContainText("Onix")
  await expect(page.getByTestId("badge-count")).toHaveText("8")
  expect(errors).toEqual([])
})

test("grows Blue with the rival scaler and shows each trainer's TR at the checkpoints", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  await page.getByRole("button", { name: "Blue Kanto · Champion", exact: true }).click()
  await expect(page.getByTestId("growth-blue")).toHaveText("0 → 170")
  await expect(page.getByTestId("archetype-blue")).toHaveText("rival")
  // The Pallet fight: TR 0, one Eevee at Lv 5.
  await expect(page.getByTestId("selected-tr")).toHaveText("0")
  await expect(page.getByTestId("team-size")).toHaveText("1")
  await expect(page.getByTestId("battle-order").locator("li")).toHaveText([/Eevee.*Lv\. 5/s])
  await expect(page.getByLabel("Lead", { exact: true })).toHaveCount(0)
  await expect(page.getByTestId("growth-table").locator("tbody tr").first()).toHaveText(
    /TR\s*0\s*49\s*90\s*129\s*170/,
  )
  await page.getByRole("button", { name: "Set 16 badges", exact: true }).click()
  await expect(page.getByTestId("selected-tr")).toHaveText("129")
  // The rival growth scaler is editable like any other: 80% at world progress 120 is TR 136.
  await page.getByText("Scalers & experiment settings", { exact: true }).click()
  await page.getByLabel("Rival growth anchor 5 value", { exact: true }).fill("80")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByTestId("selected-tr")).toHaveText("136")
  await setGrowth(page, { start: "10", archetype: "steady", peak: "180" })
  await expect(page.getByTestId("growth-blue")).toHaveText("10 → 180")
  // Steady at world progress 120: 10 + 75% of 170 = 137.5, halves round up.
  await expect(page.getByTestId("selected-tr")).toHaveText("138")
})

test("ranks the Gym Leaders against the player TR in the Gym ladder", async ({ page }) => {
  await page.goto("/#trainer-balance")
  const ladder = page.getByTestId("gym-ladder").locator(":scope > li")
  await expect(ladder).toHaveCount(24)
  await expect(page.getByTestId("ladder-tate-liza")).toContainText("Tate & Liza · double battle")
  // At world progress 0 the openers start at TR 20 (two Pokémon at Lv 14, under the level cap).
  await expect(page.getByTestId("ladder-counts")).toHaveText("0 below · 11 near · 13 above")
  await expect(ladder.first()).toContainText("Bugsy")
  await expect(page.getByTestId("ladder-brock")).toContainText("TR 20")
  await expect(page.getByTestId("ladder-brock")).toContainText("above +20")
  await expect(ladder.last()).toContainText("Sabrina")
  await expect(ladder.last()).toContainText("above +25")
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("ladder-counts")).toHaveText("10 below · 9 near · 5 above")
  await expect(ladder.last()).toContainText("Norman")
  await expect(page.getByTestId("ladder-brock")).toContainText("below -22")
  await page.getByRole("button", { name: "Set 24 badges", exact: true }).click()
  await expect(page.getByTestId("ladder-counts")).toHaveText("14 below · 5 near · 5 above")
  await expect(ladder.last()).toContainText("Juan")
  await expect(ladder.last()).toContainText("TR 185")
})

test("puts the top five by TR at the current world progress in the lineup, strongest last", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  const matches = page.getByTestId("league-lineup").locator(":scope > li")
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(matches).toHaveCount(5)
  for (const [index, name] of ["Giovanni", "Bruno", "Will", "Norman", "Agatha"].entries())
    await expect(matches.nth(index).locator("h3")).toContainText(name)
  await expect(page.getByTestId("league-names")).toContainText(
    "Indigo, Sevii Masters, Hoenn all use this lineup",
  )
  // Beatable at 8 badges (level cap Lv 50): the lineup runs TR 93–95, team level 58–59.
  await expect(page.getByTestId("league-range")).toHaveText("TR 93 … 95")
  await expect(page.getByTestId("league-match-1")).toContainText("TR 93 · Lv. 58 · 5 Pokémon")
  await expect(page.getByTestId("league-match-5")).toContainText("TR 95 · Lv. 59 · 5 Pokémon")
  // At 16 badges (level cap Lv 75) the lineup is a little above the cap.
  await page.getByRole("button", { name: "Set 16 badges", exact: true }).click()
  await expect(page.getByTestId("league-range")).toHaveText("TR 130 … 132")
  await expect(page.getByTestId("league-match-1")).toContainText("Steven")
  await expect(page.getByTestId("league-match-1")).toContainText("TR 130 · Lv. 81 · 6 Pokémon")
  await expect(page.getByTestId("league-match-5")).toContainText("TR 132 · Lv. 83 · 6 Pokémon")
  // A plateau Brock with peak TR 150 moves into match 5 and drops Steven.
  await setGrowth(page, { archetype: "plateau", peak: "150" })
  await expect(page.getByTestId("league-match-5")).toContainText("Brock")
  await expect(page.getByTestId("league-lineup")).not.toContainText("Steven")
  // Tate & Liza fight a double battle, so even far ahead they never enter the lineup.
  await page.getByRole("button", { name: /^Tate & Liza/ }).click()
  await expect(page.getByTestId("double-battle")).toHaveText("Double battle")
  await expect(page.getByTestId("double-battle-note")).toContainText("not in the league pool")
  await setGrowth(page, { archetype: "plateau", peak: "300" })
  await expect(page.getByTestId("tr-tate-liza")).toHaveText("300")
  await expect(page.getByTestId("league-lineup")).not.toContainText("Tate")
  await expect(page.getByTestId("league-match-5")).toContainText("Brock")
})

test("flags incomplete rosters and rejects invalid edits and version 4 to 7 files", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  await expect(page.getByTestId("roster-gap-count")).toHaveText("11")
  await expect(page.getByTestId("roster-incomplete")).toHaveCount(0)

  await page.getByLabel("Roster slot 1 level offset", { exact: true }).fill("-1")
  await page.getByRole("button", { name: "Apply roster", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText("roster slot 1 must have level offset 0")
  await page.getByRole("button", { name: "Move roster slot 1 down", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText("roster slot 1 must have level offset 0")
  await setGrowth(page, { start: "-3" })
  await expect(page.getByRole("alert")).toContainText("Start TR must be a whole number")
  await setGrowth(page, { start: "50", peak: "40" })
  await expect(page.getByRole("alert")).toContainText("Peak TR must be at least start TR")
  await expect(page.getByTestId("selected-tr")).toHaveText("20")
  await expect(page.getByTestId("archetype-brock")).toHaveText("steady")

  const importFile = (version: number, body: object) =>
    page.getByLabel("Import experiment file", { exact: true }).setInputFiles({
      name: `v${version}.json`,
      mimeType: "application/json",
      buffer: Buffer.from(
        JSON.stringify({
          tool: "wayfarer-trainer-balance",
          version,
          selectedTrainer: "blue",
          ...body,
        }),
      ),
    })
  await importFile(4, {
    point: { badges: 8, leagueClears: 0, completedEditions: 0 },
    seed: 1,
    flags: [],
    experiment: { version: 4, arcs: {}, trainers: {} },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 4 experiments use a retired trainer model",
  )
  await importFile(5, {
    point: { badges: 8, leagueClears: 1 },
    experiment: { version: 5, teamLevel: [[0, 15]], teamSize: [[0, 2]], trainers: {} },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 5 experiments use the retired 0–80 player TR scale",
  )
  await importFile(6, {
    point: { badges: 8 },
    experiment: {
      version: 6,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      trainers: { blue: { tr: 6, roster: [] } },
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 6 experiments give each notable trainer one fixed TR",
  )
  await importFile(7, {
    point: { badges: 8 },
    experiment: {
      version: 7,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: {},
      trainers: { blue: { startTR: 10, archetype: "rival", peakTR: 180, lead: 10, roster: [] } },
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 7 experiments give the rival a fixed lead",
  )
  await expect(page.getByTestId("badge-count")).toHaveText("0")
  await expect(page.getByTestId("selected-tr")).toHaveText("20")

  await page.getByRole("button", { name: "Lorelei Kanto · Elite Four", exact: true }).click()
  await expect(page.getByTestId("roster-incomplete")).toBeVisible()
  await expect(page.getByTestId("roster-length")).toHaveText("5 / 6")
  await expect(page.getByTestId("warnings")).toContainText("lists 5 of 6 Pokémon")
  await page.getByRole("button", { name: "Add roster slot", exact: true }).click()
  await page.getByLabel("Roster slot 6 species", { exact: true }).fill("Articuno")
  await page.getByRole("button", { name: "Apply roster", exact: true }).click()
  await expect(page.getByTestId("roster-length")).toHaveText("6 / 6")
  await expect(page.getByTestId("roster-incomplete")).toHaveCount(0)
  await expect(page.getByTestId("roster-gap-count")).toHaveText("10")
  await expect(page.getByRole("button", { name: "Add roster slot", exact: true })).toHaveCount(0)
})

test("edits the trainer, world and archetype scalers globally", async ({ page }) => {
  await page.goto("/#trainer-balance")
  await page.getByText("Scalers & experiment settings", { exact: true }).click()
  // Lance is TR 48 at world progress 0.
  await expect(page.getByTestId("size-lance")).toHaveText("4")
  // Without the (44, 4) anchor, size ramps from (43, 3) to (70, 4): TR 48 rounds to 3.
  await page.getByRole("button", { name: "Remove Team size anchor 7", exact: true }).click()
  await expect(page.getByTestId("size-lance")).toHaveText("3")
  await page.getByLabel("Team level anchor 1 value", { exact: true }).fill("30")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText("values must not decrease")
  // Falkner is TR 14: from (0, 8) to (20, 14) that is Lv. 12.2, so Lv. 12.
  await expect(page.getByTestId("level-falkner")).toHaveText("Lv. 11")
  await page.getByLabel("Team level anchor 1 value", { exact: true }).fill("8")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByTestId("level-falkner")).toHaveText("Lv. 12")

  // The wild and regular trainer level curves read the player TR and move only the world scaling readout.
  await page.getByRole("button", { name: "Set 4 badges", exact: true }).click()
  await expect(page.getByTestId("player-tr")).toHaveText("40")
  await expect(page.getByTestId("level-cap")).toHaveText("Lv. 28")
  await expect(page.getByTestId("wild-gap")).toHaveText("-4 vs cap")
  await expect(page.getByTestId("regular-trainer-gap")).toHaveText("-1 vs cap")
  await page.getByLabel("Wild level anchor 2 value", { exact: true }).fill("28")
  await page.getByLabel("Regular trainer level anchor 2 value", { exact: true }).fill("30")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByTestId("wild-level")).toHaveText("Lv. 28")
  await expect(page.getByTestId("regular-trainer-gap")).toHaveText("+2 vs cap")

  // Archetype growth scalers read world progress: late bloomer Lance (48 → 200) at 8 badges.
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("tr-lance")).toHaveText("86")
  await page.getByLabel("Late bloomer growth anchor 3 value", { exact: true }).fill("50")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByTestId("tr-lance")).toHaveText("124")
  await page.getByLabel("Steady growth anchor 1 value", { exact: true }).fill("5")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText(
    "steady growth must be 0% at world progress 0",
  )
  await page.getByLabel("Steady growth anchor 1 value", { exact: true }).fill("0")

  await page.getByRole("button", { name: "Set 24 badges", exact: true }).click()
  await expect(page.getByTestId("player-tr")).toHaveText("160")
  await expect(page.getByTestId("level-cap")).toHaveText("Lv. 100")
  await page.getByRole("button", { name: "Reset all to catalog defaults", exact: true }).click()
  await expect(page.getByTestId("tr-lance")).toHaveText("200")
  await expect(page.getByTestId("size-lance")).toHaveText("6")
  await expect(page.getByTestId("wild-level")).toHaveText("Lv. 78")
  await expect(page.getByTestId("wild-gap")).toHaveText("-22 vs cap")
  await expect(page.getByTestId("regular-trainer-level")).toHaveText("Lv. 82")
  await expect(page.getByTestId("regular-trainer-gap")).toHaveText("-18 vs cap")
})

test("keeps controls and teams usable at a narrow viewport", async ({ page }) => {
  await page.setViewportSize({ width: 390, height: 844 })
  await page.goto("/#trainer-balance")
  await page.getByLabel("Badges earned", { exact: true }).focus()
  await page.keyboard.press("ArrowRight")
  await expect(page.getByTestId("badge-count")).toHaveText("1")
  await expect(page.getByTestId("world-progress")).toHaveText("10")
  await expect(page.getByTestId("battle-order")).toBeVisible()
  await expect(page.getByTestId("growth-editor")).toBeVisible()
  await expect(page.getByTestId("gym-ladder")).toBeVisible()
  await expect(page.getByTestId("league-lineup")).toBeVisible()
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
})

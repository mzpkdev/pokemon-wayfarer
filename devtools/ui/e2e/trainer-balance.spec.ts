import { expect, test } from "webanvil/e2e"

test("explores badge progression, edits ratings and restores an exported experiment", async ({
  page,
}) => {
  const errors: string[] = []
  page.on("pageerror", (error) => errors.push(error.message))
  await page.goto("/#trainer-balance")
  await expect(page.getByRole("heading", { name: "Trainer balance", exact: true })).toBeVisible()
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 15")
  const party = page.getByTestId("generated-party")
  await expect(party.locator("li")).toHaveCount(2)
  await expect(party).toContainText("Geodude")
  await expect(party).toContainText("Lv. 12")
  await expect(party).toContainText("Onix")
  await expect(party).toContainText("Lv. 14")

  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("player-tr")).toHaveText("40")
  await expect(party.locator("li")).toHaveCount(4)
  await page.getByRole("button", { name: "Set 24 badges", exact: true }).click()
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 62")
  await expect(party.locator("li")).toHaveCount(6)

  await page.getByLabel("TR at 24 badges", { exact: true }).fill("54")
  await page.getByRole("button", { name: "Apply ratings", exact: true }).click()
  await expect(page.getByTestId("selected-tr")).toHaveText("54")
  await page.getByLabel("First league clears", { exact: true }).selectOption("2")
  await expect(page.getByTestId("selected-tr")).toHaveText("66")
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 89")

  const downloadPromise = page.waitForEvent("download")
  await page.getByRole("button", { name: "Export experiment", exact: true }).click()
  const download = await downloadPromise
  const exportedPath = await download.path()
  if (!exportedPath) throw new Error("Export did not produce a file")
  await page.getByText("Shared NPC level curve & experiment settings", { exact: true }).click()
  await page.getByRole("button", { name: "Reset all trainer defaults", exact: true }).click()
  await expect(page.getByTestId("selected-tr")).toHaveText("64")
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles(exportedPath)
  await expect(page.getByTestId("selected-tr")).toHaveText("66")
  await page.reload()
  await expect(page.getByTestId("selected-tr")).toHaveText("66")
  await expect(page.getByTestId("badge-count")).toHaveText("24")
  expect(errors).toEqual([])
})

test("rejects invalid changes without replacing the current experiment", async ({ page }) => {
  await page.goto("/#trainer-balance")
  await page.getByLabel("TR at 0 badges", { exact: true }).fill("70")
  await page.getByRole("button", { name: "Apply ratings", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText("must not decrease")
  await expect(page.getByTestId("selected-tr")).toHaveText("2")
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles({
    name: "invalid.json",
    mimeType: "application/json",
    buffer: Buffer.from('{"version":1}'),
  })
  await expect(page.getByRole("alert")).toContainText("not a version 1")
  await expect(page.getByTestId("selected-tr")).toHaveText("2")
  await page.getByLabel("Search trainers", { exact: true }).fill("does-not-exist")
  await expect(page.getByText("No trainers match these filters.")).toBeVisible()
  await page.getByRole("button", { name: "Clear filters", exact: true }).click()
  await page.getByLabel("Filter region", { exact: true }).selectOption("Hoenn")
  await page.getByLabel("Filter role", { exact: true }).selectOption("Gym Leaders")
  await expect(page.locator(".pool-table tbody tr")).toHaveCount(7)
  await page.getByRole("button", { name: "Juan Hoenn · Gym Leader", exact: true }).click()
  await expect(page.getByRole("complementary", { name: "Selected trainer" })).toContainText("Juan")
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
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
})

test("gives Blue a distinct Champion growth curve and restores only his saved defaults", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  await page.getByLabel("TR at 0 badges", { exact: true }).fill("3")
  await page.getByRole("button", { name: "Apply ratings", exact: true }).click()
  await page.getByLabel("Search trainers", { exact: true }).fill("Blue")
  await page.getByRole("button", { name: "Blue Kanto · Champion", exact: true }).click()
  await expect(page.getByTestId("selected-tr")).toHaveText("6")
  await expect(page.getByTestId("generated-party").locator("li")).toHaveCount(2)
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("selected-tr")).toHaveText("54")
  await expect(page.getByTestId("generated-party")).toContainText("Lv. 59")
  await expect(page.getByTestId("generated-party").locator("li")).toHaveCount(6)
  await expect(page.getByTestId("player-cap")).toHaveText("Lv. 42")

  // Simulate the previous locally saved Blue curve; loading must preserve it.
  await page.getByLabel("TR at 8 badges", { exact: true }).fill("36")
  await page.getByLabel("TR at 16 badges", { exact: true }).fill("46")
  await page.getByRole("button", { name: "Apply ratings", exact: true }).click()
  await page.reload()
  await expect(page.getByTestId("selected-tr")).toHaveText("36")
  await page.getByRole("button", { name: "Restore this trainer’s defaults", exact: true }).click()
  await expect(page.getByTestId("selected-tr")).toHaveText("54")
  await expect(page.getByTestId("badge-count")).toHaveText("8")
  await page.reload()
  await expect(page.getByTestId("selected-tr")).toHaveText("54")
  await page.getByLabel("Search trainers", { exact: true }).fill("Brock")
  await page.getByRole("button", { name: "Brock Kanto · Gym Leader", exact: true }).click()
  await expect(page.getByLabel("TR at 0 badges", { exact: true })).toHaveValue("3")
})

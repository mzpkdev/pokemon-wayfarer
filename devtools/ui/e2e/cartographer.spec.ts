import { expect, test } from "webanvil/e2e"

test("previews scaled encounters without remounting the map", async ({ page }) => {
  await page.goto("/")

  const mapSearch = page.getByRole("combobox", { name: "Name or map section" })
  await mapSearch.fill("LakeOfRage")
  await page.getByRole("option", { name: /LakeOfRage_hns/ }).click()
  await page
    .getByRole("navigation", { name: "Cartographer views" })
    .getByRole("button", { name: "Encounters", exact: true })
    .click()

  const slider = page.getByRole("slider", { name: "Trainer Rating" })
  await expect(slider).toHaveAttribute("aria-valuenow", "10")
  const mapRoster = page.getByLabel("Encounter roster preview")
  await expect(mapRoster).toContainText(/Magikarp/i)
  await expect(mapRoster).not.toContainText(/Gyarados/i)
  const regionalMap = page.getByRole("region", { name: "Interactive regional map" })
  const mapBounds = await regionalMap.boundingBox()
  if (!mapBounds) throw new Error("The regional map needs visible bounds")
  await regionalMap.click({ position: { x: mapBounds.width / 2, y: mapBounds.height / 2 } })
  const rosterPopup = page.getByRole("dialog", { name: /LakeOfRage_hns.*Water/ })
  await expect(rosterPopup).toBeVisible()
  const authoredGyarados = rosterPopup.locator('[aria-label="Authored slot 5 GYARADOS"]')
  await expect(authoredGyarados.getByText("MAGIKARP", { exact: true })).toBeVisible()
  await expect(authoredGyarados.getByText("Projected Lv. 11-13", { exact: true })).toBeVisible()
  const sourceSet = page.locator('details[aria-label="Source set gLakeOfRage_hns_Day"]')
  await sourceSet.locator(":scope > summary").click()
  const water = sourceSet.locator('details[aria-label="Water encounter method"]')
  await water.locator(":scope > summary").click()
  const scalingRow = water.locator("tbody tr").nth(4)
  await expect(scalingRow.getByText(/^Gyarados$/i)).toHaveCount(1)
  await expect(scalingRow.getByText(/^Magikarp$/i)).toHaveCount(1)

  const viewport = page.getByLabel("Interactive cartographer")
  await viewport.evaluate((element) => element.setAttribute("data-scaling-e2e", "mounted"))
  await slider.focus()
  for (let rating = 10; rating < 30; rating += 1) await slider.press("ArrowRight")
  await expect(slider).toHaveAttribute("aria-valuenow", "30")
  await expect(page).toHaveURL(/rating=30/)
  await expect(viewport).toHaveAttribute("data-scaling-e2e", "mounted")
  await expect(mapRoster).toContainText(/Gyarados/i)
  await expect(scalingRow.getByText(/^Gyarados$/i)).toHaveCount(2)
  await expect(scalingRow.getByText(/^Magikarp$/i)).toHaveCount(0)
  await expect(rosterPopup).toBeVisible()
  await expect(rosterPopup).toContainText("Trainer Rating 30")
  await expect(authoredGyarados.getByText("GYARADOS", { exact: true })).toBeVisible()
  await expect(authoredGyarados.getByText("Projected Lv. 23-25", { exact: true })).toBeVisible()
  await page.keyboard.press("Escape")
  await expect(rosterPopup).toHaveCount(0)
  const populationControls = page.getByRole("navigation", {
    name: "Projected encounter populations",
  })
  const waterPopulationButton = populationControls.getByRole("button", { name: "Water" })
  await waterPopulationButton.focus()
  await waterPopulationButton.press("Enter")
  await expect(rosterPopup).toBeVisible()
  await page.keyboard.press("Escape")
  await expect(waterPopulationButton).toBeFocused()

  await mapSearch.fill("CeruleanCity_Frlg")
  await page.getByRole("option", { name: /CeruleanCity_Frlg/ }).click()
  const version = page.getByRole("combobox", { name: "Game version" })
  await expect(version).toBeVisible()
  await version.selectOption("FIRERED")
  await expect(page.locator('details[aria-label="Source set sCeruleanCity_FireRed"]')).toHaveCount(
    1,
  )
  await expect(
    page.locator('details[aria-label="Source set sCeruleanCity_LeafGreen"]'),
  ).toHaveCount(0)
  await populationControls.getByRole("button", { name: "Water" }).click()
  const ceruleanPopup = page.getByRole("dialog", { name: /CeruleanCity_Frlg.*Water/ })
  await expect(
    ceruleanPopup.locator('[aria-label="Source encounter set sCeruleanCity_FireRed"]'),
  ).toHaveCount(1)
  await version.selectOption("LEAFGREEN")
  await expect(page).toHaveURL(/product=LEAFGREEN/)
  await expect(page.locator('details[aria-label="Source set sCeruleanCity_FireRed"]')).toHaveCount(
    0,
  )
  await expect(
    page.locator('details[aria-label="Source set sCeruleanCity_LeafGreen"]'),
  ).toHaveCount(1)
  await expect(ceruleanPopup).toBeVisible()
  await expect(
    ceruleanPopup.locator('[aria-label="Source encounter set sCeruleanCity_FireRed"]'),
  ).toHaveCount(0)
  await expect(
    ceruleanPopup.locator('[aria-label="Source encounter set sCeruleanCity_LeafGreen"]'),
  ).toHaveCount(1)
})

test("shows the cartographer", async ({ page }) => {
  await page.goto("/")

  await expect(page.getByRole("link", { name: "Cartographer" })).toBeVisible()
  await expect(page.getByRole("navigation", { name: "Regions" })).toBeVisible()
  await expect
    .poll(() =>
      page.evaluate(() =>
        getComputedStyle(document.documentElement)
          .getPropertyValue("--color-cartographer-habitat-land")
          .trim(),
      ),
    )
    .toBe("#7f9875")
  await expect
    .poll(() =>
      page.evaluate(() =>
        getComputedStyle(document.documentElement)
          .getPropertyValue("--color-cartographer-signal-strong")
          .trim(),
      ),
    )
    .toBe("#d7e0e7")

  const alolaOverview = page.waitForResponse(
    (response) =>
      response.url().endsWith("/maps/alola/routes/MelemeleIsle_hns.png") &&
      response.status() === 200,
  )
  await page.getByRole("button", { name: /Alola 7 maps/ }).click()
  await expect(page.getByRole("heading", { name: "Alola", exact: true })).toBeVisible()
  await alolaOverview
  await expect(page.getByLabel("Interactive cartographer").locator("canvas").first()).toBeVisible()

  const mapSearch = page.getByRole("combobox", { name: "Name or map section" })
  await mapSearch.fill("Route29")
  await page.getByRole("option", { name: /Route29_hns/ }).click()
  await expect(page.getByRole("heading", { name: "Route29_hns" })).toBeVisible()

  const exits = page.getByRole("checkbox", { name: "Exits" })
  await expect(exits).not.toBeChecked()
  await page.getByLabel("Interactive cartographer").getByText("Exits", { exact: true }).click()
  await expect(exits).toBeChecked()

  const objects = page.getByRole("checkbox", { name: "Objects" })
  await expect(objects).not.toBeChecked()
  await page.getByLabel("Interactive cartographer").getByText("Objects", { exact: true }).click()
  await expect(objects).toBeChecked()
  const trainers = page.getByRole("checkbox", { name: /Trainer/ })
  await expect(trainers).toBeChecked()
  await page
    .getByLabel("Object filters")
    .getByText(/Trainer/)
    .click()
  await expect(trainers).not.toBeChecked()

  await mapSearch.fill("Route15")
  await page.getByRole("option", { name: /Route15_hns/ }).click()
  await expect(page.getByRole("heading", { name: "Route15_hns" })).toBeVisible()
  await page
    .getByRole("complementary")
    .last()
    .locator('details[aria-label="Objects"] summary')
    .click()
  await page.getByRole("button", { name: /OBJ_EVENT_GFX_ITEM_BALL/ }).click()
  await expect(page.getByText("Object inspector")).toBeVisible()
  await expect(page.getByText("Gives PP Up")).toBeVisible()
  await expect(page.getByText("Route15_EventScript_PPup")).toBeVisible()
  await expect(page.getByText("Sprite", { exact: true })).toBeVisible()

  await mapSearch.fill("Route20")
  await page.getByRole("option", { name: /Route20_hns/ }).click()
  await expect(page.getByRole("heading", { name: "Route20_hns" })).toBeVisible()
  const route20Inspector = page.getByRole("complementary").last()
  await route20Inspector.locator('details[aria-label="Objects"] summary').click()
  await expect
    .poll(() =>
      route20Inspector.evaluate((inspector) => inspector.scrollWidth <= inspector.clientWidth),
    )
    .toBe(true)
  await expect
    .poll(() => page.evaluate(() => document.body.scrollWidth === document.body.clientWidth))
    .toBe(true)

  await mapSearch.fill("Route32")
  await page.getByRole("option", { name: /Route32_hns/ }).click()
  await expect(page.getByRole("checkbox", { name: /Topology diagnostics/ })).toHaveCount(0)

  await mapSearch.fill("RuinsOfAlph_Outside")
  await page.getByRole("option", { name: /RuinsOfAlph_Outside_hns/ }).click()
  const inspector = page.getByRole("complementary").last()
  await inspector.locator('details[aria-label="Exits"] summary').click()
  const exitCards = inspector.getByRole("button", { name: /Warp \d/ })
  await expect(exitCards).toHaveCount(10)
  await expect
    .poll(async () => {
      const inspectorBounds = await inspector.evaluate((element) => {
        const bounds = element.getBoundingClientRect()
        return { left: bounds.left, right: bounds.right }
      })
      const exitBounds = await exitCards.evaluateAll(
        (elements, bounds) =>
          elements.map((element) => {
            const cardBounds = element.getBoundingClientRect()
            return {
              contained: cardBounds.left >= bounds.left && cardBounds.right <= bounds.right,
              fits: element.scrollWidth <= element.clientWidth,
            }
          }),
        inspectorBounds,
      )
      return exitBounds.every((bounds) => bounds.contained && bounds.fits)
    })
    .toBe(true)

  await mapSearch.fill("Route32")
  await page.getByRole("option", { name: /Route32_hns/ }).click()
  const cartographerViews = page.getByRole("navigation", { name: "Cartographer views" })
  await cartographerViews.getByRole("button", { name: "Encounters", exact: true }).click()
  await expect(page.getByLabel("Interactive cartographer")).toBeVisible()
  await expect(page.getByText("encounter maps", { exact: false })).toBeVisible()
  await expect(page.getByText("runtime-valid land and water tiles", { exact: false })).toBeVisible()
  const trainerEventsToggle = page.getByRole("checkbox", { name: "Trainer events" })
  await expect(trainerEventsToggle).toBeChecked()
  const trainerEvents = page.locator('details[aria-label="Trainer events"]')
  await trainerEvents.locator("summary").click()
  await expect(trainerEvents.getByText("Battles Albert", { exact: true })).toBeVisible()
  await trainerEvents.getByRole("button", { name: /Battles Albert/ }).click()
  await expect(trainerEvents.getByRole("button", { name: /Battles Albert/ })).toHaveAttribute(
    "aria-pressed",
    "true",
  )
  await page
    .getByLabel("Interactive cartographer")
    .getByText("Trainer events", { exact: true })
    .click()
  await expect(trainerEventsToggle).not.toBeChecked()
  await expect
    .poll(() => page.evaluate(() => document.body.scrollWidth))
    .toBe(await page.evaluate(() => document.body.clientWidth))
  const runtimeTimes = page.locator('details[aria-label="Runtime encounter times"]')
  await runtimeTimes.locator("summary").click()
  await expect(runtimeTimes).toBeVisible()
  await expect(runtimeTimes.getByText("Night", { exact: true })).toBeVisible()
  await expect(runtimeTimes.getByText("gRoute32_hns_Night", { exact: true })).toHaveCount(4)
  await expect(runtimeTimes.getByText("Falls back to Day", { exact: true })).toHaveCount(8)
  const route32Set = page.locator('details[aria-label="Source set gRoute32_hns_Day"]')
  await route32Set.locator(":scope > summary").click()
  const fishing = route32Set.locator('details[aria-label="Fishing encounter method"]')
  await fishing.locator(":scope > summary").click()
  await route32Set.locator('details[aria-label="Land encounter method"] > summary').click()
  await route32Set.locator('details[aria-label="Water encounter method"] > summary').click()
  await expect
    .poll(() => route32Set.evaluate((section) => section.scrollWidth <= section.clientWidth))
    .toBe(true)
  await expect
    .poll(() => page.evaluate(() => document.body.scrollWidth === document.body.clientWidth))
    .toBe(true)
  await expect(route32Set.getByLabel("Old Rod fishing")).toBeVisible()
  await expect(route32Set.getByLabel("Good Rod fishing")).toBeVisible()
  await expect(route32Set.getByLabel("Super Rod fishing")).toBeVisible()
  await expect(route32Set.getByText("Common band", { exact: false }).first()).toBeVisible()
  await expect(route32Set.getByText("Less common band", { exact: false }).first()).toBeVisible()
  await expect(route32Set.getByText("Rare band", { exact: false }).first()).toBeVisible()
  await expect(route32Set.getByText(/^Magikarp$/i).first()).toBeVisible()
  const encounterSprite = fishing.locator('img[src*="pokemon-icons/"]').first()
  await expect(encounterSprite).toBeVisible()
  await expect
    .poll(() => encounterSprite.evaluate((image) => image.naturalWidth))
    .toBeGreaterThan(0)

  await cartographerViews.getByRole("button", { name: "World", exact: true }).click()
  const atlasOverlaps = page.getByRole("checkbox", { name: /Overlaps/ })
  await expect(atlasOverlaps).toBeVisible()
  await page
    .getByLabel("Interactive cartographer")
    .getByText(/Overlaps/)
    .click()
  await expect(atlasOverlaps).toBeChecked()
  await expect(page.getByLabel("Overlap details")).toBeVisible()

  await page.getByRole("link", { name: "Metatiles" }).click()
  await expect(page.getByRole("heading", { name: "Metatiles", exact: true })).toBeVisible()
  await expect(
    page.getByText("colors are not assumed to be universal", { exact: false }),
  ).toBeVisible()
  const unusedMetatiles = page.getByRole("checkbox", { name: "Include unused source metatiles" })
  await expect(unusedMetatiles).not.toBeChecked()
  const unusedMetatilesControl = unusedMetatiles.locator("xpath=..")

  const metatileBrowser = page.getByLabel("Metatile browser")
  const firstMetatile = metatileBrowser.locator('button[aria-label*=":0x"]').first()
  await expect(firstMetatile).toBeVisible()
  const metatileSourceId = await firstMetatile.getAttribute("aria-label")
  if (!metatileSourceId) throw new Error("A metatile needs a scoped source ID")
  await firstMetatile.click()

  const metatileInspector = page.getByLabel("Metatile inspector")
  await expect(metatileInspector.getByText(metatileSourceId, { exact: true })).toBeVisible()
  await expect(metatileInspector.getByText("Source tiles", { exact: true })).toBeVisible()
  await metatileInspector.locator('details[aria-label="Used by maps"] > summary').click()
  await expect
    .poll(() =>
      metatileInspector.evaluate((inspector) => inspector.scrollWidth <= inspector.clientWidth),
    )
    .toBe(true)
  await expect
    .poll(() => page.evaluate(() => document.body.scrollWidth === document.body.clientWidth))
    .toBe(true)

  const metatileSearch = page.getByRole("searchbox", { name: "Scoped or local ID" })
  await metatileSearch.fill(metatileSourceId)
  await expect(metatileBrowser.locator('button[aria-label*=":0x"]').first()).toBeVisible()

  await page.getByRole("searchbox", { name: "Find a render context" }).fill("BattleDome")
  await page.route(
    "**/metatiles/contexts/emerald--building--battle-dome/catalog.json",
    async (route) => {
      await new Promise((resolve) => setTimeout(resolve, 250))
      await route.continue()
    },
  )
  const battleDomeContext = page.getByRole("button", {
    name: /gTileset_Building \+ gTileset_BattleDome/,
  })
  const renderContextCard = metatileBrowser.locator(":scope > div").first()
  const renderContextCardHeight = await renderContextCard.evaluate(
    (card) => card.getBoundingClientRect().height,
  )
  await battleDomeContext.click()
  await expect(battleDomeContext).toHaveAttribute("aria-current", "true")
  await expect(battleDomeContext).toHaveAttribute("aria-busy", "true")
  await expect
    .poll(() => renderContextCard.evaluate((card) => card.getBoundingClientRect().height))
    .toBe(renderContextCardHeight)
  await expect(metatileBrowser.getByText("gTileset_General + gTileset_Cave")).toBeVisible()
  await expect(metatileBrowser.getByText("gTileset_Building + gTileset_BattleDome")).toBeVisible()
  await expect(metatileBrowser.getByText(/1 shown · 1 used/)).toBeVisible()
  const unusedMetatilesControlY = await unusedMetatilesControl.evaluate(
    (control) =>
      control.getBoundingClientRect().y - control.parentElement!.getBoundingClientRect().y,
  )
  await page.getByText("Include unused source metatiles", { exact: true }).click()
  await expect(unusedMetatiles).toBeChecked()
  await expect
    .poll(() =>
      unusedMetatilesControl.evaluate(
        (control) =>
          control.getBoundingClientRect().y - control.parentElement!.getBoundingClientRect().y,
      ),
    )
    .toBe(unusedMetatilesControlY)
  await expect(metatileBrowser.getByText(/8 shown · 1 used/)).toBeVisible()
  await metatileBrowser.getByRole("button", { name: "Secondary", exact: true }).click()
  await expect(
    metatileBrowser.locator('button[aria-label*="gTileset_BattleDome:"]').first(),
  ).toHaveAttribute("aria-label", "gTileset_BattleDome:0x140")
})

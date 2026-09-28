import { readFile } from "node:fs/promises"

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
  const order = page.getByTestId("battle-order").locator(":scope > li")
  // Brock (start TR 25, a Steady, peak TR 100) at world progress 0: TR 25, team level 18, size 2.
  await expect(page.getByTestId("selected-tr")).toHaveText("25")
  await expect(page.getByTestId("growth-brock")).toHaveText("25 → 100")
  await expect(page.getByTestId("archetype-brock")).toHaveText("Steady")
  const options = page.getByLabel("Archetype", { exact: true }).locator("option")
  await expect(options).toHaveText([
    "Steady",
    "Prodigy",
    "Sleeper",
    "Veteran",
    "Rival",
    "Legend",
    "Star",
    "Comeback",
    "Burst",
  ])
  expect(
    await options.evaluateAll((items) => items.map((item) => item.getAttribute("value"))),
  ).toEqual([
    "steady",
    "prodigy",
    "sleeper",
    "veteran",
    "rival",
    "legend",
    "star",
    "comeback",
    "burst",
  ])
  await expect(page.getByTestId("selected-level")).toHaveText("Lv. 18")
  await expect(page.getByTestId("team-size")).toHaveText("2")
  await expect(order).toHaveCount(2)
  // Final stages step down to the stage their level supports: Golem is Geodude at Lv 16,
  // Steelix is Onix at Lv 18. The filler slot fights first and the ace (roster slot 1) last.
  await expect(order.nth(0)).toContainText("Geodude→ Golem at Lv 38")
  await expect(order.nth(0)).toContainText("Lv. 16")
  // Stepped down, Golem keeps its roster slot's item; its moves come from the move pool.
  await expect(order.nth(0)).toContainText("Offset -2 · Quick Claw")
  await expect(page.getByTestId("ace-2")).toHaveCount(0)
  await expect(order.nth(1)).toContainText("Onix→ Steelix at Lv 35Ace")
  await expect(order.nth(1)).toContainText("Lv. 18")
  // The growth table shows each roster slot's stage at the world progress checkpoints: TR 25, 44,
  // 63, 81 and 100 (team level 18, 30, 41, 51 and 63; team size 2, 4, 5, 6 and 6).
  await expect(page.getByTestId("growth-slot-1")).toHaveText(
    /Slot 1\s*Ace\s*Onix\s*Lv 18\s*Onix\s*Lv 30\s*Steelix\s*Lv 41\s*Steelix\s*Lv 51\s*Steelix\s*Lv 63/,
  )
  await expect(page.getByTestId("growth-slot-2")).toHaveText(
    /Slot 2\s*Geodude\s*Lv 16\s*Graveler\s*Lv 28\s*Golem\s*Lv 39\s*Golem\s*Lv 49\s*Golem\s*Lv 61/,
  )
  // The slot-6 Aerodactyl ace (offset 0) joins only at team size 6 (TR 71), so from world
  // progress 120.
  await expect(page.getByTestId("growth-slot-6")).toHaveText(
    /Slot 6\s*Ace\s*—\s*—\s*—\s*Aerodactyl\s*Lv 51\s*Aerodactyl\s*Lv 63/,
  )
  await expect(page.getByTestId("slot-line-2")).toHaveText("Geodude → Graveler Lv 25 → Golem Lv 38")
  await expect(page.getByTestId("stage-warning-1")).toHaveCount(0)

  // Reorder and edit roster slots; the team is always the first N. The filler Crobat (Zubat at
  // Lv 16) moves up into the team.
  await page.getByRole("button", { name: "Move roster slot 3 up", exact: true }).click()
  await expect(order.nth(0)).toContainText("Zubat→ Crobat at Lv 38")
  await expect(order.nth(0)).not.toContainText("Ace")
  await page.getByLabel("Roster slot 2 species", { exact: true }).fill("Onix")
  await page.getByLabel("Roster slot 2 level offset", { exact: true }).fill("-6")
  await page.getByLabel("Roster slot 2 item", { exact: true }).fill("Hard Stone")
  await page.getByRole("button", { name: "Apply roster", exact: true }).click()
  await expect(order.nth(0)).toContainText("Onix")
  await expect(order.nth(0)).toContainText("Lv. 12")
  await expect(order.nth(0)).toContainText("Offset -6 · Hard Stone")
  // Roster slots carry no moves.
  await expect(page.getByLabel("Roster slot 2 moves", { exact: true })).toHaveCount(0)
  // An earlier stage is allowed but flagged, and it never evolves forward.
  await expect(page.getByTestId("stage-warning-2")).toHaveText(
    "Onix is not a final stage. Roster slots normally author final stages.",
  )

  // World progress is the player TR; Brock grows with it along his archetype.
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("player-tr")).toHaveText("80")
  await expect(page.getByTestId("world-progress")).toHaveText("80")
  await expect(page.getByTestId("level-cap")).toHaveText("Lv. 50")
  await expect(page.getByTestId("wild-level")).toHaveText("Lv. 40")
  await expect(page.getByTestId("wild-gap")).toHaveText("-10 vs cap")
  await expect(page.getByTestId("regular-trainer-level")).toHaveText("Lv. 44")
  await expect(page.getByTestId("regular-trainer-gap")).toHaveText("-6 vs cap")
  await expect(page.getByTestId("selected-tr")).toHaveText("63")
  await expect(page.getByTestId("tr-brock")).toHaveText("63")
  await expect(page.getByTestId("selected-level")).toHaveText("Lv. 41")
  await expect(page.getByTestId("team-size")).toHaveText("5")
  await expect(page.getByTestId("gap-brock")).toHaveText("-9")
  await expect(page.getByTestId("growth-table").locator("tbody tr").first()).toHaveText(
    /TR\s*25\s*44\s*63\s*81\s*100/,
  )
  await expect(order).toHaveCount(5)
  await expect(order.nth(3)).toContainText("Onix")
  await expect(order.nth(3)).toContainText("Lv. 35")
  await expect(order.nth(4)).toContainText("Steelix")
  await expect(order.nth(4)).not.toContainText("→")
  await expect(order.nth(4)).toContainText("Lv. 41")

  // TR is uncapped; the scalers stay flat past their last anchor (TR 160).
  await setGrowth(page, { archetype: "veteran", peak: "200" })
  await expect(page.getByTestId("selected-tr")).toHaveText("200")
  await expect(page.getByTestId("archetype-brock")).toHaveText("Veteran")
  await expect(page.getByTestId("level-brock")).toHaveText("Lv. 100")
  await expect(page.getByTestId("size-brock")).toHaveText("6")
  await expect(order).toHaveCount(6)
  await expect(order.last()).toContainText("Steelix")
  await expect(order.last()).toContainText("Lv. 100")
  // Fillers first (Omastar, Kabutops, Golem, Onix), then aces (Aerodactyl, Steelix).
  await expect(order.first()).toContainText("Omastar")
  await expect(order.first()).toContainText("Lv. 98")
  await expect(order.nth(2)).toContainText("Golem")
  await expect(order.nth(2)).toContainText("Quick Claw")
  await expect(order.nth(3)).toContainText("Onix")
  await expect(order.nth(3)).toContainText("Lv. 94")
  await expect(order.nth(4)).toContainText("AerodactylAce")
  await expect(order.nth(4)).toContainText("Hard Stone")
  await setGrowth(page, { archetype: "steady", peak: "100" })
  await expect(page.getByTestId("selected-tr")).toHaveText("63")

  const downloadPromise = page.waitForEvent("download")
  await page.getByRole("button", { name: "Export experiment", exact: true }).click()
  const exportedPath = await (await downloadPromise).path()
  if (!exportedPath) throw new Error("Export did not produce a file")
  await page.getByText("Scalers & experiment settings", { exact: true }).click()
  await page.getByRole("button", { name: "Reset all to catalog defaults", exact: true }).click()
  await expect(order.nth(3)).toContainText("Golem")
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles(exportedPath)
  await expect(order.nth(3)).toContainText("Onix")
  await page.reload()
  await expect(order.nth(3)).toContainText("Onix")
  await expect(page.getByTestId("badge-count")).toHaveText("8")
  expect(errors).toEqual([])
})

test("sets the player TR directly, with badges as presets, and reads old badge points", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  const input = page.getByLabel("Player TR", { exact: true })
  await expect(input).toHaveValue("0")
  await expect(page.getByTestId("badge-match")).toHaveText("matches 0 badges")
  // A badge preset sets the player TR from the badge formula.
  await page.getByRole("button", { name: "Set 12 badges", exact: true }).click()
  await expect(input).toHaveValue("100")
  await expect(page.getByTestId("badge-match")).toHaveText("matches 12 badges")
  // TR 95 is exactly 11 badges; TR 97 is between badge counts.
  await input.fill("95")
  await expect(page.getByTestId("badge-count")).toHaveText("11")
  await input.fill("97")
  await expect(page.getByTestId("badge-count")).toHaveText("–")
  await expect(page.getByTestId("badge-match")).toHaveText("between 11 and 12 badges")
  await expect(page.getByLabel("Badges earned", { exact: true })).toHaveValue("11")
  // Everything that reads world progress follows the typed TR.
  await expect(page.getByTestId("player-tr")).toHaveText("97")
  await expect(page.getByTestId("world-progress")).toHaveText("97")
  await expect(page.getByTestId("level-cap")).toHaveText("Lv. 61")
  await expect(page.getByTestId("wild-level")).toHaveText("Lv. 48")
  await expect(page.getByTestId("regular-trainer-level")).toHaveText("Lv. 52")
  // Steady Brock (25 → 100) at world progress 97: 25 + 75 × 97/160 = 70.47.
  await expect(page.getByTestId("selected-tr")).toHaveText("70")
  await expect(page.getByTestId("tr-brock")).toHaveText("70")
  await expect(page.getByTestId("ladder-brock")).toContainText("below -27")
  await expect(page.getByTestId("event-lineup").locator(":scope > li")).toHaveCount(5)
  // The slider runs 0–200; the field takes any larger TR, which is past 24 badges.
  await page.getByLabel("Player TR slider", { exact: true }).fill("170")
  await expect(input).toHaveValue("170")
  await expect(page.getByTestId("badge-match")).toHaveText("beyond 24 badges")
  await input.fill("300")
  await expect(page.getByTestId("player-tr")).toHaveText("300")
  await expect(page.getByTestId("level-cap")).toHaveText("Lv. 100")
  await expect(page.getByTestId("selected-tr")).toHaveText("100")
  await expect(page.getByTestId("badge-match")).toHaveText("beyond 24 badges")
  await page.reload()
  await expect(input).toHaveValue("300")

  // An earlier version 8 export saved badges instead of a player TR; it still imports.
  const downloadPromise = page.waitForEvent("download")
  await page.getByRole("button", { name: "Export experiment", exact: true }).click()
  const exported = await (await downloadPromise).path()
  if (!exported) throw new Error("Export did not produce a file")
  const saved = JSON.parse(await readFile(exported, "utf8"))
  expect(saved.point).toEqual({ playerTR: 300 })
  await page.getByLabel("Import experiment file", { exact: true }).setInputFiles({
    name: "badges.json",
    mimeType: "application/json",
    buffer: Buffer.from(JSON.stringify({ ...saved, point: { badges: 16 } })),
  })
  await expect(input).toHaveValue("120")
  await expect(page.getByTestId("badge-count")).toHaveText("16")
})

test("shows the selected trainer's milestones and a team level chart", async ({ page }) => {
  const errors: string[] = []
  page.on("pageerror", (error) => errors.push(error.message))
  await page.goto("/#trainer-balance")
  const timeline = page.getByTestId("milestones").locator(":scope > li")
  await expect(timeline.first()).toHaveText(
    /^0\s*Onix, Geodude \(team level above the level cap\)$/,
  )
  await expect(page.getByTestId("milestone-8")).toHaveText(/8\s*3rd slot \(Zubat\) joins/)
  await expect(page.getByTestId("milestone-40")).toHaveText(/40\s*4th slot \(Kabuto\) joins/)
  await expect(page.getByTestId("milestone-57")).toHaveText(/57\s*Onix → Steelix/)
  await expect(page.getByTestId("milestone-68")).toHaveText(/68\s*5th slot \(Omanyte\) joins/)
  await expect(page.getByTestId("milestone-98")).toHaveText(
    /98\s*6th slot \(Aerodactyl\) ace joins/,
  )
  await expect(timeline.last()).toHaveText(/159\s*peak TR 100/)
  await expect(timeline.first()).toHaveAttribute("aria-current", "step")
  await expect(page.getByTestId("milestone-now")).toHaveCount(0)
  // At 8 badges (player TR 80) the marker sits between Graveler → Golem, Golbat → Crobat (76)
  // and Kabuto → Kabutops (85).
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("milestone-now")).toHaveText(/80\s*Player TR now/)
  await expect(page.getByTestId("milestone-76")).toHaveText(
    /76\s*Graveler → Golem · Golbat → Crobat/,
  )
  await expect(page.getByTestId("milestone-76").locator("+ li")).toHaveAttribute(
    "data-testid",
    "milestone-now",
  )
  await page.getByLabel("Player TR", { exact: true }).fill("57")
  await expect(page.getByTestId("milestone-57")).toHaveAttribute("aria-current", "step")
  await expect(page.getByTestId("milestone-now")).toHaveCount(0)

  const chart = page.getByTestId("level-chart")
  await expect(chart).toBeVisible()
  await expect(chart).toContainText("Brock’s team level")
  await expect(chart).toContainText("Level cap")
  await expect(chart.getByText("Player TR (world progress)", { exact: true })).toBeVisible()
  await expect(chart.getByText("Player TR 57", { exact: true })).toBeVisible()
  for (const id of ["chart-team", "chart-cap"])
    expect(await page.getByTestId(id).getAttribute("d")).toMatch(
      /^M[\d.]+,[\d.]+(L[\d.]+,[\d.]+){200}$/,
    )
  // Keyboard and pointer both read values off the chart.
  const reader = chart.getByRole("slider")
  await reader.focus()
  await expect(page.getByTestId("chart-tooltip")).toContainText("Player TR 57")
  await expect(page.getByTestId("chart-tooltip")).toContainText("Lv 35 team level (TR 52)")
  await page.keyboard.press("ArrowRight")
  await expect(reader).toHaveAttribute(
    "aria-valuetext",
    /^Player TR 58: team level Lv 35, level cap Lv 38$/,
  )
  await reader.blur()
  await reader.hover()
  await expect(page.getByTestId("chart-tooltip")).toContainText(/Player TR \d+/)
  await page.getByRole("button", { name: "Blue Kanto · Champion", exact: true }).click()
  await expect(chart).toContainText("Blue’s team level")
  await expect(timeline.last()).toHaveText(/160\s*peak TR 170/)
  await expect(page.getByTestId("milestone-28")).toHaveText(
    /28\s*team level Lv 25 passes the level cap Lv 24/,
  )
  expect(errors).toEqual([])
})

test("grows Blue with the Rival scaler and shows each trainer's TR at the checkpoints", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  await page.getByRole("button", { name: "Blue Kanto · Champion", exact: true }).click()
  await expect(page.getByTestId("growth-blue")).toHaveText("0 → 170")
  await expect(page.getByTestId("archetype-blue")).toHaveText("Rival")
  // The Pallet fight: TR 0, one Eevee at Lv 5 (his signature Umbreon steps down).
  await expect(page.getByTestId("selected-tr")).toHaveText("0")
  await expect(page.getByTestId("team-size")).toHaveText("1")
  await expect(page.getByTestId("battle-order").locator(":scope > li")).toHaveText([
    /Eevee→ Umbreon at Lv 30.*Lv\. 5/s,
  ])
  // Umbreon is a final stage, so there is no final-stage warning.
  await expect(page.getByTestId("stage-warning-1")).toHaveCount(0)
  await expect(page.getByLabel("Lead", { exact: true })).toHaveCount(0)
  await expect(page.getByTestId("growth-table").locator("tbody tr").first()).toHaveText(
    /TR\s*0\s*49\s*90\s*129\s*170/,
  )
  await page.getByRole("button", { name: "Set 16 badges", exact: true }).click()
  await expect(page.getByTestId("selected-tr")).toHaveText("129")
  // The Rival growth scaler is editable like any other: 80% at world progress 120 is TR 136.
  await page.getByText("Scalers & experiment settings", { exact: true }).click()
  await page.getByLabel("Rival growth anchor 5 value", { exact: true }).fill("80")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByTestId("selected-tr")).toHaveText("136")
  await setGrowth(page, { start: "10", archetype: "steady", peak: "180" })
  await expect(page.getByTestId("growth-blue")).toHaveText("10 → 180")
  // A Steady at world progress 120: 10 + 75% of 170 = 137.5, halves round up.
  await expect(page.getByTestId("selected-tr")).toHaveText("138")
})

test("ranks the Gym Leaders against the player TR in the Gym ladder", async ({ page }) => {
  await page.goto("/#trainer-balance")
  const ladder = page.getByTestId("gym-ladder").locator(":scope > li")
  await expect(ladder).toHaveCount(24)
  await expect(page.getByTestId("ladder-tate-liza")).toContainText("Tate & Liza · double battle")
  // At world progress 0 every Gym Leader starts in the 18–40 Gym band, so all are above the
  // player: the Sleeper Winona lowest (TR 18), the Comeback Pryce highest (TR 40).
  await expect(page.getByTestId("ladder-counts")).toHaveText("0 below · 0 near · 24 above")
  await expect(ladder.first()).toContainText("Winona")
  await expect(ladder.first()).toContainText("above +18")
  await expect(page.getByTestId("ladder-brock")).toContainText("TR 25")
  await expect(page.getByTestId("ladder-brock")).toContainText("above +25")
  await expect(ladder.last()).toContainText("Pryce")
  await expect(ladder.last()).toContainText("above +40")
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  // Stars and Comebacks sit well below at 8 badges; Wattson, Janine and Erika are near.
  await expect(page.getByTestId("ladder-counts")).toHaveText("16 below · 3 near · 5 above")
  await expect(ladder.last()).toContainText("Tate & Liza")
  await expect(page.getByTestId("ladder-brock")).toContainText("below -17")
  await page.getByRole("button", { name: "Set 24 badges", exact: true }).click()
  await expect(page.getByTestId("ladder-counts")).toHaveText("14 below · 5 near · 5 above")
  await expect(ladder.last()).toContainText("Juan")
  await expect(ladder.last()).toContainText("TR 185")
})

test("ranks every eligible trainer by league score and fields the top five, strongest last", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  const names = async () =>
    page.getByTestId("event-lineup").locator(":scope > li h3").allInnerTexts()
  const entrant = (id: string) => page.getByTestId(`event-entrant-${id}`).locator("td")
  // Every catalog trainer but the Tate & Liza duo is league-eligible.
  const eligible = 37
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  // Day 0 is Indigo's event at the player's world progress, with no earlier event to fatigue anyone.
  await expect(page.getByTestId("event").locator("h3").first()).toContainText(
    "Day 0: Indigo world progress 80 · Kanto + Johto · base lineup Lv 59 · fatigued from no earlier event",
  )
  await expect(page.getByTestId("event-lineup").locator(":scope > li")).toHaveCount(5)
  await expect(page.getByTestId("event-entrants").locator("tbody tr")).toHaveCount(eligible)
  await expect(
    page.getByTestId("event-entrants").locator("tbody tr", { hasText: "in lineup" }),
  ).toHaveCount(5)
  await expect(page.getByLabel("League seed", { exact: true })).toHaveCount(0)
  // Indigo fights by ascending TR, strongest last (equal TRs in catalog order).
  await expect
    .poll(names)
    .toEqual([
      "Match 1 Will",
      "Match 2 Lt. Surge",
      "Match 3 Giovanni",
      "Match 4 Agatha",
      "Match 5 Jasmine",
    ])
  await expect(page.getByTestId("event-match-5")).toContainText("TR 95 · score 95 · Lv. 59")
  await expect(page.getByTestId("league-finalist")).toHaveText("Jasmine 95")
  // League score = floor(TR × willingness / 100). Norman (Hoenn, not a traveller) is away at
  // Indigo and pays 80; Drake (Hoenn, traveller) pays only 10.
  await expect(entrant("norman")).toHaveText([
    "27",
    "Norman",
    "95",
    "59",
    "Hoenn",
    "",
    "away",
    "80",
    "0",
    "20",
    "19",
    "",
    "",
  ])
  await expect(entrant("drake")).toHaveText([
    "12",
    "Drake",
    "93",
    "58",
    "Hoenn",
    "traveller",
    "away",
    "10",
    "0",
    "90",
    "83",
    "",
    "",
  ])
  // Aloof: the top five non-aloof are the base lineup (base lineup Lv 59). Agatha (Lv 59) joins;
  // Lance, a Legend at TR 200 (Lv 100), skips and is not ranked.
  await expect(page.getByTestId("event-aloof-agatha")).toHaveText(
    "Agatha aloof: team Lv 59 vs base lineup Lv 59 + 10 → joins",
  )
  await expect(page.getByTestId("event-aloof-lance")).toHaveText(
    "Lance aloof: team Lv 100 vs base lineup Lv 59 + 10 → skips",
  )
  await expect(page.getByTestId("event-aloof").locator("li")).toHaveCount(8)
  await expect(entrant("lance")).toHaveText([
    "—",
    "Lance",
    "200",
    "100",
    "Kanto",
    "traveller",
    "at home",
    "0",
    "0",
    "100",
    "200",
    "skips",
    "",
  ])
  await expect(entrant("jasmine").last()).toHaveText("in lineup")

  // The calendar: 12 days by default, one event a day, staggered Indigo, Hoenn, Sevii Masters.
  const calendar = page.getByTestId("calendar").locator("tbody tr")
  await expect(calendar).toHaveCount(12)
  await expect(page.getByTestId("event-0")).toContainText("Indigo")
  await expect(page.getByTestId("event-1")).toContainText("Hoenn")
  await expect(page.getByTestId("event-2")).toContainText("Sevii Masters")
  await expect(page.getByTestId("event-3")).toContainText("Indigo")
  // At 8 badges Indigo and Hoenn are open; the Masters needs 16.
  await expect(page.getByTestId("event-0-entry")).toHaveText("may enter")
  await expect(page.getByTestId("event-1-entry")).toHaveText("may enter")
  await expect(page.getByTestId("event-2-entry")).toHaveText("needs 16 badges")
  await expect(page.getByLabel("Won day 2", { exact: true })).toBeDisabled()
  // Skipping: the lineup's strongest reigns until that league's next event.
  await expect(page.getByTestId("event-0-champion")).toHaveText("Jasmine (TR 95)")
  await expect(page.getByTestId("event-1-champion")).toHaveText("Norman (TR 95)")
  await expect(page.getByTestId("champion-indigo")).toHaveText(
    "Indigo reigning champion after day 11: Karen (TR 93)",
  )
  // Fatigue: day 1 (Hoenn) reads day 0's Indigo lineup. Giovanni, a Kanto traveller who fought
  // at Indigo, pays 10 travel and 50 fatigue: willingness 40, score floor(95 × 40 / 100) = 38.
  await page.getByRole("button", { name: "Show day 1", exact: true }).click()
  await expect(page.getByTestId("event").locator("h3").first()).toContainText(
    "Day 1: Hoenn world progress 80 · Hoenn · base lineup Lv 59 · fatigued from day 0 Indigo",
  )
  await expect(entrant("giovanni").nth(8)).toHaveText("50")
  await expect(entrant("giovanni").nth(9)).toHaveText("40")
  await expect(entrant("giovanni").nth(10)).toHaveText("38")
  await expect(entrant("jasmine").nth(8)).toHaveText("50")
  await expect(entrant("norman").nth(8)).toHaveText("0")
  await page.getByRole("button", { name: "Show day 0", exact: true }).click()

  // Norman as a traveller pays only 10 away from Hoenn: floor(95 × 90 / 100) = 85.
  await page.getByRole("button", { name: /^Norman/ }).click()
  await expect(page.getByLabel("Home region", { exact: true })).toHaveValue("Hoenn")
  await expect(page.getByLabel("Traveller", { exact: true })).not.toBeChecked()
  await page.getByLabel("Traveller", { exact: true }).check()
  await expect(entrant("norman").nth(5)).toHaveText("traveller")
  await expect(entrant("norman").nth(9)).toHaveText("90")
  await expect(entrant("norman").nth(10)).toHaveText("85")
  // At home in Johto he scores his full 95 and takes the fifth place from Will.
  await page.getByLabel("Home region", { exact: true }).selectOption("Johto")
  await expect(entrant("norman").nth(6)).toHaveText("at home")
  await expect(entrant("norman").nth(10)).toHaveText("95")
  await expect(entrant("will").last()).toHaveText("")
  await expect(page.getByTestId("event-lineup")).toContainText("Norman")
  // The trainer editor carries the aloof trait: Lance is aloof; unticked, he joins Indigo and
  // fights last there.
  await page.getByRole("button", { name: /^Lance/ }).click()
  await expect(page.getByLabel("Aloof", { exact: true })).toBeChecked()
  await page.getByLabel("Aloof", { exact: true }).uncheck()
  await expect(page.getByTestId("event-aloof-lance")).toHaveCount(0)
  await expect(page.getByTestId("event-match-5")).toContainText("Lance")
  await expect(entrant("lance").first()).toHaveText("1")
  await page.getByLabel("Aloof", { exact: true }).check()
  await expect(page.getByTestId("event-aloof-lance")).toContainText("skips")
  // Tate & Liza fight a double battle, so even far ahead they are never ranked.
  await page.getByRole("button", { name: /^Tate & Liza/ }).click()
  await expect(page.getByTestId("double-battle")).toHaveText("Double battle")
  await expect(page.getByTestId("double-battle-note")).toContainText("not in the league pool")
  await setGrowth(page, { archetype: "veteran", peak: "300" })
  await expect(page.getByTestId("tr-tate-liza")).toHaveText("300")
  await expect(page.getByTestId("event-entrants")).not.toContainText("Tate")
})

test("simulates the league calendar: entry rules, wins, reigning champions and fatigue", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  // With no badges, no league is open and every event is the lineup's to win.
  await expect(page.getByTestId("event-0-entry")).toHaveText("needs 8 badges")
  await expect(page.getByLabel("Won day 0", { exact: true })).toBeDisabled()
  await page.getByRole("button", { name: "Set 16 badges", exact: true }).click()
  // 16 badges open the Masters only after a win at Indigo or Hoenn.
  await expect(page.getByTestId("event-2-entry")).toHaveText("needs an Indigo or Hoenn win")
  await expect(page.getByLabel("Won day 2", { exact: true })).toBeDisabled()
  await expect(page.getByTestId("event-0-champion")).toHaveText("Jasmine (TR 131)")
  await page.getByLabel("Won day 0", { exact: true }).check()
  await expect(page.getByTestId("event-0")).toContainText("first win")
  await expect(page.getByTestId("event-0-champion")).toHaveText("the player")
  await expect(page.getByTestId("event-2-entry")).toHaveText("may enter")
  // A win changes no lineup: day 1 still fatigues day 0's five.
  await expect(page.getByTestId("event-1")).toContainText("Winona, Juan, Phoebe, Koga, Norman")
  // A repeat win at Indigo keeps the title with the player until Indigo's next event.
  await page.getByLabel("Won day 3", { exact: true }).check()
  await expect(page.getByTestId("event-3")).toContainText("repeat win")
  await page.getByLabel("Calendar days", { exact: true }).fill("6")
  await page.getByLabel("Calendar days", { exact: true }).press("Enter")
  await expect(page.getByTestId("calendar").locator("tbody tr")).toHaveCount(6)
  await expect(page.getByTestId("champion-indigo")).toHaveText(
    "Indigo reigning champion after day 5: the player",
  )
  await expect(page.getByTestId("champion-hoenn")).toHaveText(
    "Hoenn reigning champion after day 5: Giovanni (TR 131)",
  )
  await expect(page.getByTestId("champion-sevii-masters")).toHaveText(
    "Sevii Masters reigning champion after day 5: Jasmine (TR 131)",
  )
  // The calendar length and the wins persist; the same inputs give the same calendar.
  await page.reload()
  await expect(page.getByTestId("calendar").locator("tbody tr")).toHaveCount(6)
  await expect(page.getByLabel("Won day 0", { exact: true })).toBeChecked()
  await expect(page.getByLabel("Won day 3", { exact: true })).toBeChecked()
  await expect(page.getByTestId("event-4-champion")).toHaveText("Giovanni (TR 131)")
  // They round-trip through export.
  const downloadPromise = page.waitForEvent("download")
  await page.getByRole("button", { name: "Export experiment", exact: true }).click()
  const exported = await (await downloadPromise).path()
  if (!exported) throw new Error("Export did not produce a file")
  const saved = JSON.parse(await readFile(exported, "utf8"))
  expect(saved.version).toBe(18)
  expect(saved.league).toEqual({ days: 6, wins: [0, 3] })
  // All 24 badges: the aloof Champions join the elite base lineup, and Lance reigns at Indigo.
  await page.getByRole("button", { name: "Set 24 badges", exact: true }).click()
  await page.getByLabel("Won day 0", { exact: true }).uncheck()
  await page.getByLabel("Won day 3", { exact: true }).uncheck()
  await expect(page.getByTestId("event-0-champion")).toHaveText("Lance (TR 200)")
  await expect(page.getByTestId("event-aloof-lance")).toHaveText(
    "Lance aloof: team Lv 100 vs base lineup Lv 100 + 10 → joins",
  )
  await expect(page.getByTestId("event-2-entry")).toHaveText("needs an Indigo or Hoenn win")
})

test("shows each trainer's play style and resolved AI flags, and edits the style", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  const flags = page.getByTestId("ai-flags").locator("li")
  const style = page.getByLabel("Play style", { exact: true })
  // Brock at world progress 0: TR 25 (AI skill None), a Field marshal, one ace in a team of two.
  await expect(style).toHaveValue("field_marshal")
  await expect(style.locator("option")).toHaveText([
    "Gambler",
    "Bomber",
    "Sweeper",
    "Field marshal",
    "Hexer",
    "Turtle",
    "Brawler",
    "Tactician",
  ])
  await expect(page.getByTestId("play-style-description")).toContainText(
    "Adds Powerful Status on top of Basic.",
  )
  await expect(page.getByTestId("ai-skill")).toHaveText(
    "AI skill None (tier 0, TR 0+) · 1 ace in the team",
  )
  await expect(flags).toHaveText([
    "Check Bad Move",
    "Try To Faint",
    "Check Viability",
    "Powerful Status",
    "Ace Pokemon",
  ])
  // At 8 badges Brock is TR 63: the Aware tier adds Smart Mon Choices and Assume STAB.
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("selected-tr")).toHaveText("63")
  await expect(page.getByTestId("ai-skill")).toHaveText(
    "AI skill Aware (tier 1, TR 30+) · 1 ace in the team",
  )
  await expect(flags).toHaveText([
    "Check Bad Move",
    "Try To Faint",
    "Check Viability",
    "Powerful Status",
    "Ace Pokemon",
    "Smart Mon Choices",
    "Assume STAB",
  ])
  // Lance, the boss: a Tactician at TR 200 with three aces (Double Ace protects the last two).
  await page.getByRole("button", { name: /^Lance/ }).click()
  await expect(style).toHaveValue("tactician")
  await expect(page.getByTestId("ai-skill")).toHaveText(
    "AI skill Predictive (tier 3, TR 110+) · 3 aces in the team · boss",
  )
  await expect(flags).toHaveText([
    "Check Bad Move",
    "Try To Faint",
    "Check Viability",
    "HP Aware",
    "Smart Switching",
    "Omniscient",
    "Smart Mon Choices",
    "Double Ace Pokemon",
    "Weigh Ability Prediction",
    "Predict Switch",
    "Predict Incoming Mon",
    "Predict Move",
    "Assume STAB",
    "Assume Status Moves",
  ])
  // Wattson the Bomber, edited to a Turtle: Risky and Will Suicide give way to Conservative.
  await page.getByRole("button", { name: /^Wattson/ }).click()
  await expect(style).toHaveValue("bomber")
  await expect(flags.filter({ hasText: "Will Suicide" })).toHaveCount(1)
  await style.selectOption("turtle")
  await expect(page.getByRole("status")).toContainText("Wattson is now a Turtle.")
  await expect(flags.filter({ hasText: "Conservative" })).toHaveCount(1)
  await expect(flags.filter({ hasText: /^Risky$/ })).toHaveCount(0)
  await expect(flags.filter({ hasText: "Will Suicide" })).toHaveCount(0)
  await page.reload()
  await page.getByRole("button", { name: /^Wattson/ }).click()
  await expect(style).toHaveValue("turtle")
})

test("marks ace slots and fights them last, with slot 1 locked and at most three aces", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  const order = page.getByTestId("battle-order").locator(":scope > li")
  await expect(page.getByTestId("order-rule")).toContainText("Join order is list order")
  await expect(page.getByTestId("order-rule")).toContainText("1–3 aces (2 now)")
  // Brock at TR 100 fields all six: Steelix (ace), Golem, Crobat, Kabutops, Omastar,
  // Aerodactyl (ace). Fillers fight first, aces last, each in reverse list order.
  await setGrowth(page, { start: "100", peak: "100" })
  await expect(page.getByTestId("team-size")).toHaveText("6")
  await expect(order).toHaveText([
    /^#5\s*Omastar(?!Ace)/,
    /^#4\s*Kabutops(?!Ace)/,
    /^#3\s*Crobat(?!Ace)/,
    /^#2\s*Golem(?!Ace)/,
    /^#6\s*AerodactylAce/,
    /^#1\s*SteelixAce/,
  ])
  await expect(page.getByRole("group", { name: "Roster slot 6 · ace · in team" })).toBeVisible()
  const ace = (slot: number) => page.getByLabel(`Roster slot ${slot} ace`, { exact: true })
  await expect(ace(1)).toBeChecked()
  await expect(ace(1)).toBeDisabled()
  // Kabutops becomes the third ace; a fourth ace is refused with a clear message.
  await ace(4).click()
  await expect(page.getByTestId("order-rule")).toContainText("(3 now)")
  await ace(5).click()
  await expect(page.getByRole("alert")).toContainText(
    "A roster has at most 3 aces (roster slot 1 plus two more). Clear another ace first.",
  )
  await expect(ace(5)).not.toBeChecked()
  // Aerodactyl becomes a filler slot and Omastar an ace.
  await ace(6).click()
  await expect(page.getByRole("status")).toContainText(
    "Aerodactyl (roster slot 6) is now a filler slot.",
  )
  await ace(5).click()
  await expect(page.getByTestId("order-rule")).toContainText("(3 now)")
  await expect(order).toHaveText([
    /^#6\s*Aerodactyl(?!Ace)/,
    /^#3\s*Crobat/,
    /^#2\s*Golem/,
    /^#5\s*OmastarAce/,
    /^#4\s*KabutopsAce/,
    /^#1\s*SteelixAce/,
  ])
  await expect(page.getByTestId("growth-slot-4")).toHaveText(/^Slot 4\s*Ace/)
  await expect(page.getByTestId("growth-slot-6")).not.toContainText("Ace")
})

test("flags incomplete rosters and rejects invalid edits and version 4 to 17 files", async ({
  page,
}) => {
  await page.goto("/#trainer-balance")
  // Every catalog roster lists six Pokémon.
  await expect(page.getByTestId("roster-gap-count")).toHaveText("0")
  await expect(page.getByTestId("roster-gap-list")).toHaveText("Every roster lists 6 Pokémon")
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
  // A Legend never grows, and no Gym Leader is a Legend.
  await setGrowth(page, { start: "25", archetype: "legend", peak: "100" })
  await expect(page.getByRole("alert")).toContainText(
    "A Legend never grows: set peak TR equal to start TR.",
  )
  await setGrowth(page, { start: "25", archetype: "legend", peak: "25" })
  await expect(page.getByRole("alert")).toContainText("brock: a Gym Leader cannot be a Legend")
  await expect(page.getByTestId("selected-tr")).toHaveText("25")
  await expect(page.getByTestId("archetype-brock")).toHaveText("Steady")

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
    "Version 7 experiments give the Rival a fixed lead",
  )
  await importFile(8, {
    point: { playerTR: 80 },
    experiment: {
      version: 8,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: {},
      trainers: {},
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 8 experiments have no ace slots (isAce)",
  )
  await importFile(9, {
    point: { playerTR: 80 },
    experiment: {
      version: 9,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: {},
      trainers: {},
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 9 experiments have only five archetypes",
  )
  await importFile(10, {
    point: { playerTR: 80 },
    experiment: {
      version: 10,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: { "early bloomer": [[0, 0]] },
      trainers: {},
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 10 experiments use the old archetype names",
  )
  await importFile(11, {
    point: { playerTR: 80 },
    experiment: {
      version: 11,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: {},
      trainers: {},
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 11 experiments author moves per roster slot and have no move pools",
  )
  await importFile(12, {
    point: { playerTR: 80 },
    experiment: {
      version: 12,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: {},
      trainers: {},
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 12 experiments have no home regions or traveller trait",
  )
  await importFile(13, {
    point: { playerTR: 80 },
    league: { seed: 1, at: "badges" },
    experiment: {
      version: 13,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: {},
      trainers: {},
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 13 experiments save a league seed for the retired seeded lineup draw",
  )
  await importFile(14, {
    point: { playerTR: 80 },
    league: { at: "badges" },
    experiment: {
      version: 14,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: {},
      trainers: {},
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 14 experiments have no aloof trait and the old Sleeper Champions",
  )
  await importFile(15, {
    point: { playerTR: 80 },
    league: { at: "badges" },
    experiment: {
      version: 15,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: {},
      trainers: {},
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 15 experiments save a travel style instead of the traveller trait",
  )
  await importFile(16, {
    point: { playerTR: 80 },
    league: { at: "badges" },
    experiment: {
      version: 16,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: {},
      trainers: {},
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 16 experiments have no play styles (Trainer AI)",
  )
  await importFile(17, {
    point: { playerTR: 80 },
    league: { at: "badges" },
    experiment: {
      version: 17,
      teamLevel: [[0, 15]],
      teamSize: [[0, 2]],
      wildLevel: [[0, 6]],
      routeTrainerLevel: [[0, 9]],
      archetypes: {},
      trainers: {},
    },
  })
  await expect(page.getByRole("alert")).toContainText(
    "Version 17 experiments save a league entry point for the retired standard entry sequence",
  )
  await expect(page.getByTestId("badge-count")).toHaveText("0")
  await expect(page.getByTestId("selected-tr")).toHaveText("25")

  await page.getByRole("button", { name: "Lorelei Kanto · Elite Four", exact: true }).click()
  await page.getByRole("button", { name: "Remove roster slot 6", exact: true }).click()
  await expect(page.getByTestId("roster-gap-count")).toHaveText("1")
  await expect(page.getByTestId("roster-incomplete")).toBeVisible()
  await expect(page.getByTestId("roster-length")).toHaveText("5 / 6")
  await expect(page.getByTestId("warnings")).toContainText("lists 5 of 6 Pokémon")
  await page.getByRole("button", { name: "Add roster slot", exact: true }).click()
  await page.getByLabel("Roster slot 6 species", { exact: true }).fill("Articuno")
  await page.getByRole("button", { name: "Apply roster", exact: true }).click()
  await expect(page.getByTestId("stage-warning-6")).toHaveText(
    "No evolution data for Articuno in the catalog, so it never steps down.",
  )
  await expect(page.getByTestId("roster-length")).toHaveText("6 / 6")
  await expect(page.getByTestId("roster-incomplete")).toHaveCount(0)
  await expect(page.getByTestId("roster-gap-count")).toHaveText("0")
  await expect(page.getByRole("button", { name: "Add roster slot", exact: true })).toHaveCount(0)
})

test("resolves each member's moves from the move pool, lists dormant entries and edits the pool", async ({
  page,
}) => {
  const errors: string[] = []
  page.on("pageerror", (error) => errors.push(error.message))
  await page.goto("/#trainer-balance")
  const moves = (slot: number) => page.getByTestId(`moves-${slot}`).locator("li")
  // Brock at world progress 0. An entry without a from level goes only to a member whose current
  // species or an earlier form learns it by level-up, from the learn level: the Onix ace takes Bind
  // (a Lv 1 level-up move) and Curse and claims Stealth Rock, which it already knows; Geodude is
  // below its Earthquake learn level, so it keeps its level-up moves.
  await expect(moves(1)).toHaveText([
    /^Bind\s*pool$/,
    /^Curse\s*pool$/,
    /^Rage\s*level-up$/,
    /^Stealth Rock\s*pool$/,
  ])
  await expect(moves(2)).toHaveText([
    /^Rollout\s*level-up$/,
    /^Magnitude\s*level-up$/,
    /^Strength\s*level-up$/,
    /^Rock Throw\s*level-up$/,
  ])
  await expect(page.getByTestId("pool-source")).toContainText(
    "user-directed pool draft v1: hazards and sand walls",
  )
  await expect(page.getByTestId("pool-status-1")).toHaveText("→ #1 Onix")
  await expect(page.getByTestId("pool-status-2")).toHaveText("→ #1 Onix")
  await expect(page.getByTestId("pool-status-4")).toHaveText("→ #1 Onix")
  await expect(page.getByTestId("pool-status-3")).toHaveText("dormant")
  await expect(page.getByTestId("dormant-3")).toHaveText(
    /Sandstorm\s*from Lv 20\s*below from level Lv 20/,
  )
  await expect(page.getByTestId("dormant-6")).toHaveText(/Earthquake\s*below its learn level Lv 34/)
  // Golem learns Heavy Slam by level-up, but Geodude is not Golem yet; for Onix it is an egg move.
  await expect(page.getByTestId("dormant-8")).toHaveText(
    /Heavy Slam\s*egg move: needs a from level/,
  )
  // The pool editor names the earliest level-up learner (an earlier form counts), or says the
  // entry needs a from level.
  await expect(page.getByTestId("pool-learning-6")).toHaveText(
    "Level-up: Golem, earlier form (Geodude) at Lv 34",
  )
  await expect(page.getByTestId("pool-learning-3")).toHaveText(
    "Any learner from Lv 20 (level-up: Steelix at Lv 52)",
  )
  await expect(page.getByTestId("milestone-6")).toContainText("Sandstorm wakes (Onix)")
  // Graveler learns Earthquake through its earlier form Geodude (Lv 34).
  await expect(page.getByTestId("milestone-61")).toContainText("Earthquake wakes (Graveler)")

  // A from level lets a TM learner take an entry, and holds an entry back until it.
  await page.getByLabel("Pool entry 6 from level", { exact: true }).fill("16")
  await page.getByLabel("Pool entry 4 from level", { exact: true }).fill("20")
  await page.getByRole("button", { name: "Apply move pool", exact: true }).click()
  await expect(page.getByRole("status")).toContainText("Updated Brock’s move pool.")
  await expect(page.getByTestId("pool-status-6")).toHaveText("→ #1 Onix")
  await expect(page.getByTestId("pool-learning-6")).toHaveText(
    "Any learner from Lv 16 (level-up: Golem, earlier form (Geodude) at Lv 34)",
  )
  await expect(page.getByTestId("dormant-4")).toHaveText(
    /Curse\s*from Lv 20\s*below from level Lv 20/,
  )

  // Unknown names are refused; names match regardless of case.
  await page.getByLabel("Pool entry 1 move", { exact: true }).fill("Earthshake")
  await page.getByRole("button", { name: "Apply move pool", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText(
    "Pool entry 1: “Earthshake” is not a move in the game data.",
  )
  await page.getByLabel("New pool move", { exact: true }).fill("toxic")
  await page.getByRole("button", { name: "Add to pool", exact: true }).click()
  await expect(page.getByRole("status")).toContainText("Added Toxic as pool entry 12.")
  await expect(page.getByTestId("pool-learning-12")).toHaveText(
    "TM/tutor only — needs a from level",
  )
  await page.getByLabel("New pool move", { exact: true }).fill("iron defense")
  await page.getByLabel("New pool from level", { exact: true }).fill("40")
  await page.getByRole("button", { name: "Add to pool", exact: true }).click()
  await expect(page.getByRole("status")).toContainText("Added Iron Defense as pool entry 13.")
  await expect(page.getByLabel("Pool entry 13 move", { exact: true })).toHaveValue("Iron Defense")
  await expect(page.getByLabel("Pool entry 13 from level", { exact: true })).toHaveValue("40")
  await expect(page.getByTestId("pool-learning-13")).toHaveText("No roster line learns it")
  await expect(page.getByLabel("New pool move", { exact: true })).toHaveValue("")
  // Spikes is only an egg move on Brock's lines, so it needs a from level too.
  await page.getByLabel("New pool move", { exact: true }).fill("spikes")
  await page.getByRole("button", { name: "Add to pool", exact: true }).click()
  await expect(page.getByTestId("pool-learning-14")).toHaveText("Egg move: needs a from level")
  await page.getByLabel("Pool entry 14 from level", { exact: true }).fill("20")
  await page.getByRole("button", { name: "Apply move pool", exact: true }).click()
  await expect(page.getByTestId("pool-learning-14")).toHaveText("Egg move: from Lv 20")

  // Pool order is identity: reorder and remove entries.
  await page.getByRole("button", { name: "Move pool entry 3 up", exact: true }).click()
  await expect(page.getByLabel("Pool entry 2 move", { exact: true })).toHaveValue("Sandstorm")
  await page.getByRole("button", { name: "Remove pool entry 2", exact: true }).click()
  await expect(page.getByLabel("Pool entry 2 move", { exact: true })).toHaveValue("Stealth Rock")
  await expect(page.getByLabel("Pool entry 12 move", { exact: true })).toHaveValue("Iron Defense")
  await page.reload()
  await expect(page.getByLabel("Pool entry 12 move", { exact: true })).toHaveValue("Iron Defense")
  await expect(page.getByLabel("Pool entry 5 from level", { exact: true })).toHaveValue("16")
  await expect(page.getByLabel("Pool entry 3 from level", { exact: true })).toHaveValue("20")
  expect(errors).toEqual([])
})

test("edits the trainer, world and archetype scalers globally", async ({ page }) => {
  await page.goto("/#trainer-balance")
  await page.getByText("Scalers & experiment settings", { exact: true }).click()
  // Wallace is TR 48 at world progress 0.
  await expect(page.getByTestId("size-wallace")).toHaveText("4")
  // Without the (44, 4) anchor, size ramps from (43, 3) to (56, 4): TR 48 rounds to 3.
  await page.getByRole("button", { name: "Remove Team size anchor 7", exact: true }).click()
  await expect(page.getByTestId("size-wallace")).toHaveText("3")
  await page.getByLabel("Team level anchor 1 value", { exact: true }).fill("30")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText("values must not decrease")
  // Winona is TR 18: from (0, 5) to (20, 14) that is Lv. 13.1; from (0, 10) it is Lv. 13.6.
  await expect(page.getByTestId("level-winona")).toHaveText("Lv. 13")
  await page.getByLabel("Team level anchor 1 value", { exact: true }).fill("10")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByTestId("level-winona")).toHaveText("Lv. 14")

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

  // Archetype growth scalers read world progress: the Sleeper Juan (23 → 185) at 8 badges; the
  // Legend Lance stays at TR 200.
  await page.getByRole("button", { name: "Set 8 badges", exact: true }).click()
  await expect(page.getByTestId("tr-juan")).toHaveText("64")
  await expect(page.getByTestId("tr-lance")).toHaveText("200")
  await page.getByLabel("Sleeper growth anchor 3 value", { exact: true }).fill("50")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByTestId("tr-juan")).toHaveText("104")
  await page.getByLabel("Steady growth anchor 1 value", { exact: true }).fill("5")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText(
    "steady growth must be 0% at world progress 0",
  )
  await page.getByLabel("Steady growth anchor 1 value", { exact: true }).fill("0")

  // Every scaler shows its kind; Burst is the only step scaler. The Burst Giovanni (24 → 166)
  // holds 25% (TR 60) from 4 badges until he jumps to 50% (TR 95) at 8 badges.
  await expect(page.getByTestId("scaler-kind-burst")).toHaveText("step")
  for (const id of ["steady", "legend", "star", "comeback", "teamSize"])
    await expect(page.getByTestId(`scaler-kind-${id}`)).toHaveText("interpolated")
  await expect(page.getByTestId("tr-giovanni")).toHaveText("95")
  await page.getByLabel("Player TR", { exact: true }).fill("79")
  await expect(page.getByTestId("tr-giovanni")).toHaveText("60")
  // Editing the step scaler moves the held value: 40% from world progress 40 is TR 81.
  await page.getByLabel("Burst growth anchor 2 value", { exact: true }).fill("40")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByTestId("tr-giovanni")).toHaveText("81")
  await page.getByLabel("Burst growth anchor 3 value", { exact: true }).fill("30")
  await page.getByRole("button", { name: "Apply scalers", exact: true }).click()
  await expect(page.getByRole("alert")).toContainText("burst growth values must not decrease")

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
  await expect(page.getByTestId("calendar")).toBeVisible()
  await expect(page.getByTestId("event-lineup")).toBeVisible()
  await expect(page.getByTestId("event-entrants")).toBeVisible()
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
})

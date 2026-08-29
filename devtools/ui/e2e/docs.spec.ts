import { expect, test } from "webanvil/e2e"

test("shows product docs from the current checkout", async ({ page }) => {
  await page.goto("/#docs/research/story-blocking-traversal-audit.md?section=emerald")

  await expect(page.getByRole("link", { name: "Docs" })).toHaveAttribute("aria-current", "page")
  const navigation = page.getByRole("complementary", { name: "Product documents" })
  await expect(navigation).toBeVisible()
  await expect(navigation.getByRole("link", { name: "[Feature name]" })).toHaveCount(0)
  await expect(navigation.getByRole("link", { name: "[Spec name]" })).toHaveCount(0)
  await expect(
    page.getByRole("heading", { name: "Story-blocking traversal audit", exact: true }),
  ).toBeVisible()
  await expect(page.getByRole("heading", { name: "Emerald", exact: true })).toBeFocused()
  await expect(page.getByRole("table")).toHaveCount(8)
})

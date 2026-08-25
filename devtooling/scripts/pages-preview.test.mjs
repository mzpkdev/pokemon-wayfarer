import assert from "node:assert/strict"
import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"
import test from "node:test"

import {
  assertPagesSiteSize,
  pagesSiteLimitBytes,
  previewIndexHtml,
  previewNumbersFromTsv,
  writePreviewIndex,
} from "./pages-preview.mjs"

test("keeps only valid, sorted same-repository preview numbers", () => {
  assert.deepEqual(previewNumbersFromTsv("4\tabc\n12\tdef\n4\tghi\n"), [4, 12])
  assert.throws(() => previewNumbersFromTsv("../4\tsha\n"), /Invalid pull request number/)
})

test("writes a relative preview index without a production site copy", (t) => {
  const output = fs.mkdtempSync(path.join(os.tmpdir(), "wayfarer-pages-index-"))
  t.after(() => fs.rmSync(output, { recursive: true, force: true }))

  writePreviewIndex({ output, previews: [4, 12] })

  assert.equal(fs.readdirSync(output).join(","), "index.html")
  assert.match(fs.readFileSync(path.join(output, "index.html"), "utf8"), /href="preview\/pr-4\/"/)
  assert.match(previewIndexHtml([]), /No same-repository pull requests/)
})

test("fails before a GitHub Pages artifact exceeds the configured budget", (t) => {
  const output = fs.mkdtempSync(path.join(os.tmpdir(), "wayfarer-pages-size-"))
  t.after(() => fs.rmSync(output, { recursive: true, force: true }))
  fs.writeFileSync(path.join(output, "catalog.json"), "small")

  assert.equal(assertPagesSiteSize({ directory: output }), 5)
  assert.throws(
    () => assertPagesSiteSize({ directory: output, limitBytes: 4 }),
    /above the 4-byte deployment limit/,
  )
  assert.equal(pagesSiteLimitBytes, 950 * 1024 * 1024)
})

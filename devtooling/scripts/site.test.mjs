import assert from "node:assert/strict"
import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"
import test from "node:test"

import { cleanGeneratedArtifacts } from "./clean.mjs"
import { stageStaticSite } from "./site.mjs"

test("stages the UI beside a generated catalog without replacing its files", (t) => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "wayfarer-static-site-"))
  t.after(() => fs.rmSync(root, { recursive: true, force: true }))
  const source = path.join(root, "ui-dist")
  const output = path.join(root, "catalog")
  fs.mkdirSync(source, { recursive: true })
  fs.mkdirSync(output, { recursive: true })
  fs.writeFileSync(path.join(source, "index.html"), '<script src="./assets/app.js"></script>')
  fs.mkdirSync(path.join(source, "assets"))
  fs.writeFileSync(path.join(source, "assets/app.js"), "export {}")
  fs.writeFileSync(path.join(output, "catalog.json"), '{"schemaVersion":7}\n')

  const staged = stageStaticSite({ source, output })

  assert.equal(staged, output)
  assert.equal(fs.readFileSync(path.join(output, "catalog.json"), "utf8"), '{"schemaVersion":7}\n')
  assert.equal(fs.readFileSync(path.join(output, "index.html"), "utf8"), '<script src="./assets/app.js"></script>')
  assert.equal(fs.readFileSync(path.join(output, "assets/app.js"), "utf8"), "export {}")
})

test("clean removes the staged static site with its generated catalog", (t) => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "wayfarer-static-clean-"))
  t.after(() => fs.rmSync(root, { recursive: true, force: true }))
  const site = path.join(root, "build/cartographer/map-catalog")
  fs.mkdirSync(site, { recursive: true })
  fs.writeFileSync(path.join(site, "catalog.json"), "{}\n")
  fs.writeFileSync(path.join(site, "index.html"), "<!doctype html>")

  cleanGeneratedArtifacts({ root })

  assert.equal(fs.existsSync(path.join(root, "build/cartographer")), false)
})

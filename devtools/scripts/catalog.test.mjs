import assert from "node:assert/strict"
import test from "node:test"

import { assertCatalogPrerequisites } from "./catalog.mjs"

test("catalog prerequisites report a missing C preprocessor clearly", () => {
  const previous = process.env.CPP
  process.env.CPP = "wayfarer-deliberately-missing-cpp"
  try {
    assert.throws(assertCatalogPrerequisites, {
      message:
        'Cannot generate the devtools catalog: C preprocessor command "wayfarer-deliberately-missing-cpp" was not found. See devtools/README.md prerequisites.',
    })
  } finally {
    if (previous === undefined) delete process.env.CPP
    else process.env.CPP = previous
  }
})

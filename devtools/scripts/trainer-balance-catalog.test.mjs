import assert from "node:assert/strict"
import { spawnSync } from "node:child_process"
import { fileURLToPath } from "node:url"
import test from "node:test"

const script = fileURLToPath(new URL("./trainer-balance-catalog.py", import.meta.url))

// Loads the catalog script as a module, replaces its evolution-level table with
// `rows`, and validates it against the game's species_info evolutions.
function validateTable(rows) {
  const program = `
import importlib.util, json, sys
sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location("catalog", sys.argv[1])
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
if sys.argv[2] != "null":
    module.EVOLUTION_LEVELS = [tuple(row) for row in json.loads(sys.argv[2])]
edges, _ = module.load_evolutions()
try:
    print(len(module.evolution_table(edges)))
except ValueError as error:
    print(error)
    sys.exit(1)
`
  const table = rows === null ? "null" : JSON.stringify(rows)
  const result = spawnSync("python3", ["-c", program, script, table], { encoding: "utf8" })
  if (result.error) throw result.error
  return { ok: result.status === 0, output: result.stdout.trim() || result.stderr.trim() }
}

test("the committed evolution-level table has no level evolutions", () => {
  const result = validateTable(null)
  assert.equal(result.ok, true, result.output)
})

test("the evolution-level table rejects an edge the game already levels", () => {
  assert.deepEqual(validateTable([["Graveler", "Golem", 40, "authored"]]), {
    ok: false,
    output:
      "evolution-level table Graveler->Golem: already a level evolution in species_info (Lv 38); the table covers only evolutions without a level, so remove the row",
  })
  assert.deepEqual(validateTable([["Onix", "Steelix", 35, "authored"]]), { ok: true, output: "1" })
})

// Loads the catalog script and runs its v0 roster validation on `roster`.
function validateRoster(roster) {
  const program = `
import importlib.util, json, sys
sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location("catalog", sys.argv[1])
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
try:
    module.validate_roster("Fixture", json.loads(sys.argv[2]))
    print("ok")
except ValueError as error:
    print(error)
`
  const result = spawnSync("python3", ["-c", program, script, JSON.stringify(roster)], {
    encoding: "utf8",
  })
  if (result.error) throw result.error
  return result.stdout.trim() || result.stderr.trim()
}

test("the roster validation requires roster slot 1 as an ace and at most three aces", () => {
  const slot = (isAce, levelOffset = -2) => ({
    species: "Onix",
    levelOffset,
    isAce,
    moves: "LEVEL_UP",
  })
  const roster = (...aces) =>
    [0, 1, 2, 3, 4, 5].map((index) => slot(index === 0 || aces.includes(index), index ? -2 : 0))
  assert.equal(validateRoster(roster(2, 5)), "ok")
  assert.equal(validateRoster(roster()), "ok")
  assert.equal(validateRoster(roster(2, 4, 5)), "Fixture: a roster has 1-3 aces")
  assert.equal(
    validateRoster([slot(false, 0), ...roster(2).slice(1)]),
    "Fixture: roster slot 1 (the signature Pokémon) must be an ace",
  )
  assert.equal(validateRoster([slot("yes", 0)]), "Fixture[1]: isAce must be true or false")
})

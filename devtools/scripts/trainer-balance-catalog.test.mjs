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

// Loads the catalog script and runs `call` (a Python expression over `module`
// and `args`), printing its JSON result or the ValueError message.
function run(call, args = null) {
  const program = `
import importlib.util, json, sys
sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location("catalog", sys.argv[1])
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
args = json.loads(sys.argv[2])
try:
    print(json.dumps(${call}))
except ValueError as error:
    print(error)
`
  const result = spawnSync("python3", ["-c", program, script, JSON.stringify(args)], {
    encoding: "utf8",
  })
  if (result.error) throw result.error
  const output = result.stdout.trim() || result.stderr.trim()
  try {
    return JSON.parse(output)
  } catch {
    return output
  }
}

test("the growth table applies the section 11 archetype reassignment", () => {
  const growth = run("{name: list(value) for name, value in module.GROWTH.items()}")
  const named = (archetype) =>
    Object.keys(growth).filter((name) => growth[name][1] === archetype)
  assert.deepEqual(named("star"), ["Misty", "Bugsy", "Whitney", "Flannery", "Tate & Liza"])
  assert.deepEqual(named("comeback"), ["Blaine", "Bruno", "Pryce"])
  assert.deepEqual(named("burst"), ["Giovanni", "Chuck", "Brawly"])
  assert.deepEqual(named("legend"), ["Agatha"])
  assert.equal(growth["Lt. Surge"][1], "veteran")
  assert.equal(growth["Morty"][1], "sleeper")
  assert.deepEqual(growth["Agatha"], [95, "legend", 95])
  assert.deepEqual(growth["Brock"], [25, "steady", 100])
  assert.deepEqual(growth["Blue"], [0, "rival", 170])
})

test("a Legend's peak TR must equal its start TR", () => {
  const validate = (growth) => run("module.validate_growth('Fixture', tuple(args))", growth)
  assert.equal(validate([95, "legend", 95]), null)
  assert.equal(validate([90, "legend", 95]), "Fixture: a Legend's peak TR must equal start TR")
  assert.equal(validate([10, "burst", 90]), null)
  assert.equal(validate([10, "sprint", 90]), "Fixture: unknown archetype 'sprint'")
})

test("Gym Leaders start in their archetype's sub-band and are never Legends", () => {
  const validate = (growth) => run("module.validate_gym_start('Fixture', tuple(args))", growth)
  for (const [archetype, low, high] of [
    ["sleeper", 18, 26],
    ["star", 18, 26],
    ["prodigy", 22, 30],
    ["steady", 24, 34],
    ["burst", 24, 34],
    ["veteran", 30, 40],
    ["comeback", 30, 40],
  ]) {
    assert.equal(validate([low, archetype, 100]), null)
    assert.equal(validate([high, archetype, 100]), null)
    const message = `Fixture: Gym Leader start TR must be in ${low}-${high} for ${archetype} (Gym band 18-40)`
    assert.equal(validate([low - 1, archetype, 100]), message)
    assert.equal(validate([high + 1, archetype, 100]), message)
  }
  assert.equal(validate([30, "legend", 30]), "Fixture: a Gym Leader cannot be a Legend")
  // The Rival has no sub-band, so only the whole Gym band applies.
  assert.equal(validate([18, "rival", 100]), null)
  assert.equal(
    validate([41, "rival", 100]),
    "Fixture: Gym Leader start TR must be in 18-40 for rival (Gym band 18-40)",
  )
})

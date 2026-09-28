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
  assert.equal(
    validateRoster([{ ...slot(true, 0), moves: "LEVEL_UP" }]),
    "Fixture[1]: roster slots carry no moves (they come from the move pool)",
  )
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

test("learnsets are read from the preprocessed game data for the Wayfarer build", () => {
  const data = run(
    `(lambda result: {"valid": len(result[0]), "aliases": result[1].get("MOVE_FAINT_ATTACK"),
      "tackle": "MOVE_TACKLE" in result[0], "zmove": "MOVE_BREAKNECK_BLITZ" in result[0],
      "learnsets": {token: {"levelUp": value["levelUp"][:4], "size": len(value["levelUp"]),
                            "earthquake": "MOVE_EARTHQUAKE" in value["teachable"],
                            "stealthRock": any(move == "MOVE_STEALTH_ROCK" for _, move in value["levelUp"])}
                    for token, value in result[2].items()}})(module.load_learnsets(args))`,
    ["SPECIES_ONIX", "SPECIES_GEODUDE", "SPECIES_ZUBAT"],
  )
  assert.equal(data.tackle, true)
  // Z-Moves and Max Moves follow MOVES_COUNT and are not pool moves; old names are aliases.
  assert.equal(data.zmove, false)
  assert.equal(data.aliases, "MOVE_FEINT_ATTACK")
  assert.ok(data.valid > 800)
  // P_LVL_UP_LEARNSETS is GEN_7: Onix opens with Mud Sport, Tackle, Harden and Bind.
  assert.deepEqual(data.learnsets.SPECIES_ONIX.levelUp, [
    [1, "MOVE_MUD_SPORT"],
    [1, "MOVE_TACKLE"],
    [1, "MOVE_HARDEN"],
    [1, "MOVE_BIND"],
  ])
  assert.equal(data.learnsets.SPECIES_ONIX.stealthRock, true)
  assert.deepEqual(data.learnsets.SPECIES_GEODUDE.levelUp.slice(0, 2), [
    [1, "MOVE_TACKLE"],
    [1, "MOVE_DEFENSE_CURL"],
  ])
  // Earthquake is a TM for Onix and Geodude, not Zubat.
  assert.equal(data.learnsets.SPECIES_ONIX.earthquake, true)
  assert.equal(data.learnsets.SPECIES_GEODUDE.earthquake, true)
  assert.equal(data.learnsets.SPECIES_ZUBAT.earthquake, false)
})

test("egg moves are read from egg_moves.h through each species' species_info entry", () => {
  const egg = run(
    `(lambda result: {token: value["egg"] for token, value in result[2].items()})(module.load_learnsets(args))`,
    ["SPECIES_DRATINI", "SPECIES_DRAGONITE", "SPECIES_PIKACHU", "SPECIES_PICHU"],
  )
  assert.ok(egg.SPECIES_DRATINI.includes("MOVE_EXTREME_SPEED"))
  // Evolved species and Pikachu have no egg move list of their own; Pichu does.
  assert.deepEqual(egg.SPECIES_DRAGONITE, [])
  assert.deepEqual(egg.SPECIES_PIKACHU, [])
  assert.ok(egg.SPECIES_PICHU.includes("MOVE_WISH"))
})

test("a line's Egg hatches as the root of its evolution tree, babies included", () => {
  assert.deepEqual(
    run(
      "(lambda evolutions: [module.egg_species(token, evolutions[1]) for token in args])(module.load_evolutions())",
      ["SPECIES_RAICHU", "SPECIES_PIKACHU", "SPECIES_DRAGONITE", "SPECIES_SNORLAX", "SPECIES_ONIX"],
    ),
    ["SPECIES_PICHU", "SPECIES_PICHU", "SPECIES_DRATINI", "SPECIES_MUNCHLAX", "SPECIES_ONIX"],
  )
})

test("the default moveset keeps the last four level-up moves, skipping evolution and known moves", () => {
  const learnset = [
    [0, "EVOLVE"],
    [1, "A"],
    [1, "B"],
    [5, "C"],
    [9, "A"],
    [12, "D"],
    [15, "E"],
    [20, "F"],
  ]
  assert.deepEqual(run("module.default_moveset(args, 1)", learnset), ["A", "B"])
  assert.deepEqual(run("module.default_moveset(args, 19)", learnset), ["B", "C", "D", "E"])
})

test("the pool draft gives every trainer 8-12 ordered entries with tiered from levels", () => {
  const pools = run("{name: module.draft_pool(name) for name in module.POOL_DRAFT}")
  assert.equal(Object.keys(pools).length, 38)
  for (const [name, pool] of Object.entries(pools)) {
    assert.ok(pool.length >= 8 && pool.length <= 12, `${name}: ${pool.length} entries`)
    for (const entry of pool)
      if ("fromLevel" in entry)
        assert.ok([20, 30, 38, 40, 45, 55].includes(entry.fromLevel), `${name} ${entry.move}`)
  }
  assert.deepEqual(pools.Brock.slice(0, 4), [
    { move: "Bind" },
    { move: "Stealth Rock" },
    { move: "Sandstorm", fromLevel: 20 },
    { move: "Curse" },
  ])
})

test("move pool validation requires known moves and from levels 1-100", () => {
  const validate = (pool) =>
    run("module.validate_pool('Fixture', args, {'Earthquake', 'Stone Edge'})", pool)
  assert.equal(validate([{ move: "Earthquake" }, { move: "Stone Edge", fromLevel: 40 }]), null)
  assert.equal(
    validate([{ move: "Earthshake" }]),
    "Fixture move pool [1]: unknown move 'Earthshake'",
  )
  assert.equal(
    validate([{ move: "Earthquake", fromLevel: 0 }]),
    "Fixture move pool [1]: fromLevel must be an integer from 1 to 100",
  )
  assert.equal(
    validate([{ move: "Earthquake", fromLevel: 101 }]),
    "Fixture move pool [1]: fromLevel must be an integer from 1 to 100",
  )
  assert.equal(
    validate([{ move: "Earthquake", level: 5 }]),
    "Fixture move pool [1]: an entry is a move and an optional fromLevel",
  )
})

test("the pool warnings flag unlearnable entries and TM/tutor or egg entries without a from level", () => {
  const pool = [
    { move: "Earthquake" },
    { move: "Toxic" },
    { move: "Toxic", fromLevel: 30 },
    { move: "Spikes" },
    { move: "Sky Attack" },
  ]
  // Toxic is a TM move and Spikes an egg move: both are learnable, but not by level-up.
  assert.deepEqual(
    run("module.pool_warnings(args[0], set(args[1]), set(args[2]))", [
      pool,
      ["Earthquake"],
      ["Earthquake", "Toxic", "Spikes"],
    ]),
    [["Sky Attack"], ["Toxic", "Spikes"]],
  )
})

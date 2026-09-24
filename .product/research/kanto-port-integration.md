# Kanto port integration validation

The combined Hideout, local-adventure, and Lavender Tower ports passed bounded
integration validation on 2026-09-24. Gameplay source was frozen at
`aba66d2be97c6dbc8d0dfdf5763f0e5a15f3273f`, combining #124, #125's integration
head `411b4e94dc05cac238822f819e6e8df80a3c3369`, and #127. This records local
validation; GitHub checks and merge state are recorded on the PRs.

## Gameplay evidence

`wayfarer-kanto-scope-handoff.e2e.ts` keeps one game through the Hideout door
guards, Giovanni, his physical Silph Scope reward, Tower entry through the radio
guard, Marowak, the three Tower Rockets, Fuji's rescue, and Fuji's physical Poké
Flute. Save/reload checks retain the rewards and completed story state. The test
does not inject the Scope or the story transitions under test. It arranges
combat and travel to keep the journey bounded.

`wayfarer-hideout-silph-independence.e2e.ts` completes Hideout and Silph in both
orders. It interleaves Miguel and the Mt. Moon fossil reward, then checks separate
trainer defeats, completion flags, Scope, Master Ball, and fossil across reloads.
Both Giovanni battles and both rewards use their live scripts.

The handoff journey completes Tower while Machine Part and radio progress remain
unset, then checks that the director still requires Machine Part completion.
The existing magnet-train journey also exercises the actual Machine Part errand.

The selected 15 journey files cover Hideout; the four Silph suites; Mt. Moon;
Nugget Bridge; Safari Teeth; Tower; both new integration journeys; Anne; Safari
regression; Kanto traversal; and magnet-train traversal. The first run passed
158/160 cases. Two emulator processes aborted during startup, before gameplay
(Hideout wall switch and Silph Steven service). Rerunning those two files with
fewer concurrent launches passed 26/26 on the identical ROM. All 160 selected
cases therefore have passing evidence; the first run was not wholly green.

## Shared infrastructure

Trainer IDs remain Hideout 1723–1735, unused 1736, local adventures 1737–1778,
and Tower 1779–1794, with `TRAINERS_COUNT_WAYFARER` equal to 1795. Hideout and
local defeat flags remain separate from Tower's 16-bit SaveBlock3 field. The
duplicate ordinary-battle eligibility helper was consolidated, and trainer sight
dispatch retains both adventure resolvers.

Map/catalog registrations and trainer rosters were combined before regenerating
scaling outputs and Sevii audit inputs. Independent source review found no
actionable reconciliation defects. No `map.bin` files changed.

| Check | Result |
| --- | --- |
| Wayfarer native tests | 121/121 passed |
| Native tests matching `*Tower` | 11/11 passed, including defeat bit 15 and daily reset persistence |
| Trainer scaling tests | 34/34 passed |
| Combined mapjson tests | 62/62 passed |
| Scaling generation/check | 1,781 populated trainers, 1,422 ordinary, no structural failures |
| Sevii port and content audits | Passed |
| E2E typecheck | Passed |
| Production release and reserve check | Passed |

## Build identity and reproduction

Builds ran sequentially per worktree with GCC 13.2.1 and Node 24 for the E2E
workspace. Use the repository's locked pnpm dependencies.

```sh
make -C game -j6 CXX=g++ e2e
make -C game -j6 CXX=g++ BUILD=wayfarer check TESTS=Wayfarer
make -C game -j6 CXX=g++ BUILD=wayfarer check 'TESTS=*Tower'
make -C game -j6 CXX=g++ BUILD=wayfarer release
```

Run journey files with explicit matching artifacts:

```sh
SKYEMU_ROM="$PWD/game/pokemon-wayfarer-e2e.gba" \
SKYEMU_SYMS="$PWD/game/pokemon-wayfarer-e2e.sym" \
pnpm --dir e2e exec wa test src/journeys/wayfarer-kanto-scope-handoff.e2e.ts \
  src/journeys/wayfarer-hideout-silph-independence.e2e.ts
```

Production uses 31,658,020 bytes, leaving 1,896,412 bytes free and 1,372,124 bytes
before the protected 512 KiB reserve. The generated release report enforces that
reserve.

| Artifact | SHA-256 |
| --- | --- |
| E2E ROM | `79ee689b256b8f948ce3ae1c1c2d77e585ffa33f8d4a4616edfb9a93cec3fc48` |
| E2E symbols | `d960e70c73ffc7ce9bac7b81a76a532cc8746442c78256a3fe8313805e8c7cda` |
| Production ROM | `caee896074ec3ce31dc7c1f898b43652c23fce975a25ad3a9677c65914af958e` |

Session logs and the validation index were retained under
`/tmp/kanto-integration/`; that directory is temporary and is not required to
reproduce the committed journeys.

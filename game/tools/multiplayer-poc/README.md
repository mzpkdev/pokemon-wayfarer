# Native link PoC runner

This research runner starts two separate GBA cores in mGBA 0.10.5, attaches
both to mGBA's `GBASIOLockstepNode` serial driver, and feeds each core its own
controller inputs. It does not inject received packets or synchronize game
memory. The optional existing `E2E_TESTING` request arranges a fresh game on
each cartridge before inputs start; all link traffic afterward crosses the
emulated serial cable. The ROM files are copied to distinct temporary paths so
the two cartridges cannot share a save file by filename.

Build a private, pinned mGBA static library (no system installation):

```sh
mkdir -p /tmp/wayfarer-multiplayer-poc-tools
curl -L --fail https://github.com/mgba-emu/mgba/archive/refs/tags/0.10.5.tar.gz \
  -o /tmp/wayfarer-multiplayer-poc-tools/mgba-0.10.5.tar.gz
echo '91d6fbd32abcbdf030d58d3f562de25ebbc9d56040d513ff8e5c19bee9dacf14  /tmp/wayfarer-multiplayer-poc-tools/mgba-0.10.5.tar.gz' | sha256sum -c -
tar -xzf /tmp/wayfarer-multiplayer-poc-tools/mgba-0.10.5.tar.gz \
  -C /tmp/wayfarer-multiplayer-poc-tools
cmake -S /tmp/wayfarer-multiplayer-poc-tools/mgba-0.10.5 \
  -B /tmp/wayfarer-multiplayer-poc-tools/build \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release \
  -DLIBMGBA_ONLY=ON -DBUILD_STATIC=ON -DBUILD_SHARED=OFF \
  -DUSE_FFMPEG=OFF -DUSE_MINIZIP=OFF
cmake --build /tmp/wayfarer-multiplayer-poc-tools/build --target mgba --parallel 8
```

Build the PoC test ROM using the main game build instructions, then run from
the repository root:

```sh
python3 game/tools/multiplayer-poc/run.py \
  game/pokemon-wayfarer-e2e-poc.gba game/pokemon-wayfarer-e2e-poc.elf \
  --mgba-source /tmp/wayfarer-multiplayer-poc-tools/mgba-0.10.5 \
  --mgba-build /tmp/wayfarer-multiplayer-poc-tools/build \
  --stage-e2e --require-presence \
  --output /tmp/wayfarer-multiplayer-poc-validation/link-harness/presence
```

`presence.inputs` uses `frame player key-mask` lines. Frame zero is when both
`E2E_TESTING` arrange requests have completed. Each mask is the usual GBA key
bitmask (A=0x001, B=0x002, SELECT=0x004, START=0x008, RIGHT=0x010,
LEFT=0x020, UP=0x040, DOWN=0x080, R=0x100, L=0x200). The default script
presses SELECT+L+R on both sides, then gives the players different movement.
Use `--inputs path` for another sequence. Each mask stays held until a later
line for that player changes it; write a zero-mask line to release all keys.
`bag.inputs` opens and closes the full Bag while the other player moves;
`rejoin.inputs` leaves and reconnects both carts; `map-door.inputs` uses
`--stage-position 20,12` to walk through the New Bark house door and return.
`map-door-idle.inputs` and `map-door-linger.inputs` repeat the route without
linking, while `map-door-linked-linger.inputs` keeps the two cartridges linked.
The all-black frame 230 in the original short door run was an ordinary fade:
the faded palette contained no nonzero colors at absolute frame 330, and the
interior rendered from relative frame 260 through 550 with no map-load error
in both the linked PoC and compiled-out control. The user can reproduce the
stable interior with `--stage-position 20,12 --frames 800 --capture-frames
200,230,260,300,400,550,650,700` and a linger script.
`--stage-map group,num` forwards another map ID through the existing E2E
arrange request; the default still uses the checkpoint map. With
`center-repeat.inputs --stage-map 0,1 --stage-position 47,8 --frames 2400`,
P0 walks through the Cherrygrove Pokémon Center door three times while P1
uses the Bag outside.

`--enable-saves` mounts independent emulated flash files and exports them as
`player0.sav` and `player1.sav` after both cores shut down. Supply `--save0`
and `--save1` to import prior flash files into new cores. `--stage-party`
arranges one different original Pokémon on each cartridge via the existing
E2E fixture; `--e2e-save-frame 120` invokes its real SAVE command after
staging. A full flash save can take about 280 emulated frames, so use at least
`--frames 800` for a settled baseline. To test fresh-core reload, import
the files on a run without `--stage-e2e` and use `continue-both.inputs` to
select Continue through the ordinary title menu. `--e2e-save-player 0` or
`1` restricts the save to one cartridge for a link stress case.
This tests a fresh mGBA core loading a saved cartridge image; it does not
exercise physical hardware.

`--e2e-wild-battle-frame 160 --e2e-wild-battle-player 0` starts a local wild
battle on only one staged cartridge through the existing test command;
`presence.inputs` keeps the other cartridge in the overworld. The battle
itself uses ordinary game code and controller input. For the cooperative
encounter, import the valid independent saves and use `coop-distinct.inputs`
to connect, consent with L+R+START, choose different local moves, and dismiss
the battle text. `coop-distinct-first-turn.inputs` stops after the first move
pair; `coop-abort-walk.inputs` checks field controls after cable removal.
`coop-autoadvance.inputs` retains the initial same-move research run.
For two simultaneous, independent wild battles, combine `--stage-party
--e2e-wild-battle-player both` with `both-wild-run.inputs` to flee or
`both-wild-fight.inputs` to choose attacks and finish both battles.
`map-bag-overlap.inputs --stage-position 20,12` repeats the house doorway
while the other player opens the Bag.
The runner exports
the ROM's `battle.json` and `reward.json` diagnostics when those symbols exist.
Its watch log includes display/palette state, save status, party count, and
hashes of selected save-block ranges, allowing before/after comparisons.
`audit_save.py` independently checks exported flash-sector checksums and
generation coherence, reconstructs saved blocks, decodes Potion count and the
two-byte reward ledger, and compares chosen ranges without modifying saves:

```sh
python3 game/tools/multiplayer-poc/audit_save.py OUTPUT/player0.sav \
  --compare BASELINE/player0.sav --require-coherent \
  --expect-potion 1 --expect-ledger 0xa1,2 \
  --unchanged flags,vars,party,frontier,pc
```

Its default `save-offsets.s` was generated by the ARM compiler from the
adjacent `save-offsets.c` for the PoC ROM build. Rebuild that layout when a
save structure changes; the audit report records the layout hash.

The following sequence creates its own two distinct baseline saves, plays a
cooperative encounter, cuts a *separate replay* during P0's reward save,
loads the recovered flash in fresh cores, and audits both outcomes. Run it
from the repository root with the matching E2E PoC ROM and ELF. Each command
records its exact ROM, library, inputs, imported-save hashes and arguments in
the output directory's `provenance.txt`.

```sh
poc_artifacts=/tmp/wayfarer-multiplayer-poc-repro
poc_tool=game/tools/multiplayer-poc
poc_mgba=/tmp/wayfarer-multiplayer-poc-tools
mkdir -p "$poc_artifacts"

python3 "$poc_tool/run.py" game/pokemon-wayfarer-e2e-poc.gba game/pokemon-wayfarer-e2e-poc.elf \
  --mgba-source "$poc_mgba/mgba-0.10.5" --mgba-build "$poc_mgba/build" \
  --inputs "$poc_tool/idle.inputs" --stage-e2e --stage-party --enable-saves \
  --e2e-save-frame 120 --frames 850 --output "$poc_artifacts/baseline"
python3 "$poc_tool/run.py" game/pokemon-wayfarer-e2e-poc.gba game/pokemon-wayfarer-e2e-poc.elf \
  --mgba-source "$poc_mgba/mgba-0.10.5" --mgba-build "$poc_mgba/build" \
  --inputs "$poc_tool/coop-distinct.inputs" --enable-saves \
  --save0 "$poc_artifacts/baseline/player0.sav" --save1 "$poc_artifacts/baseline/player1.sav" \
  --frames 8500 --output "$poc_artifacts/co-op" --timeout 300
python3 "$poc_tool/audit_save.py" "$poc_artifacts/co-op/player0.sav" \
  --compare "$poc_artifacts/baseline/player0.sav" --require-coherent \
  --expect-potion 1 --expect-ledger 0xa1,2 \
  --unchanged flags,vars,party,frontier,pc --output "$poc_artifacts/co-op/player0-audit.json"

python3 "$poc_tool/run.py" game/pokemon-wayfarer-e2e-poc.gba game/pokemon-wayfarer-e2e-poc.elf \
  --mgba-source "$poc_mgba/mgba-0.10.5" --mgba-build "$poc_mgba/build" \
  --inputs "$poc_tool/coop-distinct.inputs" --enable-saves \
  --save0 "$poc_artifacts/baseline/player0.sav" --save1 "$poc_artifacts/baseline/player1.sav" \
  --frames 8500 --cut-reward-phase writing --cut-player 0 --cut-delay-frames 100 \
  --output "$poc_artifacts/cut" --timeout 300
python3 "$poc_tool/audit_save.py" "$poc_artifacts/cut/player0.sav" \
  --compare "$poc_artifacts/baseline/player0.sav" --require-coherent \
  --expect-potion 0 --expect-ledger 0,0 \
  --unchanged flags,vars,party,frontier,pc --output "$poc_artifacts/cut/player0-audit.json"
python3 "$poc_tool/run.py" game/pokemon-wayfarer-e2e-poc.gba game/pokemon-wayfarer-e2e-poc.elf \
  --mgba-source "$poc_mgba/mgba-0.10.5" --mgba-build "$poc_mgba/build" \
  --inputs "$poc_tool/continue-corrupt.inputs" --enable-saves \
  --save0 "$poc_artifacts/cut/player0.sav" --save1 "$poc_artifacts/cut/player1.sav" \
  --frames 2500 --output "$poc_artifacts/reloaded"
```

The cut's newer slot is incomplete, so the flash audit selects the older
coherent baseline generation. The title menu shows a save-error warning;
`continue-corrupt.inputs` accepts it before choosing Continue. Inspect
`diagnostics.json`, `reward.json`, `cut.json`, and the captured images as well
as the audit JSON before interpreting a run.

For a pending-claim case, add `--stage-full-medicine` to the baseline command
on an E2E ROM that supports it. This deliberately fills all 92 Medicine slots
with Antidote x999. Replay the encounter with the same baseline import;
the saved reward ledger is pending and Potion count remains zero. Import that
pending save in fresh cores, use `pending-bag-toss.inputs` to discard one full
Antidote stack through the ordinary Bag and retry with L+R+A, then audit P0
for Potion 1 and ledger `0xa1,2`. This fixture is artificial; no received
link packets or reward outcomes are injected by the runner.

## Start-menu session experiments

The later session PoC adds a LINK action to the ordinary Start menu.
`ui-connect.inputs` chooses Start → LINK → Connect on both E2E-arranged
cartridges with ordinary buttons; `ui-connect-saves.inputs` does the same
after loading two distinct saves through ordinary Continue. The older
`presence.inputs` SELECT+L+R chord remains a historical research shortcut.
`ui-decline.inputs`, `ui-cancel.inputs`, `ui-arrival-held-a.inputs`, and
`ui-leave.inputs` cover explicit invitation refusal, outgoing cancellation,
an A button held across an incoming prompt, and explicit Leave. The
`release-*.inputs` scripts use a normal release ROM and saved cartridges to
exercise full invited co-op play, player-1 invitation, simultaneous invites,
an invite while the other Bag is open, and ordinary local Save.
`save-p0.inputs`, `save-p1.inputs`, `save-both.inputs`, and
`save-cancel.inputs` use ordinary Start-menu Save on E2E-arranged games; the
E2E fixture does not inject save results.

`--stage-menu-unlocks` sets the HNS Pokedex and Pokenav flags through the E2E
arrange request. DexNav is disabled in the current ROM configuration. LINK
then makes nine normal Start-menu actions: `max-menu-bottom.inputs` scrolls
to the ninth action, EXIT, and selects it, while `max-menu-link.inputs`
returns from EXIT to LINK and opens it. The option leaves normal menu logic
to render and navigate the entries.

The runner resolves symbols independently for each cartridge. Optional
`--rom1 path --elf1 path` supplies a different, matching player-1 ROM/ELF
pair; both options must appear together. Its `session.json` contains the
72-byte session diagnostic from each ROM, including status, save hold and
reconnect counters, and the stamped build ID. `--require-session` fails if
either is missing or malformed. `--expect-session-status 2,2` and
`--expect-session-error 0,0` assert final active sessions. A real-build
fingerprint rejection uses two *built* ROMs with the same valid baseline
saves and controller script:

```sh
python3 game/tools/multiplayer-poc/run.py RELEASE.gba RELEASE.elf \
  --rom1 E2E.gba --elf1 E2E.elf \
  --mgba-source /tmp/wayfarer-multiplayer-poc-tools/mgba-0.10.5 \
  --mgba-build /tmp/wayfarer-multiplayer-poc-tools/build \
  --inputs game/tools/multiplayer-poc/ui-mismatch-status.inputs \
  --enable-saves --save0 BASELINE/player0.sav --save1 BASELINE/player1.sav \
  --frames 3100 --require-session --expect-session-status 7,7 \
  --expect-session-error 7,7 --output OUTPUT
```

Replace the uppercase paths with matching build artifacts and the distinct
saved games from the baseline recipe above. The validated release-versus-E2E
pair rejects the connection with build mismatch error 7 before presence
becomes active, and reopening LINK displays the reason. An identical release
pair with `ui-connect-saves.inputs` instead reaches status 2 on both sides.

For interruption experiments, add `--cut-reward-phase writing --cut-player 0
--cut-delay-frames N` to the full co-op run. The runner stops both emulated
CPUs when P0's reward save has stayed in WRITING for N frames, then exports
the existing flash images without running more game code. `cut.json` records
the observed phase and exact cut frame. Use N=0 for the beginning of WRITING
and later values to sample partial flash sectors; `--cut-reward-phase success`
cuts after the selected save returns successfully. A fresh-core Continue run
can then check what the game actually recovers. `--cut-reward-phase before`
stops only after an agreed battle result reaches RETURNING with the reward
save still idle. A requested phase that never occurs makes the runner fail.
A partial newer slot triggers a save-error warning before Continue;
`continue-corrupt.inputs` accepts that delayed menu and loads the older save.
For cable removal, use `presence.inputs` with `--disconnect-frame 240`.
`--capture-frames` selects screenshots at input-relative frames; the watch
lines in `link.log` use absolute emulator frames, so add `input_origin_frame`
from the log when comparing them.

The wrapper compiles `link.c` against that library with its generated
`mgba/flags.h`, resolves ROM diagnostic and E2E addresses from the matching
ELF, enforces a wall-clock timeout, and writes `link.log`, `diagnostics.json`,
PNG frame captures, raw PPM images, and SHA-256 provenance into `--output`.
The 80-byte diagnostic layout is defined in `game/include/multiplayer_poc.h`.
`--require-presence` fails unless both cartridges report active sessions,
native packet sends and receives, visible peers, matching peer IDs, and
converged peer positions. The assertion is deliberately separate from the
runner's ability to advance frames.

`--disconnect-frame N` removes one mGBA serial driver at frame N to simulate
unplugging a cable. It is a research stress case, not a model of all real
cable, power-loss, or emulator network behavior. This runner does not prove
physical GBA timing, other emulator behavior, online play, or all possible
battle and flash-failure cases. Separate scoped co-op runs verify distinct
local choices and move animations, agreed results, restored original parties,
and audited reward saves. Its scheduling callbacks adapt cycle accounting
from mGBA's Qt multiplayer controller under the MPL-2.0 notice in `link.c`.

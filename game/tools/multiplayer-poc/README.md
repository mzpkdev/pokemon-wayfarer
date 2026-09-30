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
physical GBA timing, standard emulator coverage outside this pinned mGBA
build, online play, battle synchronization, or save integrity. Its scheduling
callbacks adapt cycle accounting from mGBA's Qt multiplayer controller under
the MPL-2.0 notice in `link.c`.

The research run found a remaining limitation: the house transition updates
both cartridges' map diagnostics and return travel works, but the entering
player's interior image stayed black while `CB2_Overworld` was active. The
captured input-relative frames 230 and 290 in the map-door run show this; it
must be resolved before claiming normal cross-map exploration.

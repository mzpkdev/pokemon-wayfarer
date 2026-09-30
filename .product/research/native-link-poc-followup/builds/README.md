# Build and save-layout evidence

`save-offsets.s` comes from the adjacent `save-offsets.c`, compiled with the
ROM's ARM ABI. Historical offset comments and field names in `global.h` are
not reliable after the expanded bag and other structure changes.

From `game/`:

```sh
arm-none-eabi-gcc -S \
  -o ../.product/research/native-link-poc-followup/builds/save-offsets.s \
  ../.product/research/native-link-poc-followup/builds/save-offsets.c \
  -iquote include -DMODERN=1 -DTESTING=0 -DPOKEMON_WAYFARER -DPOKEMON_HNS \
  -DE2E_TESTING=1 -DWAYFARER_MULTIPLAYER_POC=1 -std=gnu17 \
  -mthumb -mthumb-interwork -mabi=apcs-gnu -mtune=arm7tdmi -march=armv4t
```

The saved party begins at SaveBlock1 byte 572; the bag begins at 1380 and spans
2612 bytes; the experimental ledger is at 4324. The item and ledger therefore
belong to the same logical full-save generation but **not the same flash
sector**. The audit tool reconstructs sectors and validates their checksums;
it also checks generation coherence explicitly. SaveBlock3's spare-sector
chunks are outside the existing checksum, so equal reconstructed bytes do not
prove corruption protection for that block.

`integrated-e2e-source.json` identifies the first integrated development build
against base commit `1dbf6922e4fc45f2fd4f3018097f32e54a018219`; later validated
builds are identified in the parent report. The default release boot log is
from the unchanged, compiled-out artifact.

# ROM test workers

Hydra starts one worker per available CPU by default, capped at eight. When
`MAKEFLAGS` contains an explicit `-jN`, it uses that count instead (capped at
32); `-j` without a count uses all available CPUs, up to 32. If CPU detection
fails, it uses one worker. The selected count is printed before tests start.

From `game/`, `make BUILD=wayfarer check` uses the default. `make -j1
BUILD=wayfarer check` runs one worker. The worker-selection unit test can be
run without building a ROM:

```sh
cc -std=c11 -Wall -Wextra -Werror tools/mgba-rom-test-hydra/test_worker_count.c -o /tmp/hydra-worker-count-test
/tmp/hydra-worker-count-test
```

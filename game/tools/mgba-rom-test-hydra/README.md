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

## Output cap and temporary ROMs

Hydra holds each test's output until the test reports a result. It keeps at
most 1 MiB of a test's output, and at most 1 MiB of other mgba output between
two results, in whole lines. Anything past that is dropped and counted, and a
`[hydra: dropped N bytes ...]` note is printed with the test. A test that prints
in a loop until it times out can no longer grow Hydra until the machine runs
out of memory.

Each worker runs from its own copy of the ROM in
`$TMPDIR/mgba-rom-test-hydra-<pid>-XXXXXX`, removed when Hydra exits, is
interrupted, or loses its terminal. A run killed outright (`SIGKILL`, the OOM
killer) can't clean up, so at start Hydra removes directories of this form that
the current user owns and whose pid no longer exists.

The output-cap unit test also runs without a ROM:

```sh
cc -std=gnu11 -Wall -Wextra -Werror tools/mgba-rom-test-hydra/test_output_cap.c -o /tmp/hydra-output-cap-test
/tmp/hydra-output-cap-test
```

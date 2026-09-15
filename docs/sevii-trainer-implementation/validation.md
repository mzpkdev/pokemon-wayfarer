# Sevii Trainer implementation validation

Validated on branch `task/sevii-trainer-implementation` from exact foundation base
`fbc3bc33f7a37b89be850013938595155bb6ecc0`. The final validation candidate includes
`f99e57607f`.

## Delivered behavior

- The schema-v2 overlay projects all 87 ordinary Trainer objects representing 81
  base identities, including the six paired encounters. It preserves every
  non-Trainer manifest record and excludes story Rockets and Trainer Tower floor
  opponents.
- 86 objects use normal sight-battle scripts. Fisherman Tommy is the one talk-battle
  object. Dario and Rodette are ordinary exterior Trainers.
- 70 objects participate in the compact 64-family Vs. Seeker registry; the other 17
  remain single-stage. Rematches select the first alternate party at stage one and
  preserve readiness/stage across save and reload.
- Normal victories route through the generated base-defeat lookup. Rematch records
  alias their base identity's defeat bit. Blackout leaves the Trainer eligible for a
  retry.
- All selected parties use the current ordinary scaling policy. No source FRLG
  numeric Trainer ID reaches runtime.

## Shared contracts and dependencies

- `7fa44c8b75`, `501ebf73e2`, and `662bc57560` freeze the dense allocation and
  defeat contract. Runtime IDs 1515-1632 contain 118 ordinary base/rematch records;
  IDs 1633-1650 reserve 18 planned story records. `TRAINERS_COUNT_WAYFARER` is 1651
  and the partner boundary remains 2048.
- `e4674af709` publishes the compact rematch registry. `871b68ff0f` publishes the
  selected roster and scaling policy. `4c0f0cf53c` publishes shared defeat routing.
- This branch consumes the Story-owned SaveBlock3 work integrated as
  `b9299cd9d1` (upstream Story commit `0357e4a423`) and `3ee16942ab` (upstream Story
  commit `7664a169ad`). The shared Sevii aggregate is 208 bytes; its Trainer-owned
  logical payload is 67 defeat bytes, 16 two-bit stage bytes, and 8 pending bytes.
  The complete `SaveBlock3` is 1112 bytes, below the 1624-byte bound.
- Trainer owns Sevii flag slot 52 for the Wayfarer-only Vs. Seeker charging
  lifecycle. Story retains slots 11-51.

## Automated verification

All commands used GNU Make 4.4.1 and `arm-none-eabi` GCC 13.2.1. Product builds ran
serially within this worktree.

- `/tmp/sevii-build-tools/bin/make -C game BUILD=wayfarer -j2 check`: exit 0;
  4,368 passed, 349 known, 629 TODO, 6 expected failing, 9 assumptions failed,
  5,361 total. The assumptions are the suite's declared non-failures.
- Both Trainer generators passed `--check` after the final release build.
- Content-contract unit suite: 49/49 passed.
- Sevii-port preservation suite: 14/14 passed.
- Selected-roster and scaling suites: 6/6 passed.
- E2E formatting, type checking, linting, and diff checks passed.
- Headless SkyEmu ordinary-Trainer suite: 7/7 journeys passed in 54.67 seconds.
  It covers Garrett sight/victory, Crush Kin party denial and paired battle,
  representative Three/Five/Six Island defeat/reload, two real 100-step Vs.
  Seeker charges with `_2` then `_3` parties, exterior Dario, Tommy victory/reload,
  and Tommy blackout/retry. A focused real Sharon rematch chain also passed in
  28.23 seconds.

## Release measurements

The exact foundation-base evidence records 32,673,336 used bytes. The final
Wayfarer release uses 32,831,544 bytes (`__rom_end = 0x09F4F838`), a net increase of
158,208 bytes. It leaves 722,888 bytes unused, or 198,600 bytes above the required
512 KiB reserve.

| Category | Delta from exact foundation base |
| --- | ---: |
| Code | +7,000 bytes |
| Scripts | +27,808 bytes |
| Maps/layouts | +2,088 bytes |
| Trainer data | +32,700 bytes |
| Other | +88,612 bytes |
| Audio, graphics, encounters | 0 bytes |

Supported release products also build successfully:

| Product | Used ROM | EWRAM | IWRAM |
| --- | ---: | ---: | ---: |
| firered | 28,622,040 | 247,625 | 25,864 |
| leafgreen | 28,622,444 | 247,625 | 25,864 |
| emerald | 28,527,016 | 247,513 | 25,648 |
| hns | 30,809,256 | 247,593 | 25,612 |
| wayfarer | 32,831,544 | 248,781 | 25,564 |

## Integration status

This branch is ready for isolated review and integration. It does not claim final
three-branch readiness: the parent integration must merge the Trainer, Story, and
Tower domain records into the single manifest, regenerate all derived outputs, and
repeat contract, emulator, save-size, and release-reserve validation. No known
Trainer-domain blocker remains.

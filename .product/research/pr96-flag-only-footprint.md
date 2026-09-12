# Trainer-only encounters with an explicit author flag

This prototype retains the current Rock, Ball, Berry, Go Near, and Run mechanic, including anger and retaliation. It removes the NPC story registries and their authored wrappers, special loss continuation, last-party storage exceptions, trainer-sight suppression, and in-encounter medicine recovery.

The feature is disabled until an authored safe scenario enables `FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS` (`FLAG_TEMP_B`, `0xB`). The only activation conditions are that flag and `CalculatePlayerPartyCount() == 0`: fainted and Egg-only parties are nonempty and therefore never enter this mode. The flag does not convert trainer or scripted wild battles; those continue through native setup, and authors must keep an empty flagged player out of unsupported situations.

Authors enable the flag after map loading, for example in their safe map's entry script:

```asm
setflag FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS
```

They can disable it before releasing the player into unsafe content:

```asm
clearflag FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS
```

The flag clears on map changes through the engine's existing temporary-field reset. Running or fleeing from a wild encounter on the same map leaves it enabled, allowing another attempt. No production map enables it in this prototype. Authors own the safety of the scenario while it is enabled; there is no map allowlist, NPC classification table, recovery fallback, or automatic story compatibility system.

Ordinary party defeat and field-poison exhaustion use native recovery. Retaliation completes the normal battle teardown and then uses native blackout/recovery, without a feature-specific injury message or regional override. Native last-party storage protection is restored.

The comparison uses committed optimization `9cde68903d7380744d513ff156832a9f87642a68` at 32,691,240 used ROM bytes. This is independent of the Safari-action experiment: none of its gameplay or controller changes are included. Both releases use ARM GCC 13.2.1 and the Wayfarer release configuration.

| Measurement | Committed optimization | Flag-only prototype |
| --- | ---: | ---: |
| Used ROM bytes | 32,691,240 | 32,673,336 |
| Feature growth over PR base | 22,840 B (22.30 KiB) | 4,936 B (4.82 KiB) |
| Added executable code over PR base | 11,624 B (11.35 KiB) | 4,384 B (4.28 KiB) |

The net reduction is **17,904 B (17.48 KiB)**: 7,240 B less executable code, 7,696 B less other data, and 2,972 B less script data, offset by 4 B in graphics-section accounting. The reduction is 78% of the committed feature's added ROM footprint. Fixed 32 MiB file padding is excluded.

Evidence is in the task's `evidence/simplification/safe-places/` directory: the preserved `9cde68903d` baseline, release ROM/ELF/map and size report, plus source manifests. The final release ROM SHA-256 is `4a5efc952c50e4095bd8c3d9aff3582a37276573ed0e495058db74fcdc97ce6f`.

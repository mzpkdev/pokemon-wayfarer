# Wayfarer Trainer Tower host audit

`audit.py` is a deterministic, host-only proof for the frozen built-in Trainer
Tower set in `src/trainer_tower_sets.c`. It accepts no downloaded, link,
record-mixing, Mystery Gift, or e-Reader payload. It verifies the local header,
the exact four-by-eight pointer table, the authored Mixed row sequence, source
prizes, actor counts, checksums, and every selected opponent's static fields and
symbolic species, move, item, facility-class, personality, and Easy Chat
references.

Run it from `game/`:

```sh
python3 tools/wayfarer_trainer_tower/audit.py --output /tmp/trainer-tower-audit.json
python3 -m unittest discover -s tools/wayfarer_trainer_tower/tests -v
```

The frozen payload SHA-256 intentionally covers only selected local data rather
than the full C file, so unrelated source maintenance does not mask or invent a
course change. Updating that fingerprint requires reviewing regenerated JSON.

# Wayfarer Sevii shared contracts

Milestone 1 is host-only: it emits no roster, state payload, or runtime path.
Future map-owner records use the common `content_id`; their matching contract
records live in the map manifest's `contracts` object.

```json
"contracts": {
  "schema_version": 1,
  "trainer_ids": {"base": 1515, "limit": 2048, "allocations": []},
  "state_namespace": {
    "name": "wayfarer_sevii",
    "flag_base": 49152,
    "flag_capacity": 256,
    "var_base": 53248,
    "var_capacity": 32,
    "defeat_bitset": "wayfarer_sevii_trainer_defeat",
    "storage": "SaveBlock3"
  },
  "states": [],
  "transactions": []
}
```

The active Wayfarer IDs end at 1514. The fixed 1515–2047 range gives later
streams 533 explicit slots without a hole in the eventual dense Trainer table.
Slots never derive from the moving active Trainer count. `selected_trainer_render`
compiles the current selected FRLG source with Wayfarer product defines, uses the
existing trainer-scaling parser, and returns only selected rows. Each row carries
the exact compiled record and normalized source block so later generators retain
name, class, sprite, music, AI, prize, items, IVs, moves, and party order.

The `0xC000` flag and `0xD000` variable reservations describe future dedicated
SaveBlock3 storage; they do not change the current save layout. State writers
must be owned by the state owner and declare monotonic legal transitions.
Trainer defeat state uses the allocation's same slot so a later router can
select the Sevii bitset before its appended-HNS branch.
Every allocation names a `defeat_base`: a base points to itself, while a
rematch points to that stable base and therefore cannot create another defeat
bit. Trainer Tower facility opponents stay outside this persistent allocation.
Each selected battle also declares a source-checked `battle_type` (`single` or
`double`) and outcome policy: `ordinary` uses `defeat_and_blackout`; an
`objective_guard` uses `win_progress_loss_pending`.

Transactions require a receipt. A handoff attempts its destination before it
consumes the source, sets the receipt, and updates presentation. A grant omits
source consumption but still sets its receipt after successful delivery.
Ordinary Trainers cannot declare transactions or write any state other than
their own dedicated defeat bit.

States default to one-way `completion` lifecycle. A Story or Trainer Tower
payload can explicitly use `lifecycle: "transactional"` only in a reserved
Sevii variable with a matching `transaction_id`. A repeatable `claim` names
that `pending_state`, has `receipt: null`, and performs
`establish_prerequisite`, `attempt_destination`, `clear_pending`, then
presentation. The pending payload can therefore be staged again after a
successful claim, while a full destination leaves it pending. One-time
handoffs and grants retain their completion receipt, which never resets.

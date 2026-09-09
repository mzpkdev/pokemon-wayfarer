# Trainer appearance animation checks

Run from the repository root:

```sh
python3 game/tools/trainer_appearance/check_manifest.py
python3 game/tools/trainer_appearance/test_manifest.py
```

`manifest.json` records all 36 registered style/state combinations. The checker
reads the registry, graphics pointer table, descriptors, animation commands,
picture tables, PNG dimensions, and palette registrations. It rejects missing
descriptors, incompatible animation tables, absent directional/action slots,
animation frame indices outside picture tables, and picture references outside
their source PNGs. It also checks reflection palette registration. Refresh with
`--write` after a deliberate source change and review the resulting inventory.
Refreshing preserves failures and still exits nonzero.

These checks establish bounds and table compatibility. They do not establish
that a pose depicts the correct character or action. Every manifest entry keeps
visual acceptance separate; emulator checks are still required. Source-sheet
reuse must also be reviewed against the art, including the walking/running frame order.

The v1 registry includes Gold, Kris, Brendan, and May with stable saved IDs
1, 2, 5, and 6. Red and Leaf are deferred because their Acro Bike, underwater,
and watering art does not satisfy those action contracts. Their standalone
FRLG graphics remain unchanged and are not included in Wayfarer's manifest.

## Field rendering audit

| Call site | Identity source and behavior |
| --- | --- |
| `GetPlayerAvatarGraphicsIdByStateId`, `InitPlayerAvatar` | Saved local appearance; independent of active map edition. |
| `GetPlayerAvatarGraphicsIdByCurrentState` | Saved local appearance plus actual avatar movement flags. |
| `GetPlayerAvatarStateTransitionByGraphicsId` | Wayfarer restores movement flags already held by the avatar, even if different movement states share a graphics ID. Standalone builds retain legacy lookup. |
| `GetPlayerAvatarGraphicsIdByStateIdAndGender`, `GetRivalAvatarGraphicsIdByStateIdAndGender` | Explicit legacy actor identity; never consult local appearance. |
| `GetFRLGAvatarGraphicsIdByGender` / `CreateLinkPlayerSprite` | Remote FRLG actors use unconditional legacy `OBJ_EVENT_GFX_RED` / `OBJ_EVENT_GFX_LEAF` descriptors (230/231), their walking sheets, and `OBJ_EVENT_PAL_TAG_RED_LEAF`. They do not use the deferred local action descriptors. |
| `SetPlayerAvatarObjectEventIdAndObjectId` | Recreated local object retains saved story gender and actual movement state. |
| `LoadPlayerObjectEventPalette` | Explicit legacy gender palette API. |
| `LoadLocalPlayerObjectEventPalette` | Saved appearance's normal character palette. |
| `CreateFlyBirdSprite`, `CreateRockClimbBlob` | Local auxiliary effect uses the local palette API; auxiliary color compatibility still needs visible validation. |
| `FldEff_NPCFlyOut` | Retains explicit legacy helper for NPC departure scripts. |
| `field_effect_helpers.c` surf blob and arrow; `surfable.c` | Local palette API; fixed auxiliary sheet colors still need visible validation. |
| `FldEff_ORASDowsing` | Local fallback palette updated. ORAS dowsing is disabled (`I_ORAS_DOWSING_FLAG = 0`); its gender-specific rod overlay remains native. |
| `UseVsSeeker_DoPlayerAnimation` | Saved appearance's VS Seeker descriptor, preserving movement state for restoration. The four styles reuse their field pose. |

`game/test/wayfarer_appearance_movement.c` checks state restoration with deliberately
shared graphics IDs, all four saved styles across five movement modes, and explicit
rival/gender isolation. These mechanics tests require the Wayfarer test build;
the Python tests exercise the checker independently of ROM builds.

`game/test/wayfarer_appearance_remote.c` checks both remote FRLG genders against
every registered local style, including descriptor availability, dimensions,
animation/image pointers, and the legacy palette.

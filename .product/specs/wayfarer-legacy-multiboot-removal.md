# Wayfarer legacy multiboot removal

PRD: [Wayfarer legacy multiboot removal](../prds/wayfarer-legacy-multiboot-removal.md)
Implemented: No

## Scope

This specification defines how Wayfarer excludes the Pokemon
Colosseum/GameCube client, Ruby/Sapphire Berry Glitch Fix transmitter, and
e-Reader transfer program. It covers their embedded payloads, runtime entry
points, user-facing routes, e-Reader fallbacks, and build validation.

It does not remove the payload files from the repository, remove the features
from standalone products, or change ordinary GBA link play and wireless
Mystery Gift behavior.

## Behavior

### Build capabilities

The build exposes these independent boolean capabilities:

| Capability | Wayfarer | Standalone default | Controls |
| --- | --- | --- | --- |
| `ENABLE_COLOSSEUM_MULTIBOOT` | forced `0` | `1` | GameCube handshake, client payload, and execute path |
| `ENABLE_BERRY_GLITCH_FIX_MULTIBOOT` | forced `0` | `1` | title-screen route, transmitter, assets, and payload |
| `ENABLE_EREADER_TRANSFER` | forced `0` | `1` | e-Reader menu route, transfer code, payload, and Trainer Hill card import |

Each value must be `0` or `1`. The Makefile owns the product defaults and
passes the resolved values through `CPPFLAGS` so C and preprocessed data
assembly use the same setting. A Wayfarer request that attempts to set any of
the capabilities to `1` fails with a clear build error. Standalone builds may
set any capability to `0` without changing the others.

Implementation sites test the capability they consume. They must not acquire
scattered `IS_WAYFARER` conditions or use one umbrella "legacy hardware"
guard. This keeps standalone compatibility explicit and prevents one removed
feature from taking shared link code with it.

### Payload inventory and guards

The payload definitions and their direct consumers are:

| Feature | Payload definition | Symbols and direct consumers |
| --- | --- | --- |
| Pokemon Colosseum/GameCube | `game/data/multiboot_pokemon_colosseum.s` includes `game/data/mb_colosseum.gba` | `gMultiBootProgram_PokemonColosseum_Start` and `_End`; `game/include/multiboot_pokemon_colosseum.h`; copyright-screen code in `game/src/intro.c`, `game/src/intro_hns.c`, and `game/src/intro_frlg.c`; support in `game/src/libgcnmultiboot.s` and `game/include/libgcnmultiboot.h` |
| Berry Glitch Fix | `game/data/multiboot_berry_glitch_fix.s` includes `game/data/mb_berry_fix.gba` | `gMultiBootProgram_BerryGlitchFix_Start` and `_End`; `game/src/berry_fix_program.c`; `game/include/berry_fix_program.h`; title routes in `game/src/title_screen.c` and `game/src/title_screen_frlg.c`; sender in `game/src/multiboot.c`; Berry fix graphics included by `game/src/graphics.c` and declared in `game/include/graphics.h` |
| e-Reader transfer | `game/data/multiboot_ereader.s` includes `game/data/mb_ereader.gba` | `gMultiBootProgram_EReader_Start` and `_End`; `game/src/ereader_screen.c`; `game/include/ereader_screen.h`; `CB2_InitEReader` and its menu action in `game/src/mystery_gift_menu.c`, `game/include/mystery_gift_menu.h`, and `game/src/main_menu.c`; transfer support in `game/src/ereader_helpers.c` and `game/include/ereader_helpers.h` |

Each data assembly file keeps its `.incbin` and symbol pair only when its
capability is enabled. When disabled, the object contributes no payload bytes
and does not define placeholder start/end symbols. Every declaration and code
reference to an omitted symbol uses the same guard, so a disabled build cannot
link accidentally through a weak or dummy definition.

Feature-specific support that has no remaining caller is also compiled out.
For Wayfarer this includes the GameCube multiboot state and assembly routines,
the Berry transmitter state machine and its exclusive graphics, and the
e-Reader serial transfer state machine. Generic or shared link support stays
when another enabled feature uses it.

The binary files themselves are not deleted. The build continues to track
them as inputs for enabled standalone configurations.

### Pokemon Colosseum/GameCube startup

The live Wayfarer path is the HNS copyright-screen implementation in
`game/src/intro_hns.c`. Its current serial callback calls
`GameCubeMultiBoot_HandleSerialInterrupt`, initializes and polls the GameCube
multiboot state, copies `0x28000` bytes to EWRAM, and may execute the client.
The equivalent standalone paths live in `game/src/intro.c` and
`game/src/intro_frlg.c`.

With `ENABLE_COLOSSEUM_MULTIBOOT=0`, copyright setup does not install the
GameCube serial callback, initialize or poll a `GcmbStruct`, inspect the
GameCube game code, copy the client, or call an execute/quit routine. At the
former state-140 boundary, the HNS and FRLG paths start the normal fade
unconditionally. When that fade completes, they install the ordinary
`SerialCB`, return through the existing copyright completion path, and let the
following state perform its current reset and intro transition. The generic
Emerald state names receive the equivalent behavior. Visible timing and the
post-copyright destination do not change.

This guard does not apply to `BattleColosseum_2P`,
`BattleColosseum_4P`, Cable Club scripts, `cable_club.c`, or their save-location
handling. Those names refer to ordinary GBA battles. `VERSION_GAMECUBE` also
remains because it is origin metadata, not an entry point to the payload.

### Berry Glitch Fix title route

`game/src/title_screen.c` and `game/src/title_screen_frlg.c` currently recognize
`B+Select` and transition to `CB2_InitBerryFixProgram`. When the capability is
disabled, the input branch, transition callback, and feature-only declarations
are absent. `B+Select` has no replacement action. The clear-save and reset-RTC
button chords retain their existing behavior and priority.

`game/src/berry_fix_program.c` and the Berry fix payload definition compile to
no feature code or data in the disabled configuration. Berry fix graphics in
`game/src/data/graphics/berry_fix.h` are excluded from `game/src/graphics.c`
and their declarations are guarded. The duplicate unused graphics table in
`game/src/berry_fix_graphics.c` must not keep the assets live. Because
`game/src/multiboot.c` has no caller outside this transmitter today, it may be
compiled out under this capability, but only after a call-site audit confirms
that remains true.

### e-Reader menu and transfer route

The compiled route is
`main_menu.c` -> `ACTION_EREADER` -> `CB2_InitEReader` ->
`CreateEReaderTask` -> `Task_EReader`. The task sends the embedded program,
receives and validates card data, and writes a Trainer Hill special sector.
The route is currently latent because `Task_MainMenuCheckSaveFile` can advance
only as far as `HAS_MYSTERY_GIFT`, not `HAS_MYSTERY_EVENTS`. It is still guarded
so a later menu change cannot expose a route whose payload is absent.

With `ENABLE_EREADER_TRANSFER=0`, the main menu never selects
`ACTION_EREADER`, no cursor position is assigned to it, and no e-Reader label
or transfer prompt is displayed. In `HAS_MYSTERY_EVENTS`, the affected item
remains the ordinary wireless Mystery Gift action. Without a Wireless Adapter,
it uses the same invalid-action handling as `HAS_MYSTERY_GIFT`, redraws the
main menu, and shows no e-Reader-specific error. It never enters a partial
e-Reader setup screen.

The e-Reader-specific initializer and task are compiled out. The shared
`CB2_MysteryGiftEReader` callback in `game/src/mystery_gift_menu.c` remains
despite its name: `CB2_InitMysteryGift` also uses it for ordinary Mystery Gift.
The ordinary `CB2_InitMysteryGift` action, shared menu drawing, Wireless
Adapter detection, and the `link_rfu_2.c` callback check therefore remain.
Shared code may be renamed in a later cleanup, but renaming is not required by
this feature.

Mystery Gift client and server command handling remains intact for Wonder
Cards, Wonder News, stamps, scripts, and the separate visiting-trainer format.
In particular, `CLI_RECV_EREADER_TRAINER` and
`SVR_LOAD_EREADER_TRAINER` are not consumers of the 12,512-byte transfer
program and do not use this capability guard.

### e-Reader hooks and saved data

Disabling the transfer capability does not free or repurpose saved fields in
this change. In particular, the 188-byte `BattleTowerEReaderTrainer` field
guarded by `FREE_BATTLE_TOWER_E_READER` keeps its current layout, and special
save sector `SECTOR_ID_TRAINER_HILL` remains reserved. This avoids mixing a ROM
space decision with an unrelated save-layout rewrite. No migration is added.

The visiting-trainer field is consumed by `battle_special.c`,
`battle_frontier.c`, `battle_factory.c`, `battle_tower.c`, `frontier_util.c`,
`mystery_gift.c`, `mystery_gift_client.c`, and `mystery_event_script.c`.
Related script specials are registered in `game/data/specials.inc` and used by
the Sootopolis Mystery Events House and standalone FRLG Seven Island house
scripts. These paths do not reference `gMultiBootProgram_EReader_Start` and
remain outside `ENABLE_EREADER_TRANSFER`.

Wayfarer treats data already present in the Trainer Hill special sector as
unavailable. The separate visiting-trainer field keeps its existing validation
contract:

- `ValidateEReaderTrainer` reports `gSpecialVar_Result = TRUE` for an empty or
  invalid record and clears a record whose checksum is invalid. A valid record
  may continue to drive the existing visiting-trainer feature.
- Name, class, sprite, greeting, farewell, party-construction, and special
  battle helpers are called only after successful validation. If
  `FREE_BATTLE_TOWER_E_READER` removes the field in a separate configuration,
  validation must still report no record and the helpers must return empty or
  unavailable results. They must not return a value that scripts interpret as
  a valid trainer.
- The Sootopolis Mystery Events House and equivalent visiting-trainer hooks
  keep their no-visitor layout and dialogue when validation reports no record.
  An invalid record cannot open the battle room or reach
  `SPECIAL_BATTLE_EREADER`.
- Trainer Hill ignores e-Reader data in special sector 30. Challenge setup
  in `TrainerHillStartChallenge` does not call
  `ReadTrainerHillAndValidate` when transfer support is disabled. It uses the
  compiled `sChallengeData` and `sFloorData`; `GetInEReaderMode` and
  `OnTrainerHillEReaderChallengeFloor` remain false; the e-Reader palette is
  not loaded. `EReaderHandleTransfer`,
  `EReaderHelper_Timer3Callback`, `EReaderHelper_SerialCallback`, and the
  other transfer-only functions in `ereader_helpers.c` are compiled out.
  Ordinary Trainer Hill code in `trainer_hill.c` remains.

`SAVE_EREADER` in `game/include/save.h` and `game/src/save.c` is already a
deprecated alias of the ordinary link-save behavior. It may remain so callers
cannot fall through to an invalid save type; it contributes no user-facing
e-Reader route.

New games continue to initialize ordinary save state normally. Loading a
prerelease save does not clear the party, Bag, story progress, Trainer Hill
times, Mystery Gift data, or link records merely because e-Reader data is
ignored. The repository's prerelease save policy permits the stored data to
become unreachable without a migration.

### Preserved link behavior

The capability guards must not cover these shared systems:

- Cable Club scripts and `game/src/cable_club.c`;
- core link code in `game/src/link.c`;
- wireless link code in `game/src/link_rfu*.c`;
- trading in `game/src/trade.c`;
- Union Room code, including `union_room.c` and `union_room_battle.c`, and its
  maps;
- ordinary link-battle setup and `BATTLE_TYPE_LINK`; or
- the ordinary Mystery Gift initializer, client/server protocol, and saved
  Wonder Card or Wonder News handling.

This preservation requirement applies to both the Wayfarer build and a
standalone build that disables one or more legacy capabilities.

### Validation

Validation runs map-version builds sequentially because generated map files
are shared.

1. Build Wayfarer normally with `make wayfarer`. It must resolve the three
   capabilities to `0` and produce a valid ROM. Build release Wayfarer with
   `make BUILD=wayfarer release`; that build must also produce the ROM report
   and pass the active `0x09F80000` release ceiling.
2. Build Emerald, FireRed, LeafGreen, and HNS with their default capability
   values. Then build representative standalone configurations with each
   capability set to `0` independently and all three set to `0`. Every build
   must compile and link.
3. Inspect the Wayfarer map or symbol output. It must not define any of these
   pairs: `gMultiBootProgram_PokemonColosseum_Start`/`_End`,
   `gMultiBootProgram_BerryGlitchFix_Start`/`_End`, or
   `gMultiBootProgram_EReader_Start`/`_End`. It must also have no live
   Wayfarer reference to `CB2_InitBerryFixProgram`, `CB2_InitEReader`, or
   `GameCubeMultiBoot_ExecuteProgram`.
4. Inspect the standalone matrix per capability. A default build retains all
   three symbol pairs with spans of `0x28000`, `0x3BF4`, and `0x30E0`,
   respectively. Disabling one capability omits only its payload pair and live
   entry points; the other two pairs remain with their exact spans. Disabling
   all three omits all three pairs.
5. Compare `__rom_end` and the generated Wayfarer size report against the
   captured pre-change Wayfarer baseline that contains all three payloads. The
   total used-byte reduction, including the `other` category that owns the raw
   payload sections, must be at least 191,700 bytes (`0x2ECD4`). Record the
   total ROM delta and any additional code or graphics savings. Do not compare
   the padded `.gba` file length.
6. Add static coverage for the capability matrix, absent payload symbols,
   main-menu item count and action mapping, Berry chord removal, and the
   no-valid-e-Reader-trainer fallback. Cover the `HAS_MYSTERY_EVENTS` item with
   and without a Wireless Adapter and prove neither case selects
   `ACTION_EREADER`.
7. Boot Wayfarer through the copyright screen, title screen, and main menu.
   Exercise `B+Select`, clear-save, reset-RTC, Continue, New Game, and Options.
   The removed chord and GameCube activity must not divert startup.
8. Load saves with valid and invalid visiting-trainer records and a populated
   Trainer Hill special sector. Confirm that the valid visiting trainer keeps
   its existing behavior, the invalid record cannot open a battle room,
   Trainer Hill uses built-in challenges, and an ordinary save/reload
   preserves unrelated state.
9. Run `make BUILD=wayfarer check`, `make rom-report-test`, the existing
   Wayfarer static suites, and the E2E build and smoke suites. Add
   focused smoke coverage for an ordinary cable trade, an ordinary cable
   battle in a Cable Club Colosseum room, a Wireless Adapter battle or trade,
   and the currently supported wireless Mystery Gift receive/send flow. Where
   automated link-hardware coverage is unavailable, record a two-instance
   emulator or physical-hardware smoke result instead of omitting the check.

## References

- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)

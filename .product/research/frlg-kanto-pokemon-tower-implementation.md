# Lavender Pokémon Tower implementation review

The Wayfarer map selector uses the existing HNS radio station lobby as the ground floor and the FRLG Tower 2F–7F layouts as the memorial floors. The FRLG 1F layout stays unselected. The guard at `(17,4)` is the only Tower entrance: accepting sends the player to the clear 2F tile `(18,11)`, and declining leaves them in the lobby. The 2F downward stair returns to `(17,5)` in that lobby. The upper floor order remains 2F↔3F↔4F↔5F↔6F↔7F.

## Memorial content and actor placement

- The FRLG 1F Clefairy mourner's perspective moved to the former Fuji position `(11,11)` in the HNS Soul House. Her dialogue distinguishes memorials already moved to the chapel from a grave still upstairs.
- The Soul House visitor at `(8,15)` gives the disturbed-spirit warning and points to Celadon's Silph Scope. The 2F Channeler and Blue give shorter local Scope clues. These clues grant no Scope.
- Fuji's Soul House actor was removed. The existing House1 living Marowak remains a shelter resident, separate from the ghost. A single Fuji actor appears in House1 at `(8,4)` only after rescue, and the 7F actor hides on rescue. Both use his existing HNS appearance.
- The HNS radio lobby retains its radio director, host, reception, library, and services. Tower flags do not gate the Machine Part upgrade or grant the radio Poké Flute program.

## Floor events and state

The source stair tiles have FRLG behavior values `0xec` and `0xed`, which Wayfarer's step handler does not treat as warps. Elevation-3 coordinate events now send the player to inspected clear arrival tiles without changing any `map.bin`. The source upper-floor warp events remain in the maps alongside these working triggers; 2F's downward warp is redirected to the lobby. Each floor sets its Escape Rope destination to the radio lobby.

Tower item balls use normal item pickup scripts and persistent hide flags. Tower Channelers and Rockets use IDs 1779–1794, after the IDs reserved by the open Celadon Hideout and local adventures work. Their defeat state has a dedicated 16-bit save field. The Marowak story battle uses Wayfarer's wild-level projection; its calmed flag is set only after victory. Rocket hide flags, Fuji rescue, and the House1 physical Poké Flute claim are separate persistent states. An already-owned Flute settles the gift, while a full Bag leaves it claimable.

The Wayfarer wild table selects FRLG Tower land encounters on 3F–7F for day and night. Without a Silph Scope, the existing Tower battle setup presents ordinary wilds as unidentified ghosts. The [combined integration journey](kanto-port-integration.md) now verifies Celadon's actual Scope reward through Marowak, Fuji's rescue, and his physical Flute, including save/reload and independent radio progression.

## Emulator evidence

The results below describe standalone validation. The later [combined validation](kanto-port-integration.md) passed the complete selected Wayfarer native sweep (121 cases), 11 Tower-related native cases, and all 160 selected emulator cases, with two startup-aborted cases covered by a successful targeted retry.

The focused SkyEmu journey passed 13 of 13 checks on the E2E ROM. It covers early entry before Scope and radio repair, the lobby return and every floor edge, no-Scope ghost blocking, live wilds on 3F–7F, an ordinary Trainer and item, Marowak and Rocket loss/retry, the 5F healing space, full-Bag item retry, Fuji rescue and home staging, full-Bag and already-owned Flute handling, save/reload, and permanent post-rescue entry. The director's Machine Part condition and visiting-origin Blue interaction are included.

The E2E ROM build passed its map generation, source audit, trainer scaling, and symbol checks. Focused Wayfarer native tests pass for Tower defeat-bit isolation and daily reset persistence. The full Wayfarer native sweep was terminated before completion after both Tower cases passed; the earlier default Emerald native sweep completed with its existing known-failing and TODO categories. `git diff --check` and the tracked/untracked `map.bin` diff check are clean.

Rendered captures: [lobby before entry](assets/lavender-tower/lavender-tower-lobby-before-entry.png), [2F arrival](assets/lavender-tower/lavender-tower-2f-arrival.png), [lobby return](assets/lavender-tower/lavender-tower-lobby-return.png), [Soul House](assets/lavender-tower/lavender-tower-soul-house.png), [House1 before rescue](assets/lavender-tower/lavender-tower-house1-fuji-absent.png), [Fuji on 7F](assets/lavender-tower/lavender-tower-7f-fuji-before-rescue.png), and [House1 after rescue](assets/lavender-tower/lavender-tower-house1-fuji-returned.png).

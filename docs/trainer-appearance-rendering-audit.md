# Trainer appearance rendering audit

Wayfarer uses saved appearance for local gameplay displays. Intro and naming
previews use the setup context explicitly. Gender remains the story identity
and the fallback identity in legacy remote and recorded formats.

This is a source audit, not emulator evidence. See the task validation log for
builds, test runs, and captured screens. Screens and animations listed here still
need the spec's emulator acceptance checks.

## Local trainer pictures

`GetLocalPlayerFrontTrainerPicId` and `GetLocalPlayerBackTrainerPicId` resolve
saved appearance only in Wayfarer. Standalone targets retain their old gender
and build mapping. `PlayerGenderToFrontTrainerPicId` in `pokemon.c` and
`PlayerGenderToFrontTrainerPicId_Debug` in `trainer_pokemon_sprites.c` remain
explicit gender APIs; arbitrary actors must not use the local wrappers.

| Call site | Identity and behavior |
| --- | --- |
| `battle_controller_player.c:PlayerGetTrainerBackPicId` | Saved appearance for ordinary and live link battles. The local controller bypasses gender-only link records. Recorded flags retain the legacy path. Back picture selection supplies its matching animation table and palette through the existing shared drawing helpers. |
| `battle_controller_player.c:PlayerHandleDrawTrainerPic` | Facility tag battles select the saved front portrait. Recorded flags retain the legacy gender portrait. The multibattle test fixture keeps its explicit test actor. |
| `battle_controller_player.c:PlayerHandleTrainerSlide` | Uses the same selected back picture as initial drawing. |
| `battle_controller_player.c:PlayerHandleIntroTrainerBallThrow` | Uses the selected back picture's palette entry with the full trainer-picture ID in Wayfarer. |
| `battle_controller_safari.c:SafariHandleDrawTrainerPic` | Saved back picture. |
| `reshow_battle_screen.c:LoadBattlerSpriteGfx` / `CreateBattlerSprite` | Safari restores the saved back picture, animation template, and palette after a menu. Tutorial Wally/Old Man remains explicit. |
| `battle_controller_oak_old_man.c` draw, slide, and throw | Only the first-battle player branch resolves saved appearance in Wayfarer. The tutorial Old Man remains explicit. |
| `trainer_card.c:ShowPlayerTrainerCard` / `ShowTrainerCardInLink` | Tracks local ownership separately from link UI mode. A card opened for the local multiplayer ID uses saved appearance; remote cards keep legacy data. No transmitted card layout changed. |
| `trainer_card.c:CreateTrainerCardTrainerPic` | Local 64x64 portrait is drawn at (4,8) inside the 72x80 portrait window. This keeps every selected portrait within bounds independently of card edition. Remote Union Room classes and edition/gender cards retain their prior layout and assets. |
| `hall_of_fame.c:Task_Hof_DisplayPlayer` | Saved portrait for the induction screen; standalone retains the original debug mapper. |
| `hall_of_fame_frlg.c` trainer creation | Local wrapper, which preserves standalone FRLG mapping. |
| `battle_dome.c` individual and match info cards | Only `TRAINER_PLAYER` resolves saved appearance. Brain and other trainer IDs remain explicit. Tournament data stores the player sentinel; replay resolves the same immutable appearance from the save. |
| `pokedex.c` size comparison | Saved front picture. The existing silhouette treatment remains unchanged. |
| `battle_transition.c` player mugshot | Saved front picture in live gameplay; legacy gender picture for recorded playback. The gender-based background strip is a UI color theme, not character artwork. |

Hall of Fame records store teams, and the player portrait is resolved from the
current save when shown. Appearance cannot change during a save's lifetime, so
this preserves identity without extending the Hall of Fame format. New-game
save initialization clears old records.

## Field and small UI depictions

| Call site | Identity and behavior |
| --- | --- |
| `field_player_avatar.c:InitPlayerAvatar` / local graphics wrappers | Saved appearance; explicit gender/rival helpers keep their actor contract. Movement-state handling is audited and tested separately from graphics identity. |
| `event_object_movement.c:LoadPlayerObjectEventPalette` and field-effect callers | Legacy gender-only API remains explicit. Local callers resolve the selected appearance palette; see field integration changes. |
| `naming_screen.c:NamingScreen_CreatePlayerIcon` | Explicit pending context during setup and saved local appearance after setup. Rival, Pokémon, and box icon paths stay separate. |
| `oak_speech_hns.c` preview and restoration paths | Explicit pending appearance, including naming/challenge callbacks and final trainer display. |
| `decoration.c:SetUpPlacingDecorationPlayerAvatar` / `SetUpPuttingAwayDecorationPlayerAvatar` | Saved normal graphics with explicit `ANIM_STD_FACE_WEST` retain the decorating preview’s static left-facing pose. Normal and decorating descriptors both use 16x32 OAM. The normal table’s west animation repeatedly displays valid frame 2; the old decorating table repeatedly displayed its single left-facing frame 0. No decoration code requests further animations for this dummy-callback preview. Standalone decorating sheets remain unchanged. The putting-away cursor retains its separate `PLACE_DECORATION_PLAYER_TAG` palette and cleanup; the player preview loads the normal graphics descriptor’s own palette tag. |
| `cable_car.c` both player passenger creations | Saved normal graphics; other passengers keep explicit IDs. |
| `easy_chat.c` interview illustration | Saved normal graphics for the player facing the reporter. Reporter/fan artwork stays explicit. |
| `region_map.c:CreateRegionMapPlayerIcon` | Saved ID selects the existing matching Gold, Kris, Red, Leaf, Brendan, or May icon and palette before legacy build/gender handling. Icon dimensions, animation, zoom positioning, and tagged cleanup remain unchanged. Invalid saved identity creates no icon. |
| `frontier_pass.c:InitFrontierMapSprites` | The selected normal overworld sheet supplies its palette and first facing-down frame. A tagged 128-byte sheet copies the top half of the uncompressed 16x32 first frame into the existing 16x16 marker, with a copied palette released alongside the tiles. All six normal descriptors use this width and height. |
| `contest_util.c:GetContestTrainerGraphicsId`, contest map variables, and link palette loading | Local contestant index resolves saved normal graphics through a `u16` lookup and uses the corresponding palette. Other contestants use their existing record fields. `ContestPokemon.trainerGfxId` stays `u8`; creation and outgoing records keep legacy gender graphics. Both Contest Hall scripts skip their subsequent gender overwrite in Wayfarer. |
| `item_menu.c` / `item_menu_icons.c` | Art depicts the bag, not a trainer. Existing gender-based bag art and menu color theme remain unchanged. Wally's tutorial bag remains explicit. |
| `start_menu.c` save information and `berry_tag_screen.c` | Gender controls UI colors; no local character depiction is selected here. |
| `credits_hns.c` | Wayfarer's compiled credits display Suicune. The variable named player sprite is not a trainer. Its rival sprite is invisible. |
| `credits.c` / `credits_frlg.c` | Compiled only for standalone Emerald/FRLG respectively; their native protagonist scenes remain unchanged. |

## Legacy and explicit actors

`LinkPlayerGetTrainerPicId` is unchanged: it resolves only the supplied remote
record's gender/edition under the existing build rules. In particular, the HNS
legacy rule still maps to Gold/Kris. Outgoing `link.c`, RFU, Trainer Card,
secret-base, and facility exchange records retain their binary fields.

The recorded player, partner, and opponent controllers keep their original
identity inputs. They do not call the new local appearance wrappers. Live link
opponents and partners also keep their explicit link identity. Player-partner
AI controllers retain their supplied partner trainer or legacy AI test actor.

Catch-tutorial Wally and Old Man, rivals, NPC object graphics, frontier trainers,
Match Call contacts, Union Room visitors, and arbitrary trainer-picture APIs
retain explicit actor data. Gender-driven naming suggestions, rival selection,
story branches, OT metadata, and dialogue substitutions are not appearance
lookups.

## Focused tests

`game/test/wayfarer_appearance_rendering.c` checks all six saved local front and
back pictures while pending selection differs, verifies the explicit gender
mappers stay legacy, and exercises the actual local battle selector with live
link, ordinary, and recorded flags. It also checks that a remote gender-only
record does not borrow local appearance. These assertions do not substitute for
rendered screens or animation captures.

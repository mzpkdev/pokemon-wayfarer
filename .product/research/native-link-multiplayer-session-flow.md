# Native-link multiplayer: ordinary session flow

Status: the bounded session-flow experiments below passed on [draft PR #144](https://github.com/mzpkdev/pokemon-wayfarer/pull/144). This remains an off-by-default research experiment.

This batch starts at `eff36caeee3019af524d68a598e19b2280ae9d70`, following the [encounter and persistence experiments](native-link-multiplayer-followup.md). The user selected saving/reconnection, an in-game multiplayer menu, and focused independent-play stress. Standard emulator GUI and physical GBA compatibility work is deferred; earlier limitations remain unresolved. Rental parties, the fixed encounter, and the local Potion reward remain research fixtures.

## Session behavior

An ordinary save coordinates a deliberate transport shutdown, write locally, and reconnect automatically after the player returns to the field. Cancelling the save prompt leaves the session alone. Either player, including both at once, can save in the tested runs. The sampled peer-loss cases recovered control and allowed a valid local save. Explicit Leave cancels reconnect intent.

The in-game multiplayer menu exposes connection status, Connect, Leave, and cooperative invitations with explicit acceptance, decline, and cancellation. Actions close the menu before beginning field transitions. Mismatched built ROMs are rejected before presence or battle negotiation.

## Implementation

Session intent now outlives a temporary cable connection. After SAVE → YES, the saving game advertises a save request, both games use Emerald's native close handshake, and the local write waits until the cable transport is closed. A 240-frame close timeout falls back to a local close. Returning to an unlocked, settled field permits automatic reconnection; a 1,800-update retry deadline is checked between connection attempts. An attempt already underway can finish after that deadline. Choosing Leave clears that intent. These bounds count game updates, not wall-clock seconds during synchronous flash writes.

START → LINK exposes Connect, status, Leave, and invitations. Incoming invitations require an explicit Accept or Decline. An outgoing invitation can be cancelled. Each invitation carries an origin and sequence token, so a delayed reply cannot immediately accept a different invitation. Both players offering at once counts as mutual consent. The prototype still uses its fixed rental encounter; there is no campaign trainer selection or party negotiation.

The ordinary Start menu scrolls when its actions exceed eight visible rows. Invitation prompts wait until local field controls are available, hide the map-name popup, and require the player to release the buttons before a selection can register. Menu actions execute after their windows close.

Before enabling presence, the games exchange protocol version 3 and a 128-bit fingerprint. The build step computes SHA-256 over the final ROM with the fingerprint slot zeroed, stores the first 16 bytes in that slot, and verifies the marker is unique. This covers compiled content and configuration, not just a version string. Matching fingerprints are a compatibility check, not authentication. Only the `.gba` is stamped; an ELF remains useful for symbols but its zero fingerprint is rejected if executed directly.

Rewards remain local and independent. Battle transport closes before restoring each original party and attempting the existing local reward save. Reconnection waits until those operations finish. A pending reward can be retried from the menu while offline. This does not create a transaction spanning both cartridges.

## Validation record

All runs below use two independent unmodified mGBA 0.10.5 cores connected through its native serial driver. Fixture arrangement uses the existing E2E mailbox where stated; movement, menu choices, attacks, and native link traffic are not injected. Each evidence directory records ROM, ELF, runner, input, and library hashes.

| Case | Observed result | Evidence |
| --- | --- | --- |
| Ordinary Connect menu | Both players selected START → LINK → Connect and reached ACTIVE with matching fingerprints. | [Menu and diagnostics](native-link-poc-session/ui-connect-active/); [offline panel](native-link-poc-session/ui-connect-offline/player0-frame0400.png) |
| Two independent wild fights | Distinct original Pokémon attacked and took damage, each defeated its own Caterpie, then both returned to the field with reciprocal presence. One uninterrupted session attempt on each game. Wild entry and initial parties were E2E fixtures. | [Fight and return captures](native-link-poc-session/both-wild-fight-new/) |
| Repeated house travel with Bag overlap | Player 0 entered and left the house twice while player 1 used the Bag and remained outside. Both finished ACTIVE, visible, and with matching peer coordinates. | [Captures and diagnostics](native-link-poc-session/map-bag-overlap-new/) |
| Repeated Pokémon Center travel | Three ordinary door journeys into Cherrygrove's Center, including an overlap with player 1's Bag, ended ACTIVE with reciprocal presence outside. Initial map and position were E2E fixtures. | [Captures and diagnostics](native-link-poc-session/center-repeat/) |
| Release co-op menu journey | Ordinary Continue saves connected through LINK. Player 0 invited, player 1 accepted, both selected distinct moves, won, restored their exact original parties, saved one local Potion each, and automatically rejoined. No E2E code or runtime arrangement was present in this run. | [Battle, session, and independent flash audits](native-link-poc-session/release-coop/) |
| Decline and cancel | Declining an incoming invitation and cancelling an outgoing invitation aborted consent on both games without ending presence. | [Decline](native-link-poc-session/ui-decline-final/); [cancel](native-link-poc-session/ui-cancel-final/) |
| A held during invitation arrival | The incoming prompt remained pending while A was held across arrival. Only a later deliberate selection declined it. | [Visible pending prompt](native-link-poc-session/ui-arrival-held-a-final/player1-frame2100.png); [diagnostics](native-link-poc-session/ui-arrival-held-a-final/) |
| Either player saves; both save together | Each saving cartridge wrote a coherent, checksum-valid generation. All three runs resumed ACTIVE on both games after one coordinated close and a second connection attempt, with no manual reconnect input. | [Player 0](native-link-poc-session/save-p0/); [player 1](native-link-poc-session/save-p1/); [simultaneous](native-link-poc-session/save-both/) |
| Release ordinary saves | Two ordinary Continue saves connected through LINK, both saved, and automatically rejoined without E2E code or debug shortcuts. Both selected generation 2 with unchanged party/count, flags, variables, Frontier records, PC, and SaveBlock3 bytes. | [Release save flow and flash audits](native-link-poc-session/release-save-both/) |
| Save prompt cancelled | Both games stayed ACTIVE on their first connection, with zero closes and unchanged save counters. | [Cancellation trace](native-link-poc-session/save-cancel/) |
| Reciprocal, simultaneous, and deferred consent | Player 1 inviting, both offering at once, and an offer held while the recipient stayed in Bag all reached native battle without error. The Bag recipient returned and deliberately accepted. | [Player 1 invite](native-link-poc-session/p1-invites/); [simultaneous](native-link-poc-session/simultaneous-invites/); [Bag delay](native-link-poc-session/invite-while-bag/) |
| Actual mismatched builds | The identical release-ROM control connected. Release versus E2E ROMs, each with its own correct ELF addresses, both failed with BUILD_MISMATCH before presence or encounter traffic. No fingerprint bytes were injected. | [Matching control](native-link-poc-session/release-ui-connect/); [mixed-build rejection](native-link-poc-session/real-build-mismatch/); [status panel](native-link-poc-session/real-build-mismatch-ui/player0-frame3000.png) |
| Full Start menu | Unlocking the available Pokédex and Pokégear entries produced nine actions. The list scrolled to EXIT, returned to the field, and could navigate back to LINK. DexNav is compiled out in this configuration. | [Top](native-link-poc-session/max-menu-new-top/player0-frame0300.png); [bottom](native-link-poc-session/max-menu-new-bottom/player0-frame0380.png); [LINK](native-link-poc-session/max-menu-new-link/player0-frame0500.png) |
| Explicit Leave | Both players chose Leave, cleared session intent, removed peer presence, and moved independently afterward. | [Trace and diagnostics](native-link-poc-session/ui-leave-both/) |
| Cable removal during invitation | Both aborted the offer without starting a battle or granting a reward. The recipient's panel replaced Accept with reconnect status; Back closed it and normal movement worked. | [Recovery trace](native-link-poc-session/invite-interruption-recover-controls/) |
| Cable removal during ordinary Save | The saving cartridge wrote a coherent, checksum-valid generation and remained movable. Both games exhausted the bounded reconnect window and cleared session intent with a timeout error. | [Independent flash audit](native-link-poc-session/save-interruption-drop240-long/player0-audit-root.json); [session diagnostics](native-link-poc-session/save-interruption-drop240-long/session.json) |

A result-exchange cut at input frame 9,385 aborted on both games, restored the original parties, granted no reward, and left coherent unchanged saves. A nearby control at 9,400 occurred after agreement and deliberate cable closure; both games correctly completed their local reward saves. These samples show why a cable cut during result negotiation differs from a cut after the games have agreed to save. See the [interrupted exchange](native-link-poc-session/postbattle-interruption-drop9385/) and [post-agreement control](native-link-poc-session/postbattle-interruption-drop9400/). They do not establish atomic agreement or cover every timing. GUI and physical hardware remain outside this batch.


Build and source identities are retained in the [artifact summary](native-link-poc-session/builds/artifacts.json) and [build logs/manifests](native-link-poc-session/builds/).

## Build evidence

The tested E2E ROM is `f06860f08869f2c5f31d69aaf5aaa3d402fa2f7241f3ff63ecb6da667ae18779` (ELF `e38940f9302230daf3a6f23f4953bca31ce9688b1a42773d6f3b0a9b2c190493`). Its runtime fingerprint is `06d0b449e239c4b3e3a11b3061e443f0`. The release PoC ROM is `3b7a7450b1c32e04fd855cd286509b1838d4116bcf62d6121d223262f1645ebe` (ELF `bc093b4c40801b95547965f83424142c4e6248c93e044263382582597f1186a6`), with fingerprint `1041f1a1c40d69482351148978621af6`. The tags resolve into ROM space in both ELFs and verify against the emitted ROMs.

The disabled release rebuild remains byte-for-byte identical to the preceding PoC baseline: SHA-256 `a06269293728ee6ebbb9c632663292045c89b76d77e6e9f80be09352acb48099`. It contains no multiplayer PoC symbols or build-ID marker and passes the main-menu boot check. The new menu translation unit is excluded when the feature is disabled: even an empty extra object changed LTO output ordering, so compiling its body out alone did not preserve binary identity.

| Build | EWRAM | IWRAM | ROM before padding |
| --- | ---: | ---: | ---: |
| Disabled release | 249,413 B | 25,612 B | 31,731,080 B |
| Experimental release | 253,073 B | 25,612 B | 31,745,652 B |
| Experimental E2E | 258,992 B | 25,652 B | 32,119,040 B |

The experimental release leaves about 9 KiB of EWRAM. These build totals are not a stack-depth or runtime-memory safety proof.

## Remaining limits

- This is still an unmerged, disabled experiment. Standard emulator GUI linking and physical GBA/cable compatibility remain unresolved or untested; the user deferred that platform batch.
- Reconnection needs both games to become available within the bounded retry window. A missing peer or a player who stays busy past that window requires Connect again. No host migration, internet transport, RFU, four-player mode, or save-state rollback protocol was added.
- Co-op still uses one rental encounter and a one-time local Potion entitlement. Real-party eligibility, fainting/loss rules, captures, mixed-strength balancing, trainer/story credit, and divergent map state remain design work.
- Closing the cable for a save temporarily removes shared presence. Each cartridge owns its save; there is no atomic commit across both. Earlier partial-write and pending-reward experiments remain limited samples, and the flash driver's reported-error path has not been reproduced here.
- The tested routes and timing cuts are a bounded regression set, not a long-session, crowded-map, latency, runtime-memory, or performance certification. Fingerprints require identical emitted ROMs and do not authenticate peers.

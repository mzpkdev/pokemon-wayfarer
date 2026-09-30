# Multiplayer experience research for Pokémon Wayfarer

Research date: 2026-09-30. Wayfarer baseline:
`3e820fb584f3bb27cafb2ee66987e575107699dc`.
Status: research and proposed experiments. Real GBA and standard emulator
compatibility are required; gameplay policy and implementation remain undecided.

The objective is the best multiplayer experience Wayfarer can deliver: friends
can join easily, explore with independent controls, participate meaningfully in
battles, understand who earns progress, and recover from interruptions without
losing their saves. Quetzal is one feasibility example; reproducing its
implementation or finding its ancestry is not the objective.

The strongest direction is to put explicit multiplayer behavior in Wayfarer's
game source and separate it from connection transport. Public projects supply
useful parts: native overworld/co-op battle integration, remote actor movement,
emulator memory bridges, relays, and turn-based session control. No inspected
project establishes the complete experience for Wayfarer. Reuse should follow
behavior and code review, not project branding or feature-list similarity.

The required core is native ROM multiplayer over the ordinary GBA link
interface, playable on physical GBAs and standard link-capable emulators.
Internet connectivity can be supplied by compatible emulators or optional
hardware bridges. A companion app, custom emulator, or hardware network adapter
cannot be mandatory for the core co-op game. Wireless-adapter support is a
separate compatibility target, not an automatic consequence of cable support.

## What a good experience means

The hardware/emulator constraint is confirmed. Other criteria below are proposed
ways to evaluate the experience, not approved gameplay requirements.

| Player outcome | What to evaluate |
| --- | --- |
| Joining friends is straightforward. | Clear build compatibility and useful errors; simple connect flow on hardware. Online room/invitation support belongs to compatible emulator or optional bridge UX. |
| Everyone remains an independent player. | Own trainer, camera, controls, menus, party, and save; one player's menu should not routinely freeze another's exploration. |
| Playing together changes the game. | Opt-in cooperative trainer encounters and useful interactions, beyond seeing a moving avatar. |
| Progress is understandable. | Explicit rules for badges, story events, captures, items, challenge modes, and rewards when players have different saves. |
| Interruptions are recoverable. | Busy/disconnected status, reconnect at a defined boundary, and a clear outcome if interruption happens during a reward or trade. |
| Supported devices work consistently. | Prove ordinary cable play on real GBAs and selected standard emulators; test each promised online path, mobile backgrounding, input, and save export separately. |

Two-player presence is an engineering milestone. It should not be presented as
the final co-op experience. A better first playable slice combines a short shared
journey, one cooperative battle, individual rewards, and disconnect/rejoin.

### Different meanings of multiplayer

| Model | What players share | What it does not establish |
| --- | --- | --- |
| Presence / ghosts | Location, appearance, movement, status. | Shared battles, collision rules, or progression. |
| Session co-op | Presence plus explicitly shared encounters and interactions. | Automatic agreement on every quest or long-term world state. |
| Shared campaign | Agreed world events and progression across sessions. | A scalable public MMO or secure player economy. |
| Asymmetric participation | A guest controls an opponent or another role in one running game. | A second independent trainer and save. |
| Multiworld | Separate games exchange progression events or items. | A shared overworld or live co-op combat. |
| Hosted emulation / streaming | One host runs game instances and guests send input. | Local ownership of each guest's runtime, latency-free control, or distributed state replication. |

Use these distinctions when reading “multiplayer” claims in a README. Transport,
gameplay, and persistence can each be implemented while the others remain partial.

## Evidence and confidence

This investigation inspected public source and documentation, plus Wayfarer's
current source and product documents. It did not run Quetzal, establish a network
session, benchmark latency, or test multiplayer on physical GBAs. Source-level
observations establish mechanisms and integration risks, not gameplay stability.

External source links below pin the revisions inspected. Quetzal's website and
issue discussions remain live references. Unmerged Wayfarer task branches are
outside this baseline; approved designs are identified separately from code.

## Quetzal as a feasibility reference

The [project website][quetzal-site] advertises shared exploration and cooperative
trainer battles. gpSP's maintainer [confirms link-cable support][gpsp-confirmation];
the discussion reports up to four players. Quetzal's custom multiplayer source
was not located and its [multiplayer guide][quetzal-multiplayer] was unfinished
at inspection. Its exact shared-progress, latency, and recovery behavior remains
unverified here. Lesson: the desired experience is feasible in an Emerald-derived
ROM, but those public claims do not provide an implementation contract.

## Public implementations worth studying

| Priority | Reference | Use for Wayfarer |
| --- | --- | --- |
| Core implementation study | Tag Team Trials; existing Emerald/Wayfarer link code | Native remote actors, commands, and cooperative battle transitions. |
| Compatibility study | gpSP; GB-Link Netplay Bridge | Preserve working serial framing and verify optional hardware/emulator internet routes. |
| Design references | Emerald Rogue; Wired-Ariel | State separation, movement presentation, map transitions, resource ownership. |
| Experience and recovery references | Archipelago; PokéPVP; mGBA Splitscreen | Durable outcomes, controller roles, guest onboarding and hosted-play tradeoffs. |
| Alternative prototypes | GBA-PK; PokemonCoopGBA; Joybus net demo | Learn from their integration and deployment limits; do not make them core dependencies. |

Priority reflects relevance to the confirmed platform constraint, not a claim
that the code is production-ready. Each entry below separates source evidence,
reported behavior, limits, and a transferable lesson.

### Tag Team Trials and native co-op inside the ROM

[Viperio19/pokemon-tag-team-trials][ttt-repo] is the closest newly found
implementation to Wayfarer's platform requirement. Its current README largely
retains expansion boilerplate; the useful evidence is in source, not that README.

In [link.c][ttt-link], the project carries a player command and three arguments
inside existing `LINKCMD_SEND_HELD_KEYS` messages. Its
[command definitions][ttt-commands] cover movement, surfing, jumping, position,
objects, and flags. [overworld.c][ttt-overworld] consumes the commands, separates
instant actions from movement, and requests position synchronization when
returning to the field. This provides a concrete way to extend the existing
link protocol without moving gameplay into an emulator script.

The [player-two scripts][ttt-scripts] use existing link setup and multi-battle
entry points. They close the exploration link and reconnect for battles, and
require both players to agree to disconnect before saving. The scripts include
cable and wireless branches; that is source-level routing, not a verified
hardware or emulator compatibility matrix.

[cable_club.c][ttt-cable] explicitly requests two participants for its overworld
link and cooperative multi-battle setup. That is particularly relevant to
Wayfarer's existing four-participant multi-battle assumption described below.

The limitations are substantial and visible:

- Partner lookup uses `GetMultiplayerId() ^ 1`, making this a two-player design.
- The command queue has ten slots. Its enqueue function returns no overflow
  result when full. This is an inferred command-loss risk, not a reproduced
  failure; backpressure and reconciliation need investigation.
- Movement commands are skipped outside the overworld callback, while instant
  commands can still execute. Position resynchronization on returning to the
  field is therefore important; independently opening menus needs explicit tests.
- The scripts prevent leaving the demo while connected and explicitly end at a
  multiplayer demo boundary. General region travel and a full shared campaign
  are not established.
- Disconnecting for saves and mode changes exposes session transitions that
  Wayfarer needs to make predictable and recoverable.

Lesson: study this first for native actor/control integration and co-op battle
entry. Port a reviewed set of mechanisms, not the demo's map restrictions, raw
flag replication, or progress model. Retain the existing link transport where
possible and add a versioned Wayfarer session contract. No physical-hardware
test was performed here, and no top-level license declaration was found in the
inspected repository tree; clarify custom-code reuse before importing it.

### Native Emerald link communication

[pret/pokeemerald][pret-link] exposes the original game's communication code.
Its [overworld implementation][pret-overworld] includes linked-player spawning,
movement, collision handling, and graphics. This is the closest foundation for
a hardware-compatible approach, but it does not supply open-world co-op rules.

On the emulator side, [gpSP's serial protocol implementation][gpsp-protocol]
recognizes the Pokémon handshake and exchanges a checksum plus eight 16-bit
payload words. It simulates portions of the serial exchange to tolerate network
delay; it is not simply forwarding every electrical transition. The inspected
implementation uses a 240-frame peer timeout, described in source as about four
seconds. That is a gpSP behavior, not a universal Quetzal timeout.

Current [gpSP core options][gpsp-options] include a Pokémon Gen3 link-cable mode.
Older guides describing gpSP as wireless-only are therefore outdated. A Wayfarer
protocol change would still need compatibility testing against this
Pokémon-specific emulation; cable support does not guarantee arbitrary custom
serial protocols work online.

### Emerald Rogue and Rogue Assistant

Rogue separates in-game multiplayer logic from a desktop networking companion:

```text
Rogue ROM state <-> mGBA Lua memory bridge <-> Rogue Assistant
                                               |
                                          ENet network
                                               |
Rogue ROM state <-> mGBA Lua memory bridge <-> Rogue Assistant
```

Useful entry points in [rogue_multiplayer.c][rogue-game] are
`RogueMP_MainCB`, `RogueMP_OverworldCB`, `WritePlayerState`,
`ObservePlayerState`, and `Host_HandleHandshakeRequest`. Player state includes
map, coordinates, facing, movement history, and a follower. The handshake
checks game flavor, save version, and revised mode. It assigns the joining
player ID 1 with a TODO for slot allocation, so this code is not evidence of
completed arbitrary-player support.

[MultiplayerBehaviour.cpp][rogue-network] supplies host/client connections and
broadcasts through ENet. Profiles and game state use reliable packets; frequent
player-state packets use an unreliable mode. The
[Lua bridge][rogue-bridge] services reads and writes of emulated memory on frame
callbacks. These are concrete examples of keeping networking outside the ROM
while exposing a defined in-game state area.

The [state structures][rogue-state] contain Rogue-specific hub, adventure,
outfit, and progression fields. They need redesign for Wayfarer. This is an
architectural reference, not a library import. The Lua/native companion also
introduces packaging and emulator requirements that a standalone cartridge
cannot satisfy.

Lesson for Wayfarer: separate frequently refreshed presence from confirmed
interactions and persistent outcomes. An [open desync report][rogue-desync]
describes stopped avatars and the host returning to a port prompt; that is a
user report, not a reproduced defect. Native macOS/Linux build requests also
show why companion distribution becomes part of the support burden. No
top-level license for the companion's own code was found in the inspected tree;
an [open issue][rogue-license] requests one. Treat it as publicly readable
reference code pending clarification of reuse terms.

### GBA-PK Multiplayer

[GBA-PK's Alpha 4 client][gbapk-client] uses emulator Lua, TCP sockets, direct
memory access, and ROM-specific address tables. It includes Emerald mappings,
`GetPosition`, `ReceiveData`, `SendData`, and `Render`, along with battle and
trade handling. The code configures eight players and an eight-player render
limit; its comment warns that exceeding the latter can cause tiling issues.
These settings are not a tested scalability result.

The [README][gbapk-readme] still describes Alpha 3 and FireRed/LeafGreen, so it
does not fully describe the inspected code. Wayfarer's altered memory layout
and mechanics make the stock address mappings unsuitable without adaptation.
Study its memory-to-network-to-render flow; do not assume stock Emerald support
means Wayfarer support. Its [license file][gbapk-license] identifies
CC BY-NC 4.0; account for its stated terms before copying code. No reuse-license
audit of the other projects was performed.

Lesson for Wayfarer: ship one identifiable multiplayer build and surface
compatibility before connecting. In [issue 109][gbapk-artifacts], a reported
Emerald battle failure was resolved by downloading the release client/server
assets instead of the source files. It should not be cited as proof that all
Emerald battles are broken. [Issue 110][gbapk-connect] reports local success but
failed remote setup, and [issue 85][gbapk-underwater] reports underwater rendering
failure. These are useful acceptance scenarios, not verified current failures.

### Native Emerald payload with a hardware network adapter

[Wired-Ariel/gen3-poke-multiplayer][ariel-repo] contains a native C payload,
serial driver, mGBA integration, and network relay. Its README reports real-GBA
support through a Raspberry Pi Pico adapter, up to four players, overworld
movement, and Cable Club interactions between physical systems. Those runtime
claims were not reproduced here.

The inspected [payload/main.c][ariel-payload] has remote-event consumption,
mailbox handling, and overworld callbacks; [payload/sio.c][ariel-sio] operates
the GBA serial interface with the adapter as master. Its README describes
movement events plus periodic position corrections. This is an additional
reference for game-native remote actors, though its payload injection,
retail-ROM addresses, and adapter protocol would need substantial adaptation
for Wayfarer. It is not the ordinary two-cartridge link arrangement.

The README lists unresolved crashes after Record Mixing and Berry Blender,
and says the emulator script cannot participate in Cable Club. Ordinary linked
trades/battles do not establish cooperative NPC battles or shared progression.
Inspect and test the required feature rather than adopting the whole project's
compatibility claims.

Lesson for Wayfarer: update remote actors in the normal game loop, budget their
graphics alongside existing effects, and make map transitions explicit. The
project documents a Surf/HM palette corruption fix after remote status graphics
consumed the remaining palette slot. Its custom adapter and injected retail-ROM
payload are optional hardware-online references, not a replacement for ordinary
GBA-to-GBA cable support.

### Modular emulator co-op framework

[MehdiSenhajYnov/PokemonCoopGBA][coop-repo] describes an experimental alpha,
primarily targeting Run & Bun, with an Emerald US profile. Its documented
architecture is mGBA Lua plus a Node.js TCP relay. It offers ghost movement,
sprite rendering, cross-map projection, and PvP buffer synchronization described
as GBA-PK-style. The inspected `client/network.lua` implements socket and
reconnection handling.

This is another modular companion/overlay reference, not evidence of native
link-cable co-op or a Quetzal dependency. The README and networking module were
inspected; its battle correctness and claimed compatibility were not tested.

Lesson for Wayfarer: separate transport, ROM access, rendering, and battle
interaction modules. Cross-map projection and depth sorting need their own
tests. A modular wrapper is easier to investigate, but a ROM-integrated solution
can avoid its dependency on guessed memory addresses and direct OAM injection.

### PokéPVP and asymmetric guest participation

[Lauchgestalt/poke-pvp][pvp-repo] lets a browser guest control opposing trainers
in one Emerald session. Its [Lua client][pvp-client] hooks battle decisions
through mGBA breakpoints and fixed ROM addresses. The current
[relay code][pvp-server] validates message shapes and requires a controller
token while allowing other sockets to spectate. An older indexed README said
there was no authentication; the inspected code supersedes that claim.

This gives the guest a useful role without synchronizing two battle simulators,
but supplies neither an independent trainer nor a shared overworld. It requires
a scripting/breakpoint-capable emulator and therefore fails Wayfarer's core
platform requirement. Lesson: distinguish spectators from controllers and
present explicit turn choices; borrow those interaction ideas for native co-op.

### Hardware internet bridges

[GB-Link Netplay Bridge][gblink-repo] connects a real GBA through a USB adapter
to RetroArch/gpSP peers. Its [bridge implementation][gblink-code] translates
adapter exchanges to Pokémon eight-word rounds and gpSP's MPK1 protocol. The
README documents direct, lobby, and relay connections, and requires the explicit
Pokémon Gen3 cable mode for Emerald because automatic selection uses wireless.

This is a promising optional way to connect hardware and emulator players.
It does not add shared exploration or prove Wayfarer compatibility. Lesson:
retain recognizable link framing where practical, and test emulator mode and
bridge behavior as part of the supported configuration.

[KittyPBoxx/pokeemerald-net-demo][joybus-repo] takes another hardware route:
[net_conn.c][joybus-code] connects a modified GBA ROM over Joybus to a Wii
channel that relays IPv4 TCP traffic. Its documentation describes four-byte
transfers, a 4 KB Wii buffer, and non-simultaneous send/receive behavior.
Requiring a Wii or corresponding emulator setup makes this unsuitable as the
core Wayfarer experience. Lesson: hardware internet access is possible, but
custom adapters, flow control, and deployment costs must remain explicit.

### Hosted linked emulation in a browser

[Spuds0588/mgba-splitscreen][splitscreen-repo] has implemented online code in
[online.js][splitscreen-code]: the host runs all linked mGBA/WASM instances,
guests send inputs, and each receives its own screen/audio. Its README's future
online description is stale at the inspected revision. The source uses capped
JPEG frame delivery and bounds queued media; comments describe severe backlog
with earlier raw-frame transport. Six-digit room codes remain a
[design document][splitscreen-invites]; token invitations are implemented.

The benefit is keeping emulated cable synchronization local to one host. Costs
include input/video delay, host CPU and bandwidth, host-dependent availability,
and guest save custody. It is an optional play frontend, not a native-ROM
multiplayer implementation. Lesson: discard stale presentation updates, and
distinguish an actual join flow from an attractive unimplemented design.

### Archipelago Emerald and durable progress delivery

[Archipelago's Emerald client][archipelago-client] connects a modified game to
a multiworld service through BizHawk. Its code uses guarded memory operations
and indexed received-item handling; its [game documentation][archipelago-docs]
describes synchronizing progress on reconnect. Players occupy separate worlds,
so this does not implement shared overworld movement or co-op battles.

Lesson for Wayfarer: persistent outcomes need identities and a way to reconcile
what was received. A transport acknowledgment is not proof that a reward was
saved. Native co-op must implement its own receipt/recovery policy; copying this
client would introduce a mandatory emulator/service dependency.

Its [changelog][archipelago-changelog] also documents a window in which resetting
before a Wonder Trade receipt can lose the Pokémon. That report was not
reproduced here. It demonstrates why a shared-progress feature needs a crash
recovery contract in addition to a successful online exchange.

## Lessons that should shape Wayfarer

These recommendations synthesize the implementations above. They are not claims
that every referenced project already meets them.

1. **Use game-owned remote actors.** Source access lets Wayfarer create normal
   objects rather than patch retail-ROM addresses or inject graphics from Lua.
   Define ownership, collision, map membership, and despawn rules so a remote
   object cannot replace an NPC or survive into the wrong scene.
2. **Separate presence from consequential interactions.** Movement can tolerate
   delayed presentation and correction. Battle choices, trades, and rewards
   need ordered state transitions and explicit completion. A busy status must
   not accidentally become a movement or map update.
3. **Design the whole session lifecycle.** Use explicit connecting, exploring,
   requesting interaction, entering battle, returning, reconnecting, and
   disconnected states. Bound waits and release script locks on failure. Include
   a session identity so late messages from a previous session cannot act on a
   new one. Host failure should produce a recoverable exit before attempting
   seamless host migration.
4. **Agree on encounter inputs before entering battle.** Freeze trainer identity,
   teams, field conditions, difficulty policy, and each participant's role.
   Reuse Wayfarer's battle simulation and link controllers where possible.
   Reimplementing its mechanics in a separate server would create another
   engine to keep consistent with the ROM.
5. **Make progress policy visible.** Decide which events remain personal and
   which are shared. A guest should know whether helping earns a badge, only
   battle rewards, or no permanent credit. Never equate “same room” with permission
   to replace another save's flags.
6. **Treat save recovery as part of gameplay.** Record the identity and status of
   consequential outcomes before retrying them. Two consoles can fail between
   separate writes; IDs and acknowledgments alone do not make those writes
   atomic. Pick a recovery policy and test every interruption point. Trading
   needs stronger ownership-transfer rules than ordinary co-op rewards.
7. **Support exact configurations.** Match ROM/protocol/content identities,
   reject incompatible peers clearly, and publish known working emulator modes.
   Successful local linking does not establish remote connectivity, nor does
   one emulator's netplay establish cross-emulator interoperability.

For the first co-op slice, propose independent personal saves with opt-in shared
encounters. This is a manageable starting policy, not a substitute for eventually
deciding shared campaign progression. Full synchronized story play remains an
explicit research/design milestone rather than disappearing from scope.

## Architecture options for Wayfarer

These are feasibility judgments, not accepted requirements.

| Approach | Main advantage | Main cost | Suitable use |
| --- | --- | --- | --- |
| ROM multiplayer over GBA link | Keeps transport inside the existing hardware interface. | Limited session size; timing and emulator protocol constraints; substantial ROM work. | Cartridge-compatible co-op and compatible link emulators. |
| ROM multiplayer with emulator companion | Exchanges explicit state over ordinary networking; fewer serial timing constraints. | Companion is required, so it does not satisfy the core platform requirement. | Optional tooling and architecture lessons. |
| Lua overlay around existing ROM behavior | Fast route to observing and displaying remote players. | Address fragility, emulator dependence, and growing battle/save complexity; fails the core platform requirement. | Disposable experiment or source reference. |
| Dedicated authoritative server and custom client integration | Can define stronger persistence, validation, and larger-session rules. | Much larger platform project; requires an additional hardware bridge or client and changes offline ownership. | Reference for transaction and session design, not the core implementation. |

Native link is the selected architectural direction for research because it
meets the confirmed platform constraint. This does not mean every emulator can
support it, nor that different emulator brands can connect to each other online.
Maintain a tested compatibility matrix. Existing gpSP Pokémon protocol emulation
is promising, but Wayfarer's changed commands and session transitions must be
tested with it explicitly.

Standard-emulator compatibility concerns emulating the link interface. It does
not imply that a Wayfarer player can join an unmodified Emerald game or a
different ROM hack: both peers need to understand Wayfarer's protocol and content.

### Proposed separation of responsibilities

```text
Wayfarer gameplay: remote actors, encounters, rewards, save policy
                         |
Versioned session messages and explicit lifecycle
                         |
Existing GBA link framing and hardware interface
             /                              \
Physical cable and GBAs            Standard link-capable emulators
                                           |
                               Optional emulator internet transport
```

The ROM should own the same co-op rules on every platform. Emulator networking
should not be required to understand Wayfarer's story flags or rewrite its save.
Keep ordinary Pokémon link framing where practical to reduce compatibility risk;
that recommendation still needs a gpSP/native-link experiment.

Proposed authority model for trusted friends: each game owns its player's
movement and personal save; one elected encounter host supplies a frozen battle
description; both acknowledge the outcome and their own reward decision. A relay
that forwards bytes is not an authoritative game server. Offline-owned saves
also do not provide a cheat-resistant public economy. Host migration and
competitive enforcement are separate features, not consequences of connecting.

## Wayfarer integration risks

### Existing multiplayer is a foundation with separate rules

Wayfarer inherits link and multi-battle machinery, but shared exploration,
player identity, and cooperative NPC battles need their own integration.
The [link header][local-link] sets `MAX_LINK_PLAYERS` to four and exposes
`SendBlock`, `IsLinkTaskFinished`, and `CB2_LinkError`. In
[overworld.c][local-overworld], `SpawnLinkPlayers` places the participants
around one camera focus and `CB1_OverworldLink` exchanges held-key inputs.
General exploration needs map-aware remote state and lifecycle handling beyond
that existing shared-room behavior.

There is also a concrete two-player battle trap:
[battle_controllers.c][local-controllers] expects four link participants when
`BATTLE_TYPE_MULTI` is set, except for the Battle Tower path, which expects two.
Simply adding multi-battle flags can leave the game waiting for absent players.
The existing Battle Tower route is a useful two-human-versus-AI reference;
normal trainer co-op still needs explicit controller and party ownership.

In particular, [trainer scaling][local-scaling] excludes both
`BATTLE_TYPE_LINK` and `BATTLE_TYPE_INGAME_PARTNER`. Adding one of those flags
to a normal trainer encounter would change which scaling policy applies.
Co-op needs an explicit encounter policy: whose Trainer Rating sets difficulty,
who supplies each party, and which players receive rewards.

The existing linked-player renderer in [overworld.c][local-overworld] chooses
graphics using game version and gender. The
[appearance specification][local-appearance] preserves legacy link formats.
Wayfarer-specific appearance replication therefore needs a versioned extension
or a deliberate fallback appearance.

The current engine allows 16 object events
([constants/global.h][local-constants]) and 64 software sprite slots
([sprite.h][local-sprites]). Remote players, followers, map NPCs, weather, and
effects compete for constrained resources. Measure populated maps and transitions;
do not extrapolate capacity from an empty route. Prefer graceful omission of
cosmetic followers to dropping a connected player, subject to an explicit budget.

### Progress belongs to different players

Two saves may disagree about regional story state, defeated trainers, unlocked
travel, and league eligibility. A moving remote sprite does not reconcile those
differences. Sending raw save blocks or applying every received flag locally
would make unrelated progress changes possible.

This is especially relevant to Wayfarer's [regional save records][local-save]
and [flag/variable routing][local-flags]: common script IDs can resolve to
Hoenn-specific storage based on the current map. An unqualified flag update
received while the other player is in a different region can refer to different
state. Any later progression protocol should identify the region and event
semantics explicitly. Remote-map entry must also respect
[regional initialization and recovery][local-persistence], rather than only
copying coordinates.

Proposed initial policy: personal saves retain ownership of party, inventory,
badges, Trainer Rating, and story completion. Presence sharing does not grant
the visitor the host's progress. Shared battles later use a frozen encounter
description and a deliberate per-player reward decision. This policy is a
proposal requiring product agreement before implementation.

### Matching seeds do not synchronize a game

The [playthrough seed framework][local-seed] is explicitly parked and
unimplemented at this baseline. There is no existing seed API to adopt.
Even after implementation, a common seed would not reconcile different player
inputs, RNG call counts, quest state, or timing. Shared generated content needs
an agreed encounter identity and resolved inputs, or authoritative results.

The [league design][local-leagues] is also marked unimplemented at this
baseline. Future invitations, frozen lineups, hall conditions, and champion
records need an explicit multiplayer ownership policy. Do not treat a planned
NPC tag partner as an interchangeable remote human controller.

## Proposed first implementation boundary

Start with two players and an exact matching Wayfarer build, using ordinary
link hardware and at least one standard emulator. Four-player support is a
later measured goal within the existing link ceiling. The following is a
proposed experiment scope,
not a promise of the eventual feature set:

1. Connect after both games reach a safe overworld state. Exchange protocol and
   content identities before accepting gameplay messages.
2. Replicate map identity, position, facing, movement state, and a conservative
   appearance. Keep remote actors separate from persistent map NPCs.
3. Render peers only in compatible map contexts. A player entering another map
   removes the remote actor cleanly; it does not force the other player to warp.
4. Preserve each player's local menus, encounters, and story state. Represent a
   busy player explicitly rather than making the entire world wait for a menu.
5. Handle cancellation, disconnect, and return to solo play without a stuck
   script lock, phantom actor, or save mutation caused by presence updates.

If this works, add one opt-in shared trainer battle in a controlled location.
Synchronize the encounter definition, party selection, battle inputs/results,
and completion acknowledgments. Persist a per-player result identity if rewards
must survive retry/reconnect without duplication. This does not by itself solve
atomic commits across two disconnected save files; recovery rules still need
to define what happens if only one side saves.

Trading should be a later transaction-focused slice, because transferring
ownership of a Pokémon across saves is materially different from synchronizing
movement. Full story co-op should follow only after explicit rules for quest
participation, cutscenes, personal prerequisites, and rewards exist.

For synchronized battles, propose disallowing unilateral rewind/save-state load
and unsupported fast-forward combinations. These actions can move one participant
to a different history; ordinary in-game saves still need to work. A ROM cannot
reliably police every emulator feature, so unsupported actions need clear recovery
behavior and documentation. Independent exploration should not require permanent
frame-by-frame agreement on unrelated RNG or local menus.

## Experiments and acceptance evidence

| Experiment | Evidence needed to proceed |
| --- | --- |
| Existing link baseline | Two physical GBAs establish an ordinary cable interaction with the same build. Repeat on a standard emulator; record all versions and cable/player setup. |
| Presence prototype | Independent movement, map exit/re-entry, busy states, and orderly disconnect; measure frame time, memory use, and visual delay. |
| Network disruption | Delayed, lost, and interrupted traffic cannot hang scripts or apply stale messages after reconnect. Test save-state loading and fast-forward as separate compatibility cases. |
| Divergent saves | Different story flags and ratings cannot silently change the other player's save or expose unusable traversal. |
| One co-op encounter | Both players agree on opponent, field conditions, outcome, and their own rewards; retry/disconnect cannot duplicate completion. |
| Device matrix | Repeat the full journey on two physical GBAs, one local emulator setup, and each promised remote/phone setup. Optional adapter-to-emulator play is a separate target. |

Measure ROM/EWRAM/IWRAM overhead on the actual Wayfarer build. The prototype and
release must respect the [existing ROM budget][local-runtime]: a 32 MiB ceiling
and, during Hoenn development, at least 512 KiB reserved (`__rom_end` no greater
than `0x09F80000`). The source audit does not establish spare memory or acceptable
bandwidth. If the native transport
cannot meet the chosen online behavior, improve or narrow that online mode
without making a custom client mandatory for the core game.

For emulated internet tests, include controlled latency and loss, jitter,
brief outages, and long idle periods. For hardware, test cable removal during
exploration, menus, battle entry, battle results, and save/recovery, plus crowded
maps, surfing, doors, region changes, and different player progress. Record
observed join time, input response, visible remote delay, battle-transition
time, and recovery outcome. No latency, bandwidth, or memory result has yet
been measured for Wayfarer multiplayer.

The current [E2E harness][local-e2e] launches isolated SkyEmu instances and
provides screen, input, stepping, and memory operations through its
[HTTP client][local-skyemu]. No link-session control API was found in that
harness. Existing tests can help validate state transitions, but they do not
establish a working two-emulator connection. Add an explicit transport test
setup before claiming multiplayer E2E coverage; this finding does not establish
that SkyEmu itself is incapable of linking.

Wayfarer's repository policy does not require compatibility migrations for
prerelease saves. Keep any necessary multiplayer persistence changes focused
on the selected behavior rather than preserving obsolete prerelease layouts.

## Guest progression proposal and unresolved gaps

The working product proposal is “travel together, keep your own adventure.”
Joining should allow lasting progress in the guest's own save. The session host
coordinates communication; hosting does not grant ownership of the other save.
These rules are discussion proposals, not adopted requirements:

| Activity | Proposed personal result |
| --- | --- |
| Explore, catch, train, or spend items | Keep the player's own discoveries, Pokémon, experience, and inventory changes. |
| Complete an explicitly supported cooperative story encounter | Credit every participating player whose own prerequisites permit completion. |
| Help with an already completed encounter | Preserve completion without repeating its unique reward. |
| Join someone further along a story | Do not copy missing prerequisites or earlier story completion. Initially require compatible quest states for shared story encounters. |
| Challenge a Gym or league | Initially retain personal qualification and completion while connected; whether these should become cooperative remains an open product decision. |

For example, a player who already cleared a hideout could help a friend complete
their available encounter. The friend earns their own completion and reward;
the helper's save stays complete and receives no duplicate unique reward. If
both players are eligible and unfinished, one shared completion should count
for both. This example requires an authored cooperative encounter; it does not
establish that arbitrary existing scripts can be shared.

The next investigation should resolve these gaps:

1. Prove that native linking permits one player to use menus, change maps,
   enter a solo battle, or save while the other continues. Source inspection
   has not established all of these behaviors.
2. Specify conflicting map states: doors, collision, NPC visibility, puzzles,
   and cutscenes. Personal progress alone does not define a coherent shared
   scene. Explicit shared encounters may need their own temporary scene state.
3. Walk through two newcomers, a veteran helping a newcomer, and players from
   different regions. Define eligibility and credit for each, including whether
   personal Gym and league challenges make co-op too peripheral.
4. Define mixed-strength parties, cooperative difficulty, fainting, item use,
   capture ownership, and incompatible challenge options. Connection must not
   permanently raise another player's Trainer Rating or derived progression.
5. Exercise interruption before and after each completion/reward/save step.
   Specify recovery when only one cartridge records the outcome.
6. Measure ROM/RAM cost, crowded-map behavior, and exact emulator compatibility;
   clarify reference-code reuse terms before importing code.

A separate disposable proof of concept should test native two-player presence,
independent menus and map transitions, and disconnect recovery first. Record
unsupported behavior as an experimental result. Follow with one shared encounter
and persistent reward only after the transport and lifecycle are demonstrated.
Keep the experimental branch separate from this research document and from
production feature approval; a draft PR is an experiment record, not a merge
recommendation. Physical-hardware validation remains necessary even if emulator
tests pass.

## Decisions needed before an implementation specification

- Which standard emulators and online combinations will be officially supported?
  Physical GBA and ordinary emulator compatibility are required; a custom client
  is not an acceptable core dependency.
- Is the initial experience shared exploration, cooperative battles, or a shared
  campaign? Those are separate milestones.
- Is two-player support sufficient initially? Is four-player support a release
  requirement or a later experiment?
- Does a visitor retain independent progression, join the host's world state,
  or participate in an explicitly separate shared campaign save?
- How are encounter difficulty, guest eligibility, loss, rewards, and recovery
  determined when players have different progress?
- Which online bridges or hosted frontends are worth documenting as optional,
  and who will maintain compatibility tests for them?

Implementation can be scoped once these choices and the transport experiment
are resolved. This research does not authorize changing the existing single-player
progression, battle scaling, or save model.

[quetzal-site]: https://www.pokemonquetzal.app/
[quetzal-multiplayer]: https://pokemonquetzal.app/en/info/multiplayer/
[gpsp-confirmation]: https://github.com/libretro/gpsp/issues/245#issuecomment-3806743361
[gpsp-protocol]: https://github.com/libretro/gpsp/blob/5819380c2ffb0900219d700a382ee68c464ebb99/serial_proto.c#L92
[gpsp-options]: https://github.com/libretro/gpsp/blob/5819380c2ffb0900219d700a382ee68c464ebb99/libretro/libretro_core_options.h#L127
[pret-link]: https://github.com/pret/pokeemerald/blob/f90719c46ce710427cfcae65419a3fa684b34a8a/src/link.c
[pret-overworld]: https://github.com/pret/pokeemerald/blob/f90719c46ce710427cfcae65419a3fa684b34a8a/src/overworld.c
[rogue-game]: https://github.com/Pokabbie/pokeemerald-rogue/blob/a6adfcf18d7eaf99c2803e4b0bc04eca7af2f014/src/rogue_multiplayer.c
[rogue-network]: https://github.com/Pokabbie/pokeemerald-rogue-assistant/blob/51f1b9a839e8650ff6293f28a1421a62b40e0acc/RogueAssistantCpp/Src/Behaviours/MultiplayerBehaviour.cpp
[rogue-bridge]: https://github.com/Pokabbie/pokeemerald-rogue-assistant/blob/51f1b9a839e8650ff6293f28a1421a62b40e0acc/RogueAssistantCpp/Assets/RogueAssistant_mGBA.lua
[rogue-state]: https://github.com/Pokabbie/pokeemerald-rogue/blob/a6adfcf18d7eaf99c2803e4b0bc04eca7af2f014/include/rogue.h#L468
[gbapk-client]: https://github.com/TheHunterManX/GBA-PK-multiplayer/blob/eb51d531080d94ee8b96dede3f7a1b49d5be012a/GBA-PK_Client%20ALPHA%204.lua
[gbapk-readme]: https://github.com/TheHunterManX/GBA-PK-multiplayer/tree/eb51d531080d94ee8b96dede3f7a1b49d5be012a
[gbapk-license]: https://github.com/TheHunterManX/GBA-PK-multiplayer/blob/eb51d531080d94ee8b96dede3f7a1b49d5be012a/LICENSE.md
[ariel-repo]: https://github.com/Wired-Ariel/gen3-poke-multiplayer/tree/01e43f5b5e415ef2511974c5f9e0ec34d18e7a9e
[ariel-payload]: https://github.com/Wired-Ariel/gen3-poke-multiplayer/blob/01e43f5b5e415ef2511974c5f9e0ec34d18e7a9e/payload/main.c
[ariel-sio]: https://github.com/Wired-Ariel/gen3-poke-multiplayer/blob/01e43f5b5e415ef2511974c5f9e0ec34d18e7a9e/payload/sio.c
[coop-repo]: https://github.com/MehdiSenhajYnov/PokemonCoopGBA/tree/fbdb0959a3afdaf7f9e0666f3cfbda1eb3b5b76f
[local-scaling]: ../../game/src/trainer_party_scaling.c
[local-link]: ../../game/include/link.h
[local-controllers]: ../../game/src/battle_controllers.c
[local-overworld]: ../../game/src/overworld.c
[local-save]: ../../game/include/global.h
[local-flags]: ../../game/src/event_data.c
[local-persistence]: ../../game/src/wayfarer_persistence.c
[local-e2e]: ../../e2e/README.md
[local-skyemu]: ../../e2e/src/harness/skyemu/client.ts
[local-constants]: ../../game/include/constants/global.h
[local-sprites]: ../../game/include/sprite.h
[ttt-repo]: https://github.com/Viperio19/pokemon-tag-team-trials/tree/bff1263c061fe1c88c6c17873bcbcef8322dce80
[ttt-link]: https://github.com/Viperio19/pokemon-tag-team-trials/blob/bff1263c061fe1c88c6c17873bcbcef8322dce80/src/link.c#L625
[ttt-commands]: https://github.com/Viperio19/pokemon-tag-team-trials/blob/bff1263c061fe1c88c6c17873bcbcef8322dce80/include/constants/p2_commands.h
[ttt-overworld]: https://github.com/Viperio19/pokemon-tag-team-trials/blob/bff1263c061fe1c88c6c17873bcbcef8322dce80/src/overworld.c#L2967
[ttt-scripts]: https://github.com/Viperio19/pokemon-tag-team-trials/blob/bff1263c061fe1c88c6c17873bcbcef8322dce80/data/scripts/player2.inc#L585
[ttt-cable]: https://github.com/Viperio19/pokemon-tag-team-trials/blob/bff1263c061fe1c88c6c17873bcbcef8322dce80/src/cable_club.c#L584
[rogue-desync]: https://github.com/Pokabbie/pokeemerald-rogue-assistant/issues/2
[rogue-license]: https://github.com/Pokabbie/pokeemerald-rogue-assistant/issues/6
[gbapk-artifacts]: https://github.com/TheHunterManX/GBA-PK-multiplayer/issues/109
[gbapk-connect]: https://github.com/TheHunterManX/GBA-PK-multiplayer/issues/110
[gbapk-underwater]: https://github.com/TheHunterManX/GBA-PK-multiplayer/issues/85
[pvp-repo]: https://github.com/Lauchgestalt/poke-pvp/tree/70e340f51590a0b54f29623239cb15ecbbbd4b18
[pvp-client]: https://github.com/Lauchgestalt/poke-pvp/blob/70e340f51590a0b54f29623239cb15ecbbbd4b18/lua/client.lua
[pvp-server]: https://github.com/Lauchgestalt/poke-pvp/blob/70e340f51590a0b54f29623239cb15ecbbbd4b18/server/server.js
[gblink-repo]: https://github.com/GB-Link/gblink-netplay-bridge/tree/61e4f2004e0df9d0847d1c7e85eff28bbd512e0a
[gblink-code]: https://github.com/GB-Link/gblink-netplay-bridge/blob/61e4f2004e0df9d0847d1c7e85eff28bbd512e0a/src/engine/bridge/linkBridge.ts
[joybus-repo]: https://github.com/KittyPBoxx/pokeemerald-net-demo/tree/6c0ea261e3c0bad06ee2af954f306b88e3de3376
[joybus-code]: https://github.com/KittyPBoxx/pokeemerald-net-demo/blob/6c0ea261e3c0bad06ee2af954f306b88e3de3376/pokeemerald/src/net_conn.c
[splitscreen-repo]: https://github.com/Spuds0588/mgba-splitscreen/tree/b33d590ff75762f5d597e2a2a040e63522555151
[splitscreen-code]: https://github.com/Spuds0588/mgba-splitscreen/blob/b33d590ff75762f5d597e2a2a040e63522555151/mgba-splitscreen/src/online.js
[splitscreen-invites]: https://github.com/Spuds0588/mgba-splitscreen/blob/b33d590ff75762f5d597e2a2a040e63522555151/mgba-splitscreen/JOIN_CODES.md
[archipelago-client]: https://github.com/ArchipelagoMW/Archipelago/blob/0a601afbf575a4660077304a18ecb521ff1886c4/worlds/pokemon_emerald/client.py
[archipelago-docs]: https://github.com/ArchipelagoMW/Archipelago/blob/0a601afbf575a4660077304a18ecb521ff1886c4/worlds/pokemon_emerald/docs/en_Pokemon%20Emerald.md
[archipelago-changelog]: https://github.com/ArchipelagoMW/Archipelago/blob/0a601afbf575a4660077304a18ecb521ff1886c4/worlds/pokemon_emerald/CHANGELOG.md
[local-appearance]: ../specs/trainer-appearance-styles.md
[local-seed]: ../specs/playthrough-seed-framework.md
[local-leagues]: ../specs/leagues.md
[local-runtime]: ../specs/wayfarer-runtime-foundation.md

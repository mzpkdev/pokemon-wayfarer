# Native-link PoC: solo door control

Observed 2026-09-30 after the linked-door experiment in
[PoC PR #144](https://github.com/mzpkdev/pokemon-wayfarer/pull/144).
**Historical short-script control.** The same PoC-enabled ROM captured a black
frame with no active multiplayer session. The [completed follow-up](https://github.com/mzpkdev/pokemon-wayfarer/blob/eff36caeee3019af524d68a598e19b2280ae9d70/.product/research/native-link-multiplayer-followup.md#rendering-control-the-earlier-black-frame-was-a-fade)
held the player indoors longer and repeated the route with multiplayer compiled
out. Both controls showed a visible interior after the transition. The black
frame below was a fade, not a sustained indoor rendering defect. No rendering
patch was needed. This short run alone did not establish that conclusion.

## Reproduction

Use the [original pinned runner](https://github.com/mzpkdev/pokemon-wayfarer/blob/1dbf6922e4fc45f2fd4f3018097f32e54a018219/game/tools/multiplayer-poc/README.md)
and its dependency setup. Save the following as `solo-door.inputs`; it removes
only the connection chord from the recorded door journey:

```text
# Used with --stage-position 20,12: enter New Bark player's house and leave.
150 0 040
175 0 000
250 0 080
275 0 000
```

```sh
python3 game/tools/multiplayer-poc/run.py \
  game/pokemon-wayfarer-e2e-poc.gba game/pokemon-wayfarer-e2e-poc.elf \
  --mgba-source /tmp/wayfarer-multiplayer-poc-tools/mgba-0.10.5 \
  --mgba-build /tmp/wayfarer-multiplayer-poc-tools/build \
  --stage-e2e --inputs solo-door.inputs --stage-position 20,12 \
  --frames 400 --capture-frames 100,145,190,230,290,360 \
  --output /tmp/wayfarer-multiplayer-poc-validation/link-harness/solo-door-control
```

## Recorded evidence

The original output is at the local `--output` path above. The frame-290 image
was visually inspected: it is black. Input-relative frame 290 is absolute
emulator frame 391, between the observations below. Image SHA-256:
`47fbde60709a27683035d7b385f06dd1447f68c6548272758e15a335cf3bc15e`.

The trace shows map 259 with the session idle and no sent/received packets,
then return to map 0. These are observed diagnostic values. The image sample
landed during a fade; later visible frames in the longer linked, idle, and
compiled-out controls supersede the earlier unresolved-rendering inference.

```text
input_origin_frame=101
watch player=0 frame=360 mode=2 attachedMulti=2 phase=0 siocnt=2008 rcnt=0000 gLinkStatus=00000000 gShouldAdvanceLinkState=00000000 gReceivedRemoteLinkPlayers=00000000 gLinkCallback=00000000 gLinkState=00000001 sLinkOpen=00000000 gLinkMaster=00000000 gLinkLocalId=00000000 gLinkSerialCount=00000000 gRecvQueueCount=00000000 gLinkType=00000000 gMainCB2=0819f805 gMainSerialCB=00000000 gMainState=00000000 pocState=0 localMap=259 peerMap=4294967295 localXY=16,15 peerXY=0,0 tx=0 rx=0
watch player=0 frame=420 mode=2 attachedMulti=2 phase=0 siocnt=2008 rcnt=0000 gLinkStatus=00000000 gShouldAdvanceLinkState=00000000 gReceivedRemoteLinkPlayers=00000000 gLinkCallback=00000000 gLinkState=00000001 sLinkOpen=00000000 gLinkMaster=00000000 gLinkLocalId=00000000 gLinkSerialCount=00000000 gRecvQueueCount=00000000 gLinkType=00000000 gMainCB2=0819f805 gMainSerialCB=00000000 gMainState=00000000 pocState=0 localMap=0 peerMap=4294967295 localXY=27,18 peerXY=0,0 tx=0 rx=0
```

```text
mGBA version: 0.10.5
libmgba.a SHA256: ac09dc48409e9d863a640e9429b8e24569f4eb43e2689dad4c06a66a952a603f
ROM SHA256: 997dadfcc493f97b8c835243a6beb19676bb9964a31b39fae532a979ee0731ff
ELF SHA256: 29ef170e9e6cb321708a1afa76ef0a68fd643a567f4e2ec2b51ca7d8f8f0114c
gMultiplayerPocDiag=0x020351ec
gE2ETestAbi=0x09501ba0
gE2ETestRequest=0x02002b2c
gE2ETestResult=0x020015e4
```

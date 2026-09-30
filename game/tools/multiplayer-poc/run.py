#!/usr/bin/env python3
"""Build and run two isolated GBA cores connected through mGBA's SIO driver."""

import argparse
import binascii
import hashlib
import json
import os
import re
import shlex
from pathlib import Path
import shutil
import subprocess
import tempfile
import struct
import sys
import zlib


HERE = Path(__file__).resolve().parent
DIAG_FIELDS = (
    "magic", "version", "size", "state", "error", "localId", "peerId", "localMap",
    "peerMap", "localX", "localY", "peerX", "peerY", "localFacing", "peerFacing",
    "txCount", "rxCount", "lastCmd", "peerVisible", "frame",
)
BATTLE_FIELDS = (
    "magic", "version", "size", "state", "error", "encounterId", "localPlayerId",
    "localBattlerId", "peerWord", "localMoveChoices", "localLastChosenMove",
    "localLastChosenTurn", "localExecutedMoves", "localLastExecutedMove",
    "localLastExecutedTurn", "beforePartyHash", "afterPartyHash",
    "beforePartyCount", "afterPartyCount", "localOutcome", "peerOutcome",
    "resultAgreed", "rewardResult", "frame",
)
REWARD_FIELDS = (
    "magic", "version", "size", "encounterId", "ledgerState", "bagPotionCount",
    "savePhase", "saveResult", "operationResult", "grantAttempts",
    "grantsCommitted", "duplicateSuppressions", "retryCount", "saveAttempts",
)
SESSION_FIELDS = (
    "magic", "version", "size", "status", "error", "wanted", "localSaveHold",
    "peerSaveRequested", "closeCount", "reconnectCount", "attemptCount",
    "helloPagesSeen", "helloPeerConfirmed", "buildMatch", "buildId0",
    "buildId1", "buildId2", "buildId3",
)


def symbols_for(elf: Path) -> dict[str, int]:
    output = subprocess.check_output(
        [os.environ.get("NM", "arm-none-eabi-nm"), "--defined-only", str(elf)], text=True
    )
    symbols = {}
    for line in output.splitlines():
        fields = line.split()
        if len(fields) == 3:
            symbols[fields[2]] = int(fields[0], 16)
    return symbols


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def ppm_to_png(source: Path, target: Path) -> None:
    data = source.read_bytes()
    header = b"P6\n240 160\n255\n"
    pixels = data[len(header):]
    if not data.startswith(header) or len(pixels) != 240 * 160 * 3:
        raise ValueError(f"unexpected screenshot data in {source}")

    def chunk(kind: bytes, payload: bytes) -> bytes:
        return (struct.pack(">I", len(payload)) + kind + payload
                + struct.pack(">I", binascii.crc32(kind + payload) & 0xffffffff))

    rows = b"".join(b"\0" + pixels[y * 720:(y + 1) * 720] for y in range(160))
    target.write_bytes(
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", 240, 160, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(rows, 6))
        + chunk(b"IEND", b"")
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("elf", type=Path, help="matching unstripped ELF")
    parser.add_argument("--rom1", type=Path, help="optional different ROM for player 1")
    parser.add_argument("--elf1", type=Path, help="matching ELF for --rom1")
    parser.add_argument("--mgba-source", type=Path, required=True,
                        help="mGBA 0.10.5 unpacked source directory")
    parser.add_argument("--mgba-build", type=Path, required=True,
                        help="matching mGBA build directory containing libmgba.a")
    parser.add_argument("--output", type=Path, default=Path("multiplayer-poc-results"))
    parser.add_argument("--inputs", type=Path, default=HERE / "presence.inputs")
    parser.add_argument("--capture-frames", default="0,160,200,240,260,300,360",
                        help="comma-separated frames relative to E2E readiness")
    parser.add_argument("--frames", type=int, default=480,
                        help="frames after E2E staging, or after boot without staging")
    parser.add_argument("--disconnect-frame", type=int, default=0,
                        help="remove player 1's cable at this input-relative frame")
    parser.add_argument("--stage-e2e", action="store_true",
                        help="arrange two fresh New Bark games via the existing E2E mailbox")
    parser.add_argument("--stage-position", default=None,
                        help="optional raw New Bark map x,y for both E2E fixtures")
    parser.add_argument("--stage-map", default=None,
                        help="optional E2E map group,num for both fixtures")
    parser.add_argument("--stage-party", action="store_true",
                        help="arrange distinct level-8 Bulbasaur/Charmander originals")
    parser.add_argument("--stage-full-medicine", action="store_true",
                        help="fill Medicine pocket via the E2E-only fixture flag")
    parser.add_argument("--stage-menu-unlocks", action="store_true",
                        help="unlock Pokedex and Pokenav via E2E flag patches")
    parser.add_argument("--diagnostic", default="gMultiplayerPocDiag")
    parser.add_argument("--diagnostic-size", type=int, default=80)
    parser.add_argument("--enable-saves", action="store_true",
                        help="mount separate mGBA flash files and export them after core shutdown")
    parser.add_argument("--save0", type=Path, help="initial flash file for player 0")
    parser.add_argument("--save1", type=Path, help="initial flash file for player 1")
    parser.add_argument("--e2e-save-frame", type=int, default=0,
                        help="input-relative frame for the existing E2E real-save command")
    parser.add_argument("--e2e-save-player", choices=("both", "0", "1"), default="both",
                        help="which staged player receives the E2E real-save command")
    parser.add_argument("--e2e-wild-battle-frame", type=int, default=0,
                        help="input-relative frame to request the existing E2E wild battle")
    parser.add_argument("--e2e-wild-battle-player", choices=("both", "0", "1"), default="0",
                        help="staged player(s) entering local wild battle")
    parser.add_argument("--cut-reward-phase", choices=("before", "writing", "success"),
                        help="stop both CPUs when the selected player's reward save reaches this phase")
    parser.add_argument("--cut-player", type=int, choices=(0, 1), default=0)
    parser.add_argument("--cut-delay-frames", type=int, default=0,
                        help="frames to remain in the selected phase before stopping")
    parser.add_argument("--require-presence", action="store_true",
                        help="fail unless both ROMs received each other's native packets")
    parser.add_argument("--require-session", action="store_true",
                        help="fail unless both ROMs expose valid 72-byte session diagnostics")
    parser.add_argument("--expect-session-status", help="required final session status for players 0,1")
    parser.add_argument("--expect-session-error", help="required final session error for players 0,1")
    parser.add_argument("--timeout", type=int, default=180, help="wall-clock seconds")
    args = parser.parse_args()
    if bool(args.rom1) != bool(args.elf1):
        parser.error("--rom1 and --elf1 must be supplied together")
    if args.frames < 1 or args.timeout < 1 or args.diagnostic_size < 0:
        parser.error("frames and timeout must be positive; diagnostic size cannot be negative")
    if args.disconnect_frame < 0 or args.disconnect_frame >= args.frames:
        parser.error("disconnect frame must be between 0 and frames - 1")
    if args.require_presence and (args.diagnostic_size != 80 or args.disconnect_frame):
        parser.error("presence assertion needs 80-byte diagnostics and no cable removal")
    expected_session = {}
    for field, raw in (("status", args.expect_session_status),
                       ("error", args.expect_session_error)):
        if raw is None:
            continue
        try:
            values = tuple(int(part, 0) for part in raw.split(","))
        except ValueError:
            parser.error(f"session {field} needs two comma-separated integers")
        if len(values) != 2 or any(value < 0 for value in values):
            parser.error(f"session {field} needs two nonnegative integers")
        expected_session[field] = values
    if expected_session:
        args.require_session = True
    if (args.save0 or args.save1) and not args.enable_saves:
        parser.error("--save0/--save1 require --enable-saves")
    if (args.stage_party or args.stage_full_medicine or args.stage_menu_unlocks) and not args.stage_e2e:
        parser.error("stage party/full medicine/menu unlocks require --stage-e2e")
    if args.e2e_save_frame and (not args.enable_saves or not args.stage_e2e
                                or not 0 < args.e2e_save_frame < args.frames):
        parser.error("E2E save frame requires staged game, saves, and a frame within run")
    if args.e2e_wild_battle_frame and (not args.stage_e2e
                                       or not 0 < args.e2e_wild_battle_frame < args.frames):
        parser.error("E2E wild battle frame requires staged game and a frame within run")
    if args.e2e_save_frame and args.e2e_wild_battle_frame:
        parser.error("E2E save and wild battle requests cannot share one mailbox run")
    if args.cut_reward_phase and not args.enable_saves:
        parser.error("reward power cut requires --enable-saves")
    if args.cut_delay_frames < 0 or (args.cut_delay_frames and not args.cut_reward_phase):
        parser.error("cut delay must be nonnegative and requires a reward phase")
    for save in (args.save0, args.save1):
        if save and (not save.is_file() or save.stat().st_size == 0):
            parser.error(f"initial save is missing or empty: {save}")
    stage_x = stage_y = 0x8000
    stage_map_group = stage_map_num = 0xffff
    if args.stage_map is not None:
        if not args.stage_e2e:
            parser.error("stage map requires --stage-e2e")
        try:
            stage_map_group, stage_map_num = map(int, args.stage_map.split(","))
        except ValueError:
            parser.error("stage map must be group,num")
        if not all(0 <= value < 65535 for value in (stage_map_group, stage_map_num)):
            parser.error("stage map group and number must be 0..65534")
    if args.stage_position is not None:
        if not args.stage_e2e:
            parser.error("stage position requires --stage-e2e")
        try:
            stage_x, stage_y = map(int, args.stage_position.split(","))
        except ValueError:
            parser.error("stage position must be x,y")
        if not all(0 <= value <= 32767 for value in (stage_x, stage_y)):
            parser.error("stage position coordinates must be 0..32767")
    try:
        capture_frames = [int(item) for item in args.capture_frames.split(",") if item]
    except ValueError:
        parser.error("capture frames must be comma-separated integers")
    if any(frame < 0 or frame >= args.frames for frame in capture_frames):
        parser.error("capture frames must be between 0 and frames - 1")
    source = args.mgba_source.resolve()
    build = args.mgba_build.resolve()
    library = build / "libmgba.a"
    flags = build / "include/mgba/flags.h"
    for path in (source / "include/mgba/core/core.h", library, flags):
        if not path.exists():
            parser.error(f"missing mGBA build input: {path}")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    for old in output.glob("player*.png"):
        old.unlink()
    for pattern in ("player*.ppm", "player*.diag", "player*.battle", "player*.reward",
                    "player*.session"):
        for old in output.glob(pattern):
            old.unlink()
    for name in ("diagnostics.json", "battle.json", "reward.json", "session.json", "cut.json"):
        (output / name).unlink(missing_ok=True)
    (output / "link.log").unlink(missing_ok=True)
    if args.enable_saves:
        for player in range(2):
            (output / f"player{player}.sav").unlink(missing_ok=True)
    roms = (args.rom.resolve(), (args.rom1 or args.rom).resolve())
    elfs = (args.elf.resolve(), (args.elf1 or args.elf).resolve())
    symbols = tuple(symbols_for(elf) for elf in elfs)
    required = [args.diagnostic] if args.diagnostic_size else []
    if args.stage_e2e:
        required += ["gE2ETestAbi", "gE2ETestRequest", "gE2ETestResult"]
    missing = [f"player{player}:{key}" for player in range(2)
               for key in required if key not in symbols[player]]
    if args.cut_reward_phase:
        missing += [f"player{player}:gMultiplayerPocRewardDiag"
                    for player in range(2)
                    if "gMultiplayerPocRewardDiag" not in symbols[player]]
    if args.require_session:
        missing += [f"player{player}:gMultiplayerPocSessionDiag"
                    for player in range(2)
                    if "gMultiplayerPocSessionDiag" not in symbols[player]]
    if missing:
        parser.error(f"matching ELF lacks symbols: {', '.join(missing)}")
    (output / "provenance.txt").write_text(
        "mGBA version: 0.10.5\n"
        f"libmgba.a SHA256: {digest(library)}\n"
        + "".join(f"ROM{player} SHA256: {digest(rom)}\n"
                  for player, rom in enumerate(roms))
        + "".join(f"ELF{player} SHA256: {digest(elf)}\n"
                  for player, elf in enumerate(elfs))
        + f"run.py SHA256: {digest(Path(__file__).resolve())}\n"
        f"link.c SHA256: {digest(HERE / 'link.c')}\n"
        f"inputs SHA256: {digest(args.inputs.resolve())}\n"
        + "".join(f"save{player} SHA256: {digest(save.resolve())}\n"
                  for player, save in enumerate((args.save0, args.save1)) if save)
        + f"cwd: {Path.cwd()}\n"
        + f"command: {shlex.join(sys.argv)}\n"
        + "".join(f"player{player}:{key}=0x{symbols[player][key]:08x}\n"
                  for player in range(2) for key in required)
    )
    with tempfile.TemporaryDirectory(prefix="wayfarer-multiplayer-link-") as temporary:
        work = Path(temporary)
        executable = work / "link"
        subprocess.run(
            [os.environ.get("CC", "cc"), "-std=gnu11", "-DNDEBUG", "-Wall", "-Wextra",
             "-Werror", "-O2", f"-I{build / 'include'}", f"-I{source / 'include'}",
             str(HERE / "link.c"), str(library), "-o", str(executable),
             "-lm", "-lz", "-lpthread"],
            check=True,
        )
        for player, rom in enumerate(roms):
            shutil.copyfile(rom, work / f"player{player}.gba")
        for player, save in enumerate((args.save0, args.save1)):
            if save:
                shutil.copyfile(save.resolve(), work / f"player{player}.sav")
        shutil.copyfile(args.inputs.resolve(), work / "inputs.txt")
        watches = (
            ("gLinkStatus", 4, 0), ("gShouldAdvanceLinkState", 1, 0),
            ("gReceivedRemoteLinkPlayers", 1, 0), ("gLinkCallback", 4, 0),
            ("gLinkState", 1, 1, "gLink"), ("sLinkOpen", 1, 0),
            ("gLinkMaster", 1, 0, "gLink"), ("gLinkLocalId", 1, 2, "gLink"),
            ("gLinkSerialCount", 1, 13, "gLink"),
            ("gRecvQueueCount", 1, 0xfbd, "gLink"),
            ("gLinkType", 2, 0), ("gMainCB2", 4, 4, "gMain"),
            ("gMainSerialCB", 4, 24, "gMain"), ("gMainState", 1, 0x438, "gMain"),
            ("gPaletteFadeFlags", 4, 12, "gPaletteFade"),
            ("gPaletteFadeColor", 4, 16, "gPaletteFade"),
            ("gMapLayoutLoadError", 4, 0),
            ("gMapLayoutLoadErrorActive", 1, 10, "gMapLayoutLoadError"),
            ("gMapHeaderLayoutId", 2, 0x12, "gMapHeader"),
            ("gPlttBufferFaded", 2, 0),
            ("gPlttBufferUnfaded", 2, 0),
            ("gE2ERequestStatus", 1, 87, "gE2ETestRequest"),
            ("gE2ERequestCommand", 1, 86, "gE2ETestRequest"),
            ("gE2EResultStatus", 1, 14, "gE2ETestResult"),
            ("gSaveFileStatus", 1, 0), ("gSaveCounter", 4, 0),
            ("gPlayerPartyCount", 1, 0), ("gSaveBlock1Ptr", 4, 0),
            ("gSaveBlock2Ptr", 4, 0),
            ("battleState", 4, 12, "gMultiplayerPocBattleDiag"),
            ("battleError", 4, 16, "gMultiplayerPocBattleDiag"),
            ("battleChoiceCount", 4, 36, "gMultiplayerPocBattleDiag"),
            ("battleChosenMove", 4, 40, "gMultiplayerPocBattleDiag"),
            ("battleAnimationCount", 4, 48, "gMultiplayerPocBattleDiag"),
            ("battleAnimatedMove", 4, 52, "gMultiplayerPocBattleDiag"),
            ("battleAgreed", 4, 84, "gMultiplayerPocBattleDiag"),
            ("rewardLedger", 4, 16, "gMultiplayerPocRewardDiag"),
            ("rewardSavePhase", 4, 24, "gMultiplayerPocRewardDiag"),
            ("rewardCommitted", 4, 40, "gMultiplayerPocRewardDiag"),
            ("sessionStatus", 4, 12, "gMultiplayerPocSessionDiag"),
            ("sessionError", 4, 16, "gMultiplayerPocSessionDiag"),
            ("sessionWanted", 4, 20, "gMultiplayerPocSessionDiag"),
            ("sessionLocalSaveHold", 4, 24, "gMultiplayerPocSessionDiag"),
            ("sessionPeerSaveRequested", 4, 28, "gMultiplayerPocSessionDiag"),
            ("sessionCloseCount", 4, 32, "gMultiplayerPocSessionDiag"),
            ("sessionReconnectCount", 4, 36, "gMultiplayerPocSessionDiag"),
            ("sessionHelloPages", 4, 44, "gMultiplayerPocSessionDiag"),
            ("sessionBuildMatch", 4, 52, "gMultiplayerPocSessionDiag"),
        )
        watch_text = "".join(
            f"{player} {name} {symbols[player][base] + offset:08x} {width}\n"
            for player in range(2)
            for item in watches
            for name, width, offset, *alias in (item,)
            for base in (alias[0] if alias else name,)
            if base in symbols[player]
        )
        watch_text += "".join(f"{player} {name} {address:08x} 2\n"
                              for player in range(2)
                              for name, address in (("DISPCNT", 0x04000000),
                                                    ("BLDCNT", 0x04000050),
                                                    ("BLDY", 0x04000054)))
        (work / "watch.txt").write_text(watch_text)
        (work / "captures.txt").write_text("\n".join(map(str, capture_frames)) + "\n")
        command = [
            str(executable), "player0.gba", "player1.gba", str(args.frames), "inputs.txt",
            "watch.txt", "captures.txt",
            hex(symbols[0].get(args.diagnostic, 0)), str(args.diagnostic_size),
            str(args.disconnect_frame),
            hex(symbols[0].get("gE2ETestAbi", 0) if args.stage_e2e else 0),
            hex(symbols[0].get("gE2ETestRequest", 0) if args.stage_e2e else 0),
            hex(symbols[0].get("gE2ETestResult", 0) if args.stage_e2e else 0),
            str(stage_x), str(stage_y),
            "1" if args.enable_saves else "0",
            str(args.e2e_save_frame),
            {"both": "3", "0": "1", "1": "2"}[args.e2e_save_player],
            "1" if args.stage_party else "0",
            hex(symbols[0].get("gMultiplayerPocBattleDiag", 0)),
            hex(symbols[0].get("gMultiplayerPocRewardDiag", 0)),
            str(args.e2e_wild_battle_frame),
            {"both": "3", "0": "1", "1": "2"}[args.e2e_wild_battle_player],
            {None: "0", "before": "4", "writing": "2", "success": "3"}[args.cut_reward_phase],
            str(args.cut_player),
            str(args.cut_delay_frames),
            "1" if args.stage_full_medicine else "0",
            hex(symbols[1].get(args.diagnostic, 0)),
            hex(symbols[1].get("gE2ETestAbi", 0) if args.stage_e2e else 0),
            hex(symbols[1].get("gE2ETestRequest", 0) if args.stage_e2e else 0),
            hex(symbols[1].get("gE2ETestResult", 0) if args.stage_e2e else 0),
            hex(symbols[1].get("gMultiplayerPocBattleDiag", 0)),
            hex(symbols[1].get("gMultiplayerPocRewardDiag", 0)),
            "1" if args.stage_menu_unlocks else "0",
            hex(symbols[0].get("gMultiplayerPocSessionDiag", 0)),
            hex(symbols[1].get("gMultiplayerPocSessionDiag", 0)),
            str(stage_map_group), str(stage_map_num),
        ]
        environment = dict(os.environ, XDG_CONFIG_HOME=str(work), XDG_DATA_HOME=str(work))
        status = 1
        with (output / "link.log").open("w") as log:
            try:
                result = subprocess.run(command, cwd=work, env=environment,
                                        stdout=log, stderr=subprocess.STDOUT,
                                        timeout=args.timeout)
                status = result.returncode
                if status < 0:
                    log.write(f"runner terminated by signal {-status}\n")
            except subprocess.TimeoutExpired:
                log.write(f"runner exceeded {args.timeout}s wall-clock timeout\n")
        for source in work.glob("player*.ppm"):
            shutil.copyfile(source, output / source.name)
            ppm_to_png(source, output / f"{source.stem}.png")
        for player in range(2):
            source = work / f"player{player}.diag"
            if source.exists():
                shutil.copyfile(source, output / source.name)
            for suffix in ("battle", "reward", "session"):
                source = work / f"player{player}.{suffix}"
                if source.exists():
                    shutil.copyfile(source, output / source.name)
            source = work / f"player{player}.sav"
            if args.enable_saves and source.exists():
                shutil.copyfile(source, output / source.name)
    if args.cut_reward_phase:
        match = re.search(
            r"power_cut player=(\d+) phase=(\d+) firstFrame=(\d+) "
            r"cutFrame=(\d+) peerPhase=(\d+) battleState=(\d+) agreed=(\d+)",
            (output / "link.log").read_text()
        )
        if match:
            cut = dict(zip(("player", "phase", "firstFrame", "cutFrame", "peerPhase",
                            "battleState", "resultAgreed"),
                           map(int, match.groups())))
            (output / "cut.json").write_text(json.dumps(cut, indent=2) + "\n")
        else:
            print("FAIL: requested reward power cut did not occur")
            status = 1
    have_diagnostics = all((output / f"player{p}.diag").exists() for p in range(2))
    if args.diagnostic_size == 80 and have_diagnostics:
        diagnostics = [
            dict(zip(DIAG_FIELDS, struct.unpack("<20I", (output / f"player{p}.diag").read_bytes())))
            for p in range(2)
        ]
        (output / "diagnostics.json").write_text(json.dumps(diagnostics, indent=2) + "\n")
        for player, diag in enumerate(diagnostics):
            print(f"diagnostic player={player} state={diag['state']} error={diag['error']} "
                  f"id={diag['localId']} tx={diag['txCount']} rx={diag['rxCount']} "
                  f"local=({diag['localMap']},{diag['localX']},{diag['localY']}) "
                  f"peer=({diag['peerMap']},{diag['peerX']},{diag['peerY']}) "
                  f"visible={diag['peerVisible']}")
        if args.require_presence:
            if any(d["magic"] != 0x504F4331 or d["version"] != 1 or d["size"] != 80
                   or d["state"] != 2 or d["txCount"] == 0 or d["rxCount"] == 0
                   or d["peerVisible"] != 1 for d in diagnostics):
                print("FAIL: native linked presence assertion")
                status = 1
            elif any(diagnostics[p]["peerId"] != diagnostics[1-p]["localId"]
                     or diagnostics[p]["peerMap"] != diagnostics[1-p]["localMap"]
                     or diagnostics[p]["peerX"] != diagnostics[1-p]["localX"]
                     or diagnostics[p]["peerY"] != diagnostics[1-p]["localY"]
                     for p in range(2)):
                print("FAIL: peer identity or position did not converge")
                status = 1
            else:
                print("PASS: native linked presence")
    elif args.diagnostic_size == 80:
        print("FAIL: expected diagnostic data from both cartridges")
        status = 1
    for label, fields, count, magic in (
        ("battle", BATTLE_FIELDS, 24, 0x42415431),
        ("reward", REWARD_FIELDS, 14, 0x52574431),
        ("session", SESSION_FIELDS, 18, 0x53455331),
    ):
        files = [output / f"player{player}.{label}" for player in range(2)]
        if all(path.exists() for path in files):
            values = [dict(zip(fields, struct.unpack(f"<{count}I", path.read_bytes())))
                      for path in files]
            if all(value["magic"] == magic and value["size"] == count * 4
                   for value in values):
                (output / f"{label}.json").write_text(json.dumps(values, indent=2) + "\n")
                if label == "session":
                    for field, expected in expected_session.items():
                        actual = tuple(value[field] for value in values)
                        if actual != expected:
                            print(f"FAIL: session {field} expected {expected}, got {actual}")
                            status = 1
            else:
                print(f"FAIL: invalid {label} diagnostic data")
                status = 1
        elif label == "session" and args.require_session:
            print("FAIL: expected session diagnostics from both cartridges")
            status = 1
    print((output / "link.log").read_text(), end="")
    print(f"Artifacts: {output}")
    return status


if __name__ == "__main__":
    raise SystemExit(main())

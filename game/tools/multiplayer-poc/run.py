#!/usr/bin/env python3
"""Build and run two isolated GBA cores connected through mGBA's SIO driver."""

import argparse
import binascii
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import struct
import zlib


HERE = Path(__file__).resolve().parent
DIAG_FIELDS = (
    "magic", "version", "size", "state", "error", "localId", "peerId", "localMap",
    "peerMap", "localX", "localY", "peerX", "peerY", "localFacing", "peerFacing",
    "txCount", "rxCount", "lastCmd", "peerVisible", "frame",
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
    parser.add_argument("--diagnostic", default="gMultiplayerPocDiag")
    parser.add_argument("--diagnostic-size", type=int, default=80)
    parser.add_argument("--require-presence", action="store_true",
                        help="fail unless both ROMs received each other's native packets")
    parser.add_argument("--timeout", type=int, default=180, help="wall-clock seconds")
    args = parser.parse_args()
    if args.frames < 1 or args.timeout < 1 or args.diagnostic_size < 0:
        parser.error("frames and timeout must be positive; diagnostic size cannot be negative")
    if args.disconnect_frame < 0 or args.disconnect_frame >= args.frames:
        parser.error("disconnect frame must be between 0 and frames - 1")
    if args.require_presence and (args.diagnostic_size != 80 or args.disconnect_frame):
        parser.error("presence assertion needs 80-byte diagnostics and no cable removal")
    stage_x = stage_y = 0x8000
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
    for pattern in ("player*.ppm", "player*.diag"):
        for old in output.glob(pattern):
            old.unlink()
    (output / "diagnostics.json").unlink(missing_ok=True)
    (output / "link.log").unlink(missing_ok=True)
    symbols = symbols_for(args.elf.resolve())
    required = [args.diagnostic] if args.diagnostic_size else []
    if args.stage_e2e:
        required += ["gE2ETestAbi", "gE2ETestRequest", "gE2ETestResult"]
    missing = sorted(set(required) - symbols.keys())
    if missing:
        parser.error(f"matching ELF lacks symbols: {', '.join(missing)}")
    (output / "provenance.txt").write_text(
        "mGBA version: 0.10.5\n"
        f"libmgba.a SHA256: {digest(library)}\n"
        f"ROM SHA256: {digest(args.rom.resolve())}\n"
        f"ELF SHA256: {digest(args.elf.resolve())}\n"
        + "".join(f"{key}=0x{symbols[key]:08x}\n" for key in required)
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
        for player in range(2):
            shutil.copyfile(args.rom.resolve(), work / f"player{player}.gba")
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
        )
        (work / "watch.txt").write_text("".join(
            f"{name} {symbols[base] + offset:08x} {width}\n"
            for item in watches
            for name, width, offset, *alias in (item,)
            for base in (alias[0] if alias else name,)
            if base in symbols
        ))
        (work / "captures.txt").write_text("\n".join(map(str, capture_frames)) + "\n")
        command = [
            str(executable), "player0.gba", "player1.gba", str(args.frames), "inputs.txt",
            "watch.txt", "captures.txt",
            hex(symbols.get(args.diagnostic, 0)), str(args.diagnostic_size),
            str(args.disconnect_frame),
            hex(symbols.get("gE2ETestAbi", 0) if args.stage_e2e else 0),
            hex(symbols.get("gE2ETestRequest", 0) if args.stage_e2e else 0),
            hex(symbols.get("gE2ETestResult", 0) if args.stage_e2e else 0),
            str(stage_x), str(stage_y),
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
    print((output / "link.log").read_text(), end="")
    print(f"Artifacts: {output}")
    return status


if __name__ == "__main__":
    raise SystemExit(main())

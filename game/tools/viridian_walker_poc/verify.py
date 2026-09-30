#!/usr/bin/env python3
"""Exercise a fresh Viridian walker POC ROM through SkyEmu's HTTP server.

This deliberately does not use the E2E arrange hook. It starts with no save,
selects New Game on the title screen, and reads only ROM-exported telemetry.
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import threading
import urllib.parse
import urllib.request
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_SKYEMU = ROOT / "e2e/node_modules/skyemu-static/vendor/SkyEmu"
VIRIDIAN_MAP_GROUP = 0
VIRIDIAN_MAP_NUM = 32
MAX_READ_BYTES = 448
MAP_OFFSET = 7
GOALS = ("grass", "mart", "center", "route1", "route2", "route22")


class SkyEmu:
    def __init__(self, port: int):
        self.url = f"http://127.0.0.1:{port}"

    def request(self, endpoint: str, params: dict[str, str] | None = None) -> bytes:
        suffix = "?" + urllib.parse.urlencode(params) if params else ""
        with urllib.request.urlopen(self.url + endpoint + suffix, timeout=30) as response:
            return response.read().rstrip(b"\0")

    def check(self, endpoint: str, params: dict[str, str] | None = None) -> None:
        result = self.request(endpoint, params)
        if result != b"ok":
            raise RuntimeError(f"SkyEmu {endpoint} returned {result!r}, expected 'ok'")

    def read(self, address: int, size: int) -> bytes:
        result = bytearray()
        for start in range(0, size, MAX_READ_BYTES):
            count = min(MAX_READ_BYTES, size - start)
            query = urllib.parse.urlencode(
                [("addr", f"{address + start + offset:08x}") for offset in range(count)]
            )
            hex_bytes = self.request("/read_byte?" + query).decode("ascii")
            if len(hex_bytes) != count * 2:
                raise RuntimeError(f"SkyEmu memory read returned {len(hex_bytes)} hex digits")
            result.extend(bytes.fromhex(hex_bytes))
        return bytes(result)

    def u16(self, address: int) -> int:
        return struct.unpack("<H", self.read(address, 2))[0]

    def u32(self, address: int) -> int:
        return struct.unpack("<I", self.read(address, 4))[0]

    def step(self, frames: int) -> None:
        self.check("/step", {"frames": str(frames)})

    def press(self, button: str, hold: int = 2, release: int = 2) -> None:
        self.check("/input", {button: "1"})
        self.step(hold)
        self.check("/input", {button: "0"})
        self.step(release)

    def screenshot(self, path: Path) -> None:
        data = self.request("/screen")
        if not data.startswith(b"\x89PNG\r\n\x1a\n"):
            raise RuntimeError("SkyEmu /screen did not return a PNG")
        path.write_bytes(data)


def read_symbols(path: Path) -> dict[str, int]:
    symbols: dict[str, int] = {}
    for line in path.read_text().splitlines():
        fields = line.split(maxsplit=3)
        if len(fields) != 4 or len(fields[0]) != 8:
            continue
        address, binding, _, name = fields
        if name.startswith("."):
            continue
        try:
            value = int(address, 16)
        except ValueError:
            continue
        if binding == "g" or name not in symbols:
            symbols[name] = value
    return symbols


def reserve_port() -> int:
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def wait_for_ready(client: SkyEmu, process: subprocess.Popen[bytes], ready: threading.Event) -> None:
    # SkyEmu prints this line after binding the HTTP server. Waiting on that
    # output is event driven, so it needs one health inspection, not polling.
    if not ready.wait(timeout=30):
        raise RuntimeError(f"SkyEmu did not announce its HTTP server (exit={process.poll()})")
    status = json.loads(client.request("/status"))
    if client.request("/ping") != b"pong" or status.get("rom-loaded") is not True:
        raise RuntimeError(f"SkyEmu announced HTTP server but was not ready: {status}")


def read_player_object(client: SkyEmu, symbols: dict[str, int]) -> tuple[int, int, int] | None:
    base = symbols["gObjectEvents"]
    for index in range(16):
        address = base + index * 0x24
        raw = client.read(address, 0x18)
        if raw[0] & 1 and raw[2] & 1:
            x, y = struct.unpack_from("<hh", raw, 0x10)
            return address, x, y
    return None


def walker_snapshot(client: SkyEmu, symbols: dict[str, int]) -> dict[str, int]:
    raw = client.read(symbols["gViridianWalkerDebug"], 34)
    values = struct.unpack("<9H6h4B", raw)
    keys = (
        "completed", "replans", "searches", "search_nodes", "search_frames",
        "max_nodes_per_frame", "max_slice_vblanks", "grass_tiles_found",
        "blocked_steps", "x", "y",
        "goal_x", "goal_y", "next_x", "next_y", "phase", "goal",
        "last_completed_goal", "used_grass_fallback",
    )
    return dict(zip(keys, values, strict=True))


def read_location(client: SkyEmu, symbols: dict[str, int]) -> tuple[int, int, int, int] | None:
    ptr = client.u32(symbols["gSaveBlock1Ptr"])
    if not 0x02000000 <= ptr < 0x02040000:
        return None
    # This build's SaveBlock1 has a two-byte pad after saveVersion. The field
    # annotations in global.h predate saveVersion and do not include the pad.
    raw = client.read(ptr, 16)
    x, y = struct.unpack_from("<hh", raw, 4)
    map_group, map_num = struct.unpack_from("<bb", raw, 8)
    return map_group, map_num, x, y


def boot_new_game(
    client: SkyEmu, symbols: dict[str, int], output: Path, startup_only: bool
) -> int:
    client.step(120)
    client.screenshot(output / "title.png")
    # No .sav is present. New Game is the first menu choice. A is released
    # between presses so logos, title, and menu are handled by normal input.
    for index in range(50):
        client.press("A")
        client.step(56)
        location = read_location(client, symbols)
        player = read_player_object(client, symbols)
        walker = None if startup_only else walker_snapshot(client, symbols)
        if (
            location and location[:2] == (VIRIDIAN_MAP_GROUP, VIRIDIAN_MAP_NUM)
            and player is not None and (startup_only or walker["phase"] != 0)
        ):
            # The save warp is set before field graphics are fully loaded.
            # An active player and walker establish that the map is running.
            client.step(30)
            client.screenshot(output / "viridian-start.png")
            print(f"Fresh New Game reached Viridian at {location[2:]}; player object {player[1:]}", flush=True)
            return 150 + (index + 1) * 60
    client.screenshot(output / "startup-failed.png")
    raise RuntimeError(f"Fresh New Game did not reach Viridian; location={location}")


def move_player_to(
    client: SkyEmu, symbols: dict[str, int], target_x: int, target_y: int
) -> tuple[int, int]:
    """Walk by real D-pad input; only intended for the known Center sidewalk."""
    for _ in range(8):
        player = read_player_object(client, symbols)
        if player is None:
            raise RuntimeError("Player object vanished while positioning obstruction")
        _, x, y = player
        if (x, y) == (target_x, target_y):
            return x, y
        if y != target_y:
            direction = "Down" if target_y > y else "Up"
        else:
            direction = "Right" if target_x > x else "Left"
        client.press(direction, hold=2, release=18)
        updated = read_player_object(client, symbols)
        if updated is None or updated[1:] == (x, y):
            raise RuntimeError(f"Player could not walk {direction} from {(x, y)}")
    raise RuntimeError(f"Player did not reach {(target_x, target_y)}")


def try_talk_to_walker(
    client: SkyEmu, symbols: dict[str, int], output: Path,
    walker: dict[str, int],
) -> dict[str, object]:
    player = read_player_object(client, symbols)
    if player is None:
        return {"attempted": False, "reason": "No active player object"}
    dx, dy = walker["x"] - player[1], walker["y"] - player[2]
    if abs(dx) + abs(dy) != 1 or walker["phase"] == 4:
        return {"attempted": False, "reason": "Walker not visible beside player"}
    direction = "Right" if dx == 1 else "Left" if dx == -1 else "Down" if dy == 1 else "Up"
    client.press(direction, hold=2, release=2)
    client.press("A", hold=2, release=40)
    client.screenshot(output / "walker-interaction.png")
    client.press("B", hold=2, release=20)
    return {"attempted": True, "direction": direction, "frame_cost": 68,
            "screenshot": "walker-interaction.png"}


def run_walker(
    client: SkyEmu, symbols: dict[str, int], output: Path, max_frames: int, min_goals: int
) -> dict[str, object]:
    result: dict[str, object] = {
        "goal_names": list(GOALS), "completed_goals": [], "transitions": [],
        "reentries": [],
        "real_player_block": {"observed": False},
        "interaction": {"attempted": False}, "frames_after_startup": 0,
    }
    elapsed = 0
    last = walker_snapshot(client, symbols)
    if not 0 <= last["goal"] < len(GOALS):
        raise RuntimeError(f"Invalid initial walker telemetry: {last}")
    result["initial"] = last

    # The New Game spawn is at the Center's entrance approach. Clear it with
    # actual controller input so the unblocked route can finish naturally.
    player = read_player_object(client, symbols)
    if player is None or player[1:] != (30 + MAP_OFFSET, 37 + MAP_OFFSET):
        raise RuntimeError(f"Unexpected New Game player position: {player}")
    move_player_to(client, symbols, 28 + MAP_OFFSET, 37 + MAP_OFFSET)
    elapsed += 40
    client.screenshot(output / "player-clear-of-center.png")
    blocked = False
    intercept_attempted = False
    completed: list[int] = []
    transitions: list[dict[str, object]] = []
    reentries: list[dict[str, object]] = []
    last_exit_goal: int | None = None

    while elapsed < max_frames:
        # These are deliberate emulated-frame advances, rather than repeated
        # wall-clock status polling. The read occurs after each frame batch.
        advance = min(30, max_frames - elapsed)
        client.step(advance)
        elapsed += advance
        current = walker_snapshot(client, symbols)
        if current["phase"] > 4 or current["goal"] >= len(GOALS):
            raise RuntimeError(f"Invalid walker telemetry after {elapsed} frames: {current}")
        if last["phase"] == 4 and current["phase"] != 4:
            reentry = {
                "frame": elapsed,
                "from_exit": GOALS[last_exit_goal] if last_exit_goal is not None else None,
                "hidden_position": [last["x"], last["y"]],
                "entry_position": [current["x"], current["y"]],
                "entry_goal": GOALS[current["goal"]],
                "entry_phase": current["phase"],
                "first_walk_position": None,
            }
            reentries.append(reentry)
            client.screenshot(output / f"reentry-{len(reentries):02d}.png")
            print(f"frame {elapsed}: travel re-entry {reentry}", flush=True)
        if reentries and reentries[-1]["first_walk_position"] is None and current["phase"] == 2:
            reentries[-1]["first_walk_position"] = [current["x"], current["y"]]
            reentries[-1]["first_walk_frame"] = elapsed
        if current["completed"] > last["completed"]:
            goal = current["last_completed_goal"]
            if goal >= len(GOALS):
                raise RuntimeError(f"Invalid completed goal after {elapsed} frames: {current}")
            completed.append(goal)
            if goal >= 3:
                last_exit_goal = goal
            event = {"frame": elapsed, "goal": GOALS[goal], "telemetry": current}
            transitions.append(event)
            client.screenshot(output / f"goal-{len(completed):02d}-{GOALS[goal]}.png")
            print(f"frame {elapsed}: completed {GOALS[goal]}; total={current['completed']}", flush=True)

        # Intercept the first Center approach while Giovanni is still some
        # distance away. The player walks back to the original entrance tile.
        if (
            not intercept_attempted and current["goal"] == 2 and current["phase"] == 2
            and abs(current["x"] - current["goal_x"])
                + abs(current["y"] - current["goal_y"]) > 4
        ):
            intercept_attempted = True
            before = current
            try:
                move_player_to(client, symbols, 30 + MAP_OFFSET, 37 + MAP_OFFSET)
                elapsed += 40
                client.screenshot(output / "center-player-obstruction.png")
                print(f"frame {elapsed}: player walked into Center approach", flush=True)
                for _ in range(150):
                    if elapsed + 8 > max_frames:
                        break
                    client.step(8)
                    elapsed += 8
                    observed = walker_snapshot(client, symbols)
                    if (
                        observed["blocked_steps"] > before["blocked_steps"]
                        and observed["replans"] > before["replans"]
                    ):
                        blocked = True
                        result["real_player_block"] = {
                            "observed": True, "frame": elapsed, "before": before,
                            "after": observed, "player": [30, 37],
                        }
                        client.screenshot(output / "center-block-replan.png")
                        print(f"frame {elapsed}: blocked step and replan observed", flush=True)
                        interaction = try_talk_to_walker(client, symbols, output, observed)
                        result["interaction"] = interaction
                        elapsed += int(interaction.get("frame_cost", 0))
                        break
            finally:
                move_player_to(client, symbols, 28 + MAP_OFFSET, 37 + MAP_OFFSET)
                elapsed += 40
            if not blocked:
                result["real_player_block"] = {
                    "observed": False, "reason": "No blocked-step and replan counter increase during Center approach",
                    "before": before, "after": walker_snapshot(client, symbols),
                }
        last = current
        result["frames_after_startup"] = elapsed
        result["completed_goals"] = [GOALS[g] for g in completed]
        result["transitions"] = transitions
        result["reentries"] = reentries
        if (
            len(set(completed)) >= min_goals and blocked
            and (min_goals < 6 or any(r["first_walk_position"] for r in reentries))
        ):
            break
    result["final"] = walker_snapshot(client, symbols)
    result["frames_after_startup"] = elapsed
    result["distinct_goals"] = sorted({GOALS[g] for g in completed})
    result["travel_reentry_observed"] = any(r["first_walk_position"] for r in reentries)
    result["viridian"] = read_location(client, symbols)
    result["player_object"] = read_player_object(client, symbols)
    return result


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, required=True, help="built POC .gba")
    parser.add_argument("--symbols", type=Path, required=True, help="matching objdump .sym")
    parser.add_argument("--skyemu", type=Path, default=DEFAULT_SKYEMU)
    parser.add_argument("--output", type=Path, required=True, help="screenshots and JSON results")
    parser.add_argument("--max-frames", type=int, default=30000)
    parser.add_argument("--min-goals", type=int, default=3,
                        help="minimum distinct completed goals (1–6; use 6 for full tour)")
    parser.add_argument("--startup-only", action="store_true", help="debug startup before walker is linked")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    for path in (args.rom, args.symbols, args.skyemu):
        if not path.is_file():
            raise FileNotFoundError(path)
    if args.max_frames <= 0:
        raise ValueError("--max-frames must be positive")
    if not 1 <= args.min_goals <= len(GOALS):
        raise ValueError("--min-goals must be between 1 and 6")
    args.output.mkdir(parents=True, exist_ok=True)
    symbols = read_symbols(args.symbols)
    required = {"gSaveBlock1Ptr", "gObjectEvents"}
    if not args.startup_only:
        required.add("gViridianWalkerDebug")
    if missing := sorted(required - symbols.keys()):
        raise RuntimeError(f"symbol file lacks {', '.join(missing)}")
    port = reserve_port()
    with tempfile.TemporaryDirectory(prefix="viridian-walker-skyemu-") as temp:
        temp_dir = Path(temp)
        rom = temp_dir / "walker.gba"
        shutil.copyfile(args.rom, rom)
        log_path = args.output / "skyemu.log"
        with log_path.open("wb") as log:
            process = subprocess.Popen(
                ["xvfb-run", "--auto-servernum", str(args.skyemu), "http_server", str(port), str(rom)],
                env={**os.environ, "XDG_DATA_HOME": str(temp_dir / "xdg")},
                stdin=subprocess.DEVNULL,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                start_new_session=True,
            )
            announced = threading.Event()

            def copy_output() -> None:
                assert process.stdout is not None
                for line in process.stdout:
                    log.write(line)
                    log.flush()
                    if f"Starting HCS: http://localhost:{port}".encode() in line:
                        announced.set()

            output_thread = threading.Thread(target=copy_output, daemon=True)
            output_thread.start()
            client = SkyEmu(port)
            try:
                wait_for_ready(client, process, announced)
                client.check("/load_rom", {"path": str(rom), "pause": "1"})
                for button in ("A", "B", "Up", "Down", "Left", "Right", "L", "R", "Start", "Select"):
                    client.check("/input", {button: "0"})
                frame = boot_new_game(client, symbols, args.output, args.startup_only)
                result = {"startup_frames": frame, "viridian": read_location(client, symbols)}
                if not args.startup_only:
                    try:
                        result.update(run_walker(client, symbols, args.output, args.max_frames,
                                                 args.min_goals))
                    except Exception as error:
                        result["error"] = str(error)
                        result["last_walker_telemetry"] = walker_snapshot(client, symbols)
                        client.screenshot(args.output / "failure.png")
                        (args.output / "results.json").write_text(json.dumps(result, indent=2) + "\n")
                        raise
                (args.output / "results.json").write_text(json.dumps(result, indent=2) + "\n")
                if not args.startup_only:
                    if len(result["distinct_goals"]) < args.min_goals:
                        raise RuntimeError(f"Walker did not complete {args.min_goals} distinct goals")
                    if not result["real_player_block"]["observed"]:
                        raise RuntimeError("Real-player block and replan were not observed")
                    if args.min_goals == 6 and not result["travel_reentry_observed"]:
                        raise RuntimeError("No travel exit re-entry was observed")
                return 0
            finally:
                if process.poll() is None:
                    os.killpg(process.pid, signal.SIGTERM)
                    try:
                        process.wait(timeout=10)
                    except subprocess.TimeoutExpired:
                        os.killpg(process.pid, signal.SIGKILL)
                        process.wait(timeout=10)
                output_thread.join(timeout=2)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as exc:
        print(f"verification failed: {exc}", file=sys.stderr)
        sys.exit(1)

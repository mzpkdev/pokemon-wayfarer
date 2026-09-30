#!/usr/bin/env python3
"""Exercise a fresh Viridian walker POC ROM through SkyEmu's HTTP server.

This deliberately does not use the E2E arrange hook. It starts with no save,
selects New Game on the title screen, and reads only ROM-exported telemetry.
"""

from __future__ import annotations

import argparse
import contextlib
import hashlib
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
VIRIDIAN_MAP = 32
ROUTE2_MAP = 42
FOREST_GATE_MAP = (23 << 8) | 2
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


@contextlib.contextmanager
def skyemu_session(binary: Path, rom: Path, data_home: Path, log_path: Path):
    """Run one emulator process; a second session reloads the same ROM/flash."""
    port = reserve_port()
    with log_path.open("ab") as log:
        process = subprocess.Popen(
            ["xvfb-run", "--auto-servernum", str(binary), "http_server", str(port), str(rom)],
            env={**os.environ, "XDG_DATA_HOME": str(data_home)},
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
            yield client
        finally:
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGTERM)
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait(timeout=10)
            output_thread.join(timeout=2)


def read_player_object(client: SkyEmu, symbols: dict[str, int]) -> tuple[int, int, int] | None:
    base = symbols["gObjectEvents"]
    for index in range(16):
        address = base + index * 0x24
        raw = client.read(address, 0x18)
        if raw[0] & 1 and raw[2] & 1:
            x, y = struct.unpack_from("<hh", raw, 0x10)
            return address, x, y
    return None


def field_controls_locked(client: SkyEmu, symbols: dict[str, int]) -> bool:
    return bool(client.read(symbols["sLockFieldControls"], 1)[0])


def walker_snapshot(client: SkyEmu, symbols: dict[str, int]) -> dict[str, int]:
    raw = client.read(symbols["gViridianWalkerDebug"], 36)
    values = struct.unpack("<9H6h4BH", raw)
    keys = (
        "completed", "replans", "searches", "search_nodes", "search_frames",
        "max_nodes_per_frame", "max_slice_vblanks", "grass_tiles_found",
        "blocked_steps", "x", "y",
        "goal_x", "goal_y", "next_x", "next_y", "phase", "goal",
        "last_completed_goal", "used_grass_fallback", "max_slice_scanlines",
    )
    return dict(zip(keys, values, strict=True))


def actor_snapshot(client: SkyEmu, symbols: dict[str, int]) -> dict[str, int]:
    snapshot = walker_snapshot(client, symbols)
    raw = client.read(symbols["gViridianWalkerDebug"] + 36, 3)
    snapshot["current_map"], snapshot["local_id"] = struct.unpack("<HB", raw)
    return snapshot


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
    client: SkyEmu, symbols: dict[str, int], target_x: int, target_y: int,
    max_steps: int = 8,
) -> int:
    """Walk by real D-pad input along caller-selected clear waypoints."""
    frames = 0
    for _ in range(max_steps):
        player = read_player_object(client, symbols)
        if player is None:
            raise RuntimeError("Player object vanished while positioning obstruction")
        _, x, y = player
        if (x, y) == (target_x, target_y):
            return frames
        if y != target_y:
            direction = "Down" if target_y > y else "Up"
        else:
            direction = "Right" if target_x > x else "Left"
        # The first keypress may only change facing, as it did in the first
        # emulator run. Later presses may land during map-name or fade locks.
        for attempt in range(6):
            client.press(direction, hold=3, release=19)
            frames += 22
            updated = read_player_object(client, symbols)
            if updated is not None and updated[1:] != (x, y):
                break
            if attempt < 5:
                client.step(12)
                frames += 12
        else:
            raise RuntimeError(f"Player could not walk {direction} from {(x, y)} after six presses")
    raise RuntimeError(f"Player did not reach {(target_x, target_y)}")


def move_player_straight_held(
    client: SkyEmu, symbols: dict[str, int], direction: str, target: int,
    max_frames: int = 800,
) -> int:
    """Hold one real direction, stopping as soon as the target tile begins."""
    axis = 1 if direction in ("Left", "Right") else 2
    client.check("/input", {direction: "1"})
    try:
        for elapsed in range(2, max_frames + 1, 2):
            client.step(2)
            player = read_player_object(client, symbols)
            if player and player[axis] == target:
                return elapsed
    finally:
        client.check("/input", {direction: "0"})
    raise RuntimeError(f"Held {direction} did not reach grid coordinate {target}: {player}")


def read_world_record(client: SkyEmu, symbols: dict[str, int]) -> dict[str, int | str]:
    """Read the actual 12-byte SaveBlock3 record, never a RAM-only mirror."""
    base = client.u32(symbols["gSaveBlock3Ptr"])
    offset = client.u16(symbols["gViridianWalkerWorldSaveOffset"])
    size = client.u16(symbols["gViridianWalkerWorldSaveSize"])
    if not 0x02000000 <= base < 0x02040000 or size != 12:
        raise RuntimeError(f"Invalid world save record location/size: {base:#x}+{offset}, {size}")
    raw = client.read(base + offset, size)
    values = struct.unpack("<HH8B", raw)
    keys = ("current_map", "destination_map", "arrival", "state", "crossing",
            "x", "y", "goal", "dwell", "reserved")
    # Seven byte fields plus one alignment byte follow the two map IDs.
    # Keep raw bytes to compare across the real save/reload without normalizing.
    data: dict[str, int | str] = dict(zip(keys, values, strict=True))
    data["raw_hex"] = raw.hex()
    return data


def read_world_debug(client: SkyEmu, symbols: dict[str, int]) -> dict[str, int]:
    raw = client.read(symbols["gViridianWalkerWorldDebug"], 14)
    values = struct.unpack("<5H4B", raw)
    keys = ("heartbeats", "hops", "actor_exits", "from_map", "to_map",
            "last_arrival", "last_crossing", "last_x", "last_y")
    return dict(zip(keys, values, strict=True))


def read_giovanni_object(client: SkyEmu, symbols: dict[str, int], map_num: int) -> dict[str, int] | None:
    """Require a visible, active Giovanni object rather than trusting world state alone."""
    for index in range(16):
        raw = client.read(symbols["gObjectEvents"] + index * 0x24, 0x24)
        if not raw[0] & 1 or raw[1] & 0x20 or struct.unpack_from("<H", raw, 4)[0] != 485:
            continue
        if raw[9] == map_num and raw[10] == 0:
            x, y = struct.unpack_from("<hh", raw, 0x10)
            return {"slot": index, "local_id": raw[8], "x": x - MAP_OFFSET, "y": y - MAP_OFFSET}
    return None


def active_giovanni_objects(client: SkyEmu, symbols: dict[str, int]) -> list[dict[str, int]]:
    actors = []
    all_objects = client.read(symbols["gObjectEvents"], 16 * 0x24)
    for index in range(16):
        raw = all_objects[index * 0x24:(index + 1) * 0x24]
        if raw[0] & 1 and struct.unpack_from("<H", raw, 4)[0] == 485 and raw[10] == 0 and raw[9] in (32, 41, 42):
            x, y = struct.unpack_from("<hh", raw, 0x10)
            actors.append({"slot": index, "map_num": raw[9], "local_id": raw[8],
                           "x": x - MAP_OFFSET, "y": y - MAP_OFFSET})
    return actors


def seam_frame(client: SkyEmu, symbols: dict[str, int], frame: int,
               output: Path) -> dict[str, object]:
    """Read the real object and sprite state on one emulated video frame."""
    actors = []
    player = read_player_object(client, symbols)
    # SaveBlock1's map changes before camera update rebases object coordinates.
    # The player object is in the same coordinate frame as every other object:
    # Viridian north y is near 7, Route2 south y is near 86.
    route2_object_frame = player is not None and player[2] >= 50
    raw_objects = client.read(symbols["gObjectEvents"], 16 * 0x24)
    for slot in range(16):
        raw = raw_objects[slot * 0x24:(slot + 1) * 0x24]
        if not raw[0] & 1 or struct.unpack_from("<H", raw, 4)[0] != 485:
            continue
        x, y = struct.unpack_from("<hh", raw, 0x10)
        sprite_id = raw[0x23]
        sprite = client.read(symbols["gSprites"] + sprite_id * 0x44, 0x44)
        sx, sy, sx2, sy2 = struct.unpack_from("<hhhh", sprite, 0x20)
        corner_x, corner_y = struct.unpack_from("<bb", sprite, 0x28)
        flags = sprite[0x3E]
        offset_x = client.u16(symbols["gSpriteCoordOffsetX"])
        offset_y = client.u16(symbols["gSpriteCoordOffsetY"])
        offset_x = struct.unpack("<h", struct.pack("<H", offset_x))[0]
        offset_y = struct.unpack("<h", struct.pack("<H", offset_y))[0]
        actor = {"slot": slot, "map_group": raw[10], "map_num": raw[9],
                 "local_id": raw[8], "x": x - MAP_OFFSET, "y": y - MAP_OFFSET,
                 "object_invisible": bool(raw[1] & 0x20),
                 "object_offscreen": bool(raw[1] & 0x40),
                 "sprite_id": sprite_id, "sprite_in_use": bool(flags & 1),
                 "sprite_invisible": bool(flags & 4),
                 "screen_x": sx + sx2 + corner_x + (offset_x if flags & 2 else 0),
                 "screen_y": sy + sy2 + corner_y + (offset_y if flags & 2 else 0)}
        actor["rendered"] = (actor["sprite_in_use"] and not actor["sprite_invisible"]
                             and not actor["object_invisible"]
                             and -32 <= actor["screen_x"] <= 239
                             and -32 <= actor["screen_y"] <= 159)
        if route2_object_frame:
            actor["route2_x"], actor["route2_y"] = actor["x"], actor["y"]
        else:
            actor["route2_x"], actor["route2_y"] = actor["x"] - 16, actor["y"] + 80
        actors.append(actor)
    sample: dict[str, object] = {"frame": frame, "location": read_location(client, symbols),
                                 "player": player, "route2_object_frame": route2_object_frame,
                                 "world": read_world_record(client, symbols), "actors": actors}
    client.screenshot(output / f"{frame:04d}.png")
    return sample


def phase2_seam(client: SkyEmu, symbols: dict[str, int], output: Path,
                follow: str, bounce: bool) -> dict[str, object]:
    """Observe the north connection continuously, then cross using real input."""
    wait_field_unlocked(client, symbols)
    # x24 is a clear north road lane; the wandering NPC can block x26.
    move_player_straight_held(client, symbols, "Left", 24 + MAP_OFFSET)
    client.step(18)
    move_player_straight_held(client, symbols, "Up", MAP_OFFSET)
    client.step(18)
    if (location := read_location(client, symbols))[:2] != (0, 32):
        raise RuntimeError(f"Player crossed before seam observation: {location}")
    for waited in range(0, 10000, 5):
        walker = actor_snapshot(client, symbols)
        world = read_world_record(client, symbols)
        if (world["current_map"] == VIRIDIAN_MAP and walker["goal"] == 4
            and walker["phase"] == 2 and walker["y"] - MAP_OFFSET <= 4):
            break
        client.step(5)
    else:
        raise RuntimeError(f"Giovanni did not approach north seam: {walker}, {world}")

    frames_dir = output / f"seam-{follow}-frames"
    frames_dir.mkdir(exist_ok=True)
    evidence: dict[str, object] = {"follow": follow, "bounce_requested": bounce,
                                   "frames_waiting_for_approach": waited,
                                   "samples": [], "violations": []}
    samples: list[dict[str, object]] = evidence["samples"]
    violations: list[str] = evidence["violations"]
    departure_frame = None
    first_route2_actor_y = None
    last_visible_y = None
    try:
        for frame in range(340 if follow == "late" else 220):
            sample = seam_frame(client, symbols, frame, frames_dir)
            samples.append(sample)
            current_map = sample["world"]["current_map"]
            visible = [a for a in sample["actors"] if a["rendered"]]
            if len(sample["actors"]) > 1 or len(visible) > 1:
                violations.append(f"frame {frame}: duplicate Giovanni objects/sprites")
            if current_map == ROUTE2_MAP and departure_frame is None:
                departure_frame = frame
                first_route2_actor_y = sample["world"]["y"]
                evidence["departure_frame"] = frame
                evidence["departure_visible"] = bool(visible)
                if not visible:
                    violations.append(f"frame {frame}: Giovanni absent at Route2 world departure")
            if current_map == ROUTE2_MAP and sample["location"][:2] == (0, 32):
                if visible:
                    last_visible_y = visible[0]["screen_y"]
                elif last_visible_y is not None and last_visible_y >= 0:
                    violations.append(f"frame {frame}: Giovanni vanished while still on screen at y={last_visible_y}")
                    last_visible_y = None
            if departure_frame is not None:
                held = {"prompt": 1, "delayed": 60, "late": 160}[follow]
                if frame - departure_frame >= held:
                    evidence["viridian_frames_after_departure"] = frame - departure_frame
                    break
            client.step(1)
        else:
            raise RuntimeError("Seam sampling ended before Giovanni departed")
        if first_route2_actor_y is None:
            raise RuntimeError("No Route2 world departure was observed")
        if follow == "delayed" and samples[-1]["world"]["y"] > first_route2_actor_y - 2:
            violations.append("Giovanni did not continue at least two Route2 tiles before player crossing")
        if follow == "late":
            seam_samples = samples[departure_frame:]
            visible_y = [a["screen_y"] for s in seam_samples for a in s["actors"] if a["rendered"]]
            evidence["late_last_world_y"] = samples[-1]["world"]["y"]
            evidence["late_lowest_visible_screen_y"] = min(visible_y) if visible_y else None
            evidence["late_actor_visible_at_follow"] = any(a["rendered"] for a in samples[-1]["actors"])
            if samples[-1]["world"]["y"] > first_route2_actor_y - 5:
                violations.append("Giovanni stopped before walking five Route2 tiles past the seam")
            if evidence["late_actor_visible_at_follow"]:
                violations.append("Giovanni remained visible at the connection strip edge after 160 frames")
            if not visible_y or min(visible_y) > -32:
                violations.append("Giovanni did not visibly walk above the viewport before leaving it")
        # The player stands on Viridian y=0. One real Up step crosses the map
        # connection; sample every frame of the key hold and field transition.
        frame = len(samples)
        for attempt in range(5):
            client.check("/input", {"Up": "1"})
            for _ in range(3):
                client.step(1)
                sample = seam_frame(client, symbols, frame, frames_dir)
                samples.append(sample)
                frame += 1
            client.check("/input", {"Up": "0"})
            for _ in range(20):
                client.step(1)
                sample = seam_frame(client, symbols, frame, frames_dir)
                samples.append(sample)
                frame += 1
            if samples[-1]["location"][:2] == (0, 42):
                evidence["crossing_frame"] = frame - 1
                break
        else:
            raise RuntimeError(f"Player failed to cross seam: {samples[-1]['location']}")
        if bounce:
            before_bounce = read_world_debug(client, symbols)
            visible_before = [a for a in samples[-1]["actors"] if a["rendered"]]
            evidence["bounce"] = {"before_debug": before_bounce,
                                   "actor_visible_before": bool(visible_before),
                                   "start_frame": frame}
            if not visible_before:
                violations.append("Giovanni was not visible before the player bounced between maps")
            for direction, target_map in (("Down", (0, 32)), ("Up", (0, 42))):
                for attempt in range(8):
                    client.check("/input", {direction: "1"})
                    try:
                        for _ in range(3):
                            client.step(1)
                            sample = seam_frame(client, symbols, frame, frames_dir)
                            samples.append(sample)
                            frame += 1
                    finally:
                        client.check("/input", {direction: "0"})
                    for _ in range(20):
                        client.step(1)
                        sample = seam_frame(client, symbols, frame, frames_dir)
                        samples.append(sample)
                        frame += 1
                    if samples[-1]["location"][:2] == target_map:
                        evidence["bounce"][f"{direction.lower()}_crossing_frame"] = frame - 1
                        break
                else:
                    raise RuntimeError(f"Player failed to bounce {direction} into {target_map}")
            after_bounce = read_world_debug(client, symbols)
            evidence["bounce"]["after_debug"] = after_bounce
            evidence["bounce"]["world"] = read_world_record(client, symbols)
            if after_bounce["hops"] != before_bounce["hops"]:
                violations.append("Travel hop advanced visible Giovanni during player bounce")
            if evidence["bounce"]["world"]["current_map"] != ROUTE2_MAP:
                violations.append("Giovanni left Route2 during short player bounce")
        for _ in range(30):
            client.step(1)
            sample = seam_frame(client, symbols, frame, frames_dir)
            samples.append(sample)
            frame += 1
        if samples[-1]["world"]["current_map"] != ROUTE2_MAP:
            violations.append("Giovanni world record left Route2 during player crossing")
        last_rendered = None
        for sample in samples:
            if len(sample["actors"]) > 1:
                violations.append(f"frame {sample['frame']}: duplicate Giovanni objects")
            if sample["world"]["current_map"] != ROUTE2_MAP:
                continue
            rendered = [actor for actor in sample["actors"] if actor["rendered"]]
            if rendered:
                current = rendered[0]
                if last_rendered and sample["frame"] == last_rendered["frame"] + 1:
                    pixel_delta = max(abs(current["screen_x"] - last_rendered["screen_x"]),
                                      abs(current["screen_y"] - last_rendered["screen_y"]))
                    if pixel_delta > 8:
                        violations.append(f"frame {sample['frame']}: visible Giovanni jumped {pixel_delta} screen pixels")
                last_rendered = {"frame": sample["frame"], **current}
            elif last_rendered and (-16 <= last_rendered["screen_x"] <= 239
                                    and -16 <= last_rendered["screen_y"] <= 159):
                violations.append(f"frame {sample['frame']}: Giovanni disappeared at visible screen position "
                                  f"({last_rendered['screen_x']},{last_rendered['screen_y']})")
                last_rendered = None
        # Actor coordinates from source and destination maps share one Route2
        # coordinate system. A tile jump at the seam is never a walk animation.
        previous = None
        for sample in samples:
            actors = sample["actors"]
            if len(actors) != 1:
                continue
            actor = actors[0]
            if "route2_x" not in actor:
                continue
            if previous is not None:
                distance = abs(actor["route2_x"] - previous["route2_x"]) + abs(actor["route2_y"] - previous["route2_y"])
                elapsed = sample["frame"] - previous["frame"]
                if distance > 1 + elapsed // 12:
                    violations.append(f"frame {sample['frame']}: Giovanni jumped {distance} tiles in {elapsed} frames")
            previous = {"frame": sample["frame"], "route2_x": actor["route2_x"],
                        "route2_y": actor["route2_y"]}
        evidence["final_location"] = samples[-1]["location"]
        evidence["final_world"] = samples[-1]["world"]
        if violations:
            raise RuntimeError(f"North seam continuity failed: {violations[:6]}")
        return evidence
    finally:
        (output / f"seam-{follow}-trace.json").write_text(json.dumps(evidence, indent=2) + "\n")


def save_block1_prefix(client: SkyEmu, symbols: dict[str, int]) -> dict[str, object]:
    pointer = client.u32(symbols["gSaveBlock1Ptr"])
    if not 0x02000000 <= pointer < 0x02040000:
        raise RuntimeError(f"Invalid SaveBlock1 pointer {pointer:#x}")
    raw = client.read(pointer, 32)
    return {"save_version": struct.unpack_from("<H", raw)[0], "prefix_hex": raw.hex(),
            "save_counter": client.u32(symbols["gSaveCounter"])}


def wait_field_unlocked(client: SkyEmu, symbols: dict[str, int], limit: int = 900) -> int:
    for elapsed in range(0, limit + 1, 30):
        if not field_controls_locked(client, symbols) and read_player_object(client, symbols):
            return elapsed
        client.step(30)
    raise RuntimeError(f"Field controls stayed locked for {limit} emulated frames")


def wait_for_world_map(
    client: SkyEmu, symbols: dict[str, int], map_id: int, limit: int,
) -> tuple[dict[str, int | str], int]:
    for elapsed in range(0, limit + 1, 30):
        record = read_world_record(client, symbols)
        if record["current_map"] == map_id:
            return record, elapsed
        client.step(30)
    raise RuntimeError(f"Giovanni never reached saved map {map_id:#x}; last={record}")


def enter_center(client: SkyEmu, symbols: dict[str, int], output: Path, label: str) -> dict[str, object]:
    if (location := read_location(client, symbols)) is None or location[:2] != (0, 32):
        raise RuntimeError(f"Center entry requires Viridian, got {location}")
    wait_field_unlocked(client, symbols)
    move_player_to(client, symbols, 30 + MAP_OFFSET, 37 + MAP_OFFSET)
    for _ in range(10):
        client.press("Up", hold=3, release=25)
        client.step(30)
        location = read_location(client, symbols)
        if location and location[:2] == (12, 0):
            wait_field_unlocked(client, symbols)
            client.screenshot(output / f"{label}-center-inside.png")
            return {"location": location, "world": read_world_record(client, symbols),
                    "world_debug": read_world_debug(client, symbols)}
    raise RuntimeError(f"Player did not enter Center through door: {location}")


def leave_center(client: SkyEmu, symbols: dict[str, int], output: Path, label: str) -> dict[str, object]:
    if (location := read_location(client, symbols)) is None or location[:2] != (12, 0):
        raise RuntimeError(f"Center exit requires interior, got {location}")
    wait_field_unlocked(client, symbols)
    for _ in range(10):
        client.press("Down", hold=3, release=25)
        client.step(30)
        location = read_location(client, symbols)
        if location and location[:2] == (0, 32):
            wait_field_unlocked(client, symbols)
            client.screenshot(output / f"{label}-viridian-return.png")
            return {"location": location, "world": read_world_record(client, symbols),
                    "world_debug": read_world_debug(client, symbols)}
    raise RuntimeError(f"Player did not leave Center through door: {location}")


def phase2_follow(client: SkyEmu, symbols: dict[str, int], output: Path) -> dict[str, object]:
    """Walk the real Viridian north connection shortly after Giovanni exits."""
    evidence: dict[str, object] = {"fresh_world": read_world_record(client, symbols),
                                   "fresh_debug": read_world_debug(client, symbols)}
    if evidence["fresh_world"]["current_map"] != VIRIDIAN_MAP:
        raise RuntimeError(f"Fresh Giovanni is not in Viridian: {evidence['fresh_world']}")
    wait_field_unlocked(client, symbols)
    # The central x26 lane has a wandering NPC; x24 is the clear road lane.
    move_player_straight_held(client, symbols, "Left", 24 + MAP_OFFSET)
    client.step(18)
    move_player_straight_held(client, symbols, "Up", 3 + MAP_OFFSET)
    client.step(18)
    client.screenshot(output / "follow-viridian-north-wait.png")
    departed, waited = wait_for_world_map(client, symbols, ROUTE2_MAP, 10000)
    evidence["departed_to_route2"] = departed
    evidence["frames_waiting_near_exit"] = waited
    evidence["debug_after_exit"] = read_world_debug(client, symbols)
    expected_x = departed["crossing"] - 16
    exit_debug = evidence["debug_after_exit"]
    if (departed["arrival"] != 2 or exit_debug["from_map"] != VIRIDIAN_MAP
        or exit_debug["to_map"] != ROUTE2_MAP or exit_debug["last_arrival"] != 2
        or exit_debug["last_crossing"] != departed["crossing"]
        or exit_debug["last_x"] != expected_x or exit_debug["last_y"] != 79):
        raise RuntimeError(f"Route2 exit did not match north connection: {departed}, {exit_debug}")
    client.screenshot(output / "follow-giovanni-departed.png")
    # From Viridian x24, the +16 connection offset places the player at
    # Route2 x8. Stay behind the actor rather than teleporting or arranging.
    move_player_to(client, symbols, 24 + MAP_OFFSET, MAP_OFFSET, max_steps=5)
    for attempt in range(10):
        client.press("Up", hold=3, release=25)
        client.step(15)
        location = read_location(client, symbols)
        if location and location[:2] == (0, 42):
            evidence["player_route2_location"] = location
            evidence["crossing_attempts"] = attempt + 1
            break
    else:
        raise RuntimeError(f"Player did not cross north connection to Route2: {location}")
    for elapsed in range(0, 600, 10):
        actor = read_giovanni_object(client, symbols, 42)
        if actor:
            evidence["first_route2_actor"] = actor
            evidence["actor_observed_after_frames"] = elapsed
            break
        client.step(10)
    else:
        raise RuntimeError(f"Giovanni object absent on Route2 after follow; world={read_world_record(client, symbols)}")
    wait_field_unlocked(client, symbols)
    client.screenshot(output / "follow-route2-giovanni.png")
    evidence["world_on_route2"] = read_world_record(client, symbols)
    evidence["route2_actor_debug"] = actor_snapshot(client, symbols)
    evidence["route2_active_giovanni"] = active_giovanni_objects(client, symbols)
    if evidence["world_on_route2"]["current_map"] != ROUTE2_MAP:
        raise RuntimeError(f"Giovanni world record left Route2 before player arrived: {evidence['world_on_route2']}")
    if evidence["route2_actor_debug"]["current_map"] != ROUTE2_MAP or actor["local_id"] != evidence["route2_actor_debug"]["local_id"]:
        raise RuntimeError(f"Route2 object and actor telemetry disagree: {actor}, {evidence['route2_actor_debug']}")
    if len(evidence["route2_active_giovanni"]) != 1:
        raise RuntimeError(f"Expected exactly one Giovanni on Route2: {evidence['route2_active_giovanni']}")
    if actor["y"] < 68:
        raise RuntimeError(f"Giovanni was not near Route2 south edge: {actor}")
    if abs(actor["x"] - expected_x) > 5:
        raise RuntimeError(f"Giovanni did not enter near matching Route2 x={expected_x}: {actor}")
    for elapsed in range(0, 900, 30):
        client.step(30)
        moved = read_giovanni_object(client, symbols, 42)
        if moved and (moved["x"], moved["y"]) != (actor["x"], actor["y"]):
            evidence["route2_actor_moved"] = moved
            evidence["movement_after_frames"] = elapsed + 30
            client.screenshot(output / "follow-route2-moving.png")
            break
    else:
        raise RuntimeError(f"Giovanni did not continue walking on Route2: {actor}")

    before_gate = read_world_debug(client, symbols)
    evidence["before_gate_debug"] = before_gate
    for elapsed in range(0, 6000, 30):
        client.step(30)
        actors = active_giovanni_objects(client, symbols)
        if len(actors) > 1:
            raise RuntimeError(f"Duplicate Giovanni objects during Route2 walk: {actors}")
        world = read_world_record(client, symbols)
        if world["current_map"] == FOREST_GATE_MAP:
            after_gate = read_world_debug(client, symbols)
            evidence["gate_handoff"] = {
                "frames_after_first_route2_step": elapsed + 30,
                "world": world,
                "before_debug": before_gate,
                "after_debug": after_gate,
                "active_giovanni": actors,
                "last_actor_cost": actor_snapshot(client, symbols),
            }
            client.screenshot(output / "follow-route2-gate-handoff.png")
            if world["state"] != 2 or world["arrival"] != 5:
                raise RuntimeError(f"Route2 gate handoff lacks inside/door state: {world}")
            if after_gate["actor_exits"] <= before_gate["actor_exits"]:
                raise RuntimeError(f"Route2 gate transition was not reported by local actor: {after_gate}")
            if after_gate["heartbeats"] != before_gate["heartbeats"]:
                raise RuntimeError(f"Route2 gate transition came from an off-screen heartbeat: {before_gate}, {after_gate}")
            if actors:
                raise RuntimeError(f"Giovanni was not removed after entering gate: {actors}")
            return evidence
    raise RuntimeError(f"Giovanni did not enter Forest gate from Route2: {read_world_record(client, symbols)}")


def phase2_seam_return(client: SkyEmu, symbols: dict[str, int], output: Path) -> dict[str, object]:
    """Cross south ahead of Giovanni and watch his actual Route2 exit."""
    evidence: dict[str, object] = {"outbound": phase2_follow(client, symbols, output),
                                   "heartbeats": [], "samples": [], "violations": []}
    # Forest gate dwell is released by two real player map transitions. Keep
    # the player at Route2's south edge while Giovanni walks back from y52.
    for direction, expected_map in (("Down", (0, 32)), ("Up", (0, 42))):
        for attempt in range(10):
            client.press(direction, hold=3, release=25)
            client.step(15)
            if (location := read_location(client, symbols))[:2] == expected_map:
                wait_field_unlocked(client, symbols)
                evidence["heartbeats"].append({"direction": direction, "location": location,
                                                "world": read_world_record(client, symbols),
                                                "debug": read_world_debug(client, symbols)})
                client.screenshot(output / f"return-player-{direction.lower()}.png")
                break
        else:
            raise RuntimeError(f"Player did not cross {direction} for return setup: {location}")
    if evidence["heartbeats"][-1]["world"]["current_map"] != ROUTE2_MAP:
        raise RuntimeError(f"Gate dwell did not return Giovanni to Route2: {evidence['heartbeats']}")
    for elapsed in range(0, 6000, 5):
        world = read_world_record(client, symbols)
        actor = read_giovanni_object(client, symbols, 42)
        if world["current_map"] != ROUTE2_MAP:
            raise RuntimeError(f"Giovanni left Route2 before player could watch south approach: {world}")
        if actor and actor["y"] >= 75 and actor_snapshot(client, symbols)["goal"] == 10:
            evidence["return_approach_frames"] = elapsed
            evidence["return_approach_actor"] = actor
            break
        client.step(5)
    else:
        raise RuntimeError(f"Giovanni never approached Route2 south edge: {world}, {actor}")

    frames_dir = output / "seam-return-frames"
    frames_dir.mkdir(exist_ok=True)
    samples: list[dict[str, object]] = evidence["samples"]
    violations: list[str] = evidence["violations"]
    before_exit = read_world_debug(client, symbols)
    evidence["before_return_exit_debug"] = before_exit
    frame = 0
    try:
        sample = seam_frame(client, symbols, frame, frames_dir)
        samples.append(sample)
        if not any(a["rendered"] for a in sample["actors"]):
            violations.append("Giovanni was not visible at Route2 south approach")
        frame += 1
        for attempt in range(8):
            client.check("/input", {"Down": "1"})
            try:
                for _ in range(3):
                    client.step(1)
                    samples.append(seam_frame(client, symbols, frame, frames_dir))
                    frame += 1
            finally:
                client.check("/input", {"Down": "0"})
            for _ in range(20):
                client.step(1)
                samples.append(seam_frame(client, symbols, frame, frames_dir))
                frame += 1
            if samples[-1]["location"][:2] == (0, 32):
                evidence["player_crossing_frame"] = frame - 1
                break
        else:
            raise RuntimeError(f"Player did not cross south ahead of Giovanni: {samples[-1]['location']}")
        return_frame = None
        for _ in range(240):
            client.step(1)
            sample = seam_frame(client, symbols, frame, frames_dir)
            samples.append(sample)
            if sample["world"]["current_map"] == VIRIDIAN_MAP and return_frame is None:
                return_frame = frame
                evidence["actor_return_frame"] = frame
            frame += 1
            if return_frame is not None and frame - return_frame >= 30:
                break
        if return_frame is None:
            raise RuntimeError(f"Giovanni never actually crossed south into Viridian: {samples[-1]['world']}")
        if return_frame <= evidence["player_crossing_frame"]:
            violations.append("Giovanni crossed before the player cleared the Route2 edge")

        last_visible = None
        previous_actor = None
        for sample in samples:
            actors = sample["actors"]
            if len(actors) > 1:
                violations.append(f"frame {sample['frame']}: duplicate Giovanni actors")
            visible = [a for a in actors if a["rendered"]]
            if visible:
                current = visible[0]
                if last_visible and sample["frame"] == last_visible["frame"] + 1:
                    dx = abs(current["screen_x"] - last_visible["screen_x"])
                    dy = abs(current["screen_y"] - last_visible["screen_y"])
                    if max(dx, dy) > 8:
                        violations.append(f"frame {sample['frame']}: Giovanni jumped {max(dx, dy)} pixels")
                last_visible = {"frame": sample["frame"], **current}
            elif last_visible and (-16 <= last_visible["screen_x"] <= 239
                                   and -16 <= last_visible["screen_y"] <= 159):
                violations.append(f"frame {sample['frame']}: Giovanni vanished at visible screen position")
                last_visible = None
            if len(actors) == 1:
                current = actors[0]
                if previous_actor and sample["frame"] == previous_actor["frame"] + 1:
                    distance = abs(current["route2_x"] - previous_actor["route2_x"]) + abs(current["route2_y"] - previous_actor["route2_y"])
                    if distance > 1:
                        violations.append(f"frame {sample['frame']}: Giovanni jumped {distance} tiles")
                    if (sample["world"]["current_map"] == ROUTE2_MAP
                        and current["route2_y"] < previous_actor["route2_y"]):
                        violations.append(f"frame {sample['frame']}: Giovanni reversed north during south approach")
                previous_actor = {"frame": sample["frame"], **current}
        exit_sample = samples[return_frame]
        if not exit_sample["actors"] or exit_sample["actors"][0]["route2_y"] < 80:
            violations.append("World switched to Viridian before Giovanni reached Route2 south edge")
        after_exit = read_world_debug(client, symbols)
        evidence["after_return_exit_debug"] = after_exit
        if (after_exit["actor_exits"] <= before_exit["actor_exits"]
            or after_exit["from_map"] != ROUTE2_MAP or after_exit["to_map"] != VIRIDIAN_MAP
            or after_exit["last_arrival"] != 1
            or after_exit["last_x"] != after_exit["last_crossing"] + 16
            or after_exit["last_y"] != 0):
            violations.append(f"South exit telemetry did not match Route2 connection: {after_exit}")
        if after_exit["hops"] != before_exit["hops"]:
            violations.append("Off-screen hop advanced Giovanni during visible south return")
        evidence["final_location"] = samples[-1]["location"]
        evidence["final_world"] = samples[-1]["world"]
        if violations:
            raise RuntimeError(f"South seam continuity failed: {violations[:6]}")
        return evidence
    finally:
        (output / "seam-return-trace.json").write_text(json.dumps(evidence, indent=2) + "\n")


def phase2_linger(client: SkyEmu, symbols: dict[str, int], output: Path) -> dict[str, object]:
    """Use real Center transitions to tick an off-screen traveller around the graph."""
    evidence: dict[str, object] = {"fresh_world": read_world_record(client, symbols),
                                   "fresh_debug": read_world_debug(client, symbols),
                                   "heartbeats": []}
    off_map, waited = wait_for_world_map(client, symbols, ROUTE2_MAP, 10000)
    evidence["left_viridian"] = off_map
    evidence["frames_until_north_exit"] = waited
    client.screenshot(output / "linger-after-north-exit.png")
    saw_gate = False
    for index in range(8):
        inside = enter_center(client, symbols, output, f"linger-{index:02d}")
        evidence["heartbeats"].append({"transition": "enter_center", **inside})
        saw_gate |= inside["world"]["current_map"] == FOREST_GATE_MAP
        outside = leave_center(client, symbols, output, f"linger-{index:02d}")
        evidence["heartbeats"].append({"transition": "return_viridian", **outside})
        saw_gate |= outside["world"]["current_map"] == FOREST_GATE_MAP
        if saw_gate and outside["world"]["current_map"] == VIRIDIAN_MAP:
            observed_maps = [tick["world"]["current_map"] for tick in evidence["heartbeats"]]
            gate_index = observed_maps.index(FOREST_GATE_MAP)
            if ROUTE2_MAP not in observed_maps[gate_index + 1:]:
                raise RuntimeError(f"Giovanni skipped Route2 on return from gate: {observed_maps}")
            if outside["world"]["arrival"] != 1:
                raise RuntimeError(f"Giovanni returned to Viridian from wrong edge: {outside['world']}")
            for elapsed in range(0, 240, 10):
                actor = read_giovanni_object(client, symbols, 32)
                if actor:
                    evidence["reentered_actor"] = actor
                    evidence["reentered_world"] = read_world_record(client, symbols)
                    evidence["reentered_actor_debug"] = actor_snapshot(client, symbols)
                    evidence["reentry_observed_after_frames"] = elapsed
                    client.screenshot(output / "linger-north-reentry.png")
                    if actor["y"] > 8:
                        raise RuntimeError(f"Giovanni returned away from Viridian north edge: {actor}")
                    expected_x = outside["world"]["crossing"] + 16
                    if abs(actor["x"] - expected_x) > 5:
                        raise RuntimeError(f"Giovanni returned at wrong Viridian x={expected_x}: {actor}")
                    return evidence
                client.step(10)
            raise RuntimeError(f"World says Viridian after gate but actor absent: {outside['world']}")
    raise RuntimeError(f"No gate round trip and Viridian re-entry in eight Center transitions: {evidence['heartbeats']}")


def select_start_action(client: SkyEmu, symbols: dict[str, int], action: int) -> list[int]:
    wait_field_unlocked(client, symbols)
    client.press("Start", hold=3, release=30)
    for _ in range(10):
        actions = list(client.read(symbols["sCurrentStartMenuActions"], 9))
        if field_controls_locked(client, symbols) and action in actions:
            break
        client.step(30)
    else:
        raise RuntimeError(f"Start menu did not open with action {action}: {actions}")
    target = actions.index(action)
    for _ in range(12):
        cursor = client.read(symbols["sStartMenuCursorPos"], 1)[0]
        if cursor == target:
            break
        client.press("Down", hold=3, release=19)
    else:
        raise RuntimeError(f"Could not select Start action {action}: cursor={cursor}, actions={actions}")
    client.press("A", hold=3, release=30)
    return actions


def phase2_save_before(client: SkyEmu, symbols: dict[str, int], output: Path) -> dict[str, object]:
    """Save with the game's own menu while Giovanni is off the player's map."""
    evidence: dict[str, object] = {"fresh_world": read_world_record(client, symbols)}
    off_map, waited = wait_for_world_map(client, symbols, ROUTE2_MAP, 10000)
    evidence["left_viridian"] = off_map
    evidence["frames_until_north_exit"] = waited
    evidence["center_entry"] = enter_center(client, symbols, output, "save")
    before = read_world_record(client, symbols)
    evidence["before_save"] = before
    evidence["save_block1_before_save"] = save_block1_prefix(client, symbols)
    if before["current_map"] == (12 << 8):
        raise RuntimeError("Giovanni is on player's Center map before save")
    evidence["start_menu_actions"] = select_start_action(client, symbols, 5)
    client.step(120)
    client.screenshot(output / "save-confirmation.png")
    # Fresh game: Save action -> confirmation text -> Yes/No default Yes ->
    # saving text -> success message/timer. A advances each real game prompt.
    for attempt in range(12):
        if not field_controls_locked(client, symbols):
            evidence["save_ui_a_presses"] = attempt
            break
        client.press("A", hold=3, release=90)
        if attempt in (0, 1, 2, 3):
            client.screenshot(output / f"save-progress-{attempt + 1}.png")
    else:
        raise RuntimeError("Game Save menu did not return to field after 12 A presses")
    client.step(180)
    after = read_world_record(client, symbols)
    evidence["after_save"] = after
    evidence["save_block1_after_save"] = save_block1_prefix(client, symbols)
    evidence["saved_field_location"] = read_location(client, symbols)
    if after["raw_hex"] != before["raw_hex"]:
        raise RuntimeError(f"Off-map world changed during Save UI: before={before}, after={after}")
    client.screenshot(output / "save-field-after-success.png")
    return evidence


def phase2_save_after(
    client: SkyEmu, symbols: dict[str, int], output: Path, saved: dict[str, int | str],
    evidence: dict[str, object],
) -> dict[str, object]:
    """Inspect loaded flash before Continue, then verify after real Continue."""
    for elapsed in range(0, 12000, 30):
        status = client.u16(symbols["gSaveFileStatus"])
        if status == 1:
            evidence["save_status_loaded_after_frames"] = elapsed
            break
        if status == 2:
            evidence["corrupt_status_after_frames"] = elapsed
            evidence["save_block1_on_corrupt_load"] = save_block1_prefix(client, symbols)
            evidence["world_on_corrupt_load"] = read_world_record(client, symbols)
            client.step(600)
            client.screenshot(output / "reload-corrupt-title.png")
            raise RuntimeError("Reloaded ROM reported SAVE_STATUS_CORRUPT (2)")
        client.step(30)
    else:
        raise RuntimeError(f"Reloaded ROM did not find valid flash save (status={status})")
    title_record = read_world_record(client, symbols)
    evidence["title_before_continue"] = title_record
    evidence["save_block1_before_continue"] = save_block1_prefix(client, symbols)
    if title_record["raw_hex"] != saved["raw_hex"]:
        raise RuntimeError(f"Saved world differs before Continue: saved={saved}, loaded={title_record}")
    client.screenshot(output / "reload-title-with-save.png")
    for index in range(50):
        client.press("A", hold=2, release=2)
        client.step(56)
        location = read_location(client, symbols)
        player = read_player_object(client, symbols)
        callback2 = client.u32(symbols["gMain"] + 4) & ~1
        if (location and location[:2] == (12, 0) and player is not None
            and callback2 == (symbols["CB2_Overworld"] & ~1)):
            wait_field_unlocked(client, symbols)
            evidence["continued_location"] = read_location(client, symbols)
            evidence["continue_frames"] = (index + 1) * 60
            evidence["overworld_callback2"] = callback2
            break
    else:
        raise RuntimeError(f"Reloaded ROM did not Continue into Center field: {location}, callback2={callback2:#x}")
    continued = read_world_record(client, symbols)
    evidence["after_continue"] = continued
    evidence["save_block1_after_continue"] = save_block1_prefix(client, symbols)
    client.screenshot(output / "reload-center-after-continue.png")
    if continued["raw_hex"] != saved["raw_hex"]:
        raise RuntimeError(f"Saved world changed across Continue: saved={saved}, continued={continued}")
    return evidence


def gba_soft_reset(client: SkyEmu, symbols: dict[str, int]) -> dict[str, int]:
    """Restart the game through its controller chord, retaining emulated flash."""
    counter_address = symbols["gMain"] + 0x20
    before = client.u32(counter_address)
    buttons = ("A", "B", "Select", "Start")
    for button in buttons:
        client.check("/input", {button: "1"})
    try:
        client.step(2)
    finally:
        for button in buttons:
            client.check("/input", {button: "0"})
    for elapsed in range(2, 122, 2):
        after = client.u32(counter_address)
        if after < before:
            return {"vblank_before": before, "vblank_after": after,
                    "reset_observed_after_frames": elapsed}
        client.step(2)
    raise RuntimeError(f"GBA soft reset did not restart main loop (vblank={before}->{after})")


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
    # Let the one-line message finish printing before capturing it. A single
    # B while text is still printing only fast-forwards the text.
    client.step(120)
    client.screenshot(output / "walker-interaction.png")
    client.press("B", hold=2, release=20)
    client.press("B", hold=2, release=20)
    client.step(30)
    client.screenshot(output / "walker-interaction-dismissed.png")
    return {"attempted": True, "direction": direction, "frame_cost": 240,
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
    elapsed += move_player_to(client, symbols, 28 + MAP_OFFSET, 37 + MAP_OFFSET)
    client.screenshot(output / "player-clear-of-center.png")
    blocked = False
    intercept_attempted = False
    completed: list[int] = []
    transitions: list[dict[str, object]] = []
    reentries: list[dict[str, object]] = []
    hidden_exit_goal: int | None = None

    while elapsed < max_frames:
        # These are deliberate emulated-frame advances, rather than repeated
        # wall-clock status polling. The read occurs after each frame batch.
        advance = min(30, max_frames - elapsed)
        client.step(advance)
        elapsed += advance
        current = walker_snapshot(client, symbols)
        if current["phase"] > 4 or current["goal"] >= len(GOALS):
            raise RuntimeError(f"Invalid walker telemetry after {elapsed} frames: {current}")
        if last["phase"] != 4 and current["phase"] == 4:
            hidden_goal = current["last_completed_goal"]
            hidden_exit_goal = hidden_goal if hidden_goal >= 3 else None
        if last["phase"] == 4 and current["phase"] != 4 and hidden_exit_goal is not None:
            reentry = {
                "frame": elapsed,
                "from_exit": GOALS[hidden_exit_goal],
                "hidden_position": [last["x"], last["y"]],
                "entry_position": [current["x"], current["y"]],
                "entry_goal": GOALS[current["goal"]],
                "entry_phase": current["phase"],
                "first_walk_position": None,
            }
            reentries.append(reentry)
            client.screenshot(output / f"reentry-{len(reentries):02d}.png")
            print(f"frame {elapsed}: travel re-entry {reentry}", flush=True)
            hidden_exit_goal = None
        if reentries and reentries[-1]["first_walk_position"] is None and current["phase"] == 2:
            reentries[-1]["first_walk_position"] = [current["x"], current["y"]]
            reentries[-1]["first_walk_frame"] = elapsed
        if current["completed"] > last["completed"]:
            goal = current["last_completed_goal"]
            if goal >= len(GOALS):
                raise RuntimeError(f"Invalid completed goal after {elapsed} frames: {current}")
            completed.append(goal)
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
                elapsed += move_player_to(client, symbols, 30 + MAP_OFFSET, 37 + MAP_OFFSET)
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
                elapsed += move_player_to(client, symbols, 28 + MAP_OFFSET, 37 + MAP_OFFSET)
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


def check_lifecycle(client: SkyEmu, symbols: dict[str, int], output: Path) -> dict[str, object]:
    """Use real controls to leave Viridian, return, and open/close Start."""
    evidence: dict[str, object] = {}
    move_player_to(client, symbols, 30 + MAP_OFFSET, 37 + MAP_OFFSET)
    for _ in range(10):
        client.press("Up", hold=3, release=25)
        client.step(30)
        if (location := read_location(client, symbols)) and location[:2] == (12, 0):
            evidence["center_location"] = location
            client.screenshot(output / "lifecycle-center-inside.png")
            break
    else:
        raise RuntimeError(f"Player did not enter Center through its real door: {location}")

    # The indoor exit at map (7,8) returns to the Viridian Center warp.
    client.step(90)
    for _ in range(10):
        client.press("Down", hold=3, release=25)
        client.step(30)
        if (location := read_location(client, symbols)) and location[:2] == (0, 32):
            evidence["returned_location"] = location
            client.screenshot(output / "lifecycle-center-return.png")
            break
    else:
        raise RuntimeError(f"Player did not leave Center through its real exit: {location}")

    # Map loading may reset walker telemetry; take the baseline after return.
    client.step(90)
    before = walker_snapshot(client, symbols)
    evidence["after_return_before"] = before
    for _ in range(30):
        client.step(30)
        after = walker_snapshot(client, symbols)
        if after["phase"] and (after["x"], after["y"]) != (before["x"], before["y"]):
            evidence["after_return_progress"] = after
            client.screenshot(output / "lifecycle-walker-after-return.png")
            break
    else:
        raise RuntimeError(f"Walker did not move after Center return: {after}")

    client.press("Start", hold=3, release=30)
    client.screenshot(output / "lifecycle-start-menu.png")
    actions = list(client.read(symbols["sCurrentStartMenuActions"], 9))
    if 2 not in actions:
        raise RuntimeError(f"Start menu has no Bag action: {actions}")
    bag_index = actions.index(2)
    evidence["start_menu_actions"] = actions
    evidence["bag_action_index"] = bag_index
    for _ in range(12):
        cursor = client.read(symbols["sStartMenuCursorPos"], 1)[0]
        if cursor == bag_index:
            break
        client.press("Down", hold=3, release=19)
    else:
        raise RuntimeError(f"Could not select Bag action at index {bag_index}; cursor={cursor}")
    client.press("A", hold=3, release=120)
    client.screenshot(output / "lifecycle-bag.png")
    client.press("B", hold=3, release=60)
    client.screenshot(output / "lifecycle-bag-return.png")
    # Bag returns to an *open* Start menu after a field fade. A B sent during
    # that fade is ignored. Repeat with frame advances until field controls
    # actually unlock, then capture the closed-menu evidence.
    for attempt in range(8):
        if not field_controls_locked(client, symbols):
            break
        client.press("B", hold=3, release=30)
        client.step(30)
    else:
        client.screenshot(output / "lifecycle-menu-stuck.png")
        raise RuntimeError("Start menu kept field controls locked after Bag return")
    evidence["menu_exit_b_presses"] = attempt
    client.screenshot(output / "lifecycle-menu-closed.png")
    before = walker_snapshot(client, symbols)
    evidence["after_menu_before"] = before
    for _ in range(30):
        client.step(30)
        after = walker_snapshot(client, symbols)
        if after["phase"] and (after["x"], after["y"]) != (before["x"], before["y"]):
            evidence["after_menu_progress"] = after
            client.screenshot(output / "lifecycle-walker-after-menu.png")
            break
    else:
        raise RuntimeError(f"Walker did not move after Start menu returned: {after}")
    return evidence


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, required=True, help="built POC .gba")
    parser.add_argument("--symbols", type=Path, required=True, help="matching objdump .sym")
    parser.add_argument("--skyemu", type=Path, default=DEFAULT_SKYEMU)
    parser.add_argument("--output", type=Path, required=True, help="screenshots and JSON results")
    parser.add_argument("--max-frames", type=int, default=30000)
    parser.add_argument("--min-goals", type=int, default=3,
                        help="minimum distinct completed goals (1–6; use 6 for full tour)")
    parser.add_argument("--lifecycle", action="store_true",
                        help="then enter/leave Center and open/close Bag, checking walker resumes")
    parser.add_argument("--startup-only", action="store_true", help="debug startup before walker is linked")
    parser.add_argument("--phase2", choices=("follow", "linger", "save", "seam", "seam-return"),
                        help="run one fresh phase 2 scenario with real controls")
    parser.add_argument("--seam-follow", choices=("prompt", "delayed", "late"), default="prompt",
                        help="on seam scenario, wait 1, 60, or 160 frames before crossing")
    parser.add_argument("--seam-bounce", action="store_true",
                        help="after prompt crossing, bounce to Viridian and back to test heartbeat guard")
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
    if args.phase2 and (args.startup_only or args.lifecycle):
        raise ValueError("--phase2 runs separately from --startup-only and --lifecycle")
    if args.seam_bounce and args.phase2 != "seam":
        raise ValueError("--seam-bounce requires --phase2 seam")
    args.output.mkdir(parents=True, exist_ok=True)
    symbols = read_symbols(args.symbols)
    required = {"gSaveBlock1Ptr", "gObjectEvents"}
    if not args.startup_only or args.phase2:
        required.add("gViridianWalkerDebug")
    if args.lifecycle or args.phase2:
        required.update(("sCurrentStartMenuActions", "sStartMenuCursorPos", "sLockFieldControls"))
    if args.phase2:
        required.update(("gSaveBlock3Ptr", "gViridianWalkerWorldSaveOffset",
                         "gViridianWalkerWorldSaveSize"))
    if args.phase2 == "save":
        required.update(("gSaveFileStatus", "gSaveCounter", "gMain", "CB2_Overworld"))
    if args.phase2 in ("seam", "seam-return"):
        required.update(("gSprites", "gSpriteCoordOffsetX", "gSpriteCoordOffsetY",
                         "gViridianWalkerWorldDebug"))
    if missing := sorted(required - symbols.keys()):
        raise RuntimeError(f"symbol file lacks {', '.join(missing)}")
    with tempfile.TemporaryDirectory(prefix="viridian-walker-skyemu-") as temp:
        temp_dir = Path(temp)
        rom = temp_dir / "walker.gba"
        shutil.copyfile(args.rom, rom)
        log_path = args.output / "skyemu.log"
        log_path.write_bytes(b"")
        result: dict[str, object] = {"scenario": args.phase2 or "phase1",
                                     "fresh_rom_copy": True}
        try:
            with skyemu_session(args.skyemu, rom, temp_dir / "xdg", log_path) as client:
                frame = boot_new_game(client, symbols, args.output, args.startup_only)
                result.update({"startup_frames": frame, "viridian": read_location(client, symbols)})
                if args.phase2 == "follow":
                    result["phase2"] = phase2_follow(client, symbols, args.output)
                elif args.phase2 == "linger":
                    result["phase2"] = phase2_linger(client, symbols, args.output)
                elif args.phase2 == "save":
                    result["phase2"] = phase2_save_before(client, symbols, args.output)
                    result["reload_method"] = "gba_soft_reset"
                    result["reset"] = gba_soft_reset(client, symbols)
                    result["reload"] = {}
                    phase2_save_after(client, symbols, args.output,
                                      result["phase2"]["after_save"], result["reload"])
                elif args.phase2 == "seam":
                    result["phase2"] = phase2_seam(client, symbols, args.output,
                                                    args.seam_follow, args.seam_bounce)
                elif args.phase2 == "seam-return":
                    result["phase2"] = phase2_seam_return(client, symbols, args.output)
                elif not args.startup_only:
                    result.update(run_walker(client, symbols, args.output, args.max_frames,
                                             args.min_goals))
                    if len(result["distinct_goals"]) < args.min_goals:
                        raise RuntimeError(f"Walker did not complete {args.min_goals} distinct goals")
                    if not result["real_player_block"]["observed"]:
                        raise RuntimeError("Real-player block and replan were not observed")
                    if args.min_goals == 6 and not result["travel_reentry_observed"]:
                        raise RuntimeError("No travel exit re-entry was observed")
                    if args.lifecycle:
                        result["lifecycle"] = check_lifecycle(client, symbols, args.output)
                (args.output / "results.json").write_text(json.dumps(result, indent=2) + "\n")

            if args.phase2 == "save":
                # The host .sav is diagnostic only. SkyEmu may flush an incomplete
                # snapshot to it; acceptance uses the real GBA soft reset above.
                save_file = rom.with_suffix(".sav")
                if save_file.is_file():
                    result["host_save_bytes"] = save_file.stat().st_size
                    retained_save = args.output / "saved-game.sav"
                    shutil.copyfile(save_file, retained_save)
                    result["host_save_file"] = str(retained_save)
                    result["host_save_sha256"] = hashlib.sha256(retained_save.read_bytes()).hexdigest()
                (args.output / "results.json").write_text(json.dumps(result, indent=2) + "\n")
            return 0
        except Exception as error:
            result["error"] = str(error)
            (args.output / "results.json").write_text(json.dumps(result, indent=2) + "\n")
            raise


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as exc:
        print(f"verification failed: {exc}", file=sys.stderr)
        sys.exit(1)

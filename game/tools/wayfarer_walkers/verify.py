#!/usr/bin/env python3
"""Drive the E2E Wayfarer ROM in headless SkyEmu and check the notable walkers.

Ported from the travel proof of concept's verifier
(task/viridian-walker-poc:game/tools/viridian_walker_poc/verify.py) and
generalised to the stage 3 local actor (src/wayfarer_walkers.c).

Each scenario starts a fresh game through the E2E arrange mailbox, writes the
world records it needs into the saved WayfarerWorldState in RAM, triggers map
loads (E2E warps or real controller input), and proves the behaviour with
symbol reads and screenshots:

  spot    a trainer walks to a spot and plays its template
  edge    following across a map edge: one actor, converted crossing, no gap
  door    following through a door into an interior
  linger  off-screen trainers hop once per heartbeat; on-map trainers don't
  save    save, reset, Continue: identical records, actors at saved tiles
  perf    heartbeat cost over 60 warps, all 25 trainers simulated (gated by ceilings)
  seam    heartbeat cost per frame across a seamless edge (Viridian <-> Route 2)
  recross running back and forth over that seam while a heartbeat is pending
  longtrip a watched trainer whose stay ends plans a long trip (Will at Indigo)
  striplane a strip actor blocked straight ahead leaves the seam (Route 134 -> 133)
  beatspot a trainer at a water's edge runs a water beat, and only beats whose context holds
  notice  notice_player once per approach (Blue), stare_down for a stoic trainer (Lance)
  greet   two friends side by side greet each other once (Brock and Misty)
  beatpush pressing into a walker stops its beat at once and restores its facing
  determinism the same inputs give the same beat log in two runs from boot
  companion a trainer at Viridian's pond brings their ace out beside them for ace_play, then puts it away
  companionslots with exactly the walker rule's 3 object slots free no companion ever comes out
  companionfollower with the following Pokemon out and no slot to spare for it, the companion stays away
  companionpush pressing into the walker puts its companion away at once and interrupts the beat

The verifier refuses a ROM whose content hash differs from the worktree's
generated tables.h: node and spot ids would not match.

Usage:
  python3 game/tools/wayfarer_walkers/verify.py --rom ROM --symbols SYM \\
      --output .product/research/overworld-walkers [--scenario all]
"""

from __future__ import annotations

import argparse
import contextlib
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
MAX_READ_BYTES = 448
MAP_OFFSET = 7
OBJECT_SIZE = 0x24

# Maps, nodes and spots are looked up in include/constants/map_groups.h and
# the generated src/data/wayfarer_world/tables.h when the verifier starts
# (load_world_ids), so they follow the walker graph if it changes.
VIRIDIAN = ROUTE1 = ROUTE2 = CENTER = MART = None
NODE_VIRIDIAN = NODE_ROUTE1 = NODE_ROUTE2_SOUTH = NODE_CENTER = NODE_MART = None
SPOT_VIRIDIAN_WATER = SPOT_VIRIDIAN_MART_SQUARE = SPOT_ROUTE2_GRASS = SPOT_FOREST_GRASS = SPOT_CENTER_COUNTER = SPOT_MART_SHELF = None


class WorldTables:
    """The generated walker graph and spot tables, parsed from tables.h."""

    def __init__(self, game_root: Path):
        import re
        text = (game_root / "src/data/wayfarer_world/tables.h").read_text()
        groups = (game_root / "include/constants/map_groups.h").read_text()
        self.maps = {m[0]: (int(m[2]) << 8) | int(m[1])
                     for m in re.findall(r"(MAP_\w+)\s*=\s*\((\d+) \| \((\d+) << 8\)\)", groups)}

        def block(name):
            start = text.index(name + "[] = {")
            return text[start:text.index("\n};", start)]
        self.nodes = [m for m in re.findall(r"/\*\s*\d+ \*/ \{(MAP_\w+),", block("gWayfarerWorldNodes"))]
        # {map, firstEdge, x, y, edgeCount, flags}
        self.node_edges = [(int(f), int(c)) for f, c in re.findall(
            r"/\*\s*\d+ \*/ \{MAP_\w+, (\d+), \d+, \d+, (\d+),", block("gWayfarerWorldNodes"))]
        self.content_hash = int(re.search(r"gWayfarerWorldContentHash = (0x[0-9A-Fa-f]+);", text).group(1), 16)
        self.spots = [(int(n), kind, int(x), int(y)) for n, kind, x, y in re.findall(
            r"/\*\s*\d+ \*/ \{(\d+), \w+, \d+, (WORLD_SPOT_\w+), (\d+), (\d+),", block("gWayfarerWorldSpots"))]
        self.edges = [(int(t), kind, int(a), int(b), int(c)) for t, kind, a, b, c in re.findall(
            r"/\*\s*\d+ \*/ \{(\d+), (WORLD_EDGE_\w+), (\d+), (\d+), (\d+),", block("gWayfarerWorldEdges"))]

    def map_id(self, name: str) -> int:
        return self.maps[name]

    def edge_source(self, edge: int) -> int:
        """The node whose edge range holds this edge."""
        for i, (first, count) in enumerate(self.node_edges):
            if first <= edge < first + count:
                return i
        raise KeyError(edge)

    def spot(self, map_name: str, kind: str, x: int, y: int) -> tuple[int, int]:
        """(spot id, node) of the spot of this kind on this map's tile."""
        for i, (node, k, sx, sy) in enumerate(self.spots):
            if k == "WORLD_SPOT_" + kind and (sx, sy) == (x, y) and self.nodes[node] == map_name:
                return i, node
        raise KeyError(f"No {kind} spot at {(x, y)} on {map_name}")

    def main_node(self, map_name: str) -> int:
        """The node of this map holding the most spots (the lowest on a tie)."""
        counts = {}
        for node, *_ in self.spots:
            if self.nodes[node] == map_name:
                counts[node] = counts.get(node, 0) + 1
        if not counts:
            return self.nodes.index(map_name)
        return max(sorted(counts), key=lambda n: counts[n])


def load_world_ids(game_root: Path) -> WorldTables:
    tables = WorldTables(game_root)
    ids = {
        "VIRIDIAN": tables.map_id("MAP_VIRIDIAN_CITY_HNS"),
        "ROUTE1": tables.map_id("MAP_ROUTE1_HNS"),
        "ROUTE2": tables.map_id("MAP_ROUTE2_HNS"),
        "CENTER": tables.map_id("MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS"),
        "MART": tables.map_id("MAP_VIRIDIAN_CITY_MART_HNS"),
    }
    # water's edge (13, 39) facing north: Stand and face, "!"
    ids["SPOT_VIRIDIAN_WATER"], ids["NODE_VIRIDIAN"] = tables.spot("MAP_VIRIDIAN_CITY_HNS", "WATER_EDGE", 13, 39)
    ids["SPOT_VIRIDIAN_WATER_2"], _ = tables.spot("MAP_VIRIDIAN_CITY_HNS", "WATER_EDGE", 14, 39)
    ids["SPOT_VIRIDIAN_SQUARE"], _ = tables.spot("MAP_VIRIDIAN_CITY_HNS", "SQUARE", 18, 30)   # Wander in area
    ids["SPOT_VIRIDIAN_MART_SQUARE"], _ = tables.spot("MAP_VIRIDIAN_CITY_HNS", "SQUARE", 39, 30)  # 4 tiles from the Mart door
    ids["SPOT_ROUTE2_GRASS"], ids["NODE_ROUTE2_SOUTH"] = tables.spot("MAP_ROUTE2_HNS", "TALL_GRASS", 9, 58)
    ids["SPOT_FOREST_GRASS"], _ = tables.spot("MAP_VIRIDIAN_FOREST_HNS", "TALL_GRASS", 34, 15)  # past Route 2 south
    ids["SPOT_CENTER_COUNTER"], ids["NODE_CENTER"] = tables.spot("MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS", "CENTER_COUNTER", 6, 4)
    ids["SPOT_MART_SHELF"], ids["NODE_MART"] = tables.spot("MAP_VIRIDIAN_CITY_MART_HNS", "STORE", 7, 2)  # Browse
    ids["NODE_ROUTE1"] = tables.main_node("MAP_ROUTE1_HNS")
    globals().update(ids)
    globals()["TABLES"] = tables
    return tables


SLOT_BLUE = 8
SLOT_LORELEI = 9

STATE_TRAVELLING, STATE_DWELLING = 0, 1
DEST_SPOT = 1
ARRIVAL_NONE, ARRIVAL_DOOR = 0, 5
ACTIVITY_SHOP, ACTIVITY_TRAIN, ACTIVITY_FISH, ACTIVITY_CARE = 1, 3, 5, 0
DIR_SOUTH, DIR_NORTH, DIR_WEST, DIR_EAST = 1, 2, 3, 4

# E2E mailbox (include/e2e_test.h, ABI v25).
CMD_ARRANGE, CMD_SAVE, CMD_WARP = 1, 3, 8
STATUS_PENDING, STATUS_SUCCESS, STATUS_ERROR = 1, 3, 4
CHECKPOINT_NEW_BARK_AFTER_INTRO = 2
KEEP_MAP = 0xFFFF
KEEP_COORD = -32768

RECORD_FIELDS = (
    # (name, word, shift, bits) -- struct WayfarerWorldRecord
    ("node", 0, 0, 12), ("destKind", 0, 12, 2), ("destId", 0, 14, 14), ("state", 0, 28, 3),
    ("waited", 0, 31, 1), ("arrival", 1, 0, 3), ("crossing", 1, 3, 8), ("step", 1, 11, 2),
    ("activity", 1, 13, 4), ("dwell", 1, 17, 6), ("lifeEvent", 1, 23, 2), ("lifeSteps", 1, 25, 2),
    ("stayBits", 1, 27, 2), ("reserved", 1, 29, 3),
)
WALKER_DEBUG_FIELDS = (
    "spawns", "removals", "edgeExits", "warpExits", "arrivals", "rebases", "stripEntries",
    "replans", "blockedSteps", "searches", "searchFails", "localDwellBeats", "localAdvances",
    "emotes", "templateMoves", "floorChanges", "heapResets", "restores", "lastSearchNodes",
    "maxSearchNodes", "lastSearchFrames", "maxSearchFrames", "maxSliceScanlines",
    "maxSliceNodes", "workspaceBytes", "currentMap",
)
ACTOR_COUNT = 4
# struct WayfarerWorldDebug past its first 8 bytes and the 28-byte trace:
# the sliced heartbeat's fields (ROMs before it don't have them).
WORLD_DEBUG_SLICED_FIELDS = (
    "lastHeartbeatFrames", "maxHeartbeatFrames", "maxStepScanlines", "maxFrameScanlines",
    "maxFinishScanlines", "pendingAtLoad", "forcedFinishes", "workspaceLosses", "pending",
    "workspaceWaits", "deferredBegins",
)
WORLD_DEBUG_SLICED_OFFSET = 36
SCANLINES_PER_FRAME = 228
# Regression ceilings (critic loop cycle 1, T3): a scenario fails past them.
# The design target, half a frame of heartbeat plus walker work, is reported
# beside them (within_target) but not gated: a single spot choice (up to
# about 120 lines) landing on a busy frame can exceed it.
CEILING_FRAME_SCANLINES = 200   # worst frame of simulation (heartbeat steps + walkers, or walkers alone)
CEILING_WARP_FRAMES = 44        # frames per warp until the controls unlock (main: 40)
TARGET_FRAME_SCANLINES = SCANLINES_PER_FRAME // 2
# A seam frame with the queue full finishes one heartbeat at once: a typical
# seam heartbeat is about 350 lines and the measured worst about 660.
CEILING_SEAM_FINISH_SCANLINES = 1000
FLAG_RUNNING_SHOES = 0x895      # e2e catalog: runningShoes


class SkyEmu:
    def __init__(self, port: int):
        self.url = f"http://127.0.0.1:{port}"
        self.frames = 0

    def request(self, endpoint: str, params=None) -> bytes:
        if isinstance(params, list):
            suffix = "?" + urllib.parse.urlencode(params)
        else:
            suffix = "?" + urllib.parse.urlencode(params) if params else ""
        with urllib.request.urlopen(self.url + endpoint + suffix, timeout=60) as response:
            return response.read().rstrip(b"\0")

    def check(self, endpoint: str, params=None) -> None:
        result = self.request(endpoint, params)
        if result != b"ok":
            raise RuntimeError(f"SkyEmu {endpoint} returned {result!r}")

    def read(self, address: int, size: int) -> bytes:
        out = bytearray()
        for start in range(0, size, MAX_READ_BYTES):
            count = min(MAX_READ_BYTES, size - start)
            hexes = self.request("/read_byte", [("addr", f"{address + start + i:08x}") for i in range(count)])
            out.extend(bytes.fromhex(hexes.decode("ascii")))
        return bytes(out)

    def write(self, address: int, data: bytes) -> None:
        for start in range(0, len(data), 200):
            chunk = data[start:start + 200]
            self.check("/write_byte", [(f"{address + start + i:08x}", f"{b:02x}") for i, b in enumerate(chunk)])

    def u16(self, address: int) -> int:
        return struct.unpack("<H", self.read(address, 2))[0]

    def u32(self, address: int) -> int:
        return struct.unpack("<I", self.read(address, 4))[0]

    def step(self, frames: int) -> None:
        self.check("/step", {"frames": str(frames)})
        self.frames += frames

    def hold(self, button: str, value: int) -> None:
        self.check("/input", {button: str(value)})

    def press(self, button: str, hold: int = 2, release: int = 4) -> None:
        self.hold(button, 1)
        self.step(hold)
        self.hold(button, 0)
        self.step(release)

    def screenshot(self, path: Path) -> None:
        data = self.request("/screen")
        if not data.startswith(b"\x89PNG"):
            raise RuntimeError("SkyEmu /screen did not return a PNG")
        path.write_bytes(data)


def read_symbols(path: Path) -> dict[str, int]:
    symbols: dict[str, int] = {}
    for line in path.read_text().splitlines():
        fields = line.split(maxsplit=3)
        if len(fields) != 4 or len(fields[0]) != 8:
            continue
        try:
            value = int(fields[0], 16)
        except ValueError:
            continue
        if fields[1] == "g" or fields[3] not in symbols:
            symbols[fields[3]] = value
    return symbols


def reserve_port() -> int:
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


@contextlib.contextmanager
def skyemu_session(binary: Path, rom: Path, data_home: Path, log_path: Path):
    port = reserve_port()
    with log_path.open("ab") as log:
        process = subprocess.Popen(
            ["xvfb-run", "--auto-servernum", str(binary), "http_server", str(port), str(rom)],
            env={**os.environ, "XDG_DATA_HOME": str(data_home)},
            stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            start_new_session=True,
        )
        ready = threading.Event()

        def pump() -> None:
            assert process.stdout is not None
            for line in process.stdout:
                log.write(line)
                log.flush()
                if f"Starting HCS: http://localhost:{port}".encode() in line:
                    ready.set()

        thread = threading.Thread(target=pump, daemon=True)
        thread.start()
        client = SkyEmu(port)
        try:
            if not ready.wait(timeout=60):
                raise RuntimeError(f"SkyEmu did not start (exit={process.poll()})")
            client.check("/load_rom", {"path": str(rom), "pause": "1"})
            for button in ("A", "B", "Up", "Down", "Left", "Right", "L", "R", "Start", "Select"):
                client.hold(button, 0)
            yield client
        finally:
            # SkyEmu can outlive xvfb-run on SIGTERM: always kill the whole group.
            for sig in (signal.SIGTERM, signal.SIGKILL):
                with contextlib.suppress(ProcessLookupError):
                    os.killpg(process.pid, sig)
                with contextlib.suppress(subprocess.TimeoutExpired):
                    process.wait(timeout=5)
            thread.join(timeout=2)


class Game:
    def __init__(self, emu: SkyEmu, symbols: dict[str, int], output: Path):
        self.emu = emu
        self.sym = symbols
        self.output = output
        self.request_id = 0
        abi = emu.read(symbols["gE2ETestAbi"], 16)
        self.abi = dict(zip(("version", "requestSize", "resultSize", "stateSize", "requestStatusOffset",
                             "resultStatusOffset", "flagsOffset", "varsOffset"), struct.unpack("<8H", abi)))
        if self.abi["requestSize"] != 380 or self.abi["requestStatusOffset"] != 87:
            raise RuntimeError(f"Unexpected E2E request layout: {self.abi}")

    # Mailbox -----------------------------------------------------------------

    def command(self, command: int, map_id: int = KEEP_MAP, x: int = KEEP_COORD, y: int = KEEP_COORD,
                facing: int = DIR_SOUTH, max_frames: int = 3600) -> None:
        self.request_id += 1
        req = bytearray(self.abi["requestSize"])
        struct.pack_into("<I", req, 0, self.request_id)
        if map_id == KEEP_MAP:
            struct.pack_into("<HH", req, 4, KEEP_MAP, KEEP_MAP)
        else:
            struct.pack_into("<HH", req, 4, map_id >> 8, map_id & 0xFF)
        struct.pack_into("<hhI", req, 8, x, y, 1)
        req[80] = CHECKPOINT_NEW_BARK_AFTER_INTRO if command in (CMD_ARRANGE, CMD_WARP) else 0
        req[81] = facing
        req[84] = 0xFF  # keep text speed
        req[85] = 1 if command == CMD_ARRANGE else 0
        req[86] = command
        address = self.sym["gE2ETestRequest"]
        self.emu.write(address, bytes(req))
        self.emu.write(address + 87, bytes([STATUS_PENDING]))
        for _ in range(0, max_frames, 2):
            self.emu.step(2)
            raw = self.emu.read(self.sym["gE2ETestResult"], 16)
            request_id = struct.unpack_from("<I", raw, 0)[0]
            status, error = raw[14], struct.unpack_from("<H", raw, 12)[0]
            if request_id != self.request_id or status not in (STATUS_SUCCESS, STATUS_ERROR):
                continue
            if status == STATUS_ERROR:
                raise RuntimeError(f"E2E command {command} failed: error {error}, phase {raw[15]}")
            return
        raise RuntimeError(f"E2E command {command} timed out")

    def arrange(self, map_id: int, x: int, y: int, facing: int = DIR_SOUTH) -> None:
        self.command(CMD_ARRANGE, map_id, x, y, facing)
        self.settle()
        self.finish_heartbeat()

    def warp(self, map_id: int, x: int, y: int, facing: int = DIR_SOUTH) -> None:
        self.command(CMD_WARP, map_id, x, y, facing)
        self.settle()
        self.finish_heartbeat()

    def save(self) -> None:
        self.command(CMD_SAVE)

    def controls_locked(self) -> bool:
        return bool(self.emu.read(self.sym["sLockFieldControls"], 1)[0])

    def settle(self, limit: int = 600) -> None:
        for _ in range(0, limit, 2):
            if not self.controls_locked():
                return
            self.emu.step(2)
        raise RuntimeError("Field controls stayed locked")

    # World state -------------------------------------------------------------

    def walker_debug(self) -> dict:
        raw = self.emu.read(self.sym["gWayfarerWalkersDebug"], 64 + 16 * ACTOR_COUNT + 40)
        out = {"frames": struct.unpack_from("<I", raw, 0)[0]}
        values = struct.unpack_from("<26H", raw, 4)
        out.update(zip(WALKER_DEBUG_FIELDS, values))
        out["followerHidden"], out["activeActors"] = raw[56], raw[57]
        out["storySuppressed"] = struct.unpack_from("<H", raw, 58)[0]
        out["worldState"] = struct.unpack_from("<I", raw, 60)[0]
        actors = []
        for i in range(ACTOR_COUNT):
            a = raw[64 + 16 * i: 80 + 16 * i]
            slot, mode, phase, goal, obj, template, at_spot, blocked = a[:8]
            x, y = struct.unpack_from("<hh", a, 8)
            actors.append({"index": i, "slot": slot, "mode": mode, "phase": phase, "goalKind": goal,
                           "objectId": obj, "template": template, "atSpot": at_spot, "blocked": blocked,
                           "x": x, "y": y, "goalX": a[12], "goalY": a[13],
                           "goalEdge": struct.unpack_from("<H", a, 14)[0]})
        out["actors"] = actors
        tail = 64 + 16 * ACTOR_COUNT
        (out["lastHeartbeatScanlines"], out["maxHeartbeatScanlines"], out["maxSpawnScanlines"],
         out["maxUpdateScanlines"]) = struct.unpack_from("<4I", raw, tail)
        (out["backOffs"], out["handoffs"], out["visitorsVanished"], out["culls"],
         out["lastContextScanlines"], out["lastFailNodes"], out["maxFinishScanlines"],
         out["walkOffs"]) = struct.unpack_from("<8H", raw, tail + 16)
        (out["worldJobs"], out["maxJobScanlines"], out["capacityWaits"],
         out["stripTimeouts"]) = struct.unpack_from("<4H", raw, tail + 32)
        return out

    def stable(self, read):
        """Repeat a group of reads until no field frame ran in between."""
        address = self.sym["gWayfarerWalkersDebug"]
        for _ in range(20):
            before = self.emu.u32(address)
            value = read()
            if self.emu.u32(address) == before:
                return value
        raise RuntimeError("Could not take a consistent snapshot")

    @property
    def sliced(self) -> bool:
        """The ROM spreads the heartbeat over frames (round 4 and later)."""
        return "sHeartbeat" in self.sym

    def world_debug(self) -> dict:
        size = WORLD_DEBUG_SLICED_OFFSET + 2 * len(WORLD_DEBUG_SLICED_FIELDS) if self.sliced else 8
        raw = self.emu.read(self.sym["gWayfarerWorldDebug"], size)
        out = dict(zip(("heartbeats", "skippedHeartbeats", "lastHeartbeatMap", "reseats"),
                       struct.unpack_from("<4H", raw, 0)))
        if self.sliced:
            out.update(zip(WORLD_DEBUG_SLICED_FIELDS,
                           struct.unpack_from(f"<{len(WORLD_DEBUG_SLICED_FIELDS)}H", raw, WORLD_DEBUG_SLICED_OFFSET)))
        return out

    def reset_frame_maxima(self) -> None:
        """Zero the per-frame worst cases so a scenario measures only itself."""
        tail = 64 + 16 * ACTOR_COUNT
        self.emu.write(self.sym["gWayfarerWalkersDebug"] + tail + 12, bytes(4))  # maxUpdateScanlines
        self.emu.write(self.sym["gWayfarerWalkersDebug"] + tail + 34, bytes(2))  # maxJobScanlines
        if self.sliced:
            # maxHeartbeatFrames, maxStepScanlines, maxFrameScanlines, maxFinishScanlines
            self.emu.write(self.sym["gWayfarerWorldDebug"] + WORLD_DEBUG_SLICED_OFFSET + 2, bytes(8))

    def finish_heartbeat(self, limit: int = 600) -> None:
        """On a sliced ROM the map load's heartbeat runs on for a few frames
        after the controls unlock: wait until the records are settled."""
        if self.sliced:
            self.wait_for(lambda: not self.world_debug()["pending"], limit, step=1, what="the heartbeat to finish")

    def wait_heartbeat(self, before: int, limit: int = 600) -> None:
        """Until the heartbeat after `before` has finished (it may take a few frames)."""
        self.wait_for(lambda: self.world_debug()["heartbeats"] > before, limit, step=1, what="the heartbeat")

    def state_address(self) -> int:
        address = self.walker_debug()["worldState"]
        if not 0x02000000 <= address < 0x02040000:
            raise RuntimeError(f"World state address not published yet: {address:#x}")
        return address

    def world_bytes(self) -> bytes:
        return self.emu.read(self.state_address(), 216)

    def record(self, slot: int) -> dict:
        raw = self.emu.read(self.state_address() + 4 + slot * 8, 8)
        words = struct.unpack("<II", raw)
        rec = {name: (words[w] >> shift) & ((1 << bits) - 1) for name, w, shift, bits in RECORD_FIELDS}
        rec["hex"] = raw.hex()
        return rec

    def write_record(self, slot: int, **fields) -> None:
        rec = self.record(slot)
        rec.update(fields)
        words = [0, 0]
        for name, w, shift, bits in RECORD_FIELDS:
            words[w] |= (rec[name] & ((1 << bits) - 1)) << shift
        self.emu.write(self.state_address() + 4 + slot * 8, struct.pack("<II", *words))

    def local_actor_block(self) -> list:
        raw = self.emu.read(self.state_address() + 204, 9)
        out = []
        for i in range(3):
            e = raw[i * 3:i * 3 + 3]
            if e[0] & 0x3F == 63:
                out.append(None)
            else:
                out.append({"slot": e[0] & 0x3F, "facing": (e[0] >> 6) + 1, "x": e[1], "y": e[2]})
        return out

    def objects(self) -> list:
        raw = self.emu.read(self.sym["gObjectEvents"], 16 * OBJECT_SIZE)
        out = []
        for i in range(16):
            o = raw[i * OBJECT_SIZE:(i + 1) * OBJECT_SIZE]
            if not o[0] & 1:
                continue
            x, y = struct.unpack_from("<hh", o, 0x10)
            out.append({"slot": i, "gfx": struct.unpack_from("<H", o, 4)[0], "localId": o[8],
                        "mapNum": o[9], "mapGroup": o[10], "x": x - MAP_OFFSET, "y": y - MAP_OFFSET,
                        "invisible": bool(o[1] & 0x20), "isPlayer": bool(o[2] & 1),
                        "facing": struct.unpack_from("<H", o, 0x18)[0] & 0xF})
        return out

    def player(self) -> dict:
        for o in self.objects():
            if o["isPlayer"]:
                return o
        raise RuntimeError("No player object")

    def current_map(self) -> int:
        return self.walker_debug()["currentMap"]

    def actor_for(self, slot: int):
        for a in self.walker_debug()["actors"]:
            if a["slot"] == slot and a["mode"] != 0:
                return a
        return None

    def wait_for(self, predicate, limit: int, step: int = 4, what: str = "condition"):
        for _ in range(0, limit, step):
            value = predicate()
            if value:
                return value
            self.emu.step(step)
        value = predicate()
        if value:
            return value
        raise RuntimeError(f"Timed out waiting for {what}")

    def shot(self, name: str) -> str:
        path = self.output / f"{name}.png"
        self.emu.screenshot(path)
        return str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)

    def walk(self, direction: str, axis: str, target: int, limit: int = 600) -> None:
        """Hold one direction until the player's tile reaches target on axis."""
        self.emu.hold(direction, 1)
        try:
            for _ in range(0, limit, 2):
                self.emu.step(2)
                if self.player()[axis] == target:
                    break
            else:
                raise RuntimeError(f"Player did not reach {axis}={target} holding {direction}: {self.player()}")
        finally:
            self.emu.hold(direction, 0)
        self.emu.step(16)


def boot(game: Game, map_id: int, x: int, y: int, facing: int = DIR_SOUTH) -> None:
    game.emu.step(240)
    game.arrange(map_id, x, y, facing)
    game.emu.step(30)


def place_blue(game: Game, **fields) -> None:
    base = dict(destKind=DEST_SPOT, waited=0, step=0, lifeEvent=0, lifeSteps=0, stayBits=0, reserved=0)
    base.update(fields)
    game.write_record(SLOT_BLUE, **base)


# ---------------------------------------------------------------------------
# Scenarios


def scenario_spot(game: Game) -> dict:
    """Blue leaves the Viridian Center door and walks to a water's edge spot."""
    boot(game, VIRIDIAN, 16, 41, DIR_NORTH)
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_WATER, state=STATE_TRAVELLING,
               arrival=ARRIVAL_DOOR, crossing=4, activity=ACTIVITY_FISH, dwell=3)
    # A warp to the same map is no heartbeat, but it reloads the objects:
    # the actor spawns from the record we just wrote.
    game.warp(VIRIDIAN, 16, 41, DIR_NORTH)
    before = game.walker_debug()
    spawned = game.wait_for(lambda: game.actor_for(SLOT_BLUE), 120, what="Blue's actor")
    spawn_tile = (spawned["x"], spawned["y"])
    # Mid-walk, open the Bag from the Start menu and close it again: the
    # field controls lock (the AI must stay put) and the return to the field
    # resets the engine heap (the walker re-plans from its tile).
    game.emu.step(120)
    resets_before = game.walker_debug()["heapResets"]
    game.emu.press("Start")
    game.emu.step(30)
    menu_actor = game.actor_for(SLOT_BLUE)
    game.emu.press("Down")
    game.emu.press("A")
    game.emu.step(120)
    bag_actor = game.actor_for(SLOT_BLUE)
    game.emu.press("B")
    game.emu.step(120)
    if game.controls_locked():
        game.emu.press("B")
        game.emu.step(30)
    game.settle()
    menu = {"actor_menu_open": menu_actor, "actor_in_bag": bag_actor,
            "heap_resets": game.walker_debug()["heapResets"] - resets_before,
            "screenshot": game.shot("spot-after-bag")}
    arrived = game.wait_for(lambda: game.record(SLOT_BLUE)["state"] == STATE_DWELLING
                            and game.actor_for(SLOT_BLUE)["atSpot"], 2400, what="arrival at the spot")
    actor = game.actor_for(SLOT_BLUE)
    # The arrival's beat (arrive_look) turns him about and ends facing the
    # spot's way again: read the facing once it is over.
    if "sAmbience" in game.sym:
        watcher = BeatWatcher(game)
        game.wait_for(lambda: watcher.running(actor["index"]) is None, 300, step=2, what="the arrival beat's end")
        game.emu.step(4)
    obj = next(o for o in game.objects() if o["slot"] == actor["objectId"])
    screenshot_arrived = game.shot("spot-arrived")
    # Play the template: Stand and face (north, at the water). Its "!" is a
    # beat now (water_bite, or Blue's swagger "!!"), after the quiet gap.
    emotes_before = game.walker_debug()["emotes"]
    game.wait_for(lambda: game.walker_debug()["emotes"] > emotes_before, 1800, step=10, what="an icon")
    game.emu.step(30)
    after = game.walker_debug()
    screenshot_template = game.shot("spot-template")
    record = game.record(SLOT_BLUE)
    result = {
        "spawn_tile": spawn_tile,
        "menu_round_trip": menu,
        "spot_tile": (13, 39),
        "actor_tile": (actor["x"], actor["y"]),
        "facing": obj["facing"],
        "record": record,
        "template": actor["template"],
        "emotes": after["emotes"] - emotes_before,
        "searches": after["searches"] - before["searches"],
        "arrivals": after["arrivals"] - before["arrivals"],
        "screenshots": [screenshot_arrived, screenshot_template],
        "debug": after,
    }
    result["pass"] = (actor["x"], actor["y"]) == (13, 39) and obj["facing"] == DIR_NORTH \
        and record["state"] == STATE_DWELLING and record["destId"] == SPOT_VIRIDIAN_WATER \
        and result["emotes"] >= 1 and result["arrivals"] == 1 \
        and menu["heap_resets"] >= 1 and menu["actor_in_bag"] is not None
    return result


def blue_objects(game: Game, gfx: int) -> list:
    return [o for o in game.objects() if o["gfx"] == gfx]


def scenario_edge(game: Game) -> dict:
    """Blue leaves Viridian's north edge; the player follows onto Route 2."""
    boot(game, VIRIDIAN, 27, 1, DIR_NORTH)
    # Heading past Route 2 (to Viridian Forest), so the record stays
    # travelling on Route 2 and keeps the converted crossing.
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_FOREST_GRASS, state=STATE_TRAVELLING,
               arrival=ARRIVAL_DOOR, crossing=4, activity=ACTIVITY_TRAIN, dwell=3)
    exits_before = game.walker_debug()["edgeExits"]
    game.warp(VIRIDIAN, 27, 1, DIR_NORTH)
    actor = game.wait_for(lambda: game.actor_for(SLOT_BLUE), 120, what="Blue's actor")
    gfx = next(o["gfx"] for o in game.objects() if o["slot"] == actor["objectId"])
    game.wait_for(lambda: game.walker_debug()["edgeExits"] > exits_before, 3000, step=2, what="the edge exit")
    record_at_exit = game.record(SLOT_BLUE)
    exit_actor = game.actor_for(SLOT_BLUE)
    exit_tile = (exit_actor["x"], exit_actor["y"])
    heartbeats_before = game.world_debug()["heartbeats"]
    game.emu.step(4)
    screenshot_strip = game.shot("edge-strip-before-cross")

    # Follow: walk north across the seam, sampling every two frames.
    samples = []
    game.emu.hold("Up", 1)
    try:
        for _ in range(0, 160, 2):
            game.emu.step(2)
            dbg, objs = game.stable(lambda: (game.walker_debug(), game.objects()))
            # Position and map come from the walker debug block, published
            # together once per field frame (object coordinates shift later
            # in the same frame, at the camera update).
            actors = [a for a in dbg["actors"] if a["slot"] == SLOT_BLUE and a["mode"]]
            samples.append({"frame": dbg["frames"], "map": dbg["currentMap"],
                            "blue_objects": sum(1 for o in objs if o["gfx"] == gfx),
                            "blue": [(a["x"], a["y"]) for a in actors],
                            "mode": [a["mode"] for a in actors]})
            if dbg["currentMap"] == ROUTE2 and next(o for o in objs if o["isPlayer"])["y"] <= 77:
                break
    finally:
        game.emu.hold("Up", 0)
    game.emu.step(8)
    screenshot_after = game.shot("edge-route2-after-cross")
    # Blue keeps walking on Route 2 towards the tall grass.
    after_actor = game.actor_for(SLOT_BLUE)
    game.emu.step(90)
    moved_actor = game.actor_for(SLOT_BLUE)
    screenshot_moving = game.shot("edge-route2-moving")
    dbg = game.walker_debug()
    counts = [s["blue_objects"] for s in samples] + [len(s["blue"]) for s in samples]

    def route2_coords(sample):
        # Viridian's north edge is Route 2 `up` at offset 16; Route 2 is 80 tiles tall.
        x, y = sample["blue"][0]
        return (x - 16, y + 80) if sample["map"] == VIRIDIAN else (x, y)

    track = [route2_coords(s) for s in samples if s["blue"]]
    jumps = [max(abs(a[0] - b[0]), abs(a[1] - b[1])) for a, b in zip(track, track[1:])]
    expected_crossing = 6 + (exit_tile[0] - 22)
    result = {
        "exit_tile_viridian": exit_tile,
        "record_at_exit": record_at_exit,
        "expected_crossing": expected_crossing,
        "heartbeats_during_cross": game.world_debug()["heartbeats"] - heartbeats_before,
        "blue_object_counts": sorted(set(counts)),
        "max_tile_jump": max(jumps) if jumps else None,
        "track_route2_coords": track,
        "actor_after_cross": after_actor,
        "actor_after_90_frames": moved_actor,
        "rebases": dbg["rebases"],
        "samples": samples,
        "screenshots": [screenshot_strip, screenshot_after, screenshot_moving],
        "debug": dbg,
    }
    result["pass"] = (record_at_exit["node"] == NODE_ROUTE2_SOUTH and record_at_exit["crossing"] == expected_crossing
                      and set(counts) == {1} and (max(jumps) if jumps else 9) <= 1
                      and after_actor is not None and after_actor["mode"] == 1
                      and moved_actor is not None and (moved_actor["x"], moved_actor["y"]) != (after_actor["x"], after_actor["y"])
                      and samples[-1]["map"] == ROUTE2 and dbg["rebases"] >= 1)
    return result


def scenario_door(game: Game) -> dict:
    """Blue walks into the Viridian Center; the player follows him inside."""
    boot(game, VIRIDIAN, 32, 38, DIR_NORTH)
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_CENTER_COUNTER, state=STATE_TRAVELLING,
               arrival=ARRIVAL_DOOR, crossing=4, activity=ACTIVITY_CARE, dwell=2)
    exits_before = game.walker_debug()["warpExits"]
    game.warp(VIRIDIAN, 32, 38, DIR_NORTH)
    game.wait_for(lambda: game.walker_debug()["warpExits"] > exits_before, 2400, step=2, what="the door exit")
    record_at_exit = game.record(SLOT_BLUE)
    game.wait_for(lambda: game.actor_for(SLOT_BLUE) is None, 120, what="the actor's removal")
    screenshot_outside = game.shot("door-outside-after-entry")
    # Follow through the door with real input: (32, 38) -> (30, 37) -> Up.
    game.walk("Left", "x", 30)
    game.walk("Up", "y", 37)
    game.emu.hold("Up", 1)
    game.emu.step(8)
    game.emu.hold("Up", 0)
    game.wait_for(lambda: game.current_map() == CENTER, 600, what="the Center")
    game.settle()
    inside = game.wait_for(lambda: game.actor_for(SLOT_BLUE), 120, what="Blue inside")
    spawn_tile = (inside["x"], inside["y"])
    game.emu.step(30)
    screenshot_inside = game.shot("door-inside-walking")
    walking = (inside["phase"], game.actor_for(SLOT_BLUE)["phase"])
    game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("atSpot"), 1200, what="arrival at the counter")
    actor = game.actor_for(SLOT_BLUE)
    screenshot_spot = game.shot("door-inside-at-counter")
    result = {
        "record_at_exit": record_at_exit,
        "spawn_tile_inside": spawn_tile,
        "phases_after_spawn": walking,
        "player_inside": (game.player()["x"], game.player()["y"]),
        "actor_at_spot": (actor["x"], actor["y"]),
        "record": game.record(SLOT_BLUE),
        "screenshots": [screenshot_outside, screenshot_inside, screenshot_spot],
        "debug": game.walker_debug(),
    }
    # The record reached its destination node by the door, so it is already
    # Dwelling; the actor still enters by the door and walks to the counter.
    result["pass"] = (record_at_exit["node"] == NODE_CENTER and (actor["x"], actor["y"]) == (6, 4)
                      and spawn_tile != result["player_inside"] and spawn_tile != (6, 4)
                      and abs(spawn_tile[0] - 7) + abs(spawn_tile[1] - 8) <= 3)
    return result


def scenario_linger(game: Game) -> dict:
    """Off-screen trainers hop once per heartbeat; a trainer on the player's map doesn't."""
    boot(game, CENTER, 7, 6, DIR_SOUTH)
    # Lorelei, off-screen: Route 1 -> Viridian -> Route 2 south (two hops).
    game.write_record(SLOT_LORELEI, node=NODE_ROUTE1, destKind=DEST_SPOT, destId=SPOT_ROUTE2_GRASS,
                      state=STATE_TRAVELLING, arrival=ARRIVAL_NONE, crossing=0, activity=ACTIVITY_TRAIN,
                      dwell=3, waited=0, lifeEvent=0, lifeSteps=0, stayBits=0, reserved=0)
    # Blue, dwelling in the Mart with one heartbeat of dwell left.
    place_blue(game, node=NODE_MART, destId=SPOT_MART_SHELF, state=STATE_DWELLING,
               arrival=ARRIVAL_NONE, crossing=0, activity=ACTIVITY_SHOP, dwell=1)
    trace = [{"step": "start", "player_map": game.current_map(), "heartbeats": game.world_debug()["heartbeats"],
              "heartbeat_scanlines": game.walker_debug()["lastHeartbeatScanlines"],
              "lorelei": game.record(SLOT_LORELEI), "blue": game.record(SLOT_BLUE)}]
    for name, (map_id, x, y) in (("into the Mart", (MART, 4, 6)), ("into the Center", (CENTER, 7, 6)),
                                 ("into the Mart again", (MART, 4, 6)), ("into the Center again", (CENTER, 7, 6))):
        before = game.emu.frames
        game.warp(map_id, x, y, DIR_NORTH)
        frames_taken = game.emu.frames - before
        trace.append({"step": name, "player_map": game.current_map(),
                      "heartbeats": game.world_debug()["heartbeats"],
                      "heartbeat_scanlines": game.walker_debug()["lastHeartbeatScanlines"],
                      "warp_frames": frames_taken,
                      "lorelei": game.record(SLOT_LORELEI), "blue": game.record(SLOT_BLUE),
                      "blue_actor": game.actor_for(SLOT_BLUE)})
        if name == "into the Mart":
            trace[-1]["screenshot"] = game.shot("linger-mart-blue-stays")
    nodes = [t["lorelei"]["node"] for t in trace]
    hb = [t["heartbeats"] for t in trace]
    result = {"trace": trace, "lorelei_nodes": nodes, "heartbeats": hb, "debug": game.walker_debug()}
    blue_start, blue_in_mart, blue_left = trace[0]["blue"], trace[1]["blue"], trace[2]["blue"]
    result["pass"] = (
        all(b - a == 1 for a, b in zip(hb, hb[1:]))
        and nodes[:3] == [NODE_ROUTE1, NODE_VIRIDIAN, NODE_ROUTE2_SOUTH]
        and trace[2]["lorelei"]["state"] == STATE_DWELLING
        and trace[3]["lorelei"]["dwell"] == trace[2]["lorelei"]["dwell"] - 1
        # Blue on the player's new map: no dwell spent, no routine advance.
        and blue_in_mart["hex"] == blue_start["hex"]
        # Blue off the player's map again: dwell 1 -> 0, the routine advances.
        and blue_left["destId"] != SPOT_MART_SHELF
    )
    return result


SLOT_BROCK = 0


def arrange_with_badges(game: Game, map_id: int, x: int, y: int, kanto_badges: int,
                        johto_badges: int = 0, hoenn_badges: int = 0, facing: int = DIR_NORTH,
                        flags: tuple = ()) -> None:
    game.request_id += 1
    req = bytearray(game.abi["requestSize"])
    struct.pack_into("<IHHhhI", req, 0, game.request_id, map_id >> 8, map_id & 0xFF, x, y, 1)
    req[80], req[81], req[84], req[85], req[86] = CHECKPOINT_NEW_BARK_AFTER_INTRO, facing, 0xFF, 1, CMD_ARRANGE
    for i, flag in enumerate(flags):     # struct E2ETestFlagPatch flags[8] at offset 48
        struct.pack_into("<HBB", req, 48 + 4 * i, flag, 1, 0)
    req[83] = len(flags)
    req[362], req[363], req[364] = kanto_badges, johto_badges, hoenn_badges  # regional badge counts
    req[368] = 1                # apply the league circuit fixture (badges)
    submit_arrange(game, req, "arrange with badges")


def submit_arrange(game: Game, req: bytearray, what: str) -> None:
    """Send a hand-built arrange request and wait for the field."""
    address = game.sym["gE2ETestRequest"]
    game.emu.write(address, bytes(req))
    game.emu.write(address + 87, bytes([STATUS_PENDING]))
    game.wait_for(lambda: struct.unpack_from("<I", game.emu.read(game.sym["gE2ETestResult"], 4))[0] == game.request_id
                  and game.emu.read(game.sym["gE2ETestResult"] + 14, 1)[0] in (STATUS_SUCCESS, STATUS_ERROR),
                  3600, step=2, what=what)
    if game.emu.read(game.sym["gE2ETestResult"] + 14, 1)[0] != STATUS_SUCCESS:
        raise RuntimeError(f"{what} failed")
    game.settle()
    game.finish_heartbeat()


def gym_objects(game: Game, gfx: int) -> list:
    return [o for o in game.objects() if o["gfx"] == gfx and not o["isPlayer"]]


def scenario_gym(game: Game) -> dict:
    """The Gym Leader's own object and a "just leaving" visitor."""
    PEWTER = TABLES.map_id("MAP_PEWTER_CITY_HNS")
    PEWTER_GYM = TABLES.map_id("MAP_PEWTER_CITY_GYM_HNS")
    NODE_PEWTER = TABLES.main_node("MAP_PEWTER_CITY_HNS")
    SPOT_PEWTER_GYM, NODE_PEWTER_GYM = TABLES.spot("MAP_PEWTER_CITY_GYM_HNS", "GYM", 6, 14)
    game.emu.step(240)
    arrange_with_badges(game, PEWTER, 15, 18, 1)   # Boulder Badge held: Brock is simulated
    game.warp(PEWTER_GYM, 9, 11, DIR_NORTH)
    game.emu.step(30)
    brock_home = game.record(SLOT_BROCK)
    template_gfx = 0
    for o in game.objects():
        if (o["x"], o["y"]) == (6, 5) and not o["isPlayer"]:
            template_gfx = o["gfx"]
    shown_home = len(gym_objects(game, template_gfx)) if template_gfx else 0
    shot_home = game.shot("gym-leader-home")

    # Brock out in Pewter City; Blue visiting the Gym.
    game.write_record(SLOT_BROCK, node=NODE_PEWTER, destKind=DEST_SPOT, destId=SPOT_VIRIDIAN_WATER,
                      state=STATE_TRAVELLING, arrival=ARRIVAL_NONE, crossing=0, activity=ACTIVITY_FISH,
                      dwell=3, waited=0, lifeEvent=0, lifeSteps=0, stayBits=0, reserved=0)
    place_blue(game, node=NODE_PEWTER_GYM, destId=SPOT_PEWTER_GYM, state=STATE_DWELLING,
               arrival=ARRIVAL_NONE, crossing=0, activity=6, dwell=1)
    exits_before = game.walker_debug()["warpExits"]
    game.warp(PEWTER_GYM, 9, 11, DIR_NORTH)
    visitor = game.wait_for(lambda: game.actor_for(SLOT_BLUE), 120, step=1, what="the visitor")
    visitor_tile = (visitor["x"], visitor["y"])
    leader_hidden = len(gym_objects(game, template_gfx)) == 0
    game.emu.step(4)
    shot_visitor = game.shot("gym-visitor-just-leaving")
    game.wait_for(lambda: game.walker_debug()["warpExits"] > exits_before, 900, step=2, what="the visitor leaving")
    blue_after = game.record(SLOT_BLUE)
    game.emu.step(240)
    no_return = game.actor_for(SLOT_BLUE) is None
    # Walking around the Gym must not respawn the hidden leader object.
    game.walk("Up", "y", 8)
    game.walk("Left", "x", 6)
    game.walk("Up", "y", 7)
    still_hidden = len(gym_objects(game, template_gfx)) == 0
    shot_hidden = game.shot("gym-leader-out")
    result = {
        "brock_record_home": brock_home,
        "leader_template_gfx": template_gfx,
        "leader_objects_while_home": shown_home,
        "visitor_spawn_tile": visitor_tile,
        "visitor_first_seen": visitor,
        "leader_hidden_while_out": leader_hidden,
        "leader_hidden_after_walking": still_hidden,
        "blue_after_leaving": blue_after,
        "visitor_gone_and_not_respawned": no_return,
        "screenshots": [shot_home, shot_visitor, shot_hidden],
        "debug": game.walker_debug(),
    }
    result["pass"] = (brock_home["state"] == STATE_DWELLING and brock_home["node"] == NODE_PEWTER_GYM
                      and shown_home == 1 and leader_hidden and still_hidden
                      and (visitor_tile in ((6, 13), (5, 14), (7, 14))
                           # or already stepping out (spawned beside the mat and planned at once)
                           or (visitor_tile == (6, 14) and visitor["phase"] == 6 and visitor["goalKind"] == 3))
                      and visitor_tile != (9, 11)
                      and blue_after["node"] == NODE_PEWTER and blue_after["destId"] != SPOT_PEWTER_GYM
                      and no_return)
    return result


SLOT_LANCE = 10


def plain_record(**fields) -> dict:
    base = dict(destKind=DEST_SPOT, waited=0, step=0, lifeEvent=0, lifeSteps=0, stayBits=0, reserved=0)
    base.update(fields)
    return base


def scenario_budget(game: Game) -> dict:
    """Three actors on one map: capacity, one grid search at a time, Wander."""
    boot(game, VIRIDIAN, 22, 33, DIR_NORTH)
    game.write_record(SLOT_LORELEI, **plain_record(node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_SQUARE,
                      state=STATE_DWELLING, arrival=ARRIVAL_NONE, crossing=0, activity=4, dwell=3))
    game.write_record(SLOT_LANCE, **plain_record(node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_WATER_2,
                      state=STATE_TRAVELLING, arrival=ARRIVAL_DOOR, crossing=4, activity=ACTIVITY_FISH, dwell=3))
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_FOREST_GRASS, state=STATE_TRAVELLING,
               arrival=ARRIVAL_DOOR, crossing=3, activity=ACTIVITY_TRAIN, dwell=3)
    game.warp(VIRIDIAN, 22, 33, DIR_NORTH)
    max_actors = max_searching = 0
    lorelei_tiles = set()
    for _ in range(0, 1500, 2):
        game.emu.step(2)
        dbg = game.walker_debug()
        live = [a for a in dbg["actors"] if a["mode"]]
        max_actors = max(max_actors, len(live))
        max_searching = max(max_searching, sum(1 for a in live if a["phase"] == 3))
        for a in live:
            if a["slot"] == SLOT_LORELEI and a["atSpot"]:
                lorelei_tiles.add((a["x"], a["y"]))
        if _ == 300:
            shot = game.shot("budget-three-actors")
    dbg = game.walker_debug()
    result = {"max_actors": max_actors, "max_concurrent_searches": max_searching,
              "lorelei_wander_tiles": sorted(lorelei_tiles), "template_moves": dbg["templateMoves"],
              "spawns": dbg["spawns"], "searches": dbg["searches"], "maxSliceScanlines": dbg["maxSliceScanlines"],
              "maxSearchNodes": dbg["maxSearchNodes"], "maxSearchFrames": dbg["maxSearchFrames"],
              "records": {name: game.record(slot) for name, slot in (("lorelei", SLOT_LORELEI), ("lance", SLOT_LANCE),
                                                                    ("blue", SLOT_BLUE))},
              "screenshots": [shot, game.shot("budget-later")], "debug": dbg}
    result["pass"] = (max_actors == 3 and max_searching <= 1 and dbg["spawns"] >= 3
                      and dbg["templateMoves"] >= 1 and len(lorelei_tiles) >= 2
                      and dbg["maxSliceScanlines"] < 228)
    return result


def scenario_browse(game: Game) -> dict:
    """Browse in the Mart, local dwell running out, leaving on foot."""
    boot(game, MART, 4, 6, DIR_NORTH)
    place_blue(game, node=NODE_MART, destId=SPOT_MART_SHELF, state=STATE_DWELLING,
               arrival=ARRIVAL_NONE, crossing=0, activity=ACTIVITY_SHOP, dwell=2)
    game.warp(MART, 4, 6, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("atSpot"), 600, what="Blue at the shelf")
    shelves = [game.record(SLOT_BLUE)["destId"]]
    beats_before = game.walker_debug()["localDwellBeats"]
    exits_before = game.walker_debug()["warpExits"]
    shot = None
    for i in range(0, 3000, 10):
        game.emu.step(10)
        rec = game.record(SLOT_BLUE)
        if rec["node"] == NODE_MART and rec["destId"] != shelves[-1] and TABLES.spots[rec["destId"]][1] == "WORLD_SPOT_STORE":
            shelves.append(rec["destId"])
            if shot is None:
                game.emu.step(60)
                shot = game.shot("browse-next-shelf")
        if game.walker_debug()["warpExits"] > exits_before:
            break
    dbg = game.walker_debug()
    after = game.record(SLOT_BLUE)
    result = {"shelves": shelves, "next_destination": after["destId"], "local_dwell_beats": dbg["localDwellBeats"] - beats_before,
              "local_advances": dbg["localAdvances"], "warp_exits": dbg["warpExits"] - exits_before,
              "record_after": after, "screenshots": [shot] if shot else [], "debug": dbg}
    result["pass"] = (len(shelves) >= 2 and result["local_dwell_beats"] >= 2 and dbg["localAdvances"] >= 1
                      and result["warp_exits"] == 1 and after["node"] == NODE_VIRIDIAN)
    return result


def scenario_save(game: Game) -> dict:
    """Save with an actor at its spot, reset, Continue."""
    boot(game, VIRIDIAN, 16, 41, DIR_NORTH)
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_WATER, state=STATE_DWELLING,
               arrival=ARRIVAL_NONE, crossing=0, activity=ACTIVITY_FISH, dwell=3)
    game.warp(VIRIDIAN, 16, 41, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("atSpot"), 600, what="Blue at the spot")
    game.emu.step(30)
    actor_before = game.actor_for(SLOT_BLUE)
    shot_before = game.shot("save-before")
    game.save()
    records_saved = game.world_bytes()
    block_saved = game.local_actor_block()
    heartbeats_saved = game.world_debug()["heartbeats"]

    # Reset through the GBA (A+B+Start+Select) so boot reads emulated flash.
    for b in ("A", "B", "Select", "Start"):
        game.emu.hold(b, 1)
    game.emu.step(2)
    for b in ("A", "B", "Select", "Start"):
        game.emu.hold(b, 0)
    game.emu.step(60)
    status = 0
    for _ in range(0, 900, 4):
        game.emu.step(4)
        status = game.emu.u16(game.sym["gSaveFileStatus"])
        if status == 1:
            break
    if status != 1:
        raise RuntimeError(f"No valid save after reset (status {status})")
    for _ in range(60):
        game.emu.step(30)
        dbg = game.walker_debug()
        if dbg["currentMap"] == VIRIDIAN and dbg["worldState"] and not game.controls_locked() \
                and any(o["isPlayer"] for o in game.objects()):
            break
        game.emu.press("A")
    game.settle()
    game.emu.step(10)
    records_loaded = game.world_bytes()
    actor_after = game.wait_for(lambda: game.actor_for(SLOT_BLUE), 240, what="Blue restored")
    block_after = game.local_actor_block()
    world_after = game.world_debug()
    shot_after = game.shot("save-after-continue")
    dbg = game.walker_debug()
    result = {
        "records_saved_hex": records_saved[4:204].hex(),
        "records_loaded_hex": records_loaded[4:204].hex(),
        "records_identical": records_saved[:204] == records_loaded[:204],
        "blue_record_saved": records_saved[4 + 8 * SLOT_BLUE:12 + 8 * SLOT_BLUE].hex(),
        "blue_record_loaded": records_loaded[4 + 8 * SLOT_BLUE:12 + 8 * SLOT_BLUE].hex(),
        "local_actor_block_saved": block_saved,
        "local_actor_block_after": block_after,
        "actor_before": actor_before,
        "actor_after": actor_after,
        "heartbeats_saved_session": heartbeats_saved,
        "world_debug_after_continue": world_after,
        "restores": dbg["restores"],
        "screenshots": [shot_before, shot_after],
        "debug": dbg,
    }
    result["pass"] = (result["records_identical"] and world_after["heartbeats"] == 0
                      and (actor_after["x"], actor_after["y"]) == (actor_before["x"], actor_before["y"])
                      and block_saved[0] is not None
                      and (block_saved[0]["x"], block_saved[0]["y"]) == (actor_before["x"], actor_before["y"])
                      and dbg["restores"] >= 1)
    return result


def hold_until(game: Game, button: str, predicate, limit: int = 600) -> bool:
    """Hold a button until predicate() holds (checked every 2 frames)."""
    game.emu.hold(button, 1)
    try:
        for _ in range(0, limit, 2):
            game.emu.step(2)
            if predicate():
                return True
    finally:
        game.emu.hold(button, 0)
    return False


def scenario_deadend(game: Game) -> dict:
    """H2: a walker never traps the player in a 1-wide dead end (Route 30).

    Blue plans his walk to the water's edge at the tip of the dead end
    (47, 49) while the corridor (36-47, 49) is free; the player then gets
    into the corridor ahead of him, so he walks in behind the player and
    stands between them and the only way out. Before the yield rule he
    stood there for good."""
    ROUTE30 = TABLES.map_id("MAP_ROUTE30_HNS")
    spot, node = TABLES.spot("MAP_ROUTE30_HNS", "WATER_EDGE", 47, 49)
    boot(game, ROUTE30, 35, 47, DIR_SOUTH)
    place_blue(game, node=node, destId=spot, state=STATE_TRAVELLING, arrival=ARRIVAL_DOOR, crossing=0,
               activity=ACTIVITY_FISH, dwell=3)
    game.warp(ROUTE30, 35, 47, DIR_SOUTH)
    game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("phase") == 4, 400, step=2, what="Blue walking")
    # Into the corridor, ahead of Blue.
    game.walk("Down", "y", 49)
    in_corridor = hold_until(game, "Right", lambda: game.player()["x"] >= 44, limit=600)
    game.emu.step(8)
    # Blue follows into the corridor and is stopped by the player.
    behind = game.wait_for(lambda: (lambda a: a and a["y"] == 49 and 36 <= a["x"] <= 43)(game.actor_for(SLOT_BLUE)),
                           1200, what="Blue in the corridor behind the player")
    game.emu.step(40)
    shot_blocked = game.shot("deadend-blocked")
    before = game.walker_debug()
    # The player walks back out west: Blue must make room.
    escaped = hold_until(game, "Left", lambda: game.player()["x"] <= 34, limit=1800)
    shot_out = game.shot("deadend-player-out")
    after = game.walker_debug()
    result = {"player_in_corridor": in_corridor, "blue_behind_player": behind, "player_escaped": escaped,
              "player": game.player(), "back_offs": after["backOffs"] - before["backOffs"],
              "handoffs": after["handoffs"] - before["handoffs"], "blue_after": game.actor_for(SLOT_BLUE),
              "record": game.record(SLOT_BLUE), "screenshots": [shot_blocked, shot_out], "debug": after}
    result["pass"] = bool(in_corridor) and bool(behind) and escaped \
        and (result["back_offs"] >= 1 or result["handoffs"] >= 1)
    return result


def scenario_gymentry(game: Game) -> dict:
    """H3: a Gym visitor never blocks the entrance (Saffron Gym: the player
    stands on the exit mat (14, 23), its one firing exit warp; the mat's side
    tiles (13, 23) and (15, 23) carry warp events that never fire)."""
    SAFFRON_GYM = TABLES.map_id("MAP_SAFFRON_CITY_GYM_HNS")
    spot, node = TABLES.spot("MAP_SAFFRON_CITY_GYM_HNS", "GYM", 14, 23)
    game.emu.step(240)
    arrange_with_badges(game, SAFFRON_GYM, 14, 23, 8)
    place_blue(game, node=node, destId=spot, state=STATE_DWELLING, arrival=ARRIVAL_NONE, crossing=0,
               activity=6, dwell=1)
    before = game.walker_debug()
    # The player arrives on the exit mat facing into the Gym.
    game.warp(SAFFRON_GYM, 14, 23, DIR_NORTH)
    spawned = None
    for _ in range(0, 120):
        game.emu.step(1)
        spawned = spawned or game.actor_for(SLOT_BLUE)
    shot = game.shot("gymentry-visitor")
    # The player walks straight in; nothing may stand in the way.
    walked_in = hold_until(game, "Up", lambda: game.player()["y"] <= 19, limit=400)
    game.emu.step(120)
    after = game.walker_debug()
    blue = game.record(SLOT_BLUE)
    left = (after["warpExits"] - before["warpExits"]) + (after["visitorsVanished"] - before["visitorsVanished"])
    tile = (spawned["x"], spawned["y"]) if spawned else None
    result = {"visitor_spawn_tile": tile, "player_walked_in": walked_in, "visitor_left": left,
              "visitor_removed": game.actor_for(SLOT_BLUE) is None, "record_after": blue,
              "screenshots": [shot, game.shot("gymentry-after")], "debug": after}
    # With the player on the mat the visitor can't walk out through it: it
    # leaves without walking out (visitorsVanished), never standing in the
    # way inside.
    result["pass"] = (walked_in and left >= 1 and result["visitor_removed"] and blue["node"] != node
                      and (tile is None or tile not in ((14, 22), (14, 21), (14, 20))))
    return result


def scenario_stairs(game: Game) -> dict:
    """H4: the walker takes sideways stairs as the graph does (Olivine Lighthouse door)."""
    OLIVINE = TABLES.map_id("MAP_OLIVINE_CITY_HNS")
    spot, node = TABLES.spot("MAP_OLIVINE_CITY_LIGHTHOUSE_HNS", "NPC_CHAT", 5, 4)
    olivine = TABLES.main_node("MAP_OLIVINE_CITY_HNS")
    boot(game, OLIVINE, 41, 51, DIR_WEST)
    # From the Mart door (warp 3) up the sideways stairs to the Lighthouse (warp 0).
    place_blue(game, node=olivine, destId=spot, state=STATE_TRAVELLING, arrival=ARRIVAL_DOOR, crossing=3,
               activity=6, dwell=1)
    exits_before = game.walker_debug()["warpExits"]
    game.warp(OLIVINE, 41, 51, DIR_WEST)
    track, diagonal = [], 0
    for _ in range(0, 2400, 2):
        game.emu.step(2)
        dbg = game.walker_debug()
        a = next((a for a in dbg["actors"] if a["slot"] == SLOT_BLUE and a["mode"]), None)
        if a:
            if track and abs(track[-1][0] - a["x"]) == 1 and abs(track[-1][1] - a["y"]) == 1:
                diagonal += 1
            if not track or track[-1] != (a["x"], a["y"]):
                track.append((a["x"], a["y"]))
        if dbg["warpExits"] > exits_before:
            break
    rec = game.record(SLOT_BLUE)
    result = {"track": track, "diagonal_steps": diagonal, "warp_exits": game.walker_debug()["warpExits"] - exits_before,
              "record": rec, "screenshots": [game.shot("stairs-after")], "debug": game.walker_debug()}
    result["pass"] = result["warp_exits"] == 1 and rec["node"] == node and diagonal >= 1 \
        and game.walker_debug()["searchFails"] == 0
    return result


def scenario_midstep(game: Game) -> dict:
    """H1/M1: a step under way at a menu or a save never replays without collision."""
    boot(game, VIRIDIAN, 16, 41, DIR_NORTH)
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_WATER, state=STATE_TRAVELLING,
               arrival=ARRIVAL_DOOR, crossing=4, activity=ACTIVITY_FISH, dwell=3)
    game.warp(VIRIDIAN, 16, 41, DIR_NORTH)
    game.wait_for(lambda: game.actor_for(SLOT_BLUE), 160, what="Blue's actor")

    def blue_object():
        a = game.actor_for(SLOT_BLUE)
        raw = game.emu.read(game.sym["gObjectEvents"] + a["objectId"] * OBJECT_SIZE, OBJECT_SIZE)
        x, y = struct.unpack_from("<hh", raw, 0x10)
        return {"held": bool(raw[0] & 0x40), "x": x - MAP_OFFSET, "y": y - MAP_OFFSET}

    # Menu: open the Start menu while a step is under way. The AI pauses
    # while the controls are locked, so the finished step stays "held"
    # until the walker clears it; the Bag then tears the field down. When
    # the field comes back (the Start menu still open, the AI still paused)
    # the object must not move: a replayed step would shift it a tile with
    # no collision check.
    game.wait_for(lambda: blue_object()["held"], 600, step=1, what="a step under way")
    game.emu.press("Start")
    at_menu = blue_object()
    game.emu.step(30)
    game.emu.press("Down")
    game.emu.press("A")
    game.emu.step(90)
    in_bag = blue_object()
    game.emu.press("B")
    game.emu.step(90)
    back = blue_object()
    still_menu = game.controls_locked()
    game.emu.press("B")
    game.emu.step(20)
    game.settle()
    menu_ok = (back["x"], back["y"]) == (in_bag["x"], in_bag["y"]) and still_menu

    # Save on the frame a step starts, reset, Continue.
    game.wait_for(lambda: blue_object()["held"], 1200, step=1, what="another step under way")
    held_at_save = blue_object()
    game.save()
    objects_at_save = game.objects()
    block = game.local_actor_block()
    for b in ("A", "B", "Select", "Start"):
        game.emu.hold(b, 1)
    game.emu.step(2)
    for b in ("A", "B", "Select", "Start"):
        game.emu.hold(b, 0)
    game.emu.step(60)
    for _ in range(0, 900, 4):
        game.emu.step(4)
        if game.emu.u16(game.sym["gSaveFileStatus"]) == 1:
            break
    for _ in range(60):
        game.emu.step(30)
        dbg = game.walker_debug()
        if dbg["currentMap"] == VIRIDIAN and dbg["worldState"] and not game.controls_locked() \
                and any(o["isPlayer"] for o in game.objects()):
            break
        game.emu.press("A")
    restored = game.wait_for(lambda: game.actor_for(SLOT_BLUE), 400, step=1, what="Blue restored")
    objects_after_continue = game.objects()
    game.settle()
    arrived = game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("atSpot"), 2400, what="arrival after Continue")
    final = game.actor_for(SLOT_BLUE)
    result = {"menu": {"at_menu": at_menu, "in_bag": in_bag, "back_16_frames": back, "no_replayed_step": menu_ok},
              "save": {"held_at_save": held_at_save, "block": block, "restored": restored, "final": final,
                       "objects_after_continue": objects_after_continue, "objects_at_save": objects_at_save},
              "screenshots": [game.shot("midstep-after-continue")], "debug": game.walker_debug()}
    # The save can land on the frame a step is issued but not yet applied:
    # the object is then saved on the step's start tile. Either way it comes
    # back within one tile of the step under way (a replayed step would add
    # one more), and it carries on to its spot instead of freezing.
    result["pass"] = (menu_ok and at_menu["held"] and block[0] is not None
                      and abs(restored["x"] - held_at_save["x"]) + abs(restored["y"] - held_at_save["y"]) <= 1
                      and game.walker_debug()["restores"] >= 1
                      and bool(arrived) and (final["x"], final["y"]) == (13, 39))
    return result


def scenario_decoys(game: Game) -> dict:
    """M4: the Gym Leader object is matched by local id (Fuchsia's Janine decoys stay)."""
    FUCHSIA_GYM = TABLES.map_id("MAP_FUCHSIA_CITY_GYM_HNS")
    VIRIDIAN_GYM = TABLES.map_id("MAP_VIRIDIAN_CITY_GYM")
    slot_janine, slot_giovanni = 4, 7
    game.emu.step(240)
    arrange_with_badges(game, FUCHSIA_GYM, 6, 10, 8)
    away = dict(destKind=DEST_SPOT, destId=SPOT_VIRIDIAN_WATER, state=STATE_TRAVELLING, arrival=ARRIVAL_NONE,
                crossing=0, activity=ACTIVITY_FISH, dwell=3, waited=0, lifeEvent=0, lifeSteps=0, stayBits=0, reserved=0)
    game.write_record(slot_janine, node=NODE_VIRIDIAN, **away)
    game.write_record(slot_giovanni, node=NODE_ROUTE1, **away)
    game.warp(FUCHSIA_GYM, 6, 10, DIR_NORTH)
    game.emu.step(30)
    fuchsia = [o for o in game.objects() if not o["isPlayer"]]
    fuchsia_shot = game.shot("decoys-fuchsia")
    game.warp(VIRIDIAN_GYM, 4, 4, DIR_NORTH)
    game.emu.step(30)
    viridian = [o for o in game.objects() if not o["isPlayer"]]
    result = {"fuchsia_objects": fuchsia, "viridian_objects": viridian,
              "screenshots": [fuchsia_shot, game.shot("decoys-viridian-gym")]}
    janine_gfx = {o["gfx"] for o in fuchsia if o["localId"] in (1, 2, 3, 4)} if fuchsia else set()
    decoys = [o for o in fuchsia if o["localId"] in (1, 2, 3, 4, 5) and o["gfx"] in janine_gfx]
    result["decoys_visible"] = sorted(o["localId"] for o in decoys)
    result["giovanni_objects"] = [o for o in viridian if o["gfx"] == 309]
    result["pass"] = (5 not in result["decoys_visible"] and len(result["decoys_visible"]) >= 1
                      and not result["giovanni_objects"])
    return result


def scenario_perf(game: Game) -> dict:
    """Heartbeat cost with every badge held: all 25 trainers simulated."""
    game.emu.step(240)
    arrange_with_badges(game, CENTER, 7, 6, 8, 8, 8)
    game.reset_frame_maxima()
    rows = []
    for i in range(60):
        map_id, x, y = (MART, 4, 6) if i % 2 == 0 else (CENTER, 7, 6)
        heartbeats = game.world_debug()["heartbeats"]
        before = game.emu.frames
        # Frames per warp: until the controls unlock, as on earlier ROMs.
        game.command(CMD_WARP, map_id, x, y, DIR_NORTH)
        game.settle()
        frames = game.emu.frames - before
        game.wait_heartbeat(heartbeats)
        world = game.world_debug()
        rows.append({"warp": i + 1, "frames": frames,
                     "heartbeat_scanlines": game.walker_debug()["lastHeartbeatScanlines"],
                     "heartbeat_frames": world.get("lastHeartbeatFrames"),
                     "context_scanlines": game.walker_debug()["lastContextScanlines"],
                     "trace": game.emu.read(game.sym["gWayfarerWorldDebug"] + 8, 20).hex()})
    dbg = game.walker_debug()
    world = game.world_debug()
    scan = sorted(r["heartbeat_scanlines"] for r in rows)
    frames = [r["frames"] for r in rows]
    result = {"warps": rows,
              # The whole heartbeat's work (all its frames on a sliced ROM).
              "max_heartbeat_scanlines": scan[-1], "mean_heartbeat_scanlines": sum(scan) / len(scan),
              "median_heartbeat_scanlines": scan[len(scan) // 2],
              "max_heartbeat_frames": round(scan[-1] / SCANLINES_PER_FRAME, 2),
              "over_two_frames": sum(1 for v in scan if v > 2 * SCANLINES_PER_FRAME),
              "mean_warp_frames": sum(frames) / len(frames), "max_warp_frames": max(frames),
              "max_walker_update_scanlines": dbg["maxUpdateScanlines"], "world": world, "debug": dbg}
    if game.sliced:
        # What a single frame carried: the worst frame of heartbeat work, and
        # with the walkers' update in the same frame.
        result["max_step_scanlines"] = world["maxStepScanlines"]
        result["max_frame_scanlines"] = world["maxFrameScanlines"]
        result["max_finish_scanlines"] = world["maxFinishScanlines"]
        result["frames_per_heartbeat_max"] = max(r["heartbeat_frames"] for r in rows)
    result["ceilings"] = {"frame_scanlines": CEILING_FRAME_SCANLINES, "warp_frames": CEILING_WARP_FRAMES}
    result["target"] = {"frame_scanlines": TARGET_FRAME_SCANLINES,
                        "within_target": game.sliced and world["maxFrameScanlines"] <= TARGET_FRAME_SCANLINES}
    result["pass"] = (game.sliced and world["maxFrameScanlines"] <= CEILING_FRAME_SCANLINES
                      and dbg["maxUpdateScanlines"] <= CEILING_FRAME_SCANLINES
                      and max(frames) <= CEILING_WARP_FRAMES)
    return result


def scenario_seam(game: Game) -> dict:
    """Heartbeat cost per frame on a seamless edge: Viridian <-> Route 2, every badge held."""
    game.emu.step(240)
    arrange_with_badges(game, VIRIDIAN, 27, 2, 8, 8, 8)
    game.emu.step(30)
    game.reset_frame_maxima()
    rows = []
    crossings = 30
    for i in range(crossings):
        north = i % 2 == 0
        heartbeats = game.world_debug()["heartbeats"]
        before = game.emu.frames
        # Two tiles past the seam: Route 2 is 80 tiles tall, Viridian's top row is 0.
        game.walk("Up" if north else "Down", "y", 77 if north else 2)
        game.wait_heartbeat(heartbeats)
        world = game.world_debug()
        dbg = game.walker_debug()
        rows.append({"crossing": i + 1, "to": "Route 2" if north else "Viridian", "map": game.current_map(),
                     "frames": game.emu.frames - before,
                     "heartbeat_scanlines": dbg["lastHeartbeatScanlines"],
                     "heartbeat_frames": world.get("lastHeartbeatFrames")})
    world = game.world_debug()
    dbg = game.walker_debug()
    scan = sorted(r["heartbeat_scanlines"] for r in rows)
    result = {"crossings": rows, "heartbeats": world["heartbeats"],
              "max_heartbeat_scanlines": scan[-1], "median_heartbeat_scanlines": scan[len(scan) // 2],
              "mean_heartbeat_scanlines": sum(scan) / len(scan),
              "max_walker_update_scanlines": dbg["maxUpdateScanlines"], "world": world, "debug": dbg}
    if game.sliced:
        result["max_step_scanlines"] = world["maxStepScanlines"]
        result["max_frame_scanlines"] = world["maxFrameScanlines"]
        result["max_finish_scanlines"] = world["maxFinishScanlines"]
        result["frames_per_heartbeat_max"] = max(r["heartbeat_frames"] for r in rows)
        # Target: no frame carries more than half a frame of heartbeat plus walker work.
        result["within_target"] = world["maxFrameScanlines"] <= TARGET_FRAME_SCANLINES
    else:
        # The whole heartbeat ran in the seam's frame, after the walkers' update.
        result["max_frame_scanlines_estimate"] = scan[-1] + dbg["maxUpdateScanlines"]
    maps = [r["map"] for r in rows]
    result["ceilings"] = {"frame_scanlines": CEILING_FRAME_SCANLINES}
    result["pass"] = (all(m == (ROUTE2 if i % 2 == 0 else VIRIDIAN) for i, m in enumerate(maps))
                      and game.sliced and world["pendingAtLoad"] == 0 and world["forcedFinishes"] == 0
                      and world["maxFrameScanlines"] <= CEILING_FRAME_SCANLINES
                      and dbg["maxUpdateScanlines"] <= CEILING_FRAME_SCANLINES)
    return result


def scenario_recross(game: Game) -> dict:
    """E7: running back and forth over the Viridian <-> Route 2 seam, every
    badge held, so a camera transition often comes while the last crossing's
    heartbeat is still pending (pendingAtLoad). The new heartbeat then
    queues behind the pending one (deferredBegins, up to two deep) instead
    of finishing it inside the seam's frame; with the queue full only the
    oldest heartbeat finishes there (maxFinishScanlines). Gated: the case was
    hit, a heartbeat was queued, no sliced frame passed the frame ceiling,
    and no seam frame finished more than CEILING_SEAM_FINISH_SCANLINES."""
    game.emu.step(240)
    arrange_with_badges(game, VIRIDIAN, 27, 1, 8, 8, 8, flags=(FLAG_RUNNING_SHOES,))
    game.emu.step(30)
    game.reset_frame_maxima()
    start = game.world_debug()
    rows = []
    game.emu.hold("B", 1)
    try:
        for i in range(40):
            north = i % 2 == 0
            before = game.emu.frames
            target = ROUTE2 if north else VIRIDIAN
            pending = game.world_debug()["pending"]
            reached = hold_until(game, "Up" if north else "Down", lambda: game.current_map() == target, limit=240)
            rows.append({"crossing": i + 1, "to": "Route 2" if north else "Viridian", "reached": reached,
                         "frames": game.emu.frames - before, "pending_before": pending})
    finally:
        game.emu.hold("B", 0)
    game.finish_heartbeat()
    world = game.world_debug()
    dbg = game.walker_debug()
    result = {"crossings": rows, "pending_at_load": world["pendingAtLoad"] - start["pendingAtLoad"],
              "deferred_begins": world["deferredBegins"] - start["deferredBegins"],
              "heartbeats": world["heartbeats"] - start["heartbeats"],
              "max_finish_scanlines": world["maxFinishScanlines"],
              "max_frame_scanlines": world["maxFrameScanlines"], "max_step_scanlines": world["maxStepScanlines"],
              "max_walker_update_scanlines": dbg["maxUpdateScanlines"],
              "mean_crossing_frames": sum(r["frames"] for r in rows) / len(rows),
              "world": world, "debug": dbg, "screenshots": [game.shot("recross-after")]}
    result["ceilings"] = {"frame_scanlines": CEILING_FRAME_SCANLINES,
                          "seam_finish_scanlines": CEILING_SEAM_FINISH_SCANLINES}
    result["pass"] = (all(r["reached"] for r in rows) and result["pending_at_load"] >= 1
                      and result["deferred_begins"] >= 1
                      and world["maxFinishScanlines"] <= CEILING_SEAM_FINISH_SCANLINES
                      and world["maxFrameScanlines"] <= CEILING_FRAME_SCANLINES
                      and dbg["maxUpdateScanlines"] <= CEILING_FRAME_SCANLINES)
    return result


SLOT_WILL = 19
SLOT_STEVEN = 24


def scenario_longtrip(game: Game) -> dict:
    """E1: a watched trainer's stay ends and his next step is far away.

    Will dwells at home at Indigo Plateau with one heartbeat of dwell left;
    his next step (study) is the Ruins of Alph, a long trip whose first edge
    is the Elite Four fly. The routine advance (one spot choice per frame) and
    the trip's first search (a slice per frame) must not land in one frame:
    the walkers' worst update stays under the ceiling. He then hands off at
    the fly (an off-screen link) and walks out of view."""
    INDIGO = TABLES.map_id("MAP_INDIGO_PLATEAU_HNS")
    node = TABLES.main_node("MAP_INDIGO_PLATEAU_HNS")
    home_spot = next(i for i, (n, *_r) in enumerate(TABLES.spots) if n == node)
    ruins = {i for i, (n, kind, *_r) in enumerate(TABLES.spots)
             if TABLES.nodes[n] == "MAP_RUINS_OF_ALPH_OUTSIDE_HNS" and kind == "WORLD_SPOT_NAMED"}
    game.emu.step(240)
    arrange_with_badges(game, INDIGO, 11, 12, 8, 8, 8)
    game.write_record(SLOT_WILL, node=node, destKind=DEST_SPOT, destId=home_spot, state=STATE_DWELLING,
                      arrival=ARRIVAL_NONE, crossing=0, activity=8, dwell=1, waited=0, step=3,
                      lifeEvent=0, lifeSteps=0, stayBits=0, reserved=0)
    game.warp(INDIGO, 11, 12, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_WILL) or {}).get("atSpot"), 900, step=4, what="Will at his spot")
    game.reset_frame_maxima()
    before = game.walker_debug()
    # Ten dwell ticks of a second each run his last heartbeat of dwell down.
    advanced = game.wait_for(lambda: game.walker_debug()["localAdvances"] > before["localAdvances"], 900, step=2,
                             what="the local dwell running out")
    advance_frame = game.emu.frames
    planned_at = None
    for _ in range(0, 1200, 2):
        game.emu.step(2)
        dbg = game.walker_debug()
        actor = game.actor_for(SLOT_WILL)
        if dbg["handoffs"] > before["handoffs"] or actor is None or actor["mode"] == 3 or actor["phase"] in (3, 4):
            planned_at = game.emu.frames
            break
    record = game.record(SLOT_WILL)
    game.emu.step(120)
    after = game.walker_debug()
    result = {"record_after": record, "dest_is_ruins": record["destId"] in ruins,
              "frames_advance_to_plan": (planned_at - advance_frame) if planned_at else None,
              "world_jobs": after["worldJobs"] - before["worldJobs"],
              "handoffs": after["handoffs"] - before["handoffs"],
              "max_job_scanlines": after["maxJobScanlines"],
              "max_walker_update_scanlines": after["maxUpdateScanlines"],
              "ceilings": {"frame_scanlines": CEILING_FRAME_SCANLINES},
              "screenshots": [game.shot("longtrip-after")], "debug": after}
    result["pass"] = (bool(advanced) and planned_at is not None and result["dest_is_ruins"]
                      and result["world_jobs"] >= 2 and result["handoffs"] >= 1
                      and after["maxUpdateScanlines"] <= CEILING_FRAME_SCANLINES)
    return result


def scenario_striplane(game: Game) -> dict:
    """E2: a strip actor blocked straight ahead doesn't stand in the seam.

    Route 134's shallows (75-79, 26-34; the player waits at (75, 29), in view) cross into Route 133's (0, 26-32),
    whose next column is wall: a walker that steps out east becomes a strip
    actor that can't walk straight on. It must go after a moment (it was
    handed off as it stepped out) instead of standing there while visible."""
    R134 = TABLES.map_id("MAP_ROUTE134")
    spot, node133 = TABLES.spot("MAP_ROUTE133", "WATER_EDGE", 0, 29)
    lane = next(i for i, (t, kind, a, b, c) in enumerate(TABLES.edges) if t == node133 and kind == "WORLD_EDGE_EAST")
    src = TABLES.edge_source(lane)   # Route 134's shallows
    boot(game, R134, 75, 29, DIR_EAST)
    game.write_record(SLOT_STEVEN, node=src, destKind=DEST_SPOT, destId=spot, state=STATE_TRAVELLING,
                      arrival=ARRIVAL_NONE, crossing=0, activity=ACTIVITY_FISH, dwell=3, waited=0, step=0,
                      lifeEvent=0, lifeSteps=0, stayBits=0, reserved=0)
    before = game.walker_debug()
    game.warp(R134, 75, 29, DIR_EAST)
    game.wait_for(lambda: game.actor_for(SLOT_STEVEN), 400, step=2, what="Steven spawned")
    track, exit_frame, stalled, gone = [], None, None, None
    for _ in range(0, 2400):
        game.emu.step(1)
        dbg = game.walker_debug()
        a = next((a for a in dbg["actors"] if a["slot"] == SLOT_STEVEN and a["mode"]), None)
        state = (a["mode"], a["x"], a["y"]) if a else None
        if not track or track[-1][1] != state:
            track.append((game.emu.frames, state))
        if exit_frame is None and dbg["edgeExits"] > before["edgeExits"]:
            exit_frame = game.emu.frames
        if a and a["mode"] == 2:
            stalled = stalled or a
        if exit_frame is not None and a is None:
            gone = game.emu.frames
            break
    after = game.walker_debug()
    record = game.record(SLOT_STEVEN)
    result = {"strip_actor": stalled, "track": track, "player": game.player(),
              "removed_after_frames": gone - exit_frame if gone else None,
              "strip_timeouts": after["stripTimeouts"] - before["stripTimeouts"],
              "record": record, "screenshots": [game.shot("striplane-after")], "debug": after}
    result["pass"] = (bool(gone) and bool(stalled) and result["strip_timeouts"] >= 1 and record["node"] == node133)
    return result


def scenario_bridge(game: Game) -> dict:
    """F4: a path that crosses a bridge tile at its second elevation (Route 119).

    Route 119's deck (rows 84-85, elevation 4) shares elevation-15 tiles with
    the path underneath it (x 26-27, elevation 3). From the south a walker
    first reaches (26,84) and (27,84) from below; the way west along the deck
    crosses the same tiles at elevation 4. A search that keeps one state per
    tile loses everything north of the bridge."""
    route = TABLES.map_id("MAP_ROUTE119")
    sx, sy = 16, 33
    spot, node = TABLES.spot("MAP_ROUTE119", "WATER_EDGE", sx, sy)
    boot(game, route, 30, 30, DIR_SOUTH)
    place_blue(game, node=node, destId=spot, state=STATE_TRAVELLING, arrival=2,  # WORLD_ARRIVAL_SOUTH
               crossing=18, activity=ACTIVITY_FISH, dwell=3)
    before = game.walker_debug()
    game.warp(route, 30, 30, DIR_SOUTH)
    deck, arrived = [], None
    for f in range(0, 7200, 8):
        game.emu.step(8)
        a = game.actor_for(SLOT_BLUE)
        if a and 80 <= a["y"] <= 90 and (not deck or deck[-1] != (a["x"], a["y"])):
            deck.append((a["x"], a["y"]))
        if a and a["atSpot"] and (a["x"], a["y"]) == (sx, sy):
            arrived = f
            break
    after = game.walker_debug()
    record = game.record(SLOT_BLUE)
    delta = {k: after[k] - before[k] for k in ("searches", "searchFails", "arrivals", "replans")}
    result = {"spot": spot, "node": node, "arrived_after_frames": arrived, "delta": delta,
              "deck_track": deck, "record": record, "maxSliceScanlines": after["maxSliceScanlines"],
              "maxSearchNodes": after["maxSearchNodes"], "workspaceBytes": after["workspaceBytes"],
              "maxFinishScanlines": after["maxFinishScanlines"], "screenshots": [game.shot("bridge-north")], "debug": after}
    result["pass"] = (arrived is not None and delta["searchFails"] == 0 and delta["arrivals"] >= 1
                      and record["state"] == STATE_DWELLING and record["destId"] == spot)
    return result


def scenario_walkoff(game: Game) -> dict:
    """Follow-up 1: a walker that hands off walks out of view instead of
    vanishing where it stands.

    Blue stands at Viridian's pond (13, 39). The player keeps pushing against
    him: each push makes him back off, and the fourth hands him off. He then
    walks to the nearest way out of Viridian (a door or a map side) and is
    removed there or once he leaves the camera's view."""
    boot(game, VIRIDIAN, 13, 43, DIR_NORTH)
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_WATER, state=STATE_DWELLING,
               activity=ACTIVITY_FISH, dwell=6)
    game.warp(VIRIDIAN, 13, 43, DIR_NORTH)
    game.wait_for(lambda: game.actor_for(SLOT_BLUE), 400, step=2, what="Blue spawned")
    game.emu.step(60)
    before = game.walker_debug()

    def towards(axis: str, target: int, positive: str, negative: str) -> None:
        here = game.player()[axis]
        if here != target:
            hold_until(game, positive if target > here else negative,
                       lambda: game.player()[axis] == target, limit=240)

    rounds, leaving, shots = [], None, []
    for _ in range(10):
        blue = game.actor_for(SLOT_BLUE)
        if blue is None or blue["mode"] == 3:
            leaving = blue
            break
        # Stand south of Blue (or as near as the ground allows), then push north.
        towards("x", blue["x"], "Right", "Left")
        towards("y", blue["y"] + 1, "Down", "Up")
        player = game.player()
        push = {(0, 1): "Up", (0, -1): "Down", (1, 0): "Left", (-1, 0): "Right"}.get(
            (player["x"] - blue["x"], player["y"] - blue["y"]))
        if push:
            hold_until(game, push, lambda: (game.actor_for(SLOT_BLUE) or {"mode": 3})["mode"] == 3
                       or (game.actor_for(SLOT_BLUE) or {})["goalKind"] == 4, limit=90)
        rounds.append({"player": game.player(), "blue": game.actor_for(SLOT_BLUE), "pushed": push})
        game.emu.step(30)
    else:
        leaving = game.actor_for(SLOT_BLUE)
    if leaving and leaving["mode"] == 3:
        game.emu.step(20)
        shots.append(game.shot("walkoff-leaving"))
    track = []
    gone = None
    for f in range(0, 900, 4):
        game.emu.step(4)
        blue = game.actor_for(SLOT_BLUE)
        if blue is None:
            gone = f
            break
        if not track or track[-1] != (blue["x"], blue["y"]):
            track.append((blue["x"], blue["y"]))
    after = game.walker_debug()
    delta = {k: after[k] - before[k] for k in ("backOffs", "handoffs", "walkOffs", "removals")}
    result = {"rounds": rounds, "leaving": leaving, "track": track, "gone_after_frames": gone,
              "delta": delta, "player": game.player(), "record": game.record(SLOT_BLUE),
              "screenshots": shots, "debug": after}
    result["pass"] = (delta["handoffs"] >= 1 and delta["walkOffs"] >= 1 and gone is not None
                      and len(track) >= 2)
    return result


def scenario_mortar(game: Game) -> dict:
    """Follow-up 2 (F5): Mt Mortar 1F North agrees with the generator on screen,
    and (critic loop T2) the pocket is reachable through the graph.

    The generator now floods one-way stair moves as directed: nodes are
    strongly connected components. From the B1F door (15, 31) the water's
    edge (53, 14) is in the same node; Pryce's pocket (56, 20) is reachable
    from there only across water, which the off-screen simulation crosses
    (the walker hands off and walks out of view). Legs: in to (53, 14), back
    out through a door, and asked for (56, 20): a handoff, no failed search."""
    MORTAR = TABLES.map_id("MAP_MT_MORTAR_1F_NORTH_HNS")
    spot, node = TABLES.spot("MAP_MT_MORTAR_1F_NORTH_HNS", "WATER_EDGE", 53, 14)
    pocket, pocket_node = TABLES.spot("MAP_MT_MORTAR_1F_NORTH_HNS", "WATER_EDGE", 56, 20)
    back_spot, _ = TABLES.spot("MAP_MT_MORTAR_B1F_HNS", "NPC_CHAT", 39, 9)
    door = next(i for i, w in enumerate(json.loads((ROOT / "game/data/maps/MtMortar_1F_North_hns/map.json")
                                                   .read_text())["warp_events"]) if (w["x"], w["y"]) == (15, 31))
    slot = 19   # Will: a Johto trainer, so his routes may stay in Mt Mortar (Will's can't leave Kanto)
    keys = ("searches", "searchFails", "arrivals", "warpExits", "handoffs", "walkOffs", "blockedSteps")
    legs = {}

    def run(fields, until, what, limit=9000, spawn=True):
        try:
            return leg(fields, until, what, limit, spawn)
        except RuntimeError as error:
            return {"done": False, "error": str(error), "debug": game.walker_debug(),
                    "record": game.record(slot), "delta": {}, "record_map": ""}

    def leg(fields, until, what, limit, spawn):
        before = game.walker_debug()
        base = dict(destKind=DEST_SPOT, waited=0, step=0, lifeEvent=0, lifeSteps=0, stayBits=0, reserved=0)
        base.update(fields)
        game.write_record(slot, **base)
        game.warp(MORTAR, 23, 58, DIR_NORTH)    # the player waits in another node, out of the way
        spawned = game.wait_for(lambda: game.actor_for(slot), 400, step=2, what="Will spawned") if spawn else None
        done = game.wait_for(lambda: until(game.walker_debug(), before), limit, step=8, what=what)
        after = game.walker_debug()
        return {"spawned": spawned, "done": bool(done), "delta": {k: after[k] - before[k] for k in keys},
                "record": game.record(slot), "record_map": TABLES.nodes[game.record(slot)["node"]]}

    boot(game, MORTAR, 23, 58, DIR_NORTH)
    legs["in"] = run(dict(node=node, destId=spot, state=STATE_TRAVELLING, arrival=ARRIVAL_DOOR, crossing=door,
                          activity=ACTIVITY_FISH, dwell=3),
                     lambda d, b: d["arrivals"] > b["arrivals"], "Will at (53, 14)")
    shots = [game.shot("mortar-at-spot")]
    legs["back"] = run(dict(node=node, destId=back_spot, state=STATE_TRAVELLING, arrival=0, crossing=0,
                            activity=ACTIVITY_FISH, dwell=3),
                       lambda d, b: d["warpExits"] > b["warpExits"], "Will out through a door")
    legs["water"] = run(dict(node=node, destId=pocket, state=STATE_TRAVELLING, arrival=0, crossing=0,
                             activity=ACTIVITY_FISH, dwell=3),
                        lambda d, b: d["handoffs"] > b["handoffs"] and game.actor_for(slot) is None,
                        "Will handed off at the water", limit=3000, spawn=False)
    # Through the graph, off-screen (critic loop T2): from Route 42, Will
    # reaches the pocket over the ROM's own edges, one hop per heartbeat; the
    # way up Mt Mortar crosses 1F South's foam row (y=28) as a surfer does.
    route42 = TABLES.main_node("MAP_ROUTE42_HNS")
    game.write_record(slot, node=route42, destKind=DEST_SPOT, destId=pocket, state=STATE_TRAVELLING, arrival=0,
                      crossing=0, activity=ACTIVITY_FISH, dwell=3, waited=0, step=0, lifeEvent=0, lifeSteps=0,
                      stayBits=0, reserved=0)
    nodes = [route42]
    CENTER_MAP, MART_MAP = CENTER, MART
    for i in range(24):
        game.warp(MART_MAP if i % 2 == 0 else CENTER_MAP, 4 if i % 2 == 0 else 7, 6, DIR_NORTH)
        rec = game.record(slot)
        nodes.append(rec["node"])
        if rec["node"] == pocket_node:
            break
    legs_graph = {"nodes": nodes, "maps": [TABLES.nodes[n] for n in nodes], "reached": nodes[-1] == pocket_node,
                  "record": game.record(slot)}
    result = {"legs": legs, "graph_leg": legs_graph, "pocket_node": pocket_node, "node": node, "screenshots": shots,
              "debug": game.walker_debug()}
    result["pass"] = (pocket_node != node and legs_graph["reached"]
                      and all(legs[k]["done"] and legs[k]["delta"].get("searchFails") == 0 for k in legs))
    return result


def scenario_safari(game: Game) -> dict:
    """Follow-up 2 (F5): the Safari Zone stair pockets agree on screen.

    With one-way stair moves directed, the east and west lanes of Low Mid and
    Top Mid are their own nodes (the Engineers gate the expansions). Blue
    enters each main area by its south lane and walks to a spot in the main
    node; the player waits in the (separate) west lane."""
    cases = [("MAP_SAFARI_ZONE_LOW_MID_HNS", "TALL_GRASS", (23, 13), 18, (0, 14)),
             ("MAP_SAFARI_ZONE_TOP_MID_HNS", "TALL_GRASS", (10, 6), 19, (0, 15))]
    legs = {}
    for name, kind, (sx, sy), crossing, (px, py) in cases:
        map_id = TABLES.map_id(name)
        spot, node = TABLES.spot(name, kind, sx, sy)
        boot(game, map_id, px, py, DIR_EAST)
        place_blue(game, node=node, destId=spot, state=STATE_TRAVELLING, arrival=2, crossing=crossing,  # south
                   activity=ACTIVITY_TRAIN, dwell=3)
        before = game.walker_debug()
        game.warp(map_id, px, py, DIR_EAST)
        try:
            game.wait_for(lambda: game.actor_for(SLOT_BLUE), 400, step=2, what="Blue spawned")
            arrived = game.wait_for(lambda: (lambda a: a and a["atSpot"] and (a["x"], a["y"]) == (sx, sy))(
                game.actor_for(SLOT_BLUE)), 6000, step=8, what=f"Blue at {name} {sx, sy}")
            error = None
        except RuntimeError as failure:
            arrived, error = None, str(failure)
        after = game.walker_debug()
        legs[name] = {"arrived": bool(arrived), "error": error,
                      "delta": {k: after[k] - before[k] for k in ("searches", "searchFails", "arrivals", "blockedSteps")}}
    result = {"legs": legs, "debug": game.walker_debug(), "screenshots": [game.shot("safari-top-mid")]}
    result["pass"] = all(v["arrived"] and v["delta"]["searchFails"] == 0 for v in legs.values())
    return result


# ---------------------------------------------------------------------------
# Notable ambience: beats (src/wayfarer_walker_beats.c, the "Notable ambience"
# section of src/wayfarer_walkers.c). The beat state lives in a heap block
# that the static pointer sAmbience points at; its first bytes are struct
# WalkerAmbienceDebug (include/wayfarer_walker_beats.h). The block starts
# again (counters and log at 0) after every heap reset: a warp or a menu.

AMBIENCE_DEBUG_SIZE = 28
AMBIENCE_LOG_SIZE = 32
# struct WalkerAmbienceDebug's companion counters, after the log.
AMBIENCE_COMPANION_OFFSET = AMBIENCE_DEBUG_SIZE + 8 * AMBIENCE_LOG_SIZE
COMPANION_AWAY = ("slots", "follower", "vanished", "interrupted")
COMPANION_DENY = ("place", "busy", "slots", "follower", "palette", "tile")
AMBIENCE_COMPANION_SIZE = 2 * (2 + len(COMPANION_AWAY) + len(COMPANION_DENY) + 2)
BEAT_NONE = 0xFF
BEAT_EVENTS = ("start", "end", "interrupt")
SLOT_MISTY = 1
SLOT_GIOVANNI = 7


def beat_names() -> list:
    """Beat ids by index, from the world generator's report (build/wayfarer-world-report.json)."""
    report = json.loads((ROOT / "game/build/wayfarer-world-report.json").read_text())
    return report["ambience"]["beats"]


class BeatWatcher:
    """Collects the beat event ring across polls (and across block resets)."""

    def __init__(self, game: Game):
        self.game = game
        self.names = beat_names()
        self.pointer = None
        self.seen = 0
        self.events = []
        self.lost = 0
        self.counters = None

    def read(self):
        address = self.game.emu.u32(self.game.sym["sAmbience"])
        if not 0x02000000 <= address < 0x02040000:
            return None, None
        return address, self.game.emu.read(address, AMBIENCE_COMPANION_OFFSET + AMBIENCE_COMPANION_SIZE)

    def poll(self) -> list:
        """New events since the last poll."""
        address, raw = self.read()
        if raw is None:
            self.pointer = None
            return []
        count = struct.unpack_from("<H", raw, 24)[0]
        if address != self.pointer or count < self.seen:
            self.pointer, self.seen = address, 0
        start = max(self.seen, count - AMBIENCE_LOG_SIZE)
        self.lost += start - self.seen
        new = []
        for n in range(start, count):
            frame, actor, slot, beat, event = struct.unpack_from("<IBBBB", raw, AMBIENCE_DEBUG_SIZE + 8 * (n % AMBIENCE_LOG_SIZE))
            new.append({"frame": frame, "actor": actor, "slot": slot, "beat": beat,
                        "name": self.names[beat] if beat < len(self.names) else beat,
                        "event": BEAT_EVENTS[event] if event < len(BEAT_EVENTS) else event})
        self.seen = count
        self.events.extend(new)
        values = struct.unpack_from("<10H", raw, 0)
        self.counters = dict(zip(("started_idle", "started_transition", "started_react", "ended", "interrupted",
                                  "icons", "effects", "palette_skips", "icon_skips", "step_skips"), values))
        self.counters["running"] = list(raw[20:24])
        values = struct.unpack_from(f"<{AMBIENCE_COMPANION_SIZE // 2}H", raw, AMBIENCE_COMPANION_OFFSET)
        away, denied = len(COMPANION_AWAY), len(COMPANION_DENY)
        self.counters["companion"] = {
            "out": values[0], "in": values[1],
            "away": dict(zip(COMPANION_AWAY, values[2:2 + away])),
            "denied": dict(zip(COMPANION_DENY, values[2 + away:2 + away + denied])),
            "out_failed": values[2 + away + denied], "strays": values[3 + away + denied]}
        return new

    def running(self, actor_index: int):
        """The beat name the actor runs now, or None."""
        _, raw = self.read()
        if raw is None or raw[20 + actor_index] == BEAT_NONE:
            return None
        return self.names[raw[20 + actor_index]]

    def watch(self, frames: int, step: int = 2, until=None):
        """Step and poll; stop early once until(new_events) holds. Returns the event that matched."""
        for _ in range(0, frames, step):
            self.game.emu.step(step)
            for event in self.poll():
                if until and until(event):
                    return event
        return None

    def starts(self, slot=None, name=None) -> list:
        return [e for e in self.events if e["event"] == "start" and (slot is None or e["slot"] == slot)
                and (name is None or e["name"] == name)]


def object_of(game: Game, slot: int):
    actor = game.actor_for(slot)
    if actor is None:
        return None
    return next((o for o in game.objects() if o["slot"] == actor["objectId"]), None)


def place_dwelling(game: Game, slot: int, spot: int, node: int, activity: int = ACTIVITY_FISH, dwell: int = 40) -> None:
    game.write_record(slot, **plain_record(node=node, destId=spot, state=STATE_DWELLING, arrival=ARRIVAL_NONE,
                                           crossing=0, activity=activity, dwell=dwell))


def scenario_beatspot(game: Game) -> dict:
    """Acceptance 2: Blue dwelling at Viridian's pond (13, 39), facing the
    water, runs water_bite or water_wait, and never a beat whose context
    doesn't hold there (no grass, store, walking or other trainers' beats)."""
    boot(game, VIRIDIAN, 16, 43, DIR_NORTH)
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_WATER, state=STATE_DWELLING,
               activity=ACTIVITY_FISH, dwell=40)
    game.warp(VIRIDIAN, 16, 43, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("atSpot"), 600, what="Blue at the pond")
    watcher = BeatWatcher(game)
    water = watcher.watch(3600, step=2, until=lambda e: e["event"] == "start" and e["slot"] == SLOT_BLUE
                          and e["name"] in ("water_bite", "water_wait"))
    facing_at_start = (object_of(game, SLOT_BLUE) or {}).get("facing")
    actor = game.actor_for(SLOT_BLUE)
    game.emu.step(40)
    shot = game.shot("beatspot-mid-beat")
    watcher.watch(900, step=4)
    # What can hold for Blue (cocky) dwelling at a water's edge facing water,
    # the player 5+ tiles away, alone on the map.
    allowed = {"arrive_look", "leave_turn", "water_bite", "water_wait", "swagger", "blocked_sigh"}
    started = [e["name"] for e in watcher.starts(SLOT_BLUE)]
    result = {"water_beat": water, "facing_at_start": facing_at_start, "actor": actor,
              "started": started, "events": watcher.events, "counters": watcher.counters,
              "unexpected": sorted(set(started) - allowed), "screenshots": [shot], "debug": game.walker_debug()}
    result["pass"] = (water is not None and facing_at_start == DIR_NORTH and not result["unexpected"]
                      and actor is not None and (actor["x"], actor["y"]) == (13, 39))
    return result


def scenario_arrive(game: Game) -> dict:
    """The quiet gap starts passed and transition beats ignore it: Blue comes
    out of the Viridian Mart door and spawns a few tiles from a square spot
    (39, 30); he plays arrive_look as he reaches it, within seconds of
    spawning. The player stands out of notice range, so no react beat
    outranks it."""
    boot(game, VIRIDIAN, 34, 33, DIR_EAST)
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_MART_SQUARE, state=STATE_TRAVELLING,
               arrival=ARRIVAL_DOOR, crossing=3, activity=ACTIVITY_RELAX, dwell=6)
    game.warp(VIRIDIAN, 34, 33, DIR_EAST)
    spawned = game.wait_for(lambda: game.actor_for(SLOT_BLUE), 200, what="Blue's actor")
    spawn_tile = (spawned["x"], spawned["y"])
    spawn_frame = game.walker_debug()["frames"]
    watcher = BeatWatcher(game)
    arrived_frame = None
    for _ in range(0, 1200, 2):
        game.emu.step(2)
        watcher.poll()
        actor = game.actor_for(SLOT_BLUE)
        if actor and actor["atSpot"]:
            arrived_frame = game.walker_debug()["frames"]
            break
    # The arrival beat starts in the arrival's frame (maybe already polled above).
    watcher.watch(30, step=2)
    shot = game.shot("arrive-look")
    watcher.watch(300, step=4)
    starts = watcher.starts(SLOT_BLUE)
    arrive = next((e for e in starts if e["name"] == "arrive_look"), None)
    distance = abs(spawn_tile[0] - 39) + abs(spawn_tile[1] - 30)
    result = {"spawn_tile": spawn_tile, "spawn_distance": distance,
              "arrival_frames_after_spawn": None if arrived_frame is None else arrived_frame - spawn_frame,
              "arrive_look": arrive, "started": [e["name"] for e in starts],
              "events": watcher.events, "counters": watcher.counters, "player": game.player(),
              "screenshots": [shot], "debug": game.walker_debug()}
    result["pass"] = (2 <= distance <= 5 and arrived_frame is not None and arrive is not None
                      and starts and starts[0]["name"] == "arrive_look"
                      and arrive["frame"] - arrived_frame <= 2)
    return result


def approach(game: Game, watcher: BeatWatcher, y: int, hold: int = 300) -> None:
    """Walk the player along x = 13 to row y, then stand there watching."""
    here = game.player()["y"]
    if here != y:
        game.emu.hold("Up" if y < here else "Down", 1)
        try:
            for _ in range(0, 240, 2):
                game.emu.step(2)
                watcher.poll()
                if game.player()["y"] == y:
                    break
        finally:
            game.emu.hold("Up" if y < here else "Down", 0)
    watcher.watch(hold, step=4)


def scenario_notice(game: Game) -> dict:
    """Acceptance 3: notice_player fires once as the player comes within 3
    tiles of Blue (not stoic) and stays; leaving and coming back (after its
    cooldown) fires it again. Lance (stoic) at the same spot stares instead."""
    boot(game, VIRIDIAN, 13, 43, DIR_NORTH)
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_WATER, state=STATE_DWELLING,
               activity=ACTIVITY_FISH, dwell=60)
    game.warp(VIRIDIAN, 13, 43, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("atSpot"), 600, what="Blue at the pond")
    watcher = BeatWatcher(game)
    watcher.watch(120, step=4)
    phases = {}
    approach(game, watcher, 42, hold=400)                    # 3 tiles: in range, standing
    phases["first"] = len(watcher.starts(SLOT_BLUE, "notice_player"))
    shot = game.shot("notice-blue")
    approach(game, watcher, 43, hold=60)                     # 4 tiles: out of range
    watcher.watch(1300, step=8)                              # notice_player's cooldown (80 ticks)
    phases["away"] = len(watcher.starts(SLOT_BLUE, "notice_player"))
    approach(game, watcher, 42, hold=400)
    phases["second"] = len(watcher.starts(SLOT_BLUE, "notice_player"))
    blue_events = list(watcher.events)

    # Lance, stoic, at the same spot; Blue elsewhere.
    place_blue(game, node=NODE_MART, destId=SPOT_MART_SHELF, state=STATE_DWELLING, activity=ACTIVITY_SHOP, dwell=60)
    place_dwelling(game, SLOT_LANCE, SPOT_VIRIDIAN_WATER, NODE_VIRIDIAN)
    game.warp(VIRIDIAN, 13, 43, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_LANCE) or {}).get("atSpot"), 600, what="Lance at the pond")
    lance = BeatWatcher(game)
    lance.watch(120, step=4)
    approach(game, lance, 42, hold=400)
    shot_lance = game.shot("notice-lance-stare")
    result = {"blue_notice_counts": phases, "blue_events": blue_events, "lance_events": lance.events,
              "lance_stare": len(lance.starts(SLOT_LANCE, "stare_down")),
              "lance_notice": len(lance.starts(SLOT_LANCE, "notice_player")),
              "blue_stare": len(watcher.starts(SLOT_BLUE, "stare_down")),
              "screenshots": [shot, shot_lance], "debug": game.walker_debug()}
    result["pass"] = (phases["first"] == 1 and phases["away"] == 1 and phases["second"] == 2
                      and result["blue_stare"] == 0 and result["lance_stare"] == 1 and result["lance_notice"] == 0)
    return result


def scenario_greet(game: Game) -> dict:
    """Acceptance 4: Brock and Misty (friends) dwelling side by side at
    Viridian's pond greet each other with greet_friend, once each this visit."""
    game.emu.step(240)
    arrange_with_badges(game, VIRIDIAN, 16, 42, 8)   # Kanto badges: the leaders are out
    for slot in range(25):
        if slot not in (SLOT_BROCK, SLOT_MISTY):
            game.write_record(slot, **plain_record(node=NODE_MART, destId=SPOT_MART_SHELF, state=STATE_DWELLING,
                                                   arrival=ARRIVAL_NONE, crossing=0, activity=ACTIVITY_SHOP, dwell=60))
    place_dwelling(game, SLOT_BROCK, SPOT_VIRIDIAN_WATER, NODE_VIRIDIAN)
    place_dwelling(game, SLOT_MISTY, SPOT_VIRIDIAN_WATER_2, NODE_VIRIDIAN)
    game.warp(VIRIDIAN, 16, 42, DIR_NORTH)
    watcher = BeatWatcher(game)
    greeted = watcher.watch(900, step=2, until=lambda e: e["event"] == "start" and e["name"].startswith("greet"))
    game.emu.step(20)
    shot = game.shot("greet-friends")
    watcher.watch(1500, step=4)
    greetings = [(e["slot"], e["name"]) for e in watcher.starts() if e["name"] in
                 ("greet_friend", "greet_colleague", "greet_family", "size_up")]
    result = {"greetings": greetings, "events": watcher.events, "counters": watcher.counters,
              "actors": [game.actor_for(SLOT_BROCK), game.actor_for(SLOT_MISTY)],
              "screenshots": [shot], "debug": game.walker_debug()}
    result["pass"] = (greeted is not None
                      and sorted(greetings) == [(SLOT_BROCK, "greet_friend"), (SLOT_MISTY, "greet_friend")])
    return result


def scenario_beatpush(game: Game) -> dict:
    """Acceptance 6: Lance stares the player down (a long beat); the player
    presses into him: the beat stops at once, his facing comes back, and he
    then makes room as before."""
    boot(game, VIRIDIAN, 13, 43, DIR_NORTH)
    place_dwelling(game, SLOT_LANCE, SPOT_VIRIDIAN_WATER, NODE_VIRIDIAN)
    game.warp(VIRIDIAN, 13, 43, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_LANCE) or {}).get("atSpot"), 600, what="Lance at the pond")
    watcher = BeatWatcher(game)
    watcher.watch(200, step=4)
    before = game.walker_debug()
    start_facing = (object_of(game, SLOT_LANCE) or {}).get("facing")
    # Into range: he starts staring; walk on up beside him and press into him.
    stare = None
    game.emu.hold("Up", 1)
    try:
        for _ in range(0, 120):
            game.emu.step(1)
            for e in watcher.poll():
                if e["event"] == "start" and e["name"] == "stare_down":
                    stare = e
            if game.player()["y"] == 40:
                break
        pressing_from = game.walker_debug()["frames"]
        mid_beat = watcher.running(game.actor_for(SLOT_LANCE)["index"])
        shot = game.shot("beatpush-pressing")
        interrupt, facing_after = None, None
        for _ in range(0, 30):
            game.emu.step(1)
            for e in watcher.poll():
                if e["event"] == "interrupt" and e["slot"] == SLOT_LANCE:
                    interrupt = e
            if interrupt:
                break
        game.emu.step(6)
        facing_after = (object_of(game, SLOT_LANCE) or {}).get("facing")
        # Keep pressing: the yield rule as before.
        game.emu.step(80)
    finally:
        game.emu.hold("Up", 0)
    watcher.watch(120, step=4)
    after = game.walker_debug()
    result = {"stare": stare, "running_when_pressing": mid_beat, "pressing_from_frame": pressing_from,
              "interrupt": interrupt, "interrupt_delay": (interrupt["frame"] - pressing_from) if interrupt else None,
              "start_facing": start_facing, "facing_after_interrupt": facing_after,
              "back_offs": after["backOffs"] - before["backOffs"], "handoffs": after["handoffs"] - before["handoffs"],
              "lance_after": game.actor_for(SLOT_LANCE), "events": watcher.events, "counters": watcher.counters,
              "screenshots": [shot, game.shot("beatpush-after")], "debug": after}
    result["pass"] = (stare is not None and mid_beat == "stare_down" and interrupt is not None
                      and result["interrupt_delay"] <= 4 and facing_after == start_facing == DIR_NORTH
                      and (result["back_offs"] >= 1 or result["handoffs"] >= 1))
    return result


def scripted_beats(game: Game) -> dict:
    """A fixed input script (Blue at the pond, the player passing by): the
    beat log with frames relative to its start, and the RNG state after."""
    boot(game, VIRIDIAN, 13, 43, DIR_NORTH)
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_WATER, state=STATE_DWELLING,
               activity=ACTIVITY_FISH, dwell=60)
    game.warp(VIRIDIAN, 13, 43, DIR_NORTH)
    origin = game.walker_debug()["frames"]
    watcher = BeatWatcher(game)
    watcher.watch(1200, step=4)
    approach(game, watcher, 42, hold=400)
    approach(game, watcher, 43, hold=900)
    rng = game.emu.u32(game.sym["gRngValue"]) if "gRngValue" in game.sym else None
    return {"events": [(e["frame"] - origin, e["slot"], e["name"], e["event"]) for e in watcher.events],
            "lost": watcher.lost, "rng": rng, "frames": game.walker_debug()["frames"] - origin}


def scenario_determinism(game: Game) -> dict:
    """Acceptance 7: the same inputs give the same beats on the same frames,
    in two runs from boot (two SkyEmu sessions)."""
    first = scripted_beats(game)
    with tempfile.TemporaryDirectory(prefix="walkers-second-") as tmp:
        tmp = Path(tmp)
        rom = tmp / "rom.gba"
        shutil.copyfile(game.rom_path, rom)
        with skyemu_session(game.skyemu_binary, rom, tmp / "xdg", tmp / "skyemu.log") as emu:
            second = scripted_beats(Game(emu, game.sym, game.output))
    result = {"first": first, "second": second, "identical": first["events"] == second["events"],
              "rng_identical": first["rng"] == second["rng"]}
    # The RNG note is reported, not gated: the engine itself (field effects,
    # follower, weather) may advance it; the beats never read it.
    result["pass"] = result["identical"] and len(first["events"]) >= 2 and first["lost"] == 0
    return result


# ---------------------------------------------------------------------------
# The ace companion (the "Companion" section of src/wayfarer_walkers.c): one
# object with the dynamic local id 0xF9 and an overworld follower sprite
# (OBJ_EVENT_MON + species), out only while its beat runs.

COMPANION_LOCALID = 0xF9
FOLLOWER_LOCALID = 0xFE
OBJ_EVENT_MON = 1 << 14
DX = {DIR_SOUTH: (0, 1), DIR_NORTH: (0, -1), DIR_WEST: (-1, 0), DIR_EAST: (1, 0)}
ACTIVITY_RELAX = 4
ARRIVAL_WEST = 4


def species_ids() -> dict:
    import re
    text = (ROOT / "game/include/constants/species.h").read_text()
    return {m[0]: int(m[1]) for m in re.findall(r"#define (SPECIES_\w+)\s+(\d+)\b", text)}


def first_companion(slot: int) -> int:
    """The species id of the trainer's first companion candidate (all are 32x32 here)."""
    report = json.loads((ROOT / "game/build/wayfarer-world-report.json").read_text())
    return species_ids()[report["ambience"]["trainers"][slot]["companions"][0]["species"]]


def companion_objects(game: Game) -> list:
    return [o for o in game.objects() if o["localId"] == COMPANION_LOCALID]


def follower_object(game: Game):
    return next((o for o in game.objects() if o["localId"] == FOLLOWER_LOCALID), None)


def direction_towards(x: int, y: int, tx: int, ty: int) -> int:
    dx, dy = tx - x, ty - y
    if abs(dy) >= abs(dx):
        return DIR_SOUTH if dy > 0 else DIR_NORTH
    return DIR_EAST if dx > 0 else DIR_WEST


def arrange_party(game: Game, map_id: int, x: int, y: int, facing: int, party: list) -> None:
    """Arrange with a party (species ids, level 20): the first one follows the player."""
    game.request_id += 1
    req = bytearray(game.abi["requestSize"])
    struct.pack_into("<IHHhhI", req, 0, game.request_id, map_id >> 8, map_id & 0xFF, x, y, 1)
    req[80], req[81], req[84], req[85], req[86] = CHECKPOINT_NEW_BARK_AFTER_INTRO, facing, 0xFF, 1, CMD_ARRANGE
    for i, species in enumerate(party):  # struct E2ETestPartyMonFixture party[6] at offset 88, 16 bytes each
        struct.pack_into("<H4HBB", req, 88 + 16 * i, species, 0, 0, 0, 0, 20, 0)
    req[356] = len(party)               # partyCount
    submit_arrange(game, req, "arrange with a party")


class CompanionWatch:
    """Samples the companion, the walker, the player and the follower every few frames."""

    def __init__(self, game: Game, watcher: BeatWatcher, slot: int):
        self.game, self.watcher, self.slot = game, watcher, slot
        self.samples = []
        self.problems = []
        self.seen = False
        self.max_companions = 0
        self.follower_missing = 0
        self.follower_hidden_flag = 0

    def sample(self, expect_follower: bool) -> dict:
        objects = self.game.objects()
        companions = [o for o in objects if o["localId"] == COMPANION_LOCALID]
        player = next(o for o in objects if o["isPlayer"])
        follower = next((o for o in objects if o["localId"] == FOLLOWER_LOCALID), None)
        walker = object_of(self.game, self.slot)
        debug = self.game.walker_debug()
        self.max_companions = max(self.max_companions, len(companions))
        if expect_follower and (follower is None or follower["invisible"]):
            self.follower_missing += 1
        self.follower_hidden_flag = max(self.follower_hidden_flag, debug["followerHidden"])
        sample = {"frame": debug["frames"], "companions": companions, "walker": walker,
                  "player": (player["x"], player["y"], player["facing"]),
                  "running": self.watcher.running(self.game.actor_for(self.slot)["index"]) if self.game.actor_for(self.slot) else None}
        if companions:
            self.seen = True
        self.samples.append(sample)
        return sample

    def run(self, frames: int, step: int = 2, expect_follower: bool = False, until=None):
        for _ in range(0, frames, step):
            self.game.emu.step(step)
            new = self.watcher.poll()
            sample = self.sample(expect_follower)
            if until and until(new, sample):
                return sample
        return None


def check_placement(sample: dict, gfx: int) -> list:
    """What's wrong with the companion in this sample (empty when it stands right)."""
    problems = []
    companions, walker = sample["companions"], sample["walker"]
    if len(companions) != 1:
        return [f"{len(companions)} companion objects"]
    c = companions[0]
    if c["gfx"] != gfx:
        problems.append(f"gfx {c['gfx']:#x}, expected {gfx:#x}")
    if walker is None:
        return problems + ["no walker object"]
    if abs(c["x"] - walker["x"]) + abs(c["y"] - walker["y"]) != 1:
        problems.append(f"not beside the walker: {(c['x'], c['y'])} vs {(walker['x'], walker['y'])}")
    if c["facing"] != direction_towards(c["x"], c["y"], walker["x"], walker["y"]):
        problems.append(f"facing {c['facing']}, not the walker")
    return problems


def wait_ace(game: Game, watcher: BeatWatcher, slot: int, frames: int):
    return watcher.watch(frames, step=2, until=lambda e: e["event"] == "start" and e["slot"] == slot
                         and e["name"] in ("ace_play", "ace_spar"))


def scenario_companion(game: Game) -> dict:
    """Acceptance 5: Blue (cocky) dwelling at Viridian's pond, fishing, with
    every object slot and palette to spare, runs ace_play; while it runs his
    first ace (Umbreon) stands beside him, facing him, never on or straight
    ahead of the player; it is gone when the beat ends. The player's following
    Pokemon stays out throughout."""
    species = species_ids()
    expected_gfx = OBJ_EVENT_MON + first_companion(SLOT_BLUE)
    game.emu.step(240)
    arrange_party(game, VIRIDIAN, 15, 42, DIR_NORTH, [species["SPECIES_PIKACHU"]])
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_WATER, state=STATE_DWELLING,
               activity=ACTIVITY_FISH, dwell=60)
    game.warp(VIRIDIAN, 15, 42, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("atSpot"), 600, what="Blue at the pond")
    game.walk("Up", "y", 41)    # the follower comes out of its ball once the player moves
    follower_at_start = follower_object(game)
    watcher = BeatWatcher(game)
    watch = CompanionWatch(game, watcher, SLOT_BLUE)
    game.reset_frame_maxima()
    start = None
    for _ in range(0, 9000, 2):
        game.emu.step(2)
        for e in watcher.poll():
            if e["event"] == "start" and e["slot"] == SLOT_BLUE and e["name"] in ("ace_play", "ace_spar"):
                start = e
        watch.sample(expect_follower=True)
        if start:
            break
        if companion_objects(game):
            watch.problems.append("a companion came out without an ace beat")
    # The beat runs: sample until it ends.
    placement = []
    shot = None
    end = None
    out_at = None
    if start:
        for _ in range(0, 600, 2):
            game.emu.step(2)
            new = watcher.poll()
            sample = watch.sample(expect_follower=True)
            if sample["companions"]:
                if out_at is None:
                    out_at = sample
                    c = sample["companions"][0]
                    px, py, pf = sample["player"]
                    ahead = (px + DX.get(pf, (0, 0))[0], py + DX.get(pf, (0, 0))[1])
                    if (c["x"], c["y"]) in ((px, py), ahead):
                        watch.problems.append(f"on or ahead of the player: {(c['x'], c['y'])}")
                placement += check_placement(sample, expected_gfx)
                if shot is None and sample["frame"] - out_at["frame"] >= 40:
                    shot = game.shot("companion-out")
                    sample["walker_faces_companion"] = sample["walker"]["facing"] == direction_towards(
                        sample["walker"]["x"], sample["walker"]["y"], c["x"], c["y"])
                    watch.mid = sample
            end = next((e for e in new if e["slot"] == SLOT_BLUE and e["event"] in ("end", "interrupt")), None)
            if end:
                break
    game.emu.step(2)
    after = companion_objects(game)
    watcher.watch(240, step=4)
    gone_later = not companion_objects(game)
    debug = game.walker_debug()
    mid = getattr(watch, "mid", None)
    result = {"ace": start, "end": end, "companion_first_seen": out_at, "mid_beat": mid,
              "expected_gfx": expected_gfx, "placement_problems": sorted(set(placement)), "problems": watch.problems,
              "companions_after_end": after, "gone_later": gone_later,
              "follower_at_start": follower_at_start, "follower_missing_samples": watch.follower_missing,
              "follower_hidden_flag": watch.follower_hidden_flag, "counters": watcher.counters,
              "max_update_scanlines": debug["maxUpdateScanlines"], "events": watcher.events,
              "screenshots": [shot] if shot else [], "debug": debug}
    result["pass"] = bool(start and end and end["event"] == "end" and out_at and not placement and not watch.problems
                          and mid and mid.get("walker_faces_companion") and not after and gone_later
                          and follower_at_start and not follower_at_start["invisible"]
                          and watch.follower_missing == 0 and watch.follower_hidden_flag == 0
                          and watcher.counters["companion"]["out"] == 1 and watcher.counters["companion"]["in"] == 1)
    return result


def gate_ids():
    gate = TABLES.map_id("MAP_SAFARI_ZONE_GATE_HNS")
    spot, node = TABLES.spot("MAP_SAFARI_ZONE_GATE_HNS", "NPC_CHAT", 12, 24)   # beside the worker at (12, 23)
    return gate, spot, node


def scenario_companionslots(game: Game) -> dict:
    """Acceptance 5 (room): the Safari Zone Gate is crowded. With the player
    at (16, 16), its NPCs in view and Blue dwelling at a chat spot, exactly
    3 object slots are free (the walker rule's 3: Blue spawns at 4 free and
    takes one). A companion would leave 2, so companion_room never holds: no
    ace beat starts and no object with local id 0xF9 ever appears; the room
    checks fail on slots."""
    gate, spot, node = gate_ids()
    game.emu.step(240)
    game.arrange(gate, 16, 16, DIR_NORTH)
    place_blue(game, node=node, destId=spot, state=STATE_DWELLING, activity=ACTIVITY_RELAX, dwell=60)
    game.warp(gate, 16, 16, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("atSpot"), 900, what="Blue at the chat spot")
    watcher = BeatWatcher(game)
    watch = CompanionWatch(game, watcher, SLOT_BLUE)
    game.reset_frame_maxima()
    min_free = OBJECT_COUNT = 16
    for _ in range(0, 6000, 4):
        game.emu.step(4)
        watcher.poll()
        sample = watch.sample(expect_follower=False)
        min_free = min(min_free, OBJECT_COUNT - len(game.objects()))
    shot = game.shot("companionslots")
    aces = [e for e in watcher.starts(SLOT_BLUE) if e["name"] in ("ace_play", "ace_spar")]
    debug = game.walker_debug()
    companion = watcher.counters["companion"]
    result = {"free_slots_min": min_free, "free_slots_end": OBJECT_COUNT - len(game.objects()),
              "objects": game.objects(), "ace_starts": aces, "companion_seen": watch.seen,
              "beats": [e["name"] for e in watcher.starts(SLOT_BLUE)], "counters": watcher.counters,
              "max_update_scanlines": debug["maxUpdateScanlines"], "screenshots": [shot], "debug": debug}
    result["pass"] = (result["free_slots_end"] == 3 and min_free >= 3 and not aces and not watch.seen
                      and companion["out"] == 0 and companion["denied"]["slots"] >= 1
                      and len(result["beats"]) >= 2)
    return result


def scenario_companionfollower(game: Game) -> dict:
    """Acceptance 5 (follower): at the Safari Zone Gate with the player at
    (16, 20) and a following Pokemon out, 10 objects are active besides the
    follower and the walkers (the player and 9 NPCs). Blue dwells at a chat
    spot, and two more trainers travel in from the west, where the map's
    side is solid: they stay spawn candidates that can't spawn. The follower
    rule counts them: 16 - 10 - 3 walkers - 1 follower = 2 free, just enough
    to keep the follower out, while 4 slots are really free. A companion
    would leave the rule 1, so it stays away and the follower stays out
    (followerHidden 0, the follower object active). Control: once the two
    travellers are elsewhere, the companion comes out at the same spot."""
    species = species_ids()
    gate, spot, node = gate_ids()
    game.emu.step(240)
    arrange_party(game, gate, 16, 21, DIR_NORTH, [species["SPECIES_PIKACHU"]])
    place_blue(game, node=node, destId=spot, state=STATE_DWELLING, activity=ACTIVITY_RELAX, dwell=60)
    game.warp(gate, 16, 21, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("atSpot"), 900, what="Blue at the chat spot")
    game.walk("Up", "y", 20)    # the follower comes out of its ball once the player moves
    travellers = (SLOT_LORELEI, SLOT_LANCE)
    for slot in travellers:
        game.write_record(slot, **plain_record(node=node, destId=spot, state=STATE_TRAVELLING, arrival=ARRIVAL_WEST,
                                               crossing=10, activity=ACTIVITY_RELAX, dwell=3))
    watcher = BeatWatcher(game)
    watch = CompanionWatch(game, watcher, SLOT_BLUE)
    game.reset_frame_maxima()
    watch.run(6000, step=4, expect_follower=True)
    objects = game.objects()
    # The follower rule's "others": every object but the follower, the walkers and the companion.
    others = [o for o in objects if o["localId"] not in (FOLLOWER_LOCALID, COMPANION_LOCALID)
              and not 0xF5 <= o["localId"] < 0xF5 + ACTOR_COUNT]
    phase1 = {"companion_seen": watch.seen, "ace_starts": [e for e in watcher.starts(SLOT_BLUE)
                                                           if e["name"] in ("ace_play", "ace_spar")],
              "travellers_spawned": [game.actor_for(s) is not None for s in travellers],
              "others": len(others), "free_slots": 16 - len(objects), "follower": follower_object(game),
              "follower_missing_samples": watch.follower_missing, "follower_hidden_flag": watch.follower_hidden_flag,
              "counters": json.loads(json.dumps(watcher.counters))}
    shot = game.shot("companionfollower-held-back")

    # Control: the travellers go elsewhere; the rule has room again.
    for slot in travellers:
        game.write_record(slot, **plain_record(node=NODE_MART, destId=SPOT_MART_SHELF, state=STATE_DWELLING,
                                               arrival=ARRIVAL_NONE, crossing=0, activity=ACTIVITY_SHOP, dwell=60))
    control = CompanionWatch(game, watcher, SLOT_BLUE)
    control.run(9000, step=4, expect_follower=True, until=lambda new, sample: bool(sample["companions"]))
    shot_control = game.shot("companionfollower-control")
    control.run(400, step=4, expect_follower=True)
    debug = game.walker_debug()
    result = {"phase1": phase1, "control_companion_seen": control.seen,
              "control_follower_missing_samples": control.follower_missing,
              "control_follower_hidden_flag": control.follower_hidden_flag,
              "counters": watcher.counters, "max_update_scanlines": debug["maxUpdateScanlines"],
              "screenshots": [shot, shot_control], "debug": debug}
    c1 = phase1["counters"]["companion"]
    result["pass"] = (not phase1["companion_seen"] and not phase1["ace_starts"] and not any(phase1["travellers_spawned"])
                      and phase1["others"] == 10 and phase1["free_slots"] >= 4
                      and phase1["follower"] is not None and not phase1["follower"]["invisible"]
                      and phase1["follower_missing_samples"] == 0 and phase1["follower_hidden_flag"] == 0
                      and c1["out"] == 0 and c1["denied"]["follower"] >= 1 and c1["denied"]["slots"] == 0
                      and control.seen and control.follower_missing == 0 and control.follower_hidden_flag == 0)
    return result


def scenario_companionpush(game: Game) -> dict:
    """Acceptance 5 and 6: Blue's ace_play is under way with his companion out;
    the player steps up and presses into him. The beat is interrupted and the
    companion removed within a few frames of the press."""
    boot(game, VIRIDIAN, 13, 41, DIR_NORTH)
    place_blue(game, node=NODE_VIRIDIAN, destId=SPOT_VIRIDIAN_WATER, state=STATE_DWELLING,
               activity=ACTIVITY_FISH, dwell=60)
    game.warp(VIRIDIAN, 13, 41, DIR_NORTH)
    game.wait_for(lambda: (game.actor_for(SLOT_BLUE) or {}).get("atSpot"), 600, what="Blue at the pond")
    watcher = BeatWatcher(game)
    game.reset_frame_maxima()
    start = wait_ace(game, watcher, SLOT_BLUE, 9000)
    out = game.wait_for(lambda: companion_objects(game), 60, step=1, what="the companion") if start else None
    pressing_from = interrupt = gone_at = None
    shot = shot_before = None
    if out:
        # Out for a moment (its jump), then the player steps up and presses.
        game.emu.step(30)
        watcher.poll()
        shot_before = game.shot("companionpush-before")
        out = companion_objects(game)
        game.emu.hold("Up", 1)
        try:
            for _ in range(0, 120):
                game.emu.step(1)
                watcher.poll()
                player = game.player()
                if (player["x"], player["y"]) == (13, 40):
                    pressing_from = game.walker_debug()["frames"]
                    break
            shot = game.shot("companionpush-pressing")
            for _ in range(0, 30):
                frame = game.walker_debug()["frames"]
                for e in watcher.poll():
                    if e["event"] == "interrupt" and e["slot"] == SLOT_BLUE:
                        interrupt = e
                if gone_at is None and not companion_objects(game):
                    gone_at = frame
                if interrupt and gone_at is not None:
                    break
                game.emu.step(1)
        finally:
            game.emu.hold("Up", 0)
    watcher.watch(120, step=4)
    debug = game.walker_debug()
    result = {"ace": start, "companion": out, "pressing_from_frame": pressing_from, "interrupt": interrupt,
              "gone_at_frame": gone_at,
              "interrupt_delay": (interrupt["frame"] - pressing_from) if interrupt and pressing_from else None,
              "gone_delay": (gone_at - pressing_from) if gone_at is not None and pressing_from else None,
              "companions_after": companion_objects(game), "counters": watcher.counters,
              "max_update_scanlines": debug["maxUpdateScanlines"], "events": watcher.events,
              "screenshots": [x for x in (shot_before, shot) if x], "debug": debug}
    result["pass"] = (start is not None and out and interrupt is not None and interrupt["beat"] == start["beat"]
                      and result["interrupt_delay"] is not None and result["interrupt_delay"] <= 4
                      and result["gone_delay"] is not None and result["gone_delay"] <= 4
                      and not result["companions_after"]
                      and watcher.counters["companion"]["away"]["interrupted"] >= 1)
    return result


SCENARIOS = {"spot": scenario_spot, "bridge": scenario_bridge, "walkoff": scenario_walkoff, "mortar": scenario_mortar,
             "safari": scenario_safari, "edge": scenario_edge, "door": scenario_door,
             "linger": scenario_linger, "save": scenario_save, "gym": scenario_gym,
             "budget": scenario_budget, "browse": scenario_browse, "deadend": scenario_deadend,
             "gymentry": scenario_gymentry, "stairs": scenario_stairs, "midstep": scenario_midstep,
             "decoys": scenario_decoys, "perf": scenario_perf, "seam": scenario_seam,
             "recross": scenario_recross, "longtrip": scenario_longtrip, "striplane": scenario_striplane,
             "beatspot": scenario_beatspot, "arrive": scenario_arrive, "notice": scenario_notice, "greet": scenario_greet,
             "beatpush": scenario_beatpush, "determinism": scenario_determinism,
             "companion": scenario_companion, "companionslots": scenario_companionslots,
             "companionfollower": scenario_companionfollower, "companionpush": scenario_companionpush}


def check_content_hash(rom: Path, symbols: dict, tables) -> None:
    """T4: node and spot ids come from the worktree's tables.h, so the ROM
    must have been built from the same tables (the same content hash)."""
    address = symbols.get("gWayfarerWorldContentHash")
    if address is None or not 0x08000000 <= address < 0x0A000000:
        raise SystemExit("verify.py: the ROM's symbols have no gWayfarerWorldContentHash in ROM")
    with rom.open("rb") as handle:
        handle.seek(address - 0x08000000)
        value = struct.unpack("<H", handle.read(2))[0]
    if value != tables.content_hash:
        raise SystemExit(f"verify.py: the ROM's world content hash {value:#06x} differs from the worktree's "
                         f"tables.h ({tables.content_hash:#06x}); rebuild the ROM or regenerate the tables")


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--rom", type=Path, required=True)
    parser.add_argument("--symbols", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=ROOT / ".product/research/overworld-walkers")
    parser.add_argument("--skyemu", type=Path, default=DEFAULT_SKYEMU)
    parser.add_argument("--scenario", choices=sorted(SCENARIOS) + ["all"], default="all")
    args = parser.parse_args(argv)
    args.output.mkdir(parents=True, exist_ok=True)
    symbols = read_symbols(args.symbols)
    check_content_hash(args.rom, symbols, load_world_ids(ROOT / "game"))
    names = sorted(SCENARIOS) if args.scenario == "all" else [args.scenario]
    results = {}
    failed = False
    for name in names:
        with tempfile.TemporaryDirectory(prefix="walkers-") as tmp:
            tmp = Path(tmp)
            rom = tmp / "rom.gba"
            shutil.copyfile(args.rom, rom)
            print(f"== {name}", flush=True)
            try:
                with skyemu_session(args.skyemu, rom, tmp / "xdg", tmp / "skyemu.log") as emu:
                    game = Game(emu, symbols, args.output)
                    game.skyemu_binary, game.rom_path = args.skyemu, args.rom
                    try:
                        result = SCENARIOS[name](game)
                    except Exception as error:  # report the state and keep going
                        result = {"pass": False, "error": f"{type(error).__name__}: {error}"}
                        with contextlib.suppress(Exception):
                            result["failure_debug"] = game.walker_debug()
                            result["failure_objects"] = game.objects()
                            result["failure_blue"] = game.record(SLOT_BLUE)
                            result["failure_locked"] = game.controls_locked()
                            result["failure_screenshot"] = game.shot(f"{name}-failure")
                    result["emulated_frames"] = emu.frames
            except Exception as error:
                result = {"pass": False, "error": f"{type(error).__name__}: {error}"}
            results[name] = result
            failed |= not result.get("pass")
            if not result.get("pass") and (tmp / "skyemu.log").exists():
                shutil.copyfile(tmp / "skyemu.log", args.output / f"{name}-skyemu.log")
            print(f"   {'PASS' if result.get('pass') else 'FAIL'} {result.get('error', '')}", flush=True)
            (args.output / f"{name}.json").write_text(json.dumps(result, indent=2, default=str) + "\n")
    summary = {name: results[name].get("pass") for name in names}
    (args.output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps(summary))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Build a native mGBA runner and boot an isolated copy of a ROM."""
import argparse
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("elf", type=Path, help="matching unstripped ELF from the same build")
    parser.add_argument("--output", type=Path, default=Path("mgba-smoke-results"))
    parser.add_argument("--frames", type=int, default=3600)
    parser.add_argument("--timeout", type=int, default=120, help="wall-clock seconds")
    args = parser.parse_args()
    if args.frames <= 0 or args.timeout <= 0:
        parser.error("frames and timeout must be positive")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    (output / "screen.png").unlink(missing_ok=True)
    symbols = {}
    nm = subprocess.check_output(
        [os.environ.get("NM", "arm-none-eabi-nm"), "--defined-only", str(args.elf.resolve())],
        text=True,
    )
    for line in nm.splitlines():
        parts = line.split()
        if len(parts) == 3:
            symbols[parts[2]] = int(parts[0], 16)
    names = ("gMain", "gTasks", "CB2_MainMenu", "Task_HandleMainMenuInput", "gFlashMemoryPresent")
    missing = set(names) - symbols.keys()
    if missing:
        parser.error(f"ELF missing symbols: {', '.join(sorted(missing))}")
    (output / "symbols.txt").write_text("".join(f"{name}=0x{symbols[name]:08x}\n" for name in names))
    with tempfile.TemporaryDirectory(prefix="wayfarer-mgba-smoke-") as temporary:
        directory = Path(temporary)
        runner = directory / "smoke"
        subprocess.run(
            shlex.split(os.environ.get("CC", "cc"))
            + ["-std=gnu11", "-Wall", "-Wextra", "-Werror", "-O2"]
            + shlex.split(os.environ.get("CFLAGS", ""))
            + [str(Path(__file__).with_name("smoke.c")), "-o", str(runner)]
            + shlex.split(os.environ.get("LDFLAGS", "")) + ["-lmgba"],
            check=True,
        )
        # Never open the original path: mGBA must not discover an adjacent save.
        shutil.copyfile(args.rom.resolve(), directory / "game.gba")
        environment = dict(os.environ, XDG_CONFIG_HOME=str(directory), XDG_DATA_HOME=str(directory))
        command = [str(runner), "game.gba", str(args.frames)] + [hex(symbols[name]) for name in names]
        status = 1
        with (output / "boot.log").open("w") as log:
            try:
                result = subprocess.run(command, cwd=directory, env=environment,
                                        stdout=log, stderr=subprocess.STDOUT, timeout=args.timeout)
                status = result.returncode
                if status < 0:
                    log.write(f"FAIL: mGBA terminated by signal {-status}\n")
            except subprocess.TimeoutExpired:
                log.write(f"FAIL: mGBA exceeded {args.timeout}s wall-clock timeout\n")
        screenshot = directory / "screen.png"
        if screenshot.exists():
            shutil.copyfile(screenshot, output / "screen.png")
        print((output / "boot.log").read_text(), end="")
        print(f"Diagnostics: {output}")
        return status


if __name__ == "__main__":
    raise SystemExit(main())

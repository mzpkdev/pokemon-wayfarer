#!/usr/bin/env python3
"""Audit stack adjustments and call-site depths in linked Thumb functions.

This is a local-frame audit, not a whole-program stack analyzer. Indirect callees
and interrupt overlays need a separate call-chain argument. Unsupported stack
instructions and inconsistent control-flow joins fail instead of being ignored.
"""

import argparse
import hashlib
import json
import re
import struct
import subprocess
import tempfile
from collections import deque
from pathlib import Path


def register_count(text):
    registers = re.search(r"\{([^}]+)\}", text)
    if not registers:
        raise ValueError(f"missing register list: {text}")
    total = 0
    for register in registers[1].split(","):
        register = register.strip()
        match = re.fullmatch(r"r(\d+)-r(\d+)", register)
        total += int(match[2]) - int(match[1]) + 1 if match else 1
    return total


def audit(text, read_word=None):
    instructions = {}
    for line in text.splitlines():
        match = re.match(r"\s*([0-9a-f]+):\s+(\S+)(?:\s+(.*))?$", line)
        if match:
            instructions[int(match[1], 16)] = (match[2], (match[3] or "").split("@")[0].strip())
    if not instructions:
        raise ValueError("no disassembled instructions")
    addresses = sorted(instructions)
    following = dict(zip(addresses, addresses[1:]))
    previous = dict(zip(addresses[1:], addresses))
    pending = deque([(addresses[0], 0)])
    depths, calls, tables, peak = {}, [], [], 0

    def jump_table(pc, destination_register):
        # GCC's ARM7 Thumb switch: bounded unsigned index, literal table base,
        # index*4, table load, then MOV pc. Any other computed transfer fails.
        window = []
        cursor = pc
        for _ in range(5):
            cursor = previous.get(cursor)
            if cursor is None:
                raise ValueError(f"unsupported computed PC transfer at {pc:#x}")
            window.append((cursor, *instructions[cursor]))
        load, shift, literal, guard, compare = window
        index = re.fullmatch(r"(r\d+),\s*#(\d+)", compare[2])
        scaled = re.fullmatch(r"(r\d+),\s*(r\d+),\s*#2", shift[2])
        base = re.fullmatch(r"(r\d+),\s*\[pc,\s*#(\d+)\]", literal[2])
        if (read_word is None or compare[1] != "cmp" or not index
                or guard[1].removesuffix(".n") != "bhi"
                or shift[1] != "lsls" or not scaled or scaled[2] != index[1]
                or scaled[1] != destination_register
                or literal[1] != "ldr" or not base
                or len({base[1], scaled[1], index[1]}) != 3
                or load[1] != "ldr"
                or re.sub(r"\s+", "", load[2]) != f"{destination_register},[{base[1]},{destination_register}]"):
            raise ValueError(f"unsupported computed PC transfer at {pc:#x}")
        count = int(index[2]) + 1
        if count > 256:
            raise ValueError(f"unsupported jump table size at {pc:#x}")
        literal_address = ((literal[0] + 4) & ~3) + int(base[2])
        table_address = read_word(literal_address)
        targets = [read_word(table_address + 4 * i) for i in range(count)]
        if any(target not in instructions for target in targets):
            raise ValueError(f"jump table exits decoded function at {pc:#x}")
        # No branch may bypass the guard and enter the table-dispatch sequence.
        interior = set(address for address in addresses if guard[0] <= address <= pc)
        for operation, operands in instructions.values():
            target = re.match(r"([0-9a-f]+)\s+<", operands)
            if operation.startswith("b") and target and int(target[1], 16) in interior:
                raise ValueError(f"branch bypasses jump-table guard at {pc:#x}")
        tables.append({"pc": hex(pc), "address": hex(table_address), "count": count,
                       "targets": [hex(target) for target in targets],
                       "protectedInterior": [hex(address) for address in sorted(interior)]})
        return targets
    while pending:
        pc, depth = pending.popleft()
        if pc in depths:
            if depths[pc] != depth:
                raise ValueError(f"inconsistent stack depth at {pc:#x}")
            continue
        if pc not in instructions:
            raise ValueError(f"branch leaves decoded function at {pc:#x}")
        depths[pc] = depth
        operation, operands = instructions[pc]
        operation = operation.removesuffix(".n").removesuffix(".w")
        if operation.startswith("."):
            raise ValueError(f"control flow reaches data at {pc:#x}")
        if operation.startswith(("push", "pop")) and operation not in ("push", "pop"):
            raise ValueError(f"unsupported conditional stack operation at {pc:#x}")
        if operation in ("cbz", "cbnz"):
            raise ValueError(f"unsupported Thumb-2 branch at {pc:#x}")
        if re.match(r"pc\s*,", operands):
            if operation != "mov":
                raise ValueError(f"unsupported write to PC at {pc:#x}")
            for destination in jump_table(pc, operands.split(",")[1].strip()):
                pending.append((destination, depth))
            continue
        if operation == "push":
            depth += 4 * register_count(operands)
        elif operation == "pop":
            depth -= 4 * register_count(operands)
        elif re.match(r"sp\s*,", operands) and operation in ("add", "sub"):
            immediate = re.fullmatch(r"sp\s*,\s*#(0x[0-9a-f]+|\d+)", operands)
            if not immediate:
                raise ValueError(f"unsupported stack adjustment at {pc:#x}")
            depth += (1 if operation == "sub" else -1) * int(immediate[1], 0)
        elif re.match(r"sp(?:\s*,|\!)", operands) and operation not in ("cmp",):
            raise ValueError(f"unsupported write to SP at {pc:#x}: {operation} {operands}")
        if depth < 0:
            raise ValueError(f"negative stack depth at {pc:#x}")
        peak = max(peak, depth)
        target = re.match(r"([0-9a-f]+)\s+<([^>]+)>", operands)
        if operation in ("bl", "blx"):
            calls.append({"pc": hex(pc), "depth": depth, "target": target[2] if target else operands})
        if operation == "bx" and operands != "lr":
            prior = instructions.get(previous.get(pc), ("", ""))
            if prior != ("pop", "{" + operands + "}"):
                raise ValueError(f"unresolved indirect tail at {pc:#x}")
        returns = operation == "bx" or (operation == "pop" and re.search(r"\bpc\b", operands))
        if returns:
            if depth:
                raise ValueError(f"return/tail call with live frame at {pc:#x}: {depth}")
            continue
        if operation.startswith("b") and operation not in ("bl", "blx", "bic", "bics"):
            if not target:
                raise ValueError(f"unresolved branch at {pc:#x}")
            destination = int(target[1], 16)
            if destination in instructions:
                pending.append((destination, depth))
            else:
                if depth:
                    raise ValueError(f"tail branch with live frame at {pc:#x}")
                calls.append({"pc": hex(pc), "depth": depth, "target": target[2], "tail": True})
            if operation == "b":
                continue
        if pc not in following:
            raise ValueError(f"function falls through at {pc:#x}")
        pending.append((following[pc], depth))
    for table in tables:
        for other in tables:
            if set(other["targets"]) & set(table["protectedInterior"]):
                raise ValueError(f"computed branch bypasses guard at {table['pc']}")
    return {"ownPeakBytes": peak, "reachableInstructions": len(depths),
            "jumpTables": tables, "calls": sorted(calls, key=lambda call: int(call["pc"], 16))}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", type=Path, required=True)
    parser.add_argument("--functions", nargs="+", required=True, help="exact symbols or unique symbol prefixes")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    symbols = subprocess.check_output(["arm-none-eabi-nm", "-S", "--defined-only", str(args.elf)], text=True)
    functions = [line.split()[-1] for line in symbols.splitlines() if re.match(r"[0-9a-f]+ [0-9a-f]+ [tT] ", line)]
    with tempfile.TemporaryDirectory(prefix="gameplay-stack-") as directory:
        binary = Path(directory) / "linked.gba"
        subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", str(args.elf), str(binary)], check=True)
        rom = binary.read_bytes()

    def read_word(address):
        offset = address - 0x08000000
        if offset < 0 or offset + 4 > len(rom):
            raise ValueError(f"jump-table read outside linked GBA ROM: {address:#x}")
        return struct.unpack_from("<I", rom, offset)[0]

    report = {"elfSha256": hashlib.sha256(args.elf.read_bytes()).hexdigest(), "scope": "local frames; calls recorded but not expanded", "functions": {}}
    for prefix in args.functions:
        matches = [name for name in functions if name == prefix or name.startswith(prefix + ".")]
        if len(matches) != 1:
            raise ValueError(f"expected one function for {prefix!r}, got {matches}")
        name = matches[0]
        disassembly = subprocess.check_output(["arm-none-eabi-objdump", "-d", "--no-show-raw-insn", "--disassemble=" + name, str(args.elf)], text=True)
        report["functions"][name] = audit(disassembly, read_word)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")


if __name__ == "__main__":
    main()

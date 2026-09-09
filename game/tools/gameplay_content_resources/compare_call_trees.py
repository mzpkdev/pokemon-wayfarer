#!/usr/bin/env python3
"""Compare linked direct-call closures; indirect callback roots must be supplied.

This reports control/stack changes and instruction differences for review. It is
not a whole-program call-graph proof: callers must account for computed branches,
registered callbacks, and referenced data separately.
"""

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess


def decode(disassembly):
    functions = {}
    for name, body in re.findall(
            r"^[0-9a-f]+ <([^>]+)>:\n(.*?)(?=^[0-9a-f]+ <|\Z)",
            disassembly, re.MULTILINE | re.DOTALL):
        instructions, targets, controls = [], set(), []
        for operation, operand in re.findall(
                r"^\s*[0-9a-f]+:\s+(\S+)\s*(.*)$", body, re.MULTILINE):
            operand = operand.split("@")[0].strip()
            branch = operation.startswith("b") and operation not in ("bic", "bics")
            if branch:
                target = re.search(r"<([^>+]+)(?:\+[^>]*)?>", operand)
                if target and target[1] != name:
                    targets.add(target[1])
            normalized = re.sub(r"\b[0-9a-f]+\s+<([^>]+)>", r"<\1>", operand)
            instruction = operation + " " + normalized
            if not operation.startswith("."):
                instructions.append(instruction)
            if (branch or operation.startswith(("push", "pop"))
                    or re.match(r"(?:sp|pc)(?:\s*,|!)", operand)
                    or re.match(r"(?:stm|ldm).*", operation)):
                controls.append(instruction)
        functions[name] = {"instructions": instructions, "controlAndStack": controls,
                           "directTargets": sorted(targets)}
    return functions


def closure(functions, roots):
    pending, selected = list(roots), {}
    while pending:
        name = pending.pop()
        if name in selected:
            continue
        if name not in functions:
            raise ValueError(f"missing direct target or root: {name}")
        selected[name] = functions[name]
        pending.extend(functions[name]["directTargets"])
    return selected


def compare(base, head):
    common = sorted(set(base) & set(head))
    return {
        "onlyBaseline": sorted(set(base) - set(head)),
        "onlyHead": sorted(set(head) - set(base)),
        "commonFunctionCount": len(common),
        "controlOrStackChanges": [name for name in common if
            base[name]["controlAndStack"] != head[name]["controlAndStack"] or
            base[name]["directTargets"] != head[name]["directTargets"]],
        "instructionChanges": {name: {"baseline": base[name]["instructions"],
                                      "head": head[name]["instructions"]}
                               for name in common if
                               base[name]["instructions"] != head[name]["instructions"]},
        "functions": {name: {"directTargets": base[name]["directTargets"],
                             "controlAndStack": base[name]["controlAndStack"]}
                      for name in common},
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base-elf", required=True, type=Path)
    parser.add_argument("--head-elf", required=True, type=Path)
    parser.add_argument("--roots", required=True, nargs="+")
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    pairs, hashes = {}, {}
    for side, path in (("baseline", args.base_elf), ("head", args.head_elf)):
        hashes[side] = hashlib.sha256(path.read_bytes()).hexdigest()
        disassembly = subprocess.check_output(
            ["arm-none-eabi-objdump", "-d", "--no-show-raw-insn", str(path)], text=True)
        pairs[side] = closure(decode(disassembly), args.roots)
    result = {"scope": "direct-call closure; explicit roots for indirect callbacks; data not compared",
              "elfSha256": hashes, "roots": args.roots,
              **compare(pairs["baseline"], pairs["head"])}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")


if __name__ == "__main__":
    main()

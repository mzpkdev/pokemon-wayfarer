#!/usr/bin/env python3

import argparse
import ast
import json
import operator
import re
import subprocess
import tempfile
from pathlib import Path


FLAG_LOW_START = 0x020
FLAG_LOW_END = 0x4FF
FLAG_HIGH_START = 0x860
FLAG_HIGH_END = 0x95F
VAR_START = 0x4000
VAR_END = 0x40FF
HOENN_FLAG_NAMESPACE = 0x6000
HOENN_VAR_NAMESPACE = 0x7000

MACRO_RE = re.compile(r"^#define\s+([A-Za-z_][A-Za-z0-9_]*)\s+(.+)$")
IDENT_RE = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
INTEGER_SUFFIX_RE = re.compile(r"(?<=\d)[uUlL]+\b")
PERSISTENT_TOKEN_RE = re.compile(r"\b(?:FLAG|VAR)_[A-Za-z0-9_]+\b")
STARTER_TOKEN_RE = re.compile(r"\b(?:VAR_HOENN_STARTER_CHOICE|VAR_STARTER_MON)\b")

BIN_OPS = {
    ast.Add: operator.add,
    ast.Sub: operator.sub,
    ast.Mult: operator.mul,
    ast.Div: operator.floordiv,
    ast.FloorDiv: operator.floordiv,
    ast.Mod: operator.mod,
    ast.LShift: operator.lshift,
    ast.RShift: operator.rshift,
    ast.BitOr: operator.or_,
    ast.BitAnd: operator.and_,
    ast.BitXor: operator.xor,
}
UNARY_OPS = {
    ast.UAdd: operator.pos,
    ast.USub: operator.neg,
    ast.Invert: operator.invert,
}


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cpp", required=True)
    parser.add_argument("--cc", default="cc")
    parser.add_argument("--include-dir", required=True)
    parser.add_argument("--hoenn-output", required=True)
    parser.add_argument("--engine-output", required=True)
    parser.add_argument("--common-output", required=True)
    parser.add_argument("--common-data-output", required=True)
    parser.add_argument("--event-scripts", required=True)
    parser.add_argument("--scripts-dir", required=True)
    parser.add_argument("--maps-dir", required=True)
    return parser.parse_args()


def dump_macros(cpp, include_dir, product_macro):
    source = (
        '#include "constants/global.h"\n'
        '#include "constants/flags.h"\n'
        '#include "constants/vars.h"\n'
    )
    command = [cpp, "-I", include_dir, f"-D{product_macro}", "-dM", "-"]
    result = subprocess.run(command, input=source, text=True, capture_output=True, check=True)
    macros = {}
    for line in result.stdout.splitlines():
        match = MACRO_RE.match(line)
        if match:
            macros[match.group(1)] = match.group(2).strip()
    return macros


def dump_values(cc, include_dir, product_macro, names):
    source = [
        "#include <stdio.h>",
        '#include "constants/global.h"',
        '#include "constants/flags.h"',
        '#include "constants/region_map_sections.h"',
        '#include "constants/vars.h"',
        "int main(void)",
        "{",
    ]
    for name in names:
        source.append(f'    printf("{name}=%llX\\n", (unsigned long long)({name}));')
    source.extend(("    return 0;", "}", ""))

    with tempfile.TemporaryDirectory() as temp_dir:
        executable = Path(temp_dir) / "dump_constants"
        command = [
            cc,
            "-x", "c",
            "-I", include_dir,
            f"-D{product_macro}",
            "-o", str(executable),
            "-",
        ]
        subprocess.run(command, input="\n".join(source), text=True, capture_output=True, check=True)
        result = subprocess.run([str(executable)], text=True, capture_output=True, check=True)

    values = {}
    for line in result.stdout.splitlines():
        name, value = line.split("=", 1)
        values[name] = int(value, 16)
    return values


def eval_ast(node):
    if isinstance(node, ast.Expression):
        return eval_ast(node.body)
    if isinstance(node, ast.Constant) and isinstance(node.value, int):
        return node.value
    if isinstance(node, ast.BinOp) and type(node.op) in BIN_OPS:
        return BIN_OPS[type(node.op)](eval_ast(node.left), eval_ast(node.right))
    if isinstance(node, ast.UnaryOp) and type(node.op) in UNARY_OPS:
        return UNARY_OPS[type(node.op)](eval_ast(node.operand))
    raise ValueError(f"unsupported expression node: {ast.dump(node)}")


def resolve_macro(name, macros, resolved, resolving):
    if name in resolved:
        return resolved[name]
    if name in resolving:
        raise ValueError(f"recursive macro definition involving {name}")
    if name not in macros:
        raise ValueError(f"undefined macro {name}")

    resolving.add(name)
    expression = macros[name]

    def replace_identifier(match):
        identifier = match.group(0)
        if identifier in macros:
            return str(resolve_macro(identifier, macros, resolved, resolving))
        if identifier == "TRUE":
            return "1"
        if identifier == "FALSE":
            return "0"
        raise ValueError(f"unsupported identifier {identifier} in {name}")

    expression = IDENT_RE.sub(replace_identifier, expression)
    expression = INTEGER_SUFFIX_RE.sub("", expression)
    value = eval_ast(ast.parse(expression, mode="eval"))
    resolving.remove(name)
    resolved[name] = value
    return value


def resolve_constants(macros):
    resolved = {}
    skipped = {}
    for name in sorted(macros):
        try:
            resolve_macro(name, macros, resolved, set())
        except (SyntaxError, ValueError, ZeroDivisionError) as error:
            skipped[name] = str(error)
    return resolved, skipped


def hoenn_value(name, source_value):
    if name.startswith("FLAG_"):
        if FLAG_LOW_START <= source_value <= FLAG_LOW_END or FLAG_HIGH_START <= source_value <= FLAG_HIGH_END:
            return HOENN_FLAG_NAMESPACE | source_value
    elif name.startswith("VAR_") and VAR_START <= source_value <= VAR_END:
        return HOENN_VAR_NAMESPACE | (source_value - VAR_START)
    return source_value


def render_table(source_name, values):
    lines = [
        "@",
        f"@ DO NOT MODIFY THIS FILE! It is auto-generated for {source_name} script operands.",
        "@",
        "",
    ]
    for name, value in sorted(values.items()):
        lines.append(f"#undef {name}")
        if value is not None:
            lines.append(f"#define {name} 0x{value:X}")
    lines.append("")
    return "\n".join(lines)


def collect_common_names(event_scripts, scripts_dir):
    # event_scripts.s contains the common inline scripts, while data/scripts is
    # the complete set of shared script bodies that it includes. Scanning the
    # superset is intentional: inactive product branches retain their engine
    # value and therefore do not alter standalone products.
    paths = [Path(event_scripts), *sorted(Path(scripts_dir).glob("*.inc"))]
    names = set()
    for path in paths:
        names.update(PERSISTENT_TOKEN_RE.findall(path.read_text()))
    return names


def collect_hoenn_map_section_names(maps_dir):
    names = set()
    for path in sorted(Path(maps_dir).glob("*/map.json")):
        data = json.loads(path.read_text())
        if data.get("game_version", "emerald") != "emerald":
            continue
        name = data.get("region_map_section")
        if isinstance(name, str) and name.startswith("MAPSEC_"):
            names.add(name)
    return names


def collect_hoenn_starter_occurrences(maps_dir):
    occurrences = []
    for map_json in sorted(Path(maps_dir).glob("*/map.json")):
        data = json.loads(map_json.read_text())
        if data.get("game_version", "emerald") != "emerald":
            continue

        script_path = map_json.with_name("scripts.inc")
        if not script_path.exists():
            continue
        for line_number, line in enumerate(script_path.read_text().splitlines(), 1):
            for match in STARTER_TOKEN_RE.finditer(line):
                occurrences.append((script_path, line_number, match.group(0)))
    return occurrences


def audit_hoenn_starter_symbols(maps_dir):
    occurrences = collect_hoenn_starter_occurrences(maps_dir)
    raw_occurrences = [
        (path, line_number)
        for path, line_number, symbol in occurrences
        if symbol == "VAR_STARTER_MON"
    ]
    if raw_occurrences:
        locations = ", ".join(f"{path}:{line_number}" for path, line_number in raw_occurrences)
        raise ValueError(
            f"Wayfarer Hoenn source uses raw VAR_STARTER_MON at {locations}; "
            "use VAR_HOENN_STARTER_CHOICE"
        )
    return occurrences


def render_common_tables(common_names, emerald_values, engine_values):
    aliases = {}
    flag_pairs = []
    var_pairs = []

    for name in sorted(common_names):
        engine_value = engine_values.get(name)
        source_value = emerald_values.get(name)
        if engine_value is None and source_value is None:
            continue

        if engine_value is None:
            engine_value = 0
        if source_value is None:
            hoenn_mapped_value = engine_value
        else:
            hoenn_mapped_value = hoenn_value(name, source_value)

        if engine_value == hoenn_mapped_value:
            aliases[name] = engine_value
            continue

        if name.startswith("FLAG_"):
            index = len(flag_pairs)
            if index >= 0x1000:
                raise ValueError("common flag dispatch namespace exceeds 4096 entries")
            aliases[name] = 0xA000 | index
            flag_pairs.append((name, engine_value, hoenn_mapped_value))
        else:
            index = len(var_pairs)
            if index >= 0x1000:
                raise ValueError("common variable dispatch namespace exceeds 4096 entries")
            aliases[name] = 0xB000 | index
            var_pairs.append((name, engine_value, hoenn_mapped_value))

    data = [
        "//",
        "// DO NOT MODIFY THIS FILE! It is auto-generated for shared script operands.",
        "//",
        "",
        f"#define WAYFARER_COMMON_FLAG_COUNT {len(flag_pairs)}",
        f"#define WAYFARER_COMMON_VAR_COUNT {len(var_pairs)}",
        "",
        "static const u16 sWayfarerCommonFlagIds[WAYFARER_COMMON_FLAG_COUNT][2] =",
        "{",
    ]
    for name, engine_value, hoenn_value_ in flag_pairs:
        data.append(f"    {{ 0x{engine_value:X}, 0x{hoenn_value_:X} }}, // {name}")
    data.extend(("};", "", "static const u16 sWayfarerCommonVarIds[WAYFARER_COMMON_VAR_COUNT][2] =", "{"))
    for name, engine_value, hoenn_value_ in var_pairs:
        data.append(f"    {{ 0x{engine_value:X}, 0x{hoenn_value_:X} }}, // {name}")
    data.extend(("};", ""))
    return render_table("runtime-dispatched common", aliases), "\n".join(data)


def main():
    args = parse_args()
    audit_hoenn_starter_symbols(args.maps_dir)
    emerald_macros = dump_macros(args.cpp, args.include_dir, "POKEMON_EMERALD")
    engine_macros = dump_macros(args.cpp, args.include_dir, "POKEMON_WAYFARER")
    source_names = sorted(
        name for name in emerald_macros
        if name.startswith(("FLAG_", "VAR_"))
    )
    source_names.extend(sorted(collect_hoenn_map_section_names(args.maps_dir)))
    if not source_names:
        raise SystemExit("no source constants were resolved")

    engine_names = sorted(
        name for name in engine_macros
        if name.startswith(("FLAG_", "VAR_"))
    )
    engine_names.extend(name for name in source_names if name.startswith("MAPSEC_"))
    emerald_values = dump_values(args.cc, args.include_dir, "POKEMON_EMERALD", source_names)
    engine_values = dump_values(args.cc, args.include_dir, "POKEMON_WAYFARER", engine_names)

    hoenn_values = {name: hoenn_value(name, emerald_values[name]) for name in source_names}
    restore_values = {name: engine_values.get(name) for name in set(source_names) | set(engine_names)}
    common_names = collect_common_names(args.event_scripts, args.scripts_dir)
    common_table, common_data = render_common_tables(common_names, emerald_values, engine_values)

    Path(args.hoenn_output).write_text(render_table("Hoenn", hoenn_values))
    Path(args.engine_output).write_text(render_table("the Wayfarer HNS engine", restore_values))
    Path(args.common_output).write_text(common_table)
    Path(args.common_data_output).write_text(common_data)


if __name__ == "__main__":
    main()

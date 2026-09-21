#!/usr/bin/env python3
"""Generate and verify the sealed Sinnoh tileset dependency closure.

The tool deliberately needs an explicit donor checkout only while refreshing
the checked-in source assets.  Normal verification is fully offline and reads
only the checked-in manifest and production inputs.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import shlex
import subprocess
import tempfile
from pathlib import Path
from typing import Any

from foundation import (DONOR_COMMIT, DONOR_URL, load_json, selected_source,
                        sha256_bytes, sha256_file, source_hashes, tileset_path)
from freeze import exact_candidates, header_block


BASELINE = "39a2660b95677c195f8441e10fc73d0b02ff18b3"
PRIMARY = ("General", "Building")
SECONDARY = (
    "Petalburg", "Rustboro", "Jubilife", "Mauville", "Hearthome", "Celestic",
    "Veilstone", "Lilycove", "Canalave", "Snowpoint", "Sunnyshore", "EverGrande",
    "Valor", "Lavaridge", "Cave", "Pasos", "BrendansMaysHouse", "GenericBuilding",
    "PokemonCenter", "Shop", "PokemonSchool", "Facility", "RustboroGym",
    "PrettyPetalFlowerShop",
)
EXISTING = {
    "BrendansMaysHouse", "GenericBuilding", "PokemonCenter", "PokemonSchool",
    "Facility", "RustboroGym", "PrettyPetalFlowerShop",
}
NEW = {"Jubilife", "Hearthome", "Celestic", "Veilstone", "Canalave", "Snowpoint", "Sunnyshore", "Valor", "Pasos"}
FULL_VARIANTS = {"General", "Petalburg", "Rustboro", "Mauville", "Lilycove", "EverGrande", "Lavaridge"}
PARTIAL_VARIANTS = {"Building", "Cave", "Shop"}


def source_kind(symbol: str) -> str:
    return "primary" if symbol in PRIMARY else "secondary"


def sinnoh_path(symbol: str, source: Path) -> Path:
    relative = source.relative_to(tileset_path(symbol))
    return Path("data/tilesets/sinnoh") / source_kind(symbol) / tileset_path(symbol).name / relative


def target_symbol(symbol: str) -> str:
    return f"gTileset_Sinnoh_{symbol.removeprefix('gTileset_')}"


def raw_symbol(symbol: str, component: str) -> str:
    return f"g{component}_Sinnoh_{symbol.removeprefix('gTileset_')}"


def palette_paths(symbol: str, root: Path) -> list[Path]:
    source = tileset_path(symbol)
    return sorted((root / source / "palettes").glob("*.pal"))


def descriptor_class(symbol: str) -> str:
    if symbol in EXISTING:
        return "EXISTING_REFERENCE"
    return "SINNOH_NEW" if symbol in NEW else "SINNOH_VARIANT"


def component_class(symbol: str, relative: Path) -> str:
    if symbol in EXISTING:
        return "EXISTING_REFERENCE"
    if symbol in NEW:
        return "SINNOH_NEW"
    if symbol in FULL_VARIANTS and "anim" not in relative.parts:
        return "SINNOH_VARIANT"
    if symbol == "Building" and relative == Path("palettes/04.pal"):
        return "SINNOH_VARIANT"
    if symbol == "Cave" and relative.name in {"metatiles.bin", "metatile_attributes.bin"}:
        return "SINNOH_VARIANT"
    if symbol == "Shop" and relative.name == "metatiles.bin":
        return "SINNOH_VARIANT"
    return "EXISTING_REFERENCE"


def component_name(relative: Path) -> str:
    if relative.name == "tiles.png":
        return "graphics"
    if relative.name == "metatiles.bin":
        return "metatiles"
    if relative.name == "metatile_attributes.bin":
        return "metatile_attributes"
    if relative.suffix == ".pal":
        return "palette"
    return "animation"


def generated_path(path: Path) -> Path:
    if path.suffix == ".pal":
        return path.with_suffix(".gbapal")
    if path.suffix == ".png":
        return path.with_suffix(".4bpp") if "anim" in path.parts else path.with_suffix(".4bpp.fastSmol")
    return path


def decoded_path(path: Path) -> Path:
    """Return the uncompressed production bytes for a generated payload."""
    return path.with_suffix("") if path.suffix in {".smol", ".fastSmol"} else path


def descriptor_path(symbol: str) -> Path:
    return (Path("src/data/tilesets/headers.h") if descriptor_class(symbol) == "EXISTING_REFERENCE"
            else Path("src/data/tilesets/sinnoh_headers.h"))


def production_source(row: dict[str, Any]) -> Path:
    path = Path(row["generated_path"])
    return Path(str(path).replace(".gbapal", ".pal").replace(".4bpp.fastSmol", ".png").replace(".4bpp.smol", ".png").replace(".4bpp", ".png"))


def checked_source_path(root: Path, row: dict[str, Any]) -> Path:
    if row["component"] == "descriptor":
        return Path(row["generated_path"])
    if row["component"] == "animation":
        source_root = tileset_path(row["source_symbol"])
        source = Path(row["source_path"])
        copied = sinnoh_path(row["source_symbol"], source_root / source.relative_to(source_root))
        if (root / copied).is_file():
            return copied
    return production_source(row)


def tileset_gfx_dir(root: Path, *, rules_text: str | None = None) -> Path:
    """Resolve the single supported TILESETGFXDIR assignment from Make rules."""
    rules = rules_text if rules_text is not None else (root / "graphics_file_rules.mk").read_text(encoding="utf-8")
    candidates = [line for line in rules.splitlines() if re.match(r"\s*TILESETGFXDIR\s*(?::=|\?=|\+=|=)", line)]
    if len(candidates) != 1:
        raise ValueError("missing or ambiguous TILESETGFXDIR assignment")
    match = re.fullmatch(r"\s*TILESETGFXDIR\s*:=\s*([^\s#]+)\s*(?:#.*)?", candidates[0])
    if match is None:
        raise ValueError(f"unsupported TILESETGFXDIR assignment: {candidates[0]}")
    directory = Path(match.group(1))
    if directory.is_absolute() or "$" in match.group(1) or any(part in {".", ".."} for part in directory.parts):
        raise ValueError(f"unsupported TILESETGFXDIR value: {match.group(1)}")
    return directory


def graphics_encoder_args(root: Path, output: Path, *, rules_text: str | None = None) -> tuple[str, ...]:
    """Return the exact gbagfx arguments selected by the production Make rule.

    Tileset graphics are normally converted with gbagfx defaults, but a small
    set of existing targets intentionally limits their decoded tile count.
    The proof generator must use the target rule, not merely the source
    filename, otherwise a stale generic ``.4bpp`` can make the manifest look
    valid while a clean Make build emits different bytes.
    """
    raw = decoded_path(output)
    if raw.suffix != ".4bpp":
        return ()
    try:
        relative = raw.relative_to(root).as_posix()
    except ValueError:
        relative = raw.as_posix()
    rules_text = rules_text if rules_text is not None else (root / "graphics_file_rules.mk").read_text(encoding="utf-8")
    directory = tileset_gfx_dir(root, rules_text=rules_text)
    rules = rules_text.splitlines()
    target: str | None = None
    for line in rules:
        match = re.match(r"\$\(TILESETGFXDIR\)/([^:]+):", line)
        if match:
            target = (directory / match.group(1)).as_posix()
            continue
        if target != relative:
            continue
        recipe = re.match(r"\s*\$\(GFX\)\s+\$<\s+\$@(?:\s+(.*))?$", line)
        if recipe:
            return tuple(shlex.split(recipe.group(1) or ""))
        if line.startswith("\t"):
            raise ValueError(f"unsupported graphics rule recipe for {relative}: {line}")
    return ()


def generic_gbagfx_args(root: Path, output_suffix: str, source_suffix: str, *,
                         makefile_text: str | None = None) -> tuple[str, ...]:
    """Parse a generic GFX Make recipe and reject semantic drift."""
    makefile = makefile_text if makefile_text is not None else (root / "Makefile").read_text(encoding="utf-8")
    pattern = re.compile(rf"^%{re.escape(output_suffix)}:\s+%{re.escape(source_suffix)}\s*;\s+\$\(GFX\)\s+(.+?)\s*$", re.M)
    match = pattern.search(makefile)
    if match is None:
        raise ValueError(f"missing generic GFX recipe for %{output_suffix}:%{source_suffix}")
    args = tuple(shlex.split(match.group(1)))
    if args != ("$<", "$@"):
        raise ValueError(f"generic GFX recipe semantic drift for %{output_suffix}:%{source_suffix}")
    return args


def gbagfx_recipe(root: Path, source: Path, output: Path, *, rule_output: Path | None = None,
                  graphics_rules_text: str | None = None, makefile_text: str | None = None) -> tuple[str, tuple[str, ...]]:
    """Return the exact Make-derived GFX recipe for one transformed source."""
    target = rule_output if rule_output is not None else output
    if source.suffix == ".pal":
        if target.suffix != ".gbapal":
            raise ValueError(f"unexpected palette production target: {target}")
        return "Makefile:%.gbapal:%.pal", generic_gbagfx_args(root, ".gbapal", ".pal", makefile_text=makefile_text)
    if source.suffix == ".png":
        raw = decoded_path(target)
        if raw.suffix != ".4bpp":
            raise ValueError(f"unexpected PNG production target: {target}")
        generic = generic_gbagfx_args(root, ".4bpp", ".png", makefile_text=makefile_text)
        special = graphics_encoder_args(root, target, rules_text=graphics_rules_text)
        if special:
            try:
                relative = raw.relative_to(root).as_posix()
            except ValueError:
                relative = raw.as_posix()
            return f"graphics_file_rules.mk:{relative}", generic + special
        return "Makefile:%.4bpp:%.png", generic
    raise ValueError(f"unsupported GFX source: {source}")


def compression_encoder_args(root: Path, output: Path, *, makefile_text: str | None = None) -> tuple[str, ...]:
    """Parse and fail closed on the generic Make compression recipe we use."""
    expected = {
        ".fastSmol": ("-w", "$<", "$@", "false", "false", "false"),
        ".smol": ("-w", "$<", "$@"),
    }.get(output.suffix)
    if expected is None:
        return ()
    makefile = makefile_text if makefile_text is not None else (root / "Makefile").read_text(encoding="utf-8")
    pattern = re.compile(rf"^%{re.escape(output.suffix)}:\s+%\s*;\s+\$\(SMOL\)\s+(.+?)\s*$", re.M)
    match = pattern.search(makefile)
    if match is None:
        raise ValueError(f"missing generic compression recipe for {output.suffix}")
    args = tuple(shlex.split(match.group(1)))
    if args != expected:
        raise ValueError(f"compression recipe semantic drift for {output.suffix}")
    return args


def generate_component(root: Path, source: Path, output: Path, *, rule_output: Path | None = None,
                       graphics_rules_text: str | None = None, makefile_text: str | None = None) -> None:
    """Use the checked-in production encoders, never a filename-only shortcut."""
    output.parent.mkdir(parents=True, exist_ok=True)
    target = rule_output if rule_output is not None else output
    raw = output.with_suffix("") if output.suffix in {".smol", ".fastSmol"} else output
    _, recipe = gbagfx_recipe(root, source, output, rule_output=target,
                              graphics_rules_text=graphics_rules_text, makefile_text=makefile_text)
    args = [str(source) if arg == "$<" else str(raw) if arg == "$@" else arg for arg in recipe]
    subprocess.run([str(root / "tools/gbagfx/gbagfx"), *args], check=True)
    compression = compression_encoder_args(root, output, makefile_text=makefile_text)
    if compression:
        args = [str(raw) if arg == "$<" else str(output) if arg == "$@" else arg for arg in compression]
        subprocess.run([str(root / "tools/compresSmol/compresSmol"), *args], check=True)


def generate_production(root: Path, manifest: dict[str, Any]) -> None:
    for row in manifest.get("records", []):
        if row.get("asset_family") != "tileset_component" or row.get("component") == "descriptor":
            continue
        source = root / production_source(row)
        output = root / row["generated_path"]
        if source.suffix in {".pal", ".png"}:
            if not source.is_file():
                raise ValueError(f"missing production source for {row['record_id']}")
            generate_component(root, source, output)


def refresh_donor_proofs(root: Path, donor: Path, manifest: dict[str, Any]) -> None:
    """Generate donor bytes with this worktree's production encoders for proof."""
    with tempfile.TemporaryDirectory(prefix="sinnoh-donor-assets-") as directory:
        temp = Path(directory)
        for row in manifest.get("records", []):
            if row.get("asset_family") != "tileset_component":
                continue
            if row.get("component") == "descriptor":
                block = header_block(donor / row["source_path"], row["source_symbol"])
                row["donor_generated_sha256"] = sha256_bytes(block)
                row["donor_decoded_sha256"] = row["donor_generated_sha256"]
                row["donor_decoded_bytes"] = len(block)
                continue
            source = donor / row["source_path"]
            output = temp / row["generated_path"]
            if source.suffix in {".pal", ".png"}:
                generate_component(root, source, output, rule_output=root / row["generated_path"])
            else:
                output.parent.mkdir(parents=True, exist_ok=True)
                output.write_bytes(source.read_bytes())
            decoded = decoded_path(output)
            row["donor_generated_sha256"] = sha256_file(output)
            row["donor_decoded_sha256"] = sha256_file(decoded)
            row["donor_decoded_bytes"] = decoded.stat().st_size


def header_graphics(symbol: str, root: Path) -> str:
    cls = descriptor_class(symbol)
    if cls == "EXISTING_REFERENCE":
        return ""
    name = symbol.removeprefix("gTileset_")
    source = tileset_path(symbol)
    own = Path("data/tilesets/sinnoh") / source_kind(symbol) / source.name
    lines: list[str] = []
    if symbol not in {"Building", "Cave", "Shop"}:
        lines.append(f'const u32 {raw_symbol(symbol, "TilesetTiles")}[] = INCBIN_U32("{(own / "tiles.4bpp.fastSmol").as_posix()}");')
    if symbol not in {"Cave", "Shop"}:
        lines.append(f"const u16 {raw_symbol(symbol, 'TilesetPalettes')}[][16] =")
        lines.append("{")
        palette_root = root / own if symbol != "Building" and (root / own / "palettes").is_dir() else root / source
        for donor_palette in sorted((palette_root / "palettes").glob("*.pal")):
            relative = donor_palette.relative_to(palette_root)
            path = own / relative if component_class(symbol, relative) != "EXISTING_REFERENCE" else source / relative
            lines.append(f'    INCBIN_U16("{generated_path(path).as_posix()}"),')
        lines.append("};")
    return "\n".join(lines)


def descriptor_parts(symbol: str) -> tuple[str, str, str, str, str | None]:
    name = symbol.removeprefix("gTileset_")
    if symbol in EXISTING:
        return (f"gTilesetTiles_{name}", f"gTilesetPalettes_{name}", f"gMetatiles_{name}", f"gMetatileAttributes_{name}", None)
    if symbol == "Building":
        return ("gTilesetTiles_InsideBuilding", raw_symbol(symbol, "TilesetPalettes"), "gMetatiles_InsideBuilding", "gMetatileAttributes_InsideBuilding", "InitTilesetAnim_Building")
    if symbol == "Cave":
        return ("gTilesetTiles_Cave", "gTilesetPalettes_Cave", raw_symbol(symbol, "Metatiles"), raw_symbol(symbol, "MetatileAttributes"), "InitTilesetAnim_Cave")
    if symbol == "Shop":
        return ("gTilesetTiles_Shop", "gTilesetPalettes_Shop", raw_symbol(symbol, "Metatiles"), "gMetatileAttributes_Shop", None)
    callbacks = {"General": "InitTilesetAnim_General", "Rustboro": "InitTilesetAnim_Rustboro", "Mauville": "InitTilesetAnim_Mauville", "Lavaridge": "InitTilesetAnim_Lavaridge", "EverGrande": "InitTilesetAnim_EverGrande"}
    return (raw_symbol(symbol, "TilesetTiles"), raw_symbol(symbol, "TilesetPalettes"), raw_symbol(symbol, "Metatiles"), raw_symbol(symbol, "MetatileAttributes"), callbacks.get(name))


def production_runtime(root: Path) -> dict[str, tuple[str, str]]:
    """Index the paths actually consumed by production C declarations.

    This deliberately reads the graphics/metatile headers and animation callback
    source.  A matching donor filename is not evidence that a component is
    linked: mixed descriptors (Building, Cave, and Shop) can deliberately point
    at a different existing owner.
    """
    symbols: dict[str, tuple[str, str]] = {}
    graphics = "\n".join(
        path.read_text(encoding="utf-8")
        for path in (root / "src/data/tilesets/graphics.h", root / "src/data/tilesets/sinnoh_graphics.h")
        if path.is_file()
    )
    metatiles = "\n".join(
        path.read_text(encoding="utf-8")
        for path in (root / "src/data/tilesets/metatiles.h", root / "src/data/tilesets/sinnoh_metatiles.h")
        if path.is_file()
    )
    animations = (root / "src/tileset_anims.c").read_text(encoding="utf-8")
    direct = re.compile(r'const\s+(u32|u16)\s+(\w+)\[\]\s*=\s*INCBIN_U(?:32|16)\("([^"]+)"\);')
    for text in (graphics, metatiles, animations):
        for element_type, name, path in direct.findall(text):
            symbols[name] = (path, element_type)
    palettes = re.compile(r'const\s+u16\s+(\w+)\[\]\[16\]\s*=\s*\{(.*?)\n\};', re.S)
    for table, body in palettes.findall(graphics):
        paths = re.findall(r'INCBIN_U16\("([^"]+)"\)', body)
        for index, path in enumerate(paths):
            symbols[f"{table}[{index:02d}]"] = (path, "u16")
    return symbols


def runtime_component(root: Path, symbol: str, relative: Path, runtime: dict[str, tuple[str, str]]) -> tuple[str, Path, str]:
    """Resolve one manifest row to the exact symbol/path consumed at runtime."""
    component = component_name(relative)
    if component == "animation":
        expected_path = generated_path(tileset_path(symbol) / relative).as_posix()
        candidates = [name for name, declaration in runtime.items() if declaration[0] == expected_path]
        if len(candidates) != 1:
            raise ValueError(f"production animation consumer missing or ambiguous for {symbol}/{relative}")
        return candidates[0], Path(expected_path), runtime[candidates[0]][1]
    parts = descriptor_parts(symbol)
    part_index = {"graphics": 0, "palette": 1, "metatiles": 2, "metatile_attributes": 3}[component]
    runtime_symbol = parts[part_index]
    if component == "palette":
        runtime_symbol = f"{runtime_symbol}[{int(relative.stem):02d}]"
    declaration = runtime.get(runtime_symbol)
    if declaration is None:
        raise ValueError(f"production component consumer missing for {symbol}/{relative}: {runtime_symbol}")
    return runtime_symbol, Path(declaration[0]), declaration[1]


def production_alignment(component: str, element_type: str) -> int:
    """Return the production object's alignment contract.

    Animation arrays are u16 declarations, but the aggregate tileset animation
    object places their rodata symbols at four-byte boundaries.  The manifest
    therefore preserves that object-level alignment rather than inferring it
    from a PNG filename or only the C element width.
    """
    if component == "animation":
        return 4
    return {"u16": 2, "u32": 4}[element_type]


def header_metatiles(symbol: str) -> str:
    if descriptor_class(symbol) == "EXISTING_REFERENCE":
        return ""
    source = tileset_path(symbol)
    own = Path("data/tilesets/sinnoh") / source_kind(symbol) / source.name
    lines: list[str] = []
    for filename, component in (("metatiles.bin", "Metatiles"), ("metatile_attributes.bin", "MetatileAttributes")):
        if symbol in {"Building", "Shop"} and filename == "metatile_attributes.bin":
            continue
        if symbol == "Building":
            continue
        if symbol == "Shop" and filename == "metatile_attributes.bin":
            continue
        lines.append(f'const u16 {raw_symbol(symbol, component)}[] = INCBIN_U16("{(own / filename).as_posix()}");')
    return "\n".join(lines)


def header_descriptor(symbol: str) -> str:
    if descriptor_class(symbol) == "EXISTING_REFERENCE":
        return ""
    tiles, palettes, metatiles, attributes, callback = descriptor_parts(symbol)
    return "\n".join((
        f"const struct Tileset {target_symbol(symbol)} =", "{",
        f"    .isCompressed = TRUE,", f"    .isSecondary = {'FALSE' if symbol in PRIMARY else 'TRUE'},",
        f"    .tiles = {tiles},", f"    .palettes = {palettes},", f"    .metatiles = {metatiles},",
        f"    .metatileAttributes = {attributes},", f"    .callback = {callback or 'NULL'},", "};",
    ))


def write_headers(root: Path) -> None:
    base = root / "src/data/tilesets"
    rows = [symbol for symbol in (*PRIMARY, *SECONDARY) if descriptor_class(symbol) != "EXISTING_REFERENCE"]
    for filename, writer in (("sinnoh_graphics.h", lambda symbol: header_graphics(symbol, root)), ("sinnoh_metatiles.h", header_metatiles), ("sinnoh_headers.h", header_descriptor)):
        payload = "// Generated by tools/wayfarer_sinnoh_port/assets.py; do not edit.\n\n" + "\n\n".join(filter(None, (writer(symbol) for symbol in rows))) + "\n"
        (base / filename).write_text(payload, encoding="utf-8")


def record_id(symbol: str, relative: Path) -> str:
    return f"tileset.gTileset_{symbol}.{relative.as_posix().replace('/', '.')}"


def build_manifest(root: Path, donor: Path) -> dict[str, Any]:
    records: list[dict[str, Any]] = []
    runtime = production_runtime(root)
    for symbol in (*PRIMARY, *SECONDARY):
        source = tileset_path(symbol)
        donor_symbol = f"gTileset_{symbol}"
        consumers = [row["source_layout"] for row in load_json(root / "src/data/wayfarer_sinnoh_maps.json")["maps"] if donor_symbol in (row["tilesets"]["source_primary"], row["tilesets"]["source_secondary"])]
        cls = descriptor_class(symbol)
        descriptor_target = donor_symbol if cls == "EXISTING_REFERENCE" else target_symbol(symbol)
        donor_descriptor = header_block(donor / "src/data/tilesets/headers.h", donor_symbol)
        records.append({"record_id": record_id(symbol, Path("descriptor")), "asset_family": "tileset_component", "component": "descriptor", "source_symbol": donor_symbol, "proposed_target_symbol": descriptor_target, "generated_symbol": descriptor_target, "source_path": "src/data/tilesets/headers.h", "source_sha256": sha256_bytes(donor_descriptor), "source_bytes": len(donor_descriptor), "generated_path": descriptor_path(symbol).as_posix(), "generated_sha256": None, "decoded_sha256": None, "decoded_bytes": None, "array_shape": None, "alignment": 4, "compression": False, "layout_format": "emerald", "linkage": "external", "visibility": "public", "output_section": ".rodata", "consumers": consumers, "reuse_class": cls, "canonical_owner": donor_symbol if cls == "EXISTING_REFERENCE" else None, "selection_blocker": False, "rationale": "Direct checked-in Wayfarer descriptor." if cls == "EXISTING_REFERENCE" else "Independent Sinnoh descriptor preserves existing map output."})
        for path in sorted((donor / source).rglob("*")):
            if not path.is_file():
                continue
            relative = path.relative_to(donor / source)
            component_cls = component_class(symbol, relative)
            runtime_symbol, production, element_type = runtime_component(root, symbol, relative, runtime)
            component = component_name(relative)
            transformed = path.suffix in {".png", ".pal"}
            encoder_rule, encoder_args = (gbagfx_recipe(root, path, production) if transformed else (None, ()))
            records.append({"record_id": record_id(symbol, relative), "asset_family": "tileset_component", "component": component, "source_symbol": symbol, "proposed_target_symbol": descriptor_target, "generated_symbol": runtime_symbol, "source_path": (source / relative).as_posix(), "source_sha256": sha256_file(path), "source_bytes": path.stat().st_size, "generated_path": production.as_posix(), "generated_sha256": None, "decoded_sha256": None, "decoded_bytes": None, "array_shape": None, "alignment": production_alignment(component, element_type), "compression": component == "graphics" and production.suffix in {".smol", ".fastSmol"}, "encoder_rule": encoder_rule, "encoder_args": list(encoder_args) if transformed else None, "layout_format": "emerald", "linkage": "external", "visibility": "public", "output_section": ".rodata", "consumers": consumers, "reuse_class": component_cls, "canonical_owner": runtime_symbol if component_cls == "EXISTING_REFERENCE" else None, "selection_blocker": False, "rationale": "Direct byte-verified Wayfarer component." if component_cls == "EXISTING_REFERENCE" else "Dedicated Sinnoh production component."})
    groups, maps, layouts = selected_source(donor)
    for _, _, name in groups:
        layout = layouts[maps[name]["layout"]]
        paths, hashes = source_hashes(donor, name, layout)
        for family, path_key in (("blockdata", "blockdata_filepath"), ("border", "border_filepath")):
            source_path = donor / layout[path_key]
            candidates = exact_candidates(root, source_path, path_key, layout)
            proven = next((candidate for candidate in candidates if candidate["runtime_meaning_proven"]), None)
            records.append({"record_id": f"{family}.{layout['id']}", "asset_family": family,
                            "source_path": layout[path_key], "source_sha256": sha256_file(source_path),
                            "source_bytes": source_path.stat().st_size, "layout": layout["id"],
                            "layout_format": "emerald", "shape": ({"width": layout["width"], "height": layout["height"], "element_bytes": 2, "byte_length": layout["width"] * layout["height"] * 2} if family == "blockdata" else {"tile_count": 4, "element_bytes": 2, "byte_length": 8}),
                            "consumers": [layout["id"]], "candidate_matches": candidates,
                            "reuse_class": "EXACT_ALIAS" if proven else "SINNOH_NEW",
                            "canonical_owner": proven["layout"] if proven else None,
                            "selection_blocker": False, "expected_linked_delta": "deferred-to-map-catalog",
                            "rationale": "Manifest-owned canonical alias; a future MapLayout stays independent." if proven else "Deferred independent layout payload; the map catalog is not imported in this milestone.",
                            "proof": {"command": "wayfarer_sinnoh_port.audit --root <worktree>", "version": 2},
                            "source_paths": paths, "source_hashes": hashes})
    for row in records:
        if row.get("asset_family") != "tileset_component":
            continue
        row["proof"] = {"command": "tools/wayfarer_sinnoh_port/assets.py --root . --verify", "version": 3}
        row["expected_linked_delta"] = 0 if row["reuse_class"] == "EXISTING_REFERENCE" else None
        row["donor_generated_sha256"] = None
        row["donor_decoded_sha256"] = None
        row["donor_decoded_bytes"] = None
        row["checked_in_source_path"] = checked_source_path(root, row).as_posix()
        checked = root / row["checked_in_source_path"]
        row["checked_in_source_sha256"] = sha256_file(checked) if checked.is_file() else None
        if row["component"] == "animation" and animations_symbol_uses(root, row["generated_symbol"]) < 2:
            # The donor source closure includes these four legacy lava frames,
            # but the production callback array never selects them.
            row["storage_role"] = "runtime_unreachable"
            row["delivery_role"] = "unreachable_existing_animation"
        else:
            row["storage_role"] = "source_only_proof" if row["component"] == "animation" and row["checked_in_source_path"].startswith("data/tilesets/sinnoh/") else "runtime"
            row["delivery_role"] = "runtime_shared_animation" if row["storage_role"] == "source_only_proof" else "runtime"
    return {"schema_version": 1, "donor": {"url": DONOR_URL, "commit": DONOR_COMMIT}, "wayfarer_baseline": {"commit": BASELINE}, "implementation_head": "d26a524d66a2772e81efe7a3838b5702f7960719", "selection": {"release_link_enabled": False, "asset_manifest_ready": False, "blockers": []}, "records": sorted(records, key=lambda row: row["record_id"])}


def refresh_proofs(root: Path, manifest: dict[str, Any]) -> None:
    """Record hashes from the actual files emitted by the production asset rules."""
    runtime = production_runtime(root)
    for row in manifest.get("records", []):
        if row.get("asset_family") != "tileset_component":
            continue
        if row.get("component") == "descriptor":
            path = root / row["generated_path"]
            block = header_block(path, row["generated_symbol"])
            row["generated_sha256"] = sha256_bytes(block)
            row["decoded_sha256"] = row["generated_sha256"]
            row["decoded_bytes"] = len(block)
            row["array_shape"] = {"element_type": "struct Tileset", "element_count": 1}
            row["expected_linked_delta"] = 0 if row["reuse_class"] == "EXISTING_REFERENCE" else len(block)
            continue
        production = root / row["generated_path"]
        decoded = root / decoded_path(Path(row["generated_path"]))
        if not production.is_file() or not decoded.is_file():
            raise ValueError(f"production output is missing for {row['record_id']}")
        row["generated_sha256"] = sha256_file(production)
        row["decoded_sha256"] = sha256_file(decoded)
        row["decoded_bytes"] = decoded.stat().st_size
        declaration = runtime.get(row["generated_symbol"])
        if declaration is None or declaration[0] != row["generated_path"]:
            raise ValueError(f"production declaration missing for {row['record_id']}")
        element_type = declaration[1]
        width = {"u16": 2, "u32": 4}[element_type]
        row["array_shape"] = {"element_type": element_type, "element_count": row["decoded_bytes"] // width}
        row["expected_linked_delta"] = 0 if row["reuse_class"] == "EXISTING_REFERENCE" else production.stat().st_size


def resolve_layout_aliases(root: Path, manifest: dict[str, Any]) -> dict[str, dict[str, Any]]:
    """Resolve deferred MapLayout byte aliases without creating a map catalog.

    A record names a current Wayfarer layout owner, never another record.  This
    deliberately forbids alias chains/cycles and leaves each future MapLayout
    descriptor independent when the catalog is introduced.
    """
    layouts = {
        layout.get("id"): layout
        for layout in load_json(root / "data/layouts/layouts.json").get("layouts", [])
        if isinstance(layout, dict) and isinstance(layout.get("id"), str)
    }
    aliases: dict[str, dict[str, Any]] = {}
    for row in manifest.get("records", []):
        family = row.get("asset_family")
        if family not in {"blockdata", "border"} or row.get("reuse_class") != "EXACT_ALIAS":
            continue
        if row.get("alias_of") is not None:
            raise ValueError(f"layout alias chains are forbidden: {row.get('record_id')}")
        owner = row.get("canonical_owner")
        candidate = next((item for item in row.get("candidate_matches", []) if item.get("layout") == owner), None)
        if not isinstance(candidate, dict) or not candidate.get("runtime_meaning_proven"):
            raise ValueError(f"missing canonical layout owner for {row.get('record_id')}")
        layout = layouts.get(owner)
        path_key = "blockdata_filepath" if family == "blockdata" else "border_filepath"
        if not isinstance(layout, dict) or layout.get(path_key) != candidate.get("path"):
            raise ValueError(f"wrong-family or excluded layout owner for {row.get('record_id')}")
        shape = row.get("shape")
        if candidate.get("source_shape") != shape or candidate.get("candidate_shape") != shape:
            raise ValueError(f"layout alias shape mismatch for {row.get('record_id')}")
        owner_path = root / candidate["path"]
        if not owner_path.is_file() or owner_path.stat().st_size != shape.get("byte_length"):
            raise ValueError(f"missing layout alias owner bytes for {row.get('record_id')}")
        aliases[row["record_id"]] = {
            "canonical_owner": owner,
            "path": candidate["path"],
            "shape": shape,
        }
    return aliases


def verify_runtime_component_closure(root: Path, manifest: dict[str, Any]) -> None:
    """Fail closed unless each component names the C-visible runtime consumer.

    The descriptor is the sole component consumer.  Its symbols are resolved
    through actual production declarations, so neither a donor filename nor an
    invented Sinnoh raw symbol can satisfy this check.
    """
    runtime = production_runtime(root)
    descriptors = {
        row["source_symbol"]: row
        for row in manifest.get("records", [])
        if row.get("asset_family") == "tileset_component" and row.get("component") == "descriptor"
    }
    fields_by_symbol: dict[str, dict[str, str]] = {}
    field_pattern = re.compile(r'\.(tiles|palettes|metatiles|metatileAttributes|callback)\s*=\s*([^,]+),')
    for source_symbol, descriptor in descriptors.items():
        block = header_block(root / descriptor["generated_path"], descriptor["generated_symbol"]).decode("utf-8")
        fields = dict(field_pattern.findall(block))
        if set(fields) != {"tiles", "palettes", "metatiles", "metatileAttributes", "callback"}:
            raise ValueError(f"descriptor component closure missing fields for {source_symbol}")
        fields_by_symbol[source_symbol] = fields
    for row in manifest.get("records", []):
        if row.get("asset_family") != "tileset_component" or row.get("component") == "descriptor":
            continue
        source_symbol = row["source_symbol"]
        relative = Path(row["source_path"]).relative_to(tileset_path(source_symbol))
        expected_symbol, expected_path, _ = runtime_component(root, source_symbol, relative, runtime)
        if row.get("generated_symbol") != expected_symbol:
            raise ValueError(f"runtime symbol closure drift for {row['record_id']}")
        if row.get("generated_path") != expected_path.as_posix():
            raise ValueError(f"runtime path closure drift for {row['record_id']}")
        fields = fields_by_symbol.get(f"gTileset_{source_symbol}")
        if fields is None:
            raise ValueError(f"missing descriptor closure for {row['record_id']}")
        if row.get("component") == "animation":
            used = animations_symbol_uses(root, expected_symbol)
            if row.get("delivery_role") == "unreachable_existing_animation":
                if used != 1 or row.get("storage_role") != "runtime_unreachable":
                    raise ValueError(f"unreachable animation closure drift for {row['record_id']}")
            else:
                if fields["callback"] == "NULL" or used < 2:
                    raise ValueError(f"animation descriptor closure drift for {row['record_id']}")
                if row.get("delivery_role") not in {"runtime", "runtime_shared_animation"}:
                    raise ValueError(f"animation delivery role drift for {row['record_id']}")
        else:
            field = {"graphics": "tiles", "palette": "palettes", "metatiles": "metatiles", "metatile_attributes": "metatileAttributes"}[row["component"]]
            symbol_in_descriptor = expected_symbol.split("[", 1)[0]
            if fields[field] != symbol_in_descriptor:
                raise ValueError(f"descriptor component closure drift for {row['record_id']}")
        if row.get("component") != "animation" and row.get("delivery_role") != "runtime":
            raise ValueError(f"runtime delivery role drift for {row['record_id']}")


def animations_symbol_uses(root: Path, symbol: str) -> int:
    """Count C-source references; a definition alone is not a runtime consumer."""
    return (root / "src/tileset_anims.c").read_text(encoding="utf-8").count(symbol)


def verify_fresh_transformed_components(root: Path, manifest: dict[str, Any], *,
                                        graphics_rules_text: str | None = None,
                                        makefile_text: str | None = None) -> int:
    """Prove every PNG/palette production output matches its current Make recipe."""
    generic_gbagfx_args(root, ".4bpp", ".png", makefile_text=makefile_text)
    generic_gbagfx_args(root, ".gbapal", ".pal", makefile_text=makefile_text)
    compression_encoder_args(root, Path("tiles.4bpp.fastSmol"), makefile_text=makefile_text)
    compression_encoder_args(root, Path("tiles.4bpp.smol"), makefile_text=makefile_text)
    transformed = [
        row for row in manifest.get("records", [])
        if row.get("asset_family") == "tileset_component"
        and row.get("component") != "descriptor"
        and production_source(row).suffix in {".png", ".pal"}
    ]
    if not transformed:
        raise ValueError("Sinnoh asset closure lost transformed components")
    with tempfile.TemporaryDirectory(prefix="sinnoh-fresh-production-") as directory:
        temporary = Path(directory)
        for row in transformed:
            target = root / row["generated_path"]
            fresh = temporary / row["generated_path"]
            source = root / production_source(row)
            decoded_target = root / decoded_path(Path(row["generated_path"]))
            decoded_fresh = decoded_path(fresh)
            if not source.is_file() or not target.is_file() or not decoded_target.is_file():
                raise ValueError(f"stale or missing production output for {row['record_id']}")
            generate_component(root, source, fresh, rule_output=target,
                               graphics_rules_text=graphics_rules_text, makefile_text=makefile_text)
            if fresh.read_bytes() != target.read_bytes() or decoded_fresh.read_bytes() != decoded_target.read_bytes():
                raise ValueError(f"fresh transformed production drift for {row['record_id']}")
    return len(transformed)


def verify(root: Path, manifest: dict[str, Any], production: bool = True, *,
           graphics_rules_text: str | None = None, makefile_text: str | None = None) -> dict[str, int]:
    rows = manifest.get("records", [])
    if manifest.get("selection", {}).get("release_link_enabled") or manifest.get("selection", {}).get("asset_manifest_ready"):
        raise ValueError("Sinnoh asset milestone must not open the release gate")
    if len(rows) != len({row["record_id"] for row in rows}):
        raise ValueError("Sinnoh asset manifest has duplicate records")
    if manifest.get("implementation_head") != "d26a524d66a2772e81efe7a3838b5702f7960719":
        raise ValueError("Sinnoh asset manifest implementation head drifted")
    classes = {"EXISTING_REFERENCE", "SINNOH_VARIANT", "SINNOH_NEW", "EXACT_ALIAS"}
    counts = {key: 0 for key in classes}
    if production:
        verify_fresh_transformed_components(root, manifest, graphics_rules_text=graphics_rules_text,
                                            makefile_text=makefile_text)
    verify_runtime_component_closure(root, manifest)
    runtime = production_runtime(root)
    for row in rows:
        if row.get("reuse_class") not in classes or row.get("selection_blocker"):
            raise ValueError(f"unresolved or invalid record: {row.get('record_id')}")
        counts[row["reuse_class"]] += 1
        if row["asset_family"] != "tileset_component":
            continue
        source_path = production_source(row)
        if row["component"] != "descriptor" and not (root / source_path).exists() and not (root / row["generated_path"]).exists():
            raise ValueError(f"missing production source for {row['record_id']}")
        if not row.get("consumers"):
            raise ValueError(f"undeclared consumer for {row['record_id']}")
        if not row.get("generated_sha256") or not row.get("decoded_sha256") or not isinstance(row.get("decoded_bytes"), int):
            raise ValueError(f"missing generated/decoded proof for {row['record_id']}")
        if not row.get("donor_generated_sha256") or not row.get("donor_decoded_sha256") or not isinstance(row.get("donor_decoded_bytes"), int):
            raise ValueError(f"missing donor production proof for {row['record_id']}")
        checked = root / row["checked_in_source_path"]
        if not checked.is_file() or sha256_file(checked) != row.get("checked_in_source_sha256"):
            raise ValueError(f"checked-in source drift for {row['record_id']}")
        if row["reuse_class"] == "EXISTING_REFERENCE" and (row["generated_sha256"], row["decoded_sha256"], row["decoded_bytes"]) != (row["donor_generated_sha256"], row["donor_decoded_sha256"], row["donor_decoded_bytes"]):
            raise ValueError(f"exact reference production mismatch for {row['record_id']}")
        if row["layout_format"] != "emerald" or row["linkage"] != "external" or row["visibility"] != "public" or row["output_section"] != ".rodata":
            raise ValueError(f"production format/linkage drift for {row['record_id']}")
        if row["component"] != "descriptor":
            declaration = runtime.get(row["generated_symbol"])
            if declaration is None or declaration[0] != row["generated_path"]:
                raise ValueError(f"production declaration drift for {row['record_id']}")
            expected_type = declaration[1]
            width = {"u16": 2, "u32": 4}[expected_type]
            shape = row.get("array_shape")
            if not isinstance(shape, dict) or shape.get("element_type") != expected_type or shape.get("element_count") != row["decoded_bytes"] // width or row["decoded_bytes"] % width:
                raise ValueError(f"production shape drift for {row['record_id']}")
            if row.get("alignment") != production_alignment(row["component"], expected_type):
                raise ValueError(f"production alignment drift for {row['record_id']}")
            compressed = row["component"] == "graphics"
            compressed_path = row["generated_path"].endswith((".4bpp.fastSmol", ".4bpp.smol"))
            if row["compression"] != compressed or (compressed and not compressed_path):
                raise ValueError(f"production compression drift for {row['record_id']}")
            if source_path.suffix in {".png", ".pal"}:
                encoder_rule, encoder_args = gbagfx_recipe(root, source_path, Path(row["generated_path"]),
                                                           graphics_rules_text=graphics_rules_text,
                                                           makefile_text=makefile_text)
                if row.get("encoder_rule") != encoder_rule or row.get("encoder_args") != list(encoder_args):
                    raise ValueError(f"production GFX encoder rule drift for {row['record_id']}")
            elif row.get("encoder_rule") is not None or row.get("encoder_args") is not None:
                raise ValueError(f"unexpected GFX encoder contract for {row['record_id']}")
            if row["reuse_class"] != "EXISTING_REFERENCE" and compressed and not row["generated_path"].endswith(".4bpp.fastSmol"):
                raise ValueError(f"Sinnoh graphics compression drift for {row['record_id']}")
        if row["reuse_class"] == "SINNOH_VARIANT" and row["component"] != "descriptor" and (not row["proposed_target_symbol"].startswith("gTileset_Sinnoh_") or not row["generated_path"].startswith("data/tilesets/sinnoh/")):
            raise ValueError(f"variant reaches an existing target for {row['record_id']}")
        if not production:
            continue
        if row["component"] == "descriptor":
            block = header_block(root / row["generated_path"], row["generated_symbol"])
            generated_hash = sha256_bytes(block)
            decoded_bytes = len(block)
        else:
            generated = root / row["generated_path"]
            decoded = root / decoded_path(Path(row["generated_path"]))
            if not generated.is_file() or not decoded.is_file():
                raise ValueError(f"stale or missing production output for {row['record_id']}")
            generated_hash = sha256_file(generated)
            decoded_bytes = decoded.stat().st_size
            if sha256_file(decoded) != row["decoded_sha256"]:
                raise ValueError(f"decoded production hash drift for {row['record_id']}")
        if generated_hash != row["generated_sha256"] or decoded_bytes != row["decoded_bytes"]:
            raise ValueError(f"generated production hash or length drift for {row['record_id']}")
    if counts["SINNOH_NEW"] == 0 or counts["SINNOH_VARIANT"] == 0:
        raise ValueError("Sinnoh asset closure lacks new or variant payloads")
    declared = {
        row["checked_in_source_path"]
        for row in rows
        if row.get("asset_family") == "tileset_component"
        and row.get("checked_in_source_path", "").startswith("data/tilesets/sinnoh/")
    }
    actual = {
        path.relative_to(root).as_posix()
        for path in (root / "data/tilesets/sinnoh").rglob("*")
        if path.is_file() and path.suffix in {".png", ".pal", ".bin"}
    }
    if actual != declared:
        raise ValueError("checked-in Sinnoh source closure has undeclared or missing files")
    resolve_layout_aliases(root, manifest)
    return counts


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--donor-root", type=Path)
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--verify", action="store_true")
    parser.add_argument("--list-generated", action="store_true")
    parser.add_argument("--refresh-proofs", action="store_true")
    parser.add_argument("--generate", action="store_true")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    asset_path = root / "src/data/wayfarer_sinnoh_assets.json"
    if args.write:
        if args.donor_root is None:
            raise SystemExit("--write requires --donor-root")
        write_headers(root)
        manifest = build_manifest(root, args.donor_root.resolve())
        refresh_donor_proofs(root, args.donor_root.resolve(), manifest)
        asset_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
        maps_path = root / "src/data/wayfarer_sinnoh_maps.json"
        maps = load_json(maps_path)
        maps["selection"]["blockers"] = []
        maps["selection"]["reason"] = "Frozen source inventory and verified asset closure; no public travel, story, encounters, or map catalog is selected."
        for row in maps["maps"]:
            row["inclusion"]["reason"] = "Verified asset closure is ready; topology and catalog selection remain intentionally deferred."
        maps_path.write_text(json.dumps(maps, indent=2) + "\n", encoding="utf-8")
    manifest = load_json(asset_path)
    if args.generate:
        generate_production(root, manifest)
    if args.refresh_proofs:
        refresh_proofs(root, manifest)
        asset_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    if args.list_generated:
        paths = {
            row["generated_path"]
            for row in manifest["records"]
            if row.get("asset_family") == "tileset_component"
            and row.get("component") != "descriptor"
        }
        print("\n".join(sorted(paths)))
        return 0
    counts = verify(root, manifest) if args.verify else {}
    if args.output:
        args.output.write_text(json.dumps({"classes": counts}, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

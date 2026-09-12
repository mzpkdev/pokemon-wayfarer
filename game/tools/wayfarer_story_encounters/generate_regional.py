#!/usr/bin/env python3
"""Render compact regional story encounter registries from logical entry lists.

The source ``*_entries.inc`` files preserve every expanded logical record.  The
generated headers split callers from shared 32-byte descriptors so the ROM keeps
one descriptor for each identical tail while retaining source order.
"""

import argparse
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
DATA = ROOT / "src/data"
REGIONS = ("johto", "hoenn")


def calls(text, macro):
    """Return complete macro calls, including nested MAP_GROUP expressions."""
    pattern = re.compile(rf"\b{macro}\(")
    result = []
    for match in pattern.finditer(text):
        start = match.start()
        depth = 0
        for index in range(match.end() - 1, len(text)):
            if text[index] == "(":
                depth += 1
            elif text[index] == ")":
                depth -= 1
                if depth == 0:
                    result.append(text[start:index + 1])
                    break
        else:
            raise ValueError(f"unterminated {macro} call")
    return result


def arguments(call):
    body = call[call.index("(") + 1:-1]
    values = []
    depth = 0
    start = 0
    for index, char in enumerate(body):
        if char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
        elif char == "," and depth == 0:
            values.append(body[start:index].strip())
            start = index + 1
    values.append(body[start:].strip())
    return values


def logical_rows(region, text):
    prefix = "HOENN" if region == "hoenn" else f"WAYFARER_{region.upper()}"
    rows = []
    positions = []
    for kind in ("ENTRY", "SCENE"):
        macro = f"{prefix}_{kind}"
        for call in calls(text, macro):
            positions.append((text.index(call), kind, arguments(call)))
    for _, kind, values in sorted(positions):
        if kind == "ENTRY":
            if len(values) != 16:
                raise ValueError(f"{region}: entry has {len(values)} arguments")
            if region == "hoenn":
                caller, loss, trigger, key, scene, policy, dialogue, flags, map_name, local, elevation, width, height, x, y, eligible = values
                group, map_num = f"MAP_GROUP({map_name})", f"MAP_NUM({map_name})"
            else:
                caller, loss, key, scene, policy, dialogue, flags, group, map_num, local, elevation, width, height, x, y, eligible = values
                trigger = "NULL"
        else:
            if len(values) != 13:
                raise ValueError(f"{region}: scene has {len(values)} arguments")
            scene, policy, dialogue, flags, group, map_num, local, elevation, width, height, x, y, eligible = values
            caller, loss, trigger, key = "NULL", "NULL", "NULL", "0"
        descriptor = (loss, trigger, key, scene, policy, dialogue, flags, group, map_num, local, elevation, width, height, x, y, eligible)
        rows.append((caller, descriptor))
    return rows


def descriptor_initializer(descriptor):
    loss, trigger, key, scene, policy, dialogue, flags, group, map_num, local, elevation, width, height, x, y, eligible = descriptor
    return (f"{{ .lossRedirect = {loss}, .triggerScript = {trigger}, .stableKey = {key}, .sceneId = {scene}, "
            f".policy = {policy}, .dialogue = {dialogue}, .flags = {flags}, .mapGroup = {group}, .mapNum = {map_num}, "
            f".localId = {local}, .elevation = {elevation}, .activationWidth = {width}, .activationHeight = {height}, "
            f".x = {x}, .y = {y}, .isNarrativelyEligible = {eligible} }}")


def render(region, preamble, entries):
    rows = logical_rows(region, entries)
    descriptors = []
    index_by_descriptor = {}
    for _, descriptor in rows:
        if descriptor not in index_by_descriptor:
            index_by_descriptor[descriptor] = len(descriptors)
            descriptors.append(descriptor)
    if len(descriptors) > 255:
        raise ValueError(f"{region}: {len(descriptors)} descriptors exceed u8 index range")
    title = region.capitalize()
    lines = [preamble.rstrip(), "", f"const struct WayfarerStoryEncounterDescriptor gWayfarerStory{title}Descriptors[] =", "{"]
    lines += [f"    {descriptor_initializer(descriptor)}," for descriptor in descriptors]
    lines += ["};", f"const u32 gWayfarerStory{title}DescriptorCount = ARRAY_COUNT(gWayfarerStory{title}Descriptors);", f"const u8 *const gWayfarerStory{title}Callers[] =", "{"]
    lines += [f"    {caller}{'' if region == 'hoenn' else ' + 1'}," if caller != "NULL" else "    NULL," for caller, _ in rows]
    lines += ["};", f"const u8 gWayfarerStory{title}DescriptorIndices[] =", "{"]
    lines += [f"    {index_by_descriptor[descriptor]}," for _, descriptor in rows]
    lines += ["};", f"const u32 gWayfarerStory{title}EncounterCount = ARRAY_COUNT(gWayfarerStory{title}Callers);", "#else",
              f"const struct WayfarerStoryEncounterDescriptor gWayfarerStory{title}Descriptors[] = {{0}};",
              f"const u32 gWayfarerStory{title}DescriptorCount = 0;",
              f"const u8 *const gWayfarerStory{title}Callers[] = {{NULL}};",
              f"const u8 gWayfarerStory{title}DescriptorIndices[] = {{0}};",
              f"const u32 gWayfarerStory{title}EncounterCount = 0;", "#endif", ""]
    return "\n".join(lines), len(rows), len(descriptors)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    for region in REGIONS:
        preamble = (DATA / f"wayfarer_story_encounter_{region}_preamble.h").read_text()
        entries = (DATA / f"wayfarer_story_encounter_{region}_entries.inc").read_text()
        output, count, descriptor_count = render(region, preamble, entries)
        target = DATA / f"wayfarer_story_encounter_{region}.h"
        if args.check:
            if target.read_text() != output:
                raise ValueError(f"{region} regional registry is stale; run generator")
        else:
            target.write_text(output)
        print(f"Validated {region}: {count} logical rows, {descriptor_count} unique descriptors.")


if __name__ == "__main__":
    main()

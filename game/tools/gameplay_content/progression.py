"""Leaf progression source shared by runtime generation and domain audits."""
import json
from pathlib import Path
import re

SOURCE = Path(__file__).resolve().parents[2] / "data/gameplay/progression.json"
CURVE_IDS = ("wild_baseline", "ordinary_trainer_baseline", "soft_cap", "gym_baseline", "league_baseline")


def _object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _keys(value, keys):
    if not isinstance(value, dict) or set(value) != set(keys):
        raise ValueError(f"expected keys {sorted(keys)}")


def load(path=SOURCE):
    source = json.loads(Path(path).read_text(), object_pairs_hook=_object)
    _keys(source, ("schemaVersion", "curves"))
    if type(source["schemaVersion"]) is not int or source["schemaVersion"] != 1:
        raise ValueError("progression schemaVersion must be 1")
    if not isinstance(source["curves"], list):
        raise ValueError("curves must be an array")
    curves = {}
    for curve in source["curves"]:
        _keys(curve, ("id", "points", "interpolation"))
        name = curve["id"]
        if not isinstance(name, str) or not re.fullmatch(r"[a-z][a-z0-9_]*", name) or name in curves:
            raise ValueError("invalid or duplicate curve ID")
        if curve["interpolation"] != "nearest_half_up":
            raise ValueError(f"{name}: unsupported interpolation")
        if not isinstance(curve["points"], list) or not 2 <= len(curve["points"]) <= 81:
            raise ValueError(f"{name}: expected 2..81 points")
        points = []
        for point in curve["points"]:
            _keys(point, ("rating", "value"))
            rating, value = point["rating"], point["value"]
            if type(rating) is not int or type(value) is not int or not 0 <= rating <= 80 or not 0 <= value <= 100:
                raise ValueError(f"{name}: invalid integer point")
            if points and (rating <= points[-1][0] or value < points[-1][1]):
                raise ValueError(f"{name}: points must increase in Rating and not decrease in value")
            points.append((rating, value))
        if points[0][0] != 0 or points[-1][0] != 80:
            raise ValueError(f"{name}: endpoints must be 0 and 80")
        curves[name] = tuple(points)
    if not set(CURVE_IDS) <= curves.keys() or len(curves) > 65535:
        raise ValueError("missing required curves or too many curves")
    return curves


def evaluate(name, rating, curves=None):
    points = (load() if curves is None else curves)[name]
    rating = min(max(rating, 0), 80)
    for (r0, v0), (r1, v1) in zip(points, points[1:]):
        if rating <= r1:
            return v0 + (2 * (rating-r0) * (v1-v0) + r1-r0) // (2 * (r1-r0))


def render_runtime(curves=None):
    curves = load() if curves is None else curves
    # These public baseline helpers remain callable in standalone and disabled
    # builds; domain policy decides whether a battle uses their result. Wild
    # consumes the host points through its existing generated projection.
    runtime_ids = CURVE_IDS[1:]
    payloads = list(dict.fromkeys(curves[name] for name in runtime_ids))
    lines = []
    for index, points in enumerate(payloads):
        lines.append(f"static const u8 sProgressionPoints{index}[][2] = {{")
        lines.extend(f"    {{ {r}, {v} }}," for r, v in points)
        lines.append("};")
    lines.append("static const struct ProgressionCurve sProgressionCurves[] = {")
    for name in runtime_ids:
        index = payloads.index(curves[name])
        lines.append(f"    [GAMEPLAY_CURVE_{name.upper()}] = {{ sProgressionPoints{index}, ARRAY_COUNT(sProgressionPoints{index}) }},")
    lines.append("};")
    return "\n".join(lines) + "\n"

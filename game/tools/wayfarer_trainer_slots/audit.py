"""Dialogue audit: which lines of a rotating candidate name a map, landmark, building or local event.

A wandering trainer speaks its own lines in any slot of its group, so a line about the place it was written for
reads wrong elsewhere. The audit reads every intro, defeat and post-battle line of each candidate, matches it against
the map name table (and the names' stems) and a reviewed landmark word list, and flags the trainers that need a
reviewed resolution: neutral rewording or an exclusion from rotation.
"""
from __future__ import annotations

import json
from pathlib import Path
import re

TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[1]
MAP_NAMES = ROOT / 'src/data/region_map/region_map_entries.h'
LANDMARKS = TOOL / 'landmarks.json'
STEM_SUFFIX = re.compile(r'^(.+?) (CITY|TOWN|ISLAND|ISLE|ISLANDS|CAVERN|CHAMBER|FALLS|FOREST|TUNNEL|LAKE)$')


def load_words(path=LANDMARKS):
    return json.loads(Path(path).read_text())


def map_names(path=MAP_NAMES):
    names = set(re.findall(r'\.name = COMPOUND_STRING\("([^"]+)"\)', Path(path).read_text()))
    return {name.replace('{AQUA}', 'AQUA') for name in names}


def alternation(words):
    return '|'.join(re.escape(word) for word in sorted(words, key=len, reverse=True))


def build_patterns(names, words):
    """(patterns, allowed): [(kind, compiled regex)] for the map names, their stems, landmark phrases, terrain
    after a demonstrative, place phrases and local-event words; plus one regex of the reviewed phrases that cancel a match."""
    patterns = []
    stems = set()
    for name in names:
        match = STEM_SUFFIX.match(name)
        if match and match.group(1) not in words['stem_stoplist'] and len(match.group(1)) >= 5:
            stems.add(match.group(1))
    full = (names | stems)
    # A space may be missing or doubled in the scripts ("MT.MOON" for "MT. MOON").
    spaced = '|'.join(re.escape(name).replace(r'\ ', r'\s*') for name in sorted(full, key=len, reverse=True))
    patterns.append(('map name', re.compile(r'(?<![A-Za-z])(?:' + spaced + r')(?![A-Za-z])', re.I)))
    # Stems that are ordinary words in lower case ("victory") count only when written in capitals, as a name.
    patterns.append(('map name', re.compile(r'(?<![A-Za-z])(?:' + alternation(words['capital_stems']) + r')(?![A-Za-z])')))
    determiners = '|'.join(words['determiners'])
    patterns.append(('landmark', re.compile(r'\b(?:' + determiners + r')\s+(?:(?!(?:the|a|an)\b)[A-Za-z\'-]+\s+)?(?:' + alternation(words['landmark_nouns']) + r')\b', re.I)))
    patterns.append(('terrain', re.compile(r'\b(?:' + '|'.join(words['demonstratives']) + r')\s+(?:(?!(?:the|a|an)\b)[A-Za-z\'-]+\s+)?(?:' + alternation(words['terrain_nouns']) + r')\b', re.I)))
    patterns.append(('place phrase', re.compile(r'\b(?:' + alternation(words['place_phrases']) + r')\b', re.I)))
    # Case-sensitive: the scripts write proper nouns in capitals ("the GYM"), so "no contest" or "out of my league" is not an event.
    patterns.append(('local event', re.compile(r'(?<![A-Za-z])(?:' + alternation(words['local_events']) + r')(?![A-Za-z])')))
    # Phrases that tie a line to the spot it is spoken at without naming it ("rests here", "ends here", "this is where").
    patterns.append(('spot phrase', re.compile('|'.join(words['spot_phrases']), re.I)))
    patterns.append(('sequence', re.compile('|'.join(words['sequence_patterns']))))
    allowed = re.compile('|'.join(f"(?:{row['pattern']})" for row in words['allowed_phrases']))
    return patterns, allowed


def flag_text(text, built):
    """[(kind, matched text)] for one line."""
    patterns, allowed = built
    cancelled = [match.span() for match in allowed.finditer(text)]
    found = []
    for kind, pattern in patterns:
        for match in pattern.finditer(text):
            if any(start <= match.start() and match.end() <= end for start, end in cancelled):
                continue
            found.append((kind, match.group(0)))
    return found

#!/usr/bin/env python3
"""Read-only native Wayfarer flash audit. Layout is from actual ARM save-offsets.s.

Audits 32 4096-byte flash sectors (ignores/reports trailing RTC bytes). Implements
save.c's main-data checksum and full-slot ID test; additionally reports generation
coherence, which GetSaveValidStatus does NOT itself enforce. SaveBlock3 spare
chunks are outside the game's checksum and are explicitly labeled unprotected.
No flash bytes, save data or repository files are ever changed.
"""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

NAMES = ('sb1_size sb2_size sb3_size bag_offset bag_size ledger_offset flags_offset flags_size vars_offset vars_size party_offset party_size party_count_offset frontier_offset frontier_size pc_size encryption_key_offset medicine_offset medicine_count potion_id').split()
DEFAULT_LAYOUT = str(Path(__file__).with_name('save-offsets.s'))
DATA_SIZE = 3968
SECTOR_SIZE = 4096
SIGNATURE = 0x08012025


def sha(data):
    return hashlib.sha256(data).hexdigest()


def load_layout(path):
    raw = Path(path).read_bytes()
    words = [int(s, 0) for s in re.findall(rb'^\s*\.word\s+([0-9xa-fA-F]+)', raw, re.M)]
    if len(words) != len(NAMES):
        raise ValueError(f'Expected {len(NAMES)} layout words, got {len(words)}')
    return dict(zip(NAMES, words)), sha(raw)


def checksum(data):
    n = len(data) // 4
    total = sum(struct.unpack('<' + 'I' * n, data[:n * 4])) & 0xffffffff
    return ((total >> 16) + total) & 0xffff


def ranges(data, layout):
    s1, s2, s3, pc = data
    out = {'saveblock1': s1, 'saveblock2': s2, 'saveblock3_unchecksummed': s3, 'pc': pc}
    for name in ('bag', 'flags', 'vars', 'party'):
        offset, length = layout[name + '_offset'], layout[name + '_size']
        out[name] = s1[offset:offset + length]
    out['ledger'] = s1[layout['ledger_offset']:layout['ledger_offset'] + 2]
    out['party_count'] = s1[layout['party_count_offset']:layout['party_count_offset'] + 1]
    out['frontier'] = s2[layout['frontier_offset']:layout['frontier_offset'] + layout['frontier_size']]
    return out


def decode(path, layout):
    raw = Path(path).read_bytes()
    if len(raw) < 32 * SECTOR_SIZE:
        raise ValueError(f'{path}: too short for 128 KiB flash: {len(raw)}')
    sizes = [layout['sb2_size']]
    for name, chunks in [('sb1_size', 4), ('pc_size', 9)]:
        sizes.extend(max(0, min(DATA_SIZE, layout[name] - i * DATA_SIZE)) for i in range(chunks))
    slots = []
    sector_data = {}
    for slot in range(2):
        sectors = []
        good = {}
        last_counter = 0
        for physical in range(slot * 14, (slot + 1) * 14):
            sec = raw[physical * SECTOR_SIZE:(physical + 1) * SECTOR_SIZE]
            ident, stored, signature, counter = struct.unpack_from('<HHII', sec, 4084)
            calculated = checksum(sec[:sizes[ident]]) if ident < 14 else None
            valid = signature == SIGNATURE and calculated == stored and ident < 14
            sectors.append(dict(physical=physical, id=ident, counter=counter,
                                signature=f'{signature:08x}', checksum_stored=stored,
                                checksum_calculated=calculated, valid=valid))
            if valid:
                good[ident] = sec
                last_counter = counter
        full = len(good) == 14
        generations = sorted({s['counter'] for s in sectors if s['valid']})
        any_signature = any(s['signature'] == f'{SIGNATURE:08x}' for s in sectors)
        slots.append(dict(slot=slot, status='OK' if full else 'ERROR' if any_signature else 'EMPTY',
                          full_id_bitset=full, generations=generations,
                          coherent_generation=full and len(generations) == 1,
                          loader_counter=last_counter, sectors=sectors))
        sector_data[slot] = good
    valid_slots = [s for s in slots if s['full_id_bitset']]
    if not valid_slots:
        raise ValueError(f'{path}: no complete valid slot: ' + json.dumps(slots))
    if len(valid_slots) == 2:
        a, b = (s['loader_counter'] for s in valid_slots)
        if {a, b} == {0, 0xffffffff}:
            chosen_counter = 0
        else:
            chosen_counter = max(a, b)
    else:
        chosen_counter = valid_slots[0]['loader_counter']
    chosen = chosen_counter % 2  # CopySaveSlotData uses the counter parity.
    sec = sector_data[chosen]
    if len(sec) != 14:
        raise ValueError(f'{path}: loader counter selects incomplete slot {chosen}')
    sb2 = sec[0][:layout['sb2_size']]
    sb1 = b''.join(sec[i][:DATA_SIZE] for i in range(1, 5))[:layout['sb1_size']]
    pc = b''.join(sec[i][:DATA_SIZE] for i in range(5, 14))[:layout['pc_size']]
    sb3 = b''.join(sec[i][DATA_SIZE:4084] for i in range(14))[:layout['sb3_size']]
    decoded_ranges = ranges((sb1, sb2, sb3, pc), layout)
    key = struct.unpack_from('<I', sb2, layout['encryption_key_offset'])[0] & 0xffff
    bag = decoded_ranges['bag']
    decoded_bag = []
    totals = {}
    for i in range(len(bag) // 4):
        ident, encoded = struct.unpack_from('<HH', bag, i * 4)
        quantity = encoded ^ key
        decoded_bag.append((ident, quantity))
        if ident:
            totals[ident] = totals.get(ident, 0) + quantity
    med_potions = 0
    for i in range(layout['medicine_count']):
        ident, encoded = struct.unpack_from('<HH', sb1, layout['medicine_offset'] + i * 4)
        if ident == layout['potion_id']:
            med_potions += encoded ^ key
    party = decoded_ranges['party']
    flags = decoded_ranges['flags']
    var_values = struct.unpack('<' + 'H' * (len(decoded_ranges['vars']) // 2), decoded_ranges['vars'])
    report = dict(path=str(Path(path).resolve()), bytes=len(raw), sha256=sha(raw),
                  rtc_trailer_bytes=len(raw) - 32 * SECTOR_SIZE,
                  slots=slots, selected_slot=chosen, selected_counter=chosen_counter,
                  selected_coherent=slots[chosen]['coherent_generation'],
                  potion_count=med_potions, ledger=list(decoded_ranges['ledger']),
                  bag_item_totals=totals, bag_encryption_key_low16=key,
                  party_count=decoded_ranges['party_count'][0],
                  party_mon_sha256=[sha(party[i:i+100]) for i in range(0,len(party),100)],
                  flags_set=[i*8+bit for i,v in enumerate(flags) for bit in range(8) if v & (1<<bit)],
                  vars_nonzero={str(0x4000+i): v for i,v in enumerate(var_values) if v},
                  ranges={k: dict(bytes=len(v), sha256=sha(v)) for k,v in decoded_ranges.items()},
                  notes=['SaveBlock3 chunks are NOT covered by sector checksums.',
                         'Generation coherence is stronger than the game loader full-ID-bitset check.',
                         'Party/frontier report raw reconstructed bytes, not battle-semantic interpretation.'])
    return report, decoded_ranges, decoded_bag


def diff_ranges(before, after):
    result = {}
    for name in before:
        a, b = before[name], after[name]
        changed = [i for i in range(min(len(a), len(b))) if a[i] != b[i]]
        result[name] = dict(equal=a == b, changed_byte_count=len(changed),
                            first_changed_offsets=changed[:64],
                            before_sha256=sha(a), after_sha256=sha(b))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('save')
    parser.add_argument('--layout', default=DEFAULT_LAYOUT)
    parser.add_argument('--compare')
    parser.add_argument('--output', help='Write JSON here, otherwise stdout')
    parser.add_argument('--extract', help='Write reconstructed named ranges under this directory')
    parser.add_argument('--expect-potion', type=int)
    parser.add_argument('--expect-ledger', help='Two comma-separated byte values (decimal or 0x hex)')
    parser.add_argument('--unchanged', default='', help='Comma-separated range names, requires --compare')
    parser.add_argument('--require-coherent', action='store_true')
    args = parser.parse_args()
    layout, layout_sha = load_layout(args.layout)
    result, blocks, bag = decode(args.save, layout)
    result['layout'] = layout
    result['layout_sha256'] = layout_sha
    errors = []
    if args.compare:
        baseline, base_blocks, base_bag = decode(args.compare, layout)
        result['comparison'] = dict(baseline=baseline['path'], baseline_sha256=baseline['sha256'],
                                    ranges=diff_ranges(base_blocks, blocks),
                                    bag_logical_equal=base_bag == bag,
                                    potion_delta=result['potion_count']-baseline['potion_count'],
                                    before_ledger=baseline['ledger'], after_ledger=result['ledger'])
        for name in filter(None, args.unchanged.split(',')):
            if name not in blocks:
                errors.append(f'Unknown range {name}')
            elif base_blocks[name] != blocks[name]:
                errors.append(f'{name} differs from baseline')
    elif args.unchanged:
        errors.append('--unchanged requires --compare')
    if args.expect_potion is not None and result['potion_count'] != args.expect_potion:
        errors.append(f"Potion count expected {args.expect_potion}, got {result['potion_count']}")
    if args.expect_ledger:
        expected = [int(v,0) for v in args.expect_ledger.split(',')]
        if len(expected) != 2 or result['ledger'] != expected:
            errors.append(f"Ledger expected {expected}, got {result['ledger']}")
    if args.require_coherent and not result['selected_coherent']:
        errors.append('Selected slot contains mixed generations')
    result['assertions'] = dict(passed=not errors, errors=errors)
    if args.extract:
        target = Path(args.extract)
        target.mkdir(parents=True, exist_ok=True)
        for name, data in blocks.items():
            (target / (name + '.bin')).write_bytes(data)
    output = json.dumps(result, indent=2) + '\n'
    if args.output:
        Path(args.output).write_text(output)
    else:
        print(output, end='')
    if errors:
        parser.exit(1, '\n'.join(errors) + '\n')

if __name__ == '__main__':
    main()

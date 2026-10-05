#!/usr/bin/env python3
"""Add modest, reproducible Tornie spice pockets to supplied maps.

Normal terrain rows are edited directly, preserving every other byte. Legacy
seed-based scenarios opt into the equivalent loader rule without rebasing their
unit, building or reinforcement coordinates. Re-running is safe: marked maps
are checked and never converted twice.
"""

import argparse
import hashlib
from pathlib import Path
import re


SECTION = re.compile(rb"^[ \t]*\[MAP\][ \t]*\r?\n(.*?)(?=^[ \t]*\[|\Z)", re.I | re.M | re.S)
ROW = re.compile(rb"^([ \t]*(\d{3})[ \t]*=[ \t]*)([^\r\n]*)", re.M)
MARKER = b'; TornieSpiceVariants v'


def mix(value):
    value ^= value >> 16
    value = value * 0x7FEB352D & 0xFFFFFFFF
    value ^= value >> 15
    value = value * 0x846CA68B & 0xFFFFFFFF
    return value ^ (value >> 16)


def apply(terrain, width, percentage, seed, version=1):
    """Same fixed integer algorithm as include/INIMap/SpiceVariants.h."""
    terrain = bytearray(terrain)
    candidates = [i for i, tile in enumerate(terrain) if tile in b'~+']
    target = len(candidates) * max(0, min(percentage, 15)) // 100
    candidates.sort(key=lambda index: (mix(seed ^ index), index))
    converted = pocket_number = 0
    height = len(terrain) // width
    for center in candidates:
        if converted == target:
            break
        if terrain[center] not in b'~+':
            continue
        rank = mix(seed ^ center)
        green = (rank >> 8) & 1
        color = (seed + pocket_number) % 4
        pocket_number += 1
        pocket_size = min(3 + rank % 5, max(1, target // 4)) if version >= 2 else 3 + rank % 5
        pending = [center]
        cursor = pocket_count = 0
        while cursor < len(pending) and pocket_count < pocket_size and converted < target:
            index = pending[cursor]
            cursor += 1
            original = terrain[index]
            if original not in b'~+':
                continue
            terrain[index] = (b'rglw'[color] if original == ord('~') else b'RGLW'[color]) if version >= 2 else ((ord('g') if green else ord('r')) if original == ord('~') else (ord('G') if green else ord('R')))
            converted += 1
            pocket_count += 1
            x, y = index % width, index // width
            cx, cy = center % width, center // width
            for step in range(4):
                direction = (step + (rank >> 4) % 4) % 4
                dx, dy = [(1, 0), (0, 1), (-1, 0), (0, -1)][direction]
                nx, ny = x + dx, y + dy
                if 0 <= nx < width and 0 <= ny < height and abs(nx - cx) + abs(ny - cy) <= 2:
                    pending.append(ny * width + nx)
    return bytes(terrain)


def integer(section, key, default=None):
    match = re.search(rb'^[ \t]*' + key + rb'[ \t]*=[ \t]*(-?\d+)', section, re.I | re.M)
    return int(match[1]) if match else default


def rows(section):
    width, height = integer(section, b'SizeX'), integer(section, b'SizeY')
    assert width and height and width > 0 and height > 0, 'Invalid map dimensions'
    matches = list(ROW.finditer(section))
    found = {int(match[2]): match for match in matches}
    assert len(matches) == height and set(found) == set(range(height)), 'Missing or duplicate map rows'
    terrain = bytearray()
    for y in range(height):
        value = found[y][3]
        assert len(value) == width, f'Invalid length for row {y}: {len(value)} != {width}'
        terrain.extend(value)
    return width, height, found, bytes(terrain)


def process(path, root, write, check):
    raw = path.read_bytes()
    section_match = SECTION.search(raw)
    assert section_match, 'Missing MAP section'
    section = section_match[1]
    relative = path.relative_to(root).as_posix()
    seed = int.from_bytes(hashlib.sha256(relative.encode('utf-8')).digest()[:4], 'little') & 0x7FFFFFFF
    modern = integer(section, b'SizeX') is not None
    if MARKER in section:
        marker = re.search(rb'TornieSpiceVariants v([12]): percent=(\d+) seed=(\d+) original=(\d+) variants=(\d+)', section)
        assert marker and int(marker[2]) == 10 and int(marker[3]) == seed, 'Invalid variant marker'
        version = int(marker[1])
        if write and version == 1:
            section = re.sub(rb'^; TornieSpiceVariants v1:[^\r\n]*\r?\n', b'', section, flags=re.M)
            if modern:
                width, height, found, terrain = rows(section)
                terrain = terrain.translate(bytes.maketrans(b'gGrR', b'~+~+'))
                changed = apply(terrain, width, 10, seed, 2)
                for y in reversed(range(height)):
                    match = found[y]
                    section = section[:match.start(3)] + changed[y*width:(y+1)*width] + section[match.end(3):]
                original, variants = int(marker[4]), int(marker[5])
            else:
                original = variants = 0
                newline = b'\r\n' if b'\r\n' in raw else b'\n'
                section = b'SpiceVariantVersion=2' + newline + section
            newline = b'\r\n' if b'\r\n' in raw else b'\n'
            mark = f'; TornieSpiceVariants v2: percent=10 seed={seed} original={original} variants={variants}'.encode()+newline
            path.write_bytes(raw[:section_match.start(1)] + mark + section + raw[section_match.end(1):])
            return 'upgraded-grid' if modern else 'upgraded-seed'
        if modern:
            _, _, _, terrain = rows(section)
            original, variants = int(marker[4]), int(marker[5])
            assert variants <= original, 'Invalid variant budget'
            assert sum(terrain.count(tile) for tile in b'~+gGrRlLwW') == original, 'Spice tile count changed'
            assert sum(terrain.count(tile) for tile in b'gGrRlLwW') == variants, 'Variant tile count changed'
            if version >= 2 and variants >= 4:
                assert all(any(tile in terrain for tile in pair) for pair in (b'gG', b'rR', b'lL', b'wW')), 'A spice family is absent'
        else:
            assert integer(section, b'SpiceVariantPercent') == 10 and integer(section, b'SpiceVariantSeed') == seed
            if version >= 2: assert integer(section, b'SpiceVariantVersion') == 2
        return 'checked-grid' if modern else 'checked-seed'
    assert not check, 'Map has not been converted'
    newline = b'\r\n' if b'\r\n' in raw else b'\n'
    if modern:
        width, height, found, terrain = rows(section)
        original = sum(terrain.count(tile) for tile in b'~+gGrR')
        variants = sum(terrain.count(tile) for tile in b'gGrR')
        changed = apply(terrain, width, 10, seed, 2)
        variants += sum(old != new for old, new in zip(terrain, changed))
        for y in reversed(range(height)):
            match = found[y]
            section = section[:match.start(3)] + changed[y * width:(y + 1) * width] + section[match.end(3):]
    else:
        assert integer(section, b'Seed') is not None, 'Unknown map format'
        original = variants = 0
        section = f'SpiceVariantVersion=2\nSpiceVariantPercent=10\nSpiceVariantSeed={seed}\n'.encode().replace(b'\n', newline) + section
    marker = f'; TornieSpiceVariants v2: percent=10 seed={seed} original={original} variants={variants}'.encode() + newline
    updated = raw[:section_match.start(1)] + marker + section + raw[section_match.end(1):]
    if write:
        path.write_bytes(updated)
    return 'converted-grid' if modern else 'converted-seed'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--write', action='store_true', help='Apply changes (default is dry run)')
    mode.add_argument('--check', action='store_true', help='Check markers, geometry and spice counts')
    parser.add_argument('--maps', type=Path, default=Path(__file__).resolve().parents[1] / 'data' / 'maps')
    args = parser.parse_args()
    counts = {}
    for path in sorted(args.maps.rglob('*')):
        if path.suffix.lower() != '.ini':
            continue
        try:
            outcome = process(path, args.maps, args.write, args.check)
        except (AssertionError, TypeError) as error:
            raise SystemExit(f'{path}: {error}') from error
        counts[outcome] = counts.get(outcome, 0) + 1
    print(('Written' if args.write else 'Checked' if args.check else 'Dry run') + ': ' + str(counts))


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Keep declared campaign intros identical except for opponent ownership."""
from pathlib import Path
import configparser
import hashlib
import json
import re

ROOT = Path(__file__).resolve().parents[1]
OWNED_SECTIONS = {"UNITS", "STRUCTURES", "TEAMS", "REINFORCEMENTS"}


def parse(data):
    ini = configparser.ConfigParser(interpolation=None, strict=False)
    ini.optionxform = str
    ini.read_string("\n".join(line for line in data.decode("cp850").splitlines()
                              if "=" in line or line.strip().startswith(("[", ";", "#"))))
    return ini


def remap_owners(data, mapping):
    # Only house section names and ownership columns change. Unit names,
    # terrain, coordinates, orders, pictures and arbitrary other config stay
    # byte-for-byte identical to the source intro.
    result = []
    section = None
    for line in data.decode("cp850").splitlines(keepends=True):
        header = re.match(r"^(\s*)\[([^]]+)\]", line)
        if header:
            section = header[2]
            if section in mapping:
                line = line[:header.start(2)] + mapping[section] + line[header.end(2):]
        elif section in OWNED_SECTIONS and "=" in line and not line.lstrip().startswith((";", "#")):
            key, value = line.split("=", 1)
            owner = re.match(r"^(\s*)([^,]+)(\s*,)", value)
            if owner and owner[2].strip() in mapping:
                old = owner[2]
                new = mapping[old.strip()]
                trailing = old[len(old.rstrip()):]
                value = value[:owner.start(2)] + new + trailing + value[owner.end(2):]
                line = key + "=" + value
        result.append(line)
    return "".join(result).encode("cp850")


def clone_openings(plans=None, *, write=False):
    plans = plans or json.loads((ROOT / "config/CampaignPlans.json").read_text(encoding="utf-8"))
    reports = []
    for target_mod, source_mod in plans.get("openingClones", {}).items():
        source_rows = {row["house"]: row for row in plans["mods"][source_mod]}
        for target in plans["mods"][target_mod]:
            player = target["house"]
            source = source_rows[player]
            assert source["opponents"][0] != target["opponents"][0], (target_mod, player, "same first opponent")
            mapping = {player: player, **dict(zip(source["opponents"], target["opponents"]))}
            assert len(set(mapping.values())) == len(mapping), (target_mod, player, "merged ownership")
            source_path = ROOT / "mods" / source_mod / "campaign" / f'scen{source["letter"].lower()}001.ini'
            target_name = f'scen{target["letter"].lower()}001.ini'
            target_path = ROOT / "mods" / target_mod / "campaign" / target_name
            original = source_path.read_bytes()
            expected = remap_owners(original, mapping)
            before, after = parse(original), parse(expected)
            inverse = {new: old for old, new in mapping.items()}
            assert remap_owners(expected, inverse) == original, (target_mod, player, "not a pure ownership clone")
            for section in ("BASIC", "MAP", player):
                assert dict(before[section]) == dict(after[section]), (target_mod, player, section)
            for section in OWNED_SECTIONS:
                if section not in before:
                    continue
                assert set(before[section]) == set(after[section]), (target_mod, player, section, "missing orders")
                for key, value in before[section].items():
                    old = value.split(",")
                    new = after[section][key].split(",")
                    assert mapping[old[0].strip()] == new[0].strip() and old[1:] == new[1:], (target_mod, player, section, key)
            enemies = {section for section in before.sections() if section in mapping and section != player}
            assert enemies and enemies <= set(source["opponents"]), (source_mod, player, "unknown opponents")
            assert {mapping[enemy] for enemy in enemies} != enemies, (target_mod, player, "opponents did not change")
            if write:
                target_path.write_bytes(expected)
            assert target_path.read_bytes() == expected, (target_mod, player, "intro differs from source beyond opponents")
            mirror = ROOT / "mods" / target_mod / "data" / target_name
            if mirror.exists():
                if write:
                    mirror.write_bytes(expected)
                assert mirror.read_bytes() == expected, (target_mod, player, "data mirror differs")
            reports.append({"mod": target_mod, "sourceMod": source_mod, "house": player,
                            "mission": 1, "opponents": sorted(mapping[enemy] for enemy in enemies),
                            "sourceSha256": hashlib.sha256(original).hexdigest(),
                            "cloneSha256": hashlib.sha256(expected).hexdigest(),
                            "terrainEconomyForcesAndOrdersIdentical": True})
    return reports


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true", help="clone only declared first missions")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    result = {"introClones": clone_openings(write=args.write)}
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))

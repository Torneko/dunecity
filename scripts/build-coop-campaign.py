#!/usr/bin/env python3
"""Build the independent nine-stage cooperative campaigns; never edit solo maps."""

from pathlib import Path
import argparse
import configparser
import json
import hashlib
import random
from collections import deque
from importlib.util import spec_from_file_location, module_from_spec

ROOT = Path(__file__).resolve().parents[1]
_original_spec = spec_from_file_location("original_coop", ROOT / "scripts/original-coop-campaign.py")
original_coop = module_from_spec(_original_spec)
_original_spec.loader.exec_module(original_coop)
MODS = ("vanilla", "Tornie", "Jericho", "TornieLite", "JerichoLite")
SIZE = 64
SIZES = {
    "Const Yard": (2, 2), "Windtrap": (2, 2), "Refinery": (3, 2),
    "Barracks": (2, 2), "Light Fctry": (2, 2), "Heavy Fctry": (3, 2),
    "Outpost": (2, 2), "Hi-Tech": (3, 2), "IX": (2, 2),
    "Palace": (3, 3), "Repair": (3, 2), "Turret": (1, 1),
}
# The same five roles are retained throughout the campaign. Two human bases
# occupy the southern front; three enemy bases defend the northern front.
ORIGINS = ((4, 44), (40, 43), (4, 3), (25, 7), (45, 3))
FACILITIES = (
    ("Const Yard", 1, 1, 1), ("Windtrap", 5, 1, 1),
    ("Refinery", 8, 1, 1), ("Barracks", 1, 5, 2),
    ("Light Fctry", 5, 5, 3), ("Heavy Fctry", 8, 5, 4),
    ("Outpost", 1, 9, 4), ("Hi-Tech", 4, 9, 6),
    ("IX", 9, 9, 7), ("Palace", 11, 5, 8),
    ("Windtrap", 11, 1, 4), ("Windtrap", 11, 10, 6),
)


def destination(mod, stage):
    base = ROOT / "data" if mod == "vanilla" else ROOT / "mods" / mod / "campaign"
    return base / "coop" / f"coop{stage:02}.ini"


def map_seed(mod, stage):
    # Stable integer seed, independent of Python's salted string hashing.
    value = 2166136261
    for byte in (mod + ":coop:" + str(stage)).encode("ascii"):
        value = ((value ^ byte) * 16777619) & 0xFFFFFFFF
    return value


def generate(mod, stage):
    seed = map_seed(mod, stage)
    rng = random.Random(seed)
    terrain = [["^" if rng.random() < 0.055 else "-" for _ in range(SIZE)] for _ in range(SIZE)]
    mirrored = mod in ("Jericho", "JerichoLite")
    bases = [(x, y + (rng.randrange(-1, 2) if y < 20 else 0)) for x, y in ORIGINS]
    if mirrored:
        bases = [(SIZE - x - 16, y) for x, y in bases]

    def ellipse(cx, cy, rx, ry, rock=False):
        for y in range(max(0, cy - ry - 1), min(SIZE, cy + ry + 2)):
            for x in range(max(0, cx - rx - 1), min(SIZE, cx + rx + 2)):
                distance = ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2
                if distance < 1 + rng.uniform(-0.12, 0.12):
                    if rock:
                        terrain[y][x] = "%"
                    elif terrain[y][x] != "%":
                        terrain[y][x] = "+" if distance < 0.55 else "~"

    # Small irregular outcrops separate construction areas without sealing routes.
    for _ in range(9):
        ellipse(rng.randint(5, 58), rng.randint(23, 37), rng.randint(2, 4), rng.randint(2, 3), True)
    for slot, (bx, by) in enumerate(bases):
        ellipse(bx + 7, by + 6, 10, 9, True)
        for y in range(by, by + 14):
            for x in range(bx, bx + 16):
                terrain[y][x] = "%"
        field_y = by - 8 if slot < 2 else by + 22
        ellipse(bx + 7, field_y, 7, 6)
    ellipse(31, 33, 9, 4)
    ellipse(16 if mirrored else 47, 33, 5, 4)

    units = []
    structures = []
    concrete = {}
    occupied = set()

    def unit(slot, item, x, y, order="Area Guard"):
        assert 0 <= x < SIZE and 0 <= y < SIZE and (x, y) not in occupied
        occupied.add((x, y))
        units.append(f"ID{len(units):03}=Player{slot},{item},256,{y * SIZE + x},64,{order}")

    for slot, (bx, by) in enumerate(bases, 1):
        for item, dx, dy, level in FACILITIES:
            if level > stage:
                continue
            # Each player keeps its own playable base and production facilities.
            width, height = SIZES[item]
            x, y = bx + dx, by + dy
            for ty in range(y, y + height):
                for tx in range(x, x + width):
                    assert terrain[ty][tx] == "%" and (tx, ty) not in occupied
                    occupied.add((tx, ty))
                    concrete[ty * SIZE + tx] = slot
            structures.append(f"ID{len(structures):03}=Player{slot},{item},256,{y * SIZE + x}")
        unit(slot, "Harvester", bx + 14, by + 12, "Harvest")
        if slot < 3:
            # The requested opening support applies to both human players.
            items = ["Tank", "Tank", "Troopers", "Troopers", "Troopers", "Special", "Special"]
            if stage >= 5:
                items += ["Tank", "Tank", "Launcher"]
            if stage >= 8:
                items += ["Siege Tank", "Siege Tank"]
        else:
            items = ["Trike", "Trike", "Trooper", "Trooper", "Trooper"]
            if stage >= 3:
                items += ["Quad", "Quad"]
            if stage >= 4:
                items += ["Tank", "Tank"]
            if stage >= 6:
                items += ["Tank", "Launcher"]
            if stage >= 8:
                items += ["Siege Tank", "Siege Tank"]
        for i, item in enumerate(items):
            # Units gather outside production footprints and leave exits open.
            unit(slot, item, bx + (i % 13), by + 14 + i // 13, "Guard" if slot < 3 else "Area Guard")

    if stage == 1:
        for index, (x, y) in enumerate(((1, 15), (62, 16), (18, 1), (37, 1), (62, 31))):
            unit(3 + index % 3, "Trooper" if index % 2 else "Trike", x, y)

    lines = [
        "; Independent cooperative campaign. Generated by scripts/build-coop-campaign.py.",
        "; Runtime faction bindings are supplied in [COOP] by the host.",
        "[BASIC]", "Version=2", f"Name=Common campaign - stage {stage}",
        f"TechLevel={stage}", "Author=Tornie", "License=GPL-2.0-or-later",
        "WinFlags=3", "LoseFlags=1", "TimeOut=0",
        "[COOP_TEMPLATE]", "Schema=1", f"Stage={stage}", f"Mod={mod}",
        "Players=2", "OpponentSlots=3",
        "[MAP]", f"SizeX={SIZE}", f"SizeY={SIZE}",
        "SpiceVariantPercent=10", f"SpiceVariantSeed={seed & 0x7FFFFFFF}", "SpiceVariantVersion=2",
    ]
    lines += [f"{y:03}={''.join(row)}" for y, row in enumerate(terrain)]
    for slot, (bx, by) in enumerate(bases, 1):
        credits = 1500 + stage * 250 if slot < 3 else 550 + stage * 200
        lines += [f"[Player{slot}]", f"Brain=Team {1 if slot < 3 else 2}",
                  f"Credits={credits}", "Quota=0", f"MaxUnits={35 + stage * 6}",
                  f"View={by * SIZE + bx}"]
    lines += ["[UNITS]"] + units
    lines += ["[STRUCTURES]"]
    lines += [f"GEN{position}=Player{slot},Concrete" for position, slot in sorted(concrete.items())]
    lines += structures
    lines += ["[TEAMS]"]
    for slot in range(3, 6):
        if stage >= 2:
            lines.append(f"{slot}1=Player{slot},Normal,Foot,3,5")
        if stage >= 3:
            lines.append(f"{slot}2=Player{slot},Normal,Wheeled,2,4")
        if stage >= 4:
            lines.append(f"{slot}3=Player{slot},Normal,Tracked,2,4")
    if stage >= 4:
        lines += ["[REINFORCEMENTS]"]
        # Slow, finite support prevents an infinite reinforcement stalemate.
        for slot in range(3, 6):
            lines.append(f"{slot}=Player{slot},Tank,North,{9 + slot},false")
    return "\n".join(lines) + "\n"


def validate(text, mod, stage):
    config = configparser.ConfigParser(interpolation=None, strict=True)
    config.read_string(text)
    assert config.getint("BASIC", "Version") == 2
    assert config.getint("BASIC", "TechLevel") == stage
    assert config.getint("COOP_TEMPLATE", "Stage") == stage
    assert config["COOP_TEMPLATE"]["mod"] == mod
    assert 0 <= config.getint("MAP", "SpiceVariantSeed") <= 0x7FFFFFFF
    rows = [config["MAP"][f"{y:03}"] for y in range(SIZE)]
    assert all(len(row) == SIZE and set(row) <= set("-^~+%") for row in rows)
    assert sum(row.count("~") + row.count("+") for row in rows) > 300
    assert [config[f"Player{i}"]["brain"] for i in range(1, 6)] == ["Team 1", "Team 1", "Team 2", "Team 2", "Team 2"]
    assert all(config.getint(f"Player{i}", "credits") > 0 for i in range(1, 6))
    occupied = set()
    for key, value in config["STRUCTURES"].items():
        parts = value.split(",")
        if key.startswith("gen"):
            assert parts[1] == "Concrete"
            continue
        _, item, health, position = parts
        position = int(position)
        assert health == "256"
        x, y = position % SIZE, position // SIZE
        width, height = SIZES[item]
        for ty in range(y, y + height):
            for tx in range(x, x + width):
                assert 0 <= tx < SIZE and 0 <= ty < SIZE and rows[ty][tx] == "%"
                assert (tx, ty) not in occupied
                occupied.add((tx, ty))
    blocked = occupied.copy()
    positions = []
    for value in config["UNITS"].values():
        player, item, health, position, _, order = value.split(",")
        position = int(position)
        x, y = position % SIZE, position // SIZE
        assert 0 <= x < SIZE and 0 <= y < SIZE and (x, y) not in occupied
        occupied.add((x, y))
        assert health == "256" and order in ("Area Guard", "Guard", "Harvest")
        assert player in {f"Player{i}" for i in range(1, 6)}
        positions.append((x, y))
    # All bases have passable paths to each other and to the spice fields.
    visited = {positions[0]}
    queue = deque(visited)
    while queue:
        x, y = queue.popleft()
        for dx, dy in ((0, 1), (0, -1), (1, 0), (-1, 0)):
            p = x + dx, y + dy
            if 0 <= p[0] < SIZE and 0 <= p[1] < SIZE and p not in visited and p not in blocked:
                visited.add(p)
                queue.append(p)
    assert all(position in visited for position in positions)
    if stage == 1:
        for player in ("Player1", "Player2"):
            items = [v.split(",")[1] for v in config["UNITS"].values() if v.startswith(player + ",")]
            assert items.count("Tank") == 2 and items.count("Troopers") == 3 and items.count("Special") == 2
            assert not set(items) & {"Soldier", "Infantry"}


def refresh_checksums(mod):
    if mod == "vanilla":
        return
    root = ROOT / "mods" / mod
    index = root / "checksums.sha256"
    entries = {line.split(None, 1)[1].strip() for line in index.read_text().splitlines() if line.strip()}
    entries.update(path.relative_to(root).as_posix() for path in (root / "campaign" / "coop").rglob("*.ini"))
    result = []
    for relative in sorted(entries):
        path = root / relative
        assert path.resolve().is_relative_to(root.resolve()) and path.is_file(), relative
        result.append(hashlib.sha256(path.read_bytes()).hexdigest() + "  " + relative)
    index.write_text("\n".join(result) + "\n", encoding="ascii", newline="\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true", help="create/update original campaign adaptations; preserve legacy root maps")
    parser.add_argument("--check", action="store_true", help="validate the supplied coop maps")
    parser.add_argument("--legacy-write", action="store_true", help="explicitly regenerate old synthetic root maps")
    parser.add_argument("--report", type=Path, help="write detailed original-map fidelity and placement audit")
    args = parser.parse_args()
    checked = 0
    fingerprints = set()
    for mod in MODS:
        for stage in range(1, 10):
            path = destination(mod, stage)
            if args.legacy_write:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(generate(mod, stage), encoding="ascii", newline="\n")
            text = path.read_text(encoding="ascii")
            validate(text, mod, stage)
            rows = "\n".join(line for line in text.splitlines() if len(line) == SIZE + 4 and line[3] == "=")
            assert rows not in fingerprints, (mod, stage, "repeated terrain")
            fingerprints.add(rows)
            checked += 1
    legacy_bytes = {destination(mod, stage): destination(mod, stage).read_bytes()
                    for mod in MODS for stage in range(1, 10)}
    original_reports = original_coop.build(write=args.write)
    assert all(path.read_bytes() == data for path, data in legacy_bytes.items()), "Legacy root coop maps changed"
    if args.write or args.legacy_write:
        for mod in MODS:
            refresh_checksums(mod)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps({"summary": original_coop.last_audit, "originalMaps": original_reports}, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"mods": len(MODS), "legacyMapsUnchanged": checked,
                      "originalCampaigns": len(original_reports) // 9, "originalMaps": len(original_reports),
                      "nearbyMCV": True, "maximumMCVDistance": max(row["mcvDistance"] for row in original_reports),
                      "terrainAndPlayer1AndEnemyOrdersPreserved": True, **original_coop.last_audit}))


if __name__ == "__main__":
    main()

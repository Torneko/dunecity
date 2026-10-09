#!/usr/bin/env python3
"""Adapt original solo scenarios to two-player starts without editing solo files."""
from pathlib import Path
from functools import lru_cache
from collections import deque
import configparser
import hashlib
import io
import json
import struct

ROOT = Path(__file__).resolve().parents[1]
MISSIONS = (1, 2, 5, 8, 11, 14, 17, 20, 22)
NAMES = ("Harkonnen", "Atreides", "Ordos", "Fremen", "Sardaukar", "Mercenary",
         "Neutral", "Rebels", "Corruptique", "Wildspade", "Kleshmersh", "Tharpique")
LETTERS = dict(zip(NAMES, "HAOFSMNRCWKT"))
OWNED_SECTIONS = ("UNITS", "STRUCTURES", "TEAMS", "REINFORCEMENTS")
SIZES = {"const yard": (2, 2), "windtrap": (2, 2), "refinery": (3, 2),
         "barracks": (2, 2), "light fctry": (2, 2), "heavy fctry": (3, 2),
         "outpost": (2, 2), "hi-tech": (3, 2), "ix": (2, 2), "wor": (2, 2),
         "palace": (3, 3), "repair": (3, 2), "turret": (1, 1), "r-turret": (1, 1),
         "spice silo": (2, 2), "star port": (3, 3), "starport": (3, 3), "wall": (1, 1)}


def read(data):
    ini = configparser.ConfigParser(interpolation=None, strict=False)
    ini.optionxform = str
    # Legacy PAK maps can contain unused non-key height data after the INI.
    ini.read_string("\n".join(line for line in data.decode("cp850").splitlines()
                              if "=" in line or line.strip().startswith(("[", ";", "#"))))
    return ini


def canonical_source_hash(data):
    return hashlib.sha256(data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")).hexdigest()


@lru_cache(None)
def archive(name):
    data = (ROOT / "data" / name).read_bytes()
    pos, entries = 0, []
    while True:
        offset = struct.unpack_from("<I", data, pos)[0]
        pos += 4
        if not offset:
            break
        end = data.index(0, pos)
        entries.append((data[pos:end].decode("ascii").upper(), offset))
        pos = end + 1
    return {name: data[start:entries[i + 1][1] if i + 1 < len(entries) else len(data)]
            for i, (name, start) in enumerate(entries)}


def runtime_faction(mod, name):
    identity = NAMES.index(name)
    if mod == "Jericho":
        return {6: 9, 7: 10, 8: 11, 9: 6, 10: 7, 11: 8}.get(identity, identity)
    return identity


def source_bytes(mod, house, mission):
    filename = f"SCEN{LETTERS[house]}{mission:03}.INI"
    if mod != "vanilla":
        relative = f"mods/{mod}/campaign/{filename.lower()}"
        return (ROOT / relative).read_bytes(), relative
    loose = ROOT / "data/campaign_vanilla" / filename
    if house in ("Neutral", "Rebels", "Kleshmersh") and loose.is_file():
        return loose.read_bytes(), loose.relative_to(ROOT).as_posix()
    # Match FileManager::openCampaignFile, rather than accidentally taking the
    # modified A/H/O scenarios also present in Extra.PAK.
    pak = "SCENARIO.PAK" if house in NAMES[:3] else (
        "OPENSD2.PAK" if house in NAMES[3:6] else "Extra.PAK")
    if filename in archive(pak):
        return archive(pak)[filename], f"{pak}:{filename}"
    if loose.is_file():
        return loose.read_bytes(), loose.relative_to(ROOT).as_posix()
    return archive("Extra.PAK")[filename], f"Extra.PAK:{filename}"


def opponents(ini, house):
    return {name for name in NAMES if name != house and name in ini}


def normalized_owners(mod, house, roles, mission, ini):
    """Resolve wildlife and legacy implicit reinforcements, retaining payloads."""
    result, corrections, wildlife = {}, [], []
    reference, canonical = None, None
    for section in OWNED_SECTIONS:
        for key, value in ini[section].items() if section in ini else ():
            parts = [part.strip() for part in value.split(",")]
            owner = parts[0]
            resolved = owner
            if parts[1] == "Sandworm" and owner not in ini:
                resolved = "CoopWildlife"
                wildlife.append({"section": section, "key": key, "sourceOwner": owner})
            elif mod != "vanilla" and owner != house and owner not in roles:
                if reference is None:
                    row = next(row for row in json.loads((ROOT / "config/CampaignPlans.json").read_text(encoding="utf-8"))["mods"][mod]
                               if row["house"] == house)
                    template = row["vanillaTemplate"]
                    reference = read(archive("SCENARIO.PAK")[f"SCEN{template}{mission:03}.INI"])
                    first, second = {"H": ("Atreides", "Ordos"), "A": ("Ordos", "Harkonnen"), "O": ("Harkonnen", "Atreides")}[template]
                    canonical = {first: roles[0], second: roles[1], "Sardaukar": roles[2], "Fremen": roles[2],
                                 {"H": "Harkonnen", "A": "Atreides", "O": "Ordos"}[template]: house}
                original = [part.strip() for part in reference[section][key].split(",")]
                assert parts[1:] == original[1:], (mod, house, mission, section, key, "Cannot attribute implicit force canonically")
                resolved = canonical[original[0]]
                corrections.append({"section": section, "key": key, "sourceOwner": owner,
                                    "canonicalOwner": original[0], "resolvedOwner": resolved})
            result[section, key] = resolved
    return result, corrections, wildlife


def active_opponents(ini, house, owners):
    present = opponents(ini, house)
    active = present | {owner for owner in owners.values() if owner not in (house, "CoopWildlife")}
    initial = present | {owner for (section, _), owner in owners.items()
                         if section in ("UNITS", "STRUCTURES") and owner not in (house, "CoopWildlife")}
    future = {owner for (section, _), owner in owners.items() if section == "REINFORCEMENTS" and owner not in (house, "CoopWildlife")}
    return active, initial, future


@lru_cache(None)
def campaigns(mod):
    if mod != "vanilla":
        plans = json.loads((ROOT / "config/CampaignPlans.json").read_text(encoding="utf-8"))["mods"][mod]
        return [(row["house"], tuple(row["opponents"])) for row in plans]
    result = []
    for house in (*NAMES[:8], "Kleshmersh"):
        priority = []
        for mission in MISSIONS:
            ini = read(source_bytes(mod, house, mission)[0])
            owners, _, _ = normalized_owners(mod, house, (), mission, ini)
            present, _, _ = active_opponents(ini, house, owners)
            assert 1 <= len(present) <= 5, (mod, house, mission, present)
            priority.extend(name for name in NAMES if name in present and name not in priority)
        result.append((house, tuple(priority)))
    return result


def stage_roles(mod, roles, actual):
    if mod != "vanilla" or len(roles) == 3:
        assert actual <= set(roles), (mod, actual, roles)
        return roles
    # OPENSD2 finals have four or five simultaneous opponents. Preserve each
    # faction in its own slot, and bind the actual forces independently per map.
    selected = [name for name in roles if name in actual]
    selected.extend(name for name in roles if name not in actual)
    return tuple(selected[:max(3, len(actual))])


@lru_cache(None)
def terrain_cache():
    cache = json.loads((ROOT / "config/CoopCampaignTerrain.json").read_text(encoding="utf-8"))
    assert cache["sourceSha256"] == hashlib.sha256((ROOT / "src/MapSeed.cpp").read_text().encode()).hexdigest(), "Stale native MapSeed cache"
    assert all(len(value) == 4096 for value in cache["seeds"].values()), "Malformed native terrain"
    return cache["seeds"]


def terrain(ini):
    raw = terrain_cache()[ini["MAP"]["Seed"]]
    size, offset = {0: (62, 1), 1: (32, 16), 2: (21, 11)}[ini.getint("BASIC", "MapScale")]
    cells = {(x, y): raw[(y + offset) * 64 + x + offset] for y in range(size) for x in range(size)}
    # Construction slabs/walls overwrite their base terrain during loading.
    for key, value in ini["STRUCTURES"].items():
        parts = [part.strip() for part in value.split(",")]
        if key.startswith("GEN"):
            position = int(key[3:])
            cells[(position % 64 - offset, position // 64 - offset)] = "8"
    for field in ("Bloom", "Special"):
        for value in ini["MAP"].get(field, "").split(","):
            if value.strip():
                position = int(value.strip())
                # Bloom tiles are passable but cannot support MCV deployment.
                cells[(position % 64 - offset, position // 64 - offset)] = "7"
    return size, offset, cells


def occupied_cells(ini, offset):
    occupied, structures = set(), set()
    for key, value in ini["STRUCTURES"].items():
        if key.startswith("GEN") and value.split(",")[1].strip().lower() == "wall":
            position = int(key[3:])
            structures.add((position % 64 - offset, position // 64 - offset))
        if not key.startswith("ID"):
            continue
        _, item, _, position = [part.strip() for part in value.split(",")]
        x, y = int(position) % 64 - offset, int(position) // 64 - offset
        if item.lower() in ("concrete", "slab1", "slab4"):
            continue
        width, height = SIZES[item.lower()]
        structures.update((x + dx, y + dy) for dx in range(width) for dy in range(height))
    occupied.update(structures)
    for value in ini["UNITS"].values():
        position = int(value.split(",")[3])
        occupied.add((position % 64 - offset, position // 64 - offset))
    return occupied, structures


def connected_cells(cells, blocked, origin):
    starts = {(origin[0] + dx, origin[1] + dy) for dx, dy in ((-1, 0), (0, -1), (2, 0), (0, 2))}
    reached = {p for p in starts if p in cells and cells[p] != "a" and p not in blocked}
    queue = deque(sorted(reached))
    while queue:
        x, y = queue.popleft()
        for dx, dy in ((0, 1), (0, -1), (1, 0), (-1, 0)):
            point = x + dx, y + dy
            if point in cells and cells[point] != "a" and point not in blocked and point not in reached:
                reached.add(point)
                queue.append(point)
    return reached


def extra_units(ini, house, intro=False):
    size, offset, cells = terrain(ini)
    occupied, structures = occupied_cells(ini, offset)
    yard = [value.split(",") for value in ini["STRUCTURES"].values()
            if value.split(",")[:2] == [house, "Const Yard"]]
    assert len(yard) == 1, (house, "Expected one original construction yard")
    position = int(yard[0][3])
    origin = position % 64 - offset, position // 64 - offset
    reached = connected_cells(cells, structures, origin)
    candidates = []
    for x, y in reached:
        footprint = {(x + dx, y + dy) for dx in range(2) for dy in range(2)}
        distance = max(abs(x - origin[0]), abs(y - origin[1]))
        if (distance <= 10 and footprint.isdisjoint(occupied)
                and all(p in cells and cells[p] in "28" for p in footprint)):
            candidates.append((distance, abs(x - origin[0]) + abs(y - origin[1]), y, x))
    assert candidates, (house, ini["MAP"]["Seed"], "No nearby deployable MCV position")
    _, _, my, mx = min(candidates)
    footprint = {(mx + dx, my + dy) for dx in range(2) for dy in range(2)}
    occupied.update(footprint)
    wor = None
    wor_reference = origin
    if intro:
        host_wor = [value.split(",") for value in ini["STRUCTURES"].values()
                    if value.split(",")[:2] == [house, "WOR"]]
        if host_wor:
            position = int(host_wor[0][3])
            wor_reference = position % 64 - offset, position // 64 - offset
        candidates = []
        for x, y in reached:
            area = {(x + dx, y + dy) for dx in range(2) for dy in range(2)}
            if area.isdisjoint(occupied) and all(p in cells and cells[p] in "28" for p in area):
                distance = max(abs(x-wor_reference[0]), abs(y-wor_reference[1]))
                candidates.append((distance, abs(x-wor_reference[0])+abs(y-wor_reference[1]), y, x))
        assert candidates, (house, "No free connected rock footprint for guest WOR")
        for _, _, wy, wx in sorted(candidates):
            area = {(wx + dx, wy + dy) for dx in range(2) for dy in range(2)}
            remaining = connected_cells(cells, structures | footprint | area, origin)
            if any((mx + dx, my + dy) in remaining for dx, dy in ((-1,0),(0,-1),(2,0),(0,2))):
                break
        else:
            raise AssertionError((house, "Guest WOR would seal the MCV deployment spot"))
        wor = wx, wy
        area = {(wx + dx, wy + dy) for dx in range(2) for dy in range(2)}
        occupied.update(area)
        structures.update(area)
    reached = connected_cells(cells, structures | footprint, origin)
    assert any((mx + dx, my + dy) in reached for dx, dy in ((-1, 0), (0, -1), (2, 0), (0, 2))), "Sealed MCV construction spot"
    additions = {"IDCOOPMCV": f"Player2,MCV,256,{(my + offset) * 64 + mx + offset},64,Guard"}
    for key, value in ini["UNITS"].items():
        parts = [part.strip() for part in value.split(",")]
        if parts[0] != house:
            continue
        choices = []
        for x, y in reached:
            distance = max(abs(x - origin[0]), abs(y - origin[1]))
            if (distance <= 12 and (x, y) not in occupied
                    and (parts[1] != "Sandworm" or cells[x, y] in "79bc")):
                choices.append((max(abs(x - mx), abs(y - my)), distance, y, x))
        assert choices, (house, key, "No nearby free unit position")
        _, _, y, x = min(choices)
        occupied.add((x, y))
        parts[0] = "Player2"
        parts[3] = str((y + offset) * 64 + x + offset)
        additions["IDCOOP" + key[2:]] = ",".join(parts)
    return additions, {"constructionYard": origin, "mcv": (mx, my), "mcvDistance": max(abs(mx - origin[0]), abs(my - origin[1])),
                       "guestWor": wor, "worReference": wor_reference if intro else None,
                       "guestWorPosition": (wor[1]+offset)*64+wor[0]+offset if wor else None}


def destination(mod, faction, stage):
    base = ROOT / "data" if mod == "vanilla" else ROOT / "mods" / mod / "campaign"
    return base / "coop" / f"faction{faction}" / f"coop{stage:02}.ini"


def generate(mod, house, roles, stage):
    mission = MISSIONS[stage - 1]
    source, provenance = source_bytes(mod, house, mission)
    original = read(source)
    additions, placement = extra_units(original, house, stage == 1)
    owners, corrections, wildlife = normalized_owners(mod, house, roles, mission, original)
    actual, initial, future = active_opponents(original, house, owners)
    roles = stage_roles(mod, roles, actual)
    assert actual <= set(roles), (mod, house, mission, actual, roles)
    participant_count = 2 + len(roles)
    assert 5 <= participant_count <= 7, (mod, house, stage, roles)
    mapping = {house: "Player1", **{enemy: f"Player{i + 3}" for i, enemy in enumerate(roles)}}
    ini = configparser.ConfigParser(interpolation=None)
    ini.optionxform = str
    for section in original.sections():
        target = mapping.get(section, section)
        ini.add_section(target)
        for key, value in original[section].items():
            if section in OWNED_SECTIONS:
                parts = value.split(",")
                owner = owners[section, key]
                assert owner in mapping or owner == "CoopWildlife", (mod, house, mission, section, key, owner)
                parts[0] = "CoopWildlife" if owner == "CoopWildlife" else mapping[owner]
                value = ",".join(parts)
            ini[target][key] = value
    ini["BASIC"]["TechLevel"] = str(min(stage, 8) if mod == "vanilla" else stage)
    ini.add_section("Player2")
    ini["Player2"].update(dict(original[house]))
    for slot in range(1, participant_count + 1):
        section = f"Player{slot}"
        if section not in ini:
            ini.add_section(section)
            ini[section].update(Credits="0", Quota="0", MaxUnit=original[house].get("MaxUnit", "25"))
        ini[section]["Brain"] = f"Team {1 if slot <= 2 else 2}"
    ini["UNITS"].update(additions)
    if placement["guestWorPosition"] is not None:
        ini["STRUCTURES"]["IDCOOPWOR"] = f"Player2,WOR,256,{placement['guestWorPosition']}"
    ini.add_section("COOP_TEMPLATE")
    quota = original.getint(house, "Quota", fallback=0) if original.getint("BASIC", "WinFlags") & 4 else 0
    marker = ini["COOP_TEMPLATE"]
    marker.update(Schema="1", Stage=str(stage), Mod=mod, ParticipantCount=str(participant_count), SourceFaction=str(runtime_faction(mod, house)),
                  OriginalMission=str(mission), SourceHouse=house, SourceFile=provenance,
                  SourceSha256=canonical_source_hash(source), SourceHashNormalization="LF", SharedQuota=str(2 * quota),
                  OriginalWinFlags=original["BASIC"]["WinFlags"], OriginalLoseFlags=original["BASIC"]["LoseFlags"])
    if wildlife:
        marker["WildlifeOwner"] = "CoopWildlife"
    correction_owners = {entry["sourceOwner"]: mapping[entry["resolvedOwner"]] for entry in corrections}
    marker["OwnerCorrectionCount"] = str(len(corrections))
    marker["OwnerCorrectionHouseCount"] = str(len(correction_owners))
    for index, (owner, participant) in enumerate(sorted(correction_owners.items()), 1):
        marker[f"OwnerCorrection{index}"] = owner + "," + participant
    for role, enemy in enumerate(roles, 1):
        marker[f"EnemyFaction{role}"] = str(runtime_faction(mod, enemy) if enemy in actual else -1)
        marker[f"EnemyPresent{role}"] = "1" if enemy in initial else "0"
        marker[f"EnemyDeclared{role}"] = "1" if enemy in original else "0"
        marker[f"EnemyDeferred{role}"] = "1" if enemy in future and enemy not in initial else "0"
        marker[f"SourceOpponent{role}"] = enemy
    output = io.StringIO()
    ini.write(output, space_around_delimiters=False)
    text = "; Original campaign terrain and forces; cooperative starts only.\n" + output.getvalue()
    return text, {**placement, "canonicalOwnerCorrections": corrections, "wildlifeOrders": wildlife}


def validate(text, mod, house, roles, stage):
    # A full reconstruction comparison checks all original fields/objects,
    # enemy orders, timers, CHOAM, terrain options and human economy together.
    expected, placement = generate(mod, house, roles, stage)
    assert text == expected, (mod, house, stage, "Original scenario fidelity or cooperative start drift")
    ini = read(text.encode("cp850"))
    original_bytes, _ = source_bytes(mod, house, MISSIONS[stage - 1])
    original = read(original_bytes)
    for key, value in original["BASIC"].items():
        if key != "TechLevel":
            assert ini["BASIC"][key] == value, (mod, house, stage, "Original BASIC field changed", key)
    assert dict(ini["MAP"]) == dict(original["MAP"]), (mod, house, stage, "Terrain changed")
    assert {key: value for key, value in ini["Player1"].items() if key != "Brain"} == {
        key: value for key, value in original[house].items() if key != "Brain"}, (mod, house, stage, "Original player economy changed")
    for section in OWNED_SECTIONS:
        for key, value in original[section].items() if section in original else ():
            assert ini[section][key].split(",")[1:] == value.split(",")[1:], (mod, house, stage, section, key, "Original record payload changed")
    assert ini["Player1"]["Credits"] == ini["Player2"]["Credits"]
    assert ini["Player1"]["Quota"] == ini["Player2"]["Quota"]
    own = [value.split(",")[1:] for key, value in ini["UNITS"].items() if value.startswith("Player1,")]
    other = [value.split(",")[1:] for key, value in ini["UNITS"].items() if value.startswith("Player2,") and key != "IDCOOPMCV"]
    assert [parts[:2] + parts[3:] for parts in own] == [parts[:2] + parts[3:] for parts in other]
    guest_structures = [value for value in ini["STRUCTURES"].values() if value.startswith("Player2,")]
    assert guest_structures == ([f"Player2,WOR,256,{placement['guestWorPosition']}"] if stage == 1 else [])
    assert len(other) == len(own)
    size, offset, cells = terrain(original)
    occupied, _ = occupied_cells(original, offset)
    mcv_x, mcv_y = placement["mcv"]
    footprint = {(mcv_x + dx, mcv_y + dy) for dx in range(2) for dy in range(2)}
    assert footprint.isdisjoint(occupied) and all(cells[p] in "28" for p in footprint)
    occupied.update(footprint)
    for key, value in ini["UNITS"].items():
        if not key.startswith("IDCOOP") or key == "IDCOOPMCV":
            continue
        position = int(value.split(",")[3])
        point = position % 64 - offset, position // 64 - offset
        assert point in cells and cells[point] != "a" and point not in occupied, (mod, house, stage, key, "Blocked guest unit")
        occupied.add(point)
    outside_crop = []
    for section in ("UNITS", "STRUCTURES"):
        for key, value in original[section].items():
            if not key.startswith("ID"):
                continue
            position = int(value.split(",")[3])
            point = position % 64 - offset, position // 64 - offset
            if point not in cells:
                outside_crop.append({"section": section, "key": key, "sourceValue": value})
    return {"mod": mod, "sourceFaction": runtime_faction(mod, house), "sourceHouse": house,
            "stage": stage, "originalMission": MISSIONS[stage - 1], "sourceSha256": ini["COOP_TEMPLATE"]["SourceSha256"], "sourceHashNormalization": "LF",
            "startingUnitOrdersEach": len(own), "participantCount": int(ini["COOP_TEMPLATE"]["ParticipantCount"]),
            "presentOpponents": sum(int(ini["COOP_TEMPLATE"][f"EnemyPresent{i}"]) for i in range(1, int(ini["COOP_TEMPLATE"]["ParticipantCount"]) - 1)),
            "participatingOpponents": sum(int(ini["COOP_TEMPLATE"][f"EnemyFaction{i}"]) >= 0 for i in range(1, int(ini["COOP_TEMPLATE"]["ParticipantCount"]) - 1)),
            "sharedQuota": int(ini["COOP_TEMPLATE"]["SharedQuota"]), **placement,
            "historicalOutsideCropOrders": outside_crop,
            "terrainAndPlayer1AndEnemyOrdersPreserved": True}


last_audit = {}


def build(write=False):
    global last_audit
    protected = {ROOT / "data" / name for name in ("SCENARIO.PAK", "OPENSD2.PAK", "Extra.PAK")}
    directories = [ROOT / "data/campaign_vanilla"]
    directories += [ROOT / "mods" / mod / folder for mod in ("Tornie", "Jericho", "TornieLite", "JerichoLite")
                    for folder in ("campaign", "data")]
    for directory in directories:
        protected.update(path for path in directory.iterdir() if path.is_file() and path.suffix.lower() == ".ini"
                         and (path.name.upper().startswith(("SCEN", "REGION")) or path.name == "CampaignPlan.ini"))
    before = {path: hashlib.sha256(path.read_bytes()).digest() for path in protected}
    reports = []
    for mod in ("vanilla", "Tornie", "Jericho", "TornieLite", "JerichoLite"):
        for house, roles in campaigns(mod):
            for stage in range(1, 10):
                path = destination(mod, runtime_faction(mod, house), stage)
                if write:
                    text, _ = generate(mod, house, roles, stage)
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text(text, encoding="cp850", newline="\n")
                reports.append(validate(path.read_text(encoding="cp850"), mod, house, roles, stage))
    assert len(reports) == 405, len(reports)
    assert all(hashlib.sha256(path.read_bytes()).digest() == digest for path, digest in before.items()), "Original solo resource changed"
    last_audit = {"protectedSoloFilesUnchanged": len(before),
                  "canonicalCorrectionMaps": sum(bool(row["canonicalOwnerCorrections"]) for row in reports),
                  "canonicalCorrectionRecords": sum(len(row["canonicalOwnerCorrections"]) for row in reports),
                  "wildlifeMaps": sum(bool(row["wildlifeOrders"]) for row in reports),
                  "wildlifeOrders": sum(len(row["wildlifeOrders"]) for row in reports),
                  "historicalOutsideCropOrders": sum(len(row["historicalOutsideCropOrders"]) for row in reports),
                  "maximumParticipants": max(row["participantCount"] for row in reports)}
    return reports

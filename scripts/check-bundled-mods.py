#!/usr/bin/env python3
"""Validate the exact bundled payload and resources required by each RTS mod."""
from pathlib import Path
import configparser
import hashlib
import json
import re
from importlib.util import spec_from_file_location, module_from_spec

ROOT = Path(__file__).resolve().parents[1]
coop_spec = spec_from_file_location('original_coop', ROOT / 'scripts/original-coop-campaign.py')
original_coop = module_from_spec(coop_spec)
coop_spec.loader.exec_module(original_coop)
for name, letters in [('Tornie', 'HAOFSMNRCWKT'), ('TornieLite', 'HAOFSM'),
                      ('Jericho', 'HAOFSMNRCWKT'), ('JerichoLite', 'HAOFSM')]:
    root = ROOT / 'mods' / name
    config = configparser.ConfigParser(interpolation=None, strict=False)
    config.read(root / 'mod.ini', encoding='utf-8-sig')
    assert config.get('Mod', 'Display Name'), name
    assert config.get('Mod', 'Game Version') == json.loads((ROOT / 'vcpkg.json').read_text())['version'], name
    for required in ('ObjectData.ini', 'GameOptions.ini', 'QuantBot Config.ini', 'manifest.json', 'checksums.sha256'):
        assert (root / required).is_file(), (name, required)
    for letter in letters:
        assert (root / 'campaign' / ('REGION' + letter + '.INI')).is_file(), (name, letter)
        for mission in range(1, 23):
            assert (root / 'campaign' / f'scen{letter.lower()}{mission:03}.ini').is_file(), (name, letter, mission)
    for mission in range(1, 10):
        assert (root / 'campaign' / 'coop' / f'coop{mission:02}.ini').is_file(), (name, 'coop', mission)
    for house, _ in original_coop.campaigns(name):
        faction = original_coop.runtime_faction(name, house)
        for mission in range(1, 10):
            assert original_coop.destination(name, faction, mission).is_file(), (name, faction, mission)
    for section in config.sections():
        if section.startswith('Mentat '):
            for key in ('Background', 'Foreground', 'Eyes', 'Mouth'):
                if config.has_option(section, key):
                    asset = config.get(section, key)
                    assert (root / 'data' / asset).is_file(), (name, section, asset)
    object_data = (root / 'ObjectData.ini').read_text(encoding='utf-8-sig')
    assert not re.search(r'^\[(Residential Zone|Commercial Zone|Industrial Zone|Road|Nuclear Plant|Police Station|Stadium|Airport)\]', object_data, re.M), name
    checked = 0
    for line in (root / 'checksums.sha256').read_text().splitlines():
        digest, relative = line.split(None, 1)
        path = root / relative.strip()
        assert path.resolve().is_relative_to(root.resolve()), relative
        assert hashlib.sha256(path.read_bytes()).hexdigest() == digest, (name, relative)
        checked += 1
    print(f'{name}: {len(letters)} campaigns, {checked} exact payload checksums, presentation assets verified')
for house, _ in original_coop.campaigns('vanilla'):
    for mission in range(1, 10):
        assert original_coop.destination('vanilla', original_coop.runtime_faction('vanilla', house), mission).is_file(), ('vanilla', house, mission)

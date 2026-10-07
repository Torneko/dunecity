#!/usr/bin/env python3
from pathlib import Path
import configparser, hashlib, json, struct, subprocess, os

ROOT = Path(__file__).resolve().parents[1]
TOOL = ROOT / 'build/coop-terrain-tool'
TOOL.mkdir(parents=True,exist_ok=True)
source = (ROOT / 'src/MapSeed.cpp').read_text()
native = source[:source.index('MapData createMapWithSeed(')]
native = native.replace('#include <misc/SDL2pp.h>', '').replace('#include "MapSeed.h"', '')
prefix = '''#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
using Uint8=uint8_t; using Sint8=int8_t; using Uint16=uint16_t; using Sint16=int16_t;
using Uint32=uint32_t; using Sint32=int32_t;
Uint32 SDL_SwapLE32(Uint32 value) {
    const uint16_t order=1;
    if(*reinterpret_cast<const uint8_t*>(&order)) return value;
    return (value>>24)|((value>>8)&0xff00)|((value<<8)&0xff0000)|(value<<24);
}
'''
suffix = '''
int main(int argc, char** argv) {
    std::cout << "{";
    for(int i=1;i<argc;++i) {
        Uint16 terrain[4096]; createMapWithSeed(std::stoul(argv[i]), terrain);
        if(i>1) std::cout << ",";
        std::cout << "\\\"" << argv[i] << "\\\":\\\"";
        for(auto tile:terrain) std::cout << "0123456789abcdef"[tile>>4];
        std::cout << "\\\"";
    }
    std::cout << "}";
}
'''
(TOOL / 'main.cpp').write_text(prefix + native + suffix)
(TOOL / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(CoopTerrain LANGUAGES CXX)\nadd_executable(coop_terrain main.cpp)\ntarget_compile_features(coop_terrain PRIVATE cxx_std_17)\n')
subprocess.run(['cmake','-S',str(TOOL),'-B',str(TOOL/'build')] + (['-A','x64'] if os.name=='nt' else []),check=True)
subprocess.run(['cmake','--build',str(TOOL/'build'),'--config','Release'],check=True)

def seeds_in(data):
    ini=configparser.ConfigParser(interpolation=None,strict=False)
    ini.read_string('\n'.join(line for line in data.decode('cp850').splitlines()
                    if '=' in line or line.strip().startswith(('[',';','#'))))
    return {ini.getint('MAP','Seed')} if ini.has_option('MAP','Seed') else set()

seeds=set()
for mod in ('Tornie','Jericho','TornieLite','JerichoLite'):
    for path in (ROOT/'mods'/mod/'campaign').glob('scen*.ini'):
        seeds.update(seeds_in(path.read_bytes()))
for path in (ROOT/'data/campaign_vanilla').glob('scen*.ini'):
    seeds.update(seeds_in(path.read_bytes()))
for pak_name in ('SCENARIO.PAK','OPENSD2.PAK','Extra.PAK'):
    pak=(ROOT/'data'/pak_name).read_bytes()
    offset=0; entries=[]
    while True:
        start=struct.unpack_from('<I',pak,offset)[0];offset+=4
        if not start:break
        end=pak.index(0,offset);name=pak[offset:end].decode();offset=end+1
        entries.append((name,start))
    for i,(name,start) in enumerate(entries):
        if name.upper().startswith('SCEN') and name.upper().endswith('.INI'):
            seeds.update(seeds_in(pak[start:entries[i+1][1] if i+1<len(entries) else len(pak)]))
exe=TOOL/('build/Release/coop_terrain.exe' if os.name=='nt' else 'build/coop_terrain')
terrain=json.loads(subprocess.check_output([str(exe),*map(str,sorted(seeds))]))
reference=json.loads((ROOT/'config/OpeningTerrain.json').read_text())['seeds']
assert all(terrain[key]==value for key,value in reference.items())
target=ROOT/'config/CoopCampaignTerrain.json'
target.write_text(json.dumps({'source':'MapSeed.cpp createMapWithSeed; raw 64x64 tile high nibbles',
    'sourceSha256':hashlib.sha256((ROOT/'src/MapSeed.cpp').read_text().encode()).hexdigest(),
    'seeds':terrain},indent=2)+'\n')
print(json.dumps({'seeds':len(terrain),'openingReferenceMatches':True,'output':str(target)}))

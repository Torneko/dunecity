#!/usr/bin/env python3
"""Audit region routes, house identities, mirrored files and faction campaigns."""
from pathlib import Path
import configparser, json, re, struct

ROOT=Path(__file__).resolve().parents[1]
PREFIX={'HAR':'Harkonnen','ATR':'Atreides','ORD':'Ordos','FRE':'Fremen','SAR':'Sardaukar',
        'MER':'Mercenary','NEU':'Neutral','REB':'Rebels','COR':'Corruptique','WIL':'Wildspade','KLE':'Kleshmersh','THA':'Tharpique'}
LETTERS=dict(zip('HAOFSMNRCWKT',PREFIX.values()))
def read(path):
    p=configparser.ConfigParser(interpolation=None,strict=False)
    p.optionxform=str
    # The runtime INI reader ignores non-key trailing legacy data as well.
    text=path.read_bytes().decode('cp850')
    p.read_string('\n'.join(line for line in text.splitlines() if '=' in line or line.strip().startswith(('[',';','#'))))
    return p
def ints(value):return [int(v.strip()) for v in value.split(',') if v.strip()]

reports=[]
for mod,letters in [('Tornie','HAOFSMNRCWKT'),('TornieLite','HAOFSM'),('Jericho','HAOFSMNRCWKT'),('vanilla','K')]:
    directory=ROOT/'data/campaign_vanilla' if mod=='vanilla' else ROOT/'mods'/mod/'campaign'
    files={p.name.upper():p for p in directory.iterdir() if p.is_file()}
    for letter in letters:
        path=files['REGION'+letter+'.INI'];ini=read(path)
        assert ini.getint('INFO','TOTAL REGIONS')==27,path
        for piece in range(1,28):
            xy=ints(ini['PIECES'][str(piece)]);assert len(xy)==2 and 0<=xy[0]<320 and 0<=xy[1]<200,(path,piece)
        routes=0;repeats=0
        for group in range(1,9):
            section=ini['GROUP'+str(group)]
            for key,value in section.items():
                if key in PREFIX:
                    territories=ints(value)
                    assert all(1<=v<=27 for v in territories),(path,group,key)
                    repeats+=len(territories)-len(set(territories))
            # Multiple arrows can point at the same region; selection uses the
            # FIRST matching arrow, so those routes intentionally share a map.
            seen=set()
            for i in range(4):
                value=section.get('REG'+str(i+1),'');route=ints(value)
                if not route:continue
                assert len(route)==4,(path,group,i,value)
                region,arrow,x,y=route
                if not region:continue
                assert 1<=region<=27 and 0<=arrow<=8 and 0<=x<320 and 0<=y<200,(path,group,route)
                if region in seen:continue
                seen.add(region)
                mission=(group-1)*3+2+i-(1 if group==8 else 0)
                assert 1<=mission<=22 and f'SCEN{letter}{mission:03}.INI' in files,(path,group,mission)
                routes+=1
        if mod!='vanilla':assert path.read_bytes()==(ROOT/'mods'/mod/'data'/path.name).read_bytes(),path
        opponents=set()
        for mission in range(1,23):
            scenario=read(files[f'SCEN{letter}{mission:03}.INI'])
            player=LETTERS[letter]
            assert scenario.get(player,'Brain',fallback='')=='Human',(mod,letter,mission,player)
            actual={h for h in PREFIX.values() if scenario.has_section(h) and h!=player}
            opponents|=actual
            if mod=='vanilla':assert actual=={'Harkonnen' if mission<=10 else 'Sardaukar' if mission<=21 else 'Rebels'},mission
        if letter=='W':assert opponents=={'Atreides','Kleshmersh','Ordos'},(mod,opponents)
        reports.append({'mod':mod,'house':LETTERS[letter],'missions':22,'routes':routes,
                        'opponents':sorted(opponents),'intentionalRepeatedTerritories':repeats})
print(json.dumps({'regionFiles':len(reports),'campaigns':reports},ensure_ascii=True,indent=2))

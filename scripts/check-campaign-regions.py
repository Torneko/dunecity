#!/usr/bin/env python3
"""Audit region routes, house identities, mirrored files and faction campaigns."""
from pathlib import Path
import configparser, json, re, struct, collections
from importlib.util import spec_from_file_location, module_from_spec

ROOT=Path(__file__).resolve().parents[1]
PREFIX={'HAR':'Harkonnen','ATR':'Atreides','ORD':'Ordos','FRE':'Fremen','SAR':'Sardaukar',
        'MER':'Mercenary','NEU':'Neutral','REB':'Rebels','COR':'Corruptique','WIL':'Wildspade','KLE':'Kleshmersh','THA':'Tharpique'}
LETTERS=dict(zip('HAOFSMNRCWKT',PREFIX.values()))
def read(path):
    p=configparser.ConfigParser(interpolation=None,strict=False)
    p.optionxform=str
    # The runtime INI reader ignores non-key trailing legacy data as well.
    text=(path.read_bytes() if isinstance(path,Path) else path).decode('cp850')
    p.read_string('\n'.join(line for line in text.splitlines() if '=' in line or line.strip().startswith(('[',';','#'))))
    return p
def ints(value):return [int(v.strip()) for v in value.split(',') if v.strip()]

spec=spec_from_file_location('campaign_rebuild',ROOT/'scripts/rebuild-campaigns.py')
helper=module_from_spec(spec);spec.loader.exec_module(helper)
reports=[]
plans=json.loads((ROOT/'config/CampaignPlans.json').read_text())['mods']
for mod,rows in plans.items():
    names=[row['house'] for row in rows]
    for role in range(3):assert sorted(row['opponents'][role] for row in rows)==sorted(names),(mod,role)
    for row in rows:assert len(set([row['house']]+row['opponents']))==4,(mod,row)
for a,b in zip(plans['Tornie'],plans['Jericho']):assert a['house']==b['house'] and not set(a['opponents']) & set(b['opponents'])
def vanilla_scenarios(pak='Extra.PAK'):
    data=(ROOT/'data'/pak).read_bytes();pos=0;entries=[]
    while True:
        start=struct.unpack_from('<I',data,pos)[0];pos+=4
        if not start:break
        end=data.index(0,pos);name=data[pos:end].decode('ascii');pos=end+1;entries.append((name.upper(),start))
    return {name:data[start:entries[i+1][1] if i+1<len(entries) else len(data)] for i,(name,start) in enumerate(entries)}

for mod,letters in [('Tornie','HAOFSMNRCWKT'),('TornieLite','HAOFSM'),('Jericho','HAOFSMNRCWKT'),('vanilla','NRK')]:
    directory=ROOT/'data/campaign_vanilla' if mod=='vanilla' else ROOT/'mods'/mod/'campaign'
    files={p.name.upper():p for p in directory.iterdir() if p.is_file()}
    if mod=='vanilla':
        for name,content in vanilla_scenarios().items():files.setdefault(name,content)
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
                    if mod in plans:
                        plan=next(row for row in plans[mod] if row['letter']==letter)
                        assert PREFIX[key] in [plan['house']]+plan['opponents'],(path,group,key)
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
            if mod=='vanilla' and letter=='K':
                original=read(vanilla_scenarios('SCENARIO.PAK')[f'SCENH{mission:03}.INI'])
                remap={'Atreides':'Harkonnen','Ordos':'Sardaukar','Sardaukar':'Mercenary'}
                expected={target for source,target in remap.items() if original.has_section(source)}
                assert actual==expected,(mission,actual,expected)
        if mod=='vanilla' and letter=='K':assert opponents=={'Harkonnen','Sardaukar','Mercenary'},opponents
        if mod in plans:
            plan=next(row for row in plans[mod] if row['letter']==letter)
            assert opponents==set(plan['opponents']),(mod,letter,opponents,plan)
            briefing=read(ROOT/'mods'/mod/'data/CampaignPlan.ini')[plan['house']]
            assert briefing['Template']==plan['vanillaTemplate'],(mod,plan)
            assert [briefing[f'Opponent{i+1}'] for i in range(3)]==plan['opponents'],(mod,plan)
            opening=read(files[f'SCEN{letter}001.INI'])
            assert briefing['OpeningQuota']==opening[plan['house']].get('Quota','0'),(mod,plan)
            own=collections.Counter();enemies=[];positions=set()
            for value in opening['UNITS'].values():
                owner,unit,health,pos,angle,mode=value.split(',')
                if owner==plan['house']:own[unit]+=1
                else:enemies.append((unit,mode))
            assert own['Troopers']==3 and own['Trooper']==0 and own['Special']==2,(mod,letter,own)
            for i,unit in enumerate(['Tank','Tank','Troopers','Troopers','Troopers','Special','Special']):
                value=opening['UNITS'][f'ID{100+i:03}'].split(',')
                assert value[0]==plan['house'] and value[1]==unit and value[2]=='256' and value[5]=='Guard',(mod,letter,value)
                assert value[3] not in positions,(mod,letter,'overlapping bonus orders');positions.add(value[3])
            assert len(enemies)==12 and all(unit!='Special' and mode!='Hunt' for unit,mode in enemies),(mod,letter,enemies)
            mirror=ROOT/'mods'/mod/'data'/f'scen{letter.lower()}001.ini'
            if mirror.exists():assert mirror.read_bytes()==files[f'SCEN{letter}001.INI'].read_bytes(),mod
            roles=helper.canonical_roles(plan['vanillaTemplate'],plan['house'],plan['opponents'])
            vanilla=vanilla_scenarios('SCENARIO.PAK')
            for mission in range(1,23):
                scenario=read(files[f'SCEN{letter}{mission:03}.INI'])
                original=read(vanilla[f"SCEN{plan['vanillaTemplate']}{mission:03}.INI"])
                expected={roles[h] for h in original.sections() if h in roles and roles[h]!=plan['house']}
                actual={h for h in PREFIX.values() if scenario.has_section(h) and h!=plan['house']}
                assert actual==expected,(mod,letter,mission,actual,expected)
                for section in ['UNITS','REINFORCEMENTS']:
                    for key,value in scenario.items(section) if scenario.has_section(section) else []:
                        owner,unit=value.split(',')[:2]
                        assert unit.strip() not in helper.SPECIAL,(mod,letter,mission,section,key,unit)
                        if mission==1 and section=='UNITS' and owner.strip()==plan['house']:assert unit.strip() not in helper.SOLDIERS
        reports.append({'mod':mod,'house':LETTERS[letter],'missions':22,'routes':routes,
                        'opponents':sorted(opponents),'intentionalRepeatedTerritories':repeats})
print(json.dumps({'regionFiles':len(reports),'campaigns':reports},ensure_ascii=True,indent=2))

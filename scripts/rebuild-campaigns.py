#!/usr/bin/env python3
"""Rebuild the 24 full-mod campaigns from the tagged 1.0.534 forces and canonical Vanilla roles."""
from pathlib import Path
import configparser,collections,json,re,struct,subprocess,shutil,tarfile,io,argparse
from importlib.util import spec_from_file_location, module_from_spec
ROOT=Path(__file__).resolve().parents[1]
NAMES=['Harkonnen','Atreides','Ordos','Fremen','Sardaukar','Mercenary','Neutral','Rebels','Corruptique','Wildspade','Kleshmersh','Tharpique']
PREFIX=dict(zip(NAMES,['HAR','ATR','ORD','FRE','SAR','MER','NEU','REB','COR','WIL','KLE','THA']))
SPECIAL={'Devastator','Sonic Tank','Deviator','Rocket Trike','Sonic Trike','Flame Tank','Elite Launcher','Elite Siege Tank','Chemical Siege Tank'}
SOLDIERS={'Soldier','Infantry','Infantry (5)','Soldiers (5)','Infantry5'}
# All twelve factions share the corrected intro policy in both full mods.
spec=spec_from_file_location('opening_balance',ROOT/'scripts/tune-campaign-openings.py')
opening_balance=module_from_spec(spec);spec.loader.exec_module(opening_balance)
clone_spec=spec_from_file_location('opening_clones',ROOT/'scripts/campaign-opening-clones.py')
opening_clones=module_from_spec(clone_spec);clone_spec.loader.exec_module(opening_clones)

def add_opening_support(text,player,mission):
 if mission!=1:return text
 return opening_balance.tune_opening(text,player)[0]
def read(data):
 p=configparser.ConfigParser(interpolation=None,strict=False);p.optionxform=str
 p.read_string('\n'.join(l for l in data.decode('cp850').splitlines() if '=' in l or l.strip().startswith(('[',';','#'))));return p
def pak(path):
 data=path.read_bytes();pos=0;entries=[]
 while True:
  start=struct.unpack_from('<I',data,pos)[0];pos+=4
  if not start:break
  end=data.index(0,pos);name=data[pos:end].decode('ascii');pos=end+1;entries.append((name.upper(),start))
 return {name:data[start:entries[i+1][1] if i+1<len(entries) else len(data)] for i,(name,start) in enumerate(entries)}
def canonical_roles(template,player,opponents):
 base={'H':'Harkonnen','A':'Atreides','O':'Ordos'}[template]
 first,second={'H':('Atreides','Ordos'),'A':('Ordos','Harkonnen'),'O':('Harkonnen','Atreides')}[template]
 return {base:player,first:opponents[0],second:opponents[1],'Sardaukar':opponents[2],'Fremen':opponents[2]}
def rebuild():
 plans=json.loads((ROOT/'config/CampaignPlans.json').read_text())
 git=shutil.which('git') or ('C:/Program Files/Git/cmd/git.exe' if Path('C:/Program Files/Git/cmd/git.exe').exists() else 'git')
 raw=subprocess.check_output([git,'archive','--format=tar','v1.0.534','mods/Tornie/campaign','mods/Jericho/campaign'],cwd=ROOT)
 with tarfile.open(fileobj=io.BytesIO(raw)) as t:baseline={m.name:t.extractfile(m).read() for m in t if m.isfile()}
 vanilla=pak(ROOT/'data/SCENARIO.PAK');reports=[]
 for mod,rows in plans['mods'].items():
  baseline_mod=mod.removesuffix('Lite')
  briefing='; Generated from config/CampaignPlans.json; runtime briefing identities.\n'
  for row in rows:
   player=row['house'];letter=row['letter'];template=row['vanillaTemplate'];roles=canonical_roles(template,player,row['opponents'])
   for mission in range(1,23):
    rel=f'mods/{mod}/campaign/scen{letter.lower()}{mission:03}.ini';original=baseline[rel.replace('/'+mod+'/', '/'+baseline_mod+'/')];old=read(original);reference=read(vanilla[f'SCEN{template}{mission:03}.INI'])
    assert dict(old['MAP'])==dict(reference['MAP']),(rel,'terrain template')
    mapping={player:player};evidence=collections.defaultdict(collections.Counter)
    for section in ['UNITS','STRUCTURES','TEAMS','REINFORCEMENTS']:
     if section not in old or section not in reference:continue
     for key,value in old[section].items():
      if key in reference[section]:evidence[value.split(',')[0].strip()][reference[section][key].split(',')[0].strip()]+=1
    for house in NAMES:
     if house==player or not old.has_section(house):continue
     assert evidence[house],(rel,'unattributed opponent',house)
     canonical=evidence[house].most_common(1)[0][0];assert canonical in roles,(rel,house,canonical)
     mapping[house]=roles[canonical]
    assert len(set(mapping.values()))==len(mapping),(rel,'merged opponents',mapping)
    text=original.decode('cp850');token=re.compile(r'\b(?:'+ '|'.join(sorted(mapping,key=len,reverse=True))+r')\b')
    text=token.sub(lambda m:mapping[m[0]],text)
    # Normalize all custom/classic special ground vehicles to the existing map spawn marker.
    section=None;output=[];removed=[];converted=[]
    for line in text.splitlines():
     if line.startswith('['):section=line.strip('[]')
     if section in ['UNITS','REINFORCEMENTS'] and '=' in line:
      key,value=line.split('=',1);parts=[p.strip() for p in value.split(',')]
      if len(parts)>=2:
       if mission==1 and section=='UNITS' and parts[0]==player and parts[1] in SOLDIERS:removed.append(key.strip());continue
       if parts[1] in SPECIAL:parts[1]='Special';line=key+'='+','.join(parts);converted.append(section+':'+key.strip())
     output.append(line)
    eol='\r\n' if b'\r\n' in original else '\n'
    text=eol.join(output)+eol;updated=read(text.encode('cp850'))
    assert dict(updated[player])==dict(old[player]),(rel,'starting economy')
    for section in ['BASIC','MAP','CHOAM']:
     if section in old:assert dict(updated[section])==dict(old[section]),(rel,section)
    for section in ['UNITS','STRUCTURES','TEAMS','REINFORCEMENTS']:
     if section not in old:continue
     for key,value in old[section].items():
      if key in removed:continue
      a=value.split(',');b=updated[section][key].split(',');assert a[1:]==b[1:] or (section in ['UNITS','REINFORCEMENTS'] and a[1].strip() in SPECIAL and b[1]=='Special' and a[2:]==b[2:]),(rel,section,key)
    actual={h for h in NAMES if updated.has_section(h) and h!=player}
    expected={roles[h] for h in reference.sections() if h in roles and roles[h]!=player}
    assert actual==expected,(rel,actual,expected)
    text=add_opening_support(text,player,mission)
    (ROOT/rel).write_bytes(text.encode('cp850'))
    # Some legacy packs expose a second copy through the data search path.
    mirror=ROOT/f'mods/{mod}/data/scen{letter.lower()}{mission:03}.ini'
    if mirror.exists():mirror.write_bytes(text.encode('cp850'))
    reports.append({'mod':mod,'house':player,'mission':mission,'mapping':mapping,'removedStartingSoldierOrders':removed,'specialSpawnOrdersConverted':converted,'addedStartingSupport':[f'ID{i:03}' for i in range(100,107)] if mission==1 else [],'terrainAndEconomyPreserved':True,'vanillaOpponentRolesMatch':True})
   region=read(vanilla[f'REGION{template}.INI']);result=io.StringIO()
   for group in range(1,9):
    section=region['GROUP'+str(group)];new={}
    for key,value in section.items():
     if key in PREFIX.values():
      name=next(h for h,p in PREFIX.items() if p==key);assert name in roles;new[PREFIX[roles[name]]]=value
     elif key.startswith('REG'):new[key]=value
    for prefix,value in list(new.items()):
     if prefix not in PREFIX.values() or not value.strip():continue
     owner=next(h for h,p in PREFIX.items() if p==prefix);territory=value.split(',')[0].strip()
     for lang,sentence in [('ENG',f'{owner} advances across Arrakis.'),('FRE',f'{owner} progresse sur Arrakis.'),('GER',f'{owner} erobert neue Gebiete.')]:new[lang+'TXT'+territory]=sentence
    if group==8:
     for key in list(new):
      if 'TXT' in key:
       if key.startswith('ENG'):new[key]=row['opponents'][2]+' defends the final stronghold.'
       elif key.startswith('FRE'):new[key]=row['opponents'][2]+' défend la forteresse finale.'
       else:new[key]=row['opponents'][2]+' verteidigt die letzte Festung.'
    region.remove_section('GROUP'+str(group));region.add_section('GROUP'+str(group));region['GROUP'+str(group)].update(new)
   region.write(result)
   original_region=baseline[f'mods/{baseline_mod}/campaign/REGION{letter}.INI']
   eol='\r\n' if b'\r\n' in original_region else '\n'
   data=(result.getvalue().rstrip()+eol).replace('\r\n','\n').replace('\n',eol).encode('cp850')
   for folder in ['campaign','data']:(ROOT/f'mods/{mod}/{folder}/REGION{letter}.INI').write_bytes(data)
   opening=read((ROOT/f'mods/{mod}/campaign/scen{letter.lower()}001.ini').read_bytes())
   briefing+=f'\n[{player}]\nTemplate={template}\n'+''.join(f'Opponent{i+1}={name}\n' for i,name in enumerate(row['opponents']))+f"OpeningQuota={opening[player].get('Quota','0')}\n"
  (ROOT/f'mods/{mod}/data/CampaignPlan.ini').write_bytes(briefing.encode().replace(b'\n',b'\r\n'))
 intro_clones=opening_clones.clone_openings(plans,write=True)
 report={'version':'1.0.536','campaigns':sum(len(rows) for rows in plans['mods'].values()),'missions':len(reports),'baseline':'v1.0.534','maps':reports,'introClones':intro_clones,'removedStartingSoldierOrders':sum(len(r['removedStartingSoldierOrders']) for r in reports),'specialSpawnOrdersConverted':sum(len(r['specialSpawnOrdersConverted']) for r in reports)}
 return report
if __name__=='__main__':
 result=rebuild();print(json.dumps({k:v for k,v in result.items() if k!='maps'},indent=2))
 if len(__import__('sys').argv)>1:Path(__import__('sys').argv[1]).write_text(json.dumps(result,indent=2)+'\n')

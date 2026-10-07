#!/usr/bin/env python3
"""Apply the revised 1.0.535 opening balance without remapping later missions."""
from pathlib import Path
from importlib.util import spec_from_file_location, module_from_spec
import collections,configparser,hashlib,io,json,shutil,subprocess,tarfile

ROOT=Path(__file__).resolve().parents[1]
# Immutable source of the first published 1.0.535, since that tag is replaced.
BASELINE='fa2a5de3d23507f95332e243d060eeb419ce8fd5'

def read(text):
    p=configparser.ConfigParser(interpolation=None,strict=False);p.optionxform=str
    p.read_string('\n'.join(line for line in text.splitlines() if '=' in line or line.strip().startswith(('[',';','#'))))
    return p

def tune_opening(text,player):
    ini=read(text);eol='\r\n' if '\r\n' in text else '\n'
    kept=[];removed=[];passive=[];regular=[];section=None
    # Strip the earlier Wildspade-only bonus before applying the shared policy.
    previous_bonus={
        'ID045':f'{player},Tank,256,1566,64,Guard',
        'ID046':f'{player},Tank,256,1567,64,Guard',
        'ID047':f'{player},Troopers,256,1374,64,Guard',
    } if player=='Wildspade' else {}
    for line in text.splitlines():
        if line.startswith('['):section=line.strip('[]')
        if section=='UNITS' and '=' in line:
            key,value=line.split('=',1);parts=[p.strip() for p in value.split(',')]
            owner,unit=parts[:2]
            if owner==player:
                if unit in {'Trooper','Troopers','Special'} or previous_bonus.get(key)==value:
                    removed.append(key);continue
            else:
                # Six extra enemies were layered onto every original intro.
                if 39<=int(key.removeprefix('ID'))<=44:
                    removed.append(key);continue
                if unit=='Special':parts[1]='Tank';regular.append(key)
                # Kleshmersh disables the ordinary Trike in both mod trees.
                if unit=='Trike' and owner=='Kleshmersh':parts[1]='Raider Trike'
                if parts[5]=='Hunt':parts[5]='Area Guard';passive.append(key)
                # Original border entries should actually deploy inside the map.
                pos=int(parts[3]);x=max(1,min(62,pos%64));y=max(1,min(62,pos//64))
                parts[3]=str(x+64*y)
                line=key+'='+','.join(parts)
        kept.append(line)
    text=eol.join(kept)+eol;ini=read(text)
    occupied={int(v.split(',')[3]) for v in ini['UNITS'].values()}
    base=None
    for value in ini['STRUCTURES'].values():
        owner,item,health,position=value.split(',');pos=int(position)
        if owner==player and item=='Const Yard':base=pos
        # Both intro buildings are 2x2; reserving 3x3 also leaves access space.
        occupied.update(pos+dx+64*dy for dx in range(3) for dy in range(3))
    assert base is not None
    x,y=base%64,base//64
    candidates=[]
    for radius in range(1,6):
        ring=[(dx,dy) for dy in range(-radius,radius+1) for dx in range(-radius,radius+1) if max(abs(dx),abs(dy))==radius]
        for dx,dy in ring:
            pos=x+dx+64*(y+dy)
            if 1<=x+dx<=62 and 1<=y+dy<=62 and pos not in occupied:
                candidates.append(pos);occupied.add(pos)
    additions={}
    for index,unit in enumerate(['Tank','Tank','Troopers','Troopers','Troopers','Special','Special']):
        key=f'ID{100+index:03}';assert key not in ini['UNITS']
        additions[key]=f'{player},{unit},256,{candidates[index]},64,Guard'
    extra=eol.join(key+'='+value for key,value in additions.items())+eol+eol
    text=text.replace('[STRUCTURES]',extra+'[STRUCTURES]',1)
    # Five scattered sentries occupy empty passable perimeter cells. They do
    # not Hunt, and stay away from the player base and existing deployments.
    terrain=json.loads((ROOT/'config/OpeningTerrain.json').read_text())['seeds'][ini['MAP']['Seed']]
    enemy_owner=next(value.split(',')[0] for value in ini['UNITS'].values() if value.split(',')[0]!=player)
    patrols={};selected=[]
    for i,(ax,ay) in enumerate([(2,12),(2,52),(52,2),(61,48),(28,61)]):
        choices=[]
        for py in range(2,62):
            for px in range(2,62):
                pos=px+64*py
                if min(px,py,63-px,63-py)>4 or terrain[pos]=='a':continue
                if max(abs(px-x),abs(py-y))<12:continue
                if any(max(abs(px-(p%64)),abs(py-(p//64)))<6 for p in occupied):continue
                if any(max(abs(px-sx),abs(py-sy))<10 for sx,sy in selected):continue
                # A vehicle must also have a neighbouring passable exit.
                if all(terrain[pos+offset]=='a' for offset in [-1,1,-64,64]):continue
                choices.append((abs(px-ax)+abs(py-ay),py,px))
        assert choices,(player,'no passable patrol position')
        _,py,px=min(choices);selected.append((px,py));occupied.add(px+64*py)
        key=f'ID{200+i:03}';assert key not in ini['UNITS']
        unit='Tank' if i in [0,3] else 'Trooper'
        patrols[key]=f'{enemy_owner},{unit},256,{px+64*py},64,Area Guard'
    text=text.replace('[STRUCTURES]',eol.join(key+'='+value for key,value in patrols.items())+eol+eol+'[STRUCTURES]',1)
    additions.update(patrols)
    result=read(text);own=collections.Counter();enemy=collections.Counter()
    for value in result['UNITS'].values():
        owner,unit=value.split(',')[:2];(own if owner==player else enemy)[unit]+=1
    assert own['Troopers']==3 and own['Trooper']==0 and own['Special']==2
    assert sum(enemy.values())==17,(player,enemy)
    assert enemy['Special']==0
    assert all(v.split(',')[5]!='Hunt' for v in result['UNITS'].values() if v.split(',')[0]!=player)
    return text,{'player':dict(own),'enemies':dict(enemy),'enemyUnitOrders':17,
                 'addedOrders':additions,'removedOrders':removed,'rushOrdersChanged':passive,'enemySpecialsReplaced':regular}

def write_checksums(mod):
    path=ROOT/'mods'/mod/'checksums.sha256'
    lines=[]
    for line in path.read_text(encoding='utf-8').splitlines():
        _,relative=line.split(None,1);relative=relative.strip()
        lines.append(hashlib.sha256((path.parent/relative).read_bytes()).hexdigest()+'  '+relative)
    eol=b'\r\n' if b'\r\n' in path.read_bytes() else b'\n'
    path.write_bytes(eol.join(line.encode('utf-8') for line in lines)+eol)

def main():
    git=shutil.which('git') or 'C:/Program Files/Git/cmd/git.exe'
    raw=subprocess.check_output([git,'archive','--format=tar',BASELINE,'mods/Tornie/campaign','mods/Jericho/campaign'],cwd=ROOT)
    with tarfile.open(fileobj=io.BytesIO(raw)) as t:originals={m.name:t.extractfile(m).read() for m in t if m.isfile()}
    campaign_config=json.loads((ROOT/'config/CampaignPlans.json').read_text(encoding='utf-8'))
    plans=campaign_config['mods'];rows=[]
    for mod,plans in plans.items():
        if mod.endswith('Lite'):continue
        for plan in plans:
            name=f'scen{plan["letter"].lower()}001.ini';rel=f'mods/{mod}/campaign/{name}'
            original=originals[rel].decode('cp850');text,report=tune_opening(original,plan['house'])
            old,new=read(original),read(text)
            for section in ('BASIC','MAP','STRUCTURES',plan['house']):assert dict(old[section])==dict(new[section]),(rel,section)
            (ROOT/rel).write_bytes(text.encode('cp850'))
            mirror=ROOT/f'mods/{mod}/data/{name}'
            if mirror.exists():mirror.write_bytes(text.encode('cp850'))
            rows.append({'mod':mod,'house':plan['house'],**report})
        write_checksums(mod)
    clone_spec=spec_from_file_location('opening_clones',ROOT/'scripts/campaign-opening-clones.py')
    opening_clones=module_from_spec(clone_spec);clone_spec.loader.exec_module(opening_clones)
    intro_clones=opening_clones.clone_openings(campaign_config,write=True)
    for target_mod in campaign_config.get('openingClones', {}):write_checksums(target_mod)
    report={'version':'1.0.536','revision':'campaign-opening-balance','baseline':BASELINE,'intros':24,'introClones':intro_clones,
            'policy':{'bonusTanks':2,'troopersOrders':3,'individualTroopers':9,'specialOrders':2,'enemyOrders':17,'immediateHuntOrders':0},'campaigns':rows}
    import sys
    if len(sys.argv)>1:Path(sys.argv[1]).write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({k:v for k,v in report.items() if k!='campaigns'}))

if __name__=='__main__':main()

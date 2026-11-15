"""Private offline fixtures; extracted assets never ship in the candidate."""
import json,pathlib,sys,struct,re
from build_ui_method_contracts import index,contract
from build_recruit_notes_ui import fnv
from apt_source_pointers import source_pointers
sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
from inspect_tb_ast import load
ROOT=pathlib.Path(__file__).resolve().parents[3]
M=ROOT/'Modules/DynastyEnhancer'
def main():
    out=ROOT/'Build/UiDiscovery/Fixtures';out.mkdir(parents=True,exist_ok=True)
    notes=json.loads((M/'Evidence/RecruitingFog/notes-programs.json').read_text())
    reference=load(M/'Evidence/RecruitingFog/Assets/common__ui__node_cfm__recruitinghub__corenode.ast')
    desired=[contract(reference,r['definition']) for r in notes]
    vteam=pathlib.Path('C:/Apps/CFB 27/MMC/PlayerAnalytics/Research/assets/common/ui/node_cfm/corenode.ast')
    at=load(M/'Evidence/Aio141/chunks/b78eee29-09eb-265a-1167-4a724f97c001.raw')
    team=contract(at,3116176)
    cmap=json.loads((M/'Evidence/Aio141/method-map.json').read_text())
    group=(M/'Source/AioCurrencyUiPrograms.h').read_text().split('inline std::vector<notesui::Program> group3()',1)[1].split('return result;',1)[0]
    currency=[]
    for off in map(int,re.findall(r'result.push_back\(\{(\d+),',group)):
        row=next(r for r in cmap if r['new']==off);currency.append(contract(load(M/row['asset']),off))
    setups=[('vanilla',M/'Evidence/RecruitingFog/Assets/common__ui__node_cfm__recruitinghub__corenode.ast',vteam),
            ('aio',M/'Evidence/Aio141/chunks/4053c214-cac1-ede2-1167-4a724f97c001.raw',M/'Evidence/Aio141/chunks/b78eee29-09eb-265a-1167-4a724f97c001.raw'),
            ('zeo',pathlib.Path('C:/Apps/CFB 27/MMC/ModComparisons/ZEO101-TeamBuilder/combo/Assets/common/ui/node_cfm/recruitinghub/corenode.ast'),pathlib.Path('C:/Apps/CFB 27/MMC/ModComparisons/ZEO101-TeamBuilder/combo/Assets/common/ui/node_cfm/corenode.ast'))]
    for label,nfile,tfile in setups:
        report={}
        for family,file,wanted in [('notes',nfile,desired),('team',tfile,[team]),('currency',nfile,currency if label!='vanilla' else [])]:
            apt=load(file);idx=index(apt);rows=[]
            for c in wanted:
                candidates=[off for off,(pool,_,name) in idx.items() if name==c['name'] and pool[:8]==c['prefix']]
                assert len(candidates)==1,(label,c,candidates)
                off=candidates[0];pool,_,_=idx[off];size=struct.unpack_from('<I',apt.data,off+24)[0]
                rows.append(dict(offset=off,pool=pool,pointers=source_pointers(apt,off,size,[]),size=size,hash=fnv(apt.data[off+48:off+48+size])))
            (out/(label+'-'+family+'.bin')).write_bytes(apt.data)
            report[family]=rows
        (out/(label+'.json')).write_text(json.dumps(report),encoding='utf8')
        if label=='zeo':
            apt=load(tfile);idx=index(apt);off=next(off for off,(pool,_,name) in idx.items() if name=='MySchoolReport' and pool[:6]==['_global','logic_cfm','Object','model','myschoolreport','MySchoolReport'])
            pool,_,_=idx[off];size=struct.unpack_from('<I',apt.data,off+24)[0]
            (out/'zeo-team-seed.json').write_text(json.dumps(dict(offset=off,pool=pool,pointers=source_pointers(apt,off,size,[]),size=size)),encoding='utf8')
    from build_team_circle_ui import reviewed
    circleRows=[]
    for i,(apt,off,method,_) in enumerate(reviewed()):
        pool,_,_=index(apt)[off];size=struct.unpack_from('<I',apt.data,off+24)[0]
        (out/f'circle-{i}.bin').write_bytes(apt.data)
        circleRows.append(dict(offset=off,pool=pool,pointers=source_pointers(apt,off,size,[]),size=size,method=method))
    (out/'circles.json').write_text(json.dumps(circleRows),encoding='utf8')
    from build_week_module_ui import reviewed as week_module
    from build_weekly_matchup_ui import reviewed as weekly_tile
    for name,review in [('week-module',week_module),('weekly-tile',weekly_tile)]:
        apt,off,argc,regs,flags,size,body=review();pool,_,_=index(apt)[off]
        assert all(value.isascii() for value in pool)
        (out/(name+'.bin')).write_bytes(apt.data)
        (out/(name+'.json')).write_text(json.dumps([dict(offset=off,pool=pool,pointers=[])]))
    print(json.dumps(dict(fixtures=3,circleMethods=3,practiceBanners=2,privateDirectory=str(out))))
if __name__=='__main__':main()

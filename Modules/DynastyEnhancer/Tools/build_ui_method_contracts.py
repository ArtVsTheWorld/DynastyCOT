"""Emit symbolic UI contracts (identity/hashes only), never native method bodies.

Offsets in private evidence are used only to locate reviewed source methods at
build time. The generated contracts contain no asset/definition addresses.
"""
import json, pathlib, re, sys, struct
from build_recruit_notes_ui import fnv
from apt_source_pointers import source_pointers
from apt_ui_symbols import symbols

ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
from inspect_tb_ast import load

def index(apt):
    return symbols(apt)

def contract(apt,offset):
    pool,idx,name=index(apt)[offset]
    assert all(isinstance(x,str) for x in pool)
    return dict(name=name,prefix=pool[:8],count=len(pool),hash=fnv(b''.join(x.encode('latin1')+b'\0' for x in pool)),index=idx)

def cpp(c):
    return '{'+','.join([json.dumps(c['name']),'{'+','.join(json.dumps(x) for x in c['prefix'])+'}',str(c['count']),str(c['hash'])+'ull',str(c['index'])])+'}'

def main():
    vanilla=load(ROOT/'Evidence/RecruitingFog/Assets/common__ui__node_cfm__recruitinghub__corenode.ast')
    aio=load(ROOT/'Evidence/Aio141/chunks/4053c214-cac1-ede2-1167-4a724f97c001.raw')
    reports=json.loads((ROOT/'Evidence/RecruitingFog/notes-programs.json').read_text())
    notes=[contract(vanilla,r['definition']) for r in reports]
    # Every required Notes method already has the same complete instructions
    # and indexed constants. Only its location changed in the reviewed overhaul.
    aioindex=index(aio)
    for r,c in zip(reports,notes):
        matches=[off for off,(pool,idx,name) in aioindex.items() if name==c['name'] and pool[:8]==c['prefix']]
        assert len(matches)==1 and contract(aio,matches[0])==c
        assert fnv(aio.data[matches[0]+48:matches[0]+48+r['size']])==r['hash']
    team=load(ROOT/'Evidence/Aio141/chunks/b78eee29-09eb-265a-1167-4a724f97c001.raw')
    native=load(pathlib.Path('C:/Apps/CFB 27/MMC/PlayerAnalytics/Research/assets/common/ui/node_cfm/corenode.ast'))
    # These are body families, not mod-combination presets. Runtime does not
    # use the offsets or mod names to discover either implementation.
    teamcontracts=[contract(native,0x728d0),contract(team,3116176)]
    from build_week_module_ui import reviewed as week_module_reviewed
    week_module=contract(*week_module_reviewed()[:2])
    from build_weekly_matchup_ui import reviewed as weekly_reviewed
    weekly=contract(*weekly_reviewed()[:2])
    from build_team_circle_ui import reviewed
    circles=[contract(apt,off) for apt,off,_,_ in reviewed()]
    text=(ROOT/'Source/AioCurrencyUiPrograms.h').read_text()
    group=text.split('inline std::vector<notesui::Program> group3()',1)[1].split('return result;',1)[0]
    offsets=[int(x) for x in re.findall(r'result.push_back\(\{(\d+),',group)]
    rows=json.loads((ROOT/'Evidence/Aio141/method-map.json').read_text())
    currency=[];literals=[]
    def literal_contracts(apt,off,size):
        hash_=fnv(apt.data[off+48:off+48+size])
        for at,value in source_pointers(apt,off,size,[]):
            # Only reviewed ActionPushString operands may relocate by text.
            # A pointer to a constant array or nested header is not a string,
            # even if its first byte happens to be zero.
            assert apt.data[off+48+at-8:off+48+at].rstrip(b'\0')[-1]==0xa1
            text=apt.data[value:apt.data.index(0,value)].decode('latin1')
            assert len(text)<=1024 and all(ord(x)>=32 for x in text)
            literals.append((hash_,at,text))
    literal_contracts(team,3116176,4514)
    for off in offsets:
        row=next(r for r in rows if r['new']==off)
        apt=load(ROOT/row['asset']);currency.append(contract(apt,off));literal_contracts(apt,off,row['size'])
    lines=['// Generated symbolic identities and complete constant-prefix hashes. No native code.','#pragma once','namespace recruitingfog::uidiscovery {',
           'inline std::vector<Contract> notesContracts(){return {'+','.join(map(cpp,notes))+'};}',
           'inline std::vector<Contract> teamContracts(){return {'+','.join(map(cpp,teamcontracts))+'};}',
           'inline std::vector<Contract> weekModuleContracts(){return {'+cpp(week_module)+'};}',
           'inline std::vector<Contract> weeklyMatchupContracts(){return {'+cpp(weekly)+'};}',
           'inline std::vector<Contract> circleContracts(){return {'+','.join(map(cpp,circles))+'};}',
           'inline std::vector<Contract> currencyContracts(){return {'+','.join(map(cpp,currency))+'};}',
           'inline void semanticPointers(notesui::Program& p){']
    for hash_,at,text in literals:
        lines.append('if(p.hash=='+str(hash_)+'ull)for(auto& q:p.sourcePointers)if(q.offset=='+str(at)+'){q.text='+json.dumps(text)+';q.byText=true;}')
    lines+=['}','}']
    (ROOT/'Source/UiMethodContracts.generated.h').write_text('\n'.join(lines)+'\n',encoding='utf8')
    print(json.dumps(dict(notes=len(notes),teamFamilies=2,currency=len(currency),noRuntimeOffsets=True)))

if __name__=='__main__':main()

"""Generate independent exact AIO 1.4.1 Notes/color bank. Private AST input."""
from pathlib import Path
import json,sys,struct,tempfile,shutil,re,argparse
from build_recruit_notes_ui import generate_header,fnv
from apt_source_pointers import source_pointers
ROOT=Path(__file__).resolve().parents[1]
def generate(evidence,destination):
    sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools');from inspect_tb_ast import load
    source=ROOT/'Evidence/RecruitingFog/Assets/common__ui__node_cfm__recruitinghub__corenode.ast'
    target=ROOT/'Evidence/Aio141/chunks/4053c214-cac1-ede2-1167-4a724f97c001.raw'
    old,new=load(source),load(target);newroots=dict(new.root_streams());mapping={}
    for label,offset in old.root_streams():
        if label not in newroots:continue
        oi=old.decode(offset);ni=new.decode(newroots[label]);nm={}
        for i,item in enumerate(ni):
            if item[1] in (0x8e,0x9b):nm.setdefault(str(ni[i-1][2]),[]).append(item)
        for i,item in enumerate(oi):
            if item[1] not in (0x8e,0x9b):continue
            matches=nm.get(str(oi[i-1][2]),[])
            if len(matches)==1:mapping[int(item[2][3],16)-48]=matches[0][2]
    with tempfile.TemporaryDirectory() as tmp:
        work=Path(tmp);(work/'Assets').mkdir();shutil.copyfile(target,work/'Assets/common__ui__node_cfm__recruitinghub__corenode.ast')
        for name in ['notes-methods.json','notes-navigation.json','target-methods.json','notes-grade-methods.json']:
            rows=json.loads((evidence/name).read_text(encoding='utf8'))
            for row in rows:
                definition=row['definition'];oldoffset=int(definition[3],16)-48
                if oldoffset in mapping:row['definition']=mapping[oldoffset]
            (work/name).write_text(json.dumps(rows),encoding='utf8')
        header=work/'generated.h';report=generate_header(header,work)
        text=header.read_text(encoding='utf8');lines=[]
        for line in text.splitlines():
            if line.startswith(('struct SourcePointer ','struct Program ')):continue
            if line=='namespace recruitingfog::notesui {':line='namespace recruitingfog::aiorecruitnotesui {\nusing notesui::Program;'
            lines.append(line)
            match=re.match(r'result.push_back\(\{(\d+),(\d+),(\d+),(\d+),(\d+),',line)
            if match:
                off,argc,regs,flags,size=map(int,match.groups())
                for at,value in source_pointers(new,off,size):lines.append(f'result.back().sourcePointers.push_back({{{at},{value}}});')
        text='\n'.join(lines).replace('#pragma once','#pragma once\n#include "RecruitNotesUiPrograms.h"')
        destination.write_text(text+'\n',encoding='utf8')
        print(json.dumps({'aioNotesCallbacks':text.count('result.push_back('),'independentBank':True,'exactSourceFrames':True}))
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--evidence',type=Path,required=True);p.add_argument('--output',type=Path,default=ROOT/'Source/AioRecruitNotesUiPrograms.h');a=p.parse_args();generate(a.evidence,a.output)

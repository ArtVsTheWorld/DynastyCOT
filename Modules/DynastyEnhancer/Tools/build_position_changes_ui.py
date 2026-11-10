"""Grade projected Position Changes OVR text; preserve native position data."""
from pathlib import Path
import argparse,json,struct,sys
from build_recruit_notes_ui import Assembler,fnv
from build_player_card_ui import path,numeric_grade

BODY=0x4B678
SIZE=183
REGISTERS=11
EXPECTED=16293060561166550543

def suffix(a):
    # Native GetRowData returns r4, with requested column keys in r5. Column1
    # is projected OVR; column3 is depth/index and must remain numeric.
    a.missing(4,'done');a.missing(5,'done');a.number(0);a.store(3)
    a.label('loop');a.reg(3);path(a,5,'length');a.raw(0x48);a.branch('done',0xB8)
    a.reg(5);a.reg(3);a.raw(0x4E);a.number(1);a.raw(0x66);a.branch('next',0xB8)
    a.reg(4);a.reg(3);a.raw(0x4E);a.store(6);a.missing(6,'next')
    path(a,6,'display');a.store(7)
    # Native cells use a number or canonical integer text. A Number/string
    # round trip avoids the general 101-value whitelist on every row. NaN,
    # bounds and exact canonical spelling are checked before grading.
    a.reg(7);a.raw(0x44);a.string('number');a.raw(0x66);a.branch('number')
    a.reg(7);a.raw(0x44);a.string('string');a.raw(0x66);a.branch('next',0xB8)
    a.reg(7);a.raw(0x4A);a.store(8)
    a.reg(8);a.raw(0x4B);a.reg(7);a.raw(0x66);a.branch('next',0xB8)
    a.branch('grade',0x99)
    a.label('number');a.reg(7);a.store(8)
    a.label('grade');numeric_grade(a,8,8,'next','positionOvr')
    a.reg(6);a.string('display');a.reg(8);a.raw(0x4F)
    a.label('next');a.reg(3);a.raw(0x50);a.store(3);a.branch('loop',0x99)
    a.label('done');a.reg(4);a.raw(0x3E)

def emit(original=None):
    a=Assembler();a.code.extend(bytes(SIZE-3) if original is None else original[:-3]);suffix(a);return a.finish()

def generate(asset,output):
    sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
    from inspect_tb_ast import load
    apt=load(asset);native=apt.data[BODY:BODY+SIZE]
    assert fnv(native)==EXPECTED and native[-3:]==bytes.fromhex('b9043e')
    assert struct.unpack_from('<IHH',apt.data,BODY-48+8)==(2,REGISTERS,25)
    items=apt.decode(308504);assert items[0][2][5]=='PositionChangesProvider'
    definitions=[i for i,item in enumerate(items) if len(item)>3 and item[2][3]==hex(BODY)]
    assert len(definitions)==1 and items[definitions[0]-1][2][1]=='GetRowData'
    p=emit();lines=['// Generated projected-position OVR display adapter. Numeric models remain native.','#pragma once','#include "RecruitNotesUiPrograms.h"','namespace recruitingfog::positionchangesui {','inline std::vector<notesui::Program> programs(){std::vector<notesui::Program> result;']
    code='{'+','.join(hex(b) for b in p.code)+'}'
    lines.append(f'result.push_back({{{BODY-48},2,{REGISTERS},25,{SIZE},{EXPECTED}ull,{REGISTERS},0,{SIZE-3},{code}}});')
    for field,offsets in [('scaleOffsets',p.scale_offsets),('fogOffsets',p.fog_offsets),('colorOffsets',p.color_offsets)]:
        for at in offsets:lines.append(f'result.back().{field}.push_back({at});')
    for at,text in p.literals:lines.append('{const auto pointer=reinterpret_cast<uintptr_t>('+json.dumps(text)+f');memcpy(result.back().code.data()+{at},&pointer,8);}}')
    lines+=['return result;}','}'];output.write_text('\n'.join(lines)+'\n',encoding='utf8',newline='\n')
    print(json.dumps({'methods':1,'nativeBytesCopied':SIZE-3,'projectedOvrColumn':1,'nativeDataChanged':False}))

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--asset',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args();generate(args.asset,args.output)

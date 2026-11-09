"""Current NIL uses GenericToggle/DefaultToggleItem, not GenericSlider.
Only the verified RosterNILRowItem marker authorizes money presentation.
"""
from pathlib import Path
import json,struct,sys
from build_recruit_notes_ui import Assembler,fnv
from build_player_card_ui import path
from build_currency_ui import money
from build_currency_recruiteditor_ui import FixedFrameAssembler
ROOT=Path(__file__).resolve().parents[1]
def label(a,done='done'):
    a.dollars_enabled();a.number(1);a.raw(0x66);a.branch(done,0xb8)
    path(a,1,'mDECotCurrentNIL');a.raw(0x73,0x66);a.branch(done,0xb8)
    path(a,1,'mData','display');a.store(0);money(a,done,'itemMoney')
    path(a,1,'txtLabel');a.store(0);a.missing(0,done)
    path(a,0,'mData','display');a.reg(2);a.raw(0x66);a.branch(done)
    a.reg(2);a.number(1);a.reg(0);a.call('SetLabel')
    a.label(done);a.ret()

def draw(a):
    a.dollars_enabled();a.number(1);a.raw(0x66);a.branch('parentDone',0xb8)
    path(a,1,'mDECotCurrentNIL');a.raw(0x73,0x66);a.branch('parentDone',0xb8)
    path(a,1,'mCollectionControl');a.store(4);a.missing(4,'parentDone')
    a.number(0);a.reg(4);a.string('GetCurrentIndex');a.raw(0x52)
    a.number(1);a.reg(4);a.string('GetItem');a.raw(0x52);a.store(4);a.missing(4,'parentDone')
    path(a,4,'Movie');a.store(4);a.missing(4,'parentDone')
    a.set_field(4,'mDECotCurrentNIL',True)
    a.reg(4);a.store(1);label(a,'parentDone')

def generate(out):
    sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools');from inspect_tb_ast import load
    apt=load(Path('C:/Apps/CFB 27/MMC/PlayerAnalytics/Research/assets/common/ui/node_com/scriptpod.ast'))
    lines=['#pragma once','#include "RecruitNotesUiPrograms.h"','namespace recruitingfog::niltoggleui {\ninline std::vector<notesui::Program> programs(){std::vector<notesui::Program> r;']
    for off,size,argc,regs,flags,suffix in [(0x59e18,1924,0,10,25,draw),(0x56bd0,20,1,4,25,label)]:
        assert struct.unpack_from('<IHH',apt.data,off+8)==(argc,regs,flags)
        a=FixedFrameAssembler(regs);a.code.extend(bytes(size));suffix(a);p=a.finish()
        original=apt.data[off+48:off+48+size]
        lines.append('r.push_back({'+f'{off},{argc},{regs},{flags},{size},{fnv(original)}ull,{regs},0,{size},'+'{'+','.join(hex(b) for b in p.code)+'}});')
        for o in p.dollar_offsets:lines.append(f'r.back().dollarOffsets.push_back({o});')
        for o,t in p.literals:lines.append('{const auto p=reinterpret_cast<uintptr_t>('+json.dumps(t)+f');memcpy(r.back().code.data()+{o},&p,8);}}')
    lines.append('return r;}\n}');out.write_text('\n'.join(lines)+'\n')
if __name__=='__main__':generate(ROOT/'Source/NilToggleUiPrograms.h')


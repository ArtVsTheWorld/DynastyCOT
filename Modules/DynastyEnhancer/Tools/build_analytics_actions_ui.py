"""Analytics uses one native Roster flow, with fingerprinted scoped adapters.

Only custom code/contracts are emitted; native bodies are copied at runtime.
"""
from pathlib import Path
import json
import struct
import sys
from build_recruit_notes_ui import Assembler, fnv
from build_player_card_ui import path
from apt_source_pointers import source_pointers


def row_changed(a):
    # Only an actual user row event supplies explicit selection intent. Native
    # refresh rows retain their original handler and never replace that intent.
    path(a,1,'mCOTAnalytics'); a.raw(0x73,0x66); a.branch('original',0xB8)
    path(a,1,'mCOTRefreshDepth'); a.branch('original')
    path(a,1,'mCOTCardTransition'); a.branch('original')
    path(a,1,'mSpreadsheetInitialized'); a.raw(0x73,0x66); a.branch('done',0xB8)
    path(a,1,'mIsSpreadsheetEmpty'); a.branch('done')
    a.missing(28,'done'); path(a,28,'Data'); a.store(3); a.missing(3,'done')
    path(a,3,'NewRowIndex'); a.store(3)
    a.reg(3); a.raw(0x44); a.string('number'); a.raw(0x66); a.branch('done',0xB8)
    a.reg(3); a.number(0); a.raw(0x48); a.branch('done')
    a.reg(3); a.number(1); path(a,1,'mSpreadsheetDataProvider'); a.string('GetRowKey'); a.raw(0x52); a.store(7)
    a.reg(7); a.raw(0x44); a.string('string'); a.raw(0x66); a.branch('done',0xB8)
    a.reg(7); a.string(''); a.raw(0x66); a.branch('done')
    a.reg(1); a.string('mCurrentRowIndex'); a.reg(3); a.raw(0x4F)
    a.string('COT_SELECT:'); a.reg(7); a.raw(0x47); a.number(1); path(a,1,'mRosterAdapter'); a.call('GetEntryInfo')
    a.branch('original',0x99)
    a.label('done'); a.ret(); a.label('original'); a.pad_code()


def init(a):
    a.set_field(1,'mCOTRefreshDepth',0)
    a.set_field(1, 'mCOTAnalytics', True)
    a.set_field(1, 'mCOTCardTransition', False)
    a.set_field(1, 'mUseBackupRowCol', False)
    # Ordinary Roster has many more columns. Its saved focus/sort parameters
    # must never be reused as Analytics' eight-column native grid state.
    a.empty_object(0); a.set_field(0, 'currentRow', 0); a.set_field(0, 'currentCol', 5)
    a.reg(1); a.string('mBackupState'); a.reg(0); a.raw(0x4F)
    a.set_field(1, 'mCurrentRowIndex', 0); a.set_field(1, 'mCurrentColIndex', 5)
    a.ret()


def ticket(a, failure):
    a.string('COT_CARD:'); path(a, 1, 'mCurrentRowIndex'); a.number(1)
    path(a, 1, 'mSpreadsheetDataProvider'); a.string('GetRowKey'); a.raw(0x52, 0x47)
    a.number(1); path(a, 1, 'mRosterAdapter'); a.string('GetEntryInfo'); a.raw(0x52); a.store(0)
    a.missing(0, failure)
    path(a, 0, 'cotCardReady'); a.raw(0x73, 0x66); a.branch(failure, 0xB8)


def call_base_input(a, module=False):
    # Function.call retains this strategy as the native base method's receiver.
    a.reg(4); a.reg(1); a.number(2)
    if module:
        a.global_object('madden', 'screen', 'generic', 'SpreadsheetModuleStrategy', 'prototype', 'HandleInput')
    else:
        a.global_object('madden', 'ultimatecareer', 'screen', 'spreadsheets',
                        'UcmSpreadsheetModuleStrategy', 'prototype', 'HandleInput')
    a.string('call'); a.raw(0x52,0x3E)


def input_action(a):
    path(a, 1, 'mCOTAnalytics'); a.raw(0x73, 0x66); a.branch('original', 0xB8)
    for scope, key, label in [
        (('cafe', 'enum', 'Input'), 'CODE_TRIANGLE', 'consume'),
        (('cafe', 'enum', 'Input'), 'CODE_TRIANGLE_U', 'viewCard'),
        (('shared_enum', 'Input'), 'CODE_PRIMARY_ACTION', 'viewCard'),
        (('cafe', 'enum', 'Input'), 'CODE_R3', 'consume'),
    ]:
        path(a, 4, 'KeyCode'); a.global_object(*scope, key); a.raw(0x66); a.branch(label)
    # Keep native carousel input, then the common grid/filter/sort/back handler.
    # Do not run the Roster/AIO hold-to-auto-progress or editing handlers.
    a.reg(4); a.number(1); path(a, 1, 'mcSpreadsheetPlayerModule')
    a.string('HandleInput'); a.raw(0x52); a.branch('consume')
    for scope, key in [(('ion', 'enum', 'Input'), 'CODE_BACK'),
                       (('shared_enum', 'Input'), 'CODE_SECONDARY_ACTION')]:
        path(a, 4, 'KeyCode'); a.global_object(*scope, key); a.raw(0x66); a.branch('nativeBackSort')
    call_base_input(a, module=True)
    a.label('nativeBackSort'); call_base_input(a)
    a.label('viewCard')
    a.number(0); a.reg(1); a.call('_ShowManagePlayerScreen')
    a.branch('consume', 0x99)
    a.label('consume'); a.raw(0x73, 0x3E)
    a.label('original'); a.pad_code()


def show_card(a):
    path(a, 1, 'mCOTAnalytics'); a.raw(0x73, 0x66); a.branch('original', 0xB8)
    path(a, 1, 'mSpreadsheetInitialized'); a.raw(0x73, 0x66); a.branch('done', 0xB8)
    path(a, 1, 'mIsSpreadsheetEmpty'); a.branch('done')
    path(a, 1, 'mCOTCardTransition'); a.branch('done')
    ticket(a, 'done'); a.set_field(1, 'mCOTCardTransition', True)
    a.branch('original', 0x99)
    a.label('done'); a.ret()
    a.label('original'); a.pad_code()


def shutdown(a):
    # Original Roster Shutdown is the sole listener/flow lifecycle owner.
    path(a, 1, 'mCOTAnalytics'); a.raw(0x73, 0x66); a.branch('original', 0xB8)
    a.string('COT_LEAVE_VIEW'); a.number(1); path(a, 1, 'mRosterAdapter'); a.call('GetEntryInfo')
    a.set_field(1, 'mCOTAnalytics', False)
    a.set_field(1, 'mCOTCardTransition', False)
    a.label('original')
    a.pad_code()


def restore_selection(a):
    path(a, 1, 'mCOTAnalytics'); a.raw(0x73, 0x66); a.branch('done', 0xB8)
    path(a, 1, 'mRosterAdapter'); a.store(3); a.missing(3, 'done')
    a.string('COT_RETURN_SELECTION'); a.number(1); a.reg(3)
    a.string('GetEntryInfo'); a.raw(0x52); a.store(3); a.missing(3, 'done')
    path(a, 3, 'cotReturnRow'); a.store(3)
    a.reg(3); a.raw(0x44); a.string('number'); a.raw(0x66); a.branch('done', 0xB8)
    a.reg(3); a.number(0); a.raw(0x48); a.branch('done')
    a.set_field(1,'mCOTCardTransition',False)
    a.reg(1); a.string('mCurrentRowIndex'); a.reg(3); a.raw(0x4F)
    path(a, 1, 'mCurrentColIndex'); a.reg(3); a.number(2)
    path(a, 1, 'mcSpreadsheet'); a.call('ResetData')
    a.label('done')
    path(a,1,'mCOTAnalytics'); a.raw(0x73,0x66); a.branch('return',0xB8)
    a.reg(1); a.string('mCOTRefreshDepth'); path(a,1,'mCOTRefreshDepth'); a.number(1); a.raw(0x0B,0x4F)
    a.label('return'); a.ret()


def refresh_prefix(a):
    path(a, 1, 'mCOTAnalytics'); a.raw(0x73, 0x66); a.branch('nativeRefresh', 0xB8)
    a.set_field(1, 'mUseBackupRowCol', False)
    a.reg(1); a.string('mCOTRefreshDepth'); path(a,1,'mCOTRefreshDepth'); a.number(1); a.raw(0x47,0x4F)
    a.label('nativeRefresh'); a.pad_code()


def hints(a):
    path(a, 1, 'mCOTAnalytics'); a.raw(0x73, 0x66); a.branch('done', 0xB8)
    a.number(0); a.store(2); a.label('loop')
    a.reg(2); path(a, 3, 'length'); a.raw(0x48); a.branch('done', 0xB8)
    for label in ['Back', 'Sort', 'View Player Card']:
        a.reg(3); a.reg(2); a.raw(0x4E); a.member('Label'); a.string(label); a.raw(0x66); a.branch('keep')
    a.number(1); a.reg(2); a.number(2); a.reg(3); a.call('splice')
    a.branch('loop', 0x99)
    a.label('keep'); a.reg(2); a.raw(0x50); a.store(2); a.branch('loop', 0x99)
    a.label('done'); a.reg(3); a.raw(0x3E)


ORIGINAL = [('init', 0x503f0), ('input', 0x508a0), ('show', 0x514a0),
            ('cleanup', 0x50800), ('refresh', 0x511b8), ('hints', 0x50a50),
            ('row',0x50b28)]
CURRENT = [('init', 0xc5d38), ('input', 0xc61e8), ('show', 0xc7ea0),
           ('cleanup', 0xc6148), ('refresh', 0xc7bb8), ('hints', 0xc7070),
           ('row',0xc7370)]


def emit(lines, apt, specs, function):
    data = apt.data
    lines.append(f'inline std::vector<notesui::Program> {function}(){{std::vector<notesui::Program> result;')
    for kind, off in specs:
        header = data[off:off+48]
        argc = struct.unpack_from('<I', header, 8)[0]
        regs, flags = struct.unpack_from('<HH', header, 12)
        size = struct.unpack_from('<I', header, 24)[0]
        expected = {'init': 1, 'input': 1, 'show': 0, 'cleanup': 0, 'refresh': 3, 'hints': 0, 'row':1}[kind]
        if argc != expected or regs > (34 if kind=='row' else 12) or flags not in (25, 41) or not 1 <= size <= 4096:
            raise ValueError((kind, hex(off), argc, regs, flags, size))
        if kind in ('input','row'):
            params=struct.unpack_from('<Q',header,16)[0]
            expected_event=4 if kind=='input' else 28
            if struct.unpack_from('<I',data,params)[0]!=expected_event:
                raise ValueError('Unsupported native Analytics event parameter register')
        original = data[off+48:off+48+size]
        a = Assembler()
        if kind == 'init':
            copy_at, count = 0, size; a.code.extend(bytes(count)); init(a)
        elif kind == 'refresh':
            refresh_prefix(a); copy_at, count = len(a.code), size
            a.code.extend(bytes(count)); restore_selection(a)
        elif kind == 'hints':
            ending = original.rfind(bytes([0xb9, 3, 0x3e]))
            if ending < 0 or any(original[ending+3:]): raise ValueError('Unexpected native hints return')
            copy_at, count = 0, ending; a.code.extend(bytes(count)); hints(a)
        else:
            {'input': input_action, 'show': show_card, 'cleanup': shutdown,'row':row_changed}[kind](a)
            copy_at, count = len(a.code), size; a.code.extend(bytes(count))
        program = a.finish()
        code = '{' + ','.join(hex(x) for x in program.code) + '}'
        lines.append(f'result.push_back({{{off},{argc},{regs},{flags},{size},{fnv(original)}ull,{max(regs,8)},{copy_at},{count},{code}}});')
        for at, value in program.literals:
            lines.append('{const auto pointer=reinterpret_cast<uintptr_t>(' + json.dumps(value) +
                         f');memcpy(result.back().code.data()+{at},&pointer,8);}}')
        nested = []
        for at, value in source_pointers(apt, off, size, nested):
            lines.append(f'result.back().sourcePointers.push_back({{{at},{value}}});')
        for at, name, params, empty in nested:
            lines.append(f'result.back().nestedDefinitions.push_back({{{at},{name},{params},{str(empty).lower()}}});')
    lines.append('return result;}')


def generate(destination):
    sys.path.insert(0, 'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
    from inspect_tb_ast import load
    root = Path('C:/Apps/CFB 27/MMC/PlayerAnalytics/Research/assets/common/ui/node_cfm')
    candidates = list(Path('C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Compatibility').glob('AIO-Dynasty-1.4.1*/**/hub/corenode.ast'))
    if len(candidates) != 1: raise ValueError('One approved AIO hub reference is required')
    vanilla, current = load(root / 'hub/corenode.ast'), load(candidates[0])
    lines = ['// Generated custom Analytics adapters; native bodies verified before copying.',
             '#pragma once', '#include "RecruitNotesUiPrograms.h"',
             'namespace recruitingfog::analyticsactions {']
    for data, specs, prefix in [(vanilla, ORIGINAL, 'hub'), (current, CURRENT, 'current')]:
        for group, kinds in [('Main', ['init', 'input', 'show']), ('Cleanup', ['cleanup']),
                             ('Refresh', ['refresh']), ('Hints', ['hints']), ('Row',['row'])]:
            emit(lines, data, [(k, o) for k, o in specs if k in kinds], prefix + group + 'Programs')
    lines.append('}')
    destination.write_text('\n'.join(lines) + '\n', encoding='utf8')


if __name__ == '__main__':
    generate(Path('Source/AnalyticsActionsUiPrograms.h'))

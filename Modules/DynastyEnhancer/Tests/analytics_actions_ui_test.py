"""Execute Analytics action, hints and teardown without a game process."""
from pathlib import Path
import inspect
import math
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'Tools'))
from build_recruit_notes_ui import Assembler
from build_analytics_actions_ui import init, input_action, shutdown, hints, restore_selection, show_card, row_changed, refresh_prefix
from recruit_notes_vm import Machine, UNDEFINED, ScriptFunction


class ReceiverMethod:
    def __init__(self, owner, name): self.owner, self.name = owner, name
    def call(self, receiver, args):
        assert receiver is self.owner, 'Bound native/service method lost its original receiver'
        return (self.name, tuple(args))


class ActionMachine(Machine):
    def invoke(self, function, receiver, args):
        if isinstance(function, ReceiverMethod): return function.call(receiver, args)
        return super().invoke(function, receiver, args)

    @staticmethod
    def member(receiver, key):
        if isinstance(receiver, list) and key == 'splice':
            def splice(start, count):
                removed = receiver[start:start+count]
                del receiver[start:start+count]
                return removed
            return splice
        if key == 'call' and (callable(receiver) or isinstance(receiver, ReceiverMethod)):
            return lambda this, *args: machine.invoke(receiver, this, list(args))
        return Machine.member(receiver, key)


# The fixture adds only the verified AS2 NewMethod operation used by HintData.
namespace = dict(math=math, struct=struct, UNDEFINED=UNDEFINED, ScriptFunction=ScriptFunction)
source = inspect.getsource(Machine.run).replace('    def run(', 'def run(', 1).replace('\n    ', '\n')
source = source.replace('elif op in (0x52, 0xB2):', '''elif op == 0x53:
                name = stack.pop()
                receiver, count = stack.pop(), stack.pop()
                args = [stack.pop() for _ in range(count)]
                stack.append(self.invoke(self.member(receiver, name), receiver, args))
            elif op in (0x52, 0xB2):''')
exec(source, namespace)
ActionMachine.run = namespace['run']


def program(builder):
    a = Assembler()
    builder(a)
    return a.finish()



calls = []


class NativeInput(ReceiverMethod):
    def __init__(self): pass
    def call(self, receiver, args):
        assert isinstance(receiver,dict) and 'mContext' in receiver, 'Native base lost the strategy receiver'
        key=args[0]['KeyCode']
        calls.append(('base',key))
        return True


base = {'HandleInput': NativeInput(),
        'DELAY_ROW_CHANGED': 'Native Ucm Row Changed'}
generic = {'HandleInput': lambda event: calls.append(('generic', event['KeyCode'])) or False}
globals_ = {'madden': {'ultimatecareer': {'screen': {'spreadsheets': {'UcmSpreadsheetModuleStrategy': {'prototype': base, 'DELAY_ROW_CHANGED': base['DELAY_ROW_CHANGED']}}}},
                       'screen': {'generic': {'SpreadsheetModuleStrategy': {'prototype': generic}}}},
            'cafe': {'enum': {'Input': {'CODE_TRIANGLE': 20, 'CODE_TRIANGLE_U': 21, 'CODE_R3': 9}},
                     'manager': {'IntervalManagerService': {'Get': lambda *_: (_ for _ in ()).throw(AssertionError('Removed Analytics timer API was called'))}}},
            'shared_enum': {'Input': {'CODE_PRIMARY_ACTION': 7, 'CODE_SECONDARY_ACTION': 6, 'CODE_TERTIARY_ACTION': 20}},
            'ion': {'enum': {'Input': {'CODE_BACK': 10}}}}
machine = ActionMachine(globals_, [])


def with_native_row():
    a = Assembler(); row_changed(a)
    a.reg(28); a.number(1); a.reg(1); a.call('NativeRowChanged'); a.ret()
    return a.finish()


row_program = with_native_row()


def receiver_fixture(context=None):
    value = {'mContext': context if context is not None else {}, 'mSpreadsheetInitialized': True,
             'mIsSpreadsheetEmpty': False, 'mCurrentRowIndex': 4,
             'mSpreadsheetDataProvider': {'GetRowKey': lambda row: 'native-key-' + str(row)},
             'mRosterAdapter': {'GetEntryInfo': lambda key: calls.append(('ticket', key)) or {'cotCardReady': True},
                               'ConvertSerialEntry': lambda key: calls.append(('convert', key)) or {'entryKey': 'entry-' + key},
                               'SetSelectedEntry': lambda key: calls.append(('selected', key))},
             'mCommandHelper': {'RefreshCommands': lambda keys: calls.append(('commands', keys))},
             'mcSpreadsheetPlayerModule': {'HandleInput': lambda event: calls.append(('small', event['KeyCode'])) or event['KeyCode'] in [8,11]},
             '_ShowManagePlayerScreen': lambda: calls.append(('card', 4)),
             'NativeRowChanged': lambda event: calls.append(('native-row', event['Data']['NewRowIndex']))}
    value['mRosterAdapter']['SetSelectedEntry']=lambda key:(value.__setitem__('nativeSelectedEntry',key),calls.append(('selected',key)))[1]
    value['mCommandHelper']['RefreshCommands']=lambda keys:(value['mCommandHelper'].__setitem__('recordKeys',keys),calls.append(('commands',keys)))[1]
    value['NativeRowChanged']=lambda event:(value.setdefault('nativeEvents',[]).append(event),calls.append(('native-row',event['Data']['NewRowIndex'])))[1]
    value['_RowChanged'] = lambda event: machine.run(row_program, {1: value, 28: event})
    machine.run(program(init), {1: value})
    return value


receiver = receiver_fixture()
assert receiver['mCOTAnalytics'] and not receiver['mUseBackupRowCol']
assert receiver['mBackupState'] == {'currentRow': 0, 'currentCol': 5}
receiver['mCurrentRowIndex'] = 4
assert not calls, 'Native Roster Init alone owns flow entry; no second flow/service/proxy'
for key, expected in [(7, [('card', 4)]), (21, [('card', 4)]), (20, []),
                      (9, []),
                      (10, [('small', 10), ('base', 10)]), (6, [('small', 6), ('base', 6)]),
                      (8,[('small',8)]), (11, [('small', 11)]), (12, [('small', 12), ('generic', 12)])]:
    calls.clear(); machine.run(program(input_action), {1: receiver, 4: {'KeyCode': key}})
    assert calls == expected, (key, calls)
calls.clear(); machine.run(program(show_card), {1: receiver})
assert calls == [('ticket', 'COT_CARD:native-key-4')] and receiver['mCOTCardTransition']
calls.clear(); machine.run(program(show_card), {1: receiver}); assert not calls
receiver['mCOTCardTransition'] = False
ready_getter = receiver['mRosterAdapter']['GetEntryInfo']
for unavailable in [None, UNDEFINED, {}, {'cotCardReady': False}, {'cotCardReady': 1}]:
    receiver['mRosterAdapter']['GetEntryInfo'] = lambda key, value=unavailable: calls.append(('blocked-ticket', key)) or value
    calls.clear(); machine.run(program(show_card), {1: receiver})
    assert not receiver['mCOTCardTransition'] and calls == [('blocked-ticket', 'COT_CARD:native-key-4')]
    calls.clear(); machine.run(program(input_action), {1: receiver, 4: {'KeyCode': 9}})
    assert calls == [] and not receiver['mCOTCardTransition'], 'Disabled Stats action requested data or navigation'
receiver['mRosterAdapter']['GetEntryInfo'] = ready_getter
for field in ['mSpreadsheetInitialized', 'mIsSpreadsheetEmpty']:
    old = receiver[field]; receiver[field] = field == 'mIsSpreadsheetEmpty'; calls.clear()
    machine.run(program(show_card), {1: receiver}); machine.run(program(input_action), {1: receiver, 4: {'KeyCode': 9}})
    assert not calls; receiver[field] = old
rows = [{'Label': label} for label in ['Back', 'Auto Progress (Hold)', 'Sort', 'Tamper ($1M-$8M)', 'View Player Card', 'View Player Stats']]
assert machine.run(program(hints), {1: receiver, 3: rows}) is rows
assert [r['Label'] for r in rows] == ['Back', 'Sort', 'View Player Card']
machine.run(program(shutdown), {1: receiver})
assert not receiver['mCOTAnalytics'] and not receiver['mCOTCardTransition'] and calls[-1] == ('ticket', 'COT_LEAVE_VIEW')
calls.clear(); machine.run(program(input_action), {1: receiver, 4: {'KeyCode': 7}})
assert not calls, 'Ordinary Roster must reach untouched native input body'
for row in [-1, 0, 7]:
    resets = []
    returned = {'mCOTAnalytics': True, 'mCOTRefreshDepth': 1, 'mRosterAdapter': {'GetEntryInfo': lambda key: {'cotReturnRow': row}},
                'mCurrentColIndex': 5, 'mcSpreadsheet': {'ResetData': lambda *args: resets.append(args)}}
    machine.run(program(restore_selection), {1: returned})
    assert resets == ([] if row < 0 else [(row, 5)]) and returned['mCOTRefreshDepth'] == 0
for returned in [{}, {'mCOTAnalytics': False}]: machine.run(program(restore_selection), {1: returned})


# User changes immediately identify the exact player before the native body;
# there is no mod-owned delay, callback, pending event or instance counter.
receiver=receiver_fixture();calls.clear();events=[]
for row in [0,1,9]:
    event={'Data':{'NewRowIndex':row},'nativeDetail':object()};events.append(event)
    receiver['_RowChanged'](event)
    assert receiver['mCurrentRowIndex']==row
assert calls==[(kind,value) for row in [0,1,9] for kind,value in [('ticket','COT_SELECT:native-key-'+str(row)),('native-row',row)]]
assert all(left is right for left,right in zip(receiver['nativeEvents'],events)), 'Original row event/register28 changed'
assert not any('Pending' in key or 'Delay' in key or 'Token' in key or 'Instance' in key for key in receiver)

for invalid in [None,{}, {'Data':None},{'Data':{'NewRowIndex':-1}},{'Data':{'NewRowIndex':'7'}}]:
    calls.clear();receiver['_RowChanged'](invalid);assert not calls
for ordinary,depth,transition in [(False,0,False),(True,1,False),(True,0,True)]:
    receiver=receiver_fixture();receiver['mCOTAnalytics']=ordinary;receiver['mCOTRefreshDepth']=depth;receiver['mCOTCardTransition']=transition
    calls.clear();event={'Data':{'NewRowIndex':3}};receiver['_RowChanged'](event)
    assert calls==[('native-row',3)] and receiver['nativeEvents'][-1] is event, 'Programmatic/transition/native Roster event supplied user intent'

# A Back restored through native Refresh need not run Init/Shutdown. Clear the
# Card navigation guard only for a valid completed exact-row return.
for restored in [-1,4]:
    receiver=receiver_fixture();receiver['mCurrentRowIndex']=4;calls.clear()
    machine.run(program(show_card),{1:receiver});assert receiver['mCOTCardTransition']
    receiver['mRosterAdapter']['GetEntryInfo']=lambda key,r=restored:{'cotReturnRow':r} if key=='COT_RETURN_SELECTION' else calls.append(('ticket',key)) or {'cotCardReady':True}
    receiver['mcSpreadsheet']={'ResetData':lambda row,col:calls.append(('reset',row,col))}
    machine.run(program(refresh_prefix),{1:receiver});machine.run(program(restore_selection),{1:receiver})
    assert receiver['mCOTCardTransition']==(restored<0) and receiver['mCOTRefreshDepth']==0
    calls.clear();machine.run(program(show_card),{1:receiver})
    assert bool(calls)==(restored>=0), 'Cardâ†’Back left Card permanently blocked or prematurely admitted it'

# Unsupported Analytics Stats is hidden and R3 is consumed without ticket,
# object allocation, transition mutation or calling a native Stats handler.
receiver=receiver_fixture();calls.clear();before=dict(receiver)
assert machine.run(program(input_action),{1:receiver,4:{'KeyCode':9}}) is True
assert not calls and receiver==before
assert not any('COT_CARD:' in text or text in ['ConvertSerialEntry','SetSelectedEntry','RefreshCommands'] for _,text in program(input_action).literals)
a=Assembler();input_action(a);a.reg(4);a.number(1);a.reg(1);a.call('NativeInput');a.ret()
receiver['mCOTAnalytics']=False;event={'KeyCode':9};seen=[];receiver['NativeInput']=lambda value:seen.append(value)
machine.run(a.finish(),{1:receiver,4:event})
assert seen==[event] and seen[0] is event, 'Ordinary Roster R3 did not retain its exact native input event'

# These group callbacks contain only authored prefixes and exact full-body
# contracts. No experimental timer names/functions remain in generated code.
import build_analytics_actions_ui as generator
assert ('row',0x50b28) in generator.ORIGINAL and ('row',0xc7370) in generator.CURRENT
assert not any(kind=='filter' for kind,_ in generator.ORIGINAL+generator.CURRENT)
for builder in [init,input_action,show_card,shutdown,row_changed,refresh_prefix,restore_selection]:
    literals=[text for _,text in program(builder).literals]
    assert not any(any(part in text for part in ['Delay','Interval','Token','Deferred','PendingRow','Instance']) for text in literals), (builder.__name__,literals)
assert 'CODE_R3' not in globals_['ion']['enum']['Input']
print('PASS: Analytics Stats shortcut hidden/R3 consumed without allocation or navigation; ordinary Roster R3 preserved; Card/Back/direct selection/native event guards; no experimental timer.')

from pathlib import Path
import json,hashlib,re
ROOT=Path(__file__).resolve().parents[1]
MOD=ROOT/'Modules/ExtraDynastySettings'
changed=[]
def edit(relative,fn):
    path=MOD/relative
    old=path.read_text(encoding='utf-8-sig')
    new=fn(old)
    if new==old:raise RuntimeError('No change: '+relative)
    path.write_text(new,encoding='utf-8',newline='\n')
    changed.append(path)
def replace(source,old,new,count=None):
    n=source.count(old)
    if not n or (count is not None and n!=count):raise RuntimeError(f'Missing/ambiguous target ({n}): {old[:80]}')
    return source.replace(old,new)
def action(s):
    s=replace(s,'bool extended = LoadExtended();','bool extended = false; // COT uses only per-Dynasty choices.',1)
    s=replace(s,'IntPtr data = VirtualAllocEx(h, IntPtr.Zero, new UIntPtr(4096), 0x3000, 4);','IntPtr data = VirtualAllocEx(h, IntPtr.Zero, new UIntPtr(OfflineDynastySettings.DynastyClockProfiles.AllocationSize), 0x3000, 4);',1)
    s=replace(s,'byte[] bytes = new byte[4096];','byte[] bytes = new byte[OfflineDynastySettings.DynastyClockProfiles.AllocationSize];',1)
    s=replace(s,'BuildCombinedDefensiveClockData(bytes,d);','BuildCombinedDefensiveClockData(bytes,d);\n            SeedDynastyClocks(bytes);',1)
    s=s.replace('Applies across offline Dynasties.','Saved separately for this Dynasty.')
    start=s.index('            c.Hex("4883ec3089442420");',s.index('private static byte[] BuildWriterFor'))
    end=s.index('            c.Label("end");',start)
    s=s[:start]+'''            AppendDynastyClockWrite(c,b,d,false,"cotOffWrite");
            c.Branch("e9","end");
'''+s[end:]
    s=replace(s,'c.Label("saveDefensiveClock");c.MovR10(d);c.Hex("418982800800004189825c09000041ff8284080000");c.Branch("e9","end");','c.Label("saveDefensiveClock");AppendDynastyClockWrite(c,b,d,true,"cotDefWrite");c.Branch("e9","end");',1)
    start=s.index('            // The main-menu provider may be rebuilt')
    end=s.index('            c.Label("restore");',start)
    s=s[:start]+'''            AppendDynastyClockRead(c,b,d,false,"cotRuntime");
            c.Hex("83f803");c.Branch("0f85","cotRuntimeNative");c.Hex("b801000000");
            c.Label("cotRuntimeNative");
            c.Hex("89442420"); c.MovR10(d); c.Hex("41ff821008000041898214080000");
'''+s[end:]
    start=s.index('            c.Call(b + layout.Map(0x27ac630));',s.index('private static byte[] BuildEnumFor'))
    end=s.index('            c.Hex("89442420");',start)
    s=s[:start]+'''            AppendDynastyClockRead(c,b,d,false,"cotEnum");
            c.Hex("83f803");c.Branch("0f85","cotEnumNative");c.Hex("b801000000");
            c.Label("cotEnumNative");
'''+s[end:]
    return s
edit('Shared/Clock/DynastyPlayClockAction.cs',action)
def combined(s):
    start=s.index('            c.Call(',s.index('BuildCombinedSectionClockRow'))
    end=s.index('            c.Label("value");',start)
    s=s[:start]+'''            AppendDynastyClockRead(c,b,d,false,"cotMenu");
            c.MovR10(d);
'''+s[end:].replace('            c.Label("value");','',1)
    s=replace(s,'int seconds=OfflineDynastySettings.ClockPreferences.Load(System.IO.Path.GetDirectoryName(typeof(DynastyPlayClockAction).Assembly.Location));','int seconds=15;',1)
    s=s.replace('Applies across offline Dynasties.','Saved separately for this Dynasty.')
    s=replace(s,'Code c=new Code();c.Hex("4883ec28");c.MovR10(d);c.Hex("418b82800800004189825c090000");','Code c=new Code();c.Hex("4883ec28");AppendDynastyClockRead(c,b,d,true,"cotDefMenu");c.MovR10(d);c.Hex("418982800800004189825c090000");',1)
    start=s.index('                byte[] flag =',s.index('internal void Poll()'))
    end=s.index('            public void Dispose()',start)
    s=s[:start]+'''                // Persistence is owned by the detached, read-only COT helper.
            }
'''+s[end:]
    s=replace(s,'bool saved = LoadExtended();','bool saved = false;',1)
    return s
edit('Plugin/CombinedClockRuntime.cs',combined)
def extended(s):
    # Every extended-duration decision uses the CURRENT Dynasty, even immediately after a switch.
    old='c.Hex("4183ba2808000001");'
    return replace(s,old,'AppendDynastyClockRead(c,b,d,false,"cotExtended");c.Hex("83f803");',4)
edit('Shared/Clock/ExtendedClock.cs',extended)
def defensive(s):
    s=replace(s,'c.Hex("4183ba8008000000");','AppendDynastyClockRead(c,b,d,true,"cotDefEnable");c.Hex("83f800");',1)
    # The validated lookup can only return supported values; nonzero is enabled.
    start=s.index('            foreach(int seconds in new int[]{3,5,15,20,30,45,60})')
    end=s.index('            c.Label("defensiveGateOff");',start)
    s=s[:start]+'''            c.Branch("e9","defensiveGateOn");
'''+s[end:]
    s=replace(s,'c.MovR10(d);c.Hex("418b828008000085c0");','AppendDynastyClockRead(c,b,d,true,"cotDefTimer");c.MovR10(d);c.Hex("85c0");',1)
    return s
edit('Plugin/DefensivePlayClock.cs',defensive)
def helper(s):
    start=s.index('                                    // Do not silently replace an unreadable existing preference.')
    end=s.index('                                    Read(handle, context.ClockData + 0x828, 4);',start)
    s=s[:start]+s[end:]
    s=replace(s,'try { SavePreference(handle, ref savedPreference); } catch (Exception ex) { error = ex; }','try { SaveDynastyClocks(handle, ref savedClockSequence); } catch (Exception ex) { error = ex; }',1)
    s=replace(s,'                                    try { SaveDefensiveClock(handle, ref savedDefensiveClock); } catch (Exception ex) { if (error == null) error = ex; }\n','',1)
    s=replace(s,'uint savedSequence = 0;int savedDefensiveClock=-1;','uint savedSequence = 0, savedClockSequence=0;',1)
    s=replace(s,'bool ready = false, preferenceKnown = false, savedPreference = false;','bool ready = false;',1)
    at=s.index('        private static void SavePreference')
    s=s[:at]+'''        private static void SaveDynastyClocks(IntPtr handle,ref uint saved) {
            uint before=BitConverter.ToUInt32(Read(handle,context.ClockData+DynastyClockProfiles.SequenceOffset,4),0);
            if(before==saved || (before&1)!=0)return;
            byte[] table=Read(handle,context.ClockData+DynastyClockProfiles.TableOffset,DynastyClockProfiles.Capacity*8);
            uint after=BitConverter.ToUInt32(Read(handle,context.ClockData+DynastyClockProfiles.SequenceOffset,4),0);
            if(before!=after || (after&1)!=0)return;
            DynastyClockProfiles.Save(context.PluginFolder,DynastyClockProfiles.Decode(table));
            saved=after;Log("Saved per-Dynasty offensive and defensive clock choices; revision "+after+".");
        }
'''+s[at:]
    return s
edit('Plugin/PersistenceHelper.cs',helper)
def build(s):
    s=replace(s,"(Join-Path $here 'PersistenceHelper.cs')","(Join-Path $here 'PersistenceHelper.cs') (Join-Path $here 'DynastyClockProfiles.cs')",1)
    s=replace(s,"    (Join-Path $here 'CombinedClockRuntime.cs'),","    (Join-Path $here 'CombinedClockRuntime.cs'),\n    (Join-Path $here 'DynastyClockProfiles.cs'),\n    (Join-Path $here 'DynastyClockNative.cs'),",1)
    return s
edit('Plugin/build.ps1',build)
manifest=ROOT/'Baselines/changes.json'
doc=json.loads(manifest.read_text())
for p in changed:
    relative=p.relative_to(ROOT).as_posix()
    doc['changes']=[c for c in doc['changes'] if c['path']!=relative]
    doc['changes'].append(dict(path=relative,sha256=hashlib.sha256(p.read_bytes()).hexdigest(),reason='COT per-Dynasty clock table, native reads/writes and detached persistence'))
manifest.write_text(json.dumps(doc,indent=2)+'\n')
print('Updated '+str(len(changed))+' COT clock integration files.')

from pathlib import Path
import hashlib,json
root=Path(r'C:\Apps\CFB 27\MMC\DynastyCOT')
mod=root/'Modules/ExtraDynastySettings'
changed=[]
def edit(rel,fn):
 p=mod/rel;s=p.read_text(encoding='utf-8-sig');out=fn(s);assert out!=s; p.write_text(out,encoding='utf-8',newline='\n');changed.append(p)
def menu(s):
 s=s.replace('clockData=Allocate(0x1000);','clockData=Allocate(0x4000);')
 old='Marshal.WriteInt32(clockData,0x828,1);rows.Clear();labels.Clear();ui(adapter,menuWriter,5,0);'
 s=s.replace(old,'SetClock(0,101,3,5);rows.Clear();labels.Clear();ui(adapter,menuWriter,5,0);')
 s=s.replace('Check(rows[2][2]==3,"60-second clock preference displays correctly in moved row");Marshal.WriteInt32(clockData,0x828,0);','Check(rows[2][2]==3 && rows[5][2]==5,"Dynasty A displays its own offensive and defensive clocks");\n            SetClock(1,202,2,0);SetId(202);rows.Clear();labels.Clear();ui(adapter,menuWriter,5,0);\n            Check(rows[2][2]==2 && rows[5][2]==0,"Dynasty B clocks do not inherit A");\n            SetId(101);SetClock(0,101,1,15);')
 at=s.index('    static void Persistence(')
 s=s[:at]+'''    static void SetClock(int index,uint id,int offense,int defense) {
        ulong value=id|((ulong)offense<<32)|((ulong)defense<<40);
        Marshal.WriteInt64(clockData,0x1100+index*8,unchecked((long)value));
    }
'''+s[at:]
 return s
edit('Tools/GameSettingsMenuChecks.cs',menu)
def migration(s):
 s=s.replace('BitConverter.ToInt32(data,0x880)==60 && BitConverter.ToInt32(data,0x95c)==60,"Menu descriptor reloads saved new clock"','BitConverter.ToInt32(data,0x880)==15 && BitConverter.ToInt32(data,0x95c)==15,"COT descriptor ignores global defensive choice"')
 for value in [5,3,0]:
  s=s.replace(f'BitConverter.ToInt32(data,0x880)=={value} && BitConverter.ToInt32(data,0x95c)=={value}', 'BitConverter.ToInt32(data,0x880)==15 && BitConverter.ToInt32(data,0x95c)==15')
 s=s.replace('"Saved 5-second choice stays 5 Seconds"','"Legacy 5-second API remains readable without seeding Dynasty defaults"')
 s=s.replace('"3 Seconds persists and displays after reload"','"Legacy 3-second API remains readable without seeding Dynasty defaults"')
 s=s.replace('"Off persists and displays after reload"','"Legacy Off API remains readable without seeding Dynasty defaults"')
 needle='            Console.WriteLine("PASS "+checks+'
 at=s.index(needle)
 s=s[:at]+'''            string profiles=Path.Combine(folder,"OfflineDynastySettings","DynastyClocks.json");
            File.WriteAllText(profiles,"{\\"format\\":1,\\"dynasties\\":{\\"101\\":{\\"offensiveMode\\":3,\\"defensiveSeconds\\":5},\\"202\\":{\\"offensiveMode\\":0,\\"defensiveSeconds\\":0}}}");
            data=(byte[])clock.GetMethod("BuildData",f).Invoke(null,new object[]{0x200000000L});
            ulong first=BitConverter.ToUInt64(data,0x1100),second=BitConverter.ToUInt64(data,0x1108);
            Check((uint)first==101 && ((first>>32)&255)==3 && ((first>>40)&255)==5,"A saved clock pair seeds native table");
            Check((uint)second==202 && ((second>>32)&255)==0 && ((second>>40)&255)==0,"B saved clock pair seeds independently");
'''+s[at:]
 return s
edit('Tools/ClockMigrationChecks.cs',migration)
m=root/'Baselines/changes.json';doc=json.loads(m.read_text())
for path in changed:
 rel=path.relative_to(root).as_posix();doc['changes']=[x for x in doc['changes'] if x['path']!=rel]
 doc['changes'].append(dict(path=rel,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),reason='Adapt inherited fixtures to per-Dynasty clocks and enlarged native data'))
m.write_text(json.dumps(doc,indent=2)+'\n')
print('Updated clock regression fixtures')

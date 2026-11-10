"""Pin the already-built standalone artifacts; no module rebuild/install/network."""
from pathlib import Path
import hashlib,json,re,zipfile
ROOT=Path(__file__).resolve().parents[1]
DEFS=[
 ('BeyondTheField','DynastyEnhancer','Beyond the Field','BeyondTheField','1.0.7',['DynastyEnhancer','PlayerAnalytics'],'Dynasty news, recruiting feedback and scouting fog. Choose rating styles, clearer money labels and GPI player grades.'),
 ('GamedayAtmosphere','GamedayAtmosphere','Gameday Atmosphere','DynamicGamedayAtmosphere','2.1.11',['DynamicGamedayAtmosphere','DynamicAttendance'],'Fans arrive, cheer, travel and leave as games develop. Tune crowd sound, camera shake and visiting support.'),
 ('ExtraSettings','ExtraDynastySettings','Extra Settings','offline-dynasty-settings','0.2.10',['OfflineDynastySettings','DynastyPlayClock','OfflinePlaycall'],'Set separate offensive and defensive play clocks, playcall limits and cooldowns. Each Dynasty keeps its choices.'),
 ('ExpandedPractice','DrivePractice','Expanded Practice','drive-based-practice','0.8.21',['SituationalPractice'],'Run full drives or scrimmages in Dynasty or main-menu Practice. Choose your starting spot and switch sides.')]
def sha(data):return hashlib.sha256(data).hexdigest()
def main():
 modules=[];known={};programs={};mapping={old:new for new,old,*_ in DEFS}
 for name,old,title,repo,version,aliases,description in DEFS:
  archive=ROOT/f'Release/{name}-{version}.zip';proof=json.loads(archive.with_name(archive.stem+'-verification.json').read_text(encoding='utf-8'))
  assert sha(archive.read_bytes())==proof['archiveSha256']
  with zipfile.ZipFile(archive) as z:
   assert set(z.namelist())=={name+'.dll','README.txt','FEATURES.md'} and not z.testzip()
   assert sha(z.read(name+'.dll'))==proof['dllSha256']
  prefix=f'https://github.com/ArtVsTheWorld/{repo}/releases/download/v{version}/'
  notes=(ROOT/f'Packaging/Standalone/{name}/RELEASE_NOTES.md').read_text(encoding='utf-8')
  current=dict(Module=name,Version=version,Tag='v'+version,ArchiveName=archive.name,ArchiveUrl=prefix+archive.name,ArchiveSha256=proof['archiveSha256'],DllSha256=proof['dllSha256'],ArchiveBytes=archive.stat().st_size,ProofUrl=prefix+archive.stem+'-verification.json',PageUrl=f'https://github.com/ArtVsTheWorld/{repo}/releases/tag/v{version}',Notes=notes,Compatibility='Offline scope and native protections stay in each module. Use one provider of each feature. Read the module release notes for known limits.',Prerelease=False)
  modules.append(dict(Id=name,LegacyId=old,Title=title,Description=description,Repository='ArtVsTheWorld/'+repo,Aliases=aliases,Current=current))
  for path,digest in proof.get('payload',{}).items():programs[(path,digest)]=dict(Path=path,Module=name,Sha256=digest)
  if name=='GamedayAtmosphere':
   for entry in proof.get('resources',[]):
    resource=entry['name'].removeprefix('Attendance.')
    if resource.endswith('.dll') or resource=='DgaDiagnostics.exe':
     path=f'DGA-Runtime/Runtime-{version}/'+resource;programs[(path,entry['sha256'])]=dict(Path=path,Module=name,Sha256=entry['sha256'])
  for older in (ROOT/'Release').glob(name+'-*.zip'):
   if not re.fullmatch(re.escape(name)+r'-\d+\.\d+\.\d+[.]zip',older.name):continue
   with zipfile.ZipFile(older) as z:
    if name+'.dll' in z.namelist():
     digest=sha(z.read(name+'.dll'));path='Plugins/'+name+'.dll';known[(path,digest)]=dict(Path=path,Module=name,Sha256=digest)
     oldproof=older.with_name(older.stem+'-verification.json')
     if oldproof.is_file():
      verified=json.loads(oldproof.read_text(encoding='utf-8'))
      if verified.get('archiveSha256')==sha(older.read_bytes()) and verified.get('dllSha256')==digest:
       for path,programhash in verified.get('payload',{}).items():programs[(path,programhash)]=dict(Path=path,Module=name,Sha256=programhash)
 for file in ['LegacyPayloads.json','StandalonePayloads.json']:
  for entry in json.loads((ROOT/'Packaging'/file).read_text())['files']:
   path,digest,owner=entry['path'],entry['sha256'],mapping.get(entry['module'],entry['module'])
   row=dict(Path=path,Module=owner,Sha256=digest)
   if path.startswith('Plugins/') and path.endswith('.dll'):known[(path,digest)]=row
   else:programs[(path,digest)]=row
 # Include subsequent released bundle hotfixes, not just the older static
 # migration ledger. Record hashes only, never payload bytes or user data.
 for archive in (ROOT/'Release').glob('DynastyCOT-*.zip'):
  with zipfile.ZipFile(archive) as z:
   if 'package-manifest.json' not in z.namelist():continue
   for entry in json.loads(z.read('package-manifest.json'))['files']:
    path,digest=entry['path'],entry['sha256'];owner=mapping.get(entry['module'],entry['module'])
    if owner=='DynastyEnhancerNews':owner='BeyondTheField'
    row=dict(Path=path,Module=owner,Sha256=digest)
    if path=='Plugins/DynastyCOT.dll':known[(path,digest)]=row
    elif path.startswith(('COT-Runtime/','DGA-Runtime/')) or path.endswith('.Persistence.exe') or path.startswith('Plugins/SituationalPractice/') and path.endswith('.exe'):programs[(path,digest)]=row
 def program_path(path):
  cot_runtime=path.startswith('COT-Runtime/') and (path.endswith(('.dll','.exe')) or path.startswith('COT-Runtime/DynastyEnhancer/Tools/') and path.endswith('.mjs') or path.startswith('COT-Runtime/DynastyEnhancer/Content/Runtime/') and path.endswith('.json'))
  return cot_runtime or path.startswith('DGA-Runtime/Runtime-') and path.endswith(('.dll','.exe')) or path.startswith('Plugins/OfflineDynastySettings/') and path.endswith('.Persistence.exe') or path.startswith('Plugins/SituationalPractice/') and path.endswith('.Runtime.exe')
 programs={key:row for key,row in programs.items() if program_path(row['Path'])}
 doc=dict(Format=1,Version='1.1.0-rc1',Modules=modules,KnownPlugins=list(known.values()),Programs=list(programs.values()))
 output=ROOT/'Packaging/Collection/catalog.json';output.write_text(json.dumps(doc,indent=2)+'\n',encoding='utf-8')
 print('Pinned four exact standalone ZIP/DLLs;',len(known),'known plugin hashes;',len(programs),'program-only rollback references. No installs/network.')
if __name__=='__main__':main()

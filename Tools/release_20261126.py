"""Promote the approved BTF/Extra Settings candidates, never install or inject.

Explicit phases keep source pins and server asset verification ahead of release
publication. GitHub credentials stay in the existing credential-helper wrapper.
"""
from pathlib import Path
import hashlib,json,re,shutil,subprocess,sys,zipfile
from release_hub_hotfix3 import GH,run
R=Path(__file__).resolve().parents[1];W=R/'Build/Publish-20261126';O=R/'Release/November-26-Updates'
DEFS={
 'BeyondTheField':('1.0.46','1.0.43','ArtVsTheWorld/BeyondTheField','DynastyEnhancer','Update 2 Hotfix 5 (1.0.46)','BTF-1.0.46-Recruit-Lockout'),
 'ExtraSettings':('0.2.22','0.2.21','ArtVsTheWorld/offline-dynasty-settings','ExtraDynastySettings','Extra Settings 0.2.22','ExtraSettings-0.2.22-Practice-Injuries'),
}
def load(p):return json.loads(p.read_bytes())
def put(p,v):p.write_text(json.dumps(v,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def normalized(b):return b.removeprefix(b'\xef\xbb\xbf').replace(b'\r\n',b'\n')
def release(repo,tag):
 rows=json.loads(run([GH,'api',f'repos/{repo}/releases?per_page=50']))
 return next((x for x in rows if x['tag_name']==tag),None)
def sources(name):
 version,prior,repo,module,_,_=DEFS[name]
 old=load(W/'Previous'/name/f'{name}-{prior}-verification.json')
 hashes={};transforms={}
 def record(path,staged=None,expected=None):
  p=Path(path);b=(R/p).read_bytes()
  if expected:assert hashlib.sha256(b).hexdigest()==expected,p
  candidate=normalized(b)
  if p.name=='RuntimeFeed.h':
   candidate=candidate.replace(b'folder/L"dynasty-enhancer.json"',b'folder/L"beyond-the-field.json"')
   transforms[p.as_posix()]={'find':'folder/L"dynasty-enhancer.json"','replace':'folder/L"beyond-the-field.json"'}
  if staged:assert normalized(staged.read_bytes())==candidate,p
  hashes[p.as_posix()]=hashlib.sha256(normalized(b)).hexdigest()
 for path in old['sourceHashes']:record(path)
 if name=='BeyondTheField':
  base=R/'Build/AfterGameUpdate-20261119/BeyondTheFieldPort';stage=R/'Build/RecruitLockout-20261126'
  for path in load(base/'build-integrity.json')['sourceInputs']:
   p=Path(path)
   if p.parts[0]=='Native' and p.name not in ['LiveUiBodyObserver.cpp','LiveUiRepair.cpp','RecruitNotesVarietyAudit.cpp']:
    record(Path(*p.parts[1:]),stage/p)
  current=load(stage/'build-integrity.json')
  for leaf,digest in current['sourceHashes'].items():record(Path('Modules/DynastyEnhancer/Source')/leaf,expected=digest)
  payload=load(stage/'Managed/payload.json')
  for path in ['Tools/build_btf_recruit_lockout.py','Tools/build_all_ui_contracts.py','Modules/DynastyEnhancer/Tools/build_player_card_ui.py','Modules/DynastyEnhancer/Tools/build_player_ingame_ui.py','Modules/DynastyEnhancer/Tools/build_recruit_interest_ui.py','Modules/DynastyEnhancer/Tools/recruit_notes_vm.py','Modules/DynastyEnhancer/Tests/recruit_interest_ui_test.py','Modules/DynastyEnhancer/Tests/stats_card_positions_test.py','Modules/DynastyEnhancer/Tests/recruit_lockout_ui_test.py']:record(path)
  verification={'interestResponse':3242,'interestUi':409,'lockoutPresentation':2468,'statsPositions':400,'universalMethods':153,'independentPolicies':918,'cppSuites':current['verification'],'candidateVisualAcceptance':'pending','nativeStatusCheck':'Current-school native Locked=false was verified read-only despite a rank below the nominal cut; no causal attribution to other mods.'}
  generated={p.name:sha(p) for p in (stage/'Managed').glob('*.cs')}
 else:
  stage=R/'Build/PracticeInjuries-20261125';current=load(stage/'Candidate/candidate.json')
  for path,digest in current['sourceHashes'].items():
   public='Tools/build_practice_injuries.py' if path=='Build/PracticeInjuries-20261125/build_private.py' else path
   record(public,expected=digest)
  payload=current['payload'];generated=current['generatedHashes']
  for path in ['Modules/ExtraDynastySettings/Plugin/build.ps1','Modules/ExtraDynastySettings/Tools/build_game_settings_menu.py','Modules/ExtraDynastySettings/Tools/PracticeInjuryChecks.cs','Modules/ExtraDynastySettings/Tools/GameSettingsMenuChecks.cs','Modules/ExtraDynastySettings/Tools/IntegrationChecks.cs','Modules/ExtraDynastySettings/Tools/MmcHostChecks.cs','Modules/ExtraDynastySettings/Tools/test_practice_injuries.py','Modules/ExtraDynastySettings/docs/practice-injuries.txt','Modules/ExtraDynastySettings/docs/native-layout.json','Modules/ExtraDynastySettings/docs/updating.md']:record(path)
  assert all(x['passed'] for x in load(stage/'test-results.json'))
  assert load(stage/'Tests/native-results.json')['checks']==427
  verification={'preferencesMigrationRollback':49,'nativeMenuWriter':90,'nativeDecisionAndPreview':427,'positions':21,'formationSubs':139,'formationGroups':216,'formationPreview':411,'formationNames':262,'hostAndHookOwnership':'passed','candidateMenuAndWeeklyAdvanceAcceptance':'pending'}
 for path in ['Packaging/Standalone/'+name+'/README.txt','Packaging/Standalone/'+name+'/FEATURES.md','Packaging/Standalone/'+name+'/RELEASE_NOTES.md','Tools/release_20261126.py']:record(path)
 return hashes,transforms,payload,generated,verification,old
def prepare():
 O.mkdir(exist_ok=True)
 (R/'Tools/build_btf_recruit_lockout.py').write_bytes((R/'Build/RecruitLockout-20261126/build_private.py').read_bytes().rstrip()+b'\n')
 shutil.copyfile(R/'Build/PracticeInjuries-20261125/build_private.py',R/'Tools/build_practice_injuries.py')
 catalog=load(R/'Packaging/Collection/catalog.json');paths=set()
 for name,(version,prior,repo,module,title,private) in DEFS.items():
  assert load(W/(name+'-previous-release.json'))['tag_name']=='v'+prior
  candidate=R/'Release/Private'/private;proof=load(candidate/'integrity.json');dll=candidate/'Plugins'/(name+'.dll')
  expected=proof.get('dllSha256',proof.get('files',{}).get('Plugins/'+name+'.dll'))
  assert sha(dll)==expected
  hashes,transforms,payload,generated,verification,old=sources(name);paths.update(hashes)
  doc=R/'Packaging/Standalone'/name;guide=R/'docs/player-guides'/(name+'-Player-Guide.pdf')
  folder=O/name;folder.mkdir(exist_ok=True);stem=name+'-'+version
  files={dll.name:dll,'README.txt':doc/'README.txt','FEATURES.md':doc/'FEATURES.md',guide.name:guide}
  license_file=doc/'LICENSES.txt'
  if license_file.exists():assert license_file.read_bytes() in (doc/'README.txt').read_bytes()
  archive=folder/(stem+'.zip')
  with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
   for leaf,p in files.items():
    info=zipfile.ZipInfo(leaf,(2020,1,1,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED;info.external_attr=0o644<<16;z.writestr(info,p.read_bytes(),compresslevel=9)
  with zipfile.ZipFile(archive) as z:assert z.testzip() is None and z.namelist()==list(files) and hashlib.sha256(z.read(dll.name)).hexdigest()==expected
  for suffix in ['pdf','md']:shutil.copyfile(R/'docs/player-guides'/(name+'-Player-Guide.'+suffix),folder/(name+'-Player-Guide.'+suffix))
  claims=(
   [{'claim':'Full-card Feedback labels','files':['Modules/DynastyEnhancer/Tools/build_recruit_interest_ui.py']},
    {'claim':'Live scholarship offer totals; Blind concealment','files':['Modules/DynastyEnhancer/Source/LiveTables.h','Modules/DynastyEnhancer/Source/RecruitingFog.cpp']},
    {'claim':'Stats Attributes across positions','files':['Modules/DynastyEnhancer/Tools/build_player_card_ui.py','Modules/DynastyEnhancer/Tools/build_player_ingame_ui.py']},
    {'claim':'Native lockout divider and original cutoff membership','files':['Modules/DynastyEnhancer/Source/RecruitInterestResponse.h','Modules/DynastyEnhancer/Tools/build_recruit_interest_ui.py']}]
   if name=='BeyondTheField' else
   [{'claim':'Practice Injuries choices, weekly decision and preview','files':['Modules/ExtraDynastySettings/Plugin/PracticeInjuries.cs','Modules/ExtraDynastySettings/Plugin/PracticeInjuryBindings.cs']},
    {'claim':'Per-Dynasty persistence; absent choice is Default','files':['Modules/ExtraDynastySettings/Plugin/DynastyClockProfiles.cs','Modules/ExtraDynastySettings/Plugin/DynastyClockNative.cs']}])
  public=dict(format=2,name=name,version=version,gameUpdate='November 17 Update',sourceRepository='ArtVsTheWorld/DynastyCOT',sourceRevision=None,publicBaseline=prior,publicBaselineSourceRevision=old['sourceRevision'],publicBaselineDllSha256=old['dllSha256'],candidateDllUnchanged=True,fourFileZip=True,archiveSha256=sha(archive),dllSha256=expected,playerGuideSha256=sha(guide),supportFiles={},files=[dict(path=n,bytes=p.stat().st_size,sha256=sha(p)) for n,p in files.items()],verification=verification,sourceHashes=hashes,sourceHashNormalization='UTF-8 BOM removed; CRLF converted to LF',standaloneSourceTransforms=transforms,generatedInputSha256=generated,payload=payload,releaseNoteEvidence=claims)
  put(folder/(stem+'-verification.json'),public)
  item=next(m for m in catalog['Modules'] if m['Id']==name);base=f'https://github.com/{repo}/releases/download/v{version}/'
  item['Current'].update(Version=version,Tag='v'+version,ArchiveName=archive.name,ArchiveUrl=base+archive.name,ArchiveSha256=sha(archive),ArchiveBytes=archive.stat().st_size,DllSha256=expected,ProofUrl=base+stem+'-verification.json',PageUrl=f'https://github.com/{repo}/releases/tag/v{version}',Notes=(doc/'RELEASE_NOTES.md').read_text(),Prerelease=False)
  catalog['KnownPlugins'].append(dict(Path='Plugins/'+name+'.dll',Module=name,Sha256=expected))
  for path,digest in payload.items():
   if path.endswith(('.exe','.dll')):catalog['Programs'].append(dict(Path=path,Module=name,Sha256=digest))
  print('Prepared',name,version,'against public',prior,'sources',len(hashes),'DLL',expected,flush=True)
 for key in ['KnownPlugins','Programs']:catalog[key]=list({(x['Path'],x['Module'],x['Sha256']):x for x in catalog[key]}.values())
 practice=next(m for m in catalog['Modules'] if m['Id']=='ExpandedPractice');practice['Current']['Notes']=(R/'Packaging/Standalone/ExpandedPractice/RELEASE_NOTES.md').read_text()
 put(R/'Packaging/Collection/catalog.json',catalog)
 (W/'source-paths.txt').write_text('\n'.join(sorted(paths))+'\n')
def finalize():
 revision=run(['git','rev-parse','HEAD']).strip()
 for name,(version,*_) in DEFS.items():
  folder=O/name;stem=name+'-'+version;p=folder/(stem+'-verification.json');proof=load(p)
  for path,digest in proof['sourceHashes'].items():
   committed=subprocess.check_output(['git','show',revision+':'+path],cwd=R)
   assert hashlib.sha256(normalized(committed)).hexdigest()==digest,path
  proof['sourceRevision']=revision;put(p,proof)
  (folder/(stem+'-SHA256.txt')).write_text(''.join(sha(x)+'  '+x.name+'\n' for x in [folder/(stem+'.zip'),p,folder/(name+'-Player-Guide.pdf')]))
  print('Pinned',name,revision,flush=True)
def draft():
 for name,(version,prior,repo,module,title,_) in DEFS.items():
  folder=O/name;stem=name+'-'+version;proof=load(folder/(stem+'-verification.json'));rev=proof['sourceRevision']
  assert run(['git','ls-remote','origin','refs/heads/main']).split()[0]==rev
  assert json.loads(run([GH,'api',f'repos/{repo}/releases/latest']))['tag_name']=='v'+prior
  assert release(repo,'v'+version) is None,'Release already exists'
  clone=W/'Distribution'/name;clone.parent.mkdir(exist_ok=True)
  if not clone.exists():run(['git','clone','--quiet','--depth','1','https://github.com/'+repo+'.git',clone])
  assert not run(['git','status','--porcelain'],clone)
  pin=load(clone/'source-release.json');assert pin['version']==prior
  readme=(clone/'README.md').read_text().replace('/v'+prior+'/', '/v'+version+'/').replace(stem.replace(version,prior)+'.zip',stem+'.zip').replace('Download '+prior+' ZIP','Download '+version+' ZIP').replace(pin['revision'],rev)
  (clone/'README.md').write_text(readme,encoding='utf-8');doc=R/'Packaging/Standalone'/name
  shutil.copyfile(doc/'FEATURES.md',clone/'FEATURES.md')
  previous=(clone/'RELEASES.md').read_text()
  if name=='ExtraSettings':previous=strip_update(previous,first=True)
  (clone/'RELEASES.md').write_text((doc/'RELEASE_NOTES.md').read_text()+'\n---\n\n'+previous,encoding='utf-8')
  pin.update(version=version,revision=rev,archive=stem+'.zip',archiveSha256=proof['archiveSha256'],dllSha256=proof['dllSha256'],playerGuideSha256=proof['playerGuideSha256'],publicBaseline=prior);put(clone/'source-release.json',pin)
  for key in ['user.name','user.email']:run(['git','config',key,run(['git','config',key]).strip()],clone)
  run(['git','add','README.md','FEATURES.md','RELEASES.md','source-release.json'],clone);run(['git','commit','-m',title+': source-backed release'],clone)
  target=run(['git','rev-parse','HEAD'],clone).strip();run(['git','push','origin','HEAD:refs/heads/release-source-20261126-'+version],clone)
  run([GH,'release','create','v'+version,'--repo',repo,'--draft','--target',target,'--title',title,'--notes-file',doc/'RELEASE_NOTES.md'])
  assets=sorted(folder.iterdir());assert len(assets)==5
  run([GH,'release','upload','v'+version,'--repo',repo,*assets]);verify_assets(name,True)
def verify_assets(name,draft_state):
 version,_,repo,*_=DEFS[name];folder=O/name;actual=release(repo,'v'+version)
 assert actual and actual['draft']==draft_state and not actual['prerelease']
 assets={a['name']:a for a in actual['assets']};assert set(assets)=={p.name for p in folder.iterdir()}
 for p in folder.iterdir():assert assets[p.name]['size']==p.stat().st_size and assets[p.name]['digest']=='sha256:'+sha(p),p.name
 return actual
def publish():
 for name,(version,_,repo,*_) in DEFS.items():
  verify_assets(name,True);run([GH,'release','edit','v'+version,'--repo',repo,'--draft=false','--latest'])
  actual=verify_assets(name,False);latest=json.loads(run([GH,'api',f'repos/{repo}/releases/latest']));assert latest['id']==actual['id']
  clone=W/'Distribution'/name;run(['git','push','origin','HEAD:main'],clone)
  put(W/(name+'-published.json'),{'url':actual['html_url'],'version':version,'assets':[{k:a[k] for k in ['name','size','digest']} for a in actual['assets']]})
  print('Published',actual['html_url'],flush=True)
def strip_update(text,first=False):
 text=text.replace('\r','');text=re.sub(r'^#{1,6}\s*\*{0,2}Update\s+\d+[^\n]*\n','',text,count=1 if first else 0,flags=re.M)
 return re.sub(r'\n{3,}','\n\n',text)
def headings():
 for name,repo,version,title in [('ExtraSettings','ArtVsTheWorld/offline-dynasty-settings','0.2.21','Extra Settings 0.2.21'),('ExpandedPractice','ArtVsTheWorld/drive-based-practice','0.8.26','Expanded Practice 0.8.26')]:
  old=load(W/(name+'-previous-release.json'));before=json.loads(run([GH,'api',f'repos/{repo}/releases/tags/v{version}']))
  assert before['id']==old['id']
  notes=W/(name+'-previous-clean-notes.md');notes.write_text(strip_update(old['body']),encoding='utf-8')
  run([GH,'release','edit','v'+version,'--repo',repo,'--title',title,'--notes-file',notes])
  after=json.loads(run([GH,'api',f'repos/{repo}/releases/tags/v{version}']));assert after['body']==notes.read_text()
  assert [(x['id'],x['digest']) for x in before['assets']]==[(x['id'],x['digest']) for x in after['assets']]
  if name=='ExpandedPractice':
   clone=W/'Distribution'/name
   if not clone.exists():run(['git','clone','--quiet','--depth','1','https://github.com/'+repo+'.git',clone])
   assert not run(['git','status','--porcelain'],clone)
   p=clone/'RELEASES.md';p.write_text(strip_update(p.read_text(),first=True),encoding='utf-8')
   for key in ['user.name','user.email']:run(['git','config',key,run(['git','config',key]).strip()],clone)
   run(['git','add','RELEASES.md'],clone);run(['git','commit','-m','Remove numbered Update heading from latest Expanded Practice notes'],clone);run(['git','push','origin','HEAD:main'],clone)
  print('Cleaned heading without asset changes:',repo,'v'+version,flush=True)
if __name__=='__main__':
 action=sys.argv[1];assert action in ['prepare','finalize','draft','publish','headings'];globals()[action]()

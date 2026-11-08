"""Record only public payload paths/hashes from already shipped COT ZIPs."""
from pathlib import Path
import json,zipfile,re
ROOT=Path(__file__).resolve().parents[1]

def entries(doc):
    for f in doc['files']:
        path=f['path'];module=f['module']
        if path.startswith(('COT-Runtime/','LICENSES/')) or path in (
            'Plugins/DynastyCOT.dll','Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe',
            'Plugins/SituationalPractice/SituationalPractice.Runtime.exe','Plugins/SituationalPractice/SituationalPractice.Runtime.pdb'):
            if path.startswith('LICENSES/'):module='Core'
            if module=='DynastyEnhancerNews':module='DynastyEnhancer'
            yield dict(path=path,module=module,sha256=f['sha256'])
    for f in doc['embedded']:
        name=f['path'].removeprefix('resource:Attendance.')
        match=re.fullmatch(r'DynamicGamedayAtmosphereNative-(\d+\.\d+\.\d+)\.dll',name)
        if match:
            version=match[1]
            yield dict(path=f'DGA-Runtime/Runtime-{version}/{name}',module='GamedayAtmosphere',sha256=f['sha256'])
            diagnostics=next(x for x in doc['embedded'] if x['path']=='resource:Attendance.DgaDiagnostics.exe')
            yield dict(path=f'DGA-Runtime/Runtime-{version}/DgaDiagnostics.exe',module='GamedayAtmosphere',sha256=diagnostics['sha256'])

if __name__=='__main__':
    files={}
    for path in sorted((ROOT/'Release').glob('DynastyCOT-0.1.[01]-private.zip')):
        with zipfile.ZipFile(path) as z:
            for f in entries(json.loads(z.read('package-manifest.json'))):files[(f['path'],f['sha256'])]=f
    (ROOT/'Packaging/LegacyPayloads.json').write_text(json.dumps(dict(format=1,files=list(files.values())),indent=2)+'\n')
    print('Recorded',len(files),'legacy public payload hashes; no game/user data.')

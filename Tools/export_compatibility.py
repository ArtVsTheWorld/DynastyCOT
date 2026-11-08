"""Inventory source contracts for update/conflict review; never open the game."""
from pathlib import Path
import re,json,hashlib,importlib.util,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def export():
    modules={};inputs={}
    for name,folders in {
        'DynastyEnhancer':['Source'],'GamedayAtmosphere':['Source'],
        'ExtraDynastySettings':['Plugin','Shared'],'DrivePractice':['Plugin']}.items():
        references={};contracts=[];regions=[]
        for folder in folders:
            for p in sorted((ROOT/'Modules'/name/folder).rglob('*')):
                if p.suffix not in ['.cs','.cpp','.h']:continue
                rel=p.relative_to(ROOT).as_posix();inputs[rel]=sha(p)
                for line,text in enumerate(p.read_text(encoding='utf-8-sig').splitlines(),1):
                    for value in re.findall(r'\b0x([a-fA-F0-9]+)',text):
                        n=int(value,16)
                        if 0x100000<=n<0x13000000:references.setdefault(hex(n),[]).append(f'{rel}:{line}')
                    if any(token in text for token in ['makePatch(','replace(','Replace(','new Region(','MemoryCheck(','Normalize','normaliz','Prefix','Original=','Original =']):
                        contracts.append(dict(source=rel,line=line,expression=text.strip()))
        if name in ['ExtraDynastySettings','DrivePractice']:
            source=ROOT/'Modules'/name/'Tools/export_layout.py'
            spec=importlib.util.spec_from_file_location(name,source);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
            regions=module.layout()['regions']
        if name=='DynastyEnhancer':
            for file in ['CurrentBindings.h','RecruitingUiGates.h','AnalyticsGates.h']:
                p=ROOT/'Modules'/name/'Source'/file
                for rva,body in re.findall(r'\{(0x[0-9a-fA-F]+),\s*\{([^}]+)\}\}',p.read_text()):
                    data=bytes(int(v,16) for v in re.findall(r'0x([0-9a-fA-F]+)',body))
                    regions.append(dict(source=p.relative_to(ROOT).as_posix(),rva=rva,length=len(data),sha256=hashlib.sha256(data).hexdigest()))
        modules[name]=dict(validationRanges=regions,references=dict(sorted(references.items(),key=lambda x:int(x[0],16))),contractSourceLines=contracts)
    practice=ROOT/'Modules/DrivePractice/Plugin/NativeRuntime.cs';s=practice.read_text()
    sites=[int(v,16) for v in re.search(r'long\[\] Sites=\{([^}]+)',s).group(1).split(',')]
    originals=re.findall(r'Hex\("([^"]+)"\)',re.search(r'byte\[\]\[\] Originals=\{([^\n]+)',s).group(1))
    spans=[dict(module='DrivePractice',rva=hex(site),length=len(bytes.fromhex(original)),kind='text detour') for site,original in zip(sites,originals)]
    source=ROOT/'Modules/DynastyEnhancer/Source/NativeSocialMedia.cpp'
    for vt,slot,native in re.findall(r'\{(0x[0-9a-f]+),(0x[0-9a-f]+),(0x[0-9a-f]+),reinterpret_cast<void\*>',source.read_text()):
        spans.append(dict(module='DynastyEnhancer',rva=hex(int(vt,16)+int(slot,16)),length=8,kind='Social vtable slot',native=native))
    analytics_source=ROOT/'Modules/DynastyEnhancer/Source/AnalyticsNative.h'
    for vt,slot,native in re.findall(r'\{(0x[0-9a-f]+),(0x[0-9a-f]+),(0x[0-9a-f]+),reinterpret_cast<void\*>',analytics_source.read_text()):
        spans.append(dict(module='DynastyEnhancer',rva=hex(int(vt,16)+int(slot,16)),length=8,kind='Analytics dedicated vtable slot',native=native))
    return dict(format=1,gameLayout='October 29, 2026 candidate',wholeExeHashGate=False,
        scope='Static source inventory: validation ranges, exact extracted spans, all address references and source contracts. Dynamic UI asset offsets and symbolic patches remain visible in source lines; this is not live compatibility certification.',
        modules=modules,extractedPatchSpans=spans,sourceHashes=inputs,
        sharedContracts=[
            dict(id='analytics-spreadsheet-dispatch',owners=['DynastyEnhancer'],
                 policy='Existing Fog owner dispatches Analytics columns/rows/close. Existing Social owner dispatches navigation through a versioned exact-sibling in-process API. No duplicate ownership or arbitrary hook chaining.',
                 evidence='Modules/DynastyEnhancer/Tests/AnalyticsIntegrationTests.cpp'),
            dict(id='practice-text-setter',rva='0x25a3e60',patchLength=15,validationLength=32,
                 owners=['DrivePractice','DynastyEnhancer'],policy='Exact known Practice entry/code/data contract; normalize local fingerprint buffer only; reject unknown detours',
                 source='Compatibility/PracticeTextSetter.generated.h',evidence='Tests/PracticeContractChecks.cpp'),
            dict(id='settings-writer-chain',rva='0x79a5cc3',owners=['ExtraDynastySettings','DynastyEnhancer'],
                 policy='Extra Settings owns inner text hook; Enhancer owns list/writer vtable slots 0xbab9a08+0x38/+0x50 and forwards foreign keys',
                 evidence='Modules/ExtraDynastySettings/Tools/GameSettingsMenuChecks.cs'),
            dict(id='controller-phase-bridge',rva='0x5d12420',owners=['ExtraDynastySettings','DrivePractice'],
                 policy='Practice accepts only exact known Extra Settings controller bridge; retain native practice exclusions'),
            dict(id='pause-text',owners=['GamedayAtmosphere','DrivePractice'],
                 policy='Atmosphere hooks named pause-tile output at 0x7a1bf6b; Practice owns shared setter 0x25a3e60; do not broaden either binding')
        ])
def main():
    destination=ROOT/'Compatibility/inventory.json';doc=export()
    if '--check' in sys.argv:
        if json.loads(destination.read_text())!=doc:raise SystemExit('Compatibility source inventory is stale')
    else:destination.write_text(json.dumps(doc,indent=2)+'\n')
    print('PASS: four-module compatibility source inventory; '+str(len(doc['sourceHashes']))+' source fingerprints. No live game access.')
if __name__=='__main__':main()

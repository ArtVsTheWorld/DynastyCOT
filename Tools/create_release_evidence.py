"""Export allowlisted public verification evidence; never copy raw fixture reports."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[1]

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def select(document,fields):
    result={}
    for field in fields:
        value=document.get(field)
        if isinstance(value,(bool,int,float)) or value is None:result[field]=value
        elif isinstance(value,str) and re.fullmatch(r'[A-Za-z0-9._-]{1,128}',value):result[field]=value
    return result

def main():
    product=json.loads((ROOT/'cot.product.json').read_text());version=product['version'];release=ROOT/'Release'
    package=json.loads((ROOT/'Build/current-package.json').read_text());verified=json.loads(Path(package['verification']).read_text())
    archive=Path(package['archive']);setup=json.loads((ROOT/'Build/current-setup.json').read_text());exe=Path(setup['exe'])
    if verified['version']!=version or setup['version']!=version or sha(archive)!=verified['archiveSha256'] or sha(exe)!=setup['sha256']:raise ValueError('Evidence must match exact current verified candidate assets')
    count_fields=['files','originalSourcesUnchanged','oneMmcEntry','moduleSelections','coreChecks','assemblyChecks','clockProfileChecks','emittedClockChecks','detachedHelperChecks','menuWriterChecks','practiceOwnershipChecks','practiceNativeHookChecks','installerChecks','liveGameAcceptance','overhaulCompatibility','installedIntoLiveMmc']
    evidence=dict(format=1,product='DynastyCOT',version=version,stage=product.get('stage','public-test-prerelease'),sourceRevision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),sourceTreeClean=not bool(subprocess.check_output(['git','status','--porcelain','--untracked-files=normal'],cwd=ROOT,text=True).strip()),liveGameAcceptance='pending',overhaulCompatibility='pending',package=select(verified,count_fields),compatibilityNotes=product['distribution']['compatibilityNotes'])
    for name,fields in [('setup',['version','uiChecks','payloadVerified','fixtureRoundTrip','unsafeFixtureRejected','quoteAndSpacePaths','selectedAndFullUninstall','managerFolderDetection','footerAndUpdateActionsVisible','allModulesReachable','liveInstall']),('updater',['version','oldVersion','checks','liveInstall','discoveryDownloadInstallRollback','baselineOnly','crossVersion','requests'])]:
        path=ROOT/f'Build/{name}-verification.json'
        if not path.exists():raise ValueError(f'Missing {name} verification')
        doc=json.loads(path.read_text())
        if doc.get('version')!=version or doc.get('liveInstall') or doc.get('baselineOnly') or name=='updater' and (doc.get('candidateOnly') or not doc.get('crossVersion')):raise ValueError(f'{name} checks do not prove the current candidate in isolation')
        evidence[name]=select(doc,fields)
        if name=='setup':
            evidence[name]['layoutScaleCaptures']=[x for x in doc.get('scaling',[]) if type(x) is int and 50<=x<=400]
            evidence[name]['boundedWorkingAreaHeights']=[x for x in doc.get('boundedWorkingAreas',[]) if type(x) is int and 200<=x<=10000]
        if name=='updater':evidence[name]['failureCaseCount']=len(doc.get('failureCases',[]))
    if product.get('runtimeBranding'):
        path=ROOT/'Build/runtime-upgrade-verification.json'
        if not path.exists():raise ValueError('Missing branded-runtime upgrade verification')
        doc=json.loads(path.read_text())
        if doc.get('toVersion')!=version or doc.get('synthetic') is not False or doc.get('liveInstall') is not False or doc.get('rollback') is not True:
            raise ValueError('Branded-runtime upgrade checks must cover this actual source-built candidate')
        evidence['runtimeUpgrade']=select(doc,['checks','fromVersion','toVersion','synthetic','liveInstall','rollback','faultStages','hashProvenRetirement','unknownActiveRejectedBeforeMutation','dataPathsPreserved','publicGda2035','legacyLogMigration','pluginsRootOnlySharedDll'])
    same_version=ROOT/'Build/same-version-upgrade-verification.json'
    if same_version.exists():
        doc=json.loads(same_version.read_text())
        if doc.get('current',{}).get('version')==version and doc.get('current',{}).get('manifestSha256')==verified.get('manifestSha256'):
            if doc.get('synthetic') is not False or doc.get('liveInstall') is not False or doc.get('sameVersion') is not True:
                raise ValueError('Same-version upgrade evidence must use the actual isolated candidate')
            evidence['sameVersionUpgrade']=select(doc,['checks','sameVersion','faultRollbackStages','manualRollbackRestoresPreviousBinaryHashes','choicesSettingsHistoryPreserved','laterUserDataPreserved','liveInstall','synthetic'])
            for name in ['previous','current']:
                evidence['sameVersionUpgrade'][name]=select(doc[name],['version','manifestSha256','payloadSetSha256','hostSha256','nativeFogSha256','archiveSha256'])
    published=ROOT/'Build/updater-published-verification.json'
    if published.exists():
        doc=json.loads(published.read_text())
        if doc.get('version')==version and doc.get('archiveSha256')==sha(archive) and doc.get('anonymousGitHub') is True and not doc.get('liveInstall'):
            evidence['publishedUpdater']=select(doc,['checks','version','liveInstall','anonymousGitHub','explicitVersion','archiveSha256','discoveryDownloadInstallRollback'])
    evidence['assets']=[dict(name=path.name,bytes=path.stat().st_size,sha256=sha(path)) for path in [archive,exe,release/'cot-update.json']]
    destination=release/f'release-evidence-{version}.json';destination.write_text(json.dumps(evidence,indent=2)+'\n')
    assets=[archive,exe,release/'cot-update.json',destination]
    notes=ROOT/f'docs/release/{version}.md';notes_copy=release/f'release-notes-{version}.md';notes_copy.write_bytes(notes.read_bytes());assets.append(notes_copy)
    (release/f'SHA256SUMS-{version}.txt').write_text(''.join(sha(path)+'  '+path.name+'\n' for path in assets),encoding='utf-8')
    print('Prepared sanitized public release evidence and checksums:',destination.name)
if __name__=='__main__':main()

"""Exercise distinct unpublished 1.0 payloads with identical version strings.

Supply the superseded ZIP explicitly. All installation writes stay in new,
marked Build fixtures; no game, MMC process or native module is started.
"""
from pathlib import Path, PurePosixPath
import argparse
import datetime
import hashlib
import json
import os
import stat
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]
IDS = ('DynastyEnhancer', 'GamedayAtmosphere', 'ExtraDynastySettings', 'DrivePractice')
DATA = {
    'Plugins/DynastyCOT/settings.json':
        b'{"format":1,"modules":{"DynastyEnhancer":true,"GamedayAtmosphere":false,"ExtraDynastySettings":false,"DrivePractice":true}}',
    'COT-Runtime/DynastyEnhancer/DynastyEnhancementSettings/1001.json':
        b'{"fixture":"preserve opaque per-Dynasty fog/rating/money settings"}',
    'COT-Runtime/DynastyEnhancer/RuntimeHistory/1001/published.json':
        b'{"edition":"published","posts":[{"id":"unchanged-post","account":"same-account","body":"Stored history stays unchanged."}]}',
    'COT-Runtime/DynastyEnhancer/RecruitResumeHistory/1001.json':
        b'{"fixture":"persistent recruit history"}',
    'Plugins/OfflinePlaycallSettings/dynasties.json':
        b'{"fixture":"per-Dynasty play clock preferences"}',
    'Plugins/DynamicGamedayAtmosphere/DynamicGamedayAtmosphere.ini':
        b'; existing user preferences\n[DynamicGamedayAttendance]\nVisitorCrowdInfluencePercent=85\n',
    'Plugins/DynamicGamedayAtmosphere/History/crowd.json':
        b'{"fixture":"previously recorded crowd history"}',
    'Plugins/SituationalPractice/session.json':
        b'{"fixture":"practice preferences"}',
    'Plugins/OtherOverhaul.dll': b'unrelated mod, never loaded',
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def sha(path):
    return digest(path.read_bytes())


def resolve(root, relative):
    relative = relative.replace('\\', '/')
    parts = PurePosixPath(relative)
    if not relative or parts.is_absolute() or ':' in relative or '..' in parts.parts:
        raise ValueError('Unsafe fixture/package path')
    result = (root / relative).resolve()
    if not result.is_relative_to(root.resolve()):
        raise ValueError('Path leaves fixture')
    return result


def write(root, relative, data):
    target = resolve(root, relative)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(data)


def snapshot(root):
    return {p.relative_to(root).as_posix(): sha(p)
            for p in root.rglob('*') if p.is_file()
            and not p.relative_to(root).as_posix().startswith('Plugins/DynastyCOT/Backups/')}


def validate_stage(stage):
    manifest = stage / 'package-manifest.json'
    document = json.loads(manifest.read_text(encoding='utf-8'))
    if document.get('format') != 1 or document.get('product') != 'DynastyCOT':
        raise ValueError('Expected a real COT package manifest')
    if document.get('version') != '1.0.0':
        raise ValueError('This regression requires two 1.0.0 payloads')
    files = {}
    for entry in document['files']:
        relative = entry['path']
        key = relative.replace('\\', '/').casefold()
        if key in files or sha(resolve(stage, relative)) != entry['sha256']:
            raise ValueError('Duplicated, missing or modified package file: ' + relative)
        files[key] = entry
    actual = {p.relative_to(stage).as_posix().casefold()
              for p in stage.rglob('*') if p.is_file()
              and p.relative_to(stage).as_posix() != 'package-manifest.json'}
    if actual != set(files):
        raise ValueError('Package contains unlisted files')
    payload = [{'path': entry['path'], 'sha256': entry['sha256']}
               for _, entry in sorted(files.items())]
    return document, {
        'version': document['version'],
        'manifestSha256': sha(manifest),
        'hostSha256': sha(resolve(stage, 'Plugins/DynastyCOT.dll')),
        'nativeFogSha256': document.get('nativeInputs', {}).get('fog', {}).get('sha256'),
        'payloadSetSha256': digest(json.dumps(payload, sort_keys=True,
                                            separators=(',', ':')).encode()),
        'fileCount': len(files),
    }


def extract_previous(archive, target):
    seen = set()
    with zipfile.ZipFile(archive) as package:
        for entry in package.infolist():
            destination = resolve(target, entry.filename)
            key = str(destination).casefold()
            mode = entry.external_attr >> 16
            if key in seen or stat.S_ISLNK(mode):
                raise ValueError('Duplicated or linked previous ZIP entry')
            seen.add(key)
            if entry.is_dir():
                destination.mkdir(parents=True, exist_ok=True)
            else:
                destination.parent.mkdir(parents=True, exist_ok=True)
                with package.open(entry) as source, destination.open('xb') as output:
                    while chunk := source.read(65536):
                        output.write(chunk)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', type=Path, required=True,
                        help='Explicit archived superseded 1.0 ZIP; never a live folder')
    args = parser.parse_args()
    previous_archive = args.previous.resolve(strict=True)
    metadata = json.loads((ROOT / 'Build/current-package.json').read_text())
    current = Path(metadata['stage']).resolve(strict=True)
    current_doc, current_identity = validate_stage(current)

    base = ROOT / 'Build' / ('SameVersionUpgrade-' +
        datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'))
    base.mkdir()
    previous = base / 'Previous100'
    previous.mkdir()
    extract_previous(previous_archive, previous)
    previous_doc, previous_identity = validate_stage(previous)
    previous_identity['archiveSha256'] = sha(previous_archive)
    previous_identity['archiveBytes'] = previous_archive.stat().st_size

    old_binaries = {x['path']: x['sha256'] for x in previous_doc['files']
                    if x['path'].endswith(('.dll', '.exe'))}
    new_binaries = {x['path']: x['sha256'] for x in current_doc['files']
                    if x['path'].endswith(('.dll', '.exe'))}
    changed = [{'path': path, 'previousSha256': value,
                'currentSha256': new_binaries[path]}
               for path, value in old_binaries.items()
               if path in new_binaries and value != new_binaries[path]]
    if (previous_identity['payloadSetSha256'] == current_identity['payloadSetSha256']
            or not any(x['path'] != 'Plugins/DynastyCOT.dll' for x in changed)):
        raise ValueError('Previous and current payload binaries are not distinct; rebuild first')
    current_archive = Path(metadata.get('archive', ''))
    if current_archive.is_file():
        with zipfile.ZipFile(current_archive) as package:
            same_manifest = (package.read('package-manifest.json') ==
                             (current / 'package-manifest.json').read_bytes())
        current_identity['archiveMatchesStage'] = same_manifest
        if same_manifest:
            current_identity['archiveSha256'] = sha(current_archive)
            current_identity['archiveBytes'] = current_archive.stat().st_size
    checks = 0

    def check(condition, message):
        nonlocal checks
        if not condition:
            raise AssertionError(message)
        checks += 1

    def transactions(root):
        folder = root / 'Plugins/DynastyCOT/Backups'
        return set(folder.iterdir()) if folder.exists() else set()

    def run(root, package, *, success=True, extra=()):
        check(root.resolve().is_relative_to(base.resolve()) and
              (root / '.cot-fixture').is_file(), 'Fixture guard missing')
        environment = os.environ.copy()
        shell = os.environ.get('COT_TEST_POWERSHELL', 'pwsh')
        if Path(shell).name.casefold() == 'powershell.exe':
            environment = {k: v for k, v in environment.items()
                           if k.casefold() != 'psmodulepath'}
        result = subprocess.run([
            shell, '-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'Bypass',
            '-File', str(package / 'Install.ps1'), '-MmcRoot', str(root),
            '-PackageRoot', str(package), '-Fixture', '-FixtureRoot', str(base),
            *extra], capture_output=True, text=True, timeout=120, env=environment)
        output = result.stdout + result.stderr
        (base / f'{root.name}-{checks}.log').write_text(output, encoding='utf-8')
        check((result.returncode == 0) == success, output)
        return output

    def seed(name):
        root = base / name
        root.mkdir()
        write(root, '.cot-fixture', b'isolated same-version upgrade')
        write(root, 'MMCModManager.exe', b'fixture marker; never executed')
        run(root, previous)
        for path, data in DATA.items():
            write(root, path, data)
        for path, value in old_binaries.items():
            if (root / path).exists():
                check(sha(root / path) == value, 'Old binary fixture differs: ' + path)
        return root

    def data_intact(root, expected=DATA):
        for path, data in expected.items():
            check(resolve(root, path).read_bytes() == data, 'User data changed: ' + path)

    check(previous_identity['version'] == current_identity['version'] == '1.0.0',
          'Version strings differ')
    check(previous_identity['manifestSha256'] != current_identity['manifestSha256'],
          'Manifest identities must differ')
    root = seed('Upgrade100')
    before = snapshot(root)
    old_transactions = transactions(root)
    run(root, current)
    added = transactions(root) - old_transactions
    check(len(added) == 1, 'Update did not record exactly one transaction')
    upgrade_backup = added.pop()
    data_intact(root)
    for path, value in new_binaries.items():
        if (root / path).exists():
            check(sha(root / path) == value, 'Current binary not installed: ' + path)
    for entry in changed:
        check(sha(root / entry['path']) == entry['currentSha256'],
              'Same-version update retained previous binary: ' + entry['path'])
    receipt = json.loads((root / 'Plugins/DynastyCOT/installed-files.json').read_text())
    check(receipt['version'] == '1.0.0', 'Update changed the version identity')
    installed = snapshot(root)
    run(root, current)
    check(snapshot(root) == installed, 'Repeat same-version update changes payload/data')

    # Post-update user edits and new publications survive payload rollback.
    later = {
        'Plugins/DynastyCOT/settings.json':
            b'{"format":1,"modules":{"DynastyEnhancer":true,"GamedayAtmosphere":true,"ExtraDynastySettings":false,"DrivePractice":false}}',
        'COT-Runtime/DynastyEnhancer/DynastyEnhancementSettings/1001.json':
            b'{"fixture":"newer user settings after installation"}',
        'COT-Runtime/DynastyEnhancer/RuntimeHistory/1001/new-publication.json':
            b'{"id":"new-post","account":"same-account","body":"Keep this later publication."}',
    }
    for path, data in later.items():
        write(root, path, data)
    run(root, current, extra=('-Rollback', str(upgrade_backup)))
    expected = dict(before)
    expected.update({path: digest(data) for path, data in later.items()})
    check(snapshot(root) == expected, 'Manual rollback differs from old payload plus later user data')
    data_intact(root, dict(DATA, **later))
    for entry in changed:
        check(sha(root / entry['path']) == entry['previousSha256'],
              'Rollback did not restore old 1.0 binary: ' + entry['path'])
    journal = json.loads((upgrade_backup / 'transaction.json').read_text())
    check(journal['status'] == 'rolled_back', 'Manual rollback not committed')

    for fault in ('AfterPayload', 'AfterLegacyDisable', 'AfterAdapter'):
        root = seed('Fault-' + fault)
        before = snapshot(root)
        old_transactions = transactions(root)
        output = run(root, current, success=False, extra=('-FaultAtStage', fault))
        check('Injected fixture failure: ' + fault in output,
              'Failure occurred before requested commit boundary')
        check(snapshot(root) == before, 'Automatic rollback differs: ' + fault)
        data_intact(root)
        added = transactions(root) - old_transactions
        check(len(added) == 1, 'Failure did not record exactly one transaction')
        journal = json.loads((added.pop() / 'transaction.json').read_text())
        check(journal['status'] == 'rolled_back', 'Automatic rollback not committed: ' + fault)

    report = {
        'checks': checks,
        'previous': previous_identity,
        'current': current_identity,
        'changedBinaries': changed,
        'sameVersion': True,
        'faultRollbackStages': 3,
        'manualRollbackRestoresPreviousBinaryHashes': True,
        'choicesSettingsHistoryPreserved': True,
        'laterUserDataPreserved': True,
        'fixtureRoot': str(base),
        'currentStage': str(current),
        'previousArchive': str(previous_archive),
        'liveInstall': False,
        'synthetic': False,
    }
    (base / 'result.json').write_text(json.dumps(report, indent=2) + '\n')
    (ROOT / 'Build/same-version-upgrade-verification.json').write_text(
        json.dumps(report, indent=2) + '\n')
    print(f'PASS: {checks} same-version upgrade checks; '
          f'{len(changed)} distinct binary hashes; fixtures only.')


if __name__ == '__main__':
    main()

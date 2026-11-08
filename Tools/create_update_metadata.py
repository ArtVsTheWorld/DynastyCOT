"""Create credential-free GitHub update metadata from a source-verified ZIP.

Run after finalize_package.py; the metadata sits beside the ZIP, never inside
it, avoiding a checksum cycle. Only the lead publishes these local artifacts.
"""
from pathlib import Path
import hashlib
import json
import re

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    product = json.loads((ROOT / 'cot.product.json').read_text(encoding='utf-8'))
    distribution = product.get('distribution', {})
    repo = distribution.get('repository', '')
    if not re.fullmatch(r'[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+', repo):
        raise ValueError('Set distribution.repository before building update metadata')
    meta = json.loads((ROOT / 'Build/current-package.json').read_text(encoding='utf-8'))
    verification = json.loads(Path(meta['verification']).read_text(encoding='utf-8'))
    archive = Path(meta['archive'])
    version = product['version']
    if verification['version'] != version or sha(archive) != verification['archiveSha256']:
        raise ValueError('Update metadata requires the exact verified source-built archive')
    notes_path = ROOT / distribution.get('releaseNotes', 'docs/release/' + version + '.md')
    notes = notes_path.read_text(encoding='utf-8')
    compatibility = distribution.get('compatibilityNotes', '')
    if not notes.strip() or not compatibility.strip():
        raise ValueError('Release notes and explicit known compatibility information are required')
    payload = dict(format=1, product='DynastyCOT', version=version,
                   repository=repo, visibility=distribution.get('visibility', 'undecided'),
                   releaseNotes=notes, compatibilityNotes=compatibility,
                   releaseNotesSha256=hashlib.sha256(notes.encode()).hexdigest(),
                   compatibilityNotesSha256=hashlib.sha256(compatibility.encode()).hexdigest(),
                   archive=dict(name=archive.name, bytes=archive.stat().st_size,
                                sha256=sha(archive),
                                url='https://github.com/' + repo + '/releases/download/v' + version + '/' + archive.name),
                   liveGameAcceptance=verification.get('liveGameAcceptance', 'pending'))
    destination = ROOT / 'Release/cot-update.json'
    destination.write_text(json.dumps(payload, indent=2) + '\n', encoding='utf-8')
    (ROOT / ('Release/cot-update-' + version + '.json')).write_bytes(destination.read_bytes())
    print('Prepared manual-update metadata:', destination)


if __name__ == '__main__':
    main()

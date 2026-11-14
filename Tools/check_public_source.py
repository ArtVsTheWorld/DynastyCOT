"""Check the staged/public source boundary without opening private directories."""
from pathlib import Path
import json
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
DENIED_PARTS = {'build', 'evidence', 'release', 'research', 'captures', 'saves',
                'runtimehistory', 'recruitresumehistory', 'dynastyenhancementsettings',
                'node_modules', '.git', 'private'}
DENIED_SUFFIXES = {'.dll', '.exe', '.pdb', '.obj', '.ast', '.raw', '.bin', '.sav',
                   '.fbmod', '.fbproject', '.zip', '.htm', '.html', '.png', '.jpg'}
SECRET = re.compile(rb'(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,}|'
                    rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----)')
# These four screenshots were explicitly supplied for the public README.
# Other captures and game assets remain excluded.
README_IMAGES = {'docs/images/beyond-the-field.png','docs/images/gameday-atmosphere.png',
                 'docs/images/expanded-practice.png','docs/images/extra-settings.png'}
# Additional screenshots explicitly supplied for the public player guides.
README_IMAGES.update('docs/player-guides/assets/'+name+'.png' for name in
                     ['BeyondTheField','GamedayAtmosphere','ExpandedPractice','ExtraSettings'])


def check():
    # Include staged files and new nonignored source. This also works on a clone.
    args = ['git', 'ls-files', '--cached', '-z']
    if '--tracked-only' not in sys.argv:
        args += ['--others', '--exclude-standard']
    listed = subprocess.check_output(args, cwd=ROOT)
    paths = sorted(set(p.decode('utf-8') for p in listed.split(b'\0') if p))
    errors = []
    for rel in paths:
        path = Path(rel)
        authored_release_doc = rel.startswith('docs/release/') and path.suffix.lower() == '.md'
        if ((set(x.lower() for x in path.parts) & DENIED_PARTS and not authored_release_doc) or
                (path.suffix.lower() in DENIED_SUFFIXES and rel not in README_IMAGES | {'Packaging/Setup/Assets/Crown.png','Packaging/Setup/Assets/Crown.ico'}) or
                path.name.lower() in {'private-fixtures.json', 'cot.build-inputs.json',
                                     'ulm-close-games-2026.json', 'review-samples.json'}):
            errors.append({'path': rel, 'reason': 'private/generated input in public source'})
            continue
        data = (ROOT / path).read_bytes()
        if SECRET.search(data):
            errors.append({'path': rel, 'reason': 'credential/private-key pattern'})
    result = {'files': len(paths), 'errors': errors}
    print(json.dumps(result, indent=2))
    return bool(errors)


if __name__ == '__main__':
    sys.exit(check())

"""Check that classic layout/style files still match upstream 4.0.1."""
from pathlib import Path
import hashlib
import json
import os

root = Path(os.path.abspath(__file__)).parent.parent
manifest = json.loads((root / 'validation/original-ui-sha256.json').read_text(encoding='utf-8'))
failures = []
for relative, expected in manifest.items():
    actual = hashlib.sha256((root / relative).read_bytes()).hexdigest()
    if actual != expected:
        failures.append(relative)
if failures:
    raise SystemExit('Original UI/style files changed: ' + ', '.join(failures))
print(f'PASS: {len(manifest)} UI/style files match upstream 4.0.1.')

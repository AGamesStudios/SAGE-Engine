#!/usr/bin/env python3
import os
import subprocess
import sys

base = os.environ.get('GITHUB_BASE_REF')
if not base:
    sys.exit(0)

cmd = ['git', 'diff', '--name-only', f'origin/{base}..HEAD']
changed = subprocess.check_output(cmd, text=True)
if 'CHANGELOG.md' not in changed.splitlines():
    print('CHANGELOG.md must be updated for every PR', file=sys.stderr)
    sys.exit(1)


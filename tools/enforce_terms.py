import sys
from pathlib import Path
import re

PROHIBITED = [re.compile(rb'\bclass\s+\w+Agent\b'),
             re.compile(rb'from\s+[^\n]*\.agents[^\n]*'),
             re.compile(rb'/agents/'),
             re.compile(rb'\bagent(s)?\b', re.IGNORECASE)]


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    for path in root.rglob('*'):
        if path.suffix in {'.py', '.md', '.cfg', '.txt'}:
            data = path.read_bytes()
            for patt in PROHIBITED:
                if patt.search(data):
                    print(f'Prohibited term in {path}')
                    return 1
    return 0

if __name__ == '__main__':
    raise SystemExit(main())

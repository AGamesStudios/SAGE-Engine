"""Generate a new SAGE project skeleton."""
from __future__ import annotations

import sys
from pathlib import Path

from sage_engine.project import ensure_layout


def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: new_project.py <folder>")
        return
    target = Path(sys.argv[1]).resolve()
    ensure_layout(str(target))
    print(f"Created project at {target}")


if __name__ == "__main__":
    main()

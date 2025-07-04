from __future__ import annotations

import sys
from importlib import import_module
from pathlib import Path

SRC = Path(__file__).resolve().parent / "src"
if SRC.is_dir() and str(SRC) not in sys.path:
    sys.path.insert(0, str(SRC))


def _load_editor_main():
    """Return the editor's ``main`` function if available."""
    try:  # pragma: no cover - optional dependency
        return import_module("sage_editor").main
    except Exception:  # pragma: no cover - editor not installed
        return None


def main(argv: list[str] | None = None) -> int:
    """Run the editor if installed, otherwise run the engine runtime."""
    argv = argv or sys.argv[1:]
    editor_main = _load_editor_main()
    if editor_main is not None:
        return editor_main(argv)

    from engine.runtime import main as runtime_main

    return runtime_main(argv)


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    raise SystemExit(main())

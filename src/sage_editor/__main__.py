"""Entry point for running the reference editor package."""

try:  # support execution via ``python path/to/__main__.py``
    from . import main
except Exception:  # pragma: no cover - fallback if package not initialised
    from importlib import import_module

    main = import_module("sage_editor").main

if __name__ == "__main__":  # pragma: no cover - CLI entry
    raise SystemExit(main())


import sys

try:
    from sage_editor import main as run
except Exception as exc:  # pragma: no cover - import-time failure
    raise SystemExit(f"Cannot start SAGE Editor: {exc}") from exc

if __name__ == '__main__':
    raise SystemExit(run(sys.argv))

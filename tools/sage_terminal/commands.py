from __future__ import annotations

import shutil
from pathlib import Path

from sage_engine.project import ensure_layout, validate_structure, LAYOUT


def cmd_help(app, *args: str) -> str:
    commands = ' '.join(sorted(COMMAND_MAP))
    return f"Available commands: {commands}"


def cmd_newproject(app, name: str = "") -> str:
    if not name:
        return "Usage: newproject NAME"
    Path(name).mkdir(parents=True, exist_ok=True)
    ensure_layout(name)
    return f"[\u2713] created project at {Path(name).resolve()}"


def cmd_checkproject(app, path: str = ".") -> str:
    missing = validate_structure(path)
    if missing:
        return f"[\u2717] missing: {', '.join(missing)}"
    return "[\u2713] project validated successfully"


def cmd_cleanproject(app, path: str = ".") -> str:
    base = Path(path)
    if not base.exists():
        return "path not found"
    removed = 0
    for p in base.rglob("__pycache__"):
        shutil.rmtree(p, ignore_errors=True)
        removed += 1
    for p in base.rglob("*.pyc"):
        try:
            p.unlink()
            removed += 1
        except OSError:
            pass
    return f"[\u2713] removed {removed} files"


def _new_file(path: Path) -> str:
    if path.exists():
        return "[!] already exists"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("{}\n", encoding="utf-8")
    return f"[\u2713] created {path}"


def cmd_newscene(app, name: str = "") -> str:
    if not name:
        return "Usage: newscene NAME"
    path = Path("data/scenes") / f"{name}.sage_scene"
    return _new_file(path)


def cmd_newobject(app, name: str = "") -> str:
    if not name:
        return "Usage: newobject NAME"
    path = Path("data/objects") / f"{name}.sage_object"
    return _new_file(path)


def cmd_newscript(app, target: str = "", name: str = "") -> str:
    if target not in {"scene", "object"} or not name:
        return "Usage: newscript [scene|object] NAME"
    path = Path("data/scripts") / target / f"{name}.py"
    return _new_file(path)


def cmd_showlayout(app, *args: str) -> str:
    return "\n".join(LAYOUT)


def cmd_exit(app, *args: str) -> str:
    app.close()
    return ""


COMMAND_MAP = {
    "help": cmd_help,
    "newproject": cmd_newproject,
    "checkproject": cmd_checkproject,
    "cleanproject": cmd_cleanproject,
    "newscene": cmd_newscene,
    "newobject": cmd_newobject,
    "newscript": cmd_newscript,
    "showlayout": cmd_showlayout,
    "exit": cmd_exit,
    "quit": cmd_exit,
}


def execute(app, line: str) -> str:
    parts = line.strip().split()
    if not parts:
        return ""
    cmd = parts[0].lower()
    func = COMMAND_MAP.get(cmd)
    if func is None:
        return "unknown command"
    try:
        return func(app, *parts[1:])
    except Exception as exc:  # pragma: no cover - defensive
        return f"[\u2717] {exc}"

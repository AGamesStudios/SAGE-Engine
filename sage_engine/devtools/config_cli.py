"""CLI utilities to manage engine.json."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


ENGINE_JSON = Path("engine.json")


def init_config() -> None:
    if ENGINE_JSON.exists():
        raise SystemExit("engine.json already exists")
    data = {"modules": ["core"], "features": {}}
    ENGINE_JSON.write_text(json.dumps(data, indent=2), encoding="utf8")


def add_module(name: str) -> None:
    if not ENGINE_JSON.exists():
        raise SystemExit("engine.json missing")
    data = json.loads(ENGINE_JSON.read_text(encoding="utf8"))
    mods = set(data.get("modules", []))
    mods.add(name)
    data["modules"] = sorted(mods)
    ENGINE_JSON.write_text(json.dumps(data, indent=2), encoding="utf8")


def validate() -> None:
    if not ENGINE_JSON.exists():
        raise SystemExit("engine.json missing")
    json.loads(ENGINE_JSON.read_text(encoding="utf8"))


def main() -> None:
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd")
    sub.add_parser("--init")
    add = sub.add_parser("--add-module")
    add.add_argument("module")
    sub.add_parser("--validate")
    args = parser.parse_args()
    if args.cmd == "--init":
        init_config()
    elif args.cmd == "--add-module":
        add_module(args.module)
    elif args.cmd == "--validate":
        validate()
    else:
        parser.print_help()


if __name__ == "__main__":  # pragma: no cover
    main()

import argparse
import json
from pathlib import Path

from ..compat import migrate
from ..blueprint import CURRENT_SCHEMA_VERSION as BP_VERSION
from ..world import CURRENT_SCHEMA_VERSION as SCENE_VERSION


def blueprint_migrate(path: Path) -> None:
    data = json.loads(path.read_text(encoding="utf8"))
    version = str(data.get("schema_version", BP_VERSION))
    new_version, new_data = migrate("blueprint", version, BP_VERSION, data)
    if new_version != version:
        print(f"Migrated from {version} -> {new_version}")
        path.write_text(json.dumps(new_data, indent=2), encoding="utf8")
    else:
        print("Already up to date")


def compat_check(path: Path) -> None:
    data = json.loads(path.read_text(encoding="utf8"))
    version = str(data.get("schema_version", data.get("engine_version", SCENE_VERSION)))
    new_version, _ = migrate("scene", version, SCENE_VERSION, dict(data))
    if new_version != version:
        print(f"Needs migration: {version} -> {new_version}")
    else:
        print("Scene is compatible")


def main() -> None:
    parser = argparse.ArgumentParser(prog="sage")
    sub = parser.add_subparsers(dest="topic")

    bp = sub.add_parser("blueprint")
    bp_sub = bp.add_subparsers(dest="cmd")
    mig = bp_sub.add_parser("migrate")
    mig.add_argument("path")

    comp = sub.add_parser("compat")
    comp_sub = comp.add_subparsers(dest="cmd")
    chk = comp_sub.add_parser("check")
    chk.add_argument("path")

    args = parser.parse_args()
    if args.topic == "blueprint" and args.cmd == "migrate":
        blueprint_migrate(Path(args.path))
    elif args.topic == "compat" and args.cmd == "check":
        compat_check(Path(args.path))
    else:
        parser.print_help()


if __name__ == "__main__":  # pragma: no cover
    main()

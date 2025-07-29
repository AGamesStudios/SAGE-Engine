import argparse
import json
from typing import Optional
import yaml
from pathlib import Path
import subprocess
import sys

from ..compat import migrate
from ..format import (
    SAGECompiler,
    SAGEDecompiler,
    SAGESchemaSystem,
    pack_directory,
)
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


def format_compile(src: Path, dst: Path) -> None:
    SAGECompiler().compile(src, dst)


def format_decompile(src: Path, dst: Optional[Path]) -> None:
    data = SAGEDecompiler().decompile(src)
    if dst:
        dst.write_text(yaml.safe_dump(data), encoding="utf8")
    else:
        print(yaml.safe_dump(data))


def convert_file(src: Path, dst: Path) -> None:
    """Convert legacy YAML/JSON/etc. to a SAGE binary file."""
    SAGECompiler().compile(src, dst)


def validate_file(path: Path, schema: Path) -> None:
    system = SAGESchemaSystem()
    spec = yaml.safe_load(schema.read_text(encoding="utf8"))
    system.register("tmp", spec)
    data = SAGEDecompiler().decompile(path)
    system.validate("tmp", data)
    print("OK")


def pack_dir(src: Path, dst: Path) -> None:
    pack_directory(src, dst)


def build_assets_call(path: Path) -> None:
    """Run the build_assets.py helper script."""
    script = Path(__file__).resolve().parents[2] / "tools" / "build_assets.py"
    subprocess.run([sys.executable, str(script), str(path)], check=True)


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

    fmt = sub.add_parser("format")
    fmt_sub = fmt.add_subparsers(dest="cmd")
    fc = fmt_sub.add_parser("compile")
    fc.add_argument("src")
    fc.add_argument("dst")
    fd = fmt_sub.add_parser("decompile")
    fd.add_argument("src")
    fd.add_argument("dst", nargs="?")

    conv = sub.add_parser("convert")
    conv.add_argument("src")
    conv.add_argument("dst")

    val = sub.add_parser("validate")
    val.add_argument("path")
    val.add_argument("--schema", required=True)

    pack = sub.add_parser("pack")
    pack.add_argument("src")
    pack.add_argument("dst")

    build = sub.add_parser("build-assets")
    build.add_argument("path", nargs="?", default=".")

    args = parser.parse_args()
    if args.topic == "blueprint" and args.cmd == "migrate":
        blueprint_migrate(Path(args.path))
    elif args.topic == "compat" and args.cmd == "check":
        compat_check(Path(args.path))
    elif args.topic == "format" and args.cmd == "compile":
        format_compile(Path(args.src), Path(args.dst))
    elif args.topic == "format" and args.cmd == "decompile":
        format_decompile(Path(args.src), Path(args.dst) if args.dst else None)
    elif args.topic == "convert":
        convert_file(Path(args.src), Path(args.dst))
    elif args.topic == "validate":
        validate_file(Path(args.path), Path(args.schema))
    elif args.topic == "pack":
        pack_dir(Path(args.src), Path(args.dst))
    elif args.topic == "build-assets":
        build_assets_call(Path(args.path))
    else:
        parser.print_help()


if __name__ == "__main__":  # pragma: no cover
    main()

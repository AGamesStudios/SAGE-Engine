from __future__ import annotations

import argparse
import shutil
from pathlib import Path

from engine.utils import TraceProfiler


def _build(args: argparse.Namespace) -> None:
    profiler = TraceProfiler(args.profile) if args.profile else None
    if profiler:
        with profiler.phase("Input"):
            pass
        with profiler.phase("Patchers"):
            pass
        with profiler.phase("Merge"):
            pass
        with profiler.phase("Render"):
            pass
        profiler.write()


def _serve(args: argparse.Namespace) -> None:
    profiler = TraceProfiler(args.profile) if args.profile else None
    if profiler:
        with profiler.phase("Input"):
            pass
        with profiler.phase("Patchers"):
            pass
        with profiler.phase("Merge"):
            pass
        with profiler.phase("Render"):
            pass
        profiler.write()


def _featherize(args: argparse.Namespace) -> None:
    profiler = TraceProfiler(args.profile) if args.profile else None
    if profiler:
        with profiler.phase("Input"):
            pass
        with profiler.phase("Patchers"):
            pass
        with profiler.phase("Merge"):
            pass
        with profiler.phase("Render"):
            pass
        profiler.write()


def _create(args: argparse.Namespace) -> None:
    templates = {
        "platformer": Path(__file__).resolve().parent / "minimal_platformer",
        "hello_sprite_py": Path(__file__).resolve().parent / "hello_sprite_py",
    }
    src = templates.get(args.template)
    if not src:
        raise SystemExit(f"unknown template: {args.template}")
    dest = Path(args.name)
    if dest.exists():
        raise SystemExit(f"path exists: {dest}")
    shutil.copytree(src, dest)





_COMMANDS = {
    "build": _build,
    "serve": _serve,
    "featherize": _featherize,
    "create": _create,
}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="sage", description="SAGE command line")
    parser.add_argument("--profile", nargs="?", const="trace.json", help="write Chrome Trace JSON")
    sub = parser.add_subparsers(dest="cmd", required=True)
    sub.add_parser("build")
    sub.add_parser("serve")
    sub.add_parser("featherize")
    create_p = sub.add_parser("create")
    create_p.add_argument("name")
    create_p.add_argument("-t", "--template", default="platformer")
    args = parser.parse_args(argv)
    _COMMANDS[args.cmd](args)
    return 0


if __name__ == "__main__":  # pragma: no cover - entry point
    raise SystemExit(main())

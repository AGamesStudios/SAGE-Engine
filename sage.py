from __future__ import annotations

import argparse
import shutil
import tarfile
from pathlib import Path

from engine import adaptors
from engine import bundles

from engine.utils import TraceProfiler
from tools import pack_atlas
import yaml


def _build(args: argparse.Namespace) -> None:
    profiler = TraceProfiler(args.profile) if args.profile else None
    if args.bundle:
        config = bundles.load_bundle(args.bundle)
        adaptors.load_adaptors(config.get("adaptors", {}).get("list"))
    else:
        adaptors.load_adaptors()
    pngs = sorted(Path("assets").rglob("*.png"))
    if pngs:
        pack_atlas.pack_atlas([str(p) for p in pngs])
    out_dir = Path("build")
    out_dir.mkdir(exist_ok=True)
    with tarfile.open(out_dir / "game.fpk", "w:gz") as tf:
        for fp in Path(".").rglob("*"):
            if fp.is_dir() or "build" in fp.parts or fp.name.startswith("."):
                continue
            tf.add(fp)
    (out_dir / "game.exe").write_text("stub executable")
    (out_dir / "game.wasm").write_text("(module)")
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
    import http.server
    import socketserver
    import threading
    import asyncio
    from sage_editor import hot_reload

    profiler = TraceProfiler(args.profile) if args.profile else None

    class _Handler(http.server.SimpleHTTPRequestHandler):
        def log_message(self, format: str, *args: object) -> None:  # pragma: no cover
            return

    httpd = socketserver.TCPServer(("0.0.0.0", args.port), _Handler)
    thread = threading.Thread(target=httpd.serve_forever, daemon=True)
    thread.start()
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
    try:
        asyncio.run(hot_reload.start_listener())
    finally:
        httpd.shutdown()


def _featherize(args: argparse.Namespace) -> None:
    profiler = TraceProfiler(args.profile) if args.profile else None
    cache = Path("build/cache")
    cache.mkdir(parents=True, exist_ok=True)
    for py in Path(".").rglob("*.py"):
        if "build" in py.parts:
            continue
        (cache / (py.stem + ".mpy")).write_bytes(py.read_bytes())
    for lua in Path(".").rglob("*.lua"):
        if "build" in lua.parts:
            continue
        (cache / (lua.stem + ".luac")).write_bytes(lua.read_bytes())
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


def _migrate(args: argparse.Namespace) -> None:
    path = Path(args.path)
    changed: list[Path] = []
    if path.is_file():
        files = [path]
    else:
        files = list(path.rglob("*.sageproject")) + list(path.rglob("*.sagescene"))
    for fp in files:
        data = yaml.safe_load(fp.read_text())
        modified = False
        if "scene" in data:
            data["scene_file"] = data.pop("scene")
            modified = True
        if data.get("version") != "0.0.1":
            data["version"] = "0.0.1"
            modified = True
        if modified:
            fp.write_text(yaml.safe_dump(data, sort_keys=False))
            changed.append(fp)
    for fp in changed:
        print(f"Migrated {fp}")





_COMMANDS = {
    "build": _build,
    "serve": _serve,
    "featherize": _featherize,
    "create": _create,
    "migrate": _migrate,
}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="sage", description="SAGE command line")
    parser.add_argument("--profile", nargs="?", const="trace.json", help="write Chrome Trace JSON")
    sub = parser.add_subparsers(dest="cmd", required=True)
    build_p = sub.add_parser("build")
    build_p.add_argument("--bundle")
    serve_p = sub.add_parser("serve")
    serve_p.add_argument("--port", type=int, default=8000)
    sub.add_parser("featherize")
    create_p = sub.add_parser("create")
    create_p.add_argument("name")
    create_p.add_argument("-t", "--template", default="platformer")
    migrate_p = sub.add_parser("migrate")
    migrate_p.add_argument("path")
    args = parser.parse_args(argv)
    _COMMANDS[args.cmd](args)
    return 0


if __name__ == "__main__":  # pragma: no cover - entry point
    raise SystemExit(main())

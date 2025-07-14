from __future__ import annotations

import argparse
import shutil
import tarfile
from pathlib import Path
import hashlib
import json
import time
import asyncio
import signal
import threading

import websockets

from engine import adaptors
from engine import bundles

from engine.utils import TraceProfiler
from engine.utils.log import logger
from tools import pack_atlas, sound_mint
import yaml


async def _notify_reload() -> None:
    """Notify running clients to reload textures."""
    try:
        async with websockets.connect("ws://localhost:8765") as ws:
            await ws.send("toast textures updated")
    except Exception:
        pass


def _build(args: argparse.Namespace) -> None:
    profiler = TraceProfiler(args.profile) if args.profile else None
    selected = None
    if args.bundle:
        config = bundles.load_bundle(args.bundle)
        selected = config.get("adaptors", {}).get("list")
        adaptors.load_adaptors(selected)
    else:
        adaptors.load_adaptors()
    if args.bundle == "windows":
        for ogg in Path("assets").rglob("*.ogg"):
            try:
                mp3 = sound_mint.ogg_to_mp3(str(ogg))
                shutil.copy(mp3, ogg.with_suffix(".mp3"))
            except Exception:
                logger.warning("failed to convert %s", ogg)
    pngs = sorted(Path("assets").rglob("*.png"))
    if pngs:
        pack_atlas.pack_atlas([str(p) for p in pngs])
    out_dir = Path("build")
    out_dir.mkdir(exist_ok=True)
    manifest: dict[str, str] = {}
    with tarfile.open(out_dir / "game.fpk", "w:gz") as tf:
        for fp in Path(".").rglob("*"):
            if fp.is_dir() or "build" in fp.parts or fp.name.startswith("."):
                continue
            if selected and "sage_adaptors" in fp.parts:
                idx = fp.parts.index("sage_adaptors")
                if fp.parts[idx + 1] not in selected:
                    continue
            if args.bundle == "windows" and fp.suffix == ".ogg":
                continue
            tf.add(fp)
            manifest[str(fp)] = hashlib.sha256(fp.read_bytes()).hexdigest()
    with open(out_dir / "manifest.json", "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2)
    (out_dir / "game.exe").write_text("stub executable")
    (out_dir / "game.wasm").write_text("(module)")
    (out_dir / "index.html").write_text(
        "<html><body><script src='game.wasm'></script></body></html>"
    )
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
    from sage_editor import hot_reload

    profiler = TraceProfiler(args.profile) if args.profile else None

    class _Handler(http.server.SimpleHTTPRequestHandler):
        def log_message(self, format: str, *args: object) -> None:  # pragma: no cover
            return

    httpd = socketserver.TCPServer(("0.0.0.0", args.port), _Handler)

    def run_server() -> None:
        httpd.serve_forever()

    thread = threading.Thread(target=run_server, daemon=True)
    thread.start()

    stop = threading.Event()

    def watch_assets() -> None:
        last = 0.0
        while not stop.is_set():
            files = list(Path("assets").rglob("*.png")) + list(Path("assets").rglob("*.ogg"))
            cur = max((f.stat().st_mtime for f in files), default=0.0)
            if cur != last:
                pngs = [str(p) for p in Path("assets").rglob("*.png")]
                if pngs:
                    try:
                        pack_atlas.pack_atlas(pngs)
                        asyncio.run(_notify_reload())
                    except Exception:
                        pass
                last = cur
            time.sleep(1.0)

    watcher = threading.Thread(target=watch_assets, daemon=True)
    watcher.start()

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

    def handle_sig(signum, frame) -> None:  # pragma: no cover - signal handler
        stop.set()

    signal.signal(signal.SIGINT, handle_sig)

    async def ws_main() -> None:
        task = asyncio.create_task(hot_reload.start_listener())
        while not stop.is_set() and not task.done():
            await asyncio.sleep(0.1)
        stop.set()
        if not task.done():
            task.cancel()
            try:
                await task
            except asyncio.CancelledError:
                pass

    try:
        asyncio.run(ws_main())
    finally:
        stop.set()
        httpd.shutdown()
        thread.join()


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
